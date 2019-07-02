// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Fujitsu Micro F2MC-16 series

***************************************************************************/

#ifndef MAME_CPU_F2MC16_F2MC16_H
#define MAME_CPU_F2MC16_F2MC16_H 1

#pragma once

class f2mc16_device : public cpu_device
{
public:
	enum
	{
		F2MC16_PC,
		F2MC16_S,
		F2MC16_SP,
		F2MC16_ACC,
		F2MC16_R0,
		F2MC16_R1,
		F2MC16_R2,
		F2MC16_R3,
		F2MC16_R4,
		F2MC16_R5,
		F2MC16_R6,
		F2MC16_R7
	};

	// construction/destruction
	f2mc16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	f2mc16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

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
	address_space_config m_program_config;
	address_space *m_program;

	u32 m_pc;
	u8 m_acc;
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(F2MC16, f2mc16_device)

#endif // MAME_CPU_F2MC16_F2MC16_H
