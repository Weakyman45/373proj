#include "ssd1306.h"
#include <string.h>

static I2C_HandleTypeDef *s_hi2c = NULL;
static uint16_t s_i2c_address = SSD1306_I2C_ADDRESS;
static uint8_t s_buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8U];

static const uint8_t s_digits[10][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* 0 */
    {0x00, 0x42, 0x7F, 0x40, 0x00}, /* 1 */
    {0x42, 0x61, 0x51, 0x49, 0x46}, /* 2 */
    {0x21, 0x41, 0x45, 0x4B, 0x31}, /* 3 */
    {0x18, 0x14, 0x12, 0x7F, 0x10}, /* 4 */
    {0x27, 0x45, 0x45, 0x45, 0x39}, /* 5 */
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* 6 */
    {0x01, 0x71, 0x09, 0x05, 0x03}, /* 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */
    {0x06, 0x49, 0x49, 0x29, 0x1E}  /* 9 */
};

static const uint8_t s_uppercase[26][5] = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, /* A */
    {0x7F, 0x49, 0x49, 0x49, 0x36}, /* B */
    {0x3E, 0x41, 0x41, 0x41, 0x22}, /* C */
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, /* D */
    {0x7F, 0x49, 0x49, 0x49, 0x41}, /* E */
    {0x7F, 0x09, 0x09, 0x09, 0x01}, /* F */
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, /* G */
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, /* H */
    {0x00, 0x41, 0x7F, 0x41, 0x00}, /* I */
    {0x20, 0x40, 0x41, 0x3F, 0x01}, /* J */
    {0x7F, 0x08, 0x14, 0x22, 0x41}, /* K */
    {0x7F, 0x40, 0x40, 0x40, 0x40}, /* L */
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, /* M */
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, /* N */
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, /* O */
    {0x7F, 0x09, 0x09, 0x09, 0x06}, /* P */
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, /* Q */
    {0x7F, 0x09, 0x19, 0x29, 0x46}, /* R */
    {0x46, 0x49, 0x49, 0x49, 0x31}, /* S */
    {0x01, 0x01, 0x7F, 0x01, 0x01}, /* T */
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, /* U */
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, /* V */
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, /* W */
    {0x63, 0x14, 0x08, 0x14, 0x63}, /* X */
    {0x07, 0x08, 0x70, 0x08, 0x07}, /* Y */
    {0x61, 0x51, 0x49, 0x45, 0x43}  /* Z */
};

static bool write_command(uint8_t command)
{
    uint8_t packet[2] = {0x00U, command};
    return HAL_I2C_Master_Transmit(s_hi2c, s_i2c_address,
                                    packet, sizeof(packet), 100U) == HAL_OK;
}

static void get_glyph(char ch, uint8_t glyph[5])
{
    memset(glyph, 0, 5U);

    if (ch >= 'a' && ch <= 'z') {
        ch = (char)(ch - 'a' + 'A');
    }

    if (ch >= '0' && ch <= '9') {
        memcpy(glyph, s_digits[(uint8_t)(ch - '0')], 5U);
        return;
    }

    if (ch >= 'A' && ch <= 'Z') {
        memcpy(glyph, s_uppercase[(uint8_t)(ch - 'A')], 5U);
        return;
    }

    switch (ch) {
    case ' ': break;
    case '!': { const uint8_t g[5] = {0x00, 0x00, 0x5F, 0x00, 0x00}; memcpy(glyph, g, 5U); } break;
    case '%': { const uint8_t g[5] = {0x62, 0x64, 0x08, 0x13, 0x23}; memcpy(glyph, g, 5U); } break;
    case '+': { const uint8_t g[5] = {0x08, 0x08, 0x3E, 0x08, 0x08}; memcpy(glyph, g, 5U); } break;
    case '-': { const uint8_t g[5] = {0x08, 0x08, 0x08, 0x08, 0x08}; memcpy(glyph, g, 5U); } break;
    case '.': { const uint8_t g[5] = {0x00, 0x60, 0x60, 0x00, 0x00}; memcpy(glyph, g, 5U); } break;
    case '/': { const uint8_t g[5] = {0x20, 0x10, 0x08, 0x04, 0x02}; memcpy(glyph, g, 5U); } break;
    case ':': { const uint8_t g[5] = {0x00, 0x36, 0x36, 0x00, 0x00}; memcpy(glyph, g, 5U); } break;
    case '?': { const uint8_t g[5] = {0x02, 0x01, 0x51, 0x09, 0x06}; memcpy(glyph, g, 5U); } break;
    case '_': { const uint8_t g[5] = {0x40, 0x40, 0x40, 0x40, 0x40}; memcpy(glyph, g, 5U); } break;
    default:  { const uint8_t g[5] = {0x02, 0x01, 0x51, 0x09, 0x06}; memcpy(glyph, g, 5U); } break;
    }
}

bool SSD1306_Init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL) {
        return false;
    }

    s_hi2c = hi2c;
    HAL_Delay(100U);

    const uint16_t candidate_addresses[] = {
        SSD1306_I2C_ADDRESS,
        (0x3CU << 1),
        (0x3DU << 1)
    };

    bool device_found = false;
    for (uint32_t i = 0U; i < (sizeof(candidate_addresses) / sizeof(candidate_addresses[0])); ++i) {
        if (i > 0U && candidate_addresses[i] == candidate_addresses[0]) {
            continue;
        }
        if (HAL_I2C_IsDeviceReady(s_hi2c, candidate_addresses[i], 3U, 100U) == HAL_OK) {
            s_i2c_address = candidate_addresses[i];
            device_found = true;
            break;
        }
    }

    if (!device_found) {
        return false;
    }

    const uint8_t sequence[] = {
        0xAE,       /* display off */
        0xD5, 0x80, /* clock divide */
        0xA8, 0x3F, /* multiplex 1/64 */
        0xD3, 0x00, /* display offset */
        0x40,       /* start line 0 */
        0xAD, 0x8B, /* SH1106 internal DC-DC on; ignored by SSD1306 */
        0x8D, 0x14, /* charge pump on */
        0x20, 0x02, /* page addressing mode */
        0xA1,       /* segment remap */
        0xC8,       /* COM scan direction remapped */
        0xDA, 0x12, /* COM pins */
        0x81, 0xCF, /* contrast */
        0xD9, 0xF1, /* pre-charge */
        0xDB, 0x40, /* VCOM detect */
        0xA4,       /* display follows RAM */
        0xA6,       /* normal display */
        0x2E,       /* scroll off */
        0xAF        /* display on */
    };

    for (uint32_t i = 0U; i < sizeof(sequence); ++i) {
        if (!write_command(sequence[i])) {
            return false;
        }
    }

    SSD1306_Fill(false);
    return SSD1306_UpdateScreen();
}

void SSD1306_Fill(bool on)
{
    memset(s_buffer, on ? 0xFF : 0x00, sizeof(s_buffer));
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, bool on)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return;
    }

    const uint16_t index = (uint16_t)x + ((uint16_t)y / 8U) * SSD1306_WIDTH;
    const uint8_t mask = (uint8_t)(1U << (y & 7U));

    if (on) {
        s_buffer[index] |= mask;
    } else {
        s_buffer[index] &= (uint8_t)~mask;
    }
}

void SSD1306_DrawChar(uint8_t x, uint8_t y, char ch, uint8_t scale)
{
    if (scale == 0U) {
        scale = 1U;
    }

    uint8_t glyph[5];
    get_glyph(ch, glyph);

    for (uint8_t col = 0U; col < 6U; ++col) {
        const uint8_t bits = (col < 5U) ? glyph[col] : 0U;
        for (uint8_t row = 0U; row < 7U; ++row) {
            const bool pixel_on = ((bits >> row) & 0x01U) != 0U;
            for (uint8_t dx = 0U; dx < scale; ++dx) {
                for (uint8_t dy = 0U; dy < scale; ++dy) {
                    const uint16_t px = (uint16_t)x + (uint16_t)col * scale + dx;
                    const uint16_t py = (uint16_t)y + (uint16_t)row * scale + dy;
                    if (px < SSD1306_WIDTH && py < SSD1306_HEIGHT) {
                        SSD1306_DrawPixel((uint8_t)px, (uint8_t)py, pixel_on);
                    }
                }
            }
        }
    }
}

void SSD1306_DrawString(uint8_t x, uint8_t y, const char *text, uint8_t scale)
{
    if (text == NULL) {
        return;
    }

    uint16_t cursor_x = x;
    while (*text != '\0') {
        if (cursor_x + (uint16_t)(6U * scale) > SSD1306_WIDTH) {
            break;
        }
        SSD1306_DrawChar((uint8_t)cursor_x, y, *text, scale);
        cursor_x += (uint16_t)(6U * scale);
        ++text;
    }
}

bool SSD1306_UpdateScreen(void)
{
    if (s_hi2c == NULL) {
        return false;
    }

    uint8_t packet[SSD1306_WIDTH + 1U];
    packet[0] = 0x40U;

    for (uint8_t page = 0U; page < 8U; ++page) {
        if (!write_command((uint8_t)(0xB0U + page)) ||
            !write_command(0x02U) ||
            !write_command(0x10U)) {
            return false;
        }

        memcpy(&packet[1], &s_buffer[(uint16_t)page * SSD1306_WIDTH], SSD1306_WIDTH);
        if (HAL_I2C_Master_Transmit(s_hi2c, s_i2c_address,
                                    packet, sizeof(packet), 150U) != HAL_OK) {
            return false;
        }
    }

    return true;
}

bool SSD1306_SetInvert(bool invert)
{
    if (s_hi2c == NULL) {
        return false;
    }
    return write_command(invert ? 0xA7U : 0xA6U);
}
