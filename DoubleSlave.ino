// CenterAB - both motors
// xA->Freqequency1, xB->Frequency2
//CenterA at xB, CenterB at xA
//feels like "Slave"

#include <spi4teensy3.h>
#include <EEPROM.h>
#include <M3T3.h>

int duty, count, fout;
int xA, xB, foutA, foutB;

int BPM = 90;
int const numBeats = 10;
float myBeats[numBeats] = {1, 1, .45, .55, 1, 1, 1, .45, .55, 1};
int myBeatDirections[numBeats] = {-1, 1, -1, 1, 1, -1, 1, -1, 1, -1};
int myBeatPositions[numBeats] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
int myLastPosition = -1;
int torque = 330; // normal Torque Value


float millisForBeat = (60.f/BPM)*1000;
int nextBeatAt = 0;
int atBeat = 0;
int previousAtBeat = -1;
int millisReading = 0;

//float const FULL_STRENGTH = 1;
//float const LOW_STRENGTH = 0.66;
//float const MID_STRENGTH = 0.8;
//float myBeatStrength[numBeats] = {FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH};


void setup(){
  Serial.begin(9600);
  MotorA.init();
  analogReadAveraging(32);
  MotorA.torque(-300);
  delay(600);
  MotorA.torque(200);
  delay(900);
  MotorA.torque(0);
  delay(1500);
}

void loop(){

  millisReading = millis() - 3000;
  
  if(millisReading - nextBeatAt >= 0 && millisReading - nextBeatAt <= 20) {
    
    int distanceTraveled = previousAtBeat - analogRead(A1);
    int distanceExpected = myBeatPositions[atBeat > 0 ? atBeat - 1 : numBeats - 1] - myBeatPositions[atBeat];
    int signA = 0;
    if (distanceTraveled < 0) {signA = -1;}
    else if (distanceTraveled==0) {signA = 0;}
    else {signA = 1;}
    int signB = 0;
    if (distanceExpected < 0) {signB = -1;}
    else if (distanceExpected==0) {signB = 0;}
    else {signB = 1;}
 
    if(myBeats[atBeat] != 0) {
      if(myBeatPositions[atBeat] != -1 || abs(distanceTraveled) < 100 || abs(distanceTraveled) > 100 && signA != signB) {
        MotorA.start();
        MotorA.torque(torque * myBeatDirections[atBeat] - (myBeatDirections[atBeat] == 1 ? 20 : 0) /* * myBeatStrength[atBeat]*/);
      }
      
    }

    if(atBeat != previousAtBeat) {
      previousAtBeat = atBeat; // this is just to make sure we do certain things ONCE a bit later on
      if(myBeatPositions[atBeat] == -1) {
        myBeatPositions[atBeat] = analogRead(A1);
      }
      
      Serial.print(abs(distanceTraveled));
      Serial.print(" ");
      Serial.print(signA);
      Serial.print(" ");
      Serial.println(signB);        
    }
    
  } else {
    if(atBeat == previousAtBeat) {
      MotorA.stop();
      myLastPosition = analogRead(A1);
      nextBeatAt = millisReading + (int)(millisForBeat * myBeats[atBeat]) - 1;
      atBeat++;
      if(atBeat >= numBeats) {
        atBeat = 0;
        previousAtBeat = -1;
      }
    }
  }

  
  /*xA = analogRead(A1);
  Music.setFrequency1(map (xA, 0, 1023, 40, 2000)); 
  
  xB = analogRead(A9);
  Music.setFrequency2(map (xB, 0, 1023, 40, 2000)); 
  
  foutA = -6*(xA-xB); // this will peak at x=1024/6
  MotorA.torque(foutA);  // 1/4 or 1/2 ?

  foutB = -6*(xB-xA); // this will peak at x=1024/6
  MotorB.torque(foutB);  // 1/4 or 1/2 ?

  Music.setGain(float(abs(xA-xB))/1024.0f);*/
}
