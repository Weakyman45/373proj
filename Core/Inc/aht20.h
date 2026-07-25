#ifndef AHT20_H
#define AHT20_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

#define AHT20_I2C_ADDRESS (0x38U << 1)

bool AHT20_Init(I2C_HandleTypeDef *hi2c);
bool AHT20_Read(float *temperature_c, float *humidity_percent);

#endif /* AHT20_H */
