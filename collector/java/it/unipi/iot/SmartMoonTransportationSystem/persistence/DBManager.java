package it.unipi.iot.SmartMoonTransportationSystem.persistence;

import it.unipi.iot.SmartMoonTransportationSystem.utils.Parameters;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;

public class DBManager {
    private static Connection conn;
    public static String mysqlHost;
    public static int mysqlPort;
    public static String mysqlUsername;
    public static String mysqlPassword;
    public static String mysqlDatabase;

    static {
        mysqlHost = Parameters.getDBIP();
        mysqlPort = Parameters.getDBPort();
        mysqlUsername = Parameters.getDBUsername();
        mysqlPassword = Parameters.getDBPassword();
        mysqlDatabase = Parameters.getDB();
        startConnection();
    }

    public static void startConnection(){
    	try {
			Class.forName("com.mysql.jdbc.Driver");
		} catch (ClassNotFoundException e1) {
		}
        try {
            String url = "jdbc:mysql://"+ mysqlHost +":"+mysqlPort+"/"+mysqlDatabase;
            conn = DriverManager.getConnection(url, mysqlUsername, mysqlPassword);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void insertSample(int nodeId, String type, float value, String unit){
        String query = "INSERT INTO "+ type.toUpperCase() + "(Node, Sample, Unit) "
                + " VALUES ('"+nodeId+"', '"+value+"','"+unit+"');";
        try{
            PreparedStatement ps = conn.prepareStatement(query);
            ps.executeUpdate();
        } catch(Exception e){
        	e.printStackTrace();
        }
    }

    public static void insertNode(int i){
        String query = "INSERT INTO NODE(ID) "
                + " VALUES ('"+i+"');";
        try{
        	if(!nodeExists(i)) {
	            PreparedStatement ps = conn.prepareStatement(query);
	            ps.executeUpdate();
        	}
        } catch(Exception e){
        	e.printStackTrace();
        }
    }
    
    public static boolean nodeExists(int i){
        String query = "SELECT * FROM NODE WHERE ID="+i+";";
        try{
            PreparedStatement ps = conn.prepareStatement(query);
            ResultSet rs = ps.executeQuery();
            if(rs.next()) {
            	return true;
            } else return false;
        } catch(Exception e){
        	e.printStackTrace();
        }
        return false;
    }
    
    

    public static void closeConnection(){
        try {
            conn.close();
        } catch (Exception e) {
            System.out.println("Connection cannot be closed:" + e);
        }
    }
}
