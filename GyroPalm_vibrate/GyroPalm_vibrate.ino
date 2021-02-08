#include <GyroPalm.h>

GyroPalm gplm("123456");

void setup() {
  Serial.begin(115200);
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
