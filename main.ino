// Jae Park, Jada Greenbloom, Hannah Kim
// TEJ4MP with Mr. Wong
// April 19, 2021
// This program handles and controls an Arduino circuit 
// to simulate a home security system.

#include <IRremote.h>
#include <LiquidCrystal.h>

// Digital Pins
const int DIP1 = 4; // pin for DIP switch 1
const int DIP2 = 3; // pin for DIP switch 2
const int DIP3 = 6; // pin for DIP switch 3
const int DIP4 = 5; // pin for DIP switch 4
const int BUTTON = 8; // pin for door bell
const int SWITCH = 7; // pin for slideswitch
const int ALARM = 9; // pin for alarm
const int IR = 10; // pin for IR receiver
const int DOORLAMP = 11; //pin for door lamp (red LED)
// pin for ultrasonic distance sensor
const int DISTANCESENSOR = 12; 
// pin for LED with distance sensor (white LED)
const int DISTANCELIGHT = 13; 

// Analog Pins 
const int LDR = A0;

// Helper variables
bool objectDetected = false; // if the object is detected
bool doorClosed = false; // if the door is opened
int homeStatus = 0; // status of the securty system
// 0 = OFF, 1 = HOME, 2 = AWAY
// if the user is currently entering a password
bool enteringPassword = false; 
// if the alarm should be on
bool soundAlarm = false;
String userInput = ""; // users password attempt
String correctPassword = "01"; // correct password
// if the button has been pressed
int pressButton = 0;
bool pressed3 = false, pressed4 = false;	// booleans that help with DIP3 and DIP4 input
bool unlocked = false;	// house is unlocked after password is input correctly

// LCD
LiquidCrystal lcd(16, 17, 18, 19, 15, 2); // lcd display object

// Async time variables
unsigned long curTime, prevTime;


// This method resets everything
void reset(){
   	userInput = "";
   	soundAlarm = false;
   	enteringPassword = false;
   	objectDetected = false;
  	pressed3 = false;
  	pressed4 = false;
}

// This method reads input from the slide switch 
// and sets the home status based on input. It also
// resets the display on the LCD.
void setStatus(){
  int previousStatus = homeStatus;
  int num1 = digitalRead(DIP1);
  int num2 = digitalRead(DIP2);
  
  if (num1 == 0 && num2 == 0){
    homeStatus = 1;			// home
  }else if (num1 == 1 && num2 == 1){
	homeStatus = 2;			// away
  }else{
   	homeStatus = 0;			// off 
  }
  
  if (previousStatus != homeStatus){
    lcd.clear();
  }
  lcd.setCursor(0, 0);
  lcd.print("Status: ");
  if (homeStatus == 1){
  	lcd.print("Home"); 
  } else if (homeStatus == 2){
    lcd.print("Away");
  } else {
    lcd.print("Off");
  }
}


// This method checks to see if the door is open
void checkDoor(){
  doorClosed = digitalRead(SWITCH);	// switch input: 0 is right (open), 1 is left (closed)
  
  if (!doorClosed && (homeStatus == 2|| homeStatus == 1) && !unlocked){		// if door is open and status is set to away
    lcd.clear();
    enteringPassword = true;
    prevTime = curTime;
    soundAlarm = true;
  }else if (doorClosed){
    unlocked = false;
  }
}

// This method rings the door bell and reads the
// button input.
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


// This method sets the alarm based on the current state of soundAlarm
void setAlarm(){
  if (soundAlarm){
    digitalWrite(ALARM, HIGH);
  } else{
   	digitalWrite(ALARM, LOW); 
  }
}



void pingDistance(){
  	long distance, converted; 
  	// Get input
 	pinMode(DISTANCESENSOR, OUTPUT);
  	digitalWrite(DISTANCESENSOR, LOW);
  	delayMicroseconds(2);
  	digitalWrite(DISTANCESENSOR, HIGH); 
  	delayMicroseconds(2);
  	digitalWrite(DISTANCESENSOR, LOW);
  	pinMode(DISTANCESENSOR, INPUT);
  	
  	// Convert input into distance in cm
  	distance = pulseIn(DISTANCESENSOR, HIGH);
  	converted = distance /29 /2;
  	
  	//Serial.println(converted); 		// TODO: REMOVE IN FINAL
  
  	// Check if there is object in distance range
  	if (100 <= converted && converted <= 200){
    	digitalWrite(DISTANCELIGHT, HIGH); // TODO: Make this flash async
      	digitalWrite(DISTANCELIGHT, LOW);
    } else{
    	digitalWrite(DISTANCELIGHT,HIGH);
    }  
}

void setLamp(){
	//int meterReading = analogRead(POTENTIOMETER);
	//int brightness = map(meterReading, 0, 1023, 0, 255);
	int ldrReading = analogRead(LDR);
	
  	//Serial.println(ldrReading); 	// TODO: REMOVE IN FINAL
	
	// check for darkness, and show light
	if (ldrReading <= 400){ // TODO: CHECK THRESHOLD VALUE
    	digitalWrite(DOORLAMP, HIGH);
    }else{
      	analogWrite(DOORLAMP, LOW);
    }
}

void getPassword(){
 	lcd.setCursor(0, 0);
  	lcd.print("Enter Password:");
  	bool tempDIP3, tempDIP4;
  	
    // input mapping
    tempDIP3 = digitalRead(DIP3);
    tempDIP4 = digitalRead(DIP4);
      	
	if (tempDIP3 && !pressed3){
		userInput = userInput + "0";
      	pressed3 = true;
      	
    }else if (tempDIP4 && !pressed4){
		userInput = userInput + "1";
      	pressed4 = true;
    }
    
  	//Serial.println(userInput);
  	//Serial.print(prevTime);
  	//Serial.print(" is prev and cur is:");
  	//Serial.println(curTime);
  
  	if (userInput.equals(correctPassword)){
      	lcd.clear();
		soundAlarm = false;
      	unlocked = true;
      	setAlarm();
      	lcd.print("Correct");
      	delay(1000);
      	reset();
  	}else if (curTime - prevTime >= 5000){
    	lcd.clear();
    	lcd.print("Incorrect");
      	delay(6000);
      	soundAlarm = false;
      	unlocked = true;
      	reset();
  	}
}

void setup()
{
    Serial.begin(9600); // serial for debugging
 	pinMode(DIP1, INPUT);
 	pinMode(DIP2, INPUT);
  	pinMode(DIP3, INPUT);
  	pinMode(DIP4, INPUT);
  	lcd.begin (16,2);
  	// initialize the LED as an output:
    pinMode(BUTTON, INPUT);
  	pinMode(ALARM, OUTPUT);
}

void loop()
{
  	curTime = millis(); // set the time since execution
  	if (enteringPassword){
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
