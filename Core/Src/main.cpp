/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <main.hpp>
#include "NanoEdgeAI.h"
#include "knowledge.h"
#include "libneai.a"

#define AXIS_NUMBER 9
#define LEARNING_ITERATIONS 256

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "com.hpp"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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

	  // Detection process
	      fill_buffer(input_user_buffer);
	      neai_anomalydetection_detect(input_user_buffer, &similarity);

	      // Example of handling detection results
	      if (similarity > SOME_THRESHOLD) {
	        // Trigger actions based on similarity
	        Serial.println("Anomaly detected!");
	        // e.g., blink LED, ring alarm, etc.
	      }

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */

  Message msg = Message(UserStatus::Ok);
  msg.serialize();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
