package it.unipi.iot.SmartMoonTransportationSystem.mqtt;

import com.google.gson.Gson;
import it.unipi.iot.SmartMoonTransportationSystem.entities.Message;
import it.unipi.iot.SmartMoonTransportationSystem.persistence.DBManager;
import it.unipi.iot.SmartMoonTransportationSystem.utils.Parameters;
import org.eclipse.paho.client.mqttv3.*;
import java.util.*;

public class MQTTCollector implements MqttCallback {
    private final String broker;
    private final String clientID;
    private final List<String> topics;
    private MqttClient client;
    Map<String, Float> maxThresholds;
    Map<String, Float> minThresholds;
    Map<String, String> errorMessages;
    Map<String, String> okMessages;
    Map<String, List<Integer>> exceededNodes = new HashMap<String, List<Integer>>();

    public MQTTCollector(){
    	broker = "tcp://" + Parameters.getMQTTIP() + ":" + Parameters.getMQTTPort();
        clientID= Parameters.getMQTTClientID();
        topics = Parameters.getTopics();
        maxThresholds = Parameters.getMaxThresholds();
        minThresholds = Parameters.getMinThresholds();
        errorMessages = Parameters.getErrorMessages();
        okMessages = Parameters.getOkMessages();
        for(String topic: topics) {
            exceededNodes.put(topic, new ArrayList<Integer>());
        }    
    }

    public void start(){
        try {
            System.out.println("(M) Connecting to the broker..");
            client = new MqttClient(broker, clientID);
            client.setCallback(this);
            client.connect();
            System.out.println("(M) Connected to the broker");
            subscribeTopics();
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public void close(){
        try {
            System.out.println("(M) Closing the connection to the broker..");
            client.close(true);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public void subscribeTopics(){
        System.out.println("(M) Subscribing to the topics..");
        for(String topic: topics){
            try {
                client.subscribe(topic);
            } catch (MqttException e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void connectionLost(Throwable throwable) {
        System.out.println("(M) Connection lost..");
    }

    public void publish(String message, int nodeID){
        try{
            MqttMessage mqttMessage = new MqttMessage(message.getBytes());
            client.publish("alarm/" + nodeID, mqttMessage);
        }catch(MqttException e){
            e.printStackTrace();
        }
    }

    public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {}

    public void messageArrived(String s, MqttMessage mqttMessage) throws Exception {
        try{
            String jsonString = new String(mqttMessage.getPayload());
            Message message = new Gson().fromJson(jsonString, Message.class);
            if(message.getNodeID() == 0)
            	return;
            System.out.println("(M) New measurement: { id: \"" + message.getNodeID() + "\", t: \"" + message.getType() + "\", v: \"" + message.getSample() + "\", u: \"" +  message.getUnit() + "\" }");
            DBManager.insertNode((int)message.getNodeID());
            DBManager.insertSample(message.getNodeID(), message.getType(), message.getSample(), message.getUnit());
            detectAnomaly(message);
        }catch(Exception e){
            e.printStackTrace();
        }
    }

    public void detectAnomaly(Message message){
        float max = maxThresholds.get(message.getType());
        float min = minThresholds.get(message.getType());
        if((max != 0 && message.getSample() > max)){
        	if(!alreadyExceeded(message.getType(), message.getNodeID()))
        		System.out.println("(M) !!! Max " + message.getType() + " threshold exceeded by node " + message.getNodeID()+ ".");
        	else System.out.println("(M) !!! Max " + message.getType() + " threshold still exceeded by node " + message.getNodeID()+ ".");
        	addExceeded(message.getType(), message.getNodeID());
            publish(errorMessages.get(message.getType()), message.getNodeID());
        } else if((min != 0 && message.getSample() < min)){
        	if(!alreadyExceeded(message.getType(), message.getNodeID()))
        		System.out.println("(M) !!! Min " + message.getType() + " threshold exceeded by node " + message.getNodeID()+ ".");
        	else System.out.println("(M) !!! Min " + message.getType() + " threshold still exceeded by node " + message.getNodeID()+ ".");
        	publish(errorMessages.get(message.getType()), message.getNodeID());
            addExceeded(message.getType(), message.getNodeID());
        } else if((max == 0 || message.getSample() <= max) && (min == 0 || message.getSample() >= min) && alreadyExceeded(message.getType(), message.getNodeID())){
            System.out.println("(M) !!! Measurement of type " + message.getType() + " returned normal for node " + message.getNodeID());
            publish(okMessages.get(message.getType()), message.getNodeID());
            removeExceeded(message.getType(), message.getNodeID());
        }
    }

    public boolean alreadyExceeded(String type, int nodeID){
        List<Integer> listIDs = exceededNodes.get(type);
        if(listIDs.contains(nodeID))
            return true;
        else
            return false;
    }

    public void removeExceeded(String type, int nodeID){
        List<Integer> listIDs = exceededNodes.get(type);
        listIDs.remove(listIDs.indexOf(nodeID));
        exceededNodes.remove(type);
        exceededNodes.put(type, listIDs);
    }

    public void addExceeded(String type, int nodeID){
        List<Integer> listIDs = exceededNodes.get(type);
        if(!listIDs.contains(nodeID)) {
        	listIDs.add(nodeID);
        	exceededNodes.remove(type);
            exceededNodes.put(type, listIDs);
        }
    }
}
