// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-806 80 Column Cartridge for SVI-318/328

***************************************************************************/

#include "sv806.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type SV806 = &device_creator<sv806_device>;

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sv806 )
	ROM_REGION(0x1000, "gfx", 0)
	ROM_SYSTEM_BIOS(0, "en", "English Character Set")
	ROMX_LOAD("sv806.ic27",   0x0000, 0x1000, CRC(850bc232) SHA1(ed45cb0e9bd18a9d7bd74f87e620f016a7ae840f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "se", "Swedish Character Set")
	ROMX_LOAD("sv806se.ic27", 0x0000, 0x1000, CRC(daea8956) SHA1(3f16d5513ad35692488ae7d864f660e76c6e8ed3), ROM_BIOS(2))
ROM_END

const rom_entry *sv806_device::device_rom_region() const
{
	return ROM_NAME( sv806 );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sv806 )
	MCFG_SCREEN_ADD_MONOCHROME("80col", RASTER, rgb_t::green)
	MCFG_SCREEN_RAW_PARAMS((XTAL_12MHz / 6) * 8, 864, 0, 640, 317, 0, 192)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", hd6845_device, screen_update)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_MC6845_ADD("crtc", HD6845, "80col", XTAL_12MHz / 6)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(sv806_device, crtc_update_row)
MACHINE_CONFIG_END

machine_config_constructor sv806_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sv806 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv806_device - constructor
//-------------------------------------------------

sv806_device::sv806_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, SV806, "SV-806 80 Column Cartridge", tag, owner, clock, "sv806", __FILE__),
	device_svi_slot_interface(mconfig, *this),
	m_crtc(*this, "crtc"),
	m_palette(*this, "palette"),
	m_gfx(*this, "gfx"),
	m_ram_enabled(0)
{
	m_ram = std::make_unique<UINT8[]>(0x800);
	memset(m_ram.get(), 0xff, 0x800);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv806_device::device_start()
{
	// register for savestates
	save_item(NAME(m_ram_enabled));
	save_pointer(NAME(m_ram.get()), 0x800);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

MC6845_UPDATE_ROW( sv806_device::crtc_update_row )
{
	const pen_t *pen = m_palette->pens();

	for (int i = 0; i < x_count; i++)
	{
		UINT8 data = m_gfx->u8((m_ram[(ma + i) & 0x7ff] << 4) | ra);

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

READ8_MEMBER( sv806_device::mreq_r )
{
	if (offset >= 0xf000 && m_ram_enabled)
	{
		m_bus->ramdis_w(0);
		return m_ram[offset & 0x7ff];
	}

	return 0xff;
}

WRITE8_MEMBER( sv806_device::mreq_w )
{
	if (offset >= 0xf000 && m_ram_enabled)
	{
		m_bus->ramdis_w(0);
		m_ram[offset & 0x7ff] = data;
	}
}

READ8_MEMBER( sv806_device::iorq_r )
{
	if (offset == 0x51)
		return m_crtc->register_r(space, 0);

	return 0xff;
}

WRITE8_MEMBER( sv806_device::iorq_w )
{
	switch (offset)
	{
	case 0x50: m_crtc->address_w(space, 0, data); break;
	case 0x51: m_crtc->register_w(space, 0, data); break;
	case 0x58: m_ram_enabled = data; break;
	}
}
