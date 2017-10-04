

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"

/* size of the execution code cache */
#define CACHE_SIZE                  (32 * 1024 * 1024)


class sh_common_execution
{
public:
	sh_common_execution() :
		m_sh2_state(nullptr)
		, m_cache(CACHE_SIZE + sizeof(internal_sh2_state))
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
		// SH1/2 only?
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

	void SH2ADD(uint32_t m, uint32_t n);
	void SH2ADDI(uint32_t i, uint32_t n);
	void SH2ADDC(uint32_t m, uint32_t n);
	void SH2ADDV(uint32_t m, uint32_t n);
	void SH2AND(uint32_t m, uint32_t n);
	void SH2ANDI(uint32_t i);
	void SH2ANDM(uint32_t i);
	void SH2BF(uint32_t d);
	void SH2BFS(uint32_t d);
	void SH2BRA(uint32_t d);
	void SH2BRAF(uint32_t m);
	void SH2BSR(uint32_t d);
	void SH2BSRF(uint32_t m);
	void SH2BT(uint32_t d);
	void SH2BTS(uint32_t d);
	void SH2CLRMAC();
	void SH2CLRT();
	void SH2CMPEQ(uint32_t m, uint32_t n);
	void SH2CMPGE(uint32_t m, uint32_t n);
	void SH2CMPGT(uint32_t m, uint32_t n);
	void SH2CMPHI(uint32_t m, uint32_t n);
	void SH2CMPHS(uint32_t m, uint32_t n);
	void SH2CMPPL(uint32_t n);
	void SH2CMPPZ(uint32_t n);
	void SH2CMPSTR(uint32_t m, uint32_t n);
	void SH2CMPIM(uint32_t i);
	void SH2DIV0S(uint32_t m, uint32_t n);
	void SH2DIV0U();
	void SH2DIV1(uint32_t m, uint32_t n);
	void SH2DMULS(uint32_t m, uint32_t n);
	void SH2DMULU(uint32_t m, uint32_t n);
	void SH2DT(uint32_t n);
	void SH2EXTSB(uint32_t m, uint32_t n);
	void SH2EXTSW(uint32_t m, uint32_t n);
	void SH2EXTUB(uint32_t m, uint32_t n);
	void SH2EXTUW(uint32_t m, uint32_t n);
	void SH2ILLEGAL();
	void SH2JMP(uint32_t m);
	void SH2JSR(uint32_t m);
	void SH2LDCGBR(uint32_t m);
	void SH2LDCVBR(uint32_t m);
	void SH2LDCMGBR(uint32_t m);
	void SH2LDCMVBR(uint32_t m);
	void SH2LDSMACH(uint32_t m);
	void SH2LDSMACL(uint32_t m);
	void SH2LDSPR(uint32_t m);
	void SH2LDSMMACH(uint32_t m);
	void SH2LDSMMACL(uint32_t m);
	void SH2LDSMPR(uint32_t m);
	void SH2MAC_L(uint32_t m, uint32_t n);
	void SH2MAC_W(uint32_t m, uint32_t n);
	void SH2MOV(uint32_t m, uint32_t n);
	void SH2MOVBS(uint32_t m, uint32_t n);
	void SH2MOVWS(uint32_t m, uint32_t n);
	void SH2MOVLS(uint32_t m, uint32_t n);
	void SH2MOVBL(uint32_t m, uint32_t n);
	void SH2MOVWL(uint32_t m, uint32_t n);
	void SH2MOVLL(uint32_t m, uint32_t n);
	void SH2MOVBM(uint32_t m, uint32_t n);
	void SH2MOVWM(uint32_t m, uint32_t n);
	void SH2MOVLM(uint32_t m, uint32_t n);
	void SH2MOVBP(uint32_t m, uint32_t n);
	void SH2MOVWP(uint32_t m, uint32_t n);
	void SH2MOVLP(uint32_t m, uint32_t n);
	void SH2MOVBS0(uint32_t m, uint32_t n);
	void SH2MOVWS0(uint32_t m, uint32_t n);
	void SH2MOVLS0(uint32_t m, uint32_t n);
	void SH2MOVBL0(uint32_t m, uint32_t n);
	void SH2MOVWL0(uint32_t m, uint32_t n);
	void SH2MOVLL0(uint32_t m, uint32_t n);
	void SH2MOVI(uint32_t i, uint32_t n);
	void SH2MOVWI(uint32_t d, uint32_t n);
	void SH2MOVLI(uint32_t d, uint32_t n);
	void SH2MOVBLG(uint32_t d);
	void SH2MOVWLG(uint32_t d);
	void SH2MOVLLG(uint32_t d);
	void SH2MOVBSG(uint32_t d);
	void SH2MOVWSG(uint32_t d);
	void SH2MOVLSG(uint32_t d);
	void SH2MOVBS4(uint32_t d, uint32_t n);
	void SH2MOVWS4(uint32_t d, uint32_t n);
	void SH2MOVLS4(uint32_t m, uint32_t d, uint32_t n);
	void SH2MOVBL4(uint32_t m, uint32_t d);
	void SH2MOVWL4(uint32_t m, uint32_t d);
	void SH2MOVLL4(uint32_t m, uint32_t d, uint32_t n);
	void SH2MOVA(uint32_t d);
	void SH2MOVT(uint32_t n);
	void SH2MULL(uint32_t m, uint32_t n);
	void SH2MULS(uint32_t m, uint32_t n);
	void SH2MULU(uint32_t m, uint32_t n);
	void SH2NEG(uint32_t m, uint32_t n);
	void SH2NEGC(uint32_t m, uint32_t n);
	void SH2NOP(void);
	void SH2NOT(uint32_t m, uint32_t n);
	void SH2OR(uint32_t m, uint32_t n);
	void SH2ORI(uint32_t i);
	void SH2ORM(uint32_t i);
	void SH2ROTCL(uint32_t n);
	void SH2ROTCR(uint32_t n);
	void SH2ROTL(uint32_t n);
	void SH2ROTR(uint32_t n);
	void SH2RTS();
	void SH2SETT();
	void SH2SHAL(uint32_t n);
	void SH2SHAR(uint32_t n);
	void SH2SHLL(uint32_t n);
	void SH2SHLL2(uint32_t n);
	void SH2SHLL8(uint32_t n);
	void SH2SHLL16(uint32_t n);
	void SH2SHLR(uint32_t n);
	void SH2SHLR2(uint32_t n);
	void SH2SHLR8(uint32_t n);
	void SH2SHLR16(uint32_t n);
	void SH2SLEEP();
	void SH2STCSR(uint32_t n);
	void SH2STCGBR(uint32_t n);
	void SH2STCVBR(uint32_t n);
	void SH2STCMSR(uint32_t n);
	void SH2STCMGBR(uint32_t n);
	void SH2STCMVBR(uint32_t n);
	void SH2STSMACH(uint32_t n);
	void SH2STSMACL(uint32_t n);
	void SH2STSPR(uint32_t n);
	void SH2STSMMACH(uint32_t n);
	void SH2STSMMACL(uint32_t n);
	void SH2STSMPR(uint32_t n);
	void SH2SUB(uint32_t m, uint32_t n);
	void SH2SUBC(uint32_t m, uint32_t n);
	void SH2SUBV(uint32_t m, uint32_t n);
	void SH2SWAPB(uint32_t m, uint32_t n);
	void SH2SWAPW(uint32_t m, uint32_t n);
	void SH2TAS(uint32_t n);
	void SH2TRAPA(uint32_t i);
	void SH2TST(uint32_t m, uint32_t n);
	void SH2TSTI(uint32_t i);
	void SH2TSTM(uint32_t i);
	void SH2XOR(uint32_t m, uint32_t n);
	void SH2XORI(uint32_t i);
	void SH2XORM(uint32_t i);
	void SH2XTRCT(uint32_t m, uint32_t n);

	virtual void RTE() = 0;

	drc_cache           m_cache;                  /* pointer to the DRC code cache */
};