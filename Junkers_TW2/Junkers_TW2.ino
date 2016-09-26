/**
   The MySensors Arduino library handles the wireless radio link and protocol
   between your home built sensors/actuators and HA controller of choice.
   The sensors forms a self healing radio network with optional repeaters. Each
   repeater and gateway builds a routing tables in EEPROM which keeps track of the
   network topology allowing messages to be routed to nodes.

   Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
   Copyright (C) 2013-2015 Sensnology AB
   Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors

   Documentation: http://www.mysensors.org
   Support Forum: http://forum.mysensors.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

 *******************************

   REVISION HISTORY
   Version 1.0 - January 30, 2015 - Developed by GizMoCuz (Domoticz)

   DESCRIPTION
   This sketch is based on an example how to implement a Dimmable Light
   It is pure virtual and it logs messages to the serial output
   It can be used as a base sketch for actual hardware.
   Stores the last light state and level in eeprom.

*/

// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#define MY_NODE_ID 110

#include <SPI.h>
#include <MySensors.h>

#define CHILD_ID_TW2 20

/*
#define LIGHT_OFF 0
#define LIGHT_ON 1

#define RELAY_1  8  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay
*/

#define SN "Junkers TW2"
#define SV "0.1"

MyMessage TW2_Msg(CHILD_ID_TW2, S_CUSTOM);

byte address = 0x00;
int CS1 = 10; //MCP4131-103 10kOhm
int CS2 = 7; //MCP4132-502 5kOhm
u_int MCP_Calib = 0; //adjust to real resistor value of circuitry without MCP's 

void before() {
  pinMode (CS1, OUTPUT);
  digitalWrite(CS1, HIGH);
  pinMode (CS2, OUTPUT);
  digitalWrite(CS2, HIGH);
  digitalPotWrite(CS1, MCP_Calib + 108);
  digitalPotWrite(CS2, 64)
  
  /*  for (int sensor = 1, pin = RELAY_1; sensor <= NUMBER_OF_RELAYS; sensor++, pin++) {
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);
    // Set relay to last known state (using eeprom storage)
    digitalWrite(pin, RELAY_OFF);*/
  }
}

void setup()
{
  /*
  //Retreive our last light state from the eprom
  int LightState = loadState(EPROM_LIGHT_STATE);
  if (LightState <= 1) {
    LastLightState = LightState;
    int DimValue = loadState(EPROM_DIMMER_LEVEL);
    if ((DimValue > 0) && (DimValue <= 100)) {
      //There should be no Dim value of 0, this would mean LIGHT_OFF
      LastDimValue = DimValue;
    }
  }
  */
  //Here you actualy switch on/off the light with the last known dim level
  SetCurrentState2Hardware();

  Serial.println( "Node ready to receive messages..." );
}

void presentation() {
  // Send the Sketch Version Information to the Gateway
  sendSketchInfo(SN, SV);
  present(CHILD_ID_TW2, S_CUSTOM);
}

void loop()
{
  /*  for (int i = 0; i <= 128; i++)
    {
      digitalPotWrite(i);
      delay(150);
    }
    delay(1500);
    for (int i = 128; i >= 0; i--)
    {
      digitalPotWrite(i);
      delay(150);
    }*/
}


int digitalPotWrite(int CSx, int value)
{
  digitalWrite(CSx, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(CSx, HIGH);
}

void receive(const MyMessage &message)
{
  if (message.type == V_LIGHT) {
    Serial.println( "V_LIGHT command received..." );
    digitalWrite(RELAY_1, message.getBool() ? RELAY_ON : RELAY_OFF);
/*
    int lstate = atoi( message.data );
    if ((lstate < 0) || (lstate > 1)) {
      Serial.println( "V_LIGHT data invalid (should be 0/1)" );
      return;
    }
    LastLightState = lstate;
    saveState(EPROM_LIGHT_STATE, LastLightState);

    if ((LastLightState == LIGHT_ON) && (LastDimValue == 0)) {
      //In the case that the Light State = On, but the dimmer value is zero,
      //then something (probably the controller) did something wrong,
      //for the Dim value to 100%
      LastDimValue = 100;
      saveState(EPROM_DIMMER_LEVEL, LastDimValue);
    }
*/
    //When receiving a V_LIGHT command we switch the light between OFF and the last received dimmer value
    //This means if you previously set the lights dimmer value to 50%, and turn the light ON
    //it will do so at 50%
  }
  else if (message.type == V_DIMMER) {
    Serial.println( "V_DIMMER command received..." );
    int dimvalue = atoi( message.data );
    if ((dimvalue < 0) || (dimvalue > 100)) {
      Serial.println( "V_DIMMER data invalid (should be 0..100)" );
      return;
    }
    if (dimvalue == 0) {
      LastLightState = LIGHT_OFF;
    }
    else {
      digitalPotWrite(dimvalue);
      /*  LastLightState = LIGHT_ON;
        LastDimValue = dimvalue;
        saveState(EPROM_DIMMER_LEVEL, LastDimValue);*/
    }
  }
  else {
    Serial.println( "Invalid command received..." );
    return;
  }

  //Here you set the actual light state/level
  SetCurrentState2Hardware();
}

void SetCurrentState2Hardware()
{
  if (LastLightState == LIGHT_OFF) {
    Serial.println( "Light state: OFF" );
  }
  else {
    Serial.print( "Light state: ON, Level: " );
    Serial.println( LastDimValue );
  }

  //Send current state to the controller
  SendCurrentState2Controller();
}

void SendCurrentState2Controller()
{
  if ((LastLightState == LIGHT_OFF) || (LastDimValue == 0)) {
    send(TW2b_Msg.set(0));
  }
  else {
    send(TW2b_Msg.set(LastDimValue));
  }
}

