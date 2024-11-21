#include <MFRC522.h>
#include <SPI.h>

#define RST_PIN         9      // پین RST ماژول RFID
#define SS_PIN          A4     // پین SS ماژول RFID (A4 on Nano is pin 10) 
#define WAIT_TIME       10000  // مدت زمان انتظار برای شناسایی تگ به میلی‌ثانیه
#define MAX_SAVED_TAGS  10     // تعداد حداکثر تگ‌های ذخیره شده
#define BUZZER_PIN      2

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::Uid savedUIDs[MAX_SAVED_TAGS]; // ذخیره UID تگ‌های قبلی
unsigned int numSavedTags = 0; // تعداد تگ‌های ذخیره شده
int tagQuantities[MAX_SAVED_TAGS]; // موجودی تگ‌ها

void setup()
  {
    Serial.begin(9600);   // Initiate a serial communication
    SPI.begin();          // Initiate  SPI bus
    mfrc522.PCD_Init();   // Initiate MFRC522
    pinMode(BUZZER_PIN, OUTPUT);
    tone(BUZZER_PIN, 500); // Startup beep
    delay(100);
    noTone(BUZZER_PIN); 
    Serial.println("1: Define new tage");
    Serial.println("2: Use tages");
    Serial.println("1: List of tages");
  }

void loop()
  {
    
    if (Serial.available()) 
      {
        int command = Serial.parseInt(); // دریافت عدد از سریال
        if (command == 1) 
          {
            bool tagDetected = waitForTag(); // منتظر شناسایی تگ بمان
            if (tagDetected) 
              {
                tone(BUZZER_PIN, 1000); // Short beep to indicate tag read
                delay(100);
                noTone(BUZZER_PIN);
                if (!isTagSaved(mfrc522.uid)) 
                  {
                    Serial.println("New tag detected and saved.");
                    printUID(mfrc522.uid);
                    Serial.println();

                    // ذخیره UID تگ جدید
                    savedUIDs[numSavedTags] = mfrc522.uid; 
                    int quantity = 0;
                    bool quantityEntered = false;
                    while (!quantityEntered) 
                      {
                        Serial.println("Enter quantity for the tag:");
                        delay(10);
                        while (!Serial.available()) 
                          {
                            // منتظر بمان تا ورودی از کاربر در دسترس باشد
                          }
                        quantity = Serial.parseInt();
                        if (quantity > 0) 
                          { // Ensure quantity is greater than zero
                            quantityEntered = true;
                          } 
                        else 
                          {
                            Serial.println("Please enter a valid quantity (greater than 0).");
                            tone(BUZZER_PIN, 1000); // Short beep to indicate tag read
                            delay(1000);
                            noTone(BUZZER_PIN);
                          }
                      }

                    Serial.print("Quantity entered: ");
                    Serial.println(quantity);
                    tagQuantities[numSavedTags] = quantity; // ذخیره موجودی تگ
                    numSavedTags++;

                    Serial.println("Done");
                    tone(BUZZER_PIN, 1000); // Short beep to indicate tag read
                    delay(100);
                    noTone(BUZZER_PIN);
                    delay(50);
                    tone(BUZZER_PIN, 1000); // Short beep to indicate tag read
                    delay(100);
                    noTone(BUZZER_PIN);
                    // اضافه کردن کارت و مقدار وارد شده به لیست
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
                    tone(BUZZER_PIN, 1000);
                    delay(100);
                    noTone(BUZZER_PIN); 
                    delay(20);
                    tone(BUZZER_PIN, 1000);
                    delay(100);
                    noTone(BUZZER_PIN); 
                    delay(20);
                    tone(BUZZER_PIN, 1000);
                    delay(100);
                    noTone(BUZZER_PIN); 
                    delay(20);
                  }
              } 
            else 
              {
                Serial.println("No tag detected.");
              }
          }
        else if (command == 2)// بررسی تگ‌های ذخیره شده
          {
            checkSavedTags(); 
          }
        else if (command == 3)// نمایش تگ‌های ذخیره شده
          {
            printSavedTags(); 
          }
      }
  }

bool waitForTag() 
  {
    Serial.println("Waiting for tag...");
    unsigned long startTime = millis(); // زمان شروع انتظار را ذخیره کن
    while (millis() - startTime < WAIT_TIME) 
      {
        if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
          {
            delay(50);
            mfrc522.PICC_HaltA(); // متوقف کردن تگ برای جلوگیری از تداخل با تگ‌های بعدی
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
                Serial.print("Tag ");
                Serial.print(i + 1);
                Serial.print(": ");
                printUID(savedUIDs[i]);
                Serial.print(" - Quantity: ");
                Serial.println(tagQuantities[i]); // نمایش موجودی تگ
                tone(BUZZER_PIN, 2000); 
                delay(200);
                noTone(BUZZER_PIN); 
                decreaseTagQuantity(mfrc522.uid);
                return; // توقف حلقه بلافاصله پس از یافتن یک تطابق
              }
          }
      }
    Serial.println("Unknown tag.");
    //delay(100);
    tone(BUZZER_PIN, 2000); 
    delay(200);
    noTone(BUZZER_PIN);
    delay(100);
    tone(BUZZER_PIN, 2000); 
    delay(200);
    noTone(BUZZER_PIN);

  }

bool isTagSaved(MFRC522::Uid uid) 
  {
    for (int i = 0; i < numSavedTags; i++)
      {
        if (compareUID(savedUIDs[i], uid)) 
          {
            return true; // اگر یک تطابق پیدا شود، متوقف شود و درست بازگردانده شود
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
        Serial.print(" - Quantity: ");
        Serial.println(tagQuantities[i]); // نمایش موجودی تگ
      }
  }

void decreaseTagQuantity(MFRC522::Uid uid) 
  {
    for (int i = 0; i < numSavedTags; i++) 
      {
        if (compareUID(savedUIDs[i], uid)) 
          {
            if (tagQuantities[i] > 0) 
              { // بررسی موجودی قبل از کم کردن
                tagQuantities[i]--;
                Serial.println("Tag quantity decreased successfully.");
                Serial.print("New quantity: ");
                Serial.println(tagQuantities[i]);
              } 
            else 
              {
                Serial.println("Insufficient quantity. Cannot decrease.");
                delay(100);
                tone(BUZZER_PIN, 2000); 
                delay(200);
                noTone(BUZZER_PIN);
                delay(100);
                tone(BUZZER_PIN, 2000); 
                delay(200);
                noTone(BUZZER_PIN);
              }
            return;
          }
      }
    Serial.println("Tag not found.");
  }


