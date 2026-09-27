// license:BSD-3-Clause
// copyright-holders:Andrea Bogazzi

// Jaleco "FPU" math coprocessor (F-1 Super Battle)

#ifndef MAME_CPU_JALFPU_JALFPU_H
#define MAME_CPU_JALFPU_JALFPU_H

#pragma once

class jaleco_fpu_device : public cpu_device
{
public:
	enum
	{
		JALFPU_PC = 1,
		JALFPU_S0, JALFPU_S1, JALFPU_S2, JALFPU_S3, JALFPU_S4, JALFPU_S5, JALFPU_S6, JALFPU_S7,
		JALFPU_S8, JALFPU_S9, JALFPU_SA, JALFPU_SB, JALFPU_SC, JALFPU_SD, JALFPU_SE, JALFPU_SF,
		JALFPU_C6, JALFPU_C7, JALFPU_SP, JALFPU_CTRL,
		JALFPU_SIGN, JALFPU_STK0, JALFPU_STK1, JALFPU_STK2, JALFPU_STK3,
		JALFPU_DELAY, JALFPU_DTGT
	};

	jaleco_fpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_cb() { return m_irq_cb.bind(); }

	void host_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_run() override;

	virtual space_config_vector memory_space_config() const override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	enum : u8 { F_Z = 1, F_N = 2, F_C = 4, F_V = 8 };

	address_space_config m_program_config;
	address_space_config m_data_config;
	memory_access<10, 2, -2, ENDIANNESS_LITTLE>::cache m_program;
	memory_access<12, 1, -1, ENDIANNESS_LITTLE>::specific m_data;
	required_shared_ptr<u32> m_prg_ram;
	required_shared_ptr<u16> m_data_ram;

	devcb_write_line m_irq_cb;

	int m_icount;

	u16 m_pc;
	u16 m_ppc;
	u16 m_s[16];
	u16 m_c6;
	u16 m_c7;
	u8 m_flags;
	u16 m_stack[4];
	u8 m_sp;
	u16 m_sign;
	u16 m_ctrl;
	u16 m_hostreg[0x40];
	bool m_running;
	bool m_delay;
	u16 m_delay_target;

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	u16 host_r(offs_t offset);
	void host_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 host_data_r(offs_t offset);
	void host_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 host_prg_r(offs_t offset);
	void host_prg_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void set_nz(u16 v);
	bool condition(u8 code);
	u16 mem_addr(u8 mode, u8 base);
	void unimplemented(u32 op);
	void op_alu(u32 op);
	void op_muldiv(u32 op);
	void op_move(u32 op);
	void op_shift(u32 op);
	void op_group(u32 op);
	void op_branch(u32 op);
	void execute_one(u32 op);
};

DECLARE_DEVICE_TYPE(JALECO_FPU, jaleco_fpu_device)

#endif // MAME_CPU_JALFPU_JALFPU_H
