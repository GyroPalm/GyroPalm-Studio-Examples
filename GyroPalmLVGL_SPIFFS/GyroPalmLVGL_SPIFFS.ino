#include <FS.h>   //this needs to be first, or it all crashes and burns...
#include <GyroPalmEngine.h>
#include <GyroPalmLVGL.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define USE_SERIAL Serial

WiFiMulti wifiMulti;

GyroPalm *device;
GyroPalmEngine gplm("gp123456");    //declares a GyroPalm Engine object with wearableID

AXP20X_Class *power;
lv_task_t *barTask;
void lv_update_task(struct _lv_task_t *);

enum Screen { SCR_HOME };	//Screen indexes
lv_obj_t *screen[1];    //screen pointers
GyroPalmLVGL form[1];   //screen helper methods
Screen curScreen = SCR_HOME;    //default screen

lv_obj_t * msgboxRead;

static const uint8_t num_masters = 4;   //number of trained gestures
static const uint8_t DOF = 3;           //DOF of each gesture
static const uint8_t sample_size = 50;  //data points per gesture

typedef struct {
    char wearableID[15];
    char secret[15];
    int interval;
    char endpoint[50];
    long lastSteps;
    long gesturesPerformed;
    long powerCycles;
    int brightness;
    int screenTimeout;
    int numGestures;
    char gestureNames[num_masters][15];
    char gestureIDs[num_masters][15];
    int master[num_masters][DOF][sample_size];       //array to store master gesture
} MainSettings;

MainSettings mySettings;

void saveSPIFFS(int mode)
{
    switch (mode)
    {
        case 1:
        {
            strncpy( mySettings.wearableID, "data A", sizeof(mySettings.wearableID) );
            mySettings.interval = 41;
            for (int i = 0; i < 2; i++) {
                for (int dof = 0; dof < 3; dof++) {
                    for (int samples = 0; samples < 5; samples++) {
                        mySettings.master[i][dof][samples] = samples + 10;
                    }
                }
            }
        }
        break;

        case 2:
        {
            strncpy( mySettings.wearableID, "data B", sizeof(mySettings.wearableID) );
            mySettings.interval = 42;
            for (int i = 0; i < 2; i++) {
                for (int dof = 0; dof < 3; dof++) {
                    for (int samples = 0; samples < 5; samples++) {
                        mySettings.master[i][dof][samples] = samples + 20;
                    }
                }
            }
        }
        break;

        case 3:
        {
            strncpy( mySettings.wearableID, "data C", sizeof(mySettings.wearableID) );
            mySettings.interval = 43;
            for (int i = 0; i < 2; i++) {
                for (int dof = 0; dof < 3; dof++) {
                    for (int samples = 0; samples < 5; samples++) {
                        mySettings.master[i][dof][samples] = samples + 30;
                    }
                }
            }
        }
        break;

        default: break;
    }
    File myFile = SPIFFS.open("/settings.txt", "w");
    myFile.write((byte *)&mySettings, sizeof(mySettings));
    myFile.close();
}

void readSPIFFS() 
{
    /*
    File myFile = SPIFFS.open("/settings.txt", "r");
    myFile.read((byte *)&mySettings, sizeof(mySettings));
    myFile.close();
    */
    Serial.printf( "read: %s, %i, %s\n", mySettings.wearableID, mySettings.interval, mySettings.endpoint );
    String msg = String(mySettings.wearableID) + "\n";
    msg.concat(String(mySettings.interval) + "\n");
    msg.concat(String(mySettings.endpoint) + "\n");
    for (int i = 0; i < 2; i++) {
        msg.concat("G" + String(i) + ": ");
        for (int dof = 0; dof < 3; dof++) {
            if (dof == 0) {
                msg.concat("X: ");
            } else if (dof == 1) {
                msg.concat("Y: ");                
            } else if (dof == 2) {
                msg.concat("Z: ");                
            }
            
            for (int samples = 0; samples < 5; samples++) {
                msg.concat(String(mySettings.master[i][dof][samples]) + ",");
            }
        }
        msg.concat("\n");
    }

    showMsg(msg);
    sendSettings("https://webhook.site/e256125a-740d-47af-87eb-5f8f80d475ec");
}

void lv_update_task(struct _lv_task_t *data) {
    int battPercent = power->getBattPercentage();
    bool isCharging = power->isChargeing();
    form[curScreen].updateBar(battPercent, isCharging);
    form[curScreen].setTime(gplm.getTime());     //update Time View
}

void showMsg(String msg) {
    msgboxRead = form[curScreen].createMsgBox((char *)msg.c_str(), PROMPT_OK, msgbox_handler, false);
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
            {
                if (btnName == "Set A") {
                    saveSPIFFS(1);
                }
                else if (btnName == "Set B") {
                    saveSPIFFS(2);
                }
                else if (btnName == "Set C") {
                    saveSPIFFS(3);
                }
                else if (btnName == "Files") {

                    File root = SPIFFS.open("/");
                    File file = root.openNextFile();

                    String msg = "FILES: \n";
                    while (file) {
                        msg.concat(String(file.name()) + "\n");
                        file = root.openNextFile();
                    }
                    showMsg(msg);

                    file.close();
                }
                else if (btnName == "Get") {
                    getSettings("https://webhook.site/e256125a-740d-47af-87eb-5f8f80d475ec");
                }
                else if (btnName == "Retrieve") {
                    readSPIFFS();
                }
            }
            break;

            default: break;
		}
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
			form[curScreen].createLabel(0, -54, "SPIFFS Test");    //show element
			form[curScreen].createButton(-77, 15, "Set A", btn_event_handler, true, 59);    //show element
			form[curScreen].createButton(0, 15, "Set B", btn_event_handler, true, 59);    //show element
			form[curScreen].createButton(77, 15, "Set C", btn_event_handler, true, 59);    //show element
			form[curScreen].createButton(-58, 84, "Get", btn_event_handler, true, 98);    //show element
			form[curScreen].createButton(58, 84, "Retrieve", btn_event_handler, true, 98);    //show element

            form[curScreen].showScreen(ANIM_NONE);   //show the screen w/ no animation
        }
        break;


        default: break;
    }
}

void formatSPIFFS()
{
    if (!SPIFFS.begin(true)) {
        Serial.println("Error mounting SPIFFS");
        return;
    }
    if (!SPIFFS.exists("/formatComplete.txt")) 
    {
        Serial.println("Please wait 30 secs for SPIFFS to be formatted");
        SPIFFS.format();
        Serial.println("Spiffs formatted");

        File f = SPIFFS.open("/formatComplete.txt", "w");
        if (!f) {
            Serial.println("file open failed");
        } else {
            f.println("Format Complete");
        }
        f.close();
    } 
    else {
        Serial.println("SPIFFS is formatted. Moving along...");
    }
    /*
    File f = SPIFFS.open("/settings.txt", "r");
    if (!f) {
        Serial.println("file open failed");
    }
    else
    {
        // Read 3 lines from SPIFFS. Line 1: sensorID, Line 2: interval, Line 3: endpoint
        for (int i = 0; i < 3; i++) {
            String s = f.readStringUntil('\n');
            if (i == 0)
            {
                s.trim();
                //sensorID = s;
                //Serial.print("sensorID: ");
                //Serial.println(sensorID);
            }
            else if (i == 1)
            {
                s.trim();
                //interval = s.toInt();
                //Serial.print("Interval: ");
                //Serial.println(interval);
            }
            else if (i == 2)
            {
                s.trim();
                //endpoint = s;
                //Serial.print("Endpoint: ");
                //Serial.println(endpoint);
            }
        }
        f.close();
    }
    */

}

void sendSettings(String endpoint) {
    if((wifiMulti.run() == WL_CONNECTED)) {
        // Convert mySettings Struct to JSON -------------------------------
        const size_t capacity = 10*JSON_ARRAY_SIZE(3) + 3*JSON_ARRAY_SIZE(10) + 30*JSON_ARRAY_SIZE(52) + JSON_OBJECT_SIZE(12) + 5410;
        DynamicJsonBuffer jsonBuffer(capacity);

        JsonObject& root = jsonBuffer.createObject();
        root["wearableID"] = mySettings.wearableID;
        root["secret"] = mySettings.secret;
        root["interval"] = mySettings.interval;
        root["endpoint"] = mySettings.endpoint;
        root["numGestures"] = mySettings.numGestures;

        JsonArray& gestureNames = root.createNestedArray("gestureNames");
        for (int i = 0; i < mySettings.numGestures; i++) {
            char * gestNames = mySettings.gestureNames[i];
            gestureNames.add(gestNames);
        }

        JsonArray& gestureIDs = root.createNestedArray("gestureIDs");
        for (int i = 0; i < mySettings.numGestures; i++) {
            char * gestIDs = mySettings.gestureIDs[i];
            gestureIDs.add(gestIDs);
        }

        JsonArray& master = root.createNestedArray("master");
        for (int i = 0; i < mySettings.numGestures; i++) {
            JsonArray& master_gest = master.createNestedArray();
            for (int dof = 0; dof < 3; dof++) {
                if (dof == 0) {
                    JsonArray& master_dofX = master_gest.createNestedArray();
                    for (int samples = 0; samples < sample_size; samples++) {
                        master_dofX.add(mySettings.master[i][dof][samples]);
                    }
                }
                else if (dof == 1) {
                    JsonArray& master_dofY = master_gest.createNestedArray();
                    for (int samples = 0; samples < sample_size; samples++) {
                        master_dofY.add(mySettings.master[i][dof][samples]);
                    }
                }
                else if (dof == 2) {
                    JsonArray& master_dofZ = master_gest.createNestedArray();
                    for (int samples = 0; samples < sample_size; samples++) {
                        master_dofZ.add(mySettings.master[i][dof][samples]);
                    }
                }
            }
        }

        root["lastSteps"] = mySettings.lastSteps;
        root["gesturesPerformed"] = mySettings.gesturesPerformed;
        root["powerCycles"] = mySettings.powerCycles;
        root["brightness"] = mySettings.brightness;
        root["screenTimeout"] = mySettings.screenTimeout;

        // End of mySettings Struct to JSON -------------------------------

        HTTPClient http;
        http.setUserAgent("GyroPalmEncore");
        
        USE_SERIAL.print("[HTTP] begin...\n");
        http.begin(endpoint); //HTTP
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Wearable-Id", "abc123");

        // start connection and send HTTP header
        String packet;
        root.printTo(packet);
        int httpCode = http.POST(packet);

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
}

void getSettings(String endpoint) {
    if((wifiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;
        http.setUserAgent("GyroPalmEncore");
        
        USE_SERIAL.print("[HTTP] begin...\n");
        http.begin(endpoint); //HTTP
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Wearable-Id", "abc123");

        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                //USE_SERIAL.println(payload);
                // Convert mySettings JSON to Struct -------------------------------
                const size_t capacity = 10*JSON_ARRAY_SIZE(3) + 3*JSON_ARRAY_SIZE(10) + 30*JSON_ARRAY_SIZE(52) + JSON_OBJECT_SIZE(12) + 5410;
                DynamicJsonBuffer jsonBuffer(capacity);

                JsonObject& root = jsonBuffer.parseObject(payload);
                strlcpy(mySettings.wearableID, root["wearableID"], sizeof(mySettings.wearableID));
                strlcpy(mySettings.secret, root["secret"], sizeof(mySettings.secret));
                mySettings.interval = root["interval"];
                strlcpy(mySettings.endpoint, root["endpoint"], sizeof(mySettings.endpoint));
                mySettings.numGestures = root["numGestures"];

                const char* gestureNames_0 = root["gestureNames"][0]; // "Swipe Up"
                const char* gestureIDs_0 = root["gestureIDs"][0]; // "gestureAAA"

                for (int i = 0; i < mySettings.numGestures; i++) {
                    strlcpy(mySettings.gestureNames[i], root["gestureNames"][i], sizeof(mySettings.gestureNames[i]));
                    strlcpy(mySettings.gestureIDs[i], root["gestureIDs"][i], sizeof(mySettings.gestureIDs[i]));
                }

                mySettings.lastSteps = root["lastSteps"];
                mySettings.gesturesPerformed = root["gesturesPerformed"];
                mySettings.powerCycles = root["powerCycles"];
                mySettings.brightness = root["brightness"];
                mySettings.screenTimeout = root["screenTimeout"];
                // End of mySettings JSON to Struct -------------------------------

                Serial.print("WearableID: ");
                Serial.print(mySettings.wearableID);
                Serial.print("\tSecret: ");
                Serial.print(mySettings.secret);
                Serial.print("\tInterval: ");
                Serial.println(mySettings.interval);

                Serial.print("Endpoint: ");
                Serial.print(mySettings.endpoint);
                Serial.print("\tnumGestures: ");
                Serial.print(mySettings.numGestures);
                Serial.print("\tlastSteps: ");
                Serial.println(mySettings.lastSteps);

                Serial.print("gesturesPerformed: ");
                Serial.print(mySettings.gesturesPerformed);
                Serial.print("\tpowerCycles: ");
                Serial.print(mySettings.powerCycles);
                Serial.print("\tbrightness: ");
                Serial.print(mySettings.brightness);
                Serial.print("\tscreenTimeout: ");
                Serial.println(mySettings.screenTimeout);

                Serial.println("gestureNames:");
                for (int i = 0; i < mySettings.numGestures; i++) {
                    Serial.print(mySettings.gestureNames[i]);
                    Serial.print("\t");
                    Serial.println(mySettings.gestureIDs[i]);
                }

                JsonArray& master_0 = root["master"][0];

                JsonArray& master = root["master"];
                for (int i = 0; i < mySettings.numGestures; i++) {
                    Serial.println("Gest" + String(i));
                    JsonArray& master_gest = master[i];
                    for (int dof = 0; dof < 3; dof++) {
                        if (dof == 0) {
                            Serial.print("X: ");
                            JsonArray& master_dofX = master_gest[dof];
                            for (int samples = 0; samples < sample_size; samples++) {
                                mySettings.master[i][dof][samples] = master_dofX[samples];
                                Serial.print(mySettings.master[i][dof][samples]);
                                Serial.print(",");
                            }
                            Serial.println();
                        }
                        else if (dof == 1) {
                            Serial.print("Y: ");
                            JsonArray& master_dofY = master_gest[dof];
                            for (int samples = 0; samples < sample_size; samples++) {
                                mySettings.master[i][dof][samples] = master_dofY[samples];
                                Serial.print(mySettings.master[i][dof][samples]);
                                Serial.print(",");
                            }
                            Serial.println();
                        }
                        else if (dof == 2) {
                            Serial.print("Z: ");
                            JsonArray& master_dofZ = master_gest[dof];
                            for (int samples = 0; samples < sample_size; samples++) {
                                mySettings.master[i][dof][samples] = master_dofZ[samples];
                                Serial.print(mySettings.master[i][dof][samples]);
                                Serial.print(",");
                            }
                            Serial.println();
                        }
                    }
                }
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
}

void setup() {

	gplm.begin();
	delay(100);
	gplm.listenEvents(false);    //starts listening for events

    gplm.setPwrQuickPressCallback(onPwrQuickPress);

	device = gplm.wearable; //gives control to the developer to run device methods
	device->lvgl_begin();   //Initiate LVGL core
	device->bl->adjust(120);    //Lower the brightness
	power = gplm.power;		//gives control to the developer to access power methods
	power->setChargeControlCur(500);    //enable fast charging

	showApp(curScreen);

    formatSPIFFS(); //format SPIFFS if first time

    wifiMulti.addAP("DLwireless", "22342234");

    strncpy( mySettings.wearableID, "gp123456789", sizeof(mySettings.wearableID) );
    strncpy( mySettings.secret, "12345", sizeof(mySettings.secret) );
    mySettings.interval = 40;
    strncpy( mySettings.endpoint, "syncota.com", sizeof(mySettings.endpoint) );
    mySettings.lastSteps = 1;
    mySettings.gesturesPerformed = 5;
    mySettings.powerCycles = 100;
    mySettings.brightness = 120;
    mySettings.screenTimeout = 10;
    mySettings.numGestures = 2;
    strncpy( mySettings.gestureNames[0], "Swipe Up", sizeof(mySettings.gestureNames[0]) );
    strncpy( mySettings.gestureNames[1], "Swipe Down", sizeof(mySettings.gestureNames[1]) );
    strncpy( mySettings.gestureIDs[0], "gestureAAA", sizeof(mySettings.gestureIDs[0]) );
    strncpy( mySettings.gestureIDs[1], "gestureBBB", sizeof(mySettings.gestureIDs[1]) );
    for (int i = 0; i < mySettings.numGestures; i++) {
        for (int dof = 0; dof < 3; dof++) {
            for (int samples = 0; samples < sample_size; samples++) {
                mySettings.master[i][dof][samples] = samples + 80 + dof + i;
            }
        }
    }

}

void loop() {

	lv_task_handler();
	delay(50);

}
