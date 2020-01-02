#include <ESP8266WiFi.h>
#include <time.h>

const char* ssid      =  "";       // Your network SSID (name)
const char* password  =  "";   // Your network password


int dst               =  1;     // summertime <> wintertime 
int year              =  0;     
byte month            =  0;     // Jan. = 1 to Dec. = 12
byte mday             =  0;     // Day of month 1 to 31
byte hour             =  0;     
byte tzHours          =  1;     // GMT + 1 hour for Berlin
byte minute           =  0;

static bool setupDone = false;  
static bool checkDST  = false;  
static byte oldMinute = 30; 


void setup() {
   Serial.begin(115200);                          
}                                                 


void loop() {
    if (setupDone == false) {  
       SetupNTP(); 
    }
                                     
      GetTime();                                      
      PrintTime(); 
      delay(60000);                                    
}                                                  



void SetupNTP() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(50);                                  
    }
    
    Serial.print("\nWiFi connected to ");
    Serial.println(ssid);
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    configTime(tzHours * 3600, dst * 0, "0.europe.pool.ntp.org", "pool.ntp.org", "time.nist.gov");// fritz.box
    Serial.println("\nWaiting for time stamp...");
    
    while (!time(nullptr)) {
        Serial.print(".");
        delay(1000);
    }
    
    setupDone = true;
    Serial.println("\nUTC set up done\n");

    delay(250);
    time_t now = time(nullptr);                                                             
}                                                 
 

void GetTime() {
    time_t now = time(nullptr);                     
    struct tm * timeinfo;
    time(&now);
    timeinfo = gmtime(&now);                         
    minute  = timeinfo->tm_min;                       
    hour    = timeinfo->tm_hour;     
    mday    = timeinfo->tm_mday;     
    month   = timeinfo->tm_mon; 
    month   += 1; 
    year    = timeinfo->tm_year + 1900;               
  
    if (checkDST == false || minute == 0) {    // update every hour            
        if (oldMinute != minute) {             // only once during a minute             
  
            /***  Correct DST and get updated time stamp via NTP  ***/
            dst = summertime_EU(year, month, mday, hour, tzHours);
            
            if (dst == 1) {
                configTime(tzHours * 3600, dst * 3600, "0.europe.pool.ntp.org", "pool.ntp.org", "time.nist.gov");// fritz.box
                Serial.println("Summer Time\n");
            }
            else {
                configTime(tzHours * 3600, dst * 3600, "0.europe.pool.ntp.org", "pool.ntp.org", "time.nist.gov");// fritz.box
                Serial.println("Winter Time\n");
            }          
            oldMinute = minute;
        }       
        checkDST = true;
    }                                                                         
}


// get correct dst value (summertime <> wintertime)
boolean summertime_EU(int _year, byte _month, byte _day, byte _hour, byte _tzHours)

{
  if (_month < 3 || _month > 10) return false;   // keine Sommerzeit in Jan, Feb, Nov, Dez
  if (_month > 3 && _month < 10) return true;    // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (_month == 3 && (_hour + 24 * _day) >= (1 + _tzHours + 24 * (31 - (5 * _year / 4 + 4) % 7)) || _month == 10 && (_hour + 24 * _day) < (1 + _tzHours + 24 * (31 - (5 * _year / 4 + 1) % 7)))
    return true;
  else
    return false;
} 
                                            
void PrintTime() {                              
    if (hour < 10) Serial.print("0");              
    Serial.print(hour);
    Serial.print(":");
    if (minute < 10) Serial.print("0");            
    Serial.println(minute);
}                                                  
