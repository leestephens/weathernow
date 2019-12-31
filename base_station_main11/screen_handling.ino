void hs_tosettingsPushCallback(void *ptr)
{
  Serial.println("Switch to Settings");
  char text_data[10];
  struct tm timeinfo;

  txt_ssid.setText(ssid);
}

void hs_fromsettingsPushCallback(void *ptr)
{
  Serial.println("Switch back from Settings");
  screenUpdateRequired = 1;
}

void hs_updatetimePushCallback(void *ptr)
{
  Serial.println("Updating time");
  Serial.print("Current time: ");
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("failed to obtain old time");
    return;
  } else {
    Serial.print(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.print("Updated time: ");
  if(!getLocalTime(&timeinfo)){
    Serial.println("failed to obtain new time");
    return;
  } else {
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }
}

void updateHomeScreen() {
  char scratch[10];
  float f;
  int i;
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  } 


//Update picture

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi Picture Updated - Problem");
      pic_wifistatus.setPic(PIC_WIFI_OFF);
    } else {
      Serial.println("WiFi Picture Updated - Ok");
      pic_wifistatus.setPic(PIC_WIFI_ON);
    }


    

// Update Text

  f = return_recent(&timeinfo, M_TEMP);
  snprintf(scratch, 10, "%d", (int)round(f));
  txt_temp.setText((f!=-99.0) ? scratch : "--");


  f = return_recent(&timeinfo, M_HUM);
  snprintf(scratch, 10, "%d", (int)round(f));
  txt_hum.setText((f!=-99.0) ? scratch : "--");

  f = return_recent(&timeinfo, M_PRESS);
  snprintf(scratch, 10, "%d", (int)round(f));
  txt_press.setText((f!=-99.0) ? scratch : "--");

  f = return_recent(&timeinfo, M_WIND);
  snprintf(scratch, 10, "%d", (int)round(f));
  txt_windspd.setText((f>=0) ? scratch : "--");

  int recent_wdir = return_recent_wdir(&timeinfo);
  if (recent_wdir>=0) {
    int res = floor((recent_wdir+11.25)/22.5);
    if (res<0) res=0;
    if (res>15) res=0;
    Serial.print("Wind direction is:");
    Serial.print(recent_wdir);
    Serial.print(" so picture: ");
    Serial.print(res);
    Serial.print(" -> ");
    Serial.println(res+WINDDIR_PIC_OFFSET);
    pic_wdir.setPic(res+WINDDIR_PIC_OFFSET);
  } else {
    pic_wdir.setPic(BLANK_PICTURE);
  }


  f = return_recent(&timeinfo, M_GUST);
  snprintf(scratch, 10, "%2d", (int)round(f));
  txt_maxgust.setText((f>-1) ? scratch : "--");

  f = rain_today(&timeinfo);
  snprintf(scratch, 10, "%1.1f", f);
  txt_raintod.setText((f>-1) ? scratch : "--");


  snprintf(scratch, 10, "   %02d:%02d", timeinfo.tm_hour, timeinfo.tm_min % 60);
  txt_time.setText(scratch);
  if (recent_measurement_received) {
    txt_time.Set_background_color_bco(65535); // White is 65535
    txt_time.Set_font_color_pco(0); // Black is 0
    pic_error.setPic(BLANK_PICTURE);
  } else {
    txt_time.Set_background_color_bco(45060); // Red is 45060
    txt_time.Set_font_color_pco(65535); // White is 65535
    pic_error.setPic(PIC_ERROR);
  }

  Serial.println("Screen elements updated");

}
