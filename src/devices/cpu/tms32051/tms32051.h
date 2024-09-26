// license:BSD-3-Clause
// copyright-holders:Ville Linde
#ifndef MAME_CPU_TMS32051_TMS32051_H
#define MAME_CPU_TMS32051_TMS32051_H

#pragma once


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
	uint16_t iptr;
	uint16_t avis;
	uint16_t ovly;
	uint16_t ram;
	uint16_t mpmc;
	uint16_t ndx;
	uint16_t trm;
	uint16_t braf;
};

struct TMS32051_ST0
{
	uint16_t dp;
	uint16_t intm;
	uint16_t ovm;
	uint16_t ov;
	uint16_t arp;
};

struct TMS32051_ST1
{
	uint16_t arb;
	uint16_t cnf;
	uint16_t tc;
	uint16_t sxm;
	uint16_t c;
	uint16_t hm;
	uint16_t xf;
	uint16_t pm;
};


class tms32051_device : public cpu_device
{
public:
	// construction/destruction
	tms32051_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t cpuregs_r(offs_t offset);
	void cpuregs_w(offs_t offset, uint16_t data);

	void tms32051_internal_data(address_map &map) ATTR_COLD;
	void tms32051_internal_pgm(address_map &map) ATTR_COLD;
protected:
	tms32051_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_pgm, address_map_constructor internal_data);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 5; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	typedef void ( tms32051_device::*opcode_func )();
	static const opcode_func s_opcode_table[256];
	static const opcode_func s_opcode_table_be[256];
	static const opcode_func s_opcode_table_bf[256];

	uint16_t m_pc;
	uint16_t m_op;
	int32_t m_acc;
	int32_t m_accb;
	int32_t m_preg;
	uint16_t m_treg0;
	uint16_t m_treg1;
	uint16_t m_treg2;
	uint16_t m_ar[8];
	int32_t m_rptc;

	uint16_t m_bmar;
	int32_t m_brcr;
	uint16_t m_paer;
	uint16_t m_pasr;
	uint16_t m_indx;
	uint16_t m_dbmr;
	uint16_t m_arcr;

	TMS32051_ST0 m_st0;
	TMS32051_ST1 m_st1;
	TMS32051_PMST m_pmst;

	uint16_t m_ifr;
	uint16_t m_imr;

	uint16_t m_pcstack[8];
	int m_pcstack_ptr;

	uint16_t m_rpt_start, m_rpt_end;

	uint16_t m_cbcr;
	uint16_t m_cbsr1;
	uint16_t m_cber1;
	uint16_t m_cbsr2;
	uint16_t m_cber2;

	struct
	{
		int tddr;
		int psc;
		uint16_t tim;
		uint16_t prd;
	} m_timer;

	struct
	{
		uint16_t drr;
		uint16_t dxr;
		uint16_t spc;
	} m_serial;

	struct
	{
		int32_t acc;
		int32_t accb;
		uint16_t arcr;
		uint16_t indx;
		TMS32051_PMST pmst;
		int32_t preg;
		TMS32051_ST0 st0;
		TMS32051_ST1 st1;
		int32_t treg0;
		int32_t treg1;
		int32_t treg2;
	} m_shadow;

	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<16, 1, -1, ENDIANNESS_LITTLE>::specific m_io;
	int m_icount;

	bool m_idle;

	inline void CHANGE_PC(uint16_t new_pc);
	inline uint16_t PM_READ16(uint16_t address);
	inline void PM_WRITE16(uint16_t address, uint16_t data);
	inline uint16_t DM_READ16(uint16_t address);
	inline void DM_WRITE16(uint16_t address, uint16_t data);
	inline void PUSH_STACK(uint16_t pc);
	inline uint16_t POP_STACK();
	inline int32_t SUB(uint32_t a, uint32_t b, bool shift16);
	inline int32_t ADD(uint32_t a, uint32_t b, bool shift16);
	inline void UPDATE_AR(int ar, int step);
	inline void UPDATE_ARP(int nar);
	uint16_t GET_ADDRESS();
	inline bool GET_ZLVC_CONDITION(int zlvc, int zlvc_mask);
	inline bool GET_TP_CONDITION(int tp);
	inline int32_t PREG_PSCALER(int32_t preg);
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
	void delay_slot(uint16_t startpc);
	void check_interrupts();
	void save_interrupt_context();
	void restore_interrupt_context();
};


class tms32053_device : public tms32051_device
{
public:
	// construction/destruction
	tms32053_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void tms32053_internal_data(address_map &map) ATTR_COLD;
	void tms32053_internal_pgm(address_map &map) ATTR_COLD;
protected:
	virtual void device_reset() override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(TMS32051, tms32051_device)
DECLARE_DEVICE_TYPE(TMS32053, tms32053_device)

#endif // MAME_CPU_TMS32051_TMS32051_H
