#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

const int SDAPIN   = 10;
const int RSTPIN   =  9;
MFRC522 rfidReader(SDAPIN, RSTPIN); // RFID-Reader

const int REDLED   = 5;  // Uno -> Pin 8
const int GREENLED = 6;  // Uno -> Pin 7
const int SERVOPIN = 3;  // Uno -> Pin 6
Servo door;

//////// ID of the right card //////////////
byte sesam[] = {0x60, 0xA6, 0x36, 0x11};


///////State variables////////////
int state = 0;
int servo = 0;

/*//////LED blinking///////
const long interval = 100;           // interval at which to blink (milliseconds)
unsigned long previousMillis = 0;        // will store last time LED was updated
int ledState = LOW;             // ledState used to set the LED
*/

void setup()
{

  door.attach(SERVOPIN);
  pinMode(REDLED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  pinMode(SERVOPIN, OUTPUT);

 // Serial.begin(9600); // Serielle Verbindung

  SPI.begin(); // open SPI-connection

  rfidReader.PCD_Init(); // Initial. RFID-reader
 // Serial.println("Doorlock is activated");

  door.write(0); // reset Servo
  signalDoorLocked();
  servo = 0;


}



//////////////////////////////////////////////////
//
// acceptedRFID() and unacceptedRFID()
// is the new RFID the same as the one in sesam[]?
//
//////////////////////////////////////////////////
bool acceptedRFID(byte uid[4]) {
  return
      (rfidReader.uid.uidByte[0] == sesam[0]) &&
      (rfidReader.uid.uidByte[1] == sesam[1]) &&
      (rfidReader.uid.uidByte[2] == sesam[2]) &&
      (rfidReader.uid.uidByte[3] == sesam[3]);
}

bool unacceptedRFID(byte uid[4]) {
  return
      (rfidReader.uid.uidByte[0] != sesam[0]) &&
      (rfidReader.uid.uidByte[1] != sesam[1]) &&
      (rfidReader.uid.uidByte[2] != sesam[2]) &&
      (rfidReader.uid.uidByte[3] != sesam[3]) &&
      (rfidReader.uid.uidByte[0] != 0) &&
      (rfidReader.uid.uidByte[1] != 0) &&
      (rfidReader.uid.uidByte[2] != 0) &&
      (rfidReader.uid.uidByte[3] != 0) ;
}

//////////////////////////////////////////////////
//
// openDoor()
//   turn servo by 120°
//
//////////////////////////////////////////////////
void openDoor() {
    door.write(120);
}

//////////////////////////////////////////////////
//
// closeDoor()
//   turn servo back by 120°
//
//////////////////////////////////////////////////
void closeDoor() {
   door.write(0);
}
//////LED blink functions////////////
void signalDoorLocked() {
  digitalWrite(REDLED, HIGH);
  digitalWrite(GREENLED, LOW);
}

void signalDooropened() {
  digitalWrite(REDLED, LOW);
  digitalWrite(GREENLED, HIGH);
}

void signalAccessGranted() {
 for (int i = 0; i < 5; i++) {
    digitalWrite(GREENLED, HIGH);
    delay(100);
    digitalWrite(GREENLED, LOW);
    delay(100);
  }
}

void signalAccessRefused() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(REDLED, HIGH);
    delay(100);
    digitalWrite(REDLED, LOW);
    delay(100);
  }
}
/*
void ledblink(int ledPin){
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}
*/
void loop()
{
  ///////Show the State in LEDs///////////
     if (servo == 0){
      signalDoorLocked();
    }
  if (servo == 1){
      signalDooropened();
    }

  if (rfidReader.PICC_IsNewCardPresent() && rfidReader.PICC_ReadCardSerial()) {           // Read card UID and store it in array
   //   Serial.print("New card found... ID is =>  ");
   //   Serial.print("/");
         for (byte i = 0; i < rfidReader.uid.size; i++) {
    //        Serial.print(rfidReader.uid.uidByte[i],HEX);
    //        Serial.print("/");
         }
        }
  else if (!rfidReader.PICC_IsNewCardPresent() && !rfidReader.PICC_ReadCardSerial())        //Reset State and set UID array to 0,0,0,0
       {state = 0;
           for (byte i = 0; i < rfidReader.uid.size; i++) {
            rfidReader.uid.uidByte[i]=0;
         }
        }

    if (acceptedRFID(rfidReader.uid.uidByte) && servo == 0 && state == 0) {
         signalAccessGranted();
    //     Serial.println("Access granted => open servo");
         openDoor();
         servo = 1;
         state = 1;
        }
    if (acceptedRFID(rfidReader.uid.uidByte) && servo == 1 && state == 0) {
         signalAccessGranted();
     //    Serial.println("Access granted => close servo");
         closeDoor();
         servo = 0;
         state = 1;
        }
    if (unacceptedRFID(rfidReader.uid.uidByte) == true)
        {
     //   Serial.println("Access denied");
        signalAccessRefused();
        }


 }
