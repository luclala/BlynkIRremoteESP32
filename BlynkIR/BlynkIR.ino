/* IRremoteESP32: IRsendDemo - demonstrates sending IR codes with IRsend.

   Version 1.0 June 2017
   Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009,
   Copyright 2009 Ken Shirriff, http://arcfn.com

   An IR LED with resistor and NPN transistor must be connected

   Suggested circuit:
       https://github.com/markszabo/IRremoteESP8266/wiki#ir-sending

   Common mistakes & tips:
 *   * Don't just connect the IR LED directly to the pin, it won't
       have enough current to drive the IR LED effectively.
 *   * Make sure you have the IR LED polarity correct.
       See: https://learn.sparkfun.com/tutorials/polarity/diode-and-led-polarity
 *   * Typical digital camera/phones can be used to see if the IR LED is flashed.
       Replace the IR LED with a normal LED if you don't have a digital camera
       when debugging.
 *   * Avoid using the following pins unless you really know what you are doing:
 *     * Pin 0/D3: Can interfere with the boot/program mode & support circuits.
 *     * Pin 1/TX/TXD0: Any serial transmissions from the ESP8266 will interfere.
 *     * Pin 3/RX/RXD0: Any serial transmissions to the ESP8266 will interfere.

*/
//Includes
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TimeLib.h>
#include <WidgetRTC.h>
#include "src/IRsend.h"
#include "auth.h" //this file contains auth, ssid and pass
//char auth[] = "1234567890" //Blynk Auth Token;
//char ssid[] = "WiFi SSID";
//char pass[] = "password";


#define ONEMARK 400    //Nr. of usecs for the led to be pulsed for a '1' bit.
#define ONESPACE 400   //Nr. of usecs for the led to be fully off for a '1' bit.
#define ZEROMARK 400   //Nr. of usecs for the led to be pulsed for a '0' bit.
#define ZEROSPACE 1200 //Nr. of usecs for the led to be fully off for a '0' bit.
#define IRLED 26
#define BLYNK_PRINT   Serial
#define GPIO_LED      GPIO_NUM_27 /*LED output pin 27 */


BlynkTimer timer;
WidgetRTC rtc;
WidgetLED led1(V2);

String startTime, stopTime;
int currentHour = 0, currentMin = 0;
int startHour = 1, startMin = 1;
int stopHour = 1, stopMin = 1;
boolean AC_ON = false;
int Button = 0;  //V4

IRsend irsend(IRLED);  // an IR led is connected to the IRLED gpio

//Function that gets start time from Blynk app
BLYNK_WRITE(V0) {
  TimeInputParam t(param);
  startHour = t.getStartHour();
  startMin = t.getStartMinute();
  startTime = String(t.getStartHour()) + ":" + String(t.getStartMinute());
  Serial.println(String("Start time: ") + startTime);
}

//Function that gets stop time from Blynk app
BLYNK_WRITE(V1) {
  TimeInputParam t(param);
  stopHour = t.getStartHour();
  stopMin = t.getStartMinute();
  stopTime = String(t.getStartHour()) + ":" + String(t.getStartMinute());
  Serial.println(String("Stop time: ") + stopTime);
}

//Function that reads Start/stop button V4
BLYNK_WRITE(V4) {
  Button = param.asInt();
}

// Digital clock display of the time
void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute();
  currentHour = hour();
  currentMin = minute();
  //String currentDate = String(day()) + " " + month() + " " + year();
  Serial.print("Current time: ");
  Serial.println(currentTime);
  //Serial.print(currentDate);
  //Serial.println();

  // Send time to the App
  Blynk.virtualWrite(V3, currentTime);
  // Send date to the App
  //Blynk.virtualWrite(V2, currentDate);

}

void sendIR()
{
  irsend.enableIROut(38000, 45); //frequency, duty cycle
  //send header
  irsend.mark(3200);
  irsend.space(1600);
  //send command
  irsend.sendData(ONEMARK, ONESPACE, ZEROMARK, ZEROSPACE, 0xCF, 8, 1);
  irsend.sendData(ONEMARK, ONESPACE, ZEROMARK, ZEROSPACE, 0x00, 8, 1);
  irsend.sendData(ONEMARK, ONESPACE, ZEROMARK, ZEROSPACE, 0xA0, 8, 1);
  irsend.sendData(ONEMARK, ONESPACE, ZEROMARK, ZEROSPACE, 0xC0, 8, 1);
  irsend.sendData(ONEMARK, ONESPACE, ZEROMARK, ZEROSPACE, 0xE0, 8, 1);
  irsend.sendData(ONEMARK, ONESPACE, ZEROMARK, ZEROSPACE, 0xC5, 8, 1);
  irsend.sendData(ONEMARK, ONESPACE, ZEROMARK, ZEROSPACE, 0xE6, 8, 1);
  irsend.sendData(ONEMARK, ONESPACE, ZEROMARK, ZEROSPACE, 0xDF, 8, 1);
  irsend.sendData(ONEMARK, ONESPACE, ZEROMARK, ZEROSPACE, 0x7F, 8, 1);
}

void setup() {

  pinMode(GPIO_LED, OUTPUT);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);

  // Begin synchronizing time
  rtc.begin();
  timer.setInterval(10000L, clockDisplay);

  led1.off();
  irsend.begin();

  Serial.begin(115200); // Debug console
}

void loop() {

  Blynk.run();
  timer.run();

  //Checks if it's time to turn on or off the AC
  if (((startHour == currentHour && startMin == currentMin)||(Button)) && AC_ON == false) {
    digitalWrite(GPIO_LED, HIGH);
    led1.on();
    sendIR(); //sends IR start/stop command
    Serial.println("IR command sent");
    AC_ON = true;
  }
  else if (((stopHour == currentHour && stopMin == currentMin)||(Button)) && AC_ON == true) {
    digitalWrite(GPIO_LED, LOW);
    led1.off();
    sendIR(); //sends IR start/stop command
    Serial.println("IR command sent");
    AC_ON = false;
  }


}
