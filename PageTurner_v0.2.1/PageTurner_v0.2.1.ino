/* etfnew2
 *  Page Turning Mechanism v0.1.1
 *  
 *  Excecutes the page turning sequence based on a button press.  Can be repeated infinite times.
 *  
 *  The sequence is as follows:
 *  Lower the wheel
 *  Spin the wheel
 *  Seperate pages using splitter arm
 *  **FLIP THE PAGE, DONE MANUALLY**
 *  Raise the wheel
 *  Return Splitter Arm to natural postion
*/
#include <Servo.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int button = 3;                   //the pin the button is connected to
int seperatorPositions[] = {0,1,2};   //the states for the page seperator arm (full release, on page, holding page)
int sweeperPositions[] = {0,1,2};  //the states for the turning arm (fully down, preloaded, turn)
int bookmarkPositions[] = {0,1}; //state for bookmark (left side, right side)
int seperatorPin = 0;
int sweeperPin = 0;
int bookmarkPin = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Servo seperator;
Servo sweeper;
Servo bookmark;

int numPresses = 0;
unsigned long releaseTime;
int doubleTapThreshold = 400;
bool wasPressed = true;

void setup() {
  Serial.begin(9600);
  pinMode(button, INPUT);
  
  seperator.attach(seperatorPin);
  sweeper.attach(sweeperPin);
  bookmark.attach(bookmarkPin);
  Serial.println("The device is on!");
}

void loop() {
  bool buttonState = digitalRead(button) == HIGH;
  //Serial.println(numPresses);
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
  sweeper.write(sweeperPositions[2]); //flip the page
  seperator.write(seperatorPositions[0]); //retract the seperator arm
  delay(5000);    //this is the time between sweeper returning and the preload sequence.  it exists to give time to allow manual override of a misflipped page

  sweeper.write(sweeperPositions[0]); //return the sweeper arm to natural state
  delay(1000);
  preloadSequence();
}

//sweeper must be fully down on the left, and seperator must be fully retracted
void preloadSequence(){
  Serial.println("Preload sequence!");
  seperator.write(seperatorPositions[1]); //engage the page
  delay(1000);
  seperator.write(seperatorPositions[2]); //lift the page

  delay(3000); //this is the amount of time for static to release additional pages

  sweeper.write(sweeperPositions[1]); //pre load the page
  delay(1000);
  Serial.println("Sequence Finished!");
}

void bookmarkSequence(){
  Serial.println("Bookmark Sequence!");
  sweeper.write(sweeperPositions[0]); //get the sweeper and seperator out of the way
  seperator.write(seperatorPositions[0]);

  bookmark.write(bookmarkPositions[1]); //flip the bookmark
  delay(1000);
  bookmark.write(bookmarkPositions[0]); //return the bookmark
  delay(1000);
  preloadSequence();
}


