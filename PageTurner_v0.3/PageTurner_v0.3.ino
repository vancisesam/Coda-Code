/* 
 *  Page Turning Mechanism v0.3
*/
/////////////////////////////////////////////////////////////////////////////////////////
int doubleTapThreshold = 400;
bool triggerState = LOW;
int batteryThreshold = 800;
float forceThreshold = 0.03;             //current threshold for the analog read which signals sufficient pressure on the separator arm
int forceHoldTime = 500;                      //amount of time (ms) the separator arm holds onto page
int preloadDelay = 2150;                  //amount of time (ms) between separator arm retraction and sweeper preload
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int deviceLed = A1;
int pedalLed = A0; 

int button = A7; //the pin the button is connected to
int triggerMonitor = A6; // the pin for the trigger power monitor
int deviceMonitor = A5; // the pin for the battery monitor

int currentSensor = A2;              //the pin for the current sensing device

int separatorEndstop = A4;            //the pin for the separator photointerruptor
int sweeperSensors[] = {11,9,12};           //the end stop photointerruptor for the sweeper
int bookmarkSensors[] = {A3,10}; //the pins for the bookmark sensors (left, right)


int separatorPins[] = {5,7,2};             //(PWM, input1, input2) for separator motor
int sweeperPins[] = {6,8,4};               //sweeper motor
int bookmarkPins[] = {3,8,4};              //bookmark motor

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MOVING_AVG_SIZE 500
int currentQueue[MOVING_AVG_SIZE];
int numPresses = 0;
unsigned long releaseTime;
bool wasPressed = true;

void setup() {
  Serial.begin(9600);
  pinMode(deviceLed, OUTPUT);
  pinMode(pedalLed, OUTPUT);
  pinMode(separatorPins[1], OUTPUT);
  pinMode(separatorPins[2], OUTPUT);
  pinMode(sweeperPins[1], OUTPUT);
  pinMode(sweeperPins[2], OUTPUT);
  digitalWrite(pedalLed, HIGH);
  digitalWrite(deviceLed, HIGH);

  Serial.println("The device is on!");
  bootup();
}

void loop(){
  bool buttonState = analogRead(button) > 500;
  if(buttonState && !wasPressed){   //just depressed the button
    numPresses++;
    if(numPresses == 2){
      numPresses = 0;
      wasPressed = false;
      bookmarkSequence();
      return;
    }
  }
  if(!buttonState && wasPressed){   //just released the button
    if(numPresses == 1){
      releaseTime = millis();
    }
  }
  
  //this is the timeout
  if((millis() - releaseTime)>doubleTapThreshold){
    if(buttonState){ //if you are currently pressing, don;t forget that
      numPresses = 1; 
    }
    else if(numPresses == 1){ //we have pressed once, and waited for the timeout
      numPresses = 0;
      wasPressed = false;
      forwardSequence();
      return;
    }
    else{
      numPresses = 0;
    }
  }
  //keep track of the old state
  wasPressed = buttonState;
}

/*Cycles all the arms through their positions
 * The states of the arms should be as follows when calling this method:
 * sweeper is in the middle state, with a page preloaded
 * seperation arm is in the "stowed" state, out of the way of the page turn
 */
void forwardSequence(){
  Serial.println("Forward Sequence");
  //flip the page
  motorWrite(100, sweeperPins);  //flipper motor forward

  while(!digitalRead(sweeperSensors[2]) == triggerState){};
  //motorWrite(0, sweeperPins); //flipper stopped

  
  //return the sweeper arm to natural state
  
  //motorWrite(0, sweeperPins);
  delay(10);
  motorWrite(-50, sweeperPins);
  
  while(!digitalRead(sweeperSensors[0]) == triggerState){};
  //delay(100); //delay till stop; ************************************************************
  motorWrite(0, sweeperPins); 
  preloadSequence();
}

//sweeper must be fully down on the right, and separator must be fully retracted
//************************************************************************************************
void preloadSequence(){ //PRELOAD MOTOR SPEEDS IN THIS FUNCTION
//  Serial.println("DO IT NOW");
//  delay(2000);
  Serial.println("Preload sequence!");
  //engage the page
  motorWrite(50, separatorPins); //PRELOAD SPEED
  float force = 0;
  unsigned long  initialTime = millis();
  while(true){
    force = getAvgCurrent(analogRead(currentSensor));
    //Serial.println(force);
    if(((millis() - initialTime) > 500 && (force > forceThreshold))){
      break;
    }
  }; //read the current from the motor to measure the force
  //motorWrite(0, separatorPins);
  delay(forceHoldTime);
  motorWrite(-9, separatorPins); //begin retracting the arm //RETRACT SPEED
  delay(preloadDelay);  //this is the magic time to wait before bringing up the sweeper arm for preload
  motorWrite(0, separatorPins); //Stoping motor for test
  motorWrite(55, sweeperPins);
//  
//  bool runSweeper = true;
//  bool sweepFinished = false;
//  bool runSeparator = true;
//  bool sepFinished = false;
//  while(!(sweepFinished && sepFinished)){
//    runSweeper = !digitalRead(sweeperSensors[1]) == triggerState;
//    runSeparator = !digitalRead(separatorEndstop) == triggerState;
//    if(!runSweeper){
//      if(!sweepFinished){
//        motorWrite(0, sweeperPins);
//        sweepFinished = true;
//      }
//    }
//    if(!runSeparator){
//      if(!sepFinished){
//        motorWrite(0, separatorPins);
//        Serial.print("sep done!");
//        sepFinished = true;
//      }      
//    }
  //Delay for a bit 
  bool runSweeper = true;
  bool sweepFinished = false;
  bool runSeparator = true;
  bool sepFinished = false;
  while(!sweepFinished){
    runSweeper = !digitalRead(sweeperSensors[1]) == triggerState;
    if(!runSweeper){
      if(!sweepFinished){
        motorWrite(0, sweeperPins);
        sweepFinished = true;
      }
    }
  };  //move arm up
  motorWrite(-20, separatorPins); 
  delay(50);
  
  int numTrips = 0;
  while(numTrips < 50){
    if(digitalRead(separatorEndstop) == triggerState){
      numTrips++;
    }
    else{
      numTrips = 0;
    }
    Serial.println(numTrips);
  }
  motorWrite(0, separatorPins);
  
  
  Serial.println("Sequence Finished!");
}


void bookmarkSequence(){
  Serial.println("Bookmark Sequence!");
  //get the sweeper out of the way
  motorWrite(10, sweeperPins); 
  while(!digitalRead(sweeperSensors[0]) == triggerState){};
  motorWrite(0, sweeperPins);
  
  //flip the bookmark
  motorWrite(10, bookmarkPins);
  while(!digitalRead(bookmarkSensors[1]) == triggerState){};  //move to the right
  motorWrite(-10, bookmarkPins);
  while(!digitalRead(bookmarkSensors[0]) == triggerState){};  //move back to the left
  motorWrite(0, bookmarkPins);
  
  preloadSequence();
}

void bootup(){
  setupAvg();
  Serial.println("Bootup Finished!");
}

void motorWrite(int motorSpeed, int pins[]){
 if (motorSpeed > 0)          // it's forward
 {  digitalWrite(pins[1], LOW);
    digitalWrite(pins[2], HIGH);
 }
 else if(motorSpeed < 0)                         // it's reverse
 {  digitalWrite(pins[1], HIGH);
    digitalWrite(pins[2], LOW);
 }
 else{  //its off
    digitalWrite(pins[1], LOW);
    digitalWrite(pins[2], LOW);
 }
 motorSpeed = abs(motorSpeed);
 motorSpeed = constrain(motorSpeed, 0, 255);   // Just in case...
 analogWrite(pins[0], motorSpeed);
}
void setupAvg(){
  for(int i = 0; i < MOVING_AVG_SIZE; i++){
    currentQueue[i] = 0;
  }
}

float getAvgCurrent(int newCurrent){
  int dummy = newCurrent;
  float sum = newCurrent;
  for(int i = 1; i < MOVING_AVG_SIZE; i++){
    currentQueue[i-1] = dummy;
    dummy = currentQueue[i];
    sum += currentQueue[i-1];
  }
  currentQueue[0] = newCurrent;
  //Serial.print("total:  ");
  //Serial.println(sum);
  float avg = sum/MOVING_AVG_SIZE;
  //Serial.print("avg: ");
  //Serial.println(avg);
  return avg;
}

void checkBatteries(){
  bool blinkDevice = false;
  bool blinkTrigger = false;
  if(analogRead(deviceMonitor)<batteryThreshold){
    blinkDevice = true;
  }
  if(digitalRead(triggerMonitor)==HIGH){
   blinkTrigger = false;
  }

  for(int i = 0; i< 4; i++){
    if(blinkDevice){
      digitalWrite(deviceLed, LOW); 
    }
    if(blinkTrigger){
      digitalWrite(pedalLed, LOW);
    }
    delay(500);
    digitalWrite(deviceLed,HIGH); 
    digitalWrite(pedalLed, HIGH);
  }
}





