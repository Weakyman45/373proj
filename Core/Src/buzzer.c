#include "buzzer.h"
#include <stddef.h>

static GPIO_TypeDef *s_port = NULL;
static uint16_t s_pin = 0U;
static bool s_muted = false;
static bool s_output_on = false;
static uint8_t s_pulses_remaining = 0U;
static uint32_t s_next_change_ms = 0U;
static uint32_t s_on_time_ms = 150U;
static uint32_t s_off_time_ms = 150U;

static void output_set(bool on)
{
    s_output_on = on;
    if (s_port != NULL) {
        HAL_GPIO_WritePin(s_port, s_pin, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void Buzzer_Init(GPIO_TypeDef *port, uint16_t pin)
{
    s_port = port;
    s_pin = pin;
    s_muted = false;
    s_pulses_remaining = 0U;
    output_set(false);
}

void Buzzer_SetMuted(bool muted)
{
    s_muted = muted;
    if (s_muted) {
        Buzzer_Stop();
    }
}

bool Buzzer_IsMuted(void)
{
    return s_muted;
}

void Buzzer_Start(uint8_t pulse_count)
{
    Buzzer_StartTimed(pulse_count, 150U, 150U);
}

void Buzzer_StartTimed(uint8_t pulse_count, uint32_t on_time_ms, uint32_t off_time_ms)
{
    if (s_muted || pulse_count == 0U || s_port == NULL) {
        return;
    }

    s_on_time_ms = (on_time_ms == 0U) ? 1U : on_time_ms;
    s_off_time_ms = off_time_ms;
    s_pulses_remaining = pulse_count;
    output_set(true);
    s_next_change_ms = HAL_GetTick() + s_on_time_ms;
}

bool Buzzer_IsActive(void)
{
    return s_pulses_remaining != 0U;
}

void Buzzer_Stop(void)
{
    s_pulses_remaining = 0U;
    output_set(false);
}

void Buzzer_Task(void)
{
    if (s_pulses_remaining == 0U) {
        return;
    }

    const uint32_t now = HAL_GetTick();
    if ((int32_t)(now - s_next_change_ms) < 0) {
        return;
    }

    if (s_output_on) {
        output_set(false);
        --s_pulses_remaining;
        if (s_pulses_remaining == 0U) {
            return;
        }
        s_next_change_ms = now + s_off_time_ms;
    } else {
        output_set(true);
        s_next_change_ms = now + s_on_time_ms;
    }
}
