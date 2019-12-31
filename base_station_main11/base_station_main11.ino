// --------------- INCLUDES

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Nextion.h>
#include "time.h"

// --------------- DEFINES

#define M_TEMP 0
#define M_PRESS 1
#define M_HUM 2
#define M_WIND 3
#define M_GUST 4
#define M_RAIN 5
#define M_WDIR 6
#define M_TIME 7
#define M_MEASURE 8

#define WINDCHART_CARDINALS 16
#define WINDCHART_SPEEDS 11

#define SCREEN_DESIGNS 2
#define DESIGN_STD 0
#define DESIGN_DARK 1

#define WINDDIR_PIC_OFFSET 11
#define BLANK_PICTURE 8

#define PIC_WIFI_ON 29
#define PIC_WIFI_OFF 28
#define PIC_ERROR 27

#define STALE_DATA_TIME 600000000

#define SUMMARY_RAW 0
#define SUMMARY_HOUR 1
#define SUMMARY_DAY 2
#define SUMMARY_WEEK 3
#define SUMMARY_MONTH 4

// --------------- CONSTANTS

//const char* ssid       = "BTHub5-7T8R";
//const char* password   = "bc293b7659";

const char* ssid   = "SKYFRPEM";
const char* password = "Jch2HAMem25M";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

const int measure_in_array = 6;
const int measures_recorded = 1440;

const unsigned long time_check = 14400000;

const char* measure_names[] = {"Temperature", "Pressure", "Humidity",
                             "Wind Speed", "Max 3s Gust", "Rainfall",
                             "Wind Direction", "Measured at", "Measurements"};

const char* measure_units[] = {"&deg;C", "hPa", "&percnt; RH",
                             "mph", "mph", "mm",
                             ".", ".", "."};

const char* measure_codes[] = {"temp", "press", "hum", "wind", "gust", "rain", "wdir"};

const char* DirTable[] = {"N","NNE","NE","ENE","E","ESE", "SE","SSE","S","SSW","SW","WSW", "W","WNW","NW","NNW","N"}; 

const char* wind_speeds[] = {"Calm", "Light air", "Light breeze", "Gentle breeze", "Moderate breeze",
                             "Fresh breeze", "Strong breeze", "Near gale", "Gale", "Severe gale", "Storm" };

const char* google_host = "script.google.com";

const int httpsPort = 443;

// --------------- VARIABLES

WebServer server(80);

/*
 * 
float current_temp;
float current_press ;
float current_hum;
float current_windspd;
int current_winddir;
float current_maxgust;
float current_raintod;

*/

unsigned long total_measure;

float measure[measure_in_array][measures_recorded];
bool measure_rx[measure_in_array][measures_recorded];
int measure_wdir[measures_recorded];
bool measure_wdir_rx[measures_recorded];

struct tm time_measure;


NexPage page_startup = NexPage(0, 0, "Startup");
NexPage page_main = NexPage(1, 0, "Main_Std");
NexPage page_main_d = NexPage(2, 0, "Main_Dark");
NexPage page_settings = NexPage(3, 0, "Settings_Std");
NexPage page_settings_d = NexPage(4, 0, "Settings_Dark");

// Page 0 Startup
NexPicture pic_progress = NexPicture(0, 1, "std_wifi");
NexPicture pic_tasks = NexPicture(0, 2, "std_wifi");
NexPicture pic_results = NexPicture(0, 3, "std_wifi");


// Page 1 Main Standard
NexHotspot hs_tosettings = NexHotspot(1, 1, "std_stg");
NexText txt_time = NexText(1, 2, "std_time");
NexPicture pic_error = NexPicture(1, 3, "std_erricon");
NexText txt_maxgust = NexText(1, 4, "std_gust");
NexPicture pic_wdir = NexPicture(1, 5, "std_wdir");
NexText txt_windspd = NexText(1, 6, "std_wind");
NexText txt_raintod = NexText(1, 7, "std_rain");
NexText txt_press = NexText(1, 8, "std_press");
NexText txt_hum = NexText(1, 9, "std_hum");
NexText txt_temp = NexText(1, 10, "std_temp");
NexPicture pic_wifistatus = NexPicture(1, 11, "std_wifi");

// Page 3 Settings
NexHotspot hs_fromsettings = NexHotspot(3, 5, "stg_back");
NexText txt_ssid = NexText(3, 1, "stg_ssid");
NexPicture pic_darkmode = NexPicture(3, 3, "stg_dark");
NexHotspot hs_updatetime = NexHotspot(3, 5, "stg_time");

// Other variables
volatile bool screenUpdateRequired = true;

// Timer Variables
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile bool timer_action_required;

// Windchart
int windchart[WINDCHART_CARDINALS][WINDCHART_SPEEDS];

// Error status
volatile bool recent_measurement_received = false;
int recent_measurement_hour = -99;
int recent_measurement_min = -99;
hw_timer_t * timer_recent_measure = NULL;
//portMUX_TYPE timerMux_recent = portMUX_INITALIZER_UNLOCKED;
String error_messages;
bool error_messages_available = false;
// Temporary zone
int current_winddir_test;

NexTouch *nex_listen_list[] =
{
  &hs_tosettings,
  &hs_fromsettings,
  &hs_updatetime,
  NULL
};


// ------------------ ISR TIMER HANDLER

void IRAM_ATTR isr_timer_handler() {
  portENTER_CRITICAL_ISR(&timerMux);
  timer_action_required = true;
  portEXIT_CRITICAL_ISR(&timerMux);
  return;
}

void IRAM_ATTR isr_timer_recent_handler() {
  portENTER_CRITICAL_ISR(&timerMux);
  recent_measurement_received = false;
  screenUpdateRequired = true;
  portEXIT_CRITICAL_ISR(&timerMux);
  return;
}


// --------------- SETUP

void setup() {
  // Set base variables
  Serial.begin(115200);

  Serial.print("Setup start");

  reset_all_measures();
  
  total_measure = 0;

  current_winddir_test = 0;

  error_messages = "";

  Serial.print("Setup start");

  // Setup screen
  nexInit(115200); // default 115200
  hs_tosettings.attachPush(hs_tosettingsPushCallback, &hs_tosettings);
  hs_fromsettings.attachPush(hs_fromsettingsPushCallback, &hs_fromsettings);
  hs_updatetime.attachPush(hs_updatetimePushCallback, &hs_updatetime);


  // Connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  pic_wifistatus.setPic(PIC_WIFI_ON);
  


  // Setup webserver
  if (MDNS.begin("weathernow")) {
    Serial.println("MDNS responder started");
  } else {
    Serial.println("MDNS responder not started");
    error_messages += "Startup: MDNS responder failed. ";
    error_messages_available = true;
  }

  // Root page
  server.on("/", handleRoot);
  server.on("/style.css", handleStyleCSS);

  // Update from the sensor
  server.on("/update", handleUpdate);

  // Display chart of in memory data
  server.on("/chart", handle_chart);
  server.on("/style_chart.css", handleStyleCSSChart);
  
  // Settings pages
  server.on("/cloud_upload", handle_cloud_upload);
  server.on("/timer", handle_timer);
  server.on("/screen_mode", handle_screen_mode);
  server.on("/time", handle_time);
  server.on("/reset_error", handle_reset_error);
  server.on("/restart", handle_restart);

  // Google data display
  server.on("/cloud_chart", handle_cloud_chart);
  
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");


  // Set time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  bool good_time = false;
  while (!good_time) {
    if(!getLocalTime(&timeinfo)){
      Serial.println("Setup: failed to obtain time");
      delay(1000);
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    } else {
      Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
      good_time = true;
    }
  }

  // Configure Timer
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &isr_timer_handler, true);
  timerAlarmWrite(timer, 60000000, true);
  timerAlarmEnable(timer);

  timer_recent_measure = timerBegin(1, 80, true);
  timerAttachInterrupt(timer_recent_measure, &isr_timer_recent_handler, true);
  timerAlarmWrite(timer_recent_measure, STALE_DATA_TIME, true);
  timerAlarmEnable(timer_recent_measure);

  page_main.show();
}


// --------------- MAIN LOOP

void loop() {
  // Check for connections
  server.handleClient();

  // Check for user input
  nexLoop(nex_listen_list);

  // Update screen
  if (screenUpdateRequired) {
    updateHomeScreen();
    screenUpdateRequired = 0;
  }

  // Check time on periodic basis
  if (timer_action_required) {
    // another minute has passed

    // Check we are still on wifi
    if (WiFi.status() != WL_CONNECTED) {
      pic_wifistatus.setPic(PIC_WIFI_OFF);
      Serial.println("We lost WiFi - trying to reconnect...");
      WiFi.begin(ssid, password);
      screenUpdateRequired = 1;    // Overly simplistic?
    } else {
      pic_wifistatus.setPic(PIC_WIFI_ON);
    }
    
    
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
    } else {

      // if 2:00, 6:00, 10:00, 14:00, 18:00 or 22:00
      if (((timeinfo.tm_hour+2)%4==0)&& (timeinfo.tm_min == 0)) {
        Serial.println("Updating time from NTP");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      }


      // if hour is 00, 10, 20, 30, 40, 50 then upload - happens before midnight reset
      if ((timeinfo.tm_min%10)==0) {
        String submission = "";
        // get average, min and max for previous 10 minutes for each measure
        for(int i=0;i<7;i++) {
          String submission_element = submit_avg(&timeinfo, i);
          if (submission_element.length()>0) {
            submission += submission_element;
            submission += "&";
          }
        }
        int l = submission.length();
        if (l>2) {
          submission = submission.substring(0, l-1);
          Serial.print("Item to submit to google: ");
          Serial.println(submission);
          if (!submit_results(submission)) {
            error_messages += "Google upload failed: ";
            char scratch[10];
            snprintf(scratch, 10, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min % 60);
            error_messages += scratch;
            error_messages += "\r\n";
            error_messages_available = true;

          }
        }
      }

      // if midnight, reboot
      if ((timeinfo.tm_hour == 0) && (timeinfo.tm_min == 2)) {
        Serial.println("Resetting all measures");
        reset_all_measures();
        ESP.restart();
      }
    }

    // reset flag
    timer_action_required = false;
  }

}
