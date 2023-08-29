// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion 3-Fax Modem

    TODO: add other devices
    - 32K RAM (KM62256)
    - R6653-21
    - Flash N28F010-150

**********************************************************************/

#include "emu.h"
#include "3fax.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSION_3FAX_MODEM, psion_3fax_modem_device, "psion_3fax", "Psion 3-Fax Modem")


//-------------------------------------------------
//  ROM( 3fax )
//-------------------------------------------------

ROM_START(3fax)
	ROM_REGION(0x100000, "rom", 0)
	ROM_LOAD("3fax_v2.01.rom", 0x0000, 0x100000, CRC(55c7c95b) SHA1(85d6f7e3cd316be5f49e7dd25755673144fce3d7))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *psion_3fax_modem_device::device_rom_region() const
{
	return ROM_NAME(3fax);
}


void psion_3fax_modem_device::asic4_map(address_map &map)
{
	map(0x400000, 0x4fffff).rom().region("rom", 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void psion_3fax_modem_device::device_add_mconfig(machine_config &config)
{
	PSION_ASIC4(config, m_asic4);
	m_asic4->set_ext_info_byte(0xac6);
	m_asic4->set_addrmap(0, &psion_3fax_modem_device::asic4_map);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psion_3fax_modem_device - constructor
//-------------------------------------------------

psion_3fax_modem_device::psion_3fax_modem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_3FAX_MODEM, tag, owner, clock)
	, device_psion_sibo_interface(mconfig, *this)
	, m_asic4(*this, "asic4")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_3fax_modem_device::device_start()
{
}
