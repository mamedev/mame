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
		F2MC16_PC, F2MC16_PS, F2MC16_USP, F2MC16_SSP, F2MC16_ACC,
		F2MC16_PCB, F2MC16_DTB, F2MC16_USB, F2MC16_SSB, F2MC16_ADB, F2MC16_DPR,
		F2MC16_RW0, F2MC16_RW1, F2MC16_RW2, F2MC16_RW3,
		F2MC16_RW4, F2MC16_RW5, F2MC16_RW6, F2MC16_RW7,
		F2MC16_RL0, F2MC16_RL1, F2MC16_RL2, F2MC16_RL3,
		F2MC16_R0, F2MC16_R1, F2MC16_R2, F2MC16_R3, F2MC16_R4, F2MC16_R5, F2MC16_R6, F2MC16_R7
	};

	enum
	{
		F_I = 0x40,
		F_S = 0x20,
		F_T = 0x10,
		F_N = 0x08,
		F_Z = 0x04,
		F_V = 0x02,
		F_C = 0x01
	};

	// construction/destruction
	f2mc16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	f2mc16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

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

	u16 m_pc, m_usp, m_ssp, m_ps;
	u8 m_pcb, m_dtb, m_usb, m_ssb, m_adb, m_dpr;
	u32 m_acc, m_temp;
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(F2MC16, f2mc16_device)

#endif // MAME_CPU_F2MC16_F2MC16_H
