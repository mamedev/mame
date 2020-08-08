// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_MN1880_MN1880_H
#define MAME_CPU_MN1880_MN1880_H

#pragma once

class mn1880_device : public cpu_device
{
public:
	mn1880_device(const machine_config &config, const char *tag, device_t *owner, u32 clock);

	enum {
		MN1880_IP, MN1880_FS,
		MN1880_XP, MN1880_YP,
		MN1880_XPL, MN1880_XPH, MN1880_YPL, MN1880_YPH,
		MN1880_SP, MN1880_LP
	};

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

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	// address spaces
	address_space_config m_program_config;
	address_space_config m_data_config;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_data;

	// internal state
	u16 m_ip;
	u8 m_fs;
	u16 m_xp;
	u16 m_yp;
	u16 m_sp;
	u16 m_lp;
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(MN1880, mn1880_device)

#endif // MAME_CPU_MN1880_MN1880_H
