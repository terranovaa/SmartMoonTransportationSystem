#include <stdbool.h>
#include <stdio.h>
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/log.h"
#include "./regolith.h"
#include "../../utils/utils.h"

#define LOG_MODULE "sensor"
#define LOG_LEVEL LOG_LEVEL_APP

process_event_t REGOLITH_SAMPLE_EVENT;
process_event_t REGOLITH_SUB_EVENT;
process_event_t REGOLITH_ALARM_EVENT;
process_event_t REGOLITH_ASTRONAUT_EVENT;

PROCESS(regolith_sensor_process, "Regolith sensor process");

PROCESS_THREAD(regolith_sensor_process, ev, data){
    static struct etimer et;
    static struct process *subscriber;
    static bool alarm;
    static int sample;
    
    PROCESS_BEGIN();

    alarm = false;
    subscriber = (struct process *)data;

    LOG_INFO("Regolith sensor process started...\n");

    REGOLITH_SAMPLE_EVENT = process_alloc_event();
    REGOLITH_ALARM_EVENT = process_alloc_event();
    REGOLITH_ASTRONAUT_EVENT = process_alloc_event();

    PROCESS_WAIT_EVENT_UNTIL(ev == REGOLITH_SUB_EVENT);
    etimer_set(&et, CLOCK_SECOND*REGOLITH_SAMPLING_INTERVAL);
    sample = (int)rand_sample_range(REGOLITH_LOWER_BOUND, REGOLITH_UPPER_BOUND);
  
    while(true) {
        PROCESS_YIELD();
        if(etimer_expired(&et)){
	    if(!alarm){
            	sample = (int)rand_sample_variation_range(sample, REGOLITH_MAX_VARIATION, REGOLITH_LOWER_BOUND, REGOLITH_UPPER_BOUND);
	    } else {
		/* use MAX_VARIATION = 1 in order to simulate a state with lower variability, and avoid situations where the alarm comes and go, allowing the manual fix to be performed */ 
		sample = (int)rand_sample_variation_range(sample, 1, REGOLITH_LOWER_BOUND, REGOLITH_UPPER_BOUND);
	    }
	    /* present and known only in the regolith case, since determined apriori
 	     based on the capacity of the transportation system */
	    if(sample >= REGOLITH_MAX_THRESHOLD)
		alarm = true;
            process_post(subscriber, REGOLITH_SAMPLE_EVENT, &sample);
            etimer_reset(&et);
        } else if(ev == REGOLITH_ALARM_EVENT){
	    // set or disable the alarm
	    alarm = data;
        } else if(ev == REGOLITH_ASTRONAUT_EVENT){
	    // reset the alarm and reset to a value in the normal range
            alarm = false;
	    sample = (int)rand_sample_range(REGOLITH_LOWER_BOUND, REGOLITH_MAX_THRESHOLD);
 	    process_post(subscriber, REGOLITH_SAMPLE_EVENT, &sample);
            etimer_reset(&et);
        }
	
    }
    PROCESS_END();
}
