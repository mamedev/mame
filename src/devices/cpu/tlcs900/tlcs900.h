// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_CPU_TLCS900_TLCS900_H
#define MAME_CPU_TLCS900_TLCS900_H

#pragma once


enum tlcs900_inputs
{
	TLCS900_NMI=0,
	TLCS900_INTWD,
	TLCS900_INT0,
	TLCS900_INTAD,
	TLCS900_INT1,
	TLCS900_INT2,
	TLCS900_INT3,
	TLCS900_INT4,
	TLCS900_INT5,
	TLCS900_INT6,
	TLCS900_INT7,
	TLCS900_INT8,
	TLCS900_TIO,
	TLCS900_NUM_INPUTS
};


enum
{
	TLCS900_PC=1, TLCS900_SR,
	TLCS900_XWA0, TLCS900_XBC0, TLCS900_XDE0, TLCS900_XHL0,
	TLCS900_XWA1, TLCS900_XBC1, TLCS900_XDE1, TLCS900_XHL1,
	TLCS900_XWA2, TLCS900_XBC2, TLCS900_XDE2, TLCS900_XHL2,
	TLCS900_XWA3, TLCS900_XBC3, TLCS900_XDE3, TLCS900_XHL3,
	TLCS900_XIX, TLCS900_XIY, TLCS900_XIZ, TLCS900_XNSP, TLCS900_XSSP,
	TLCS900_DMAS0, TLCS900_DMAS1, TLCS900_DMAS2, TLCS900_DMAS3,
	TLCS900_DMAD0, TLCS900_DMAD1, TLCS900_DMAD2, TLCS900_DMAD3,
	TLCS900_DMAC0, TLCS900_DMAC1, TLCS900_DMAC2, TLCS900_DMAC3,
	TLCS900_DMAM0, TLCS900_DMAM1, TLCS900_DMAM2, TLCS900_DMAM3
};


DECLARE_DEVICE_TYPE(TMP95C061, tmp95c061_device)
DECLARE_DEVICE_TYPE(TMP95C063, tmp95c063_device)


class tlcs900h_device : public cpu_device
{
public:
	// configuration helpers
	void set_am8_16(int am8_16) { m_am8_16 = am8_16; }

protected:
	// construction/destruction
	tlcs900h_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; } /* FIXME */
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; } /* FIXME */
	virtual uint32_t execute_input_lines() const noexcept override { return 6; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

protected:
	int m_am8_16;
	address_space_config m_program_config;

	uint8_t RDMEM(offs_t addr) { return m_program->read_byte( addr ); }
	uint16_t RDMEMW(offs_t addr) { return m_program->read_word( addr ); }
	uint32_t RDMEML(offs_t addr) { return m_program->read_dword( addr ); }
	void WRMEM(offs_t addr, uint8_t data) { m_program->write_byte( addr, data ); }
	void WRMEMW(offs_t addr,uint16_t data) { m_program->write_word( addr, data ); }
	void WRMEML(offs_t addr,uint32_t data) { m_program->write_dword( addr, data ); }

	/* registers */
	PAIR    m_xwa[4];
	PAIR    m_xbc[4];
	PAIR    m_xde[4];
	PAIR    m_xhl[4];
	PAIR    m_xix;
	PAIR    m_xiy;
	PAIR    m_xiz;
	PAIR    m_xssp;
	PAIR    m_xnsp;
	PAIR    m_pc;
	PAIR    m_sr;
	PAIR    m_f2; /* f' */
	/* DMA registers */
	PAIR    m_dmas[4];
	PAIR    m_dmad[4];
	PAIR    m_dmac[4];
	PAIR    m_dmam[4];

	/* Internal timers, irqs, etc */
	uint8_t   m_reg[0xa0];
	uint32_t  m_timer_pre;
	uint8_t   m_timer[6];
	int     m_timer_change[4];
	bool    m_prefetch_clear;
	uint8_t   m_prefetch_index;
	uint8_t   m_prefetch[4];

	/* Current state of input levels */
	int     m_level[TLCS900_NUM_INPUTS];
	int     m_check_irqs;
	int     m_ad_cycles_left;
	int     m_nmi_state;

	/* used during execution */
	PAIR    m_dummy; /* for illegal register references */
	uint8_t   m_op;
	PAIR    m_ea1, m_ea2;
	PAIR    m_imm1, m_imm2;
	int m_cycles;
	uint8_t   *m_p1_reg8, *m_p2_reg8;
	uint16_t  *m_p1_reg16, *m_p2_reg16;
	uint32_t  *m_p1_reg32, *m_p2_reg32;

	int m_halted;
	int m_icount;
	int m_regbank;
	address_space *m_program;

	typedef void (tlcs900h_device::*ophandler)();
	struct tlcs900inst
	{
		ophandler opfunc;
		int     operand1;
		int     operand2;
		int     cycles;
	};
	static const tlcs900inst s_mnemonic_80[256];
	static const tlcs900inst s_mnemonic_88[256];
	static const tlcs900inst s_mnemonic_90[256];
	static const tlcs900inst s_mnemonic_98[256];
	static const tlcs900inst s_mnemonic_a0[256];
	static const tlcs900inst s_mnemonic_b0[256];
	static const tlcs900inst s_mnemonic_b8[256];
	static const tlcs900inst s_mnemonic_c0[256];
	static const tlcs900inst s_mnemonic_c8[256];
	static const tlcs900inst s_mnemonic_d0[256];
	static const tlcs900inst s_mnemonic_d8[256];
	static const tlcs900inst s_mnemonic_e0[256];
	static const tlcs900inst s_mnemonic_e8[256];
	static const tlcs900inst s_mnemonic_f0[256];
	static const tlcs900inst s_mnemonic[256];

	inline uint8_t RDOP();
	virtual void tlcs900_check_hdma() = 0;
	virtual void tlcs900_check_irqs() = 0;
	virtual void tlcs900_handle_ad() = 0;
	virtual void tlcs900_handle_timers() = 0;

	int condition_true( uint8_t cond );
	uint8_t *get_reg8_current( uint8_t reg );
	uint16_t *get_reg16_current( uint8_t reg );
	uint32_t *get_reg32_current( uint8_t reg );
	PAIR *get_reg( uint8_t reg );
	uint8_t *get_reg8( uint8_t reg );
	uint16_t *get_reg16( uint8_t reg );
	uint32_t *get_reg32( uint8_t reg );
	void parity8( uint8_t a );
	void parity16( uint16_t a );
	void parity32( uint32_t a );
	uint8_t adc8( uint8_t a, uint8_t b);
	uint16_t adc16( uint16_t a, uint16_t b);
	uint32_t adc32( uint32_t a, uint32_t b);
	uint8_t add8( uint8_t a, uint8_t b);
	uint16_t add16( uint16_t a, uint16_t b);
	uint32_t add32( uint32_t a, uint32_t b);
	uint8_t sbc8( uint8_t a, uint8_t b);
	uint16_t sbc16( uint16_t a, uint16_t b);
	uint32_t sbc32( uint32_t a, uint32_t b);
	uint8_t sub8( uint8_t a, uint8_t b);
	uint16_t sub16( uint16_t a, uint16_t b);
	uint32_t sub32( uint32_t a, uint32_t b);
	uint8_t and8( uint8_t a, uint8_t b);
	uint16_t and16( uint16_t a, uint16_t b);
	uint32_t and32( uint32_t a, uint32_t b);
	uint8_t or8( uint8_t a, uint8_t b);
	uint16_t or16( uint16_t a, uint16_t b);
	uint32_t or32( uint32_t a, uint32_t b);
	uint8_t xor8( uint8_t a, uint8_t b);
	uint16_t xor16( uint16_t a, uint16_t b);
	uint32_t xor32( uint32_t a, uint32_t b);
	void ldcf8( uint8_t a, uint8_t b );
	void ldcf16( uint8_t a, uint8_t b );
	void andcf8( uint8_t a, uint8_t b );
	void andcf16( uint8_t a, uint8_t b );
	void orcf8( uint8_t a, uint8_t b );
	void orcf16( uint8_t a, uint8_t b );
	void xorcf8( uint8_t a, uint8_t b );
	void xorcf16( uint8_t a, uint8_t b );
	uint8_t rl8( uint8_t a, uint8_t s );
	uint16_t rl16( uint16_t a, uint8_t s );
	uint32_t rl32( uint32_t a, uint8_t s );
	uint8_t rlc8( uint8_t a, uint8_t s );
	uint16_t rlc16( uint16_t a, uint8_t s );
	uint32_t rlc32( uint32_t a, uint8_t s );
	uint8_t rr8( uint8_t a, uint8_t s );
	uint16_t rr16( uint16_t a, uint8_t s );
	uint32_t rr32( uint32_t a, uint8_t s );
	uint8_t rrc8( uint8_t a, uint8_t s );
	uint16_t rrc16( uint16_t a, uint8_t s );
	uint32_t rrc32( uint32_t a, uint8_t s );
	uint8_t sla8( uint8_t a, uint8_t s );
	uint16_t sla16( uint16_t a, uint8_t s );
	uint32_t sla32( uint32_t a, uint8_t s );
	uint8_t sra8( uint8_t a, uint8_t s );
	uint16_t sra16( uint16_t a, uint8_t s );
	uint32_t sra32( uint32_t a, uint8_t s );
	uint8_t srl8( uint8_t a, uint8_t s );
	uint16_t srl16( uint16_t a, uint8_t s );
	uint32_t srl32( uint32_t a, uint8_t s );
	uint16_t div8( uint16_t a, uint8_t b );
	uint32_t div16( uint32_t a, uint16_t b );
	uint16_t divs8( int16_t a, int8_t b );
	uint32_t divs16( int32_t a, int16_t b );
	void op_ADCBMI();
	void op_ADCBMR();
	void op_ADCBRI();
	void op_ADCBRM();
	void op_ADCBRR();
	void op_ADCWMI();
	void op_ADCWMR();
	void op_ADCWRI();
	void op_ADCWRM();
	void op_ADCWRR();
	void op_ADCLMR();
	void op_ADCLRI();
	void op_ADCLRM();
	void op_ADCLRR();
	void op_ADDBMI();
	void op_ADDBMR();
	void op_ADDBRI();
	void op_ADDBRM();
	void op_ADDBRR();
	void op_ADDWMI();
	void op_ADDWMR();
	void op_ADDWRI();
	void op_ADDWRM();
	void op_ADDWRR();
	void op_ADDLMR();
	void op_ADDLRI();
	void op_ADDLRM();
	void op_ADDLRR();
	void op_ANDBMI();
	void op_ANDBMR();
	void op_ANDBRI();
	void op_ANDBRM();
	void op_ANDBRR();
	void op_ANDWMI();
	void op_ANDWMR();
	void op_ANDWRI();
	void op_ANDWRM();
	void op_ANDWRR();
	void op_ANDLMR();
	void op_ANDLRI();
	void op_ANDLRM();
	void op_ANDLRR();
	void op_ANDCFBIM();
	void op_ANDCFBIR();
	void op_ANDCFBRM();
	void op_ANDCFBRR();
	void op_ANDCFWIR();
	void op_ANDCFWRR();
	void op_BITBIM();
	void op_BITBIR();
	void op_BITWIR();
	void op_BS1BRR();
	void op_BS1FRR();
	void op_CALLI();
	void op_CALLM();
	void op_CALR();
	void op_CCF();
	void op_CHGBIM();
	void op_CHGBIR();
	void op_CHGWIR();
	void op_CPBMI();
	void op_CPBMR();
	void op_CPBRI();
	void op_CPBRM();
	void op_CPBRR();
	void op_CPWMI();
	void op_CPWMR();
	void op_CPWRI();
	void op_CPWRM();
	void op_CPWRR();
	void op_CPLMR();
	void op_CPLRI();
	void op_CPLRM();
	void op_CPLRR();
	void op_CPD();
	void op_CPDR();
	void op_CPDW();
	void op_CPDRW();
	void op_CPI();
	void op_CPIR();
	void op_CPIW();
	void op_CPIRW();
	void op_CPLBR();
	void op_CPLWR();
	void op_DAABR();
	void op_DB();
	void op_DECBIM();
	void op_DECBIR();
	void op_DECWIM();
	void op_DECWIR();
	void op_DECLIR();
	void op_DECF();
	void op_DIVBRI();
	void op_DIVBRM();
	void op_DIVBRR();
	void op_DIVWRI();
	void op_DIVWRM();
	void op_DIVWRR();
	void op_DIVSBRI();
	void op_DIVSBRM();
	void op_DIVSBRR();
	void op_DIVSWRI();
	void op_DIVSWRM();
	void op_DIVSWRR();
	void op_DJNZB();
	void op_DJNZW();
	void op_EI();
	void op_EXBMR();
	void op_EXBRR();
	void op_EXWMR();
	void op_EXWRR();
	void op_EXTSWR();
	void op_EXTSLR();
	void op_EXTZWR();
	void op_EXTZLR();
	void op_HALT();
	void op_INCBIM();
	void op_INCBIR();
	void op_INCWIM();
	void op_INCWIR();
	void op_INCLIR();
	void op_INCF();
	void op_JPI();
	void op_JPM();
	void op_JR();
	void op_JRL();
	void op_LDBMI();
	void op_LDBMM();
	void op_LDBMR();
	void op_LDBRI();
	void op_LDBRM();
	void op_LDBRR();
	void op_LDWMI();
	void op_LDWMM();
	void op_LDWMR();
	void op_LDWRI();
	void op_LDWRM();
	void op_LDWRR();
	void op_LDLRI();
	void op_LDLRM();
	void op_LDLRR();
	void op_LDLMR();
	void op_LDAW();
	void op_LDAL();
	void op_LDCBRR();
	void op_LDCWRR();
	void op_LDCLRR();
	void op_LDCFBIM();
	void op_LDCFBIR();
	void op_LDCFBRM();
	void op_LDCFBRR();
	void op_LDCFWIR();
	void op_LDCFWRR();
	void op_LDD();
	void op_LDDR();
	void op_LDDRW();
	void op_LDDW();
	void op_LDF();
	void op_LDI();
	void op_LDIR();
	void op_LDIRW();
	void op_LDIW();
	void op_LDX();
	void op_LINK();
	void op_MAX();
	void op_MDEC1();
	void op_MDEC2();
	void op_MDEC4();
	void op_MINC1();
	void op_MINC2();
	void op_MINC4();
	void op_MIRRW();
	void op_MULBRI();
	void op_MULBRM();
	void op_MULBRR();
	void op_MULWRI();
	void op_MULWRM();
	void op_MULWRR();
	void op_MULAR();
	void op_MULSBRI();
	void op_MULSBRM();
	void op_MULSBRR();
	void op_MULSWRI();
	void op_MULSWRM();
	void op_MULSWRR();
	void op_NEGBR();
	void op_NEGWR();
	void op_NOP();
	void op_NORMAL();
	void op_ORBMI();
	void op_ORBMR();
	void op_ORBRI();
	void op_ORBRM();
	void op_ORBRR();
	void op_ORWMI();
	void op_ORWMR();
	void op_ORWRI();
	void op_ORWRM();
	void op_ORWRR();
	void op_ORLMR();
	void op_ORLRI();
	void op_ORLRM();
	void op_ORLRR();
	void op_ORCFBIM();
	void op_ORCFBIR();
	void op_ORCFBRM();
	void op_ORCFBRR();
	void op_ORCFWIR();
	void op_ORCFWRR();
	void op_PAAWR();
	void op_PAALR();
	void op_POPBM();
	void op_POPBR();
	void op_POPWM();
	void op_POPWR();
	void op_POPWSR();
	void op_POPLR();
	void op_PUSHBI();
	void op_PUSHBM();
	void op_PUSHBR();
	void op_PUSHWI();
	void op_PUSHWM();
	void op_PUSHWR();
	void op_PUSHLR();
	void op_RCF();
	void op_RESBIM();
	void op_RESBIR();
	void op_RESWIR();
	void op_RET();
	void op_RETCC();
	void op_RETD();
	void op_RETI();
	void op_RLBM();
	void op_RLWM();
	void op_RLBIR();
	void op_RLBRR();
	void op_RLWIR();
	void op_RLWRR();
	void op_RLLIR();
	void op_RLLRR();
	void op_RLCBM();
	void op_RLCWM();
	void op_RLCBIR();
	void op_RLCBRR();
	void op_RLCWIR();
	void op_RLCWRR();
	void op_RLCLIR();
	void op_RLCLRR();
	void op_RLDRM();
	void op_RRBM();
	void op_RRWM();
	void op_RRBIR();
	void op_RRBRR();
	void op_RRWIR();
	void op_RRWRR();
	void op_RRLIR();
	void op_RRLRR();
	void op_RRCBM();
	void op_RRCWM();
	void op_RRCBIR();
	void op_RRCBRR();
	void op_RRCWIR();
	void op_RRCWRR();
	void op_RRCLIR();
	void op_RRCLRR();
	void op_RRDRM();
	void op_SBCBMI();
	void op_SBCBMR();
	void op_SBCBRI();
	void op_SBCBRM();
	void op_SBCBRR();
	void op_SBCWMI();
	void op_SBCWMR();
	void op_SBCWRI();
	void op_SBCWRM();
	void op_SBCWRR();
	void op_SBCLMR();
	void op_SBCLRI();
	void op_SBCLRM();
	void op_SBCLRR();
	void op_SCCBR();
	void op_SCCWR();
	void op_SCF();
	void op_SETBIM();
	void op_SETBIR();
	void op_SETWIR();
	void op_SLABM();
	void op_SLAWM();
	void op_SLABIR();
	void op_SLABRR();
	void op_SLAWIR();
	void op_SLAWRR();
	void op_SLALIR();
	void op_SLALRR();
	void op_SLLBM();
	void op_SLLWM();
	void op_SLLBIR();
	void op_SLLBRR();
	void op_SLLWIR();
	void op_SLLWRR();
	void op_SLLLIR();
	void op_SLLLRR();
	void op_SRABM();
	void op_SRAWM();
	void op_SRABIR();
	void op_SRABRR();
	void op_SRAWIR();
	void op_SRAWRR();
	void op_SRALIR();
	void op_SRALRR();
	void op_SRLBM();
	void op_SRLWM();
	void op_SRLBIR();
	void op_SRLBRR();
	void op_SRLWIR();
	void op_SRLWRR();
	void op_SRLLIR();
	void op_SRLLRR();
	void op_STCFBIM();
	void op_STCFBIR();
	void op_STCFBRM();
	void op_STCFBRR();
	void op_STCFWIR();
	void op_STCFWRR();
	void op_SUBBMI();
	void op_SUBBMR();
	void op_SUBBRI();
	void op_SUBBRM();
	void op_SUBBRR();
	void op_SUBWMI();
	void op_SUBWMR();
	void op_SUBWRI();
	void op_SUBWRM();
	void op_SUBWRR();
	void op_SUBLMR();
	void op_SUBLRI();
	void op_SUBLRM();
	void op_SUBLRR();
	void op_SWI();
	void op_TSETBIM();
	void op_TSETBIR();
	void op_TSETWIR();
	void op_UNLK();
	void op_XORBMI();
	void op_XORBMR();
	void op_XORBRI();
	void op_XORBRM();
	void op_XORBRR();
	void op_XORWMI();
	void op_XORWMR();
	void op_XORWRI();
	void op_XORWRM();
	void op_XORWRR();
	void op_XORLMR();
	void op_XORLRI();
	void op_XORLRM();
	void op_XORLRR();
	void op_XORCFBIM();
	void op_XORCFBIR();
	void op_XORCFBRM();
	void op_XORCFBRR();
	void op_XORCFWIR();
	void op_XORCFWRR();
	void op_ZCF();
	void prepare_operands(const tlcs900inst *inst);
	void op_80();
	void op_88();
	void op_90();
	void op_98();
	void op_A0();
	void op_A8();
	void op_B0();
	void op_B8();
	void op_C0();
	void oC8();
	void op_D0();
	void oD8();
	void op_E0();
	void op_E8();
	void op_F0();
};

class tmp95c061_device : public tlcs900h_device
{
public:
	// construction/destruction
	tmp95c061_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto port1_read()  { return m_port1_read.bind(); }
	auto port1_write() { return m_port1_write.bind(); }
	auto port2_write() { return m_port2_write.bind(); }
	auto port5_read()  { return m_port5_read.bind(); }
	auto port5_write() { return m_port5_write.bind(); }
	auto port6_write() { return m_port6_write.bind(); }
	auto port7_read()  { return m_port7_read.bind(); }
	auto port7_write() { return m_port7_write.bind(); }
	auto port8_read()  { return m_port8_read.bind(); }
	auto port8_write() { return m_port8_write.bind(); }
	auto port9_read()  { return m_port9_read.bind(); }
	auto porta_read()  { return m_porta_read.bind(); }
	auto porta_write() { return m_porta_write.bind(); }
	auto portb_read()  { return m_portb_read.bind(); }
	auto portb_write() { return m_portb_write.bind(); }

	DECLARE_READ8_MEMBER( internal_r );
	DECLARE_WRITE8_MEMBER( internal_w );

	void tmp95c061_mem16(address_map &map);
	void tmp95c061_mem8(address_map &map);
protected:
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual void tlcs900_check_hdma() override;
	virtual void tlcs900_check_irqs() override;
	virtual void tlcs900_handle_ad() override;
	virtual void tlcs900_handle_timers() override;

	void tlcs900_change_tff( int which, int change );
	int tlcs900_process_hdma( int channel );
	void update_porta();

private:
	uint8_t   m_to1;
	uint8_t   m_to3;

	// Port 1: 8 bit I/O. Shared with D8-D15
	devcb_read8    m_port1_read;
	devcb_write8   m_port1_write;

	// Port 2: 8 bit output only. Shared with A16-A23
	devcb_write8   m_port2_write;

	// Port 5: 4 bit I/O. Shared with HWR, BUSRQ, BUSAK, RW
	devcb_read8    m_port5_read;
	devcb_write8   m_port5_write;

	// Port 6: 6 bit I/O. Shared with CS0, CS1, CS3/LCAS, RAS, REFOUT
	devcb_read8    m_port6_read;
	devcb_write8   m_port6_write;

	// Port 7: 8 bit I/O. Shared with PG0-OUT, PG1-OUT
	devcb_read8    m_port7_read;
	devcb_write8   m_port7_write;

	// Port 8: 6 bit I/O. Shared with TXD0, TXD1, RXD0, RXD1, CTS0, SCLK0, SCLK1
	devcb_read8    m_port8_read;
	devcb_write8   m_port8_write;

	// Port 9: 4 bit input only. Shared with AN0-AN3
	devcb_read8    m_port9_read;

	// Port A: 4 bit I/O. Shared with WAIT, TI0, TO1, TO2
	devcb_read8    m_porta_read;
	devcb_write8   m_porta_write;

	// Port B: 8 bit I/O. Shared with TI4/INT4, TI5/INT5, TI6/INT6, TI7/INT7, TO4, TO5, TO6
	devcb_read8    m_portb_read;
	devcb_write8   m_portb_write;
};


class tmp95c063_device : public tlcs900h_device
{
public:
	// construction/destruction
	tmp95c063_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( internal_r );
	DECLARE_WRITE8_MEMBER( internal_w );

	// configuration helpers
	auto port1_read()  { return m_port1_read.bind(); }
	auto port1_write() { return m_port1_write.bind(); }
	auto port2_write() { return m_port2_write.bind(); }
	auto port5_read()  { return m_port5_read.bind(); }
	auto port5_write() { return m_port5_write.bind(); }
	auto port6_read()  { return m_port6_read.bind(); }
	auto port6_write() { return m_port6_write.bind(); }
	auto port7_read()  { return m_port7_read.bind(); }
	auto port7_write() { return m_port7_write.bind(); }
	auto port8_read()  { return m_port8_read.bind(); }
	auto port8_write() { return m_port8_write.bind(); }
	auto port9_read()  { return m_port9_read.bind(); }
	auto port9_write() { return m_port9_write.bind(); }
	auto porta_read()  { return m_porta_read.bind(); }
	auto porta_write() { return m_porta_write.bind(); }
	auto portb_read()  { return m_portb_read.bind(); }
	auto portb_write() { return m_portb_write.bind(); }
	auto portc_read()  { return m_portc_read.bind(); }
	auto portd_read()  { return m_portd_read.bind(); }
	auto portd_write() { return m_portd_write.bind(); }
	auto porte_read()  { return m_porte_read.bind(); }
	auto porte_write() { return m_porte_write.bind(); }
	template <size_t Bit> auto an_read() { return m_an_read[Bit].bind(); }

	void tmp95c063_mem16(address_map &map);
	void tmp95c063_mem8(address_map &map);
protected:
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void execute_set_input(int inputnum, int state) override;
	virtual void tlcs900_check_hdma() override;
	virtual void tlcs900_check_irqs() override;
	virtual void tlcs900_handle_ad() override;
	virtual void tlcs900_handle_timers() override;

private:
	// Port 1: 8 bit I/O. Shared with d8-d15
	devcb_read8    m_port1_read;
	devcb_write8   m_port1_write;

	// Port 2: 8 bit output only. Shared with a16-a23
	devcb_write8   m_port2_write;

	// Port 5: 6 bit I/O
	devcb_read8    m_port5_read;
	devcb_write8   m_port5_write;

	// Port 6: 8 bit I/O. Shared with cs1, cs3 & dram control
	devcb_read8    m_port6_read;
	devcb_write8   m_port6_write;

	// Port 7: 8 bit I/O
	devcb_read8    m_port7_read;
	devcb_write8   m_port7_write;

	// Port 8: 8 bit I/O. Shared with SCOUT, WAIT, NMI2, INT0-INT3
	devcb_read8    m_port8_read;
	devcb_write8   m_port8_write;

	// Port 9: 8 bit I/O. Shared with clock input and output for the 8-bit timers
	devcb_read8    m_port9_read;
	devcb_write8   m_port9_write;

	// Port A: 8 bit I/O. Shared with serial channels 0/1
	devcb_read8    m_porta_read;
	devcb_write8   m_porta_write;

	// Port B: 8 bit I/O. Shared with 16bit timers
	devcb_read8    m_portb_read;
	devcb_write8   m_portb_write;

	// Port C: 8 bit input only. Shared with analogue inputs
	devcb_read8    m_portc_read;

	// Port D: 5 bit I/O. Shared with int8_t
	devcb_read8    m_portd_read;
	devcb_write8   m_portd_write;

	// Port E: 8 bit I/O.
	devcb_read8    m_porte_read;
	devcb_write8   m_porte_write;

	// analogue inputs, sampled at 10 bits
	devcb_read16   m_an_read[8];
};

#endif // MAME_CPU_TLCS900_TLCS900_H
