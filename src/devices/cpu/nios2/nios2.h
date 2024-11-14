// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_NIOS2_NIOS2_H
#define MAME_CPU_NIOS2_NIOS2_H

#pragma once

class nios2_device : public cpu_device
{
public:
	enum {
		NIOS2_ZERO = 0, NIOS2_AT, NIOS2_R2, NIOS2_R3, NIOS2_R4, NIOS2_R5, NIOS2_R6, NIOS2_R7,
		NIOS2_R8, NIOS2_R9, NIOS2_R10, NIOS2_R11, NIOS2_R12, NIOS2_R13, NIOS2_R14, NIOS2_R15,
		NIOS2_R16, NIOS2_R17, NIOS2_R18, NIOS2_R19, NIOS2_R20, NIOS2_R21, NIOS2_R22, NIOS2_R23,
		NIOS2_ET, NIOS2_BT, NIOS2_GP, NIOS2_SP, NIOS2_FP, NIOS2_EA, NIOS2_BA, NIOS2_RA,
		NIOS2_PC,
		NIOS2_STATUS, NIOS2_BSTATUS, NIOS2_ESTATUS, NIOS2_IENABLE,
		NIOS2_IPENDING, NIOS2_CPUID, NIOS2_CTLID, NIOS2_EXCEPTION,
		NIOS2_PTEADDR, NIOS2_TLBACC, NIOS2_TLBMISC, NIOS2_ECCINJ,
		NIOS2_BADADDR, NIOS2_CONFIG, NIOS2_MPUBASE, NIOS2_MPUACC
	};

	// construction/destruction
	nios2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	u32 get_reg(unsigned r) const noexcept { return r == 0 ? 0 : m_gpr[r]; }

	u32 read_ctl(unsigned r) const;
	void write_ctl(unsigned r, u32 val);

	// address space
	address_space_config m_program_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_space;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;

	// internal registers
	u32 m_gpr[32];
	u32 m_ctl[16];
	u32 m_pc;
	s32 m_icount;
};

// device type definition
DECLARE_DEVICE_TYPE(NIOS2, nios2_device)

#endif // MAME_CPU_NIOS2_NIOS2_H
