// license:GPL-2.0+
// copyright-holders:Raphael Nabet, Robbbert
/*
    video/apexc.cpp : APEXC video

    Since the APEXC has no video display, we display the control panel.

    Additionally, We display one page of teletyper output.
*/

#include "emu.h"
#include "includes/apexc.h"

const rgb_t apexc_state::palette_table[] =
{
	rgb_t::white(),
	rgb_t::black(),
	rgb_t(255, 0, 0),
	rgb_t(50, 0, 0)
};

enum
{
	/* size and position of panel window */
	panel_window_width = 256,
	panel_window_height = 64,
	panel_window_offset_x = 0,
	panel_window_offset_y = 0,
	/* size and position of teletyper window */
	teletyper_window_width = 256,
	teletyper_window_height = 128,
	teletyper_window_offset_x = 0,
	teletyper_window_offset_y = panel_window_height
};

static const rectangle panel_window(
	panel_window_offset_x,  panel_window_offset_x+panel_window_width-1, /* min_x, max_x */
	panel_window_offset_y,  panel_window_offset_y+panel_window_height-1/* min_y, max_y */
);
static const rectangle teletyper_window(
	teletyper_window_offset_x,  teletyper_window_offset_x+teletyper_window_width-1, /* min_x, max_x */
	teletyper_window_offset_y,  teletyper_window_offset_y+teletyper_window_height-1/* min_y, max_y */
);
enum
{
	teletyper_scroll_step = 8
};
static const rectangle teletyper_scroll_clear_window(
	teletyper_window_offset_x,  teletyper_window_offset_x+teletyper_window_width-1, /* min_x, max_x */
	teletyper_window_offset_y+teletyper_window_height-teletyper_scroll_step,    teletyper_window_offset_y+teletyper_window_height-1 /* min_y, max_y */
);
//static const int var_teletyper_scroll_step = - teletyper_scroll_step;

void apexc_state::apexc_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, palette_table);
}

void apexc_state::video_start()
{
	int width = m_screen->width();
	int height = m_screen->height();

	m_bitmap = std::make_unique<bitmap_ind16>(width, height);
	m_bitmap->fill(0, /*machine().visible_area*/teletyper_window);
}

/* draw a small 8*8 LED (well, there were no LEDs at the time, so let's call this a lamp ;-) ) */
void apexc_state::draw_led(bitmap_ind16 &bitmap, int x, int y, int state)
{
	for (int yy = 1; yy < 7; yy++)
		for (int xx = 1; xx < 7; xx++)
			bitmap.pix16(y + yy, x + xx) = state ? 2 : 3;
}

/* write a single char on screen */
void apexc_state::draw_char(bitmap_ind16 &bitmap, char character, int x, int y, int color)
{
	m_gfxdecode->gfx(0)->transpen(bitmap, bitmap.cliprect(), character-32, color, 0, 0, x + 1, y, 0);
}

/* write a string on screen */
void apexc_state::draw_string(bitmap_ind16 &bitmap, const char *buf, int x, int y, int color)
{
	while (*buf)
	{
		draw_char(bitmap, *buf, x, y, color);

		x += 8;
		buf++;
	}
}


uint32_t apexc_state::screen_update_apexc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, /*machine().visible_area*/panel_window);
	draw_string(bitmap, "power", 8, 0, 0);
	draw_string(bitmap, "running", 8, 8, 0);
	draw_string(bitmap, "data :", 0, 24, 0);

	copybitmap(bitmap, *m_bitmap, 0, 0, 0, 0, teletyper_window);

	draw_led(bitmap, 0, 0, 1);
	draw_led(bitmap, 0, 8, m_maincpu->state_int(APEXC_STATE));

	for (int i = 0; i < 32; i++)
	{
		draw_led(bitmap, i*8, 32, (m_panel_data_reg << i) & 0x80000000UL);
		char the_char = '0' + ((i + 1) % 10);
		draw_char(bitmap, the_char, i*8, 40, 0);
		if (((i + 1) % 10) == 0)
		{
			the_char = '0' + ((i + 1) / 10);
			draw_char(bitmap, the_char, i*8, 48, 0);
		}
	}
	return 0;
}

void apexc_state::teletyper_init()
{
	m_letters = false;
	m_pos = 0;
}

void apexc_state::teletyper_linefeed()
{
	uint8_t buf[teletyper_window_width];

	for (int y = teletyper_window_offset_y; y < teletyper_window_offset_y + teletyper_window_height - teletyper_scroll_step; y++)
	{
		extract_scanline8(*m_bitmap, teletyper_window_offset_x, y+teletyper_scroll_step, teletyper_window_width, buf);
		draw_scanline8(*m_bitmap, teletyper_window_offset_x, y, teletyper_window_width, buf, m_palette->pens());
	}

	m_bitmap->fill(0, teletyper_scroll_clear_window);
}

void apexc_state::teletyper_putchar(int character)
{
	static const char ascii_table[2][32] =
	{
		{
			'0',                '1',                '2',                '3',
			'4',                '5',                '6',                '7',
			'8',                '9',                '+',                '-',
			'z',                '.',                'd',                '=',
			' ',                'y',                /*'@'*/'\200'/*theta*/,'\n'/*Line Space*/,
			',',                /*'&'*/'\201'/*Sigma*/,'x',             '/',
			'\r'/*Carriage Return*/,/*'!'*/'\202'/*Phi*/,'_'/*???*/,    '\0'/*Figures*/,
			/*'#'*/'\203'/*pi*/,')',                '(',                '\0'/*Letters*/
		},
		{
			' '/*???*/,         'T',                'B',                'O',
			'E',                'H',                'N',                'M',
			'A',                'L',                'R',                'G',
			'I',                'P',                'C',                'V',
			' ',                'Z',                'D',                '\n'/*Line Space*/,
			'S',                'Y',                'F',                'X',
			'\r'/*Carriage Return*/,'W',            'J',                '\0'/*Figures*/,
			'U',                'Q',                'K',                '\0'/*Letters*/
		}
	};

	char buffer[2] = "x";

	character &= 0x1f;

	switch (character)
	{
	case 19:
		/* Line Space */
		teletyper_linefeed();
		break;

	case 24:
		/* Carriage Return */
		m_pos = 0;
		break;

	case 27:
		/* Figures */
		m_letters = false;
		break;

	case 31:
		/* Letters */
		m_letters = true;
		break;

	default:
		/* Any printable character... */

		if (m_pos >= 32)
		{   /* if past right border, wrap around */
			teletyper_linefeed(); /* next line */
			m_pos = 0;            /* return to start of line */
		}

		/* print character */
		buffer[0] = ascii_table[m_letters][character];      /* lookup ASCII equivalent in table */
		buffer[1] = '\0';                                   /* terminate string */
		draw_string(*m_bitmap, buffer, 8*m_pos, 176, 0);    /* print char */
		m_pos++;                                            /* step carriage forward */
		break;
	}
}
