// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Datamover - 68K-based extension board

#include "emu.h"
#include "datamover.h"

DEFINE_DEVICE_TYPE(MTU130_DATAMOVER0, mtu130_datamover0_device, "datamover0", "Datamover")
DEFINE_DEVICE_TYPE(MTU130_DATAMOVER1, mtu130_datamover1_device, "datamover1", "Datamover (alt address)")

mtu130_datamover0_device::mtu130_datamover0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mtu130_datamover_base_device(mconfig, MTU130_DATAMOVER0, tag, owner, 0xbfbe, clock)
{
}

mtu130_datamover1_device::mtu130_datamover1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	mtu130_datamover_base_device(mconfig, MTU130_DATAMOVER1, tag, owner, 0xbfbc, clock)
{
}

mtu130_datamover_base_device::mtu130_datamover_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t base_address, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	mtu130_extension_interface(mconfig, *this),
	m_cpu(*this, "cpu"),
	m_ram(*this, "ram"),
	m_base_address(base_address)
{
}


void mtu130_datamover_base_device::write23(offs_t offset, u8 data)
{
	if(!m_ram_visible)
		return;
	if(m_ram_xor)
		offset ^= 1;
	offset |= m_ram_bank << 17;
	if(offset & 1)
		m_ram[offset >> 1] = (m_ram[offset >> 1] & 0xff00) | data;
	else
		m_ram[offset >> 1] = (m_ram[offset >> 1] & 0x00ff) | (data << 8);
}

u8 mtu130_datamover_base_device::read23(offs_t offset)
{
	if(!m_ram_visible)
		return 0xff;
	if(m_ram_xor)
		offset ^= 1;
	offset |= m_ram_bank << 17;
	if(offset & 1)
		return m_ram[offset >> 1];
	else
		return m_ram[offset >> 1] >> 8;

}

void mtu130_datamover_base_device::map_io(address_space_installer &space)
{
	space.install_device(m_base_address, m_base_address+1, *this, &mtu130_datamover_base_device::m6502_map);
}

void mtu130_datamover_base_device::device_start()
{
	save_item(NAME(m_ram_visible));
	save_item(NAME(m_ram_xor));
	save_item(NAME(m_ram_bank));
	save_item(NAME(m_irq4_req));
	save_item(NAME(m_irq7_req));
	save_item(NAME(m_irq6502_req));
	save_item(NAME(m_irq6502_en));
}

void mtu130_datamover_base_device::device_reset()
{
	m_cpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_cpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	m_ram_visible = true;
	m_ram_xor = false;
	m_ram_bank = 0;
	m_irq4_req = false;
	m_irq7_req = false;
	m_irq6502_req = false;
	m_irq6502_en = false;
}


void mtu130_datamover_base_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_cpu, 10_MHz_XTAL/10*8);
	m_cpu->set_addrmap(AS_PROGRAM, &mtu130_datamover_base_device::m68k_map);
	m_cpu->set_addrmap(m68000_device::AS_CPU_SPACE, &mtu130_datamover_base_device::m68k_cs_map);
}

void mtu130_datamover_base_device::m6502_map(address_map &map)
{
	map(0, 0).r(FUNC(mtu130_datamover_base_device::status_r)).w(FUNC(mtu130_datamover_base_device::control_w));
	map(1, 1).w(FUNC(mtu130_datamover_base_device::enable_w));
}

void mtu130_datamover_base_device::m68k_map(address_map &map)
{
	map(0x000000, 0x0fffff).ram().share(m_ram);
	map(0x100001, 0x100001).rw(FUNC(mtu130_datamover_base_device::irq_req_r), FUNC(mtu130_datamover_base_device::irq_req_w));
}

void mtu130_datamover_base_device::m68k_cs_map(address_map &map)
{
	map(0xfffff3, 0xfffff3).lr8(NAME([] () -> u8 { return 0x19; }));
	map(0xfffff5, 0xfffff5).lr8(NAME([] () -> u8 { return 0x1a; }));
	map(0xfffff7, 0xfffff7).lr8(NAME([] () -> u8 { return 0x1b; }));
	map(0xfffff9, 0xfffff9).lr8(NAME([this] () -> u8 { m_irq4_req = false; m_cpu->set_input_line(4, CLEAR_LINE); return 0x1c; }));
	map(0xfffffb, 0xfffffb).lr8(NAME([] () -> u8 { return 0x1d; }));
	map(0xfffffd, 0xfffffd).lr8(NAME([] () -> u8 { return 0x1e; }));
	map(0xffffff, 0xffffff).lr8(NAME([this] () -> u8 { m_irq7_req = false; m_cpu->set_input_line(7, CLEAR_LINE); return 0x1f; }));
}

void mtu130_datamover_base_device::control_w(u8 data)
{
	if(data & 0x80)
		m_irq6502_req = false;

	if(data & 0x40)
		m_irq4_req = true;

	m_irq6502_en = data & 0x20;

	if(data & 0x10)
		m_irq7_req = true;

	m_cpu->set_input_line(INPUT_LINE_HALT, data & 0x08 ? CLEAR_LINE : ASSERT_LINE);
	m_cpu->set_input_line(INPUT_LINE_RESET, data & 0x04 ? CLEAR_LINE : ASSERT_LINE);

	if(!(data & 0x04))
		m_irq4_req = m_irq7_req = m_irq6502_req = false;

	m_ram_xor = data & 0x02;

	set_irq(m_irq6502_req && m_irq6502_en);
	m_cpu->set_input_line(4, m_irq4_req ? ASSERT_LINE : CLEAR_LINE);
	m_cpu->set_input_line(7, m_irq7_req ? ASSERT_LINE : CLEAR_LINE);
}

void mtu130_datamover_base_device::irq_req_w(u8 data)
{
	m_irq6502_req = data & 0x01;
	set_irq(m_irq6502_req && m_irq6502_en);
}

u8  mtu130_datamover_base_device::irq_req_r()
{
	return m_irq6502_req;
}

void mtu130_datamover_base_device::enable_w(u8 data)
{
	m_ram_visible = !(data & 8);
	m_ram_bank = data & 7;
}

u8 mtu130_datamover_base_device::status_r()
{
	return
		(m_irq6502_req ? 0x80 : 0x00) |
		(m_irq4_req ? 0x40 : 0x00) |
		(m_irq6502_en ? 0x20 : 0x00) |
		(m_cpu->input_state(INPUT_LINE_HALT) ? 0x00 : 0x10) |
		(m_ram_visible ? 0x00 : 0x08) |
		m_ram_bank;
}
