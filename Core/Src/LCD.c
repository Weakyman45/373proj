/************************************************************************
 * LCD_fixed.c
 * 1602A / HD44780-compatible LCD driver for STM32F103, 8-bit mode.
 * Assumption: LCD DB0..DB7 are connected to PA0..PA7.
 ************************************************************************/
#include "LCD.h"
#include "main.h"

#define LCD_DATA_PORT          GPIOA
#define LCD_DATA_MASK          0x00FFU

#define LCD_FUNCTION_8BIT_2LINE 0x38U
#define LCD_DISPLAY_OFF         0x08U
#define LCD_DISPLAY_ON          0x0CU
#define LCD_CLEAR_DISPLAY       0x01U
#define LCD_ENTRY_MODE          0x06U
#define LCD_RETURN_HOME         0x02U

static void LCD_Write8(uint8_t value)
{
    /* Preserve PA8..PA15, update only PA0..PA7. */
    LCD_DATA_PORT->ODR = (LCD_DATA_PORT->ODR & ~LCD_DATA_MASK) | (uint32_t)value;
}

static void LCD_EnablePulse(void)
{
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
    for (volatile uint32_t i = 0; i < 80; ++i) { __NOP(); }
    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
    for (volatile uint32_t i = 0; i < 80; ++i) { __NOP(); }
}

static void LCD_Send(uint8_t value, GPIO_PinState rs)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs);

#ifdef LCD_RW_Pin
    /* Write mode. If RW is physically tied to GND, this line is harmless if
       the CubeMX label exists; otherwise remove the LCD_RW GPIO label/code. */
    HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET);
#endif

    LCD_Write8(value);
    LCD_EnablePulse();

    /* We are not reading the busy flag, so use safe fixed delays. */
    if (value == LCD_CLEAR_DISPLAY || value == LCD_RETURN_HOME) {
        HAL_Delay(2);
    } else {
        HAL_Delay(1);
    }
}

void LCD_init(void)
{
    /* LCD needs time after power becomes stable. */
    HAL_Delay(40);

    HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
#ifdef LCD_RW_Pin
    HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET);
#endif

    /* Standard 8-bit initialization sequence. */
    LCD_Write_Command(LCD_FUNCTION_8BIT_2LINE);
    HAL_Delay(5);
    LCD_Write_Command(LCD_FUNCTION_8BIT_2LINE);
    HAL_Delay(1);
    LCD_Write_Command(LCD_FUNCTION_8BIT_2LINE);
    HAL_Delay(1);

    LCD_Write_Command(LCD_DISPLAY_OFF);
    LCD_Clear();
    LCD_Write_Command(LCD_ENTRY_MODE);
    LCD_Write_Command(LCD_DISPLAY_ON);
}

void LCD_Clear(void)
{
    LCD_Write_Command(LCD_CLEAR_DISPLAY);
    HAL_Delay(2);
}

void LCD_Write_Command(uchar com)
{
    LCD_Send((uint8_t)com, GPIO_PIN_RESET);  /* RS = 0: instruction */
}

void LCD_Write_Data(uchar dat)
{
    LCD_Send((uint8_t)dat, GPIO_PIN_SET);    /* RS = 1: display data */
}

uchar LCD_Read_State(void)
{
    /* This lab configuration normally does not read the LCD busy flag. */
    return 0;
}

void LCD_Set_Position(uchar x, uchar y)
{
    if (x > 15U) x = 15U;

    if (y == 0U) {
        LCD_Write_Command((uchar)(0x80U + x));       /* line 1: DDRAM 0x00..0x0F */
    } else {
        LCD_Write_Command((uchar)(0x80U + 0x40U + x)); /* line 2: DDRAM 0x40..0x4F */
    }
}

void LCD_Display_Char(uchar ch, uchar x, uchar y)
{
    LCD_Set_Position(x, y);
    LCD_Write_Data(ch);
}

void LCD_Display_String(uchar x, uchar y, const char *str)
{
    LCD_Set_Position(x, y);

    while (*str != '\0' && x < 16U) {
        LCD_Write_Data((uchar)*str);
        ++str;
        ++x;
    }
}
