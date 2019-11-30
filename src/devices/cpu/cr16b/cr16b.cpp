// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    National Semiconductor CompactRISC CR16B

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "cr16b.h"
#include "cr16bdasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(CR16B, cr16b_device, "cr16b", "CompactRISC CR16B")

cr16b_device::cr16b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 16, 21, 0, map)
	, m_space(nullptr)
	, m_cache(nullptr)
	, m_regs{0}
	, m_pc(0)
	, m_isp(0)
	, m_intbase(0)
	, m_psr(0)
	, m_cfg(0)
	, m_dcr(0)
	, m_dsr(0)
	, m_car(0)
	, m_icount(0)
{
}

// TODO: figure out some actual device types instead
cr16b_device::cr16b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cr16b_device(mconfig, CR16B, tag, owner, clock, address_map_constructor())
{
}

std::unique_ptr<util::disasm_interface> cr16b_device::create_disassembler()
{
	return std::make_unique<cr16b_disassembler>();
}

device_memory_interface::space_config_vector cr16b_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_space_config),
	};
}

void cr16b_device::device_start()
{
	m_space = &space(AS_PROGRAM);
	m_cache = m_space->cache<1, 0, ENDIANNESS_LITTLE>();

	set_icountptr(m_icount);

	state_add(CR16_PC, "PC", m_pc).mask(0x1ffffe);
	state_add(STATE_GENPC, "GENPC", m_pc).callimport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).callimport().noshow();
	state_add(CR16_ISP, "ISP", m_isp).mask(0x00ffff).formatstr("%06X");
	state_add(CR16_INTBASE, "INTBASE", m_intbase).mask(0x1ffffe);
	state_add(CR16_PSR, "PSR", m_psr).mask(0x0ee7).formatstr("%04X");
	for (int i = 0; i < 13; i++)
		state_add(CR16_R0 + i, string_format("R%d", i).c_str(), m_regs[i]);
	state_add(CR16_R13, "ERA", m_regs[13]);
	state_add(CR16_R14, "RA", m_regs[14]);
	state_add(CR16_R15, "SP", m_regs[15]);

	save_item(NAME(m_regs));
	save_item(NAME(m_pc));
	save_item(NAME(m_isp));
	save_item(NAME(m_intbase));
	save_item(NAME(m_psr));
	save_item(NAME(m_cfg));
	save_item(NAME(m_dcr));
	save_item(NAME(m_dsr));
	save_item(NAME(m_car));
}

void cr16b_device::device_reset()
{
	// Save old values of PC and PSR in R0 and R1
	m_regs[0] = (m_pc & 0x01fffe) >> 1;
	m_regs[1] = (m_pc & 0x1e0000) >> 5 | (m_psr & 0x0fff);

	// Reset internal registers
	m_pc = 0;
	m_cfg = 0;
	m_dcr = 0;
	m_psr = 0x0200; // PSR.E = 1
}

void cr16b_device::execute_run()
{
	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void cr16b_device::execute_set_input(int inputnum, int state)
{
	// TODO
}
