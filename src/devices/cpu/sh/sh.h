

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
	void ADD(uint32_t m, uint32_t n);

	void ADD(const uint16_t opcode);
	void ADDI(const uint16_t opcode);
	void ADDC(const uint16_t opcode);
	void ADDV(const uint16_t opcode);
	void AND(const uint16_t opcode);
	void ANDI(const uint16_t opcode);
	void ANDM(const uint16_t opcode);
	void BF(const uint16_t opcode);
	void BFS(const uint16_t opcode);
	void BRA(const uint16_t opcode);
	void BRAF(const uint16_t opcode);
	void BSR(const uint16_t opcode);
	void BSRF(const uint16_t opcode);
	void BT(const uint16_t opcode);
	void BTS(const uint16_t opcode);
	void CLRMAC(const uint16_t opcode);
	void CLRT(const uint16_t opcode);
	void CMPEQ(const uint16_t opcode);
	void CMPGE(const uint16_t opcode);
	void CMPGT(const uint16_t opcode);
	void CMPHI(const uint16_t opcode);
	void CMPHS(const uint16_t opcode);
	void CMPPL(const uint16_t opcode);
	void CMPPZ(const uint16_t opcode);
	void CMPSTR(const uint16_t opcode);
	void CMPIM(const uint16_t opcode);
	void DIV0S(const uint16_t opcode);
	void DIV0U(const uint16_t opcode);
	void DIV1(const uint16_t opcode);
	void DMULS(const uint16_t opcode);
	void DMULU(const uint16_t opcode);
	void DT(const uint16_t opcode);
	void EXTSB(const uint16_t opcode);
	void EXTSW(const uint16_t opcode);
	void EXTUB(const uint16_t opcode);
	void EXTUW(const uint16_t opcode);
	void JMP(const uint16_t opcode);
	void JSR(const uint16_t opcode);
	void LDCGBR(const uint16_t opcode);
	void LDCVBR(const uint16_t opcode);
	void LDCMGBR(const uint16_t opcode);
	void LDCMVBR(const uint16_t opcode);
	void LDSMACH(const uint16_t opcode);
	void LDSMACL(const uint16_t opcode);
	void LDSPR(const uint16_t opcode);
	void LDSMMACH(const uint16_t opcode);
	void LDSMMACL(const uint16_t opcode);
	void LDSMPR(const uint16_t opcode);
	void MAC_L(const uint16_t opcode);
	void MAC_W(const uint16_t opcode);
	void MOV(const uint16_t opcode);
	void MOVBS(const uint16_t opcode);
	void MOVWS(const uint16_t opcode);
	void MOVLS(const uint16_t opcode);
	void MOVBL(const uint16_t opcode);
	void MOVWL(const uint16_t opcode);
	void MOVLL(const uint16_t opcode);
	void MOVBM(const uint16_t opcode);
	void MOVWM(const uint16_t opcode);
	void MOVLM(const uint16_t opcode);
	void MOVBP(const uint16_t opcode);
	void MOVWP(const uint16_t opcode);
	void MOVLP(const uint16_t opcode);
	void MOVBS0(const uint16_t opcode);
	void MOVWS0(const uint16_t opcode);
	void MOVLS0(const uint16_t opcode);
	void MOVBL0(const uint16_t opcode);
	void MOVWL0(const uint16_t opcode);
	void MOVLL0(const uint16_t opcode);
	void MOVI(const uint16_t opcode);
	void MOVWI(const uint16_t opcode);
	void MOVLI(const uint16_t opcode);
	void MOVBLG(const uint16_t opcode);
	void MOVWLG(const uint16_t opcode);
	void MOVLLG(const uint16_t opcode);
	void MOVBSG(const uint16_t opcode);
	void MOVWSG(const uint16_t opcode);
	void MOVLSG(const uint16_t opcode);
	void MOVBS4(const uint16_t opcode);
	void MOVWS4(const uint16_t opcode);
	void MOVLS4(const uint16_t opcode);
	void MOVBL4(const uint16_t opcode);
	void MOVWL4(const uint16_t opcode);
	void MOVLL4(const uint16_t opcode);
	void MOVA(const uint16_t opcode);
	void MOVT(const uint16_t opcode);
	void MULL(const uint16_t opcode);
	void MULS(const uint16_t opcode);
	void MULU(const uint16_t opcode);
	void NEG(const uint16_t opcode);
	void NEGC(const uint16_t opcode);
	void NOP(const uint16_t opcode);
	void NOT(const uint16_t opcode);
	void OR(const uint16_t opcode);
	void ORI(const uint16_t opcode);
	void ORM(const uint16_t opcode);
	void ROTCL(const uint16_t opcode);
	void ROTCR(const uint16_t opcode);
	void ROTL(const uint16_t opcode);
	void ROTR(const uint16_t opcode);
	void RTS(const uint16_t opcode);
	void SETT(const uint16_t opcode);
	void SHAL(const uint16_t opcode);
	void SHAR(const uint16_t opcode);
	void SHLL(const uint16_t opcode);
	void SHLL2(const uint16_t opcode);
	void SHLL8(const uint16_t opcode);
	void SHLL16(const uint16_t opcode);
	void SHLR(const uint16_t opcode);
	void SHLR2(const uint16_t opcode);
	void SHLR8(const uint16_t opcode);
	void SHLR16(const uint16_t opcode);
	void STCSR(const uint16_t opcode);
	void STCGBR(const uint16_t opcode);
	void STCVBR(const uint16_t opcode);
	void STCMSR(const uint16_t opcode);
	void STCMGBR(const uint16_t opcode);
	void STCMVBR(const uint16_t opcode);
	void STSMACH(const uint16_t opcode);
	void STSMACL(const uint16_t opcode);
	void STSPR(const uint16_t opcode);
	void STSMMACH(const uint16_t opcode);
	void STSMMACL(const uint16_t opcode);
	void STSMPR(const uint16_t opcode);
	void SUB(const uint16_t opcode);
	void SUBC(const uint16_t opcode);
	void SUBV(const uint16_t opcode);
	void SWAPB(const uint16_t opcode);
	void SWAPW(const uint16_t opcode);
	void TAS(const uint16_t opcode);
	void TST(const uint16_t opcode);
	void TSTI(const uint16_t opcode);
	void TSTM(const uint16_t opcode);
	void XOR(const uint16_t opcode);
	void XORI(const uint16_t opcode);
	void XORM(const uint16_t opcode);
	void XTRCT(const uint16_t opcode);

	void MOVCAL(const uint16_t opcode);
	void CLRS(const uint16_t opcode);
	void SETS(const uint16_t opcode);


	drc_cache           m_cache;                  /* pointer to the DRC code cache */
};