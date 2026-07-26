/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUZZER_TEST_DURATION_MS 1000U
#define BUZZER_TEST_GAP_MS      1000U
#define BUZZER_TEST_GPIO_PORT GPIOA
#define BUZZER_TEST_PIN       GPIO_PIN_8
#define OLED_TEST_ACTIVE      1U
#define OLED_TEST_HOLD_MS     1200U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
void MX_TIM2_Init(void);
void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static inline void Buzzer_On(void)
{
  HAL_GPIO_WritePin(BUZZER_TEST_GPIO_PORT, BUZZER_TEST_PIN, GPIO_PIN_SET);
}

static inline void Buzzer_Off(void)
{
  HAL_GPIO_WritePin(BUZZER_TEST_GPIO_PORT, BUZZER_TEST_PIN, GPIO_PIN_RESET);
}

static void Buzzer_TestPinInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  HAL_GPIO_WritePin(BUZZER_TEST_GPIO_PORT, BUZZER_TEST_PIN, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = BUZZER_TEST_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_TEST_GPIO_PORT, &GPIO_InitStruct);
}

static void DWT_DelayMs(uint32_t duration_ms)
{
  const uint32_t cycles = (SystemCoreClock / 1000U) * duration_ms;

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  while (DWT->CYCCNT < cycles)
  {
  }
}

static void Buzzer_Beep(uint32_t duration_ms)
{
  Buzzer_On();
  DWT_DelayMs(duration_ms);
  Buzzer_Off();
}

static void OLED_DrawBorder(void)
{
  for (uint8_t x = 0U; x < SSD1306_WIDTH; ++x)
  {
    SSD1306_DrawPixel(x, 0U, true);
    SSD1306_DrawPixel(x, SSD1306_HEIGHT - 1U, true);
  }

  for (uint8_t y = 0U; y < SSD1306_HEIGHT; ++y)
  {
    SSD1306_DrawPixel(0U, y, true);
    SSD1306_DrawPixel(SSD1306_WIDTH - 1U, y, true);
  }
}

static bool OLED_ShowTextTest(void)
{
  SSD1306_Fill(false);
  OLED_DrawBorder();
  SSD1306_DrawString(10U, 6U, "OLED TEST", 2U);
  SSD1306_DrawString(37U, 27U, "SSD1306", 1U);
  SSD1306_DrawString(34U, 39U, "I2C 0X3C", 1U);
  SSD1306_DrawString(31U, 51U, "DISPLAY OK", 1U);
  return SSD1306_UpdateScreen();
}

static bool OLED_ShowPixelTest(void)
{
  SSD1306_Fill(false);

  for (uint8_t y = 0U; y < SSD1306_HEIGHT; ++y)
  {
    for (uint8_t x = 0U; x < SSD1306_WIDTH; ++x)
    {
      if ((((x / 8U) + (y / 8U)) & 1U) == 0U)
      {
        SSD1306_DrawPixel(x, y, true);
      }
    }
  }

  return SSD1306_UpdateScreen();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* Always take exception vectors from this application's Flash image.
     This also makes debug launches independent of the BOOT pin mapping. */
  SCB->VTOR = FLASH_BASE;
  __DSB();
  __ISB();

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
  /* USER CODE BEGIN 2 */
  Buzzer_TestPinInit();

  /* Keep the proven buzzer test available without changing its behavior. */
  if (OLED_TEST_ACTIVE == 0U)
  {
    __disable_irq();
    while (1)
    {
      Buzzer_Beep(BUZZER_TEST_DURATION_MS);
      DWT_DelayMs(BUZZER_TEST_GAP_MS);
    }
  }

  MX_I2C1_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (!SSD1306_Init(&hi2c1))
    {
      HAL_Delay(500U);
      continue;
    }

    while (OLED_ShowTextTest())
    {
      /* A5 lights every pixel without depending on display RAM addressing. */
      if (!SSD1306_SetEntireDisplay(true))
      {
        break;
      }
      HAL_Delay(OLED_TEST_HOLD_MS);

      if (!SSD1306_SetEntireDisplay(false))
      {
        break;
      }
      HAL_Delay(OLED_TEST_HOLD_MS);

      if (!SSD1306_SetInvert(true))
      {
        break;
      }
      HAL_Delay(OLED_TEST_HOLD_MS / 2U);

      if (!SSD1306_SetInvert(false) || !OLED_ShowPixelTest())
      {
        break;
      }
      HAL_Delay(OLED_TEST_HOLD_MS);
    }

    (void)SSD1306_SetInvert(false);
    HAL_Delay(500U);
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

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM2_Init(void)
{
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 99;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
#ifdef USE_FULL_ASSERT
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
