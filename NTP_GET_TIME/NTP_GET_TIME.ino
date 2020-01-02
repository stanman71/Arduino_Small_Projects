

#include <ESP8266WiFi.h>
#include <time.h>

const char* ssid     = "";
const char* password = "";
int timezone         = 2;
int dst              = 1;        // 1 = summertime > true

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  configTime(timezone * 3600, dst * 0, "0.europe.pool.ntp.org", "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
}

void loop() {
  time_t now;
  struct tm * timeinfo;
  time(&now);
  timeinfo = localtime(&now);  
  int hour   = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;  
  Serial.print(hour);  
  Serial.print(":");    
  Serial.println(minute);
  delay(1000);
}
