#include <GyroPalmEngine.h>

GyroPalm *device;   //declares a null device
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID

void setup() {
    gplm.begin();
    delay(1000);
    device = gplm.wearable;                   //gives control to the developer to run device methods
    device->tft->setTextColor(random(0xFFFF));
    device->tft->drawString(" Wearable Vibrate", 3, 25, 4);
}

void loop() {
  Serial.println(gplm.getWearableID());
  delay(1000);
  gplm.vibrateTap();
  delay(1000);
  gplm.vibratePress();
  delay(1000);
  gplm.vibrateHold();
  delay(1000);
}
