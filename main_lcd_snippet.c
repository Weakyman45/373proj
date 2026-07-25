/* Add near the other includes in main.c */
#include "LCD.h"

/* In main(), call LCD_init() after MX_GPIO_Init(). */
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    LCD_init();
    LCD_Clear();
    LCD_Display_String(0, 0, "Hello, LCD!");
    LCD_Display_String(0, 1, "ECE3730J Lab3");

    while (1)
    {
        /* Nothing required here for a static display. */
    }
}
