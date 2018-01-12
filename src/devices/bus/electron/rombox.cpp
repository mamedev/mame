// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Slogger Rombox

    The Rombox offers the following facilities
    - 8 Sideways ROM / RAM slots

**********************************************************************/


#include "emu.h"
#include "rombox.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_ROMBOX, electron_rombox_device, "electron_rombox", "Slogger Rombox")


//-------------------------------------------------
//  MACHINE_DRIVER( rombox )
//-------------------------------------------------

static INPUT_PORTS_START( rombox )
	PORT_START("OPTION")
	PORT_CONFNAME(0x01, 0x00, "A1") // not implemented
	PORT_CONFSETTING(0x00, "ROM")
	PORT_CONFSETTING(0x01, "RAM")
	PORT_CONFNAME(0x02, 0x00, "A2") // not implemented
	PORT_CONFSETTING(0x00, "8K or 16K")
	PORT_CONFSETTING(0x01, "4K")
	PORT_CONFNAME(0x04, 0x00, "B")
	PORT_CONFSETTING(0x00, "ROM 0,1,2,3")
	PORT_CONFSETTING(0x02, "ROM 12,13,14,15")
INPUT_PORTS_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

ioport_constructor electron_rombox_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rombox );
}

MACHINE_CONFIG_MEMBER( electron_rombox_device::device_add_mconfig )
	/* rom sockets */
	MCFG_GENERIC_SOCKET_ADD("rom1", generic_plain_slot, "electron_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(electron_rombox_device, rom1_load)
	MCFG_GENERIC_SOCKET_ADD("rom2", generic_plain_slot, "electron_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(electron_rombox_device, rom2_load)
	MCFG_GENERIC_SOCKET_ADD("rom3", generic_plain_slot, "electron_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(electron_rombox_device, rom3_load)
	MCFG_GENERIC_SOCKET_ADD("rom4", generic_plain_slot, "electron_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(electron_rombox_device, rom4_load)
	MCFG_GENERIC_SOCKET_ADD("rom5", generic_plain_slot, "electron_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(electron_rombox_device, rom5_load)
	MCFG_GENERIC_SOCKET_ADD("rom6", generic_plain_slot, "electron_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(electron_rombox_device, rom6_load)
	MCFG_GENERIC_SOCKET_ADD("rom7", generic_plain_slot, "electron_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(electron_rombox_device, rom7_load)
	MCFG_GENERIC_SOCKET_ADD("rom8", generic_plain_slot, "electron_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(electron_rombox_device, rom8_load)

	/* pass-through */
	MCFG_ELECTRON_PASSTHRU_EXPANSION_SLOT_ADD(nullptr)
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_rombox_device - constructor
//-------------------------------------------------

electron_rombox_device::electron_rombox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_ROMBOX, tag, owner, clock),
		device_electron_expansion_interface(mconfig, *this),
	m_rom(*this, "rom%u", 1),
	m_option(*this, "OPTION")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_rombox_device::device_start()
{
	m_slot = dynamic_cast<electron_expansion_slot_device *>(owner());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_rombox_device::device_reset()
{
	std::string region_tag;
	memory_region *tmp_reg;

	int rom_base = (m_option->read() & 0x04) ? 0 : 12;
	for (int i = 0; i < 4; i++)
	{
		if (m_rom[i] && (tmp_reg = memregion(region_tag.assign(m_rom[i]->tag()).append(GENERIC_ROM_REGION_TAG).c_str())))
		{
			machine().root_device().membank("bank2")->configure_entry(4 + i, tmp_reg->base());
		}

		if (m_rom[i + 4] && (tmp_reg = memregion(region_tag.assign(m_rom[i + 4]->tag()).append(GENERIC_ROM_REGION_TAG).c_str())))
		{
			machine().root_device().membank("bank2")->configure_entry(rom_base + i, tmp_reg->base());
		}
	}
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

image_init_result electron_rombox_device::load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");

	// socket accepts 8K and 16K ROM only
	if (size != 0x2000 && size != 0x4000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid size: Only 8K/16K is supported");
		return image_init_result::FAIL;
	}

	slot->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}
