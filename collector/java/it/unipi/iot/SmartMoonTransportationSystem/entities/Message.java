package it.unipi.iot.SmartMoonTransportationSystem.entities;

public class Message {
    private int id;
    private String t;
    private float v;
    private String u;

    public int getNodeID() {
        return id;
    }

    public String getType() {
        return t;
    }
    public float getSample() {
        return v;
    }
    
    public String getUnit() {
        return u;
    }

}

