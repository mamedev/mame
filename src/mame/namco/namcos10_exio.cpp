// license:BSD-3-Clause
// copyright-holders:windyfairy
#include "emu.h"
#include "namcos10_exio.h"

#include "logmacro.h"

DEFINE_DEVICE_TYPE(NAMCOS10_EXIO,      namcos10_exio_device,      "namcos10_exio",   "Namco System 10 EXIO")
DEFINE_DEVICE_TYPE(NAMCOS10_EXIO_BASE, namcos10_exio_base_device, "namcos10_exio_g", "Namco System 10 EXIO(G)")
DEFINE_DEVICE_TYPE(NAMCOS10_MGEXIO,    namcos10_mgexio_device,    "namcos10_mgexio", "Namco System 10 MGEXIO")

// EXIO(G) has the bare minimum: CPLD, audio output jacks, gun I/O, and a card edge connector for additional I/O
namcos10_exio_base_device::namcos10_exio_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t ident_code)
	: device_t(mconfig, type, tag, owner, clock), m_ident_code(ident_code)
{
}

namcos10_exio_base_device::namcos10_exio_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namcos10_exio_base_device(mconfig, NAMCOS10_EXIO_BASE, tag, owner, clock, 0x32)
{
}

////////////////////////////////////

namcos10_exio_device::namcos10_exio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namcos10_exio_base_device(mconfig, NAMCOS10_EXIO, tag, owner, clock, 0x30),
	m_maincpu(*this, "exio_mcu"),
	m_ram(*this, "exio_ram")
{
}

void namcos10_exio_device::device_start()
{
	save_item(NAME(m_is_active));
}

void namcos10_exio_device::device_reset()
{
}

void namcos10_exio_device::device_reset_after_children()
{
	m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
	m_is_active = false;
}

void namcos10_exio_device::map(address_map &map)
{
	map(0x000100, 0x002fff).ram(); // TODO: Stack and such is stored here, how large should this really be?
	map(0x003000, 0x007fff).ram().mirror(0xff8000).share(m_ram);
}

void namcos10_exio_device::device_add_mconfig(machine_config &config)
{
	// TODO: tmp95c061 doesn't have a serial implementation yet so JVS communication won't work for now
	TMP95C061(config, m_maincpu, XTAL(22'118'400));
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos10_exio_device::map);

	m_maincpu->port1_read().set(FUNC(namcos10_exio_device::port_read<1>));
	m_maincpu->port5_read().set(FUNC(namcos10_exio_device::port_read<5>));
	m_maincpu->port7_read().set(FUNC(namcos10_exio_device::port_read<7>));
	m_maincpu->port8_read().set(FUNC(namcos10_exio_device::port_read<8>));
	m_maincpu->port9_read().set(FUNC(namcos10_exio_device::port_read<9>));
	m_maincpu->porta_read().set(FUNC(namcos10_exio_device::port_read<10>));
	m_maincpu->portb_read().set(FUNC(namcos10_exio_device::port_read<11>));
	m_maincpu->port1_write().set(FUNC(namcos10_exio_device::port_write<1>));
	m_maincpu->port2_write().set(FUNC(namcos10_exio_device::port_write<2>));
	m_maincpu->port5_write().set(FUNC(namcos10_exio_device::port_write<5>));
	m_maincpu->port6_write().set(FUNC(namcos10_exio_device::port_write<6>));
	m_maincpu->port7_write().set(FUNC(namcos10_exio_device::port_write<7>));
	m_maincpu->port8_write().set(FUNC(namcos10_exio_device::port_write<8>));
	m_maincpu->porta_write().set(FUNC(namcos10_exio_device::port_write<10>));
	m_maincpu->portb_write().set(FUNC(namcos10_exio_device::port_write<11>));
}

uint16_t namcos10_exio_device::cpu_status_r()
{
	uint16_t r = m_is_active ? 1 : 0;
	return (r << 8) | r;
}

void namcos10_exio_device::ctrl_w(uint16_t data)
{
	logerror("%s: exio_ctrl_w %04x\n", machine().describe_context(), data);

	if (data == 3) {
		// Probably this should just reset the entire CPU once the program is loaded in memory
		// The CPU was halted as soon as it was started so just set the PC and resume it.
		auto pc = ((m_ram[0x4f02 / 2] << 16) | m_ram[0x4f00 / 2]) & 0xffffff;
		m_maincpu->set_state_int(TLCS900_PC, pc);
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}
}

void namcos10_exio_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(offset, 0))
		m_ram[offset / 2] = ((data & 0xff) << 8) | (m_ram[offset / 2] & 0xff);
	else
		m_ram[offset / 2] = (m_ram[offset / 2] & 0xff00) | (data & 0xff);
}

uint16_t namcos10_exio_device::ram_r(offs_t offset)
{
	if (BIT(offset, 0))
		return BIT(m_ram[offset / 2], 8, 8);

	return BIT(m_ram[offset / 2], 0, 8);
}

template <int port>
void namcos10_exio_device::port_write(offs_t offset, uint8_t data)
{
	logerror("%s: exio_port%d_write %02x\n", machine().describe_context(), port, data);

	if (port == 8) {
		// HACK: Simple check to just know when the CPU is alive
		m_is_active |= data != 0;
	}
}

template <int port>
uint8_t namcos10_exio_device::port_read(offs_t offset)
{
	logerror("%s: exio_port%d_read\n", machine().describe_context(), port);
	return 0;
}

////////////////////////////////////

namcos10_mgexio_device::namcos10_mgexio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: namcos10_exio_base_device(mconfig, NAMCOS10_MGEXIO, tag, owner, clock, 0x33),
	m_maincpu(*this, "exio_mcu"),
	m_ram(*this, "exio_ram"),
	m_nvram(*this, "nvram")
{
}

void namcos10_mgexio_device::device_start()
{
	save_item(NAME(m_active_state));
	m_nvram->set_base(m_ram, 0x8000);
}

void namcos10_mgexio_device::device_reset()
{
}

void namcos10_mgexio_device::device_reset_after_children()
{
	m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
	m_active_state = 0;
}

void namcos10_mgexio_device::map(address_map &map)
{
	map(0x00000, 0x7ffff).ram().share(m_ram);
}

void namcos10_mgexio_device::io_map(address_map &map)
{
	// TODO: Hook up I/O ports
}

void namcos10_mgexio_device::device_add_mconfig(machine_config &config)
{
	H83007(config, m_maincpu, 14.746_MHz_XTAL);
	m_maincpu->set_mode_a20();
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos10_mgexio_device::map);
	m_maincpu->set_addrmap(AS_IO, &namcos10_mgexio_device::io_map);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}

uint16_t namcos10_mgexio_device::cpu_status_r()
{
	// pacmball's code call bit 1 the "sub_cpu_enable_flag"
	auto r = m_active_state == 2 ? 2 : 0;

	if (m_active_state == 1)
		m_active_state = 2; // needs to see this as not active at least once

	return r;
}

void namcos10_mgexio_device::ctrl_w(uint16_t data)
{
	logerror("%s: exio_ctrl_w %04x\n", machine().describe_context(), data);

	if (data == 3) {
		m_maincpu->resume(SUSPEND_REASON_HALT);
		m_active_state = 1;
	}
}

void namcos10_mgexio_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(offset, 0) == 1)
		m_ram[offset / 2] = (m_ram[offset / 2] & 0xff00) | data;
	else
		m_ram[offset / 2] = (data << 8) | (m_ram[offset / 2] & 0xff);
}

uint16_t namcos10_mgexio_device::ram_r(offs_t offset)
{
	if (BIT(offset, 0) == 1)
		return m_ram[offset / 2] & 0xff;
	else
		return (m_ram[offset / 2] >> 8) & 0xff;
}
