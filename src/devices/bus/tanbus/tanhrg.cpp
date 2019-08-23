// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Tangerine High Resolution Graphics Card (MT0055 Iss2)

    http://www.microtan.ukpc.net/pageProducts.html#VIDEO

    TODO:
    - option to merge MT65 chunky and HRG screens

**********************************************************************/


#include "emu.h"
#include "tanhrg.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_TANHRG, tanbus_tanhrg_device, "tanbus_tanhrg", "Tangerine High Resolution Graphics Card (monochrome)")
DEFINE_DEVICE_TYPE(TANBUS_TANHRGC, tanbus_tanhrgc_device, "tanbus_tanhrgc", "Tangerine High Resolution Graphics Card (colour)")

//-------------------------------------------------
//  INPUT_PORTS( tanhrg )
//-------------------------------------------------

INPUT_PORTS_START(tanhrg)
	PORT_START("DSW")
	PORT_DIPNAME(0x07, 0x04, "Address Space") PORT_DIPLOCATION("DSW:1,2,3")
	PORT_DIPSETTING(0x00, "&0000-&1FFF (Invalid - Do Not Use)")
	PORT_DIPSETTING(0x01, "&2000-&3FFF")
	PORT_DIPSETTING(0x02, "&4000-&5FFF")
	PORT_DIPSETTING(0x03, "&6000-&7FFF")
	PORT_DIPSETTING(0x04, "&8000-&9FFF")
	PORT_DIPSETTING(0x05, "&A000-&BFFF (Invalid - Do Not Use)")
	PORT_DIPSETTING(0x06, "&C000-&DFFF")
	PORT_DIPSETTING(0x07, "&E000-&FFFF")
	PORT_DIPNAME(0x08, 0x00, "Video") PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "Inhibit RAM (INHRAM)") PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "Block Enable (BE)") PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
INPUT_PORTS_END


INPUT_PORTS_START(tanhrgc)
	PORT_START("DSW_1")
	PORT_DIPNAME(0x07, 0x02, "Address Space - Red") PORT_DIPLOCATION("DSW_R:1,2,3")
	PORT_DIPSETTING(0x00, "&0000-&1FFF (Invalid - Do Not Use)")
	PORT_DIPSETTING(0x01, "&2000-&3FFF")
	PORT_DIPSETTING(0x02, "&4000-&5FFF")
	PORT_DIPSETTING(0x03, "&6000-&7FFF")
	PORT_DIPSETTING(0x04, "&8000-&9FFF")
	PORT_DIPSETTING(0x05, "&A000-&BFFF (Invalid - Do Not Use)")
	PORT_DIPSETTING(0x06, "&C000-&DFFF")
	PORT_DIPSETTING(0x07, "&E000-&FFFF")
	PORT_DIPNAME(0x08, 0x00, "Video") PORT_DIPLOCATION("DSW_R:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "Inhibit RAM (INHRAM)") PORT_DIPLOCATION("DSW_R:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "Block Enable (BE)") PORT_DIPLOCATION("DSW_R:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))

	PORT_START("DSW_2")
	PORT_DIPNAME(0x07, 0x04, "Address Space - Green") PORT_DIPLOCATION("DSW_G:1,2,3")
	PORT_DIPSETTING(0x00, "&0000-&1FFF (Invalid - Do Not Use)")
	PORT_DIPSETTING(0x01, "&2000-&3FFF")
	PORT_DIPSETTING(0x02, "&4000-&5FFF")
	PORT_DIPSETTING(0x03, "&6000-&7FFF")
	PORT_DIPSETTING(0x04, "&8000-&9FFF")
	PORT_DIPSETTING(0x05, "&A000-&BFFF (Invalid - Do Not Use)")
	PORT_DIPSETTING(0x06, "&C000-&DFFF")
	PORT_DIPSETTING(0x07, "&E000-&FFFF")
	PORT_DIPNAME(0x08, 0x00, "Video") PORT_DIPLOCATION("DSW_G:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "Inhibit RAM (INHRAM)") PORT_DIPLOCATION("DSW_G:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "Block Enable (BE)") PORT_DIPLOCATION("DSW_G:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))

	PORT_START("DSW_3")
	PORT_DIPNAME(0x07, 0x03, "Address Space - Blue") PORT_DIPLOCATION("DSW_B:1,2,3")
	PORT_DIPSETTING(0x00, "&0000-&1FFF (Invalid - Do Not Use)")
	PORT_DIPSETTING(0x01, "&2000-&3FFF")
	PORT_DIPSETTING(0x02, "&4000-&5FFF")
	PORT_DIPSETTING(0x03, "&6000-&7FFF")
	PORT_DIPSETTING(0x04, "&8000-&9FFF")
	PORT_DIPSETTING(0x05, "&A000-&BFFF (Invalid - Do Not Use)")
	PORT_DIPSETTING(0x06, "&C000-&DFFF")
	PORT_DIPSETTING(0x07, "&E000-&FFFF")
	PORT_DIPNAME(0x08, 0x00, "Video") PORT_DIPLOCATION("DSW_B:4")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "Inhibit RAM (INHRAM)") PORT_DIPLOCATION("DSW_B:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "Block Enable (BE)") PORT_DIPLOCATION("DSW_B:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor tanbus_tanhrg_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tanhrg);
}


ioport_constructor tanbus_tanhrgc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tanhrgc);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_tanhrg_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(6_MHz_XTAL, 384, 0, 256, 312, 0, 256);
	m_screen->set_screen_update(FUNC(tanbus_tanhrg_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}


void tanbus_tanhrgc_device::device_add_mconfig(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(6_MHz_XTAL, 384, 0, 256, 312, 0, 256);
	m_screen->set_screen_update(FUNC(tanbus_tanhrgc_device::screen_update));

	PALETTE(config, m_palette, palette_device::RGB_3BIT);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_tanhrg_device - constructor
//-------------------------------------------------

tanbus_tanhrg_device::tanbus_tanhrg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_TANHRG, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_dsw(*this, "DSW")
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
{
}


tanbus_tanhrgc_device::tanbus_tanhrgc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_TANHRGC, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_dsw(*this, "DSW_%u", 1)
	, m_screen(*this, "screen")
	, m_palette(*this, "palette")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_tanhrg_device::device_start()
{
	m_videoram = std::make_unique<uint8_t[]>(0x2000);

	/* randomize video memory contents */
	for (uint16_t addr = 0; addr < 0x2000; addr++)
		m_videoram[addr] = machine().rand() & 0xff;

	save_pointer(NAME(m_videoram), 0x2000);
}


void tanbus_tanhrgc_device::device_start()
{
	m_videoram = std::make_unique<uint8_t[]>(0x6000);

	/* randomize video memory contents */
	for (uint16_t addr = 0; addr < 0x6000; addr++)
		m_videoram[addr] = machine().rand() & 0xff;

	save_pointer(NAME(m_videoram), 0x6000);
}


//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_tanhrg_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	offs_t addr = (m_dsw->read() & 7) << 13;

	if ((!BIT(m_dsw->read(), 5) || be) && (offset & 0xe000) == addr)
	{
		data = m_videoram[offset & 0x1fff];
	}

	return data;
}


uint8_t tanbus_tanhrgc_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	offs_t addr_r = (m_dsw[0]->read() & 7) << 13;
	offs_t addr_g = (m_dsw[1]->read() & 7) << 13;
	offs_t addr_b = (m_dsw[2]->read() & 7) << 13;

	if ((!BIT(m_dsw[0]->read(), 5) || be) && (offset & 0xe000) == addr_r)
	{
		data = m_videoram[0x0000 | (offset & 0x1fff)];
	}
	else if ((!BIT(m_dsw[1]->read(), 5) || be) && (offset & 0xe000) == addr_g)
	{
		data = m_videoram[0x2000 | (offset & 0x1fff)];
	}
	else if ((!BIT(m_dsw[2]->read(), 5) || be) && (offset & 0xe000) == addr_b)
	{
		data = m_videoram[0x4000 | (offset & 0x1fff)];
	}

	return data;
}


//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_tanhrg_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	offs_t addr = (m_dsw->read() & 7) << 13;

	if ((!BIT(m_dsw->read(), 5) || be) && (offset & 0xe000) == addr)
	{
		m_videoram[offset & 0x1fff] = data;
	}
}


void tanbus_tanhrgc_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	offs_t addr_r = (m_dsw[0]->read() & 7) << 13;
	offs_t addr_g = (m_dsw[1]->read() & 7) << 13;
	offs_t addr_b = (m_dsw[2]->read() & 7) << 13;

	if ((!BIT(m_dsw[0]->read(), 5) || be) && (offset & 0xe000) == addr_r)
	{
		m_videoram[0x0000 | (offset & 0x1fff)] = data;
	}
	else if ((!BIT(m_dsw[1]->read(), 5) || be) && (offset & 0xe000) == addr_g)
	{
		m_videoram[0x2000 | (offset & 0x1fff)] = data;
	}
	else if ((!BIT(m_dsw[2]->read(), 5) || be) && (offset & 0xe000) == addr_b)
	{
		m_videoram[0x4000 | (offset & 0x1fff)] = data;
	}
}


//-------------------------------------------------
//  set_inhibit_lines
//-------------------------------------------------

void tanbus_tanhrg_device::set_inhibit_lines(offs_t offset, int &inhram, int &inhrom)
{
	offs_t addr = (m_dsw->read() & 7) << 13;

	if (BIT(m_dsw->read(), 4) && (offset & 0xe000) == addr)
	{
		inhram = 1;
	}
};


void tanbus_tanhrgc_device::set_inhibit_lines(offs_t offset, int &inhram, int &inhrom)
{
	offs_t addr_r = (m_dsw[0]->read() & 7) << 13;
	offs_t addr_g = (m_dsw[1]->read() & 7) << 13;
	offs_t addr_b = (m_dsw[2]->read() & 7) << 13;

	if ((BIT(m_dsw[0]->read(), 4) && (offset & 0xe000) == addr_r) || (BIT(m_dsw[1]->read(), 4) && (offset & 0xe000) == addr_g) || (BIT(m_dsw[2]->read(), 4) && (offset & 0xe000) == addr_b))
	{
		inhram = 1;
	}
};


//-------------------------------------------------
//  screen_update
//-------------------------------------------------

uint32_t tanbus_tanhrg_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t offset;
	uint32_t *p;

	for (int y = 0; y < 256; y++)
	{
		p = &bitmap.pix32(y);

		for (int x = 0; x < 256; x++)
		{
			offset = (y * 32) + (x / 8);

			*p++ = m_palette->pen_color(BIT(m_videoram[offset], x & 7));
		}
	}

	return 0;
}


uint32_t tanbus_tanhrgc_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t r, g, b;
	uint16_t offset;
	uint32_t *p;

	for (int y = 0; y < 256; y++)
	{
		p = &bitmap.pix32(y);

		for (int x = 0; x < 256; x++)
		{
			offset = (y * 32) + (x / 8);

			r = m_videoram[0x0000 | offset];
			b = m_videoram[0x2000 | offset];
			g = m_videoram[0x4000 | offset];

			*p++ = m_palette->pen_color(BIT(b, x & 7) << 2 | BIT(g, x & 7) << 1 | BIT(r, x & 7));
		}
	}

	return 0;
}
