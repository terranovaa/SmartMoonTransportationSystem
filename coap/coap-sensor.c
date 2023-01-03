/*
 * Copyright (c) 2020, Carlo Vallati, University of Pisa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os/sys/log.h"
#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "os/sys/etimer.h"
#include "os/net/ipv6/uip-ds6.h"
#include "os/dev/leds.h"
#include "coap-log.h"
#include "dev/button-hal.h"

#include "../sensing/temperature/temperature.h"
#include "../sensing/dust/dust.h"
#include "../sensing/regolith/regolith.h"
#include "./resources/temperature/res-temperature.h"
#include "./resources/dust/res-dust.h"
#include "./resources/regolith/res-regolith.h"

#define LOG_MODULE "coap-sensor"
#define LOG_LEVEL LOG_LEVEL_APP

// States list
#define COAP_INIT  0
#define COAP_STARTED 1
#define COAP_REGISTERED 2

// Server IP
#define SERVER "coap://[fd00::1]:5683"
char *service_url = "/registration";

// COAP resources
extern coap_resource_t res_temperature;
extern coap_resource_t res_dust;
extern coap_resource_t res_regolith;

static bool temp_error;
static bool dust_error;
static bool regolith_error;

static uint8_t state;
static int node_id;

// Function passed to COAP_BLOCKING_REQUEST() to handle responses
void client_chunk_handler(coap_message_t *response){
  	if(response == NULL) {
    		puts("Request timed out");
    		return;
  	}
 	state = COAP_REGISTERED;
	LOG_INFO("Received response, moving to the COAP_REGISTERED state..\n");
	leds_off(LEDS_ALL);
}

static bool have_connectivity(void){
    	if(uip_ds6_get_global(ADDR_PREFERRED) == NULL || uip_ds6_defrt_choose() == NULL) {
        	return false;
    	}
    	return true;
}

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

PROCESS(coap_sensor, "CoAP Sensor");
AUTOSTART_PROCESSES(&coap_sensor);

PROCESS_THREAD(coap_sensor, ev, data){
	static coap_endpoint_t server;
	static coap_message_t request[1]; // array of 1 so this way the packet can be treated as pointer as usual	
	PROCESS_BEGIN();
	
  	state = COAP_INIT;
	LOG_INFO("Started COAP node with id %d\n", node_id);
  	node_id = linkaddr_node_addr.u8[7];

	// populate the data structure with the URL of the server
  	coap_endpoint_parse(SERVER, strlen(SERVER), &server);

  	// prepare the message and set method to be called
  	coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
  	coap_set_header_uri_path(request, service_url);
	leds_on(LEDS_ALL);
	// Temperature resource
	res_temperature_start();
  	process_start(&temp_sensor_process, NULL);
  	process_post(&temp_sensor_process, TEMP_SUB_EVENT, &coap_sensor);
	
	// Dust resource
	res_dust_start();
  	process_start(&dust_sensor_process, NULL);
  	process_post(&dust_sensor_process, DUST_SUB_EVENT, &coap_sensor);
	
	// Regolith resource
	res_regolith_start();
 	process_start(&regolith_sensor_process, NULL);
  	process_post(&regolith_sensor_process, REGOLITH_SUB_EVENT, &coap_sensor);
	
	LOG_INFO("Node in the COAP_INIT state\n");

	while(1) {
    		PROCESS_WAIT_EVENT();
    		if(state==COAP_INIT){
        		if(have_connectivity()==true){
            			state = COAP_STARTED;
				LOG_INFO("Found connectivity. Moving to the COAP_STARTED state..\n");
			}
     		}
     		if(state == COAP_STARTED){
			LOG_INFO("Sending a blocking request to the server..\n");
			COAP_BLOCKING_REQUEST(&server, request, client_chunk_handler);
			// move to COAP_REGISTERED when the response arrives
			
     		}
		if(state == COAP_REGISTERED){
     			if(ev == TEMP_SAMPLE_EVENT || ev == DUST_SAMPLE_EVENT || ev == REGOLITH_SAMPLE_EVENT){
        			int sample= *((int *)data);
				if (ev == TEMP_SAMPLE_EVENT){
	    				LOG_INFO("New temperature measurement of value: %d.\n", sample);
	    				res_temperature_update(node_id,sample);
					temp_error = res_temperature_alarm();
					// no need to actuate on the moon
				} else if (ev == DUST_SAMPLE_EVENT) {
	    				LOG_INFO("New dust measurement of value: %d.\n", sample);
	    				res_dust_update(node_id,sample);
					dust_error = res_dust_alarm();
					process_post(&dust_sensor_process, DUST_ALARM_EVENT, (int*)dust_error);
				} else if (ev == REGOLITH_SAMPLE_EVENT) {
	    				LOG_INFO("New regolith measurement of value: %d.\n", sample);
	    				res_regolith_update(node_id,sample);
					regolith_error = res_regolith_alarm();
					process_post(&regolith_sensor_process, REGOLITH_ALARM_EVENT, (int*)regolith_error);
 				}
     			} else if(ev == button_hal_press_event && regolith_error){
				LOG_INFO("Button pressed while in regolith error, sending the REGOLITH_ASTRONAUT_EVENT");
				regolith_error = false;
				res_regolith_alarm_disable();
				process_post(&regolith_sensor_process, REGOLITH_ASTRONAUT_EVENT, (int*)regolith_error);
   	 		}
			update_leds();	
  		}
	}
  	PROCESS_END();
}

