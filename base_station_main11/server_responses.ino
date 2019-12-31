const char *root_header = "\
<html><head>\
<meta http-equiv='refresh' content='60'/>\
<link href='https://fonts.googleapis.com/css?family=Saira+Extra+Condensed:300,700&display=swap' rel=stylesheet>\
<link href='https://fonts.googleapis.com/icon?family=Material+Icons' rel=stylesheet>\
<link rel=stylesheet type=text/css href=style.css>\
<title>WeatherNow</title>\
        <script>\n\
            function show_menu() {\
                document.getElementById('s-menu-id').style.left='0px';\
            }\n\
            function hide_menu() {\
                document.getElementById('s-menu-id').style.left='-600px';\
            }\n\
            function clear_errors() {\
            }\n\
            function show_restart() {\
            }\n\
        </script>\n\
</head>\n\
    <body onload='hide_menu()'>\
        <div class='s-menu' id='s-menu-id'>\
            <ul class=s-menu-list>\
                <li class=s-menu-header><a href='#' onClick='hide_menu()'><i class='material-icons md-light md-36'>arrow_back</i> Settings</a></li>\
                <li class=s-menu-item><a href='/ssid'><i class='material-icons md-dark md-36'>wifi</i> SSID</a></li>\
                <li class=s-menu-item><a href='/cloud_upload'><i class='material-icons md-dark md-36'>cloud_upload</i> Upload</a></li>\
                <li class=s-menu-item><a href='/timer'><i class='material-icons md-dark md-36'>timer</i> Measurement timing</a></li>\
                <li class=s-menu-item><a href='/screen_mode'><i class='material-icons md-dark md-36'>settings_brightness</i> Screen mode</a></li>\
                <li class=s-menu-item><a href='/time'><i class='material-icons md-dark md-36'>access_time</i> Current time</a></li>\
                <li class=s-menu-item id=s-menu-clear-errors><a href='#' onClick='clear_errors()'><i class='material-icons md-dark md-36'>warning</i> CLEAR ERRORS</a></li>\
                <li class=s-menu-item><a href='#' onclick='show_restart()'><i class='material-icons md-dark md-36'>power_settings_new</i> RESTART</a></li>\
           </ul>\
        </div>\
        <div class=title-bar>\
            <div class=title-icon onclick='show_menu'>\
                <a href='#' onclick='show_menu()'>\
                    <i class='material-icons md-light md-48' id=s-menu-open>menu</i>\
                </a>\
            </div>\ 
<div class=title-text>\
WeatherNow\
</div>\
</div>\n\
<div class=listing>";

const char *root_footer = "</body></html>";

// --------------- ROOT
void handleRoot() {
  char scratch[10];
  String html;
  float f;
  int i;
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  } 

  html += root_header;
  
  f = return_recent(&timeinfo, M_TEMP); 
  snprintf(scratch, 10, "%2.1f", f);
  html += listingItem(M_TEMP, (f!=-99.0) ? scratch : "--");

  f = return_recent(&timeinfo, M_HUM);
  snprintf(scratch, 10, "%2.1f", f);
  html += listingItem(M_HUM, (f>-99.0) ? scratch : "--");

  f = return_recent(&timeinfo, M_PRESS);
  snprintf(scratch, 10, "%3.1f", f);
  html += listingItem(M_PRESS, (f>-99.0) ? scratch : "--");

  i = return_recent_wdir(&timeinfo);
  textForWindDir(scratch, &timeinfo);
  html += listingItem(M_WDIR, (i>-1) ? scratch : "--");

  f = return_recent(&timeinfo, M_WIND);
  snprintf(scratch, 10, "%2.1f", f);
  html += listingItem(M_WIND, (f>-99.0) ? scratch : "--");

  f = return_recent(&timeinfo, M_GUST);
  snprintf(scratch, 10, "%2.1f", f);
  html += listingItem(M_GUST, (f>-99.0) ? scratch : "--");

  f = rain_today(&timeinfo);
  snprintf(scratch, 10, "%1.1f", f);
  html += listingItem(M_RAIN, (f>-1) ? scratch : "--");

  
  snprintf(scratch, 10, "%02d:%02d", recent_measurement_hour, recent_measurement_min % 60);
  html += listingItem(M_TIME, (recent_measurement_hour>=0) ? scratch: "--", !recent_measurement_received);

  html += "</div>"; // END LISTING ITEMS

  textForMeasurements(scratch);
  html += "<div class=sub-bar>Measurements received: ";
  html += scratch;
  html += "</div>";

  if (error_messages_available) {
    Serial.print("Adding error messages: ");
    Serial.println(error_messages);
    html += "<div class=error-bar>";
    html += error_messages;
    html += "</div>";
  }

  html += root_footer;

  server.send(200, "text/html", html);
}

void textForMeasurements(char *buffer) {
  unsigned long temp_measure;
  
  if (total_measure<99999) {
    snprintf(buffer, 10, "%'d", total_measure);
  } else if (total_measure<999999) {
    temp_measure = total_measure/1000;
    snprintf(buffer, 10, "%'d k", temp_measure);
  } else if (total_measure<999999999) {
    temp_measure = total_measure/1000000;
    snprintf(buffer, 10, "%'d m", temp_measure);
  } else if (total_measure<999999999999) {
    temp_measure = total_measure/1000000000;
    snprintf(buffer, 10, "%'d b", temp_measure);
  }
}

void textForWindDir(char *buffer, struct tm *timeinfo) {
  int res;

  int recent_wdir = return_recent_wdir(timeinfo);

  if (recent_wdir>=0) {
    res = floor((recent_wdir+11.25)/22.5);
    strcpy(buffer, DirTable[res]);
  } else {
    strcpy(buffer, "--");
  }
}

String listingItem(int which, String value) {
  return listingItem(which, value, false);
}

String listingItem(int which, String value, bool error) {

  String ret;
  if (which<7) {
    ret += "<a href=/chart?data=";
    ret += which;
    ret += ">";
  }
  if (error) {
    ret += "<div class=measure-error>\n";
  } else {
    ret += "<div class=measure>\n";
  }
  ret += "<div class=measure-title>\n";
  ret += measure_names[which];
  ret += "</div>\n<div class=measure-units>";
  ret += measure_units[which];
  ret += "</div>\n";
  if (error) {
    ret += "<div class=measure-value-error>";
  } else {
    ret += "<div class=measure-value>";
  }
  ret += value;
  ret += "</div></div>\n";
  if (which<7) {
    ret += "</a>";
  }
  return ret;
}

String time_from_index(unsigned int index) {
  String result;
  int r_hours = index / 60;
  int r_minutes = index % 60;
  
  if (r_hours<10) {
    result = "0";
    result += r_hours;
  } else {
    result = r_hours;
  }
  result += ":";
  if (r_minutes<10) {
    result += "0";
    result += r_minutes;
  } else {
    result += r_minutes;
  }
  return result;
}


// --------------- Update
void handleUpdate() {
  bool values_received = 0;
  float rx_temp;
  float rx_press;
  float rx_hum;
  float rx_windspd;
  int rx_winddir;
  float rx_maxgust;
  float rx_raintod;
  unsigned long start_measure;

   struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  } 
  int rx_index = (timeinfo.tm_hour * 60) + timeinfo.tm_min ;
  Serial.print("RX Index: ");
  Serial.print(rx_index);
  Serial.print(" RX Measure: ");
  Serial.println(server.args());

  start_measure = total_measure;
  
  if (server.method() == HTTP_GET) {
    for (uint8_t i = 0; i < server.args(); i++) {

       // TEMPERATURE RECEPTION
       if (server.argName(i)=="temp") {
        rx_temp = server.arg(i).toFloat();

        
        if (rx_temp > -20.0 && rx_temp < 50.0) {
          measure[M_TEMP][rx_index] = rx_temp;
          measure_rx[M_TEMP][rx_index] = true;
          total_measure++;
        }
       }
       // PRESSURE RECEPTION
       if (server.argName(i)=="press") {
        rx_press = server.arg(i).toFloat();
        
        if (rx_press > 920.0 && rx_press < 1060.0) {
          measure[M_PRESS][rx_index] = rx_press;
          measure_rx[M_PRESS][rx_index] = true;
          total_measure++;
        }
       }
       
       // HUMIDITY RECEPTION
       if (server.argName(i)=="hum") {
        rx_hum = server.arg(i).toFloat();
        
        if (rx_hum > 0 && rx_hum < 100) {
          measure[M_HUM][rx_index] = rx_hum;
          measure_rx[M_HUM][rx_index] = true;
          total_measure++;
        }
       }
       
       // WIND SPEED RECEPTION
       if (server.argName(i)=="wind") {
        rx_windspd = server.arg(i).toFloat();
        
        if (rx_windspd >= 0 && rx_windspd < 120) {
          measure[M_WIND][rx_index] = rx_windspd;
          measure_rx[M_WIND][rx_index] = true;
          total_measure++;
        }
       }
       
       // WIND DIRECTION RECEPTION
       if (server.argName(i)=="wdir") {
        rx_winddir = server.arg(i).toInt();
        
        if (rx_winddir >= 0 && rx_winddir < 360) {
          measure_wdir[rx_index] = rx_winddir;
          measure_wdir_rx[rx_index] = true;
          total_measure++;
        }
       }
       
       // MAX 3S GUST RECEPTION
       if (server.argName(i)=="gust") {
        rx_maxgust = server.arg(i).toFloat();
        
        if (rx_maxgust >= 0 && rx_maxgust < 120) {
          measure[M_GUST][rx_index] = rx_maxgust;
          measure_rx[M_GUST][rx_index] = true;
          total_measure++;
        }
       }
              
       // RAIN TODAY RECEPTION
       if (server.argName(i)=="rain") {
        rx_raintod = server.arg(i).toFloat();

        if (rx_raintod >= 0 && rx_raintod < 10) {
          measure[M_RAIN][rx_index] = rx_raintod;
          measure_rx[M_RAIN][rx_index] = true;
          total_measure++;
        }
      }
    }


  
  }

  
  if (total_measure>start_measure) {
    server.send(200, "text/plain", "$60000");
    screenUpdateRequired = 1;
    // reset timer_recent_measure
    timerWrite(timer_recent_measure, 0);
    Serial.println("Timer recent_measure reset");

    // update recent_measurement values
    portENTER_CRITICAL_ISR(&timerMux);
    recent_measurement_received = true;
    portEXIT_CRITICAL_ISR(&timerMux);
    recent_measurement_hour = timeinfo.tm_hour;
    recent_measurement_min = timeinfo.tm_min;
  } else {
    server.send(200, "text/plain", "$0");
  }
}






// --------------- JSON


void handle_chart() {
  struct tm timeinfo;
  bool good_message = false;

  String message;
  
  int required_dataset;
  
  if (server.method() == HTTP_GET) {
    for (uint8_t i = 0; i < server.args(); i++) {
       Serial.print(server.argName(i));
       Serial.print(": ");
       Serial.println(server.arg(i));

       if (server.argName(i)=="data") {
        required_dataset = server.arg(i).toInt();
       } 
       if ((required_dataset>-1)&&(required_dataset<=measure_in_array)) {
          if(!getLocalTime(&timeinfo)){
            Serial.println("Failed to obtain time");
          } else {
            switch (required_dataset) {
              case M_WDIR:
                message = html_for_wdir_chart(&timeinfo);
                break;
              case M_WIND:
              case M_GUST:
                message = html_for_wind_speed_chart(&timeinfo);
                break;
              default:
                message = html_for_general_chart(&timeinfo, required_dataset);
            }
            good_message = true;
          }

       } 
     }
  }
  if (!good_message) {
    message = "No data";
  }
  //Serial.println("HTML is: ");
  //Serial.println(message);
  server.send(200, "text/html", message);
}

String navigation_html(int which) {

  String html = "<ul class=nav-list><li class=nav-list-item><a href=/>Home</a></li>";
  html += "<li class=nav-list-item";
  if (which==M_TEMP) {
    html += "-active";
  }
  html += "><a href=/chart?data=0>";
  html += measure_names[M_TEMP];
  html += "</a></li>";
  html += "<li class=nav-list-item";
  if (which==M_PRESS) {
    html += "-active";
  }
  html += "><a href=/chart?data=1>";
  html += measure_names[M_PRESS];
  html += "</a></li>";
  html += "<li class=nav-list-item";
  if (which==M_HUM) {
    html += "-active";
  }
  html += "><a href=/chart?data=2>";
  html += measure_names[M_HUM];
  html += "</a></li>";
  html += "<li class=nav-list-item";
  if (which==M_WIND) {
    html += "-active";
  }
  html += "><a href=/chart?data=3>";
  html += measure_names[M_WIND];
  html += "</a></li>";
  html += "<li class=nav-list-item";
  if (which==M_RAIN) {
    html += "-active";
  }
  html += "><a href=/chart?data=5>";
  html += measure_names[M_RAIN];
  html += "</a></li>";
  html += "<li class=nav-list-item";
  if (which==M_WDIR) {
    html += "-active";
  }
  html += "><a href=/chart?data=6>";
  html += measure_names[M_WDIR];
  html += "</a></li></ul>\n\n";

  return html;

}

// 

// --------------- Handle WDIR chart

const char *wdir_chart_header = "<html><head><meta http-equiv='refresh' content='60'/><title>WeatherNow Chart</title>\
<link rel=stylesheet type=text/css href=style_chart.css>\
<link href='https://fonts.googleapis.com/css?family=Saira+Extra+Condensed:300,700&display=swap' rel=stylesheet>\
<script src=https://code.highcharts.com/highcharts.js></script>\
<script src=https://code.highcharts.com/highcharts-more.js></script>\
<script src=https://code.highcharts.com/modules/exporting.js></script>\
<script src=https://code.highcharts.com/modules/export-data.js></script>\
<script>document.addEventListener('DOMContentLoaded', function () {\
var myChart = Highcharts.setOptions(\
    {\
        chart: {\
            style: {\
                fontFamily: 'Saira Extra Condensed'\
            }\
        },\
        time: {\
            useUTC: false\
        }\
    }\
);\
Highcharts.chart('container', {\
series: [";

const char *wdir_chart_jscript = "],\
chart: {\
        polar: true,\
        type: 'column'\
    },\
    title: {\
            style: {\
            fontWeight: 700,\
            fontSize: '3em'\
        },\
        text: 'Wind rose'\
    },\
    pane: {\
        size: '85%'\
    },\
    legend: {\
        align: 'right',\
        verticalAlign: 'top',\
        y: 100,\
        layout: 'vertical'\
    },\
    xAxis: {\
        tickmarkPlacement: 'on',\
        categories: [ 'N', 'NNE', 'NE', 'ENE', 'E', 'ESE', 'SE', 'SSE', 'S', 'SSW', 'SW', 'WSW', 'W', 'WNW', 'NW', 'NNW' ]\
    },\n\
    yAxis: {\
        min: 0,\
        endOnTick: false,\
        showLastLabel: true,\
        title: {\
            text: 'Frequency (%)'\
        },\
        labels: {\
            formatter: function () {\
                return this.value + '%';\
            }\
        },\
        reversedStacks: false\
    },\
    tooltip: {\
        valueSuffix: '%'\
    },\n\
    plotOptions: {\
        series: {\
            stacking: 'normal',\
            shadow: false,\
            groupPadding: 0,\
            pointPlacement: 'on'\
        }\
    }\
});\n\
});\n\
</script></head><body>";

//  style=display:none

const char *wdir_chart_end = "<dvi class=chart-spacer><div id=container></div></div></body</html>";

// ------------------------- Wind direction chart


String html_for_wdir_chart(struct tm *req_time) {
  String html = wdir_chart_header;
  int windchart_entries = populate_windchart(req_time);

  //     {
  //        name: '10-20 kmph',
  //        data: [1.5, 4.5, 3.0, 1.5, 4.5, 3.0, 1.5, 4.5, 3.0, 1.5, 4.5, 3.0, 1.5, 4.5, 3.0, 1.5]
  //      },

  for(int i=0;i<WINDCHART_SPEEDS;i++) {
    html += "{ name: '";
    html += wind_speeds[i];
    html += "',\ndata: [";
    for(int j=0;j<WINDCHART_CARDINALS;j++) {
      float entry_percentage = ((float)windchart[j][i]/windchart_entries)*100;
      char scratch[10];
      snprintf(scratch, 10, "%1.2f", entry_percentage);
      html += scratch;
      if (j<WINDCHART_CARDINALS-1) html += ",";
    }
    html += "]\n}";
    if (i<WINDCHART_SPEEDS-1) html += ",\n";
    
  }  

  /*if (windchart_entries>0) {
    html += wdir_chart_start;

    for(int i=0;i<WINDCHART_CARDINALS;i++) {
      html += "<tr><td class=dir>";
      html += DirTable[i];
      html += "</td>";
      for(int j=0;j<WINDCHART_SPEEDS;j++) {
        html += "<td class=data>";
        float entry_percentage = ((float)windchart[i][j]/windchart_entries)*100;

        char scratch[10];
        snprintf(scratch, 10, "%1.2f", entry_percentage);

        html += scratch;
        html += "</td>";
      }
      html+="</tr>\n";
      // do we need to add total??
    }
  }*/

  html += wdir_chart_jscript;

  html += navigation_html(M_WDIR);
      
  html += wdir_chart_end;

  return html;
}

const char *windspeed_head = "<html>\n\
    <head>\n\
        <meta http-equiv='refresh' content='60'/>\n\
        <title>WeatherNow Wind Speed Chart</title>\n\
        <link rel=stylesheet type=text/css href=style_chart.css>\n\
        <link href='https://fonts.googleapis.com/css?family=Saira+Extra+Condensed:300,700&display=swap' rel=stylesheet>\n\
        <script src=https://code.highcharts.com/highcharts.js></script>\n\
        <script src=https://code.highcharts.com/modules/exporting.js></script>\n\
        <script src=https://code.highcharts.com/modules/export-data.js></script>\n\
        <script>\n\
document.addEventListener('DOMContentLoaded', function () {var myChart =\n\
Highcharts.setOptions(\n\
    {\n\
        chart: {\
            style: {\
                fontFamily: 'Saira Extra Condensed'\
            }\
        },\
        time: {\n\
            useUTC: false\n\
        }\n\
    }\n\
);\n\
Highcharts.chart('container', {\n\
chart: {zoomType: 'x'},title: { \
        style: {\
            fontWeight: 700,\
            fontSize: '3em'\
        },\
        text: 'Wind Speed'},subtitle: {text: document.ontouchstart === undefined ?'Click and drag in the plot area to zoom in' : 'Pinch the chart to zoom in'},\n\
xAxis: {\n\
    type: 'datetime'\n\
},\n\
yAxis: {\n\
    title: {\
       text: 'Wind Speed'\
    },\n\
    plotBands: [\n\
        {\n\
            from: 0,\n\
            to: 1,\n\
            label: { text: 'Calm'},\n\
            style: {\n\
                    color: '#606060'\n\
                }\n\
        },\n\
        {\n\
            color: 'rgba(174, 241, 249, 0.75)',\n\
            from: 1,\n\
            to: 3,\n\
            label: {\n\
                text: 'Light air',\n\         
                style: {\n\
                    color: '#7A7A7A'\n\
                }\n\
            }\n\
        },\n\
        {\n\
            color: 'rgba(150,247,220, 0.5)',\n\
            from: 3, to: 7,\n\
            label: {\n\
                text: 'Light breeze',\n\
                style: {\n\
                    color: '#606060'\n\
                }}},\n\
        {\n\
            color: 'rgba(150,247,180, 0.75)',\n\
            from: 7,\n\
            to: 12,\n\
            label: {\n\
                text: 'Gentle breeze',  \n\          
                style: {\n\
                    color: '#7A7A7A'\n\
                }\n\
            }\n\
        },\n\
        {\n\
            color: 'rgba(111,244,111, 0.5)',\n\
            from: 12,\n\
            to: 18,\n\
            label: { text: 'Moderate breeze',\n\
                style: {\n\
                    color: '#606060'\n\
                }}\n\
        },\n\
        {\n\
            color: 'rgba(115,237,18, 0.75)',\n\
            from: 18,\n\
            to: 24,\n\
            label: {\n\
                text: 'Fresh breeze', \n\           
                style: {\n\
                    color: '#7A7A7A'\n\
                }\n\
            }\n\
        },\n\
        {\n\
            color: 'rgba(164,237,18, 0.5)',\n\
            from: 24,\n\
            to: 31,\n\
            label: { text: 'Strong breeze',\n\
                style: {\n\
                    color: '#606060'\n\
                }}\n\
        },\n\
        {\n\
            color: 'rgba(218,237,18, 0.75)',\n\
            from: 31,\n\
            to: 38,\n\
            label: {\n\
                text: 'Near gale',  \n\          
                style: {\n\
                    color: '#7A7A7A'\n\
                }\n\
            }\n\
        },\n\
        {\n\
            color: 'rgba(237,194,18, 0.5)',\n\
            from: 38,\n\
            to: 46,\n\
            label: { text: 'Gale',\n\
                style: {\n\
                    color: '#606060'\n\
                }}\n\
        },\n\
        {\n\
            color: 'rgba(237,99,18, 0.75)',\n\
            from: 46,\n\
            to: 54,\n\
            label: {\n\
                text: 'Severe gale',     \n\       
                style: {\n\
                    color: '#7A7A7A'\n\
                }\n\
            }\n\
        },\n\
        {\n\
            color: 'rgba(213,16,45, 0.5)',\n\
            from: 54,\n\
            to: 155,\n\
            label: { text: 'Storm',\n\
                style: {\n\
                    color: '#606060'\n\
                }}\n\
        }\n\
    ]\n\
},\n\
series: [\n\
    {\n\
        name: 'Wind Speed',\n\
        lineWidth: 4,\n\
        color: 'black',\n\
        data: ";

const char *windspeed_inter_series = "},\n { name: 'Max 3s Gust', data: ";
        
const char *windspeed_tail = "\n\
}]});});\n\
</script>\n\
</head>\n\
<body>\n";

const char *windspeed_body = "<div class=chart-spacer><div id=container style=min-width: 310px; height: 400px; margin: 0 auto></div></div>\n";

const char *windspeed_end = "</body></html>";

// ------------------------- Wind Speed Chart

String html_for_wind_speed_chart(struct tm *req_time) {
  String html = windspeed_head;
  html += return_json_day(req_time, M_WIND);
  html += windspeed_inter_series;
  html += return_json_day(req_time, M_GUST);
  html += windspeed_tail;
  html += navigation_html(M_WIND);
  html += windspeed_body;

  html += return_html_min_max_today(req_time, M_WIND);
  html += return_html_min_max_today(req_time, M_GUST);



  html += windspeed_end;

  return html;
}

String return_html_min_max_today(struct tm *req_time, int which) {

  String result = "<ul class=mm-summary-listing>\n<li class=mm-summary-list-title>";
  result += measure_names[which];
  result += " ";
  result += measure_units[which];
  // if req_time is recent & also not -99.0 then
  result += "</li>\n<li class=mm-summary-list-item><strong>";
  // else
  // change the background to red
  float value_now = return_recent(req_time, which);
  if (value_now!=-99.0) {
    result += value_now;
    result += "</strong> now</li>\n<li class=mm-summary-list-item><strong>";
    int minimum_index = return_index_minmax(req_time, false, which);
    result += measure[which][minimum_index];
    result += "</strong> @ ";
    result += time_from_index(minimum_index);
    result += "</li>\n";

    result += "<li class=mm-summary-list-item><strong>";
    int maximum_index = return_index_minmax(req_time, true, which);

    result += measure[which][maximum_index];
    result += "</strong> @ ";
    result += time_from_index(maximum_index);
  } else {
    result += "No values received today";
  }
  result += "</li>\n</ul>\n";


  return result;
}


// ------------------------- General chart

String html_for_general_chart(struct tm *req_time, int which) {
  String html = "<html><head><meta http-equiv='refresh' content='60'/><title>WeatherNow Chart</title>";
  html += "<link rel=stylesheet type=text/css href=style_chart.css>";
  html += "<link href='https://fonts.googleapis.com/css?family=Saira+Extra+Condensed:300,700&display=swap' rel=stylesheet>";
            html += "<script src=https://code.highcharts.com/highcharts.js></script>";
html += "<script src=https://code.highcharts.com/modules/exporting.js></script>";
html += "<script src=https://code.highcharts.com/modules/export-data.js></script>\n";

html += "<script>\n";

html += "document.addEventListener('DOMContentLoaded', function () {";
    html += "var myChart =\n";

    html += "Highcharts.setOptions({\
        chart: {\
            style: {\
                fontFamily: 'Saira Extra Condensed'\
            }\
        },\
        time: {useUTC: false}});\n";


html += "Highcharts.chart('container', {\n";
          html +=   "chart: {";
                html += "zoomType: 'x'";
            html += "},";
            html += "title: {\
              style: {\
                fontWeight: 700,\
                fontSize: '3em'\
        },";
            
                html += "text: '";
              html += measure_names[which];
            html += "'},";
            html += "subtitle: {";
                html += "text: document.ontouchstart === undefined ?";
                    html += "'Click and drag in the plot area to zoom in' : 'Pinch the chart to zoom in'";
            html += "},\n";
            html += "xAxis: {";
                html += "type: 'datetime'";

            html += "},\n";
            html += "yAxis: {";
                html += "title: {";
                    html += "text: '";
                html += measure_names[which];
                html += "'}";
            html += "},plotOptions: {\
    series: {\
        color: '#1976d2'\
    }\
},\n";
            html += "series: [{";
                html += "name: '";
                html += measure_names[which];
                html += "',\n";
                html += "data: ";

            html += return_json_day(req_time, which);

            html += "\n}]";
         html += "});";
    html += "}";
html += ");";

html += "</script></head>";

html += "<body>";

html += navigation_html(which);

html += windspeed_body;

html += return_html_min_max_today(req_time, which);

html += "</body></html>";

return html;
}

// --------------- NOT FOUND
void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);

}
