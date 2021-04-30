#include <GyroPalmEngine.h>
#include <GyroPalmLVGL.h>

LV_FONT_DECLARE(digital_play_st_48);
LV_FONT_DECLARE(liquidCrystal_nor_64);

GyroPalm *device;
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID

AXP20X_Class *power;

lv_task_t* task1;
void lv_update_task(struct _lv_task_t *);

//Screen indexes
enum Screen { SCR_HOME, SCR_SETTINGS };
//Screens
lv_obj_t *screen[2];    //screen pointers
GyroPalmLVGL form[2];   //screen helper methods
Screen curScreen = SCR_HOME;    //default screen

uint32_t steps = 0;

static void event_handler(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        Serial.printf("Clicked: %s\n", lv_list_get_btn_text(obj));
        String btnName = lv_list_get_btn_text(obj);

        switch (curScreen)
        {
            case SCR_HOME:  //button from the home screen
                if (btnName == "Settings") {
                    showApp(SCR_SETTINGS);  //switch to settings screen
                }

                if (btnName == "Delete Me") {
                    // your action
                    lv_obj_del(obj);    //removes self when clicked
                }

                if (btnName == "Show Bar") {
                    form[curScreen].createBar(task1, lv_update_task);

                    lv_obj_t * label = lv_obj_get_child(obj, NULL); // get button text
                    lv_label_set_text(label, "Hide Bar"); // change button text
                }

                if (btnName == "Hide Bar") {
                    form[curScreen].removeBar();

                    lv_obj_t * label = lv_obj_get_child(obj, NULL); // get button text
                    lv_label_set_text(label, "Show Bar"); // change button text
                }
            break;
            case SCR_SETTINGS:  //button from the settings screen
                if (btnName == "Home") {
                    // your action
                    showApp(SCR_HOME);
                }
            break;

            default: break;
        }
    }
}

void onPwrQuickPress()
{
    /*
    After the AXP202 interrupt is triggered, the interrupt status must be cleared,
    * otherwise the next interrupt will not be triggered
    */
    power->clearIRQ();

    // We are sleeping the device when power button pressed
    device->displaySleep();
    device->powerOff();
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
}

void onStep(uint32_t stepCount)
{
    steps = stepCount;  //set global var
}

void lv_update_task(struct _lv_task_t *data) {
    int battPercent = power->getBattPercentage();
    bool isCharging = power->isChargeing();
    form[curScreen].updateBar(battPercent, isCharging);

    form[curScreen].toggleTime(steps, gplm.getTime());  //toggle between steps and time
    // OR just pick one: 
    // form[curScreen].setTime(gplm.getTime());     //update Time View
    // form[curScreen].setSteps(steps);        //update Steps View
}

void showApp(int page) {
    if ((Screen) page != curScreen) {
        form[curScreen].removeBar();    //remove old StatusBar before proceeding
    }

    switch (page)
    {
        case SCR_HOME:
            //Draw home UI
            curScreen = (Screen) page;
            form[curScreen].init(screen[curScreen]);  //now defining Home screen items
            form[curScreen].createBar(task1, lv_update_task);
            form[curScreen].setTime(gplm.getTime());
            form[curScreen].createLabel(0, -60, "GyroPalm UI Example");  //create a new label
            form[curScreen].createButton(0, -10, "Settings", event_handler);  //create a new button
            form[curScreen].createButton(0, 40, "Delete Me", event_handler);  //create a new button
            form[curScreen].createButton(0, 90, "Hide Bar", event_handler);  //create a new button
            form[curScreen].showScreen(ANIM_LEFT);   //show the screen w/ sliding left animation
        break;

        case SCR_SETTINGS:
            //Draw settings UI
            curScreen = (Screen) page;
            form[curScreen].init(screen[curScreen]);  //now defining Settings screen items
            form[curScreen].createBar(task1, lv_update_task);
            form[curScreen].setTime(gplm.getTime());
            form[curScreen].showIcon(BAR_BLUETOOTH);
            form[curScreen].showIcon(BAR_GLANCE);
            form[curScreen].createLabel(0, -60, "GyroPalm Settings");  //create a new label
            form[curScreen].createButton(0, 40, "Home", event_handler);  //create a new button
            form[curScreen].showScreen(ANIM_RIGHT);   //show the screen w/ sliding right animation
        break;

        default: break;
    }
}

void setup() {
    gplm.begin();
    delay(100);
    gplm.listenEvents(false);    //starts listening for events (set to true for verbose)

    // Include only the callbacks you need:
    gplm.setStepCallback(onStep);
    gplm.setPwrQuickPressCallback(onPwrQuickPress);
    delay(200);

    device = gplm.wearable;                   //gives control to the developer to run device methods
    
    device->lvgl_begin();               //Initiate LVGL core
    device->bl->adjust(120);            //Lower the brightness

    power = gplm.power;              //gives control to the developer to access power methods
    power->setChargeControlCur(500);    //enable fast charging

    showApp(curScreen);
}

void loop() {
    lv_task_handler();
    delay(50);
}