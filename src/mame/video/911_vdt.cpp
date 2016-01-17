// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    TI 911 VDT core.  To be operated with the TI 990 line of computers (can be connected to
    any model, as communication uses the CRU bus).

    Raphael Nabet 2002

TODO:
    * add more flexibility, so that we can create multiple-terminal configurations.
    * support test mode???
*/


#include "emu.h"
#include "911_vdt.h"
#include "911_chr.h"
#include "911_key.h"

#define MAX_VDT 1

static const gfx_layout fontlayout_7bit =
{
	7, 10,          /* 7*10 characters */
	128,            /* 128 characters */
	1,              /* 1 bit per pixel */
	{ 0 },
	{ 1, 2, 3, 4, 5, 6, 7 },            /* straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	10*8            /* every char takes 10 consecutive bytes */
};

static const gfx_layout fontlayout_8bit =
{
	7, 10,          /* 7*10 characters */
	128,            /* 128 characters */
	1,              /* 1 bit per pixel */
	{ 0 },
	{ 1, 2, 3, 4, 5, 6, 7 },                /* straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	10*8            /* every char takes 10 consecutive bytes */
};

static GFXDECODE_START( vdt911 )
	// Caution: Array must use same order as vdt911_model_t
	// US
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_US_chr_offset, fontlayout_7bit, 0, 4 )

	// UK
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_UK_chr_offset, fontlayout_7bit, 0, 4 )

	// French (without accented characters)
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_US_chr_offset, fontlayout_7bit, 0, 4 )

	// German
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_german_chr_offset, fontlayout_7bit, 0, 4 )

	// Swedish
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_swedish_chr_offset, fontlayout_7bit, 0, 4 )

	// Norwegian
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_norwegian_chr_offset, fontlayout_7bit, 0, 4 )

	// Japanese
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_japanese_chr_offset, fontlayout_8bit, 0, 4 )

	// Arabic
	// GFXDECODE_ENTRY( vdt911_chr_region, vdt911_arabic_chr_offset, fontlayout_8bit, 0, 4 )

	// FrenchWP (contains accented characters)
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_frenchWP_chr_offset, fontlayout_7bit, 0, 4 )
GFXDECODE_END

static const unsigned char vdt911_colors[] =
{
	0x00,0x00,0x00, /* black */
	0xC0,0xC0,0xC0, /* low intensity */
	0xFF,0xFF,0xFF  /* high intensity */
};

static const unsigned short vdt911_palette[] =
{
	0, 2,   /* high intensity */
	0, 1,   /* low intensity */
	2, 0,   /* high intensity, reverse */
	2, 1    /* low intensity, reverse */
};

/*
    Macros for model features
*/
/* TRUE for Japanese and Arabic terminals, which use 8-bit charcodes and keyboard shift modes */
#define USES_8BIT_CHARCODES() ((m_model == vdt911_model_Japanese) /*|| (m_model == vdt911_model_Arabic)*/)
/* TRUE for keyboards which have this extra key (on the left of TAB/SKIP)
    (Most localized keyboards have it) */
#define HAS_EXTRA_KEY_67() (! ((m_model == vdt911_model_US) || (m_model == vdt911_model_UK) || (m_model == vdt911_model_French)))
/* TRUE for keyboards which have this extra key (on the right of space),
    AND do not use it as a modifier */
#define HAS_EXTRA_KEY_91() ((m_model == vdt911_model_German) || (m_model == vdt911_model_Swedish) || (m_model == vdt911_model_Norwegian))

/*
    Initialize vdt911 palette
*/
PALETTE_INIT_MEMBER(vdt911_device, vdt911)
{
	UINT8 i, r, g, b;

	for ( i = 0; i < 3; i++ )
	{
		r = vdt911_colors[i*3]; g = vdt911_colors[i*3+1]; b = vdt911_colors[i*3+2];
		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for(i=0;i<8;i++)
		palette.set_pen_indirect(i, vdt911_palette[i]);
}

/*
    Copy a character bitmap array to another location in memory
*/
static void copy_character_matrix_array(const UINT8 char_array[128][10], UINT8 *dest)
{
	int i, j;

	for (i=0; i<128; i++)
		for (j=0; j<10; j++)
			*(dest++) = char_array[i][j];
}

/*
    Patch a character bitmap array according to an array of char_override_t
*/
static void apply_char_overrides(int nb_char_overrides, const char_override_t char_overrides[], UINT8 *dest)
{
	int i, j;

	for (i=0; i<nb_char_overrides; i++)
	{
		for (j=0; j<10; j++)
			dest[char_overrides[i].char_index*10+j] = char_defs[char_overrides[i].symbol_index][j];
	}
}

const device_type VDT911 = &device_creator<vdt911_device>;

vdt911_device::vdt911_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VDT911, "911 VDT", tag, owner, clock, "vdt911", __FILE__),
		m_beeper(*this, "beeper"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_keyint_line(*this),
		m_lineint_line(*this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void vdt911_device::device_config_complete()
{
}

enum
{
	BLINK_TIMER,
	BEEP_TIMER,
	LINE_TIMER
};

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vdt911_device::device_start()
{
	m_last_key_pressed = 0x80;

	m_keyboard_data_ready = false;
	m_display_enable = false;
	m_blink_state = false;

	m_keyint_line.resolve();
	m_lineint_line.resolve();

	/* set up cursor blink clock.  2Hz frequency -> .25s half-period. */
	/*m_blink_clock =*/

	// m_beeper->set_frequency(2000);

	m_blink_timer = timer_alloc(BLINK_TIMER);
	m_beep_timer = timer_alloc(BEEP_TIMER);
	m_line_timer = timer_alloc(LINE_TIMER);

	m_blink_timer->adjust(attotime::from_msec(0), 0, attotime::from_msec(250));

	UINT8 *base;
	UINT8 *chr = machine().root_device().memregion(vdt911_chr_region)->base();

	/* set up US character definitions */
	base = chr+vdt911_US_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);

	/* set up UK character definitions */
	base = chr+vdt911_UK_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(UK_overrides)/sizeof(char_override_t), UK_overrides, base);

	/* French character set is identical to US character set */

	/* set up German character definitions */
	base = chr+vdt911_german_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(german_overrides)/sizeof(char_override_t), german_overrides, base);

	/* set up Swedish/Finnish character definitions */
	base = chr+vdt911_swedish_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(swedish_overrides)/sizeof(char_override_t), swedish_overrides, base);

	/* set up Norwegian/Danish character definitions */
	base = chr+vdt911_norwegian_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(norwegian_overrides)/sizeof(char_override_t), norwegian_overrides, base);

	/* set up French word processing character definitions */
	base = chr+vdt911_frenchWP_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(frenchWP_overrides)/sizeof(char_override_t), frenchWP_overrides, base);

	/* set up Katakana Japanese character definitions */
	base = chr+vdt911_japanese_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(japanese_overrides)/sizeof(char_override_t), japanese_overrides, base);
	base = chr+vdt911_japanese_chr_offset+128*vdt911_single_char_len;
	copy_character_matrix_array(char_defs+char_defs_katakana_base, base);

#if 0
	/* set up Arabic character definitions */
	base = chr+vdt911_arabic_chr_offset;
	copy_character_matrix_array(char_defs+char_defs_US_base, base);
	apply_char_overrides(sizeof(arabic_overrides)/sizeof(char_override_t), arabic_overrides, base);
	base = chr+vdt911_arabic_chr_offset+128*vdt911_single_char_len;
	copy_character_matrix_array(char_defs+char_defs_arabic_base, base);
#endif
}


void vdt911_device::device_reset()
{
	m_model = (vdt911_model_t)ioport("LOCALE")->read();
	m_screen_size = (vdt911_screen_size_t)ioport("SCREEN")->read();

	if (m_screen_size == char_960)
		m_cursor_address_mask = 0x3ff;   /* 1kb of RAM */
	else
		m_cursor_address_mask = 0x7ff;   /* 2 kb of RAM */

	m_line_timer->adjust(attotime::from_msec(0), 0, attotime::from_hz(get_refresh_rate()));
}

/*
    Timer callbacks
*/
void vdt911_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case BLINK_TIMER:
		m_blink_state = !m_blink_state;
		break;
	case BEEP_TIMER:
		m_beeper->set_state(0);
		break;
	case LINE_TIMER:
		check_keyboard();
		m_lineint_line(ASSERT_LINE);
		m_lineint_line(CLEAR_LINE);
		break;
	}
}

/*
    CRU interface read
*/
READ8_MEMBER( vdt911_device::cru_r )
{
	int reply=0;

	offset &= 0x1;

	if (!m_word_select)
	{   /* select word 0 */
		switch (offset)
		{
		case 0:
			reply = m_display_RAM[m_cursor_address];
			break;

		case 1:
			reply = m_keyboard_data & 0x7f;
			if (m_keyboard_data_ready)
				reply |= 0x80;
			break;
		}
	}
	else
	{   /* select word 1 */
		switch (offset)
		{
		case 0:
			reply = m_cursor_address & 0xff;
			break;

		case 1:
			reply = (m_cursor_address >> 8) & 0x07;
			if (m_keyboard_data & 0x80)
				reply |= 0x08;
			/*if (!m_terminal_ready)
			    reply |= 0x10;*/
			if (m_previous_word_select)
				reply |= 0x20;
			/*if (m_keyboard_parity_error)
			    reply |= 0x40;*/
			if (m_keyboard_data_ready)
				reply |= 0x80;
			break;
		}
	}

	return reply;
}

/*
    CRU interface write
*/
WRITE8_MEMBER( vdt911_device::cru_w )
{
	offset &= 0xf;

	if (!m_word_select)
	{   /* select word 0 */
		switch (offset)
		{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
			/* display memory write data */
			if (data)
				m_data_reg |= (1 << offset);
			else
				m_data_reg &= ~ (1 << offset);
			break;

		case 0x8:
			/* write data strobe */
				m_display_RAM[m_cursor_address] = m_data_reg;
			break;

		case 0x9:
			/* test mode */
			/* ... */
			break;

		case 0xa:
			/* cursor move */
			if (data)
				m_cursor_address--;
			else
				m_cursor_address++;
			m_cursor_address &= m_cursor_address_mask;
			break;

		case 0xb:
			/* blinking cursor enable */
			m_blinking_cursor_enable = data;
			break;

		case 0xc:
			/* keyboard interrupt enable */
			m_keyboard_interrupt_enable = data;
			m_keyint_line(m_keyboard_interrupt_enable && m_keyboard_data_ready);
			break;

		case 0xd:
			/* dual intensity enable */
			m_dual_intensity_enable = data;
			break;

		case 0xe:
			/* display enable */
			m_display_enable = data;
			break;

		case 0xf:
			/* select word */
			m_previous_word_select = m_word_select;
			m_word_select = data;
			break;
		}
	}
	else
	{   /* select word 1 */
		switch (offset)
		{
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
		case 0x4:
		case 0x5:
		case 0x6:
		case 0x7:
		case 0x8:
		case 0x9:
		case 0xa:
			/* cursor address */
			if (data)
				m_cursor_address |= (1 << offset);
			else
				m_cursor_address &= ~ (1 << offset);
			m_cursor_address &= m_cursor_address_mask;
			break;

		case 0xb:
			/* not used */
			break;

		case 0xc:
			/* display cursor */
			m_display_cursor = data;
			break;

		case 0xd:
			/* keyboard acknowledge */
			if (m_keyboard_data_ready)
			{
				m_keyboard_data_ready = 0;
				if (m_keyboard_interrupt_enable)
					m_keyint_line(CLEAR_LINE);
			}
			/*m_keyboard_parity_error = 0;*/
			break;

		case 0xe:
			/* beep enable strobe - not tested */
			m_beeper->set_state(1);
			m_beep_timer->adjust(attotime::from_usec(300));
			break;

		case 0xf:
			/* select word */
			m_previous_word_select = m_word_select;
			m_word_select = data;
			break;
		}
	}
}

/*
    Video refresh
*/
void vdt911_device::refresh(bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y)
{
	gfx_element *gfx = m_gfxdecode->gfx(m_model);
	int height = (m_screen_size == char_960) ? 12 : /*25*/24;

	int use_8bit_charcodes = USES_8BIT_CHARCODES();
	int address = 0;
	int i, j;
	int cur_char;
	int color;

	/*if (use_8bit_charcodes)
	    color = vdt->dual_intensity_enable ? 1 : 0;*/

	if (!m_display_enable)
	{
		rectangle my_rect(x, x + 80*7 - 1, y, y + height*10 - 1);

		bitmap.fill(0, my_rect);
	}
	else
	{
		for (i=0; i<height; i++)
		{
			for (j=0; j<80; j++)
			{
				cur_char = m_display_RAM[address];
				/* does dual intensity work with 8-bit character set? */
				color = (m_dual_intensity_enable && (cur_char & 0x80)) ? 1 : 0;
				if (!use_8bit_charcodes)
					cur_char &= 0x7f;

				/* display cursor in reverse video */
				if ((address == m_cursor_address) && m_display_cursor
					&& ((!m_blinking_cursor_enable) || m_blink_state))
				color += 2;

				address++;

				gfx->opaque(bitmap, cliprect, cur_char, color, 0, 0,
					x+j*7, y+i*10);
			}
		}
	}
}

static const unsigned char (*const key_translate[])[91] =
{   /* array must use same order as vdt911_model_t!!! */
	/* US */
	US_key_translate,
	/* UK */
	US_key_translate,
	/* French */
	French_key_translate,
	/* German */
	German_key_translate,
	/* Swedish */
	Swedish_key_translate,
	/* Norwegian */
	Norwegian_key_translate,
	/* Japanese */
	Japanese_key_translate,
	/* Arabic */
	/*Arabic_key_translate,*/
	/* FrenchWP */
	FrenchWP_key_translate
};

/*
    keyboard handler: should be called regularly by machine code, for instance
    every Video Blank Interrupt.
*/
void vdt911_device::check_keyboard()
{
	enum modifier_state_t
	{
		/* states for Western keyboards and katakana/Arabic keyboards in romaji/Latin mode */
		lower_case = 0, upper_case, shift, control,
		/* states for katakana/Arabic keyboards in katakana/Arabic mode */
		foreign, foreign_shift,
		/* special value to stop repeat if the modifier state changes */
		special_debounce = -1
	};

	static unsigned char repeat_timer;
	enum { repeat_delay = 5 /* approx. 1/10s */ };

	UINT16 key_buf[6];
	int i, j;
	modifier_state_t modifier_state;
	int repeat_mode;

	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };

	/* read current key state */
	for (i = 0; i < 6; i++)
	{
		key_buf[i] = ioport(keynames[i])->read();
	}

	/* parse modifier keys */
	if ((USES_8BIT_CHARCODES())
		&& ((key_buf[5] & 0x0400) || ((!(key_buf[5] & 0x0100)) && m_foreign_mode)))
	{   /* we are in katakana/Arabic mode */
		m_foreign_mode = true;

		if ((key_buf[4] & 0x0400) || (key_buf[5] & 0x0020))
			modifier_state = foreign_shift;
		else
			modifier_state = foreign;
	}
	else
	{   /* we are using a Western keyboard, or a katakana/Arabic keyboard in
        romaji/Latin mode */
		m_foreign_mode = false;

		if (key_buf[3] & 0x0040)
			modifier_state = control;
		else if ((key_buf[4] & 0x0400) || (key_buf[5] & 0x0020))
			modifier_state = shift;
		else if ((key_buf[0] & 0x2000))
			modifier_state = upper_case;
		else
			modifier_state = lower_case;
	}


	/* test repeat key */
	repeat_mode = key_buf[2] & 0x0002;


	/* remove modifier keys */
	key_buf[0] &= ~0x2000;
	key_buf[2] &= ~0x0002;
	key_buf[3] &= ~0x0040;
	key_buf[4] &= ~0x0400;
	key_buf[5] &= ~0x0120;

	/* remove unused keys */
	if (! HAS_EXTRA_KEY_91())
		key_buf[5] &= ~0x0400;

	if (! HAS_EXTRA_KEY_67())
		key_buf[4] &= ~0x0004;


	if (! repeat_mode)
		/* reset REPEAT timer if the REPEAT key is not pressed */
		repeat_timer = 0;

	if (!(m_last_key_pressed & 0x80) && (key_buf[m_last_key_pressed >> 4] & (1 << (m_last_key_pressed & 0xf))))
	{
		/* last key has not been released */
		if (modifier_state == m_last_modifier_state)
		{
			/* handle REPEAT mode if applicable */
			if ((repeat_mode) && (++repeat_timer == repeat_delay))
			{
				if (m_keyboard_data_ready)
				{   /* keyboard buffer full */
					repeat_timer--;
				}
				else
				{   /* repeat current key */
					m_keyboard_data_ready = 1;
					repeat_timer = 0;
				}
			}
		}
		else
		{
			repeat_timer = 0;
			m_last_modifier_state = special_debounce;
		}
	}
	else
	{
		m_last_key_pressed = 0x80;

		if (m_keyboard_data_ready)
		{   /* keyboard buffer full */
			/* do nothing */
		}
		else
		{
			for (i=0; i<6; i++)
			{
				for (j=0; j<16; j++)
				{
					if (key_buf[i] & (1 << j))
					{
						m_last_key_pressed = (i << 4) | j;
						m_last_modifier_state = modifier_state;

						m_keyboard_data = (int)key_translate[m_model][modifier_state][m_last_key_pressed];
						m_keyboard_data_ready = 1;
						if (m_keyboard_interrupt_enable)
							m_keyint_line(ASSERT_LINE);
						return;
					}
				}
			}
		}
	}
}

int vdt911_device::get_refresh_rate()
{
	// European models have 50 Hz
	return ((m_model == vdt911_model_US) || (m_model == vdt911_model_Japanese))? 60 : 50;
}

UINT32 vdt911_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	refresh(bitmap, cliprect, 0, 0);
	return 0;
}

INPUT_PORTS_START( vdt911 )
	PORT_START( "LOCALE" )
	PORT_CONFNAME( 0x0f, 0x00, "Terminal language" )
		PORT_CONFSETTING( vdt911_model_US, "English US" )
		PORT_CONFSETTING( vdt911_model_UK, "English UK" )
		PORT_CONFSETTING( vdt911_model_French, "French" )
		PORT_CONFSETTING( vdt911_model_German, "German" )
		PORT_CONFSETTING( vdt911_model_Swedish, "Swedish" )
		PORT_CONFSETTING( vdt911_model_Norwegian, "Norwegian" )
		PORT_CONFSETTING( vdt911_model_Japanese, "Japanese" )
		// PORT_CONFSETTING( vdt911_model_Arabic, "Arabic" )
		PORT_CONFSETTING( vdt911_model_FrenchWP, "French Word Processing" )

	PORT_START( "SCREEN" )
	PORT_CONFNAME( 0x01, char_960, "Terminal screen size" )
		PORT_CONFSETTING( char_960, "960 chars (12 lines)")
		PORT_CONFSETTING( char_1920, "1920 chars (24 lines)")

	PORT_START("KEY0")  /* keys 1-16 */                                                                 \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)        \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)        \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)        \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)        \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)        \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)        \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)        \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)        \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CMD") PORT_CODE(KEYCODE_F9)       \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(red)") PORT_CODE(KEYCODE_F10)        \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ERASE FIELD") PORT_CODE(KEYCODE_END)  \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ERASE INPUT") PORT_CODE(KEYCODE_PGDN) \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(grey)") PORT_CODE(KEYCODE_F11)       \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("UPPER CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)  PORT_TOGGLE\
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1)        \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2)        \
																								\
	PORT_START("KEY1")  /* keys 17-32 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3)        \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4)        \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5)        \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6)        \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7)        \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8)        \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9)        \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0)        \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+ [") PORT_CODE(KEYCODE_MINUS)    \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- ]") PORT_CODE(KEYCODE_EQUALS)   \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("_ =") PORT_CODE(KEYCODE_BACKSPACE)    \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 (numpad)") PORT_CODE(KEYCODE_7_PAD) \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 (numpad)") PORT_CODE(KEYCODE_8_PAD) \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (numpad)") PORT_CODE(KEYCODE_9_PAD) \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("PRINT") PORT_CODE(KEYCODE_PRTSCR) \
																								\
	PORT_START("KEY2")  /* keys 33-48 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(up)") PORT_CODE(KEYCODE_UP)      \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REPEAT") PORT_CODE(KEYCODE_LALT)  \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER_PAD)  \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)      \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)      \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)      \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)      \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)      \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)      \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)      \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)      \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)      \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CHAR (left/right)") PORT_CODE(KEYCODE_OPENBRACE)  \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("FIELD (left/right)") PORT_CODE(KEYCODE_CLOSEBRACE)    \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) \
																								\
	PORT_START("KEY3")  /* keys 49-64 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 (numpad)") PORT_CODE(KEYCODE_4_PAD) \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 (numpad)") PORT_CODE(KEYCODE_5_PAD) \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 (numpad)") PORT_CODE(KEYCODE_6_PAD) \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(left)") PORT_CODE(KEYCODE_LEFT)  \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)    \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(right)") PORT_CODE(KEYCODE_RIGHT)    \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)      \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)      \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)      \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)      \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)      \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)      \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)      \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)      \
																								\
	PORT_START("KEY4")  /* keys 65-80 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON)    \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE)   \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(not on US keyboard)") PORT_CODE(KEYCODE_BACKSLASH)   \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SKIP TAB") PORT_CODE(KEYCODE_TAB) \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 (numpad)") PORT_CODE(KEYCODE_1_PAD) \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 (numpad)") PORT_CODE(KEYCODE_2_PAD) \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 (numpad)") PORT_CODE(KEYCODE_3_PAD) \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INS CHAR") PORT_CODE(KEYCODE_INSERT)  \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(down)") PORT_CODE(KEYCODE_DOWN)  \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL CHAR") PORT_CODE(KEYCODE_DEL) \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) \
		PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)      \
		PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)      \
		PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)      \
		PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)      \
		PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)      \
																								\
	PORT_START("KEY5")  /* keys 81-91 */                                                                \
		PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)      \
		PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)      \
		PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA)    \
		PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) \
		PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH)    \
		PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_RSHIFT) \
		PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 (numpad)") PORT_CODE(KEYCODE_0_PAD) \
		PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". (numpad)") PORT_CODE(KEYCODE_DEL_PAD)   \
		PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(not on US keyboard)") PORT_CODE(KEYCODE_MINUS_PAD)   \
		PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)  \
		PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(not on US keyboard)") PORT_CODE(KEYCODE_PLUS_PAD)
INPUT_PORTS_END

static MACHINE_CONFIG_FRAGMENT( vdt911 )
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, vdt911_device, screen_update)

	MCFG_SCREEN_SIZE(560, 280)
	MCFG_SCREEN_VISIBLE_AREA(0, 560-1, 0, /*250*/280-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.50)

	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INDIRECT_ENTRIES(3)
	MCFG_PALETTE_INIT_OWNER(vdt911_device, vdt911)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", vdt911)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor vdt911_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vdt911 );
}

ioport_constructor vdt911_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( vdt911 );
}
