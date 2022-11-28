#ifndef GARAGE_H
#define GARAGE_H

#include <Ticker.h>
#include <Ultrasonic.h>
#include "DHTStable.h"

#define DHTTYPE           DHT22     // DHT 22 (AM2302)
#define TEMPELEMENTS              1
#define SONICELEMENTS             5
#define ULTRAECHO 0 // D3
#define ULTRATRIG 2 // D4
#define MOTOR 5 // D1
#define DHTPIN 4 // D2

enum event_t {INIT, DOOR_OPEN, DOOR_CLOSED_CAR_IN, DOOR_CLOSED_CAR_OUT};

class Garage
{

public:
  /**
   * Get the Garage instance
   * @return Pointer to Garage instance
   */
  static Garage* getInstance();


  /**
   * Opens the Garage
   */
  void open();

  /**
   * Closes the Garage
   */
  void close();

  /**
   * Static wrapper function used in Ticker callback
   * http://www.newty.de/fpt/callback.html#static
   */
  static void Wrapper_To_Call_relayOff();

  /**
   * Static wrapper function used in Ticker callback
   * http://www.newty.de/fpt/callback.html#static
   */
  static void Wrapper_To_Call_updateSensors();

  /**
   * Static wrapper function used in Ticker callback
   * http://www.newty.de/fpt/callback.html#static
   */
  static void Wrapper_To_Call_updateState();


  /**
   * Called periodically to update the timers
   */
  void update();

  /**
   * Get current temperatur
   * @param  t Char array where temperature is stored
   * @return Pointer to temperture array
   */
  char* getTemp(char*);

  /**
  * Get current hunidity
  * @param  h CHar array where humitidy is stored
  * @return Pointer to humidity array
   */
  char* getHumid(char*);

  /**
   * Get last door event
   * @param  e Char array where event is stored
   * @return Pointer to event array
   */
  char* getEvent(char*);

  /**
   * Set a callback function.
   * The callback will be called when a new door event is detected
   * @param callback [description]
   */
  void setDoorStateCallback(fptr callback);
  void setSensorCallback(fptr callback);



private:
  /* Private constructor to prevent instancing. */
  Garage();
  /* Here will be the instance stored. */
  static Garage* instance;
  void garageCntrl(bool up);
  void relayOn();
  void relayOff();
  void updateSensors();
  void updateState();
  float avg(float *rgFloat, int length, float newVal, uint8 index);
  int median(int* values, int length);


  // Variables
  Ticker *pTimer = new Ticker(Wrapper_To_Call_relayOff, 300, 1, MILLIS);
  Ticker *pTimer2 = new Ticker(Wrapper_To_Call_updateSensors, 60000 , 0, MILLIS);
  Ticker *pTimer3 = new Ticker(Wrapper_To_Call_updateState, 200 , 0, MILLIS);
  Ultrasonic *pSonic = new Ultrasonic(ULTRATRIG, ULTRAECHO);
  DHTStable *pDht =  new DHTStable();
  
  event_t lastEvent = INIT;
  float temp;
  float humid;

  float rgTemp[TEMPELEMENTS];
  float rgHumid[TEMPELEMENTS];
  int lastTempIndex = 0;

  int rgSonic[SONICELEMENTS];
  int lastSonicIndex = 0;

  fptr doorstateCall;
  fptr sensorCall;

};

#endif
