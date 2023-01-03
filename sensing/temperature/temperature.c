#include <stdbool.h>
#include <stdio.h>
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "./temperature.h"
#include "../../utils/utils.h"

#define LOG_MODULE "sensor"
#define LOG_LEVEL LOG_LEVEL_APP

process_event_t TEMP_SAMPLE_EVENT;
process_event_t TEMP_SUB_EVENT;

PROCESS(temp_sensor_process, "Temperature sensor process");

PROCESS_THREAD(temp_sensor_process, ev, data){
    static struct etimer et;
    static struct process *subscriber;
    static int sample;
    
    PROCESS_BEGIN();
    subscriber = (struct process *)data;

    LOG_INFO("Temperature sensor process started...\n");

    TEMP_SAMPLE_EVENT = process_alloc_event();
    PROCESS_WAIT_EVENT_UNTIL(ev == TEMP_SUB_EVENT);
    etimer_set(&et, CLOCK_SECOND*TEMP_SAMPLING_INTERVAL);
    sample = (int)rand_sample_range(TEMP_LOWER_BOUND, TEMP_UPPER_BOUND);
    
    while(true) {
        PROCESS_YIELD();
        if(etimer_expired(&et)){
	    /* regardless the possibility of an alert in case of too high/low temperature
		we don't have the possibility of actuating the moon environment */
	    sample = (int)rand_sample_variation_range(sample, TEMP_MAX_VARIATION, TEMP_LOWER_BOUND, TEMP_UPPER_BOUND);
	    process_post(subscriber, TEMP_SAMPLE_EVENT, &sample);
            etimer_reset(&et);
        }
    }
    PROCESS_END();
}
