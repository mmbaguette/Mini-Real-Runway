# Mini Real Runway
A working model of a runway made from printed paper and LED Strips that react to how close a real plane is approaching a runway in real time.

![Four colour printed papers stuck together representing a model of a runway with LED strips on both sides](https://github.com/user-attachments/assets/af18b858-2311-43b1-999f-3e4f68448d66)

An ESP8266 Arduino controller fetches live aircraft data off of any given airport from [flight radar 24](https://www.flightradar24.com)'s API, in my case Toronto Pearson International Airport (YYZ).

![Screenshot of flight radar 24 of an Air Canada plane approaching the runway](https://github.com/user-attachments/assets/5dcc5705-163d-491b-bce6-64f8eb750913)

Depending on how close the next closest aircraft is to landing, each pair of LED lights on both sides of the runway light up from to end, as demonstrated in the photo above. All lights turned on means that the aircraft is right above the runway, while only the first pair turned on means the aircraft is far away. A 4-line LCD display (not shown below) indicates the flight number, alitude, airline and aircraft model of the plane that's landing.

![Runway without LED lights on](https://github.com/user-attachments/assets/9aa8fe4d-e65f-471c-bdcc-72806cac7e97)

Each pair of LED lights on both sides is connected with wires flowing below the runway itself so that each pair lights up together.

![Wires connected below four pieces of paper](https://github.com/user-attachments/assets/99818a45-f750-412b-b9f1-e5a549ff7251)

This project was made in June 2023 and only published now, so the photos are a little old.
