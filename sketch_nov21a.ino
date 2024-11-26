// --------------------------------------------------------------- Wiring ------------------------------------------------------
// --------------------------------------------------------------  RFID SDA  >> Arduino  A4  ------------------------------------
// --------------------------------------------------------------  RFID SCK  >> Arduino  13 ------------------------------------
// --------------------------------------------------------------  RFID MOSI >> Arduino  11 ------------------------------------
// --------------------------------------------------------------  RFID MISO >> Arduino  12 ------------------------------------
// --------------------------------------------------------------  RFID RQ   >> Arduino  no connection -------------------------
// --------------------------------------------------------------  RFID GND  >> Arduino  GND -----------------------------------
// --------------------------------------------------------------  RFID RST  >> Arduino  9 -------------------------------------
// --------------------------------------------------------------  RFID 3.3V >> Arduino  +3.3V ---------------------------------


// --------------------------------------------------------------  Relay GND  >> Arduino  GND -----------------------------------
// --------------------------------------------------------------  Relay VCC  >> Arduino  5V ------------------------------------
// --------------------------------------------------------------  Relay IN-1 >> Arduino  3 -------------------------------------


// --------------------------------------------------------------  Buzzer +  >> Arduino  2 --------------------------------------
// --------------------------------------------------------------  Buzzer -  >> Arduino  GND ------------------------------------


#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see your RFID reader datasheet
#define SS_PIN          A4         // A4 on Nano is pin 10
#define WAIT_TIME       10000      // Time to wait for tag (milliseconds)
#define MAX_SAVED_TAGS  10         // Maximum number of tags to store
#define BUZZER_PIN      2
#define Relay_PIN       3

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::Uid savedUIDs[MAX_SAVED_TAGS]; 
unsigned int numSavedTags = 0; 

void setup() 
  {
    Serial.begin(9600);            // Initialize serial communication
    SPI.begin();                    // Initialize SPI bus
    mfrc522.PCD_Init();             // Initialize MFRC522 
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(Relay_PIN, OUTPUT);     // Set relay pin as output
    tone(BUZZER_PIN, 500);         // Startup beep
    delay(100);
    noTone(BUZZER_PIN); 
    Serial.println("1: Define new tag");
    Serial.println("2: Use tags");
    Serial.println("3: List of tags"); 
  }

void loop()
  {
    if (Serial.available()) 
      {
        String input = Serial.readStringUntil('\n');
        if (input.length() > 0) 
          {
            int command = input.toInt(); 
            if (command == 1)
              {
                defineNewTag();
              } 
            else if (command == 2) 
              {
                while(1) checkSavedTags(); 
              } 
            else if (command == 3) 
              {
                printSavedTags(); 
              } 
            else 
              {
                Serial.println("Invalid command. Please try again.");
              }
          }
      }
  }

void defineNewTag() 
  {
    bool tagDetected = waitForTag(); 
    if (tagDetected) 
      {
        tone(BUZZER_PIN, 1000); 
        delay(100);
        noTone(BUZZER_PIN);
        if (!isTagSaved(mfrc522.uid)) 
          {
            Serial.println(F("New tag detected and saved."));
            printUID(mfrc522.uid);
            Serial.println();

            savedUIDs[numSavedTags] = mfrc522.uid;
            numSavedTags++;

            Serial.println(F("Done"));
            tone(BUZZER_PIN, 1000); 
            delay(100);
            noTone(BUZZER_PIN);
            delay(50);
            tone(BUZZER_PIN, 1000); 
            delay(100);
            noTone(BUZZER_PIN);
            if (numSavedTags >= MAX_SAVED_TAGS) 
              {
                tone(BUZZER_PIN, 200); 
                delay(300);
                noTone(BUZZER_PIN);    
                Serial.println(F("Maximum number of tags reached."));
              }
          } 
        else 
          {
            Serial.println(F("Tag already saved."));
          }
      } 
    else 
      {
        Serial.println(F("No tag detected."));
      }
  }

bool waitForTag() 
  {
    Serial.println("Waiting for tag...");
    unsigned long startTime = millis(); 
    while (1) //millis() - startTime < WAIT_TIME
      {
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
          {
            delay(50);
            mfrc522.PICC_HaltA(); 
            return true;
          }
      }
    return false;
  }

void printUID(MFRC522::Uid uid) 
  {
    for (byte i = 0; i < uid.size; i++) 
      {
        Serial.print(uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(uid.uidByte[i], HEX);
      }
  }

void checkSavedTags()
  {
    Serial.println("Checking saved tags...");
    for (int i = 0; i < numSavedTags; i++) 
      {
        if (waitForTag()) 
          {
            if (compareUID(savedUIDs[i], mfrc522.uid)) 
              {
                Serial.println("Access granted.");
                digitalWrite(Relay_PIN, HIGH);  // Unlock
                delay(5000);  // Keep unlocked for 5 seconds
                digitalWrite(Relay_PIN, LOW);  // Lock
                return; 
              }
          }
      }
    Serial.println("Access denied. Unknown tag.");
    tone(BUZZER_PIN, 2000);  
    delay(500);
    noTone(BUZZER_PIN);
  }

bool isTagSaved(MFRC522::Uid uid) 
  {
    for (int i = 0; i < numSavedTags; i++) 
      {
        if (compareUID(savedUIDs[i], uid)) return true; 
      }
    return false;
  }

bool compareUID(MFRC522::Uid uid1, MFRC522::Uid uid2) 
  {
    if (uid1.size == uid2.size) 
      {
        for (byte i = 0; i < uid1.size; i++) 
          {
            if (uid1.uidByte[i] != uid2.uidByte[i]) return false;
          }
        return true;
      }
    return false;
  }

void printSavedTags() 
  {
    tone(BUZZER_PIN, 500); 
    delay(100);
    noTone(BUZZER_PIN);
    Serial.println(F("Saved tags:"));
    for (int i = 0; i < numSavedTags; i++) 
      {
        Serial.print("Tag ");
        Serial.print(i + 1);
        Serial.print(": ");
        printUID(savedUIDs[i]);
        Serial.println(); 
      }
  }
