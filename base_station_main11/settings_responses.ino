/*
 * FUNCTION handle_cloud_upload
 */
void handle_cloud_upload() {
  
}

/*
 * FUNCTION handle_timer
 */
void handle_timer() {
  
}

/*
 * FUNCTION handle_screen_mode
 */
void handle_screen_mode() {
  
}

/*
 * FUNCTION handle_time
 */
void handle_time() {
  
}

/*
 * FUNCTION handle_reset_error
 */
void handle_reset_error() {
  
}

/*
 * FUNCTION handle_restart
 */
const char *restart_html = "<!DOCTYPE html>\
<html>\
    <head>\
        <title>WeatherNow is restarting</title>\
        <link href='https://fonts.googleapis.com/css?family=Saira+Extra+Condensed:300,700&display=swap' rel=stylesheet>\
        <style>\
            body {\
                font-family: 'Saira Extra Condensed', Helvetica, Arial, Sans-Serif;\
                margin: 20px;\
                background-color: #1976d2;\
                color: #ffffff;\
                font-size: 3em;\
            }\
            .counter {\
                display: inline;\
            }\
        </style>\
    </head>\n\
    <body>\
            WeatherNow is restarting.\
            We'll reload this page in <div class=counter id='counter'>60</div> seconds.\
            Hopefully, it will be ready by then :)\n\
            <script>\
                setInterval(function() {\
                    var div = document.querySelector('#counter');\
                    var count = div.textContent * 1 - 1;\
                    div.textContent = count;\
                    if (count <= 0) {\
                        window.location.replace('/');\
                    }\
                }, 1000);\
            </script>\
    </body>\
</html>";

void handle_restart() {
  server.send(200, "text/html", restart_html);
  delay(5000);
  ESP.restart();
  
}
