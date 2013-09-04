#ifndef __TLCS900_H__
#define __TLCS900_H__



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


extern const device_type TLCS900H;
extern const device_type TMP95C063;


#define MCFG_TLCS900_CONFIG( _to1, _to3, _port_read, _port_write ) \
	tlcs900h_device::set_to1( *device, DEVCB2_##_to1 ); \
	tlcs900h_device::set_to3( *device, DEVCB2_##_to3 ); \
	tlcs900h_device::set_port_read( *device, DEVCB2_##_port_read ); \
	tlcs900h_device::set_port_write( *device, DEVCB2_##_port_write );


class tlcs900h_device : public cpu_device
{
public:
	// construction/destruction
	tlcs900h_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tlcs900h_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, address_map_constructor internal_map);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_to1(device_t &device, _Object object) { return downcast<tlcs900h_device &>(device).m_to1.set_callback(object); }
	template<class _Object> static devcb2_base &set_to3(device_t &device, _Object object) { return downcast<tlcs900h_device &>(device).m_to3.set_callback(object); }
	template<class _Object> static devcb2_base &set_port_read(device_t &device, _Object object) { return downcast<tlcs900h_device &>(device).m_port_read.set_callback(object); }
	template<class _Object> static devcb2_base &set_port_write(device_t &device, _Object object) { return downcast<tlcs900h_device &>(device).m_port_write.set_callback(object); }

	DECLARE_READ8_MEMBER( tlcs900_internal_r );
	DECLARE_WRITE8_MEMBER( tlcs900_internal_w );
	DECLARE_READ8_MEMBER( tmp95c063_internal_r );
	DECLARE_WRITE8_MEMBER( tmp95c063_internal_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; } /* FIXME */
	virtual UINT32 execute_max_cycles() const { return 1; } /* FIXME */
	virtual UINT32 execute_input_lines() const { return 6; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : NULL;
	}

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 7; } /* FIXME */
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

protected:
	address_space_config m_program_config;

	devcb2_write8   m_to1;
	devcb2_write8   m_to3;

	devcb2_read8    m_port_read;
	devcb2_write8   m_port_write;

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
	UINT8   m_reg[0xa0];
	UINT32  m_timer_pre;
	UINT8   m_timer[6];
	UINT8   m_tff1;
	UINT8   m_tff3;
	int     m_timer_change[4];
	bool    m_prefetch_clear;
	UINT8   m_prefetch_index;
	UINT8   m_prefetch[4];

	/* Current state of input levels */
	int     m_level[TLCS900_NUM_INPUTS];
	int     m_check_irqs;
	int     m_ad_cycles_left;
	int     m_nmi_state;

	/* used during execution */
	PAIR    m_dummy; /* for illegal register references */
	UINT8   m_op;
	PAIR    m_ea1, m_ea2;
	PAIR    m_imm1, m_imm2;
	int m_cycles;
	UINT8   *m_p1_reg8, *m_p2_reg8;
	UINT16  *m_p1_reg16, *m_p2_reg16;
	UINT32  *m_p1_reg32, *m_p2_reg32;

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

	inline UINT8 RDOP();
	int tlcs900_process_hdma( int channel );
	virtual void tlcs900_check_hdma();
	virtual void tlcs900_check_irqs();
	virtual void tlcs900_handle_ad();
	void tlcs900_change_tff( int which, int change );
	virtual void tlcs900_handle_timers();

	int condition_true( UINT8 cond );
	UINT8 *get_reg8_current( UINT8 reg );
	UINT16 *get_reg16_current( UINT8 reg );
	UINT32 *get_reg32_current( UINT8 reg );
	PAIR *get_reg( UINT8 reg );
	UINT8 *get_reg8( UINT8 reg );
	UINT16 *get_reg16( UINT8 reg );
	UINT32 *get_reg32( UINT8 reg );
	void parity8( UINT8 a );
	void parity16( UINT16 a );
	void parity32( UINT32 a );
	UINT8 adc8( UINT8 a, UINT8 b);
	UINT16 adc16( UINT16 a, UINT16 b);
	UINT32 adc32( UINT32 a, UINT32 b);
	UINT8 add8( UINT8 a, UINT8 b);
	UINT16 add16( UINT16 a, UINT16 b);
	UINT32 add32( UINT32 a, UINT32 b);
	UINT8 sbc8( UINT8 a, UINT8 b);
	UINT16 sbc16( UINT16 a, UINT16 b);
	UINT32 sbc32( UINT32 a, UINT32 b);
	UINT8 sub8( UINT8 a, UINT8 b);
	UINT16 sub16( UINT16 a, UINT16 b);
	UINT32 sub32( UINT32 a, UINT32 b);
	UINT8 and8( UINT8 a, UINT8 b);
	UINT16 and16( UINT16 a, UINT16 b);
	UINT32 and32( UINT32 a, UINT32 b);
	UINT8 or8( UINT8 a, UINT8 b);
	UINT16 or16( UINT16 a, UINT16 b);
	UINT32 or32( UINT32 a, UINT32 b);
	UINT8 xor8( UINT8 a, UINT8 b);
	UINT16 xor16( UINT16 a, UINT16 b);
	UINT32 xor32( UINT32 a, UINT32 b);
	void ldcf8( UINT8 a, UINT8 b );
	void ldcf16( UINT8 a, UINT8 b );
	void andcf8( UINT8 a, UINT8 b );
	void andcf16( UINT8 a, UINT8 b );
	void orcf8( UINT8 a, UINT8 b );
	void orcf16( UINT8 a, UINT8 b );
	void xorcf8( UINT8 a, UINT8 b );
	void xorcf16( UINT8 a, UINT8 b );
	UINT8 rl8( UINT8 a, UINT8 s );
	UINT16 rl16( UINT16 a, UINT8 s );
	UINT32 rl32( UINT32 a, UINT8 s );
	UINT8 rlc8( UINT8 a, UINT8 s );
	UINT16 rlc16( UINT16 a, UINT8 s );
	UINT32 rlc32( UINT32 a, UINT8 s );
	UINT8 rr8( UINT8 a, UINT8 s );
	UINT16 rr16( UINT16 a, UINT8 s );
	UINT32 rr32( UINT32 a, UINT8 s );
	UINT8 rrc8( UINT8 a, UINT8 s );
	UINT16 rrc16( UINT16 a, UINT8 s );
	UINT32 rrc32( UINT32 a, UINT8 s );
	UINT8 sla8( UINT8 a, UINT8 s );
	UINT16 sla16( UINT16 a, UINT8 s );
	UINT32 sla32( UINT32 a, UINT8 s );
	UINT8 sra8( UINT8 a, UINT8 s );
	UINT16 sra16( UINT16 a, UINT8 s );
	UINT32 sra32( UINT32 a, UINT8 s );
	UINT8 srl8( UINT8 a, UINT8 s );
	UINT16 srl16( UINT16 a, UINT8 s );
	UINT32 srl32( UINT32 a, UINT8 s );
	UINT16 div8( UINT16 a, UINT8 b );
	UINT32 div16( UINT32 a, UINT16 b );
	UINT16 divs8( INT16 a, INT8 b );
	UINT32 divs16( INT32 a, INT16 b );
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


class tmp95c063_device : public tlcs900h_device
{
public:
	// construction/destruction
	tmp95c063_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_reset();
	virtual void execute_set_input(int inputnum, int state);
	virtual void tlcs900_check_hdma();
	virtual void tlcs900_check_irqs();
	virtual void tlcs900_handle_ad();
	virtual void tlcs900_handle_timers();
};

#endif
