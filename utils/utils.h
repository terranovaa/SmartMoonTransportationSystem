#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

float rand_sample_variation_range(float sample, float max_variation, float lower_bound, float upper_bound);

float rand_sample_range(float lower_bound, float upper_bound);

void buffer_json_message(char *buffer, size_t buffer_size, int node_id, char* type, int value, char* unit);

#endif
