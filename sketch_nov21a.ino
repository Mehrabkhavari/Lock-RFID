#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9          // Configurable, see your RFID reader datasheet
#define SS_PIN          A4         // A4 on Nano is pin 10
#define WAIT_TIME       10000     // Time to wait for tag (milliseconds)
#define MAX_SAVED_TAGS  10         // Maximum number of tags to store
#define BUZZER_PIN      2
#define Relay_PIN       3

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::Uid savedUIDs[MAX_SAVED_TAGS]; 
unsigned int numSavedTags = 0; 
int tagAccessLevels[MAX_SAVED_TAGS];  // Access levels for each tag

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
        int command = Serial.parseInt(); 
        if (command == 1) 
          {
            bool tagDetected = waitForTag(); 
            if (tagDetected) 
              {
                tone(BUZZER_PIN, 1000); 
                delay(100);
                noTone(BUZZER_PIN);
                if (!isTagSaved(mfrc522.uid)) 
                  {
                    Serial.println("New tag detected and saved.");
                    printUID(mfrc522.uid);
                    Serial.println();
          
                    savedUIDs[numSavedTags] = mfrc522.uid;
          
                    // Get access level for the tag
                    int accessLevel = 0; 
                    bool levelEntered = false;
                    while (!levelEntered) 
                      {
                        Serial.println("Enter access level for the tag (1 for allowed, 0 for denied):");
                        delay(500);
                        while (!Serial.available()) {}
            
                        if (Serial.available() > 0) 
                          {  
                            while (Serial.available() > 0) 
                              { // Clear the buffer
                                Serial.read();
                              }
                            // <-- Check if there's input
                            accessLevel = Serial.parseInt();
                            if (accessLevel == 0 || accessLevel == 1) 
                              {
                                levelEntered = true;
                              } 
                            else 
                              {
                                Serial.println("Invalid access level. Please enter 0 or 1.");
                              }
                          } 
                      }
          
                    Serial.print("Access level entered: ");
                    Serial.println(accessLevel);
                    tagAccessLevels[numSavedTags] = accessLevel;
                    numSavedTags++;
          
                    Serial.println("Done");
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
                        Serial.println("Maximum number of tags reached.");
                      }
                  } 
                else 
                  {
                    Serial.println("Tag already saved.");
                    // ... (buzzer feedback)
                  }
              }
            else 
              {
                Serial.println("No tag detected.");
              }
          } 
        else if (command == 2) 
          {
            checkSavedTags(); 
          } 
        else if (command == 3) 
          {
            printSavedTags(); 
          }
      }
  }

bool waitForTag() 
  {
    Serial.println("Waiting for tag...");
    unsigned long startTime = millis(); 
    while (millis() - startTime < WAIT_TIME) 
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
                if (tagAccessLevels[i] == 1) 
                  { 
                    Serial.println("Access granted.");
                    digitalWrite(Relay_PIN, HIGH);  // Unlock
                    delay(5000);                   // Keep unlocked for 5 seconds
                    digitalWrite(Relay_PIN, LOW);   // Lock
                  } 
                else 
                  {
                    Serial.println("Access denied.");
                    tone(BUZZER_PIN, 2000);  
                    delay(500);
                    noTone(BUZZER_PIN);
                  }
                return; 
              }
          }
      }
    Serial.println("Unknown tag.");
    // ... (buzzer feedback for unknown tag)
  }

bool isTagSaved(MFRC522::Uid uid) 
  {
    for (int i = 0; i < numSavedTags; i++) 
      {
        if (compareUID(savedUIDs[i], uid)) 
          {
            return true; 
          }
      }
    return false;
  }

bool compareUID(MFRC522::Uid uid1, MFRC522::Uid uid2) 
  {
    if (uid1.size == uid2.size) 
      {
        for (byte i = 0; i < uid1.size; i++) 
          {
            if (uid1.uidByte[i] != uid2.uidByte[i]) 
              {
                return false;
              }
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
    Serial.println("Saved tags:");
    for (int i = 0; i < numSavedTags; i++) 
      {
        Serial.print("Tag ");
        Serial.print(i + 1);
        Serial.print(": ");
        printUID(savedUIDs[i]);
        Serial.print(" - Access Level: ");
        Serial.println(tagAccessLevels[i]); 
      }
  }
