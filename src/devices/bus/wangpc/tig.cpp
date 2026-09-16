// license:BSD-3-Clause
// copyright-holders:Curt Coder, Fausto Pracek
/**********************************************************************

    Wang PC Text/Image/Graphics controller emulation

    The card carries two uPD7220 GDCs sharing one 800x600 monochrome
    screen: GDC0 draws the text plane out of its own frame and font
    memory, GDC1 draws a 1bpp graphics plane on top of it.

    Text plane (GDC0 address space):

    - 0x0000-0x0fff frame memory, one word per character cell: the
      character code is in the low byte and the attributes are in the
      high byte.  Only 4K words are fitted and the address is decoded
      incompletely, so the buffer repeats every 0x1000 words: the BIOS
      relies on that, scrolling by walking SAD one row at a time and
      letting it wrap around from 0x0ff0 back to 0x0040
    - 0x4000-0x7fff font memory, 32 words per character indexed by
      code << 5, one word per scan line with the leftmost dot in bit 9;
      the alternate font lives at +0x2000

    The cell is 10 dots wide and the display list programs 24 scan
    lines per row, so the visible text screen is 800x600.

    I/O map (relative to the card's slot address):

    - 0x00-0x1e attribute registers
    - 0x20/0x22 GDC0 (text)
    - 0x24/0x26 GDC1 (graphics)
    - 0x28      underscore scan line
    - 0x2a      option register: bit 0 selects which GDC the DMA
                channel talks to, bits 1-3 pick the bus DREQ line,
                bit 4 selects the option ID reported at 0xfe
    - 0xfc      reset

**********************************************************************/

#include "emu.h"
#include "tig.h"

#include "screen.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG 0

#define OPTION_ID_0         0x13
#define OPTION_ID_1         0x17

#define UPD7720_0_TAG       "upd7220_0"
#define UPD7720_1_TAG       "upd7220_1"
#define SCREEN_TAG          "screen"

#define DMA_GRAPHICS        BIT(m_option, 0)
#define DMA_DREQ1           BIT(m_option, 1)
#define DMA_DREQ2           BIT(m_option, 2)
#define DMA_DREQ3           BIT(m_option, 3)
#define DMA_ID              BIT(m_option, 4)

#define ATTR_ALT_FONT       BIT(data, 8)
#define ATTR_UNDERSCORE     BIT(data, 9)
#define ATTR_BLINK          BIT(data, 10)
#define ATTR_REVERSE        BIT(data, 11)
#define ATTR_BLANK          BIT(data, 12)
#define ATTR_BOLD           BIT(data, 13)
#define ATTR_SUBSCRIPT      BIT(data, 14)
#define ATTR_SUPERSCRIPT    BIT(data, 15)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WANGPC_TIG, wangpc_tig_device, "wangpc_tig", "Want PC Text/Image/Graphics Controller")


//-------------------------------------------------
//  ROM( wangpc_tig )
//-------------------------------------------------

ROM_START( wangpc_tig )
	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "377-3072.l26", 0x000, 0x100, NO_DUMP ) // PAL10L8
	ROM_LOAD( "377-3073.l16", 0x000, 0x100, NO_DUMP ) // PAL10L8
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *wangpc_tig_device::device_rom_region() const
{
	return ROM_NAME( wangpc_tig );
}


//-------------------------------------------------
//  UPD7220_INTERFACE( hgdc0_intf )
//-------------------------------------------------

void wangpc_tig_device::upd7220_0_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x0fff).mirror(0x3000).ram().share("frame_ram"); // frame buffer
	map(0x4000, 0x7fff).ram().share("font_ram"); // font memory
}

UPD7220_DRAW_TEXT_LINE_MEMBER( wangpc_tig_device::hgdc_draw_text )
{
	for (int column = 0; column < pitch; column++)
	{
		uint16_t const code = m_frame_ram[(addr + column) & 0x0fff];
		uint8_t const chr = code & 0xff;
		uint16_t const data = code; // attribute bits live in the high byte

		for (int sy = 0; sy < lr; sy++)
		{
			// the font stores 32 rows per character; the cell displays
			// 24 of them, sub/superscript shift the window
			uint8_t row = sy;

			if (ATTR_SUPERSCRIPT)
				row = sy + 6;
			else if (ATTR_SUBSCRIPT)
				row = (sy >= 6) ? sy - 6 : 0;

			offs_t faddr = (ATTR_ALT_FONT ? 0x2000 : 0) | (chr << 5) | (row & 0x1f);
			uint16_t dots = m_font_ram[faddr & 0x3fff];

			bool const cursor = (cursor_on && (addr + column) == cursor_addr);

			if (cursor || (ATTR_UNDERSCORE && sy == (m_underline & 0x1f)))
				dots = 0x3ff;

			if (ATTR_BLANK)
				dots = 0;

			if (ATTR_REVERSE)
				dots ^= 0x3ff;

			// the GDC can be programmed with more rows than the bitmap
			// holds (e.g. during a 25/35-line mode switch): clip
			if (y + sy >= 0 && y + sy < bitmap.height())
			{
				for (int bit = 0; bit < 10; bit++)
				{
					int const x = (column * 10) + bit;
					if (x >= bitmap.width())
						break;
					int const color = BIT(dots, 9 - bit) ? (ATTR_BOLD ? 2 : 1) : 0;

					bitmap.pix(y + sy, x) = m_palette->pen(color);
				}
			}
		}
	}
}


//-------------------------------------------------
//  UPD7220_INTERFACE( hgdc1_intf )
//-------------------------------------------------

void wangpc_tig_device::upd7220_1_map(address_map &map)
{
	map.global_mask(0xffff);
	map(0x0000, 0xffff).ram().share("gfx_ram"); // graphics memory
}

UPD7220_DISPLAY_PIXELS_MEMBER( wangpc_tig_device::hgdc_display_pixels )
{
	// 1 bpp graphics layer, drawn over the text layer: only set pixels
	// are written so the text underneath stays visible
	if (y < 0 || y >= bitmap.height())
		return;

	uint16_t const data = m_gfx_ram[address & 0xffff];

	for (int i = 0; i < 16; i++)
	{
		int const px = x + i;

		if (px >= bitmap.width())
			break;

		if (BIT(data, i))
			bitmap.pix(y, px) = m_palette->pen(1);
	}
}


//-------------------------------------------------
//  machine_config( wangpc_tig )
//-------------------------------------------------

void wangpc_tig_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG).set_color(rgb_t::green()));
	screen.set_screen_update(FUNC(wangpc_tig_device::screen_update));
	screen.set_size(80*10, 25*24);
	screen.set_visarea(0, 80*10-1, 0, 25*24-1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_refresh_hz(60);

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	UPD7220(config, m_hgdc0, XTAL(52'832'000)/28);
	m_hgdc0->set_addrmap(0, &wangpc_tig_device::upd7220_0_map);
	m_hgdc0->set_draw_text(FUNC(wangpc_tig_device::hgdc_draw_text));
	m_hgdc0->set_screen(SCREEN_TAG);
	m_hgdc0->set_char_dots(10); // the character cell is 10 dots wide
	m_hgdc0->drq_wr_callback().set(FUNC(wangpc_tig_device::gdc0_drq_w));

	UPD7220(config, m_hgdc1, XTAL(52'832'000)/28);
	m_hgdc1->set_addrmap(0, &wangpc_tig_device::upd7220_1_map);
	m_hgdc1->set_display_pixels(FUNC(wangpc_tig_device::hgdc_display_pixels));
	m_hgdc1->set_screen(SCREEN_TAG);
	m_hgdc1->drq_wr_callback().set(FUNC(wangpc_tig_device::gdc1_drq_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_tig_device - constructor
//-------------------------------------------------

wangpc_tig_device::wangpc_tig_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_TIG, tag, owner, clock),
	device_wangpcbus_card_interface(mconfig, *this),
	m_hgdc0(*this, UPD7720_0_TAG),
	m_hgdc1(*this, UPD7720_1_TAG),
	m_frame_ram(*this, "frame_ram"),
	m_font_ram(*this, "font_ram"),
	m_gfx_ram(*this, "gfx_ram"),
	m_option(0), m_underline(0),
	m_gdc0_drq(0), m_gdc1_drq(0),
	m_palette(*this, "palette")
{
}


//-------------------------------------------------
//  DRQ routing - the option register selects which
//  GDC drives the DMA channel and which bus DREQ
//  line carries the request
//-------------------------------------------------

void wangpc_tig_device::gdc0_drq_w(int state)
{
	m_gdc0_drq = state;
	update_drq();
}

void wangpc_tig_device::gdc1_drq_w(int state)
{
	m_gdc1_drq = state;
	update_drq();
}

void wangpc_tig_device::update_drq()
{
	int const state = DMA_GRAPHICS ? m_gdc1_drq : m_gdc0_drq;

	if (DMA_DREQ1) m_bus->drq1_w(state);
	if (DMA_DREQ2) m_bus->drq2_w(state);
	if (DMA_DREQ3) m_bus->drq3_w(state);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_tig_device::device_start()
{
	// state saving
	save_item(NAME(m_option));
	save_item(NAME(m_attr));
	save_item(NAME(m_underline));
	save_item(NAME(m_gdc0_drq));
	save_item(NAME(m_gdc1_drq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_tig_device::device_reset()
{
	m_option = 0;
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

uint32_t wangpc_tig_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_hgdc0->screen_update(screen, bitmap, cliprect);
	m_hgdc1->screen_update(screen, bitmap, cliprect);

	return 0;
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

uint16_t wangpc_tig_device::wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0x20/2:
		case 0x22/2:
			data = m_hgdc0->read(offset);
			break;

		case 0x24/2:
		case 0x26/2:
			data = m_hgdc1->read(offset);
			break;

		case 0xfe/2:
			data = 0xff00 | (DMA_ID ? OPTION_ID_1 : OPTION_ID_0);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_tig_device::wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
	if (sad(offset) && ACCESSING_BITS_0_7)
	{
		switch (offset & 0x7f)
		{
		case 0x00/2: case 0x02/2: case 0x04/2: case 0x06/2: case 0x08/2: case 0x0a/2: case 0x0c/2: case 0x0e/2:
		case 0x10/2: case 0x12/2: case 0x14/2: case 0x16/2: case 0x18/2: case 0x1a/2: case 0x1c/2: case 0x1e/2:
			if (LOG) logerror("TIG attribute %u: %02x\n", offset & 0x0f, data & 0xff);

			m_attr[offset & 0x0f] = data & 0xff;
			break;

		case 0x20/2:
		case 0x22/2:
			m_hgdc0->write(offset, data);
			break;

		case 0x24/2:
		case 0x26/2:
			m_hgdc1->write(offset, data);
			break;

		case 0x28/2:
			if (LOG) logerror("TIG underline %02x\n", data & 0xff);

			m_underline = data & 0xff;
			break;

		case 0x2a/2:
			if (LOG) logerror("TIG option %02x\n", data & 0xff);

			// release the DREQ lines of the old routing before switching
			if (DMA_DREQ1) m_bus->drq1_w(0);
			if (DMA_DREQ2) m_bus->drq2_w(0);
			if (DMA_DREQ3) m_bus->drq3_w(0);

			m_option = data & 0xff;
			update_drq();
			break;

		case 0xfc/2:
			device_reset();
			break;
		}
	}
}


//-------------------------------------------------
//  wangpcbus_dack_r - DMA read
//-------------------------------------------------

uint8_t wangpc_tig_device::wangpcbus_dack_r(int line)
{
	uint8_t data;

	if (DMA_GRAPHICS)
	{
		data = m_hgdc1->dack_r();
	}
	else
	{
		data = m_hgdc0->dack_r();
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_dack_w - DMA write
//-------------------------------------------------

void wangpc_tig_device::wangpcbus_dack_w(int line, uint8_t data)
{
	if (DMA_GRAPHICS)
	{
		m_hgdc1->dack_w(data);
	}
	else
	{
		m_hgdc0->dack_w(data);
	}
}


//-------------------------------------------------
//  wangpcbus_have_dack - DMA acknowledge
//-------------------------------------------------

bool wangpc_tig_device::wangpcbus_have_dack(int line)
{
	return (line == 1 && DMA_DREQ1) || (line == 2 && DMA_DREQ2) || (line == 3 && DMA_DREQ3);
}
