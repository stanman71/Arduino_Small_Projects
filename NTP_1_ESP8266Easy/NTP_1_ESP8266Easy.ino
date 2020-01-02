/*******************************************************************************/
/* ESP8266EasyNTP       Using simple ESP8266 function configTime() of the
                        library time.h to get the UTC time via NTP.
                        Enhanced with code to determine european daylight saving
                        time.

  Arduino test environment:
  Arduino IDE:          1.8.5
  Hardware:             WeMos D1 mini
  Libraries:            ESP8266WiFi.h, time.h (both included in
                        ESP8266 platform package)
  Reference:            https://wiki.wemos.cc/products:d1:d1_mini
                        https://gitter.im/esp8266/Arduino/archives/2016/12/28
                        https://forum.arduino.cc/index.php?topic=553026.0
                        http://forum.arduino.cc/index.php?topic=172044.msg1278536#msg1278536

  Author:               Olaf Meier
  Date:                 2018/06/17
/*******************************************************************************/
/***  Include libraries, declare constants and variables for WiFi and NTP   ***/
#include <ESP8266WiFi.h>
#include <time.h>
const char* ssid                  =  "";           // Your network SSID (name)
const char* password              =  "";           // Your network password
const unsigned int BAUDRATE       =  115200;       // Baud rate for serial monitor
int dst                           =  1;            // If summer time, is set to 1
int year                          =  0;            // Must enter 4-digits!
byte month                        =  0;            // Jan. = 1 to Dec. = 12
byte mday                         =  0;            // Day of month 1 to 31
byte wday                         =  0;            // Monday = 1 to Sunday = 7
int yday                          =  0;            // Days since Jan. 1st 1 to 366
byte hour                         =  0;            // 0 to 23
byte tzHours                      =  1;            // GMT + 1 hour for Berlin
byte minute                       =  0;
byte second                       =  0;
/***  Some helper for cyclic intervals  ***/
byte oldSecond                    =  0;            // 1s interval
/*******************************************************************************/
/*******************************************************************************/
void setup() {
  Serial.begin(BAUDRATE);                          // ...set up the serial output
}                                                  // End of setup()
/*******************************************************************************/
/*******************************************************************************/
void loop() {
  SetupNTP();                                      // Init WiFI and NTP
  GetTime();                                       // Get updated MEZ/MESZ (NTP)
  PrintTime();                                     // Display time, toggle LED
}                                                  // End of void loop()
/*******************************************************************************/
/*******************************************************************************/
/***  Set up WiFi / NTP and get UTC  ***/
void SetupNTP() {
  /***  Set up of WiFi, LED and NTP time  ***/
  static bool setupDone           =  false;        // Just run this setup once
  if (setupDone == false) {
    /***  Set up WiFI  ***/
    Serial.setDebugOutput(false);                  // true -> debug on
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(50);                                   // Connecting needs time
    }
    Serial.print("\nWiFi connected to ");
    Serial.println(ssid);
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    /*** Set up UTC ctime via NTP  ***/
    // https://forum.arduino.cc/index.php?topic=553026.0
    // configTime(timezone * 3600, dst * 3600, "pool.ntp.org", "time.nist.gov");
    configTime(tzHours * 3600, dst * 0, "0.europe.pool.ntp.org", "pool.ntp.org", "time.nist.gov");// fritz.box
    Serial.println("\nWaiting for time stamp...");
    while (!time(nullptr)) {
      Serial.print(".");
      delay(1000);
    }
    pinMode(LED_BUILTIN, OUTPUT);                  // Heartbeat internal LED
    setupDone = true;
    Serial.println("\nUTC set up done\n");
    /***  Get NTP-Time  ***/
    /***  In case of a slightly wrong time stamp, increase delay() below  ***/
    delay(250);
    time_t now = time(nullptr);                    // Update time to now
    Serial.print("ctime: ");
    Serial.println(ctime(&now));
  }                                                // Just do all above once only
}                                                  // End of function SetupNTP()                                                 // End of function SetupNTP()
/*******************************************************************************/
/*******************************************************************************/
void GetTime() {
  /***  Update time to the predefined variables  ***/
  time_t now = time(nullptr);                      // Update time to now
  /*****************************************************************************/
  /***  Create a structure for gmtime  ***/
  /***  Structure (http://www.cplusplus.com/reference/ctime/gmtime/)  ***/
  /***  https://gitter.im/esp8266/Arduino/archives/2016/12/28  ***/
  struct tm * timeinfo;
  time(&now);
  timeinfo = gmtime(&now);                         // Also as localtime
  second = timeinfo->tm_sec;                       // Seconds 0 to 61 (2 leap sec)
  minute = timeinfo->tm_min;                       // Minutes 0 to 59
  hour = timeinfo->tm_hour;                        // Hours 0 to 23
  mday = timeinfo->tm_mday;                        // Day of month 1 to 31
  wday = timeinfo->tm_wday;                        // Days since Sunday 0 to 6
  if (wday == 0) wday = 7;                         // 1 = Monday, 2 = Tuesday, ....
  month = timeinfo->tm_mon;                        // Months since Jan. 0 to 11
  month += 1;                                      // Month now 1 to 12
  year = timeinfo->tm_year + 1900;                 // Year with 4 digits
  yday = timeinfo->tm_yday;                        // Days since 1. of Jan. 0 to 365
  yday += 1;                                       // 1st Jan. = 1; 2nd Jan. = 2; ...
  /*****************************************************************************/
  /***  Check DST once per hour only  ***/
  static byte oldMinute           =  30;           // Must be > 0
  static bool checkDST            =  false;        // Just run this setup once...
  if (checkDST == false || minute == 0) {          // ...or every hour
    if (oldMinute != minute) {                     // Do only one time per cycle
      /*************************************************************************/
      /***  Correct DST and get updated time stamp via NTP  ***/
      dst = summertime_EU(year, month, mday, hour, tzHours);
      if (dst == 1) {
        configTime(tzHours * 3600, dst * 3600, "0.europe.pool.ntp.org", "pool.ntp.org", "time.nist.gov");// fritz.box
        Serial.println("Daylight Saving Time\n");
      }
      else {
        configTime(tzHours * 3600, dst * 3600, "0.europe.pool.ntp.org", "pool.ntp.org", "time.nist.gov");// fritz.box
        Serial.println("Standard Winter Time\n");
      }
      oldMinute = minute;
    }
    checkDST = true;
  }                                                // End of checkDST
}                                                  // End of function
/*******************************************************************************/
/*******************************************************************************/
/***  Check whether there is daylight saving time aka summer time  ***/
/***  Daylight saving time (function from Jurs  ***/
// http://forum.arduino.cc/index.php?topic=172044.msg1278536#msg1278536
/*************************************************************************/
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
boolean summertime_EU(int _year, byte _month, byte _day, byte _hour, byte _tzHours)
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
{
  if (_month < 3 || _month > 10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
  if (_month > 3 && _month < 10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
  if (_month == 3 && (_hour + 24 * _day) >= (1 + _tzHours + 24 * (31 - (5 * _year / 4 + 4) % 7)) || _month == 10 && (_hour + 24 * _day) < (1 + _tzHours + 24 * (31 - (5 * _year / 4 + 1) % 7)))
    return true;
  else
    return false;
}                                                  // End of function summertime_EU
/*******************************************************************************/
/*******************************************************************************/
/***  Show the updated time each second via serial monitor and toggle LED  ***/
void PrintTime() {                                 // Toggle LED and show time
  if (oldSecond != second) {
    oldSecond = second;
    /***  Show date and time via serial monitor  ***/
    if (hour < 10) Serial.print("0");              // Show a leading 0
    Serial.print(hour);
    Serial.print(":");
    if (minute < 10) Serial.print("0");            // Show a leading 0
    Serial.print(minute);
    Serial.print(":");
    if (second < 10) Serial.print("0");            // Show a leading 0
    Serial.println(second);
    Serial.print(wday);
    Serial.print(". ");
    if (mday < 10) Serial.print("0");              // Show a leading 0
    Serial.print(mday);
    Serial.print(".");
    if (month < 10) Serial.print("0");             // Show a leading 0
    Serial.print(month);
    Serial.print(".");
    Serial.println(year);
    Serial.print("Days of the year: ");
    Serial.println(yday);
    Serial.println("");
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
}                                                  // End of function SetupNTP()
/*******************************************************************************/
/*******************************************************************************/
