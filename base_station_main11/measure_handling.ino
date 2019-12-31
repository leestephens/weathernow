void reset_all_measures() {
  
  for(int i=0;i<measures_recorded;i++) {
    for(int j=0;j<measure_in_array;j++) {
      measure[j][i] = -99.0;
      measure_rx[j][i] = false;
    }
    measure_wdir[i] = -99.0;
    measure_wdir_rx[i] = false;
  }

}


float return_recent(struct tm *req_time, int which) {
  int r_hour = req_time->tm_hour;
  int r_minutes = req_time->tm_min;

  int check_index = (r_hour * 60) + r_minutes;

  while (check_index>=0) {
    if (measure_rx[which][check_index]) {
      return measure[which][check_index];
    }
    check_index--;
  }
  return -99.0;
}

int return_recent_wdir(struct tm *req_time) {
  int r_hour = req_time->tm_hour;
  int r_minutes = req_time->tm_min;

  int check_index = (r_hour * 60) + r_minutes;

  while (check_index>=0) {
    if (measure_wdir_rx[check_index]) {
      return measure_wdir[check_index];
    }
    check_index--;
  }
  return -99;  
}

float rain_today(struct tm *req_time) {
  int hour = req_time->tm_hour;
  int minutes = req_time->tm_min;

  int check_index = (hour * 60) + minutes;

  float result = 0;

    for(int i=0;i<=check_index;i++) {
    if (measure_rx[M_RAIN][i]) {
      result += measure[M_RAIN][i];
    }
  }
  return result;
}

String return_json_day(struct tm *req_time, int which) {
  String result;
  char scratch[12];

  result = "[";

  int r_hour = req_time->tm_hour;
  int r_minutes = req_time->tm_min;

  struct tm calculated;
  calculated.tm_year = req_time->tm_year;
  calculated.tm_mon = req_time->tm_mon;
  calculated.tm_mday = req_time->tm_mday;
  calculated.tm_sec = 0;
  calculated.tm_isdst = req_time->tm_isdst;

  int check_index = (r_hour * 60) + r_minutes;

  for(int i=0;i<=check_index;i++) {
    if (measure_rx[which][i]) {
      result += "[";
      calculated.tm_hour = i / 60;
      calculated.tm_min = i % 60;

      snprintf(scratch, 12, "%d", mktime(&calculated));      
      result += scratch;
      result += "000, ";
      snprintf(scratch, 12, "%1.1f", measure[which][i]);
      result += scratch;
      result += "],";
    }
  }
  result += "]";

  return result;
}



int return_index_minmax(struct tm *req_time, bool maximum_required, int which) {
  // if maximum_required = true, return index of max else return index of min
  int result = -1;
  float min_max_so_far;

  
  int r_hour = req_time->tm_hour;
  int r_minutes = req_time->tm_min;

  int check_index = (r_hour * 60) + r_minutes;
  
  if (maximum_required) {
    min_max_so_far = -99.0; // we're looking for a high number
  } else {
    min_max_so_far = 1100.0; // we're looking for a low number
  }
  bool found_one = false;

  for(int i=0;i<=check_index;i++) {
    if (measure_rx[which][i]) {
      found_one = true;
      if (maximum_required) {
        if (measure[which][i]>min_max_so_far) { // we've found a bigger value
          min_max_so_far = measure[which][i];
          result = i;
        }
      } else {
        if (measure[which][i]<min_max_so_far) {
          min_max_so_far = measure[which][i];
          result = i;
        }
      }
    }
  }

  if (!found_one) {
    return -1;
  }
  return result;
}

// ----------------------- Create windchart table

int populate_windchart(struct tm *req_time) {

  int windchart_entries = 0;

  int r_hour = req_time->tm_hour;
  int r_minutes = req_time->tm_min;

  int check_index = (r_hour * 60) + r_minutes;

  // initialise windchart
  for(int i=0;i<WINDCHART_CARDINALS;i++) {
    for(int j=0;j<WINDCHART_SPEEDS;j++) {
      windchart[i][j] = 0;
    }
  }

  // loop over entries
  for(int k=0;k<=check_index;k++) {
    if (measure_rx[M_WIND][k] && measure_wdir_rx[k]) {
      windchart_entries++;
      int x = floor((measure_wdir[k]+11.25)/22.5);
      if (x>=WINDCHART_CARDINALS) x=0;
      
      int y = mph_to_beaufort(measure[M_WIND][k]);
      
      windchart[x][y]++;

      Serial.print("Pop-WC: found an entry at ");
      Serial.print(x);
      Serial.print(", ");
      Serial.println(y);
    }
  }
  

  return windchart_entries;
}

int mph_to_beaufort(float mph) {
  // NB Technically beaufort -1 - as we're using them for array access!!
  if (mph<1) return 0;
  if (mph<3) return 1;
   if (mph<7) return 2;
   if (mph<12) return 3;
   if (mph<18) return 4;
   if (mph<24) return 5;
   if (mph<31) return 6;
   if (mph<38) return 7;
   if (mph<46) return 8;
   return 9; // Again, technically scale goes 11 Violent storm 64-72mph and 12 Hurricane >=73mph

  
}
