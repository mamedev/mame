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
	z80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void z80_set_cycle_tables(const uint8_t *op, const uint8_t *cb, const uint8_t *ed, const uint8_t *xy, const uint8_t *xycb, const uint8_t *ex);
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
		NONE, CB, DD, ED, FD, XY_CB
	};

	z80_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	void init_instructions();

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 2; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 16; }
	virtual uint32_t execute_input_lines() const noexcept override { return 4; }
	virtual uint32_t execute_default_irq_vector(int inputnum) const noexcept override { return 0xff; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

#undef PROTOTYPES
#define PROTOTYPES(prefix) \
	ops_type prefix##_00; ops_type prefix##_01; ops_type prefix##_02; ops_type prefix##_03; \
	ops_type prefix##_04; ops_type prefix##_05; ops_type prefix##_06; ops_type prefix##_07; \
	ops_type prefix##_08; ops_type prefix##_09; ops_type prefix##_0a; ops_type prefix##_0b; \
	ops_type prefix##_0c; ops_type prefix##_0d; ops_type prefix##_0e; ops_type prefix##_0f; \
	ops_type prefix##_10; ops_type prefix##_11; ops_type prefix##_12; ops_type prefix##_13; \
	ops_type prefix##_14; ops_type prefix##_15; ops_type prefix##_16; ops_type prefix##_17; \
	ops_type prefix##_18; ops_type prefix##_19; ops_type prefix##_1a; ops_type prefix##_1b; \
	ops_type prefix##_1c; ops_type prefix##_1d; ops_type prefix##_1e; ops_type prefix##_1f; \
	ops_type prefix##_20; ops_type prefix##_21; ops_type prefix##_22; ops_type prefix##_23; \
	ops_type prefix##_24; ops_type prefix##_25; ops_type prefix##_26; ops_type prefix##_27; \
	ops_type prefix##_28; ops_type prefix##_29; ops_type prefix##_2a; ops_type prefix##_2b; \
	ops_type prefix##_2c; ops_type prefix##_2d; ops_type prefix##_2e; ops_type prefix##_2f; \
	ops_type prefix##_30; ops_type prefix##_31; ops_type prefix##_32; ops_type prefix##_33; \
	ops_type prefix##_34; ops_type prefix##_35; ops_type prefix##_36; ops_type prefix##_37; \
	ops_type prefix##_38; ops_type prefix##_39; ops_type prefix##_3a; ops_type prefix##_3b; \
	ops_type prefix##_3c; ops_type prefix##_3d; ops_type prefix##_3e; ops_type prefix##_3f; \
	ops_type prefix##_40; ops_type prefix##_41; ops_type prefix##_42; ops_type prefix##_43; \
	ops_type prefix##_44; ops_type prefix##_45; ops_type prefix##_46; ops_type prefix##_47; \
	ops_type prefix##_48; ops_type prefix##_49; ops_type prefix##_4a; ops_type prefix##_4b; \
	ops_type prefix##_4c; ops_type prefix##_4d; ops_type prefix##_4e; ops_type prefix##_4f; \
	ops_type prefix##_50; ops_type prefix##_51; ops_type prefix##_52; ops_type prefix##_53; \
	ops_type prefix##_54; ops_type prefix##_55; ops_type prefix##_56; ops_type prefix##_57; \
	ops_type prefix##_58; ops_type prefix##_59; ops_type prefix##_5a; ops_type prefix##_5b; \
	ops_type prefix##_5c; ops_type prefix##_5d; ops_type prefix##_5e; ops_type prefix##_5f; \
	ops_type prefix##_60; ops_type prefix##_61; ops_type prefix##_62; ops_type prefix##_63; \
	ops_type prefix##_64; ops_type prefix##_65; ops_type prefix##_66; ops_type prefix##_67; \
	ops_type prefix##_68; ops_type prefix##_69; ops_type prefix##_6a; ops_type prefix##_6b; \
	ops_type prefix##_6c; ops_type prefix##_6d; ops_type prefix##_6e; ops_type prefix##_6f; \
	ops_type prefix##_70; ops_type prefix##_71; ops_type prefix##_72; ops_type prefix##_73; \
	ops_type prefix##_74; ops_type prefix##_75; ops_type prefix##_76; ops_type prefix##_77; \
	ops_type prefix##_78; ops_type prefix##_79; ops_type prefix##_7a; ops_type prefix##_7b; \
	ops_type prefix##_7c; ops_type prefix##_7d; ops_type prefix##_7e; ops_type prefix##_7f; \
	ops_type prefix##_80; ops_type prefix##_81; ops_type prefix##_82; ops_type prefix##_83; \
	ops_type prefix##_84; ops_type prefix##_85; ops_type prefix##_86; ops_type prefix##_87; \
	ops_type prefix##_88; ops_type prefix##_89; ops_type prefix##_8a; ops_type prefix##_8b; \
	ops_type prefix##_8c; ops_type prefix##_8d; ops_type prefix##_8e; ops_type prefix##_8f; \
	ops_type prefix##_90; ops_type prefix##_91; ops_type prefix##_92; ops_type prefix##_93; \
	ops_type prefix##_94; ops_type prefix##_95; ops_type prefix##_96; ops_type prefix##_97; \
	ops_type prefix##_98; ops_type prefix##_99; ops_type prefix##_9a; ops_type prefix##_9b; \
	ops_type prefix##_9c; ops_type prefix##_9d; ops_type prefix##_9e; ops_type prefix##_9f; \
	ops_type prefix##_a0; ops_type prefix##_a1; ops_type prefix##_a2; ops_type prefix##_a3; \
	ops_type prefix##_a4; ops_type prefix##_a5; ops_type prefix##_a6; ops_type prefix##_a7; \
	ops_type prefix##_a8; ops_type prefix##_a9; ops_type prefix##_aa; ops_type prefix##_ab; \
	ops_type prefix##_ac; ops_type prefix##_ad; ops_type prefix##_ae; ops_type prefix##_af; \
	ops_type prefix##_b0; ops_type prefix##_b1; ops_type prefix##_b2; ops_type prefix##_b3; \
	ops_type prefix##_b4; ops_type prefix##_b5; ops_type prefix##_b6; ops_type prefix##_b7; \
	ops_type prefix##_b8; ops_type prefix##_b9; ops_type prefix##_ba; ops_type prefix##_bb; \
	ops_type prefix##_bc; ops_type prefix##_bd; ops_type prefix##_be; ops_type prefix##_bf; \
	ops_type prefix##_c0; ops_type prefix##_c1; ops_type prefix##_c2; ops_type prefix##_c3; \
	ops_type prefix##_c4; ops_type prefix##_c5; ops_type prefix##_c6; ops_type prefix##_c7; \
	ops_type prefix##_c8; ops_type prefix##_c9; ops_type prefix##_ca; ops_type prefix##_cb; \
	ops_type prefix##_cc; ops_type prefix##_cd; ops_type prefix##_ce; ops_type prefix##_cf; \
	ops_type prefix##_d0; ops_type prefix##_d1; ops_type prefix##_d2; ops_type prefix##_d3; \
	ops_type prefix##_d4; ops_type prefix##_d5; ops_type prefix##_d6; ops_type prefix##_d7; \
	ops_type prefix##_d8; ops_type prefix##_d9; ops_type prefix##_da; ops_type prefix##_db; \
	ops_type prefix##_dc; ops_type prefix##_dd; ops_type prefix##_de; ops_type prefix##_df; \
	ops_type prefix##_e0; ops_type prefix##_e1; ops_type prefix##_e2; ops_type prefix##_e3; \
	ops_type prefix##_e4; ops_type prefix##_e5; ops_type prefix##_e6; ops_type prefix##_e7; \
	ops_type prefix##_e8; ops_type prefix##_e9; ops_type prefix##_ea; ops_type prefix##_eb; \
	ops_type prefix##_ec; ops_type prefix##_ed; ops_type prefix##_ee; ops_type prefix##_ef; \
	ops_type prefix##_f0; ops_type prefix##_f1; ops_type prefix##_f2; ops_type prefix##_f3; \
	ops_type prefix##_f4; ops_type prefix##_f5; ops_type prefix##_f6; ops_type prefix##_f7; \
	ops_type prefix##_f8; ops_type prefix##_f9; ops_type prefix##_fa; ops_type prefix##_fb; \
	ops_type prefix##_fc; ops_type prefix##_fd; ops_type prefix##_fe; ops_type prefix##_ff;

	void illegal_1();
	ops_type illegal_1(ops_type ref); // TODO rm
	void illegal_2();

	PROTOTYPES(op)
	PROTOTYPES(cb)
	PROTOTYPES(dd)
	PROTOTYPES(ed)
	PROTOTYPES(fd)
	PROTOTYPES(xycb)

	void halt();
	void leave_halt();
	uint8_t in(uint16_t port);
	void out(uint16_t port, uint8_t value);
	virtual uint8_t rm(uint16_t addr);
	uint8_t rm_reg(uint16_t addr);
	void rm16(uint16_t addr, PAIR &r);
	virtual void wm(uint16_t addr, uint8_t value);
	void wm16(uint16_t addr, PAIR &r);
	void wm16_sp(PAIR &r);
	virtual uint8_t rop();
	virtual uint8_t arg();
	virtual uint16_t arg16();
	ops_type arg16_n();
	void eax();
	void eay();
	void pop(PAIR &r);
	void push(PAIR &r);
	void jp(void);
	void jp_cond(bool cond);
	void jr();
	void jr_cond(bool cond, uint8_t opcode);
	void call();
	void call_cond(bool cond, uint8_t opcode);
	void ret_cond(bool cond, uint8_t opcode);
	void retn();
	void reti();
	void ld_r_a();
	void ld_a_r();
	void ld_i_a();
	void ld_a_i();
	void rst(uint16_t addr);
	uint8_t inc(uint8_t value);
	uint8_t dec(uint8_t value);
	void rlca();
	void rrca();
	void rla();
	void rra();
	void rrd();
	void rld();
	void add_a(uint8_t value);
	void adc_a(uint8_t value);
	void sub(uint8_t value);
	void sbc_a(uint8_t value);
	void neg();
	void daa();
	void and_a(uint8_t value);
	void or_a(uint8_t value);
	void xor_a(uint8_t value);
	void cp(uint8_t value);
	void ex_af();
	void ex_de_hl();
	void exx();
	void ex_sp(PAIR &r);
	void add16(PAIR &dr, PAIR &sr);
	void adc_hl(PAIR &r);
	void sbc_hl(PAIR &r);
	uint8_t rlc(uint8_t value);
	uint8_t rrc(uint8_t value);
	uint8_t rl(uint8_t value);
	uint8_t rr(uint8_t value);
	uint8_t sla(uint8_t value);
	uint8_t sra(uint8_t value);
	uint8_t sll(uint8_t value);
	uint8_t srl(uint8_t value);
	void bit(int bit, uint8_t value);
	void bit_hl(int bit, uint8_t value);
	void bit_xy(int bit, uint8_t value);
	uint8_t res(int bit, uint8_t value);
	uint8_t set(int bit, uint8_t value);
	void ldi();
	void cpi();
	void ini();
	void outi();
	void ldd();
	void cpd();
	void ind();
	void outd();
	void ldir();
	void cpir();
	void inir();
	void otir();
	void lddr();
	void cpdr();
	void indr();
	void otdr();
	void ei();

	ops_type ops_flat(std::vector<ops_type> op_map);
	ops_type * do_exec();
	ops_type next_op();
	void calculate_icount();
	virtual void check_interrupts();
	void take_interrupt();
	void take_nmi();
	void nomreq_ir(s8 cycles);
	void nomreq_addr(u16 addr, s8 cycles);

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
	uint8_t           m_r;
	uint8_t           m_r2;
	uint8_t           m_iff1;
	uint8_t           m_iff2;
	uint8_t           m_halt;
	uint8_t           m_im;
	uint8_t           m_i;
	uint8_t           m_nmi_state;          /* nmi line state */
	uint8_t           m_nmi_pending;        /* nmi pending */
	uint8_t           m_irq_state;          /* irq line state */
	int               m_wait_state;         // wait line state
	int               m_busrq_state;        // bus request line state
	uint8_t           m_after_ei;           /* are we in the EI shadow? */
	uint8_t           m_after_ldair;        /* same, but for LD A,I or LD A,R */
	uint32_t          m_ea;

int	tmp_max_overlap = 0; // TODO tmp
	u8                m_cycle;
	op_prefix         m_prefix;
	op_prefix         m_prefix_next;
	u8                m_opcode;
	int               m_icount;
	int               m_icount_executing;
	PAIR              m_m_shared;
	uint8_t           m_rtemp;
	const uint8_t *   m_cc_op;
	const uint8_t *   m_cc_cb;
	const uint8_t *   m_cc_ed;
	const uint8_t *   m_cc_xy;
	const uint8_t *   m_cc_xycb;
	const uint8_t *   m_cc_ex;
};

DECLARE_DEVICE_TYPE(Z80, z80_device)

class nsc800_device : public z80_device
{
public:
	nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_input_lines() const noexcept override { return 7; }
	virtual void execute_set_input(int inputnum, int state) override;

	virtual void check_interrupts() override;
	void take_interrupt_nsc800();
	uint8_t m_nsc800_irq_state[4]; /* state of NSC800 restart interrupts A, B, C */
};

DECLARE_DEVICE_TYPE(NSC800, nsc800_device)


#endif // MAME_CPU_Z80_Z80_H
