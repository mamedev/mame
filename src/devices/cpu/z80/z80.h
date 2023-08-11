// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#ifndef MAME_CPU_Z80_Z80_H
#define MAME_CPU_Z80_Z80_H

#pragma once

#include "machine/z80daisy.h"

enum
{
	NSC800_RSTA = INPUT_LINE_IRQ0 + 1,
	NSC800_RSTB,
	NSC800_RSTC,
	Z80_INPUT_LINE_WAIT,
	Z80_INPUT_LINE_BOGUSWAIT, /* WAIT pin implementation used to be nonexistent, please remove this when all drivers are updated with Z80_INPUT_LINE_WAIT */
	Z80_INPUT_LINE_BUSRQ
};

enum
{
	Z80_PC = STATE_GENPC, Z80_SP = 1,
	Z80_A, Z80_B, Z80_C, Z80_D, Z80_E, Z80_H, Z80_L,
	Z80_AF, Z80_BC, Z80_DE, Z80_HL,
	Z80_IX, Z80_IY, Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2,
	Z80_R, Z80_I, Z80_IM, Z80_IFF1, Z80_IFF2, Z80_HALT,
	Z80_DC0, Z80_DC1, Z80_DC2, Z80_DC3, Z80_WZ
};

class z80_device : public cpu_device, public z80_daisy_chain_interface
{
public:
	z80_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void z80_set_cycle_tables(const u8 *op, const u8 *cb, const u8 *ed, const u8 *xy, const u8 *xycb, const u8 *ex);
	void z80_set_m1_cycles(u8 m1_cycles) { m_m1_cycles = m1_cycles; }
	void z80_set_memrq_cycles(u8 memrq_cycles) { m_memrq_cycles = memrq_cycles; }
	void z80_set_iorq_cycles(u8 iorq_cycles) { m_iorq_cycles = iorq_cycles; }
	void z80_set_cycles_multiplier(u8 multiplier) { m_cycles_multiplier = multiplier; }
	template <typename... T> void set_memory_map(T &&... args) { set_addrmap(AS_PROGRAM, std::forward<T>(args)...); }
	template <typename... T> void set_m1_map(T &&... args) { set_addrmap(AS_OPCODES, std::forward<T>(args)...); }
	template <typename... T> void set_io_map(T &&... args) { set_addrmap(AS_IO, std::forward<T>(args)...); }
	auto irqack_cb() { return m_irqack_cb.bind(); }
	auto refresh_cb() { return m_refresh_cb.bind(); }
	auto nomreq_cb() { return m_nomreq_cb.bind(); }
	auto halt_cb() { return m_halt_cb.bind(); }

protected:
	using ops_type = std::vector<std::function<void()>>;

	enum op_prefix : u8
	{
		NONE = 0, CB, DD, ED, FD, XY_CB
	};

	class op_builder
	{
	public:
		op_builder (z80_device& z80) : m_z80(z80) {};

		ops_type get_steps() { return m_steps; }

		op_builder * foo(std::function<void()> step) { return this; } // discards content. needed to make macros balanced
		op_builder * add(ops_type steps)
		{
			assert(!steps.empty());
			auto to = m_if_condition == nullptr ? &m_steps : &m_if_steps;
			to->insert(to->end(), steps.begin(), steps.end());
			return this;
		}
		op_builder * add(std::function<void()> step)
		{
			auto to = m_if_condition == nullptr ? &m_steps : &m_if_steps;
			to->push_back(step);
			return this;
		}
		op_builder * call(std::function<ops_type()> steps) { return add(steps()); }
		op_builder * do_if(std::function<bool()> if_condition) { m_if_condition = if_condition;  return this; }
		op_builder * do_else() { assert(!m_if_steps.empty()); m_else_at = m_if_steps.size(); return this; }
		op_builder * edo()
		{
			assert(!m_if_steps.empty());
			auto cond = m_if_condition;
			m_if_condition = nullptr;
			z80_device& z80 = m_z80;
			if (m_else_at)
			{
				auto steps = m_else_at + 1;
				add([&z80, steps, cond](){ if (!cond()) { z80.m_cycle += steps; }});
				add({m_if_steps.begin(), m_if_steps.begin() + m_else_at});
				steps = m_if_steps.size() - m_else_at;
				add([&z80, steps](){ z80.m_cycle += steps; });
				add({m_if_steps.begin() + m_else_at, m_if_steps.end()});
				m_else_at = 0;
			}
			else
			{
				auto steps = m_if_steps.size();
				add([&z80, steps, cond](){ if (!cond()) { z80.m_cycle += steps; }});
				add(m_if_steps);
			}
			m_if_steps.clear();
  			return this;
		}
		ops_type jump(op_prefix prefix, u8 opcode)
		{ 
			z80_device& z80 = m_z80;
			add([&z80, prefix, opcode](){ z80.m_cycle = ~0; z80.m_prefix = prefix; z80.m_opcode = opcode; });
			return m_steps;
		}
		ops_type jump(u8 opcode) { return jump(NONE, opcode); }
		ops_type build() { add(m_z80.next_op()); return m_steps; }
	private:
		z80_device& m_z80;
		ops_type m_steps;
		std::function<bool()> m_if_condition = nullptr;
		ops_type m_if_steps;
		u8 m_else_at = 0;
	};

	z80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	void init_op_steps();

	// device_execute_interface overrides
	virtual bool cpu_is_interruptible() const override { return true; }
	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 16; }
	virtual u32 execute_input_lines() const noexcept override { return 4; }
	virtual u32 execute_default_irq_vector(int inputnum) const noexcept override { return 0xff; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual u32 translate_memory_address(u16 address) { return address; }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void calculate_icount();
	void execute_cycles(u8 icount);

	void halt();
	void leave_halt();

	virtual u8 data_read(u16 addr);
	virtual void data_write(u16 addr, u8 value);
	virtual u8 opcode_read();
	virtual u8 arg_read();

	/* deprecated */ void rm16(u16 addr, PAIR &r);
	/* deprecated */ void wm16_sp(PAIR &r);

	void illegal_1();
	void illegal_2();
	ops_type in();
	ops_type out();
	ops_type rm();
	ops_type rm_reg();
	ops_type rm16();
	ops_type wm();
	ops_type wm16();
	ops_type wm16_sp();
	ops_type rop();
	ops_type arg();
	ops_type arg16();

	ops_type eax();
	ops_type eay();
	ops_type push();
	ops_type pop();
	ops_type jp();
	ops_type jp_cond();
	ops_type jr();
	ops_type jr_cond(u8 opcode);
	ops_type call();
	ops_type call_cond(u8 opcode);
	ops_type ret_cond(u8 opcode);
	ops_type retn();
	ops_type reti();
	ops_type ld_r_a();
	ops_type ld_a_r();
	ops_type ld_i_a();
	ops_type ld_a_i();
	ops_type rst(u16 addr);
	void inc(u8 &r);
	void dec(u8 &r);
	void rlca();
	void rrca();
	void rla();
	void rra();
	ops_type rrd();
	ops_type rld();
	void add_a(u8 value);
	void adc_a(u8 value);
	void sub(u8 value);
	void sbc_a(u8 value);
	void neg();
	void daa();
	void and_a(u8 value);
	void or_a(u8 value);
	void xor_a(u8 value);
	void cp(u8 value);
	void exx();
	ops_type ex_sp();
	ops_type add16();
	ops_type adc_hl();
	ops_type sbc_hl();
	u8 rlc(u8 value);
	u8 rrc(u8 value);
	u8 rl(u8 value);
	u8 rr(u8 value);
	u8 sla(u8 value);
	u8 sra(u8 value);
	u8 sll(u8 value);
	u8 srl(u8 value);
	void bit(int bit, u8 value);
	void bit_hl(int bit, u8 value);
	void bit_xy(int bit, u8 value);
	u8 res(int bit, u8 value);
	u8 set(int bit, u8 value);
	ops_type ldi();
	ops_type cpi();
	ops_type ini();
	ops_type outi();
	ops_type ldd();
	ops_type cpd();
	ops_type ind();
	ops_type outd();
	ops_type ldir();
	ops_type cpir();
	ops_type inir();
	ops_type otir();
	ops_type lddr();
	ops_type cpdr();
	ops_type indr();
	ops_type otdr();
	void ei();

	ops_type next_op();
	virtual void check_interrupts();
	void take_interrupt();
	void take_nmi();
	ops_type nomreq_ir(s8 cycles);
	ops_type nomreq_addr(s8 cycles);

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_opcodes_config;
	const address_space_config m_io_config;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_args;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_opcodes;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;

	devcb_write_line m_irqack_cb;
	devcb_write8 m_refresh_cb;
	devcb_write8 m_nomreq_cb;
	devcb_write_line m_halt_cb;

	PAIR              m_prvpc;
	PAIR              m_pc;
	PAIR              m_sp;
	PAIR              m_af;
	PAIR              m_bc;
	PAIR              m_de;
	PAIR              m_hl;
	PAIR              m_ix;
	PAIR              m_iy;
	PAIR              m_wz;
	PAIR              m_af2;
	PAIR              m_bc2;
	PAIR              m_de2;
	PAIR              m_hl2;
	u8                m_r;
	u8                m_r2;
	u8                m_iff1;
	u8                m_iff2;
	u8                m_halt;
	u8                m_im;
	u8                m_i;
	u8                m_nmi_state;          /* nmi line state */
	u8                m_nmi_pending;        /* nmi pending */
	u8                m_irq_state;          /* irq line state */
	int               m_wait_state;         // wait line state
	int               m_busrq_state;        // bus request line state
	u8                m_after_ei;           /* are we in the EI shadow? */
	u8                m_after_ldair;        /* same, but for LD A,I or LD A,R */
	u32               m_ea;

	u8                m_cycle;
	op_prefix         m_prefix;
	u8                m_opcode;
	int               m_icount;
	int               m_icount_executing;
	PAIR16            m_m_shared_addr;
	PAIR16            m_m_shared_data;
	PAIR16            m_m_shared_data2;
	u8                m_rtemp;

	ops_type m_op_steps[6][0x100];
	const u8 *   m_cc_op;
	const u8 *   m_cc_cb;
	const u8 *   m_cc_ed;
	const u8 *   m_cc_xy;
	const u8 *   m_cc_xycb;
	const u8 *   m_cc_ex;

	u8 m_cycles_multiplier = 1; // multiplier for based cycles. deprecated to use except legacy with synthetic clock (e.g. "system1") till update.
	u8 m_m1_cycles = 4;
	u8 m_memrq_cycles = 3;
	u8 m_iorq_cycles = 4;
};

DECLARE_DEVICE_TYPE(Z80, z80_device)

class nsc800_device : public z80_device
{
public:
	nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_input_lines() const noexcept override { return 7; }
	virtual void execute_set_input(int inputnum, int state) override;

	virtual void check_interrupts() override;
	void take_interrupt_nsc800();
	u8 m_nsc800_irq_state[4]; /* state of NSC800 restart interrupts A, B, C */
};

DECLARE_DEVICE_TYPE(NSC800, nsc800_device)


#endif // MAME_CPU_Z80_Z80_H
