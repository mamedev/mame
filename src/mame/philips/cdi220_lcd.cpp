// license:BSD-3-Clause
// copyright-holders: Vincent.Halver

#include "emu.h"
#include "cdi220_lcd.h"

// 14 segment display font
static const uint8_t cdi220_lcd_char[20*22] =
{
	 0, 14,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 10,  0,
	14, 14, 14,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 10, 10, 10,
	14, 14, 14, 14,  9,  9,  9,  9,  9,  9,  1,  9,  9,  9,  9,  9, 10, 10, 10, 10,
	14, 14, 14, 14, 16,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  2, 10, 10, 10, 10,
	14, 14, 14, 14, 16, 16,  0,  0,  1,  1,  1,  1,  0,  0,  2,  2, 10, 10, 10, 10,
	14, 14, 14, 14, 16, 16, 16,  0,  1,  1,  1,  1,  0,  2,  2,  2, 10, 10, 10, 10,
	14, 14, 14, 14, 16, 16, 16, 16,  1,  1,  1,  1,  2,  2,  2,  2, 10, 10, 10, 10,
	14, 14, 14, 14,  0, 16, 16, 16,  1,  1,  1,  1,  2,  2,  2,  0, 10, 10, 10, 10,
	14, 14, 14, 14,  0,  0, 16, 16,  1,  1,  1,  1,  2,  2,  0,  0, 10, 10, 10, 10,
	14, 14, 14, 14, 15, 15, 15, 15, 15,  6,  6,  3,  3,  3,  3,  3, 10, 10, 10, 10,
	 0, 14, 14, 15, 15, 15, 15, 15, 15,  6,  6,  3,  3,  3,  3,  3,  3, 10, 10, 10,
	13, 13, 13, 15, 15, 15, 15, 15, 15,  6,  6,  3,  3,  3,  3,  3,  3, 11, 11,  0,
	13, 13, 13, 13, 15, 15, 15, 15, 15,  6,  6,  3,  3,  3,  3,  3, 11, 11, 11, 11,
	13, 13, 13, 13,  0,  0,  5,  5,  1,  1,  1,  1,  4,  4,  0,  0, 11, 11, 11, 11,
	13, 13, 13, 13,  0,  5,  5,  5,  1,  1,  1,  1,  4,  4,  4,  0, 11, 11, 11, 11,
	13, 13, 13, 13,  5,  5,  5,  5,  1,  1,  1,  1,  4,  4,  4,  4, 11, 11, 11, 11,
	13, 13, 13, 13,  5,  5,  5,  0,  1,  1,  1,  1,  0,  4,  4,  4, 11, 11, 11, 11,
	13, 13, 13, 13,  5,  5,  0,  0,  1,  1,  1,  1,  0,  0,  4,  4, 11, 11, 11, 11,
	13, 13, 13, 13,  5,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  4, 11, 11, 11, 11,
	13, 13, 13, 13, 12, 12, 12, 12, 12,  1, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11,
	13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11,
	 0, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 11,  0
};

// 3x5 pixel font. Only A-Z are needed.
static const uint16_t cdi220_lcd_font[26] =
{
	0x7bed, 0x6bae, 0x3923, 0x6b6e, 0x79a7, 0x79a4, 0x396b, 0x5bed, 0x7497, 0x126a,     // A-J
	0x5bad, 0x4927, 0x5fed, 0x5ffd, 0x2b6a, 0x6ba4, 0x2b7b, 0x6bad, 0x388e, 0x7492,     // K-T
	0x5b6f, 0x5b6a, 0x5bfd, 0x5aad, 0x5a92, 0x72a7                                      // U-Z
};

static constexpr int LCD_DIGIT_X = 34;
static constexpr int LCD_DIGIT_Y = 12;
static constexpr int LCD_DIGIT_WIDTH = 24;
static constexpr int LCD_RIGHT_X = 202;
static constexpr int LCD_SIDE_Y0 = 12;
static constexpr int LCD_SIDE_Y1 = 18;
static constexpr int LCD_SIDE_Y2 = 24;

static const struct
{
	uint16_t id;
	const char *text;
	int x, y;
} cdi220_lcd_indicators[] =
{
	{ 0x2000, "MUTE",					2, LCD_SIDE_Y0 },
	{ 0x0004, "PAUSE",					2, LCD_SIDE_Y1 },
	{ 0x1000, "PLAY",					2, LCD_SIDE_Y2 },
	{ 0x0400, "FTS",          LCD_DIGIT_X,           6 },
	{ 0x0100, "COMPACT DISC", LCD_RIGHT_X, LCD_SIDE_Y0 },
	{ 0x0200, "INTERACTIVE",  LCD_RIGHT_X, LCD_SIDE_Y1 },
	{ 0x0800, "GRAPHICS",     LCD_RIGHT_X, LCD_SIDE_Y2 },

	// TODO: Unused indicator lights. If these are drawn, there is an unknown behavior.
	{ 0x0001, "ONE",      0, 0 },
	{ 0x0002, "TWO",      0, 0 },
	{ 0x0008, "FOUR",	  0, 0 },
	{ 0x0010, "FIVE",     0, 0 },
	{ 0x0020, "SIX",      0, 0 },
	{ 0x0040, "SEVEN",    0, 0 },
	{ 0x0080, "EIGHT",    0, 0 },
	{ 0x4000, "FIFTEEN",  0, 0 },
	{ 0x8000, "SIXTEEN",  0, 0 }
};

static const char *const cdi220_lcd_digit_legend[16] =
{
	"",         "",         // 0: play icon, unused
	" TRACK",   "SHUFFLE",  // 1
	"UNKNOWN",  "UNKNOWN",  // 2: unused, unused
	"REPEAT",   "TOTAL",    // 3
	"",         "UNKNOWN",  // 4: colon, unused
	"SCAN",     "TRACK",    // 5
	"UNKNOWN",  "TIME",     // 6: unused, time
	"",         ""          // 7: extra indicators
};

void draw_lcd_text(bitmap_rgb32& bitmap, const rectangle& bounds, int x, int y, const char* text)
{
	for (const char* p = text; *p != '\0'; p++)
	{
		if (*p >= 'A' && *p <= 'Z')
		{
			const uint16_t glyph = cdi220_lcd_font[*p - 'A'];
			for (int row = 0; row < 5; row++)
			{
				for (int col = 0; col < 3; col++)
				{
					if (bounds.contains(x + col, y + row) && BIT(glyph, 14 - (row * 3 + col)))
						bitmap.pix(y + row, x + col) = rgb_t::white();
				}
			}
		}
		x += 4;
	}
}

void cdi220_lcd::draw_digit(bitmap_rgb32& bitmap, const rectangle& bounds, const uint16_t data, uint8_t idx)
{
	const int x0 = LCD_DIGIT_X + idx * LCD_DIGIT_WIDTH;
	for (int y = 0; y < D_HEIGHT; y++)
	{
		for (int x = 0; x < D_WIDTH; x++)
		{
			const uint8_t seg = cdi220_lcd_char[y * D_WIDTH + x];
			if (seg != 0 && BIT(data, seg - 1))
				bitmap.pix(LCD_DIGIT_Y + y, x0 + x) = rgb_t::white();
		}
	}

	uint16_t y0 = (idx == 1) ? 6 : 0;
	uint16_t y1 = (idx == 1) ? 0 : 6;
	if (BIT(data, 7))
		draw_lcd_text(bitmap, bounds, x0, y0, cdi220_lcd_digit_legend[idx * 2]);

	if (BIT(data, 6))
		draw_lcd_text(bitmap, bounds, x0, y1, cdi220_lcd_digit_legend[idx * 2 + 1]);
}

void cdi220_lcd::draw(bitmap_rgb32 &bitmap, const rectangle & bounds, const uint8_t *lcd_state)
{
	bitmap.fill(rgb_t::black(), bounds);

	for (int lcd = 7; lcd > 0; lcd--)
	{
		const uint16_t data = (lcd_state[lcd * 2] << 8) | lcd_state[lcd * 2 + 1];
		draw_digit(bitmap, bounds, data, 7 - lcd);
	}

	const uint16_t colon_data = (lcd_state[3 * 2] << 8) | lcd_state[3 * 2 + 1];
	if (BIT(colon_data, 7))
	{
		const int x0 = LCD_DIGIT_X + (7 - 3) * LCD_DIGIT_WIDTH;
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 2; x++)
			{
				bitmap.pix(LCD_DIGIT_Y + 6 + y, x0 + 21 + x) = rgb_t::white();
				bitmap.pix(LCD_DIGIT_Y + 14 + y, x0 + 21 + x) = rgb_t::white();
			}
		}
	}

	// Draws a small or big triangle
	const uint16_t play_data = (lcd_state[7 * 2] << 8)	| lcd_state[7 * 2 + 1];
	bool upper = BIT(play_data, 7);
	bool lower = BIT(play_data, 6);
	if (upper || lower)
	{
		for (int y = 0; y < 13; y++)
		{
			const int half = (y <= 6) ? y : (12 - y);
			for (int x = 0; x <= half; x++)
			{
				if((lower && (half-x-2>0)) || (upper && (half-x-2<0)))
					bitmap.pix(18 + y, 24 + x) = rgb_t::white();
			}
		}
	}

	uint16_t indicators = (lcd_state[0] << 8) | lcd_state[1];

	for (const auto& ind : cdi220_lcd_indicators)
	{
		if (indicators & ind.id)
			draw_lcd_text(bitmap, bounds, ind.x, ind.y, ind.text);
	}
}

