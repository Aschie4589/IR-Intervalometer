// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "HTML_variables.h"
#include <NikonRemote.h>

//   WIFI SETUP
const char* ssid     = "D3400-Intervalometer";
const char* password = "VerySecurePassword";


//   PIN DEFINITIONS
const int remote_shutter_pin = 14;

//   CREATE OBJECTS
// Create a Remote object to shoot with
NikonRemote remote(remote_shutter_pin); //start on pin TX
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// Create AsyncEventSource on /events. This is used to communicate an updated timing/progress status to the browser
AsyncEventSource events("/events");


// Other variables

// Store shooting settings from browser
float exposure = 15;
float delay_betw_shots = 3;
int number_of_shots = 20;

// Keep track of current shot number & timings
bool is_shooting = false;
int current_shooting_phase = 0; // 0 : exposure; 1 : delay
unsigned long int start_millis = 0;

float current_exposure = 0;
float current_delay = 0;
int current_shot = 0;

// Variables used for clock syncronization
long long int first_timestamp_browser = 0;
long long int second_timestamp_browser = 0;
long int first_timestamp = 0;
long int second_timestamp = 0;
double correction_constant = 1;

// Keep track of time intervals
unsigned long int prev_milliseconds = 0;
unsigned long int current_milliseconds = 0;

void syncronize_clock(){
  correction_constant = double(second_timestamp_browser-first_timestamp_browser)/double(second_timestamp-first_timestamp);
  Serial.print("Correction constant: ");
  Serial.println(correction_constant);
  return;
}

void update_browser_info(){
  // Very much self-explainatory, it sends an event with a JSON string containing the required info
  String baseJSON = String("{\"exposure\":\"EXP\",\"delay\":\"DEL\",\"shot\":\"SHOT\"}");
  baseJSON.replace(String("EXP"), String(current_exposure));
  baseJSON.replace(String("DEL"), String(current_delay));
  baseJSON.replace(String("SHOT"), String(current_shot));
  Serial.println(baseJSON);
  events.send(baseJSON.c_str(), "update-progress", millis());
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  Serial.print("Setting AP (Access Point)…");

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // HOMEPAGE
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  /// SERVE HOMEPAGE RESOURCES (js, css....)
  server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/css", bootstrapmincss);
  });
  server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/javascript", bootstrapminjs);
  });
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/javascript", jqueryjs);
  });
  server.on("/bootstrap-input-spinner.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/javascript", bootstrapinputspinner);
  });


  // RESPOND TO COMMANDS

  // Single shot
  server.on("/shoot", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200);
    remote.click();
  });
  // Start button pressed
  server.on("/start-shooting", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Pressed start!");
    request->send(200);

    AsyncWebHeader* h_exp = request->getHeader("exposure");
    AsyncWebHeader* h_del = request->getHeader("delay");
    AsyncWebHeader* h_num = request->getHeader("number-shots");

    exposure = h_exp->value().toFloat();
    delay_betw_shots = h_del->value().toFloat();
    number_of_shots = h_num->value().toInt();

    Serial.print("Number of shots: ");
    Serial.println(number_of_shots);
    Serial.print("Exposure: ");
    Serial.println(exposure);
    Serial.print("Delay between shots: ");
    Serial.println(delay_betw_shots);

    current_exposure = 0;
    current_delay = 0;
    current_shot = 0;
    current_shooting_phase = 0;
    is_shooting = true;

    update_browser_info();

    start_millis = millis();
    remote.click();

  });
  // Stop button pressed
  server.on("/stop-shooting", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200);
    Serial.println("Aborting");
    if (is_shooting && current_shooting_phase == 0){
      remote.click();
    }
    is_shooting = false;

  });

  server.on("/sync-timing", HTTP_POST, [](AsyncWebServerRequest *request){
    request -> send(200);

    //Obtain time-stamp sent as header
    if(request->hasHeader("Initial")){
      first_timestamp = millis();
      AsyncWebHeader* h = request->getHeader("Sync-date");
      first_timestamp_browser = atoll(h -> value().c_str());

    }
    if(request->hasHeader("Final")){
      second_timestamp = millis();
      AsyncWebHeader* h = request->getHeader("Sync-date");
      second_timestamp_browser = atoll(h -> value().c_str());

      syncronize_clock();
    }
  });

  // Handle client request for current status
  server.on("/status", HTTP_POST, [](AsyncWebServerRequest *request){
    request->send(200);
    update_browser_info();
  });

  events.onConnect([](AsyncEventSourceClient *client){
    //send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!",NULL,millis(),1000);
  });
  // Make sure the server is sending messages on "./events"
  server.addHandler(&events);
  // Start server
  server.begin();

}

void loop(){
  if (is_shooting) {
    if (current_shooting_phase == 0){
      // camera is currently shooting, wait for the right time and then close the shutter.
      // The check below ensures that in the last 800 milliseconds before shooting, all the arduino does is wait. This gives more precise timing.
      if (start_millis + 1000*exposure*correction_constant - 800 < millis()){
        // Time to close the shutter. Time it so all works fine.
        // Very naive first guess.
        delay(start_millis+1000*exposure*correction_constant-millis());
        remote.click();

        //update the relevant variables so that we jump to the right phase next iteration
        current_shooting_phase = 1;
        current_exposure = exposure;
        current_delay = 0;

        update_browser_info();

      }
    } else if (current_shooting_phase == 1){
      if (start_millis + 1000*exposure*correction_constant + 1000*delay_betw_shots*correction_constant - 800 < millis()){
        delay(start_millis + 1000*exposure*correction_constant + 1000*delay_betw_shots*correction_constant-millis());

        current_shooting_phase = 0;
        current_exposure = 0;
        current_shot = current_shot + 1;

        if (current_shot < number_of_shots){
          remote.click();
        }
        //restart the delay counter so tht we are ready for next iteration
        start_millis = millis();

      }
    }
  }
  if (is_shooting && current_shot >= number_of_shots){
    is_shooting = false;
    // Tell the browser that we're done!
    events.send("", "shooting-done", millis());
  }
}
