#include "studymate_app.h"
#include "studymate_config.h"
#include "ssd1306.h"
#include "aht20.h"
#include "debug_log.h"
#include "buzzer.h"
#include "main.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    MODE_FOCUS = 0,
    MODE_SHORT_BREAK,
    MODE_LONG_BREAK
} TimerMode;

typedef enum {
    STATE_IDLE = 0,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_ALARM
} AppState;

typedef enum {
    ENV_ERROR = 0,
    ENV_OK,
    ENV_COLD,
    ENV_HOT,
    ENV_DRY,
    ENV_HUMID
} EnvironmentState;

static I2C_HandleTypeDef *s_hi2c = NULL;
static UART_HandleTypeDef *s_huart = NULL;

static TimerMode s_mode = MODE_FOCUS;
static AppState s_state = STATE_IDLE;
static EnvironmentState s_environment = ENV_ERROR;
static EnvironmentState s_previous_environment = ENV_ERROR;

static uint32_t s_remaining_seconds = STUDYMATE_FOCUS_SECONDS;
static uint8_t s_sensor_second_divider = 0U;

static float s_temperature_c = 0.0f;
static float s_humidity_percent = 0.0f;
static bool s_sensor_valid = false;
static bool s_sensor_ready = false;
static bool s_oled_ready = false;
static bool s_ui_dirty = true;

static bool s_display_inverted = false;
static uint32_t s_alarm_flash_deadline_ms = 0U;

static volatile uint8_t s_pending_seconds = 0U;
static volatile uint8_t s_event_start = 0U;
static volatile uint8_t s_event_mode = 0U;
static volatile uint8_t s_event_reset = 0U;
static volatile uint8_t s_event_mute = 0U;

static uint32_t s_last_start_ms = 0U;
static uint32_t s_last_mode_ms = 0U;
static uint32_t s_last_reset_ms = 0U;
static uint32_t s_last_mute_ms = 0U;

static const char *mode_name(TimerMode mode)
{
    switch (mode) {
    case MODE_FOCUS:       return "FOCUS";
    case MODE_SHORT_BREAK: return "SHORT";
    case MODE_LONG_BREAK:  return "LONG";
    default:               return "?";
    }
}

static const char *state_name(AppState state)
{
    switch (state) {
    case STATE_IDLE:    return "IDLE";
    case STATE_RUNNING: return "RUN";
    case STATE_PAUSED:  return "PAUSE";
    case STATE_ALARM:   return "ALARM";
    default:            return "?";
    }
}

static const char *environment_name(EnvironmentState environment)
{
    switch (environment) {
    case ENV_OK:    return "OK";
    case ENV_COLD:  return "COLD";
    case ENV_HOT:   return "HOT";
    case ENV_DRY:   return "DRY";
    case ENV_HUMID: return "HUMID";
    case ENV_ERROR:
    default:        return "ERR";
    }
}

static uint32_t mode_duration_seconds(TimerMode mode)
{
    switch (mode) {
    case MODE_FOCUS:       return STUDYMATE_FOCUS_SECONDS;
    case MODE_SHORT_BREAK: return STUDYMATE_SHORT_BREAK_SECONDS;
    case MODE_LONG_BREAK:  return STUDYMATE_LONG_BREAK_SECONDS;
    default:               return STUDYMATE_FOCUS_SECONDS;
    }
}

static EnvironmentState classify_environment(bool valid, float temperature_c, float humidity_percent)
{
    if (!valid) {
        return ENV_ERROR;
    }
    if (temperature_c < STUDYMATE_TEMP_LOW_C) {
        return ENV_COLD;
    }
    if (temperature_c > STUDYMATE_TEMP_HIGH_C) {
        return ENV_HOT;
    }
    if (humidity_percent < STUDYMATE_HUMIDITY_LOW_PERCENT) {
        return ENV_DRY;
    }
    if (humidity_percent > STUDYMATE_HUMIDITY_HIGH_PERCENT) {
        return ENV_HUMID;
    }
    return ENV_OK;
}

static void restore_normal_display(void)
{
    if (s_oled_ready && s_display_inverted) {
        (void)SSD1306_SetInvert(false);
        s_display_inverted = false;
    }
}

static void reset_current_mode(void)
{
    Buzzer_Stop();
    restore_normal_display();
    s_state = STATE_IDLE;
    s_remaining_seconds = mode_duration_seconds(s_mode);
    s_ui_dirty = true;
}

static void enter_alarm(void)
{
    s_state = STATE_ALARM;
    s_remaining_seconds = 0U;
    s_alarm_flash_deadline_ms = HAL_GetTick() + 500U;
    s_display_inverted = false;
    Buzzer_Start(3U);
    s_ui_dirty = true;

    DebugLog_Printf("EVENT timer finished, mode=%s\r\n", mode_name(s_mode));
}

static void process_button_events(void)
{
    if (s_event_reset != 0U) {
        s_event_reset = 0U;
        reset_current_mode();
        DebugLog_Printf("BUTTON K3 reset\r\n");
    }

    if (s_event_mode != 0U) {
        s_event_mode = 0U;
        if (s_state == STATE_IDLE) {
            s_mode = (TimerMode)(((uint8_t)s_mode + 1U) % 3U);
            s_remaining_seconds = mode_duration_seconds(s_mode);
            s_ui_dirty = true;
            DebugLog_Printf("BUTTON K2 mode=%s\r\n", mode_name(s_mode));
        } else {
            DebugLog_Printf("BUTTON K2 ignored because state=%s\r\n", state_name(s_state));
        }
    }

    if (s_event_start != 0U) {
        s_event_start = 0U;

        switch (s_state) {
        case STATE_IDLE:
            if (s_remaining_seconds == 0U) {
                s_remaining_seconds = mode_duration_seconds(s_mode);
            }
            s_state = STATE_RUNNING;
            break;
        case STATE_RUNNING:
            s_state = STATE_PAUSED;
            break;
        case STATE_PAUSED:
            s_state = STATE_RUNNING;
            break;
        case STATE_ALARM:
            reset_current_mode();
            break;
        default:
            reset_current_mode();
            break;
        }

        s_ui_dirty = true;
        DebugLog_Printf("BUTTON K1 state=%s\r\n", state_name(s_state));
    }

    if (s_event_mute != 0U) {
        s_event_mute = 0U;
        Buzzer_SetMuted(!Buzzer_IsMuted());
        s_ui_dirty = true;
        DebugLog_Printf("BUTTON K4 sound=%s\r\n", Buzzer_IsMuted() ? "OFF" : "ON");
    }
}

static void process_one_second(void)
{
    if (s_state == STATE_RUNNING) {
        if (s_remaining_seconds > 0U) {
            --s_remaining_seconds;
        }

        DebugLog_Printf("TIMER %s %02lu:%02lu\r\n",
                        mode_name(s_mode),
                        (unsigned long)(s_remaining_seconds / 60U),
                        (unsigned long)(s_remaining_seconds % 60U));

        if (s_remaining_seconds == 0U) {
            enter_alarm();
        }
    }

    ++s_sensor_second_divider;
    if (s_sensor_second_divider >= STUDYMATE_SENSOR_PERIOD_SECONDS) {
        s_sensor_second_divider = 0U;
        s_sensor_ready = s_sensor_ready || AHT20_Init(s_hi2c);
        s_sensor_valid = s_sensor_ready && AHT20_Read(&s_temperature_c, &s_humidity_percent);

        s_previous_environment = s_environment;
        s_environment = classify_environment(s_sensor_valid, s_temperature_c, s_humidity_percent);

        if (s_previous_environment == ENV_OK &&
            s_environment != ENV_OK &&
            s_environment != ENV_ERROR &&
            s_state != STATE_ALARM) {
            Buzzer_Start(1U);
            DebugLog_Printf("WARNING comfort changed to %s\r\n", environment_name(s_environment));
        }

        if (s_sensor_valid) {
            const int32_t t10 = (int32_t)(s_temperature_c * 10.0f
                                        + ((s_temperature_c >= 0.0f) ? 0.5f : -0.5f));
            const int32_t h10 = (int32_t)(s_humidity_percent * 10.0f + 0.5f);
            DebugLog_Printf("SENSOR T=%ld.%ldC H=%ld.%ld%% comfort=%s\r\n",
                            (long)(t10 / 10),
                            (long)((t10 < 0 ? -t10 : t10) % 10),
                            (long)(h10 / 10),
                            (long)(h10 % 10),
                            environment_name(s_environment));
        } else {
            DebugLog_Printf("SENSOR read error\r\n");
        }
    }

    s_ui_dirty = true;
}

static void draw_environment_line(void)
{
    char line[24];

    if (s_sensor_valid) {
        const int32_t t10 = (int32_t)(s_temperature_c * 10.0f
                                    + ((s_temperature_c >= 0.0f) ? 0.5f : -0.5f));
        const int32_t abs_t10 = (t10 < 0) ? -t10 : t10;
        const int32_t humidity = (int32_t)(s_humidity_percent + 0.5f);

        (void)snprintf(line, sizeof(line), "T:%s%ld.%ldC H:%ld%%",
                       (t10 < 0) ? "-" : "",
                       (long)(abs_t10 / 10),
                       (long)(abs_t10 % 10),
                       (long)humidity);
    } else {
        (void)snprintf(line, sizeof(line), "T:--.-C H:--%%");
    }

    SSD1306_DrawString(0U, 30U, line, 1U);
}

static void update_oled(void)
{
    if (!s_oled_ready) {
        return;
    }

    SSD1306_Fill(false);

    if (s_state == STATE_ALARM) {
        SSD1306_DrawString(18U, 0U, "*** TIME UP ***", 1U);
        SSD1306_DrawString(34U, 12U, "00:00", 2U);
        SSD1306_DrawString(20U, 32U, "SESSION FINISH", 1U);
        SSD1306_DrawString(0U, 43U, "K1 ACK", 1U);
        SSD1306_DrawString(0U, 54U, "K3 RESET", 1U);
        SSD1306_DrawString(78U, 54U, Buzzer_IsMuted() ? "MUTE" : "BEEP", 1U);
    } else {
        char line[24];
        (void)snprintf(line, sizeof(line), "%s %s", mode_name(s_mode), state_name(s_state));
        SSD1306_DrawString(0U, 0U, line, 1U);

        (void)snprintf(line, sizeof(line), "%02lu:%02lu",
                       (unsigned long)(s_remaining_seconds / 60U),
                       (unsigned long)(s_remaining_seconds % 60U));
        SSD1306_DrawString(34U, 10U, line, 2U);

        draw_environment_line();

        (void)snprintf(line, sizeof(line), "COMFORT:%s", environment_name(s_environment));
        SSD1306_DrawString(0U, 40U, line, 1U);

        (void)snprintf(line, sizeof(line), "SOUND:%s", Buzzer_IsMuted() ? "OFF" : "ON");
        SSD1306_DrawString(0U, 49U, line, 1U);
        SSD1306_DrawString(0U, 57U, "K1 GO K2 MODE", 1U);
    }

    if (!SSD1306_UpdateScreen()) {
        s_oled_ready = false;
        DebugLog_Printf("OLED update failed\r\n");
    }
}

static void alarm_visual_task(void)
{
    if (!s_oled_ready) {
        return;
    }

    if (s_state != STATE_ALARM) {
        restore_normal_display();
        return;
    }

    const uint32_t now = HAL_GetTick();
    if ((int32_t)(now - s_alarm_flash_deadline_ms) >= 0) {
        s_display_inverted = !s_display_inverted;
        (void)SSD1306_SetInvert(s_display_inverted);
        s_alarm_flash_deadline_ms = now + 500U;
    }
}

void StudyMate_Init(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart)
{
    s_hi2c = hi2c;
    s_huart = huart;

    DebugLog_Init(s_huart);
    Buzzer_Init(BUZZER_GPIO_Port, BUZZER_Pin);

    DebugLog_Printf("\r\nStudyMate boot\r\n");
#if STUDYMATE_DEMO_MODE
    DebugLog_Printf("Timing profile: DEMO\r\n");
#else
    DebugLog_Printf("Timing profile: REAL POMODORO\r\n");
#endif

    DebugLog_I2CScan(s_hi2c);

    s_oled_ready = SSD1306_Init(s_hi2c);
    DebugLog_Printf("OLED %s, expected 7-bit address 0x%02X\r\n",
                    s_oled_ready ? "OK" : "ERROR",
                    (unsigned int)(SSD1306_I2C_ADDRESS >> 1));

    s_sensor_ready = AHT20_Init(s_hi2c);
    DebugLog_Printf("AHT20 %s, expected 7-bit address 0x38\r\n",
                    s_sensor_ready ? "OK" : "ERROR");

    s_mode = MODE_FOCUS;
    s_state = STATE_IDLE;
    s_remaining_seconds = mode_duration_seconds(s_mode);
    s_environment = ENV_ERROR;
    s_previous_environment = ENV_ERROR;
    s_sensor_second_divider = STUDYMATE_SENSOR_PERIOD_SECONDS - 1U;
    s_ui_dirty = true;

    if (s_oled_ready) {
        SSD1306_Fill(false);
        SSD1306_DrawString(10U, 14U, "STUDYMATE", 2U);
        SSD1306_DrawString(28U, 39U, "OLED+AHT20", 1U);
        (void)SSD1306_UpdateScreen();
        HAL_Delay(700U);
    }

    DebugLog_Printf("Controls: K1 start/pause, K2 mode, K3 reset, K4 mute\r\n");
}

void StudyMate_Task(void)
{
    process_button_events();

    bool process_tick = false;
    __disable_irq();
    if (s_pending_seconds > 0U) {
        --s_pending_seconds;
        process_tick = true;
    }
    __enable_irq();

    if (process_tick) {
        process_one_second();
    }

    Buzzer_Task();
    alarm_visual_task();

    if (s_ui_dirty) {
        s_ui_dirty = false;
        update_oled();
    }
}

void StudyMate_On10msTick(void)
{
    static uint8_t tick_divider = 0U;

    ++tick_divider;
    if (tick_divider >= 100U) {
        tick_divider = 0U;
        if (s_pending_seconds < 255U) {
            ++s_pending_seconds;
        }
    }
}

static bool debounce_accept(uint32_t now, uint32_t *last_time)
{
    if (*last_time == 0U || (uint32_t)(now - *last_time) >= STUDYMATE_BUTTON_DEBOUNCE_MS) {
        *last_time = now;
        return true;
    }
    return false;
}

void StudyMate_OnExti(uint16_t gpio_pin)
{
    const uint32_t now = HAL_GetTick();

    if (gpio_pin == BTN_START_Pin && debounce_accept(now, &s_last_start_ms)) {
        s_event_start = 1U;
    } else if (gpio_pin == BTN_MODE_Pin && debounce_accept(now, &s_last_mode_ms)) {
        s_event_mode = 1U;
    } else if (gpio_pin == BTN_RESET_Pin && debounce_accept(now, &s_last_reset_ms)) {
        s_event_reset = 1U;
    } else if (gpio_pin == BTN_MUTE_Pin && debounce_accept(now, &s_last_mute_ms)) {
        s_event_mute = 1U;
    }
}
