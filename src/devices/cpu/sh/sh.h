

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"

/* size of the execution code cache */
#define CACHE_SIZE                  (32 * 1024 * 1024)

#define SH2_MAX_FASTRAM       4


/***************************************************************************
    DEBUGGING
**************************************************************************/

#define DISABLE_FAST_REGISTERS              (0) // set to 1 to turn off usage of register caching
#define SINGLE_INSTRUCTION_MODE             (0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* compilation boundaries -- how far back/forward does the analysis extend? */
#define COMPILE_BACKWARDS_BYTES         64
#define COMPILE_FORWARDS_BYTES          256
#define COMPILE_MAX_INSTRUCTIONS        ((COMPILE_BACKWARDS_BYTES/2) + (COMPILE_FORWARDS_BYTES/2))
#define COMPILE_MAX_SEQUENCE            64


enum
{
	SH4_PC = 1, SH_SR, SH4_PR, SH4_GBR, SH4_VBR, SH4_DBR, SH4_MACH, SH4_MACL,
	SH4_R0, SH4_R1, SH4_R2, SH4_R3, SH4_R4, SH4_R5, SH4_R6, SH4_R7,
	SH4_R8, SH4_R9, SH4_R10, SH4_R11, SH4_R12, SH4_R13, SH4_R14, SH4_R15, SH4_EA
};

class sh_common_execution : public cpu_device
{

public:
	sh_common_execution(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endianness, address_map_constructor internal)
		: cpu_device(mconfig, type, tag, owner, clock)
		, m_sh2_state(nullptr)
		, m_cache(CACHE_SIZE + sizeof(internal_sh2_state))
		, m_drcuml(nullptr)
		//, m_drcuml(*this, m_cache, 0, 1, 32, 1)
		, m_drcoptions(0)
		, m_entry(nullptr)
		, m_read8(nullptr)
		, m_write8(nullptr)
		, m_read16(nullptr)
		, m_write16(nullptr)
		, m_read32(nullptr)
		, m_write32(nullptr)
		, m_interrupt(nullptr)
		, m_nocode(nullptr)
		, m_out_of_cycles(nullptr)
	{ }

	// Data that needs to be stored close to the generated DRC code
	struct internal_sh2_state
	{
		uint32_t  pc;
		uint32_t  pr;
		uint32_t  sr;
		uint32_t  mach;
		uint32_t  macl;
		uint32_t  r[16];
		uint32_t  ea;

		uint32_t  pending_irq;
		uint32_t  pending_nmi;
		int32_t   irqline;
		uint32_t  evec;               // exception vector for DRC
		uint32_t  irqsr;              // IRQ-time old SR for DRC
		uint32_t  target;             // target for jmp/jsr/etc so the delay slot can't kill it
		int     internal_irq_level;
		int     icount;
		uint8_t   sleep_mode;
		uint32_t  arg0;              /* print_debug argument 1 */
		uint32_t  gbr;
		uint32_t  vbr;

		uint32_t  m_delay;
	};

	internal_sh2_state *m_sh2_state;

	virtual uint8_t RB(offs_t A) = 0;
	virtual uint16_t RW(offs_t A) = 0;
	virtual uint32_t RL(offs_t A) = 0;
	virtual void WB(offs_t A, uint8_t V) = 0;
	virtual void WW(offs_t A, uint16_t V) = 0;
	virtual void WL(offs_t A, uint32_t V) = 0;

protected:

	void ADD(uint32_t m, uint32_t n);
	void ADDI(uint32_t i, uint32_t n);
	void ADDC(uint32_t m, uint32_t n);
	void ADDV(uint32_t m, uint32_t n);
	void AND(uint32_t m, uint32_t n);
	void ANDI(uint32_t i);
	void ANDM(uint32_t i);
	void BF(uint32_t d);
	void BFS(uint32_t d);
	void BRA(uint32_t d);
	void BRAF(uint32_t m);
	void BSR(uint32_t d);
	void BSRF(uint32_t m);
	void BT(uint32_t d);
	void BTS(uint32_t d);
	void CLRMAC();
	void CLRT();
	void CMPEQ(uint32_t m, uint32_t n);
	void CMPGE(uint32_t m, uint32_t n);
	void CMPGT(uint32_t m, uint32_t n);
	void CMPHI(uint32_t m, uint32_t n);
	void CMPHS(uint32_t m, uint32_t n);
	void CMPPL(uint32_t n);
	void CMPPZ(uint32_t n);
	void CMPSTR(uint32_t m, uint32_t n);
	void CMPIM(uint32_t i);
	void DIV0S(uint32_t m, uint32_t n);
	void DIV0U();
	void DIV1(uint32_t m, uint32_t n);
	void DMULS(uint32_t m, uint32_t n);
	void DMULU(uint32_t m, uint32_t n);
	void DT(uint32_t n);
	void EXTSB(uint32_t m, uint32_t n);
	void EXTSW(uint32_t m, uint32_t n);
	void EXTUB(uint32_t m, uint32_t n);
	void EXTUW(uint32_t m, uint32_t n);
	void JMP(uint32_t m);
	void JSR(uint32_t m);
	void LDCGBR(uint32_t m);
	void LDCVBR(uint32_t m);
	void LDCMGBR(uint32_t m);
	void LDCMVBR(uint32_t m);
	void LDSMACH(uint32_t m);
	void LDSMACL(uint32_t m);
	void LDSPR(uint32_t m);
	void LDSMMACH(uint32_t m);
	void LDSMMACL(uint32_t m);
	void LDSMPR(uint32_t m);
	void MAC_L(uint32_t m, uint32_t n);
	void MAC_W(uint32_t m, uint32_t n);
	void MOV(uint32_t m, uint32_t n);
	void MOVBS(uint32_t m, uint32_t n);
	void MOVWS(uint32_t m, uint32_t n);
	void MOVLS(uint32_t m, uint32_t n);
	void MOVBL(uint32_t m, uint32_t n);
	void MOVWL(uint32_t m, uint32_t n);
	void MOVLL(uint32_t m, uint32_t n);
	void MOVBM(uint32_t m, uint32_t n);
	void MOVWM(uint32_t m, uint32_t n);
	void MOVLM(uint32_t m, uint32_t n);
	void MOVBP(uint32_t m, uint32_t n);
	void MOVWP(uint32_t m, uint32_t n);
	void MOVLP(uint32_t m, uint32_t n);
	void MOVBS0(uint32_t m, uint32_t n);
	void MOVWS0(uint32_t m, uint32_t n);
	void MOVLS0(uint32_t m, uint32_t n);
	void MOVBL0(uint32_t m, uint32_t n);
	void MOVWL0(uint32_t m, uint32_t n);
	void MOVLL0(uint32_t m, uint32_t n);
	void MOVI(uint32_t i, uint32_t n);
	void MOVWI(uint32_t d, uint32_t n);
	void MOVLI(uint32_t d, uint32_t n);
	void MOVBLG(uint32_t d);
	void MOVWLG(uint32_t d);
	void MOVLLG(uint32_t d);
	void MOVBSG(uint32_t d);
	void MOVWSG(uint32_t d);
	void MOVLSG(uint32_t d);
	void MOVBS4(uint32_t d, uint32_t n);
	void MOVWS4(uint32_t d, uint32_t n);
	void MOVLS4(uint32_t m, uint32_t d, uint32_t n);
	void MOVBL4(uint32_t m, uint32_t d);
	void MOVWL4(uint32_t m, uint32_t d);
	void MOVLL4(uint32_t m, uint32_t d, uint32_t n);
	void MOVA(uint32_t d);
	void MOVT(uint32_t n);
	void MULL(uint32_t m, uint32_t n);
	void MULS(uint32_t m, uint32_t n);
	void MULU(uint32_t m, uint32_t n);
	void NEG(uint32_t m, uint32_t n);
	void NEGC(uint32_t m, uint32_t n);
	void NOP(void);
	void NOT(uint32_t m, uint32_t n);
	void OR(uint32_t m, uint32_t n);
	void ORI(uint32_t i);
	void ORM(uint32_t i);
	void ROTCL(uint32_t n);
	void ROTCR(uint32_t n);
	void ROTL(uint32_t n);
	void ROTR(uint32_t n);
	void RTS();
	void SETT();
	void SHAL(uint32_t n);
	void SHAR(uint32_t n);
	void SHLL(uint32_t n);
	void SHLL2(uint32_t n);
	void SHLL8(uint32_t n);
	void SHLL16(uint32_t n);
	void SHLR(uint32_t n);
	void SHLR2(uint32_t n);
	void SHLR8(uint32_t n);
	void SHLR16(uint32_t n);
	void SLEEP();
	void STCSR(uint32_t n);
	void STCGBR(uint32_t n);
	void STCVBR(uint32_t n);
	void STCMSR(uint32_t n);
	void STCMGBR(uint32_t n);
	void STCMVBR(uint32_t n);
	void STSMACH(uint32_t n);
	void STSMACL(uint32_t n);
	void STSPR(uint32_t n);
	void STSMMACH(uint32_t n);
	void STSMMACL(uint32_t n);
	void STSMPR(uint32_t n);
	void SUB(uint32_t m, uint32_t n);
	void SUBC(uint32_t m, uint32_t n);
	void SUBV(uint32_t m, uint32_t n);
	void SWAPB(uint32_t m, uint32_t n);
	void SWAPW(uint32_t m, uint32_t n);
	void TAS(uint32_t n);
	void TST(uint32_t m, uint32_t n);
	void TSTI(uint32_t i);
	void TSTM(uint32_t i);
	void XOR(uint32_t m, uint32_t n);
	void XORI(uint32_t i);
	void XORM(uint32_t i);
	void XTRCT(uint32_t m, uint32_t n);

	void op0010(uint16_t opcode);
	void op0011(uint16_t opcode);
	void op0110(uint16_t opcode);
	void op1000(uint16_t opcode);
	void op1100(uint16_t opcode);

	void execute_one(const uint16_t opcode);

	virtual void execute_one_0000(uint16_t opcode) = 0;
	virtual void execute_one_4000(uint16_t opcode) = 0;
	virtual void execute_one_f000(uint16_t opcode) = 0;

	virtual void RTE() = 0;
	virtual void LDCSR(const uint16_t opcode) = 0;
	virtual void LDCMSR(const uint16_t opcode) = 0;
	virtual void TRAPA(uint32_t i) = 0;
	virtual	void ILLEGAL() = 0;

	drc_cache           m_cache;                  /* pointer to the DRC code cache */

public:
	/* fast RAM */
	uint32_t              m_fastram_select;
	struct
	{
		offs_t              start;                      /* start of the RAM block */
		offs_t              end;                        /* end of the RAM block */
		bool                readonly;                   /* true if read-only */
		void *              base;                       /* base in memory where the RAM lives */
	} m_fastram[SH2_MAX_FASTRAM];

	int m_pcfsel;                 // last pcflush entry set
	//int m_maxpcfsel;              // highest valid pcflush entry
	uint32_t m_pcflushes[16];           // pcflush entries

	virtual void init_drc_frontend()
	{
		fatalerror("init_drc_frontend base");
	}

	void drc_start();


	void sh2drc_add_fastram(offs_t start, offs_t end, uint8_t readonly, void *base);

	direct_read_data *m_direct;

	std::unique_ptr<drcuml_state>      m_drcuml;                 /* DRC UML generator state */
	uint32_t              m_drcoptions;         /* configurable DRC options */

	/* internal stuff */
	uint8_t               m_cache_dirty;                /* true if we need to flush the cache */

	/* parameters for subroutines */
	//uint64_t              m_numcycles;              /* return value from gettotalcycles */
	//uint32_t              m_arg1;                   /* print_debug argument 2 */
	//uint32_t              m_irq;                /* irq we're taking */

	/* register mappings */
	uml::parameter      m_regmap[16];                 /* parameter to register mappings for all 16 integer registers */

	uml::code_handle *  m_entry;                      /* entry point */
	uml::code_handle *  m_read8;                  /* read byte */
	uml::code_handle *  m_write8;                 /* write byte */
	uml::code_handle *  m_read16;                 /* read half */
	uml::code_handle *  m_write16;                    /* write half */
	uml::code_handle *  m_read32;                 /* read word */
	uml::code_handle *  m_write32;                    /* write word */

	uml::code_handle *  m_interrupt;              /* interrupt */
	uml::code_handle *  m_nocode;                 /* nocode */
	uml::code_handle *  m_out_of_cycles;              /* out of cycles exception handler */


protected:
	// device-level overrides
	virtual void device_start() override;
};

class sh_frontend : public drc_frontend
{
public:
	sh_frontend(sh_common_execution *device, uint32_t window_start, uint32_t window_end, uint32_t max_sequence);

protected:
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev) override;

private:
	bool describe_group_2(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_3(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_6(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_8(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	bool describe_group_12(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode);
	
	virtual bool describe_group_0(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) = 0;
	virtual bool describe_group_4(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) = 0;
	virtual bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, uint16_t opcode) = 0;

	sh_common_execution *m_sh;
};
