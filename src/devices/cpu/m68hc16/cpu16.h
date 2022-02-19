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
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	// register helpers
	void debug_set_pcbase(u32 value);
	void debug_set_pc(u32 value);
	void debug_set_ccr(u16 value);
	u16 get_k() noexcept;
	void set_k(u16 value) noexcept;

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
	u32 m_index_regs[3];
	u32 m_sp;

	// MAC registers
	u16 m_hr;
	u16 m_ir;
	u64 m_am;
	bool m_sl;
	u8 m_index_mask[2];

	// misc. state
	s32 m_icount;
};

class mc68hc16z1_device : public cpu16_device
{
public:
	// device type constructor
	mc68hc16z1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declaration
DECLARE_DEVICE_TYPE(MC68HC16Z1, mc68hc16z1_device)


#endif // MAME_CPU_M68HC16_CPU16_H
