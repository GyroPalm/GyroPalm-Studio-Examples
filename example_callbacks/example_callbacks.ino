#include <GyroPalmEngine.h>

GyroPalmEngine gplm("gp123456");

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
            Serial.println("forward direction.");
        break;
        
        case 1:
            Serial.println("backward direction.");
        break;
        
        case 3:
            Serial.println("left direction.");
        break;
        
        case 2:
            Serial.println("righht direction.");
        break;
        
        case 5:
            Serial.println("upside direction.");
        break;

        default:
            Serial.println("no direction.");
        break;
        
    }
}
void onActivity(const char* act)
{
    Serial.println(act);
}
void onSnap(int snapTimes)
{
    Serial.print("You snapped it ");
    Serial.print(snapTimes);
    Serial.println(" times.");
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
    gplm.listenEvents(true);    //starts the core 0 task

    //gplm.setTiltCallback(onDeviceTilt);
    gplm.setActivityCallback(onActivity);
    gplm.setSnapCallback(onSnap);
    gplm.setStepCallback(onStep);
    gplm.setGlanceCallback(onGlance);

    delay(1000);
    Serial.println("I'm alive!!!");

}

void loop() {
    Serial.print("Hello World. Loop runs on Core: ");
    Serial.println(xPortGetCoreID());
    delay(1000);
}
