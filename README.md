# Smart Lock with RFID and Relay

This project guides you through building a smart lock system using an Arduino board, an RFID reader, a relay module, and a solenoid lock. The system allows you to control access by using RFID tags to unlock a door.

## Features:

* **RFID Authentication:** Uses RFID tags to authenticate users and control access.
* **Relay Control:**  Activates a relay to unlock the door when a valid RFID tag is detected.
* **Buzzer Feedback:**  Provides audio feedback to indicate access granted or denied.
* **Expandable:** Easily add more RFID tags to the system for multiple users.


## Table of Contents:

  - [Hardware Components](#Hardware-Components)
  - [Software](#Software)
  - [Wiring Diagram](#Wiring-Diagram)
  - [Code](#Code)
  - [How it Works](#How-it-Works)
  - [Applications](#Applications)
  - [Contributing](#Contributing)

## Hardware Components:

* Arduino board (e.g., Arduino Uno)
* MFRC522 RFID Reader
* RFID Tags
* Relay Module
* Solenoid Lock
* Buzzer
* Jumper wires
* Breadboard (optional)

## Software:

* Arduino IDE

## Wiring Diagram:

![Screenshot 2024-12-01 150112](https://github.com/user-attachments/assets/8e5d7a11-ea75-43a9-8555-97eefda89687)


To complete the project, connect the components as follows:

**RFID Reader to Arduino:**
* SDA to A4
* SCK to 13
* MOSI to 11
* MISO to 12
* GND to GND
* RST to 9
* 3.3V to +3.3V
  
**Relay Module to Arduino:**
* GND to GND
* VCC to 5V
* IN-1 to 3

**Buzzer to Arduino:**
 * <+> to 2
 * <-> to GND



## How it Works:

The RFID reader detects an RFID tag and reads its UID. The Arduino then checks if this UID matches any of the saved UIDs. If a match is found, the Arduino activates the relay to unlock the door for 5 seconds. If no match is found, the buzzer sounds to indicate access is denied.


## Code:

The Arduino code for this project is provided below. You can easily modify the code to add more RFID tags or change the behavior of the buzzer and relay.

## Description:


***Include Libraries***

```c++
#include <SPI.h>
#include <MFRC522.h>
```

These lines include the necessary libraries for SPI communication and the MFRC522 RFID reader.

***Define Constants***

```c++
#define RST_PIN         9          // Configurable, see your RFID reader datasheet
#define SS_PIN          A4         // A4 on Nano is pin 10
#define WAIT_TIME       10000      // Time to wait for tag (milliseconds)
#define MAX_SAVED_TAGS  10         // Maximum number of tags to store
#define BUZZER_PIN      2
#define Relay_PIN       3
```
Constants are defined for the RFID reader's reset and select pins, the waiting time for detecting tags, the maximum number of tags to store, and the pin numbers for the buzzer and relay.

***Create MFRC522 Instance***

```c++
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::Uid savedUIDs[MAX_SAVED_TAGS]; 
unsigned int numSavedTags = 0;
```
Here, an instance of the MFRC522 class is created, and an array is initialized to store the UIDs of saved tags, along with a counter for the number of saved tags.

***Setup Function***

```c++

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
```
The setup function initializes serial communication, the SPI bus, and the MFRC522 module. It also sets the buzzer and relay pins as outputs and provides a startup beep. Finally, it prints the menu options to the serial monitor.

***Loop Function***

```c++
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
```
The loop function waits for user input via the serial monitor. Depending on the input, it calls the appropriate function to define a new tag, use tags, or print the list of saved tags.

***Define New Tag Function***

```c++
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
```
This function waits for a tag to be presented. If a new tag is detected, it saves the tag's UID, increments the counter, and provides feedback via the buzzer. If the tag is already saved, it informs the user.

***Wait for Tag Function***

```c++
bool waitForTag() 
  {
    Serial.println("Waiting for tag...");
    while (1) 
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
```
This function waits indefinitely for a new tag to be presented. Once a tag is detected, it returns true.

***Print UID Function***

```c++
void printUID(MFRC522::Uid uid) 
  {
    for (byte i = 0; i < uid.size; i++) 
      {
        Serial.print(uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(uid.uidByte[i], HEX);
      }
  }
```
This function prints the UID of the detected tag to the serial monitor.

***Check Saved Tags Function***

```c++
void checkSavedTags()
  {
    Serial.println("Checking saved tags...");
    if (waitForTag()) 
      {
        if (isTagSaved(mfrc522.uid)) 
          {
            Serial.println("Access granted.");
            digitalWrite(Relay_PIN, HIGH);  // Unlock
            delay(5000);  // Keep unlocked for 5 seconds
            digitalWrite(Relay_PIN, LOW);  // Lock
          } 
        else 
          {
            Serial.println("Access denied. Unknown tag.");
            tone(BUZZER_PIN, 2000);  
            delay(500);
            noTone(BUZZER_PIN);
          }
      }
  }
```
This function continuously checks for tags. If a saved tag is detected, it grants access by unlocking the relay for 5 seconds. If the tag is not recognized, it denies access and activates the buzzer.

***Is Tag Saved Function***

```c++
bool isTagSaved(MFRC522::Uid uid) 
  {
    for (int i = 0; i < numSavedTags; i++) 
      {
        if (compareUID(savedUIDs[i], uid)) return true; 
      }
    return false;
  }
```
This function checks if a detected tag's UID matches any saved UID. If a match is found, it returns true; otherwise, it returns false.

***Compare UID Function***

```c++
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
```
This function compares two UIDs byte by byte to check if they are identical.

***Print Saved Tags Function***

```c++
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
```
This function prints the list of saved tags to the serial monitor and provides a short beep as feedback.





## Applications:

* Home and office security systems
* Access control for restricted areas
* Educational projects involving RFID technology

## Contributing:

Feel free to fork this repository and contribute your own improvements or modifications. 


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Disclaimer**: This code is provided "as is", without warranty of any kind. Use it at your own risk.
