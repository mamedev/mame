// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    prodosromdrive.cpp

    Implementation of the ProDOS ROM Drive card

    This is a hobbyist board which provides a minimal SmartPort API
    interface to boot from a 1 MiB disk image stored in a 27C080 or
    compatible EPROM.

    Board and firmware by Terence J. Boldt
    http://apple2.ca/
    https://github.com/tjboldt/ProDOS-ROM-Drive

    The firmware requires a IIe or better, and some of the games
    included require a 65C02 (enhanced IIe).

*********************************************************************/

#include "emu.h"
#include "prodosromdrive.h"

namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_pdromdrive_device : public device_t,
							public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_pdromdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_pdromdrive_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;

private:
	required_region_ptr<u8> m_rom;

	u16 m_latch;
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_pdromdrive_device::a2bus_pdromdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_pdromdrive_device(mconfig, A2BUS_PRODOSROMDRIVE, tag, owner, clock)
{
}

a2bus_pdromdrive_device::a2bus_pdromdrive_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_rom(*this, "romdrive")
{
}

void a2bus_pdromdrive_device::device_start()
{
	save_item(NAME(m_latch));
}

ROM_START( pdromdrive )
	ROM_REGION(0x100000, "romdrive", 0)
	ROM_LOAD( "gameswithfirmware.bin", 0x000000, 0x100000, CRC(a1efd23b) SHA1(f0733b7a68126adc5247a1008de2ea4f65ad645a) )
ROM_END

const tiny_rom_entry *a2bus_pdromdrive_device::device_rom_region() const
{
	return ROM_NAME(pdromdrive);
}

uint8_t a2bus_pdromdrive_device::read_c0nx(uint8_t offset)
{
	return m_rom[(m_latch << 4) | (offset & 0xf)];
}

void a2bus_pdromdrive_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_latch &= 0xff00;
			m_latch |= data;
			break;

		case 1:
			m_latch &= 0x00ff;
			m_latch |= (data << 8);
			break;
	}
}

uint8_t a2bus_pdromdrive_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset + 0x300];
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_PRODOSROMDRIVE, device_a2bus_card_interface, a2bus_pdromdrive_device, "a2pdromdr", "ProDOS ROM Drive")
