


const char* style_css = "\
        body {\
            font-family: 'Saira Extra Condensed', Helvetica, Arial, Sans-Serif;\
            margin: 0 0 0 0;\
            background-color: #ffffff;\
            }\n\
        P {\
            text-align: center;\
            font-size: 5em;\
            margin: 0;\
            background-color: #ffffff;\
        }\
        .title-bar {\
            display: grid;\
            grid-template-columns: 100px auto;\        
            background-color: #1976d2;\
            align-items: center;\
            margin: 20px;\
            border-radius: 4px;\
            box-shadow: 1px 2px 10px rgba(0, 0, 0, .5);\
        }\n\
        .title-icon {\
            padding-left: 20px;\
        }\n\
        .title-text {\
            align-content: center;\
            color: #ffffff;\
            font-size: 3em;\
        }\n\
        .sub-bar {\
            margin: 0px 20px 10px 20px;\
            background-color: #ffffff;\
            color: #888888;\
            font-size: 2em;\
            padding-left: 20px;\
            box-shadow: 1px 2px 5px rgba(0, 0, 0, .5);\
            border-radius: 4px;\
        }\n\
        .error-bar {\
            margin: 0px 20px 10px 20px;\
            background-color: #b00020;\
            color: #ffffff;\
            font-size: 2em;\
            padding-left: 20px;\
            box-shadow: 1px 2px 10px rgba(0, 0, 0, .5);\
            border-radius: 4px;\
        }\n\
        .listing {\
            display: grid;\
            grid-column-gap: 4%;\
            padding: 20px;\
            grid-template-columns: 48% 48%;\
        }\n\
        .measure {\
            border-radius: 4px;\
            grid-template-columns: auto auto;\
            display: grid;\
            background-color: #ffffff;\
            break-inside: avoid;\
            color: #888888;\
            font-size: 3em;\
            margin-bottom: 20px;\
            box-shadow: 1px 2px 5px rgba(0, 0, 0, .5);\
        }\n\
        .measure-error {\
            border-radius: 4px;\
            grid-template-columns: auto auto;\
            display: grid;\
            background-color: #b00020;\
            break-inside: avoid;\
            color: #ffffff;\
            font-size: 3em;\
            margin-bottom: 20px;\
            box-shadow: 1px 2px 5px rgba(0, 0, 0, .5);\
        }\n\ 
        .measure-title {\
            text-align: left;\
            padding-left: 20px;\
        }\n\
        .measure-units {\
            text-align: right;\
            padding-right: 20px;\
        }\n\
        .measure-value {\
            grid-column: 1 / span 2;\
            font-size: 4em;\
            line-height: 1em;\
            text-align: center;\
            color: #000000;\
        }\n\ 
        .measure-value-error {\
            grid-column: 1 / span 2;\
            font-size: 4em;\
            line-height: 1em;\
            text-align: center;\
            color: #ffffff;\
        }\n\           
        a {\
            color: inherit;\
            text-decoration: none;\
        }\n\
        .material-icons.md-light {\
            color: rgba(255, 255, 255, 1);\
        }\n\
        .material-icons.md-dark {\
            color: #000000;\
        }\n\
        .material-icons.md-48 {\
            font-size: 48px;\
        }\n\
        .material-icons.md-36 {\
            font-size: 36px;\
            vertical-align: middle;\
        }\n\
        .s-menu {\
            border-radius: 4px;\
            box-shadow: 1px 2px 20px rgba(0, 0, 0, .5);\
            width: 400px;\
            height: 90%;\
            padding: 0px;\
            font-size: 4em;\
            position: fixed;\
            top: 0;\
            background-color: #ffffff;\
            transition: all 0.3s ease;\
            margin: 0;\
         }\n\
         .s-menu-list {\
            padding: 0px;\
         }\n\
         .s-menu-header {\
                list-style: none;\
                background-color: #1976d2;\  
                color: #ffffff;\
                padding-left: 20px;\
                margin-top: 30px;\
            }\n\
            .s-menu-item {\
                list-style: none;\
                background-color: #ffffff;\
                vertical-align: bottom;\
                padding-left: 20px;\
            }\n\
            .s-menu-item:hover {\
                background-color: #eeeeee;\
            }\n\
        @media only screen and (orientation: landscape) {\n\
            .listing {\
                grid-template-columns: 23.5% 23.5% 23.5% 23.5%;\
                grid-gap: 2%;\
            }\n\
            .measure {\
                font-size: 1.5em;\
            }\n\
            .measure-error {\
                font-size: 1.5em;\
            }\n\
            .s-menu {\
              font-size: 2em;\
            }\n\
        }";


void handleStyleCSS() {
  server.send(200, "text/css", style_css);
}

const char* style_css_graph = "\
        body {\
            font-family: 'Saira Extra Condensed', Helvetica, Arial, Sans-Serif;\
            margin: 0 0 0 0;\
            background-color: #ffffff;\
            }\n\
        P {  \
            text-align: center;  \
            font-size: 1em;  \
            margin: 10px 20px 10px 20px;\
            background-color: #ffffff;\
        }\n\
        .nav-list {\
            list-style: none;\
            margin: 0 0 0 0;\
            display: grid;\
            grid-gap: 10px;\
            grid-template-columns: auto auto auto auto auto auto auto;\
            background-color: #ffffff;\
            padding: 10px 20px 10px 20px;\
            align-content: center;\
            font-size: 2em;\
        }\n\
        .nav-list-item {\
            border-radius: 4px;\
            box-shadow: 1px 2px 5px rgba(0, 0, 0, .5);\
            background-color: #ffffff;\
            color: #888888;\
            text-align: center;\
            font-weight: 300;\
        }\n\
        .nav-list-item-active {\
            border-radius: 4px;\
            box-shadow: 1px 2px 5px rgba(0, 0, 0, .5);\
            background-color: #1976d2;\
            color: #ffffff;\
            text-align: center;\
            font-weight: 300;\
        }\n\
        .mm-summary-listing {\
            border-radius: 4px;\
            box-shadow: 1px 2px 5px rgba(0, 0, 0, .5);\
            list-style: none;\
            margin: 20px;\
            display: grid;  \
            grid-gap: 10px;  \
            grid-template-columns: auto auto auto auto;  \
            background-color: #ffffff;  \
            padding: 10px 20px 10px 20px;\
            align-content: center;\
            font-size: 2em;\
        }\n\
        strong {\
            color: #000000;\
        }\n\
        .mm-summary-list-title {\
            font-weight: 300;\
            font-size: 1em;\
            background-color: #ffffff;\
            text-align: center;\
        }\n\
        .mm-summary-list-item {\
            background-color: #ffffff;\
            color: #888888;\
            text-align: center;\
        }\n\
        .chart-spacer {\
            border-radius: 4px;\
            margin-left: 20px;\
            margin-right: 20px;\
            box-shadow: 1px 2px 5px rgba(0, 0, 0, .5);\
       }\n\
        a:link {\
  text-decoration: none;\
  color: inherit;\
}\n\
a:visited {\n\
  text-decoration: none;\
  color: inherit;\
}\n\
a:hover {\
  text-decoration: underline;\
  color: inherit:\
}\n\
";


void handleStyleCSSChart() {
  server.send(200, "text/css", style_css_graph);
}
