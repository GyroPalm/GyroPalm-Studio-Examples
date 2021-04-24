#include <GyroPalmEngine.h>
#include <GyroPalmLVGL.h>

LV_FONT_DECLARE(digital_play_st_48);
LV_FONT_DECLARE(liquidCrystal_nor_64);

GyroPalm *device;
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID
GyroPalmLVGL ui;
StatusBar bar;

TFT_eSPI *tft;
AXP20X_Class *power;
bool irq = false;

lv_task_t* task1;
void lv_update_task(struct _lv_task_t *);

static void event_handler(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        Serial.printf("Clicked: %s\n", lv_list_get_btn_text(obj));
        String btnName = lv_list_get_btn_text(obj);

        if (btnName == "Hello") {
            // your action
        }
        if (btnName == "Delete Me") {
            // your action
            lv_obj_del(obj);    //removes self when clicked
        }
        if (btnName == "Lights ON") {
            // your lights ON code

            lv_obj_t * label = lv_obj_get_child(obj, NULL); // get button text
            lv_label_set_text(label, "Lights OFF"); // change button text
        }
        if (btnName == "Lights OFF") {
            // your lights OFF code

            lv_obj_t * label = lv_obj_get_child(obj, NULL); // get button text
            lv_label_set_text(label, "Lights ON"); // change button text
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

void lv_update_task(struct _lv_task_t *data) {
    int battPercent = device->power->getBattPercentage();
    bool isCharging = device->power->isChargeing();
    bar.updateBattLevel(battPercent);
    bar.updateBattIcon(battPercent, isCharging);
}

void showApp() {
  lv_obj_t *screen = lv_scr_act();

  static lv_style_t mainStyle;  //This is the main theme
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

  ui.createLabel(0, -60, "GyroPalm UI Example");
  ui.createButton(0, -10, "Hello", event_handler);
  ui.createButton(0, 40, "Delete Me", event_handler);
  ui.createButton(0, 90, "Lights ON", event_handler);

  task1 = lv_task_create(lv_update_task, 2000, LV_TASK_PRIO_LOWEST, NULL);
}

void hideApp() {
  lv_task_del(task1);
  bar.deleteObj();
}

void setup() {
    gplm.begin();
    delay(100);
    gplm.listenEvents(false);    //starts listening for events (set to true for verbose)

    // Include only the callbacks you need:
    gplm.setPwrQuickPressCallback(onPwrQuickPress);
    delay(200);

    device = gplm.wearable;                   //gives control to the developer to run device methods

    device->lvgl_begin();               //Initiate LVGL core
    device->bl->adjust(120);            //Lower the brightness

    power = device->power;              //define power object
    power->setChargeControlCur(500);    //enable fast charging

    showApp();
}

void loop() {
    lv_task_handler();
    delay(50);
}