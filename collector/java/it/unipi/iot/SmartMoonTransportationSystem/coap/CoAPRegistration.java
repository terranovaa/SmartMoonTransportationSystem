package it.unipi.iot.SmartMoonTransportationSystem.coap;

import it.unipi.iot.SmartMoonTransportationSystem.utils.Parameters;
import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.CoAP;
import org.eclipse.californium.core.coap.MediaTypeRegistry;
import org.eclipse.californium.core.coap.Response;
import org.eclipse.californium.core.server.resources.CoapExchange;
import java.util.HashMap;
import java.util.List;

public class CoAPRegistration extends CoapResource {
    private static HashMap<String, Object> resources;
    private static List<String> topics;

    public CoAPRegistration(String name) {
        super(name);
        topics = Parameters.getTopics();
        resources = new HashMap<String, Object>();
    }
    
    public void handlePOST(CoapExchange exchange) {
    		System.out.print("(C) Received a request from node with HostAddr: ");
    		Response response = new Response(CoAP.ResponseCode.CONTINUE);
            String sender = exchange.getSourceAddress().getHostAddress();
            int code;
            if(register(sender))
                code = 1;
            else code = 0;
            response.getOptions().setContentFormat(MediaTypeRegistry.APPLICATION_JSON);
            response.setPayload("{ \"registration\": "+code+" }");
            exchange.respond(response);
    }

    public static boolean register(String HostAddr){
    	System.out.println(HostAddr);
        if (!resources.containsKey(HostAddr)) {
        	System.out.println("(C) Adding the resource observer for the topics:");
        	for(String topic: topics) {
                resources.put(HostAddr+"_"+topic, new CoAPObserver(HostAddr,topic));
                System.out.println("(C) - " + topic);
        	}
        	return true;
        } else {
            System.out.printf("(C) Sensor %s was already active...\n", HostAddr);
            return false;
        }
    }

    public static void setResourceAlarm(final CoapClient resource, boolean mode) {
        resource.put(new CoapHandler() {
            public void onLoad(CoapResponse coapResponse) {
                if (coapResponse != null) {
                    if(!coapResponse.isSuccess())
                        System.out.printf("(C) Cannot send PUT request to the node %s\n", resource.getURI());
                }
            }
            public void onError() {
                System.out.printf("(C) Cannot connect to %s\n", resource.getURI());
            }
        }, "alarm=" + mode, MediaTypeRegistry.TEXT_PLAIN);
    }

    public static void removeResource(String HostAddr) {
    	((CoAPObserver)resources.get(HostAddr)).relation.proactiveCancel();
    	for(String topic: topics)
    		resources.remove(HostAddr+"_"+topic);
        System.out.printf("(C) Removed resource %s\n", HostAddr);
    }
}
