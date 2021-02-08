#include <GyroPalmEngine.h>
#include <BleKeyboard.h>

GyroPalmEngine gplm("gp123456");
BleKeyboard bleKeyboard("GyroPalm Perform BLE");

bool snapped = false;

//------------Callback functions-----------------------------------------
void onDeviceTilt(int direction)
{
    /* normally, returns -1 unless tilted a new direction
    4 = upright
    0 = forward
    1 = backward
    3 = left
    2 = right
    5 = upsidedown
    */
    Serial.print("Device tilted in the ");
    switch(direction)
    {
        case 4:
            Serial.println("Upright direction.");
        break;
        
        case 0:
            Serial.println("forward direction.");  //swipe left wrt wearer
            if (snapped) {
                bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
                delay(100);
                bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
            }
        break;
        
        case 1:
            Serial.println("backward direction.");  //swipe right wrt wearer
            if (snapped) {
                bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
            }
        break;
        
        case 3:
            Serial.println("left direction.");
        break;
        
        case 2:
            Serial.println("right direction.");
        break;
        
        case 5:
            Serial.println("upside direction.");
        break;

        default:
            Serial.println("no direction.");
        break;
    }
    snapped = false;    //remove control
}
void onActivity(const char* act)
{
    Serial.println(act);
}
void onSnap(int snapTimes)
{
    if (snapTimes == 1) {
        snapped = true;
    }
    else {
        snapped = false;
    }
    Serial.print("Snaped ");
    Serial.print(snapTimes);
    Serial.println(" times");
    if (snapTimes == 2) {
        bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
    }
}
void onStep(uint32_t stepCount)
{
    Serial.print("Steps: ");
    Serial.println(stepCount);
    Serial.print("This Task runs on Core: ");
    Serial.println(xPortGetCoreID());
}
void onGlance()
{
    Serial.println("Haha made you look!");
}
//------------Callback functions-----------------------------------------

void setup() {
    gplm.begin();
    delay(100);
    bleKeyboard.begin();
    gplm.listenEvents(true);    //starts the core 0 task

    gplm.setTiltCallback(onDeviceTilt);
    //gplm.setActivityCallback(onActivity);
    gplm.setSnapCallback(onSnap);
    //gplm.setStepCallback(onStep);
    //gplm.setGlanceCallback(onGlance);

    delay(1000);
    Serial.println("I'm alive!!!");

}

void loop() {
    Serial.print("Hello World. Loop runs on Core: ");
    Serial.println(xPortGetCoreID());
    delay(1000);
    /*
    if(bleKeyboard.isConnected()) {
    Serial.println("Sending 'Hello world'...");
    bleKeyboard.print("Hello world");

    delay(1000);

    Serial.println("Sending Enter key...");
    bleKeyboard.write(KEY_RETURN);

    delay(1000);

    Serial.println("Sending Play/Pause media key...");
    bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);

    delay(1000);

    Serial.println("Sending Ctrl+Alt+Delete...");
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press(KEY_DELETE);
    delay(100);
    bleKeyboard.releaseAll();
  }
  */
}
