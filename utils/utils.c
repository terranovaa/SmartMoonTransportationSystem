#include "./utils.h"

float rand_sample_range(float lower_bound, float upper_bound){
	return ((float)rand() / (float)RAND_MAX)*(upper_bound - lower_bound) + lower_bound;
}

float rand_sample_variation_range(float sample, float max_variation, float lower_bound, float upper_bound){
	while(1){
		float variation = rand_sample_range(0,max_variation);
		sample = rand_sample_range(sample-variation, sample+variation);
		if(sample < lower_bound){
			continue;
		} else if(sample > upper_bound){
			continue;
		} else {
			break;
		}
	}
       	return sample;
}

void buffer_json_message(char *buffer, size_t buffer_size, int node_id, char* type, int value, char* unit){
  	memset(buffer, 0, buffer_size);
  	sprintf(buffer, "{\"id\":\"%d\",\"t\":\"%s\",\"v\":\"%d\",\"u\":\"%s\"}",
          node_id, type, value, unit);
}

