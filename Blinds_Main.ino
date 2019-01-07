#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

int bluePin = 2;    //IN1 on the ULN2003 Board, BLUE end of the Blue/Yellow motor coil
int pinkPin = 3;    //IN2 on the ULN2003 Board, PINK end of the Pink/Orange motor coil
int yellowPin = 4;  //IN3 on the ULN2003 Board, YELLOW end of the Blue/Yellow motor coil
int orangePin = 5;  //IN4 on the ULN2003 Board, ORANGE end of the Pink/Orange motor coil
//Keeps track of the current step.
//We'll use a zero based index. 
int currentStep = 0;
unsigned int maxstep = 36864;
int eeAddress = 0;
const char* ssid = "Ermenegildo Zegna";
const char* password = "hhjw-ofvq-pafm";

const char* mqtt_server = "192.168.100.113";


#define blinds_position_topic "blinds/position"

char msg[50];

char buf[40]; //For MQTT data receive

WiFiClient espClient;
PubSubClient client(espClient);
 



void setup() {
  EEPROM.begin(4);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
  pinMode(bluePin, OUTPUT);
  pinMode(pinkPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(orangePin,OUTPUT);
  
  digitalWrite(bluePin, LOW);
  digitalWrite(pinkPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(orangePin, LOW);
}

void setup_wifi(){

  delay(10);


  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

}

void callback(char* topic, byte* payload, unsigned int length) {
  unsigned int step;
  EEPROM.get(eeAddress, step);
  int i = 0;
    // create character buffer with ending null terminator (string)
  for(i=0; i<length; i++) {    
    buf[i] = payload[i];
  }
  buf[i] = '\0';
  String msgString = String(buf);
  if (String(topic) == "blinds/command")
    {
        if (payload[0] == '1')  //go up
        {
    while(step>0){
  
  switch(currentStep){
    case 0:
      digitalWrite(bluePin, HIGH);
      digitalWrite(pinkPin, HIGH);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, LOW);

      break;
    case 1:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, HIGH);
      digitalWrite(yellowPin, HIGH);
      digitalWrite(orangePin, LOW);

      break;
    case 2:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, HIGH);
      digitalWrite(orangePin, HIGH);
      break;
    case 3:

      digitalWrite(bluePin, HIGH);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, HIGH);
      break;
  }
    step--;
      //sprintf(msg, "%ld", step);
     // client.publish(blinds_position_topic, msg);
  currentStep = (++currentStep < 4) ? currentStep : 0;
  
  //30ms delay for having enough torque for lifting and rolling blinds
if(step>=15000&&step<25000)
{
  delay(25);
}
else if(step<15000)
{
  delay(20);
}
 else 
 {
  delay(30);
 }
}
  EEPROM.put(eeAddress, step);
  EEPROM.commit();
         }
     else if (payload[0] == '0')
        {
           while(step<=maxstep){
  
  switch(currentStep){
    case 0:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, HIGH);

      break;
    case 1:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, HIGH);
      digitalWrite(orangePin, LOW);

      break;
    case 2:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, HIGH);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, LOW);
      break;
    case 3:
      digitalWrite(bluePin, HIGH);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, LOW);
      break;
  }
  step++;
  currentStep = (++currentStep < 4) ? currentStep : 0;
    //  sprintf(msg, "%ld", step);
     // client.publish(blinds_position_topic, msg);
  //2000 microseconds, or 2 milliseconds seems to be 
  //about the shortest delay that is usable.  Anything
  //lower and the motor starts to freeze. 
  //delayMicroseconds(2250);
  delay(2);
}
  EEPROM.put(eeAddress, step);
  EEPROM.commit();
        }
  digitalWrite(bluePin, LOW);
  digitalWrite(pinkPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(orangePin, LOW);
        }
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("ESP8266BYJ48")) {
      // ... and resubscribe
      client.subscribe("blinds/command");
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}



void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
