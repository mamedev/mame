// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC PM-001B Medium-Resolution Video Controller emulation

**********************************************************************/

/*

    TODO:

    - character clock
    - blink

    The IBM PC Emulation Option is a daughter board carried by this card and
    is driven through the card's own I/O block.  What is modelled here was
    reverse engineered from LOADIBM.EXE of the Wang "IBM PC Emulation Option"
    software, version 2.11 (1986), which probes it like this:

    - write 0x00 to +0x10/+0x12 (video option register), then read +0x50 and
      require the low nibble to be clear to accept the card
    - read +0x20: bit 2 set means the monochrome emulation option is fitted,
      bit 3 set means the board provides the display itself (when clear the
      software goes looking for a TIG card to drive the high resolution
      monitor)
    - write 0x04 to +0x20 and 0x01 to +0xfa to switch the option memory in,
      then size it by probing at 0xf4000: boards that answer there carry 32K
      at 0xf4000-0xfbfff, the others 16K at 0xf8000-0xfbfff

    The emulation board of the separate "emulator card" (option ID 0x16) is
    driven the same way but answers at 0xc0000 and takes 0xc5 in +0xfa.

    Note that the host memory map has to extend past 0xf3fff for this window
    to be reachable at all.

*/

#include "emu.h"
#include "mvc.h"

#include "screen.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0

#define OPTION_ID           0x15

#define MC6845_TAG          "mc6845"
#define SCREEN_TAG          "screen"

#define VIDEO_RAM_SIZE      0x800
#define CHAR_RAM_SIZE       0x1000
#define BITMAP_RAM_SIZE     0x4000
#define IBM_RAM_SIZE        0x4000

#define OPTION_VRAM         BIT(m_option, 0)
#define OPTION_VSYNC        BIT(m_option, 3)

#define IBM_OPTION_INSTALLED    (m_sw->read() & 0x03)
#define IBM_OPTION_32K          BIT(m_sw->read(), 1)
#define IBM_OPTION_STANDALONE   BIT(m_sw->read(), 2)

// bit 2 of the option register switches the memory window in, bit 0 is set
// only once the emulated IBM software has taken the display over: the Wang
// software draws its own panels with the window already enabled
#define IBM_DISPLAY_MODE        (ibm_ram_enabled() && BIT(m_ibm_option, 0))

#define ATTR_BLINK          BIT(attr, 0)
#define ATTR_REVERSE        BIT(attr, 1)
#define ATTR_BLANK          BIT(attr, 2)
#define ATTR_BOLD           BIT(attr, 3)
#define ATTR_OVERSCORE      BIT(attr, 4)
#define ATTR_UNDERSCORE     BIT(attr, 5)
#define ATTR_SUBSCRIPT      BIT(attr, 6)
#define ATTR_SUPERSCRIPT    BIT(attr, 7)

static const rgb_t PALETTE_MVC[] =
{
	rgb_t::black(),
	rgb_t(0x00, 0x80, 0x00),
	rgb_t(0x00, 0xff, 0x00)
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WANGPC_MVC, wangpc_mvc_device, "wangpc_mvc", "Wang PC Medium Resolution Video Card")


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

MC6845_UPDATE_ROW( wangpc_mvc_device::crtc_update_row )
{
	for (int sx = 0; sx < 50; sx++)
	{
		offs_t const addr = (y * 50) + sx;
		uint16_t data = m_bitmap_ram[addr];

		for (int bit = 0; bit < 16; bit++)
		{
			int const x = (sx * 16) + bit;
			int const color = BIT(data, 15) && de;

			bitmap.pix(vbp + y, hbp + x) = PALETTE_MVC[color];

			data <<= 1;
		}
	}

	for (int column = 0; column < x_count; column++)
	{
		uint16_t const code = m_video_ram[((ma + column) & 0x7ff)];
		uint8_t attr = code & 0xff;

		// with the IBM PC emulation option driving the display, the attribute
		// byte is the one the emulated software wrote and follows the
		// monochrome adapter meaning, not the Wang one
		if (IBM_DISPLAY_MODE)
			attr = ibm_attribute(attr);

		uint8_t new_ra = ra + 1;

		if (ATTR_SUPERSCRIPT)
		{
			new_ra = ra + 3;
		}
		else if (ATTR_SUBSCRIPT)
		{
			new_ra = ra;
		}

		offs_t const addr = ((code >> 8) << 4) | (new_ra & 0x0f);
		uint16_t data = m_char_ram[addr & 0xfff];

		if ((column == cursor_x) || (!ra && ATTR_OVERSCORE) || ((ra == 9) && ATTR_UNDERSCORE))
		{
			data = 0xffff;
		}

		for (int bit = 0; bit < 10; bit++)
		{
			int const x = (column * 10) + bit;
			int color = ((BIT(data, 9) & ~ATTR_BLANK) ^ ATTR_REVERSE);

			if ((color | bitmap.pix(vbp + y, hbp + x)) & ATTR_BOLD)
				color = 2;
			if (color)
				bitmap.pix(vbp + y, hbp + x) = de ? PALETTE_MVC[color] : rgb_t::black();

			data <<= 1;
		}
	}
}

void wangpc_mvc_device::vsync_w(int state)
{
	if (OPTION_VSYNC && state)
	{
		set_irq(ASSERT_LINE);
	}
}

//-------------------------------------------------
//  machine_config( wangpc_mvc )
//-------------------------------------------------

void wangpc_mvc_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG));
	screen.set_screen_update(MC6845_TAG, FUNC(mc6845_device::screen_update));
	screen.set_size(80*10, 25*12);
	screen.set_visarea(0, 80*10-1, 0, 25*12-1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_refresh_hz(60);

	MC6845_1(config, m_crtc, XTAL(14'318'181)/16);
	m_crtc->set_screen(SCREEN_TAG);
	m_crtc->set_show_border_area(true);
	m_crtc->set_char_width(10);
	m_crtc->set_update_row_callback(FUNC(wangpc_mvc_device::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(wangpc_mvc_device::vsync_w));
}



//-------------------------------------------------
//  INPUT_PORTS( wangpc_mvc )
//-------------------------------------------------

static INPUT_PORTS_START( wangpc_mvc )
	PORT_START("SW")
	PORT_CONFNAME( 0x07, 0x00, "IBM PC Emulation Option" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "Monochrome (16K)" )
	PORT_CONFSETTING(    0x03, "Monochrome (32K)" )
	// the settings above make the software look for a TIG card in the next
	// slot to put the emulated display on; these declare that the board
	// drives the display itself, which is what this implementation does
	PORT_CONFSETTING(    0x05, "Monochrome (16K), self-contained" )
	PORT_CONFSETTING(    0x07, "Monochrome (32K), self-contained" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor wangpc_mvc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( wangpc_mvc );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  set_irq -
//-------------------------------------------------

inline void wangpc_mvc_device::set_irq(int state)
{
	m_irq = state;

	m_bus->irq3_w(m_irq);
}


//-------------------------------------------------
//  ibm_ram_enabled - is the IBM PC emulation
//  option memory window turned on?
//-------------------------------------------------

inline bool wangpc_mvc_device::ibm_ram_enabled() const
{
	return IBM_OPTION_INSTALLED && BIT(m_ibm_option, 2) && BIT(m_ibm_enable, 0);
}


//-------------------------------------------------
//  ibm_ram_base - start of the option memory
//  window (32K boards decode 16K lower)
//-------------------------------------------------

inline offs_t wangpc_mvc_device::ibm_ram_base() const
{
	return IBM_OPTION_32K ? 0xf4000 : 0xf8000;
}


//-------------------------------------------------
//  ibm_attribute - translate an IBM monochrome
//  adapter attribute into the Wang one
//-------------------------------------------------

inline uint8_t wangpc_mvc_device::ibm_attribute(uint8_t attr) const
{
	uint8_t result = 0;

	if (BIT(attr, 7))                       // blink
		result |= 0x01;
	if ((attr & 0x77) == 0x70)              // reverse video
		result |= 0x02;
	if ((attr & 0x77) == 0x00)              // non display
		result |= 0x04;
	if (BIT(attr, 3))                       // high intensity
		result |= 0x08;
	if ((attr & 0x07) == 0x01)              // underline
		result |= 0x20;

	return result;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_mvc_device - constructor
//-------------------------------------------------

wangpc_mvc_device::wangpc_mvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_MVC, tag, owner, clock),
	device_wangpcbus_card_interface(mconfig, *this),
	m_crtc(*this, MC6845_TAG),
	m_video_ram(*this, "video_ram", VIDEO_RAM_SIZE*2, ENDIANNESS_LITTLE),
	m_char_ram(*this, "char_ram", CHAR_RAM_SIZE*2, ENDIANNESS_LITTLE),
	m_bitmap_ram(*this, "bitmap_ram", BITMAP_RAM_SIZE*2, ENDIANNESS_LITTLE),
	m_ibm_ram(*this, "ibm_ram", IBM_RAM_SIZE*2, ENDIANNESS_LITTLE),
	m_sw(*this, "SW"),
	m_option(0),
	m_ibm_option(0),
	m_ibm_enable(0),
	m_irq(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_mvc_device::device_start()
{
	// state saving
	save_item(NAME(m_option));
	save_item(NAME(m_ibm_option));
	save_item(NAME(m_ibm_enable));
	save_item(NAME(m_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_mvc_device::device_reset()
{
	m_option = 0;
	m_ibm_option = 0;
	m_ibm_enable = 0;

	set_irq(CLEAR_LINE);
}


//-------------------------------------------------
//  wangpcbus_mrdc_r - memory read
//-------------------------------------------------

uint16_t wangpc_mvc_device::wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (OPTION_VRAM)
	{
		if (offset >= 0xe0000/2 && offset < 0xe8000/2)
		{
			data = m_bitmap_ram[offset & 0x3fff];
		}
		else if (offset >= 0xf0000/2 && offset < 0xf1000/2)
		{
			data = m_video_ram[offset & 0x7ff];
		}
		else if (offset >= 0xf2000/2 && offset < 0xf4000/2)
		{
			data = m_char_ram[offset & 0xfff];
		}
	}

	if (ibm_ram_enabled())
	{
		if (offset >= ibm_ram_base()/2 && offset < 0xfc000/2)
		{
			data = m_ibm_ram[offset - ibm_ram_base()/2];
		}
		else if (offset >= 0xb0000/2 && offset < 0xb1000/2)
		{
			// the character memory also answers where an IBM monochrome
			// adapter would be, with the character in the low byte
			data = swapendian_int16(m_video_ram[offset & 0x7ff]);
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_mvc_device::wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
	if (OPTION_VRAM)
	{
		if (offset >= 0xe0000/2 && offset < 0xe8000/2)
		{
			m_bitmap_ram[offset & 0x3fff] = data;
		}
		else if (offset >= 0xf0000/2 && offset < 0xf1000/2)
		{
			m_video_ram[offset & 0x7ff] = data;
		}
		else if (offset >= 0xf2000/2 && offset < 0xf4000/2)
		{
			m_char_ram[offset & 0xfff] = data;
		}
	}

	if (ibm_ram_enabled())
	{
		if (offset >= ibm_ram_base()/2 && offset < 0xfc000/2)
		{
			// the option memory takes byte writes: DOS loads the emulation
			// ROM image into it a byte at a time
			COMBINE_DATA(&m_ibm_ram[offset - ibm_ram_base()/2]);
		}
		else if (offset >= 0xb0000/2 && offset < 0xb1000/2)
		{
			// see wangpcbus_mrdc_r: the emulated IBM software writes the
			// character in the low byte, the Wang card keeps it in the high
			// one, so the bytes swap on the way through
			uint16_t const swapped = swapendian_int16(data);
			uint16_t const swapped_mask = swapendian_int16(mem_mask);
			offs_t const addr = offset & 0x7ff;

			m_video_ram[addr] = (swapped & swapped_mask) | (m_video_ram[addr] & ~swapped_mask);
		}
	}
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

uint16_t wangpc_mvc_device::wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0x20/2:
			// bit 2 reports the IBM PC emulation option, bit 3 that the board
			// drives the display itself instead of needing a TIG card
			data = 0xff00 | (IBM_OPTION_INSTALLED ? 0x04 : 0x00) | (IBM_OPTION_STANDALONE ? 0x08 : 0x00);
			break;

		case 0x50/2:
			// the low nibble has to read back clear for the emulation
			// software to accept the card
			data = 0xff00;
			break;

		case 0xfe/2:
			data = 0xff00 | (m_irq << 7) | OPTION_ID;

			set_irq(CLEAR_LINE);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_mvc_device::wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
	if (sad(offset) && ACCESSING_BITS_0_7)
	{
		switch (offset & 0x7f)
		{
		case 0x00/2:
			m_crtc->address_w(data & 0xff);
			break;

		case 0x02/2:
			m_crtc->register_w(data & 0xff);
			break;

		case 0x10/2:
		case 0x12/2:
			if (LOG) logerror("MVC option %02x\n", data & 0xff);

			m_option = data & 0xff;
			break;

		case 0x20/2:
			if (LOG) logerror("MVC IBM option %02x\n", data & 0xff);

			m_ibm_option = data & 0xff;
			break;

		case 0xfa/2:
			if (LOG) logerror("MVC IBM enable %02x\n", data & 0xff);

			m_ibm_enable = data & 0xff;
			break;
		}
	}
}
