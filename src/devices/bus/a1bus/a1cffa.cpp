// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a1cffa.c

    Rich Dreher's Compact Flash for Apple I

*********************************************************************/

#include "emu.h"
#include "a1cffa.h"

#include "bus/ata/ataintf.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define CFFA_ROM_REGION "cffa_rom"
#define CFFA_ATA_TAG    "cffa_ata"

ROM_START( cffa )
	ROM_REGION(0x2000, CFFA_ROM_REGION, 0)
	ROM_LOAD ("cffaromv1.1.bin", 0x0000, 0x1fe0, CRC(bf6b55ad) SHA1(6a290be18485a06f243a3561c4e01be5aafa4bfe) )
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a1bus_cffa_device:
		public device_t,
		public device_a1bus_card_interface
{
public:
	// construction/destruction
	a1bus_cffa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t cffa_r(offs_t offset);
	void cffa_w(offs_t offset, uint8_t data);

protected:
	a1bus_cffa_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	required_device<ata_interface_device> m_ata;

private:
	required_region_ptr<uint8_t> m_rom;
	uint16_t m_lastdata;
	bool m_writeprotect;
};

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
	install_device(0xafe0, 0xafff, read8sm_delegate(*this, FUNC(a1bus_cffa_device::cffa_r)), write8sm_delegate(*this, FUNC(a1bus_cffa_device::cffa_w)));
	install_bank(0x9000, 0xafdf, &m_rom[0]);

	save_item(NAME(m_lastdata));
	save_item(NAME(m_writeprotect));
}

void a1bus_cffa_device::device_reset()
{
	m_writeprotect = false;
	m_lastdata = 0;
}

uint8_t a1bus_cffa_device::cffa_r(offs_t offset)
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
			m_lastdata = m_ata->cs0_r((offset & 0xf) - 8, 0xff);
			return m_lastdata & 0x00ff;

		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			return m_ata->cs0_r((offset & 0xf) - 8, 0xff);
	}

	return 0xff;
}

void a1bus_cffa_device::cffa_w(offs_t offset, uint8_t data)
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
			m_ata->cs0_w((offset & 0xf) - 8, data, 0xff);
			break;

		case 0x9:
		case 0xa:
		case 0xb:
		case 0xc:
		case 0xd:
		case 0xe:
		case 0xf:
			m_ata->cs0_w((offset & 0xf) - 8, data, 0xff);
			break;

	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A1BUS_CFFA, device_a1bus_card_interface, a1bus_cffa_device, "cffa1", "CFFA Compact Flash for Apple I")
