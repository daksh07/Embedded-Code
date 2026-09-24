#include "stm32c031xx.h"
#include "stm32c0xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    I2C_HandleTypeDef *i2c_handle;
} LCD_HandleTypeDef;

void LCD_init(LCD_HandleTypeDef *lcd_handle);
bool LCD_print_msg(LCD_HandleTypeDef *lcd_handle, char *msg);
bool LCD_clear(LCD_HandleTypeDef *lcd_handle);
void LCD_new_line_cur(LCD_HandleTypeDef *lcd_handle, uint8_t line_num, uint8_t cur_pos);

