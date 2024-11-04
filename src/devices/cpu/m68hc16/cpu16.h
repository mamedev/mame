// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_M68HC16_CPU16_H
#define MAME_CPU_M68HC16_CPU16_H

#pragma once

class cpu16_device : public cpu_device
{
public:
	enum
	{
		CPU16_PC, CPU16_FWA,
		CPU16_IPIPEA, CPU16_IPIPEB, CPU16_IPIPEC,
		CPU16_CCR, CPU16_K,
		CPU16_A, CPU16_B,
		CPU16_D, CPU16_E,
		CPU16_X, CPU16_Y, CPU16_Z, CPU16_SP,
		CPU16_IX, CPU16_IY, CPU16_IZ,
		CPU16_XK, CPU16_YK, CPU16_ZK, CPU16_SK, CPU16_EK,
		CPU16_HR, CPU16_IR,
		CPU16_AM, CPU16_SL,
		CPU16_XMSK, CPU16_YMSK
	};

protected:
	// construction/destruction
	cpu16_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	enum class seq : u16;
	static const seq s_inst_decode[4][256];

	// register helpers
	void debug_set_pcbase(u32 value);
	void set_pc(u32 value) noexcept;
	void debug_set_ccr(u16 value);
	u16 get_k() noexcept;
	void set_k(u16 value) noexcept;
	u16 get_ix(int which) const noexcept;
	void set_ix(int which, u16 value) noexcept;
	u32 add_ix_masked(int which, int offset) const noexcept;
	u8 get_xk(int which) const noexcept;
	void set_xk(int which, u8 value) noexcept;
	void set_a(u8 value) noexcept;
	void set_b(u8 value) noexcept;

	// arithmetic and condition code helpers
	void set_nzv8(u8 data, bool v) noexcept;
	void set_nzv16(u16 data, bool v) noexcept;
	void set_z16(u16 data) noexcept;
	u8 adc8(u8 data1, u8 data2, bool cin) noexcept;
	u8 sbc8(u8 data1, u8 data2, bool cin) noexcept;
	u16 adc16(u16 data1, u16 data2, bool cin) noexcept;
	u16 sbc16(u16 data1, u16 data2, bool cin) noexcept;
	u8 rol8(u8 data, bool cin) noexcept;
	u16 rol16(u16 data, bool cin) noexcept;
	u8 ror8(u8 data, bool cin) noexcept;
	u16 ror16(u16 data, bool cin) noexcept;
	u8 asr8(u8 data) noexcept;
	u16 asr16(u16 data) noexcept;
	void mulu8() noexcept;
	void mulu16() noexcept;
	void muls16(bool frac) noexcept;
	void divu16(bool frac) noexcept;
	u64 accumulate32(u32 data, bool v) noexcept;
	void aslm() noexcept;
	void asrm() noexcept;
	u16 saturation_value(u64 sum) const noexcept;

	// misc. execution helpers
	void advance() noexcept;
	void pshm_step(int n);
	void pulm_step(int n);
	bool cc_test(u8 cc) const noexcept;

	// address spaces
	address_space_config m_program_config;
	address_space_config m_data_config;
	memory_access<20, 1, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<20, 1, 0, ENDIANNESS_BIG>::specific m_data;

	// basic registers
	u32 m_pc;
	u32 m_fwa;
	u16 m_fetch_pipe[3];
	u16 m_ccr;
	u16 m_d;
	u16 m_e;
	u8 m_ek;
	u32 m_index_regs[4];

	// MAC registers
	u16 m_hr;
	u16 m_ir;
	u64 m_am;
	bool m_sl;
	u8 m_index_mask[2];

	// misc. state
	seq m_sequence;
	seq m_return_sequence;
	u32 m_ea;
	u16 m_tmp;
	bool m_start;
	s32 m_icount;
};

#endif // MAME_CPU_M68HC16_CPU16_H
