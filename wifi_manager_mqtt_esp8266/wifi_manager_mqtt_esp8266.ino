#include <FS.h>   
#include <ESP8266WiFi.h>          
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          
#include <ArduinoJson.h>     
#include <PubSubClient.h>     

// RESET 
int PIN_RESET_SETTING = 15;

// MQTT
char ieeeAddr[40];
char mqtt_server[40];

WiFiClient espClient;
PubSubClient client(espClient);

// wifi manager
bool shouldSaveConfig = false;

// ###########
// mqtt server
// ###########

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
    
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);

        if (client.connect(clientId.c_str())) { 
            Serial.println("MQTT Connected...");

        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}


// ############
// wifi manager
// ############

void saveConfigCallback () {
    Serial.println("save new config...");
    shouldSaveConfig = true;
}

void wifi_manager(boolean reset_setting) {

    WiFiManager wifiManager;

    // reset settings ?
    if (reset_setting == true) {
        wifiManager.resetSettings();     // reset wifi settings
        SPIFFS.format();                 // reset config.txt
    }    

    // mounting filesystem
    
    Serial.print("mounting FS...");

    if (SPIFFS.begin()) {    
      
        Serial.println("successful");

        // read config file
        
        if (SPIFFS.exists("/config.json")) {
            Serial.print("reading config file...");
            File config_File = SPIFFS.open("/config.json", "r");

            if (config_File) {
              
                size_t size = config_File.size();
                std::unique_ptr<char[]> buf(new char[size]);
                config_File.readBytes(buf.get(), size);
                
                DynamicJsonDocument config_json(128);
                DeserializationError error = deserializeJson(config_json, buf.get());
                
                if (!error) {
                    strcpy(mqtt_server, config_json["mqtt_server"]);
                    Serial.println("successful");
          
                } else {
                    Serial.println("failed");
                }
                config_File.close();
            } 
        }
        
    } else {
        Serial.println("failed");
    }

    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);

    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.addParameter(&custom_mqtt_server);

    if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        ESP.reset();
        delay(5000);
    }
  
    strcpy(mqtt_server, custom_mqtt_server.getValue());

    // save new config

    if (shouldSaveConfig) {
        
        DynamicJsonDocument json_data(128);
        json_data["mqtt_server"] = mqtt_server;
    
        File configFile = SPIFFS.open("/config.json", "w");
        
        if (!configFile) {
            Serial.println("failed to open config file for writing");
        }
        
        serializeJson(json_data, Serial);
        Serial.println();
        serializeJson(json_data, configFile);
        configFile.close();
        Serial.println("new config file saved");
    }
}


// ########
// ieeeAddr
// ########

void get_ieeeAddr() {
    Serial.print("mounting FS...");
  
    if (SPIFFS.begin()) {    
      
        Serial.println("successful");

        // read ieeeAddr file
        
        if (SPIFFS.exists("/ieeeAddr.json")) {
            Serial.print("reading ieeeAddr file...");
            File ieeeAddr_File = SPIFFS.open("/ieeeAddr.json", "r");

            if (ieeeAddr_File) {
                size_t size = ieeeAddr_File.size();
                std::unique_ptr<char[]> buf(new char[size]);
                ieeeAddr_File.readBytes(buf.get(), size);
                
                DynamicJsonDocument ieeeAddr_json(128);
                DeserializationError error = deserializeJson(ieeeAddr_json, buf.get());
                
                if (!error) {
                    strcpy(ieeeAddr, ieeeAddr_json["ieeeAddr"]);
                    Serial.println("successful");
          
                } else {
                    Serial.println("failed");
                }
                ieeeAddr_File.close();
            } 

        // ieeeAddr file not exist, generate new ieeeAddr + file
            
        } else {

            Serial.print("generate new ieeeAddr...");

            int randNumber            = random(100000, 999999);
            String ieeeAddr_generated = "0x" + String(randNumber); 
            
            ieeeAddr_generated.toCharArray(ieeeAddr,40); 

            DynamicJsonDocument ieeeAddr_json(128);
            ieeeAddr_json["ieeeAddr"] = ieeeAddr_generated;
        
            File ieeeAddr_File = SPIFFS.open("/ieeeAddr.json", "w");
            
            if (!ieeeAddr_File) {
                Serial.println("failed to open ieeeAddr file for writing");
            }
    
            serializeJson(ieeeAddr_json, Serial);
            Serial.println();
            serializeJson(ieeeAddr_json, ieeeAddr_File);
            ieeeAddr_File.close();
            Serial.println("new ieeeAddr file saved");
        }

    } else {
        Serial.println("failed");
    }
}


// #####
// setup
// #####

void setup() {
    Serial.begin(115200);
    Serial.println();

    pinMode(PIN_RESET_SETTING,INPUT);

    //if (digitalRead(PIN_RESET_SETTING) == 1) {
    //    wifi_manager(true);  // reset settings
    //} else {
    //    wifi_manager(false);
    //}
  
    wifi_manager(false);
    Serial.println(mqtt_server);

    get_ieeeAddr();
    Serial.println(ieeeAddr);
    
    client.setServer(mqtt_server, 1883);

}


// ####
// loop
// ####

void loop() {

    if (!client.connected()) {
        reconnect();
    }
    client.loop();

}
