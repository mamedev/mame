// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Z80 CPU Module

****************************************************************************/

#include "emu.h"
#include "z80cpu.h"

#include "cpu/z80/z80.h"

namespace {

//**************************************************************************
//  Z80 CPU base class
//**************************************************************************

class z80cpu_base : public device_t
{
protected:
	// construction/destruction
	z80cpu_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void addrmap_mem(address_map &map) { map.unmap_value_high(); }
	void addrmap_io(address_map &map) { map.unmap_value_high(); }

	// object finders
	required_device<z80_device> m_maincpu;
};

z80cpu_base::z80cpu_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu(*this, "maincpu")
{
}

void z80cpu_base::device_start()
{
}

void z80cpu_base::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, DERIVED_CLOCK(1,1));
	m_maincpu->set_addrmap(AS_PROGRAM, &z80cpu_base::addrmap_mem);
	m_maincpu->set_addrmap(AS_IO, &z80cpu_base::addrmap_io);
}

//**************************************************************************
//  RC2014 Z80 CPU module
//  Module author: Spencer Owen
//**************************************************************************

class z80cpu_device : public z80cpu_base, public device_rc2014_card_interface
{
public:
	// construction/destruction
	z80cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

	virtual void card_int_w(int state) override { m_maincpu->set_input_line(INPUT_LINE_IRQ0, state); }
};

z80cpu_device::z80cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80cpu_base(mconfig, RC2014_Z80CPU, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
{
}

void z80cpu_device::device_start()
{
	m_maincpu->set_daisy_config(m_bus->get_daisy_chain());
}

void z80cpu_device::device_resolve_objects()
{
	m_bus->assign_installer(AS_PROGRAM, &m_maincpu->space(AS_PROGRAM));
	m_bus->assign_installer(AS_IO, &m_maincpu->space(AS_IO));
}

//**************************************************************************
//  RC2014 Z80 CPU 2.1 module
//  Module author: Spencer Owen
//**************************************************************************

class z80cpu21_device : public z80cpu_base, public device_rc2014_ext_card_interface
{
public:
	// construction/destruction
	z80cpu21_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

	virtual void card_int_w(int state) override { m_maincpu->set_input_line(INPUT_LINE_IRQ0, state); }
	virtual void card_nmi_w(int state) override { m_maincpu->set_input_line(INPUT_LINE_NMI, state); }
};

z80cpu21_device::z80cpu21_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80cpu_base(mconfig, RC2014_Z80CPU_21, tag, owner, clock)
	, device_rc2014_ext_card_interface(mconfig, *this)
{
}

void z80cpu21_device::device_start()
{
	m_maincpu->set_daisy_config(m_bus->get_daisy_chain());
}

void z80cpu21_device::device_resolve_objects()
{
	m_bus->assign_installer(AS_PROGRAM, &m_maincpu->space(AS_PROGRAM));
	m_bus->assign_installer(AS_IO, &m_maincpu->space(AS_IO));
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_Z80CPU, device_rc2014_card_interface, z80cpu_device, "rc2014_z80", "RC2014 Z80 CPU module")
DEFINE_DEVICE_TYPE_PRIVATE(RC2014_Z80CPU_21, device_rc2014_ext_card_interface, z80cpu21_device, "rc2014_z8021", "RC2014 Z80 CPU 2.1 module")
