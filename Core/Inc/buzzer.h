#ifndef BUZZER_H
#define BUZZER_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

void Buzzer_Init(GPIO_TypeDef *port, uint16_t pin);
void Buzzer_SetMuted(bool muted);
bool Buzzer_IsMuted(void);
void Buzzer_Start(uint8_t pulse_count);
void Buzzer_Stop(void);
void Buzzer_Task(void);

#endif /* BUZZER_H */
