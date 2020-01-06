// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_RX01_RX01_H
#define MAME_CPU_RX01_RX01_H

#pragma once

class rx01_cpu_device : public cpu_device
{
public:
	enum {
		RX01_PC,
		RX01_CNTR, RX01_SR, RX01_SPAR,
		RX01_R0, RX01_R1, RX01_R2, RX01_R3, RX01_R4, RX01_R5, RX01_R6, RX01_R7,
		RX01_R8, RX01_R9, RX01_R10, RX01_R11, RX01_R12, RX01_R13, RX01_R14, RX01_R15,
		RX01_BAR,
		RX01_CRC
	};

	// device type constructor
	rx01_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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
	void scratchpad_map(address_map &map);

	// address spaces
	address_space_config m_rom_config;
	address_space_config m_sp_config;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_rom_cache;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_sp_cache;

	// internal state
	u16 m_pc;
	u8 m_cntr;
	u8 m_sr;
	u8 m_spar;
	u16 m_bar;
	u16 m_crc;
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(RX01_CPU, rx01_cpu_device)

#endif // MAME_CPU_RX01_RX01_H
