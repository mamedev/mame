// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Clock Module

****************************************************************************/

#include "emu.h"
#include "clock.h"
#include "machine/clock.h"

namespace {

//**************************************************************************
//  RC2014 Clock module
//  Module author: Spencer Owen
//**************************************************************************

class single_clock_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	single_clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void clk_w(int state) { m_bus->clk_w(state); }
};

single_clock_device::single_clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_SINGLE_CLOCK, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
{
}

void single_clock_device::device_start()
{
	m_bus->set_bus_clock(XTAL(7'372'800));
}

void single_clock_device::device_add_mconfig(machine_config &config)
{
	clock_device &clock(CLOCK(config, "clock", XTAL(7'372'800)));
	clock.signal_handler().set(FUNC(single_clock_device::clk_w));
}

//**************************************************************************
//  RC2014 Dual Clock module
//  Module author: Spencer Owen
//**************************************************************************

//**************************************************************************
//  dual_clock_base
//**************************************************************************

class dual_clock_base : public device_t
{
protected:
	// construction/destruction
	dual_clock_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void clk_w(int state) = 0;
	virtual void clk2_w(int state) = 0;

	// base-class members
	required_device<clock_device> m_clock_1;
	required_device<clock_device> m_clock_2;
	required_ioport m_clk_sel_1;
	required_ioport m_clk_sel_2;
};

dual_clock_base::dual_clock_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_clock_1(*this, "clock1")
	, m_clock_2(*this, "clock2")
	, m_clk_sel_1(*this, "CLOCK1")
	, m_clk_sel_2(*this, "CLOCK2")
{
}

void dual_clock_base::device_start()
{
}

static constexpr u32 clock_mapping[] =
{
	7'372'800/1,
	7'372'800/2,
	7'372'800/3,
	7'372'800/6,
	7'372'800/8,
	7'372'800/12,
	7'372'800/24,
	10'000,
	0, // TODO: Support manual clocking (not possible for now)
	0  // TODO: Support external clocking
};

void dual_clock_base::device_reset()
{
	u32 clk1 = clock_mapping[m_clk_sel_1->read()];
	m_clock_1->set_clock(clk1);

	u32 clk2 = clock_mapping[m_clk_sel_2->read()];
	m_clock_2->set_clock(clk2);

	notify_clock_changed();
}

static INPUT_PORTS_START( dual_clock_jumpers )
	PORT_START("CLOCK1")
	PORT_CONFNAME( 0xf, 0x0, "Clock 1" )
	PORT_CONFSETTING( 0x0, "7.3728 MHz" )
	PORT_CONFSETTING( 0x1, "3.6864 MHz" )
	PORT_CONFSETTING( 0x2, "2.4576 MHz" )
	PORT_CONFSETTING( 0x3, "1.2288 MHz" )
	PORT_CONFSETTING( 0x4, "0.9216 MHz" )
	PORT_CONFSETTING( 0x5, "0.6144 MHz" )
	PORT_CONFSETTING( 0x6, "0.3072 MHz" )
	PORT_CONFSETTING( 0x7, "Slow" )
	PORT_CONFSETTING( 0x8, "Manual" )
	PORT_CONFSETTING( 0x9, "External" )
	PORT_START("CLOCK2")
	PORT_CONFNAME( 0xf, 0x0, "Clock 2" )
	PORT_CONFSETTING( 0x0, "7.3728 MHz" )
	PORT_CONFSETTING( 0x1, "3.6864 MHz" )
	PORT_CONFSETTING( 0x2, "2.4576 MHz" )
	PORT_CONFSETTING( 0x3, "1.2288 MHz" )
	PORT_CONFSETTING( 0x4, "0.9216 MHz" )
	PORT_CONFSETTING( 0x5, "0.6144 MHz" )
	PORT_CONFSETTING( 0x6, "0.3072 MHz" )
	PORT_CONFSETTING( 0x7, "Slow" )
	PORT_CONFSETTING( 0x8, "Manual" )
	PORT_CONFSETTING( 0x9, "External" )
INPUT_PORTS_END

ioport_constructor dual_clock_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( dual_clock_jumpers );
}

void dual_clock_base::device_add_mconfig(machine_config &config)
{
	CLOCK(config, m_clock_1, 0);
	m_clock_1->signal_handler().set(FUNC(dual_clock_base::clk_w));

	CLOCK(config, m_clock_2, 0);
	m_clock_2->signal_handler().set(FUNC(dual_clock_base::clk2_w));
}

//**************************************************************************
//  RC2014 Dual Clock module in extended bus
//**************************************************************************

class dual_clock_device : public dual_clock_base, public device_rc2014_ext_card_interface
{
public:
	// construction/destruction
	dual_clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	// base-class overrides
	void clk_w(int state) override { m_bus->clk_w(state); }
	void clk2_w(int state) override { m_bus->clk2_w(state); }
};

dual_clock_device::dual_clock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_clock_base(mconfig, RC2014_DUAL_CLOCK, tag, owner, clock)
	, device_rc2014_ext_card_interface(mconfig, *this)
{
}

void dual_clock_device::device_reset()
{
	m_bus->set_bus_clock(clock_mapping[m_clk_sel_1->read()]);
	dual_clock_base::device_reset();
}

//**************************************************************************
//  RC2014 Dual Clock module in standard bus
//**************************************************************************

class dual_clock_device_40pin : public dual_clock_base, public device_rc2014_card_interface
{
public:
	// construction/destruction
	dual_clock_device_40pin(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	// base-class overrides
	void clk_w(int state) override { m_bus->clk_w(state); }
	void clk2_w(int state) override { }
};

dual_clock_device_40pin::dual_clock_device_40pin(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_clock_base(mconfig, RC2014_DUAL_CLOCK_40P, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
{
}

void dual_clock_device_40pin::device_reset()
{
	m_bus->set_bus_clock(clock_mapping[m_clk_sel_1->read()]);
	dual_clock_base::device_reset();
}

}
//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_SINGLE_CLOCK, device_rc2014_card_interface, single_clock_device, "rc2014_clock", "RC2014 Clock module")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_DUAL_CLOCK, device_rc2014_ext_card_interface, dual_clock_device, "rc2014_dual_clock", "RC2014 Dual Clock module")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_DUAL_CLOCK_40P, device_rc2014_card_interface, dual_clock_device_40pin, "rc2014_dual_clock_40p", "RC2014 Dual Clock module (40 pin)")
