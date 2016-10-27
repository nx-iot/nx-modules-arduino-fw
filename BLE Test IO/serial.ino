/*
 * Serial Port over BLE
 * Create UART service compatible with Nordic's *nRF Toolbox* and Adafruit's *Bluefruit LE* iOS/Android apps.
 *
 * BLESerial class implements same protocols as Arduino's built-in Serial class and can be used as it's wireless
 * replacement. Data transfers are routed through a BLE service with TX and RX characteristics. To make the 
 * service discoverable all UUIDs are NUS (Nordic UART Service) compatible.
 *
 * Please note that TX and RX characteristics use Notify and WriteWithoutResponse, so there's no guarantee
 * that the data will make it to the other end. However, under normal circumstances and reasonable signal
 * strengths everything works well.
 */


// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEPeripheral.h>
#include "BLESerial.h"

// define pins (varies per shield/board)
#define BLE_REQ   9//10
#define BLE_RDY   8//2
#define BLE_RST   UNUSED//9
// LED and button pin
#define LED_PIN     3
#define BUTTON_PIN  4
// create ble serial instance, see pinouts above
BLESerial BLESerial(BLE_REQ, BLE_RDY, BLE_RST);
//1815
// create service
BLEService               ledService           = BLEService("1815");

// create switch and button characteristic
BLECharCharacteristic    switchCharacteristic = BLECharCharacteristic("2A56", BLERead | BLEWrite);
BLECharCharacteristic    buttonCharacteristic = BLECharCharacteristic("2A58", BLERead | BLENotify);

void setup() {
  // custom services and characteristics can be added as well
  BLESerial.setLocalName("UART");
  
  // set LED pin to output mode, button pin to input mode
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT); 
  // set advertised local name and service UUID
  BLESerial.setAdvertisedServiceUuid(ledService.uuid());
  // add service and characteristics
  BLESerial.addAttribute(ledService);
  BLESerial.addAttribute(switchCharacteristic);
  BLESerial.addAttribute(buttonCharacteristic);
  
  Serial.begin(115200);
  BLESerial.begin();
}

void loop() {
  BLESerial.poll();
  delay(10);
  forward();
  led();
  // loopback();
  // spam();
}
void led(){
  // read the current button pin state
  char buttonValue = digitalRead(BUTTON_PIN);

  // has the value changed since the last read
  bool buttonChanged = (buttonCharacteristic.value() != buttonValue);

  
  if (buttonChanged) {
    // button state changed, update characteristics
    switchCharacteristic.setValue(buttonValue);
    buttonCharacteristic.setValue(buttonValue);
  }

  if (switchCharacteristic.written() || buttonChanged) {
    // update LED, either central has written to characteristic or button state has changed
    if (switchCharacteristic.value()) {
      Serial.println(F("LED on"));
      digitalWrite(LED_PIN, HIGH);
    } else {
      Serial.println(F("LED off"));
      digitalWrite(LED_PIN, LOW);
    }
  }  
}

// forward received from Serial to BLESerial and vice versa
void forward() {
  if (BLESerial && Serial) {
    int byte;
    while ((byte = BLESerial.read()) > 0) Serial.write((char)byte);
    while ((byte = Serial.read()) > 0) BLESerial.write((char)byte);
  }
}

// echo all received data back
void loopback() {
  if (BLESerial) {
    int byte;
    while ((byte = BLESerial.read()) > 0) BLESerial.write(byte);
  }
}

// periodically sent time stamps
void spam() {
  if (BLESerial) {
    BLESerial.print(millis());
    BLESerial.println(" tick-tacks!");
    delay(1000);
  }
}
