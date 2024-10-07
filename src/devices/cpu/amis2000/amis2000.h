// license:BSD-3-Clause
// copyright-holders:hap
/*

  AMI S2000-family MCU cores

*/

#ifndef MAME_CPU_AMIS2000_AMIS2000_H
#define MAME_CPU_AMIS2000_AMIS2000_H

#pragma once

class amis2000_base_device : public cpu_device
{
public:
	// generic input pins (4 bits each)
	auto read_k() { return m_read_k.bind(); }
	auto read_i() { return m_read_i.bind(); }

	// 8-bit external databus coupled as input/output pins
	auto read_d() { return m_read_d.bind(); }
	auto write_d() { return m_write_d.bind(); }

	// 13-bit external address bus coupled as output pins
	auto write_a() { return m_write_a.bind(); }

	// F_out pin (only for S2152)
	auto write_f() { return m_write_f.bind(); }

	// S2000 has a hardcoded 7seg table, that (unlike S2200) is officially
	// uncustomizable, but wildfire proves to be an exception to that rule.
	void set_7seg_table(const u8 *ptr) { m_7seg_table = ptr; } // d0=A, d1=B, etc.

	void data_64x4(address_map &map) ATTR_COLD;
	void data_80x4(address_map &map) ATTR_COLD;
	void program_1_5k(address_map &map) ATTR_COLD;
	void program_1k(address_map &map) ATTR_COLD;

protected:
	// construction/destruction
	amis2000_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 bu_bits, u8 callstack_bits, u8 callstack_depth, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; } // 4 cycles per machine cycle
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); } // "
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_icount;
	int m_state_count;

	u8 m_bu_bits;
	u16 m_bu_mask;
	u8 m_callstack_bits;  // number of program counter bits held in callstack
	u16 m_callstack_mask;
	u8 m_callstack_depth; // callstack levels: 3 on 2000/2150, 5 on 2200/2400
	u16 m_callstack[5];   // max 5
	u16 m_pc;             // 13-bit program counter
	u8 m_ppr;             // prepared page register (PP 1)
	u8 m_pbr;             // prepared bank register (PP 2)
	bool m_skip;          // skip next opcode, including PP prefixes
	u8 m_op;
	u8 m_prev_op;         // previous opcode, needed for PP, LAI, LB*
	u8 m_f;               // generic flags: 2 on 2000/2150, 6 on 2200/2400
	u8 m_carry;           // carry flag
	u8 m_bl;              // 4-bit ram index x
	u8 m_bu;              // 2/3-bit ram index y
	u8 m_acc;             // 4-bit accumulator
	u8 m_e;               // 4-bit generic register
	u8 m_ki_mask;         // 4-bit k/i-pins select latch
	u8 m_d;               // 8-bit d-pins latch
	bool m_d_active;      // d-pins available for direct i/o(floating), or outputting d-latch
	u8 m_d_polarity;      // invert d-latch output
	u16 m_a;              // 13-bit a-pins latch (master strobe latch)

	// i/o handlers
	const u8 *m_7seg_table;
	devcb_read8 m_read_k;
	devcb_read8 m_read_i;
	devcb_read8 m_read_d;
	devcb_write8 m_write_d;
	devcb_write16 m_write_a;
	devcb_write_line m_write_f;

	// misc internal helpers
	u8 ram_r();
	void ram_w(u8 data);
	void pop_callstack();
	void push_callstack();
	void d_latch_out(bool active);

	// opcode handlers
	virtual void op_lai();
	virtual void op_lab();
	virtual void op_lae();
	virtual void op_xab();
	virtual void op_xabu();
	virtual void op_xae();
	virtual void op_lbe();
	virtual void op_lbep();
	virtual void op_lbz();
	virtual void op_lbf();

	virtual void op_lam();
	virtual void op_xc();
	virtual void op_xci();
	virtual void op_xcd();
	virtual void op_stm();
	virtual void op_rsm();

	virtual void op_inp();
	virtual void op_out();
	virtual void op_disb();
	virtual void op_disn();
	virtual void op_mvs();
	virtual void op_psh();
	virtual void op_psl();
	virtual void op_eur();

	virtual void op_pp();
	virtual void op_jmp();
	virtual void op_jms();
	virtual void op_rt();
	virtual void op_rts();
	virtual void op_nop();
	virtual void op_halt();

	virtual void op_szc();
	virtual void op_szm();
	virtual void op_szi();
	virtual void op_szk();
	virtual void op_sbe();
	virtual void op_sam();
	virtual void op_sos();
	virtual void op_tf1();
	virtual void op_tf2();

	virtual void op_adcs();
	virtual void op_adis();
	virtual void op_add();
	virtual void op_and();
	virtual void op_xor();
	virtual void op_stc();
	virtual void op_rsc();
	virtual void op_cma();
	virtual void op_sf1();
	virtual void op_rf1();
	virtual void op_sf2();
	virtual void op_rf2();
};


class amis2000_cpu_device : public amis2000_base_device
{
public:
	amis2000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class amis2150_cpu_device : public amis2000_base_device
{
public:
	amis2150_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class amis2152_cpu_device : public amis2000_base_device
{
public:
	amis2152_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// digital-to-frequency converter
	u8 m_d2f_latch;
	emu_timer *m_d2f_timer;
	int m_fout_state;

	void d2f_timer_clock();
	TIMER_CALLBACK_MEMBER(d2f_timer_cb);

	// opcode handlers
	virtual void op_szk() override;
};


DECLARE_DEVICE_TYPE(AMI_S2000, amis2000_cpu_device)
DECLARE_DEVICE_TYPE(AMI_S2150, amis2150_cpu_device)
DECLARE_DEVICE_TYPE(AMI_S2152, amis2152_cpu_device)

#endif // MAME_CPU_AMIS2000_AMIS2000_H
