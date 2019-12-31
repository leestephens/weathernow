// Bonsall sheet ID
//String google_spreadsheet_id = "1mKYTlJHldU7TUb-y6CQyVl3XGPB1nPG-IvwQXTDoJVw";

// Lee On Solent sheet ID
String google_spreadsheet_id = "1_3jc2vSdTrUChp8tnwe4FeCWMpzj_aQZJaKx8me5_4o";

void handle_cloud_chart() {
  struct tm timeinfo;
  bool good_message = false;

  String message;
  
  int required_dataset = 0;
  int required_days = 7;
  int required_summary = SUMMARY_DAY;
  
  if (server.method() == HTTP_GET) {
    for (uint8_t i = 0; i < server.args(); i++) {
       Serial.print(server.argName(i));
       Serial.print(": ");
       Serial.println(server.arg(i));

       if (server.argName(i)=="data") {
        required_dataset = server.arg(i).toInt();
       }
       if (server.argName(i)=="days") {
        required_days = server.arg(i).toInt();
       }
       if (server.argName(i)=="summary") {
        required_summary = server.arg(i).toInt();
       }
    }
    if (required_days<=0) {
      required_days=7;
    }
    if (required_days>365) {
      required_days=7;
    }
    if ((required_summary<0)||(required_summary>SUMMARY_MONTH))  {
      required_summary = SUMMARY_DAY;
    }
       
       if ((required_dataset>-1)&&(required_dataset<=measure_in_array)) {
          if(!getLocalTime(&timeinfo)){
            Serial.println("Failed to obtain time");
          } else {
            switch (required_dataset) {
              case M_WDIR:
                //message = html_for_wdir_chart(&timeinfo);
                break;
              case M_WIND:
              case M_GUST:
                //message = html_for_wind_speed_chart(&timeinfo);
                break;
              default:
                message = html_for_google_general_chart(&timeinfo, required_dataset, required_days, required_summary);
            }
            good_message = true;
          }

       } 
     }
  if (!good_message) {
    message = "No data";
  }
  server.send(200, "text/html", message);  
}

const char *cloud_chart_header = "<html><head>\
    <title>WeatherNow Chart</title>\
    <script src='https://code.highcharts.com/highcharts.js'></script>\
    <script src='https://code.highcharts.com/highcharts-more.js'></script>\
    <script src='https://code.highcharts.com/modules/data.js'></script>\
    <script src='https://code.highcharts.com/modules/exporting.js'></script>\
<script>\
document.addEventListener('DOMContentLoaded',\
    function () {\
        var myChart =\
        Highcharts.chart('container', {\
            chart: {\
                type: 'datetime'\
            },\
            data: {\
                csvURL: 'https://docs.google.com/a/google.com/spreadsheets/d/";

const char *cloud_chart_footer = "}\
        );\
    }\
);\
</script></head>\
<body>\
<div class=chart-spacer><div id=container style=min-width: 310px; height: 400px; margin: 0 auto></div></div>\
</body></html>";

const char *cloud_chart_columns[] = { "B", "C", "D", "E", "F", "G", "H"};

const char *cloud_chart_date_column = "A";

String html_for_google_general_chart(struct tm *req_time, int which, int days_of_data, int summarised_level) {
  String cloud_chart_html = cloud_chart_header;
  cloud_chart_html += google_spreadsheet_id;
  cloud_chart_html += "/gviz/tq?tq="; // Query logic
  cloud_chart_html += encoded_query(which, days_ago_as_yyyy_mm_dd(req_time, days_of_data), summarised_level);
  cloud_chart_html += "&tqx=out:csv'";
  cloud_chart_html += "},            title: {text: '";
  cloud_chart_html += measure_names[which];
  cloud_chart_html += "'}";
  cloud_chart_html += cloud_chart_footer;
  return cloud_chart_html;
}

String encoded_query(int which, String date_yyyy_mm_dd, int summary_level) {
  String response = "select%20";
  if (summary_level == SUMMARY_RAW) {
    response += "A%2C%20";
  } else {
    response += "toDate(A)%2C%20";
  }
  if (summary_level == SUMMARY_RAW) {
    response += cloud_chart_columns[which];
  } else if (summary_level == SUMMARY_HOUR) {
    // um.
  } else { // hopefully it's SUMMARY_DAY
    response += "avg(";
    response += cloud_chart_columns[which];
    response += ")%2C%20max(";
    response += cloud_chart_columns[which];
    response += ")%2C%20min(";
    response += cloud_chart_columns[which];
    response += ")";
  }
  response += "%20where%20A%3Edate%22";
  response += date_yyyy_mm_dd;
  response += "%22";
  if (summary_level > SUMMARY_RAW) {
    response += "%20group%20by%20toDate(A)";
  }
  response += "%20order%20by%20";
  if (summary_level == SUMMARY_RAW) {
    response += "A";
  } else if (summary_level == SUMMARY_HOUR) {
    // um.
  } else { // it's SUMMARY_DAY
    response +=  "toDate(A)";
  }

  Serial.print("Google sheet query for chart is: ");
  Serial.println(response);
  
  return response;
}

String days_ago_as_yyyy_mm_dd(struct tm *req_time, int days) {
  
 
  long days_in_seconds = (days-1)*24*60*60;
  time_t req_time_t = mktime(req_time);
  req_time_t -= days_in_seconds;

  char buff[12];
  strftime(buff, 20, "%Y-%m-%d", localtime(&req_time_t));

  String result = "";

  result += buff;
  
  return result;
}
