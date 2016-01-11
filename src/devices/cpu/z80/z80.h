// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#pragma once

#ifndef __Z80_H__
#define __Z80_H__

#include "z80daisy.h"

#define MCFG_Z80_SET_IRQACK_CALLBACK(_devcb) \
	devcb = &z80_device::set_irqack_cb(*device, DEVCB_##_devcb);

#define MCFG_Z80_SET_REFRESH_CALLBACK(_devcb) \
	devcb = &z80_device::set_refresh_cb(*device, DEVCB_##_devcb);

enum
{
	NSC800_RSTA = INPUT_LINE_IRQ0 + 1,
	NSC800_RSTB,
	NSC800_RSTC,
	Z80_INPUT_LINE_WAIT,
	Z80_INPUT_LINE_BUSRQ
};

enum
{
	Z80_PC, Z80_SP,
	Z80_A, Z80_B, Z80_C, Z80_D, Z80_E, Z80_H, Z80_L,
	Z80_AF, Z80_BC, Z80_DE, Z80_HL,
	Z80_IX, Z80_IY, Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2,
	Z80_R, Z80_I, Z80_IM, Z80_IFF1, Z80_IFF2, Z80_HALT,
	Z80_DC0, Z80_DC1, Z80_DC2, Z80_DC3, Z80_WZ,

	Z80_GENPC = STATE_GENPC,
	Z80_GENSP = STATE_GENSP,
	Z80_GENPCBASE = STATE_GENPCBASE
};

class z80_device : public cpu_device
{
public:
	z80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( irq_line );

	void z80_set_cycle_tables(const UINT8 *op, const UINT8 *cb, const UINT8 *ed, const UINT8 *xy, const UINT8 *xycb, const UINT8 *ex);
	template<class _Object> static devcb_base &set_irqack_cb(device_t &device, _Object object) { return downcast<z80_device &>(device).m_irqack_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_refresh_cb(device_t &device, _Object object) { return downcast<z80_device &>(device).m_refresh_cb.set_callback(object); }

protected:
	z80_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 2; }
	virtual UINT32 execute_max_cycles() const override { return 16; }
	virtual UINT32 execute_input_lines() const override { return 4; }
	virtual UINT32 execute_default_irq_vector() const override { return 0xff; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

#undef PROTOTYPES
#define PROTOTYPES(prefix) \
	void prefix##_00(); void prefix##_01(); void prefix##_02(); void prefix##_03(); \
	void prefix##_04(); void prefix##_05(); void prefix##_06(); void prefix##_07(); \
	void prefix##_08(); void prefix##_09(); void prefix##_0a(); void prefix##_0b(); \
	void prefix##_0c(); void prefix##_0d(); void prefix##_0e(); void prefix##_0f(); \
	void prefix##_10(); void prefix##_11(); void prefix##_12(); void prefix##_13(); \
	void prefix##_14(); void prefix##_15(); void prefix##_16(); void prefix##_17(); \
	void prefix##_18(); void prefix##_19(); void prefix##_1a(); void prefix##_1b(); \
	void prefix##_1c(); void prefix##_1d(); void prefix##_1e(); void prefix##_1f(); \
	void prefix##_20(); void prefix##_21(); void prefix##_22(); void prefix##_23(); \
	void prefix##_24(); void prefix##_25(); void prefix##_26(); void prefix##_27(); \
	void prefix##_28(); void prefix##_29(); void prefix##_2a(); void prefix##_2b(); \
	void prefix##_2c(); void prefix##_2d(); void prefix##_2e(); void prefix##_2f(); \
	void prefix##_30(); void prefix##_31(); void prefix##_32(); void prefix##_33(); \
	void prefix##_34(); void prefix##_35(); void prefix##_36(); void prefix##_37(); \
	void prefix##_38(); void prefix##_39(); void prefix##_3a(); void prefix##_3b(); \
	void prefix##_3c(); void prefix##_3d(); void prefix##_3e(); void prefix##_3f(); \
	void prefix##_40(); void prefix##_41(); void prefix##_42(); void prefix##_43(); \
	void prefix##_44(); void prefix##_45(); void prefix##_46(); void prefix##_47(); \
	void prefix##_48(); void prefix##_49(); void prefix##_4a(); void prefix##_4b(); \
	void prefix##_4c(); void prefix##_4d(); void prefix##_4e(); void prefix##_4f(); \
	void prefix##_50(); void prefix##_51(); void prefix##_52(); void prefix##_53(); \
	void prefix##_54(); void prefix##_55(); void prefix##_56(); void prefix##_57(); \
	void prefix##_58(); void prefix##_59(); void prefix##_5a(); void prefix##_5b(); \
	void prefix##_5c(); void prefix##_5d(); void prefix##_5e(); void prefix##_5f(); \
	void prefix##_60(); void prefix##_61(); void prefix##_62(); void prefix##_63(); \
	void prefix##_64(); void prefix##_65(); void prefix##_66(); void prefix##_67(); \
	void prefix##_68(); void prefix##_69(); void prefix##_6a(); void prefix##_6b(); \
	void prefix##_6c(); void prefix##_6d(); void prefix##_6e(); void prefix##_6f(); \
	void prefix##_70(); void prefix##_71(); void prefix##_72(); void prefix##_73(); \
	void prefix##_74(); void prefix##_75(); void prefix##_76(); void prefix##_77(); \
	void prefix##_78(); void prefix##_79(); void prefix##_7a(); void prefix##_7b(); \
	void prefix##_7c(); void prefix##_7d(); void prefix##_7e(); void prefix##_7f(); \
	void prefix##_80(); void prefix##_81(); void prefix##_82(); void prefix##_83(); \
	void prefix##_84(); void prefix##_85(); void prefix##_86(); void prefix##_87(); \
	void prefix##_88(); void prefix##_89(); void prefix##_8a(); void prefix##_8b(); \
	void prefix##_8c(); void prefix##_8d(); void prefix##_8e(); void prefix##_8f(); \
	void prefix##_90(); void prefix##_91(); void prefix##_92(); void prefix##_93(); \
	void prefix##_94(); void prefix##_95(); void prefix##_96(); void prefix##_97(); \
	void prefix##_98(); void prefix##_99(); void prefix##_9a(); void prefix##_9b(); \
	void prefix##_9c(); void prefix##_9d(); void prefix##_9e(); void prefix##_9f(); \
	void prefix##_a0(); void prefix##_a1(); void prefix##_a2(); void prefix##_a3(); \
	void prefix##_a4(); void prefix##_a5(); void prefix##_a6(); void prefix##_a7(); \
	void prefix##_a8(); void prefix##_a9(); void prefix##_aa(); void prefix##_ab(); \
	void prefix##_ac(); void prefix##_ad(); void prefix##_ae(); void prefix##_af(); \
	void prefix##_b0(); void prefix##_b1(); void prefix##_b2(); void prefix##_b3(); \
	void prefix##_b4(); void prefix##_b5(); void prefix##_b6(); void prefix##_b7(); \
	void prefix##_b8(); void prefix##_b9(); void prefix##_ba(); void prefix##_bb(); \
	void prefix##_bc(); void prefix##_bd(); void prefix##_be(); void prefix##_bf(); \
	void prefix##_c0(); void prefix##_c1(); void prefix##_c2(); void prefix##_c3(); \
	void prefix##_c4(); void prefix##_c5(); void prefix##_c6(); void prefix##_c7(); \
	void prefix##_c8(); void prefix##_c9(); void prefix##_ca(); void prefix##_cb(); \
	void prefix##_cc(); void prefix##_cd(); void prefix##_ce(); void prefix##_cf(); \
	void prefix##_d0(); void prefix##_d1(); void prefix##_d2(); void prefix##_d3(); \
	void prefix##_d4(); void prefix##_d5(); void prefix##_d6(); void prefix##_d7(); \
	void prefix##_d8(); void prefix##_d9(); void prefix##_da(); void prefix##_db(); \
	void prefix##_dc(); void prefix##_dd(); void prefix##_de(); void prefix##_df(); \
	void prefix##_e0(); void prefix##_e1(); void prefix##_e2(); void prefix##_e3(); \
	void prefix##_e4(); void prefix##_e5(); void prefix##_e6(); void prefix##_e7(); \
	void prefix##_e8(); void prefix##_e9(); void prefix##_ea(); void prefix##_eb(); \
	void prefix##_ec(); void prefix##_ed(); void prefix##_ee(); void prefix##_ef(); \
	void prefix##_f0(); void prefix##_f1(); void prefix##_f2(); void prefix##_f3(); \
	void prefix##_f4(); void prefix##_f5(); void prefix##_f6(); void prefix##_f7(); \
	void prefix##_f8(); void prefix##_f9(); void prefix##_fa(); void prefix##_fb(); \
	void prefix##_fc(); void prefix##_fd(); void prefix##_fe(); void prefix##_ff();

	void illegal_1();
	void illegal_2();

	PROTOTYPES(op)
	PROTOTYPES(cb)
	PROTOTYPES(dd)
	PROTOTYPES(ed)
	PROTOTYPES(fd)
	PROTOTYPES(xycb)

	void halt();
	void leave_halt();
	UINT8 in(UINT16 port);
	void out(UINT16 port, UINT8 value);
	UINT8 rm(UINT16 addr);
	void rm16(UINT16 addr, PAIR &r);
	void wm(UINT16 addr, UINT8 value);
	void wm16(UINT16 addr, PAIR &r);
	UINT8 rop();
	UINT8 arg();
	UINT16 arg16();
	void eax();
	void eay();
	void pop(PAIR &r);
	void push(PAIR &r);
	void jp(void);
	void jp_cond(bool cond);
	void jr();
	void jr_cond(bool cond, UINT8 opcode);
	void call();
	void call_cond(bool cond, UINT8 opcode);
	void ret_cond(bool cond, UINT8 opcode);
	void retn();
	void reti();
	void ld_r_a();
	void ld_a_r();
	void ld_i_a();
	void ld_a_i();
	void rst(UINT16 addr);
	UINT8 inc(UINT8 value);
	UINT8 dec(UINT8 value);
	void rlca();
	void rrca();
	void rla();
	void rra();
	void rrd();
	void rld();
	void add_a(UINT8 value);
	void adc_a(UINT8 value);
	void sub(UINT8 value);
	void sbc_a(UINT8 value);
	void neg();
	void daa();
	void and_a(UINT8 value);
	void or_a(UINT8 value);
	void xor_a(UINT8 value);
	void cp(UINT8 value);
	void ex_af();
	void ex_de_hl();
	void exx();
	void ex_sp(PAIR &r);
	void add16(PAIR &dr, PAIR &sr);
	void adc_hl(PAIR &r);
	void sbc_hl(PAIR &r);
	UINT8 rlc(UINT8 value);
	UINT8 rrc(UINT8 value);
	UINT8 rl(UINT8 value);
	UINT8 rr(UINT8 value);
	UINT8 sla(UINT8 value);
	UINT8 sra(UINT8 value);
	UINT8 sll(UINT8 value);
	UINT8 srl(UINT8 value);
	void bit(int bit, UINT8 value);
	void bit_hl(int bit, UINT8 value);
	void bit_xy(int bit, UINT8 value);
	UINT8 res(int bit, UINT8 value);
	UINT8 set(int bit, UINT8 value);
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

	void take_interrupt();

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_decrypted_opcodes_config;
	const address_space_config m_io_config;
	address_space *m_program;
	address_space *m_decrypted_opcodes;
	address_space *m_io;
	direct_read_data *m_direct;
	direct_read_data *m_decrypted_opcodes_direct;
	devcb_write_line m_irqack_cb;
	devcb_write16 m_refresh_cb;

	PAIR            m_prvpc;
	PAIR            m_pc;
	PAIR            m_sp;
	PAIR            m_af;
	PAIR            m_bc;
	PAIR            m_de;
	PAIR            m_hl;
	PAIR            m_ix;
	PAIR            m_iy;
	PAIR            m_wz;
	PAIR            m_af2;
	PAIR            m_bc2;
	PAIR            m_de2;
	PAIR            m_hl2;
	UINT8           m_r;
	UINT8           m_r2;
	UINT8           m_iff1;
	UINT8           m_iff2;
	UINT8           m_halt;
	UINT8           m_im;
	UINT8           m_i;
	UINT8           m_nmi_state;          /* nmi line state */
	UINT8           m_nmi_pending;        /* nmi pending */
	UINT8           m_irq_state;          /* irq line state */
	int             m_wait_state;         // wait line state
	int             m_busrq_state;        // bus request line state
	UINT8           m_after_ei;           /* are we in the EI shadow? */
	UINT8           m_after_ldair;        /* same, but for LD A,I or LD A,R */
	UINT32          m_ea;
	device_irq_acknowledge_delegate m_irq_callback;

	int             m_icount;
	z80_daisy_chain m_daisy;
	UINT8           m_rtemp;
	const UINT8 *   m_cc_op;
	const UINT8 *   m_cc_cb;
	const UINT8 *   m_cc_ed;
	const UINT8 *   m_cc_xy;
	const UINT8 *   m_cc_xycb;
	const UINT8 *   m_cc_ex;
};

extern const device_type Z80;

class nsc800_device : public z80_device
{
public:
	nsc800_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_input_lines() const override { return 7; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	void take_interrupt_nsc800();
	UINT8 m_nsc800_irq_state[4]; /* state of NSC800 restart interrupts A, B, C */
};

extern const device_type NSC800;


#endif /* __Z80_H__ */
