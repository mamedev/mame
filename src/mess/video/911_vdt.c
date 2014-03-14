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
	/* array must use same order as vdt911_model_t!!! */
	/* US */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_US_chr_offset, fontlayout_7bit, 0, 4 )
	/* UK */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_UK_chr_offset, fontlayout_7bit, 0, 4 )
	/* French */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_US_chr_offset, fontlayout_7bit, 0, 4 )
	/* German */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_german_chr_offset, fontlayout_7bit, 0, 4 )
	/* Swedish */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_swedish_chr_offset, fontlayout_7bit, 0, 4 )
	/* Norwegian */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_norwegian_chr_offset, fontlayout_7bit, 0, 4 )
	/* FrenchWP */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_frenchWP_chr_offset, fontlayout_7bit, 0, 4 )
	/* Japanese */
	GFXDECODE_ENTRY( vdt911_chr_region, vdt911_japanese_chr_offset, fontlayout_8bit, 0, 4 )
	/* Arabic */
	/* GFXDECODE_ENTRY( vdt911_chr_region, vdt911_arabic_chr_offset, fontlayout_8bit, 0, 4 ) */
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
/* TRUE for japanese and arabic terminals, which use 8-bit charcodes and keyboard shift modes */
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

vdt911_device::vdt911_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, VDT911, "911 VDT", tag, owner, clock, "vdt911", __FILE__),
		m_beeper(*this, ":beeper"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_int_line(*this)
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
	BEEP_TIMER
};

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vdt911_device::device_start()
{
	m_last_key_pressed = 0x80;

	/* set up cursor blink clock.  2Hz frequency -> .25s half-period. */
	/*m_blink_clock =*/

	// m_beeper->set_frequency(2000);

	m_blink_timer = timer_alloc(BLINK_TIMER);
	m_beep_timer = timer_alloc(BEEP_TIMER);

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
}

/*
    Time callbacks
*/
void vdt911_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case BLINK_TIMER:
		m_blink_state = !m_blink_state;
		logerror("Blink timer\n");
		break;
	case BEEP_TIMER:
		m_beeper->set_state(0);
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
			m_int_line(m_keyboard_interrupt_enable && m_keyboard_data_ready);
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
					m_int_line(CLEAR_LINE);
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

				gfx->opaque(*m_palette, bitmap, cliprect, cur_char, color, 0, 0,
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
void vdt911_device::keyboard()
{
	enum modifier_state_t
	{
		/* states for western keyboards and katakana/arabic keyboards in romaji/latin mode */
		lower_case = 0, upper_case, shift, control,
		/* states for katakana/arabic keyboards in katakana/arabic mode */
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
		key_buf[i] = machine().root_device().ioport(keynames[i])->read();
	}

	/* parse modifier keys */
	if ((USES_8BIT_CHARCODES())
		&& ((key_buf[5] & 0x0400) || ((!(key_buf[5] & 0x0100)) && m_foreign_mode)))
	{   /* we are in katakana/arabic mode */
		m_foreign_mode = true;

		if ((key_buf[4] & 0x0400) || (key_buf[5] & 0x0020))
			modifier_state = foreign_shift;
		else
			modifier_state = foreign;
	}
	else
	{   /* we are using a western keyboard, or a katakana/arabic keyboard in
        romaji/latin mode */
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
							m_int_line(ASSERT_LINE);
						return;
					}
				}
			}
		}
	}
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
		PORT_CONFSETTING( vdt911_model_FrenchWP, "French Word Processing" )
		PORT_CONFSETTING( vdt911_model_Japanese, "Japanese" )
		// PORT_CONFSETTING( vdt911_model_Arabic, "Arabic" )

	PORT_START( "SCREEN" )
	PORT_CONFNAME( 0x01, char_960, "Terminal screen size" )
		PORT_CONFSETTING( char_960, "960 chars (12 lines)")
		PORT_CONFSETTING( char_1920, "1920 chars (24 lines)")
INPUT_PORTS_END

static MACHINE_CONFIG_FRAGMENT( vdt911 )
	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INDIRECT_ENTRIES(3)
	MCFG_PALETTE_INIT_OWNER(vdt911_device, vdt911)

	MCFG_GFXDECODE_ADD("gfxdecode", vdt911)
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
