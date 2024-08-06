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
	Z80_INPUT_LINE_BOGUSWAIT, // WAIT pin implementation used to be nonexistent, please remove this when all drivers are updated with Z80_INPUT_LINE_WAIT
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

	void z80_set_m1_cycles(u8 m1_cycles) { m_m1_cycles = m1_cycles; }
	void z80_set_memrq_cycles(u8 memrq_cycles) { m_memrq_cycles = memrq_cycles; }
	void z80_set_iorq_cycles(u8 iorq_cycles) { m_iorq_cycles = iorq_cycles; }

	template <typename... T> void set_memory_map(T &&... args) { set_addrmap(AS_PROGRAM, std::forward<T>(args)...); }
	template <typename... T> void set_m1_map(T &&... args) { set_addrmap(AS_OPCODES, std::forward<T>(args)...); }
	template <typename... T> void set_io_map(T &&... args) { set_addrmap(AS_IO, std::forward<T>(args)...); }
	auto irqack_cb() { return m_irqack_cb.bind(); }
	auto refresh_cb() { return m_refresh_cb.bind(); }
	auto nomreq_cb() { return m_nomreq_cb.bind(); }
	auto halt_cb() { return m_halt_cb.bind(); }
	auto busack_cb() { return m_busack_cb.bind(); }
	auto branch_cb() { return m_branch_cb.bind(); }
	auto irqfetch_cb() { return m_irqfetch_cb.bind(); }
	auto reti_cb() { return m_reti_cb.bind(); }

protected:
	z80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual bool cpu_is_interruptible() const override { return true; }
	virtual u32 execute_min_cycles() const noexcept override { return 2; }
	virtual u32 execute_max_cycles() const noexcept override { return 16; }
	virtual u32 execute_input_lines() const noexcept override { return 4; }
	virtual u32 execute_default_irq_vector(int inputnum) const noexcept override { return 0xff; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;
	virtual u32 translate_memory_address(u16 address) { return address; }

	// device_state_interface implementation
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void illegal_1();
	void illegal_2();

	void halt();
	void leave_halt();
	void inc(u8 &r);
	void dec(u8 &r);
	void rlca();
	void rrca();
	void rla();
	void rra();
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
	void ei();
	void set_f(u8 f);
	void block_io_interrupted_flags();

	virtual void do_op();

	virtual u8 data_read(u16 addr);
	virtual void data_write(u16 addr, u8 value);
	virtual u8 stack_read(u16 addr) { return data_read(addr); }
	virtual void stack_write(u16 addr, u8 value) { return data_write(addr, value); }
	virtual u8 opcode_read();
	virtual u8 arg_read();

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
	devcb_write_line m_busack_cb;

	// Extra callbacks that do not map to any documented signals.
	// Used by derived classes to customise instruction behaviour.
	devcb_write_line m_branch_cb;
	devcb_write_line m_irqfetch_cb;
	devcb_write_line m_reti_cb;

	PAIR         m_prvpc;
	PAIR         m_pc;
	PAIR         m_sp;
	PAIR         m_af;
	PAIR         m_bc;
	PAIR         m_de;
	PAIR         m_hl;
	PAIR         m_ix;
	PAIR         m_iy;
	PAIR         m_wz;
	PAIR         m_af2;
	PAIR         m_bc2;
	PAIR         m_de2;
	PAIR         m_hl2;
	u8           m_qtemp;
	u8           m_q;
	u8           m_r;
	u8           m_r2;
	u8           m_iff1;
	u8           m_iff2;
	u8           m_halt;
	u8           m_im;
	u8           m_i;
	u8           m_nmi_state;          // nmi pin state
	bool         m_nmi_pending;        // nmi pending
	u8           m_irq_state;          // irq pin state
	int          m_wait_state;         // wait pin state
	int          m_busrq_state;        // bus request pin state
	u8           m_busack_state;       // bus acknowledge pin state
	bool         m_after_ei;           // are we in the EI shadow?
	bool         m_after_ldair;        // same, but for LD A,I or LD A,R
	u32          m_ea;

	int          m_icount;
	int          m_tmp_irq_vector;
	PAIR16       m_shared_addr;
	PAIR16       m_shared_data;
	PAIR16       m_shared_data2;
	u8           m_rtemp;

	u32 m_ref;
	u8 m_m1_cycles;
	u8 m_memrq_cycles;
	u8 m_iorq_cycles;

	static bool tables_initialised;
	static u8 SZ[0x100];       // zero and sign flags
	static u8 SZ_BIT[0x100];   // zero, sign and parity/overflow (=zero) flags for BIT opcode
	static u8 SZP[0x100];      // zero, sign and parity flags
	static u8 SZHV_inc[0x100]; // zero, sign, half carry and overflow flags INC r8
	static u8 SZHV_dec[0x100]; // zero, sign, half carry and overflow flags DEC r8

	static u8 SZHVC_add[2 * 0x100 * 0x100];
	static u8 SZHVC_sub[2 * 0x100 * 0x100];
};

DECLARE_DEVICE_TYPE(Z80, z80_device)

class nsc800_device : public z80_device
{
public:
	nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_input_lines() const noexcept override { return 7; }
	virtual void execute_set_input(int inputnum, int state) override;

	virtual void do_op() override;
	u8 m_nsc800_irq_state[4]; // state of NSC800 restart interrupts A, B, C
};

DECLARE_DEVICE_TYPE(NSC800, nsc800_device)


#endif // MAME_CPU_Z80_Z80_H
