

const int ALARM = 9;


// Variables for Potentiometer/LED System
const int POTENTIOMETER = A0;
const int DOORLAMP = 11;
const int LDR = A1;

// Variables for IR Sensor
const int IRSENSOR = 10;

// Variables for the indoor distance sensor
const int DISTANCESENSOR = 12;
bool objectDetected = false;
const int DISTANCELIGHT = 13;

bool soundAlarm = false;




void setAlarm(){
  if (soundAlarm){
    
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
  	
  	Serial.println(converted); 
  	// Check if there is object in distance range
  	if (100 <= converted && converted <= 200){
    	digitalWrite(DISTANCELIGHT, HIGH); 
      	digitalWrite(DISTANCELIGHT, LOW);
    } else{
    	digitalWrite(DISTANCELIGHT,HIGH);
    }  
}

void setLamp(){
	int meterReading = analogRead(POTENTIOMETER);
	int brightness = map(meterReading, 0, 1023, 0, 255);
	int ldrReading = analogRead(LDR);
	Serial.println(ldrReading);
	// check for darkness, and show light
	if (ldrReading <= 300){ // TODO: CHECK THRESHOLD VALUE
    	analogWrite(DOORLAMP, brightness);
    }else{
      	analogWrite(DOORLAMP,0);
    }
}

void setup()
{
    Serial.begin(9600); // serial for debugging
}

void loop()
{
  	setLamp();
    pingDistance();

  	delay(100);
}