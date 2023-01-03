package it.unipi.iot.SmartMoonTransportationSystem;

import it.unipi.iot.SmartMoonTransportationSystem.coap.CoAPCollector;
import it.unipi.iot.SmartMoonTransportationSystem.mqtt.MQTTCollector;

public final class Collector {
    public static void main(String[] args) {
        MQTTCollector mqttCollector = new MQTTCollector();
        mqttCollector.start();
        CoAPCollector coapCollector = new CoAPCollector();
        coapCollector.start();
    }
}

