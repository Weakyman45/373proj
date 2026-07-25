#ifndef BUZZER_H
#define BUZZER_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

void Buzzer_Init(GPIO_TypeDef *port, uint16_t pin);
void Buzzer_SetMuted(bool muted);
bool Buzzer_IsMuted(void);
void Buzzer_Start(uint8_t pulse_count);
void Buzzer_StartTimed(uint8_t pulse_count, uint32_t on_time_ms, uint32_t off_time_ms);
bool Buzzer_IsActive(void);
void Buzzer_Stop(void);
void Buzzer_Task(void);

#endif /* BUZZER_H */
