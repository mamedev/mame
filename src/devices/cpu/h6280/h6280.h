// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*****************************************************************************

    h6280.h Portable Hu6280 emulator interface

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.

******************************************************************************/

#ifndef MAME_CPU_H6280_H6280_H
#define MAME_CPU_H6280_H6280_H

#pragma once

#include "sound/c6280.h"

#define H6280_LAZY_FLAGS  0

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> h6280_device

// Used by core CPU interface
class h6280_device : public cpu_device, public device_mixer_interface
{
public:
	// construction/destruction
	h6280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// public interfaces
	void set_irq_line(int irqline, int state);

	// configuration
	auto port_in_cb() { return m_port_in_cb.bind(); } // K0-7 at Pinout
	auto port_out_cb() { return m_port_out_cb.bind(); } // O0-7 at Pinout

	// hack to fix music speed in some Data East games (core bug, external strapping, DE 45 customization or ???)
	void set_timer_scale(int scale) { m_timer_scale = scale; }

	/* functions for use by the PSG and joypad port only! */
	uint8_t io_get_buffer();
	void io_set_buffer(uint8_t);

protected:
	// register enumeration
	enum
	{
		H6280_PC = 1,
		H6280_S,
		H6280_P,
		H6280_A,
		H6280_X,
		H6280_Y,
		H6280_IRQ_MASK,
		H6280_TIMER_STATE,
		H6280_NMI_STATE,
		H6280_IRQ1_STATE,
		H6280_IRQ2_STATE,
		H6280_IRQT_STATE,
		H6280_MPR0,
		H6280_MPR1,
		H6280_MPR2,
		H6280_MPR3,
		H6280_MPR4,
		H6280_MPR5,
		H6280_MPR6,
		H6280_MPR7
	};

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// opcode accessors
	uint8_t program_read8(offs_t addr);
	void program_write8(offs_t addr, uint8_t data);
	uint8_t program_read8z(offs_t addr);
	void program_write8z(offs_t addr, uint8_t data);
	uint16_t program_read16(offs_t addr);
	uint16_t program_read16z(offs_t addr);
	void push(uint8_t value);
	void pull(uint8_t &value);
	uint8_t read_opcode();
	uint8_t read_opcode_arg();

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

	PROTOTYPES(op)

	uint32_t translated(uint16_t addr);
	void h6280_cycles(int cyc);
	void set_nz(uint8_t n);
	void clear_t();
	void do_interrupt(uint16_t vector);
	void check_and_take_irq_lines();
	void check_irq_lines();
	void check_vdc_vce_penalty(uint16_t addr);
	void bra(bool cond);
	void ea_zpg();
	void ea_tflg();
	void ea_zpx();
	void ea_zpy();
	void ea_abs();
	void ea_abx();
	void ea_aby();
	void ea_zpi();
	void ea_idx();
	void ea_idy();
	void ea_ind();
	void ea_iax();
	uint8_t rd_imm();
	uint8_t rd_zpg();
	uint8_t rd_zpx();
	uint8_t rd_zpy();
	uint8_t rd_abs();
	uint8_t rd_abx();
	uint8_t rd_aby();
	uint8_t rd_zpi();
	uint8_t rd_idx();
	uint8_t rd_idy();
	uint8_t rd_tfl();
	void wr_zpg(uint8_t tmp);
	void wr_zpx(uint8_t tmp);
	void wr_zpy(uint8_t tmp);
	void wr_abs(uint8_t tmp);
	void wr_abx(uint8_t tmp);
	void wr_aby(uint8_t tmp);
	void wr_zpi(uint8_t tmp);
	void wr_idx(uint8_t tmp);
	void wr_idy(uint8_t tmp);
	void wb_ea(uint8_t tmp);
	void wb_eaz(uint8_t tmp);
	void compose_p(uint8_t set, uint8_t clr);
	void tadc(uint8_t tmp);
	void adc(uint8_t tmp);
	void tand(uint8_t tmp);
	void and_a(uint8_t tmp);
	uint8_t asl(uint8_t tmp);
	void bbr(int bit, uint8_t tmp);
	void bbs(int bit, uint8_t tmp);
	void bcc();
	void bcs();
	void beq();
	void bit(uint8_t tmp);
	void bmi();
	void bne();
	void bpl();
	void brk();
	void bsr();
	void bvc();
	void bvs();
	void cla();
	void clc();
	void cld();
	void cli();
	void clv();
	void clx();
	void cly();
	void cmp(uint8_t tmp);
	void cpx(uint8_t tmp);
	void cpy(uint8_t tmp);
	uint8_t dec(uint8_t tmp);
	void dex();
	void dey();
	void teor(uint8_t tmp);
	void eor(uint8_t tmp);
	uint8_t inc(uint8_t tmp);
	void inx();
	void iny();
	void jmp();
	void jsr();
	void lda(uint8_t tmp);
	void ldx(uint8_t tmp);
	void ldy(uint8_t tmp);
	uint8_t lsr(uint8_t tmp);
	void nop();
	void tora(uint8_t tmp);
	void ora(uint8_t tmp);
	void pha();
	void php();
	void phx();
	void phy();
	void pla();
	void plp();
	void plx();
	void ply();
	uint8_t rmb(int bit, uint8_t tmp);
	uint8_t rol(uint8_t tmp);
	uint8_t ror(uint8_t tmp);
	void rti();
	void rts();
	void sax();
	void say();
	void tsbc(uint8_t tmp);
	void sbc(uint8_t tmp);
	void sec();
	void sed();
	void sei();
	void set();
	uint8_t smb(int bit, uint8_t tmp);
	void st0(uint8_t tmp);
	void st1(uint8_t tmp);
	void st2(uint8_t tmp);
	uint8_t sta();
	uint8_t stx();
	uint8_t sty();
	uint8_t stz();
	void sxy();
	void tai();
	void tam(uint8_t tmp);
	void tax();
	void tay();
	void tdd();
	void tia();
	void tii();
	void tin();
	void tma(uint8_t tmp);
	uint8_t trb(uint8_t tmp);
	uint8_t tsb(uint8_t tmp);
	void tsx();
	void tst(uint8_t imm, uint8_t tmp);
	void txa();
	void txs();
	void tya();
	void csh();
	void csl();

	enum
	{
		H6280_RESET_VEC = 0xfffe,
		H6280_NMI_VEC =  0xfffc,
		H6280_TIMER_VEC = 0xfffa,
		H6280_IRQ1_VEC = 0xfff8,
		H6280_IRQ2_VEC = 0xfff6 /* Aka BRK vector */
	};

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_io_config;

	// CPU registers
	PAIR  m_ppc;            /* previous program counter */
	PAIR  m_pc;             /* program counter */
	PAIR  m_sp;             /* stack pointer (always 100 - 1FF) */
	PAIR  m_zp;             /* zero page address */
	PAIR  m_ea;             /* effective address */
	uint8_t m_a;              /* Accumulator */
	uint8_t m_x;              /* X index register */
	uint8_t m_y;              /* Y index register */
	uint8_t m_p;              /* Processor status */
	uint8_t m_mmr[8];         /* Hu6280 memory mapper registers */
	uint8_t m_irq_mask;       /* interrupt enable/disable */
	uint8_t m_timer_status;   /* timer status */
	uint8_t m_timer_ack;      /* timer acknowledge */
	uint8_t m_clocks_per_cycle; /* 4 = low speed mode, 1 = high speed mode */
	int32_t m_timer_value;    /* timer interrupt */
	int32_t m_timer_load;     /* reload value */
	uint8_t m_nmi_state;
	uint8_t m_irq_state[3];
	uint8_t m_irq_pending;
#if H6280_LAZY_FLAGS
	int32_t m_nz;         /* last value (lazy N and Z flag) */
#endif
	uint8_t m_io_buffer;  /* last value written to the PSG, timer, and interrupt pages */

	// internal registers
	void internal_map(address_map &map) ATTR_COLD;
	uint8_t irq_status_r(offs_t offset);
	void irq_status_w(offs_t offset, uint8_t data);

	uint8_t timer_r();
	void timer_w(offs_t offset, uint8_t data);

	uint8_t port_r();
	void port_w(uint8_t data);

	uint8_t io_buffer_r();
	void psg_w(offs_t offset, uint8_t data);

	devcb_read8 m_port_in_cb;
	devcb_write8 m_port_out_cb;

	required_device<c6280_device> m_psg;

	// other internal states
	int m_icount;

	uint8_t m_timer_scale;

	// address spaces
	memory_access<21, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<21, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access< 2, 0, 0, ENDIANNESS_LITTLE>::specific m_io;

	typedef void (h6280_device::*ophandler)();

	ophandler m_opcode[256];

	static const ophandler s_opcodetable[256];
};

DECLARE_DEVICE_TYPE(H6280, h6280_device)

#endif // MAME_CPU_H6280_H6280_H
