// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    booti.cpp

    Implementation of the BOOTI card

    The BOOTI is an Apple II interface to the CH376 USB module.
    The CH376 is intended for use with small microcontrollers (or,
    you know, the 6502) to give them access to FAT-formatted
    flash drives.  See ch376.cpp for details.

    C0n0: read/write data to CH376
    C0n1: read status/write command to CH376
    C0n4: $C800 ROM bank (0-3)

*********************************************************************/

#include "emu.h"
#include "booti.h"

#include "machine/at28c64b.h"
#include "machine/ch376.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

ROM_START( booti )
	ROM_REGION(0x2000, "flash", 0)
	ROM_LOAD( "bootifw09l.bin", 0x000000, 0x002000, CRC(be3f21ff) SHA1(f505ad4685cd44e4cce5b8d6d27b9c4fea159f11) )
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_booti_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_booti_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_booti_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

private:
	required_device<at28c64b_device> m_flash;
	required_device<ch376_device> m_ch376;

	int m_rombank;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_booti_device::device_add_mconfig(machine_config &config)
{
	AT28C64B(config, "flash", 0);

	CH376(config, "ch376");
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_booti_device::device_rom_region() const
{
	return ROM_NAME( booti );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_booti_device::a2bus_booti_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_flash(*this, "flash"),
	m_ch376(*this, "ch376"),
	m_rombank(0)
{
}

a2bus_booti_device::a2bus_booti_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_booti_device(mconfig, A2BUS_BOOTI, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_booti_device::device_start()
{
	save_item(NAME(m_rombank));
}

void a2bus_booti_device::device_reset()
{
	m_rombank = 0;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_booti_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
		case 1:
			return m_ch376->read(offset);

		case 4:
			return m_rombank / 0x800;
	}
	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_booti_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
		case 1:
			m_ch376->write(offset, data);
			break;

		case 4:
			m_rombank = 0x800 * (data & 3);
			break;

		default:
			printf("Write %02x to c0n%x (%s)\n", data, offset, machine().describe_context().c_str());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_booti_device::read_cnxx(uint8_t offset)
{
	// slot image at 0
	return m_flash->read(offset);
}

void a2bus_booti_device::write_cnxx(uint8_t offset, uint8_t data)
{
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_booti_device::read_c800(uint16_t offset)
{
	return m_flash->read(offset + m_rombank);
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_booti_device::write_c800(uint16_t offset, uint8_t data)
{
	// the card appears not to pass writes to $CFFF to the EEPROM,
	// otherwise there's a false read of the next opcode after a $CFFF write and we crash.
	if (offset < 0x7ff)
	{
		m_flash->write(offset + m_rombank, data);
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_BOOTI, device_a2bus_card_interface, a2bus_booti_device, "a2booti", "Booti Card")
