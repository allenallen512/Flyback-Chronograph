#include <AccelStepper.h>
#include <RTClib.h>


// Initialize the stepper libraries for two motors on different sets of pins:
AccelStepper stepper(AccelStepper::FULL4WIRE, 8, 9, 10, 11);
AccelStepper stepper2(AccelStepper::FULL4WIRE, 4, 5, 6, 7);
RTC_DS3231 rtc;


#define stepsPerRevolution 200
unsigned long currentMinPos = 0;
enum ClockMode { TIME_KEEPING_MODE,
                 RESTING_CHRONOGRAPH_MODE,
                 ACTIVE_CHRONOGRAPH_MODE };
ClockMode currentMode;
int minsPos[60];
int positions[60];
int hourPos[12] = { 0, 267, 533, 800, 1067, 1333, 1600, 1867, 2133, 2400, 2667, 2933 };  //1 hour to 12 hours
// int hourPos[12] = {257, 514, 790, 1057, 1324, 1590, 1857, 2124, 2390, 2657, 2924, 3190 };  //1 hour to 12 hours
int minute = 0;
int hour = 0;
long startTime = 0;
long currentTime = 0;
int currSecond = 0;
int moveInterval = 1000;
int mode = 0;
bool resetCLK;
int totalsteps;


void stepperIdle() {
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
}
void stepper2Idle() {
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
}
void setupPositions() {
  int stepCount = 0;  // Counter to keep track of steps


  // sets the positions for each spoke on the clock in the form:
  // (currentSecond, corresponding position out of 200)
  // example: positions == {3, 6, 10, 13, 16, 20, ... 200}
  for (int i = 1; i < 60; i++) {
    if (stepCount < 2) {
      positions[i] = 3 + positions[i - 1];
    } else {
      positions[i] = 4 + positions[i - 1];
    }
    stepCount = (stepCount + 1) % 3;
  }
}
void setMinutePos() {
  int count = 1;
  minsPos[0] = 0;


  for (int i = 1; i < 60; ++i) {
    if (count % 2 == 0) {
      minsPos[i] = minsPos[i - 1] + 4;
    } else if (i == 15 || i == 31 || i == 45) {
      minsPos[i] = minsPos[i - 1] + 4;
    } else {
      minsPos[i] = minsPos[i - 1] + 5;
    }
    count += 1;
  }
}


void moveToTime() {
  Serial.println("in the move to time function");
  DateTime now = rtc.now();
  minute = now.minute();
  currSecond = now.second();
  hour = now.hour();
  hour = hour % 12;
  Serial.print("in the move to time function, the current time is: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
  // Calculate total steps to position minute hand
  totalsteps = minsPos[minute] + hourPos[hour];
  Serial.print("we will move: ");
  Serial.println(totalsteps);


  // Move the stepper motor to the current minute position
  stepper.moveTo(totalsteps);


  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
  stepper.setCurrentPosition(totalsteps);  //???
}




void moveSeconds() {
  Serial.print("in the seconds func the current second is: ");
  currentTime = millis();  //returns the nunmber of milliseconds passed since the arduino started the current program
  Serial.println(currSecond);
  stepper2.moveTo(positions[currSecond]);
  // stepper2.runSpeed();
  while (stepper2.distanceToGo() != 0) {
    Serial.println("in the first while loop");
    // Serial.println("inside ");
    stepper2.run();  // Continue running the motor until movement is completed
  }
  DateTime now = rtc.now();
  currSecond = now.second();
  startTime = currentTime;
  currSecond = (currSecond + 1) % 60;  // increments the number of seconds & keeping it within the 0-60 range
  if (currSecond == 0) {
    delay(1000);
    stepper2.moveTo(200);
    // stepper2.runSpeed();
    while (stepper2.distanceToGo() != 0) {
      Serial.println("stuck in the second");
      stepper2.run();  // Continue running the motor until movement is completed
    }
    startTime = currentTime;
    stepper2.setCurrentPosition(0);
  }
}


void setup() {
  Serial.begin(9600);
  // Initialize stepper
  stepper.setMaxSpeed(250);
  stepper.setAcceleration(250);
  stepper.setSpeed(500);
  stepper.setCurrentPosition(0);


  stepper2.setMaxSpeed(500);
  stepper2.setAcceleration(500);
  stepper2.setSpeed(1500);
  stepper2.setCurrentPosition(0);
  positions[0] = 0;


  //initializing the movement arrays
  setupPositions();
  setMinutePos();


  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }


  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // Read current time from RTC
  DateTime now = rtc.now();
  minute = now.minute();
  currSecond = now.second();
  hour = hour % 12;


  // Calculate total steps to position minute hand
  int totalsteps = minsPos[minute] + hourPos[hour];


  moveToTime();


  Serial.print("Current time set to: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.println(minute);
  stepperIdle();


  // stepper.moveTo(totalsteps + 100);
  // while (stepper.distanceToGo() != 0) {
  //   stepper.run();
  // }
  // stepper.setCurrentPosition(minsPos[minute]);


  // Serial.println("just mvoed to an hour? ");


  // sets the current second to the second read by the RTC
  // stepper2.moveTo(positions[currSecond]);  // moves the hand to start at this time
  // stepper2.run();                          // moves to the current second as fast as possible
  stepper2Idle();
}


void loop() {
  currentTime = millis();


  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');


    if (input.equals("mode1")) {
      currentMode = TIME_KEEPING_MODE;
      DateTime now = rtc.now();
      currSecond = now.second();
    } else if (input.equals("mode2")) {
      currentMode = RESTING_CHRONOGRAPH_MODE;
      Serial.println("mode 2 was entered");
    } else if (input.equals("mode3")) {
      currentMode = ACTIVE_CHRONOGRAPH_MODE;
      currSecond = 0;
      minute = 0;
      hour = 0;
      Serial.println("mode 3 was entered");
      stepper.setCurrentPosition(0);
      stepper2.setCurrentPosition(0);
    }
  }


  if (currentMode == TIME_KEEPING_MODE) {
    DateTime modetime = rtc.now();
    currSecond = modetime.second();
    minute = modetime.minute();
    hour = modetime.hour();
    hour = hour % 12;


    if (currentTime - startTime >= moveInterval) {
      Serial.print("the seconds we're: ");
      Serial.print(currSecond);
      Serial.print("||");
      Serial.print("the minute we're at: ");
      Serial.println(minute);


      // moves to the next position at the current run speed
      stepper2.moveTo(positions[currSecond]);
      while (stepper2.distanceToGo() != 0) {
        // Serial.println("inside the first loop");
        stepper2.run();  // Continue running the motor until movement is completed
      }


      int target_dist = minsPos[minute] + hourPos[hour];


      stepper.moveTo(target_dist);
      while (stepper.distanceToGo() != 0) {
        Serial.println("the minute hand moved here");
        stepper.run();
      }


      // Update the current time from RTC after every movement
      DateTime now = rtc.now();
      currSecond = now.second();
      startTime = currentTime;


      // currSecond = (currSecond + 1) % 60;  // increments the number of seconds & keeping it within the 0-60 range


      if (currSecond == 59) {
        Serial.println("seconds is now 0");
        minute = (minute + 1) % 60;
        Serial.print("new minute: ");
        Serial.println(minute);


        if (minute != 0) {
          currentMinPos = minsPos[minute] + hourPos[hour];
          Serial.print("the hour is: ");
          Serial.println(hour);
          Serial.print("the new position:");
          Serial.println(currentMinPos);


          stepper.moveTo(currentMinPos);
          while (stepper.distanceToGo() != 0) {
            stepper.run();
          }
          stepper.setCurrentPosition(currentMinPos);
        } else {
          stepper.moveTo(267);
          while (stepper.distanceToGo() != 0) {
            stepper.run();
          }


          hour = (hour + 1) % 12;
          if (hour == 0) {
            hour = 12;
          }
          stepper.setCurrentPosition(0);
        }
        Serial.println("incremenet the minute");


        delay(1000);
        stepper2.move(3);
        // stepper2.runSpeed();


        while (stepper2.distanceToGo() != 0) {
          // Serial.println("inside the second");
          stepper2.run();  // Continue running the motor until movement is completed
        }
        startTime = currentTime;
        stepper2.setCurrentPosition(0);
      }
      stepperIdle();
    }
    stepper2Idle();
  } else if (currentMode == RESTING_CHRONOGRAPH_MODE) {
    int total = minsPos[minute] + hourPos[hour];
    Serial.println("in the second mode -- resting mode");
    stepper.stop();
    stepper2.stop();
    Serial.print("the second it stopped at is: ");
    Serial.print(currSecond);
    Serial.print("||");
    Serial.print("the seconds it has to move back: ");
    Serial.println(currSecond);
    stepper2.moveTo(0);
    while (stepper2.distanceToGo() != 0) {
      stepper2.run();
    }
    stepper2Idle();


    Serial.print("the total number of steps we moved is: ");
    Serial.println(currentMinPos);


    stepper.moveTo(0);
    while (stepper.distanceToGo() != 0) {
      stepper.run();
    }


    stepper2.setCurrentPosition(0);
    stepper.setCurrentPosition(0);


    // if (totalsteps > 1600) {  //spin clock wise since it is more than halfway
    //   spin_distance = 3200 - totalsteps;
    //   stepper.moveTo(spin_distance);
    //   while (stepper.distanceToGo() != 0) {
    //     stepper.run();
    //   }
    // } else {
    //   spin_distance = -totalsteps;
    //   stepper.move(spin_distance);
    //   while (stepper.distanceToGo() != 0) {
    //     stepper.run();
    //   }
    // }
    stepperIdle();
    stepper2Idle();


  } else if (currentMode == ACTIVE_CHRONOGRAPH_MODE) {
    hour = hour % 12;


    if (currentTime - startTime >= moveInterval) {
      Serial.print("the seconds im  at: ");
      Serial.print(currSecond);
      Serial.print( "|| ");
      Serial.print("the minute: ");
      Serial.print(minute);
      Serial.print(" || ");
      Serial.print("hour: ");
      Serial.println(hour);


      // moves to the next position at the current run speed
      stepper2.moveTo(positions[currSecond] );
      // stepper2.move(4);
      Serial.print("the seconds position is: ");
      Serial.println(positions[currSecond]);
      // stepper2.runSpeed();
      while (stepper2.distanceToGo() != 0) {
        // Serial.println("inside the first loop");
        stepper2.run();  // Continue running the motor until movement is completed
      }


      int target_dist = minsPos[minute] + hourPos[hour];
      Serial.print("the minute and hour hands are at position: ");
      Serial.println(target_dist);


      stepper.moveTo(target_dist);
      while (stepper.distanceToGo() != 0) {
        stepper.run();
      }


      startTime = currentTime;
      currSecond = (currSecond + 1) % 60;  // increments the number of seconds & keeping it within the 0-60 range


      if (currSecond == 59) {
        Serial.println("seconds is now 0");
        minute = (minute + 1) % 60;
        Serial.print("new minute");
        Serial.println(minute);


        if (minute != 0) {
          Serial.println("the minute isnt zero, update min and move on");
          currentMinPos = minsPos[minute] + hourPos[hour];
          Serial.println(currentMinPos);
          stepper.moveTo(currentMinPos);
          while (stepper.distanceToGo() != 0) {
            stepper.run();
          }
          stepper.setCurrentPosition(currentMinPos);
        } else {
          Serial.println("the minute is zero, move a little then update min and move");
          stepper.move(3);
          while (stepper.distanceToGo() != 0) {
            stepper.run();
          }
          hour = (hour + 1) % 12;
          if (hour == 0) {
            hour = 12;
          }
          stepper.setCurrentPosition(minsPos[minute] + hourPos[hour]);
        }
        Serial.println("incremenet the minute");
        delay(1000);
        stepper2.move(3);
        // stepper2.runSpeed();


        while (stepper2.distanceToGo() != 0) {
          stepper2.run();  // Continue running the motor until movement is completed
        }
        startTime = currentTime;
        stepper2.setCurrentPosition(0);
      }
      stepperIdle();
      // stepper2Idle();
    }
    // stepper2Idle();
  }
}



