#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#define TEMP_SAMPLING_INTERVAL   25
/* the range [-130,120] is not used since it represents extreme positions in the Moon.
   We use a smaller range [-20,80] in order to deal with temperaure errors only in a smaller
   amount of cases, also because we do not have the possibility to actuate on it.
*/
#define TEMP_UPPER_BOUND        80
#define TEMP_LOWER_BOUND        -20
#define TEMP_MAX_VARIATION	8

PROCESS_NAME(temp_sensor_process);
	
extern process_event_t TEMP_SAMPLE_EVENT;
extern process_event_t TEMP_SUB_EVENT;

#endif 
