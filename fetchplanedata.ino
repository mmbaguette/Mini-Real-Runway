/**
   BasicHTTPSClient.ino

    Created on: 20.08.2018

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecureBearSSL.h>

#define WIFI_SSID "WIFI-SSID-HERE" //       HIDE THESE IF 
#define WIFI_PASSWORD "WIFI-PASSWORD-HERE" // PUBLISHING ONLINE

//TODO: Sometimes for some reason it doesn't show any values like callsign when it says Attention!; ex: Attention!   departing from  at 1050 feet is approaching!


//You will need an online flight tracker to adjust some of these values. track a real plane approaching the runway or use a map to see where it has what values (position, altitude)
String airportCode = "YYZ"; //Airport code of the airport the planes will be arriving at
const float detectionBoxSize = 0.3; //controls the size of the bounding box around the airport that will pick up incoming aircraft (longitude/latitude units); use google maps and right click where you want to see longitude and latitude of that point on the map
const int landingHeight = 700; //the altitude when the aircraft is considered "landed" or over the runway; the LED runway lights will all turn off when the plane is focus reaches this height or lower, so make this a little smaller (~100 less) if you want the last lights to stay on for a few more seconds after the plane has landed; or just increase the last altitude in runwayHeights by that same amount
int minimumAlt = 750; //minimum altitude for an aircraft in order to be considered for arrival
const bool debug = false; //enable debug mode (more verbose print statements)
const int[LED_PAIRS] runwayHeights = [ //the height in feet of when each LED pair should start to light up when the approaching aircraft reaches that height, from farthest to closest. make sure you have just as many values as you have in LED_PAIRS; this might be obvious but don't but 10 different heights if you only have three pairs of LED lights. It has to be the same number of LED lights that you have set up. ADD A COMMA AFTER EACH NUMBER 
  3000,
  2000,
  1300,
  800, // this last height is when all the runway lights are finally lit; it will stay on until the plane reaches the landingHeight altitude, so change this depending on how long you want to keep the runway lights on after landing
]; // all runway turn off when the plane reaches landingHeight

//DO NOT EDIT UNLESS YOU WANT TO CHANGE THE PROGRAM
const uint8_t fingerprint[20] = {0x0e, 0x20, 0x99, 0x08, 0xC2, 0xF8, 0xEB, 0xC5, 0x53, 0xB7, 0x08, 0x1F, 0xEE, 0xFA, 0x00, 0xAC, 0x95, 0xEA, 0x1A, 0x1B};
//TODO: Watch a video on cpp functions to organize this thing into functions!
const char nearbyPlanesURLbase[] = "https://data-cloud.flightradar24.com/zones/fcgi/feed.js?faa=1&bounds=";
String airportDataURL = String("https://api.flightradar24.com/common/v1/airport.json?plugin[]=details&code=" + String(airportCode));
const char regexFilter[] = "(\\[(.*?)\\]\\,)|(\\[(.*?)\\])";
String nearbyPlanesURL = ""; //this will be filled in later 
const int commasInPlaneList = 18; //number of commas in a plane list
const int runwayHeightsSize = sizeof(runwayHeights)/sizeof(runwayHeights[0]); //number of heights in runwayHeights

ESP8266WiFiMulti WiFiMulti;

void setup() {
  airportCode.toUpperCase(); //raise airport code to capital letters for string comparison
  Serial.begin(115200);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  Serial.println("Wait 5s...");
  delay(5000);

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    // client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    
    if (nearbyPlanesURL == "") { //if we still haven't fetched airport coordinates; after the first loop this should be done if connection was successful
      if (https.begin(*client, airportDataURL)) {  // Fetch airport data (lat and long of airport)
        Serial.printf("[HTTPS] GET %s...\n", airportDataURL.c_str());
        // start connection and send HTTP header
        int httpCode = https.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            Serial.println("Processing payload...");

            if (https.getString()=="") {
              Serial.println("Payload is empty!");
              return;
            }

            StaticJsonDocument<96> filter;
            filter["result"]["response"]["airport"]["pluginData"]["details"]["position"] = true;
            //deserialize online JSON file
            StaticJsonDocument<512> doc;
            DeserializationError error = deserializeJson(doc, https.getString(), DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(14));

            if (error) { //error handling for json deserialization library
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.f_str());
              Serial.printf("Payload (len: %d):\n", https.getString().length());
              Serial.println(https.getString());
              return;
            }

            JsonObject airportPosition = doc["result"]["response"]["airport"]["pluginData"]["details"]["position"];
            float airportLatitude = airportPosition["latitude"]; //retrieve centre location of airport
            float airportLongitude = airportPosition["longitude"];

            float y1 = airportLatitude + detectionBoxSize; //create bounding box around the centre of the airport
            float y2 = airportLatitude - detectionBoxSize;
            float x1 = airportLongitude - detectionBoxSize;
            float x2 = airportLongitude + detectionBoxSize;
            
            char nearbyPlaneURLcoords[24];
            sprintf(nearbyPlaneURLcoords, "%2.1f,%2.1f,%2.1f,%2.1f", y1,y2,x1,x2);
            nearbyPlanesURL = String(nearbyPlanesURLbase + String(nearbyPlaneURLcoords)); //TODO: the x's and y's are the same!
            
            Serial.printf("Airport %s is located at %2.1f, %2.1f. Bounding box (lat/Y, lon/X) top left: %2.1f, %2.1f, " 
              "bottom right: %2.1f, %2.1f\n", airportCode, airportLatitude, airportLongitude, y1,x1,y2,x2);
          }
        } else {
          Serial.printf("[HTTPS] GET airportDataURL... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }

        https.end();
      } else {
        Serial.printf("[HTTPS] Unable to connect\n");
        return;
      }
    }

    if (nearbyPlanesURL != "" && https.begin(*client, nearbyPlanesURL)) {  // if we successfully retrieved
      Serial.printf("[HTTPS] GET %s...\n", nearbyPlanesURL.c_str());
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been sent and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          
          if (payload.length()==0) {
            Serial.println("Payload is empty!");
            return;
          }
          
          int altitude = 0; //initialize flight data of the approaching (closest but not too close, and with a low altitude) aircraft
          String model = "";
          String departureAirport = "";
          String callsign = "";

          int index = 0;
          Serial.println("Processing flight data...");

          while (true) { //first calculate size need to create char* array
            int res1 = payload.indexOf('[', index); //result returns -1 if none, otherwise index of first char

            if (res1 >= 0) {
              index = res1+1; //change the index variable twice (here and after res2), just in case there's no 
                              //closing bracket for some reason (hackers returning bad data)
              int res2 = payload.indexOf(']', index); //find the end of this list [ ]

              if (res2 > -1) {
                String planeList = payload.substring(res1,res2); //MAKE SURE THIS <<DOES NOT>> INCLUDE THE CLOSING BRACKET
                planeList.replace("[","");
                planeList.replace("]",""); //remove open and closing brackets
                
                if (debug) {
                  Serial.print("\n\nProcessing planeList: ");
                  Serial.println(planeList);
                }

                int commasInThisList = -1; //number of commas in planeList string for while loop to keep track of (starts at 0 like list indices)
                int nextCommaIndex = 0; //where this value starts (including the comma) relative to planeList

                String thisArrivalAir = ""; //TODO: in the future replace all these String types with char buffer datatypes because actually the JSON values received are always the same length; arrival airport
                String thisDepAir = ""; //data of this particular plane we're searching; departure airport
                String thisCallsign = "";
                String thisModel = "";
                int thisAltitude = 0;
                
                while (true) { //make sure this list has 18 commas like every list containing plane info should have                  
                  if (nextCommaIndex > -1) { //if comma found (indexOf() returns int greater than -1)
                    commasInThisList++; //we found another comma
                    int nextNextComma = planeList.indexOf(',', nextCommaIndex+1); //the comma after nextComma
                    int valueEndIndex = (nextNextComma>0) ? nextNextComma : planeList.length(); //where this value ends relative to planeList (next comma if there is one, or the end of this list)
                    String value = planeList.substring(nextCommaIndex+1, valueEndIndex); //get the value between the two commas or between last comma and end of string
                    
                    if (debug)
                      Serial.printf("%d: %s, ", commasInThisList, value); //display that value in serial monitor with its index
                    
                    value.replace("\"", ""); //remove quotation marks "" around string JSON values. DO NOT CHANGE this line or it will break the program

                    switch (commasInThisList) { //assign values from each list index we're interested in (ex: index/case 8 is aircraft model)
                      case 4: //altitude
                        thisAltitude = value.toInt(); //toInt() returns the long/int of the numerical string, or 0 if it's not a number
                        break; //end case statement
                      case 8: //aircraft model
                        thisModel = value;
                        break;
                      case 11: //departure airport
                        thisDepAir = value;
                        break;
                      case 12: //arrival airport
                        thisArrivalAir = value;
                        break;
                      case 16: //callsign
                        thisCallsign = value;
                        break;
                    } //end of switch case
                    nextCommaIndex = planeList.indexOf(',', nextCommaIndex+1); //find the next comma after this
                  } else { //no more commas in this list
                    break;
                  }
                } //end of while loop that loops through individual plane list
                if (commasInThisList!=commasInPlaneList) {
                  if (debug) {
                    Serial.print("\nNot a plane list: ");
                    Serial.print(planeList);
                  }
                  continue;
                }

                if ((thisArrivalAir == airportCode) // if this plane is even arriving at our airport
                    && (altitude == 0 || altitude > thisAltitude) //if altitude has not been set yet or the last plane landed, or this plane's altitude is smaller than the last closest plane we looked at
                    && (thisAltitude > minimumAlt) // if this aircraft is close enough to be considered
                    && (thisAltitude > landingHeight)) {  //if this aircraft has not landed yet
                  altitude = thisAltitude; //override the previous variables of the last plane who's further than this new plane we now want
                  model = thisModel;
                  departureAirport = thisDepAir;
                  callsign = thisCallsign;
                  Serial.printf("%s %s departed from %s, is at %d feet from the ground\n", model, callsign, departureAirport, altitude);
                }
              } else //theres no more closing brackets
                break;
            } else //there no more opening brackets
              break;
          } //end of while loop that loops through each aircraft

          if (altitude == 0) { //the for loop did not find any reasonable approaching aircraft
            Serial.println("\nNo approaching aircraft found!");
            Serial.println("Payload:");
            //Serial.println(payload);
          } else {
            Serial.printf("Attention! %s %s departed from %s at %d feet is approaching!\n", model, callsign, departureAirport, altitude);
          }

          /*TODO: The lights should light up in terms of how close the aircraft is to landing (how close in altitude). 
          does not need to start at first sequence of lights. ex. if second pair of lights are on that means the aircraft is 1000-1200 ft from ground 
          */

          int LEDsUp = 1; //number of LEDs lit up; at least 1
          for (int i=1; i<runwayHeightsSize+1; i++) { //check which heights the altitude variable is smaller than to find how many LEDs should light up
            if (altitude < runwayHeights[i])
              LEDsUp = i;
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}
