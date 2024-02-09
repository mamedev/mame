// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "h8s2000.h"
#include "h8s2000d.h"

h8s2000_device::h8s2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor map_delegate) :
	h8h_device(mconfig, type, tag, owner, clock, map_delegate)
{
	m_has_exr = true;
}

std::unique_ptr<util::disasm_interface> h8s2000_device::create_disassembler()
{
	return std::make_unique<h8s2000_disassembler>();
}

// FIXME: one-state bus cycles are only provided for on-chip ROM & RAM in H8S/2000 and H8S/2600.
// All other accesses take *at least* two states each, and additional wait states are often programmed for external memory!

uint16_t h8s2000_device::read16i(uint32_t adr)
{
	m_icount--;
	return m_cache.read_word(adr & ~1);
}

uint8_t h8s2000_device::read8(uint32_t adr)
{
	m_icount--;
	return m_program.read_byte(adr);
}

void h8s2000_device::write8(uint32_t adr, uint8_t data)
{
	m_icount--;
	m_program.write_byte(adr, data);
}

uint16_t h8s2000_device::read16(uint32_t adr)
{
	m_icount--;
	return m_program.read_word(adr & ~1);
}

void h8s2000_device::write16(uint32_t adr, uint16_t data)
{
	m_icount--;
	m_program.write_word(adr & ~1, data);
}

void h8s2000_device::internal(int cycles)
{
	m_icount -= cycles;
}

#include "cpu/h8/h8s2000.hxx"
