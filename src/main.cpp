
#include <Arduino.h>
#include "iotUpdater.h"
#include "credentials.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Garage.h"
#include <Ticker.h>
extern "C" {
  #include "user_interface.h"

}

#define CODE_VERSION "V1.1"
// FORWARD DECLARATIONS

bool setup_wifi();
void tack();
void doorstateEventListener();
void sensorEventListener();
boolean mqttReconnect();

// VARIABLES
Garage* garage = Garage::getInstance();

Ticker timer1(tack, 1000,0, MILLIS);

char ssid[] = SSIDNAME;
char password[] = PASSWORD;

WiFiClient espClient;

IPAddress mqttServer(192, 168, 1, 200);
void mqttCallback(char* topic, byte* payload, unsigned int length);
PubSubClient mqttClient(mqttServer, 1883, mqttCallback, espClient);

long lastMqttReconnectAttempt = 0;
int mqttReconnectCounter = 0;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(2000);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Turn LED off

  setup_wifi();

  garage->setDoorStateCallback(doorstateEventListener);
  garage->setSensorCallback(sensorEventListener);

  timer1.start();

  mqttReconnect();

  iotUpdater(true);
}

void loop() {
  // put your main code here, to run repeatedly:
  //
  if (!mqttClient.connected()) {
    mqttReconnect();
  } else {
    // Client connected
    mqttClient.loop();
  }

  timer1.update();
  garage->update();
}


bool setup_wifi() {

  delay(10);

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSIDNAME);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


  if(WiFi.status() == WL_CONNECTED){

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
  }

return false;
}

boolean mqttReconnect(){

  char clientName[50];
  long now = millis();

  if (now - lastMqttReconnectAttempt > 3000) {
    lastMqttReconnectAttempt = now;
    // Attempt to reconnect

    snprintf (clientName, 50, "%ld", system_get_chip_id());
    if (mqttClient.connect(clientName, MQTTUSER, MQTTPASSWORD)) {
      char str[30];
      strcpy(str, "The Garage is (re)connected ");
      strcat(str, CODE_VERSION);
      mqttClient.publish("stat/garage/hello", str);
      snprintf (str, 50, "%i", mqttReconnectCounter++);
      mqttClient.publish("stat/garage/reconnect", str);
      mqttClient.subscribe("cmnd/garage/door");
      mqttClient.subscribe("cmnd/garage/temp");
      mqttClient.subscribe("cmnd/garage/humid");
      mqttClient.subscribe("cmnd/garage/doorstate");
      mqttClient.subscribe("cmnd/garage/reboot");

    }
  }
  return mqttClient.connected();
}


// CALLBACKS
//


void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived

  // Internally, the mqtt client uses the same buffer for both inbound and outbound
  // messages. The application should create its own copy of the values if they
  // are required beyond this.
  char t[50], p[50], buffer[50];
  snprintf (t, 50, "%s", topic);
  snprintf (p, 50, "%s",payload);
  Serial.println(t);

  if(strcmp(t, "cmnd/garage/door") == 0){
    if(strcmp(p, "Open") == 0){
       garage->open();
    } else {
       garage->close();
    }

  } else   if(strcmp(t, "cmnd/garage/temp") == 0){
     mqttClient.publish("stat/garage/temp",  garage->getTemp(buffer), true);

  } else if (strcmp(t, "cmnd/garage/humid") == 0) {
     mqttClient.publish("stat/garage/humid", garage->getHumid(buffer), true);

  } else if (strcmp(t, "cmnd/garage/doorstate") == 0) {
     mqttClient.publish("stat/garage/doorstate", garage->getEvent(buffer), true);

  } else if (strcmp(t, "cmnd/garage/reboot") == 0) {
     ESP.reset();
  }
}


void doorstateEventListener(){
  // Something wonderful has happened
  char eventStr[50];
  mqttClient.publish("stat/garage/doorstate", garage->getEvent(eventStr), true);
}

void sensorEventListener(){
  // Something wonderful has happened
  char eventStr[50];
  mqttClient.publish("stat/garage/temp", garage->getTemp(eventStr), true);
  mqttClient.publish("stat/garage/humid", garage->getHumid(eventStr), true);
  
}

void tack() {
  char buffer[50];
  Serial.print("Temp: ");
  Serial.println(garage->getTemp(buffer));
  Serial.print("Humid: ");
  Serial.println(garage->getHumid(buffer));
}
