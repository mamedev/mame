// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Enik Land
/*********************************************************************

    sega315_5124.c

    Implementation of video hardware chips used by Sega System E,
    Master System, and Game Gear.

**********************************************************************/

/*

To do:

  - Display mode 1 (text)
  - Display mode 3 (multicolor)
  - Sprite doubling bug of the 315-5124 chip
  - Verify timing on the Game Gear (315-5378 chip)


SMS Display Timing
------------------
    For more information, please see:
    - http://cgfm2.emuviews.com/txt/msvdp.txt
    - http://www.smspower.org/forums/viewtopic.php?p=44198

A scanline contains the following sections:
  - horizontal sync     9  E9-ED   => HSYNC high
  - left blanking       2  ED-EE
  - color burst        14  EE-F5   => increment line counter/generate interrupts/etc
  - left blanking       8  F5-F9
  - left border        13  F9-FF
  - active display    256  00-7F
  - right border       15  80-87
  - right blanking      8  87-8B
  - horizontal sync    17  8B-93   => HSYNC low

  Although the processing done for a section happens when HCount is in the
  specified range (e.g. 00-7F for active display), probably there is a delay
  until its signal is shown on screen, as happens on the TMS9918 chip
  according to this timing diagram:
      http://www.smspower.org/Development/TMS9918MasterTimingDiagram


NTSC frame timing
                       256x192         256x224        256x240 (doesn't work on real hardware)
  - vertical blanking   3  D5-D7        3  E5-E7       3  ED-EF
  - top blanking       13  D8-E4       13  E8-F4      13  F0-FC
  - top border         27  E5-FF       11  F5-FF       3  FD-FF
  - active display    192  00-BF      224  00-DF     240  00-EF
  - bottom border      24  C0-D7        8  E0-E7       0  F0-F0
  - bottom blanking     3  D8-DA        3  E8-EA       3  F0-F2


PAL frame timing
                       256x192         256x224        256x240
  - vertical blanking   3  BA-BC        3  CA-CC       3  D2-D4
  - top blanking       13  BD-C9       13  CD-D9      13  D5-E1
  - top border         54  CA-FF       38  DA-FF      30  E2-FF
  - active display    192  00-BF      224  00-DF     240  00-EF
  - bottom border      48  C0-EF       32  E0-FF      24  F0-07
  - bottom blanking     3  F0-F2        3  00-02       3  08-0A

*/

#include "emu.h"
#include "video/315_5124.h"


#define STATUS_VINT           0x80  /* Pending vertical interrupt flag */
#define STATUS_SPROVR         0x40  /* Sprite overflow flag */
#define STATUS_SPRCOL         0x20  /* Object collision flag */
#define STATUS_HINT           0x02  /* Pending horizontal interrupt flag */

#define VINT_HPOS             24
#define VINT_FLAG_HPOS        24
#define HINT_HPOS             26
#define NMI_HPOS              28 /* not verified */
#define VCOUNT_CHANGE_HPOS    23
#define SPROVR_HPOS           24
#define SPRCOL_BASEHPOS       59
#define XSCROLL_HPOS          21
#define DISPLAY_DISABLED_HPOS 24 /* not verified, works if above 18 (for 'pstrike2') and below 25 (for 'fantdizzy') */
#define DISPLAY_CB_HPOS       2  /* fixes 'roadrash' (SMS game) title scrolling, due to line counter reload timing */

#define DRAW_TIME_GG        94      /* 9 + 2 + 14 + 8 + 13 + 96/2 */
#define DRAW_TIME_SMS       46      /* 9 + 2 + 14 + 8 + 13 */

#define PRIORITY_BIT          0x1000
#define BACKDROP_COLOR        ((m_vdp_mode == 4 ? 0x10 : 0x00) + (m_reg[0x07] & 0x0f))

#define VERTICAL_BLANKING     0
#define TOP_BLANKING          1
#define TOP_BORDER            2
#define ACTIVE_DISPLAY_V      3
#define BOTTOM_BORDER         4
#define BOTTOM_BLANKING       5

static const UINT8 ntsc_192[6] = { 3, 13, 27, 192, 24, 3 };
static const UINT8 ntsc_224[6] = { 3, 13, 11, 224,  8, 3 };
static const UINT8 ntsc_240[6] = { 3, 13,  3, 240,  0, 3 };
static const UINT8 pal_192[6]  = { 3, 13, 54, 192, 48, 3 };
static const UINT8 pal_224[6]  = { 3, 13, 38, 224, 32, 3 };
static const UINT8 pal_240[6]  = { 3, 13, 30, 240, 24, 3 };


const device_type SEGA315_5124 = &device_creator<sega315_5124_device>;
const device_type SEGA315_5246 = &device_creator<sega315_5246_device>;
const device_type SEGA315_5378 = &device_creator<sega315_5378_device>;


PALETTE_INIT_MEMBER(sega315_5124_device, sega315_5124)
{
	int i;
	for (i = 0; i < 64; i++)
	{
		int r = i & 0x03;
		int g = (i & 0x0c) >> 2;
		int b = (i & 0x30) >> 4;
		palette.set_pen_color(i, pal2bit(r), pal2bit(g), pal2bit(b));
	}
	/* sms and sg1000-mark3 uses a different palette for modes 0 to 3 - see http://www.smspower.org/Development/Palette */
	/* TMS9918 palette */
	palette.set_pen_color(64+ 0,   0,   0,   0); // palette.set_pen_color(64+ 0,   0,   0,   0);
	palette.set_pen_color(64+ 1,   0,   0,   0); // palette.set_pen_color(64+ 1,   0,   0,   0);
	palette.set_pen_color(64+ 2,   0, 170,   0); // palette.set_pen_color(64+ 2,  33, 200,  66);
	palette.set_pen_color(64+ 3,   0, 255,   0); // palette.set_pen_color(64+ 3,  94, 220, 120);
	palette.set_pen_color(64+ 4,   0,   0,  85); // palette.set_pen_color(64+ 4,  84,  85, 237);
	palette.set_pen_color(64+ 5,   0,   0, 255); // palette.set_pen_color(64+ 5, 125, 118, 252);
	palette.set_pen_color(64+ 6,  85,   0,   0); // palette.set_pen_color(64+ 6, 212,  82,  77);
	palette.set_pen_color(64+ 7,   0, 255, 255); // palette.set_pen_color(64+ 7,  66, 235, 245);
	palette.set_pen_color(64+ 8, 170,   0,   0); // palette.set_pen_color(64+ 8, 252,  85,  84);
	palette.set_pen_color(64+ 9, 255,   0,   0); // palette.set_pen_color(64+ 9, 255, 121, 120);
	palette.set_pen_color(64+10,  85,  85,   0); // palette.set_pen_color(64+10, 212, 193,  84);
	palette.set_pen_color(64+11, 255, 255,   0); // palette.set_pen_color(64+11, 230, 206, 128);
	palette.set_pen_color(64+12,   0,  85,   0); // palette.set_pen_color(64+12,  33, 176,  59);
	palette.set_pen_color(64+13, 255,   0, 255); // palette.set_pen_color(64+13, 201,  91, 186);
	palette.set_pen_color(64+14,  85,  85,  85); // palette.set_pen_color(64+14, 204, 204, 204);
	palette.set_pen_color(64+15, 255, 255, 255); // palette.set_pen_color(64+15, 255, 255, 255);
}


PALETTE_INIT_MEMBER(sega315_5378_device, sega315_5378)
{
	int i;
	for (i = 0; i < 4096; i++)
	{
		int r = i & 0x000f;
		int g = (i & 0x00f0) >> 4;
		int b = (i & 0x0f00) >> 8;
		palette.set_pen_color(i, pal4bit(r), pal4bit(g), pal4bit(b));
	}
}


// default address map
static ADDRESS_MAP_START( sega315_5124, AS_0, 8, sega315_5124_device )
	AM_RANGE(0x0000, VRAM_SIZE-1) AM_RAM
ADDRESS_MAP_END


sega315_5124_device::sega315_5124_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t( mconfig, SEGA315_5124, "Sega 315-5124 VDP", tag, owner, clock, "sega315_5124", __FILE__)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_cram_size( SEGA315_5124_CRAM_SIZE )
	, m_palette_offset( 0 )
	, m_supports_224_240( false )
	, m_is_pal(false)
	, m_int_cb(*this)
	, m_pause_cb(*this)
	, m_space_config("videoram", ENDIANNESS_LITTLE, 8, 14, 0, NULL, *ADDRESS_MAP_NAME(sega315_5124))
	, m_palette(*this, "palette")
{
}


sega315_5124_device::sega315_5124_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 cram_size, UINT8 palette_offset, bool supports_224_240, const char *shortname, const char *source)
	: device_t( mconfig, type, name, tag, owner, clock, shortname, source)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_cram_size( cram_size )
	, m_palette_offset( palette_offset )
	, m_supports_224_240( supports_224_240 )
	, m_is_pal(false)
	, m_int_cb(*this)
	, m_pause_cb(*this)
	, m_space_config("videoram", ENDIANNESS_LITTLE, 8, 14, 0, NULL, *ADDRESS_MAP_NAME(sega315_5124))
	, m_palette(*this, "palette")
{
}


sega315_5246_device::sega315_5246_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega315_5124_device( mconfig, SEGA315_5246, "Sega 315-5246 VDP", tag, owner, clock, SEGA315_5124_CRAM_SIZE, 0, true, "sega315_5246", __FILE__)
{
}


sega315_5378_device::sega315_5378_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega315_5124_device( mconfig, SEGA315_5378, "Sega 315-5378 VDP", tag, owner, clock, SEGA315_5378_CRAM_SIZE, 0x10, true, "sega315_5378", __FILE__)
{
}


void sega315_5124_device::set_display_settings()
{
	const bool M1 = m_reg[0x01] & 0x10;
	const bool M2 = m_reg[0x00] & 0x02;
	const bool M3 = m_reg[0x01] & 0x08;
	const bool M4 = m_reg[0x00] & 0x04;

	m_y_pixels = 192;

	if (M4)
	{
		/* mode 4 */
		m_vdp_mode = 4;
		if ( m_supports_224_240 )
		{
			if (M2)
			{
				if (M1 && !M3)
				{
					m_y_pixels = 224;   /* 224-line display */
				}
				else if (!M1 && M3)
				{
					m_y_pixels = 240;   /* 240-line display */
				}
			}
		}
	}
	else
	{
		/* original TMS9918 mode */
		if (!M1 && !M2 && !M3)
		{
			m_vdp_mode = 0;
		}
#if 0
		/* Mode 1, not implemented */
		else if (M1 && !M2 && !M3)
		{
			m_vdp_mode = 1;
		}
#endif
		else if (!M1 && M2 && !M3)
		{
			m_vdp_mode = 2;
		}
#if 0
		/* Mode 3, not implemented */
		else if (!M1 && !M2 && M3)
		{
			m_vdp_mode = 3;
		}
#endif
		else
		{
			logerror("Unknown video mode detected (M1 = %c, M2 = %c, M3 = %c, M4 = %c)\n", M1 ? '1' : '0', M2 ? '1' : '0', M3 ? '1' : '0', M4 ? '1' : '0');
		}
	}

	set_frame_timing();
	m_cram_dirty = 1;
}


void sega315_5124_device::set_frame_timing()
{
	switch (m_y_pixels)
	{
		case 192:
			m_frame_timing = (m_is_pal) ? pal_192 : ntsc_192;
			break;

		case 224:
			m_frame_timing = (m_is_pal) ? pal_224 : ntsc_224;
			break;

		case 240:
			m_frame_timing = (m_is_pal) ? pal_240 : ntsc_240;
			break;
	}
}


READ8_MEMBER( sega315_5124_device::vcount_read )
{
	const int active_scr_start = m_frame_timing[VERTICAL_BLANKING] + m_frame_timing[TOP_BLANKING] + m_frame_timing[TOP_BORDER];
	int vpos = m_screen->vpos();

	if (m_screen->hpos() < VCOUNT_CHANGE_HPOS)
	{
		vpos--;
		if (vpos < 0)
			vpos += m_screen->height();
	}

	return (vpos - active_scr_start) & 0xff;
}


READ8_MEMBER( sega315_5124_device::hcount_read )
{
	return m_hcounter;
}


void sega315_5124_device::hcount_latch_at_hpos( int hpos )
{
	const int active_scr_start = 46;      /* 9 + 2 + 14 + 8 + 13 */

	/* The hcount value returned by the VDP seems to be based on the previous hpos */
	int hclock = hpos - 1;
	if (hclock < 0)
		hclock += SEGA315_5124_WIDTH;

	m_hcounter = ((hclock - active_scr_start) >> 1) & 0xff;
}


void sega315_5378_device::set_sega315_5124_compatibility_mode( bool sega315_5124_compatibility_mode )
{
	m_sega315_5124_compatibility_mode = sega315_5124_compatibility_mode;
	m_cram_mask = (!m_sega315_5124_compatibility_mode) ? (SEGA315_5378_CRAM_SIZE - 1) : (SEGA315_5124_CRAM_SIZE - 1);
	m_draw_time = m_sega315_5124_compatibility_mode ? DRAW_TIME_SMS : DRAW_TIME_GG;
}


void sega315_5124_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch( id )
	{
	case TIMER_LINE:
		process_line_timer();
		break;

	case TIMER_FLAGS:
		/* Activate flags that were pending until the end of the line. */
		check_pending_flags();
		break;

	case TIMER_DRAW:
		update_palette();
		draw_scanline( SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH, param, m_screen->vpos() - param );
		break;

	case TIMER_LBORDER:
		{
			rectangle rec;
			rec.min_y = rec.max_y = param;

			update_palette();

			/* Draw left border */
			rec.min_x = SEGA315_5124_LBORDER_START;
			rec.max_x = SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 1;
			m_tmpbitmap.fill(m_palette->pen(m_current_palette[BACKDROP_COLOR]), rec);
			m_y1_bitmap.fill(( m_reg[0x07] & 0x0f ) ? 1 : 0, rec);
		}
		break;

	case TIMER_RBORDER:
		{
			rectangle rec;
			rec.min_y = rec.max_y = param;

			update_palette();

			/* Draw right border */
			rec.min_x = SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256;
			rec.max_x = rec.min_x + SEGA315_5124_RBORDER_WIDTH - 1;
			m_tmpbitmap.fill(m_palette->pen(m_current_palette[BACKDROP_COLOR]), rec);
			m_y1_bitmap.fill(( m_reg[0x07] & 0x0f ) ? 1 : 0, rec);
		}
		break;

	case TIMER_HINT:
		if ((m_pending_status & STATUS_HINT) || (m_status & STATUS_HINT))
		{
			if ((m_reg[0x00] & 0x10))
			{
				m_irq_state = 1;

				if ( !m_int_cb.isnull() )
					m_int_cb(ASSERT_LINE);
			}
		}
		break;

	case TIMER_VINT:
		if ((m_pending_status & STATUS_VINT) || (m_status & STATUS_VINT))
		{
			if ((m_reg[0x01] & 0x20))
			{
				m_irq_state = 1;

				if ( !m_int_cb.isnull() )
					m_int_cb(ASSERT_LINE);
			}
		}
		break;

	case TIMER_NMI:
		if ( !m_pause_cb.isnull() )
			m_pause_cb(0);
		break;
	}
}


void sega315_5124_device::process_line_timer()
{
	const int vpos = m_screen->vpos();
	int vpos_limit = m_frame_timing[VERTICAL_BLANKING] + m_frame_timing[TOP_BLANKING]
					+ m_frame_timing[TOP_BORDER] + m_frame_timing[ACTIVE_DISPLAY_V]
					+ m_frame_timing[BOTTOM_BORDER] + m_frame_timing[BOTTOM_BLANKING];

	/* copy current values in case they are not changed until latch time */
	m_display_disabled = !(m_reg[0x01] & 0x40);
	m_reg8copy = m_reg[0x08];

	vpos_limit -= m_frame_timing[BOTTOM_BLANKING];

	/* Check if we're below the bottom border */
	if (vpos >= vpos_limit)
	{
		m_line_counter = m_reg[0x0a];
		return;
	}

	vpos_limit -= m_frame_timing[BOTTOM_BORDER];

	/* Check if we're in the bottom border area */
	if (vpos >= vpos_limit)
	{
		if (vpos == vpos_limit)
		{
			if (m_line_counter == 0x00)
			{
				m_line_counter = m_reg[0x0a];
				m_hint_timer->adjust( m_screen->time_until_pos( vpos, HINT_HPOS ) );
				m_pending_status |= STATUS_HINT;
			}
			else
			{
				m_line_counter--;
			}
		}
		else
		{
			m_line_counter = m_reg[0x0a];
		}

		if (vpos == vpos_limit + 1)
		{
			m_vint_timer->adjust( m_screen->time_until_pos( vpos, VINT_HPOS ) );
			m_pending_status |= STATUS_VINT;
		}

		/* Draw borders */
		m_lborder_timer->adjust( m_screen->time_until_pos( vpos, SEGA315_5124_LBORDER_START ), vpos );
		m_rborder_timer->adjust( m_screen->time_until_pos( vpos, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 ), vpos );

		/* Draw middle of the border */
		/* We need to do this through the regular drawing function */
		/* so sprite collisions can occur on the border. */
		select_sprites( vpos - (vpos_limit - m_frame_timing[ACTIVE_DISPLAY_V]) );
		m_draw_timer->adjust( m_screen->time_until_pos( vpos, m_draw_time ), vpos_limit - m_frame_timing[ACTIVE_DISPLAY_V] );
		return;
	}

	vpos_limit -= m_frame_timing[ACTIVE_DISPLAY_V];

	/* Check if we're in the active display area */
	if (vpos >= vpos_limit)
	{
		if (vpos == vpos_limit)
		{
			m_reg9copy = m_reg[0x09];
		}

		if (m_line_counter == 0x00)
		{
			m_line_counter = m_reg[0x0a];
			m_hint_timer->adjust( m_screen->time_until_pos( vpos, HINT_HPOS ) );
			m_pending_status |= STATUS_HINT;
		}
		else
		{
			m_line_counter--;
		}

		/* Draw borders */
		m_lborder_timer->adjust( m_screen->time_until_pos( vpos, SEGA315_5124_LBORDER_START ), vpos );
		m_rborder_timer->adjust( m_screen->time_until_pos( vpos, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 ), vpos );

		/* Draw active display */
		select_sprites( vpos - vpos_limit );
		m_draw_timer->adjust( m_screen->time_until_pos( vpos, m_draw_time ), vpos_limit );
		return;
	}

	vpos_limit -= m_frame_timing[TOP_BORDER];

	/* Check if we're in the top border area */
	if (vpos >= vpos_limit)
	{
		m_line_counter = m_reg[0x0a];

		/* Check if we're on the last line of the top border */
		if (vpos == vpos_limit + m_frame_timing[TOP_BORDER] - 1)
		{
			m_nmi_timer->adjust( m_screen->time_until_pos( vpos, NMI_HPOS ) );
		}

		/* Draw borders */
		m_lborder_timer->adjust( m_screen->time_until_pos( vpos, SEGA315_5124_LBORDER_START ), vpos );
		m_rborder_timer->adjust( m_screen->time_until_pos( vpos, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 ), vpos );

		/* Draw middle of the border */
		/* We need to do this through the regular drawing function */
		/* so sprite collisions can occur on the border. */
		select_sprites( vpos - (vpos_limit + m_frame_timing[TOP_BORDER]) );
		m_draw_timer->adjust( m_screen->time_until_pos( vpos, m_draw_time ), vpos_limit + m_frame_timing[TOP_BORDER] );
		return;
	}

	/* we're in the vertical or top blanking area */
	m_line_counter = m_reg[0x0a];
}


READ8_MEMBER( sega315_5124_device::vram_read )
{
	UINT8 temp;

	/* SMS 2 & GG behaviour. Seems like the latched data is passed straight through */
	/* to the address register when in the middle of doing a command.               */
	/* Cosmic Spacehead needs this, among others                                    */
	/* Clear pending write flag */
	m_pending_reg_write = 0;

	/* Return read buffer contents */
	temp = m_buffer;

	if ( !space.debugger_access() )
	{
		/* Load read buffer */
		m_buffer = this->space().read_byte(m_addr & 0x3fff);

		/* Bump internal address register */
		m_addr += 1;
	}
	return temp;
}


void sega315_5124_device::check_pending_flags()
{
	int hpos;

	if (!m_pending_status)
	{
		return;
	}

	/* A timer ensures that this function will run at least at end of each line.
	   When this function runs through a CPU instruction executed when the timer
	   was about to fire, the time added in the CPU timeslice may make hpos()
	   return some position in the beginning of next line. To ensure the instruction
	   will get updated status, here a maximum hpos is set if the timer reports no
	   remaining time, what could also occur due to the ahead time of the timeslice. */
	if (m_pending_flags_timer->remaining() == attotime::zero)
	{
		hpos = SEGA315_5124_WIDTH - 1;
	}
	else
	{
		hpos = m_screen->hpos();
	}

	if ((m_pending_status & STATUS_HINT) && hpos >= HINT_HPOS)
	{
		m_pending_status &= ~STATUS_HINT;
		m_status |= STATUS_HINT;   // fake flag, it is overridden on register read.
	}
	if ((m_pending_status & STATUS_VINT) && hpos >= VINT_FLAG_HPOS)
	{
		m_pending_status &= ~STATUS_VINT;
		m_status |= STATUS_VINT;
	}
	if ((m_pending_status & STATUS_SPROVR) && hpos >= SPROVR_HPOS)
	{
		m_pending_status &= ~STATUS_SPROVR;
		m_status |= STATUS_SPROVR;
	}
	if ((m_pending_status & STATUS_SPRCOL) && hpos >= m_pending_sprcol_x)
	{
		m_pending_status &= ~STATUS_SPRCOL;
		m_status |= STATUS_SPRCOL;
		m_pending_sprcol_x = 0;
	}
}


READ8_MEMBER( sega315_5124_device::register_read )
{
	UINT8 temp;

	check_pending_flags();
	temp = m_status;

	if ( !space.debugger_access() )
	{
		/* Clear pending write flag */
		m_pending_reg_write = 0;

		m_status &= ~(STATUS_VINT | STATUS_SPROVR | STATUS_SPRCOL | STATUS_HINT);

		if (m_irq_state == 1)
		{
			m_irq_state = 0;

			if ( !m_int_cb.isnull() )
				m_int_cb(CLEAR_LINE);
		}
	}

	/* low 5 bits return non-zero data (it fixes PGA Tour Golf course map introduction) */
	return temp | 0x1f;
}


WRITE8_MEMBER( sega315_5124_device::vram_write )
{
	/* SMS 2 & GG behaviour. Seems like the latched data is passed straight through */
	/* to the address register when in the middle of doing a command.               */
	/* Cosmic Spacehead needs this, among others                                    */
	/* Clear pending write flag */
	m_pending_reg_write = 0;

	switch(m_addrmode)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			this->space().write_byte(m_addr & 0x3fff, data);
			break;

		case 0x03:
			cram_write(data);
			break;
	}

	m_buffer = data;
	m_addr += 1;
}


WRITE8_MEMBER( sega315_5124_device::register_write )
{
	int reg_num;

	if (m_pending_reg_write == 0)
	{
		m_addr = (m_addr & 0xff00) | data;
		m_pending_reg_write = 1;
	}
	else
	{
		/* Clear pending write flag */
		m_pending_reg_write = 0;

		m_addrmode = (data >> 6) & 0x03;
		m_addr = (data << 8) | (m_addr & 0xff);
		switch (m_addrmode)
		{
		case 0:     /* VRAM reading mode */
			m_buffer = this->space().read_byte(m_addr & 0x3fff);
			m_addr += 1;
			break;

		case 1:     /* VRAM writing mode */
			break;

		case 2:     /* VDP register write */
			reg_num = data & 0x0f;
			m_reg[reg_num] = m_addr & 0xff;
			//logerror("%s: %s: setting register %x to %02x\n", machine().describe_context(), tag(), reg_num, m_addr & 0xff);

			switch (reg_num)
			{
			case 0:
				set_display_settings();
				if (m_addr & 0x02)
					logerror("overscan enabled.\n");
				break;
			case 1:
				set_display_settings();
				if (m_screen->hpos() <= DISPLAY_DISABLED_HPOS)
					m_display_disabled = !(m_reg[0x01] & 0x40);
				break;
			case 8:
				if (m_screen->hpos() <= XSCROLL_HPOS)
					m_reg8copy = m_reg[0x08];
			}

			check_pending_flags();

			if ( ( reg_num == 0 && (m_status & STATUS_HINT) ) ||
					( reg_num == 1 && (m_status & STATUS_VINT) ) )
			{
				// For HINT disabling through register 00:
				// "Line IRQ VCount" test, of Flubba's VDPTest ROM, disables HINT to wait
				// for next VINT, but HINT occurs when the operation is about to execute.
				// So here, where the setting is done, the irq_state needs to be cleared.
				//
				// For VINT disabling through register 01:
				// When running eagles5 on the smskr driver the irq_state is 1 because of some
				// previos HINTs that occurred. eagles5 sets register 01 to 0x02 and expects
				// the irq state to be cleared after that.
				// The following bit of code takes care of that.
				//
				if ( ( reg_num == 0 && !(m_reg[0x00] & 0x10) ) ||
						( reg_num == 1 && !(m_reg[0x01] & 0x20) ) )
				{
					if (m_irq_state == 1)
					{
						m_irq_state = 0;

						if ( !m_int_cb.isnull() )
						{
							m_int_cb(CLEAR_LINE);
						}
					}
				}
				else
				{
					// For register 01 and VINT enabling:
					// Assert the IRQ line for the scoreboard of robocop3,
					// on the sms/smspal driver, be displayed correctly.
					//
					// Assume the same behavior for reg0+HINT.
					//
					m_irq_state = 1;

					if ( !m_int_cb.isnull() )
						m_int_cb(ASSERT_LINE);
				}
			}
			m_addrmode = 0;
			break;

		case 3:     /* CRAM writing mode */
			break;
		}
	}
}


UINT16 sega315_5124_device::get_name_table_row(int row)
{
	return ((row >> 3) << 6) & (((m_reg[0x02] & 0x01) << 10) | 0x3bff);
}


UINT16 sega315_5246_device::get_name_table_row(int row)
{
	return (row >> 3) << 6;
}


UINT16 sega315_5378_device::get_name_table_row(int row)
{
	return (row >> 3) << 6;
}


void sega315_5124_device::draw_scanline_mode4( int *line_buffer, int *priority_selected, int line )
{
	int tile_column;
	int y_scroll, scroll_mod;
	int pixel_x, pixel_plot_x;
	int bit_plane_0, bit_plane_1, bit_plane_2, bit_plane_3;
	UINT16 name_table_address;

	/* if top 2 rows of screen not affected by horizontal scrolling, then x_scroll = 0 */
	/* else x_scroll = m_reg8copy                                                      */
	const int x_scroll = (((m_reg[0x00] & 0x40) && (line < 16)) ? 0 : 0x0100 - m_reg8copy);

	const int x_scroll_start_column = (x_scroll >> 3);             /* x starting column tile */

	if ( m_y_pixels != 192 )
	{
		name_table_address = ((m_reg[0x02] & 0x0c) << 10) | 0x0700;
		scroll_mod = 256;
	}
	else
	{
		name_table_address = (m_reg[0x02] << 10) & 0x3800;
		scroll_mod = 224;
	}

	/* Draw background layer */
	for (tile_column = 0; tile_column < 33; tile_column++)
	{
		UINT16 tile_data;
		int tile_selected, palette_selected, horiz_selected, vert_selected, priority_select;
		int tile_line;

		/* Rightmost 8 columns for SMS (or 2 columns for GG) not affected by */
		/* vertical scrolling when bit 7 of reg[0x00] is set */
		y_scroll = ((m_reg[0x00] & 0x80) && (tile_column > 23)) ? 0 : m_reg9copy;

		tile_line = ((tile_column + x_scroll_start_column) & 0x1f) << 1;
		tile_data = space().read_word(name_table_address + get_name_table_row((line + y_scroll) % scroll_mod) + tile_line);

		tile_selected = (tile_data & 0x01ff);
		priority_select = tile_data & PRIORITY_BIT;
		palette_selected = (tile_data >> 11) & 0x01;
		vert_selected = (tile_data >> 10) & 0x01;
		horiz_selected = (tile_data >> 9) & 0x01;

		tile_line = line - ((0x07 - (y_scroll & 0x07)) + 1);
		if (vert_selected)
			tile_line = 0x07 - tile_line;

		bit_plane_0 = space().read_byte(((tile_selected << 5) + ((tile_line & 0x07) << 2)) + 0x00);
		bit_plane_1 = space().read_byte(((tile_selected << 5) + ((tile_line & 0x07) << 2)) + 0x01);
		bit_plane_2 = space().read_byte(((tile_selected << 5) + ((tile_line & 0x07) << 2)) + 0x02);
		bit_plane_3 = space().read_byte(((tile_selected << 5) + ((tile_line & 0x07) << 2)) + 0x03);

		for (pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			UINT8 pen_bit_0, pen_bit_1, pen_bit_2, pen_bit_3;
			UINT8 pen_selected;

			pen_bit_0 = (bit_plane_0 >> (7 - pixel_x)) & 0x01;
			pen_bit_1 = (bit_plane_1 >> (7 - pixel_x)) & 0x01;
			pen_bit_2 = (bit_plane_2 >> (7 - pixel_x)) & 0x01;
			pen_bit_3 = (bit_plane_3 >> (7 - pixel_x)) & 0x01;

			pen_selected = (pen_bit_3 << 3 | pen_bit_2 << 2 | pen_bit_1 << 1 | pen_bit_0);
			if (palette_selected)
				pen_selected |= 0x10;

			if (!horiz_selected)
			{
				pixel_plot_x = pixel_x;
			}
			else
			{
				pixel_plot_x = 7 - pixel_x;
			}

			pixel_plot_x = (0 - (x_scroll & 0x07) + (tile_column << 3) + pixel_plot_x);
			if (pixel_plot_x >= 0 && pixel_plot_x < 256)
			{
				//logerror("%x %x\n", pixel_plot_x, line);
				if (tile_column == 0 && (x_scroll & 0x07))
				{
					/* when the first column hasn't completely entered in the screen, its
					   background is filled only with color #0 of the selected palette */
					line_buffer[pixel_plot_x] = m_current_palette[palette_selected ? 0x10 : 0x00];
					priority_selected[pixel_plot_x] = priority_select;
				}
				else
				{
					line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
					priority_selected[pixel_plot_x] = priority_select | (pen_selected & 0x0f);
				}
			}
		}
	}
}


void sega315_5124_device::select_sprites( int line )
{
	int max_sprites;

	/* At this point the VDP vcount still doesn't refer the new line,
	   because the logical start point is slightly shifted on the scanline */
	int parse_line = line - 1;

	/* Check if SI is set */
	m_sprite_height = (m_reg[0x01] & 0x02) ? 16 : 8;
	/* Check if MAG is set */
	m_sprite_zoom = (m_reg[0x01] & 0x01) ? 2 : 1;

	if (m_sprite_zoom > 1)
	{
		/* Divide before use the value for comparison, same later with sprite_y, or
		   else an off-by-one bug could occur, as seen with Tarzan, for Game Gear */
		parse_line >>= 1;
	}

	m_sprite_count = 0;

	if ( m_vdp_mode == 0 || m_vdp_mode == 2 )
	{
		/* TMS9918 compatibility sprites */

		max_sprites = 4;

		m_sprite_base = ((m_reg[0x05] & 0x7f) << 7);

		for (int sprite_index = 0; (sprite_index < 32 * 4) && (m_sprite_count <= max_sprites); sprite_index += 4)
		{
			int sprite_y = space().read_byte(m_sprite_base + sprite_index);
			if (sprite_y == 0xd0)
				break;

			if (sprite_y >= 240)
			{
				sprite_y -= 256; /* wrap from top if y position is >= 240 */
			}

			if (m_sprite_zoom > 1)
			{
				sprite_y >>= 1;
			}

			if ((parse_line >= sprite_y) && (parse_line < (sprite_y + m_sprite_height)))
			{
				if (m_sprite_count < max_sprites)
				{
					int sprite_x = space().read_byte( m_sprite_base + sprite_index + 1 );
					int sprite_tile_selected = space().read_byte( m_sprite_base + sprite_index + 2 );
					UINT8 flags = space().read_byte( m_sprite_base + sprite_index + 3 );

					int sprite_line = parse_line - sprite_y;

					if (m_sprite_height == 16)
					{
						sprite_tile_selected &= 0xfc;

						if (sprite_line > 0x07)
						{
							sprite_tile_selected += 1;
							sprite_line -= 8;
						}
					}

					m_sprite_x[m_sprite_count] = sprite_x;
					m_sprite_tile_selected[m_sprite_count] = sprite_tile_selected;
					m_sprite_flags[m_sprite_count] = flags;
					m_sprite_pattern_line[m_sprite_count] = ((m_reg[0x06] & 0x07) << 11) + sprite_line;
				}
				m_sprite_count++;
			}
		}
	}
	else
	{
		/* Regular sprites */

		max_sprites = 8;

		m_sprite_base = ((m_reg[0x05] << 7) & 0x3f00);

		for (int sprite_index = 0; (sprite_index < 64) && (m_sprite_count <= max_sprites); sprite_index++)
		{
			int sprite_y = space().read_byte(m_sprite_base + sprite_index);
			if (m_y_pixels == 192 && sprite_y == 0xd0)
				break;

			if (sprite_y >= 240)
			{
				sprite_y -= 256; /* wrap from top if y position is >= 240 */
			}

			if (m_sprite_zoom > 1)
			{
				sprite_y >>= 1;
			}

			if ((parse_line >= sprite_y) && (parse_line < (sprite_y + m_sprite_height)))
			{
				if (m_sprite_count < max_sprites)
				{
					int sprite_x = space().read_byte( m_sprite_base + 0x80 + (sprite_index << 1) );
					int sprite_tile_selected = space().read_byte( m_sprite_base + 0x81 + (sprite_index << 1) );

					if (m_reg[0x00] & 0x08)
					{
						sprite_x -= 0x08;    /* sprite shift */
					}

					if (m_reg[0x06] & 0x04)
					{
						sprite_tile_selected += 256; /* pattern table select */
					}

					if (m_sprite_height == 16)
					{
						sprite_tile_selected &= 0x01fe; /* force even index */
					}

					int sprite_line = parse_line - sprite_y;

					if (sprite_line > 0x07)
					{
						sprite_tile_selected += 1;
					}

					m_sprite_x[m_sprite_count] = sprite_x;
					m_sprite_tile_selected[m_sprite_count] = sprite_tile_selected;
					m_sprite_pattern_line[m_sprite_count] = ((sprite_line & 0x07) << 2);
				}
				m_sprite_count++;
			}
		}
	}

	if ( m_sprite_count > max_sprites )
	{
		/* Too many sprites per line */

		m_sprite_count = max_sprites;

		/* Overflow is flagged only on active display and when VINT isn't active */
		if (!(m_status & STATUS_VINT) && line >= 0 && line < m_frame_timing[ACTIVE_DISPLAY_V])
		{
			m_pending_status |= STATUS_SPROVR;
		}
	}
}


void sega315_5124_device::draw_sprites_mode4( int *line_buffer, int *priority_selected, int line )
{
	bool sprite_col_occurred = false;
	int sprite_col_x = 255;
	UINT8 collision_buffer[256];
	int plot_min_x = 0;

	if (m_display_disabled || m_sprite_count == 0)
		return;

	/* Sprites aren't drawn and collisions don't occur on column 0 if it is disabled.
	   Note: On Megadrive/Genesis VDP, collisions occur on the disabled column 0. */
	if (m_reg[0x00] & 0x20)
		plot_min_x = 8;

	memset(collision_buffer, 0, sizeof(collision_buffer));

	/* Draw sprite layer */
	for (int sprite_buffer_index = m_sprite_count - 1; sprite_buffer_index >= 0; sprite_buffer_index--)
	{
		int sprite_x = m_sprite_x[sprite_buffer_index];
		int sprite_tile_selected = m_sprite_tile_selected[sprite_buffer_index];
		UINT16 sprite_pattern_line = m_sprite_pattern_line[sprite_buffer_index];

		UINT8 bit_plane_0 = space().read_byte((sprite_tile_selected << 5) + sprite_pattern_line + 0x00);
		UINT8 bit_plane_1 = space().read_byte((sprite_tile_selected << 5) + sprite_pattern_line + 0x01);
		UINT8 bit_plane_2 = space().read_byte((sprite_tile_selected << 5) + sprite_pattern_line + 0x02);
		UINT8 bit_plane_3 = space().read_byte((sprite_tile_selected << 5) + sprite_pattern_line + 0x03);

		for (int pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			int pixel_plot_x;
			UINT8 pen_bit_0 = (bit_plane_0 >> (7 - pixel_x)) & 0x01;
			UINT8 pen_bit_1 = (bit_plane_1 >> (7 - pixel_x)) & 0x01;
			UINT8 pen_bit_2 = (bit_plane_2 >> (7 - pixel_x)) & 0x01;
			UINT8 pen_bit_3 = (bit_plane_3 >> (7 - pixel_x)) & 0x01;
			UINT8 pen_selected = (pen_bit_3 << 3 | pen_bit_2 << 2 | pen_bit_1 << 1 | pen_bit_0) | 0x10;

			if (pen_selected == 0x10)
			{
				/* Transparent palette so skip draw */
				continue;
			}

			if (m_sprite_zoom > 1)
			{
				/* sprite doubling is enabled */
				pixel_plot_x = sprite_x + (pixel_x << 1);
			}
			else
			{
				pixel_plot_x = sprite_x + pixel_x;
			}

			/* Draw at pixel position and, if zoomed, at pixel+1 */
			for (int zoom = 0; zoom < m_sprite_zoom; zoom++)
			{
				pixel_plot_x += zoom;

				/* check to prevent going outside of active display area */
				if (pixel_plot_x < plot_min_x || pixel_plot_x > 255)
				{
					continue;
				}

				/* Draw sprite pixel */
				/* Check if the background has lower priority */
				if (!(priority_selected[pixel_plot_x] & PRIORITY_BIT))
				{
					line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
					priority_selected[pixel_plot_x] = pen_selected;
				}
				else
				{
					/* Check if the higher priority background has transparent pixel */
					if (priority_selected[pixel_plot_x] == PRIORITY_BIT)
					{
						line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
						priority_selected[pixel_plot_x] = pen_selected;
					}
				}
				if (collision_buffer[pixel_plot_x] != 1)
				{
					collision_buffer[pixel_plot_x] = 1;
				}
				else
				{
					sprite_col_occurred = true;
					sprite_col_x = MIN(sprite_col_x, pixel_plot_x);
				}
			}
		}
		if (sprite_col_occurred)
		{
			m_pending_status |= STATUS_SPRCOL;
			m_pending_sprcol_x = SPRCOL_BASEHPOS + sprite_col_x;
		}
	}
}


void sega315_5124_device::draw_sprites_tms9918_mode( int *line_buffer, int line )
{
	bool sprite_col_occurred = false;
	int sprite_col_x = 255;
	UINT8 collision_buffer[256];

	if (m_display_disabled || m_sprite_count == 0)
		return;

	memset(collision_buffer, 0, sizeof(collision_buffer));

	/* Draw sprite layer */
	for (int sprite_buffer_index = m_sprite_count - 1; sprite_buffer_index >= 0; sprite_buffer_index--)
	{
		int sprite_x = m_sprite_x[sprite_buffer_index];
		int sprite_tile_selected = m_sprite_tile_selected[sprite_buffer_index];
		UINT16 sprite_pattern_line = m_sprite_pattern_line[sprite_buffer_index];
		UINT8 flags = m_sprite_flags[sprite_buffer_index];
		int pen_selected = m_palette_offset + ( flags & 0x0f );

		if (flags & 0x80)
			sprite_x -= 32;

		for (int height = 8; height <= m_sprite_height; height += 8)
		{
			if (height == 16)
			{
				sprite_tile_selected += 2;
				sprite_x += (m_sprite_zoom > 1 ? 16 : 8);
			}

			UINT8 pattern = space().read_byte( sprite_pattern_line + sprite_tile_selected * 8 );

			for (int pixel_x = 0; pixel_x < 8; pixel_x++)
			{
				if (pen_selected && (pattern & (1 << (7 - pixel_x))))
				{
					int pixel_plot_x;
					if (m_sprite_zoom > 1)
					{
						pixel_plot_x = sprite_x + (pixel_x << 1);
					}
					else
					{
						pixel_plot_x = sprite_x + pixel_x;
					}

					/* Draw at pixel position and, if zoomed, at pixel+1 */
					for (int zoom = 0; zoom < m_sprite_zoom; zoom++)
					{
						pixel_plot_x += zoom;

						/* check to prevent going outside of active display area */
						if (pixel_plot_x < 0 || pixel_plot_x > 255)
						{
							continue;
						}

						line_buffer[pixel_plot_x] = m_current_palette[pen_selected];

						if (collision_buffer[pixel_plot_x] != 1)
						{
							collision_buffer[pixel_plot_x] = 1;
						}
						else
						{
							sprite_col_occurred = true;
							sprite_col_x = MIN(sprite_col_x, pixel_plot_x);
						}
					}
				}
			}
		}
		if (sprite_col_occurred)
		{
			m_pending_status |= STATUS_SPRCOL;
			m_pending_sprcol_x = SPRCOL_BASEHPOS + sprite_col_x;
		}
	}
}


void sega315_5124_device::draw_scanline_mode2( int *line_buffer, int line )
{
	int tile_column;
	int pixel_x, pixel_plot_x;
	UINT16 name_table_base, color_base, pattern_base;
	int pattern_mask, color_mask, pattern_offset;

	name_table_base =  ((m_reg[0x02] & 0x0f) << 10) + ((line >> 3) * 32);
	color_base = ((m_reg[0x03] & 0x80) << 6);
	color_mask = ((m_reg[0x03] & 0x7f) << 3) | 0x07;
	pattern_base = ((m_reg[0x04] & 0x04) << 11);
	pattern_mask = ((m_reg[0x04] & 0x03) << 8) | 0xff;
	pattern_offset = (line & 0xc0) << 2;

	/* Draw background layer */
	for (tile_column = 0; tile_column < 32; tile_column++)
	{
		UINT8 name = space().read_byte( name_table_base + tile_column );
		UINT8 pattern;
		UINT8 colors;

		pattern = space().read_byte(pattern_base + (((pattern_offset + name) & pattern_mask) * 8) + (line & 0x07) );
		colors = space().read_byte(color_base + (((pattern_offset + name) & color_mask) * 8) + (line & 0x07) );

		for (pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			UINT8 pen_selected;

			if (pattern & (1 << (7 - pixel_x)))
			{
				pen_selected = colors >> 4;
			}
			else
			{
				pen_selected = colors & 0x0f;
			}

			if (!pen_selected)
				pen_selected = BACKDROP_COLOR;

			pixel_plot_x = (tile_column << 3) + pixel_x;

			pen_selected += m_palette_offset;

			line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
		}
	}
}


void sega315_5124_device::draw_scanline_mode0( int *line_buffer, int line )
{
	int tile_column;
	int pixel_x, pixel_plot_x;
	UINT16 name_base, color_base, pattern_base;

	name_base = ((m_reg[0x02] & 0x0f) << 10) + ((line >> 3) * 32);
	color_base = ((m_reg[0x03] << 6) & (VRAM_SIZE - 1));
	pattern_base = ((m_reg[0x04] << 11) & (VRAM_SIZE - 1));

	/* Draw background layer */
	for (tile_column = 0; tile_column < 32; tile_column++)
	{
		UINT8 name = space().read_byte( name_base + tile_column );
		UINT8 pattern;
		UINT8 colors;

		pattern = space().read_byte( pattern_base + (name * 8) + (line & 0x07) );
		colors = space().read_byte( color_base + ( name >> 3 ) );

		for (pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			int pen_selected;

			if (pattern & (1 << (7 - pixel_x)))
				pen_selected = colors >> 4;
			else
				pen_selected = colors & 0x0f;

			pen_selected += m_palette_offset;

			pixel_plot_x = (tile_column << 3) + pixel_x;
			line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
		}
	}
}


void sega315_5124_device::draw_scanline( int pixel_offset_x, int pixel_plot_y, int line )
{
	int blitline_buffer[256];
	int priority_selected[256];

	/* Sprite processing is restricted because collisions on top border of extended
	   resolution break the scoreboard of Fantasy Dizzy (SMS) on smspal driver */

	if ( line < m_frame_timing[ACTIVE_DISPLAY_V] )
	{
		memset(priority_selected, 1, sizeof(priority_selected));

		switch( m_vdp_mode )
		{
		case 0:
			if ( line >= 0 )
			{
				draw_scanline_mode0( blitline_buffer, line );
			}
			if ( line >= 0 || ( line >= -13 && m_y_pixels == 192 ) )
			{
				draw_sprites_tms9918_mode( blitline_buffer, line );
			}
			break;

		case 2:
			if ( line >= 0 )
			{
				draw_scanline_mode2( blitline_buffer, line );
			}
			if ( line >= 0 || ( line >= -13 && m_y_pixels == 192 ) )
			{
				draw_sprites_tms9918_mode( blitline_buffer, line );
			}
			break;

		case 4:
		default:
			if ( line >= 0 )
			{
				draw_scanline_mode4( blitline_buffer, priority_selected, line );
			}
			if ( line >= 0 || ( line >= -13 && m_y_pixels == 192 ) )
			{
				draw_sprites_mode4( blitline_buffer, priority_selected, line );
			}
			break;
		}
	}

	/* Check if display is disabled or we're below/above active area */
	if (m_display_disabled || line < 0 || line >= m_frame_timing[ACTIVE_DISPLAY_V])
	{
		rectangle rec;
		rec.min_y = rec.max_y = pixel_plot_y + line;

		rec.min_x = pixel_offset_x;
		rec.max_x = pixel_offset_x + 255;
		m_tmpbitmap.fill(m_palette->pen(m_current_palette[BACKDROP_COLOR]), rec);
		m_y1_bitmap.fill(( m_reg[0x07] & 0x0f ) ? 1 : 0, rec);
	}
	else
	{
		blit_scanline(blitline_buffer, priority_selected, pixel_offset_x, pixel_plot_y, line);
	}
}


void sega315_5124_device::blit_scanline( int *line_buffer, int *priority_selected, int pixel_offset_x, int pixel_plot_y, int line )
{
	UINT32 *p_bitmap = &m_tmpbitmap.pix32(pixel_plot_y + line, pixel_offset_x);
	UINT8  *p_y1 = &m_y1_bitmap.pix8(pixel_plot_y + line, pixel_offset_x);
	int x = 0;

	if (m_vdp_mode == 4 && (m_reg[0x00] & 0x20))
	{
		/* Fill column 0 with overscan color from m_reg[0x07] */
		do
		{
			p_bitmap[x] = m_palette->pen(m_current_palette[BACKDROP_COLOR]);
			p_y1[x] = ( m_reg[0x07] & 0x0f ) ? 1 : 0;
		}
		while(++x < 8);
	}

	do
	{
		p_bitmap[x] = m_palette->pen(line_buffer[x]);
		p_y1[x] = ( priority_selected[x] & 0x0f ) ? 1 : 0;
	}
	while(++x < 256);
}


void sega315_5378_device::blit_scanline( int *line_buffer, int *priority_selected, int pixel_offset_x, int pixel_plot_y, int line )
{
	if (m_sega315_5124_compatibility_mode)
	{
		sega315_5124_device::blit_scanline(line_buffer, priority_selected, pixel_offset_x, pixel_plot_y, line);
	}
	else
	{
		UINT32 *p_bitmap = &m_tmpbitmap.pix32(pixel_plot_y + line, pixel_offset_x);
		UINT8  *p_y1 = &m_y1_bitmap.pix8(pixel_plot_y + line, pixel_offset_x);
		int x = 0;

		/* border on left side of the GG active screen */
		do
		{
			p_bitmap[x] = m_palette->pen(m_current_palette[BACKDROP_COLOR]);
			p_y1[x] = ( m_reg[0x07] & 0x0f ) ? 1 : 0;
		}
		while (++x < 48);

		if ( line >= 24 && line < 168 )
		{
			do
			{
				p_bitmap[x] = m_palette->pen(line_buffer[x]);
				p_y1[x] = ( priority_selected[x] & 0x0f ) ? 1 : 0;
			}
			while (++x < 208);
		}
		else
		{
			/* top/bottom GG border */
			do
			{
				p_bitmap[x] = m_palette->pen(m_current_palette[BACKDROP_COLOR]);
				p_y1[x] = ( m_reg[0x07] & 0x0f ) ? 1 : 0;
			}
			while (++x < 208);
		}

		/* border on right side of the GG active screen */
		do
		{
			p_bitmap[x] = m_palette->pen(m_current_palette[BACKDROP_COLOR]);
			p_y1[x] = ( m_reg[0x07] & 0x0f ) ? 1 : 0;
		}
		while (++x < 256);
	}
}


void sega315_5124_device::update_palette()
{
	int i;

	/* Exit if palette has no changes */
	if (m_cram_dirty == 0)
	{
		return;
	}
	m_cram_dirty = 0;

	if (m_vdp_mode != 4)
	{
		for(i = 0; i < 16; i++)
		{
			m_current_palette[i] = 64 + i;
		}
		return;
	}

	for (i = 0; i < 32; i++)
	{
		m_current_palette[i] = m_CRAM[i] & 0x3f;
	}
}


void sega315_5378_device::update_palette()
{
	int i;

	/* Exit if palette has no changes */
	if (m_cram_dirty == 0)
	{
		return;
	}
	m_cram_dirty = 0;

	if (m_sega315_5124_compatibility_mode)
	{
		for (i = 0; i < 32; i++)
		{
			m_current_palette[i] = ((m_CRAM[i] & 0x30) << 6) | ((m_CRAM[i] & 0x0c ) << 4) | ((m_CRAM[i] & 0x03) << 2);
		}
	}
	else
	{
		for (i = 0; i < 32; i++)
		{
			m_current_palette[i] = (m_CRAM[2*i] | (m_CRAM[2*i+1] << 8)) & 0x0fff;
		}
	}
}


void sega315_5124_device::cram_write(UINT8 data)
{
	UINT16 address = m_addr & m_cram_mask;
	if (data != m_CRAM[address])
	{
		m_CRAM[address] = data;
		m_cram_dirty = 1;
	}
}


void sega315_5378_device::cram_write(UINT8 data)
{
	if (m_sega315_5124_compatibility_mode)
	{
		sega315_5124_device::cram_write(data);
	}
	else
	{
		if (m_addr & 1)
		{
			UINT16 address = (m_addr & m_cram_mask) & ~1;
			if (m_buffer != m_CRAM[address] || data != m_CRAM[address + 1])
			{
				m_CRAM[address] = m_buffer;
				m_CRAM[address + 1] = data;
				m_cram_dirty = 1;
			}
		}
	}
}


UINT32 sega315_5124_device::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	copybitmap(bitmap, m_tmpbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


// MegaDrive/Genesis VDP (315-5313) is currently coded as superset of the 315-5124
// To support properly SMS VDP in MegaTech and MegaPlay, we start the 315-5124
// in all systems using MegaDrive/Genesis VDP, but this affects the performance
// of the emulator hence we stop it in systems that don't need it
// Proper way to handle this would be implement the 315-5124 modes in the 315-5313
// device instead of running the two chips separately...
void sega315_5124_device::stop_timers()
{
	m_display_timer->adjust(attotime::never);
	m_pending_flags_timer->adjust(attotime::never);
	m_hint_timer->adjust(attotime::never);
	m_vint_timer->adjust(attotime::never);
	m_nmi_timer->adjust(attotime::never);
	m_draw_timer->adjust(attotime::never);
	m_lborder_timer->adjust(attotime::never);
	m_rborder_timer->adjust(attotime::never);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

void sega315_5124_device::vdp_postload()
{
	set_frame_timing();
}

void sega315_5124_device::device_start()
{
	/* Resolve callbacks */
	m_int_cb.resolve();
	m_pause_cb.resolve();

	/* Allocate video RAM */
	m_frame_timing = (m_is_pal) ? pal_192 : ntsc_192;

	/* Make temp bitmap for rendering */
	m_screen->register_screen_bitmap(m_tmpbitmap);
	m_screen->register_screen_bitmap(m_y1_bitmap);

	m_display_timer = timer_alloc(TIMER_LINE);
	m_display_timer->adjust(m_screen->time_until_pos(0, DISPLAY_CB_HPOS), 0, m_screen->scan_period());
	m_pending_flags_timer = timer_alloc(TIMER_FLAGS);
	m_pending_flags_timer->adjust(m_screen->time_until_pos(0, SEGA315_5124_WIDTH - 1), 0, m_screen->scan_period());
	m_draw_timer = timer_alloc(TIMER_DRAW);
	m_lborder_timer = timer_alloc(TIMER_LBORDER);
	m_rborder_timer = timer_alloc(TIMER_RBORDER);
	m_hint_timer = timer_alloc(TIMER_HINT);
	m_vint_timer = timer_alloc(TIMER_VINT);
	m_nmi_timer = timer_alloc(TIMER_NMI);

	save_item(NAME(m_status));
	save_item(NAME(m_pending_status));
	save_item(NAME(m_pending_sprcol_x));
	save_item(NAME(m_reg8copy));
	save_item(NAME(m_reg9copy));
	save_item(NAME(m_addrmode));
	save_item(NAME(m_addr));
	save_item(NAME(m_cram_mask));
	save_item(NAME(m_cram_dirty));
	save_item(NAME(m_pending_reg_write));
	save_item(NAME(m_buffer));
	save_item(NAME(m_sega315_5124_compatibility_mode));
	save_item(NAME(m_display_disabled));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_vdp_mode));
	save_item(NAME(m_y_pixels));
	save_item(NAME(m_line_counter));
	save_item(NAME(m_hcounter));
	save_item(NAME(m_reg));
	save_item(NAME(m_current_palette));

	// these were created with register_screen_bitmap which is dynamic, and will reallocate if the screen size changes, saving them is NOT safe with the current core.
	// The Genesis VDP (315_5313.c) which uses this as a base in order to support the legacy SMS operaiton mode can change resolutions for example.
	//save_item(NAME(m_tmpbitmap));
	//save_item(NAME(m_y1_bitmap));
	save_item(NAME(m_draw_time));
	save_item(NAME(m_sprite_base));
	save_item(NAME(m_sprite_pattern_line));
	save_item(NAME(m_sprite_tile_selected));
	save_item(NAME(m_sprite_x));
	save_item(NAME(m_sprite_flags));
	save_item(NAME(m_sprite_count));
	save_item(NAME(m_sprite_height));
	save_item(NAME(m_sprite_zoom));
	save_item(NAME(m_CRAM));

	machine().save().register_postload(save_prepost_delegate(FUNC(sega315_5124_device::vdp_postload), this));
}


void sega315_5124_device::device_reset()
{
	int i;

	/* Most register are 0x00 at power-up */
	for (i = 0; i < 16; i++)
		m_reg[i] = 0x00;

	m_reg[0x02] = 0x0e;
	m_reg[0x0a] = 0xff;

	m_status = 0;
	m_pending_status = 0;
	m_pending_sprcol_x = 0;
	m_pending_reg_write = 0;
	m_reg8copy = 0;
	m_reg9copy = 0;
	m_addrmode = 0;
	m_addr = 0;
	m_sega315_5124_compatibility_mode = false;
	m_display_disabled = false;
	m_cram_mask = m_cram_size - 1;
	m_cram_dirty = 1;
	m_buffer = 0;
	m_irq_state = 0;
	m_line_counter = 0;
	m_hcounter = 0;
	m_draw_time = DRAW_TIME_SMS;

	for (i = 0; i < 0x20; i++)
		m_current_palette[i] = 0;

	set_display_settings();

	/* Clear RAM */
	memset(m_CRAM, 0, sizeof(m_CRAM));
}

static MACHINE_CONFIG_FRAGMENT( sega315_5124 )
	MCFG_PALETTE_ADD("palette", SEGA315_5124_PALETTE_SIZE)
	MCFG_PALETTE_INIT_OWNER(sega315_5124_device, sega315_5124)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor sega315_5124_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sega315_5124 );
}


void sega315_5378_device::device_reset()
{
	sega315_5124_device::device_reset();
	m_draw_time = DRAW_TIME_GG;
}

static MACHINE_CONFIG_FRAGMENT( sega315_5378 )
	MCFG_PALETTE_ADD("palette", SEGA315_5378_PALETTE_SIZE)
	MCFG_PALETTE_INIT_OWNER(sega315_5378_device, sega315_5378)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor sega315_5378_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sega315_5378 );
}
