#include <GyroPalmEngine.h>
#include <GyroPalmLVGL.h>

GyroPalm *device;
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID

AXP20X_Class *power;
lv_task_t *barTask;
void lv_update_task(struct _lv_task_t *);

enum Screen { SCR_HOME, SCR_PAGE1 };	//Screen indexes
lv_obj_t *screen[2];    //screen pointers
GyroPalmLVGL form[2];   //screen helper methods
Screen curScreen = SCR_HOME;    //default screen
// End AutoGenerated Globals - DO NOT EDIT ABOVE

lv_obj_t * btnPage1;
lv_obj_t * btnHome;
bool isActive = false;  //whether we are active
int lastActivated = 0;  //timestamp of when we double snap

void lv_update_task(struct _lv_task_t *data) {
    int battPercent = power->getBattPercentage();
    bool isCharging = power->isChargeing();
    form[curScreen].updateBar(battPercent, isCharging);
    form[curScreen].setTime(gplm.getTime());     //update Time View
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

static void btn_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        String btnName = lv_list_get_btn_text(obj);
		Serial.printf("Clicked: %s\n", btnName);

        switch (curScreen)
        {
            case SCR_HOME:
                if (btnName == "Page 1") {
                    showApp(SCR_PAGE1);
                }
            break;

            case SCR_PAGE1:
                if (btnName == "Home") {
                    showApp(SCR_HOME);
                }
            break;

            default: break;
		}
	}
}

static void slider_event_handler(lv_obj_t * slider, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
		int sliderVal = lv_slider_get_value(slider);
		
        switch (curScreen)
        {
            case SCR_HOME:
			
            break;

            default: break;
        }
    }
}

void onDeviceTilt(int direction)
{
    if (isActive != true) {
        return;
    } else {
        isActive = false;
    }
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
            lv_event_send(btnPage1, LV_EVENT_CLICKED, NULL);
        break;

        case LH_RIGHT:
            Serial.println("Right direction.");
            lv_event_send(btnHome, LV_EVENT_CLICKED, NULL);
        break;

        case LH_UPSIDEDOWN:
            Serial.println("Upsidedown direction.");
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
            isActive = true;
            lastActivated = millis();
        break;       
    }
}

void onGlance(bool isGlanced)
{
    if (isGlanced) {
        form[curScreen].showIcon(BAR_GLANCE);
    } else {
        form[curScreen].hideIcon(BAR_GLANCE);
    }
}

void showApp(int page) {
    if ((Screen) page != curScreen) {
        form[curScreen].removeBar();    //remove old StatusBar before proceeding
    }

    switch (page)
    {
		case SCR_HOME:
        {
            //Draw screen UI
            curScreen = (Screen) page;
            form[curScreen].init(screen[curScreen]);  //now defining screen items
            form[curScreen].createBar(barTask, lv_update_task);
            form[curScreen].setTime(gplm.getTime());
			form[curScreen].createLED(-77, -54, true);    //show element
			form[curScreen].createLED(0, -54, true);    //show element
			form[curScreen].createLED(77, -54, true);    //show element
			form[curScreen].createLabel(0, 15, "Hello World");    //show element
			btnPage1 = form[curScreen].createButton(-58, 84, "Page 1", btn_event_handler, true, 98);    //show element
			form[curScreen].createButton(58, 84, "Test", btn_event_handler, true, 98);    //show element

            form[curScreen].showScreen(ANIM_LEFT);   //show the screen w/ no animation
        }
        break;

		case SCR_PAGE1:
        {
            //Draw screen UI
            curScreen = (Screen) page;
            form[curScreen].init(screen[curScreen]);  //now defining screen items
            form[curScreen].createBar(barTask, lv_update_task);
            form[curScreen].setTime(gplm.getTime());
			form[curScreen].createSlider(-58, -37, 0, 100, slider_event_handler, 98, 50);    //show element
			form[curScreen].createSlider(58, -37, 0, 100, slider_event_handler, 98, 50);    //show element
			btnHome = form[curScreen].createButton(-77, 67, "Home", btn_event_handler, true, 59);    //show element
			form[curScreen].createLabel(0, 67, "Test");    //show element
			form[curScreen].createLED(77, 67, true);    //show element

            form[curScreen].showScreen(ANIM_NONE);   //show the screen w/ no animation
        }
        break;

        default: break;
    }
}

void setup() {

	gplm.begin();
	delay(100);
	gplm.listenEvents(false);    //starts listening for events

    // Include only the callbacks you need:
    gplm.setPwrQuickPressCallback(onPwrQuickPress);
    gplm.setTiltCallback(onDeviceTilt);
    gplm.setSnapCallback(onSnap);
    gplm.setGlanceCallback(onGlance);

	device = gplm.wearable; //gives control to the developer to run device methods
	device->lvgl_begin();   //Initiate LVGL core
	device->bl->adjust(120);    //Lower the brightness
	power = gplm.power;		//gives control to the developer to access power methods
	power->setChargeControlCur(500);    //enable fast charging

	showApp(curScreen);

}

void loop() {
    if (isActive) { //validate activation time
        if (millis() - lastActivated > 3000) {  //been more than 2 seconds
            isActive = false;
        }
    }

	lv_task_handler();
	delay(50);
}
