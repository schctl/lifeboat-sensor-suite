#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000
#define INTERRUPT_PIN 6 // Change this to the pin connected to the INT pin of the MAX30100

PulseOximeter pox;

// Interrupt service routine
void ICACHE_RAM_ATTR handleInterrupt() {
    // Handle the interrupt by reading data from MAX30100
    pox.update();
}


// Time at which the last beat occurred
uint32_t tsLastReport = 0;

// Callback routine is executed when a pulse is detected
void onBeatDetected() {
    Serial.println("♥ Beat!");
}

void setup() {
    Serial.begin(115200);

    Serial.print("Initializing pulse oximeter..");

    // Initialize sensor
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

  // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

    // Register a callback routine
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
    // Read from the sensor
    pox.update();

    // Grab the updated heart rate and SpO2 levels
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");

        Serial.swap();
        Serial.print(pox.getHeartRate());
        Serial.print(",");
        Serial.print(35);
        Serial.print(",");
        Serial.print(pox.getSpO2());
        Serial.print(",");
        Serial.println("131,78,68,0.0,53,28.82106941,95.66666667");

        tsLastReport = millis();
    }
}
