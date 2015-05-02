// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a1cffa.c

    Rich Dreher's Compact Flash for Apple I

*********************************************************************/

#include "a1cffa.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#define CFFA_ROM_REGION "cffa_rom"
#define CFFA_ATA_TAG    "cffa_ata"

const device_type A1BUS_CFFA = &device_creator<a1bus_cffa_device>;

MACHINE_CONFIG_FRAGMENT( cffa )
	MCFG_ATA_INTERFACE_ADD(CFFA_ATA_TAG, ata_devices, "hdd", NULL, false)
MACHINE_CONFIG_END

ROM_START( cffa )
	ROM_REGION(0x2000, CFFA_ROM_REGION, 0)
	ROM_LOAD ("cffaromv1.1.bin", 0x0000, 0x1fe0, CRC(bf6b55ad) SHA1(6a290be18485a06f243a3561c4e01be5aafa4bfe) )
ROM_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a1bus_cffa_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cffa );
}

const rom_entry *a1bus_cffa_device::device_rom_region() const
{
	return ROM_NAME( cffa );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a1bus_cffa_device::a1bus_cffa_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, A1BUS_CFFA, "CFFA Compact Flash for Apple I", tag, owner, clock, "cffa1", __FILE__),
		device_a1bus_card_interface(mconfig, *this),
		m_ata(*this, CFFA_ATA_TAG)
{
}

a1bus_cffa_device::a1bus_cffa_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_a1bus_card_interface(mconfig, *this),
		m_ata(*this, CFFA_ATA_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a1bus_cffa_device::device_start()
{
	set_a1bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(CFFA_ROM_REGION).c_str())->base();

	install_device(0xafe0, 0xafff, read8_delegate(FUNC(a1bus_cffa_device::cffa_r), this), write8_delegate(FUNC(a1bus_cffa_device::cffa_w), this));
	install_bank(0x9000, 0xafdf, 0, 0, (char *)"bank_cffa1", m_rom);

	save_item(NAME(m_lastdata));
	save_item(NAME(m_writeprotect));
}

void a1bus_cffa_device::device_reset()
{
	m_writeprotect = false;
	m_lastdata = 0;
}

READ8_MEMBER(a1bus_cffa_device::cffa_r)
{
	switch (offset & 0xf)
	{
		case 0x0:
			return m_lastdata>>8;

		case 0x3:
			m_writeprotect = false;
			break;

		case 0x4:
			m_writeprotect = true;
			break;

		case 0x8:
			m_lastdata = m_ata->read_cs0(space, (offset & 0xf) - 8, 0xff);
			return m_lastdata & 0x00ff;

		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			return m_ata->read_cs0(space, (offset & 0xf) - 8, 0xff);
	}

	return 0xff;
}

WRITE8_MEMBER(a1bus_cffa_device::cffa_w)
{
	switch (offset & 0xf)
	{
		case 0x0:
			m_lastdata &= 0x00ff;
			m_lastdata |= data<<8;
			break;

		case 0x3:
			m_writeprotect = false;
			break;

		case 0x4:
			m_writeprotect = true;
			break;


		case 0x8:
			m_ata->write_cs0(space, (offset & 0xf) - 8, data, 0xff);
			break;

		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			m_ata->write_cs0(space, (offset & 0xf) - 8, data, 0xff);
			break;

	}
}
