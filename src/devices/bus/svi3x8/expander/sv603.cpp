// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-603 Coleco Game Adapter for SVI-318/328

***************************************************************************/

#include "emu.h"
#include "sv603.h"

#include "softlist.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SV603, sv603_device, "sv603", "SV-603 Coleco Game Adapter")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sv603 )
	ROM_REGION(0x2000, "bios", 0)
	ROM_LOAD("sv603.ic10", 0x0000, 0x2000, CRC(19e91b82) SHA1(8a30abe5ffef810b0f99b86db38b1b3c9d259b78))
ROM_END

const tiny_rom_entry *sv603_device::device_rom_region() const
{
	return ROM_NAME( sv603 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(sv603_device::device_add_mconfig)
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("snd", SN76489A, XTAL(10'738'635) / 3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	// cartridge slot
	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "coleco_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom,col")
	MCFG_GENERIC_LOAD(sv603_device, cartridge)
	SOFTWARE_LIST(config, "cart_list").set_original("coleco");
MACHINE_CONFIG_END


//**************************************************************************
//  CARTRIDGE
//**************************************************************************

DEVICE_IMAGE_LOAD_MEMBER( sv603_device, cartridge )
{
	uint32_t size = m_cart_rom->common_get_size("rom");

	m_cart_rom->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart_rom->common_load_rom(m_cart_rom->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv603_device - constructor
//-------------------------------------------------

sv603_device::sv603_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SV603, tag, owner, clock),
	device_svi_expander_interface(mconfig, *this),
	m_bios(*this, "bios"),
	m_snd(*this, "snd"),
	m_cart_rom(*this, "cartslot")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv603_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sv603_device::device_reset()
{
	m_expander->ctrl1_w(0);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER( sv603_device::mreq_r )
{
	m_expander->romdis_w(0);

	if (offset < 0x8000)
		return m_cart_rom->read_rom(offset);

	if (offset >= 0x8000 && offset < 0xa000)
	{
		m_expander->ramdis_w(0);
		return m_bios->as_u8(offset & 0x1fff);
	}

	return 0xff;
}

WRITE8_MEMBER( sv603_device::mreq_w )
{
	m_expander->romdis_w(0);
}

READ8_MEMBER( sv603_device::iorq_r )
{
	if (offset >= 0xa0 && offset <= 0xbf)
		return m_expander->excs_r(space, offset);

	return 0xff;
}

WRITE8_MEMBER( sv603_device::iorq_w )
{
	if (offset >= 0xa0 && offset <= 0xbf)
		m_expander->excs_w(space, offset, data);

	if (offset >= 0xe0 && offset <= 0xff)
		m_snd->write(data);
}
