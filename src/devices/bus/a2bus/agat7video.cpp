// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat7video.cpp

    Implementation of Agat-7 onboard video.

	5 video modes:
	- 32x32 color text
	- 64x32 mono text with reverse video
	- 64x64 color graphics
	- 128x128 color graphics
	- 256x256 mono graphics

	Character generator ROM could have 128 or 256 chars.

    C7xx: video mode select

*********************************************************************/

#include "agat7video.h"
#include "includes/apple2.h"

#include "screen.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_AGAT7VIDEO = device_creator<a2bus_agat7video_device>;

static const gfx_layout ksm_charlayout =
{
	8, 8,                   /* 7x8 pixels in 10x11 cell */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( ksm )
	GFXDECODE_ENTRY("gfx1", 0x0000, ksm_charlayout, 0, 1)
GFXDECODE_END

MACHINE_CONFIG_FRAGMENT( agat7video )
	MCFG_SCREEN_ADD("a7screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_10_5MHz, 672, 0, 512, 312, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(a2bus_agat7video_device, screen_update)
	MCFG_SCREEN_PALETTE("a7palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "a7palette", ksm);

	MCFG_PALETTE_ADD("a7palette", 16)
	MCFG_PALETTE_INIT_OWNER(a2bus_agat7video_device, agat7)
MACHINE_CONFIG_END

ROM_START( agat7video )
	ROM_REGION(0x0800, "gfx1", 0)
	ROM_LOAD( "agathe7.fnt", 0x0000, 0x0800, CRC(fcffb490) SHA1(0bda26ae7ad75f74da835c0cf6d9928f9508844c))
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_agat7video_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( agat7video );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_agat7video_device::device_rom_region() const
{
	return ROM_NAME( agat7video );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_agat7video_device::a2bus_agat7video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A2BUS_AGAT7VIDEO, "Agat-7 video card", tag, owner, clock, "agat7video", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_palette(*this, "a7palette")
{
}

a2bus_agat7video_device::a2bus_agat7video_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_palette(*this, "a7palette")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_agat7video_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_start_address = 0x7800;

	m_ram_dev = machine().device<ram_device>(":" RAM_TAG);
	m_ram_ptr = machine().root_device().memregion("maincpu")->base();
	m_char_ptr = machine().root_device().memregion("gfx1")->base();
	m_char_size = machine().root_device().memregion("gfx1")->bytes();
}

void a2bus_agat7video_device::device_reset()
{
}


/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_agat7video_device::read_cnxx(address_space &space, uint8_t offset)
{
	do_io(offset);
	return 0;
}

/*-------------------------------------------------
    write_cnxx - called for writes to this card's cnxx space
-------------------------------------------------*/
void a2bus_agat7video_device::write_cnxx(address_space &space, uint8_t offset, uint8_t data)
{
	do_io(offset);
}


void a2bus_agat7video_device::do_io(int offset)
{
	switch (offset & 3)
	{
	case 0:
		m_video_mode = GRAPHICS_LORES;
		m_start_address = (offset) << 9;
		logerror("offset %04X, video mode 0 (GRAPHICS_LORES)\n", m_start_address);
		break;

	case 1:
		m_video_mode = GRAPHICS_HIRES;
		m_start_address = ((offset & 0x3f) - 0x01) << 9;
		logerror("offset %04X, video mode 1 (GRAPHICS_HIRES)\n", m_start_address);
		break;

	case 2:
		if (offset > 0x80) {
			m_video_mode = TEXT_HIRES;
			m_start_address = (offset - 0x82) << 9;
			logerror("offset %04X, video mode 2 (TEXT_HIRES)\n", m_start_address);
		} else {
			m_video_mode = TEXT_LORES;
			m_start_address = (offset - 0x02) << 9;
			logerror("offset %04X, video mode 2 (TEXT_LORES)\n", m_start_address);
		}
		break;

	case 3:
		m_video_mode = GRAPHICS_MONO;
		m_start_address = ((offset & 0x3f) - 0x03) << 9;
		logerror("offset %04X, video mode 3 (GRAPHICS_MONO)\n", m_start_address);
		break;
	}
}


void a2bus_agat7video_device::plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
	const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg)
{
	int x, y, i;
	const uint8_t *chardata;
	uint16_t color;

	/* look up the character data */
	chardata = &textgfx_data[(code * 8)];

	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 8; x++)
		{
			color = (chardata[y] & (1 << (7-x))) ? fg : bg;

			for (i = 0; i < xscale; i++)
			{
				bitmap.pix16(ypos + y, xpos + (x * xscale) + i) = color;
			}
		}
	}
}

void a2bus_agat7video_device::text_update_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t address;
	uint8_t ch, attr;
	int fg = 0;
	int bg = 0;

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 32; col++)
		{
			/* calculate address */
			address = m_start_address + (col * 2) + (row * 8);
			ch = m_ram_dev->read(address);
			attr = m_ram_dev->read(address + 1);
			if (BIT(attr, 5)) {
				fg = BITSWAP8(attr,7,6,5,3,4,2,1,0) & 15;
				bg = 0;
			} else {
				fg = 0;
				bg = BITSWAP8(attr,7,6,5,3,4,2,1,0) & 15;
			}
			plot_text_character(bitmap, col * 16, row, 2, ch, m_char_ptr, m_char_size, fg, bg);
		}
	}
}

void a2bus_agat7video_device::text_update_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col;
	uint32_t address;
	uint8_t ch;
	int fg, bg;

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	if (m_start_address & 0x800) {
		fg = 7; bg = 0;
	} else {
		fg = 0; bg = 7;
	}

	for (row = beginrow; row <= endrow; row += 8)
	{
		for (col = 0; col < 64; col++)
		{
			/* calculate address */
			address = m_start_address + col + (row * 8);
			ch = m_ram_dev->read(address);
			plot_text_character(bitmap, col * 8, row, 1, ch, m_char_ptr, m_char_size, fg, bg);
		}
	}
}

void a2bus_agat7video_device::graph_update_mono(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col, b;
	uint32_t address;
	uint16_t *p;
	uint8_t gfx, v;
	int fg = 7, bg = 0;

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (row = beginrow; row <= endrow; row++)
	{
		p = &bitmap.pix16(row);
		for (col = 0; col < 32; col++)
		{
			address = m_start_address + col + (row * 0x20);
			gfx = m_ram_dev->read(address);

			for (b = 0; b < 8; b++)
			{
				v = (gfx & 0x80);
				gfx <<= 1;
				*(p++) = v ? fg : bg;
				*(p++) = v ? fg : bg;
			}
		}
	}
}

void a2bus_agat7video_device::graph_update_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col, b;
	uint32_t address;
	uint16_t *p;
	uint8_t gfx, v;

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (row = beginrow; row <= endrow; row++)
	{
		p = &bitmap.pix16(row);
		for (col = 0; col < 0x40; col++)
		{
			address = m_start_address + col + ((row/2) * 0x40);
			gfx = m_ram_dev->read(address);

			for (b = 0; b < 2; b++)
			{
				v = (gfx & 0xf0) >> 4;
				gfx <<= 4;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
			}
		}
	}
}

void a2bus_agat7video_device::graph_update_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow)
{
	int row, col, b;
	uint32_t address;
	uint16_t *p;
	uint8_t gfx, v;

	beginrow = std::max(beginrow, cliprect.min_y - (cliprect.min_y % 8));
	endrow = std::min(endrow, cliprect.max_y - (cliprect.max_y % 8) + 7);

	for (row = beginrow; row <= endrow; row++)
	{
		p = &bitmap.pix16(row);
		for (col = 0; col < 0x20; col++)
		{
			address = m_start_address + col + ((row/4) * 0x20);
			gfx = m_ram_dev->read(address);

			for (b = 0; b < 2; b++)
			{
				v = (gfx & 0xf0) >> 4;
				gfx <<= 4;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
				*(p++) = v;
			}
		}
	}
}


uint32_t a2bus_agat7video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	switch (m_video_mode)
	{
	case TEXT_LORES:
		text_update_lores(screen, bitmap, cliprect, 0, 255);
		break;

	case TEXT_HIRES:
		text_update_hires(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_MONO:
		graph_update_mono(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_LORES:
		graph_update_lores(screen, bitmap, cliprect, 0, 255);
		break;

	case GRAPHICS_HIRES:
		graph_update_hires(screen, bitmap, cliprect, 0, 255);
		break;

	default:
		graph_update_mono(screen, bitmap, cliprect, 0, 255);
		break;
	}

	return 0;
}

// per http://agatcomp.ru/Reading/IiO/87-2-077.djvu
static const rgb_t agat7_palette[] =
{
	rgb_t::black(),
	rgb_t(0xFF, 0x00, 0x00),  /* White */
	rgb_t(0x00, 0xFF, 0x00),  /* White */
	rgb_t(0xFF, 0xFF, 0x00),  /* White */
	rgb_t(0x00, 0x00, 0xFF),  /* White */
	rgb_t(0xFF, 0x00, 0xFF),  /* White */
	rgb_t(0xFF, 0xFF, 0x00),  /* White */
	rgb_t(0xFF, 0xFF, 0xFF),  /* White */
	rgb_t::black(),
	rgb_t(0x7F, 0x00, 0x00),  /* White */
	rgb_t(0x00, 0x7F, 0x00),  /* White */
	rgb_t(0x7F, 0x7F, 0x00),  /* White */
	rgb_t(0x00, 0x00, 0x7F),  /* White */
	rgb_t(0x7F, 0x00, 0x7F),  /* White */
	rgb_t(0x7F, 0x7F, 0x00),  /* White */
	rgb_t(0x7F, 0x7F, 0x7F)   /* White */
};

PALETTE_INIT_MEMBER(a2bus_agat7video_device, agat7)
{
	palette.set_pen_colors(0, agat7_palette, ARRAY_LENGTH(agat7_palette));
}
