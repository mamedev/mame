// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_H8500_H8500_H
#define MAME_CPU_H8500_H8500_H

#pragma once

class h8500_device : public cpu_device
{
public:
	enum {
		H8500_PC,
		H8500_SR, H8500_CCR,
		H8500_CP, H8500_DP, H8500_EP, H8500_TP,
		H8500_BR,
		H8500_R0, H8500_R1, H8500_R2, H8500_R3,
		H8500_R4, H8500_R5, H8500_FP, H8500_SP
	};

	void set_mode(u8 mode) { assert(!configured()); m_mode_control = mode; }

protected:
	h8500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, int buswidth, int ramsize, int defmode, address_map_constructor map);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 2); }
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	u8 mode_control() const { return m_mode_control; }
	virtual bool h8_maximum_mode() const noexcept { return m_mode_control == 3 || m_mode_control == 4; } // all except H8/570

private:
	void debug_set_pc(offs_t pc) noexcept;

	// address spaces
	address_space_config m_program_config;
	address_space *m_program;

	// misc. configuration
	u8 m_mode_control;
	[[maybe_unused]] u16 m_ram_size;

	// internal registers
	u16 m_pc;
	u16 m_ppc;
	u16 m_sr;
	u8 m_cp;
	u8 m_dp;
	u8 m_ep;
	u8 m_tp;
	u8 m_br;
	u16 m_r[8];

	// execution state
	s32 m_icount;
};

#endif // MAME_CPU_H8500_H8500_H
