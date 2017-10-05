
#include "sh.h"

#define Rn  ((opcode>>8)&15)
#define Rm  ((opcode>>4)&15)

/* Bits in SR */
#define T   0x00000001
#define S   0x00000002
#define I   0x000000f0
#define Q   0x00000100
#define M   0x00000200


#define BUSY_LOOP_HACKS 0 



void sh_common_execution::device_start()
{
	/* allocate the implementation-specific state from the full cache */
	m_sh2_state = (internal_sh2_state *)m_cache.alloc_near(sizeof(internal_sh2_state));

	save_item(NAME(m_sh2_state->pc));
	save_item(NAME(m_sh2_state->sr));
	save_item(NAME(m_sh2_state->pr));
	save_item(NAME(m_sh2_state->gbr));
	save_item(NAME(m_sh2_state->vbr));
	save_item(NAME(m_sh2_state->mach));
	save_item(NAME(m_sh2_state->macl));
	save_item(NAME(m_sh2_state->r));
	save_item(NAME(m_sh2_state->ea));
	save_item(NAME(m_sh2_state->m_delay));
	save_item(NAME(m_sh2_state->pending_irq));
	save_item(NAME(m_sh2_state->pending_nmi));
	save_item(NAME(m_sh2_state->irqline));
	save_item(NAME(m_sh2_state->evec));
	save_item(NAME(m_sh2_state->irqsr));
	save_item(NAME(m_sh2_state->target));
	save_item(NAME(m_sh2_state->internal_irq_level));
	save_item(NAME(m_sh2_state->sleep_mode));
	save_item(NAME(m_sh2_state->icount));

	m_sh2_state->pc = 0;
	m_sh2_state->pr = 0;
	m_sh2_state->sr = 0;
	m_sh2_state->gbr = 0;
	m_sh2_state->vbr = 0;
	m_sh2_state->mach = 0;
	m_sh2_state->macl = 0;
	memset(m_sh2_state->r, 0, sizeof(m_sh2_state->r));
	m_sh2_state->ea = 0;
	m_sh2_state->m_delay = 0;
	m_sh2_state->pending_irq = 0;
	m_sh2_state->pending_nmi = 0;
	m_sh2_state->irqline = 0;
	m_sh2_state->evec = 0;
	m_sh2_state->irqsr = 0;
	m_sh2_state->target = 0;
	m_sh2_state->internal_irq_level = 0;
	m_sh2_state->icount = 0;
	m_sh2_state->sleep_mode = 0;
	m_sh2_state->arg0 = 0;

	state_add(SH4_PC, "PC", m_sh2_state->pc).formatstr("%08X").callimport();
	state_add(SH_SR, "SR", m_sh2_state->sr).formatstr("%08X").callimport();
	state_add(SH4_PR, "PR", m_sh2_state->pr).formatstr("%08X");
	state_add(SH4_GBR, "GBR", m_sh2_state->gbr).formatstr("%08X");
	state_add(SH4_VBR, "VBR", m_sh2_state->vbr).formatstr("%08X");
	state_add(SH4_MACH, "MACH", m_sh2_state->mach).formatstr("%08X");
	state_add(SH4_MACL, "MACL", m_sh2_state->macl).formatstr("%08X");
	state_add(SH4_R0, "R0", m_sh2_state->r[0]).formatstr("%08X");
	state_add(SH4_R1, "R1", m_sh2_state->r[1]).formatstr("%08X");
	state_add(SH4_R2, "R2", m_sh2_state->r[2]).formatstr("%08X");
	state_add(SH4_R3, "R3", m_sh2_state->r[3]).formatstr("%08X");
	state_add(SH4_R4, "R4", m_sh2_state->r[4]).formatstr("%08X");
	state_add(SH4_R5, "R5", m_sh2_state->r[5]).formatstr("%08X");
	state_add(SH4_R6, "R6", m_sh2_state->r[6]).formatstr("%08X");
	state_add(SH4_R7, "R7", m_sh2_state->r[7]).formatstr("%08X");
	state_add(SH4_R8, "R8", m_sh2_state->r[8]).formatstr("%08X");
	state_add(SH4_R9, "R9", m_sh2_state->r[9]).formatstr("%08X");
	state_add(SH4_R10, "R10", m_sh2_state->r[10]).formatstr("%08X");
	state_add(SH4_R11, "R11", m_sh2_state->r[11]).formatstr("%08X");
	state_add(SH4_R12, "R12", m_sh2_state->r[12]).formatstr("%08X");
	state_add(SH4_R13, "R13", m_sh2_state->r[13]).formatstr("%08X");
	state_add(SH4_R14, "R14", m_sh2_state->r[14]).formatstr("%08X");
	state_add(SH4_R15, "R15", m_sh2_state->r[15]).formatstr("%08X");
	state_add(SH4_EA, "EA", m_sh2_state->ea).formatstr("%08X");

	state_add(STATE_GENSP, "GENSP", m_sh2_state->r[15]).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_sh2_state->sr).formatstr("%20s").noshow();

	m_icountptr = &m_sh2_state->icount;


}


void sh_common_execution::drc_start()
{
	//memset(m_irq_queue, 0, sizeof(m_irq_queue));
	//m_maxpcfsel = 0;
	memset(m_pcflushes, 0, sizeof(m_pcflushes));

	//m_numcycles = 0;
	//m_arg1 = 0;
	//m_irq = 0;
	m_fastram_select = 0;
	memset(m_fastram, 0, sizeof(m_fastram));

	/* reset per-driver pcflushes */
	m_pcfsel = 0;

	/* initialize the UML generator */
	uint32_t flags = 0;
	m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, flags, 1, 32, 1);

	/* add symbols for our stuff */
	m_drcuml->symbol_add(&m_sh2_state->pc, sizeof(m_sh2_state->pc), "pc");
	m_drcuml->symbol_add(&m_sh2_state->icount, sizeof(m_sh2_state->icount), "icount");
	for (int regnum = 0; regnum < 16; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		m_drcuml->symbol_add(&m_sh2_state->r[regnum], sizeof(m_sh2_state->r[regnum]), buf);
	}
	m_drcuml->symbol_add(&m_sh2_state->pr, sizeof(m_sh2_state->pr), "pr");
	m_drcuml->symbol_add(&m_sh2_state->sr, sizeof(m_sh2_state->sr), "sr");
	m_drcuml->symbol_add(&m_sh2_state->gbr, sizeof(m_sh2_state->gbr), "gbr");
	m_drcuml->symbol_add(&m_sh2_state->vbr, sizeof(m_sh2_state->vbr), "vbr");
	m_drcuml->symbol_add(&m_sh2_state->macl, sizeof(m_sh2_state->macl), "macl");
	m_drcuml->symbol_add(&m_sh2_state->mach, sizeof(m_sh2_state->macl), "mach");

	/* initialize the front-end helper */
	init_drc_frontend();

	/* compute the register parameters */
	for (int regnum = 0; regnum < 16; regnum++)
	{
		m_regmap[regnum] = uml::mem(&m_sh2_state->r[regnum]);
	}

	/* if we have registers to spare, assign r0, r1, r2 to leftovers */
	/* WARNING: do not use synthetic registers that are mapped here! */
	if (!DISABLE_FAST_REGISTERS)
	{
		drcbe_info beinfo;
		m_drcuml->get_backend_info(beinfo);
		if (beinfo.direct_iregs > 4)
		{
			m_regmap[0] = uml::I4;
		}
		if (beinfo.direct_iregs > 5)
		{
			m_regmap[1] = uml::I5;
		}
		if (beinfo.direct_iregs > 6)
		{
			m_regmap[2] = uml::I6;
		}
	}

	/* mark the cache dirty so it is updated on next execute */
	m_cache_dirty = true;


	save_item(NAME(m_pcfsel));
	//save_item(NAME(m_maxpcfsel));
	save_item(NAME(m_pcflushes));
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1100  1       -
 *  ADD     Rm,Rn
 */
void sh_common_execution::ADD(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] += m_sh2_state->r[m];
}

/*  code                 cycles  t-bit
 *  0111 nnnn iiii iiii  1       -
 *  ADD     #imm,Rn
 */
void sh_common_execution::ADDI(uint32_t i, uint32_t n)
{
	m_sh2_state->r[n] += (int32_t)(int16_t)(int8_t)i;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1110  1       carry
 *  ADDC    Rm,Rn
 */
void sh_common_execution::ADDC(uint32_t m, uint32_t n)
{
	uint32_t tmp0, tmp1;

	tmp1 = m_sh2_state->r[n] + m_sh2_state->r[m];
	tmp0 = m_sh2_state->r[n];
	m_sh2_state->r[n] = tmp1 + (m_sh2_state->sr & T);
	if (tmp0 > tmp1)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	if (tmp1 > m_sh2_state->r[n])
		m_sh2_state->sr |= T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1111  1       overflow
 *  ADDV    Rm,Rn
 */
void sh_common_execution::ADDV(uint32_t m, uint32_t n)
{
	int32_t dest, src, ans;

	if ((int32_t) m_sh2_state->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((int32_t) m_sh2_state->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	m_sh2_state->r[n] += m_sh2_state->r[m];
	if ((int32_t) m_sh2_state->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 0 || src == 2)
	{
		if (ans == 1)
			m_sh2_state->sr |= T;
		else
			m_sh2_state->sr &= ~T;
	}
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1001  1       -
 *  AND     Rm,Rn
 */
void sh_common_execution::AND(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] &= m_sh2_state->r[m];
}

/*  code                 cycles  t-bit
 *  1100 1001 iiii iiii  1       -
 *  AND     #imm,R0
 */
void sh_common_execution::ANDI(uint32_t i)
{
	m_sh2_state->r[0] &= i;
}

/*  code                 cycles  t-bit
 *  1100 1101 iiii iiii  1       -
 *  AND.B   #imm,@(R0,GBR)
 */
void sh_common_execution::ANDM(uint32_t i)
{
	uint32_t temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = i & RB( m_sh2_state->ea );
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 2;
}

/*  code                 cycles  t-bit
 *  1000 1011 dddd dddd  3/1     -
 *  BF      disp8
 */
void sh_common_execution::BF(uint32_t d)
{
	if ((m_sh2_state->sr & T) == 0)
	{
		int32_t disp = ((int32_t)d << 24) >> 24;
		m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
		m_sh2_state->icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1111 dddd dddd  3/1     -
 *  BFS     disp8
 */
void sh_common_execution::BFS(uint32_t d)
{
	if ((m_sh2_state->sr & T) == 0)
	{
		int32_t disp = ((int32_t)d << 24) >> 24;
		m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
		m_sh2_state->icount--;
	}
}

/*  code                 cycles  t-bit
 *  1010 dddd dddd dddd  2       -
 *  BRA     disp12
 */
void sh_common_execution::BRA(uint32_t d)
{
	int32_t disp = ((int32_t)d << 20) >> 20;

#if BUSY_LOOP_HACKS
	if (disp == -2)
	{
		uint32_t next_opcode = RW(m_sh2_state->pc & AM);
		/* BRA  $
		 * NOP
		 */
		if (next_opcode == 0x0009)
			m_sh2_state->icount %= 3;   /* cycles for BRA $ and NOP taken (3) */
	}
#endif
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0010 0011  2       -
 *  BRAF    Rm
 */
void sh_common_execution::BRAF(uint32_t m)
{
	m_sh2_state->m_delay = m_sh2_state->pc + m_sh2_state->r[m] + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  1011 dddd dddd dddd  2       -
 *  BSR     disp12
 */
void sh_common_execution::BSR(uint32_t d)
{
	int32_t disp = ((int32_t)d << 20) >> 20;

	m_sh2_state->pr = m_sh2_state->pc + 2;
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0000 0011  2       -
 *  BSRF    Rm
 */
void sh_common_execution::BSRF(uint32_t m)
{
	m_sh2_state->pr = m_sh2_state->pc + 2;
	m_sh2_state->m_delay = m_sh2_state->pc + m_sh2_state->r[m] + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  1000 1001 dddd dddd  3/1     -
 *  BT      disp8
 */
void sh_common_execution::BT(uint32_t d)
{
	if ((m_sh2_state->sr & T) != 0)
	{
		int32_t disp = ((int32_t)d << 24) >> 24;
		m_sh2_state->pc = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
		m_sh2_state->icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1101 dddd dddd  2/1     -
 *  BTS     disp8
 */
void sh_common_execution::BTS(uint32_t d)
{
	if ((m_sh2_state->sr & T) != 0)
	{
		int32_t disp = ((int32_t)d << 24) >> 24;
		m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
		m_sh2_state->icount--;
	}
}

/*  code                 cycles  t-bit
 *  0000 0000 0010 1000  1       -
 *  CLRMAC
 */
void sh_common_execution::CLRMAC()
{
	m_sh2_state->mach = 0;
	m_sh2_state->macl = 0;
}

/*  code                 cycles  t-bit
 *  0000 0000 0000 1000  1       -
 *  CLRT
 */
void sh_common_execution::CLRT()
{
	m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0000  1       comparison result
 *  CMP_EQ  Rm,Rn
 */
void sh_common_execution::CMPEQ(uint32_t m, uint32_t n)
{
	if (m_sh2_state->r[n] == m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0011  1       comparison result
 *  CMP_GE  Rm,Rn
 */
void sh_common_execution::CMPGE(uint32_t m, uint32_t n)
{
	if ((int32_t) m_sh2_state->r[n] >= (int32_t) m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0111  1       comparison result
 *  CMP_GT  Rm,Rn
 */
void sh_common_execution::CMPGT(uint32_t m, uint32_t n)
{
	if ((int32_t) m_sh2_state->r[n] > (int32_t) m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0110  1       comparison result
 *  CMP_HI  Rm,Rn
 */
void sh_common_execution::CMPHI(uint32_t m, uint32_t n)
{
	if ((uint32_t) m_sh2_state->r[n] > (uint32_t) m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0010  1       comparison result
 *  CMP_HS  Rm,Rn
 */
void sh_common_execution::CMPHS(uint32_t m, uint32_t n)
{
	if ((uint32_t) m_sh2_state->r[n] >= (uint32_t) m_sh2_state->r[m])
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0101  1       comparison result
 *  CMP_PL  Rn
 */
void sh_common_execution::CMPPL(uint32_t n)
{
	if ((int32_t) m_sh2_state->r[n] > 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0001  1       comparison result
 *  CMP_PZ  Rn
 */
void sh_common_execution::CMPPZ(uint32_t n)
{
	if ((int32_t) m_sh2_state->r[n] >= 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1100  1       comparison result
 * CMP_STR  Rm,Rn
 */
void sh_common_execution::CMPSTR(uint32_t m, uint32_t n)
{
	uint32_t temp;
	int32_t HH, HL, LH, LL;
	temp = m_sh2_state->r[n] ^ m_sh2_state->r[m];
	HH = (temp >> 24) & 0xff;
	HL = (temp >> 16) & 0xff;
	LH = (temp >> 8) & 0xff;
	LL = temp & 0xff;
	if (HH && HL && LH && LL)
		m_sh2_state->sr &= ~T;
	else
		m_sh2_state->sr |= T;
}

/*  code                 cycles  t-bit
 *  1000 1000 iiii iiii  1       comparison result
 *  CMP/EQ #imm,R0
 */
void sh_common_execution::CMPIM(uint32_t i)
{
	uint32_t imm = (uint32_t)(int32_t)(int16_t)(int8_t)i;

	if (m_sh2_state->r[0] == imm)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 0111  1       calculation result
 *  DIV0S   Rm,Rn
 */
void sh_common_execution::DIV0S(uint32_t m, uint32_t n)
{
	if ((m_sh2_state->r[n] & 0x80000000) == 0)
		m_sh2_state->sr &= ~Q;
	else
		m_sh2_state->sr |= Q;
	if ((m_sh2_state->r[m] & 0x80000000) == 0)
		m_sh2_state->sr &= ~M;
	else
		m_sh2_state->sr |= M;
	if ((m_sh2_state->r[m] ^ m_sh2_state->r[n]) & 0x80000000)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0000 0000 0001 1001  1       0
 *  DIV0U
 */
void sh_common_execution::DIV0U()
{
	m_sh2_state->sr &= ~(M | Q | T);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0100  1       calculation result
 *  DIV1 Rm,Rn
 */
void sh_common_execution::DIV1(uint32_t m, uint32_t n)
{
	uint32_t tmp0;
	uint32_t old_q;

	old_q = m_sh2_state->sr & Q;
	if (0x80000000 & m_sh2_state->r[n])
		m_sh2_state->sr |= Q;
	else
		m_sh2_state->sr &= ~Q;

	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->sr & T);

	if (!old_q)
	{
		if (!(m_sh2_state->sr & M))
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] -= m_sh2_state->r[m];
			if(!(m_sh2_state->sr & Q))
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr |= Q;
				else
					m_sh2_state->sr &= ~Q;
			else
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr &= ~Q;
				else
					m_sh2_state->sr |= Q;
		}
		else
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] += m_sh2_state->r[m];
			if(!(m_sh2_state->sr & Q))
			{
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr &= ~Q;
				else
					m_sh2_state->sr |= Q;
			}
			else
			{
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr |= Q;
				else
					m_sh2_state->sr &= ~Q;
			}
		}
	}
	else
	{
		if (!(m_sh2_state->sr & M))
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] += m_sh2_state->r[m];
			if(!(m_sh2_state->sr & Q))
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr |= Q;
				else
					m_sh2_state->sr &= ~Q;
			else
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr &= ~Q;
				else
					m_sh2_state->sr |= Q;
		}
		else
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] -= m_sh2_state->r[m];
			if(!(m_sh2_state->sr & Q))
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr &= ~Q;
				else
					m_sh2_state->sr |= Q;
			else
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr |= Q;
				else
					m_sh2_state->sr &= ~Q;
		}
	}

	tmp0 = (m_sh2_state->sr & (Q | M));
	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  DMULS.L Rm,Rn */
void sh_common_execution::DMULS(uint32_t m, uint32_t n)
{
	uint32_t RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	uint32_t temp0, temp1, temp2, temp3;
	int32_t tempm, tempn, fnLmL;

	tempn = (int32_t) m_sh2_state->r[n];
	tempm = (int32_t) m_sh2_state->r[m];
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	if ((int32_t) (m_sh2_state->r[n] ^ m_sh2_state->r[m]) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	temp1 = (uint32_t) tempn;
	temp2 = (uint32_t) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	m_sh2_state->mach = Res2;
	m_sh2_state->macl = Res0;
	m_sh2_state->icount--;
}

/*  DMULU.L Rm,Rn */
void sh_common_execution::DMULU(uint32_t m, uint32_t n)
{
	uint32_t RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	uint32_t temp0, temp1, temp2, temp3;

	RnL = m_sh2_state->r[n] & 0x0000ffff;
	RnH = (m_sh2_state->r[n] >> 16) & 0x0000ffff;
	RmL = m_sh2_state->r[m] & 0x0000ffff;
	RmH = (m_sh2_state->r[m] >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	m_sh2_state->mach = Res2;
	m_sh2_state->macl = Res0;
	m_sh2_state->icount--;
}

/*  DT      Rn */
void sh_common_execution::DT(uint32_t n)
{
	m_sh2_state->r[n]--;
	if (m_sh2_state->r[n] == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
#if BUSY_LOOP_HACKS
	{
		uint32_t next_opcode = RW(m_sh2_state->pc & AM);
		/* DT   Rn
		 * BF   $-2
		 */
		if (next_opcode == 0x8bfd)
		{
			while (m_sh2_state->r[n] > 1 && m_sh2_state->icount > 4)
			{
				m_sh2_state->r[n]--;
				m_sh2_state->icount -= 4;   /* cycles for DT (1) and BF taken (3) */
			}
		}
	}
#endif
}

/*  EXTS.B  Rm,Rn */
void sh_common_execution::EXTSB(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = ((int32_t)m_sh2_state->r[m] << 24) >> 24;
}

/*  EXTS.W  Rm,Rn */
void sh_common_execution::EXTSW(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = ((int32_t)m_sh2_state->r[m] << 16) >> 16;
}

/*  EXTU.B  Rm,Rn */
void sh_common_execution::EXTUB(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->r[m] & 0x000000ff;
}

/*  EXTU.W  Rm,Rn */
void sh_common_execution::EXTUW(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->r[m] & 0x0000ffff;
}

/*  JMP     @Rm */
void sh_common_execution::JMP(uint32_t m)
{
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->icount--; // not in SH4 implementation?
}

/*  JSR     @Rm */
void sh_common_execution::JSR(uint32_t m)
{
	m_sh2_state->pr = m_sh2_state->pc + 2;
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->icount--;
}

/*  LDC     Rm,GBR */
void sh_common_execution::LDCGBR(uint32_t m)
{
	m_sh2_state->gbr = m_sh2_state->r[m];
}

/*  LDC     Rm,VBR */
void sh_common_execution::LDCVBR(uint32_t m)
{
	m_sh2_state->vbr = m_sh2_state->r[m];
}

/*  LDC.L   @Rm+,GBR */
void sh_common_execution::LDCMGBR(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->gbr = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
	m_sh2_state->icount -= 2;
}

/*  LDC.L   @Rm+,VBR */
void sh_common_execution::LDCMVBR(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->vbr = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
	m_sh2_state->icount -= 2;
}

/*  LDS     Rm,MACH */
void sh_common_execution::LDSMACH(uint32_t m)
{
	m_sh2_state->mach = m_sh2_state->r[m];
}

/*  LDS     Rm,MACL */
void sh_common_execution::LDSMACL(uint32_t m)
{
	m_sh2_state->macl = m_sh2_state->r[m];
}

/*  LDS     Rm,PR */
void sh_common_execution::LDSPR(uint32_t m)
{
	m_sh2_state->pr = m_sh2_state->r[m];
}

/*  LDS.L   @Rm+,MACH */
void sh_common_execution::LDSMMACH(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->mach = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
}

/*  LDS.L   @Rm+,MACL */
void sh_common_execution::LDSMMACL(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->macl = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
}

/*  LDS.L   @Rm+,PR */
void sh_common_execution::LDSMPR(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->pr = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
}

/*  MAC.L   @Rm+,@Rn+ */
void sh_common_execution::MAC_L(uint32_t m, uint32_t n)
{
	uint32_t RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	uint32_t temp0, temp1, temp2, temp3;
	int32_t tempm, tempn, fnLmL;

	tempn = (int32_t) RL( m_sh2_state->r[n] );
	m_sh2_state->r[n] += 4;
	tempm = (int32_t) RL( m_sh2_state->r[m] );
	m_sh2_state->r[m] += 4;
	if ((int32_t) (tempn ^ tempm) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	temp1 = (uint32_t) tempn;
	temp2 = (uint32_t) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	if (m_sh2_state->sr & S)
	{
		Res0 = m_sh2_state->macl + Res0;
		if (m_sh2_state->macl > Res0)
			Res2++;
		Res2 += (m_sh2_state->mach & 0x0000ffff);
		if (((int32_t) Res2 < 0) && (Res2 < 0xffff8000))
		{
			Res2 = 0x00008000;
			Res0 = 0x00000000;
		}
		else if (((int32_t) Res2 > 0) && (Res2 > 0x00007fff))
		{
			Res2 = 0x00007fff;
			Res0 = 0xffffffff;
		}
		m_sh2_state->mach = Res2;
		m_sh2_state->macl = Res0;
	}
	else
	{
		Res0 = m_sh2_state->macl + Res0;
		if (m_sh2_state->macl > Res0)
			Res2++;
		Res2 += m_sh2_state->mach;
		m_sh2_state->mach = Res2;
		m_sh2_state->macl = Res0;
	}
	m_sh2_state->icount -= 2;
}

/*  MAC.W   @Rm+,@Rn+ */
void sh_common_execution::MAC_W(uint32_t m, uint32_t n)
{
	int32_t tempm, tempn, dest, src, ans;
	uint32_t templ;

	tempn = (int32_t) RW( m_sh2_state->r[n] );
	m_sh2_state->r[n] += 2;
	tempm = (int32_t) RW( m_sh2_state->r[m] );
	m_sh2_state->r[m] += 2;
	templ = m_sh2_state->macl;
	tempm = ((int32_t) (short) tempn * (int32_t) (short) tempm);
	if ((int32_t) m_sh2_state->macl >= 0)
		dest = 0;
	else
		dest = 1;
	if ((int32_t) tempm >= 0)
	{
		src = 0;
		tempn = 0;
	}
	else
	{
		src = 1;
		tempn = 0xffffffff;
	}
	src += dest;
	m_sh2_state->macl += tempm;
	if ((int32_t) m_sh2_state->macl >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (m_sh2_state->sr & S)
	{
		if (ans == 1)
			{
				if (src == 0)
					m_sh2_state->macl = 0x7fffffff;
				if (src == 2)
					m_sh2_state->macl = 0x80000000;
			}
	}
	else
	{
		m_sh2_state->mach += tempn;
		if (templ > m_sh2_state->macl)
			m_sh2_state->mach += 1;
	}
	m_sh2_state->icount -= 2;
}

/*  MOV     Rm,Rn */
void sh_common_execution::MOV(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->r[m];
}

/*  MOV.B   Rm,@Rn */
void sh_common_execution::MOVBS(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n];
	WB( m_sh2_state->ea, m_sh2_state->r[m] & 0x000000ff);
}

/*  MOV.W   Rm,@Rn */
void sh_common_execution::MOVWS(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n];
	WW( m_sh2_state->ea, m_sh2_state->r[m] & 0x0000ffff);
}

/*  MOV.L   Rm,@Rn */
void sh_common_execution::MOVLS(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->r[m] );
}

/*  MOV.B   @Rm,Rn */
void sh_common_execution::MOVBL(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->ea );
}

/*  MOV.W   @Rm,Rn */
void sh_common_execution::MOVWL(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @Rm,Rn */
void sh_common_execution::MOVLL(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOV.B   Rm,@-Rn */
void sh_common_execution::MOVBM(uint32_t m, uint32_t n)
{
	/* SMG : bug fix, was reading m_sh2_state->r[n] */
	uint32_t data = m_sh2_state->r[m] & 0x000000ff;

	m_sh2_state->r[n] -= 1;
	WB( m_sh2_state->r[n], data );
}

/*  MOV.W   Rm,@-Rn */
void sh_common_execution::MOVWM(uint32_t m, uint32_t n)
{
	uint32_t data = m_sh2_state->r[m] & 0x0000ffff;

	m_sh2_state->r[n] -= 2;
	WW( m_sh2_state->r[n], data );
}

/*  MOV.L   Rm,@-Rn */
void sh_common_execution::MOVLM(uint32_t m, uint32_t n)
{
	uint32_t data = m_sh2_state->r[m];

	m_sh2_state->r[n] -= 4;
	WL( m_sh2_state->r[n], data );
}

/*  MOV.B   @Rm+,Rn */
void sh_common_execution::MOVBP(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->r[m] );
	if (n != m)
		m_sh2_state->r[m] += 1;
}

/*  MOV.W   @Rm+,Rn */
void sh_common_execution::MOVWP(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->r[m] );
	if (n != m)
		m_sh2_state->r[m] += 2;
}

/*  MOV.L   @Rm+,Rn */
void sh_common_execution::MOVLP(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = RL( m_sh2_state->r[m] );
	if (n != m)
		m_sh2_state->r[m] += 4;
}

/*  MOV.B   Rm,@(R0,Rn) */
void sh_common_execution::MOVBS0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n] + m_sh2_state->r[0];
	WB( m_sh2_state->ea, m_sh2_state->r[m] & 0x000000ff );
}

/*  MOV.W   Rm,@(R0,Rn) */
void sh_common_execution::MOVWS0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n] + m_sh2_state->r[0];
	WW( m_sh2_state->ea, m_sh2_state->r[m] & 0x0000ffff );
}

/*  MOV.L   Rm,@(R0,Rn) */
void sh_common_execution::MOVLS0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n] + m_sh2_state->r[0];
	WL( m_sh2_state->ea, m_sh2_state->r[m] );
}

/*  MOV.B   @(R0,Rm),Rn */
void sh_common_execution::MOVBL0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m] + m_sh2_state->r[0];
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->ea );
}

/*  MOV.W   @(R0,Rm),Rn */
void sh_common_execution::MOVWL0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m] + m_sh2_state->r[0];
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @(R0,Rm),Rn */
void sh_common_execution::MOVLL0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m] + m_sh2_state->r[0];
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOV     #imm,Rn */
void sh_common_execution::MOVI(uint32_t i, uint32_t n)
{
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t)(int8_t) i;
}

/*  MOV.W   @(disp8,PC),Rn */
void sh_common_execution::MOVWI(uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @(disp8,PC),Rn */
void sh_common_execution::MOVLI(uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = ((m_sh2_state->pc + 2) & ~3) + disp * 4;
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOV.B   @(disp8,GBR),R0 */
void sh_common_execution::MOVBLG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp;
	m_sh2_state->r[0] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->ea );
}

/*  MOV.W   @(disp8,GBR),R0 */
void sh_common_execution::MOVWLG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 2;
	m_sh2_state->r[0] = (int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @(disp8,GBR),R0 */
void sh_common_execution::MOVLLG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 4;
	m_sh2_state->r[0] = RL( m_sh2_state->ea );
}

/*  MOV.B   R0,@(disp8,GBR) */
void sh_common_execution::MOVBSG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp;
	WB( m_sh2_state->ea, m_sh2_state->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp8,GBR) */
void sh_common_execution::MOVWSG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 2;
	WW( m_sh2_state->ea, m_sh2_state->r[0] & 0x0000ffff );
}

/*  MOV.L   R0,@(disp8,GBR) */
void sh_common_execution::MOVLSG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 4;
	WL( m_sh2_state->ea, m_sh2_state->r[0] );
}

/*  MOV.B   R0,@(disp4,Rn) */
void sh_common_execution::MOVBS4(uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[n] + disp;
	WB( m_sh2_state->ea, m_sh2_state->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp4,Rn) */
void sh_common_execution::MOVWS4(uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[n] + disp * 2;
	WW( m_sh2_state->ea, m_sh2_state->r[0] & 0x0000ffff );
}

/* MOV.L Rm,@(disp4,Rn) */
void sh_common_execution::MOVLS4(uint32_t m, uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[n] + disp * 4;
	WL( m_sh2_state->ea, m_sh2_state->r[m] );
}

/*  MOV.B   @(disp4,Rm),R0 */
void sh_common_execution::MOVBL4(uint32_t m, uint32_t d)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[m] + disp;
	m_sh2_state->r[0] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->ea );
}

/*  MOV.W   @(disp4,Rm),R0 */
void sh_common_execution::MOVWL4(uint32_t m, uint32_t d)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[m] + disp * 2;
	m_sh2_state->r[0] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @(disp4,Rm),Rn */
void sh_common_execution::MOVLL4(uint32_t m, uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[m] + disp * 4;
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOVA    @(disp8,PC),R0 */
void sh_common_execution::MOVA(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = ((m_sh2_state->pc + 2) & ~3) + disp * 4;
	m_sh2_state->r[0] = m_sh2_state->ea;
}

/*  MOVT    Rn */
void sh_common_execution::MOVT(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->sr & T;
}

/*  MUL.L   Rm,Rn */
void sh_common_execution::MULL(uint32_t m, uint32_t n)
{
	m_sh2_state->macl = m_sh2_state->r[n] * m_sh2_state->r[m];
	m_sh2_state->icount--;
}

/*  MULS    Rm,Rn */
void sh_common_execution::MULS(uint32_t m, uint32_t n)
{
	m_sh2_state->macl = (int16_t) m_sh2_state->r[n] * (int16_t) m_sh2_state->r[m];
}

/*  MULU    Rm,Rn */
void sh_common_execution::MULU(uint32_t m, uint32_t n)
{
	m_sh2_state->macl = (uint16_t) m_sh2_state->r[n] * (uint16_t) m_sh2_state->r[m];
}

/*  NEG     Rm,Rn */
void sh_common_execution::NEG(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = 0 - m_sh2_state->r[m];
}

/*  NEGC    Rm,Rn */
void sh_common_execution::NEGC(uint32_t m, uint32_t n)
{
	uint32_t temp;

	temp = m_sh2_state->r[m];
	m_sh2_state->r[n] = -temp - (m_sh2_state->sr & T);
	if (temp || (m_sh2_state->sr & T))
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  NOP */
void sh_common_execution::NOP(void)
{
}

/*  NOT     Rm,Rn */
void sh_common_execution::NOT(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = ~m_sh2_state->r[m];
}

/*  OR      Rm,Rn */
void sh_common_execution::OR(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] |= m_sh2_state->r[m];
}

/*  OR      #imm,R0 */
void sh_common_execution::ORI(uint32_t i)
{
	m_sh2_state->r[0] |= i;
}

/*  OR.B    #imm,@(R0,GBR) */
void sh_common_execution::ORM(uint32_t i)
{
	uint32_t temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = RB( m_sh2_state->ea );
	temp |= i;
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 2;
}

/*  ROTCL   Rn */
void sh_common_execution::ROTCL(uint32_t n)
{
	uint32_t temp;

	temp = (m_sh2_state->r[n] >> 31) & T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->sr & T);
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | temp;
}

/*  ROTCR   Rn */
void sh_common_execution::ROTCR(uint32_t n)
{
	uint32_t temp;
	temp = (m_sh2_state->sr & T) << 31;
	if (m_sh2_state->r[n] & T)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 1) | temp;
}

/*  ROTL    Rn */
void sh_common_execution::ROTL(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->r[n] >> 31);
}

/*  ROTR    Rn */
void sh_common_execution::ROTR(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 1) | (m_sh2_state->r[n] << 31);
}

/*  RTS */
void sh_common_execution::RTS()
{
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->pr;
	m_sh2_state->icount--;
}

/*  SETT */
void sh_common_execution::SETT()
{
	m_sh2_state->sr |= T;
}

/*  SHAL    Rn      (same as SHLL) */
void sh_common_execution::SHAL(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] <<= 1;
}

/*  SHAR    Rn */
void sh_common_execution::SHAR(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] = (uint32_t)((int32_t)m_sh2_state->r[n] >> 1);
}

/*  SHLL    Rn      (same as SHAL) */
void sh_common_execution::SHLL(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] <<= 1;
}

/*  SHLL2   Rn */
void sh_common_execution::SHLL2(uint32_t n)
{
	m_sh2_state->r[n] <<= 2;
}

/*  SHLL8   Rn */
void sh_common_execution::SHLL8(uint32_t n)
{
	m_sh2_state->r[n] <<= 8;
}

/*  SHLL16  Rn */
void sh_common_execution::SHLL16(uint32_t n)
{
	m_sh2_state->r[n] <<= 16;
}

/*  SHLR    Rn */
void sh_common_execution::SHLR(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] >>= 1;
}

/*  SHLR2   Rn */
void sh_common_execution::SHLR2(uint32_t n)
{
	m_sh2_state->r[n] >>= 2;
}

/*  SHLR8   Rn */
void sh_common_execution::SHLR8(uint32_t n)
{
	m_sh2_state->r[n] >>= 8;
}

/*  SHLR16  Rn */
void sh_common_execution::SHLR16(uint32_t n)
{
	m_sh2_state->r[n] >>= 16;
}


/*  STC     SR,Rn */
void sh_common_execution::STCSR(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->sr;
}

/*  STC     GBR,Rn */
void sh_common_execution::STCGBR(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->gbr;
}

/*  STC     VBR,Rn */
void sh_common_execution::STCVBR(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->vbr;
}

/*  STC.L   SR,@-Rn */
void sh_common_execution::STCMSR(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->sr );
	m_sh2_state->icount--;
}

/*  STC.L   GBR,@-Rn */
void sh_common_execution::STCMGBR(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->gbr );
	m_sh2_state->icount--;
}

/*  STC.L   VBR,@-Rn */
void sh_common_execution::STCMVBR(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->vbr );
	m_sh2_state->icount--;
}

/*  STS     MACH,Rn */
void sh_common_execution::STSMACH(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->mach;
}

/*  STS     MACL,Rn */
void sh_common_execution::STSMACL(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->macl;
}

/*  STS     PR,Rn */
void sh_common_execution::STSPR(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->pr;
}

/*  STS.L   MACH,@-Rn */
void sh_common_execution::STSMMACH(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->mach );
}

/*  STS.L   MACL,@-Rn */
void sh_common_execution::STSMMACL(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->macl );
}

/*  STS.L   PR,@-Rn */
void sh_common_execution::STSMPR(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->pr );
}

/*  SUB     Rm,Rn */
void sh_common_execution::SUB(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] -= m_sh2_state->r[m];
}

/*  SUBC    Rm,Rn */
void sh_common_execution::SUBC(uint32_t m, uint32_t n)
{
	uint32_t tmp0, tmp1;

	tmp1 = m_sh2_state->r[n] - m_sh2_state->r[m];
	tmp0 = m_sh2_state->r[n];
	m_sh2_state->r[n] = tmp1 - (m_sh2_state->sr & T);
	if (tmp0 < tmp1)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	if (tmp1 < m_sh2_state->r[n])
		m_sh2_state->sr |= T;
}

/*  SUBV    Rm,Rn */
void sh_common_execution::SUBV(uint32_t m, uint32_t n)
{
	int32_t dest, src, ans;

	if ((int32_t) m_sh2_state->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((int32_t) m_sh2_state->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	m_sh2_state->r[n] -= m_sh2_state->r[m];
	if ((int32_t) m_sh2_state->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 1)
	{
		if (ans == 1)
			m_sh2_state->sr |= T;
		else
			m_sh2_state->sr &= ~T;
	}
	else
		m_sh2_state->sr &= ~T;
}

/*  SWAP.B  Rm,Rn */
void sh_common_execution::SWAPB(uint32_t m, uint32_t n)
{
	uint32_t temp0, temp1;

	temp0 = m_sh2_state->r[m] & 0xffff0000;
	temp1 = (m_sh2_state->r[m] & 0x000000ff) << 8;
	m_sh2_state->r[n] = (m_sh2_state->r[m] >> 8) & 0x000000ff;
	m_sh2_state->r[n] = m_sh2_state->r[n] | temp1 | temp0;
}

/*  SWAP.W  Rm,Rn */
void sh_common_execution::SWAPW(uint32_t m, uint32_t n)
{
	uint32_t temp;

	temp = (m_sh2_state->r[m] >> 16) & 0x0000ffff;
	m_sh2_state->r[n] = (m_sh2_state->r[m] << 16) | temp;
}

/*  TAS.B   @Rn */
void sh_common_execution::TAS(uint32_t n)
{
	uint32_t temp;
	m_sh2_state->ea = m_sh2_state->r[n];
	/* Bus Lock enable */
	temp = RB( m_sh2_state->ea );
	if (temp == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	temp |= 0x80;
	/* Bus Lock disable */
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 3;
}

/*  TST     Rm,Rn */
void sh_common_execution::TST(uint32_t m, uint32_t n)
{
	if ((m_sh2_state->r[n] & m_sh2_state->r[m]) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  TST     #imm,R0 */
void sh_common_execution::TSTI(uint32_t i)
{
	uint32_t imm = i & 0xff;

	if ((imm & m_sh2_state->r[0]) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  TST.B   #imm,@(R0,GBR) */
void sh_common_execution::TSTM(uint32_t i)
{
	uint32_t imm = i & 0xff;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	if ((imm & RB( m_sh2_state->ea )) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	m_sh2_state->icount -= 2;
}

/*  XOR     Rm,Rn */
void sh_common_execution::XOR(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] ^= m_sh2_state->r[m];
}

/*  XOR     #imm,R0 */
void sh_common_execution::XORI(uint32_t i)
{
	uint32_t imm = i & 0xff;
	m_sh2_state->r[0] ^= imm;
}

/*  XOR.B   #imm,@(R0,GBR) */
void sh_common_execution::XORM(uint32_t i)
{
	uint32_t imm = i & 0xff;
	uint32_t temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = RB( m_sh2_state->ea );
	temp ^= imm;
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 2;
}

/*  XTRCT   Rm,Rn */
void sh_common_execution::XTRCT(uint32_t m, uint32_t n)
{
	uint32_t temp;

	temp = (m_sh2_state->r[m] << 16) & 0xffff0000;
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 16) & 0x0000ffff;
	m_sh2_state->r[n] |= temp;
}

/*  SLEEP */
void sh_common_execution::SLEEP()
{
	/* 0 = normal mode */
	/* 1 = enters into power-down mode */
	/* 2 = go out the power-down mode after an exception */
	if(m_sh2_state->sleep_mode != 2)
		m_sh2_state->pc -= 2;
	m_sh2_state->icount -= 2;
	/* Wait_for_exception; */
	if(m_sh2_state->sleep_mode == 0)
		m_sh2_state->sleep_mode = 1;
	else if(m_sh2_state->sleep_mode == 2)
		m_sh2_state->sleep_mode = 0;
}

/* Common dispatch */

void sh_common_execution::op0010(uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: MOVBS(Rm, Rn);                break;
	case  1: MOVWS(Rm, Rn);                break;
	case  2: MOVLS(Rm, Rn);                break;
	case  3: ILLEGAL();                         break;
	case  4: MOVBM(Rm, Rn);                break;
	case  5: MOVWM(Rm, Rn);                break;
	case  6: MOVLM(Rm, Rn);                break;
	case  7: DIV0S(Rm, Rn);                break;
	case  8: TST(Rm, Rn);                  break;
	case  9: AND(Rm, Rn);                  break;
	case 10: XOR(Rm, Rn);                  break;
	case 11: OR(Rm, Rn);                   break;
	case 12: CMPSTR(Rm, Rn);               break;
	case 13: XTRCT(Rm, Rn);                break;
	case 14: MULU(Rm, Rn);                 break;
	case 15: MULS(Rm, Rn);                 break;
	}
}

void sh_common_execution::op0011(uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: CMPEQ(Rm, Rn);                break;
	case  1: ILLEGAL();                         break;
	case  2: CMPHS(Rm, Rn);                break;
	case  3: CMPGE(Rm, Rn);                break;
	case  4: DIV1(Rm, Rn);                 break;
	case  5: DMULU(Rm, Rn);                break;
	case  6: CMPHI(Rm, Rn);                break;
	case  7: CMPGT(Rm, Rn);                break;
	case  8: SUB(Rm, Rn);                  break;
	case  9: ILLEGAL();                         break;
	case 10: SUBC(Rm, Rn);                 break;
	case 11: SUBV(Rm, Rn);                 break;
	case 12: ADD(Rm, Rn);                  break;
	case 13: DMULS(Rm, Rn);                break;
	case 14: ADDC(Rm, Rn);                 break;
	case 15: ADDV(Rm, Rn);                 break;
	}
}

void sh_common_execution::op0110(uint16_t opcode)
{
	switch (opcode & 15)
	{
	case  0: MOVBL(Rm, Rn);                break;
	case  1: MOVWL(Rm, Rn);                break;
	case  2: MOVLL(Rm, Rn);                break;
	case  3: MOV(Rm, Rn);                  break;
	case  4: MOVBP(Rm, Rn);                break;
	case  5: MOVWP(Rm, Rn);                break;
	case  6: MOVLP(Rm, Rn);                break;
	case  7: NOT(Rm, Rn);                  break;
	case  8: SWAPB(Rm, Rn);                break;
	case  9: SWAPW(Rm, Rn);                break;
	case 10: NEGC(Rm, Rn);                 break;
	case 11: NEG(Rm, Rn);                  break;
	case 12: EXTUB(Rm, Rn);                break;
	case 13: EXTUW(Rm, Rn);                break;
	case 14: EXTSB(Rm, Rn);                break;
	case 15: EXTSW(Rm, Rn);                break;
	}
}

void sh_common_execution::op1000(uint16_t opcode)
{
	switch ( opcode  & (15<<8) )
	{
	case  0 << 8: MOVBS4(opcode & 0x0f, Rm);   break;
	case  1 << 8: MOVWS4(opcode & 0x0f, Rm);   break;
	case  2<< 8: ILLEGAL();                 break;
	case  3<< 8: ILLEGAL();                 break;
	case  4<< 8: MOVBL4(Rm, opcode & 0x0f);    break;
	case  5<< 8: MOVWL4(Rm, opcode & 0x0f);    break;
	case  6<< 8: ILLEGAL();                 break;
	case  7<< 8: ILLEGAL();                 break;
	case  8<< 8: CMPIM(opcode & 0xff);     break;
	case  9<< 8: BT(opcode & 0xff);        break;
	case 10<< 8: ILLEGAL();                 break;
	case 11<< 8: BF(opcode & 0xff);        break;
	case 12<< 8: ILLEGAL();                 break;
	case 13<< 8: BTS(opcode & 0xff);       break;
	case 14<< 8: ILLEGAL();                 break;
	case 15<< 8: BFS(opcode & 0xff);       break;
	}
}


void sh_common_execution::op1100(uint16_t opcode)
{
	switch (opcode & (15<<8))
	{
	case  0<<8: MOVBSG(opcode & 0xff);     break;
	case  1<<8: MOVWSG(opcode & 0xff);     break;
	case  2<<8: MOVLSG(opcode & 0xff);     break;
	case  3<<8: TRAPA(opcode & 0xff);      break; // sh2/4 differ
	case  4<<8: MOVBLG(opcode & 0xff);     break;
	case  5<<8: MOVWLG(opcode & 0xff);     break;
	case  6<<8: MOVLLG(opcode & 0xff);     break;
	case  7<<8: MOVA(opcode & 0xff);       break;
	case  8<<8: TSTI(opcode & 0xff);       break;
	case  9<<8: ANDI(opcode & 0xff);       break;
	case 10<<8: XORI(opcode & 0xff);       break;
	case 11<<8: ORI(opcode & 0xff);            break;
	case 12<<8: TSTM(opcode & 0xff);       break;
	case 13<<8: ANDM(opcode & 0xff);       break;
	case 14<<8: XORM(opcode & 0xff);       break;
	case 15<<8: ORM(opcode & 0xff);            break;
	}
}


void sh_common_execution::execute_one(const uint16_t opcode)
{
	switch(opcode & 0xf000)
	{
		case 0x0000: execute_one_0000(opcode); break;
		case 0x1000: MOVLS4(Rm, opcode & 0x0f, Rn);	break;
		case 0x2000: op0010(opcode); break;
		case 0x3000: op0011(opcode); break;
		case 0x4000: execute_one_4000(opcode); break;
		case 0x5000: MOVLL4(Rm, opcode & 0x0f, Rn); break;
		case 0x6000: op0110(opcode); break;
		case 0x7000: ADDI(opcode & 0xff, Rn); break;
		case 0x8000: op1000(opcode); break;
		case 0x9000: MOVWI(opcode & 0xff, Rn); break;
		case 0xa000: BRA(opcode & 0xfff); break;
		case 0xb000: BSR(opcode & 0xfff); break;
		case 0xc000: op1100(opcode); break;
		case 0xd000: MOVLI(opcode & 0xff, Rn); break;
		case 0xe000: MOVI(opcode & 0xff, Rn); break;
		case 0xf000: execute_one_f000(opcode); break;
	}
}

// DRC / UML related


/*-------------------------------------------------
    sh2drc_add_fastram - add a new fastram
    region
-------------------------------------------------*/

void sh_common_execution::sh2drc_add_fastram(offs_t start, offs_t end, uint8_t readonly, void *base)
{
	if (m_fastram_select < ARRAY_LENGTH(m_fastram))
	{
		m_fastram[m_fastram_select].start = start;
		m_fastram[m_fastram_select].end = end;
		m_fastram[m_fastram_select].readonly = readonly;
		m_fastram[m_fastram_select].base = base;
		m_fastram_select++;
	}
}

using namespace uml;

void cfunc_fastirq(void *param) { ((sh_common_execution *)param)->func_fastirq(); };
void cfunc_unimplemented(void *param) { ((sh_common_execution *)param)->func_unimplemented(); }
void cfunc_MAC_W(void *param) { ((sh_common_execution *)param)->func_MAC_W(); }
void cfunc_MAC_L(void *param) { ((sh_common_execution *)param)->func_MAC_L(); }
void cfunc_DIV1(void *param) { ((sh_common_execution *)param)->func_DIV1(); }
void cfunc_ADDV(void *param) { ((sh_common_execution *)param)->func_ADDV(); }
void cfunc_SUBV(void *param) { ((sh_common_execution *)param)->func_SUBV(); }
void cfunc_printf_probe(void *param) { ((sh_common_execution *)param)->func_printf_probe(); }

void sh_common_execution::func_fastirq()
{
	sh2_exception("fastirq",m_sh2_state->irqline);
}

void sh_common_execution::func_unimplemented()
{
	// set up an invalid opcode exception
	m_sh2_state->evec = RL( m_sh2_state->vbr + 4 * 4 );
	m_sh2_state->evec &= m_am;
	m_sh2_state->irqsr = m_sh2_state->sr;
	// claim it's an NMI, because it pretty much is
	m_sh2_state->pending_nmi = 1;
}

void sh_common_execution::func_MAC_W()
{
	uint16_t opcode;
	int n, m;

	// recover the opcode
	opcode = m_sh2_state->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	MAC_W(m, n);
}


void sh_common_execution::func_MAC_L()
{
	uint16_t opcode;
	int n, m;

	// recover the opcode
	opcode = m_sh2_state->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	MAC_L(m, n);
}


void sh_common_execution::func_DIV1()
{
	uint16_t opcode;
	int n, m;

	// recover the opcode
	opcode = m_sh2_state->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	DIV1(m, n);
}


void sh_common_execution::func_ADDV()
{
	uint16_t opcode;
	int n, m;

	// recover the opcode
	opcode = m_sh2_state->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	ADDV(m, n);
}


void sh_common_execution::func_SUBV()
{
	uint16_t opcode;
	int n, m;

	// recover the opcode
	opcode = m_sh2_state->arg0;

	// extract the operands
	n = Rn;
	m = Rm;

	SUBV(m, n);
}


void sh_common_execution::func_printf_probe()
{
	uint32_t pc = m_sh2_state->pc;

	printf(" PC=%08X          r0=%08X  r1=%08X  r2=%08X\n",
		pc,
		(uint32_t)m_sh2_state->r[0],
		(uint32_t)m_sh2_state->r[1],
		(uint32_t)m_sh2_state->r[2]);
	printf(" r3=%08X  r4=%08X  r5=%08X  r6=%08X\n",
		(uint32_t)m_sh2_state->r[3],
		(uint32_t)m_sh2_state->r[4],
		(uint32_t)m_sh2_state->r[5],
		(uint32_t)m_sh2_state->r[6]);
	printf(" r7=%08X  r8=%08X  r9=%08X  r10=%08X\n",
		(uint32_t)m_sh2_state->r[7],
		(uint32_t)m_sh2_state->r[8],
		(uint32_t)m_sh2_state->r[9],
		(uint32_t)m_sh2_state->r[10]);
	printf(" r11=%08X  r12=%08X  r13=%08X  r14=%08X\n",
		(uint32_t)m_sh2_state->r[11],
		(uint32_t)m_sh2_state->r[12],
		(uint32_t)m_sh2_state->r[13],
		(uint32_t)m_sh2_state->r[14]);
	printf(" r15=%08X  macl=%08X  mach=%08X  gbr=%08X\n",
		(uint32_t)m_sh2_state->r[15],
		(uint32_t)m_sh2_state->macl,
		(uint32_t)m_sh2_state->mach,
		(uint32_t)m_sh2_state->gbr);
	printf(" evec %x irqsr %x pc=%08x\n",
		(uint32_t)m_sh2_state->evec,
		(uint32_t)m_sh2_state->irqsr, (uint32_t)m_sh2_state->pc);
}

/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/
void sh_common_execution::generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, bool allow_exception)
{
	/* check full interrupts if pending */
	if (compiler->checkints)
	{
		code_label skip = compiler->labelnum++;

		compiler->checkints = false;
		compiler->labelnum += 4;

		/* check for interrupts */
		UML_MOV(block, mem(&m_sh2_state->irqline), 0xffffffff);     // mov irqline, #-1
		UML_CMP(block, mem(&m_sh2_state->pending_nmi), 0);          // cmp pending_nmi, #0
		UML_JMPc(block, COND_Z, skip+2);                    // jz skip+2

		UML_MOV(block, mem(&m_sh2_state->pending_nmi), 0);          // zap pending_nmi
		UML_JMP(block, skip+1);                     // and then go take it (evec is already set)

		UML_LABEL(block, skip+2);                   // skip+2:
		UML_MOV(block, mem(&m_sh2_state->evec), 0xffffffff);        // mov evec, -1
		UML_MOV(block, I0, 0xffffffff);         // mov r0, -1 (r0 = irq)
		UML_AND(block, I1,  I0, 0xffff);                // and r1, r0, 0xffff

		UML_LZCNT(block, I1, mem(&m_sh2_state->pending_irq));       // lzcnt r1, pending_irq
		UML_CMP(block, I1, 32);             // cmp r1, #32
		UML_JMPc(block, COND_Z, skip+4);                    // jz skip+4

		UML_SUB(block, mem(&m_sh2_state->irqline), 31, I1);     // sub irqline, #31, r1

		UML_LABEL(block, skip+4);                   // skip+4:
		UML_CMP(block, mem(&m_sh2_state->internal_irq_level), 0xffffffff);  // cmp internal_irq_level, #-1
		UML_JMPc(block, COND_Z, skip+3);                    // jz skip+3
		UML_CMP(block, mem(&m_sh2_state->internal_irq_level), mem(&m_sh2_state->irqline));      // cmp internal_irq_level, irqline
		UML_JMPc(block, COND_LE, skip+3);                   // jle skip+3

		UML_MOV(block, mem(&m_sh2_state->irqline), mem(&m_sh2_state->internal_irq_level));      // mov r0, internal_irq_level

		UML_LABEL(block, skip+3);                   // skip+3:
		UML_CMP(block, mem(&m_sh2_state->irqline), 0xffffffff);     // cmp irqline, #-1
		UML_JMPc(block, COND_Z, skip+1);                    // jz skip+1
		UML_CALLC(block, cfunc_fastirq, this);               // callc fastirq

		UML_LABEL(block, skip+1);                   // skip+1:
		UML_CMP(block, mem(&m_sh2_state->evec), 0xffffffff);        // cmp evec, 0xffffffff
		UML_JMPc(block, COND_Z, skip);                  // jz skip

		UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
		UML_MOV(block, I0, R32(15));                // mov r0, R15
		UML_MOV(block, I1, mem(&m_sh2_state->irqsr));           // mov r1, irqsr
		UML_CALLH(block, *m_write32);                    // call write32

		UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
		UML_MOV(block, I0, R32(15));                // mov r0, R15
		UML_MOV(block, I1, param);              // mov r1, nextpc
		UML_CALLH(block, *m_write32);                    // call write32

		UML_HASHJMP(block, 0, mem(&m_sh2_state->evec), *m_nocode);       // hashjmp m_sh2_state->evec

		UML_LABEL(block, skip);                         // skip:
	}

	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, mem(&m_sh2_state->icount), mem(&m_sh2_state->icount), MAPVAR_CYCLES);    // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                        // mapvar  cycles,0
		if (allow_exception)
			UML_EXHc(block, COND_S, *m_out_of_cycles, param);
																					// exh     out_of_cycles,nextpc
	}
	compiler->cycles = 0;
}

bool sh_common_execution::generate_group_2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 15)
	{
	case  0: // MOVBS(Rm, Rn);
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_AND(block, I1, R32(Rm), 0xff);  // and r1, Rm, 0xff
		UML_CALLH(block, *m_write8);

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  1: // MOVWS(Rm, Rn);
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_AND(block, I1, R32(Rm), 0xffff);    // and r1, Rm, 0xffff
		UML_CALLH(block, *m_write16);

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  2: // MOVLS(Rm, Rn);
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_MOV(block, I1, R32(Rm));        // mov r1, Rm
		UML_CALLH(block, *m_write32);

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  3:
		return false;

	case  4: // MOVBM(Rm, Rn);
		UML_MOV(block, I1, R32(Rm));        // mov r1, Rm
		UML_SUB(block, R32(Rn), R32(Rn), 1);    // sub Rn, Rn, 1
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_CALLH(block, *m_write8);         // call write8

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  5: // MOVWM(Rm, Rn);
		UML_MOV(block, I1, R32(Rm));        // mov r1, Rm
		UML_SUB(block, R32(Rn), R32(Rn), 2);    // sub Rn, Rn, 2
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_CALLH(block, *m_write16);            // call write16

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  6: // MOVLM(Rm, Rn);
		UML_MOV(block, I1, R32(Rm));        // mov r1, Rm
		UML_SUB(block, R32(Rn), R32(Rn), 4);    // sub Rn, Rn, 4
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_CALLH(block, *m_write32);            // call write32

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 13: // XTRCT(Rm, Rn);
		UML_SHL(block, I0, R32(Rm), 16);        // shl r0, Rm, #16
		UML_AND(block, I0, I0, 0xffff0000); // and r0, r0, #0xffff0000

		UML_SHR(block, I1, R32(Rn), 16);        // shr, r1, Rn, #16
		UML_AND(block, I1, I1, 0xffff);     // and r1, r1, #0x0000ffff

		UML_OR(block, R32(Rn), I0, I1);     // or Rn, r0, r1
		return true;

	case  7: // DIV0S(Rm, Rn);
		UML_MOV(block, I0, mem(&m_sh2_state->sr));              // move r0, sr
		UML_AND(block, I0, I0, ~(Q|M|T));       // and r0, r0, ~(Q|M|T) (clear the Q,M, and T bits)

		UML_TEST(block, R32(Rn), 0x80000000);           // test Rn, #0x80000000
		UML_JMPc(block, COND_Z, compiler->labelnum);            // jz labelnum

		UML_OR(block, I0, I0, Q);               // or r0, r0, Q
		UML_LABEL(block, compiler->labelnum++);             // labelnum:

		UML_TEST(block, R32(Rm), 0x80000000);           // test Rm, #0x80000000
		UML_JMPc(block, COND_Z, compiler->labelnum);            // jz labelnum

		UML_OR(block, I0, I0, M);               // or r0, r0, M
		UML_LABEL(block, compiler->labelnum++);             // labelnum:

		UML_XOR(block, I1, R32(Rn), R32(Rm));           // xor r1, Rn, Rm
		UML_TEST(block, I1, 0x80000000);            // test r1, #0x80000000
		UML_JMPc(block, COND_Z, compiler->labelnum);            // jz labelnum

		UML_OR(block, I0, I0, T);               // or r0, r0, T
		UML_LABEL(block, compiler->labelnum++);             // labelnum:
		UML_MOV(block, mem(&m_sh2_state->sr), I0);              // mov sr, r0
		return true;

	case  8: // TST(Rm, Rn);
		UML_AND(block, I0, mem(&m_sh2_state->sr), ~T);  // and r0, sr, ~T (clear the T bit)
		UML_TEST(block, R32(Rm), R32(Rn));      // test Rm, Rn
		UML_JMPc(block, COND_NZ, compiler->labelnum);   // jnz compiler->labelnum

		UML_OR(block, I0, I0, T);   // or r0, r0, T
		UML_LABEL(block, compiler->labelnum++);         // desc->pc:

		UML_MOV(block, mem(&m_sh2_state->sr), I0);      // mov m_sh2_state->sr, r0
		return true;

	case 12: // CMPSTR(Rm, Rn);
		UML_XOR(block, I0, R32(Rn), R32(Rm));   // xor r0, Rn, Rm       (temp)

		UML_SHR(block, I1, I0, 24); // shr r1, r0, #24  (HH)
		UML_AND(block, I1, I1, 0xff);   // and r1, r1, #0xff

		UML_SHR(block, I2, I0, 16); // shr r2, r0, #16  (HL)
		UML_AND(block, I2, I2, 0xff);   // and r2, r2, #0xff

		UML_SHR(block, I3, I0, 8);  // shr r3, r0, #8   (LH)
		UML_AND(block, I3, I3, 0xff);   // and r3, r3, #0xff

		UML_AND(block, I7, I0, 0xff);   // and r7, r0, #0xff    (LL)

		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~T);   // and sr, sr, ~T (clear the T bit)

		UML_CMP(block, I1, 0);      // cmp r1, #0
		UML_JMPc(block, COND_Z, compiler->labelnum);    // jnz labelnum
		UML_CMP(block, I2, 0);      // cmp r2, #0
		UML_JMPc(block, COND_Z, compiler->labelnum);    // jnz labelnum
		UML_CMP(block, I3, 0);      // cmp r3, #0
		UML_JMPc(block, COND_Z, compiler->labelnum);    // jnz labelnum
		UML_CMP(block, I7, 0);      // cmp r7, #0
		UML_JMPc(block, COND_NZ, compiler->labelnum+1); // jnz labelnum

		UML_LABEL(block, compiler->labelnum++);     // labelnum:
		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), T); // or sr, sr, T

		UML_LABEL(block, compiler->labelnum++);     // labelnum+1:
		return true;

	case  9: // AND(Rm, Rn);
		UML_AND(block, R32(Rn), R32(Rn), R32(Rm));  // and Rn, Rn, Rm
		return true;

	case 10: // XOR(Rm, Rn);
		UML_XOR(block, R32(Rn), R32(Rn), R32(Rm));  // xor Rn, Rn, Rm
		return true;

	case 11: // OR(Rm, Rn);
		UML_OR(block, R32(Rn), R32(Rn), R32(Rm));   // or Rn, Rn, Rm
		return true;

	case 14: // MULU(Rm, Rn);
		UML_AND(block, I0, R32(Rm), 0xffff);                // and r0, Rm, 0xffff
		UML_AND(block, I1, R32(Rn), 0xffff);                // and r1, Rn, 0xffff
		UML_MULU(block, mem(&m_sh2_state->macl), mem(&m_sh2_state->ea), I0, I1);    // mulu macl, ea, r0, r1
		return true;

	case 15: // MULS(Rm, Rn);
		UML_SEXT(block, I0, R32(Rm), SIZE_WORD);                // sext r0, Rm
		UML_SEXT(block, I1, R32(Rn), SIZE_WORD);                // sext r1, Rn
		UML_MULS(block, mem(&m_sh2_state->macl), mem(&m_sh2_state->ea), I0, I1);    // muls macl, ea, r0, r1
		return true;
	}

	return false;
}


bool sh_common_execution::generate_group_3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, uint32_t ovrpc)
{
	switch (opcode & 15)
	{
	case  0: // CMPEQ(Rm, Rn); (equality)
		UML_CMP(block, R32(Rn), R32(Rm));       // cmp Rn, Rm
		UML_SETc(block, COND_E, I0);            // set E, r0
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, 1); // rolins sr, r0, 0, 1
		return true;

	case  2: // CMPHS(Rm, Rn); (unsigned greater than or equal)
		UML_CMP(block, R32(Rn), R32(Rm));       // cmp Rn, Rm
		UML_SETc(block, COND_AE, I0);       // set AE, r0
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, 1); // rolins sr, r0, 0, 1
		return true;

	case  3: // CMPGE(Rm, Rn); (signed greater than or equal)
		UML_CMP(block, R32(Rn), R32(Rm));       // cmp Rn, Rm
		UML_SETc(block, COND_GE, I0);       // set GE, r0
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, 1); // rolins sr, r0, 0, 1
		return true;

	case  6: // CMPHI(Rm, Rn); (unsigned greater than)
		UML_CMP(block, R32(Rn), R32(Rm));       // cmp Rn, Rm
		UML_SETc(block, COND_A, I0);            // set A, r0
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, 1); // rolins sr, r0, 0, 1
		return true;

	case  7: // CMPGT(Rm, Rn); (signed greater than)
		UML_CMP(block, R32(Rn), R32(Rm));       // cmp Rn, Rm
		UML_SETc(block, COND_G, I0);            // set G, r0
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, 1); // rolins sr, r0, 0, 1
		return true;

	case  1:
	case  9:
		return false;

	case  4: // DIV1(Rm, Rn);
		save_fast_iregs(block);
		UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
		UML_CALLC(block, cfunc_DIV1, this);
		load_fast_iregs(block);
		return true;

	case  5: // DMULU(Rm, Rn);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			UML_MULU(block, mem(&m_sh2_state->macl), mem(&m_sh2_state->mach), R32(Rn), R32(Rm));
			return true;
		}
		break;

	case 13: // DMULS(Rm, Rn);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			UML_MULS(block, mem(&m_sh2_state->macl), mem(&m_sh2_state->mach), R32(Rn), R32(Rm));
			return true;
		}
		break;

	case  8: // SUB(Rm, Rn);
		UML_SUB(block, R32(Rn), R32(Rn), R32(Rm));  // sub Rn, Rn, Rm
		return true;

	case 12: // ADD(Rm, Rn);
		UML_ADD(block, R32(Rn), R32(Rn), R32(Rm));  // add Rn, Rn, Rm
		return true;

	case 10: // SUBC(Rm, Rn);
		UML_CARRY(block, mem(&m_sh2_state->sr), 0); // carry = T (T is bit 0 of SR)
		UML_SUBB(block, R32(Rn), R32(Rn), R32(Rm)); // addc Rn, Rn, Rm
		UML_SETc(block, COND_C, I0);                // setc    i0, C
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, T); // rolins sr,i0,0,T
		return true;

	case 11: // SUBV(Rm, Rn);
		save_fast_iregs(block);
		UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
		UML_CALLC(block, cfunc_SUBV, this);
		load_fast_iregs(block);
		return true;

	case 14: // ADDC(Rm, Rn);
		UML_CARRY(block, mem(&m_sh2_state->sr), 0); // carry = T (T is bit 0 of SR)
		UML_ADDC(block, R32(Rn), R32(Rn), R32(Rm)); // addc Rn, Rn, Rm
		UML_SETc(block, COND_C, I0);                // setc    i0, C
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, T); // rolins sr,i0,0,T
		return true;

	case 15: // ADDV(Rm, Rn);
		save_fast_iregs(block);
		UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
		UML_CALLC(block, cfunc_ADDV, this);
		load_fast_iregs(block);
		return true;
	}
	return false;
}


bool sh_common_execution::generate_group_6(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 15)
	{
	case  0: // MOVBL(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));        // mov r0, Rm
		SETEA(0);                   // debug: ea = r0
		UML_CALLH(block, *m_read8);          // call read8
		UML_SEXT(block, R32(Rn), I0, SIZE_BYTE);    // sext Rn, r0, BYTE

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  1: // MOVWL(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));        // mov r0, Rm
		SETEA(0);                   // debug: ea = r0
		UML_CALLH(block, *m_read16);         // call read16
		UML_SEXT(block, R32(Rn), I0, SIZE_WORD);    // sext Rn, r0, WORD

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  2: // MOVLL(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));        // mov r0, Rm
		SETEA(0);                   // debug: ea = r0
		UML_CALLH(block, *m_read32);         // call read32
		UML_MOV(block, R32(Rn), I0);        // mov Rn, r0

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  3: // MOV(Rm, Rn);
		UML_MOV(block, R32(Rn), R32(Rm));       // mov Rn, Rm
		return true;

	case  7: // NOT(Rm, Rn);
		UML_XOR(block, R32(Rn), R32(Rm), 0xffffffff);   // xor Rn, Rm, 0xffffffff
		return true;

	case  9: // SWAPW(Rm, Rn);
		UML_ROL(block, R32(Rn), R32(Rm), 16);   // rol Rn, Rm, 16
		return true;

	case 11: // NEG(Rm, Rn);
		UML_SUB(block, R32(Rn), 0, R32(Rm));    // sub Rn, 0, Rm
		return true;

	case 12: // EXTUB(Rm, Rn);
		UML_AND(block, R32(Rn), R32(Rm), 0x000000ff);   // and Rn, Rm, 0xff
		return true;

	case 13: // EXTUW(Rm, Rn);
		UML_AND(block, R32(Rn), R32(Rm), 0x0000ffff);   // and Rn, Rm, 0xffff
		return true;

	case 14: // EXTSB(Rm, Rn);
		UML_SEXT(block, R32(Rn), R32(Rm), SIZE_BYTE);       // sext Rn, Rm, BYTE
		return true;

	case 15: // EXTSW(Rm, Rn);
		UML_SEXT(block, R32(Rn), R32(Rm), SIZE_WORD);       // sext Rn, Rm, WORD
		return true;

	case  4: // MOVBP(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));        // mov r0, Rm
		UML_CALLH(block, *m_read8);          // call read8
		UML_SEXT(block, R32(Rn), I0, SIZE_BYTE);        // sext Rn, r0, BYTE

		if (Rm != Rn)
			UML_ADD(block, R32(Rm), R32(Rm), 1);    // add Rm, Rm, #1

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  5: // MOVWP(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));        // mov r0, Rm
		UML_CALLH(block, *m_read16);         // call read16
		UML_SEXT(block, R32(Rn), I0, SIZE_WORD);        // sext Rn, r0, WORD

		if (Rm != Rn)
			UML_ADD(block, R32(Rm), R32(Rm), 2);    // add Rm, Rm, #2

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  6: // MOVLP(Rm, Rn);
		UML_MOV(block, I0, R32(Rm));        // mov r0, Rm
		UML_CALLH(block, *m_read32);         // call read32
		UML_MOV(block, R32(Rn), I0);        // mov Rn, r0

		if (Rm != Rn)
			UML_ADD(block, R32(Rm), R32(Rm), 4);    // add Rm, Rm, #4

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  8: // SWAPB(Rm, Rn);
		UML_AND(block, I0, R32(Rm), 0xffff0000);    // and r0, Rm, #0xffff0000
		UML_AND(block, I1, R32(Rm), 0x000000ff);    // and r0, Rm, #0x000000ff
		UML_AND(block, I2, R32(Rm), 0x0000ff00);    // and r0, Rm, #0x0000ff00
		UML_SHL(block, I1, I1, 8);      // shl r1, r1, #8
		UML_SHR(block, I2, I2, 8);      // shr r2, r2, #8
		UML_OR(block, I0, I0, I1);      // or r0, r0, r1
		UML_OR(block, R32(Rn), I0, I2);     // or Rn, r0, r2
		return true;

	case 10: // NEGC(Rm, Rn);
		UML_MOV(block, I0, mem(&m_sh2_state->sr));      // mov r0, sr (save SR)
		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~T);   // and sr, sr, ~T (clear the T bit)
		UML_CARRY(block, I0, 0);    // carry = T (T is bit 0 of SR)
		UML_SUBB(block, R32(Rn), 0, R32(Rm));   // subb Rn, #0, Rm

		UML_JMPc(block, COND_NC, compiler->labelnum);   // jnc labelnum

		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), T); // or sr, sr, T

		UML_LABEL(block, compiler->labelnum++);     // labelnum:

		return true;
	}

	return false;
}

bool sh_common_execution::generate_group_8(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	int32_t disp;
	uint32_t udisp;
	code_label templabel;

	switch ( opcode  & (15<<8) )
	{
	case  0 << 8: // MOVBS4(opcode & 0x0f, Rm);
		udisp = (opcode & 0x0f);
		UML_ADD(block, I0, R32(Rm), udisp);     // add r0, Rm, udisp
		UML_MOV(block, I1, R32(0));         // mov r1, R0
		UML_CALLH(block, *m_write8);             // call write8

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  1 << 8: // MOVWS4(opcode & 0x0f, Rm);
		udisp = (opcode & 0x0f) * 2;
		UML_ADD(block, I0, R32(Rm), udisp);     // add r0, Rm, udisp
		UML_MOV(block, I1, R32(0));         // mov r1, R0
		UML_CALLH(block, *m_write16);                // call write16

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  2<< 8:
	case  3<< 8:
	case  6<< 8:
	case  7<< 8:
	case 10<< 8:
	case 12<< 8:
	case 14<< 8:
		return false;

	case  4<< 8: // MOVBL4(Rm, opcode & 0x0f);
		udisp = opcode & 0x0f;
		UML_ADD(block, I0, R32(Rm), udisp);     // add r0, Rm, udisp
		SETEA(0);
		UML_CALLH(block, *m_read8);              // call read8
		UML_SEXT(block, R32(0), I0, SIZE_BYTE);         // sext R0, r0, BYTE

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  5<< 8: // MOVWL4(Rm, opcode & 0x0f);
		udisp = (opcode & 0x0f)*2;
		UML_ADD(block, I0, R32(Rm), udisp);     // add r0, Rm, udisp
		SETEA(0);
		UML_CALLH(block, *m_read16);             // call read16
		UML_SEXT(block, R32(0), I0, SIZE_WORD);         // sext R0, r0, WORD

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  8<< 8: // CMPIM(opcode & 0xff);
		UML_AND(block, I0, mem(&m_sh2_state->sr), ~T);  // and r0, sr, ~T (clear the T bit)

		UML_SEXT(block, I1, opcode&0xff, SIZE_BYTE);    // sext r1, opcode&0xff, BYTE
		UML_CMP(block, I1, R32(0));         // cmp r1, R0
		UML_JMPc(block, COND_NZ, compiler->labelnum);   // jnz compiler->labelnum   (if negative)

		UML_OR(block, I0, I0, T);   // or r0, r0, T

		UML_LABEL(block, compiler->labelnum++);         // labelnum:
		UML_MOV(block, mem(&m_sh2_state->sr), I0);      // mov m_sh2_state->sr, r0
		return true;

	case  9<< 8: // BT(opcode & 0xff);
		UML_TEST(block, mem(&m_sh2_state->sr), T);      // test m_sh2_state->sr, T
		UML_JMPc(block, COND_Z, compiler->labelnum);    // jz compiler->labelnum

		disp = ((int32_t)opcode << 24) >> 24;
		m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;    // m_sh2_state->ea = destination

		generate_update_cycles(block, compiler, m_sh2_state->ea, true);    // <subtract cycles>
		UML_HASHJMP(block, 0, m_sh2_state->ea, *m_nocode);   // jmp m_sh2_state->ea

		UML_LABEL(block, compiler->labelnum++);         // labelnum:
		return true;

	case 11<< 8: // BF(opcode & 0xff);
		UML_TEST(block, mem(&m_sh2_state->sr), T);      // test m_sh2_state->sr, T
		UML_JMPc(block, COND_NZ, compiler->labelnum);   // jnz compiler->labelnum

		disp = ((int32_t)opcode << 24) >> 24;
		m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;        // m_sh2_state->ea = destination

		generate_update_cycles(block, compiler, m_sh2_state->ea, true);    // <subtract cycles>
		UML_HASHJMP(block, 0, m_sh2_state->ea, *m_nocode);   // jmp m_sh2_state->ea

		UML_LABEL(block, compiler->labelnum++);         // labelnum:
		return true;

	case 13<< 8: // BTS(opcode & 0xff);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			UML_TEST(block, mem(&m_sh2_state->sr), T);      // test m_sh2_state->sr, T
			UML_JMPc(block, COND_Z, compiler->labelnum);    // jz compiler->labelnum

			disp = ((int32_t)opcode << 24) >> 24;
			m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;        // m_sh2_state->ea = destination

			templabel = compiler->labelnum;         // save our label
			compiler->labelnum++;               // make sure the delay slot doesn't use it
			generate_delay_slot(block, compiler, desc, m_sh2_state->ea-2);

			generate_update_cycles(block, compiler, m_sh2_state->ea, true);    // <subtract cycles>
			UML_HASHJMP(block, 0, m_sh2_state->ea, *m_nocode);   // jmp m_sh2_state->ea

			UML_LABEL(block, templabel);            // labelnum:
			return true;
		}
		break;

	case 15<< 8: // BFS(opcode & 0xff);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			UML_TEST(block, mem(&m_sh2_state->sr), T);      // test m_sh2_state->sr, T
			UML_JMPc(block, COND_NZ, compiler->labelnum);   // jnz compiler->labelnum

			disp = ((int32_t)opcode << 24) >> 24;
			m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;        // m_sh2_state->ea = destination

			templabel = compiler->labelnum;         // save our label
			compiler->labelnum++;               // make sure the delay slot doesn't use it
			generate_delay_slot(block, compiler, desc, m_sh2_state->ea-2); // delay slot only if the branch is taken

			generate_update_cycles(block, compiler, m_sh2_state->ea, true);    // <subtract cycles>
			UML_HASHJMP(block, 0, m_sh2_state->ea, *m_nocode);   // jmp m_sh2_state->ea

			UML_LABEL(block, templabel);            // labelnum:
			return true;
		}
		break;
	}

	return false;
}

bool sh_common_execution::generate_group_12(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	uint32_t scratch;

	switch (opcode & (15<<8))
	{
	case  0<<8: // MOVBSG(opcode & 0xff);
		scratch = (opcode & 0xff);
		UML_ADD(block, I0, mem(&m_sh2_state->gbr), scratch);    // add r0, gbr, scratch
		UML_AND(block, I1, R32(0), 0xff);       // and r1, R0, 0xff
		UML_CALLH(block, *m_write8);             // call write8

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  1<<8: // MOVWSG(opcode & 0xff);
		scratch = (opcode & 0xff) * 2;
		UML_ADD(block, I0, mem(&m_sh2_state->gbr), scratch);    // add r0, gbr, scratch
		UML_AND(block, I1, R32(0), 0xffff);     // and r1, R0, 0xffff
		UML_CALLH(block, *m_write16);                // call write16

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  2<<8: // MOVLSG(opcode & 0xff);
		scratch = (opcode & 0xff) * 4;
		UML_ADD(block, I0, mem(&m_sh2_state->gbr), scratch);    // add r0, gbr, scratch
		UML_MOV(block, I1, R32(0));         // mov r1, R0
		UML_CALLH(block, *m_write32);                // call write32

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  3<<8: // TRAPA(opcode & 0xff);
		scratch = (opcode & 0xff) * 4;
		UML_ADD(block, mem(&m_sh2_state->ea), mem(&m_sh2_state->vbr), scratch); // add ea, vbr, scratch

		UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
		UML_MOV(block, I0, R32(15));                // mov r0, R15
		UML_MOV(block, I1, mem(&m_sh2_state->sr));              // mov r1, sr
		UML_CALLH(block, *m_write32);                    // write32

		UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
		UML_MOV(block, I0, R32(15));                // mov r0, R15
		UML_MOV(block, I1, desc->pc+2);             // mov r1, pc+2
		UML_CALLH(block, *m_write32);                    // write32

		UML_MOV(block, I0, mem(&m_sh2_state->ea));              // mov r0, ea
		UML_CALLH(block, *m_read32);                 // read32
		UML_HASHJMP(block, 0, I0, *m_nocode);        // jmp (r0)

		return true;

	case  4<<8: // MOVBLG(opcode & 0xff);
		scratch = (opcode & 0xff);
		UML_ADD(block, I0, mem(&m_sh2_state->gbr), scratch);    // add r0, gbr, scratch
		UML_CALLH(block, *m_read8);              // call read16
		UML_SEXT(block, R32(0), I0, SIZE_BYTE);         // sext R0, r0, BYTE

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  5<<8: // MOVWLG(opcode & 0xff);
		scratch = (opcode & 0xff) * 2;
		UML_ADD(block, I0, mem(&m_sh2_state->gbr), scratch);    // add r0, gbr, scratch
		UML_CALLH(block, *m_read16);             // call read16
		UML_SEXT(block, R32(0), I0, SIZE_WORD);         // sext R0, r0, WORD

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  6<<8: // MOVLLG(opcode & 0xff);
		scratch = (opcode & 0xff) * 4;
		UML_ADD(block, I0, mem(&m_sh2_state->gbr), scratch);    // add r0, gbr, scratch
		UML_CALLH(block, *m_read32);             // call read32
		UML_MOV(block, R32(0), I0);         // mov R0, r0

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case  7<<8: // MOVA(opcode & 0xff);
		scratch = (opcode & 0xff) * 4;
		scratch += ((desc->pc + 4) & ~3);

		UML_MOV(block, R32(0), scratch);            // mov R0, scratch
		return true;

	case  8<<8: // TSTI(opcode & 0xff);
		scratch = opcode & 0xff;

		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~T);   // and sr, sr, ~T (clear the T bit)
		UML_AND(block, I0, R32(0), scratch);        // and r0, R0, scratch
		UML_CMP(block, I0, 0);          // cmp r0, #0
		UML_JMPc(block, COND_NZ, compiler->labelnum);       // jnz labelnum

		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), T); // or sr, sr, T

		UML_LABEL(block, compiler->labelnum++);         // labelnum:
		return true;

	case  9<<8: // ANDI(opcode & 0xff);
		UML_AND(block, R32(0), R32(0), opcode & 0xff);  // and r0, r0, opcode & 0xff
		return true;

	case 10<<8: // XORI(opcode & 0xff);
		UML_XOR(block, R32(0), R32(0), opcode & 0xff);  // xor r0, r0, opcode & 0xff
		return true;

	case 11<<8: // ORI(opcode & 0xff);
		UML_OR(block, R32(0), R32(0), opcode & 0xff);   // or r0, r0, opcode & 0xff
		return true;

	case 12<<8: // TSTM(opcode & 0xff);
		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~T);   // and sr, sr, ~T (clear the T bit)
		UML_ADD(block, I0, R32(0), mem(&m_sh2_state->gbr)); // add r0, R0, gbr
		UML_CALLH(block, *m_read8);              // read8

		UML_AND(block, I0, I0, opcode & 0xff);
		UML_CMP(block, I0, 0);          // cmp r0, #0
		UML_JMPc(block, COND_NZ, compiler->labelnum);       // jnz labelnum

		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), T); // or sr, sr, T

		UML_LABEL(block, compiler->labelnum++);         // labelnum:
		return true;

	case 13<<8: // ANDM(opcode & 0xff);
		UML_ADD(block, I0, R32(0), mem(&m_sh2_state->gbr)); // add r0, R0, gbr
		UML_CALLH(block, *m_read8);              // read8

		UML_AND(block, I1, I0, opcode&0xff);    // and r1, r0, #opcode&0xff
		UML_ADD(block, I0, R32(0), mem(&m_sh2_state->gbr)); // add r0, R0, gbr
		SETEA(0);
		UML_CALLH(block, *m_write8);             // write8
		return true;

	case 14<<8: // XORM(opcode & 0xff);
		UML_ADD(block, I0, R32(0), mem(&m_sh2_state->gbr)); // add r0, R0, gbr
		UML_CALLH(block, *m_read8);              // read8

		UML_XOR(block, I1, I0, opcode&0xff);    // xor r1, r0, #opcode&0xff
		UML_ADD(block, I0, R32(0), mem(&m_sh2_state->gbr)); // add r0, R0, gbr
		SETEA(0);
		UML_CALLH(block, *m_write8);             // write8
		return true;

	case 15<<8: // ORM(opcode & 0xff);
		UML_ADD(block, I0, R32(0), mem(&m_sh2_state->gbr)); // add r0, R0, gbr
		UML_CALLH(block, *m_read8);              // read8

		UML_OR(block, I1, I0, opcode&0xff); // or r1, r0, #opcode&0xff
		UML_ADD(block, I0, R32(0), mem(&m_sh2_state->gbr)); // add r0, R0, gbr
		SETEA(0);
		UML_CALLH(block, *m_write8);             // write8
		return true;
	}

	return false;
}



#undef Rn
#undef Rm

#undef T
#undef S
#undef I
#undef Q
#undef M

