// license:BSD-3-Clause
// copyright-holders:Steve Ellenoff, Manuel Abadia, Couriersud

#include "emu.h"
#include "i80c52.h"
#include "mcs51dasm.h"

DEFINE_DEVICE_TYPE(I80C32, i80c32_device, "i80c32", "Intel 80C32")
DEFINE_DEVICE_TYPE(I80C52, i80c52_device, "i80c52", "Intel 80C52")
DEFINE_DEVICE_TYPE(I87C52, i87c52_device, "i87c52", "Intel 87C52")
DEFINE_DEVICE_TYPE(I87C51FA, i87c51fa_device, "i87c51fa", "Intel 87C51FA")
DEFINE_DEVICE_TYPE(I80C51GB, i80c51gb_device, "i80c51gb", "Intel 80C51GB")
DEFINE_DEVICE_TYPE(AT89C52, at89c52_device, "at89c52", "Atmel AT89C52")
DEFINE_DEVICE_TYPE(AT89S52, at89s52_device, "at89s52", "Atmel AT89S52")
DEFINE_DEVICE_TYPE(DS80C320, ds80c320_device, "ds80c320", "Dallas DS80C320 HSM")

i80c52_device::i80c52_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width)
	: i8052_device(mconfig, type, tag, owner, clock, program_width)
{
	m_has_pd = true;
}

i80c52_device::i80c52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c52_device(mconfig, I80C52, tag, owner, clock, 13)
{
}

i80c32_device::i80c32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c52_device(mconfig, I80C32, tag, owner, clock, 0)
{
}


i87c52_device::i87c52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c52_device(mconfig, I87C52, tag, owner, clock, 13)
{
}

i87c51fa_device::i87c51fa_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width)
	: i80c52_device(mconfig, type, tag, owner, clock, program_width)
{
}

i87c51fa_device::i87c51fa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i87c51fa_device(mconfig, I87C51FA, tag, owner, clock, 13)
{
}

i80c51gb_device::i80c51gb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i87c51fa_device(mconfig, I80C51GB, tag, owner, clock, 0)
{
}

at89c52_device::at89c52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c52_device(mconfig, AT89C52, tag, owner, clock, 13)
{
}

at89s52_device::at89s52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c52_device(mconfig, AT89S52, tag, owner, clock, 13)
{
}

ds80c320_device::ds80c320_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: i80c52_device(mconfig, DS80C320, tag, owner, clock, 0)
{
}

void i80c52_device::device_start()
{
	i8052_device::device_start();
	save_item(NAME(m_saddr));
	save_item(NAME(m_saden));
}

void i80c52_device::device_reset()
{
	i8052_device::device_reset();
	m_saddr = 0;
	m_saden = 0;
}

void i80c52_device::sfr_map(address_map &map)
{
	i8052_device::sfr_map(map);
	map(0xa9, 0xa9).rw(FUNC(i80c52_device::saddr_r), FUNC(i80c52_device::saddr_w));
	map(0xb7, 0xb7).rw(FUNC(i80c52_device::iph_r  ), FUNC(i80c52_device::iph_w  ));
	map(0xb9, 0xb9).rw(FUNC(i80c52_device::saden_r), FUNC(i80c52_device::saden_w));

/* Philips 80C52 */
/* ============= */
/* Reduced EMI Mode
 * The AO bit (AUXR.0) in the AUXR register when set disables the
 * ALE output.
 */
	map(0x8e, 0x8e); // AUXR

/* The dual DPTR structure (see Figure 12) is a way by which the
 * 80C52/54/58 will specify the address of an external data memory
 * location. There are two 16-bit DPTR registers that address the
 * external memory, and a single bit called DPS = AUXR1/bit0 that
 * allows the program code to switch between them.
 */
	map(0xa2, 0xa2); // AUXR1
}

u8 i80c52_device::iph_r()
{
	return m_iph;
}

void i80c52_device::iph_w(u8 data)
{
	m_iph = data;
	update_irq_prio();
}

u8 i80c52_device::saddr_r()
{
	return m_saddr;
}

void i80c52_device::saddr_w(u8 data)
{
	m_saddr = data;
}

u8 i80c52_device::saden_r()
{
	return m_saden;
}

void i80c52_device::saden_w(u8 data)
{
	m_saden = data;
}

std::unique_ptr<util::disasm_interface> i80c52_device::create_disassembler()
{
	return std::make_unique<i80c52_disassembler>();
}

std::unique_ptr<util::disasm_interface> i87c51fa_device::create_disassembler()
{
	return std::make_unique<i8xc51fx_disassembler>();
}

std::unique_ptr<util::disasm_interface> i80c51gb_device::create_disassembler()
{
	return std::make_unique<i8xc51gb_disassembler>();
}

std::unique_ptr<util::disasm_interface> ds80c320_device::create_disassembler()
{
	return std::make_unique<ds80c320_disassembler>();
}
