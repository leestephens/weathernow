String submit_avg(struct tm *req_time, int which) {
  char scratch[10];

  int r_hour = req_time->tm_hour;
  int r_minutes = req_time->tm_min;

  int check_index = (r_hour * 60) + r_minutes;

  if (check_index==0) {
    check_index = (23 * 60) + 59;
  }
  String ret;
  
  if (which==M_WDIR) {
    int recent_wdir[10];
    // gather all 10? values in recent_wdir
    for(int i=0;i<10;i++) {
      if (measure_wdir_rx[check_index]) {
        recent_wdir[i] = measure_wdir[check_index];
      } else {
        recent_wdir[i] = -99;
      }
      check_index--;
      if (check_index<0) break;
    }

    // average them all
    int average = average_wdir(recent_wdir);
    if (average>=0) {
      ret += "wdir=";
      ret += average;
      return ret;
    } else {
      return "";
    }
  } else {
    float recent_vals[10];
    float recent_avg = 0;
    int values_found = 0;
    // gather all 10? values in recent_vals, find min and max along the way
    for(int k=0;k<10;k++) {
      if (measure_rx[which][check_index]) {
        recent_vals[k] = measure[which][check_index];
        values_found++;
        recent_avg += recent_vals[k];
      } else {
        recent_vals[k] = -99.0;
      }
      check_index--;
      if (check_index<0) break;
    }
    // average
    if (values_found>0) {
      recent_avg /= values_found;

      ret += measure_codes[which];
      ret += "=";
      snprintf(scratch, 10, "%2.1f", recent_avg);
      ret += scratch;


      return ret;
    }
  }
  return "";
}

int submit_results(String json_details) {

  // Bonsall Version
  //String baseurl = "https://script.google.com/macros/s/AKfycbyczQpp3gQCwDvlJ1n6zzjtjoMaxmFw1-UBx4Y4ObzJUEstcPU/exec?";
  // Lee on Solent Version
  String baseurl = "https://script.google.com/macros/s/AKfycbwEhmJrWdu9UT3ByR3xqrieDd78q_tcxwH9cLw5jj2L9kdg5ZE/exec?";

  WiFiClientSecure secure_client;

  // Screen upload indicator on
  

  if (!secure_client.connect(google_host, httpsPort)) {
    Serial.println("connection to google failed");
    error_messages += "Google upload: submit_results: couldn't connect.\r\n";
    error_messages_available = true;
    return 0;
  }

  //
  // LOCATION SPECIFIC DETAILS HERE
  //
  String url = baseurl + json_details;
  
  secure_client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + google_host + "\r\n" +
         "User-Agent: LMJS_ESP32_WeatherNow\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (secure_client.connected()) {
    String line = secure_client.readStringUntil('\n');
    if (line == "\r") {
     Serial.println("headers received");
      break;
    }
  }
  String line = secure_client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");

  secure_client.stop();

  // Screen upload indicator off

  
  return 1;
}

int average_wdir(int list[]) {
  int sum = -99;
  int D;
  int delta;
  int found=0;

  for (int i=1; i<10; i++) {
    if (list[i]>=0) {
      if (sum==-99) {
        sum = list[i];
        D = sum;
      } else {
        delta = list[i] - D;
        if (delta < -180) D = D + delta + 360;
        else if (delta < 180) D = D + delta;
        else D = D + delta + 360;
        sum = sum + D;
      }
      found++;
    }
  }

  if (found == 0) {
    return -99;
  }

  return (sum/found);
}
