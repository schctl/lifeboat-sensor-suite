#include <stdint.h>
#include <string.h>

UART mySerial(digitalPinToPinName(3), digitalPinToPinName(2));

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);

  mySerial.begin(9600);
  while (!mySerial);

}

void loop() {
  // put your main code here, to run repeatedly:

    for (int i = 0; i < 10; i++) {

      mySerial.println("180,41,0,115,60,84,0,60,1.8,62,24,93.66");
      delay(1);
    }

    delay(500);

    for (int i = 0; i < 10; i++) {

      mySerial.println("180,41,0,115,60,84,0,60,1.8,62,24,93.66");
      delay(1);
    }

    if (mySerial.available()) {
      String read = mySerial.readStringUntil('\n');
      read.trim();
      Serial.println(read);
    }
  
  delay(550);
}
