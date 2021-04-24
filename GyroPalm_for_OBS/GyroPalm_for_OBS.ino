#include <GyroPalmEngine.h>
#include <GyroPalmLVGL.h>
#include <BleKeyboard.h>

LV_FONT_DECLARE(digital_play_st_48);
LV_FONT_DECLARE(liquidCrystal_nor_64);

GyroPalm *device;
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID
BleKeyboard bleKeyboard("GyroPalm Perform BLE");
StatusBar bar;

TFT_eSPI *tft;
AXP20X_Class *power;
bool irq = false;

int snapsPerformed = 0;
long timePerformed = 0;

lv_task_t* task1;
void lv_update_task(struct _lv_task_t *);


static void event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CLICKED) {
        Serial.printf("Clicked: %s\n", lv_list_get_btn_text(obj));
        String btnName = lv_list_get_btn_text(obj);
        if (btnName == "Live") {
            bleKeyboard.write(KEY_ESC);
        }
        if (btnName == "Clone") {
            bleKeyboard.press(KEY_LEFT_CTRL);
            bleKeyboard.press('`'); //backtick
            delay(100);
            bleKeyboard.releaseAll();
        }
        if (btnName == "Mute Mic") {
            bleKeyboard.press(KEY_LEFT_ALT);
            bleKeyboard.press('a');
            delay(100);
            bleKeyboard.releaseAll();

            lv_obj_t * label = lv_obj_get_child(obj, NULL);
            lv_label_set_text(label, "Unmute Mic");
        }
        if (btnName == "Unmute Mic") {
            bleKeyboard.press(KEY_LEFT_ALT);
            bleKeyboard.press('a');
            delay(100);
            bleKeyboard.releaseAll();

            lv_obj_t * label = lv_obj_get_child(obj, NULL);
            lv_label_set_text(label, "Mute Mic");
        }
        if (btnName == "Sleep") {
            /*
            After the AXP202 interrupt is triggered, the interrupt status must be cleared,
            * otherwise the next interrupt will not be triggered
            */
            power->clearIRQ();

            device->displaySleep();
            device->powerOff();
            esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
            esp_deep_sleep_start();
        }
    }
}

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

        case LH_LEFT:   //a snap and flick to the left will toggle the mute
            if (millis() - timePerformed <= 2000 && snapsPerformed == 1) {
                bleKeyboard.press(KEY_LEFT_ALT);
                bleKeyboard.press('a');
                delay(100);
                bleKeyboard.releaseAll();
                
                timePerformed = 0;  //expire it
            }
            else if  (millis() - timePerformed <= 2800 && snapsPerformed == 2) {
                bleKeyboard.press(KEY_LEFT_CTRL);
                bleKeyboard.press('`'); //backtick
                delay(100);
                bleKeyboard.releaseAll();
                
                timePerformed = 0;  //expire it
            }
            snapsPerformed = 0; //reset because we're done
        break;

        case LH_RIGHT:  //we're right back! Resume
            if (millis() - timePerformed <= 2500 && snapsPerformed == 2) {
                bleKeyboard.write(KEY_ESC);
                
                timePerformed = 0;  //expire it
            }
            snapsPerformed = 0; //reset because we're done
        break;

        default:

        break;
    }
}

void onSnap(int snapTimes)
{
    snapsPerformed = snapTimes;
    timePerformed = millis();
}

void onGlance(bool isGlanced)
{
    if (isGlanced) {
        gplm.vibrateTap();
    }
}

void onPwrQuickPress()
{
    /*
    After the AXP202 interrupt is triggered, the interrupt status must be cleared,
    * otherwise the next interrupt will not be triggered
    */
    power->clearIRQ();

    device->displaySleep();
    device->powerOff();
    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
}

void lv_update_task(struct _lv_task_t *data) {
  int battPercent = device->power->getBattPercentage();
  bool isCharging = device->power->isChargeing();
  bar.updateBattLevel(battPercent);
  bar.updateBattIcon(battPercent, isCharging);
}

void setupWatch() {
    //device = GyroPalm::getWatch();
    //device->begin();
    //device->openBL();
    device->lvgl_begin();

    //Lower the brightness
    device->bl->adjust(120);

    power = device->power;

    power->setChargeControlCur(500);    //set charge current to 500 mA
}

void createButton(int x, int y, char *btnName) {
    lv_obj_t * label;
    lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_event_cb(btn1, event_handler);
    lv_obj_align(btn1, NULL, LV_ALIGN_CENTER, x, y);

    label = lv_label_create(btn1, NULL);
    lv_label_set_text(label, btnName);
}

void createLabel(int x, int y, char *lblName) {
    lv_obj_t * label;
    label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, lblName);
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, x, y);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
}

void showApp() {
  lv_obj_t *screen = lv_scr_act();

  static lv_style_t mainStyle;
  lv_style_init(&mainStyle);
  lv_style_set_radius(&mainStyle, LV_OBJ_PART_MAIN, 0);
  lv_style_set_bg_color(&mainStyle, LV_OBJ_PART_MAIN, LV_COLOR_GRAY);
  lv_style_set_bg_opa(&mainStyle, LV_OBJ_PART_MAIN, LV_OPA_0);
  lv_style_set_border_width(&mainStyle, LV_OBJ_PART_MAIN, 0);
  lv_style_set_text_color(&mainStyle, LV_OBJ_PART_MAIN, LV_COLOR_WHITE);
  lv_style_set_image_recolor(&mainStyle, LV_OBJ_PART_MAIN, LV_COLOR_WHITE);

  bar.createIcons(screen);
  int battPercent = device->power->getBattPercentage();
  bool isCharging = device->power->isChargeing();
  bar.updateBattLevel(battPercent);
  bar.updateBattIcon(battPercent, isCharging);

  createLabel(0, -60, "GyroPalm for OBS");
  createButton(0, -10, "Live");
  createButton(0, 40, "Clone");
  createButton(0, 90, "Mute Mic");

  task1 = lv_task_create(lv_update_task, 2000, LV_TASK_PRIO_LOWEST, NULL);
}

void hideApp() {
  lv_task_del(task1);
  //lv_obj_del(mainBar);
  bar.deleteObj();
}

void setup() {
    gplm.begin();
    delay(100);
    gplm.listenEvents(false);    //starts listening for events (set to true for verbose)

    // Include only the callbacks you need:
    gplm.setPwrQuickPressCallback(onPwrQuickPress);
    gplm.setTiltCallback(onDeviceTilt);
    gplm.setSnapCallback(onSnap);
    gplm.setGlanceCallback(onGlance);
    delay(200);

    device = gplm.wearable;                   //gives control to the developer to run device methods
    setupWatch();
    showApp();
    bleKeyboard.begin();
}

void loop() {
    lv_task_handler();
    delay(50);
}