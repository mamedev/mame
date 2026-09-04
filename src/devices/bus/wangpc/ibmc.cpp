// license:BSD-3-Clause
// copyright-holders:Fausto Pracek
/**********************************************************************

    Wang PC-PM007 IBM PC Color Emulation Card

    LOADIBM scans the bus for option ID 0x16 before anything else and,
    when found, runs the colour emulation environment against this card.

    Table 15-3 of the Technical Reference names 0x16 "IBM PC Color
    Emulation", which is the PM007.  The character-only boards, PM101 and
    PM008, answer 0x11 - the same code as the plain monochrome monitor
    card - under the name "Wang/IBM Monochrome Emulation".

    Memory map, established by tracing LOADIBM 2.11 and the running
    emulation environment:

    - 0xb8000-0xbbfff   16K of CGA-style video memory
    - 0xbc000-0xbffff   character generator RAM: LOADIBM downloads the
                        font from IBMFONTC.FNT here before the panel is
                        shown, so the card needs no character ROM
    - 0xc0000-0xc3fff   16K of RAM

    The RAM window must not extend past 0xc4000: LOADIBM probes the word
    right there ('TC') and picks its loading strategy on the result.  On
    the real card the probe fails, which routes the loader to the INT
    21h/4B03 overlay path - the colour firmware only ever shipped as
    ROMBIOSC.EXE, there is no ROMBIOSC.BIN.

    The firmware programs the card's own MC6845 at +0x00/+0x02 with the
    standard 80-column CGA parameter set and writes text into the video
    memory, IBM PC style: character in the even byte, attribute in the
    odd byte.

**********************************************************************/

#include "emu.h"
#include "ibmc.h"

#include "screen.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define OPTION_ID               0x16

#define OPTION_ID_ENABLED       0x17

#define MC6845_TAG      "mc6845"
#define SCREEN_TAG      "screen"

#define RAM_SIZE        0x2000      // 8K words = 16K bytes: 0xc0000-0xc3fff

#define RAM_BASE        (0xc0000/2)

#define VIDEO_RAM_SIZE  0x2000      // 8K words = 16K bytes at 0xb8000

#define CHAR_RAM_SIZE   0x2000      // 8K words = 16K bytes at 0xbc000

// the standard CGA palette
static const rgb_t PALETTE_IBMC[16] =
{
	rgb_t(0x00, 0x00, 0x00), rgb_t(0x00, 0x00, 0xaa),
	rgb_t(0x00, 0xaa, 0x00), rgb_t(0x00, 0xaa, 0xaa),
	rgb_t(0xaa, 0x00, 0x00), rgb_t(0xaa, 0x00, 0xaa),
	rgb_t(0xaa, 0x55, 0x00), rgb_t(0xaa, 0xaa, 0xaa),
	rgb_t(0x55, 0x55, 0x55), rgb_t(0x55, 0x55, 0xff),
	rgb_t(0x55, 0xff, 0x55), rgb_t(0x55, 0xff, 0xff),
	rgb_t(0xff, 0x55, 0x55), rgb_t(0xff, 0x55, 0xff),
	rgb_t(0xff, 0xff, 0x55), rgb_t(0xff, 0xff, 0xff)
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WANGPC_IBMC, wangpc_ibmc_device, "wangpc_ibmc", "Wang PC-PM007 IBM PC Color Emulation Card")


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

//-------------------------------------------------
//  screen_update - the display is drawn from the
//  video memory and the mode register alone.  The
//  6845 is kept for the sync outputs the status
//  register reports, but the screen geometry stays
//  fixed: the emulated software reprograms the
//  timings freely (Flight Simulator switches to
//  one-line character rows) and following them
//  would reshape the screen under MAME's feet.
//-------------------------------------------------

uint32_t wangpc_ibmc_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	// bit 3 of the mode control register is video enable
	if (!BIT(m_control, 3))
		return 0;

	if (BIT(m_control, 1))
	{
		// CGA graphics: two interleaved banks of 0x1000 words, even scan
		// lines in the first, odd in the second
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			if (y >= 200) break;
			offs_t const base = ((y & 1) << 12) | ((y >> 1) * 40);
			uint32_t *const dst = &bitmap.pix(y);

			for (int column = 0; column < 40; column++)
			{
				uint16_t const data = m_video_ram[(base + column) & (VIDEO_RAM_SIZE - 1)];

				if (BIT(m_control, 4))
				{
					// 640x200, one bit per pixel
					rgb_t const fg = PALETTE_IBMC[m_color & 0x0f];

					for (int bit = 0; bit < 16; bit++)
						dst[(column * 16) + bit] = BIT(data, 7 - (bit & 7) + (bit & 8)) ? fg : rgb_t::black();
				}
				else
				{
					// 320x200, two bits per pixel, drawn double width
					for (int pel = 0; pel < 8; pel++)
					{
						int const shift = 6 - ((pel & 3) << 1) + ((pel & 4) << 1);
						int const pix = (data >> shift) & 3;

						rgb_t color;
						if (pix == 0)
							color = PALETTE_IBMC[m_color & 0x0f];
						else if (BIT(m_color, 5))
							color = PALETTE_IBMC[(pix << 1) | 1 | ((m_color & 0x10) >> 1)];
						else
							color = PALETTE_IBMC[(pix << 1) | ((m_color & 0x10) >> 1)];

						dst[(column * 16) + (pel * 2)] = color;
						dst[(column * 16) + (pel * 2) + 1] = color;
					}
				}
			}
		}
	}
	else
	{
		// 80x25 text with the 8x8 soft font.  Start address and cursor
		// come from the shadowed 6845 registers.
		offs_t const start = ((m_crtc_regs[12] << 8) | m_crtc_regs[13]) & (VIDEO_RAM_SIZE - 1);
		offs_t const cursor = ((m_crtc_regs[14] << 8) | m_crtc_regs[15]) & (VIDEO_RAM_SIZE - 1);
		bool const cursor_on = (screen.frame_number() & 0x10) != 0;

		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			if (y >= 200) break;
			int const row = y >> 3;
			int const ra = y & 0x07;
			uint32_t *const dst = &bitmap.pix(y);

			for (int column = 0; column < 80; column++)
			{
				offs_t const ma = (start + (row * 80) + column) & (VIDEO_RAM_SIZE - 1);
				uint16_t const code = m_video_ram[ma];
				uint8_t const attr = code >> 8;

				// the font download writes one glyph byte per word, low
				// byte: a character is 8 consecutive words, one line each
				uint8_t glyph = m_char_ram[(((code & 0xff) << 3) | ra) & (CHAR_RAM_SIZE - 1)] & 0xff;

				if (ma == cursor && cursor_on && ra >= 6)
					glyph = 0xff;

				rgb_t const fg = PALETTE_IBMC[attr & 0x0f];
				rgb_t const bg = PALETTE_IBMC[(attr >> 4) & 0x07];

				for (int bit = 0; bit < 8; bit++)
				{
					dst[(column * 8) + bit] = BIT(glyph, 7) ? fg : bg;
					glyph <<= 1;
				}
			}
		}
	}

	return 0;
}


//-------------------------------------------------
//  machine_config( wangpc_ibmc )
//-------------------------------------------------

void wangpc_ibmc_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(wangpc_ibmc_device::screen_update));
	screen.set_size(80*8, 25*8);
	screen.set_visarea(0, 80*8-1, 0, 25*8-1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_refresh_hz(60);

	// the 6845 supplies the sync state the status register reports; the
	// display itself is drawn by screen_update, so no screen is attached
	// and the timings the software programs cannot reshape the visible area
	MC6845_1(config, m_crtc, XTAL(14'318'181)/8);
	m_crtc->set_screen(nullptr);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline bool wangpc_ibmc_device::ram_enabled() const
{
	// bit 2 of the option register switches the memory windows in.  The
	// running firmware turns the card on by writing 0x07 here without
	// touching the enable register again, so the 0xc5 enable value the
	// loader writes during detection is not part of the gate.
	return BIT(m_option, 2);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

wangpc_ibmc_device::wangpc_ibmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_IBMC, tag, owner, clock),
	device_wangpcbus_card_interface(mconfig, *this),
	m_crtc(*this, MC6845_TAG),
	m_screen(*this, SCREEN_TAG),
	m_ram(*this, "ram", RAM_SIZE * 2, ENDIANNESS_LITTLE),
	m_video_ram(*this, "video_ram", VIDEO_RAM_SIZE * 2, ENDIANNESS_LITTLE),
	m_char_ram(*this, "char_ram", CHAR_RAM_SIZE * 2, ENDIANNESS_LITTLE),
	m_option(0),
	m_enable(0),
	m_mode(0),
	m_live(0),
	m_control(0),
	m_color(0),
	m_reg30(0),
	m_crtc_idx(0)
{
	std::fill(std::begin(m_crtc_regs), std::end(m_crtc_regs), 0);
}

void wangpc_ibmc_device::device_start()
{
	save_item(NAME(m_option));
	save_item(NAME(m_enable));
	save_item(NAME(m_mode));
	save_item(NAME(m_live));
	save_item(NAME(m_control));
	save_item(NAME(m_color));
	save_item(NAME(m_reg30));
	save_item(NAME(m_crtc_idx));
	save_item(NAME(m_crtc_regs));
}

void wangpc_ibmc_device::device_reset()
{
	m_option = 0;
	m_enable = 0;
	m_mode = 0;
	m_live = 0;
	m_control = 0;
	m_color = 0;
	m_reg30 = 0;
}


//-------------------------------------------------
//  wangpcbus_mrdc_r - memory read
//-------------------------------------------------

uint16_t wangpc_ibmc_device::wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (ram_enabled())
	{
		if (offset >= RAM_BASE && offset < RAM_BASE + RAM_SIZE)
			data = m_ram[offset - RAM_BASE];
		else if (offset >= 0xb8000/2 && offset < 0xbc000/2)
			data = m_video_ram[offset & (VIDEO_RAM_SIZE - 1)];
		else if (offset >= 0xbc000/2 && offset < 0xc0000/2)
			data = m_char_ram[offset & (CHAR_RAM_SIZE - 1)];
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_ibmc_device::wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
	if (offset >= 0xbc000/2 && offset < 0xc0000/2)
	{
		// LOADIBM downloads the font while the option register is still
		// zero, so the character generator takes writes unconditionally
		COMBINE_DATA(&m_char_ram[offset & (CHAR_RAM_SIZE - 1)]);
	}
	else if (ram_enabled())
	{
		if (offset >= RAM_BASE && offset < RAM_BASE + RAM_SIZE)
		{
			COMBINE_DATA(&m_ram[offset - RAM_BASE]);
		}
		else if (offset >= 0xb8000/2 && offset < 0xbc000/2)
		{
			COMBINE_DATA(&m_video_ram[offset & (VIDEO_RAM_SIZE - 1)]);
		}
	}
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

uint16_t wangpc_ibmc_device::wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0x02/2:
			data = 0xff00 | m_crtc->register_r();
			break;

		case 0x20/2:
			data = 0xff00 | m_option;
			break;

		case 0xc0/2:
			data = 0xff00 | m_live;
			break;

		case 0xfe/2:
			// the loader writes 0x10 to +0x2a and then expects the ID to
			// read back as 0x17 instead of 0x16 before it will go on
			data = 0xff00 | (BIT(m_mode, 4) ? OPTION_ID_ENABLED : OPTION_ID);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_ibmc_device::wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
	if (sad(offset) && ACCESSING_BITS_0_7)
	{
		switch (offset & 0x7f)
		{
		case 0x00/2:
			m_crtc_idx = data & 0x1f;
			m_crtc->address_w(data & 0xff);
			break;

		case 0x02/2:
			m_crtc_regs[m_crtc_idx] = data & 0xff;
			m_crtc->register_w(data & 0xff);
			break;

		case 0x10/2:
			// mode control: written with 0x01 while the display is set
			// up, 0xe9 once it is running, 0x00 to shut the card down
			m_control = data & 0xff;
			break;

		case 0x20/2:
			m_option = data & 0xff;
			break;

		case 0x2a/2:
			m_mode = data & 0xff;
			break;

		case 0x30/2:
			// written once with 0xff as the firmware brings the display up
			m_reg30 = data & 0xff;
			break;

		case 0xe0/2:
			// colour select, in the same shape as the IBM PC register at
			// 0x3d9: the firmware translates the emulated software's
			// writes into this one (0x30 for the standard palette on a
			// black background, 0x3f with a white background)
			m_color = data & 0xff;
			break;

		case 0xc0/2:
			// the loader counts 3,2,1,0 into this register while it sets
			// the card up and writes 0x04 as its last act before handing
			// control to the emulation environment
			m_live = data & 0xff;
			break;

		case 0xfa/2:
			m_enable = data & 0xff;
			break;
		}
	}
}
