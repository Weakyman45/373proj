#include "debug_log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef *s_huart = NULL;

void DebugLog_Init(UART_HandleTypeDef *huart)
{
    s_huart = huart;
}

void DebugLog_Printf(const char *format, ...)
{
    if (s_huart == NULL || format == NULL) {
        return;
    }

    char buffer[192];
    va_list args;
    va_start(args, format);
    const int count = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (count <= 0) {
        return;
    }

    uint16_t length = (uint16_t)count;
    if (length >= sizeof(buffer)) {
        length = (uint16_t)(sizeof(buffer) - 1U);
    }

    (void)HAL_UART_Transmit(s_huart, (uint8_t *)buffer, length, 100U);
}

void DebugLog_I2CScan(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL) {
        return;
    }

    DebugLog_Printf("I2C scan start\r\n");
    uint8_t found = 0U;

    for (uint8_t address = 1U; address < 0x7FU; ++address) {
        if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(address << 1), 1U, 5U) == HAL_OK) {
            DebugLog_Printf("  found 7-bit address 0x%02X\r\n", address);
            ++found;
        }
    }

    DebugLog_Printf("I2C scan done, %u device(s)\r\n", found);
}
