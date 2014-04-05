/*********************************************************************

    sega315_5124.c

    Implementation of video hardware chips used by Sega System E,
    Master System, and Game Gear.

**********************************************************************/

/*

To do:

  - Display mode 1 (text)
  - Display mode 3 (multicolor)


SMS Display Timing
------------------
    For more information, please see:
    - http://cgfm2.emuviews.com/txt/msvdp.txt
    - http://www.smspower.org/forums/viewtopic.php?p=44198

A scanline contains the following sections:
  - horizontal sync     1  ED      => HSYNC high/increment line counter/generate interrupts/etc
  - left blanking       2  ED-EE
  - color burst        14  EE-EF
  - left blanking       8  F5-F9
  - left border        13  F9-FF
  - active display    256  00-7F
  - right border       15  80-87
  - right blanking      8  87-8B
  - horizontal sync    25  8B-97   => HSYNC low


NTSC frame timing
                       256x192         256x224        256x240 (doesn't work on real hardware)
  - vertical blanking   3  D5-D7        3  E5-E7       3  EE-F0
  - top blanking       13  D8-E4       13  E8-F4      13  F1-FD
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
#define VINT_FLAG_HPOS        23
#define HINT_HPOS             26
#define VCOUNT_CHANGE_HPOS    23
#define SPROVR_HPOS           23
#define SPRCOL_BASEHPOS       59
#define X_SCROLL_HPOS         21

/* Display callback delay that fixes a flicker issue at top of screen when
   playing 'fantdizzy' (SMS PAL game), due to time when display is disabled. */
#define DISPLAY_CB_HPOS       6

#define DRAW_TIME_GG        86      /* 1 + 2 + 14 + 8 + 96/2 */
#define DRAW_TIME_SMS       25      /* 1 + 2 + 14 + 8 */

#define PRIORITY_BIT          0x1000
#define BACKDROP_COLOR        ((m_vdp_mode == 4 ? 0x10 : 0x00) + (m_reg[0x07] & 0x0f))

#define INIT_VCOUNT           0
#define VERTICAL_BLANKING     1
#define TOP_BLANKING          2
#define TOP_BORDER            3
#define ACTIVE_DISPLAY_V      4
#define BOTTOM_BORDER         5
#define BOTTOM_BLANKING       6

static const UINT8 ntsc_192[7] = { 0xd5, 3, 13, 27, 192, 24, 3 };
static const UINT8 ntsc_224[7] = { 0xe5, 3, 13, 11, 224,  8, 3 };
static const UINT8 ntsc_240[7] = { 0xee, 3, 13,  3, 240,  0, 3 };
static const UINT8 pal_192[7]  = { 0xba, 3, 13, 54, 192, 48, 3 };
static const UINT8 pal_224[7]  = { 0xca, 3, 13, 38, 224, 32, 3 };
static const UINT8 pal_240[7]  = { 0xd2, 3, 13, 30, 240, 24, 3 };


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
	/* TMS9918 palette */
	palette.set_pen_color(64+ 0,   0,   0,   0);
	palette.set_pen_color(64+ 1,   0,   0,   0);
	palette.set_pen_color(64+ 2,  33, 200,  66);
	palette.set_pen_color(64+ 3,  94, 220, 120);
	palette.set_pen_color(64+ 4,  84,  85, 237);
	palette.set_pen_color(64+ 5, 125, 118, 252);
	palette.set_pen_color(64+ 6, 212,  82,  77);
	palette.set_pen_color(64+ 7,  66, 235, 245);
	palette.set_pen_color(64+ 8, 252,  85,  84);
	palette.set_pen_color(64+ 9, 255, 121, 120);
	palette.set_pen_color(64+10, 212, 193,  84);
	palette.set_pen_color(64+11, 230, 206, 128);
	palette.set_pen_color(64+12,  33, 176,  59);
	palette.set_pen_color(64+13, 201,  91, 186);
	palette.set_pen_color(64+14, 204, 204, 204);
	palette.set_pen_color(64+15, 255, 255, 255);
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
	: device_t( mconfig, SEGA315_5124, "Sega 315-5124", tag, owner, clock, "sega315_5124", __FILE__)
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
	: sega315_5124_device( mconfig, SEGA315_5246, "Sega 315-5246", tag, owner, clock, SEGA315_5124_CRAM_SIZE, 0, true, "sega315_5246", __FILE__)
{
}


sega315_5378_device::sega315_5378_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: sega315_5124_device( mconfig, SEGA315_5378, "Sega 315-5378", tag, owner, clock, SEGA315_5378_CRAM_SIZE, 0x10, true, "sega315_5378", __FILE__)
{
}


void sega315_5124_device::set_display_settings()
{
	bool M1 = m_reg[0x01] & 0x10;
	bool M2 = m_reg[0x00] & 0x02;
	bool M3 = m_reg[0x01] & 0x08;
	bool M4 = m_reg[0x00] & 0x04;

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
	m_cram_dirty = 1;
}


READ8_MEMBER( sega315_5124_device::vcount_read )
{
	int vpos = m_screen->vpos();

	if (m_screen->hpos() < VCOUNT_CHANGE_HPOS)
	{
		vpos--;
		if (vpos < 0)
			vpos += m_screen->height();
	}

	return (m_frame_timing[INIT_VCOUNT] + vpos) & 0xff;
}


READ8_MEMBER( sega315_5124_device::hcount_read )
{
	return m_hcounter;
}


void sega315_5124_device::hcount_latch_at_hpos( int hpos )
{
	/* The emulation core returns a screen hpos that is one position ahead in comparison
	   with the expected VDP hclock value, if the same range is used (from 0 to width-1). */
	int hclock = hpos - 1;
	if (hclock < 0)
		hclock += m_screen->width();

	/* Calculate and store the new hcount. */
	m_hcounter = ((hclock - 46) >> 1) & 0xff;
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

	case TIMER_DRAW:
		update_palette();
		draw_scanline( SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH, param, m_screen->vpos() - param );
		break;

	case TIMER_CHECK_HINT:
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

	case TIMER_CHECK_VINT:
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
	}
}


void sega315_5124_device::process_line_timer()
{
	rectangle rec;
	int vpos = m_screen->vpos();
	int vpos_limit = m_frame_timing[VERTICAL_BLANKING] + m_frame_timing[TOP_BLANKING]
					+ m_frame_timing[TOP_BORDER] + m_frame_timing[ACTIVE_DISPLAY_V]
					+ m_frame_timing[BOTTOM_BORDER] + m_frame_timing[BOTTOM_BLANKING];

	rec.min_y = rec.max_y = vpos;

	m_display_disabled = !(m_reg[0x01] & 0x40);

	/* Activate flags that were pending until the end of previous line. */
	check_pending_flags(m_screen->width());

	/* Check if we're on the last line of a frame */
	if (vpos == vpos_limit - 1)
	{
		m_line_counter = m_reg[0x0a];
		if ( !m_pause_cb.isnull() )
			m_pause_cb(0);

		return;
	}

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
				m_check_hint_timer->adjust( m_screen->time_until_pos( vpos, HINT_HPOS ) );
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
			m_check_vint_timer->adjust( m_screen->time_until_pos( vpos, VINT_HPOS ) );
			m_pending_status |= STATUS_VINT;
		}

		update_palette();

		/* Draw left border */
		rec.min_x = SEGA315_5124_LBORDER_START;
		rec.max_x = SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 1;
		m_tmpbitmap.fill(m_palette->pen(m_current_palette[BACKDROP_COLOR]), rec);
		m_y1_bitmap.fill(1, rec);

		/* Draw right border */
		rec.min_x = SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256;
		rec.max_x = rec.min_x + SEGA315_5124_RBORDER_WIDTH - 1;
		m_tmpbitmap.fill(m_palette->pen(m_current_palette[BACKDROP_COLOR]), rec);
		m_y1_bitmap.fill(1, rec);

		/* Draw middle of the border */
		/* We need to do this through the regular drawing function so it will */
		/* be included in the gamegear scaling functions */
		draw_scanline( SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH, vpos_limit - m_frame_timing[ACTIVE_DISPLAY_V], vpos - (vpos_limit - m_frame_timing[ACTIVE_DISPLAY_V]) );
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
		m_reg8copy = m_reg[0x08];

		if (m_line_counter == 0x00)
		{
			m_line_counter = m_reg[0x0a];
			m_check_hint_timer->adjust( m_screen->time_until_pos( vpos, HINT_HPOS ) );
			m_pending_status |= STATUS_HINT;
		}
		else
		{
			m_line_counter--;
		}

		update_palette();

		/* Draw left border */
		rec.min_x = SEGA315_5124_LBORDER_START;
		rec.max_x = SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 1;
		m_tmpbitmap.fill(m_palette->pen(m_current_palette[BACKDROP_COLOR]), rec);
		m_y1_bitmap.fill(1, rec);

		/* Draw right border */
		rec.min_x = SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256;
		rec.max_x = rec.min_x + SEGA315_5124_RBORDER_WIDTH - 1;
		m_tmpbitmap.fill(m_palette->pen(m_current_palette[BACKDROP_COLOR]), rec);
		m_y1_bitmap.fill(1, rec);

		select_sprites( vpos - vpos_limit );
		if ( m_draw_time > 0 )
		{
			m_draw_timer->adjust( m_screen->time_until_pos( vpos, m_draw_time ), vpos_limit );
		}
		else
		{
			draw_scanline( SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH, vpos_limit, vpos - vpos_limit );
		}
		return;
	}

	vpos_limit -= m_frame_timing[TOP_BORDER];

	/* Check if we're in the top border area */
	if (vpos >= vpos_limit)
	{
		m_line_counter = m_reg[0x0a];
		update_palette();

		/* Draw left border */
		rec.min_x = SEGA315_5124_LBORDER_START;
		rec.max_x = SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 1;
		m_tmpbitmap.fill(m_palette->pen(m_current_palette[BACKDROP_COLOR]), rec);
		m_y1_bitmap.fill(1, rec);

		/* Draw right border */
		rec.min_x = SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256;
		rec.max_x = rec.min_x + SEGA315_5124_RBORDER_WIDTH - 1;
		m_tmpbitmap.fill(m_palette->pen(m_current_palette[BACKDROP_COLOR]), rec);
		m_y1_bitmap.fill(1, rec);

		/* Draw middle of the border */
		/* We need to do this through the regular drawing function so it will */
		/* be included in the gamegear scaling functions */
		select_sprites( vpos - (vpos_limit + m_frame_timing[TOP_BORDER]) );
		draw_scanline( SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH, vpos_limit + m_frame_timing[TOP_BORDER], vpos - (vpos_limit + m_frame_timing[TOP_BORDER]) );
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


void sega315_5124_device::check_pending_flags( int hpos )
{
	if ((m_pending_status & STATUS_HINT) && hpos >= HINT_HPOS)
	{
		m_pending_status &= ~STATUS_HINT;
		m_status |= STATUS_HINT;
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

	check_pending_flags(m_screen->hpos());
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
			{
				UINT16 address = m_addr & m_cram_mask;
				if (data != m_CRAM->u8(address))
				{
					m_CRAM->u8(address) = data;
					m_cram_dirty = 1;
				}
			}
			break;
	}

	m_buffer = data;
	m_addr += 1;
}


WRITE8_MEMBER( sega315_5124_device::register_write )
{
	int reg_num;
	int hpos = m_screen->hpos();

	check_pending_flags(hpos);

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
			//logerror("%s: %s: setting register %x to %02x\n", machine().describe_context(), tag(), reg_num, m_addr & 0xf );
			if ( reg_num == 0 && ( m_addr & 0x02 ) )
				logerror("overscan enabled.\n");

			if (reg_num == 0 || reg_num == 1)
				set_display_settings();

			if (reg_num == 8 && hpos <= X_SCROLL_HPOS)
				m_reg8copy = m_reg[0x08];

			if ( ( reg_num == 0 && (m_status & STATUS_HINT) ) ||
					( reg_num == 1 && (m_status & STATUS_VINT) ) )
			{
				// For HINT disabling through register 00:
				// "Line IRQ VCount" test, of Flubba's VDPTest ROM, disables HINT to wait
				// for next VINT, but HINT occurs when the operation is about to execute.
				// So here, where the setting is done, the irq_state needs to be cleared.
				//
				// For VINT disabling through register 01:
				// When running eagles5 on the sms2kr driver the irq_state is 1 because of some
				// previos HINTs that occured. eagles5 sets register 01 to 0x02 and expects
				// the irq state to be cleared after that.
				// The following bit of code takes care of that.
				//
				if ( ( (m_status & STATUS_HINT) && !(m_reg[0x00] & 0x10) ) ||
						( (m_status & STATUS_VINT) && !(m_reg[0x01] & 0x20) ) )
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


UINT16 sega315_5124_device::get_name_table_address()
{
	UINT16 result;

	if ( m_y_pixels != 192 )
	{
		result = ((m_reg[0x02] & 0x0c) << 10) | 0x0700;
	}
	else
	{
		result = (m_reg[0x02] << 10) & 0x3800;
	}

	return result & (((m_reg[0x02] & 0x01) << 10) | 0x3bff);
}


UINT16 sega315_5246_device::get_name_table_address()
{
	UINT16 result;

	if ( m_y_pixels != 192 )
	{
		result = ((m_reg[0x02] & 0x0c) << 10) | 0x0700;
	}
	else
	{
		result = (m_reg[0x02] << 10) & 0x3800;
	}

	return result;
}


UINT16 sega315_5378_device::get_name_table_address()
{
	UINT16 result;

	if ( m_y_pixels != 192 )
	{
		result = ((m_reg[0x02] & 0x0c) << 10) | 0x0700;
	}
	else
	{
		result = (m_reg[0x02] << 10) & 0x3800;
	}

	return result;
}


void sega315_5124_device::draw_scanline_mode4( int *line_buffer, int *priority_selected, int line )
{
	int tile_column;
	int x_scroll, y_scroll, x_scroll_start_column;
	int pixel_x, pixel_plot_x;
	int bit_plane_0, bit_plane_1, bit_plane_2, bit_plane_3;
	int scroll_mod = ( m_y_pixels != 192 ) ? 256 : 224;
	UINT16 name_table_address = get_name_table_address();

	/* if top 2 rows of screen not affected by horizontal scrolling, then x_scroll = 0 */
	/* else x_scroll = m_reg8copy                                                      */
	x_scroll = (((m_reg[0x00] & 0x40) && (line < 16)) ? 0 : 0x0100 - m_reg8copy);

	x_scroll_start_column = (x_scroll >> 3);             /* x starting column tile */

	/* Draw background layer */
	for (tile_column = 0; tile_column < 33; tile_column++)
	{
		UINT16 tile_data;
		int tile_selected, palette_selected, horiz_selected, vert_selected, priority_select;
		int tile_line;

		/* Rightmost 8 columns for SMS (or 2 columns for GG) not affected by */
		/* vertical scrolling when bit 7 of reg[0x00] is set */
		y_scroll = ((m_reg[0x00] & 0x80) && (tile_column > 23)) ? 0 : m_reg9copy;

		tile_line = ((tile_column + x_scroll_start_column) & 0x1f) * 2;
		tile_data = space().read_word(name_table_address + ((((line + y_scroll) % scroll_mod) >> 3) << 6) + tile_line);

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

		for (pixel_x = 0; pixel_x < 8 ; pixel_x++)
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
				line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
				priority_selected[pixel_plot_x] = priority_select | (pen_selected & 0x0f);
			}
		}
	}
}


void sega315_5124_device::select_sprites( int line )
{
	int sprite_index = 0;
	int max_sprites = 0;

	m_sprite_count = 0;
	m_sprite_base = ((m_reg[0x05] << 7) & 0x3f00);

	if ( m_vdp_mode == 0 || m_vdp_mode == 2 )
	{
		/* TMS9918 compatibility sprites */

		max_sprites = 4;

		m_sprite_base = ((m_reg[0x05] & 0x7f) << 7);
		m_sprite_height = 8;

		if (m_reg[0x01] & 0x02)                         /* Check if SI is set */
			m_sprite_height = m_sprite_height * 2;
		if (m_reg[0x01] & 0x01)                         /* Check if MAG is set */
			m_sprite_height = m_sprite_height * 2;

		for (sprite_index = 0; (sprite_index < 32 * 4) && (m_sprite_count <= max_sprites); sprite_index += 4)
		{
			int sprite_y = space().read_byte(m_sprite_base + sprite_index);
			if (sprite_y == 0xd0)
				break;
			sprite_y += 1;

			if (sprite_y > 240)
			{
				sprite_y -= 256;
			}

			if ((line >= sprite_y) && (line < (sprite_y + m_sprite_height)))
			{
				if (m_sprite_count < max_sprites)
				{
					m_selected_sprite[m_sprite_count] = sprite_index;
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
		m_sprite_height = (m_reg[0x01] & 0x02) ? 16 : 8;
		m_sprite_zoom = (m_reg[0x01] & 0x01) ? 2 : 1;

		for (sprite_index = 0; (sprite_index < 64) && (m_sprite_count <= max_sprites); sprite_index++)
		{
			int sprite_y = space().read_byte(m_sprite_base + sprite_index);
			if (m_y_pixels == 192 && sprite_y == 0xd0)
				break;
			sprite_y += 1; /* sprite y position starts at line 1 */

			if (sprite_y > 240)
			{
				sprite_y -= 256; /* wrap from top if y position is > 240 */
			}

			if ((line >= sprite_y) && (line < (sprite_y + m_sprite_height * m_sprite_zoom)))
			{
				if (m_sprite_count < max_sprites)
				{
					m_selected_sprite[m_sprite_count] = sprite_index;
				}
				m_sprite_count++;
			}
		}
	}

	if ( m_sprite_count > max_sprites )
	{
		/* Too many sprites per line */

		m_sprite_count = max_sprites;

		if (line >= 0 && line < m_frame_timing[ACTIVE_DISPLAY_V])
		{
			m_pending_status |= STATUS_SPROVR;
		}
	}
}


void sega315_5124_device::draw_sprites_mode4( int *line_buffer, int *priority_selected, int line )
{
	bool sprite_col_occurred = false;
	int sprite_col_x = m_screen->width();

	/* Draw sprite layer */

	/* Check if display is disabled */
	if (m_display_disabled)
		return;

	memset(m_collision_buffer, 0, SEGA315_5124_WIDTH);

	for (int sprite_buffer_index = m_sprite_count - 1; sprite_buffer_index >= 0; sprite_buffer_index--)
	{
		int sprite_index = m_selected_sprite[sprite_buffer_index];
		int sprite_y = space().read_byte( m_sprite_base + sprite_index ) + 1; /* sprite y position starts at line 1 */
		int sprite_x = space().read_byte( m_sprite_base + 0x80 + (sprite_index << 1) );
		int sprite_tile_selected = space().read_byte( m_sprite_base + 0x81 + (sprite_index << 1) );

		if (sprite_y > 240)
		{
			sprite_y -= 256; /* wrap from top if y position is > 240 */
		}

		if (m_reg[0x00] & 0x08)
		{
			sprite_x -= 0x08;    /* sprite shift */
		}

		if (m_reg[0x06] & 0x04)
		{
			sprite_tile_selected += 256; /* pattern table select */
		}

		if (m_reg[0x01] & 0x02)
		{
			sprite_tile_selected &= 0x01fe; /* force even index */
		}

		int sprite_line = (line - sprite_y) / m_sprite_zoom;

		if (sprite_line > 0x07)
		{
			sprite_tile_selected += 1;
		}

		UINT8 bit_plane_0 = space().read_byte(((sprite_tile_selected << 5) + ((sprite_line & 0x07) << 2)) + 0x00);
		UINT8 bit_plane_1 = space().read_byte(((sprite_tile_selected << 5) + ((sprite_line & 0x07) << 2)) + 0x01);
		UINT8 bit_plane_2 = space().read_byte(((sprite_tile_selected << 5) + ((sprite_line & 0x07) << 2)) + 0x02);
		UINT8 bit_plane_3 = space().read_byte(((sprite_tile_selected << 5) + ((sprite_line & 0x07) << 2)) + 0x03);

		for (int pixel_x = 0; pixel_x < 8 ; pixel_x++)
		{
			UINT8 pen_bit_0 = (bit_plane_0 >> (7 - pixel_x)) & 0x01;
			UINT8 pen_bit_1 = (bit_plane_1 >> (7 - pixel_x)) & 0x01;
			UINT8 pen_bit_2 = (bit_plane_2 >> (7 - pixel_x)) & 0x01;
			UINT8 pen_bit_3 = (bit_plane_3 >> (7 - pixel_x)) & 0x01;
			UINT8 pen_selected = (pen_bit_3 << 3 | pen_bit_2 << 2 | pen_bit_1 << 1 | pen_bit_0) | 0x10;

			if (pen_selected == 0x10)       /* Transparent palette so skip draw */
			{
				continue;
			}

			if (m_sprite_zoom > 1)
			{
				/* sprite doubling is enabled */
				int pixel_plot_x = sprite_x + (pixel_x << 1);

				/* check to prevent going outside of active display area */
				if (pixel_plot_x < 0 || pixel_plot_x > 255)
				{
					continue;
				}

				if (!(priority_selected[pixel_plot_x] & PRIORITY_BIT))
				{
					line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
					priority_selected[pixel_plot_x] = pen_selected;
					line_buffer[pixel_plot_x + 1] = m_current_palette[pen_selected];
					priority_selected[pixel_plot_x + 1] = pen_selected;
				}
				else
				{
					if (priority_selected[pixel_plot_x] == PRIORITY_BIT)
					{
						line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
						priority_selected[pixel_plot_x] = pen_selected;
					}
					if (priority_selected[pixel_plot_x + 1] == PRIORITY_BIT)
					{
						line_buffer[pixel_plot_x + 1] = m_current_palette[pen_selected];
						priority_selected[pixel_plot_x + 1] = pen_selected;
					}
				}
				if (m_collision_buffer[pixel_plot_x] != 1)
				{
					m_collision_buffer[pixel_plot_x] = 1;
				}
				else
				{
					sprite_col_occurred = true;
					sprite_col_x = MIN(sprite_col_x, pixel_plot_x);
				}
				if (m_collision_buffer[pixel_plot_x + 1] != 1)
				{
					m_collision_buffer[pixel_plot_x + 1] = 1;
				}
				else
				{
					sprite_col_occurred = true;
					sprite_col_x = MIN(sprite_col_x, pixel_plot_x);
				}
			}
			else
			{
				int pixel_plot_x = sprite_x + pixel_x;

				/* check to prevent going outside of active display area */
				if (pixel_plot_x < 0 || pixel_plot_x > 255)
				{
					continue;
				}

				if (!(priority_selected[pixel_plot_x] & PRIORITY_BIT))
				{
					line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
					priority_selected[pixel_plot_x] = pen_selected;
				}
				else
				{
					if (priority_selected[pixel_plot_x] == PRIORITY_BIT)
					{
						line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
						priority_selected[pixel_plot_x] = pen_selected;
					}
				}
				if (m_collision_buffer[pixel_plot_x] != 1)
				{
					m_collision_buffer[pixel_plot_x] = 1;
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
	int sprite_col_x = m_screen->width();
	UINT16 sprite_pattern_base = ((m_reg[0x06] & 0x07) << 11);

	/* Draw sprite layer */

	/* Check if display is disabled */
	if (m_display_disabled)
		return;

	memset(m_collision_buffer, 0, SEGA315_5124_WIDTH);

	for (int sprite_buffer_index = m_sprite_count - 1; sprite_buffer_index >= 0; sprite_buffer_index--)
	{
		int sprite_index = m_selected_sprite[sprite_buffer_index];
		int sprite_y = space().read_byte( m_sprite_base + sprite_index ) + 1;
		int sprite_x = space().read_byte( m_sprite_base + sprite_index + 1 );
		UINT8 flags = space().read_byte( m_sprite_base + sprite_index + 3 );
		int pen_selected = m_palette_offset + ( flags & 0x0f );

		if (sprite_y > 240)
			sprite_y -= 256;

		if (flags & 0x80)
			sprite_x -= 32;

		int sprite_tile_selected = space().read_byte( m_sprite_base + sprite_index + 2 );
		int sprite_line = line - sprite_y;

		if (m_reg[0x01] & 0x01)
			sprite_line >>= 1;

		if (m_reg[0x01] & 0x02)
		{
			sprite_tile_selected &= 0xfc;

			if (sprite_line > 0x07)
			{
				sprite_tile_selected += 1;
				sprite_line -= 8;
			}
		}

		UINT8 pattern = space().read_byte( sprite_pattern_base + sprite_tile_selected * 8 + sprite_line );

		for (int pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			if (m_reg[0x01] & 0x01)
			{
				int pixel_plot_x = sprite_x + pixel_x * 2;

				if (pixel_plot_x < 0 || pixel_plot_x > 255)
				{
					continue;
				}

				if (pen_selected && (pattern & (1 << (7 - pixel_x))))
				{
					line_buffer[pixel_plot_x] = m_current_palette[pen_selected];

					if (m_collision_buffer[pixel_plot_x] != 1)
					{
						m_collision_buffer[pixel_plot_x] = 1;
					}
					else
					{
						sprite_col_occurred = true;
						sprite_col_x = MIN(sprite_col_x, pixel_plot_x);
					}

					line_buffer[pixel_plot_x+1] = m_current_palette[pen_selected];

					if (m_collision_buffer[pixel_plot_x + 1] != 1)
					{
						m_collision_buffer[pixel_plot_x + 1] = 1;
					}
					else
					{
						sprite_col_occurred = true;
						sprite_col_x = MIN(sprite_col_x, pixel_plot_x);
					}
				}
			}
			else
			{
				int pixel_plot_x = sprite_x + pixel_x;

				if (pixel_plot_x < 0 || pixel_plot_x > 255)
				{
					continue;
				}

				if (pen_selected && (pattern & (1 << (7 - pixel_x))))
				{
					line_buffer[pixel_plot_x] = m_current_palette[pen_selected];

					if (m_collision_buffer[pixel_plot_x] != 1)
					{
						m_collision_buffer[pixel_plot_x] = 1;
					}
					else
					{
						sprite_col_occurred = true;
						sprite_col_x = MIN(sprite_col_x, pixel_plot_x);
					}
				}
			}
		}

		if (m_reg[0x01] & 0x02)
		{
			sprite_tile_selected += 2;
			pattern = space().read_byte( sprite_pattern_base + sprite_tile_selected * 8 + sprite_line );
			sprite_x += (m_reg[0x01] & 0x01 ? 16 : 8);

			for (int pixel_x = 0; pixel_x < 8; pixel_x++)
			{
				if (m_reg[0x01] & 0x01)
				{
					int pixel_plot_x = sprite_x + pixel_x * 2;

					if (pixel_plot_x < 0 || pixel_plot_x > 255)
					{
						continue;
					}

					if (pen_selected && (pattern & (1 << (7 - pixel_x))))
					{
						line_buffer[pixel_plot_x] = m_current_palette[pen_selected];

						if (m_collision_buffer[pixel_plot_x] != 1)
						{
							m_collision_buffer[pixel_plot_x] = 1;
						}
						else
						{
							sprite_col_occurred = true;
							sprite_col_x = MIN(sprite_col_x, pixel_plot_x);
						}

						line_buffer[pixel_plot_x+1] = m_current_palette[pen_selected];

						if (m_collision_buffer[pixel_plot_x + 1] != 1)
						{
							m_collision_buffer[pixel_plot_x + 1] = 1;
						}
						else
						{
							sprite_col_occurred = true;
							sprite_col_x = MIN(sprite_col_x, pixel_plot_x);
						}
					}
				}
				else
				{
					int pixel_plot_x = sprite_x + pixel_x;

					if (pixel_plot_x < 0 || pixel_plot_x > 255)
					{
						continue;
					}

					if (pen_selected && (pattern & (1 << (7 - pixel_x))))
					{
						line_buffer[pixel_plot_x] = m_current_palette[pen_selected];

						if (m_collision_buffer[pixel_plot_x] != 1)
						{
							m_collision_buffer[pixel_plot_x] = 1;
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

	/* Draw background layer */
	name_table_base =  ((m_reg[0x02] & 0x0f) << 10) + ((line >> 3) * 32);
	color_base = ((m_reg[0x03] & 0x80) << 6);
	color_mask = ((m_reg[0x03] & 0x7f) << 3) | 0x07;
	pattern_base = ((m_reg[0x04] & 0x04) << 11);
	pattern_mask = ((m_reg[0x04] & 0x03) << 8) | 0xff;
	pattern_offset = (line & 0xc0) << 2;

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

	/* Draw background layer */
	name_base = ((m_reg[0x02] & 0x0f) << 10) + ((line >> 3) * 32);
	color_base = ((m_reg[0x03] << 6) & (VRAM_SIZE - 1));
	pattern_base = ((m_reg[0x04] << 11) & (VRAM_SIZE - 1));

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
	int x;
	int *blitline_buffer = m_line_buffer;
	int priority_selected[256];

	if ( line < m_frame_timing[ACTIVE_DISPLAY_V] )
	{
		switch( m_vdp_mode )
		{
		case 0:
			memset(priority_selected, 1, sizeof(priority_selected));
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
			memset(priority_selected, 1, sizeof(priority_selected));
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
			memset(priority_selected, 0, sizeof(priority_selected));
			if ( line >= 0 )
			{
				draw_scanline_mode4( blitline_buffer, priority_selected, line );
			}
			if ( line >= 0 || ( line >= -13 && m_y_pixels == 192 ) )
			{
				draw_sprites_mode4( blitline_buffer, priority_selected, line );
				if ( line >= 0 )
				{
					/* Fill column 0 with overscan color from m_reg[0x07] */
					if (m_reg[0x00] & 0x20)
					{
						blitline_buffer[0] = m_current_palette[BACKDROP_COLOR];
						priority_selected[0] = 1;
						blitline_buffer[1] = m_current_palette[BACKDROP_COLOR];
						priority_selected[1] = 1;
						blitline_buffer[2] = m_current_palette[BACKDROP_COLOR];
						priority_selected[2] = 1;
						blitline_buffer[3] = m_current_palette[BACKDROP_COLOR];
						priority_selected[3] = 1;
						blitline_buffer[4] = m_current_palette[BACKDROP_COLOR];
						priority_selected[4] = 1;
						blitline_buffer[5] = m_current_palette[BACKDROP_COLOR];
						priority_selected[5] = 1;
						blitline_buffer[6] = m_current_palette[BACKDROP_COLOR];
						priority_selected[6] = 1;
						blitline_buffer[7] = m_current_palette[BACKDROP_COLOR];
						priority_selected[7] = 1;
					}
				}
			}
			break;
		}
	}

	UINT32 *p_bitmap = &m_tmpbitmap.pix32(pixel_plot_y + line, pixel_offset_x);
	UINT8  *p_y1 = &m_y1_bitmap.pix8(pixel_plot_y + line, pixel_offset_x);

	/* Check if display is disabled or we're below/above active area */
	if (m_display_disabled || line < 0 || line >= m_frame_timing[ACTIVE_DISPLAY_V])
	{
		for (x = 0; x < 256; x++)
		{
			p_bitmap[x] = m_palette->pen(m_current_palette[BACKDROP_COLOR]);
			p_y1[x] = 1;
		}
	}
	else
	{
		for (x = 0; x < 256; x++)
		{
			p_bitmap[x] = m_palette->pen(blitline_buffer[x]);
			p_y1[x] = ( priority_selected[x] & 0x0f ) ? 0 : 1;
		}
	}
}


void sega315_5378_device::draw_scanline( int pixel_offset_x, int pixel_plot_y, int line )
{
	int x;
	int *blitline_buffer = m_line_buffer;
	int priority_selected[256];

	if ( line < m_frame_timing[ACTIVE_DISPLAY_V] )
	{
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
			memset(priority_selected, 0, sizeof(priority_selected));
			if ( line >= 0 )
			{
				draw_scanline_mode4( blitline_buffer, priority_selected, line );
			}
			if ( line >= 0 || ( line >= -13 && m_y_pixels == 192 ) )
			{
				draw_sprites_mode4( blitline_buffer, priority_selected, line );
				if ( line >= 0 )
				{
					/* Fill column 0 with overscan color from m_reg[0x07] */
					if (m_reg[0x00] & 0x20)
					{
						blitline_buffer[0] = m_current_palette[BACKDROP_COLOR];
						priority_selected[0] = 1;
						blitline_buffer[1] = m_current_palette[BACKDROP_COLOR];
						priority_selected[1] = 1;
						blitline_buffer[2] = m_current_palette[BACKDROP_COLOR];
						priority_selected[2] = 1;
						blitline_buffer[3] = m_current_palette[BACKDROP_COLOR];
						priority_selected[3] = 1;
						blitline_buffer[4] = m_current_palette[BACKDROP_COLOR];
						priority_selected[4] = 1;
						blitline_buffer[5] = m_current_palette[BACKDROP_COLOR];
						priority_selected[5] = 1;
						blitline_buffer[6] = m_current_palette[BACKDROP_COLOR];
						priority_selected[6] = 1;
						blitline_buffer[7] = m_current_palette[BACKDROP_COLOR];
						priority_selected[7] = 1;
					}
				}
			}
			break;
		}
	}

	/* Check if display is disabled or we're below/above active area */
	if (m_display_disabled || line < 0 || line >= m_frame_timing[ACTIVE_DISPLAY_V])
	{
		for (x = 0; x < 256; x++)
		{
			blitline_buffer[x] = m_current_palette[BACKDROP_COLOR];
		}
	}

	if (m_sega315_5124_compatibility_mode)
	{
		int *combineline_buffer = m_line_buffer + ((line & 0x03) + 1) * 256;
		int plot_x = 48;

		/* Do horizontal scaling */
		for (x = 8; x < 248;)
		{
			int combined;

			/* Take red and green from first pixel, and blue from second pixel */
			combined = (blitline_buffer[x] & 0x00ff) | (blitline_buffer[x + 1] & 0x0f00);
			combineline_buffer[plot_x] = combined;

			/* Take red from second pixel, and green and blue from third pixel */
			combined = (blitline_buffer[x + 1] & 0x000f) | (blitline_buffer[x + 2] & 0x0ff0);
			combineline_buffer[plot_x + 1] = combined;
			x += 3;
			plot_x += 2;
		}

		/* Do vertical scaling for a screen with 192 or 224 lines
		   Lines 0-2 and 221-223 have no effect on the output on the GG screen.
		   We will calculate the gamegear lines as follows:
		   GG_0 = 1/6 * SMS_3 + 1/3 * SMS_4 + 1/3 * SMS_5 + 1/6 * SMS_6
		   GG_1 = 1/6 * SMS_4 + 1/3 * SMS_5 + 1/3 * SMS_6 + 1/6 * SMS_7
		   GG_2 = 1/6 * SMS_6 + 1/3 * SMS_7 + 1/3 * SMS_8 + 1/6 * SMS_9
		   GG_3 = 1/6 * SMS_7 + 1/3 * SMS_8 + 1/3 * SMS_9 + 1/6 * SMS_10
		   GG_4 = 1/6 * SMS_9 + 1/3 * SMS_10 + 1/3 * SMS_11 + 1/6 * SMS_12
		   .....
		   GG_142 = 1/6 * SMS_216 + 1/3 * SMS_217 + 1/3 * SMS_218 + 1/6 * SMS_219
		   GG_143 = 1/6 * SMS_217 + 1/3 * SMS_218 + 1/3 * SMS_219 + 1/6 * SMS_220
		*/
		{
			int gg_line;
			int my_line = pixel_plot_y + line - (SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT);
			int *line1, *line2, *line3, *line4;

			/* First make sure there's enough data to draw anything */
			/* We need one more line of data if we're on line 8, 11, 14, 17, etc */
			if (my_line < 6 || my_line > 220 || ((my_line - 8) % 3 == 0))
			{
				return;
			}

			gg_line = ((my_line - 6) / 3) * 2;

			/* If we're on SMS line 7, 10, 13, etc we're on an odd GG line */
			if (my_line % 3)
			{
				gg_line++;
			}

			/* Calculate the line we will be drawing on */
			pixel_plot_y = SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_192_TBORDER_HEIGHT + 24 + gg_line;

			/* Setup our source lines */
			line1 = m_line_buffer + (((my_line - 3) & 0x03) + 1) * 256;
			line2 = m_line_buffer + (((my_line - 2) & 0x03) + 1) * 256;
			line3 = m_line_buffer + (((my_line - 1) & 0x03) + 1) * 256;
			line4 = m_line_buffer + (((my_line - 0) & 0x03) + 1) * 256;

			for (x = 0 + 48; x < 160 + 48; x++)
			{
				rgb_t   c1 = m_palette->pen(line1[x]);
				rgb_t   c2 = m_palette->pen(line2[x]);
				rgb_t   c3 = m_palette->pen(line3[x]);
				rgb_t   c4 = m_palette->pen(line4[x]);
				m_tmpbitmap.pix32(pixel_plot_y, pixel_offset_x + x) =
					rgb_t((c1.r() / 6 + c2.r() / 3 + c3.r() / 3 + c4.r() / 6 ),
						(c1.g() / 6 + c2.g() / 3 + c3.g() / 3 + c4.g() / 6 ),
						(c1.b() / 6 + c2.b() / 3 + c3.b() / 3 + c4.b() / 6 ) );
			}
			return;
		}
		blitline_buffer = combineline_buffer;
	}

	for (x = 0; x < 256; x++)
	{
		m_tmpbitmap.pix32(pixel_plot_y + line, pixel_offset_x + x) = m_palette->pen(blitline_buffer[x]);
	}
}


void sega315_5124_device::update_palette()
{
	int i;

	/* Exit if palette is has no changes */
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
		m_current_palette[i] = m_CRAM->u8(i) & 0x3f;
	}
}


void sega315_5378_device::update_palette()
{
	int i;

	/* Exit if palette is has no changes */
	if (m_cram_dirty == 0)
	{
		return;
	}
	m_cram_dirty = 0;

	if (m_sega315_5124_compatibility_mode)
	{
		for (i = 0; i < 32; i++)
		{
			m_current_palette[i] = ((m_CRAM->u8(i) & 0x30) << 6) | ((m_CRAM->u8(i) & 0x0c ) << 4) | ((m_CRAM->u8(i) & 0x03) << 2);
		}
	}
	else
	{
		for (i = 0; i < 32; i++)
		{
			m_current_palette[i] = ((m_CRAM->u8(i * 2 + 1) << 8) | m_CRAM->u8(i * 2)) & 0x0fff;
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
	m_check_hint_timer->adjust(attotime::never);
	m_check_vint_timer->adjust(attotime::never);
	m_draw_timer->adjust(attotime::never);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

void sega315_5124_device::vdp_postload()
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

void sega315_5124_device::device_start()
{
	/* Resolve callbacks */
	m_int_cb.resolve();
	m_pause_cb.resolve();

	/* Allocate video RAM */
	astring tempstring;
	m_CRAM = machine().memory().region_alloc(subtag(tempstring,"vdp_cram"), SEGA315_5378_CRAM_SIZE, 1, ENDIANNESS_LITTLE);
	m_line_buffer = auto_alloc_array(machine(), int, 256 * 5);

	m_frame_timing = (m_is_pal) ? pal_192 : ntsc_192;

	/* Make temp bitmap for rendering */
	m_screen->register_screen_bitmap(m_tmpbitmap);
	m_screen->register_screen_bitmap(m_y1_bitmap);

	m_display_timer = timer_alloc(TIMER_LINE);
	m_display_timer->adjust(m_screen->time_until_pos(0, DISPLAY_CB_HPOS), 0, m_screen->scan_period());
	m_draw_timer = timer_alloc(TIMER_DRAW);
	m_check_hint_timer = timer_alloc(TIMER_CHECK_HINT);
	m_check_vint_timer = timer_alloc(TIMER_CHECK_VINT);

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
	save_pointer(NAME(m_line_buffer), 256 * 5);
	save_item(NAME(m_collision_buffer));
	save_item(NAME(m_tmpbitmap));
	save_item(NAME(m_y1_bitmap));
	save_item(NAME(m_draw_time));
	save_item(NAME(m_sprite_base));
	save_item(NAME(m_selected_sprite));
	save_item(NAME(m_sprite_count));
	save_item(NAME(m_sprite_height));
	save_item(NAME(m_sprite_zoom));

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
	memset(m_CRAM->base(), 0, SEGA315_5378_CRAM_SIZE);
	memset(m_line_buffer, 0, 256 * 5 * sizeof(int));
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
