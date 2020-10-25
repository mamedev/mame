// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Motorola M88000 RISC microprocessors

***************************************************************************/

#ifndef MAME_CPU_M88000_M88000_H
#define MAME_CPU_M88000_M88000_H

#pragma once

class mc88100_device : public cpu_device
{
public:
	enum {
		M88000_PC,
		M88000_R1, M88000_R2, M88000_R3,
		M88000_R4, M88000_R5, M88000_R6, M88000_R7,
		M88000_R8, M88000_R9, M88000_R10, M88000_R11,
		M88000_R12, M88000_R13, M88000_R14, M88000_R15,
		M88000_R16, M88000_R17, M88000_R18, M88000_R19,
		M88000_R20, M88000_R21, M88000_R22, M88000_R23,
		M88000_R24, M88000_R25, M88000_R26, M88000_R27,
		M88000_R28, M88000_R29, M88000_R30, M88000_R31,
		M88000_PSR, M88000_VBR
	};

	// construction/destruction
	mc88100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// address spaces
	address_space_config m_code_config;
	address_space_config m_data_config;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_inst_cache;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_data_space;

	// register storage
	u32 m_pc;
	u32 m_r[32];
	u32 m_cr[21];

	s32 m_icount;
};

// device type declaration
DECLARE_DEVICE_TYPE(MC88100, mc88100_device)

#endif // MAME_CPU_M88000_M88000_H
