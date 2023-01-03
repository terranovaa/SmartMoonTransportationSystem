package it.unipi.iot.SmartMoonTransportationSystem.utils;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.*;

public class Parameters {
    private static String DBIP;
    private static int DBPort;
    private static String DBUsername;
    private static String DBPassword;
    private static String DB;
    private static String MQTTIP;
    private static int MQTTPort;
    private static String MQTTClientID;
    private static int CoAPPort;
    private static Map<String, Float> maxThresholds = new HashMap<String, Float>();
    private static Map<String, Float> minThresholds = new HashMap<String, Float>();
    private static Map<String, String> errorMessages = new HashMap<String, String>();
    private static Map<String, String> okMessages = new HashMap<String, String>();
    private static List<String> topics;

    static {
        FileInputStream fis;
        System.out.println("Loading parameters...");
        try {
            fis = new FileInputStream("config.properties");
        } catch (FileNotFoundException ex) {
            throw new RuntimeException(ex);
        }
        Properties prop = new Properties();
        try {
            prop.load(fis);
        } catch (IOException ex) {
            throw new RuntimeException(ex);
        }

        DBIP = prop.getProperty("DBIP");
        DBPort = Integer.parseInt(prop.getProperty("DBPort"));
        DBUsername = prop.getProperty("DBUsername");
        DBPassword = prop.getProperty("DBPassword");
        DB = prop.getProperty("DB");

        MQTTIP = prop.getProperty("MQTTIP");
        MQTTPort = Integer.parseInt(prop.getProperty("MQTTPort"));
        MQTTClientID = prop.getProperty("MQTTClientID");

        CoAPPort = Integer.parseInt(prop.getProperty("CoAPPort"));

        int topics_number = Integer.parseInt(prop.getProperty("topics_number"));
        topics = new ArrayList<String>();
        for (int i = 1; i <= topics_number; ++i)
                topics.add(String.valueOf(prop.get("topic_"+i)));

        for (String topic : topics) {
            float min = Float.parseFloat(prop.getProperty(topic + "MinThreshold", "0"));
            float max = Float.parseFloat(prop.getProperty(topic + "MaxThreshold", "0"));
            String error = prop.getProperty(topic + "ErrorCode", "0");
            String ok = prop.getProperty(topic + "OKCode", "0");
            maxThresholds.put(topic, max);
            minThresholds.put(topic, min);
            errorMessages.put(topic, error);
            okMessages.put(topic, ok);
        }
    }

    public static int getCoAPPort() {
        return CoAPPort;
    }

    public static int getDBPort() {
        return DBPort;
    }

    public static String getMQTTClientID() {
        return MQTTClientID;
    }

    public static int getMQTTPort() {
        return MQTTPort;
    }

    public static Map<String, Float> getMaxThresholds() {
        return maxThresholds;
    }

    public static Map<String, Float> getMinThresholds() {
        return minThresholds;
    }

    public static Map<String, String> getErrorMessages() {
        return errorMessages;
    }

    public static Map<String, String> getOkMessages() {
        return okMessages;
    }

    public static String getDB() {
        return DB;
    }

    public static String getDBIP() {
        return DBIP;
    }

    public static String getDBPassword() {
        return DBPassword;
    }

    public static String getDBUsername() {
        return DBUsername;
    }

    public static String getMQTTIP() {
        return MQTTIP;
    }

    public static List<String> getTopics() {
        return topics;
    }

}


