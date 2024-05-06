Project created by Allen Li and Vivek Amarnani Fall 2023 for CSCE 462 at Texas A&M. 

The project is written in C++ code for Arduino. Uses "AccelStepper" library.

Functionalities include two motors to control 3 hands: one for hour and one for minutes and seconds. Minute and seconds hand was retrofitted to a gearbox from a normal wall clock.
Clock has regular clock mode which connects to a real-time clock module for accurate time keeping. When initiated into chronograph mode the hands will fly back to 00:00 waiting for user to 
start or stop the time. After exiting time-keeping mode, clock will automatically move to current time, making up for the gap. 

Parts include: 
1. Arduino UNO
2  - NEMA 17 Stepper Motors
2  - L298 Motor Drivers 
1 - Clock gear box, hands
1 - DS3231 Real Time Clock Module 
2 - 5V-6V power supply
2 - Male to female power supply adapter
1 - Infrared receiver + remote sender



