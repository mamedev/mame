// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Control Universal CUBIO 64/80 Channel Digital I/O Module

**********************************************************************/

#include "emu.h"
#include "cubio.h"

#include "machine/6522via.h"
#include "machine/input_merger.h"


namespace {

class cu_cubio_device : public device_t, public device_acorn_bus_interface
{
protected:
	cu_cubio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_acorn_bus_interface(mconfig, *this)
		, m_via(*this, "via%u", 0)
		, m_sw(*this, "SW%u", 1)
	{
	}

	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_device_array<via6522_device, 4> m_via;

private:
	required_ioport_array<3> m_sw;

	void irq_w(int state)
	{
		m_bus->irq_w(state);
	}
};


class cu_cubio_race_device : public cu_cubio_device
{
public:
	cu_cubio_race_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: cu_cubio_device(mconfig, CU_CUBIO_R, tag, owner, clock)
		, m_control(*this, "CTRL%u", 1)
	{
	}

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_ioport_array<4> m_control;
};


//-------------------------------------------------
//  INPUT_PORTS( cubio )
//-------------------------------------------------

INPUT_PORTS_START( cubio )
	PORT_START("SW1")
	PORT_CONFNAME(0x01, 0x00, "Device Select")
	PORT_DIPSETTING(0x00, "VIA")
	PORT_DIPSETTING(0x01, "PIA")
	PORT_START("SW2")
	PORT_CONFNAME(0x0f, 0x00, "Address Select - Block")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x03, "3")
	PORT_DIPSETTING(0x04, "4")
	PORT_DIPSETTING(0x05, "5")
	PORT_DIPSETTING(0x06, "6")
	PORT_DIPSETTING(0x07, "7")
	PORT_DIPSETTING(0x08, "8")
	PORT_DIPSETTING(0x09, "9")
	PORT_DIPSETTING(0x0a, "A")
	PORT_DIPSETTING(0x0b, "B")
	PORT_DIPSETTING(0x0c, "C")
	PORT_DIPSETTING(0x0d, "D")
	PORT_DIPSETTING(0x0e, "E")
	PORT_DIPSETTING(0x0f, "F")
	PORT_START("SW3")
	PORT_CONFNAME(0x0f, 0x00, "Address Select - Page")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x03, "3")
	PORT_DIPSETTING(0x04, "4")
	PORT_DIPSETTING(0x05, "5")
	PORT_DIPSETTING(0x06, "6")
	PORT_DIPSETTING(0x07, "7")
	PORT_DIPSETTING(0x08, "8")
	PORT_DIPSETTING(0x09, "9")
	PORT_DIPSETTING(0x0a, "A")
	PORT_DIPSETTING(0x0b, "B")
	PORT_DIPSETTING(0x0c, "C")
	PORT_DIPSETTING(0x0d, "D")
	PORT_DIPSETTING(0x0e, "E")
	PORT_DIPSETTING(0x0f, "F")
INPUT_PORTS_END

INPUT_PORTS_START( cubio_race )
	PORT_INCLUDE(cubio)

	PORT_START("CTRL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(1) PORT_NAME("P1 North") PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_NAME("P1 East")  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1) PORT_NAME("P1 West")  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1) PORT_NAME("P1 South") PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_PLAYER(1) PORT_NAME("P1 Turbo")
	PORT_START("CTRL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(2) PORT_NAME("P2 North") PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_NAME("P2 East")  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2) PORT_NAME("P2 West")  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2) PORT_NAME("P2 South") PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_PLAYER(2) PORT_NAME("P2 Turbo")
	PORT_START("CTRL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(3) PORT_NAME("P3 North") PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(3) PORT_NAME("P3 East")  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(3) PORT_NAME("P3 West")  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(3) PORT_NAME("P3 South") PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_PLAYER(3) PORT_NAME("P3 Turbo")
	PORT_START("CTRL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(4) PORT_NAME("P4 North") PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(4) PORT_NAME("P4 East")  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(4) PORT_NAME("P4 West")  PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(4) PORT_NAME("P4 South") PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_PLAYER(4) PORT_NAME("P4 Turbo")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor cu_cubio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cubio );
}

ioport_constructor cu_cubio_race_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cubio_race );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cu_cubio_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set(FUNC(cu_cubio_device::irq_w));

	MOS6522(config, m_via[0], DERIVED_CLOCK(1, 1));
	m_via[0]->irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));
	MOS6522(config, m_via[1], DERIVED_CLOCK(1, 1));
	m_via[1]->irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));
	MOS6522(config, m_via[2], DERIVED_CLOCK(1, 1));
	m_via[2]->irq_handler().set("irqs", FUNC(input_merger_device::in_w<2>));
	MOS6522(config, m_via[3], DERIVED_CLOCK(1, 1));
	m_via[3]->irq_handler().set("irqs", FUNC(input_merger_device::in_w<3>));
}

void cu_cubio_race_device::device_add_mconfig(machine_config &config)
{
	cu_cubio_device::device_add_mconfig(config);

	m_via[0]->readpa_handler().set_ioport("CTRL1").rshift(4);
	m_via[0]->readpb_handler().set_ioport("CTRL1").mask(0x0f);
	m_via[1]->readpa_handler().set_ioport("CTRL2").rshift(4);
	m_via[1]->readpb_handler().set_ioport("CTRL2").mask(0x0f);
	m_via[2]->readpa_handler().set_ioport("CTRL3").rshift(4);
	m_via[2]->readpb_handler().set_ioport("CTRL3").mask(0x0f);
	m_via[3]->readpa_handler().set_ioport("CTRL4").rshift(4);
	m_via[3]->readpb_handler().set_ioport("CTRL4").mask(0x0f);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cu_cubio_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0x0000, 0x000f, emu::rw_delegate(*m_via[0], FUNC(via6522_device::read)), emu::rw_delegate(*m_via[0], FUNC(via6522_device::write)));
	space.install_readwrite_handler(0x0010, 0x001f, emu::rw_delegate(*m_via[1], FUNC(via6522_device::read)), emu::rw_delegate(*m_via[1], FUNC(via6522_device::write)));
	space.install_readwrite_handler(0x0020, 0x002f, emu::rw_delegate(*m_via[2], FUNC(via6522_device::read)), emu::rw_delegate(*m_via[2], FUNC(via6522_device::write)));
	space.install_readwrite_handler(0x0030, 0x003f, emu::rw_delegate(*m_via[3], FUNC(via6522_device::read)), emu::rw_delegate(*m_via[3], FUNC(via6522_device::write)));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(CU_CUBIO_R, device_acorn_bus_interface, cu_cubio_race_device, "cu_cubio_race", "Control Universal CUBIO w/ Race Controllers")
