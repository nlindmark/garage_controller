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
// FORWARD DECLARATIONS

bool setup_wifi();
void tack();
void eventListener();
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



void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(2000);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Turn LED off

  setup_wifi();

  garage->setCallback(eventListener);

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
      mqttClient.publish("/home/garage/hello","The Garage says Hello");
      mqttClient.publish("/home/garage/temp",  "0", true);  // Retained
      mqttClient.publish("/home/garage/humid", "0", true);  // Retained
      mqttClient.publish("/home/garage/event", "0", true);  // Retained
      mqttClient.subscribe("/home/garage/door/control");
      mqttClient.subscribe("/home/garage/status");
      mqttClient.subscribe("/home/garage/ECU/reboot");

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

  if(strcmp(t, "/home/garage/door/control") == 0){
    if(strcmp(p, "Open") == 0){
      garage->open();
    } else {
      garage->close();
    }

  } else   if(strcmp(t, "/home/garage/status") == 0){
    mqttClient.publish("/home/garage/temp",  garage->getTemp(buffer), true);
    mqttClient.publish("/home/garage/humid", garage->getHumid(buffer), true);
    mqttClient.publish("/home/garage/door/status", garage->getEvent(buffer), true);

  } else if (strcmp(t, "/home/garage/ECU/reboot") == 0) {
    ESP.reset();
  }
}


void eventListener(){
  // Something wonderful has happened
  char eventStr[50];
  mqttClient.publish("/home/garage/event", garage->getEvent(eventStr), true);
}

void tack() {
  char buffer[50];
  Serial.print("Temp: ");
  Serial.println(garage->getTemp(buffer));
  Serial.print("Humid: ");
  Serial.println(garage->getHumid(buffer));
}
