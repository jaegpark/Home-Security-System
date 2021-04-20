// Chief Programmer: Jae Park
// Chief Engineer: Hannah Kim
// Project Manager: Jada Greenbloom
// TEJ4MP with Mr. Wong
// April 19, 2021
// This program handles and controls an Arduino circuit 
// to simulate a home security system.

#include <IRremote.h>
#include <LiquidCrystal.h>

//-------------Global Variables-------------//


// Digital Pins
const int DIP1 = 4; 						// pin for DIP switch 1
const int DIP2 = 3; 						// pin for DIP switch 2
const int DIP3 = 6; 						// pin for DIP switch 3
const int DIP4 = 5; 						// pin for DIP switch 4
const int BUTTON = 8; 						// pin for door bell
const int SWITCH = 7; 						// pin for slideswitch
const int ALARM = 9; 						// pin for alarm
const int IR = 10; 							// pin for IR receiver
const int DOORLAMP = 11; 					// pin for door lamp (red LED)
const int DISTANCESENSOR = 12; 				// pin for ultrasonic distance sensor
const int DISTANCELIGHT = 13; 				// pin for LED with distance sensor (white LED)

// Analog Pins 
const int LDR = A0;

// Helper variables
bool objectDetected = false; 				// if the object is detected
bool doorClosed = false; 					// if the door is opened
int homeStatus = 0; 						// status of the securty system
											// 0 = OFF, 1 = HOME, 2 = AWAY
bool enteringPassword = false;	 			// if the user is currently entering a password
bool soundAlarm = false; 					// if the alarm should be on
String userInput = ""; 						// users password attempt
String correctPassword = "01"; 				// correct password
int pressButton = 0;						// if the button has been pressed
bool pressed3 = false, pressed4 = false;	// booleans that help with DIP3 and DIP4 input
bool unlocked = false;						// house is unlocked after password is input correctly
bool statusOveride = false; 				// set to true once the remote overides the DIP switch for home status
bool toggleLight = false;					// used to toggle the blinking LED for the distance sensor


// LCD
LiquidCrystal lcd(16, 17, 18, 19, 15, 2); 	// lcd display object

// IR Remote
IRrecv receiver(IR); 
decode_results results;

// Time variables
unsigned long curTime, prevTime;


/*
	This helper method resets most of the helper variables related to the alarm control panel.
*/ 
void reset(){
   	userInput = "";
   	soundAlarm = false;
   	enteringPassword = false;
   	objectDetected = false;
  	pressed3 = false;
  	pressed4 = false;
}

/* 
  This method reads input from the slide switch and sets the home status based on input. 
  It also resets the display on the LCD.
*/
void setStatus(){
  int previousStatus = homeStatus;
  int num1 = digitalRead(DIP1);
  int num2 = digitalRead(DIP2);
  
  if (!statusOveride){
        if (num1 == 0 && num2 == 0){
          	homeStatus = 1;			// BOTH SWITCHES DOWN = At Home
        }else if (num1 == 1 && num2 == 1){
          	homeStatus = 2;			// BOTH SWITCHES ON = Away
        }else{
          	homeStatus = 0;			// one switch up, the other down = Off
        }
  }
  
  if (receiver.decode(&results)) {
      	if(results.value == 0xFD08F7){ 			// if user presses 1 on the remote, map to Off
          	homeStatus = 0;
          	statusOveride = true;
        }else if (results.value == 0xFD8877){ 	// if user presses 2 on the remote, map to At Home
          	homeStatus = 1;
          	statusOveride = true;
        }else if (results.value == 0xFD48B7){ 	// if user presses 3 on the remote, map to Away
          	homeStatus = 2;
          	statusOveride = true;
        }else if (results.value == 0xFD58A7){ 	// if user presses 9 on the remote, map to unlock the remote override
          	statusOveride = false;
        }
		receiver.resume();
  }
  
  if (previousStatus != homeStatus){
    lcd.clear();			// More efficient display update (only update if status has changed)
  }
  
  lcd.setCursor(0, 0);
  lcd.print("Status: ");
  switch(homeStatus){
   	case 0:
    	lcd.print("Off");
    	break;
    case 1:
    	lcd.print("Home");
    	break;
    case 2:
    	lcd.print("Away");
    	break;
  }
}


/*
	 This method checks to see if the door is open, and sets the status 
     of the alarm after checking the home status.
*/
void checkDoor(){
  doorClosed = digitalRead(SWITCH);	// switch input: 0 is right (open), 1 is left (closed)
  
  // if door is open and status is set to away/home and the door is locked, ring alarm and set values for time
  if (!doorClosed && (homeStatus == 2|| homeStatus == 1) && !unlocked){		
    lcd.clear();
    enteringPassword = true;
    prevTime = curTime;
    soundAlarm = true;
  }else if (doorClosed){	// reset the lock on door to locked once the door is closed
    unlocked = false;
  }else if (homeStatus == 0 && !doorClosed){	// chime (rapid beep) if the door is opened when status is Off
    digitalWrite(ALARM, HIGH);
    delay(200);
    digitalWrite(ALARM, LOW);
  }
}

/* 
	This method rings the piezo to mimic a doorbell and reads input from the button.
*/
void soundBell(){
  // checks to see if the button has been pressed
  pressButton = digitalRead(BUTTON);
  
  // if the button was pressed
  if (pressButton == 1){
    // ring the door bell for 1 second
    digitalWrite (ALARM, HIGH);
    delay (1000);
  } else{ 
    digitalWrite (ALARM, LOW);
  }	
}


/*
	This method sets the alarm based on the current state of soundAlarm.
    It can turn off/on the alarm, and is called frequently by other methods.
*/
void setAlarm(){
  if (soundAlarm){
    digitalWrite(ALARM, HIGH);
  } else{
   	digitalWrite(ALARM, LOW); 
  }
}

/*
	This method pings the distance sensor and controls the distance (red) LED.
    It also calls the alarm if an object is detected when the homeStatus is set to away.
*/
void pingDistance(){
  	long distance, converted; 
  	
  // Get input
 	pinMode(DISTANCESENSOR, OUTPUT);
  	digitalWrite(DISTANCESENSOR, LOW);
  	delayMicroseconds(2);
  	digitalWrite(DISTANCESENSOR, HIGH); 
  	delayMicroseconds(10);
  	digitalWrite(DISTANCESENSOR, LOW);
  	pinMode(DISTANCESENSOR, INPUT);
  	
  	// Convert input into distance in cm
  	distance = pulseIn(DISTANCESENSOR, HIGH);
  	converted = distance /29;
	Serial.println(converted);

  	// Check if there is object in distance range
  	if (100 <= converted && converted <= 200){
        if (homeStatus == 2){				// Call alarm if status is away
			soundAlarm = true;
            setAlarm();
          	delay(5000);
          	soundAlarm = false;
          	setAlarm();
        }
        if (toggleLight) {
			digitalWrite(DISTANCELIGHT, HIGH); 
        }else {
      		digitalWrite(DISTANCELIGHT, LOW);
        }
      	toggleLight = !toggleLight;
    } else{
    	digitalWrite(DISTANCELIGHT,HIGH);
    }  
}

/*
	This method controls the brightness of the door lamp (white) LED, based on the voltage 
    given by the potentiometer. It also cuts the light when the environment is above a certain
    brightness.
*/
void setLamp(){
	int ldrReading = analogRead(LDR);
	// check for darkness, and show light
	if (ldrReading <= 400){ // temporary threshold of 400.
    	digitalWrite(DOORLAMP, HIGH);
    }else{
      	analogWrite(DOORLAMP, LOW);
    }
}

/*
	This method gets the user's password input from the 3 and 4 DIP switches.
    It then calls the setAlarm() method based on user input.
*/
void getPassword(){
  	// Prompt user for input
 	lcd.setCursor(0, 0);
  	lcd.print("Enter Password:");
  	bool tempDIP3, tempDIP4;
  	
    // input mapping
    tempDIP3 = digitalRead(DIP3);
    tempDIP4 = digitalRead(DIP4);
      	
	if (tempDIP3 && !pressed3){
      	userInput = userInput + "0";
      	pressed3 = true;
      	lcd.print(0);
    }else if (tempDIP4 && !pressed4){
		userInput = userInput + "1";
      	pressed4 = true;
      	lcd.print(1);
    }
  
  	if (receiver.decode(&results)) {
      	if(results.value == 0xFD30CF){
        	lcd.clear();
          	soundAlarm = false;
          	unlocked = true;
          	setAlarm();
          	lcd.print("Override");
          	delay(2000);
          	reset();
        }
		receiver.resume();
    }
  
  	// Decision making based on correct password unlock/over 5 s taken
  	if (userInput.equals(correctPassword)){
      	lcd.clear();
		soundAlarm = false;
      	unlocked = true;
      	setAlarm();
      	lcd.print("Correct");
      	delay(2000);
      	reset();
  	}else if (curTime - prevTime >= 5000){
    	lcd.clear();
    	lcd.print("Incorrect");
      	delay(6000);
      	soundAlarm = false;
      	unlocked = true;
      	setAlarm();
      	reset();
  	}
}

void setup()
{
    Serial.begin(9600); 	// serial for debugging
 	
  	// initialize DIP switch
  	pinMode(DIP1, INPUT);
 	pinMode(DIP2, INPUT);
  	pinMode(DIP3, INPUT);
  	pinMode(DIP4, INPUT);
  	
  	lcd.begin (16,2);
  	pinMode(BUTTON, INPUT);
  	pinMode(ALARM, OUTPUT);
  
  	receiver.enableIRIn();

}

void loop()
{
  	curTime = millis(); // set the time since execution
  
  	if (enteringPassword){		// give priority to if user is entering password
   		getPassword(); 
    }else{
     	setStatus();
      	setLamp();
      	checkDoor();
      	pingDistance();
    }
  	
	soundBell();
	setAlarm();
  	delay(100);
}
