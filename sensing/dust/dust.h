#ifndef DUST_H_
#define DUST_H_

#define DUST_SAMPLING_INTERVAL   13
#define DUST_UPPER_BOUND        100
#define DUST_LOWER_BOUND        0
#define DUST_MAX_VARIATION	30
#define DUST_MITIGATION_VARIATION	5	

PROCESS_NAME(dust_sensor_process);
	
extern process_event_t DUST_SAMPLE_EVENT;
extern process_event_t DUST_SUB_EVENT;
extern process_event_t DUST_ALARM_EVENT;

#endif 
