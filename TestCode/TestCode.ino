/* 
 *  Test Code
*/
/////////////////////////////////////////////////////////////////////////////////////////
int doubleTapThreshold = 400;
bool triggerState = HIGH;
int forceThreshold = 0;             //current threshold for the analog read which signals sufficient pressure on the separator arm
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int powerIndicator = A1;
int triggerIndicator = A0; 

int button = A6; //the pin the button is connected to
int triggerPower = A7; // the pin for the trigger power
int batteryMonitor = A5; // the pin for the battery monitor

int currentSensor = A4;              //the pin for the current sensing device

int separatorEndstop = 8;            //the pin for the separator photointerruptor
int sweeperSensors[] = {9,10,A3};           //the end stop photointerruptor for the sweeper
int bookmarkSensors[] = {11,12}; //the pins for the bookmark sensors (left, right)

int separatorPins[] = {5,7,2};             //(PWM, input1, input2) for separator motor
int sweeperPins[] = {6,A2,4};               //sweeper motor
int bookmarkPins[] = {3,A2,4};              //bookmark motor

 

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int numPresses = 0;
unsigned long releaseTime;
bool wasPressed = true;

void setup() {
  Serial.begin(9600);
  
  pinMode(powerIndicator, OUTPUT);
  pinMode(triggerIndicator, OUTPUT);

  motorWrite(0, sweeperPins);
  motorWrite(0, separatorPins);
  motorWrite(0, bookmarkPins);

  Serial.println("The device is on!");
}

void loop(){
  digitalWrite(powerIndicator, HIGH);
  digitalWrite(triggerIndicator, HIGH);
  
//  Photosensor Tests
  Serial.print("sensor 1: ");
  Serial.print(digitalRead(separatorEndstop));
  Serial.print(" ");
  Serial.print("sensor 2: ");
  Serial.print(digitalRead(sweeperSensors[1]));
  Serial.print(" ");
  Serial.print("sensor 3: ");
  Serial.print(digitalRead(sweeperSensors[0]));
  Serial.print(" ");
  Serial.print("sensor 4: ");
  Serial.print(digitalRead(bookmarkSensors[0]));
  Serial.print(" ");
  Serial.print("sensor 5: ");
  Serial.print(digitalRead(bookmarkSensors[1]));
  Serial.print(" ");
  Serial.print("sensor 6: ");
  Serial.println(digitalRead(sweeperSensors[2]));

//  Motor Tests
//  motorWrite(15, separatorPins);
//  motorWrite(15, sweeperPins);
//  delay(300);
//  motorWrite(-15, sweeperPins);
//  motorWrite(-15, separatorPins);
//  delay(300);
//  motorWrite(16, separatorPins);
//  motorWrite(16, sweeperPins);
//  delay(15);
//  motorWrite(0, sweeperPins);
//  motorWrite(0, separatorPins);
//  delay(1000);

//  Current Sensing (Sensing on the board in the range of mV but not sensing on Arduino)
//motorWrite(-50, separatorPins);
//  for(int i = 0; i < 1000; i++){
//    Serial.print("Sensing: ");
//    Serial.print(analogRead(currentSensor));
//    Serial.println(" ");
//    delay(1);
//  }
//  motorWrite(0, separatorPins);
//  delay(1000);
  

//  LED Testing
//
//  digitalWrite(powerIndicators[0], HIGH);
//  delay(500);
//  digitalWrite(powerIndicators[1], HIGH);
//  delay(500);
//  digitalWrite(triggerIndicators[0], LOW);
//  delay(500);
//  digitalWrite(triggerIndicators[1], LOW);
//  delay(500);
//  delay(50);
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
