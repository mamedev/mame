// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dsp32.h
    Interface file for the portable DSP32 emulator.

***************************************************************************/

#pragma once

#ifndef __DSP32_H__
#define __DSP32_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// IRQ sources
const int DSP32_IRQ0        = 0;        // IRQ0
const int DSP32_IRQ1        = 1;        // IRQ1

// pin signal bits
const int DSP32_OUTPUT_PIF  = 0x01;
const int DSP32_OUTPUT_PDF  = 0x02;

// register enumeration
enum
{
	// CAU
	DSP32_PC=1,
	DSP32_R0,
	DSP32_R1,
	DSP32_R2,
	DSP32_R3,
	DSP32_R4,
	DSP32_R5,
	DSP32_R6,
	DSP32_R7,
	DSP32_R8,
	DSP32_R9,
	DSP32_R10,
	DSP32_R11,
	DSP32_R12,
	DSP32_R13,
	DSP32_R14,
	DSP32_R15,
	DSP32_R16,
	DSP32_R17,
	DSP32_R18,
	DSP32_R19,
	DSP32_R20,
	DSP32_R21,
	DSP32_R22,
	DSP32_PIN,
	DSP32_POUT,
	DSP32_IVTP,

	// DAU
	DSP32_A0,
	DSP32_A1,
	DSP32_A2,
	DSP32_A3,
	DSP32_DAUC,

	// PIO
	DSP32_PAR,
	DSP32_PDR,
	DSP32_PIR,
	DSP32_PCR,
	DSP32_EMR,
	DSP32_ESR,
	DSP32_PCW,
	DSP32_PIOP,

	// SIO
	DSP32_IBUF,
	DSP32_ISR,
	DSP32_OBUF,
	DSP32_OSR,
	DSP32_IOC
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define MCFG_DSP32C_OUTPUT_CALLBACK(_write) \
	devcb = &dsp32c_device::set_output_pins_callback(*device, DEVCB_##_write);

// ======================> dsp32c_device

class dsp32c_device : public cpu_device
{
public:
	// construction/destruction
	dsp32c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_output_pins_callback(device_t &device, _Object object) { return downcast<dsp32c_device &>(device).m_output_pins_changed.set_callback(object); }


	// public interfaces
	void pio_w(int reg, int data);
	int pio_r(int reg);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// memory accessors
	UINT32 ROPCODE(offs_t pc);
	UINT8 RBYTE(offs_t addr);
	void WBYTE(offs_t addr, UINT8 data);
	UINT16 RWORD(offs_t addr);
	UINT32 RLONG(offs_t addr);
	void WWORD(offs_t addr, UINT16 data);
	void WLONG(offs_t addr, UINT32 data);

	// interrupts
	void check_irqs();
	void set_irq_line(int irqline, int state);

	void update_pcr(UINT16 newval);
	void update_pins(void);
	void illegal(UINT32 op);
	void unimplemented(UINT32 op);
	void execute_one();

	// CAU helpers
	UINT32 cau_read_pi_special(UINT8 i);
	void cau_write_pi_special(UINT8 i, UINT32 val);
	UINT8 cau_read_pi_1byte(int pi);
	UINT16 cau_read_pi_2byte(int pi);
	UINT32 cau_read_pi_4byte(int pi);
	void cau_write_pi_1byte(int pi, UINT8 val);
	void cau_write_pi_2byte(int pi, UINT16 val);
	void cau_write_pi_4byte(int pi, UINT32 val);

	// DAU helpers
	double dau_get_amult(int aidx);
	double dau_get_anzflags();
	UINT8 dau_get_avuflags();
	void remember_last_dau(int aidx);
	void dau_set_val_noflags(int aidx, double res);
	void dau_set_val_flags(int aidx, double res);
	double dsp_to_double(UINT32 val);
	UINT32 double_to_dsp(double val);
	double dau_read_pi_special(int i);
	void dau_write_pi_special(int i, double val);
	double dau_read_pi_double_1st(int pi, int multiplier);
	double dau_read_pi_double_2nd(int pi, int multiplier, double xval);
	UINT32 dau_read_pi_4bytes(int pi);
	UINT16 dau_read_pi_2bytes(int pi);
	void dau_write_pi_double(int pi, double val);
	void dau_write_pi_4bytes(int pi, UINT32 val);
	void dau_write_pi_2bytes(int pi, UINT16 val);

	// common condition routine
	int condition(int cond);

	// CAU branch instruction implementation
	void nop(UINT32 op);
	void goto_t(UINT32 op);
	void goto_pl(UINT32 op);
	void goto_mi(UINT32 op);
	void goto_ne(UINT32 op);
	void goto_eq(UINT32 op);
	void goto_vc(UINT32 op);
	void goto_vs(UINT32 op);
	void goto_cc(UINT32 op);
	void goto_cs(UINT32 op);
	void goto_ge(UINT32 op);
	void goto_lt(UINT32 op);
	void goto_gt(UINT32 op);
	void goto_le(UINT32 op);
	void goto_hi(UINT32 op);
	void goto_ls(UINT32 op);
	void goto_auc(UINT32 op);
	void goto_aus(UINT32 op);
	void goto_age(UINT32 op);
	void goto_alt(UINT32 op);
	void goto_ane(UINT32 op);
	void goto_aeq(UINT32 op);
	void goto_avc(UINT32 op);
	void goto_avs(UINT32 op);
	void goto_agt(UINT32 op);
	void goto_ale(UINT32 op);
	void goto_ibe(UINT32 op);
	void goto_ibf(UINT32 op);
	void goto_obf(UINT32 op);
	void goto_obe(UINT32 op);
	void goto_pde(UINT32 op);
	void goto_pdf(UINT32 op);
	void goto_pie(UINT32 op);
	void goto_pif(UINT32 op);
	void goto_syc(UINT32 op);
	void goto_sys(UINT32 op);
	void goto_fbc(UINT32 op);
	void goto_fbs(UINT32 op);
	void goto_irq1lo(UINT32 op);
	void goto_irq1hi(UINT32 op);
	void goto_irq2lo(UINT32 op);
	void goto_irq2hi(UINT32 op);
	void dec_goto(UINT32 op);
	void call(UINT32 op);
	void goto24(UINT32 op);
	void call24(UINT32 op);
	void do_i(UINT32 op);
	void do_r(UINT32 op);

	// CAU 16-bit arithmetic implementation
	void add_si(UINT32 op);
	void add_ss(UINT32 op);
	void mul2_s(UINT32 op);
	void subr_ss(UINT32 op);
	void addr_ss(UINT32 op);
	void sub_ss(UINT32 op);
	void neg_s(UINT32 op);
	void andc_ss(UINT32 op);
	void cmp_ss(UINT32 op);
	void xor_ss(UINT32 op);
	void rcr_s(UINT32 op);
	void or_ss(UINT32 op);
	void rcl_s(UINT32 op);
	void shr_s(UINT32 op);
	void div2_s(UINT32 op);
	void and_ss(UINT32 op);
	void test_ss(UINT32 op);
	void add_di(UINT32 op);
	void subr_di(UINT32 op);
	void addr_di(UINT32 op);
	void sub_di(UINT32 op);
	void andc_di(UINT32 op);
	void cmp_di(UINT32 op);
	void xor_di(UINT32 op);
	void or_di(UINT32 op);
	void and_di(UINT32 op);
	void test_di(UINT32 op);

	// CAU 24-bit arithmetic implementation
	void adde_si(UINT32 op);
	void adde_ss(UINT32 op);
	void mul2e_s(UINT32 op);
	void subre_ss(UINT32 op);
	void addre_ss(UINT32 op);
	void sube_ss(UINT32 op);
	void nege_s(UINT32 op);
	void andce_ss(UINT32 op);
	void cmpe_ss(UINT32 op);
	void xore_ss(UINT32 op);
	void rcre_s(UINT32 op);
	void ore_ss(UINT32 op);
	void rcle_s(UINT32 op);
	void shre_s(UINT32 op);
	void div2e_s(UINT32 op);
	void ande_ss(UINT32 op);
	void teste_ss(UINT32 op);
	void adde_di(UINT32 op);
	void subre_di(UINT32 op);
	void addre_di(UINT32 op);
	void sube_di(UINT32 op);
	void andce_di(UINT32 op);
	void cmpe_di(UINT32 op);
	void xore_di(UINT32 op);
	void ore_di(UINT32 op);
	void ande_di(UINT32 op);
	void teste_di(UINT32 op);

	// CAU load/store implementation
	void load_hi(UINT32 op);
	void load_li(UINT32 op);
	void load_i(UINT32 op);
	void load_ei(UINT32 op);
	void store_hi(UINT32 op);
	void store_li(UINT32 op);
	void store_i(UINT32 op);
	void store_ei(UINT32 op);
	void load_hr(UINT32 op);
	void load_lr(UINT32 op);
	void load_r(UINT32 op);
	void load_er(UINT32 op);
	void store_hr(UINT32 op);
	void store_lr(UINT32 op);
	void store_r(UINT32 op);
	void store_er(UINT32 op);
	void load24(UINT32 op);

	// DAU form 1 implementation
	void d1_aMpp(UINT32 op);
	void d1_aMpm(UINT32 op);
	void d1_aMmp(UINT32 op);
	void d1_aMmm(UINT32 op);
	void d1_0px(UINT32 op);
	void d1_0mx(UINT32 op);
	void d1_1pp(UINT32 op);
	void d1_1pm(UINT32 op);
	void d1_1mp(UINT32 op);
	void d1_1mm(UINT32 op);
	void d1_aMppr(UINT32 op);
	void d1_aMpmr(UINT32 op);
	void d1_aMmpr(UINT32 op);
	void d1_aMmmr(UINT32 op);

	// DAU form 2 implementation
	void d2_aMpp(UINT32 op);
	void d2_aMpm(UINT32 op);
	void d2_aMmp(UINT32 op);
	void d2_aMmm(UINT32 op);
	void d2_aMppr(UINT32 op);
	void d2_aMpmr(UINT32 op);
	void d2_aMmpr(UINT32 op);
	void d2_aMmmr(UINT32 op);

	// DAU form 3 implementation
	void d3_aMpp(UINT32 op);
	void d3_aMpm(UINT32 op);
	void d3_aMmp(UINT32 op);
	void d3_aMmm(UINT32 op);
	void d3_aMppr(UINT32 op);
	void d3_aMpmr(UINT32 op);
	void d3_aMmpr(UINT32 op);
	void d3_aMmmr(UINT32 op);

	// DAU form 4 implementation
	void d4_pp(UINT32 op);
	void d4_pm(UINT32 op);
	void d4_mp(UINT32 op);
	void d4_mm(UINT32 op);
	void d4_ppr(UINT32 op);
	void d4_pmr(UINT32 op);
	void d4_mpr(UINT32 op);
	void d4_mmr(UINT32 op);

	// DAU form 5 implementation
	void d5_ic(UINT32 op);
	void d5_oc(UINT32 op);
	void d5_float(UINT32 op);
	void d5_int(UINT32 op);
	void d5_round(UINT32 op);
	void d5_ifalt(UINT32 op);
	void d5_ifaeq(UINT32 op);
	void d5_ifagt(UINT32 op);
	void d5_float24(UINT32 op);
	void d5_int24(UINT32 op);
	void d5_ieee(UINT32 op);
	void d5_dsp(UINT32 op);
	void d5_seed(UINT32 op);

	// dma helpers
	void dma_increment();
	void dma_load();
	void dma_store();

	// configuration
	const address_space_config      m_program_config;

	// internal state
	UINT32          m_r[32];
	UINT32          m_pin, m_pout;
	UINT32          m_ivtp;
	UINT32          m_nzcflags;
	UINT32          m_vflags;

	double          m_a[6];
	double          m_NZflags;
	UINT8           m_VUflags;

	double          m_abuf[4];
	UINT8           m_abufreg[4];
	UINT8           m_abufVUflags[4];
	UINT8           m_abufNZflags[4];
	int             m_abufcycle[4];
	int             m_abuf_index;

	INT32           m_mbufaddr[4];
	UINT32          m_mbufdata[4];
	int             m_mbuf_index;

	UINT16          m_par;
	UINT8           m_pare;
	UINT16          m_pdr;
	UINT16          m_pdr2;
	UINT16          m_pir;
	UINT16          m_pcr;
	UINT16          m_emr;
	UINT8           m_esr;
	UINT16          m_pcw;
	UINT8           m_piop;

	UINT32          m_ibuf;
	UINT32          m_isr;
	UINT32          m_obuf;
	UINT32          m_osr;

	UINT32          m_iotemp;

	// internal stuff
	int             m_lastp;
	int             m_icount;
	UINT8           m_lastpins;
	UINT32          m_ppc;
	address_space * m_program;
	direct_read_data *m_direct;

	devcb_write32 m_output_pins_changed;
	// tables
	static void (dsp32c_device::*const s_dsp32ops[])(UINT32 op);
	static const UINT32 s_regmap[4][16];
};


extern const device_type DSP32C;



#endif /* __DSP32_H__ */
