#include <GyroPalmFramework.h>

GyroPalm *device;   //declares a null device
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID

void setup() {
  Serial.begin(115200);                     //start the serial
  Serial.println(gplm.getWearableID());     //print out the wearableID
  gplm.begin();                             //begins display, sensors, etc
  device = gplm.wearable;                   //gives control to the developer to run device methods
  gplm.startDisplay();                      //shows the default example screen
  gplm.vibratePress();                      //vibrate briefly
}

void loop() {
  gplm.startDisplay();                      //continue showing default example screen
  delay(2000);
  device->tft->setCursor(30, 155);          //move cursor to third line
  device->tft->print("  Double fart: ");    //change "double snap" to "double fart"
  delay(2000);                              //wait couple seconds
}
