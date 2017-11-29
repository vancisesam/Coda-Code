/* 
 *  Page Turning Mechanism v0.3
*/
/////////////////////////////////////////////////////////////////////////////////////////
int doubleTapThreshold = 400;
bool triggerState = LOW;
int batteryThreshold = 800;
int forceThreshold = 0;             //current threshold for the analog read which signals sufficient pressure on the separator arm
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int button = A6;                      //the pin the button is connected to
int triggerMonitor = A7;                //the battery monitor for the footpedal
int deviceMonitor = A5;               //the battery monitor for the device
int deviceLed[] = {A0, A1};           //green, red
int pedalLed[] = {12, 13};            //green, red
int separatorEndstop = 0;             //the pin for the separator photointerruptor
int currentSensor = 2;                //the pin for the current sensing device
int sweeperEndstop = 0;               //the end stop photointerruptor for the sweeper
int sweeperMidsensor = 0;             //the sweeper midsensor
int bookmarkSensors[] = {0,0};        //the pins for the bookmark sensors (left, right)

int separatorPins[] = {5,3,4};        //(PWM, input1, input2) for separator motor
int sweeperPins[] = {10,A2,A3};       //sweeper motor
int bookmarkPins[] = {11,A2,A3};      //bookmark motor

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int numPresses = 0;
unsigned long releaseTime;
bool wasPressed = true;

void setup() {
  Serial.begin(9600);
  pinMode(deviceLed[0], OUTPUT);
  pinMode(deviceLed[1], OUTPUT);
  pinMode(pedalLed[0], OUTPUT);
  pinMode(pedalLed[1], OUTPUT);


  Serial.println("The device is on!");
  bootup();
}

void loop(){
  bool buttonState = digitalRead(button) == HIGH;
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
  motorWrite(10, sweeperPins);  //flipper motor forward
  while(!digitalRead(sweeperEndstop) == triggerState){};
  motorWrite(0, sweeperPins); //flipper stopped
  
  //return the sweeper arm to natural state
  motorWrite(-10, sweeperPins);
  while(!digitalRead(sweeperMidsensor) == triggerState){}; // first time past photointerruptor
  while(digitalRead(sweeperMidsensor) == triggerState){};  //in the first photointerruptor
  while(!digitalRead(sweeperMidsensor) == triggerState){}; //second time in the photointerruptor
  motorWrite(0, sweeperPins);
  
  preloadSequence();
}

//sweeper must be fully down on the right, and separator must be fully retracted
void preloadSequence(){
  Serial.println("Preload sequence!");
  //engage the page
  motorWrite(10, separatorPins);
  while(analogRead(currentSensor) < forceThreshold){}; //read the current from the motor to measure the force
  motorWrite(-10, separatorPins); //begin retracting the arm 
  delay(500);  //this is the magic time to wait before bringing up the sweeper arm for preload
  motorWrite(10, sweeperPins);
  while(digitalRead(sweeperMidsensor) == triggerState){};  //still in the first photointerruptor
  while(!digitalRead(sweeperMidsensor) == triggerState){}; //second time in the photointerruptor
  motorWrite(0, sweeperPins);
  while(!digitalRead(separatorEndstop) == triggerState){};
  motorWrite(0, separatorPins);
  Serial.println("Sequence Finished!");
}


void bookmarkSequence(){
  Serial.println("Bookmark Sequence!");
  //get the sweeper out of the way
  motorWrite(-10, separatorPins); 
  while(digitalRead(sweeperMidsensor) == triggerState){};  //still in the first photointerruptor
  while(!digitalRead(sweeperMidsensor) == triggerState){}; //moved to the second photointerruptor
  motorWrite(0, separatorPins);
  
  //flip the bookmark
  motorWrite(10, bookmarkPins);
  while(!digitalRead(bookmarkSensors[1]) == triggerState){};  //move to the right
  motorWrite(-10, bookmarkPins);
  while(!digitalRead(bookmarkSensors[0]) == triggerState){};  //move back to the left
  motorWrite(0, bookmarkPins);
  
  preloadSequence();
}

void bootup(){
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
    return;
 }
 motorSpeed = abs(motorSpeed);
 motorSpeed = constrain(motorSpeed, 0, 255);   // Just in case...
 analogWrite(pins[0], motorSpeed);
}

void checkBatteries(){
  if(analogRead(deviceMonitor)<batteryThreshold)){
    digitalWrite(deviceLed[1], HIGH);
  }
  else{
    digitalWrite(deviceLed[0], HIGH);
  }
  if(digitalRead(triggerMonitor)==HIGH){
   digitalWrite(pedalLed[1], HIGH);
  }
  else{
    digitalWrite(pedalLed[0], HIGH);
  }
  delay(1000);
  digitalWrite(deviceLed[0], LOW);
  digitalWrite(deviceLed[0], LOW);
  digitalWrite(pedalLed[0], LOW);
  digitalWrite(pedalLed[0], LOW);
}





