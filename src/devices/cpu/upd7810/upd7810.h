// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#ifndef MAME_CPU_UPD7810_UPD7810_H
#define MAME_CPU_UPD7810_UPD7810_H

#pragma once


/*
  all types have internal ram at 0xff00-0xffff
  7810
  7811 (4kbyte),7812(8),7814(16) have internal rom at 0x0000
*/


enum
{
	UPD7810_PC=1, UPD7810_SP, UPD7810_PSW,
	UPD7810_EA, UPD7810_V, UPD7810_A, UPD7810_VA,
	UPD7810_BC, UPD7810_B, UPD7810_C, UPD7810_DE, UPD7810_D, UPD7810_E, UPD7810_HL, UPD7810_H, UPD7810_L,
	UPD7810_EA2, UPD7810_V2, UPD7810_A2, UPD7810_VA2,
	UPD7810_BC2, UPD7810_B2, UPD7810_C2, UPD7810_DE2, UPD7810_D2, UPD7810_E2, UPD7810_HL2, UPD7810_H2, UPD7810_L2,
	UPD7810_CNT0, UPD7810_CNT1, UPD7810_TM0, UPD7810_TM1, UPD7810_ECNT, UPD7810_ECPT, UPD7810_ETM0, UPD7810_ETM1,
	UPD7810_MA, UPD7810_MB, UPD7810_MCC, UPD7810_MC, UPD7810_MM, UPD7810_MF,
	UPD7810_TMM, UPD7810_ETMM, UPD7810_EOM, UPD7810_SML, UPD7810_SMH,
	UPD7810_ANM, UPD7810_MKL, UPD7810_MKH, UPD7810_ZCM,
	UPD7810_TXB, UPD7810_RXB, UPD7810_CR0, UPD7810_CR1, UPD7810_CR2, UPD7810_CR3,
	UPD7810_AN0, UPD7810_AN1, UPD7810_AN2, UPD7810_AN3, UPD7810_AN4, UPD7810_AN5, UPD7810_AN6, UPD7810_AN7,
	UPD7810_TXD, UPD7810_RXD, UPD7810_SCK, UPD7810_TI, UPD7810_TO, UPD7810_CI, UPD7810_CO0, UPD7810_CO1,
	UPD7810_LV0, UPD7810_LV1
};

/* IRQ lines */
#define UPD7810_INTF1       0
#define UPD7810_INTF2       1
#define UPD7810_INTF0       2
#define UPD7810_INTFE1      4


class upd7810_device : public cpu_device
{
public:
	// construction/destruction
	upd7810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto to_func() { return m_to_func.bind(); }
	auto co0_func() { return m_co0_func.bind(); }
	auto co1_func() { return m_co1_func.bind(); }
	auto txd_func() { return m_txd_func.bind(); }
	auto rxd_func() { return m_rxd_func.bind(); }
	auto an0_func() { return m_an_func[0].bind(); }
	auto an1_func() { return m_an_func[1].bind(); }
	auto an2_func() { return m_an_func[2].bind(); }
	auto an3_func() { return m_an_func[3].bind(); }
	auto an4_func() { return m_an_func[4].bind(); }
	auto an5_func() { return m_an_func[5].bind(); }
	auto an6_func() { return m_an_func[6].bind(); }
	auto an7_func() { return m_an_func[7].bind(); }

	auto pa_in_cb() { return m_pa_in_cb.bind(); }
	auto pb_in_cb() { return m_pb_in_cb.bind(); }
	auto pc_in_cb() { return m_pc_in_cb.bind(); }
	auto pd_in_cb() { return m_pd_in_cb.bind(); }
	auto pf_in_cb() { return m_pf_in_cb.bind(); }
	auto pa_out_cb() { return m_pa_out_cb.bind(); }
	auto pb_out_cb() { return m_pb_out_cb.bind(); }
	auto pc_out_cb() { return m_pc_out_cb.bind(); }
	auto pd_out_cb() { return m_pd_out_cb.bind(); }
	auto pf_out_cb() { return m_pf_out_cb.bind(); }

	void set_pa_pullups(uint8_t p) { m_pa_pullups = p; }
	void set_pb_pullups(uint8_t p) { m_pb_pullups = p; }
	void set_pc_pullups(uint8_t p) { m_pc_pullups = p; }
	void set_pd_pullups(uint8_t p) { m_pd_pullups = p; }
	void set_pf_pullups(uint8_t p) { m_pf_pullups = p; }

	auto pt_in_cb() { return m_pt_in_cb.bind(); }

	void pa_w(uint8_t data, uint8_t mem_mask = ~0);
	void pb_w(uint8_t data, uint8_t mem_mask = ~0);
	void pc_w(uint8_t data, uint8_t mem_mask = ~0);
	void pd_w(uint8_t data, uint8_t mem_mask = ~0);
	void pf_w(uint8_t data, uint8_t mem_mask = ~0);

	void sck_w(int state);

protected:
	void upd_internal_128_ram_map(address_map &map) ATTR_COLD;
	void upd_internal_256_ram_map(address_map &map) ATTR_COLD;
	void upd_internal_4096_rom_128_ram_map(address_map &map) ATTR_COLD;
	void upd_internal_4096_rom_256_ram_map(address_map &map) ATTR_COLD;

	// flags
	enum
	{
		CY = 0x01,
		F1 = 0x02,
		L0 = 0x04,
		L1 = 0x08,
		HC = 0x10,
		SK = 0x20,
		Z  = 0x40,
		F7 = 0x80
	};

	// IRR flags
	enum
	{
		INTFNMI = 0x0001,
		INTFT0  = 0x0002,
		INTFT1  = 0x0004,
		INTF1   = 0x0008,
		INTF2   = 0x0010,
		INTFE0  = 0x0020,
		INTFE1  = 0x0040,
		INTFEIN = 0x0080,
		INTFAD  = 0x0100,
		INTFSR  = 0x0200,
		INTFST  = 0x0400,
		INTER   = 0x0800,
		INTOV   = 0x1000,
		INTF0   = 0x2000
	};

	// ITF flags
	enum
	{
		INTAN4 = 0x0001,
		INTAN5 = 0x0002,
		INTAN6 = 0x0004,
		INTAN7 = 0x0008,
		INTSB  = 0x0010
	};

	upd7810_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 3 - 1) / 3; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 3); }
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 40; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return true; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void update_sio(int cycles);
	virtual void handle_timers(int cycles);
	virtual void upd7810_take_irq();

	void upd7810_handle_timer0(int cycles, int clkdiv);
	void upd7810_handle_timer1(int cycles, int clkdiv);

	void upd7810_to_output_change(int state);
	void upd7810_co0_output_change();
	void upd7810_co1_output_change();

	devcb_write_line  m_to_func;
	devcb_write_line  m_co0_func;
	devcb_write_line  m_co1_func;
	devcb_write_line  m_txd_func;
	devcb_read_line   m_rxd_func;
	devcb_read8::array<8> m_an_func;

	devcb_read8       m_pa_in_cb;
	devcb_read8       m_pb_in_cb;
	devcb_read8       m_pc_in_cb;
	devcb_read8       m_pd_in_cb;
	devcb_read8       m_pf_in_cb;
	devcb_write8      m_pa_out_cb;
	devcb_write8      m_pb_out_cb;
	devcb_write8      m_pc_out_cb;
	devcb_write8      m_pd_out_cb;
	devcb_write8      m_pf_out_cb;

	devcb_read8       m_pt_in_cb; // TODO: uPD7807 only

	typedef void (upd7810_device::*opcode_func)();

	struct opcode_s {
		opcode_func opfunc;
		uint8_t oplen;
		uint8_t cycles;
		uint8_t cycles_skip;
		uint8_t mask_l0_l1;
	};

	virtual void configure_ops();

	static const struct opcode_s s_op48[256];
	static const struct opcode_s s_op4C[256];
	static const struct opcode_s s_op4D[256];
	static const struct opcode_s s_op60[256];
	static const struct opcode_s s_op64[256];
	static const struct opcode_s s_op70[256];
	static const struct opcode_s s_op74[256];
	static const struct opcode_s s_opXX_7810[256];
	static const struct opcode_s s_opXX_7807[256];
	static const struct opcode_s s_op48_7801[256];
	static const struct opcode_s s_op4C_7801[256];
	static const struct opcode_s s_op4D_7801[256];
	static const struct opcode_s s_op60_7801[256];
	static const struct opcode_s s_op64_7801[256];
	static const struct opcode_s s_op70_7801[256];
	static const struct opcode_s s_op74_7801[256];
	static const struct opcode_s s_opXX_7801[256];
	static const struct opcode_s s_op48_78c05[256];
	static const struct opcode_s s_op4C_78c05[256];
	static const struct opcode_s s_op4D_78c05[256];
	static const struct opcode_s s_op60_78c05[256];
	static const struct opcode_s s_op64_78c05[256];
	static const struct opcode_s s_op70_78c05[256];
	static const struct opcode_s s_op74_78c05[256];
	static const struct opcode_s s_opXX_78c05[256];
	static const struct opcode_s s_op48_78c06[256];
	static const struct opcode_s s_op4C_78c06[256];
	static const struct opcode_s s_op4D_78c06[256];
	static const struct opcode_s s_op60_78c06[256];
	static const struct opcode_s s_op64_78c06[256];
	static const struct opcode_s s_op70_78c06[256];
	static const struct opcode_s s_op74_78c06[256];
	static const struct opcode_s s_opXX_78c06[256];

	address_space_config m_program_config;
	memory_view m_ram_view;

	PAIR    m_ppc;    /* previous program counter */
	PAIR    m_pc;     /* program counter */
	PAIR    m_sp;     /* stack pointer */
	uint8_t   m_op;     /* opcode */
	uint8_t   m_op2;    /* opcode part 2 */
	uint8_t   m_iff;    /* interrupt enable flip flop */
	uint8_t   m_iff_pending;
	uint8_t   m_psw;    /* processor status word */
	PAIR    m_ea;     /* extended accumulator */
	PAIR    m_va;     /* accumulator + vector register */
	PAIR    m_bc;     /* 8bit B and C registers / 16bit BC register */
	PAIR    m_de;     /* 8bit D and E registers / 16bit DE register */
	PAIR    m_hl;     /* 8bit H and L registers / 16bit HL register */
	PAIR    m_ea2;    /* alternate register set */
	PAIR    m_va2;
	PAIR    m_bc2;
	PAIR    m_de2;
	PAIR    m_hl2;
	PAIR    m_cnt;    /* 8 bit timer counter */
	PAIR    m_tm;     /* 8 bit timer 0/1 comparator inputs */
	PAIR    m_ecnt;   /* timer counter register / capture register */
	PAIR    m_etm;    /* timer 0/1 comparator inputs */
	uint8_t   m_ma;     /* port A input or output mask */
	uint8_t   m_mb;     /* port B input or output mask */
	uint8_t   m_mcc;    /* port C control/port select */
	uint8_t   m_mc;     /* port C input or output mask */
	uint8_t   m_mm;     /* memory mapping */
	uint8_t   m_mf;     /* port F input or output mask */
	uint8_t   m_mt;     /* port T input threshold level */
	uint8_t   m_tmm;    /* timer 0 and timer 1 operating parameters */
	uint8_t   m_etmm;   /* 16-bit multifunction timer/event counter */
	uint8_t   m_eom;    /* 16-bit timer/event counter output control */
	uint8_t   m_sml;    /* serial interface parameters low */
	uint8_t   m_smh;    /* -"- high */
	uint8_t   m_panm;   /* previous analog to digital converter operating parameters */
	uint8_t   m_anm;    /* analog to digital converter operating parameters */
	uint8_t   m_mkl;    /* interrupt mask low */
	uint8_t   m_mkh;    /* -"- high */
	uint8_t   m_zcm;    /* bias circuitry for ac zero-cross detection */
	uint8_t   m_pa_in;  /* port A,B,C,D,F inputs */
	uint8_t   m_pb_in;
	uint8_t   m_pc_in;
	uint8_t   m_pd_in;
	uint8_t   m_pf_in;
	uint8_t   m_pa_out; /* port A,B,C,D,F outputs */
	uint8_t   m_pb_out;
	uint8_t   m_pc_out;
	uint8_t   m_pd_out;
	uint8_t   m_pf_out;
	uint8_t   m_pa_pullups;
	uint8_t   m_pb_pullups;
	uint8_t   m_pc_pullups;
	uint8_t   m_pd_pullups;
	uint8_t   m_pf_pullups;
	uint8_t   m_cr[4];  /* analog digital conversion registers */
	uint8_t   m_txb;    /* transmitter buffer */
	uint8_t   m_rxb;    /* receiver buffer */
	uint8_t   m_txd;    /* port C control line states */
	uint8_t   m_rxd;
	uint8_t   m_sck;
	uint8_t   m_ti;
	uint8_t   m_to;
	uint8_t   m_ci;
	uint8_t   m_lv0;    /* level flip flop for co0 */
	uint8_t   m_lv1;    /* level flip flop for co1 */
	uint8_t   m_co0;
	uint8_t   m_co1;
	uint16_t  m_irr;    /* interrupt request register */
	uint16_t  m_itf;    /* interrupt test flag register */
	int     m_nmi;    /* keep track of current nmi state. Needed for 7810 irq checking. */
	int     m_int1;   /* keep track of current int1 state. Needed for irq checking. */
	int     m_int2;   /* keep track to current int2 state. Needed for irq checking. */

	/* internal helper variables */
	uint16_t  m_txs;    /* transmitter shift register */
	uint16_t  m_rxs;    /* receiver shift register */
	uint8_t   m_txcnt;  /* transmitter shift register bit count */
	uint8_t   m_rxcnt;  /* receiver shift register bit count */
	uint8_t   m_txbuf;  /* transmitter buffer was written */
	int32_t   m_ovc0;   /* overflow counter for timer 0 (for clock div 12/384) */
	int32_t   m_ovc1;   /* overflow counter for timer 0 (for clock div 12/384) */
	int32_t   m_ovce;   /* overflow counter for ecnt */
	int32_t   m_ovcf;   /* overflow counter for fixed clock div 3 mode */
	int32_t   m_ovcs;   /* overflow counter for serial I/O */
	uint8_t   m_edges;  /* rising/falling edge flag for serial I/O */
	uint16_t  m_adcnt;  /* A/D converter cycle count */
	uint8_t   m_adtot;  /* A/D converter total cycles per conversion */
	uint8_t   m_tmpcr;  /* temporary analog digital conversion register */
	int     m_shdone; /* A/D converter sample and hold done */
	int     m_adout;  /* currently selected A/D converter output register */
	int     m_adin;   /* currently selected A/D converter input */
	int     m_adrange;/* in scan mode, A/D converter range (AN0-AN3 or AN4-AN7) */

	const struct opcode_s *m_opXX;    /* opcode table */
	const struct opcode_s *m_op48;
	const struct opcode_s *m_op4C;
	const struct opcode_s *m_op4D;
	const struct opcode_s *m_op60;
	const struct opcode_s *m_op64;
	const struct opcode_s *m_op70;
	const struct opcode_s *m_op74;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_opcodes;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	int m_icount;

	uint8_t RP(offs_t port);
	void WP(offs_t port, uint8_t data);
	void upd7810_write_EOM();
	void upd7810_write_TXB();
	void upd7810_sio_output();
	void upd7810_sio_input();
	void illegal();
	void illegal2();
	void SLRC_A();
	void SLRC_B();
	void SLRC_C();
	void SLLC_A();
	void SLLC_B();
	void SLLC_C();
	void SK_NV();
	void SK_CY();
	void SK_HC();
	void SK_Z();
	void SKN_NV();
	void SKN_CY();
	void SKN_HC();
	void SKN_Z();
	void SLR_A();
	void SLR_B();
	void SLR_C();
	void SLL_A();
	void SLL_B();
	void SLL_C();
	void JEA();
	void CALB();
	void CLC();
	void STC();
	void MUL_A();
	void MUL_B();
	void MUL_C();
	void RLR_A();
	void RLR_B();
	void RLR_C();
	void RLL_A();
	void RLL_B();
	void RLL_C();
	void RLD();
	void RRD();
	void NEGA();
	void HALT();
	void DIV_A();
	void DIV_B();
	void DIV_C();
	void SKIT_NMI();
	void SKIT_FT0();
	void SKIT_FT1();
	void SKIT_F1();
	void SKIT_F2();
	void SKIT_FE0();
	void SKIT_FE1();
	void SKIT_FEIN();
	void SKIT_FAD();
	void SKIT_FSR();
	void SKIT_FST();
	void SKIT_ER();
	void SKIT_OV();
	void SKIT_AN4();
	void SKIT_AN5();
	void SKIT_AN6();
	void SKIT_AN7();
	void SKIT_SB();
	void SKNIT_NMI();
	void SKNIT_FT0();
	void SKNIT_FT1();
	void SKNIT_F1();
	void SKNIT_F2();
	void SKNIT_FE0();
	void SKNIT_FE1();
	void SKNIT_FEIN();
	void SKNIT_FAD();
	void SKNIT_FSR();
	void SKNIT_FST();
	void SKNIT_ER();
	void SKNIT_OV();
	void SKNIT_AN4();
	void SKNIT_AN5();
	void SKNIT_AN6();
	void SKNIT_AN7();
	void SKNIT_SB();
	void LDEAX_D();
	void LDEAX_H();
	void LDEAX_Dp();
	void LDEAX_Hp();
	void LDEAX_D_xx();
	void LDEAX_H_A();
	void LDEAX_H_B();
	void LDEAX_H_EA();
	void LDEAX_H_xx();
	void STEAX_D();
	void STEAX_H();
	void STEAX_Dp();
	void STEAX_Hp();
	void STEAX_D_xx();
	void STEAX_H_A();
	void STEAX_H_B();
	void STEAX_H_EA();
	void STEAX_H_xx();
	void DSLR_EA();
	void DSLL_EA();
	void TABLE();
	void DRLR_EA();
	void DRLL_EA();
	void STOP();
	void DMOV_EA_ECNT();
	void DMOV_EA_ECPT();
	void DMOV_ETM0_EA();
	void DMOV_ETM1_EA();
	void MOV_A_PA();
	void MOV_A_PB();
	void MOV_A_PC();
	void MOV_A_PD();
	void MOV_A_PF();
	void MOV_A_MKH();
	void MOV_A_MKL();
	void MOV_A_ANM();
	void MOV_A_SMH();
	void MOV_A_EOM();
	void MOV_A_TMM();
	void MOV_A_PT();
	void MOV_A_RXB();
	void MOV_A_CR0();
	void MOV_A_CR1();
	void MOV_A_CR2();
	void MOV_A_CR3();
	void MOV_PA_A();
	void MOV_PB_A();
	void MOV_PC_A();
	void MOV_PD_A();
	void MOV_PF_A();
	void MOV_MKH_A();
	void MOV_MKL_A();
	void MOV_ANM_A();
	void MOV_SMH_A();
	void MOV_SML_A();
	void MOV_EOM_A();
	void MOV_ETMM_A();
	void MOV_TMM_A();
	void MOV_MM_A();
	void MOV_MCC_A();
	void MOV_MA_A();
	void MOV_MB_A();
	void MOV_MC_A();
	void MOV_MF_A();
	void MOV_TXB_A();
	void MOV_TM0_A();
	void MOV_TM1_A();
	void MOV_MT_A();
	void MOV_ZCM_A();
	void ANA_V_A();
	void ANA_A_A();
	void ANA_B_A();
	void ANA_C_A();
	void ANA_D_A();
	void ANA_E_A();
	void ANA_H_A();
	void ANA_L_A();
	void XRA_V_A();
	void XRA_A_A();
	void XRA_B_A();
	void XRA_C_A();
	void XRA_D_A();
	void XRA_E_A();
	void XRA_H_A();
	void XRA_L_A();
	void ORA_V_A();
	void ORA_A_A();
	void ORA_B_A();
	void ORA_C_A();
	void ORA_D_A();
	void ORA_E_A();
	void ORA_H_A();
	void ORA_L_A();
	void ADDNC_V_A();
	void ADDNC_A_A();
	void ADDNC_B_A();
	void ADDNC_C_A();
	void ADDNC_D_A();
	void ADDNC_E_A();
	void ADDNC_H_A();
	void ADDNC_L_A();
	void GTA_V_A();
	void GTA_A_A();
	void GTA_B_A();
	void GTA_C_A();
	void GTA_D_A();
	void GTA_E_A();
	void GTA_H_A();
	void GTA_L_A();
	void SUBNB_V_A();
	void SUBNB_A_A();
	void SUBNB_B_A();
	void SUBNB_C_A();
	void SUBNB_D_A();
	void SUBNB_E_A();
	void SUBNB_H_A();
	void SUBNB_L_A();
	void LTA_V_A();
	void LTA_A_A();
	void LTA_B_A();
	void LTA_C_A();
	void LTA_D_A();
	void LTA_E_A();
	void LTA_H_A();
	void LTA_L_A();
	void ADD_V_A();
	void ADD_A_A();
	void ADD_B_A();
	void ADD_C_A();
	void ADD_D_A();
	void ADD_E_A();
	void ADD_H_A();
	void ADD_L_A();
	void ADC_V_A();
	void ADC_A_A();
	void ADC_B_A();
	void ADC_C_A();
	void ADC_D_A();
	void ADC_E_A();
	void ADC_H_A();
	void ADC_L_A();
	void SUB_V_A();
	void SUB_A_A();
	void SUB_B_A();
	void SUB_C_A();
	void SUB_D_A();
	void SUB_E_A();
	void SUB_H_A();
	void SUB_L_A();
	void NEA_V_A();
	void NEA_A_A();
	void NEA_B_A();
	void NEA_C_A();
	void NEA_D_A();
	void NEA_E_A();
	void NEA_H_A();
	void NEA_L_A();
	void SBB_V_A();
	void SBB_A_A();
	void SBB_B_A();
	void SBB_C_A();
	void SBB_D_A();
	void SBB_E_A();
	void SBB_H_A();
	void SBB_L_A();
	void EQA_V_A();
	void EQA_A_A();
	void EQA_B_A();
	void EQA_C_A();
	void EQA_D_A();
	void EQA_E_A();
	void EQA_H_A();
	void EQA_L_A();
	void ANA_A_V();
	void ANA_A_B();
	void ANA_A_C();
	void ANA_A_D();
	void ANA_A_E();
	void ANA_A_H();
	void ANA_A_L();
	void XRA_A_V();
	void XRA_A_B();
	void XRA_A_C();
	void XRA_A_D();
	void XRA_A_E();
	void XRA_A_H();
	void XRA_A_L();
	void ORA_A_V();
	void ORA_A_B();
	void ORA_A_C();
	void ORA_A_D();
	void ORA_A_E();
	void ORA_A_H();
	void ORA_A_L();
	void ADDNC_A_V();
	void ADDNC_A_B();
	void ADDNC_A_C();
	void ADDNC_A_D();
	void ADDNC_A_E();
	void ADDNC_A_H();
	void ADDNC_A_L();
	void GTA_A_V();
	void GTA_A_B();
	void GTA_A_C();
	void GTA_A_D();
	void GTA_A_E();
	void GTA_A_H();
	void GTA_A_L();
	void SUBNB_A_V();
	void SUBNB_A_B();
	void SUBNB_A_C();
	void SUBNB_A_D();
	void SUBNB_A_E();
	void SUBNB_A_H();
	void SUBNB_A_L();
	void LTA_A_V();
	void LTA_A_B();
	void LTA_A_C();
	void LTA_A_D();
	void LTA_A_E();
	void LTA_A_H();
	void LTA_A_L();
	void ADD_A_V();
	void ADD_A_B();
	void ADD_A_C();
	void ADD_A_D();
	void ADD_A_E();
	void ADD_A_H();
	void ADD_A_L();
	void ONA_A_V();
	void ONA_A_A();
	void ONA_A_B();
	void ONA_A_C();
	void ONA_A_D();
	void ONA_A_E();
	void ONA_A_H();
	void ONA_A_L();
	void ADC_A_V();
	void ADC_A_B();
	void ADC_A_C();
	void ADC_A_D();
	void ADC_A_E();
	void ADC_A_H();
	void ADC_A_L();
	void OFFA_A_V();
	void OFFA_A_A();
	void OFFA_A_B();
	void OFFA_A_C();
	void OFFA_A_D();
	void OFFA_A_E();
	void OFFA_A_H();
	void OFFA_A_L();
	void SUB_A_V();
	void SUB_A_B();
	void SUB_A_C();
	void SUB_A_D();
	void SUB_A_E();
	void SUB_A_H();
	void SUB_A_L();
	void NEA_A_V();
	void NEA_A_B();
	void NEA_A_C();
	void NEA_A_D();
	void NEA_A_E();
	void NEA_A_H();
	void NEA_A_L();
	void SBB_A_V();
	void SBB_A_B();
	void SBB_A_C();
	void SBB_A_D();
	void SBB_A_E();
	void SBB_A_H();
	void SBB_A_L();
	void EQA_A_V();
	void EQA_A_B();
	void EQA_A_C();
	void EQA_A_D();
	void EQA_A_E();
	void EQA_A_H();
	void EQA_A_L();
	void MVI_PA_xx();
	void MVI_PB_xx();
	void MVI_PC_xx();
	void MVI_PD_xx();
	void MVI_PF_xx();
	void MVI_MKH_xx();
	void MVI_MKL_xx();
	void ANI_PA_xx();
	void ANI_PB_xx();
	void ANI_PC_xx();
	void ANI_PD_xx();
	void ANI_PF_xx();
	void ANI_MKH_xx();
	void ANI_MKL_xx();
	void XRI_PA_xx();
	void XRI_PB_xx();
	void XRI_PC_xx();
	void XRI_PD_xx();
	void XRI_PF_xx();
	void XRI_MKH_xx();
	void XRI_MKL_xx();
	void ORI_PA_xx();
	void ORI_PB_xx();
	void ORI_PC_xx();
	void ORI_PD_xx();
	void ORI_PF_xx();
	void ORI_MKH_xx();
	void ORI_MKL_xx();
	void ADINC_PA_xx();
	void ADINC_PB_xx();
	void ADINC_PC_xx();
	void ADINC_PD_xx();
	void ADINC_PF_xx();
	void ADINC_MKH_xx();
	void ADINC_MKL_xx();
	void GTI_PA_xx();
	void GTI_PB_xx();
	void GTI_PC_xx();
	void GTI_PD_xx();
	void GTI_PF_xx();
	void GTI_MKH_xx();
	void GTI_MKL_xx();
	void SUINB_PA_xx();
	void SUINB_PB_xx();
	void SUINB_PC_xx();
	void SUINB_PD_xx();
	void SUINB_PF_xx();
	void SUINB_MKH_xx();
	void SUINB_MKL_xx();
	void LTI_PA_xx();
	void LTI_PB_xx();
	void LTI_PC_xx();
	void LTI_PD_xx();
	void LTI_PF_xx();
	void LTI_MKH_xx();
	void LTI_MKL_xx();
	void ADI_PA_xx();
	void ADI_PB_xx();
	void ADI_PC_xx();
	void ADI_PD_xx();
	void ADI_PF_xx();
	void ADI_MKH_xx();
	void ADI_MKL_xx();
	void ONI_PA_xx();
	void ONI_PB_xx();
	void ONI_PC_xx();
	void ONI_PD_xx();
	void ONI_PF_xx();
	void ONI_MKH_xx();
	void ONI_MKL_xx();
	void ACI_PA_xx();
	void ACI_PB_xx();
	void ACI_PC_xx();
	void ACI_PD_xx();
	void ACI_PF_xx();
	void ACI_MKH_xx();
	void ACI_MKL_xx();
	void OFFI_PA_xx();
	void OFFI_PB_xx();
	void OFFI_PC_xx();
	void OFFI_PD_xx();
	void OFFI_PF_xx();
	void OFFI_MKH_xx();
	void OFFI_MKL_xx();
	void SUI_PA_xx();
	void SUI_PB_xx();
	void SUI_PC_xx();
	void SUI_PD_xx();
	void SUI_PF_xx();
	void SUI_MKH_xx();
	void SUI_MKL_xx();
	void NEI_PA_xx();
	void NEI_PB_xx();
	void NEI_PC_xx();
	void NEI_PD_xx();
	void NEI_PF_xx();
	void NEI_MKH_xx();
	void NEI_MKL_xx();
	void SBI_PA_xx();
	void SBI_PB_xx();
	void SBI_PC_xx();
	void SBI_PD_xx();
	void SBI_PF_xx();
	void SBI_MKH_xx();
	void SBI_MKL_xx();
	void EQI_PA_xx();
	void EQI_PB_xx();
	void EQI_PC_xx();
	void EQI_PD_xx();
	void EQI_PF_xx();
	void EQI_MKH_xx();
	void EQI_MKL_xx();
	void MVI_ANM_xx();
	void MVI_SMH_xx();
	void MVI_EOM_xx();
	void MVI_TMM_xx();
	void ANI_ANM_xx();
	void ANI_SMH_xx();
	void ANI_EOM_xx();
	void ANI_TMM_xx();
	void XRI_ANM_xx();
	void XRI_SMH_xx();
	void XRI_EOM_xx();
	void XRI_TMM_xx();
	void ORI_ANM_xx();
	void ORI_SMH_xx();
	void ORI_EOM_xx();
	void ORI_TMM_xx();
	void ADINC_ANM_xx();
	void ADINC_SMH_xx();
	void ADINC_EOM_xx();
	void ADINC_TMM_xx();
	void GTI_ANM_xx();
	void GTI_SMH_xx();
	void GTI_EOM_xx();
	void GTI_TMM_xx();
	void SUINB_ANM_xx();
	void SUINB_SMH_xx();
	void SUINB_EOM_xx();
	void SUINB_TMM_xx();
	void LTI_ANM_xx();
	void LTI_SMH_xx();
	void LTI_EOM_xx();
	void LTI_TMM_xx();
	void ADI_ANM_xx();
	void ADI_SMH_xx();
	void ADI_EOM_xx();
	void ADI_TMM_xx();
	void ONI_ANM_xx();
	void ONI_SMH_xx();
	void ONI_EOM_xx();
	void ONI_TMM_xx();
	void ACI_ANM_xx();
	void ACI_SMH_xx();
	void ACI_EOM_xx();
	void ACI_TMM_xx();
	void OFFI_ANM_xx();
	void OFFI_SMH_xx();
	void OFFI_EOM_xx();
	void OFFI_TMM_xx();
	void SUI_ANM_xx();
	void SUI_SMH_xx();
	void SUI_EOM_xx();
	void SUI_TMM_xx();
	void NEI_ANM_xx();
	void NEI_SMH_xx();
	void NEI_EOM_xx();
	void NEI_TMM_xx();
	void SBI_ANM_xx();
	void SBI_SMH_xx();
	void SBI_EOM_xx();
	void SBI_TMM_xx();
	void EQI_ANM_xx();
	void EQI_SMH_xx();
	void EQI_EOM_xx();
	void EQI_TMM_xx();
	void SSPD_w();
	void LSPD_w();
	void SBCD_w();
	void LBCD_w();
	void SDED_w();
	void LDED_w();
	void SHLD_w();
	void LHLD_w();
	void EADD_EA_A();
	void EADD_EA_B();
	void EADD_EA_C();
	void ESUB_EA_A();
	void ESUB_EA_B();
	void ESUB_EA_C();
	void MOV_V_w();
	void MOV_A_w();
	void MOV_B_w();
	void MOV_C_w();
	void MOV_D_w();
	void MOV_E_w();
	void MOV_H_w();
	void MOV_L_w();
	void MOV_w_V();
	void MOV_w_A();
	void MOV_w_B();
	void MOV_w_C();
	void MOV_w_D();
	void MOV_w_E();
	void MOV_w_H();
	void MOV_w_L();
	void ANAX_B();
	void ANAX_D();
	void ANAX_H();
	void ANAX_Dp();
	void ANAX_Hp();
	void ANAX_Dm();
	void ANAX_Hm();
	void XRAX_B();
	void XRAX_D();
	void XRAX_H();
	void XRAX_Dp();
	void XRAX_Hp();
	void XRAX_Dm();
	void XRAX_Hm();
	void ORAX_B();
	void ORAX_D();
	void ORAX_H();
	void ORAX_Dp();
	void ORAX_Hp();
	void ORAX_Dm();
	void ORAX_Hm();
	void ADDNCX_B();
	void ADDNCX_D();
	void ADDNCX_H();
	void ADDNCX_Dp();
	void ADDNCX_Hp();
	void ADDNCX_Dm();
	void ADDNCX_Hm();
	void GTAX_B();
	void GTAX_D();
	void GTAX_H();
	void GTAX_Dp();
	void GTAX_Hp();
	void GTAX_Dm();
	void GTAX_Hm();
	void SUBNBX_B();
	void SUBNBX_D();
	void SUBNBX_H();
	void SUBNBX_Dp();
	void SUBNBX_Hp();
	void SUBNBX_Dm();
	void SUBNBX_Hm();
	void LTAX_B();
	void LTAX_D();
	void LTAX_H();
	void LTAX_Dp();
	void LTAX_Hp();
	void LTAX_Dm();
	void LTAX_Hm();
	void ADDX_B();
	void ADDX_D();
	void ADDX_H();
	void ADDX_Dp();
	void ADDX_Hp();
	void ADDX_Dm();
	void ADDX_Hm();
	void ONAX_B();
	void ONAX_D();
	void ONAX_H();
	void ONAX_Dp();
	void ONAX_Hp();
	void ONAX_Dm();
	void ONAX_Hm();
	void ADCX_B();
	void ADCX_D();
	void ADCX_H();
	void ADCX_Dp();
	void ADCX_Hp();
	void ADCX_Dm();
	void ADCX_Hm();
	void OFFAX_B();
	void OFFAX_D();
	void OFFAX_H();
	void OFFAX_Dp();
	void OFFAX_Hp();
	void OFFAX_Dm();
	void OFFAX_Hm();
	void SUBX_B();
	void SUBX_D();
	void SUBX_H();
	void SUBX_Dp();
	void SUBX_Hp();
	void SUBX_Dm();
	void SUBX_Hm();
	void NEAX_B();
	void NEAX_D();
	void NEAX_H();
	void NEAX_Dp();
	void NEAX_Hp();
	void NEAX_Dm();
	void NEAX_Hm();
	void SBBX_B();
	void SBBX_D();
	void SBBX_H();
	void SBBX_Dp();
	void SBBX_Hp();
	void SBBX_Dm();
	void SBBX_Hm();
	void EQAX_B();
	void EQAX_D();
	void EQAX_H();
	void EQAX_Dp();
	void EQAX_Hp();
	void EQAX_Dm();
	void EQAX_Hm();
	void ANI_V_xx();
	void ANI_A_xx();
	void ANI_B_xx();
	void ANI_C_xx();
	void ANI_D_xx();
	void ANI_E_xx();
	void ANI_H_xx();
	void ANI_L_xx();
	void XRI_V_xx();
	void XRI_A_xx();
	void XRI_B_xx();
	void XRI_C_xx();
	void XRI_D_xx();
	void XRI_E_xx();
	void XRI_H_xx();
	void XRI_L_xx();
	void ORI_V_xx();
	void ORI_A_xx();
	void ORI_B_xx();
	void ORI_C_xx();
	void ORI_D_xx();
	void ORI_E_xx();
	void ORI_H_xx();
	void ORI_L_xx();
	void ADINC_V_xx();
	void ADINC_A_xx();
	void ADINC_B_xx();
	void ADINC_C_xx();
	void ADINC_D_xx();
	void ADINC_E_xx();
	void ADINC_H_xx();
	void ADINC_L_xx();
	void GTI_V_xx();
	void GTI_A_xx();
	void GTI_B_xx();
	void GTI_C_xx();
	void GTI_D_xx();
	void GTI_E_xx();
	void GTI_H_xx();
	void GTI_L_xx();
	void SUINB_V_xx();
	void SUINB_A_xx();
	void SUINB_B_xx();
	void SUINB_C_xx();
	void SUINB_D_xx();
	void SUINB_E_xx();
	void SUINB_H_xx();
	void SUINB_L_xx();
	void LTI_V_xx();
	void LTI_A_xx();
	void LTI_B_xx();
	void LTI_C_xx();
	void LTI_D_xx();
	void LTI_E_xx();
	void LTI_H_xx();
	void LTI_L_xx();
	void ADI_V_xx();
	void ADI_A_xx();
	void ADI_B_xx();
	void ADI_C_xx();
	void ADI_D_xx();
	void ADI_E_xx();
	void ADI_H_xx();
	void ADI_L_xx();
	void ONI_V_xx();
	void ONI_A_xx();
	void ONI_B_xx();
	void ONI_C_xx();
	void ONI_D_xx();
	void ONI_E_xx();
	void ONI_H_xx();
	void ONI_L_xx();
	void ACI_V_xx();
	void ACI_A_xx();
	void ACI_B_xx();
	void ACI_C_xx();
	void ACI_D_xx();
	void ACI_E_xx();
	void ACI_H_xx();
	void ACI_L_xx();
	void OFFI_V_xx();
	void OFFI_A_xx();
	void OFFI_B_xx();
	void OFFI_C_xx();
	void OFFI_D_xx();
	void OFFI_E_xx();
	void OFFI_H_xx();
	void OFFI_L_xx();
	void SUI_V_xx();
	void SUI_A_xx();
	void SUI_B_xx();
	void SUI_C_xx();
	void SUI_D_xx();
	void SUI_E_xx();
	void SUI_H_xx();
	void SUI_L_xx();
	void NEI_V_xx();
	void NEI_A_xx();
	void NEI_B_xx();
	void NEI_C_xx();
	void NEI_D_xx();
	void NEI_E_xx();
	void NEI_H_xx();
	void NEI_L_xx();
	void SBI_V_xx();
	void SBI_A_xx();
	void SBI_B_xx();
	void SBI_C_xx();
	void SBI_D_xx();
	void SBI_E_xx();
	void SBI_H_xx();
	void SBI_L_xx();
	void EQI_V_xx();
	void EQI_A_xx();
	void EQI_B_xx();
	void EQI_C_xx();
	void EQI_D_xx();
	void EQI_E_xx();
	void EQI_H_xx();
	void EQI_L_xx();
	void ANAW_wa();
	void DAN_EA_BC();
	void DAN_EA_DE();
	void DAN_EA_HL();
	void XRAW_wa();
	void DXR_EA_BC();
	void DXR_EA_DE();
	void DXR_EA_HL();
	void ORAW_wa();
	void DOR_EA_BC();
	void DOR_EA_DE();
	void DOR_EA_HL();
	void ADDNCW_wa();
	void DADDNC_EA_BC();
	void DADDNC_EA_DE();
	void DADDNC_EA_HL();
	void GTAW_wa();
	void DGT_EA_BC();
	void DGT_EA_DE();
	void DGT_EA_HL();
	void SUBNBW_wa();
	void DSUBNB_EA_BC();
	void DSUBNB_EA_DE();
	void DSUBNB_EA_HL();
	void LTAW_wa();
	void DLT_EA_BC();
	void DLT_EA_DE();
	void DLT_EA_HL();
	void ADDW_wa();
	void DADD_EA_BC();
	void DADD_EA_DE();
	void DADD_EA_HL();
	void ONAW_wa();
	void DON_EA_BC();
	void DON_EA_DE();
	void DON_EA_HL();
	void ADCW_wa();
	void DADC_EA_BC();
	void DADC_EA_DE();
	void DADC_EA_HL();
	void OFFAW_wa();
	void DOFF_EA_BC();
	void DOFF_EA_DE();
	void DOFF_EA_HL();
	void SUBW_wa();
	void DSUB_EA_BC();
	void DSUB_EA_DE();
	void DSUB_EA_HL();
	void NEAW_wa();
	void DNE_EA_BC();
	void DNE_EA_DE();
	void DNE_EA_HL();
	void SBBW_wa();
	void DSBB_EA_BC();
	void DSBB_EA_DE();
	void DSBB_EA_HL();
	void EQAW_wa();
	void DEQ_EA_BC();
	void DEQ_EA_DE();
	void DEQ_EA_HL();
	void NOP();
	void LDAW_wa();
	void INX_SP();
	void DCX_SP();
	void LXI_S_w();
	void ANIW_wa_xx();
	void MOV_A_EAH();
	void MOV_A_EAL();
	void MOV_A_B();
	void MOV_A_C();
	void MOV_A_D();
	void MOV_A_E();
	void MOV_A_H();
	void MOV_A_L();
	void EXA();
	void EXX();
	void EXR();
	void INX_BC();
	void DCX_BC();
	void LXI_B_w();
	void ORIW_wa_xx();
	void MOV_EAH_A();
	void MOV_EAL_A();
	void MOV_B_A();
	void MOV_C_A();
	void MOV_D_A();
	void MOV_E_A();
	void MOV_H_A();
	void MOV_L_A();
	void INRW_wa();
	void JB();
	void INX_DE();
	void DCX_DE();
	void LXI_D_w();
	void GTIW_wa_xx();
	void LDAX_B();
	void LDAX_D();
	void LDAX_H();
	void LDAX_Dp();
	void LDAX_Hp();
	void LDAX_Dm();
	void LDAX_Hm();
	void DCRW_wa();
	void BLOCK();
	void INX_HL();
	void DCX_HL();
	void LXI_H_w();
	void LTIW_wa_xx();
	void STAX_B();
	void STAX_D();
	void STAX_H();
	void STAX_Dp();
	void STAX_Hp();
	void STAX_Dm();
	void STAX_Hm();
	void CALL_w();
	void INR_A();
	void INR_B();
	void INR_C();
	void LXI_EA_s();
	void ONIW_wa_xx();
	void PRE_48();
	void MVIX_BC_xx();
	void MVIX_DE_xx();
	void MVIX_HL_xx();
	void PRE_4C();
	void PRE_4D();
	void JRE();
	void EXH();
	void DCR_A();
	void DCR_B();
	void DCR_C();
	void JMP_w();
	void OFFIW_wa_xx();
	void BIT_0_wa();
	void BIT_1_wa();
	void BIT_2_wa();
	void BIT_3_wa();
	void BIT_4_wa();
	void BIT_5_wa();
	void BIT_6_wa();
	void BIT_7_wa();
	void SKN_bit();
	void SETB();
	void CLR();
	void SK_bit();
	void PRE_60();
	void DAA();
	void RETI();
	void STAW_wa();
	void PRE_64();
	void NEIW_wa_xx();
	void MVI_V_xx();
	void MVI_A_xx();
	void MVI_B_xx();
	void MVI_C_xx();
	void MVI_D_xx();
	void MVI_E_xx();
	void MVI_H_xx();
	void MVI_L_xx();
	void PRE_70();
	void MVIW_wa_xx();
	void SOFTI();
	void PRE_74();
	void EQIW_wa_xx();
	void CALF();
	void CALT();
	void POP_VA();
	void POP_BC();
	void POP_DE();
	void POP_HL();
	void POP_EA();
	void DMOV_EA_BC();
	void DMOV_EA_DE();
	void DMOV_EA_HL();
	void INX_EA();
	void DCX_EA();
	void EI();
	void LDAX_D_xx();
	void LDAX_H_A();
	void LDAX_H_B();
	void LDAX_H_EA();
	void LDAX_H_xx();
	void PUSH_VA();
	void PUSH_BC();
	void PUSH_DE();
	void PUSH_HL();
	void PUSH_EA();
	void DMOV_BC_EA();
	void DMOV_DE_EA();
	void DMOV_HL_EA();
	void RET();
	void RETS();
	void DI();
	void STAX_D_xx();
	void STAX_H_A();
	void STAX_H_B();
	void STAX_H_EA();
	void STAX_H_xx();
	void JR();
	void CALT_7801();
	void IN();
	void OUT();
	void MOV_A_S();
	void MOV_S_A();
	void PEN();
	void PER();
	void PEX();
	void SIO();
	void SKIT_F0();
	void SKNIT_F0();
	void STM();
	void STM_7801();
	void MOV_MC_A_7801();
	void base_device_start();
};


class upd78c10_device : public upd7810_device
{
public:
	// construction/destruction
	upd78c10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	upd78c10_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map);
};


class upd7811_device : public upd7810_device
{
public:
	// construction/destruction
	upd7811_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class upd78c11_device : public upd78c10_device
{
public:
	// construction/destruction
	upd78c11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class upd7807_device : public upd7810_device
{
public:
	// construction/destruction
	upd7807_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void configure_ops() override;
};


class upd7801_device : public upd7810_device
{
public:
	// construction/destruction
	upd7801_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
	virtual void execute_set_input(int inputnum, int state) override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void handle_timers(int cycles) override;
	virtual void upd7810_take_irq() override;
	virtual void configure_ops() override;
};


class upd78c05_device : public upd7810_device
{
public:
	// construction/destruction
	upd78c05_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	upd78c05_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 4); }
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void handle_timers(int cycles) override;
	virtual void configure_ops() override;
};


class upd78c06_device : public upd78c05_device
{
public:
	// construction/destruction
	upd78c06_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void configure_ops() override;
};


DECLARE_DEVICE_TYPE(UPD7810,  upd7810_device)
DECLARE_DEVICE_TYPE(UPD78C10, upd78c10_device)
DECLARE_DEVICE_TYPE(UPD7811,  upd7811_device)
DECLARE_DEVICE_TYPE(UPD78C11, upd78c11_device)
DECLARE_DEVICE_TYPE(UPD7807,  upd7807_device)
DECLARE_DEVICE_TYPE(UPD7801,  upd7801_device)
DECLARE_DEVICE_TYPE(UPD78C05, upd78c05_device)
DECLARE_DEVICE_TYPE(UPD78C06, upd78c06_device)

#endif // MAME_CPU_UPD7810_UPD7810_H
