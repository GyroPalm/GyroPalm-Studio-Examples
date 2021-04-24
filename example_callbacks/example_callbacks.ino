#include <GyroPalmEngine.h>

GyroPalm *device;   //declares a null device
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID

//------------Callback functions-----------------------------------------
void onDeviceTilt(int direction)
{
    /*You should use Enums here, but here are raw values if you want:
      0 = forward, 1 = backward, 2 = right, 3 = left, 4 = upright, 5 = upsidedown

      For left-handed, use LH_FORWARD, etc.
      For right-handed, use RH_FORWARD, etc.
      For device orientation, use TILT_FORWARD, etc.
    */
    Serial.print("Tilted in the ");
    switch(direction)
    {
        case LH_UPRIGHT:
            Serial.println("Upright direction.");
        break;
        
        case LH_FORWARD:
            Serial.println("Forward direction.");
        break;
        
        case LH_BACKWARD:
            Serial.println("Backward direction.");
        break;
        
        case LH_LEFT:
            Serial.println("Left direction.");
        break;
        
        case LH_RIGHT:
            Serial.println("Right direction.");
        break;
        
        case LH_UPSIDEDOWN:
            Serial.println("Upsidedown direction.");
        break;
    }
}

void onActivity(int activity)
{
    switch(activity)
    {
        case WALKING:
            Serial.println("User is WALKING");
        break;

        case RUNNING:
            Serial.println("User is RUNNING");
        break;

        case IDLE:
            Serial.println("User is IDLE");
        break;
    }
}

void onSnap(int snapTimes)
{
    switch(snapTimes)
    {
        case SINGLE_SNAP:
            Serial.println("Performed Single Snap");
        break;
        
        case DOUBLE_SNAP:
            Serial.println("Performed Double Snap");
        break;       
    }
}

void onRawSnap()
{
    Serial.println("Unfiltered Snap Event");
}

void onStep(uint32_t stepCount)
{
    Serial.print("Steps: ");
    Serial.println(stepCount);
}

void onGlance(bool isGlanced)
{
	if (isGlanced) {
	    Serial.println("Watch is ready");
	}
    else {
        Serial.println("Watch is on standby");
    }
}
//------------Callback functions-----------------------------------------

void setup() {
    gplm.begin();
    delay(100);
    gplm.listenEvents(false);    //starts listening for gesture events (set to true for verbose)

    // Include only the callbacks you need:
    gplm.setTiltCallback(onDeviceTilt);
    //gplm.setActivityCallback(onActivity);
    gplm.setSnapCallback(onSnap);
    //gplm.setRawSnapCallback(onRawSnap);
    gplm.setStepCallback(onStep);
    //gplm.setGlanceCallback(onGlance);

    delay(1000);
    device = gplm.wearable;                   //gives control to the developer to run device methods
    Serial.println("I am alive!!!");

    device->tft->setTextColor(TFT_CYAN);
    device->tft->drawString("GyroPalm Callbacks", 5, 25, 4);
}

void loop() {
    Serial.print("Hello World. Loop runs on Core: ");
    Serial.println(xPortGetCoreID());
    delay(2000);
}
