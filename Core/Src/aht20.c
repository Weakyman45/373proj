#include "aht20.h"
#include <stddef.h>

static I2C_HandleTypeDef *s_hi2c = NULL;

bool AHT20_Init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL) {
        return false;
    }

    s_hi2c = hi2c;
    HAL_Delay(40U);

    if (HAL_I2C_IsDeviceReady(s_hi2c, AHT20_I2C_ADDRESS, 3U, 100U) != HAL_OK) {
        return false;
    }

    const uint8_t soft_reset = 0xBAU;
    (void)HAL_I2C_Master_Transmit(s_hi2c, AHT20_I2C_ADDRESS,
                                  (uint8_t *)&soft_reset, 1U, 100U);
    HAL_Delay(20U);

    uint8_t init_command[3] = {0xBEU, 0x08U, 0x00U};
    if (HAL_I2C_Master_Transmit(s_hi2c, AHT20_I2C_ADDRESS,
                                init_command, sizeof(init_command), 100U) != HAL_OK) {
        return false;
    }

    HAL_Delay(10U);
    return true;
}

bool AHT20_Read(float *temperature_c, float *humidity_percent)
{
    if (s_hi2c == NULL || temperature_c == NULL || humidity_percent == NULL) {
        return false;
    }

    uint8_t trigger[3] = {0xACU, 0x33U, 0x00U};
    uint8_t data[6] = {0};

    if (HAL_I2C_Master_Transmit(s_hi2c, AHT20_I2C_ADDRESS,
                                trigger, sizeof(trigger), 100U) != HAL_OK) {
        return false;
    }

    HAL_Delay(85U);

    if (HAL_I2C_Master_Receive(s_hi2c, AHT20_I2C_ADDRESS,
                               data, sizeof(data), 100U) != HAL_OK) {
        return false;
    }

    if ((data[0] & 0x80U) != 0U) {
        return false;
    }

    const uint32_t raw_humidity = ((uint32_t)data[1] << 12)
                                | ((uint32_t)data[2] << 4)
                                | ((uint32_t)data[3] >> 4);

    const uint32_t raw_temperature = (((uint32_t)data[3] & 0x0FU) << 16)
                                   | ((uint32_t)data[4] << 8)
                                   | (uint32_t)data[5];

    const float humidity = ((float)raw_humidity * 100.0f) / 1048576.0f;
    const float temperature = ((float)raw_temperature * 200.0f) / 1048576.0f - 50.0f;

    if (humidity < 0.0f || humidity > 100.0f || temperature < -50.0f || temperature > 100.0f) {
        return false;
    }

    *humidity_percent = humidity;
    *temperature_c = temperature;
    return true;
}
