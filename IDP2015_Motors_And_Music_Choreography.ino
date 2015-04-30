#include <spi4teensy3.h>
#include <EEPROM.h>
#include <M3T3.h>

// Config
int const BPM = 90;
int const numBeats = 10;
int const toruqeDuration = 20;
float const myBeats[numBeats] = {1, 1, .45, .55, 1, 1, 1, .45, .55, 1};
int const myBeatDirections[numBeats] = {-1, 1, -1, 1, 1, -1, 1, -1, 1, -1};
int const torque = 330; // normal Torque Value
int const choreographyCountGoal = 1;
float const torqueResponseWhileDancing = -.65;
const int connectionMode = 1; // -1 or 1

// Variables
int xA, xB;
float millisForBeat = (60.f/BPM)*1000;
int nextBeatAt = 0;
int atBeat = 0;
int previousAtBeat = -1;
int millisReading = 0;
int choreographyCount = 0; // how many times we "choreographed"

// for our new audio accents
int xA_old, xB_old; 
boolean stepping = false;
int stepThreshold = 75;
int stepA_dist, stepB_dist;
long prevTime;
// end audio additions

//int myBeatPositions[numBeats] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
//int myLastPosition = -1;
//float const FULL_STRENGTH = 1;
//float const LOW_STRENGTH = 0.66;
//float const MID_STRENGTH = 0.8;
//float myBeatStrength[numBeats] = {FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH, FULL_STRENGTH};


void setup(){
  Serial.begin(9600);
  MotorA.init();
  MotorB.init();
  Music.init();
  Music.setGain(0);
  prevTime = millis();
  
  analogReadAveraging(32);
  resetPositions();
} // end of 


void resetPositions() {
  // Put Motors to Places
  MotorA.start();
  MotorB.start();
  MotorA.torque(-300);
  MotorB.torque(-300);
  delay(600);
  MotorA.torque(200);
  MotorB.torque(233); // seems to need more torque to move
  delay(900);
  MotorA.torque(0);
  MotorB.torque(0);
  delay(2500);
} // end of resetPositions()


void loop(){

  millisReading = millis() - 4000; // first millisecond seems to be "lost" - nothing happens during it. Three more we get from the "setup" method. Hence 4000 subtraction.
  
  if(choreographyCount != -1 && choreographyCount < choreographyCountGoal && millisReading - nextBeatAt >= 0 && millisReading - nextBeatAt <= toruqeDuration) {
    
    /*int distanceTraveled = previousAtBeat - analogRead(A1);
    int distanceExpected = myBeatPositions[atBeat > 0 ? atBeat - 1 : numBeats - 1] - myBeatPositions[atBeat];
    int signA = 0;
    if (distanceTraveled < 0) {signA = -1;}
    else if (distanceTraveled==0) {signA = 0;}
    else {signA = 1;}
    int signB = 0;
    if (distanceExpected < 0) {signB = -1;}
    else if (distanceExpected==0) {signB = 0;}
    else {signB = 1;}*/
 
    if(myBeats[atBeat] != 0) {
      //if(myBeatPositions[atBeat] != -1 || abs(distanceTraveled) < 100 || abs(distanceTraveled) > 100 && signA != signB) {
        MotorA.start();
        MotorA.torque(torque * myBeatDirections[atBeat] - (myBeatDirections[atBeat] == 1 ? 20 : 0) /* * myBeatStrength[atBeat]*/);
        MotorB.start();
        MotorB.torque(connectionMode * torque * myBeatDirections[atBeat] - (myBeatDirections[atBeat] == 1 ? 20 : 0) /* * myBeatStrength[atBeat]*/);
      //}
    }
    
    if(atBeat != previousAtBeat) {
      previousAtBeat = atBeat; // this is just to make sure we do certain things ONCE a bit later on
      /*if(myBeatPositions[atBeat] == -1) {
        myBeatPositions[atBeat] = analogRead(A1);
      }
      Serial.print(abs(distanceTraveled));
      Serial.print(" ");
      Serial.print(signA);
      Serial.print(" ");
      Serial.println(signB);*/       
    }

  } else if (choreographyCount != -1 && choreographyCount < choreographyCountGoal) {
   if (atBeat == previousAtBeat) {
      MotorA.stop();
      MotorB.stop();
      //myLastPosition = analogRead(A1);
      nextBeatAt = millisReading + (int)(millisForBeat * myBeats[atBeat]) - 1;
      atBeat++;
      if(atBeat >= numBeats) {
        atBeat = 0;
        previousAtBeat = -1;
        choreographyCount++;
      }
    }
  } else if (choreographyCount == choreographyCountGoal) {
    choreographyCount = -1;
    //resetPositions();
    delay(2000);
    // Prepare to Dance
    MotorA.start();
    MotorA.start();
  } else {
      xA = analogRead(A1);
      xB = analogRead(A9);
      dance();
      steppingSound(); // calls the audio function
      
      if(connectionMode == -1 && abs(xA+xB) > 1400) {
        awkward2(); 
      } else if (connectionMode == 1 && abs(xA-xB) >= 800) {
        awkward(); 
      }
  }

} // end of loop()


void dance() { 
  int foutA = torqueResponseWhileDancing*(xA-xB); 
  MotorA.torque(foutA);  
  int foutB = connectionMode * torqueResponseWhileDancing*(xB-xA); 
  MotorB.torque(foutB);   
} // end of dance()


void awkward() {
    xA = analogRead(A1);
    xB = analogRead(A9);  
    int p = 300;
    int foutA = -3*(xA-xB); 
    MotorA.torque(foutA/2);  
    int foutB = -3*(xB-xA); 
    MotorB.torque(foutB/2); 
    MotorA.start();
    MotorB.start();
    MotorA.torque(foutA + p);
    MotorB.torque(foutB + p);
    delay (10);
    
    // ADDED MUSIC STUFF HERE
    Music.setWaveform1(TRIANGLE);
    Music.setWaveform2(SINE);
    Music.setWaveform3(SINE);
    Music.setFrequency1(150);
    Music.setFrequency2(100);
    Music.setFrequency3(65);
    Music.setGain1(0.1 * stepA_dist);
    Music.setGain2(0.1 * stepB_dist);
    Music.setGain2(0.1 * stepB_dist);
    Music.setGain1(0.1 * stepA_dist);
    Music.setGain3(0.1 * stepB_dist);
    Music.setGain3(0.1 * stepA_dist);
    Music.setPortamento(39);
    Serial.println("wobble");
    // END AUDIO ADDITIONS
    
    MotorA.torque(0);
    MotorB.torque(0);
    delay (175);
    MotorA.torque(foutA - p);
    MotorB.torque(foutB - p);
    delay (20);
    MotorA.torque(0);
    MotorB.torque(0);
    delay (375);
} // end of awkward()

void awkward2() {
  xA = analogRead(A1);
  xB = analogRead(A9);  
  int p = 0;
  int foutA = -1*(1200-abs(xA-xB)); 
  int foutB = -1*(1200-abs(xA-xB)); 
  MotorA.start();
  MotorB.start();
  MotorA.torque(foutA + p);
  MotorB.torque(foutB + p);
  delay (10);
  MotorA.torque(0);
  MotorB.torque(0);
  delay (175);
  MotorA.torque(foutA - p);
  MotorB.torque(foutB - p);
  delay (20);
  MotorA.torque(0);
  MotorB.torque(0);
  delay (375);
} // end of awkward()

void steppingSound(){ //  a new function for audio
  stepA_dist = abs(xA_old - xA);
  stepB_dist = abs(xB_old - xB);

  if (stepA_dist >= stepThreshold) {

    stepping = true;

    Music.setWaveform1(SAW);
    Music.setWaveform2(SINE);
    Music.setWaveform3(TAN1);
    Music.setFrequency1(560 + );
    Music.setFrequency2(280);
    Music.setFrequency3(561);
    Music.setGain1(0.0001 * stepA_dist);
    Music.setGain2(0.0001 * stepA_dist);
    Music.setGain3(0.0001 * stepA_dist);
    Serial.println("stepping soundA");

  } else if (stepB_dist >= stepThreshold) {

    stepping = true;

    Music.setWaveform1(SAW);
    Music.setWaveform2(SINE);
    Music.setWaveform3(TAN1);
    Music.setFrequency1(600);
    Music.setFrequency2(468);
    Music.setFrequency3(561);
    Music.setGain1(0.0001 * stepB_dist);
    Music.setGain2(0.0001 * stepB_dist);
    Serial.println("stepping soundB");

  } else {
    stepping = false; 
  }

  if (stepping == false) {
    Music.setGain(0.6995f*Music.getGain());
  }
  
  if(millis() - prevTime >= 500){
    xA_old = xA;
    xB_old = xB;
    prevTime = millis();
  }
}

