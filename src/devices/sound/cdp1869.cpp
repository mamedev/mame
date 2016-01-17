// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1869/1870/1876 Video Interface System (VIS) emulation

**********************************************************************/

/*

    TODO:

    - white noise
    - scanline based update
    - CMSEL output

*/

#include "emu.h"
#include "cdp1869.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define CDP1869_WEIGHT_RED      30 // % of max luminance
#define CDP1869_WEIGHT_GREEN    59
#define CDP1869_WEIGHT_BLUE     11

#define CDP1869_COLUMNS_HALF    20
#define CDP1869_COLUMNS_FULL    40
#define CDP1869_ROWS_HALF       12
#define CDP1869_ROWS_FULL_PAL   25
#define CDP1869_ROWS_FULL_NTSC  24

enum
{
	CDB0 = 0,
	CDB1,
	CDB2,
	CDB3,
	CDB4,
	CDB5,
	CCB0,
	CCB1
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type CDP1869 = &device_creator<cdp1869_device>;

// I/O map
DEVICE_ADDRESS_MAP_START( io_map, 8, cdp1869_device )
	AM_RANGE(0x03, 0x03) AM_WRITE(out3_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(out4_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(out5_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(out6_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(out7_w)
ADDRESS_MAP_END

// character RAM map
DEVICE_ADDRESS_MAP_START( char_map, 8, cdp1869_device )
	AM_RANGE(0x000, 0x3ff) AM_READWRITE(char_ram_r, char_ram_w)
ADDRESS_MAP_END

// page RAM map
DEVICE_ADDRESS_MAP_START( page_map, 8, cdp1869_device )
	AM_RANGE(0x000, 0x7ff) AM_READWRITE(page_ram_r, page_ram_w)
ADDRESS_MAP_END

// default address map
static ADDRESS_MAP_START( cdp1869, AS_0, 8, cdp1869_device )
	AM_RANGE(0x000, 0x7ff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  is_ntsc - is device in NTSC mode
//-------------------------------------------------

inline bool cdp1869_device::is_ntsc()
{
	return m_read_pal_ntsc() ? false : true;
}


//-------------------------------------------------
//  read_page_ram_byte - read a page RAM byte at
//  the given address
//-------------------------------------------------

inline UINT8 cdp1869_device::read_page_ram_byte(offs_t pma)
{
	return space().read_byte(pma);
}


//-------------------------------------------------
//  write_page_ram_byte - write a page RAM byte at
//  the given address
//-------------------------------------------------

inline void cdp1869_device::write_page_ram_byte(offs_t pma, UINT8 data)
{
	space().write_byte(pma, data);
}


//-------------------------------------------------
//  read_char_ram_byte - read a char RAM byte at
//  the given address
//-------------------------------------------------

inline UINT8 cdp1869_device::read_char_ram_byte(offs_t pma, offs_t cma, UINT8 pmd)
{
	UINT8 data = 0;

	if (!m_in_char_ram_func.isnull())
	{
		data = m_in_char_ram_func(pma, cma, pmd);
	}

	return data;
}


//-------------------------------------------------
//  write_char_ram_byte - write a char RAM byte at
//  the given address
//-------------------------------------------------

inline void cdp1869_device::write_char_ram_byte(offs_t pma, offs_t cma, UINT8 pmd, UINT8 data)
{
	if (!m_out_char_ram_func.isnull())
	{
		m_out_char_ram_func(pma, cma, pmd, data);
	}
}


//-------------------------------------------------
//  read_pcb - read page control bit
//-------------------------------------------------

inline int cdp1869_device::read_pcb(offs_t pma, offs_t cma, UINT8 pmd)
{
	int pcb = 0;

	if (!m_in_pcb_func.isnull())
	{
		pcb = m_in_pcb_func(pma, cma, pmd);
	}

	return pcb;
}


//-------------------------------------------------
//  update_prd_changed_timer -
//-------------------------------------------------

inline void cdp1869_device::update_prd_changed_timer()
{
	int start = CDP1869_SCANLINE_PREDISPLAY_START_PAL;
	int end = CDP1869_SCANLINE_PREDISPLAY_END_PAL;
	int next_state;
	int scanline = m_screen->vpos();
	int next_scanline;

	if (is_ntsc())
	{
		start = CDP1869_SCANLINE_PREDISPLAY_START_NTSC;
		end = CDP1869_SCANLINE_PREDISPLAY_END_NTSC;
	}

	if (scanline < start)
	{
		next_scanline = start;
		next_state = ASSERT_LINE;
	}
	else if (scanline < end)
	{
		next_scanline = end;
		next_state = CLEAR_LINE;
	}
	else
	{
		next_scanline = start;
		next_state = ASSERT_LINE;
	}

	if (m_dispoff)
	{
		next_state = CLEAR_LINE;
	}

	attotime duration = m_screen->time_until_pos(next_scanline);
	m_prd_timer->adjust(duration, next_state);
}


//-------------------------------------------------
//  get_rgb - get RGB value
//-------------------------------------------------

inline rgb_t cdp1869_device::get_rgb(int i, int c, int l)
{
	int luma = 0, r, g, b;

	luma += (l & 4) ? CDP1869_WEIGHT_RED : 0;
	luma += (l & 1) ? CDP1869_WEIGHT_GREEN : 0;
	luma += (l & 2) ? CDP1869_WEIGHT_BLUE : 0;

	luma = (luma * 0xff) / 100;

	r = (c & 4) ? luma : 0;
	g = (c & 1) ? luma : 0;
	b = (c & 2) ? luma : 0;

	return rgb_t(r, g, b);
}


//-------------------------------------------------
//  get_lines - get number of character lines
//-------------------------------------------------

inline int cdp1869_device::get_lines()
{
	if (m_line16 && !m_dblpage)
	{
		return 16;
	}
	else if (!m_line9)
	{
		return 9;
	}
	else
	{
		return 8;
	}
}


//-------------------------------------------------
//  get_pmemsize - get page memory size
//-------------------------------------------------

inline UINT16 cdp1869_device::get_pmemsize(int cols, int rows)
{
	int pmemsize = cols * rows;

	if (m_dblpage) pmemsize *= 2;
	if (m_line16) pmemsize *= 2;

	return pmemsize;
}


//-------------------------------------------------
//  get_pma - get page memory address
//-------------------------------------------------

inline UINT16 cdp1869_device::get_pma()
{
	if (m_dblpage)
	{
		return m_pma;
	}
	else
	{
		return m_pma & 0x3ff;
	}
}


//-------------------------------------------------
//  get_pen - get pen for color bits
//-------------------------------------------------

inline int cdp1869_device::get_pen(int ccb0, int ccb1, int pcb)
{
	int r = 0, g = 0, b = 0;

	switch (m_col)
	{
	case 0:
		r = ccb0;
		b = ccb1;
		g = pcb;
		break;

	case 1:
		r = ccb0;
		b = pcb;
		g = ccb1;
		break;

	case 2:
	case 3:
		r = pcb;
		b = ccb0;
		g = ccb1;
		break;
	}

	int color = (r << 2) + (b << 1) + g;

	if (m_cfc)
	{
		return color + ((m_bkg + 1) * 8);
	}
	else
	{
		return color;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cdp1869_device - constructor
//-------------------------------------------------

cdp1869_device::cdp1869_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CDP1869, "RCA CDP1869", tag, owner, clock, "cdp1869", __FILE__),
	device_sound_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_read_pal_ntsc(*this),
	m_write_prd(*this),
	m_color_clock(0),
	m_stream(nullptr),
	m_palette(*this, "palette"),
	m_space_config("pageram", ENDIANNESS_LITTLE, 8, 11, 0, nullptr, *ADDRESS_MAP_NAME(cdp1869))
{
}

static MACHINE_CONFIG_FRAGMENT( cdp1869 )
	MCFG_PALETTE_ADD("palette", 8+64)
	MCFG_PALETTE_INIT_OWNER(cdp1869_device, cdp1869)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor cdp1869_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cdp1869 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cdp1869_device::device_start()
{
	// resolve callbacks
	m_read_pal_ntsc.resolve_safe(0);
	m_write_prd.resolve_safe();
	m_in_pcb_func.bind_relative_to(*owner());
	m_in_char_ram_func.bind_relative_to(*owner());
	m_out_char_ram_func.bind_relative_to(*owner());

	// allocate timers
	m_prd_timer = timer_alloc();
	m_dispoff = 0;
	update_prd_changed_timer();

	// initialize palette
	m_bkg = 0;

	// create sound stream
	m_stream = machine().sound().stream_alloc(*this, 0, 1, machine().sample_rate());

	// initialize other
	m_tonediv = 0;
	m_tonefreq = 0;
	m_toneamp = 0;
	m_dblpage = 0;
	m_line16 = 0;
	m_line9 = 0;
	m_fresvert = 0;
	m_freshorz = 0;
	m_hma = 0;
	m_col = 0;
	m_incr = 0;
	m_signal = 0;
	m_cfc = 0;
	m_toneoff = 0;
	m_cmem = 0;

	// register for state saving
	save_item(NAME(m_prd));
	save_item(NAME(m_dispoff));
	save_item(NAME(m_fresvert));
	save_item(NAME(m_freshorz));
	save_item(NAME(m_cmem));
	save_item(NAME(m_dblpage));
	save_item(NAME(m_line16));
	save_item(NAME(m_line9));
	save_item(NAME(m_cfc));
	save_item(NAME(m_col));
	save_item(NAME(m_bkg));
	save_item(NAME(m_pma));
	save_item(NAME(m_hma));
	save_item(NAME(m_signal));
	save_item(NAME(m_incr));
	save_item(NAME(m_toneoff));
	save_item(NAME(m_wnoff));
	save_item(NAME(m_tonediv));
	save_item(NAME(m_tonefreq));
	save_item(NAME(m_toneamp));
	save_item(NAME(m_wnfreq));
	save_item(NAME(m_wnamp));
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void cdp1869_device::device_post_load()
{
	update_prd_changed_timer();
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void cdp1869_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_write_prd(param);
	m_prd = param;

	update_prd_changed_timer();
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *cdp1869_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == 0) ? &m_space_config : nullptr;
}


//-------------------------------------------------
//  initialize_palette - initialize palette
//-------------------------------------------------

PALETTE_INIT_MEMBER(cdp1869_device, cdp1869)
{
	// color-on-color display (CFC=0)
	int i;

	for (i = 0; i < 8; i++)
	{
		palette.set_pen_color(i, get_rgb(i, i, 15));
	}

	// tone-on-tone display (CFC=1)
	for (int c = 0; c < 8; c++)
	{
		for (int l = 0; l < 8; l++)
		{
			palette.set_pen_color(i, get_rgb(i, c, l));
			i++;
		}
	}
}


//-------------------------------------------------
//  sound_stream_update - handle update requests for
//  our sound stream
//-------------------------------------------------

void cdp1869_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// reset the output stream
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));

	INT16 signal = m_signal;
	stream_sample_t *buffer = outputs[0];

	if (!m_toneoff && m_toneamp)
	{
		double frequency = (clock() / 2) / (512 >> m_tonefreq) / (m_tonediv + 1);
//      double amplitude = m_toneamp * ((0.78*5) / 15);

		int rate = machine().sample_rate() / 2;

		/* get progress through wave */
		int incr = m_incr;

		if (signal < 0)
		{
			signal = -(m_toneamp * (0x07fff / 15));
		}
		else
		{
			signal = m_toneamp * (0x07fff / 15);
		}

		while( samples-- > 0 )
		{
			*buffer++ = signal;
			incr -= frequency;
			while( incr < 0 )
			{
				incr += rate;
				signal = -signal;
			}
		}

		/* store progress through wave */
		m_incr = incr;
		m_signal = signal;
	}
/*
    if (!m_wnoff)
    {
        double amplitude = m_wnamp * ((0.78*5) / 15);

        for (int wndiv = 0; wndiv < 128; wndiv++)
        {
            double frequency = (clock() / 2) / (4096 >> m_wnfreq) / (wndiv + 1):

            sum_square_wave(buffer, frequency, amplitude);
        }
    }
*/

}


//-------------------------------------------------
//  draw_line - draw character line
//-------------------------------------------------

void cdp1869_device::draw_line(bitmap_rgb32 &bitmap, const rectangle &rect, int x, int y, UINT8 data, int color)
{
	int i;
	pen_t fg = m_palette->pen(color);

	data <<= 2;

	for (i = 0; i < CDP1869_CHAR_WIDTH; i++)
	{
		if (data & 0x80)
		{
			bitmap.pix32(y, x) = fg;

			if (!m_fresvert)
			{
				bitmap.pix32(y + 1, x) = fg;
			}

			if (!m_freshorz)
			{
				bitmap.pix32(y, x + 1) = fg;

				if (!m_fresvert)
				{
					bitmap.pix32(y + 1, x + 1) = fg;
				}
			}
		}

		if (!m_freshorz)
		{
			x++;
		}

		x++;

		data <<= 1;
	}
}


//-------------------------------------------------
//  draw_char - draw character
//-------------------------------------------------

void cdp1869_device::draw_char(bitmap_rgb32 &bitmap, const rectangle &rect, int x, int y, UINT16 pma)
{
	UINT8 pmd = read_page_ram_byte(pma);

	for (UINT8 cma = 0; cma < get_lines(); cma++)
	{
		UINT8 data = read_char_ram_byte(pma, cma, pmd);

		int ccb0 = BIT(data, CCB0);
		int ccb1 = BIT(data, CCB1);
		int pcb = read_pcb(pma, cma, pmd);

		int color = get_pen(ccb0, ccb1, pcb);

		draw_line(bitmap, rect, rect.min_x + x, rect.min_y + y, data, color);

		y++;

		if (!m_fresvert)
		{
			y++;
		}
	}
}


//-------------------------------------------------
//  out3_w - register 3 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out3_w )
{
	/*
	  bit   description

	    0   bkg green
	    1   bkg blue
	    2   bkg red
	    3   cfc
	    4   disp off
	    5   colb0
	    6   colb1
	    7   fres horz
	*/

	m_bkg = data & 0x07;
	m_cfc = BIT(data, 3);
	m_dispoff = BIT(data, 4);
	m_col = (data & 0x60) >> 5;
	m_freshorz = BIT(data, 7);
}


//-------------------------------------------------
//  out4_w - register 4 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out4_w )
{
	/*
	  bit   description

	    0   tone amp 2^0
	    1   tone amp 2^1
	    2   tone amp 2^2
	    3   tone amp 2^3
	    4   tone freq sel0
	    5   tone freq sel1
	    6   tone freq sel2
	    7   tone off
	    8   tone / 2^0
	    9   tone / 2^1
	   10   tone / 2^2
	   11   tone / 2^3
	   12   tone / 2^4
	   13   tone / 2^5
	   14   tone / 2^6
	   15   always 0
	*/

	m_toneamp = offset & 0x0f;
	m_tonefreq = (offset & 0x70) >> 4;
	m_toneoff = BIT(offset, 7);
	m_tonediv = (offset & 0x7f00) >> 8;

	m_stream->update();
}


//-------------------------------------------------
//  out5_w - register 5 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out5_w )
{
	/*
	  bit   description

	    0   cmem access mode
	    1   x
	    2   x
	    3   9-line
	    4   x
	    5   16 line hi-res
	    6   double page
	    7   fres vert
	    8   wn amp 2^0
	    9   wn amp 2^1
	   10   wn amp 2^2
	   11   wn amp 2^3
	   12   wn freq sel0
	   13   wn freq sel1
	   14   wn freq sel2
	   15   wn off
	*/

	m_cmem = BIT(offset, 0);
	m_line9 = BIT(offset, 3);
	m_line16 = BIT(offset, 5);
	m_dblpage = BIT(offset, 6);
	m_fresvert = BIT(offset, 7);
	m_wnamp = (offset & 0x0f00) >> 8;
	m_wnfreq = (offset & 0x7000) >> 12;
	m_wnoff = BIT(offset, 15);

	m_stream->update();

	if (m_cmem)
	{
		m_pma = offset;
	}
	else
	{
		m_pma = 0;
	}
}


//-------------------------------------------------
//  out6_w - register 6 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out6_w )
{
	/*
	  bit   description

	    0   pma0 reg
	    1   pma1 reg
	    2   pma2 reg
	    3   pma3 reg
	    4   pma4 reg
	    5   pma5 reg
	    6   pma6 reg
	    7   pma7 reg
	    8   pma8 reg
	    9   pma9 reg
	   10   pma10 reg
	   11   x
	   12   x
	   13   x
	   14   x
	   15   x
	*/

	m_pma = offset & 0x7ff;
}


//-------------------------------------------------
//  out7_w - register 7 write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::out7_w )
{
	/*
	  bit   description

	    0   x
	    1   x
	    2   hma2 reg
	    3   hma3 reg
	    4   hma4 reg
	    5   hma5 reg
	    6   hma6 reg
	    7   hma7 reg
	    8   hma8 reg
	    9   hma9 reg
	   10   hma10 reg
	   11   x
	   12   x
	   13   x
	   14   x
	   15   x
	*/

	m_hma = offset & 0x7fc;
}


//-------------------------------------------------
//  char_ram_r - character RAM read
//-------------------------------------------------

READ8_MEMBER( cdp1869_device::char_ram_r )
{
	UINT8 cma = offset & 0x0f;
	UINT16 pma;

	if (m_cmem)
	{
		pma = get_pma();
	}
	else
	{
		pma = offset;
	}

	if (m_dblpage)
	{
		cma &= 0x07;
	}

	UINT8 pmd = read_page_ram_byte(pma);

	return read_char_ram_byte(pma, cma, pmd);
}


//-------------------------------------------------
//  char_ram_w - character RAM write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::char_ram_w )
{
	UINT8 cma = offset & 0x0f;
	UINT16 pma;

	if (m_cmem)
	{
		pma = get_pma();
	}
	else
	{
		pma = offset;
	}

	if (m_dblpage)
	{
		cma &= 0x07;
	}

	UINT8 pmd = read_page_ram_byte(pma);

	write_char_ram_byte(pma, cma, pmd, data);
}


//-------------------------------------------------
//  page_ram_r - page RAM read
//-------------------------------------------------

READ8_MEMBER( cdp1869_device::page_ram_r )
{
	UINT16 pma;

	if (m_cmem)
	{
		pma = get_pma();
	}
	else
	{
		pma = offset;
	}

	return read_page_ram_byte(pma);
}


//-------------------------------------------------
//  page_ram_w - page RAM write
//-------------------------------------------------

WRITE8_MEMBER( cdp1869_device::page_ram_w )
{
	UINT16 pma;

	if (m_cmem)
	{
		pma = get_pma();
	}
	else
	{
		pma = offset;
	}

	write_page_ram_byte(pma, data);
}


//-------------------------------------------------
//  page_ram_w - predisplay
//-------------------------------------------------

READ_LINE_MEMBER( cdp1869_device::predisplay_r )
{
	return m_prd;
}


//-------------------------------------------------
//  pal_ntsc_r - PAL/NTSC
//-------------------------------------------------

READ_LINE_MEMBER( cdp1869_device::pal_ntsc_r )
{
	return m_read_pal_ntsc();
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

UINT32 cdp1869_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	rectangle screen_rect, outer;

	if (is_ntsc())
	{
		outer.min_x = CDP1869_HBLANK_END;
		outer.max_x = CDP1869_HBLANK_START - 1;
		outer.min_y = CDP1869_SCANLINE_VBLANK_END_NTSC;
		outer.max_y = CDP1869_SCANLINE_VBLANK_START_NTSC - 1;
		screen_rect.min_x = CDP1869_SCREEN_START_NTSC;
		screen_rect.max_x = CDP1869_SCREEN_END - 1;
		screen_rect.min_y = CDP1869_SCANLINE_DISPLAY_START_NTSC;
		screen_rect.max_y = CDP1869_SCANLINE_DISPLAY_END_NTSC - 1;
	}
	else
	{
		outer.min_x = CDP1869_HBLANK_END;
		outer.max_x = CDP1869_HBLANK_START - 1;
		outer.min_y = CDP1869_SCANLINE_VBLANK_END_PAL;
		outer.max_y = CDP1869_SCANLINE_VBLANK_START_PAL - 1;
		screen_rect.min_x = CDP1869_SCREEN_START_PAL;
		screen_rect.max_x = CDP1869_SCREEN_END - 1;
		screen_rect.min_y = CDP1869_SCANLINE_DISPLAY_START_PAL;
		screen_rect.max_y = CDP1869_SCANLINE_DISPLAY_END_PAL - 1;
	}

	outer &= cliprect;
	bitmap.fill(m_palette->pen(m_bkg), outer);

	if (!m_dispoff)
	{
		int width = CDP1869_CHAR_WIDTH;
		int height = get_lines();

		if (!m_freshorz)
		{
			width *= 2;
		}

		if (!m_fresvert)
		{
			height *= 2;
		}

		int cols = m_freshorz ? CDP1869_COLUMNS_FULL : CDP1869_COLUMNS_HALF;
		int rows = screen_rect.height() / height;

		UINT16 pmemsize = get_pmemsize(cols, rows);
		UINT16 addr = m_hma;

		for (int sy = 0; sy < rows; sy++)
		{
			for (int sx = 0; sx < cols; sx++)
			{
				int x = sx * width;
				int y = sy * height;

				draw_char(bitmap, screen_rect, x, y, addr);

				addr++;

				if (addr == pmemsize) addr = 0;
			}
		}
	}
	return 0;
}
