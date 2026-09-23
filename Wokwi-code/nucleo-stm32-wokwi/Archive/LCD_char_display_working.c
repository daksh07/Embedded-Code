/* Summary
We configure the RCC to internal high speed clock (HSI) 48MHz along with Flash latency 
to match that speed else flash access will be slower and fill memory with junk
First we setup the oscillator
Then we setup all the clocks
Then we setup the I2C GPIO in I2C_MspInit
The I2C itself is setup in MX_I2C1_Init
Then we build the function send_data to send 4 bits long data along
Another send_char function that converts a char into sendable data
Finally LCD_Init to initliase the LCD to receive 4 bits data and turn on properly
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
#include <string.h>
#include <stdbool.h>
#include <sys/_intsup.h>

#define I2C_LCD_ADDR 0x27<<1
UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c;
volatile bool btn_press = false;
static uint32_t last_tick = 0;
static uint32_t tick = 0;
uint8_t char_A[] = {0b01001101, 0b01001001,0b00011101, 0b00011001};

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void LCD_init(void);
void Error_Handler(void);
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin);
void EXTI4_15_IRQHandler(void);
bool btn_pressed(void);
bool send_data(char a);
bool send_nibble(uint8_t nibble, uint8_t RS);

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
    LCD_init();
    char a = 'A';
    char b = 'B';
    send_data(a);
    send_data(b);
    while (1)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *)"Test output\n", strlen("Test output\n"), HAL_MAX_DELAY);
        HAL_Delay(1000);
        if (btn_pressed()){
          HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
          HAL_UART_Transmit(&huart2, (uint8_t *)"Button Pressed\n", strlen("Button Pressed\n"), HAL_MAX_DELAY);
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

/*
 * LCD_init
 * The first four send_nibble() calls are special: the display doesn't yet
 * know it's in 4-bit mode, so these go as standalone nibbles, not pairs.
 * This exact sequence (0x3 x3, then 0x2) resets the controller reliably
 * regardless of its power-on state. RS=0 throughout — these are all commands.
 * Intialisation sequence on page 45 in datasheet
 */
static void LCD_init(void)
{
  HAL_Delay(45);          // Wait >40ms after power-on

  send_nibble(0x3, 0);
  HAL_Delay(5);           // >4.1ms

  send_nibble(0x3, 0);
  HAL_Delay(1);           // >100us

  send_nibble(0x3, 0);

  send_nibble(0x2, 0);    // Switches into 4-bit mode. From here on, every
                           // write is a proper high/low nibble pair.

  // Function Set: 0x28 = 4-bit, 2-line, 5x8 font
  send_nibble(0x2, 0);
  send_nibble(0x8, 0);

  // Display Off: 0x08 (must be off before Display Clear)
  send_nibble(0x0, 0);
  send_nibble(0x8, 0);

  // Display Clear: 0x01
  send_nibble(0x0, 0);
  send_nibble(0x1, 0);
  HAL_Delay(2);            // Slow op — needs >1.6ms

  // Entry Mode Set: 0x06 = auto-increment cursor, no shift
  send_nibble(0x0, 0);
  send_nibble(0x6, 0);

  // Display On: 0x0C = display on, cursor/blink off
  send_nibble(0x0, 0);
  send_nibble(0xC, 0);
}

/*
 * send_data
 * Sends one ASCII character to the data register (RS=1). A char is
 * already a small int in C, so no conversion — just split into nibbles.
 * Returns true only if both nibble sends succeed.
 */
bool send_data(char a)
{
  // High nibble: mask + shift down to bits 0-3 (send_nibble shifts up itself).
  // Low nibble: already sitting in bits 0-3 — no masking needed.
  uint8_t nibble[2] = {(a & 0xF0) >> 4, a};

  uint8_t RS = 1;
  uint8_t count = 0;

  for (uint8_t i = 0; i < 2; i++)
  {
    if (send_nibble(nibble[i], RS))
    {
      count++;
    }
  }

  if (count == 2)
  {
    return true;
  }
  return false;
}

/*
 * send_nibble
 * PCF8574->HD44780 mapping: bit0=RS, bit1=R/W(always 0), bit2=E, bit3=Backlight,
 * bits4-7=data nibble. HD44780 latches on E's falling edge, so one nibble
 * is two transmissions of the same byte with E flipped 1->0 between them.
 * 'nibble' is expected unshifted (bits 0-3); this function shifts it into place.
 */
bool send_nibble(uint8_t nibble, uint8_t RS)
{
  nibble = nibble << 4;

  uint8_t lowerbits[2] = {0};
  if (RS == 1)
  {
    lowerbits[0] = 0b1101;   // RS=1, E=1, Backlight=1
    lowerbits[1] = 0b1001;   // RS=1, E=0, Backlight=1
  }
  else
  {
    lowerbits[0] = 0b1100;   // RS=0, E=1, Backlight=1
    lowerbits[1] = 0b1000;   // RS=0, E=0, Backlight=1
  }

  uint8_t data_send[2] = {nibble | lowerbits[0], nibble | lowerbits[1]};

  bool ok1 = (HAL_I2C_Master_Transmit(&hi2c, I2C_LCD_ADDR, &data_send[0], 1, HAL_MAX_DELAY) == HAL_OK);

  HAL_Delay(1);   // Hold between E=1 and E=0 — overkill but simple for now

  bool ok2 = (HAL_I2C_Master_Transmit(&hi2c, I2C_LCD_ADDR, &data_send[1], 1, HAL_MAX_DELAY) == HAL_OK);

  return ok1 && ok2;
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