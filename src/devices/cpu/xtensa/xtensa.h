// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_XTENSA_XTENSA_H
#define MAME_CPU_XTENSA_XTENSA_H

#pragma once


class xtensa_device : public cpu_device
{
public:
	xtensa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	enum {
		XTENSA_PC,
		XTENSA_A0, XTENSA_A1, XTENSA_A2, XTENSA_A3,
		XTENSA_A4, XTENSA_A5, XTENSA_A6, XTENSA_A7,
		XTENSA_A8, XTENSA_A9, XTENSA_A10, XTENSA_A11,
		XTENSA_A12, XTENSA_A13, XTENSA_A14, XTENSA_A15
	};

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
	// address space
	address_space_config m_space_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_space;

	// internal state
	u32 m_a[16];
	u32 m_pc;
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(XTENSA, xtensa_device)

#endif // MAME_CPU_XTENSA_XTENSA_H
