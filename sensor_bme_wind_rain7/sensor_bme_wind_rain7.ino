//
// Standard target so far has been:
// Board: Wemos Lolin32
// Upload: 115200
// Port: /dev/cu.usbserial-1430

// --------------- INCLUDES

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>  
#include <esp_wifi.h>             
#include <BME280I2C.h>
#include <Wire.h>


// --------------- DEFINES

#define WIND_ELEMENT_LOOP_LENGTH 1000
#define WIND_DIRECTION_MAX_ELEMENTS 630

#define BUCKET_SIZE 0.2794
#define WIND_SPEED_REVOLUTION 1.492

#define HTTP_PORT 80

#define PIN_WIND_SPEED 25
#define PIN_RAIN 32
#define PIN_WIND_DIR 33
#define PIN_SDA 16
#define PIN_SCL 17
#define PIN_LED 22

#define ERROR_CODE_BME_SENSOR 2
#define ERROR_CODE_WIFI 3
#define ERROR_CODE_NOSERVER 4
#define ERROR_CODE_NORESPONSE 5

// --------------- CONSTANTS

//const char* ssid       = "BTHub5-7T8R";
//const char* password   = "bc293b7659";

const char* ssid   = "SKYFRPEM";
const char* password = "Jch2HAMem25M";

const IPAddress allzeros(0,0,0,0);

// Lee On Solent static guess
const IPAddress static_guess(192,168,0,7); // being the assigned address of the indoor host - fallback in case mDNS fails

// Bonsall static guess
//const IPAddress static_guess(192,168,1,126); // being the assigned address of the indoor host - fallback in case mDNS fails

//const char* ssid       = "SKY93B9A";
//const char* password   = "BFTPWFCC";

const char* host = "weathernow";

// Bonsall altitude
//const float weathernow_station_altitude = 225.0; // in metres

// Lee On Solent altitude
const float weathernow_station_altitude = 5.0; // in metres

// --------------- VARIABLES

unsigned long previous_wind_calc;
unsigned long time_between_submissions;

unsigned long previous_submission_event;

volatile unsigned int tip_count, wind_clicks;

volatile unsigned long last_rain_event, last_wind_event;

unsigned int wind_index;

unsigned int recorded_wind_direction[WIND_DIRECTION_MAX_ELEMENTS];

float current_press, current_temp, current_hum;

float running_average_windspd, total_windspd_for_average;

float current_maxgust, gust_last, gust_before_last;

//BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,

BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_Off,
   BME280::SpiEnable_False,
   0x76 // I2C address. I2C specific.
);

BME280I2C bme(settings);

WiFiClient client;


/*
 * Setup
 */
 
void setup() {
  // Initialise variables
  previous_wind_calc = 0;
  time_between_submissions = 5000;
  previous_submission_event = 0;
  tip_count = 0;
  wind_clicks = 0;
  last_rain_event = 0;
  last_wind_event = 0;

  current_press = -99.0;
  current_temp = -99.0;
  current_hum = -99.0;

  wind_index = 0;

  running_average_windspd = 0;
  total_windspd_for_average = 0;

  current_maxgust = 0;
  gust_last = 0;
  gust_before_last = 0;

  // Setup functions
  Serial.begin(115200);

  // Set interrupt pins and routines
  pinMode(PIN_WIND_SPEED, INPUT_PULLUP); // input from wind meters windspeed sensor
  pinMode(PIN_RAIN, INPUT_PULLUP); // input from wind meters rain gauge sensor
  attachInterrupt(PIN_RAIN, isr_rain_sensor, FALLING);
  attachInterrupt(PIN_WIND_SPEED, isr_wind_speed, FALLING);

  // Reset LED
  pinMode(22, OUTPUT);
  digitalWrite(22, HIGH);   // turn the LED off CHECK IF MOVE TO OTHER BOARD


  // Check condition for WiFi SSID/PW reset
  // if touch event {
  // wait a while and recheck touch and if that
  // reset_wifi_credentials();
  
  // Connect WiFi
  connect_wifi();

  // Setup BME sensor
  Wire.begin(PIN_SDA, PIN_SCL);
  while(!bme.begin()) {
    display_error(ERROR_CODE_BME_SENSOR);
  }
  bme.setSettings(settings);

  // Set interrupts
  interrupts();
}

/*
 * loop
 */

void loop() {
  unsigned long this_loop = millis();

  // If a second has passed, calculate wind elements
  if ((this_loop - previous_wind_calc)>WIND_ELEMENT_LOOP_LENGTH) {

    if ((this_loop - previous_wind_calc)>(WIND_ELEMENT_LOOP_LENGTH*1.25)) {
      Serial.println("More than 25% over WIND_ELEMENT_LOOP.");
      // Reset and try again
      wind_clicks = 0;
    } else {
      update_wind_record();
      wind_index++;
      if (wind_index>=WIND_DIRECTION_MAX_ELEMENTS) {
        Serial.println("Problem, we've overshot the wind elements array, let's fail gracefully");
        wind_index = 0;
      }
    }
    previous_wind_calc = this_loop;
  }

  if ((this_loop - previous_submission_event)>time_between_submissions) {
    time_between_submissions = submit_readings();
    previous_submission_event = this_loop;
  }

}

/*
 * Connect WiFi
 */

bool connect_wifi() {


  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, password);

  for(int i=0;i<10;i++) {
    if (WiFi.status() != WL_CONNECTED) {
      delay(500);
    } else {
        i = 10;
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    display_error(ERROR_CODE_WIFI);
    return false;
  }
  MDNS.begin("sensor_bme_wind_rain");
  return true;
}

/*
 * Update wind records each second
 */

void update_wind_record() {
    // Calculate wind speed over this second
    float current_windspd = (float)wind_clicks * 1.492; // Was / WIND_MEASUREMENT_ELAPSED_TIME;
    wind_clicks = 0; //Reset and start watching for new wind

    // Update running average of wind
    total_windspd_for_average += current_windspd;
    running_average_windspd = total_windspd_for_average/(wind_index+1); 

    // Check wind direction
    recorded_wind_direction[wind_index] = get_wind_direction();

    // Check if this is a gust
    float last_3s_gust_total = gust_before_last + gust_last + current_windspd;
 
    if (last_3s_gust_total > 0) {
      current_maxgust = max(current_maxgust, last_3s_gust_total / 3);
    }

    gust_before_last = gust_last;
    gust_last = current_windspd;
    
}

/*
 * Submit readings
 */

int submit_readings() {
  bool success = false;
  int next_submission_response_rx = 0;
  
  // Check WiFi
  if (WiFi.status() != WL_CONNECTED) {
    if (!connect_wifi()) {
      display_error(ERROR_CODE_WIFI);
      return 0;
    }
  }

  // Get BME Readings
  read_bme_sensor(); // Updates current_temp, current_hum, current_press

  // Calculate rainfall
  float total_rainfall = tip_count * BUCKET_SIZE;

  // Construct URL
  String url = "/update?";
  char scratch[10];
  
  snprintf(scratch, 10, "%2.2f", current_temp);
  url += "temp=";
  url += scratch;

  snprintf(scratch, 10, "%2.2f", current_hum);
  url += "&hum=";
  url += scratch;

  // Source for calculation is:
  // https://keisan.casio.com/exec/system/1224575267

  float adjusted_sea_level_pressure = pow(1-((0.0065*weathernow_station_altitude)/
                                              (current_temp+(0.0065*weathernow_station_altitude)+273.15)),
                                              -5.257)*current_press;

   Serial.print("Current pressure: ");
   Serial.println(current_press);
   Serial.print("Sea level pressure: ");
   Serial.print(adjusted_sea_level_pressure);                                           

  if (current_press >0) {
    snprintf(scratch, 10, "%3.2f", adjusted_sea_level_pressure/100);
    url += "&press=";
    url += scratch;
  }
  
  snprintf(scratch, 10, "%2.2f", running_average_windspd);
  url += "&wind=";
  url += scratch;
  total_windspd_for_average = 0;

  snprintf(scratch, 10, "%d", average_wind_direction());
  url += "&wdir=";
  url += scratch;
  wind_index = 0;


  snprintf(scratch, 10, "%2.2f", current_maxgust);
  url += "&gust=";
  url += scratch;
  current_maxgust = 0;

  snprintf(scratch, 10, "%1.4f", total_rainfall);
  url += "&rain=";
  url += scratch;
  

  // Connect and read response
  IPAddress weathernow = MDNS.queryHost(host, 15000);

  Serial.println(weathernow);
  if (weathernow==allzeros) {
    weathernow = static_guess;
  }
  Serial.println(weathernow);
  if (!client.connect(weathernow, HTTP_PORT)) {
    display_error(ERROR_CODE_NOSERVER);
    Serial.println("Client connection failed");
  } 
  Serial.print("Submitting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 15000) {
      Serial.println("Client timeout receiving response");
      client.stop();
      display_error(ERROR_CODE_NORESPONSE);
      return 0;
    }
  }
  while(client.available()) {
    String http_response = client.readStringUntil('$');
    String next_submission = client.readString();
    success = true;
    next_submission_response_rx = next_submission.toInt(); 
  }

  // Close connection
  client.stop();

  // Return response
  if (success) {
    tip_count = 0;
    return next_submission_response_rx;
  } else {
    return 0;
  }
}

/*
 * Read BME Sensor
 */

void read_bme_sensor() {

   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_Pa);

   bme.read(current_press, current_temp, current_hum, tempUnit, presUnit);

}

// --------------- ERROR ROUTINE

void display_error(int code) {
  for (int i=0;i<3;i++) {
    for (int j=0;j<code;j++) {
      digitalWrite(PIN_LED, LOW);   // turn the LED on CHECK PIN NUMBER AND LOGIC LEVEL IF MOVE TO OTHER BOARD
      delay(200);
      digitalWrite(PIN_LED, HIGH);   // turn the LED off 
      delay(200);
    }
    delay(1000);
  }
}


// Calculate average wind direction

/*
http://abelian.org/vlf/bearings.html
let the set of N raw bearings be b[1..N].
   sum = D = b[1];
   for( i from 2 to N)
   {
      delta = b[i] - D;
      if( delta < -180) D = D + delta + 360;
      else
      if( delta < 180) D = D + delta;
      else
         D = D + delta - 360;
      sum = sum + D;
   } 
   mean = sum/N;
   */

int average_wind_direction() {

  int sum = recorded_wind_direction[0];
  int D = sum;
  int delta;

  for (int i=1; i<wind_index; i++) {
    delta = recorded_wind_direction[i] - D;
    if (delta < -180) D = D + delta + 360;
    else if (delta < 180) D = D + delta;
    else D = D + delta + 360;
    sum = sum + D;
  }

  if (wind_index == 0) return 0;

  return sum/wind_index;
  
}

/*
 * Get Wind Direction
 */

int get_wind_direction() {
  unsigned int adc;

  adc = analogRead(PIN_WIND_DIR); // get the current reading from the sensor

  // The following table is ADC readings for the wind direction sensor output, sorted from low to high.
  // Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
  // Note that these are not in compass degree order! See Weather Meters datasheet for more information.

  if (adc <133) return (113);
  if (adc <188) return (68);
  if (adc <271) return (90);
  if (adc <450) return (158);
  if (adc <682) return (135);
  if (adc <884) return (203);
  if (adc <1204) return (180);
  if (adc <1551) return (23);
  if (adc <1932) return (45);
  if (adc <2262) return (248);
  if (adc <2463) return (225);
  if (adc <2774) return (338);
  if (adc <3039) return (0);
  if (adc <3295) return (293);
  if (adc <3653) return (315);
  if (adc <3973) return (270);
  return (-1); // error, disconnected?
}

// --------------- INTERRUPT ROUTINES

void isr_wind_speed() {
  unsigned long wind_time = millis();
  if (wind_time - last_wind_event > 10) { // Ignore switch-bounce glitches less than 10ms (149.2MPH max reading) after the reed switch closes 
    wind_clicks++; //There is 1.492MPH for each click per second.
    last_wind_event = wind_time; //Grab the current time
  }
}

// Interrupt handler routine that is triggered when the rain sensor fires
void isr_rain_sensor() {
  unsigned long rain_time = millis();
  if((rain_time - last_rain_event) > 100 ) { // debounce of sensor signal 
    tip_count++; 
    last_rain_event = rain_time; 
  } 
} 
