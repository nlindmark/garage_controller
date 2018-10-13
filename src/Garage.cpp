
#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include <Ticker.h>
#include "Garage.h"



/* Null, because instance will be initialized on demand. */
Garage* Garage::instance = 0;

Garage* Garage::getInstance() {
  if (instance == 0)
  {
    instance = new Garage();
    instance->pDht->begin();
    instance->pTimer2->start();
    instance->pTimer3->start();
    instance->doorstateCall = NULL;
    instance->sensorCall = NULL;
    

    pinMode(MOTOR, OUTPUT);
    pinMode(ULTRATRIG, OUTPUT);
    pinMode(ULTRAECHO, INPUT);
    digitalWrite(MOTOR, LOW);

    // First inital read
    instance->updateSensors();
    instance->updateState();

  }
  return instance;
}

void Garage::open() {
  garageCntrl(true);
}

void Garage::close() {
  garageCntrl(false);
}



void Garage::setDoorStateCallback(fptr callback){
  doorstateCall = callback;
}
void Garage::setSensorCallback(fptr callback){
  sensorCall = callback;
}



// static wrapper-function to be able to callback the member function Display()
void Garage::Wrapper_To_Call_relayOff() {
  // call member
  instance->relayOff();
}

void Garage::Wrapper_To_Call_updateSensors() {
  // call member
  instance->updateSensors();
}

void Garage::Wrapper_To_Call_updateState() {
  // call member
  instance->updateState();
}

void Garage::update() {
  pTimer->update();
  pTimer2->update();
  pTimer3->update();
}

char* Garage::getTemp(char* t) {
  snprintf(t, 50, "%f", temp);
  return t;
}

char* Garage::getHumid(char* h) {
  snprintf(h, 50, "%f", humid);
  return h;
}

char* Garage::getEvent(char* e) {
  switch(lastEvent){
    case INIT:
    snprintf(e, 50, "%s", "Init");
    break;
    case DOOR_OPEN:
    snprintf(e, 50, "%s", "Door open");
    break;
    case DOOR_CLOSED_CAR_IN:
    snprintf(e, 50, "%s", "Door closed, car in");
    break;
    case DOOR_CLOSED_CAR_OUT:
    snprintf(e, 50, "%s", "Door closed");
  }

  return e;
}

// PRIVATE
Garage::Garage() {}

void Garage::relayOff() {
  digitalWrite(MOTOR, LOW);
  //digitalWrite(LED_BUILTIN, HIGH);
}

void Garage::relayOn() {
  digitalWrite(MOTOR, HIGH);
  //digitalWrite(LED_BUILTIN, LOW);
}

void Garage::garageCntrl(bool up) {
  relayOn();
  pTimer->start();
}

void Garage::updateSensors() {
  // Check temp and humidity
  temp = avg(rgTemp, TEMPELEMENTS, pDht->readTemperature(), lastTempIndex);
  humid = avg(rgHumid, TEMPELEMENTS, pDht->readHumidity(), lastTempIndex);
  lastTempIndex = ++lastTempIndex%TEMPELEMENTS;

  if(sensorCall != NULL) sensorCall();
}

void Garage::updateState(){

  event_t event;
  uint32_t distance = pSonic->distanceRead();

  // Sense checking
  if(distance < 0 || distance > 300) {
    return;
  }

  // Pick out median distance
  rgSonic[lastSonicIndex] = distance;
  lastSonicIndex = ++lastSonicIndex%SONICELEMENTS;
  distance = median(rgSonic, SONICELEMENTS);

  if(distance < 40){
    event =  DOOR_OPEN;
  } else if(distance < 180){
    event = DOOR_CLOSED_CAR_IN;
  } else {
    event = DOOR_CLOSED_CAR_OUT;
  }

  if(lastEvent != event){
    lastEvent = event;
    if(doorstateCall != NULL) doorstateCall();
  }
}

float Garage::avg(float *rgFloat, int length,  float newVal, uint8 index) {

  float avg = 0.0f;

  if(!isnan(newVal)) {
    rgFloat[index] = newVal;
  }

  for(int i = 0; i < length; i++) {
    avg += rgFloat[i];
  }

  avg /= length;

  return avg;
}




int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

int Garage::median(int* values, int length)
{
  int n;
  qsort (values, length, sizeof(int), compare);
  for (n=0; n<length; n++){
    printf ("%d ",values[n]);
  }

  return values[length/2];
}
