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
		RX01_CRC,
		RX01_UNIT, RX01_LDHD
	};

	enum : u16 {
		FF_IOB0 = 1 << 0,
		FF_IOB1 = 1 << 1,
		FF_IOB2 = 1 << 2,
		FF_IOB3 = 1 << 3,
		FF_IOB4 = 1 << 4,
		FF_IOB5 = 1 << 5,
		FF_IOB6 = 1 << 6,
		FF_WRTBUF = 1 << 7,
		FF_FLAG = 1 << 8
	};

	// device type constructor
	rx01_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	void scratchpad_map(address_map &map);

	// internal helpers
	u8 mux_out();
	bool sep_data();
	bool missing_clk();
	bool drv_sel_trk0();
	bool test_condition();
	void shift_crc(bool data);
	void set_flag(bool j, bool k);

	// address spaces
	address_space_config m_inst_config;
	address_space_config m_sp_config;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_inst_cache;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_sp_cache;

	// internal state
	u16 m_pc;
	u16 m_ppc;
	u8 m_mb;
	bool m_br_condition;
	bool m_inst_disable;
	bool m_inst_repeat;
	u8 m_cntr;
	u8 m_sr;
	u8 m_spar;
	u16 m_bar;
	u16 m_crc;
	u16 m_flags;
	bool m_unit;
	bool m_load_head;
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(RX01_CPU, rx01_cpu_device)

#endif // MAME_CPU_RX01_RX01_H
