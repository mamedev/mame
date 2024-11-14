// license:BSD-3-Clause
// copyright-holders:Kelvin Sherlock
/*********************************************************************

    lancegs.cpp

    Apple II LANceGS Card

    Kelvin Sherlock with assistance from Geoff Weiss

    SMSC LAN 91c96 and 24c04 EEPROM


    Bit 1 of C0nF switches between EEPROM (1) and Ethernet (0, default).
    This bit is write only.

    In EEPROM Mode:

    C0n4 - Clock Low
    C0n5 - Clock High
    C0n6 - Data Low
    C0n7 - Data High
    C0n8 - Read Data
    C0nF - toggle EEPROM/Ethernet


    Known EEPROM locations. All strings high ascii, 0-terminated.

    0x00: Identification string ("LANceGS Ethernet Card (c) 2000 Joachim Lange")
    0x30: Revision string
    0x40: Revision Code (1 byte)
    0x48: Production date, yy m d (4 bytes)
    0x4c: Service date, yy m d (4 bytes)
    0x50: Serial Number (2 bytes)
    0x60: MAC Address (6 bytes)
    0x66: Destination Mac Address (6 bytes) *
    0x80: IP Address (4 bytes) *
    0x84: Destination IP Address (4 bytes) *
    0x88: Netmask (4 bytes) *
    0x8c: Gateway (4 bytes) *

    * configurable with included software.


*********************************************************************/

#include "emu.h"
#include "lancegs.h"

#include "machine/smc91c9x.h"
#include "machine/i2cmem.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_lancegs_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_lancegs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_lancegs_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

private:
	required_device<smc91c96_device> m_netinf;
	required_device<i2c_24c04_device> m_i2cmem;
	bool m_shadow;

	void netinf_irq_w(int state);
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_lancegs_device::a2bus_lancegs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_lancegs_device(mconfig, A2BUS_LANCEGS, tag, owner, clock)
{
}

a2bus_lancegs_device::a2bus_lancegs_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_netinf(*this, "smc91c96"),
	m_i2cmem(*this, "i2cmem"),
	m_shadow(false)
{
}

void a2bus_lancegs_device::device_add_mconfig(machine_config &config)
{
	SMC91C96(config, m_netinf, 20_MHz_XTAL); // Datasheet fig 12.26, pg 122.
	I2C_24C04(config, m_i2cmem, 0).set_address(0x80).set_e0(1);

	m_netinf->irq_handler().set(FUNC(a2bus_lancegs_device::netinf_irq_w));
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_lancegs_device::device_start()
{
	save_item(NAME(m_shadow));
}

void a2bus_lancegs_device::device_reset()
{
	m_shadow = false;
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_lancegs_device::read_c0nx(uint8_t offset)
{
	if (m_shadow) {
		switch(offset) {
			case 0x08: /* read data */
				return 0xfe | m_i2cmem->read_sda();
				break;
			case 0x0f:
				return 0x33;
				break;
			default:
				return 0xff;
				break;
		}
	} else {
		const uint16_t mask = offset & 0x01 ? 0xff00: 0x00ff;
		const uint16_t value = m_netinf->read(offset >> 1, mask);
		return offset & 0x01 ? value >> 8 : value;
	}
}

/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_lancegs_device::write_c0nx(uint8_t offset, uint8_t data)
{
	if (offset == 0x0f) {
		m_shadow = data & 0x01;
		return;
	}

	if (m_shadow) {
		switch(offset) {
			case 0x04: /* set clock low */
				m_i2cmem->write_scl(0);
				break;
			case 0x05: /* set clock high */
				m_i2cmem->write_scl(1);
				break;
			case 0x06: /* set data low */
				m_i2cmem->write_sda(0);
				break;
			case 0x07: /* set data high */
				m_i2cmem->write_sda(1);
				break;
			default:
				break;
		}
	} else {
		const uint16_t mask = offset & 0x01 ? 0xff00: 0x00ff;
		const uint16_t value = offset & 0x01 ? data << 8 : data;
		m_netinf->write(offset >> 1, value, mask);
	}
}

void a2bus_lancegs_device::netinf_irq_w(int state)
{
	if (state) {
		raise_slot_irq();
	} else {
		lower_slot_irq();
	}
}


ROM_START(lancegs)
	ROM_REGION(0x0200, "i2cmem", 0)
	ROM_LOAD("lancegs.nv", 0x0000, 0x0200, NO_DUMP)
ROM_END

const tiny_rom_entry *a2bus_lancegs_device::device_rom_region() const
{
	return ROM_NAME(lancegs);
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_LANCEGS, device_a2bus_card_interface, a2bus_lancegs_device, "a2lancegs", "///SHH Systeme LANceGS")
