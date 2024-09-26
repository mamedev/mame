// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    AT&T WE32100 32-Bit Microprocessor

***************************************************************************/

#ifndef MAME_CPU_WE32000_WE32100_H
#define MAME_CPU_WE32000_WE32100_H

#pragma once


class we32100_device : public cpu_device
{
public:
	enum {
		WE_R0, WE_R1, WE_R2, WE_R3, WE_R4, WE_R5, WE_R6, WE_R7, WE_R8,
		WE_R9, WE_R10, WE_R11, WE_R12, WE_R13, WE_R14, WE_R15,
		WE_FP, WE_AP, WE_PSW, WE_SP, WE_PCBP, WE_ISP, WE_PC
	};

	// construction/destruction
	we32100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	// address space
	address_space_config m_space_config;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_space;

	// internal state
	u32 m_r[16];
	s32 m_icount;
};

// device type declaration
DECLARE_DEVICE_TYPE(WE32100, we32100_device)

#endif // MAME_CPU_WE32000_WE32100_H
