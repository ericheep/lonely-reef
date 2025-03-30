#define DEBUG true

#include <Ethernet.h>

#define ARDUINO_IP 10, 70, 0, 11
#define ARDUINO_PORT 8888
#define MAX_IP 10, 70, 0, 10
#define MAX_PORT 5432

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(ARDUINO_IP);
IPAddress maxIP(MAX_IP);
EthernetUDP Udp;

#include "OSCBundle.h"
const char *buttonOSCMsg = "/button";
const char *potOSCMsg = "/pot";

#include "Bounce2.h"
#define NUM_BUTTONS 3
byte BUTTON_PINS[NUM_BUTTONS] = { 2, 3, 5 };
Bounce buttons[NUM_BUTTONS];

#define NUM_POTENTIOMETERS 3
int POT_PINS[NUM_POTENTIOMETERS] = { A3, A4, A5 };
int potentiometerValues[NUM_POTENTIOMETERS];

#include <FastLED.h>
#define NUM_LEDS 1
#define DATA_PIN 3

// Define the array of leds
CRGB leds[NUM_LEDS];

#define DATA_PIN 3
void setup() {
  if (DEBUG) Serial.begin(9600);

  Ethernet.begin(mac, ip);
  Ethernet.setRetransmissionCount(1);
  Ethernet.setRetransmissionTimeout(1);

  if (Udp.begin(ARDUINO_PORT) == 1 && DEBUG) {
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
  }

  FastLED.addLeds<WS2812, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

  for (byte i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP);
  }

  for (byte i = 0; i < NUM_POTENTIOMETERS; i++) {
    potentiometerValues[i] = 0;
  }
}

void loop() {
  readButtons();
  readPotentiometers();
}

void readButtons() {
  for (byte i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].update();
    if (buttons[i].fell() || buttons[i].rose()) {
      int state = buttons[i].read();
      if (DEBUG) Serial.print("Button " + String(i) + ": " + String(state) + "\n");

      sendButtonOSC(i, state);
    }
  }
}

void readPotentiometers() {
  for (byte i = 0; i < NUM_POTENTIOMETERS; i++) {
    int value = analogRead(POT_PINS[i]);
    if (value != potentiometerValues[i]) {
      potentiometerValues[i] = value;
      if (DEBUG) Serial.print("Pot " + String(i) + ": " + String(value) + "\n");
      
      sendPotentiometerOSC(i, value);
    }
  }
}

void sendButtonOSC(int buttonIndex, int buttonState) {
  OSCMessage outMsg(buttonOSCMsg);
  outMsg.add(buttonIndex);
  outMsg.add(buttonState);

  Udp.beginPacket(maxIP, MAX_PORT);
  outMsg.send(Udp);
  Udp.endPacket();
  outMsg.empty();
}

void sendPotentiometerOSC(int potIndex, int potValue) {
  OSCMessage outMsg(potOSCMsg);
  outMsg.add(potIndex);
  outMsg.add(potValue);

  Udp.beginPacket(maxIP, MAX_PORT);
  outMsg.send(Udp);
  Udp.endPacket();
  outMsg.empty();
}