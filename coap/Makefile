CONTIKI_PROJECT = coap-sensor
all: $(CONTIKI_PROJECT)

#PLATFORMS_ONLY = cc26x0-cc13x0
#PLATFORMS_EXCLUDE = sky z1 nrf52dk

CONTIKI=../../..
ROOT=..

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

# Include the CoAP implementation
include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

MODULES_REL += $(ROOT)/sensing/
MODULES_REL += $(ROOT)/sensing/temperature/
MODULES_REL += $(ROOT)/sensing/dust/
MODULES_REL += $(ROOT)/sensing/regolith/
MODULES_REL += $(ROOT)/utils/

# Include CoAP resources' handlers
MODULES_REL += ./resources
MODULES_REL += ./resources/temperature
MODULES_REL += ./resources/dust
MODULES_REL += ./resources/regolith

include $(CONTIKI)/Makefile.include
