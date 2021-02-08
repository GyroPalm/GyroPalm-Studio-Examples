#include <GyroPalmEngine.h>

GyroPalm *device;   //declares a null device
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID

void setup() {
  Serial.println(gplm.getWearableID());     //print out the wearableID
  gplm.begin();                             //begins display, sensors, etc
  gplm.startDisplay();                      //shows the default example screen
  gplm.vibratePress();                      //vibrate briefly
  device = gplm.wearable;                   //gives control to the developer to run device methods
}

void loop() {
  gplm.startDisplay();                      //continue showing default example screen
  delay(2000);
  device->tft->setCursor(30, 155);          //move cursor to third line
  device->tft->print("  Double click: ");    //change "double snap" to "double click"
  delay(2000);                              //wait couple seconds
}