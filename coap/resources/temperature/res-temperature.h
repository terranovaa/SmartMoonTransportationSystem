#ifndef RES_TEMPERATURE_H_
#define RES_TEMPERATURE_H_

void res_temperature_start(void);

void res_temperature_update(int node, int value);

bool res_temperature_alarm(void);

#endif
