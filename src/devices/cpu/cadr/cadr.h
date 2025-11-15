// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#ifndef MAME_CPU_CADR_CADR_H
#define MAME_CPU_CADR_CADR_H

#pragma once

class cadr_cpu_device : public cpu_device
{
public:
	cadr_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u16 diag_r(offs_t offset);
	void diag_w(offs_t offset, u16 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; } // TODO
	virtual u32 execute_max_cycles() const noexcept override { return 2; } // TODO
	virtual void execute_run() override;
	virtual void execute_set_input(int linenum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	static constexpr u8 ADDRESS_BITS = 16;
	static constexpr u8 EXTERNAL_ADDRESS_BITS = 24;

	address_space_config m_program_config;
	address_space_config m_data_config;
	memory_view m_inst_view;
	memory_access<ADDRESS_BITS, 3, -3, ENDIANNESS_BIG>::specific m_program;
	memory_access<EXTERNAL_ADDRESS_BITS, 2, -2, ENDIANNESS_BIG>::specific m_data;
	required_shared_ptr<u64> m_imem; // 16k x 48 bits
	int m_icount;
	u16 m_prev_pc;
	u16 m_pc; // 16 bits
	u16 m_next_pc; // 16 bits
	u64 m_ir;
	bool m_n;
	u32 m_a_mem[1024]; // 1024 x 32 bits
	u32 m_m_mem[32]; // 32 x 32 bits
	u32 m_a; // A bus, 32 bits
	u32 m_m; // M bus, 32 bits
	u32 m_q; // 32 bits
	u32 m_pdl[1024]; // 1024 x 32 bits
	u16 m_pdl_index; // 10 bits
	u16 m_pdl_pointer; // 10 bits
	u32 m_md; // 32 bits
	u32 m_read_delay;
	u32 m_read_data; // results of in-flight read operation
	u32 m_oa_reg_lo; // 26 bits
	u32 m_oa_reg_hi; // 22 bits
	u32 m_spc[32]; // 32 x 19 bits
	u8 m_spc_pointer; // 5 bits
	u32 m_vma; // 32 bits
	u8 m_vma_map_l1[0x800]; // 2048 x 5 bits
	u32 m_vma_map_l2[0x400]; // 1024 x 24 bits
	u32 m_dpc[0x800]; // 2048 x 17 bits
	u32 m_dispatch_constant; // 10 bits
	u32 m_ic; // interrupt control
	u32 m_lc; // 32 bits, 26 bits + interrupt enable bits
	u8 m_diag_mode;
	bool m_access_fault;
	bool m_write_fault;
	bool m_page_fault;
	bool m_popj;
	bool m_interrupt_pending;

	void program_map(address_map &map) ATTR_COLD;
	void write();
	void read();
	void execute_alu();
	void execute_jump();
	void execute_dispatch();
	void execute_byte();
	void get_m_source();
	void alu_operation(u32 &res, u32 &carry_out);
	void write_vma_map();
	u32 get_output(u32 m, u32 alu_out, u32 alu_carry);
	void write_destination(u32 output);
	bool jump_condition(s32 a, s32 m);
	void add32(u32 x, u32 y, u32 carry_in, u32 &res, u32 &carry_out);
	void sub32(u32 x, u32 y, u32 carry_in, u32 &res, u32 &carry_out);
	void push_spc(u16 data);
	u16 pop_spc();
	void instruction_stream();
};

DECLARE_DEVICE_TYPE(CADR, cadr_cpu_device);

#endif // MAME_CPU_CADR_CADR_H
