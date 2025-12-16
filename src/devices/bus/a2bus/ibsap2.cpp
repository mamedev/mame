// license:BSD-3-Clause
// copyright-holders:AJR
/*********************************************************************

    IBS Computertechnik AP 2 Serial Interface

    This V.24/RS-232-C interface card, released around 1981, is
    nearly equivalent to Apple's Serial Interface Card in firmware
    features and switch settings, though it uses a 6551 ACIA like
    the later Super Serial Card.

    The firmware only recognizes two commands in input mode: ESC L
    for lowercase mode and ESC U for uppercase mode.

**********************************************************************/

#include "emu.h"
#include "ibsap2.h"

#include "bus/rs232/rs232.h"
#include "machine/mos6551.h"

namespace {

class ibsap2_device : public device_t, public device_a2bus_card_interface
{
public:
	// device type constructor
	ibsap2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_a2bus_card_interface overrides
	virtual u8 read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;
	virtual u8 read_cnxx(u8 offset) override;
	virtual u8 read_c800(u16 offset) override;
	virtual bool take_c800() const override;
	virtual void reset_from_bus() override;

private:
	// miscellaneous handlers
	void acia_irq_w(int state);

	// object finders
	required_device<mos6551_device> m_acia;
	required_region_ptr<u8> m_eprom;
	required_ioport m_switches;
};

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

ibsap2_device::ibsap2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, A2BUS_IBSAP2, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_acia(*this, "acia")
	, m_eprom(*this, "eprom")
	, m_switches(*this, "SWITCHES")
{
}

void ibsap2_device::device_start()
{
}

u8 ibsap2_device::read_c0nx(u8 offset)
{
	u8 result = 0xff;
	if (BIT(offset, 3))
		result &= m_acia->read(offset & 3);
	if (!BIT(offset, 2))
		result &= m_switches->read();
	return result;
}

void ibsap2_device::write_c0nx(u8 offset, u8 data)
{
	if (offset >= 0x0c)
		m_acia->write(offset & 3, data);
	else
		logerror("Writing %02X to register C0n%X\n", data, offset);
}

u8 ibsap2_device::read_cnxx(u8 offset)
{
	return m_eprom[0x700 + offset];
}

u8 ibsap2_device::read_c800(u16 offset)
{
	return m_eprom[offset];
}

bool ibsap2_device::take_c800() const
{
	return true;
}

void ibsap2_device::reset_from_bus()
{
	m_acia->reset();
}

void ibsap2_device::acia_irq_w(int state)
{
	if (state == ASSERT_LINE)
		raise_slot_irq();
	else
		lower_slot_irq();
}

static INPUT_PORTS_START(ibsap2)
	PORT_START("SWITCHES")
	PORT_DIPNAME(0x07, 0x06, "Baud Rate") PORT_DIPLOCATION("S1:1,2,3")
	PORT_DIPSETTING(0x00, "109.92")
	PORT_DIPSETTING(0x01, "134.58")
	PORT_DIPSETTING(0x02, "300")
	PORT_DIPSETTING(0x03, "1200")
	PORT_DIPSETTING(0x04, "2400")
	PORT_DIPSETTING(0x05, "4800")
	PORT_DIPSETTING(0x06, "9600")
	PORT_DIPSETTING(0x07, "19200")
	PORT_DIPNAME(0x08, 0x08, "Delay After CR") PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x30, 0x00, "Line Width") PORT_DIPLOCATION("S1:5,6")
	PORT_DIPSETTING(0x00, "40")
	PORT_DIPSETTING(0x10, "72")
	PORT_DIPSETTING(0x20, "80")
	PORT_DIPSETTING(0x30, "132")
	PORT_DIPNAME(0x40, 0x00, "Output LF After CR") PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "Lowercase Characters") PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING(0x80, "Invert")
	PORT_DIPSETTING(0x00, "Convert to Uppercase")
INPUT_PORTS_END

ioport_constructor ibsap2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ibsap2);
}

void ibsap2_device::device_add_mconfig(machine_config &config)
{
	MOS6551(config, m_acia, 0);
	m_acia->set_xtal(1.8432_MHz_XTAL);
	m_acia->irq_handler().set(FUNC(ibsap2_device::acia_irq_w));
	m_acia->rts_handler().set("v24", FUNC(rs232_port_device::write_rts));
	m_acia->txd_handler().set("v24", FUNC(rs232_port_device::write_txd));
	m_acia->dtr_handler().set("v24", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &v24(RS232_PORT(config, "v24", default_rs232_devices, nullptr));
	v24.cts_handler().set(m_acia, FUNC(mos6551_device::write_cts));
	v24.rxd_handler().set(m_acia, FUNC(mos6551_device::write_rxd));
	v24.dcd_handler().set(m_acia, FUNC(mos6551_device::write_dcd));
	v24.dsr_handler().set(m_acia, FUNC(mos6551_device::write_dsr));
}

ROM_START(ibsap2)
	ROM_REGION(0x800, "eprom", 0)
	ROM_LOAD("ibs ap2 v24 v2.1 19821119.bin", 0x000, 0x800, CRC(febb85c7) SHA1(f40e7e50521636b441f86794bd19b77ecf9a67b4))
ROM_END

const tiny_rom_entry *ibsap2_device::device_rom_region() const
{
	return ROM_NAME(ibsap2);
}

} // anonymous namespace

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_IBSAP2, device_a2bus_card_interface, ibsap2_device, "ibsap2", "IBS Computertechnik AP 2 Serial Interface")
