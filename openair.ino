#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

const char* ssid     = "";           // insert your SSID
const char* password = "";   // insert your password

WiFiClient client;

const char* server = "opendata2.epa.gov.tw";  // server's address
const char* resource = "/AQX.json";

char response[22000], parse_res[500]; // this fixed sized buffers works well for this project using the NodeMCU.

void setup(){
    delay(3000);
    // initialize serial
    Serial.begin(250000);

    // initialize WiFi
    WiFi.begin(ssid, password);

    //Connection WiFi
    Serial.println("");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    Serial.println("Wi-Fi is Connected!!!");
}

void loop(){

        // connect to server
        bool ok = client.connect(server, 80);
        while(!ok){
            Serial.print(".");
            delay(500);
        }
        Serial.println("");
        Serial.println("Client is Connected!!!");

        //Send request to resource
        client.print("GET ");
        client.print(resource);
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(server);
        client.println("Connection: close");
        client.println();

        delay(100);

        //Reading stream and remove headers
        client.setTimeout(5000);
        client.readBytes(response, 5000);

        // process JSON
        DynamicJsonBuffer jsonBuffer;
        int eol = sizeof(response);
        int bgni = 0;
        int endi = 0;
        int cnti = 0;
        bool siteFound = false;
        bool noSuchSite = false;
        char* location, site, publishTime, pm25, pm10, psi;
        char* siteName = "萬華";

        while(!siteFound) {
          bool jsonBeginFound = false;
          bool jsonEndFound = false;
          while((!jsonBeginFound || !jsonEndFound) && !noSuchSite){
              cnti++;
              if(int(response[cnti]) == 93 || cnti == eol) {
                noSuchSite = true;
              } else if(int(response[cnti]) == 123) { // check for the "{"
                jsonBeginFound = true;
                bgni = cnti;
              } else if(int(response[cnti]) == 125) { // check for the "}"
                jsonEndFound = true;
                endi = cnti;
              }
          }

          if (!noSuchSite) {
            //restructure by shifting the correct data
  //          Serial.println("restructure");
            for(int c=0; c<(endi-bgni+1); c++){
                parse_res[c] = response[((c+bgni))];
  //            Serial.print(parse_res[c]);
            }

            JsonObject& root = jsonBuffer.parseObject(parse_res);

  //          if (!root.success()) {
  //            Serial.println("JSON parsing failed!");
  //          }
  //          else {
  //            Serial.println("JSON parsing worked!");
  //          }

            const char* site = root["SiteName"];
            if(strcmp(site, siteName) == 0) {
              siteFound = true;
              const char* location = root["County"];
              const char* publishTime = root["PublishTime"];
              const char* pm25 = root["PM2.5"];
              const char* pm10 = root["PM10"];
              const char* psi = root["PSI"];

              // Print data to Serial
              Serial.print(location);
              Serial.print(" ");
              Serial.print(site);
              Serial.print("區 ");
              Serial.println(publishTime);
              Serial.print("PM2.5: ");
              Serial.println(pm25);
              Serial.print("PM10: ");
              Serial.println(pm10);
              Serial.print("PSI: ");
              Serial.println(psi);
              Serial.println("----------");
            }
        } else {
            siteFound = true;
            Serial.print("找不到");
            Serial.print(siteName);
            Serial.println("區的資料");
          }
        }

        client.stop(); // disconnect from server

        for(int x=0; x<10; x++){ // wait for new connection with progress indicator
            Serial.print(".");
            delay(1000); // the OWM free plan API does NOT allow more then 60 calls per minute
        }

        Serial.println("");

    }
