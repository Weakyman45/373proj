#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define SSD1306_WIDTH  128U
#define SSD1306_HEIGHT  64U

#ifndef SSD1306_I2C_ADDRESS
#define SSD1306_I2C_ADDRESS (0x3CU << 1)
#endif

bool SSD1306_Init(I2C_HandleTypeDef *hi2c);
void SSD1306_Fill(bool on);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, bool on);
void SSD1306_DrawChar(uint8_t x, uint8_t y, char ch, uint8_t scale);
void SSD1306_DrawString(uint8_t x, uint8_t y, const char *text, uint8_t scale);
bool SSD1306_UpdateScreen(void);
bool SSD1306_SetInvert(bool invert);

#endif /* SSD1306_H */
