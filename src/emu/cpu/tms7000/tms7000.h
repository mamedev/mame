/*****************************************************************************
 *
 *   tms7000.h (c header file)
 *   Portable TMS7000 emulator (Texas Instruments 7000)
 *
 *   Copyright tim lindner, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     tlindner@macmess.org
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/

#pragma once

#ifndef __TMS7000_H__
#define __TMS7000_H__


enum { TMS7000_PC=1, TMS7000_SP, TMS7000_ST, TMS7000_IDLE, TMS7000_T1_CL, TMS7000_T1_PS, TMS7000_T1_DEC };

enum { TMS7000_VCC, TMS7000_VSS };

enum { TMS7000_NMOS, TMS7000_CMOS };

enum
{
	TMS7000_IRQ1_LINE = 0,   /* INT1 */
	TMS7000_IRQ2_LINE,       /* INT2 */
	TMS7000_IRQ3_LINE,       /* INT3 */
	TMS7000_IRQNONE = 255
};

enum
{
	TMS7000_PORTA = 0,
	TMS7000_PORTB,
	TMS7000_PORTC,
	TMS7000_PORTD
};


class tms7000_device : public cpu_device
{
public:
	// construction/destruction
	tms7000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms7000_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_WRITE8_MEMBER( tms70x0_pf_w );
	DECLARE_READ8_MEMBER( tms70x0_pf_r );
	DECLARE_WRITE8_MEMBER( tms7000_internal_w );
	DECLARE_READ8_MEMBER( tms7000_internal_r );

	void tms7000_A6EC1();

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 48; }
	virtual UINT32 execute_input_lines() const { return 3; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : NULL ); }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;
	address_space_config m_io_config;

	typedef void ( tms7000_device::*opcode_func ) ();
	static const opcode_func s_opfn[0x100];
	static const opcode_func s_opfn_exl[0x100];
	const opcode_func *m_opcode;

	static UINT16 bcd_add( UINT16 a, UINT16 b );
	static UINT16 bcd_tencomp( UINT16 a );
	static UINT16 bcd_sub( UINT16 a, UINT16 b);

	PAIR        m_pc;         /* Program counter */
	UINT8       m_sp;     /* Stack Pointer */
	UINT8       m_sr;     /* Status Register */
	UINT8       m_irq_state[3];   /* State of the three IRQs */
	UINT8       m_rf[0x80];   /* Register file (SJE) */
	UINT8       m_pf[0x100];  /* Perpherial file */
	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;
	int         m_icount;
	int         m_div_by_16_trigger;
	int         m_cycles_per_INT2;
	UINT8       m_t1_capture_latch; /* Timer 1 capture latch */
	INT8        m_t1_prescaler;   /* Timer 1 prescaler (5 bits) */
	INT16       m_t1_decrementer; /* Timer 1 decrementer (8 bits) */
	UINT8       m_idle_state; /* Set after the execution of an idle instruction */

	inline UINT16 RM16( UINT32 mAddr );
	inline UINT16 RRF16( UINT32 mAddr );
	inline void WRF16( UINT32 mAddr, PAIR p );
	void tms7000_check_IRQ_lines();
	void tms7000_do_interrupt( UINT16 address, UINT8 line );
	void illegal();
	void adc_b2a();
	void adc_r2a();
	void adc_r2b();
	void adc_r2r();
	void adc_i2a();
	void adc_i2b();
	void adc_i2r();
	void add_b2a();
	void add_r2a();
	void add_r2b();
	void add_r2r();
	void add_i2a();
	void add_i2b();
	void add_i2r();
	void and_b2a();
	void and_r2a();
	void and_r2b();
	void and_r2r();
	void and_i2a();
	void and_i2b();
	void and_i2r();
	void andp_a2p();
	void andp_b2p();
	void movp_i2p();
	void andp_i2p();
	void br_dir();
	void br_ind();
	void br_inx();
	void btjo_b2a();
	void btjo_r2a();
	void btjo_r2b();
	void btjo_r2r();
	void btjo_i2a();
	void btjo_i2b();
	void btjo_i2r();
	void btjop_ap();
	void btjop_bp();
	void btjop_ip();
	void btjz_b2a();
	void btjz_r2a();
	void btjz_r2b();
	void btjz_r2r();
	void btjz_i2a();
	void btjz_i2b();
	void btjz_i2r();
	void btjzp_ap();
	void btjzp_bp();
	void btjzp_ip();
	void call_dir();
	void call_ind();
	void call_inx();
	void clr_a();
	void clr_b();
	void clr_r();
	void clrc();
	void cmp_ba();
	void cmp_ra();
	void cmp_rb();
	void cmp_rr();
	void cmp_ia();
	void cmp_ib();
	void cmp_ir();
	void cmpa_dir();
	void cmpa_ind();
	void cmpa_inx();
	void dac_b2a();
	void dac_r2a();
	void dac_r2b();
	void dac_r2r();
	void dac_i2a();
	void dac_i2b();
	void dac_i2r();
	void dec_a();
	void dec_b();
	void dec_r();
	void decd_a();
	void decd_b();
	void decd_r();
	void dint();
	void djnz_a();
	void djnz_b();
	void djnz_r();
	void dsb_b2a();
	void dsb_r2a();
	void dsb_r2b();
	void dsb_r2r();
	void dsb_i2a();
	void dsb_i2b();
	void dsb_i2r();
	void eint();
	void idle();
	void inc_a();
	void inc_b();
	void inc_r();
	void inv_a();
	void inv_b();
	void inv_r();
	void jc();
	void jeq();
	void jl();
	void jmp();
	void j_jn();
	void jne();
	void jp();
	void jpz();
	void lda_dir();
	void lda_ind();
	void lda_inx();
	void ldsp();
	void mov_a2b();
	void mov_b2a();
	void mov_a2r();
	void mov_b2r();
	void mov_r2a();
	void mov_r2b();
	void mov_r2r();
	void mov_i2a();
	void mov_i2b();
	void mov_i2r();
	void movd_imm();
	void movd_r();
	void movd_inx();
	void movp_a2p();
	void movp_b2p();
	void movp_r2p();
	void movp_p2a();
	void movp_p2b();
	void mpy_ba();
	void mpy_ra();
	void mpy_rb();
	void mpy_rr();
	void mpy_ia();
	void mpy_ib();
	void mpy_ir();
	void nop();
	void or_b2a();
	void or_r2a();
	void or_r2b();
	void or_r2r();
	void or_i2a();
	void or_i2b();
	void or_i2r();
	void orp_a2p();
	void orp_b2p();
	void orp_i2p();
	void pop_a();
	void pop_b();
	void pop_r();
	void pop_st();
	void push_a();
	void push_b();
	void push_r();
	void push_st();
	void reti();
	void rets();
	void rl_a();
	void rl_b();
	void rl_r();
	void rlc_a();
	void rlc_b();
	void rlc_r();
	void rr_a();
	void rr_b();
	void rr_r();
	void rrc_a();
	void rrc_b();
	void rrc_r();
	void sbb_ba();
	void sbb_ra();
	void sbb_rb();
	void sbb_rr();
	void sbb_ia();
	void sbb_ib();
	void sbb_ir();
	void setc();
	void sta_dir();
	void sta_ind();
	void sta_inx();
	void stsp();
	void sub_ba();
	void sub_ra();
	void sub_rb();
	void sub_rr();
	void sub_ia();
	void sub_ib();
	void sub_ir();
	void trap_0();
	void trap_1();
	void trap_2();
	void trap_3();
	void trap_4();
	void trap_5();
	void trap_6();
	void trap_7();
	void trap_8();
	void trap_9();
	void trap_10();
	void trap_11();
	void trap_12();
	void trap_13();
	void trap_14();
	void trap_15();
	void trap_16();
	void trap_17();
	void trap_18();
	void trap_19();
	void trap_20();
	void trap_21();
	void trap_22();
	void trap_23();
	void swap_a();
	void swap_b();
	void swap_r();
	void swap_r_exl();
	void tstb();
	void xchb_a();
	void xchb_b();
	void xchb_r();
	void xor_b2a();
	void xor_r2a();
	void xor_r2b();
	void xor_r2r();
	void xor_i2a();
	void xor_i2b();
	void xor_i2r();
	void xorp_a2p();
	void xorp_b2p();
	void xorp_i2p();
	void tms7000_service_timer1();

};


class tms7000_exl_device : public tms7000_device
{
public:
	// construction/destruction
	tms7000_exl_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type TMS7000;
extern const device_type TMS7000_EXL;


#endif /* __TMS7000_H__ */
