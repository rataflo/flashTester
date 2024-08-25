/*
  Flash tester with arduino.
  Fire flash X times and see if the flash actually flash.

  Flo Galès 2024: Do What The Fuck You Want License / License rien à branler.
*/

#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <TimerOne.h>
#include "OzOLED.h"
#include <ClickEncoder.h>
#include <menu.h>
#include <menuIO/OzOledAsciiOut.h>
#include <menuIO/clickEncoderIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>

using namespace Menu;

/* PINS */
#define PIN_SENSOR A1
#define FLASH_PIN 4

// Encoder
#define BTN_EN1 8
#define BTN_EN2 9
#define BTN_ENC 10

/* GLOBALS */
#define oled OzOled // oled screen
unsigned long startFlash = 0;
#define FLASH_SYNC_DURATION 150 // time in ms for activating the flash pin.

#define EEPROM_ADRESS 0
struct storage {
  byte checkCode = 10;
  int thresold = 100;
  int flashCycles = 50; // Number of flashing counter, max 1024
  int flashRecycle = 3; // Number ofsecond between flashs, max 1024
  int flashGood = 0;
  int flashBad = 0;
};

storage parameters;

// Rotary encoder & Menu definition
ClickEncoder clickEncoder(BTN_EN2, BTN_EN1, BTN_ENC,4);
ClickEncoderStream encStream(clickEncoder,1);
MENU_INPUTS(in, &encStream);
void timerIsr() {clickEncoder.service();}

#define MAX_DEPTH 2
#define fontW 8
#define fontH 8

const panel panels[] MEMMODE={{0,0,128/fontW,64/fontH}};
navNode* nodes[sizeof(panels)/sizeof(panel)];//navNodes to store navigation status
panelsList pList(panels,nodes,1);//a list of panels and nodes
idx_t tops[MAX_DEPTH]={0,0};//store cursor positions for each level
OzOledAsciiOut outOLED(&oled,tops,pList);//oled output device menu driver
menuOut* constMEM outputs[] MEMMODE={&outOLED};//list of output devices
outputsList out(outputs,1);//outputs list

result saveParams(eventMask e, prompt &item){
  EEPROM.updateBlock(EEPROM_ADRESS, parameters);
  return proceed;
}

result launchTest(eventMask e, prompt &item){
  oled.clearDisplay();
  oled.printString(" Testing...", 0, 0);
  oled.printString("-Good:0", 0, 2);
  oled.printString("-Bad:0", 0, 3);

  parameters.flashGood = 0;
  parameters.flashBad = 0;
  unsigned long currentMicros = 0;
  unsigned long wait = parameters.flashRecycle * 1000;

  oled.printString(String("-Good:" + String(parameters.flashGood)).c_str(), 0, 2);
  oled.printString(String("-Bad:" + String(parameters.flashBad)).c_str(), 0, 3);

  for(int i = 0; i < parameters.flashCycles; i++){
    
    oled.printString(String("-Test " + String(i+1) + "/" + String(parameters.flashCycles)).c_str(), 0, 1);
    // Fire flash
    startFlash = millis();
    currentMicros = startFlash;
    digitalWrite(FLASH_PIN, HIGH);
    
    // Wait for flash until next trigger
    int sensorValue = 1024 - analogRead(PIN_SENSOR);
    while(sensorValue < parameters.thresold && currentMicros - startFlash  < wait){
      sensorValue = 1024 - analogRead(PIN_SENSOR);
      currentMicros = millis();
      if(currentMicros - startFlash > FLASH_SYNC_DURATION){
        digitalWrite(FLASH_PIN, LOW);
      }
    }

    if(sensorValue > parameters.thresold){
      parameters.flashGood++;
    }else{
      parameters.flashBad++;
    }

    digitalWrite(FLASH_PIN, LOW);

    // Update screen.
    //oled.printString(String("Sensor:" + String(sensorValue)).c_str(), 0, 3);
    oled.printString(String("-Good:" + String(parameters.flashGood)).c_str(), 0, 2);
    oled.printString(String("-Bad:" + String(parameters.flashBad)).c_str(), 0, 3);

    // Save values in eeprom
    EEPROM.updateBlock(EEPROM_ADRESS, parameters);

    //wait for recycling
    while(currentMicros - startFlash  < wait){
      currentMicros = millis();
    }
  }
  oled.clearDisplay();
  out.refresh();
  return proceed;
}

MENU(mainMenu," FLASH TESTER", doNothing, noEvent, wrapStyle
  ,FIELD(parameters.thresold,"Thresold","",0,1024,1,0,saveParams ,exitEvent, noStyle)
  ,FIELD(parameters.flashCycles,"Cycles","",0,1024,1,0,saveParams ,exitEvent, noStyle)
  ,FIELD(parameters.flashRecycle,"Recycling","s",0,1024,1,0,saveParams ,exitEvent, noStyle)
  ,FIELD(parameters.flashGood,"Good","",0,1024,1,0,doNothing,noEvent,noStyle)
  ,FIELD(parameters.flashBad,"Bad","",0,1024,1,0,doNothing,noEvent,noStyle)
  ,OP("Launch test", launchTest, enterEvent)
);

NAVROOT(nav,mainMenu,MAX_DEPTH, in, out);//the navigation root object

result idle(menuOut &o, idleEvent e) {
  o.clear();
  switch(e) {
    case idleStart:o.println("suspending menu!");break;
    case idling:o.println("suspended...");break;
    case idleEnd:o.println("resuming menu.");break;
  }
  return proceed;
}

void setup() {
  pinMode(PIN_SENSOR, INPUT);
  pinMode(FLASH_PIN, OUTPUT);
  //Serial.begin(9600);
  oled.init();
  oled.clearDisplay();
  //Vertical flip
  oled.sendCommand(0xC8);
  oled.sendCommand(0xA1);
  oled.printString("Flash tester V0.9");
  nav.idleTask=idle;//point a function to be used when menu is suspended
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  clickEncoder.setAccelerationEnabled(true);

  EEPROM.readBlock(EEPROM_ADRESS, parameters);

  // Check verif code, if not correct init eeprom.
  if(parameters.checkCode != 11){
    parameters.checkCode = 11;
    parameters.thresold = 900;
    parameters.flashCycles = 5; 
    parameters.flashRecycle = 3;
    parameters.flashGood = 0;
    parameters.flashBad = 0;
    EEPROM.writeBlock(EEPROM_ADRESS, parameters);
  }

  mainMenu[3].enabled=disabledStatus;
  mainMenu[4].enabled=disabledStatus;
}

void loop() {
  
  nav.poll();
}


