// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    RC2014 Z80 CPU Module

****************************************************************************/

#include "emu.h"
#include "z80cpu.h"

#include "cpu/z80/z80.h"

class z80cpu_device : public device_t, public device_rc2014_card_interface
{
public:
	// construction/destruction
	z80cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_resolve_objects() override;
private:
/*	void bus_mem_w(offs_t offset, u8 data) { m_bus->space(AS_PROGRAM).write_byte(offset, data); }
	u8 bus_mem_r(offs_t offset) { return m_bus->space(AS_PROGRAM).read_byte(offset); }
	void bus_io_w(offs_t offset, u8 data) { m_bus->space(AS_IO).write_byte(offset, data); }
	u8 bus_io_r(offs_t offset) { return m_bus->space(AS_IO).read_byte(offset); }
*/
	void addrmap_mem(address_map &map);
	void addrmap_io(address_map &map);

	// object finders
	required_device<z80_device> m_maincpu;
};

DEFINE_DEVICE_TYPE_PRIVATE(RC2014_Z80CPU, device_rc2014_card_interface, z80cpu_device, "rc2014_z80", "RC2014 Z80 CPU Module")

z80cpu_device::z80cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RC2014_Z80CPU, tag, owner, clock)
	, device_rc2014_card_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
{
}

void z80cpu_device::device_start()
{
}

void z80cpu_device::addrmap_mem(address_map &map)
{
	map.unmap_value_high(); // unmapped addresses return 0xff
}

void z80cpu_device::addrmap_io(address_map &map)
{
	map.global_mask(0xff);  // use 8-bit ports
}

void z80cpu_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, DERIVED_CLOCK(1,1));
	m_maincpu->set_addrmap(AS_PROGRAM, &z80cpu_device::addrmap_mem);
	m_maincpu->set_addrmap(AS_IO, &z80cpu_device::addrmap_io);
}

void z80cpu_device::device_resolve_objects()
{
	m_bus->assign_spaces(&m_maincpu->space(AS_PROGRAM), &m_maincpu->space(AS_IO));
}
