// Begin AutoGenerated Includes - DO NOT EDIT BELOW
#include <GyroPalmEngine.h>
#include <GyroPalmLVGL.h>
// End AutoGenerated Includes - DO NOT EDIT ABOVE

// Begin AutoGenerated Globals - DO NOT EDIT BELOW
GyroPalm *device;
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID
GyroPalmRealtime realtime;          //declares GyroPalm Realtime object

AXP20X_Class *power;
lv_task_t *barTask;
void lv_update_task(struct _lv_task_t *);

enum Screen { SCR_HOME };	//Screen indexes
lv_obj_t *screen[1];    //screen pointers
GyroPalmLVGL form[1];   //screen helper methods
Screen curScreen = SCR_HOME;    //default screen
// End AutoGenerated Globals - DO NOT EDIT ABOVE

lv_task_t* heartbeatTask;
void lv_heartbeat_task(struct _lv_task_t *);

lv_obj_t * msgboxRead;
lv_obj_t * btnConnect;

// Begin AutoGenerated Callbacks - DO NOT EDIT BELOW
void lv_update_task(struct _lv_task_t *data) {
    int battPercent = power->getBattPercentage();
    bool isCharging = power->isChargeing();
    form[curScreen].updateBar(battPercent, isCharging);
    form[curScreen].setTime(gplm.getTime());     //update Time View
}

static void btn_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED) {
        String btnName = lv_list_get_btn_text(obj);
		Serial.printf("Clicked: %s\n", btnName);

        switch (curScreen)
        {
            case SCR_HOME:
                if (btnName == "Connect") {
                    realtime.connect(gplm.myLicense);   //securely connect to GyroPalm Realtime

                    lv_obj_t * label = lv_obj_get_child(obj, NULL); // get button text
                    lv_label_set_text(label, "Disconnect"); // change button text
                }
                if (btnName == "Disconnect") {
                    realtime.disconnect();

                    lv_obj_t * label = lv_obj_get_child(obj, NULL); // get button text
                    lv_label_set_text(label, "Connect"); // change button text
                }
                if (btnName == "Test A") {
                    realtime.sendSerial("Test A");      //send Realtime String
                }
                if (btnName == "Test B") {
                    realtime.sendSerial("Test B");      //send Realtime String
                }
            break;

            default: break;
		}
	}
}

// End AutoGenerated Callbacks - DO NOT EDIT ABOVE

void showMsg(String msg) {
    msgboxRead = form[curScreen].createMsgBox((char *)msg.c_str(), PROMPT_OK, msgbox_handler, true);
}

static void msgbox_handler(lv_obj_t *obj, String btnText)
{
    if (obj == msgboxRead) {
        Serial.println("Response from MsgBox A");
        msgboxRead = NULL;
    }
    Serial.print("User response: ");
    Serial.println(btnText);
}

// Begin AutoGenerated Screens - DO NOT EDIT BELOW
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
			form[curScreen].createLabel(0, -54, "Realtime Test");    //show element
			btnConnect = form[curScreen].createButton(0, 15, "Connect", btn_event_handler, true, 200);    //show element
			form[curScreen].createButton(-58, 84, "Test A", btn_event_handler, true, 98);    //show element
			form[curScreen].createButton(58, 84, "Test B", btn_event_handler, true, 98);    //show element

            form[curScreen].showScreen(ANIM_NONE);   //show the screen w/ no animation
        }
        break;


        default: break;
    }
}
// End AutoGenerated Screens - DO NOT EDIT ABOVE

void onRealtimeConnection(bool isConnected)
{
    Serial.print("Connection: ");

    lv_obj_t * label = lv_obj_get_child(btnConnect, NULL); // get button text

    if (isConnected) {
        Serial.println("Connected");
        lv_label_set_text(label, "Disconnect"); // change button text
    } else {
        Serial.println("Disconnected");
        lv_label_set_text(label, "Connect"); // change button text
    }
}

void onRealtimeIncoming(String msg)
{
    Serial.print("Incoming: ");
    Serial.println(msg);

    const size_t capacity = JSON_OBJECT_SIZE(2) + 60;
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject& root = jsonBuffer.parseObject(msg);

    String action = root["action"];
    String command = root["command"];

    if (action.indexOf("data") > -1) {
        if (command.indexOf("vibrateTap") > -1) {
            gplm.vibrateTap();
        }
        else if (command.indexOf("vibratePress") > -1) {
            gplm.vibratePress();
        }
        else if (command.indexOf("vibrateHold") > -1) {
            gplm.vibrateHold();
        }
        else if (command.indexOf("msg") > -1) {
            showMsg(String(command));
        }
    }
}

void lv_heartbeat_task(struct _lv_task_t *data) {
    realtime.heartbeat();   //keep GyroPalmRealtime connection alive
}

void setup() {
	
    // Begin AutoGenerated Setup - DO NOT EDIT BELOW
	gplm.begin();
	delay(100);
	gplm.listenEvents(false);    //starts listening for events


	device = gplm.wearable; //gives control to the developer to run device methods
	device->lvgl_begin();   //Initiate LVGL core
	device->bl->adjust(120);    //Lower the brightness
	power = gplm.power;		//gives control to the developer to access power methods
	power->setChargeControlCur(500);    //enable fast charging

	showApp(curScreen);
	// End AutoGenerated Setup - DO NOT EDIT ABOVE

    realtime.connectWiFi();             //Establish connection with saved WiFi, or opens captive portal
    realtime.verboseFlag = false;       //Set to true if you want to see Realtime debug

    // Attach Realtime callbacks
    realtime.setConnectionCallback(onRealtimeConnection);
    realtime.setIncomingCallback(onRealtimeIncoming);

    heartbeatTask = lv_task_create(lv_heartbeat_task, 50000, LV_TASK_PRIO_LOWEST, NULL); //run task every 50 seconds
}

void loop() {
	realtime.loop();    //need this for GyroPalm Realtime to operate

    // Begin AutoGenerated Loop - DO NOT EDIT BELOW
	lv_task_handler();
	delay(50);
	// End AutoGenerated Loop - DO NOT EDIT ABOVE
}
