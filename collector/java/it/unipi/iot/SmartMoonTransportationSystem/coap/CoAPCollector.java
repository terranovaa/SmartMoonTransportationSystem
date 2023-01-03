package it.unipi.iot.SmartMoonTransportationSystem.coap;

import org.eclipse.californium.core.CoapServer;

public class CoAPCollector extends CoapServer{
    public CoAPCollector() {
        this.add(new CoAPRegistration("registration"));
        System.out.println("(C) Starting the CoAP Server...");
    }
}

