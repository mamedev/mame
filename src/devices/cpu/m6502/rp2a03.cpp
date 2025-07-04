// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    rp2a03.cpp

    6502, NES variant

***************************************************************************/

#include "emu.h"
#include "rp2a03.h"
#include "rp2a03d.h"

DEFINE_DEVICE_TYPE(RP2A03_CORE, rp2a03_core_device, "rp2a03_core", "Ricoh RP2A03 core") // needed for some VT systems with XOP instead of standard APU
DEFINE_DEVICE_TYPE(RP2A03,      rp2a03_device,      "rp2a03",      "Ricoh RP2A03")      // earliest version, found in punchout, spnchout, dkong3, VS. systems, and some early Famicoms
DEFINE_DEVICE_TYPE(RP2A03G,     rp2a03g_device,     "rp2a03g",     "Ricoh RP2A03G")     // later revision, found in front-loader NES


void rp2a03_device::rp2a03_map(address_map &map)
{
	map(0x4000, 0x4013).w(m_apu, FUNC(nesapu_device::write));
	map(0x4015, 0x4015).lw8(NAME([this](u8 data) { m_apu->write(0x15, data); }));
	map(0x4017, 0x4017).lw8(NAME([this](u8 data) { m_apu->write(0x17, data); }));
	map(0x4015, 0x4015).r(m_apu, FUNC(nesapu_device::status_r));
	// 0x4014 w -> NES sprite DMA (is this internal?)
	// 0x4016 w -> d0-d2: RP2A03 OUT0,OUT1,OUT2
	// 0x4016 r -> d0-d4: RP2A03 IN0
	// 0x4017 r -> d0-d4: RP2A03 IN1
}



rp2a03_core_device::rp2a03_core_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: m6502_device(mconfig, type, tag, owner, clock)
{
}

rp2a03_core_device::rp2a03_core_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rp2a03_core_device(mconfig, RP2A03_CORE, tag, owner, clock)
{
}



rp2a03_device::rp2a03_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: rp2a03_core_device(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_apu(*this, "nesapu")
{
	program_config.m_internal_map = address_map_constructor(FUNC(rp2a03_device::rp2a03_map), this);
}

rp2a03_device::rp2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rp2a03_device(mconfig, RP2A03, tag, owner, clock)
{
}

rp2a03g_device::rp2a03g_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: rp2a03_device(mconfig, RP2A03G, tag, owner, clock)
{
}



std::unique_ptr<util::disasm_interface> rp2a03_core_device::create_disassembler()
{
	return std::make_unique<rp2a03_disassembler>();
}

void rp2a03_device::apu_irq(int state)
{
	// games relying on the APU_IRQ don't seem to work anyway? (nes software list : timelord, mig29sf, firehawk)
	set_input_line(RP2A03_APU_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t rp2a03_device::apu_read_mem(offs_t offset)
{
	return mintf->program.read_byte(offset);
}

void rp2a03_device::device_add_mconfig(machine_config &config)
{
	APU_2A03(config, m_apu, DERIVED_CLOCK(1,1));
	m_apu->irq().set(FUNC(rp2a03_device::apu_irq));
	m_apu->mem_read().set(FUNC(rp2a03_device::apu_read_mem));
	m_apu->add_route(ALL_OUTPUTS, *this, 1.0, 0);
}

void rp2a03g_device::device_add_mconfig(machine_config &config)
{
	NES_APU(config, m_apu, DERIVED_CLOCK(1,1));
	m_apu->irq().set(FUNC(rp2a03g_device::apu_irq));
	m_apu->mem_read().set(FUNC(rp2a03g_device::apu_read_mem));
	m_apu->add_route(ALL_OUTPUTS, *this, 1.0, 0);
}


#include "cpu/m6502/rp2a03.hxx"
