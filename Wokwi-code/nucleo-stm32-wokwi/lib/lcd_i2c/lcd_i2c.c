#include "lcd_i2c.h"
#include <stdint.h>
#include <string.h>

#define I2C_LCD_ADDR 0x27<<1
static bool send_data(LCD_HandleTypeDef *lcd_handle, char a);
static bool send_nibble(LCD_HandleTypeDef *lcd_handle, uint8_t nibble, uint8_t RS);

static bool send_data(LCD_HandleTypeDef *lcd_handle, char a)
{
    uint8_t nibble[2] = {(a & 0xF0)>>4, a};
    uint8_t RS = 1;
    uint8_t count = 0;
    for (uint8_t i = 0; i < 2; i ++){
        if (send_nibble(lcd_handle, nibble[i], RS)){
        count++;
        }
    }
    if (count == 2){
        return true;
    }
    return false;
}

static bool send_nibble(LCD_HandleTypeDef *lcd_handle, uint8_t nibble, uint8_t RS)
{
    nibble = nibble << 4;
    uint8_t lowerbits[2] = {0};
    if (RS == 1){
        lowerbits[0] = 0b1101;
        lowerbits[1] = 0b1001;
    }
    else{
        lowerbits[0] = 0b1100;
        lowerbits[1] = 0b1000;
    }
    uint8_t data_send[2] = {nibble | lowerbits[0], nibble | lowerbits[1]};
    bool ok1 = (HAL_I2C_Master_Transmit(lcd_handle->i2c_handle, I2C_LCD_ADDR, &data_send[0], 1, HAL_MAX_DELAY) == HAL_OK);
    HAL_Delay(1);  // only here — the gap between E=1 and E=0
    bool ok2 = (HAL_I2C_Master_Transmit(lcd_handle->i2c_handle, I2C_LCD_ADDR, &data_send[1], 1, HAL_MAX_DELAY) == HAL_OK);
    return ok1 && ok2;
}

void LCD_init(LCD_HandleTypeDef *lcd_handle)
{
    HAL_Delay(45);
    send_nibble(lcd_handle, 0x3, 0);
    HAL_Delay(5);
    send_nibble(lcd_handle, 0x3, 0);
    HAL_Delay(1);
    send_nibble(lcd_handle, 0x3, 0);
    send_nibble(lcd_handle, 0x2, 0);
    send_nibble(lcd_handle, 0x2, 0);
    send_nibble(lcd_handle, 0x8, 0);
    send_nibble(lcd_handle, 0x0, 0);
    send_nibble(lcd_handle, 0x8, 0);
    send_nibble(lcd_handle, 0x0, 0);
    send_nibble(lcd_handle, 0x1, 0);
    HAL_Delay(2);
    send_nibble(lcd_handle, 0x0, 0);
    send_nibble(lcd_handle, 0x6, 0);
    send_nibble(lcd_handle, 0x0, 0);
    send_nibble(lcd_handle, 0xC, 0);
}

bool LCD_print_msg(LCD_HandleTypeDef* lcd_handle, char *msg)
{
    for (uint8_t i = 0; i < strlen(msg); i++){
    if (send_data(lcd_handle, msg[i]) != true){
      return false;
    }
    }
    return true;
}

bool LCD_clear(LCD_HandleTypeDef *lcd_handle)
{
    send_nibble(lcd_handle, 0x0, 0);
    send_nibble(lcd_handle, 0x1, 0);
    HAL_Delay(2);   
    return true;
}

void LCD_new_line_cur(LCD_HandleTypeDef *lcd_handle, uint8_t line_num, uint8_t cur_pos)
{
    /*
    | Line | Start address | Command byte (0x80 | address) |
    |---|---|---|
    | 1 | 0x00 | 0x80 |
    | 2 | 0x40 | 0xC0 |
    | 3 | 0x14 | 0x94 |
    | 4 | 0x54 | 0xD4 |
    */
    switch (line_num) {
        case 1:
            send_nibble(lcd_handle, 0x8, 0);
            send_nibble(lcd_handle, 0x0 + cur_pos, 0);
            break;
        case 2:
            send_nibble(lcd_handle, 0xC, 0);
            send_nibble(lcd_handle, 0x0 + cur_pos, 0);
            break;
        case 3:
            send_nibble(lcd_handle, 0x9, 0);
            send_nibble(lcd_handle, 0x4 + cur_pos, 0);
            break;
        case 4:
            send_nibble(lcd_handle, 0xD, 0);
            send_nibble(lcd_handle, 0x4 + cur_pos, 0);
            break;
    }
    HAL_Delay(2);   
}