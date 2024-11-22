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
#include <MFRC522.h> // For RFID
#include <Wire.h> // For OLED
#include <Adafruit_GFX.h> // For OLED
#include <Adafruit_SSD1306.h> // For SPI
#include <Adafruit_SH1106.h> // For I2C

#define RST_PIN         9          // Configurable, see your RFID reader datasheet
#define SS_PIN          A4         // A4 on Nano is pin 10
#define WAIT_TIME       10000     // Time to wait for tag (milliseconds)
#define MAX_SAVED_TAGS  5         // Maximum number of tags to store
#define BUZZER_PIN      2
#define Relay_PIN       3

#define Up_PIN      7
#define Dn_PIN      8
#define En_PIN      6

//////////////////////For OLED/////////////////////////////////////////
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI  4
#define OLED_CLK  5 
#define OLED_DC    A1
#define OLED_CS    A2
#define OLED_RESET A0
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#define NUMFLAKES     20 // Number of snowflakes in the animation example
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16


#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif
///////////////////////////////////////////////////////////////////////

//////////////For RFID/////////////////////////////////////////
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::Uid savedUIDs[MAX_SAVED_TAGS]; 
unsigned int numSavedTags = 0; 
int tagAccessCounts[MAX_SAVED_TAGS];  // Allowed access counts for each tag
///////////////////////////////////////////////////////////////////////



void setup()
  {
    Serial.begin(9600);            // Initialize serial communication

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC)) 
      {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
      }

    SPI.begin();                    // Initialize SPI bus
    mfrc522.PCD_Init();             // Initialize MFRC522 

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(Relay_PIN, OUTPUT);     // Set relay pin as output
    pinMode(Up_PIN, INPUT_PULLUP);
    pinMode(Dn_PIN, INPUT_PULLUP);
    pinMode(En_PIN, INPUT_PULLUP);

    tone(BUZZER_PIN, 500);         // Startup beep
    delay(100);
    noTone(BUZZER_PIN); 
    /*Serial.println(F("1: Define new tag"));
    Serial.println(F("2: Use tags"));
    Serial.println(F("3: List of tags")); */

    // Clear the buffer.
    display.clearDisplay();
    display.display();

    // Display static text
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 9);
    display.println(F("Please select one"));
    display.println(F("1: Define new tag"));
    display.println(F("2: Use tags"));
    display.println(F("3: List of tags")); 

    
    
  }
int position = 1;
int flag = 0;
void menue()
  {
    if(!digitalRead(Up_PIN) && position > 1) position--, delay(50), flag = 1; // Invert lines
    if(!digitalRead(Dn_PIN) && position < 3) position++, delay(50), flag = 1; // Invert lines 
    if(!digitalRead(En_PIN))
    {
      
    }

    if(position == 1 && flag == 1)invertLines(16, 24); // Invert lines
    else if(position == 2 && flag == 1)invertLines(24, 32); // Invert lines
    else if(position == 3 && flag == 1)invertLines(32, 40); // Invert lines
    flag = 0;
    display.display();
  }

void loop()
  {
    menue();
  }


void action()
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
                    Serial.println(F("New tag detected and saved."));
                    printUID(mfrc522.uid);
                    Serial.println();

                    savedUIDs[numSavedTags] = mfrc522.uid;

                    // Get allowed access count for the tag
                    int accessCount = 0;
                    bool countEntered = false;
                    while (!countEntered) 
                      {
                        Serial.println(F("Enter the number of allowed accesses for this tag:"));
                        delay(10);
                        while (!Serial.available()) {} 
                        accessCount = Serial.parseInt();
                        if (accessCount > 0) 
                          {
                            countEntered = true;
                          } 
                        else 
                          {
                            Serial.println(F("Invalid access count. Please enter a number greater than 0."));
                          }
                      }

                    Serial.print(F("Access count entered: "));
                    Serial.println(accessCount);
                    tagAccessCounts[numSavedTags] = accessCount;
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

void invertLines(int startLine, int endLine) 
  {
    for (int y = startLine; y <= endLine; y++) 
      {
        for (int x = 0; x < SCREEN_WIDTH; x++) 
          {
            int color = display.getPixel(x, y);
            if (color == WHITE) 
              {
                display.drawPixel(x, y, BLACK);
              }
            else
              {
                display.drawPixel(x, y, WHITE);
              }
          }
      }
    display.display(); 
  }

bool waitForTag() 
  {
    Serial.println(F("Waiting for tag..."));
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
    Serial.println(F("Checking saved tags..."));
    for (int i = 0; i < numSavedTags; i++)
      {
        if (waitForTag()) 
          {
            if (compareUID(savedUIDs[i], mfrc522.uid)) 
              {
                if (tagAccessCounts[i] > 0) 
                  { 
                    Serial.println(F("Access granted."));
                    tagAccessCounts[i]--; // Decrement the access count
                    digitalWrite(Relay_PIN, HIGH);  // Unlock
                    delay(5000);                   // Keep unlocked for 5 seconds
                    digitalWrite(Relay_PIN, LOW);   // Lock
                    Serial.print(F("Remaining access count: "));
                    Serial.println(tagAccessCounts[i]);
                  } 
                else 
                  {
                    Serial.println(F("Access denied. No remaining access count."));
                    tone(BUZZER_PIN, 2000);  
                    delay(500);
                    noTone(BUZZER_PIN);
                  }
                return; 
              }
          }
      }
    Serial.println(F("Unknown tag."));
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
    Serial.println(F("Saved tags:"));
    for (int i = 0; i < numSavedTags; i++) 
      {
        Serial.print("Tag ");
        Serial.print(i + 1);
        Serial.print(": ");
        printUID(savedUIDs[i]);
        Serial.print(F(" - Remaining Access Count: "));
        Serial.println(tagAccessCounts[i]); 
      }
  }
