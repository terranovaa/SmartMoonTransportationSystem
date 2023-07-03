# Smart Moon Transportation System :moon:
The goal of this project is the design and implementation of an intelligent wireless
sensor network used to monitor and control a regolith transport solution for the Moon.
<br>
Lunar regolith is one of the most common resources present on the moon and it is
an important material used for different ISRU activities, but also mineral processing
and construction.
<br>
A set of regolith transport solutions is turned into a set of smart transportation systems
that can be used to monitor and control the extraction and transportation process
while minimizing dust hazards, possible errors in the amount of regolith transported,
and considering the typical moon’s temperature extreme conditions.
<img width="686" alt="image" src="https://github.com/terranovaa/SmartMoonTransportationSystem/assets/61695945/93f7e236-e35f-4903-b931-68184c606adf">

## Project Structure
The project is organized as follows:
- coap/ contains the source code of CoAP nodes
- mqtt/ contains the source code of MQTT nodes
- sensing/ contains the source code of the emulated sensors
- utils/ contains utility functions
- border_router/ contains the source code of the Contiki-NG Border Router
- collector/ contains the source code of the Java collector
- grafana/ contains the grafana dashboard
- COOJA-simulation.csc is the Cooja simulation
- SMTS_MySQL.sql is the SQL file that can be used to build the database

## Requirements to run the project
- Contiki-NG
- Cooja simulator for the simulation
- Grafana
- Mosquitto MQTT broker
- MySQL

## Project setup
<b>Step 1.</b> Download the project into the contiki-ng/examples folder
<b>Step 2.</b> Import the Grafana dashboard<br/>
<b>Step 3.</b> Setup the database <br/>

Create the SMTS DB:
<pre>
mysql -u root -p
CREATE DATABASE SMTS;
</pre>
Import the database structure:
<pre>
mysql -u root -p SMTS < SMTS_MySQL.sql
</pre>
<b>Step 4.</b> Run the MQTT broker
<pre>
sudo service mosquitto start
</pre>
<b>Step 5.</b> Run the collector
<br/>
<pre>
java -jar collector/collector.jar
</pre>
The application.properties can be changed to change the metrics to monitor or other parameters.

## COOJA Simulation
<b>Step 1.</b> Import the "COOJA Simulation.csc" file in Cooja<br/>
<b>Step 2.</b> Use tunslip6 
<pre>
cd border_router && make TARGET=cooja connect-router-cooja
</pre>

## Contiki-NG CC2650 Setup
<b>Step 1.</b> Flash the code on all sensor nodes:<br/>

Border router:
<pre>
 cd border_router && make TARGET=cc26x0-cc13x0 BOARD=/launchpad/cc2650 border_router
</pre>

MQTT node:
<pre>
 cd mqtt && make TARGET=cc26x0-cc13x0 BOARD=/launchpad/cc2650 mqtt-sensor
</pre>

CoAP node:
<pre>
 cd coap && make TARGET=cc26x0-cc13x0 BOARD=/launchpad/cc2650 coap-sensor
</pre>
<b>Step 2.</b> Power on all sensors <br>
<b>Step 3.</b> Use tunslip6 to connect with the border router 
<pre>
make TARGET=cc26x0-cc13x0 BOARD=/launchpad/cc2650 connect-router PORT=/dev/ttyACM0
</pre>
