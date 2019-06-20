// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ELAN Microelectronics RISC II (RII) Series

    Architecture is very similar to the GI/Microchip PIC series, with
    16-bit opcodes and a paged 8-bit register file.

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "riscii.h"
#include "riidasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(RISCII, riscii_series_device, "riscii", "Elan RISC II")


std::unique_ptr<util::disasm_interface> riscii_series_device::create_disassembler()
{
	return std::make_unique<riscii_disassembler>();
}

riscii_series_device::riscii_series_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, RISCII, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 18, -1)
	, m_program(nullptr)
	, m_pc(0)
	, m_acc(0)
	, m_fsr0(0)
	, m_bsr(0)
	, m_tabptr(0)
	, m_stkptr(0)
	, m_cpucon(0)
	, m_status(0)
	, m_icount(0)
{
	m_fsr1.w = 0;
	m_prod.w = 0;
}

device_memory_interface::space_config_vector riscii_series_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
	};
}

void riscii_series_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	set_icountptr(m_icount);

	state_add(RII_PC, "PC", m_pc).mask(0x3ffff);
	state_add(STATE_GENPC, "GENPC", m_pc).callimport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).callimport().noshow();
	state_add(RII_ACC, "ACC", m_acc);
	state_add(RII_FSR0, "FSR0", m_fsr0);
	state_add(RII_FSR1, "FSR1", m_fsr1.b.l);
	state_add(RII_BSR, "BSR", m_bsr).mask(0x1f);
	state_add(RII_BSR1, "BSR1", m_fsr1.b.h).mask(0x1f);
	state_add(RII_TABPTR, "TABPTR", m_tabptr).mask(0xffffff);
	state_add(RII_STKPTR, "STKPTR", m_stkptr);
	state_add(RII_CPUCON, "CPUCON", m_cpucon).mask(0x9f);
	state_add(RII_STATUS, "STATUS", m_status);
	state_add(RII_PROD, "PROD", m_prod.w);

	save_item(NAME(m_pc));
	save_item(NAME(m_acc));
	save_item(NAME(m_fsr0));
	save_item(NAME(m_bsr));
	save_item(NAME(m_fsr1.w));
	save_item(NAME(m_tabptr));
	save_item(NAME(m_stkptr));
	save_item(NAME(m_cpucon));
	save_item(NAME(m_status));
	save_item(NAME(m_prod.w));
}

void riscii_series_device::device_reset()
{
	m_pc = 0x00000;
	m_fsr0 = 0x00;
	m_bsr = 0x00;
	m_fsr1.w = 0x0080;
	m_tabptr = 0x000000;
	m_stkptr = 0x00;
	m_cpucon &= 0x01;
}

void riscii_series_device::execute_run()
{
	debugger_instruction_hook(m_pc);

	m_icount = 0;
}

void riscii_series_device::execute_set_input(int inputnum, int state)
{
	// TODO
}
