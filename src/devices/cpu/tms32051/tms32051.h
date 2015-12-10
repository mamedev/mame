// license:BSD-3-Clause
// copyright-holders:Ville Linde
#pragma once

#ifndef __TMS32051_H__
#define __TMS32051_H__


enum
{
	TMS32051_INT1 = 0,
	TMS32051_INT2,
	TMS32051_INT3,
	TMS32051_TINT,
	TMS32051_RINT,
	TMS32051_XINT,
	TMS32051_TRNT,
	TMS32051_TXNT,
	TMS32051_INT4
};

struct TMS32051_PMST
{
	UINT16 iptr;
	UINT16 avis;
	UINT16 ovly;
	UINT16 ram;
	UINT16 mpmc;
	UINT16 ndx;
	UINT16 trm;
	UINT16 braf;
};

struct TMS32051_ST0
{
	UINT16 dp;
	UINT16 intm;
	UINT16 ovm;
	UINT16 ov;
	UINT16 arp;
};

struct TMS32051_ST1
{
	UINT16 arb;
	UINT16 cnf;
	UINT16 tc;
	UINT16 sxm;
	UINT16 c;
	UINT16 hm;
	UINT16 xf;
	UINT16 pm;
};


class tms32051_device : public cpu_device
{
public:
	// construction/destruction
	tms32051_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms32051_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_READ16_MEMBER( cpuregs_r );
	DECLARE_WRITE16_MEMBER( cpuregs_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 5; }
	virtual UINT32 execute_input_lines() const override { return 6; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr ); }
	virtual bool memory_read(address_spacenum spacenum, offs_t offset, int size, UINT64 &value) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	address_space_config m_program_config;
	address_space_config m_data_config;

	typedef void ( tms32051_device::*opcode_func )();
	static const opcode_func s_opcode_table[256];
	static const opcode_func s_opcode_table_be[256];
	static const opcode_func s_opcode_table_bf[256];

	UINT16 m_pc;
	UINT16 m_op;
	INT32 m_acc;
	INT32 m_accb;
	INT32 m_preg;
	UINT16 m_treg0;
	UINT16 m_treg1;
	UINT16 m_treg2;
	UINT16 m_ar[8];
	INT32 m_rptc;

	UINT16 m_bmar;
	INT32 m_brcr;
	UINT16 m_paer;
	UINT16 m_pasr;
	UINT16 m_indx;
	UINT16 m_dbmr;
	UINT16 m_arcr;

	TMS32051_ST0 m_st0;
	TMS32051_ST1 m_st1;
	TMS32051_PMST m_pmst;

	UINT16 m_ifr;
	UINT16 m_imr;

	UINT16 m_pcstack[8];
	int m_pcstack_ptr;

	UINT16 m_rpt_start, m_rpt_end;

	UINT16 m_cbcr;
	UINT16 m_cbsr1;
	UINT16 m_cber1;
	UINT16 m_cbsr2;
	UINT16 m_cber2;

	struct
	{
		int tddr;
		int psc;
		UINT16 tim;
		UINT16 prd;
	} m_timer;

	struct
	{
		UINT16 drr;
		UINT16 dxr;
		UINT16 spc;
	} m_serial;

	struct
	{
		INT32 acc;
		INT32 accb;
		UINT16 arcr;
		UINT16 indx;
		TMS32051_PMST pmst;
		INT32 preg;
		TMS32051_ST0 st0;
		TMS32051_ST1 st1;
		INT32 treg0;
		INT32 treg1;
		INT32 treg2;
	} m_shadow;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	int m_icount;

	inline void CHANGE_PC(UINT16 new_pc);
	inline UINT16 PM_READ16(UINT16 address);
	inline void PM_WRITE16(UINT16 address, UINT16 data);
	inline UINT16 DM_READ16(UINT16 address);
	inline void DM_WRITE16(UINT16 address, UINT16 data);
	inline void PUSH_STACK(UINT16 pc);
	inline UINT16 POP_STACK();
	inline INT32 SUB(UINT32 a, UINT32 b);
	inline INT32 ADD(UINT32 a, UINT32 b);
	inline void UPDATE_AR(int ar, int step);
	inline void UPDATE_ARP(int nar);
	UINT16 GET_ADDRESS();
	inline int GET_ZLVC_CONDITION(int zlvc, int zlvc_mask);
	inline int GET_TP_CONDITION(int tp);
	inline INT32 PREG_PSCALER(INT32 preg);
	void op_invalid();
	void op_abs();
	void op_adcb();
	void op_add_mem();
	void op_add_simm();
	void op_add_limm();
	void op_add_s16_mem();
	void op_addb();
	void op_addc();
	void op_adds();
	void op_addt();
	void op_and_mem();
	void op_and_limm();
	void op_and_s16_limm();
	void op_andb();
	void op_bsar();
	void op_cmpl();
	void op_crgt();
	void op_crlt();
	void op_exar();
	void op_lacb();
	void op_lacc_mem();
	void op_lacc_limm();
	void op_lacc_s16_mem();
	void op_lacl_simm();
	void op_lacl_mem();
	void op_lact();
	void op_lamm();
	void op_neg();
	void op_norm();
	void op_or_mem();
	void op_or_limm();
	void op_or_s16_limm();
	void op_orb();
	void op_rol();
	void op_rolb();
	void op_ror();
	void op_rorb();
	void op_sacb();
	void op_sach();
	void op_sacl();
	void op_samm();
	void op_sath();
	void op_satl();
	void op_sbb();
	void op_sbbb();
	void op_sfl();
	void op_sflb();
	void op_sfr();
	void op_sfrb();
	void op_sub_mem();
	void op_sub_s16_mem();
	void op_sub_simm();
	void op_sub_limm();
	void op_subb();
	void op_subc();
	void op_subs();
	void op_subt();
	void op_xor_mem();
	void op_xor_limm();
	void op_xor_s16_limm();
	void op_xorb();
	void op_zalr();
	void op_zap();
	void op_adrk();
	void op_cmpr();
	void op_lar_mem();
	void op_lar_simm();
	void op_lar_limm();
	void op_ldp_mem();
	void op_ldp_imm();
	void op_mar();
	void op_sar();
	void op_sbrk();
	void op_b();
	void op_bacc();
	void op_baccd();
	void op_banz();
	void op_banzd();
	void op_bcnd();
	void op_bcndd();
	void op_bd();
	void op_cala();
	void op_calad();
	void op_call();
	void op_calld();
	void op_cc();
	void op_ccd();
	void op_intr();
	void op_nmi();
	void op_retc();
	void op_retcd();
	void op_rete();
	void op_reti();
	void op_trap();
	void op_xc();
	void op_bldd_slimm();
	void op_bldd_dlimm();
	void op_bldd_sbmar();
	void op_bldd_dbmar();
	void op_bldp();
	void op_blpd_bmar();
	void op_blpd_imm();
	void op_dmov();
	void op_in();
	void op_lmmr();
	void op_out();
	void op_smmr();
	void op_tblr();
	void op_tblw();
	void op_apl_dbmr();
	void op_apl_imm();
	void op_cpl_dbmr();
	void op_cpl_imm();
	void op_opl_dbmr();
	void op_opl_imm();
	void op_splk();
	void op_xpl_dbmr();
	void op_xpl_imm();
	void op_apac();
	void op_lph();
	void op_lt();
	void op_lta();
	void op_ltd();
	void op_ltp();
	void op_lts();
	void op_mac();
	void op_macd();
	void op_madd();
	void op_mads();
	void op_mpy_mem();
	void op_mpy_simm();
	void op_mpy_limm();
	void op_mpya();
	void op_mpys();
	void op_mpyu();
	void op_pac();
	void op_spac();
	void op_sph();
	void op_spl();
	void op_spm();
	void op_sqra();
	void op_sqrs();
	void op_zpr();
	void op_bit();
	void op_bitt();
	void op_clrc_ov();
	void op_clrc_ext();
	void op_clrc_hold();
	void op_clrc_tc();
	void op_clrc_carry();
	void op_clrc_cnf();
	void op_clrc_intm();
	void op_clrc_xf();
	void op_idle();
	void op_idle2();
	void op_lst_st0();
	void op_lst_st1();
	void op_pop();
	void op_popd();
	void op_pshd();
	void op_push();
	void op_rpt_mem();
	void op_rpt_limm();
	void op_rpt_simm();
	void op_rptb();
	void op_rptz();
	void op_setc_ov();
	void op_setc_ext();
	void op_setc_hold();
	void op_setc_tc();
	void op_setc_carry();
	void op_setc_xf();
	void op_setc_cnf();
	void op_setc_intm();
	void op_sst_st0();
	void op_sst_st1();
	void op_group_be();
	void op_group_bf();
	void delay_slot(UINT16 startpc);
	void check_interrupts();
	void save_interrupt_context();
	void restore_interrupt_context();
};


class tms32053_device : public tms32051_device
{
public:
	// construction/destruction
	tms32053_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_config_complete() override;
	virtual void device_reset() override;
};


extern const device_type TMS32051;
extern const device_type TMS32053;


#endif /* __TMS32051_H__ */
