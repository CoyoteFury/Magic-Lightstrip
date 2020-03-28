/*********

  Title       : FastLED Complete Web Server
  Author      : F.MONTEDORI
  Date        : March, 2020 
  Description : Control your led strip from any web browser with a friendly UI
                First time connnect to WiFi ESP_Light_379357_AutoConnectAP to autoconnect to your network
  
*********/

/*********
  Load EEPROM library to store scenes
*********/
#include <EEPROM.h>
byte selectedscene = 1; // Default scene
byte selectedsceneMax = 33;

/*********
  Load FastLED library
*********/
#include <FastLED.h> 

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

  //  FastLED Fixed definitions cannot change on the fly.
  #define LED_DT 23                                             // Serial data pin
  #define LED_CK 22                                             // Clock pin for WS2801 or APA102
  #define COLOR_ORDER BGR                                       // It's GRB for WS2812B
  #define LED_TYPE APA102                                       // What kind of strip are you using (APA102, WS2801 or WS2812B)?
  #define NUM_LEDS 44                                           // Number of LED's

  #define MAX_BRIGHTNESS     128                                // Thats full on, watch the power!
  #define MIN_BRIGHTNESS      16                                // set to a minimum of 25%
  #define BRIGHTNESS          96                                // Default = 96
  #define FRAMES_PER_SECOND  120

  bool gReverseDirection = false;

  // Default Color and brightness
  long RGBcode = 0xffA95c;
  int CURBRIGHTNESS = 96;

  // Gradient ResetCycle Virtual button and Timer
  byte ResetCycle = 0;
  int gradientTimer = 10; 

  // Global variables can be changed on the fly.
  uint8_t max_bright = 128;                                     // Overall brightness.
  struct CRGB leds[NUM_LEDS];                                   // Initialize our LED array.

  uint8_t gHue = 0;                                             // rotating "base color" used by many of the patterns

  CRGBPalette16 currentPalette;                                 // Palette definitions
  CRGBPalette16 targetPalette;
  
  CRGBPalette16 SunrisePalette;
  CRGBPalette16 SunsetPalette;
  
  TBlendType currentBlending = LINEARBLEND;
  
  // Lightnings scene declaration
  uint8_t frequency = 50;                                       // controls the interval between strikes
  uint8_t flashes = 8;                                          //the upper limit of flashes per strike
  unsigned int dimmer = 1;

  uint8_t ledstart;                                             // Starting location of a flash
  uint8_t ledlen;                                               // Length of a flash
 
// Define Custom Color palette

// Gradient palette "aurora_borealis_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pd/astro/tn/aurora_borealis.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 196 bytes of program space.

DEFINE_GRADIENT_PALETTE( aurora_borealis_gp ) {
    0,  16,  6, 15,
   47,  35, 11, 25,
   90,  66, 18, 35,
   95,  74, 21, 40,
  143, 121, 29, 44,
  180, 155, 81, 69,
  185, 150,100, 68,
  191, 142,122, 67,
  196, 130,146, 63,
  201, 118,169, 59,
  207, 100,184, 52,
  212,  84,197, 48,
  239,  32,119, 23,
  244,  26, 71, 24,
  249,  24, 46, 25,
  255,  22, 28, 26};

DEFINE_GRADIENT_PALETTE( candles_gp ) {
    0, 255,255,0,  
  110, 255,50,0,
  200, 255,150,0,
  255, 255,255,0};

DEFINE_GRADIENT_PALETTE( sunrise_surf_gp ) {
    0, 0,0,0,
   51, 77,37,76,
  102, 247,110,72,
  178, 253,240,196,
  255, 130,160,194};

DEFINE_GRADIENT_PALETTE( sunset_surf_gp ) {
   0,  130,160,194,
   51, 253,240,196,
  127, 247,110,72,
  191, 77,37,76,
  255, 0,0,0};

DEFINE_GRADIENT_PALETTE( sunset_red_gp ) {
   0,  255,0,0,
   51, 248,122,85,
  127, 159,93,108,
  204, 92,81,119,
  255, 0,0,0};

DEFINE_GRADIENT_PALETTE( sunset_blue_gp ) {
   0, 185,242,250,
  90, 7,57,86,
 178, 2,18,47,
 255, 0,0,0};

DEFINE_GRADIENT_PALETTE( halloween_gp ) {
  0, 27,25,25,
  64, 150,150,150,
  127, 118,0,254,
  184, 255,104,0,
  255, 88,255,0};

DEFINE_GRADIENT_PALETTE( thunderstorm_gp ) {
  0, 189,189,253,
  84, 188,117,249,
  128, 83,94,235,
  255, 0,33,138};

DEFINE_GRADIENT_PALETTE( romance_gp ) {
  0, 206,150,251,
  84, 255,143,207,
  128, 0,194,186,
  255, 3,122,144};

DEFINE_GRADIENT_PALETTE( neon_gp ) {
  0, 0,255,140,
  84, 1,255,255,
  128, 255,99,251,
  255, 102,25,255};

DEFINE_GRADIENT_PALETTE( holiday_gp ) {
  0, 199,239,38,
  84, 255,170,1,
  128, 18,178,150,
  255, 2,93,159};

DEFINE_GRADIENT_PALETTE( meditation_gp ) {
  0, 186,177,144,
  84, 40,180,182,
  128, 74,149,31,
  255, 10,17,27};

DEFINE_GRADIENT_PALETTE( outdoor_gp ) {
  0, 12,132,50,
  84, 88,196,121,
  128, 255,221,15,
  255, 233,155,0};

/*********
  Load WiFiManager library
*********/
#include <WiFiManager.h>

WiFiManager wm;                           // Create an instance of WiFiManager
const char* ssid = "ESP_Light_379357_AutoConnectAP";
const char* password = "your_password";

/*********
  Load Async Web Server library and SPIFFS
*********/
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
AsyncWebServer server(80);

// HTML web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>esp personal wireless lighting strip</title>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <meta name="mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <link rel="icon" type="image/png" href="/Magic-icon.png">
    <script src="iro.min.js"></script>

    <style>
    html {
  text-align: center;
  font-size: 12px;
  font-family: Helvetica;
  text-decoration: none;
  color: #ffffff;
}

body {
  background-color: #1f1f1f;
}

header {
  left: 0;
  width: 100%;
  top: 0;
  padding: 8px 0;
    position: fixed;        
  background-color: #242424;
}

main {
    padding-top: 80px; /* Header height */
    padding-bottom: 50px; /* Footer Height */
}

footer {
  left: 0;
  width: 100%;
  bottom: 0;
    position: fixed;  
    padding: 20px 0;      
  font-size: 10px;
  background-color: #1b1b1b;
  border-top: 1px solid #3d3d3d;
}

  a:link {
    color: #ffffff;
    text-decoration:none;   
  }
  
  a:visited {
    color: #ffffff;
    text-decoration:none;   
  }
    
  h1,h2 {
    margin: 8px 0;
    text-transform: uppercase;
  }
  
  h1 {
    font-size: 20px;
  }
  
  h2 {
    font-size: 12px;
    color: gray;
  }
  
  form {
    display: inline;
  }
  
  input[type=text], select {
      width: 80%;
      padding: 12px 20px;
      margin: 8px 0px;
      display: inline-block;
      border: 1px solid #505050;
      border-radius: 6px;
      box-sizing: border-box;
      font-size: 18px;
      text-align:center;
      background-color: #1f1f1f;
      color: white;
  }
  
  select {
    -moz-appearance: window;
      -webkit-appearance: none;
      text-align-last:center; 
  }
  
  input[type=range] {
      width: 80%;
      padding: 12px 20px;
      margin: 8px 0px;
      display: inline-block;
      box-sizing: border-box;
      background-color: #1f1f1f;
  }
  
  .button {
    background-color: #505050;
    border: none;
    color: white;
    padding: 8px 40px;
    text-decoration: none;
    font-size: 16px;
    margin: 20px 20px;
    cursor: pointer;
    border-radius: 16px;
    width: 140px;
  }

  input:focus, textarea:focus, textarea:focus, input:active, textarea:active, input:active, select:focus, input, textarea {
      outline-style: none !important;
      outline: none !important;
      outline: 0 !important;
      border: 2px solid #ccc;
  }

/* START switch ON-OFF */
  .switch {
    right: 20px;
      top: 20px;
      position: absolute;
      width: 60px;
      height: 34px;
  }

  .switch input { 
      opacity: 0;
      width: 0;
      height: 0;
  }

  .slider {
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #7b7b7b;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 34px;
  }

  .slider:before {
    position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      -webkit-transition: .4s;
      transition: .4s;
      border-radius: 50%;
  }

  input:checked + .slider {
      background-color: #ffa95c;
  }

  input:checked + .slider:before {
      -webkit-transform: translateX(26px);
      -ms-transform: translateX(26px);
      transform: translateX(26px);
  }
/* END switch ON-OFF */    
    </style>
    
  </head>
  <script type="text/javascript">

    // get URL variables
    function getQueryVariable(variable)
    {
     var query = window.location.search.substring(1);
       var vars = query.split("&");
       for (var i=0;i<vars.length;i++) {
               var pair = vars[i].split("=");
               if(pair[0] == variable){return pair[1];}
       }
       return(undefined);
    }
     
    // Verify ON-OFF toggle switch
    function switchon(checkboxElement) {
      if (checkboxElement.checked) {
      // ON
      window.location.href='/?scene=1';
      } else {
      // OFF
      window.location.href='/?scene=0';
      }
  }
    
  </script>
  <body background="">

    <!-- Header -->    
      <header id="header">
        <h1>Magic lightStrip</h1>
        <h2>Control your visual atmosphere</h2>
        <label class="switch">
        <script>
        // If scene = 0, switch off 
        if (getQueryVariable("scene") == 0 ) {
          document.write('<input name="switch_on-off" type="checkbox" onchange="switchon(this)">');
              } else {
                document.write('<input name="switch_on-off" type="checkbox" checked onchange="switchon(this)">');
              }
            
        </script>   
          <span class="slider"></span>
    </label>
      </header>

    <!-- Main -->
      <main>


                   
              <form action="/" id="my-form">
              
              <div align="center" id="picker" name="picker"></div>
              <script>
                if (getQueryVariable("RGB") == undefined) {
                  var colorStarter = "#ffa95c";
                } else {
                    var colorStarter = "#" + getQueryVariable("RGB").substring(3);
                }
                    var colorPicker = new iro.ColorPicker('#picker', {
                     
                  color: colorStarter,
                  borderWidth: 0,
                  layout: [
                  {
                      component: iro.ui.Wheel,
                      options: {
                        borderColor: '#ffffff'
                    }
                  },
                  {
                      component: iro.ui.Slider,
                      options: {
                        borderColor: '#ffffff'
                      }
                  }
                  ]
               });                    
               
              // if color change adjust RGB field text            
              colorPicker.on('color:change', function(color) {
                 document.getElementById("input_RGB").setAttribute('value',color.hexString);
              });
              
              // if scene >= 1 or false (first time) hue will be set by default 100
              if (getQueryVariable("scene") == 0) 
              {
                colorPicker.color.value = "0";
              }
              
              </script>
              
              <br />
              <input type="range" id="input_brightness" name="brightness" min="16" max="128" step="8"><br />
              <script type="text/javascript">
                if (getQueryVariable("brightness") == undefined) {
                  document.getElementById("input_brightness").setAttribute('value',"96");
                } else {
                  document.getElementById("input_brightness").setAttribute('value',getQueryVariable("brightness"));
                }
              </script>
              <br />
              
              <input type="text" id="input_RGB" name="RGB">
              <script type="text/javascript">      
                document.getElementById("input_RGB").setAttribute('value',colorStarter);
              </script>
              <br />
                 
          <select id="scene" name="scene">
          <option value="1">Choose a scene</option>
        
        <script type="text/javascript">
          var x = document.getElementById("scene"); 
          var scenes = ["GSTR_BASIC","Off","On","GEND_BASIC","GSTR_HUE","Energize","Concentrate","Read","Relax","GEND_HUE","GSTR_ANIMATIONS","One Pixel","Rainbow","Confetti","Sinelon",
          "bpm","Juggle","Fire","Plasma","Lightnings","Snow","Color Wipe","Meteor Rain","Theater Chase","Cylon Bounce","Strobe","Sparkle","Aurora Borealis","Candles","GEND_ANIMATIONS","GSTR_ATMOSPHERE",
          "Colorloop","Sunrise","Sunset","Neon","Thunderstorm","Romance","Halloween","Holiday","Medidation","Outdoor","GEND_ATMOSPHERE"]; 
          var j = 0;
    for(var i = 0; i < scenes.length; i++) {
          if(scenes[i].substring(0,5) == "GSTR_") { // Detect Group STaRt
            document.write('<optgroup label="' + scenes[i].substring(5) + '" >');
          }
          else if(scenes[i].substring(0,5) == "GEND_") { // Detect Group END
            document.write('</optgroup>');
          }
          else {
            document.write('<option value=' + j + '>' + scenes[i] + '</option>');
            j++
          }
                
        }
        </script>
        </select>
        <br />
        <script>
        if (getQueryVariable("scene") == undefined) {
                  x.selectedIndex = "2"; // = scene number 1 : Poweron
                } else {
                  x.selectedIndex = parseInt(getQueryVariable("scene")) +1; // must add 1 because of "Choose an scene" text in list
                }
        </script>
        

      <select id="timer" name="timer">
          <option value="0">Atmosphere Cycle Length</option>
        
        <script type="text/javascript">
            var x = document.getElementById("timer"); 
            var timers = ["1 minute", "5 minutes", "10 minutes", "20 minutes", "30 minutes", "1 hour", "1,5 hour" ,"2 hours", "3 hours", "4 hours", "5 hours"]; 
            var timers_value = [1, 5, 10, 20, 30, 60, 90, 120, 180, 240, 300 ];
            var j = 0;
          for(var i = 0; i < timers.length; i++) {
              document.write('<option value=' + timers_value[i] + '>' + timers[i] + '</option>');
          }
        </script>
        </select>
    <br />

                            
              </form>
      
      <a href="/?RGB=%23ffa95c&brightness=96&scene=1"><button class="button">Reset</button></a>
      <button class="button" type="submit" form="my-form">Set</button>
            
      </main>

    <!-- Footer -->
      <footer id="footer">
          &copy; 2020 <a href="YOUR_SITE">ESP32+FastLED</a>. All rights reserved. 
      </footer>

    <!-- Scripts -->

  </body>
</html>
)rawliteral";


/*********

  SETUP
  
*********/

void setup(){
  Serial.begin(115200);
  
  //  WiFi Declarations
  WiFi.mode(WIFI_STA);
  
  delay(1000);
  Serial.println("\n");
  
  if(!wm.autoConnect(ssid, password))
    Serial.println("WiFi connection error.");
  else
    Serial.println("WiFi connection success!");

  // Mounting SPIFFS
  if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }

  //  FastLED Declarations
  //  LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS);      // Use this for WS2812B
  LEDS.addLeds<LED_TYPE, LED_DT, LED_CK, COLOR_ORDER>(leds, NUM_LEDS);  // Use this for WS2801 or APA102
  
  /* set master brightness control, limited betwen :
     MIN_BRIGHTNESS: if mappedValue is less than our defined MIN_BRIGHTNESS.
     MAX_BRIGHTNESS: if mappedValue is greater than our defined MAX_BRIGHTNESS
  */ 
  FastLED.setBrightness(constrain(BRIGHTNESS, MIN_BRIGHTNESS, MAX_BRIGHTNESS));

  // Plasma palette declaration
  currentPalette = OceanColors_p;
  
  //  Web Server Declarations
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
 
    int paramsNr = request->params();
    Serial.println(paramsNr);
 
    for(int i=0;i<paramsNr;i++){
 
        AsyncWebParameter* p = request->getParam(i);
        // Show parameters in Serial
        Serial.print("Param name: ");
        Serial.println(p->name());
        
        // Get values from arguments
        if (p->name() == "RGB") {
          String colorString = p->value().substring(1); // remove #
          RGBcode = strtol(colorString.c_str(), NULL, 16);
          Serial.println(RGBcode);
        }
        else if (p->name() == "brightness") {
           CURBRIGHTNESS = p->value().toInt();
        }
        else if (p->name() == "scene") {
           selectedscene = p->value().toInt();
           // If scene changed, reset GradiantCycle and shuffle Palette
           ResetCycle = 1;
           selectRandomSunrisePalette();
           selectRandomSunsetPalette();
        }
        else if (p->name() == "timer") {
           gradientTimer = p->value().toInt();
           if (gradientTimer == 0) {
              gradientTimer = 10;
           }
        }
        Serial.print("Param value: ");
        Serial.println(p->value());
        Serial.println("------");
    }
 
    request->send(200, "text/html", index_html);
  });

  server.on("/iro.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/iro.min.js", "text/javascript");
  });

  server.on("/Magic-icon.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Magic-icon.png", "image/png");
  });

  server.begin();
}

/*********

  LOOP
  
*********/
 
void loop(){

  // If GPIO4 (D4) is touched, reset WiFi settings
  // This part is disabled because touch is too sensitive
  // if(touchRead(T0) < 50)
  // {
  //  Serial.println("GPIO4 was touched : Reset WiFi settings then reboot...");
  //  wm.resetSettings();
  //  ESP.restart();
  // }

  FastLED.setBrightness(constrain(CURBRIGHTNESS, MIN_BRIGHTNESS, MAX_BRIGHTNESS));

  // EEPROM Access
  EEPROM.get(0,selectedscene); 

  if(selectedscene>selectedsceneMax) { 
    selectedscene=0;
    EEPROM.put(0,0);  
  }

  if(selectedscene<0) { 
    selectedscene=selectedsceneMax;
  }

  switch(selectedscene) {

    case 0  : {
                Poweroff();
                break;
              }
    
    case 1  : {
                Poweron(RGBcode,CURBRIGHTNESS);
                break;
              }


    case 2  : {
                // Energize (07h00-09h00)
                Poweron(0xFFFDF8,CURBRIGHTNESS);
                break;
              }

    case 3  : {
                // Concentrate (09h00-17h00)
                Poweron(0xFFD5B3,CURBRIGHTNESS);
                break;
              }
              
    case 4  : {
                // Read (17h00-20h00)
                Poweron(0xFFAD66,CURBRIGHTNESS);
                break;
              }
              
    case 5  : {
                // Relax (20h00-23h00)
                Poweron(0xFF942B,56);
                break;
              } 

    case 6  : {
                OnePixel();
                break;
              }
             
    case 7  : {
                rainbow();
                break;
              }
              
    case 8  : {
                confetti();
                break;
              }

    case 9  : {
                sinelon();
                break;
              }

    case 10  : {
                bpm();
                break;
              }

    case 11  : {
                juggle();
                break;
              }

    case 12  : {
                #define FIRE_FRAMES_PER_SECOND  60
                // Fire2012 by Mark Kriegsman, July 2012
                // as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
                //// 
                // This basic one-dimensional 'fire' simulation works roughly as follows:
                // There's a underlying array of 'heat' cells, that model the temperature
                // at each point along the line.  Every cycle through the simulation, 
                // four steps are performed:
                //  1) All cells cool down a little bit, losing heat to the air
                //  2) The heat from each cell drifts 'up' and diffuses a little
                //  3) Sometimes randomly new 'sparks' of heat are added at the bottom
                //  4) The heat from each cell is rendered as a color into the leds array
                //     The heat-to-color mapping uses a black-body radiation approximation.
                //
                // Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
                //
                // This simulation scales it self a bit depending on NUM_LEDS; it should look
                // "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
                //
                // I recommend running this simulation at anywhere from 30-100 frames per second,
                // meaning an interframe delay of about 10-35 milliseconds.
                //
                // Looks best on a high-density LED setup (60+ pixels/meter).
                //
                //
                // There are two main parameters you can play with to control the look and
                // feel of your fire: COOLING (used in step 1 above), and SPARKING (used
                // in step 3 above).
                //
                // COOLING: How much does the air cool as it rises?
                // Less cooling = taller flames.  More cooling = shorter flames.
                // Default 50, suggested range 20-100 
                #define COOLING  55
                // SPARKING: What chance (out of 255) is there that a new spark will be lit?
                // Higher chance = more roaring fire.  Lower chance = more flickery fire.
                // Default 120, suggested range 50-200.
                #define SPARKING 120
                Fire2012();
                break;
                }

    case 13  : {
                #define qsubd(x, b)  ((x>b)?b:0)                              // Digital unsigned subtraction macro. if result <0, then => 0. Otherwise, take on fixed value.
                #define qsuba(x, b)  ((x>b)?x-b:0)                            // Analog Unsigned subtraction macro. if result <0, then => 0
                
                uint8_t max_bright = 255;                                     // Overall brightness definition. It can be changed on the fly.
               
                plasma();
                break;
              }

    case 14  : {
                FastLED.clear();
                FastLED.setBrightness(MAX_BRIGHTNESS);
                uint8_t max_bright = 255;                                      // Overall brightness definition. It can be changed on the fly.
                lightnings();
                break;
              }
              
    case 15  : {
                // SnowSparkle - Color (red, green, blue), sparkle delay, speed delay
                SnowSparkle(0x101010, 20, random(100,1000));
                break;
              }

    case 16 : {
                // colorWipe - Color (RGBCode), speed delay 
                colorWipe(RGBcode, 50);
                colorWipe(0x000000, 50);
                break;
              }
              
    case 17 : {
                // meteorRain - Hex Color, meteor size, trail decay, random trail decay (true/false), speed delay 
                meteorRain(RGBcode,10, 64, true, 30);
                break;
              }

    case 18 : {
                // theatherChase - Hex Color, speed delay
                theaterChase(RGBcode,50);
                break;
              }
              
    case 19 : {
                // CylonBounce - Color (hex Color), eye size, speed delay, end pause
                CylonBounce(RGBcode, 4, 50, 50);
                break;
              }
              
    case 20 : {
                // Strobe - Color (red, green, blue), number of flashes, flash speed, end pause
                Strobe(RGBcode, 10, 50, 1000);
                break;
              }

    case 21 : {
                // Sparkle - Color (red, green, blue), speed delay
                Sparkle(RGBcode, 0);
                break;
              }

    case 22 : {
                // Aurora
                aurora( aurora_borealis_gp, 10000/FRAMES_PER_SECOND );
                break;
              }

    case 23  : {
                // Candles
                aurora( candles_gp, 1000/FRAMES_PER_SECOND );
                break;
              }

    case 24  : {
                // ColorLoop
                // GradientPaletteCylce(Color Palette, length of a color in a cycle, infinite loop or stop at the end?)
                GradientPalette(Rainbow_gp, gradientTimer, true);
                break;
              }   

    case 25  : {
                // Sunrise
                // GradientPaletteCylce(Color Palette, length of a cycle in minutes, infinite loop or stop at the end?)               
                GradientPalette(SunrisePalette, gradientTimer, false);
                break;
              }   
 
    case 26  : {
                // Sunset
                // GradientPaletteCylce(Color Palette, length of a cycle in minutes, infinite loop or stop at the end?)               
                GradientPalette(SunsetPalette, gradientTimer, false);
                break;
              }

    case 27  : {
                // Neon
                // GradientPaletteCylce(Color Palette, length of a cycle in minutes, infinite loop or stop at the end?)               
                GradientPalette(neon_gp, gradientTimer, true);
                break;
              }   

    case 28  : {
                // Thunderstorm
                // GradientPaletteCylce(Color Palette, length of a cycle in minutes, infinite loop or stop at the end?)               
                GradientPalette(thunderstorm_gp, gradientTimer, true);
                break;
              }

    case 29  : {
                // Romance
                // GradientPaletteCylce(Color Palette, length of a cycle in minutes, infinite loop or stop at the end?)               
                GradientPalette(romance_gp, gradientTimer, true);
                break;
              }   

    case 30  : {
                // Halloween
                // GradientPaletteCylce(Color Palette, length of a cycle in minutes, infinite loop or stop at the end?)               
                GradientPalette(halloween_gp, gradientTimer, true);
                break;
              }   

    case 31  : {
                // Holiday
                // GradientPaletteCylce(Color Palette, length of a cycle in minutes, infinite loop or stop at the end?)               
                GradientPalette(holiday_gp, gradientTimer, true);
                break;
              }   
    case 32  : {
                // Meditation
                // GradientPaletteCylce(Color Palette, length of a cycle in minutes, infinite loop or stop at the end?)               
                GradientPalette(meditation_gp, gradientTimer, true);
                break;
              }

    case 33  : {
                // Outdoor
                // GradientPaletteCylce(Color Palette, length of a cycle in minutes, infinite loop or stop at the end?)               
                GradientPalette(outdoor_gp, gradientTimer, true);
                break;
              }   
    }
}

/*********

  LED scene Functions
  
*********/
void Poweroff(){
    FastLED.clear();
    FastLED.show();
}

void Poweron(long PoweronRGBcode, int PoweronBrightness){
    fill_solid( leds, NUM_LEDS, CRGB(PoweronRGBcode));
    FastLED.setBrightness(constrain(PoweronBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS));
    FastLED.show();
}

void OnePixel(){
        for(int dot = 0; dot < NUM_LEDS; dot++) { 
            leds[dot] = CRGB(RGBcode);
            FastLED.show();
            // clear this led for the next time around the loop
            leds[dot] = CRGB::Black;
            delay(30);
        }
}

void rainbow() 
{
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    // FastLED's built-in rainbow generator
    fill_rainbow( leds, NUM_LEDS, gHue, 7);
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FRAMES_PER_SECOND); 
    FastLED.show();  
}

void confetti() 
{
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy( leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV( gHue + random8(64), 200, 255);
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FRAMES_PER_SECOND); 
    FastLED.show(); 
}

void aurora(CRGBPalette16 palette, int SpeedDelay) 
{
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, gHue+(i*2));
  }
  // insert a delay to keep the framerate modest
  FastLED.delay(SpeedDelay); 
  FastLED.show(); 
}

void sinelon()
{
    EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy( leds, NUM_LEDS, 20);
    int pos = beatsin16( 13, 0, NUM_LEDS-1 );
    leds[pos] += CHSV( gHue, 255, 192);
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FRAMES_PER_SECOND); 
    FastLED.show(); 
}

void bpm()
{
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  FastLED.show(); 
}

void juggle() {
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  FastLED.show(); 
}

void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
    // insert a delay to keep the framerate modest
    FastLED.show();
    FastLED.delay(1000/FRAMES_PER_SECOND);
}

void plasma() {                                                 // This is the heart of this program. Sure is short. . . and fast.

  EVERY_N_MILLISECONDS(1000) {
    Serial.println(LEDS.getFPS());                            // Optional check of our fps.
  }

  EVERY_N_MILLISECONDS(100) {
    uint8_t maxChanges = 24; 
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
  }


  EVERY_N_SECONDS(5) {                                 // Change the target palette to a random one every 5 seconds.
    uint8_t baseC = random8();                         // You can use this as a baseline colour if you want similar hues in the next line.
    targetPalette = CRGBPalette16(CHSV(baseC+random8(32), 192, random8(128,255)), CHSV(baseC+random8(32), 255, random8(128,255)), CHSV(baseC+random8(32), 192, random8(128,255)), CHSV(baseC+random8(32), 255, random8(128,255)));
  }

  EVERY_N_MILLISECONDS(50) {                           // FastLED based non-blocking delay to update/display the sequence.
  int thisPhase = beatsin8(6,-64,64);                  // Setting phase change for a couple of waves.
  int thatPhase = beatsin8(7,-64,64);

  for (int k=0; k<NUM_LEDS; k++) {                     // For each of the LED's in the strand, set a brightness based on a wave as follows:

    int colorIndex = cubicwave8((k*23)+thisPhase)/2 + cos8((k*15)+thatPhase)/2;           // Create a wave and add a phase change and add another wave with its own phase change.. Hey, you can even change the frequencies if you wish.
    int thisBright = qsuba(colorIndex, beatsin8(7,0,96));                                 // qsub gives it a bit of 'black' dead space by setting sets a minimum value. If colorIndex < current value of beatsin8(), then bright = 0. Otherwise, bright = colorIndex..

    leds[k] = ColorFromPalette(currentPalette, colorIndex, thisBright, currentBlending);  // Let's now add the foreground colour.
  }
  }
  FastLED.show();
} // plasma()

void lightnings() {
  
  ledstart = random8(NUM_LEDS);                               // Determine starting location of flash
  ledlen = random8(NUM_LEDS-ledstart);                        // Determine length of flash (not to go beyond NUM_LEDS-1)
  
  for (int flashCounter = 0; flashCounter < random8(3,flashes); flashCounter++) {
    if(flashCounter == 0) dimmer = 5;                         // the brightness of the leader is scaled down by a factor of 5
    else dimmer = random8(1,3);                               // return strokes are brighter than the leader
    
    fill_solid(leds+ledstart,ledlen,CHSV(255, 0, 255/dimmer));
    FastLED.show();                       // Show a section of LED's
    delay(random8(4,10));                                     // each flash only lasts 4-10 milliseconds
    fill_solid(leds+ledstart,ledlen,CHSV(255,0,0));           // Clear the section of LED's
    FastLED.show();
    
    if (flashCounter == 0) delay (150);                       // longer delay until next flash after the leader
    
    delay(50+random8(100));                                   // shorter delay between strokes  
  } // for()
  
  delay(random8(frequency)*100);                              // delay between strikes
  
} // lightnings()

void SnowSparkle(long snowRGBcode, int SparkleDelay, int SpeedDelay) {
// Set a LED color (not yet visible)
  fill_solid(leds, NUM_LEDS, CRGB(snowRGBcode));
  
  int Pixel = random(NUM_LEDS);
  // Set all LEDs to a given color and apply it (visible)
     leds[Pixel] = CRGB(0xffffff);
  FastLED.show();
  delay(SparkleDelay);
  // Set all LEDs to a given color and apply it (visible)
     leds[Pixel] = CRGB(snowRGBcode);
  FastLED.show();
  delay(SpeedDelay);
}

void colorWipe(long WipeRGBcode, int SpeedDelay) {
  for(uint16_t dot=0; dot<NUM_LEDS; dot++) {
      leds[dot] = CRGB(WipeRGBcode);
      FastLED.show();
      delay(SpeedDelay);
  }
}

void meteorRain(long MeteorRGBcode, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  FastLED.clear();
  
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
    
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        leds[j].fadeToBlackBy( meteorTrailDecay );       
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
           leds[i-j] = CRGB(MeteorRGBcode);
      } 
    }
    FastLED.show();
    delay(SpeedDelay);
  }
}

void theaterChase(long theaterRGBcode, int SpeedDelay) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < NUM_LEDS; i=i+3) {
        leds[i+q] = CRGB(theaterRGBcode);   //turn every third pixel on
      }
      FastLED.show();
     
      delay(SpeedDelay);
     
      for (int i=0; i < NUM_LEDS; i=i+3) {
        leds[i+q] = CRGB(0x000000);        //turn every third pixel off
      }
    }
  }
}

void CylonBounce(long CylonRGBcode, int EyeSize, int SpeedDelay, int ReturnDelay){

    long r = CylonRGBcode >> 16;
    long g = CylonRGBcode >> 8 & 0xFF;
    long b = CylonRGBcode & 0xFF;
        /*
    valueR = value.substring(0,value.indexOf(',')).toInt();
    valueG = value.substring(value.indexOf(',')+1,value.lastIndexOf(',')).toInt();
    valueB = value.substring(value.lastIndexOf(',')+1).toInt();
    */


  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    FastLED.clear();
    leds[i] = CRGB(CylonRGBcode);
    for(int j = 1; j <= EyeSize; j++) {
      leds[i+j] = CRGB(CylonRGBcode); 
    }
    leds[i+EyeSize+1] = CRGB(r/10, g/10, b/10);
    FastLED.show();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    FastLED.clear();
    
    leds[i] = CRGB(r/10, g/10, b/10);
    for(int j = 1; j <= EyeSize; j++) {
      leds[i+j] = CRGB(CylonRGBcode);
    }
    leds[i+EyeSize+1] = CRGB(r/10, g/10, b/10);
    FastLED.show();
    delay(SpeedDelay);
  }
  
  delay(ReturnDelay);
}

void GradientPalette(CRGBPalette16 myPal, int Length, boolean Loop) {
  
  // how often (in seconds) should the heat color increase?
  // for the default of 30 minutes, this should be about every 7 seconds
  // 7 seconds x 256 gradient steps = 1,792 seconds = ~30 minutes

  float interval = ((float)(Length * 60) / 256)*1000;
  // current gradient palette color index
  static uint8_t heatIndex = 0; // start out at 0

  if ( ResetCycle != 0 ) {
    heatIndex = 0;
    ResetCycle = 0;
  }
  // To prevent gradientPalette loop stop color cycle at 240
  int heatIndexLimit = 255; 
  if (Loop == false) {
      heatIndexLimit = 240;
  }

  // call the gradient palette
  CRGB color = ColorFromPalette(myPal, heatIndex);
  
  // fill the entire strip with the current color
  fill_solid(leds, NUM_LEDS, color);
  
  // slowly increase the heat
  EVERY_N_MILLIS_I ( intervalCycle,100 ) {

    intervalCycle.setPeriod(interval);

    // stop incrementing at 255, we don't want to overflow back to 0 
    if (heatIndex < heatIndexLimit) {
      heatIndex++;
    }
    
    if (Loop == true && heatIndex >= 255) {
      heatIndex = 0;
    }
  } 
  FastLED.show();
}

void Strobe(long StrobeRGBcode, int StrobeCount, int FlashDelay, int EndPause){
  for(int j = 0; j < StrobeCount; j++) {
    fill_solid( leds, NUM_LEDS, CRGB(StrobeRGBcode));
    FastLED.show();
    delay(FlashDelay);
    FastLED.clear();
    FastLED.show();
    delay(FlashDelay);
  }
 
 delay(EndPause);
}

void Sparkle(long StrobeRGBcode, int SpeedDelay) {
  int Pixel = random(NUM_LEDS);
  leds[Pixel] = CRGB(StrobeRGBcode);
  FastLED.show();
  delay(SpeedDelay);
  FastLED.clear();
}

// Pick a random palette from a list
void selectRandomSunsetPalette() {
     
switch(random8(3)) {
        case 0:
        SunsetPalette = sunset_surf_gp;
        break;
        
        case 1:
        SunsetPalette = sunset_red_gp;
        break;
        
        case 2:
        SunsetPalette = sunset_blue_gp;
        break;
        
      }
     
}

// Pick a random palette from a list
void selectRandomSunrisePalette() {
     
switch(random8(3)) {
        case 0:
        SunrisePalette = sunrise_surf_gp;
        break;
        
        case 1:
        SunrisePalette = sunrise_surf_gp;
        break;
        
        case 2:
        SunrisePalette = sunrise_surf_gp;
        break;
        
      }
     
}
