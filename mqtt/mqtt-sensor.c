#include <string.h>
#include <strings.h>

#include "contiki.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "os/sys/log.h"

#include "mqtt-sensor.h"
#include "../sensing/temperature/temperature.h"
#include "../sensing/dust/dust.h"
#include "../sensing/regolith/regolith.h"
#include "../utils/utils.h"

#define LOG_MODULE "mqtt-sensor"
#ifdef MQTT_SENSOR_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_SENSOR_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_DBG
#endif

#define MQTT_TEMP_OK                   "1"
#define MQTT_DUST_OK                   "2"
#define MQTT_REGOLITH_OK               "3"
#define MQTT_TEMP_ERROR                "-1"
#define MQTT_DUST_ERROR                "-2"
#define MQTT_REGOLITH_ERROR            "-3"

static bool temp_error;
static bool dust_error;
static bool regolith_error;

#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1" // MQTT broker address
static const char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;
// Assumption: broker does not require authentication

#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_PUBLISH_INTERVAL    (30 * CLOCK_SECOND)

#define MAX_TCP_SEGMENT_SIZE    32  // maximum TCP segment size for outgoing segments of our socket
#define CONFIG_IP_ADDR_STR_LEN   64

static struct mqtt_message *msg_ptr = 0; // data structure to be used to decode a received message
static struct mqtt_connection conn; // data structure storing the connection status

// data structure for client ID and topics
#define BUFFER_SIZE 64
static char client_id[BUFFER_SIZE];
static char temp_topic[BUFFER_SIZE];
static char dust_topic[BUFFER_SIZE];
static char regolith_topic[BUFFER_SIZE];
static char sub_topic[BUFFER_SIZE];

// application level buffers
#define APP_BUFFER_SIZE 256
static char temp_buffer[APP_BUFFER_SIZE];
static char dust_buffer[APP_BUFFER_SIZE];
static char regolith_buffer[APP_BUFFER_SIZE];

#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)
static struct etimer periodic_timer; // periodic timer to check the state of the MQTT client

// set of possible states
#define STATE_INIT    	      0 // initial state
#define STATE_NET_OK          1 // network initialized
#define STATE_CONNECTING      2 // connecting to the MQTT broker
#define STATE_CONNECTED       3 // succesfull connection
#define STATE_SUBSCRIBED      4 // suscribed to the topic of interest
#define STATE_DISCONNECTED    5 // disconnected from the MQTT broker
static uint8_t state;

static int node_id; // to be set to the portion of the linkaddr

PROCESS_NAME(mqtt_sensor_process);
AUTOSTART_PROCESSES(&mqtt_sensor_process);
PROCESS(mqtt_sensor_process, "MQTT Sensor");

static void update_leds(void){
	// handle leds
	if (!dust_error && !regolith_error && !temp_error) {
                leds_off(LEDS_ALL);
               	leds_on(LEDS_GREEN);
        } else {
		leds_off(LEDS_ALL);
               	leds_on(LEDS_RED);
	}
}

static void pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk, uint16_t chunk_len){
	if(strcmp(topic, sub_topic) == 0) {
	 	if(strcmp((const char *)chunk, MQTT_TEMP_ERROR) == 0 && !temp_error){
            		LOG_INFO("!!! Received temperature error..\n");
	    		temp_error = true;
	    		// we do not send any information to the sensor since we cannot perform any acuating operation on the moon
        	} else if (strcmp((const char *)chunk, MQTT_TEMP_OK) == 0){
			LOG_INFO("!!! Temperature error solved..\n");
			temp_error = false;
			// we do not send any information to the sensor since we cannot perform any acuating operation on the moon
        	} else if (strcmp((const char *)chunk, MQTT_DUST_ERROR) == 0 && !dust_error){
            		LOG_INFO("!!! Received dust error..\n");
	    		dust_error = true;
			// activate dust mitigation system
	    		process_post(&dust_sensor_process, DUST_ALARM_EVENT, (int*)true);
        	} else if (strcmp((const char *)chunk, MQTT_DUST_OK) == 0){
            		LOG_INFO("!!! Dust error solved..\n");
	    		dust_error = false;
			// deactivate dust mitigation system
            		process_post(&dust_sensor_process, DUST_ALARM_EVENT, (int*)false);
        	} else if (strcmp((const char *)chunk, MQTT_REGOLITH_ERROR) == 0 && !regolith_error){
            		LOG_INFO("!!! Received regolith error..\n");
	    		regolith_error = true;
			// simulate lower variability status in order to wait for the astronaut to handle the regolith problem
	    		process_post(&regolith_sensor_process, REGOLITH_ALARM_EVENT, (int*)true);
        	} else if (strcmp((const char *)chunk, MQTT_REGOLITH_OK)==0){
            		LOG_INFO("!!! Regolith error solved..\n");
	    		regolith_error = false;
			// deactivate alarm since the regolith problem has been solved without astronaut's intervent
            		process_post(&regolith_sensor_process, REGOLITH_ALARM_EVENT, (int*)false);
        	}
		update_leds();
    	}
}

static void publish(char* topic, char* buffer){
	int status = mqtt_publish(&conn, NULL, topic, (uint8_t *)buffer, strlen(buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
	switch(status) {
	    	case MQTT_STATUS_OK:
	        	return;
	        case MQTT_STATUS_NOT_CONNECTED_ERROR: {
	            LOG_ERR("Publishing failed. Error: MQTT_STATUS_NOT_CONNECTED_ERROR.\n");
	            state = STATE_DISCONNECTED;
	            return;
	        }
	        case MQTT_STATUS_OUT_QUEUE_FULL: {
	            return;
	        }
	        default:
	            LOG_ERR("Publishing failed. Error: unknown.\n");
	            return;
	}
}

static void mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data){
 	switch(event) {
  		case MQTT_EVENT_CONNECTED: {
    			printf("Connection completed..\n");
    			state = STATE_CONNECTED;
			leds_off(LEDS_ALL);
    			break;
  		}
  		case MQTT_EVENT_DISCONNECTED: {
    			printf("MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));
    			state = STATE_DISCONNECTED;
    			process_poll(&mqtt_sensor_process);
    			break;
  		}
  		case MQTT_EVENT_PUBLISH: {
    			msg_ptr = data;
    			pub_handler(msg_ptr->topic, strlen(msg_ptr->topic), msg_ptr->payload_chunk, msg_ptr->payload_length);
    			break;
  		}
 		case MQTT_EVENT_SUBACK: {
    		#if MQTT_311
    			mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;
    			if(suback_event->success) {
      				printf("Application is subscribed to topic successfully\n");
    			} else {
      				printf("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
    			}
		#else
    			printf("Application is subscribed to topic successfully\n");
		#endif
    			break;
  		}
  		case MQTT_EVENT_UNSUBACK: {
    			printf("Application is unsubscribed to topic successfully\n");
    			break;
  		}
  		case MQTT_EVENT_PUBACK: {
    			printf("Publishing complete.\n");
    			break;
  		}
  		default:
    			printf("Application got a unhandled MQTT event: %i\n", event);
    			break;
    	}
}

static bool have_connectivity(void){
  	if(uip_ds6_get_global(ADDR_PREFERRED) == NULL || uip_ds6_defrt_choose() == NULL) {
    		return false;
  	}
 	return true;
}

PROCESS_THREAD(mqtt_sensor_process, ev, data){
	PROCESS_BEGIN();
	mqtt_status_t status;
	char broker_address[CONFIG_IP_ADDR_STR_LEN];
  	node_id = linkaddr_node_addr.u8[7];
	LOG_INFO("MQTT Sensor Process\nID: %d", node_id);
  
  	// initialize the ClientID as MAC address
  	snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

	// broker registration					 
  	mqtt_register(&conn, &mqtt_sensor_process, client_id, mqtt_event, MAX_TCP_SEGMENT_SIZE);
		  
  	state=STATE_INIT;
		    
  	// start all sensors and post SUB event
  	process_start(&temp_sensor_process, NULL);
  	process_post(&temp_sensor_process, TEMP_SUB_EVENT, &mqtt_sensor_process);
  	process_start(&dust_sensor_process, NULL);
  	process_post(&dust_sensor_process, DUST_SUB_EVENT, &mqtt_sensor_process);
  	process_start(&regolith_sensor_process, NULL);
  	process_post(&regolith_sensor_process, REGOLITH_SUB_EVENT, &mqtt_sensor_process);

	leds_on(LEDS_ALL);
  	// initialize periodic timer to check the status 
  	etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);

  	// Main loop
  	while(1) {
		PROCESS_YIELD();
		if((ev == PROCESS_EVENT_TIMER && data == &periodic_timer) || ev == PROCESS_EVENT_POLL){	  		
			// states management			  
		  	if(state==STATE_INIT){
				// check for connectivity
			 	if(have_connectivity()==true){
				 	state = STATE_NET_OK;
					LOG_INFO("Connectivity found, going towards STATE_NET_OK\n");
				}	
		  	} 
			if(state == STATE_NET_OK){
				// attempt the connection to the MQTT server
				 LOG_INFO("Connecting to the MQTT server...\n");
				 memcpy(broker_address, broker_ip, strlen(broker_ip));
				 mqtt_connect(&conn, broker_address, DEFAULT_BROKER_PORT,
							   (DEFAULT_PUBLISH_INTERVAL * 3) / CLOCK_SECOND,
							   MQTT_CLEAN_SESSION_ON);
				 state = STATE_CONNECTING;
			}
			if(state==STATE_CONNECTED){
				// when connected to the broker, we subscribe to the alarm topic
				char topic[16];
        			sprintf(topic, "alarm/%d", node_id);
        			strcpy(sub_topic,topic);
				status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);
				LOG_INFO("Subscribing to the alarm/%d topic...\n", node_id);
				if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
					LOG_ERR("Tried to subscribe but command queue was full!\n");
					PROCESS_EXIT();
				}	
				// topics where the sensor will publish			
				strcpy(temp_topic, "temperature");
  				strcpy(dust_topic, "dust");
				strcpy(regolith_topic, "regolith");		  
				state = STATE_SUBSCRIBED;
		 	} 
		 	if(state == STATE_DISCONNECTED){
		  	 	LOG_ERR("Disconnected from the MQTT broker...\nComing back to the initial state..\n");	
		 	 	state = STATE_INIT;
		 	}
			 etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);
    		} else if (state == STATE_SUBSCRIBED && ev == TEMP_SAMPLE_EVENT){
			// new temperature sample and able to publish
			LOG_INFO("New temperature sample of value %d...\n", *((int *)data));
			buffer_json_message(temp_buffer, APP_BUFFER_SIZE, node_id, "temperature", *((int *)data), "C");
			publish(temp_topic, temp_buffer);
	      	 } else if(state == STATE_SUBSCRIBED && ev == DUST_SAMPLE_EVENT){
			// new dust sample and able to publish
			LOG_INFO("New dust sample of value %d...\n", *((int *)data));
			buffer_json_message(dust_buffer, APP_BUFFER_SIZE, node_id, "dust", *((int *)data), "mg/m^3");
			publish(dust_topic, dust_buffer);
		} else if(state == STATE_SUBSCRIBED && ev == REGOLITH_SAMPLE_EVENT){
			// new regolith sample and able to publish
			LOG_INFO("New regolith sample of value %d...\n", *((int *)data));
			buffer_json_message(regolith_buffer, APP_BUFFER_SIZE, node_id, "regolith", *((int *)data), "KG");
			publish(regolith_topic, regolith_buffer);
		} else if(state == STATE_SUBSCRIBED && ev == button_hal_press_event){
			// button pressed, astronaut manually handled the regolith problem
			LOG_INFO("Button pressed. Sending the astronaut event..\n");
			regolith_error = false;
		   	process_post(&regolith_sensor_process, REGOLITH_ASTRONAUT_EVENT, false);
		}
		if(state == STATE_SUBSCRIBED){
			update_leds();
		}
 	}
	PROCESS_END();
}
