CONTIKI_PROJECT = mqtt-sensor
all: $(CONTIKI_PROJECT)

CONTIKI=../../..
ROOT=..

#PLATFORMS_ONLY = cc26x0-cc13x0 
#PLATFORMS_EXCLUDE = sky z1 nrf52dk

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

include $(CONTIKI)/Makefile.dir-variables
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/mqtt

MODULES_REL += arch/platform/$(TARGET)
MODULES_REL += $(ROOT)/sensing/
MODULES_REL += $(ROOT)/sensing/temperature/
MODULES_REL += $(ROOT)/sensing/dust/
MODULES_REL += $(ROOT)/sensing/regolith/
MODULES_REL += $(ROOT)/utils/

include $(CONTIKI)/Makefile.include
