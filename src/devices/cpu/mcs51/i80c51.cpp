// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#include "emu.h"
#include "i80c51.h"
#include "mcs51dasm.h"

DEFINE_DEVICE_TYPE(I80C31, i80c31_device, "i80c31", "Intel 80C31")
DEFINE_DEVICE_TYPE(I80C51, i80c51_device, "i80c51", "Intel 80C51")
DEFINE_DEVICE_TYPE(I87C51, i87c51_device, "i87c51", "Intel 87C51")
DEFINE_DEVICE_TYPE(AT89C4051, at89c4051_device, "at89c4051", "Atmel AT89C4051")
DEFINE_DEVICE_TYPE(P80C552, p80c552_device, "p80c552", "Philips P80C552")
DEFINE_DEVICE_TYPE(P87C552, p87c552_device, "p87c552", "Philips P87C552")
DEFINE_DEVICE_TYPE(P80C562, p80c562_device, "p80c562", "Philips P80C562")

void i80c51_device::device_start()
{
	mcs51_cpu_device::device_start();
	m_slave_address = 0;
	m_slave_mask = 0;

	save_item(NAME(m_slave_address));
	save_item(NAME(m_slave_mask));
}

void i80c51_device::sfr_map(address_map &map)
{
	mcs51_cpu_device::sfr_map(map);
	map(0xa9, 0xa9).rw(FUNC(i80c51_device::slave_address_r), FUNC(i80c51_device::slave_address_w));
	map(0xb9, 0xb9).rw(FUNC(i80c51_device::slave_mask_r), FUNC(i80c51_device::slave_mask_w));
}

void i80c51_device::slave_address_w(u8 data)
{
	m_slave_address = data;
}

u8 i80c51_device::slave_address_r()
{
	return m_slave_address;
}

void i80c51_device::slave_mask_w(u8 data)
{
	m_slave_mask = data;
}

u8 i80c51_device::slave_mask_r()
{
	return m_slave_mask;
}

bool i80c51_device::manage_idle_on_interrupt(u8 ints)
{
	/* any interrupt terminates idle mode */
	set_idl(0);
	/* external interrupt wakes up */
	if (ints & (BIT(m_tcon, TCON_IE0) | BIT(m_tcon, TCON_IE1)))
		set_pd(0);
	return BIT(m_pcon, PCON_PD);
}


i80c31_device::i80c31_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c51_device(mconfig, I80C31, tag, owner, clock, 0, 7)
{
}

i80c51_device::i80c51_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int io_width)
	: mcs51_cpu_device(mconfig, type, tag, owner, clock, program_width, io_width)
{
	m_has_pd = true;
}

i80c51_device::i80c51_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c51_device(mconfig, I80C51, tag, owner, clock, 12, 7)
{
}

i87c51_device::i87c51_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c51_device(mconfig, I87C51, tag, owner, clock, 12, 7)
{
}

at89c4051_device::at89c4051_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c51_device(mconfig, AT89C4051, tag, owner, clock, 12, 7)
{
}


p80c562_device::p80c562_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int io_width)
	: i80c51_device(mconfig, type, tag, owner, clock, program_width, io_width)
{
}

p80c562_device::p80c562_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: p80c562_device(mconfig, P80C562, tag, owner, clock, 0, 8)
{
}

p80c552_device::p80c552_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: p80c562_device(mconfig, P80C552, tag, owner, clock, 0, 8)
{
}

p87c552_device::p87c552_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: p80c562_device(mconfig, P87C552, tag, owner, clock, 12, 8)
{
}

std::unique_ptr<util::disasm_interface> i80c31_device::create_disassembler()
{
	return std::make_unique<i80c51_disassembler>();
}

std::unique_ptr<util::disasm_interface> i80c51_device::create_disassembler()
{
	return std::make_unique<i80c51_disassembler>();
}

std::unique_ptr<util::disasm_interface> p80c562_device::create_disassembler()
{
	return std::make_unique<p8xc562_disassembler>();
}

std::unique_ptr<util::disasm_interface> p80c552_device::create_disassembler()
{
	return std::make_unique<p8xc552_disassembler>();
}

std::unique_ptr<util::disasm_interface> p87c552_device::create_disassembler()
{
	return std::make_unique<p8xc552_disassembler>();
}
