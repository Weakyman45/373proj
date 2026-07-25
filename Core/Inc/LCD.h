/************************************************************************
 * LCD_fixed.h
 * 1602A / HD44780-compatible LCD driver for STM32F103, 8-bit mode.
 * Assumption: LCD DB0..DB7 are connected to PA0..PA7.
 ************************************************************************/
#ifndef LCD_H_
#define LCD_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef uint8_t uchar;

void LCD_init(void);
void LCD_Clear(void);

void LCD_Write_Command(uchar com);
void LCD_Write_Data(uchar dat);

uchar LCD_Read_State(void);   /* Not used when RW is tied low. */

void LCD_Set_Position(uchar x, uchar y);
void LCD_Display_Char(uchar ch, uchar x, uchar y);
void LCD_Display_String(uchar x, uchar y, const char *str);

#endif /* LCD_H_ */
