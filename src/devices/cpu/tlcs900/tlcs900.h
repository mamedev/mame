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


#define MCFG_TLCS900H_AM8_16( am8_16 ) tlcs900h_device::set_am8_16( *device, am8_16 );

class tlcs900h_device : public cpu_device
{
public:
	// static configuration helpers
	static void set_am8_16(device_t &device, int am8_16) { downcast<tlcs900h_device &>(device).m_am8_16 = am8_16; }

protected:
	// construction/destruction
	tlcs900h_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; } /* FIXME */
	virtual uint32_t execute_max_cycles() const override { return 1; } /* FIXME */
	virtual uint32_t execute_input_lines() const override { return 6; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual util::disasm_interface *create_disassembler() override;

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
	void _ADCBMI();
	void _ADCBMR();
	void _ADCBRI();
	void _ADCBRM();
	void _ADCBRR();
	void _ADCWMI();
	void _ADCWMR();
	void _ADCWRI();
	void _ADCWRM();
	void _ADCWRR();
	void _ADCLMR();
	void _ADCLRI();
	void _ADCLRM();
	void _ADCLRR();
	void _ADDBMI();
	void _ADDBMR();
	void _ADDBRI();
	void _ADDBRM();
	void _ADDBRR();
	void _ADDWMI();
	void _ADDWMR();
	void _ADDWRI();
	void _ADDWRM();
	void _ADDWRR();
	void _ADDLMR();
	void _ADDLRI();
	void _ADDLRM();
	void _ADDLRR();
	void _ANDBMI();
	void _ANDBMR();
	void _ANDBRI();
	void _ANDBRM();
	void _ANDBRR();
	void _ANDWMI();
	void _ANDWMR();
	void _ANDWRI();
	void _ANDWRM();
	void _ANDWRR();
	void _ANDLMR();
	void _ANDLRI();
	void _ANDLRM();
	void _ANDLRR();
	void _ANDCFBIM();
	void _ANDCFBIR();
	void _ANDCFBRM();
	void _ANDCFBRR();
	void _ANDCFWIR();
	void _ANDCFWRR();
	void _BITBIM();
	void _BITBIR();
	void _BITWIR();
	void _BS1BRR();
	void _BS1FRR();
	void _CALLI();
	void _CALLM();
	void _CALR();
	void _CCF();
	void _CHGBIM();
	void _CHGBIR();
	void _CHGWIR();
	void _CPBMI();
	void _CPBMR();
	void _CPBRI();
	void _CPBRM();
	void _CPBRR();
	void _CPWMI();
	void _CPWMR();
	void _CPWRI();
	void _CPWRM();
	void _CPWRR();
	void _CPLMR();
	void _CPLRI();
	void _CPLRM();
	void _CPLRR();
	void _CPD();
	void _CPDR();
	void _CPDW();
	void _CPDRW();
	void _CPI();
	void _CPIR();
	void _CPIW();
	void _CPIRW();
	void _CPLBR();
	void _CPLWR();
	void _DAABR();
	void _DB();
	void _DECBIM();
	void _DECBIR();
	void _DECWIM();
	void _DECWIR();
	void _DECLIR();
	void _DECF();
	void _DIVBRI();
	void _DIVBRM();
	void _DIVBRR();
	void _DIVWRI();
	void _DIVWRM();
	void _DIVWRR();
	void _DIVSBRI();
	void _DIVSBRM();
	void _DIVSBRR();
	void _DIVSWRI();
	void _DIVSWRM();
	void _DIVSWRR();
	void _DJNZB();
	void _DJNZW();
	void _EI();
	void _EXBMR();
	void _EXBRR();
	void _EXWMR();
	void _EXWRR();
	void _EXTSWR();
	void _EXTSLR();
	void _EXTZWR();
	void _EXTZLR();
	void _HALT();
	void _INCBIM();
	void _INCBIR();
	void _INCWIM();
	void _INCWIR();
	void _INCLIR();
	void _INCF();
	void _JPI();
	void _JPM();
	void _JR();
	void _JRL();
	void _LDBMI();
	void _LDBMM();
	void _LDBMR();
	void _LDBRI();
	void _LDBRM();
	void _LDBRR();
	void _LDWMI();
	void _LDWMM();
	void _LDWMR();
	void _LDWRI();
	void _LDWRM();
	void _LDWRR();
	void _LDLRI();
	void _LDLRM();
	void _LDLRR();
	void _LDLMR();
	void _LDAW();
	void _LDAL();
	void _LDCBRR();
	void _LDCWRR();
	void _LDCLRR();
	void _LDCFBIM();
	void _LDCFBIR();
	void _LDCFBRM();
	void _LDCFBRR();
	void _LDCFWIR();
	void _LDCFWRR();
	void _LDD();
	void _LDDR();
	void _LDDRW();
	void _LDDW();
	void _LDF();
	void _LDI();
	void _LDIR();
	void _LDIRW();
	void _LDIW();
	void _LDX();
	void _LINK();
	void _MAX();
	void _MDEC1();
	void _MDEC2();
	void _MDEC4();
	void _MINC1();
	void _MINC2();
	void _MINC4();
	void _MIRRW();
	void _MULBRI();
	void _MULBRM();
	void _MULBRR();
	void _MULWRI();
	void _MULWRM();
	void _MULWRR();
	void _MULAR();
	void _MULSBRI();
	void _MULSBRM();
	void _MULSBRR();
	void _MULSWRI();
	void _MULSWRM();
	void _MULSWRR();
	void _NEGBR();
	void _NEGWR();
	void _NOP();
	void _NORMAL();
	void _ORBMI();
	void _ORBMR();
	void _ORBRI();
	void _ORBRM();
	void _ORBRR();
	void _ORWMI();
	void _ORWMR();
	void _ORWRI();
	void _ORWRM();
	void _ORWRR();
	void _ORLMR();
	void _ORLRI();
	void _ORLRM();
	void _ORLRR();
	void _ORCFBIM();
	void _ORCFBIR();
	void _ORCFBRM();
	void _ORCFBRR();
	void _ORCFWIR();
	void _ORCFWRR();
	void _PAAWR();
	void _PAALR();
	void _POPBM();
	void _POPBR();
	void _POPWM();
	void _POPWR();
	void _POPWSR();
	void _POPLR();
	void _PUSHBI();
	void _PUSHBM();
	void _PUSHBR();
	void _PUSHWI();
	void _PUSHWM();
	void _PUSHWR();
	void _PUSHLR();
	void _RCF();
	void _RESBIM();
	void _RESBIR();
	void _RESWIR();
	void _RET();
	void _RETCC();
	void _RETD();
	void _RETI();
	void _RLBM();
	void _RLWM();
	void _RLBIR();
	void _RLBRR();
	void _RLWIR();
	void _RLWRR();
	void _RLLIR();
	void _RLLRR();
	void _RLCBM();
	void _RLCWM();
	void _RLCBIR();
	void _RLCBRR();
	void _RLCWIR();
	void _RLCWRR();
	void _RLCLIR();
	void _RLCLRR();
	void _RLDRM();
	void _RRBM();
	void _RRWM();
	void _RRBIR();
	void _RRBRR();
	void _RRWIR();
	void _RRWRR();
	void _RRLIR();
	void _RRLRR();
	void _RRCBM();
	void _RRCWM();
	void _RRCBIR();
	void _RRCBRR();
	void _RRCWIR();
	void _RRCWRR();
	void _RRCLIR();
	void _RRCLRR();
	void _RRDRM();
	void _SBCBMI();
	void _SBCBMR();
	void _SBCBRI();
	void _SBCBRM();
	void _SBCBRR();
	void _SBCWMI();
	void _SBCWMR();
	void _SBCWRI();
	void _SBCWRM();
	void _SBCWRR();
	void _SBCLMR();
	void _SBCLRI();
	void _SBCLRM();
	void _SBCLRR();
	void _SCCBR();
	void _SCCWR();
	void _SCF();
	void _SETBIM();
	void _SETBIR();
	void _SETWIR();
	void _SLABM();
	void _SLAWM();
	void _SLABIR();
	void _SLABRR();
	void _SLAWIR();
	void _SLAWRR();
	void _SLALIR();
	void _SLALRR();
	void _SLLBM();
	void _SLLWM();
	void _SLLBIR();
	void _SLLBRR();
	void _SLLWIR();
	void _SLLWRR();
	void _SLLLIR();
	void _SLLLRR();
	void _SRABM();
	void _SRAWM();
	void _SRABIR();
	void _SRABRR();
	void _SRAWIR();
	void _SRAWRR();
	void _SRALIR();
	void _SRALRR();
	void _SRLBM();
	void _SRLWM();
	void _SRLBIR();
	void _SRLBRR();
	void _SRLWIR();
	void _SRLWRR();
	void _SRLLIR();
	void _SRLLRR();
	void _STCFBIM();
	void _STCFBIR();
	void _STCFBRM();
	void _STCFBRR();
	void _STCFWIR();
	void _STCFWRR();
	void _SUBBMI();
	void _SUBBMR();
	void _SUBBRI();
	void _SUBBRM();
	void _SUBBRR();
	void _SUBWMI();
	void _SUBWMR();
	void _SUBWRI();
	void _SUBWRM();
	void _SUBWRR();
	void _SUBLMR();
	void _SUBLRI();
	void _SUBLRM();
	void _SUBLRR();
	void _SWI();
	void _TSETBIM();
	void _TSETBIR();
	void _TSETWIR();
	void _UNLK();
	void _XORBMI();
	void _XORBMR();
	void _XORBRI();
	void _XORBRM();
	void _XORBRR();
	void _XORWMI();
	void _XORWMR();
	void _XORWRI();
	void _XORWRM();
	void _XORWRR();
	void _XORLMR();
	void _XORLRI();
	void _XORLRM();
	void _XORLRR();
	void _XORCFBIM();
	void _XORCFBIR();
	void _XORCFBRM();
	void _XORCFBRR();
	void _XORCFWIR();
	void _XORCFWRR();
	void _ZCF();
	void prepare_operands(const tlcs900inst *inst);
	void _80();
	void _88();
	void _90();
	void _98();
	void _A0();
	void _A8();
	void _B0();
	void _B8();
	void _C0();
	void oC8();
	void _D0();
	void oD8();
	void _E0();
	void _E8();
	void _F0();
};

#define MCFG_TMP95C061_PORT1_READ( _port_read ) \
	devcb = &tmp95c061_device::set_port1_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C061_PORT1_WRITE( _port_write ) \
	devcb = &tmp95c061_device::set_port1_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C061_PORT2_WRITE( _port_write ) \
	devcb = &tmp95c061_device::set_port2_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C061_PORT5_READ( _port_read ) \
	devcb = &tmp95c061_device::set_port5_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C061_PORT5_WRITE( _port_write ) \
	devcb = &tmp95c061_device::set_port5_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C061_PORT6_READ( _port_read ) \
	devcb = &tmp95c061_device::set_port6_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C061_PORT6_WRITE( _port_write ) \
	devcb = &tmp95c061_device::set_port6_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C061_PORT7_READ( _port_read ) \
	devcb = &tmp95c061_device::set_port7_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C061_PORT7_WRITE( _port_write ) \
	devcb = &tmp95c061_device::set_port7_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C061_PORT8_READ( _port_read ) \
	devcb = &tmp95c061_device::set_port8_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C061_PORT8_WRITE( _port_write ) \
	devcb = &mp95c061_device::set_port8_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C061_PORT9_READ( _port_read ) \
	devcb = &tmp95c061_device::set_port9_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C061_PORTA_READ( _port_read ) \
	devcb = &tmp95c061_device::set_porta_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C061_PORTA_WRITE( _port_write ) \
	devcb = &tmp95c061_device::set_porta_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C061_PORTB_READ( _port_read ) \
	devcb = &tmp95c061_device::set_portb_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C061_PORTB_WRITE( _port_write ) \
	devcb = &tmp95c061_device::set_portb_write( *device, DEVCB_##_port_write );

class tmp95c061_device : public tlcs900h_device
{
public:
	// construction/destruction
	tmp95c061_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template <class Object> static devcb_base &set_port1_read(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port1_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port1_write(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port1_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port2_write(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port2_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port5_read(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port5_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port5_write(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port5_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port6_write(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port6_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port7_read(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port7_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port7_write(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port7_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port8_read(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port8_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port8_write(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port8_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port9_read(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_port9_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_porta_read(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_porta_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_porta_write(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_porta_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_portb_read(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_portb_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_portb_write(device_t &device, Object &&cb) { return downcast<tmp95c061_device &>(device).m_portb_write.set_callback(std::forward<Object>(cb)); }

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


#define MCFG_TMP95C063_PORT0_READ( _port_read ) tmp95c063_device::set_port0_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT0_WRITE( _port_write ) tmp95c063_device::set_port0_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORT1_READ( _port_read ) tmp95c063_device::set_port1_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT1_WRITE( _port_write ) tmp95c063_device::set_port1_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORT2_READ( _port_read ) tmp95c063_device::set_port2_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT2_WRITE( _port_write ) tmp95c063_device::set_port2_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORT3_READ( _port_read ) tmp95c063_device::set_port3_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT3_WRITE( _port_write ) tmp95c063_device::set_port3_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORT4_READ( _port_read ) tmp95c063_device::set_port4_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT4_WRITE( _port_write ) tmp95c063_device::set_port4_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORT5_READ( _port_read ) tmp95c063_device::set_port5_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT5_WRITE( _port_write ) tmp95c063_device::set_port5_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORT6_READ( _port_read ) tmp95c063_device::set_port6_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT6_WRITE( _port_write ) tmp95c063_device::set_port6_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORT7_READ( _port_read ) tmp95c063_device::set_port7_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT7_WRITE( _port_write ) tmp95c063_device::set_port7_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORT8_READ( _port_read ) tmp95c063_device::set_port8_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT8_WRITE( _port_write ) tmp95c063_device::set_port8_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORT9_READ( _port_read ) tmp95c063_device::set_port9_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORT9_WRITE( _port_write ) tmp95c063_device::set_port9_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORTA_READ( _port_read ) tmp95c063_device::set_porta_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORTA_WRITE( _port_write ) tmp95c063_device::set_porta_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORTB_READ( _port_read ) tmp95c063_device::set_portb_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORTB_WRITE( _port_write ) tmp95c063_device::set_portb_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORTC_READ( _port_read ) tmp95c063_device::set_portc_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORTC_WRITE( _port_write ) tmp95c063_device::set_portc_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORTD_READ( _port_read ) tmp95c063_device::set_portd_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORTD_WRITE( _port_write ) tmp95c063_device::set_portd_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_PORTE_READ( _port_read ) tmp95c063_device::set_porte_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_PORTE_WRITE( _port_write ) tmp95c063_device::set_porte_write( *device, DEVCB_##_port_write );
#define MCFG_TMP95C063_AN0_READ( _port_read ) tmp95c063_device::set_an0_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_AN1_READ( _port_read ) tmp95c063_device::set_an1_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_AN2_READ( _port_read ) tmp95c063_device::set_an2_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_AN3_READ( _port_read ) tmp95c063_device::set_an3_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_AN4_READ( _port_read ) tmp95c063_device::set_an4_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_AN5_READ( _port_read ) tmp95c063_device::set_an5_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_AN6_READ( _port_read ) tmp95c063_device::set_an6_read( *device, DEVCB_##_port_read );
#define MCFG_TMP95C063_AN7_READ( _port_read ) tmp95c063_device::set_an7_read( *device, DEVCB_##_port_read );

class tmp95c063_device : public tlcs900h_device
{
public:
	// construction/destruction
	tmp95c063_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( internal_r );
	DECLARE_WRITE8_MEMBER( internal_w );

	// static configuration helpers
	template <class Object> static devcb_base &set_port1_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port1_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port1_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port1_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port2_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port2_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port5_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port5_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port5_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port5_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port6_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port6_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port6_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port6_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port7_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port7_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port7_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port7_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port8_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port8_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port8_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port8_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port9_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port9_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port9_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_port9_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_porta_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_porta_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_porta_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_porta_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_portb_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_portb_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_portb_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_portb_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_portc_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_portc_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_portd_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_portd_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_portd_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_portd_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_porte_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_porte_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_porte_write(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_porte_write.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_an0_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_an0_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_an1_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_an1_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_an2_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_an2_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_an3_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_an3_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_an4_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_an4_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_an5_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_an5_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_an6_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_an6_read.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_an7_read(device_t &device, Object &&cb) { return downcast<tmp95c063_device &>(device).m_an7_read.set_callback(std::forward<Object>(cb)); }

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
	devcb_read16       m_an0_read;
	devcb_read16       m_an1_read;
	devcb_read16       m_an2_read;
	devcb_read16       m_an3_read;
	devcb_read16       m_an4_read;
	devcb_read16       m_an5_read;
	devcb_read16       m_an6_read;
	devcb_read16       m_an7_read;
};

#endif // MAME_CPU_TLCS900_TLCS900_H
