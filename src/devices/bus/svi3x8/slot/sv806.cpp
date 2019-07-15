// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-806 80 Column Cartridge for SVI-318/328

***************************************************************************/

#include "emu.h"
#include "sv806.h"

#include "screen.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SV806, sv806_device, "sv806", "SV-806 80 Column Cartridge")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sv806 )
	ROM_REGION(0x1000, "gfx", 0)
	ROM_SYSTEM_BIOS(0, "en", "English Character Set")
	ROMX_LOAD("sv806.ic27",   0x0000, 0x1000, CRC(850bc232) SHA1(ed45cb0e9bd18a9d7bd74f87e620f016a7ae840f), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "se", "Swedish Character Set")
	ROMX_LOAD("sv806se.ic27", 0x0000, 0x1000, CRC(daea8956) SHA1(3f16d5513ad35692488ae7d864f660e76c6e8ed3), ROM_BIOS(1))
ROM_END

const tiny_rom_entry *sv806_device::device_rom_region() const
{
	return ROM_NAME( sv806 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sv806_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "80col", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw((XTAL(12'000'000) / 6) * 8, 864, 0, 640, 317, 0, 192);
	screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	HD6845S(config, m_crtc, XTAL(12'000'000) / 6); // HD6845 (variant not verified)
	m_crtc->set_screen("80col");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(sv806_device::crtc_update_row), this);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv806_device - constructor
//-------------------------------------------------

sv806_device::sv806_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SV806, tag, owner, clock),
	device_svi_slot_interface(mconfig, *this),
	m_crtc(*this, "crtc"),
	m_palette(*this, "palette"),
	m_gfx(*this, "gfx"),
	m_ram_enabled(0)
{
	m_ram = std::make_unique<uint8_t[]>(0x800);
	memset(m_ram.get(), 0xff, 0x800);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv806_device::device_start()
{
	// register for savestates
	save_item(NAME(m_ram_enabled));
	save_pointer(NAME(m_ram), 0x800);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW( sv806_device::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	for (int i = 0; i < x_count; i++)
	{
		uint8_t data = m_gfx->as_u8((m_ram[(ma + i) & 0x7ff] << 4) | ra);

		if (i == cursor_x)
			data = 0xff;

		bitmap.pix32(y, i * 8 + 0) = pen[BIT(data, 7)];
		bitmap.pix32(y, i * 8 + 1) = pen[BIT(data, 6)];
		bitmap.pix32(y, i * 8 + 2) = pen[BIT(data, 5)];
		bitmap.pix32(y, i * 8 + 3) = pen[BIT(data, 4)];
		bitmap.pix32(y, i * 8 + 4) = pen[BIT(data, 3)];
		bitmap.pix32(y, i * 8 + 5) = pen[BIT(data, 2)];
		bitmap.pix32(y, i * 8 + 6) = pen[BIT(data, 1)];
		bitmap.pix32(y, i * 8 + 7) = pen[BIT(data, 0)];
	}
}

uint8_t sv806_device::mreq_r(offs_t offset)
{
	if (offset >= 0xf000 && m_ram_enabled)
	{
		m_bus->ramdis_w(0);
		return m_ram[offset & 0x7ff];
	}

	return 0xff;
}

void sv806_device::mreq_w(offs_t offset, uint8_t data)
{
	if (offset >= 0xf000 && m_ram_enabled)
	{
		m_bus->ramdis_w(0);
		m_ram[offset & 0x7ff] = data;
	}
}

uint8_t sv806_device::iorq_r(offs_t offset)
{
	if (offset == 0x51)
		return m_crtc->register_r();

	return 0xff;
}

void sv806_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x50: m_crtc->address_w(data); break;
	case 0x51: m_crtc->register_w(data); break;
	case 0x58: m_ram_enabled = data; break;
	}
}
