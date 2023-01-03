#ifndef REGOLITH_H_
#define REGOLITH_H_

#define REGOLITH_SAMPLING_INTERVAL   7
#define REGOLITH_LOWER_BOUND        0
#define REGOLITH_UPPER_BOUND	1000
#define REGOLITH_MAX_VARIATION	300
#define REGOLITH_MAX_THRESHOLD  300

PROCESS_NAME(regolith_sensor_process);
	
extern process_event_t REGOLITH_SAMPLE_EVENT;
extern process_event_t REGOLITH_SUB_EVENT;
extern process_event_t REGOLITH_ALARM_EVENT;
extern process_event_t REGOLITH_ASTRONAUT_EVENT;

#endif 
