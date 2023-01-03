#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coap-engine.h"
#include "os/sys/log.h"

#include "./res-dust.h"
#include "../../../utils/utils.h"

#define LOG_MODULE "coap-sensor"
#define LOG_LEVEL LOG_LEVEL_APP

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);
static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

static int sample;
static int node_id; 
static bool alarm;

EVENT_RESOURCE(res_dust,
        "title=\"dust\";rt=\"Text\" POST/PUT value=<value>;obs",
        res_get_handler,
        NULL,
        res_put_handler,
        NULL,
        res_event_handler);

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
    int message_length = 512;
    char message[message_length];
    int length;
    buffer_json_message(message, message_length, node_id, "dust", sample, "mg/m^3");
    length = strlen(message);
    memcpy(buffer, message, length);
    coap_set_header_content_format(response, APPLICATION_JSON);
    coap_set_header_etag(response, (uint8_t*)&length, 1);
    coap_set_payload(response, buffer, length);
}

static void res_event_handler(void){
    coap_notify_observers(&res_dust);
}

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset){
    const char* overall_value = NULL;
    size_t len = 0;
    char value[6];
    memset(value, 0, 5);

    len = coap_get_post_variable(request, "alarm", &overall_value);
    memcpy(value, overall_value, len);
    if (strncmp(value, "true", len) == 0){
            alarm = true;
            LOG_INFO("Dust alarm enabled\n");
    } else if (strncmp(value, "false", len) == 0){
            alarm = false;
            LOG_INFO("Dust alarm disabled\n");
    } else {
        LOG_ERR("Not valid request.\n");
      	coap_set_status_code(response, BAD_REQUEST_4_00);
    }  
}


void res_dust_start(void){
    sample = 0;
    coap_activate_resource(&res_dust, "dust");
}

void res_dust_update(int node, int value){
    node_id = node;
    sample = value;
    res_dust.trigger();
}

bool res_dust_alarm(void){
    return alarm;
}
