#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

const int SDAPIN   = 10;
const int RSTPIN   =  9;

MFRC522 rfidReader(SDAPIN, RSTPIN);

const int LED_R   = 5;
const int LED_G = 6;
const int SERVOPIN = 3;
Servo door;

enum State {CLOSED, OPEN};

State servo_state;

void open() {
    Serial.println("Servo to OPEN position.");
    door.write(120);
    servo_state = OPEN;
}

void close() {
    Serial.println("Servo to CLOSED position.");
    door.write(0);
    servo_state = CLOSED;
}

void signalLocked() {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);
}

void signalUnlocked() {
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, HIGH);
}

void signalReading() {
    for (int i = 0; i < 20; i++) {
        digitalWrite(LED_G, HIGH);
        digitalWrite(LED_R, LOW);
        delay(20);
        digitalWrite(LED_R, HIGH);
        digitalWrite(LED_G, LOW);
        delay(20);
    }
    digitalWrite(LED_R, LOW);
}

void signalGranted() {
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_G, HIGH);
        delay(100);
        digitalWrite(LED_G, LOW);
        delay(100);
    }
}

void signalDenied() {
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_R, HIGH);
        delay(100);
        digitalWrite(LED_R, LOW);
        delay(100);
    }
}

void sendCorrectSignal() {
    if ( servo_state == CLOSED ) {
        signalLocked();
    }
    else {
        signalUnlocked();
    }
}

void setup() {
    door.attach(SERVOPIN);
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(SERVOPIN, OUTPUT);

    Serial.begin(115200);

    SPI.begin();

    rfidReader.PCD_Init(); // Initialize. RFID-reader

    // initial position is closed
    close();
    sendCorrectSignal();
}

void loop() {

    if ( ! rfidReader.PICC_IsNewCardPresent()) {
        return;
    }

    Serial.println("Reading card");
    signalReading();
    if ( ! rfidReader.PICC_ReadCardSerial()) {
        Serial.println("Card removed");
        signalDenied();
        sendCorrectSignal();
        return;
    }

    // If the rfid chip is still present after signalReading()
    // this part of the code is reached

    if ( servo_state == CLOSED) {
        open();
        signalGranted();
    }
    else {
        close();
        signalDenied();
    }
    sendCorrectSignal();
    // Leave some time to remove the chip.
    delay(5000);
}
