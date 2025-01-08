// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 FDC

****************************************************************************/

#include "emu.h"
#include "fdc.h"

#include "machine/upd765.h"
#include "imagedev/floppy.h"

namespace {

//**************************************************************************
//  RC2014 Floppy Disk Controller FDC9266
//  Module author: Dr. Scott M. Baker
//**************************************************************************

class rc2014_fdc9266_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	rc2014_fdc9266_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
private:
	void control_w(offs_t offset, uint8_t data);

	required_ioport m_addr;
	required_ioport_array<4> m_jp;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
};

rc2014_fdc9266_device::rc2014_fdc9266_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_FDC9266, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_addr(*this, "SV1")
	, m_jp(*this, "JP%u", 1U)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0U)
{
}

void rc2014_fdc9266_device::device_start()
{
}

void rc2014_fdc9266_device::device_reset()
{
	uint8_t base = m_addr->read() << 5; // SV1
	// A15-A8 and A1 not connected
	m_bus->installer(AS_IO)->install_read_handler(base+0x10, base+0x10, 0, 0xff02, 0, read8smo_delegate(m_fdc, FUNC(upd765a_device::msr_r)));
	m_bus->installer(AS_IO)->install_readwrite_handler(base+0x11, base+0x11, 0, 0xff02, 0, read8smo_delegate(m_fdc, FUNC(upd765a_device::fifo_r)), write8smo_delegate(m_fdc, FUNC(upd765a_device::fifo_w)));
	m_bus->installer(AS_IO)->install_write_handler(base+0x18, base+0x18, 0, 0xff02, 0, write8sm_delegate(*this, FUNC(rc2014_fdc9266_device::control_w)));
	// TODO: Use jumpers
}

void rc2014_fdc9266_device::control_w(offs_t, uint8_t data)
{
	// D0 - TC
	// D1 - MOTEA
	// D2 - MOTEB
	// D3 - P2
	// D4 - P1
	// D5 - P0
	// D6 - DENSEL
	// D7 - FDC_RST (trough inverter)
	m_fdc->tc_w(BIT(data,0)? true : false);
	if (m_floppy[0]->get_device())
		m_floppy[0]->get_device()->mon_w(!BIT(data,1));
	if (m_floppy[1]->get_device())
		m_floppy[1]->get_device()->mon_w(!BIT(data,2));

	m_fdc->set_rate(BIT(data,6) ? 500000 : 250000);
	m_fdc->reset_w(!BIT(data,7) ? ASSERT_LINE : CLEAR_LINE);
}

static void rc2014_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

void rc2014_fdc9266_device::device_add_mconfig(machine_config &config)
{
	// FDC9266
	UPD765A(config, m_fdc, XTAL(8'000'000), true, true);

	// floppy drives
	FLOPPY_CONNECTOR(config, m_floppy[0], rc2014_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], rc2014_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);
}

static INPUT_PORTS_START( rc2014_fdc9266_jumpers )
	PORT_START("SV1")
	PORT_CONFNAME( 0x7, 0x2, "Base Address" )
	PORT_CONFSETTING( 0x0, "0x00" )
	PORT_CONFSETTING( 0x1, "0x20" )
	PORT_CONFSETTING( 0x2, "0x40" )
	PORT_CONFSETTING( 0x3, "0x60" )
	PORT_CONFSETTING( 0x4, "0x80" )
	PORT_CONFSETTING( 0x5, "0xa0" )
	PORT_CONFSETTING( 0x6, "0xc0" )
	PORT_CONFSETTING( 0x7, "0xe0" )
	PORT_START("JP1")
	PORT_CONFNAME( 0x1, 0x1, "Two Side" )
	PORT_CONFSETTING( 0x0, "GND" )
	PORT_CONFSETTING( 0x1, "None" )
	PORT_START("JP2")
	PORT_CONFNAME( 0x1, 0x0, "/FAULT" )
	PORT_CONFSETTING( 0x0, "None" )
	PORT_CONFSETTING( 0x1, "+5V" )
	PORT_START("JP3")
	PORT_CONFNAME( 0x1, 0x1, "MINI" )
	PORT_CONFSETTING( 0x0, "GND" )
	PORT_CONFSETTING( 0x1, "+5V" )
	PORT_START("JP4")
	PORT_CONFNAME( 0x1, 0x4, "RDY" )
	PORT_CONFSETTING( 0x0, "GND" )
	PORT_CONFSETTING( 0x4, "DC/RDY" )
INPUT_PORTS_END

ioport_constructor rc2014_fdc9266_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_fdc9266_jumpers );
}

//**************************************************************************
//  RC2014 Floppy Disk Controller WD37C65
//  Module author: Dr. Scott M. Baker
//**************************************************************************

class rc2014_wd37c65_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	rc2014_wd37c65_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// DACK confirmation is same as pulsing TC
	uint8_t dack_r(address_space &space, offs_t) { m_fdc->tc_w(true);  m_fdc->tc_w(false); return space.unmap(); }
private:
	required_ioport m_addr;
	required_ioport_array<2> m_jp;
	required_device<wd37c65c_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
};

rc2014_wd37c65_device::rc2014_wd37c65_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_WD37C65, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_addr(*this, "SV1")
	, m_jp(*this, "JP%u", 1U)
	, m_fdc(*this, "fdc")
	, m_floppy(*this, "fdc:%u", 0U)
{
}

void rc2014_wd37c65_device::device_start()
{
}

void rc2014_wd37c65_device::device_reset()
{
	uint8_t base = m_addr->read() << 5; // SV1
	// A15-A8 and A1 not connected
	m_bus->installer(AS_IO)->install_read_handler(base+0x10, base+0x10, 0, 0xff02, 0, read8smo_delegate(m_fdc, FUNC(wd37c65c_device::msr_r)));
	m_bus->installer(AS_IO)->install_readwrite_handler(base+0x11, base+0x11, 0, 0xff02, 0, read8smo_delegate(m_fdc, FUNC(wd37c65c_device::fifo_r)), write8smo_delegate(m_fdc, FUNC(wd37c65c_device::fifo_w)));

	// A15-A8 and A0 and A1 not connected
	m_bus->installer(AS_IO)->install_write_handler(base+0x08, base+0x08, 0, 0xff06, 0, write8smo_delegate(m_fdc, FUNC(wd37c65c_device::ccr_w)));
	m_bus->installer(AS_IO)->install_write_handler(base+0x18, base+0x18, 0, 0xff06, 0, write8smo_delegate(m_fdc, FUNC(wd37c65c_device::dor_w)));
	m_bus->installer(AS_IO)->install_read_handler(base+0x18, base+0x18, 0, 0xff06, 0, read8m_delegate(*this, FUNC(rc2014_wd37c65_device::dack_r)));
	// TODO: Use jumpers
}

void rc2014_wd37c65_device::device_add_mconfig(machine_config &config)
{
	WD37C65C(config, m_fdc, 16_MHz_XTAL);

	// floppy drives
	FLOPPY_CONNECTOR(config, m_floppy[0], rc2014_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], rc2014_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);
}

static INPUT_PORTS_START( rc2014_wd37c65_jumpers )
	PORT_START("SV1")
	PORT_CONFNAME( 0x7, 0x2, "Base Address" )
	PORT_CONFSETTING( 0x0, "0x00" )
	PORT_CONFSETTING( 0x1, "0x20" )
	PORT_CONFSETTING( 0x2, "0x40" )
	PORT_CONFSETTING( 0x3, "0x60" )
	PORT_CONFSETTING( 0x4, "0x80" )
	PORT_CONFSETTING( 0x5, "0xa0" )
	PORT_CONFSETTING( 0x6, "0xc0" )
	PORT_CONFSETTING( 0x7, "0xe0" )
	PORT_START("JP1")
	PORT_CONFNAME( 0x1, 0x1, "DACK" )
	PORT_CONFSETTING( 0x0, "Share with DOR" )
	PORT_CONFSETTING( 0x1, "Use offset 0" )
	PORT_START("JP2")
	PORT_CONFNAME( 0x1, 0x0, "TC" )
	PORT_CONFSETTING( 0x0, "Port offset 4" )
	PORT_CONFSETTING( 0x1, "+5V" )
INPUT_PORTS_END

ioport_constructor rc2014_wd37c65_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rc2014_wd37c65_jumpers );
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_FDC9266, device_rc2014_card_interface, rc2014_fdc9266_device, "rc2014_fdc9266", "RC2014 Floppy Disk Controller FDC9266")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_WD37C65, device_rc2014_card_interface, rc2014_wd37c65_device, "rc2014_wd37c65", "RC2014 Floppy Disk Controller WD37C65")
