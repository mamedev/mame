

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


	virtual void RTE() = 0;
	virtual void LDCSR(const uint16_t opcode) = 0;
	virtual void LDCMSR(const uint16_t opcode) = 0;
	virtual void TRAPA(uint32_t i) = 0;
	virtual	void ILLEGAL() = 0;


	drc_cache           m_cache;                  /* pointer to the DRC code cache */
};