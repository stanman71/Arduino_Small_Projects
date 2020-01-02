#include <FS.h>                   
#include <SPIFFS.h>  
#include <WiFi.h>          
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>          
#include <ArduinoJson.h>     

// RESET 
int PIN_RESET_SETTING = 15;

char ieeeAddr[40];
char smarthome_server[40];

WiFiClient espClient;

// wifi manager
bool shouldSaveConfig = false;


// ############
// wifi manager
// ############

void saveConfigCallback () {
    Serial.println("save new config...");
    shouldSaveConfig = true;
}

void wifi_manager(boolean reset_setting) {

    WiFiManager wifiManager;

    // reset settings 
    if (reset_setting == true) {
        SPIFFS.format();                

        // workaround delete wifi config
        // https://github.com/espressif/arduino-esp32/issues/400
        
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //load the flash-saved configs
        esp_wifi_init(&cfg); //initiate and allocate wifi resources (does not matter if connection fails)
        delay(2000); //wait a bit
        
        if(esp_wifi_restore()!=ESP_OK){
            Serial.println("WiFi is not initialized by esp_wifi_init ");
        } else {
            Serial.println("WiFi Configurations Cleared!");
        }     
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
                    strcpy(smarthome_server, config_json["smarthome_server"]);
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

    WiFiManagerParameter custom_smarthome_server("server", "smarthome server", smarthome_server, 40);

    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.addParameter(&custom_smarthome_server);

    if (!wifiManager.autoConnect("AutoConnectAP", "password")) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
        delay(5000);
    }
  
    strcpy(smarthome_server, custom_smarthome_server.getValue());

    // save new config

    if (shouldSaveConfig) {
        
        DynamicJsonDocument json_data(128);
        json_data["smarthome_server"] = smarthome_server;
    
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
            String ieeeAddr_generated = "5x" + String(randNumber); 
            
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
    Serial.println(smarthome_server);

    get_ieeeAddr();
    Serial.println(ieeeAddr); 
}


// ####
// loop
// ####

void loop() {
  
}
