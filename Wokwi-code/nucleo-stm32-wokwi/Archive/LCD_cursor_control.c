/* Summary
In this program I simple ask the user to press the button on the lcd
the lcd then displays the button count
the lcd library was updated for finer cursor control
the lcd cursor now can be placed anywhere according to the need
*/

#include "Legacy/stm32_hal_legacy.h"
#include "stm32c031xx.h"
#include "stm32c0xx_hal.h"
#include "stm32c0xx_hal_cortex.h"
#include "stm32c0xx_hal_def.h"
#include "stm32c0xx_hal_gpio.h"
#include "stm32c0xx_hal_gpio_ex.h"
#include "stm32c0xx_hal_i2c.h"
#include "stm32c0xx_hal_rcc.h"
#include "stm32c0xx_hal_uart.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/_intsup.h>
#include "lcd_i2c.h"

#define I2C_LCD_ADDR 0x27<<1
UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c;
LCD_HandleTypeDef hlcd;
volatile bool btn_press = false;
static uint32_t last_tick = 0;
static uint32_t tick = 0;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
void Error_Handler(void);
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin);
void EXTI4_15_IRQHandler(void);
bool btn_pressed(void);

void SysTick_Handler(void)
{
  HAL_IncTick();
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_I2C1_Init();
    hlcd.i2c_handle = &hi2c;
    LCD_init(&hlcd);
    char msg[] = "Press Button";
    char msg2[] = "Count";
    uint8_t count = 1;
    char count_char[1];
    
    LCD_print_msg(&hlcd, msg);
    while (1)
    { 
        HAL_UART_Transmit(&huart2, (uint8_t *)"Test output\n", strlen("Test output\n"), HAL_MAX_DELAY);
        HAL_Delay(1000);
        if (btn_pressed()){
          HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
          HAL_UART_Transmit(&huart2, (uint8_t *)"Button Pressed\n", strlen("Button Pressed\n"), HAL_MAX_DELAY);
          LCD_new_line_cur(&hlcd,3,0);
          LCD_print_msg(&hlcd, msg2);
          LCD_new_line_cur(&hlcd, 3, strlen(msg2)+1);
          itoa(count, count_char, 10);
          LCD_print_msg(&hlcd, count_char);
          count++; 
        }
    }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (uartHandle->Instance == USART2)
  {
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

void MX_I2C1_Init(void){
  hi2c.Instance = I2C1;
  hi2c.Init.Timing = 0x10805D88;
  hi2c.Init.OwnAddress1 = 0;
  hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c.Init.OwnAddress2 = 0;
  hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c) != HAL_OK){
    Error_Handler();
  }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_I2C1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_I2C1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

void EXTI4_15_IRQHandler(){
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin){
  btn_press = true;
  tick = HAL_GetTick();
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
}

bool btn_pressed(){
  if (tick - last_tick > 500){
    if (btn_press == true){
      btn_press = false;
      last_tick = tick;
      return true;
    }
  }
  btn_press = false;
  return false;
}