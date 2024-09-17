/* Includes --------------------------------------------------------------------*/
#include "NanoEdgeAI.h"
#include "knowledge.h"
#include "libneai.a"
#include <ETH.h>
#include <ArduinoIoTCloud.h>
#include <WiFiConnectionManager.h> // Required for Arduino IoT Cloud, but will not be used
#include <thingProperties.h> // Required for Arduino IoT Cloud
#include "stm32f4xx_hal.h" // Include HAL library for STM32F4 series


#define MAX30100_ADDRESS 0x57 // I2C address of the MAX30100 sensor
#define MAX30100_REG_PARTICLE_COUNT 0x03 // Example register for particle count
#define MAX30100_REG_SPO2 0x04

I2C_HandleTypeDef hi2c1;
/* Number of samples for learning: set by user ---------------------------------*/
#define LEARNING_ITERATIONS 256
#define AXIS_NUMBER 9
float input_user_buffer[DATA_INPUT_USER * AXIS_NUMBER]; // Buffer of input values

float manualBP = 120.0; // Example value, update as needed
float manualBMI = 22.0;
// Define your sensor reading functions (placeholders)
void I2C_Init() {
    __HAL_RCC_I2C1_CLK_ENABLE();
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000; // 100 kHz
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

void MAX30100_Init() {
    // Reset the MAX30100
    uint8_t reset_cmd = 0x40; // Command to reset
    HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDRESS, 0x09, I2C_MEMADD_SIZE_8BIT, &reset_cmd, 1, 1000);
    HAL_Delay(100); // Wait for reset

    // Configure MAX30100 (e.g., sample rate, LED current)
    uint8_t config[] = {0x07, 0x24}; // Example configuration
    HAL_I2C_Mem_Write(&hi2c1, MAX30100_ADDRESS, 0x06, I2C_MEMADD_SIZE_8BIT, config, sizeof(config), 1000);
}

float readHeartRate() {
  // Implement your heart rate sensor reading logic here
	uint8_t data[6]; // Read 6 bytes from the sensor
	    if (HAL_I2C_Mem_Read(&hi2c1, MAX30100_ADDRESS, MAX30100_REG_PARTICLE_COUNT, I2C_MEMADD_SIZE_8BIT, data, sizeof(data), 1000) == HAL_OK) {
	        // Process the raw data to compute heart rate
	        // This is an example; replace with your actual computation logic
	        uint16_t rawHeartRate = (data[0] << 8) | data[1];
	        return (float)rawHeartRate; // Example conversion
	    }
	    return 0.0;
}

float readSpO2() {
  // Implement your SpO2 sensor reading logic here
	uint8_t data[6]; // Read 6 bytes from the sensor
	    if (HAL_I2C_Mem_Read(&hi2c1, MAX30100_ADDRESS, MAX30100_REG_SPO2, I2C_MEMADD_SIZE_8BIT, data, sizeof(data), 1000) == HAL_OK) {
	        // Process the raw data to compute SpO2
	        // This is an example; replace with your actual computation logic
	        uint16_t rawSpO2 = (data[0] << 8) | data[1];
	        return (float)rawSpO2; // Example conversion
	    }
	    return 0.0; // Return 0 if I2C read failed
}

float readTemperature() {
  // Implement your temperature sensor reading logic here
  return 36.5; // Example value
}

float readBP() {
  // Implement your BP reading logic here
  return 120.0; // Example value (manual input)
}
float calculatePulsePressure(float systolicBP, float diastolicBP) {
    return systolicBP - diastolicBP;
}

float calculateMAP(float systolicBP, float diastolicBP) {
    return diastolicBP + (systolicBP - diastolicBP) / 3;
}

float calculateBMI(float weight, float height) {
    return weight / (height * height);
}


void fill_buffer(float input_buffer[]) {
  for (int i = 0; i < DATA_INPUT_USER; i++) {
    // Fill buffer with sensor data
	   float heartRate = readHeartRate();
	   float spo2 = readSpO2();
	   float temperature = readTemperature();
	   float bp = readBP(); // Manual input
	   float pulsePressure = calculatePulsePressure(bp, 80.0); // Example diastolic value
	   float map = calculateMAP(bp, 80.0); // Example diastolic value
	   float bmi = calculateBMI(70.0, 1.75); // Example weight and height

	   input_buffer[i * AXIS_NUMBER] = heartRate;
	           input_buffer[i * AXIS_NUMBER + 1] = spo2;
	           input_buffer[i * AXIS_NUMBER + 2] = temperature;
	           input_buffer[i * AXIS_NUMBER + 3] = pulsePressure;
	           input_buffer[i * AXIS_NUMBER + 4] = map;
	           input_buffer[i * AXIS_NUMBER + 5] = bmi;
	           input_buffer[i * AXIS_NUMBER + 6] = manualBP; // Example of including BP manually
	           input_buffer[i * AXIS_NUMBER + 7] = 0.0; // Placeholder for unused axis values
	           input_buffer[i * AXIS_NUMBER + 8] = 0.0; // Placeholder for unused axis values
    // Continue filling buffer as needed
  }
}

// Function to set up Ethernet connection
void setupEthernet() {
  // Initialize Ethernet (make sure ETH.begin() is appropriate for your hardware)
  ETH.begin(); // This might need adjustment based on the Ethernet library you use
  HAL_Delay(1000); // Give it some time to initialize
  Serial.println("Ethernet initialized");

  // Optionally print the IP address assigned
  Serial.print("IP Address: ");
  Serial.println(Ethernet.localIP());
}

void SystemClock_Config(void) {
  // Implement your system clock configuration here, based on STM32F4 settings
}

int main(void) {
  // Initialization ------------------------------------------------------------
  HAL_Init();
  SystemClock_Config(); // Initialize system clock

  // Initialize serial communication
  Serial.begin(115200);

  // Initialize Ethernet
  setupEthernet();

  // Initialize serial communication
  HAL_UART_Init(); // Ensure you have properly initialized UART

  // Initialize I2C
  I2C_Init();

      // Initialize MAX30100 sensor
  MAX30100_Init();


  // Initialize Arduino IoT Cloud
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // NanoEdge AI initialization
  enum neai_state error_code = neai_anomalydetection_init();
  uint8_t similarity = 0;

  if (error_code != NEAI_OK) {
    // Handle initialization error
    Serial.println("NanoEdge AI initialization failed");
    while (1); // Halt on error
  }

  // Learning process ----------------------------------------------------------
  for (uint16_t iteration = 0; iteration < LEARNING_ITERATIONS; iteration++) {
    fill_buffer(input_user_buffer);
    neai_anomalydetection_learn(input_user_buffer);
  }

  // Main loop ------------------------------------------------------------------
  while (1) {
    // Update Arduino IoT Cloud
    ArduinoCloud.update();
    HAL_Init();
    I2C_Init();
    MAX30100_Init();


    // Read sensor values
    float heartRate = readHeartRate();
    float spo2 = readSpO2();
    float temperature = readTemperature();
    float bp = readBP(); // Manual input
    float pulsePressure = calculatePulsePressure(bp, 80.0); // Example diastolic value
    float map = calculateMAP(bp, 80.0); // Example diastolic value
    float bmi = calculateBMI(70.0, 1.75); // Example weight and height

    // Update the cloud properties
    heartRateValue = heartRate;
    spo2Value = spo2;
    temperatureValue = temperature;
    bpValue = bp;
    pulsePressureValue = pulsePressure; // Ensure this property is defined in Arduino IoT Cloud
    mapValue = map; // Ensure this property is defined in Arduino IoT Cloud
    bmiValue = bmi;


    // Detection process
    fill_buffer(input_user_buffer);
    neai_anomalydetection_detect(input_user_buffer, &similarity);

    // Example of handling detection results
    if (similarity > SOME_THRESHOLD) {
      // Trigger actions based on similarity
      Serial.println("Anomaly detected!");
      // e.g., blink LED, ring alarm, etc.
    }
    while (1) {
            float heartRate = readHeartRate();
            float spo2 = readSpO2();

            // Print sensor values
                    Serial.printf("Heart Rate: %.2f bpm \n", heartRate);
                    Serial.printf("SpO2: %.2f\n", spo2);
                    Serial.printf("Temperature: %.2f C\n", temperature);
                    Serial.printf("BP: %.2f mmHg\n", bp);
                    Serial.printf("Pulse Pressure: %.2f mmHg\n", pulsePressure);
                    Serial.printf("MAP: %.2f mmHg\n", map);
                    Serial.printf("BMI: %.2f\n", bmi);
            // Wait before the next update
            HAL_Delay(10000); // 10 seconds
    }
    }
}
