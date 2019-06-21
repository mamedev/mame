// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ELAN Microelectronics RISC II (RII) Series

    Architecture is very similar to the GI/Microchip PIC series, with
    16-bit opcodes and a banked 8-bit register file with special registers
    for indirect access. (It has no relation to Berkeley RISC II.)

    Currently this device is just a stub with no actual execution core.

***************************************************************************/

#include "emu.h"
#include "riscii.h"
#include "riidasm.h"

// device type definitions
DEFINE_DEVICE_TYPE(RISCII, riscii_series_device, "riscii", "Elan RISC II")


void riscii_series_device::regs_map(address_map &map)
{
	map(0x0000, 0x007f).mirror(m_bankmask << 8).ram(); // TODO: special function registers
	for (unsigned b = 0; b <= m_maxbank; b++)
		map(0x0080 | (b << 8), 0x00ff | (b << 8)).ram();
}

std::unique_ptr<util::disasm_interface> riscii_series_device::create_disassembler()
{
	return std::make_unique<riscii_disassembler>();
}

riscii_series_device::riscii_series_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, unsigned prgbits, unsigned bankbits, uint8_t maxbank)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, prgbits, -1)
	, m_regs_config("register", ENDIANNESS_LITTLE, 8, 8 + bankbits, 0, address_map_constructor(FUNC(riscii_series_device::regs_map), this))
	, m_program(nullptr)
	, m_regs(nullptr)
	, m_prgbits(prgbits)
	, m_bankmask((1 << bankbits) - 1)
	, m_maxbank(maxbank)
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

riscii_series_device::riscii_series_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: riscii_series_device(mconfig, RISCII, tag, owner, clock, 18, 5, 0x1f)
{
}

device_memory_interface::space_config_vector riscii_series_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA, &m_regs_config)
	};
}

void riscii_series_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_regs = &space(AS_DATA);

	set_icountptr(m_icount);

	state_add(RII_PC, "PC", m_pc).mask((1 << m_prgbits) - 1);
	state_add(STATE_GENPC, "GENPC", m_pc).callimport().noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).callimport().noshow();
	state_add(RII_ACC, "ACC", m_acc);
	state_add(RII_BSR, "BSR", m_bsr).mask(m_bankmask);
	state_add(RII_FSR0, "FSR0", m_fsr0);
	state_add(RII_BSR1, "BSR1", m_fsr1.b.h).mask(m_bankmask);
	state_add(RII_FSR1, "FSR1", m_fsr1.b.l); // TODO: high bit forced to 1
	state_add(RII_TABPTR, "TABPTR", m_tabptr).mask(0x800000 + (1 << (m_prgbits + 1)) - 1);
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
