package it.unipi.iot.SmartMoonTransportationSystem.coap;

import com.google.gson.Gson;
import com.google.gson.stream.JsonReader;
import it.unipi.iot.SmartMoonTransportationSystem.entities.Message;
import it.unipi.iot.SmartMoonTransportationSystem.persistence.DBManager;
import it.unipi.iot.SmartMoonTransportationSystem.utils.Parameters;
import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapHandler;
import org.eclipse.californium.core.CoapObserveRelation;
import org.eclipse.californium.core.CoapResponse;
import java.io.StringReader;
import java.util.Map;

public class CoAPObserver {
    private final CoapClient node;
    private final Map<String, Float> maxThresholds;
    private final Map<String, Float> minThresholds;
    private final String type;
    public final CoapObserveRelation relation;
    private boolean alarm;

    public CoAPObserver(final String HostAddr, String type) {
        maxThresholds = Parameters.getMaxThresholds();
        minThresholds = Parameters.getMinThresholds();
        int CoAPPort = Parameters.getCoAPPort();
        this.type = type;
        node = new CoapClient("coap://[" + HostAddr + "]:"+CoAPPort+"/"+type);
        relation = node.observe(
                new CoapHandler() {
                	boolean inserted = false;
                    public void onLoad(final CoapResponse coapResponse) {
                        String content = new String(coapResponse.getPayload());
                        Gson parser = new Gson();
                        JsonReader reader = new JsonReader(new StringReader(content));
                        Message message = parser.fromJson(reader, Message.class);
                        if(message.getNodeID() == 0)
                        	return;
                        if(!inserted) {
                        	DBManager.insertNode((int)message.getNodeID());
                        	inserted = true;
                        }
                        DBManager.insertSample(message.getNodeID(), message.getType(), message.getSample(), message.getUnit());
                        System.out.println("(C) New measurement: { id: \"" + message.getNodeID() + "\", t: \"" + message.getType() + "\", v: \"" + message.getSample() + "\", u: \"" +  message.getUnit() + "\" }");
                        detectAnomaly(message.getSample());
                    }
                    public void onError() {
                        System.err.printf("(C) Error while observing node %s...\n", HostAddr);
                        relation.proactiveCancel();
                        CoAPRegistration.removeResource(HostAddr);
                    }
                }
        );
    }

    public void detectAnomaly(float sample){
        float minThreshold = minThresholds.get(type);
        float maxThreshold = maxThresholds.get(type);
        if ((sample < minThreshold || sample > maxThreshold) && !alarm) {
            System.out.printf("(C) !!! Triggering node %s alarm...\n", node.getURI());
            CoAPRegistration.setResourceAlarm(node, true);
            alarm = true;
        } else if (sample >= minThreshold && sample <= maxThreshold && alarm) {
            System.out.printf("(C) !!! Removing node %s alarm\n", node.getURI());
            CoAPRegistration.setResourceAlarm(node, false);
            alarm = false;
        }
    }
}