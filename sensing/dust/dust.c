#include <stdbool.h>
#include <stdio.h>
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "./dust.h"
#include "../../utils/utils.h"

#define LOG_MODULE "sensor"
#define LOG_LEVEL LOG_LEVEL_APP

process_event_t DUST_SAMPLE_EVENT;
process_event_t DUST_SUB_EVENT;
process_event_t DUST_ALARM_EVENT;

PROCESS(dust_sensor_process, "Dust sensor process");

PROCESS_THREAD(dust_sensor_process, ev, data){
    static struct etimer et;
    static struct process *subscriber;
    static bool alarm;
    static int sample;

    PROCESS_BEGIN();

    alarm = false;
    subscriber = (struct process *)data;
   
    LOG_INFO("Dust sensor process started...\n");

    DUST_SAMPLE_EVENT = process_alloc_event();
    DUST_ALARM_EVENT = process_alloc_event();

    PROCESS_WAIT_EVENT_UNTIL(ev == DUST_SUB_EVENT);

    etimer_set(&et, CLOCK_SECOND*DUST_SAMPLING_INTERVAL);
    sample = (int)rand_sample_range(DUST_LOWER_BOUND, DUST_UPPER_BOUND); 
  
    while(true) {
        PROCESS_YIELD();
        if(etimer_expired(&et)){
            if(!alarm){
		sample = (int)rand_sample_variation_range(sample, DUST_MAX_VARIATION, DUST_LOWER_BOUND, DUST_UPPER_BOUND);
            } else{
		// dust mitigation system
		sample -= DUST_MITIGATION_VARIATION;
            }
            process_post(subscriber, DUST_SAMPLE_EVENT, &sample);
            etimer_reset(&et);
        } else if(ev == DUST_ALARM_EVENT){
	    // set or disable the alarm
            alarm = data;
        }
    }
    PROCESS_END();
}
