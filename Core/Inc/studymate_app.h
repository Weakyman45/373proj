#ifndef STUDYMATE_APP_H
#define STUDYMATE_APP_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

void StudyMate_Init(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart);
void StudyMate_Task(void);
void StudyMate_On10msTick(void);
void StudyMate_OnExti(uint16_t gpio_pin);

#endif /* STUDYMATE_APP_H */
