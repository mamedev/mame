// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_NIOS2_NIOS2_H
#define MAME_CPU_NIOS2_NIOS2_H

#pragma once

class nios2_device : public cpu_device
{
public:
	enum {
		NIOS2_R1 = 1, NIOS2_R2, NIOS2_R3, NIOS2_R4, NIOS2_R5, NIOS2_R6, NIOS2_R7,
		NIOS2_R8, NIOS2_R9, NIOS2_R10, NIOS2_R11, NIOS2_R12, NIOS2_R13, NIOS2_R14, NIOS2_R15,
		NIOS2_R16, NIOS2_R17, NIOS2_R18, NIOS2_R19, NIOS2_R20, NIOS2_R21, NIOS2_R22, NIOS2_R23,
		NIOS2_R24, NIOS2_R25, NIOS2_R26, NIOS2_R27, NIOS2_R28, NIOS2_R29, NIOS2_R30, NIOS2_R31,
		NIOS2_PC,
	};

	// construction/destruction
	nios2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	// address space
	address_space_config m_program_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;

	// internal registers
	u32 m_gpr[32];
	u32 m_pc;
	s32 m_icount;
};

// device type definition
DECLARE_DEVICE_TYPE(NIOS2, nios2_device)

#endif // MAME_CPU_NIOS2_NIOS2_H
