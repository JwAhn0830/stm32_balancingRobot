/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>
#include <string.h>
#include "mpu6050.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

MPU6050_config config;
MPU6050 mpu6050;
float aceelAngle_X;
float gyroAngle_X;
float X_angle;

float previousAngle = 0;
float angle = 0;
float setPoint = 0;
/*-------------------------------------------------------------------------------------*/
float bat = 0;

float MAX_SPEED = 999;
float deadzone = 540;//540;

float p, i, d;
float kp = 25;//20
float ki = 0;//0.08
float kd = 0.4;//0.7 max

float error;
float previousError;
int pidOutput = 0;

char message[100];
// test parameters;
//float test_aceelAngle_X;
//float test_gyroAngle_X;
//int test_count_tim = 0;
//int test_count_main = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void sendUart(char *ptr) {
	HAL_UART_Transmit_DMA(&huart3, (uint8_t *)ptr, strlen(ptr));
}
float getVoltage() {
	float adcValue;
	float voltage;
	float totalVoltage;

    HAL_ADC_Start(&hadc2);
    HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY);
    HAL_ADC_Stop(&hadc2);

	adcValue = HAL_ADC_GetValue(&hadc2);
	voltage = adcValue * 0.000837;
	totalVoltage = voltage * 2.5;

	return totalVoltage;
}

float getAngle() {
	readAccel_MPU6050(&hi2c1, &mpu6050);
	readGyro_MPU6050(&hi2c1, &mpu6050);
	aceelAngle_X = atan((mpu6050.Ay)/(sqrt((pow(mpu6050.Ax, 2) + pow(mpu6050.Az, 2))))) * (180 / 3.14);
	gyroAngle_X = (mpu6050.Gx * 0.001);
	X_angle = (0.95 * (previousAngle + (gyroAngle_X * 0.001))) + (0.05 * aceelAngle_X);
	previousAngle = X_angle;
	return X_angle;
}

void go() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, 1);

	htim3.Instance->CCR1 = pidOutput;
	htim3.Instance->CCR2 = pidOutput;
}

void back() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, 1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, 0);

	htim3.Instance->CCR1 = pidOutput;
	htim3.Instance->CCR2 = pidOutput;
}

void stop() {
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, 0);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, 0);
}

void pid() {
	 error = angle - setPoint;

	 p = kp * error;
	 i = i + (ki * error * 0.001);
	 d = kd * ((error - previousError) / 0.001);
	 previousError = error;
	 pidOutput = p + i + d;

	 if (pidOutput > 0)
		 pidOutput += deadzone;
	 else if (pidOutput < 0)
		 pidOutput -= deadzone;

	 if (pidOutput > MAX_SPEED)
		 pidOutput = MAX_SPEED;
	 else if (pidOutput < -MAX_SPEED)
		 pidOutput = -MAX_SPEED;

	 if (pidOutput < 0)
	     pidOutput *= -1;

	 if(error > 0)
		 go();
	 else if(error < 0)
		 back();
	 else //(error == 0)
		 stop();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// 1ms (1kHz)
	if (htim->Instance == TIM2) {
		angle = getAngle();
		pid();
	}
	// 1s
//	if (htim->Instance == TIM1) {
//		bat = getVoltage();
//		sprintf(message, "{\"bat\": %.2f, \"angle\": %.2f, \"setPoint\": %.2f}\r\n", bat, angle, setPoint);
//		sendUart(message);
//		test_count_tim++;
//	}
}

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
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC2_Init();
  MX_USB_PCD_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //ENA
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //ENB
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); //Buzzer
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0);
  config_MPU6050_DEFAULT(&config);

  TIM4->PSC = 2000;
  HAL_Delay(100);
  TIM4->PSC = 1500;
  HAL_Delay(100);
  TIM4->PSC = 1000;
  HAL_Delay(100);

  HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1); //Buzzer

  while(1){
	  if (init_MPU6050(&hi2c1, &config) == MPU6050_OK){
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1);
		  break;
	  }
  }

  for (int i = 0; i< 100; i ++) {
	  setPoint += getAngle();
  }
  setPoint = setPoint / 100;

  HAL_TIM_Base_Start_IT(&htim2); //timer interrupt 1ms
  HAL_TIM_Base_Start_IT(&htim1); //timer interrupt 1s
  go();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
//	  if (bat < 6) {
//		  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
//	  }
//	  else
//		  HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
