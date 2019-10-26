// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a1cffa.c

    Rich Dreher's Compact Flash for Apple I

*********************************************************************/

#include "emu.h"
#include "a1cffa.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#define CFFA_ROM_REGION "cffa_rom"
#define CFFA_ATA_TAG    "cffa_ata"

DEFINE_DEVICE_TYPE(A1BUS_CFFA, a1bus_cffa_device, "cffa1", "CFFA Compact Flash for Apple I")

ROM_START( cffa )
	ROM_REGION(0x2000, CFFA_ROM_REGION, 0)
	ROM_LOAD ("cffaromv1.1.bin", 0x0000, 0x1fe0, CRC(bf6b55ad) SHA1(6a290be18485a06f243a3561c4e01be5aafa4bfe) )
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a1bus_cffa_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
}

const tiny_rom_entry *a1bus_cffa_device::device_rom_region() const
{
	return ROM_NAME( cffa );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a1bus_cffa_device::a1bus_cffa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: a1bus_cffa_device(mconfig, A1BUS_CFFA, tag, owner, clock)
{
}

a1bus_cffa_device::a1bus_cffa_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a1bus_card_interface(mconfig, *this)
	, m_ata(*this, CFFA_ATA_TAG)
	, m_rom(*this, CFFA_ROM_REGION)
	, m_lastdata(0)
	, m_writeprotect(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a1bus_cffa_device::device_start()
{
	install_device(0xafe0, 0xafff, read8_delegate(*this, FUNC(a1bus_cffa_device::cffa_r)), write8_delegate(*this, FUNC(a1bus_cffa_device::cffa_w)));
	install_bank(0x9000, 0xafdf, "bank_cffa1", &m_rom[0]);

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
			m_lastdata = m_ata->read_cs0((offset & 0xf) - 8, 0xff);
			return m_lastdata & 0x00ff;

		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			return m_ata->read_cs0((offset & 0xf) - 8, 0xff);
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
			m_ata->write_cs0((offset & 0xf) - 8, data, 0xff);
			break;

		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			m_ata->write_cs0((offset & 0xf) - 8, data, 0xff);
			break;

	}
}
