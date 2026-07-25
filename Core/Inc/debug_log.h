#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include "stm32f1xx_hal.h"

void DebugLog_Init(UART_HandleTypeDef *huart);
void DebugLog_Printf(const char *format, ...);
void DebugLog_I2CScan(I2C_HandleTypeDef *hi2c);

#endif /* DEBUG_LOG_H */
