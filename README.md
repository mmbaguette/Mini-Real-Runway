# Mini Real Runway
A working model of a runway made from printed paper and LED Strips that react to how close a real plane is approaching a runway in real time.

![Four colour printed papers stuck together representing a model of a runway with LED strips on both sides](https://github.com/user-attachments/assets/af18b858-2311-43b1-999f-3e4f68448d66)

An ESP8266 Arduino microcontroller fetches live aircraft data from Toronto Pearson Airport (YYZ) using [flight radar 24](https://www.flightradar24.com)'s API.

![Screenshot of flight radar 24 of an Air Canada plane approaching the runway](https://github.com/user-attachments/assets/5dcc5705-163d-491b-bce6-64f8eb750913)

Depending on how close the next closest aircraft is to landing, each pair of LED lights on both sides of the runway light up from to end, as demonstrated in the photo above. All lights turned on means that the aircraft is right above the runway, while only the first lit up means the aircraft is far away. An LCD display (not shown below) indicates the flight number, altitude, airline and aircraft model of the plane that's landing.

![Runway without LED lights on](https://github.com/user-attachments/assets/9aa8fe4d-e65f-471c-bdcc-72806cac7e97)

Each pair of LED lights on both sides is connected with wires flowing below the runway itself so that each pair lights up together.

![Wires connected below four pieces of paper](https://github.com/user-attachments/assets/99818a45-f750-412b-b9f1-e5a549ff7251)


## The Code
Here's a sample of [fetchplanedata.ino](fetchplanedata.ino), where you're expected to change some of these variables to match your preferences (airport code, minimum altitude for approach, etc.).
```cpp
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
```

This project was made in June 2023 and only published now, so the photos are a little old.
