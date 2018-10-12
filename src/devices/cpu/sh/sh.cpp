// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "sh.h"
#include "sh_dasm.h"
#include "cpu/drcumlsh.h"

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

	set_icountptr(m_sh2_state->icount);

	m_program = &space(AS_PROGRAM);
}


void sh_common_execution::drc_start()
{
	/* DRC helpers */
	memset(m_pcflushes, 0, sizeof(m_pcflushes));

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
	m_sh2_state->r[n] = tmp1 + (m_sh2_state->sr & SH_T);
	if (tmp0 > tmp1)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
	if (tmp1 > m_sh2_state->r[n])
		m_sh2_state->sr |= SH_T;
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
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
	else
		m_sh2_state->sr &= ~SH_T;
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
	if ((m_sh2_state->sr & SH_T) == 0)
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
	if ((m_sh2_state->sr & SH_T) == 0)
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
	if ((m_sh2_state->sr & SH_T) != 0)
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
	if ((m_sh2_state->sr & SH_T) != 0)
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
	m_sh2_state->sr &= ~SH_T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0000  1       comparison result
 *  CMP_EQ  Rm,Rn
 */
void sh_common_execution::CMPEQ(uint32_t m, uint32_t n)
{
	if (m_sh2_state->r[n] == m_sh2_state->r[m])
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0011  1       comparison result
 *  CMP_GE  Rm,Rn
 */
void sh_common_execution::CMPGE(uint32_t m, uint32_t n)
{
	if ((int32_t) m_sh2_state->r[n] >= (int32_t) m_sh2_state->r[m])
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0111  1       comparison result
 *  CMP_GT  Rm,Rn
 */
void sh_common_execution::CMPGT(uint32_t m, uint32_t n)
{
	if ((int32_t) m_sh2_state->r[n] > (int32_t) m_sh2_state->r[m])
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0110  1       comparison result
 *  CMP_HI  Rm,Rn
 */
void sh_common_execution::CMPHI(uint32_t m, uint32_t n)
{
	if ((uint32_t) m_sh2_state->r[n] > (uint32_t) m_sh2_state->r[m])
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0010  1       comparison result
 *  CMP_HS  Rm,Rn
 */
void sh_common_execution::CMPHS(uint32_t m, uint32_t n)
{
	if ((uint32_t) m_sh2_state->r[n] >= (uint32_t) m_sh2_state->r[m])
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0101  1       comparison result
 *  CMP_PL  Rn
 */
void sh_common_execution::CMPPL(uint32_t n)
{
	if ((int32_t) m_sh2_state->r[n] > 0)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0001  1       comparison result
 *  CMP_PZ  Rn
 */
void sh_common_execution::CMPPZ(uint32_t n)
{
	if ((int32_t) m_sh2_state->r[n] >= 0)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
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
		m_sh2_state->sr &= ~SH_T;
	else
		m_sh2_state->sr |= SH_T;
}

/*  code                 cycles  t-bit
 *  1000 1000 iiii iiii  1       comparison result
 *  CMP/EQ #imm,R0
 */
void sh_common_execution::CMPIM(uint32_t i)
{
	uint32_t imm = (uint32_t)(int32_t)(int16_t)(int8_t)i;

	if (m_sh2_state->r[0] == imm)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 0111  1       calculation result
 *  DIV0S   Rm,Rn
 */
void sh_common_execution::DIV0S(uint32_t m, uint32_t n)
{
	if ((m_sh2_state->r[n] & 0x80000000) == 0)
		m_sh2_state->sr &= ~SH_Q;
	else
		m_sh2_state->sr |= SH_Q;
	if ((m_sh2_state->r[m] & 0x80000000) == 0)
		m_sh2_state->sr &= ~SH_M;
	else
		m_sh2_state->sr |= SH_M;
	if ((m_sh2_state->r[m] ^ m_sh2_state->r[n]) & 0x80000000)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  code                 cycles  t-bit
 *  0000 0000 0001 1001  1       0
 *  DIV0U
 */
void sh_common_execution::DIV0U()
{
	m_sh2_state->sr &= ~(SH_M | SH_Q | SH_T);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0100  1       calculation result
 *  DIV1 Rm,Rn
 */
void sh_common_execution::DIV1(uint32_t m, uint32_t n)
{
	uint32_t tmp0;
	uint32_t old_q;

	old_q = m_sh2_state->sr & SH_Q;
	if (0x80000000 & m_sh2_state->r[n])
		m_sh2_state->sr |= SH_Q;
	else
		m_sh2_state->sr &= ~SH_Q;

	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->sr & SH_T);

	if (!old_q)
	{
		if (!(m_sh2_state->sr & SH_M))
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] -= m_sh2_state->r[m];
			if(!(m_sh2_state->sr & SH_Q))
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr |= SH_Q;
				else
					m_sh2_state->sr &= ~SH_Q;
			else
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr &= ~SH_Q;
				else
					m_sh2_state->sr |= SH_Q;
		}
		else
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] += m_sh2_state->r[m];
			if(!(m_sh2_state->sr & SH_Q))
			{
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr &= ~SH_Q;
				else
					m_sh2_state->sr |= SH_Q;
			}
			else
			{
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr |= SH_Q;
				else
					m_sh2_state->sr &= ~SH_Q;
			}
		}
	}
	else
	{
		if (!(m_sh2_state->sr & SH_M))
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] += m_sh2_state->r[m];
			if(!(m_sh2_state->sr & SH_Q))
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr |= SH_Q;
				else
					m_sh2_state->sr &= ~SH_Q;
			else
				if(m_sh2_state->r[n] < tmp0)
					m_sh2_state->sr &= ~SH_Q;
				else
					m_sh2_state->sr |= SH_Q;
		}
		else
		{
			tmp0 = m_sh2_state->r[n];
			m_sh2_state->r[n] -= m_sh2_state->r[m];
			if(!(m_sh2_state->sr & SH_Q))
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr &= ~SH_Q;
				else
					m_sh2_state->sr |= SH_Q;
			else
				if(m_sh2_state->r[n] > tmp0)
					m_sh2_state->sr |= SH_Q;
				else
					m_sh2_state->sr &= ~SH_Q;
		}
	}

	tmp0 = (m_sh2_state->sr & (SH_Q | SH_M));
	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
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
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
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
	//m_sh2_state->icount--; // not in SH4 implementation?
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
	if (m_sh2_state->sr & SH_S)
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
	if (m_sh2_state->sr & SH_S)
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
	m_sh2_state->r[n] = m_sh2_state->sr & SH_T;
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
	m_sh2_state->r[n] = -temp - (m_sh2_state->sr & SH_T);
	if (temp || (m_sh2_state->sr & SH_T))
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
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
	m_sh2_state->icount -= 2; // not in SH2 implementation?
}

/*  OR.B    #imm,@(R0,GBR) */
void sh_common_execution::ORM(uint32_t i)
{
	uint32_t temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = RB( m_sh2_state->ea );
	temp |= i;
	WB( m_sh2_state->ea, temp );
	//m_sh2_state->icount -= 2; // not in SH4 implementation?
}

/*  ROTCL   Rn */
void sh_common_execution::ROTCL(uint32_t n)
{
	uint32_t temp;

	temp = (m_sh2_state->r[n] >> 31) & SH_T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->sr & SH_T);
	m_sh2_state->sr = (m_sh2_state->sr & ~SH_T) | temp;
}

/*  ROTCR   Rn */
void sh_common_execution::ROTCR(uint32_t n)
{
	uint32_t temp;
	temp = (m_sh2_state->sr & SH_T) << 31;
	if (m_sh2_state->r[n] & SH_T)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 1) | temp;
}

/*  ROTL    Rn */
void sh_common_execution::ROTL(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~SH_T) | ((m_sh2_state->r[n] >> 31) & SH_T);
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->r[n] >> 31);
}

/*  ROTR    Rn */
void sh_common_execution::ROTR(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~SH_T) | (m_sh2_state->r[n] & SH_T);
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
	m_sh2_state->sr |= SH_T;
}

/*  SHAL    Rn      (same as SHLL) */
void sh_common_execution::SHAL(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~SH_T) | ((m_sh2_state->r[n] >> 31) & SH_T);
	m_sh2_state->r[n] <<= 1;
}

/*  SHAR    Rn */
void sh_common_execution::SHAR(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~SH_T) | (m_sh2_state->r[n] & SH_T);
	m_sh2_state->r[n] = (uint32_t)((int32_t)m_sh2_state->r[n] >> 1);
}

/*  SHLL    Rn      (same as SHAL) */
void sh_common_execution::SHLL(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~SH_T) | ((m_sh2_state->r[n] >> 31) & SH_T);
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
	m_sh2_state->sr = (m_sh2_state->sr & ~SH_T) | (m_sh2_state->r[n] & SH_T);
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
	m_sh2_state->r[n] = tmp1 - (m_sh2_state->sr & SH_T);
	if (tmp0 < tmp1)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
	if (tmp1 < m_sh2_state->r[n])
		m_sh2_state->sr |= SH_T;
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
			m_sh2_state->sr |= SH_T;
		else
			m_sh2_state->sr &= ~SH_T;
	}
	else
		m_sh2_state->sr &= ~SH_T;
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
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
	temp |= 0x80;
	/* Bus Lock disable */
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 3;
}

/*  TST     Rm,Rn */
void sh_common_execution::TST(uint32_t m, uint32_t n)
{
	if ((m_sh2_state->r[n] & m_sh2_state->r[m]) == 0)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  TST     #imm,R0 */
void sh_common_execution::TSTI(uint32_t i)
{
	uint32_t imm = i & 0xff;

	if ((imm & m_sh2_state->r[0]) == 0)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
}

/*  TST.B   #imm,@(R0,GBR) */
void sh_common_execution::TSTM(uint32_t i)
{
	uint32_t imm = i & 0xff;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	if ((imm & RB( m_sh2_state->ea )) == 0)
		m_sh2_state->sr |= SH_T;
	else
		m_sh2_state->sr &= ~SH_T;
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

// SH4 cases fall through to here too
void sh_common_execution::execute_one_0000(uint16_t opcode)
{
	// 04,05,06,07 always the same, 0c,0d,0e,0f always the same, other change based on upper bits

	switch (opcode & 0x3F)
	{
	case 0x00: ILLEGAL();                       break;
	case 0x01: ILLEGAL();                       break;
	case 0x02: STCSR(Rn);                  break;
	case 0x03: BSRF(Rn);                   break;
	case 0x04: MOVBS0(Rm, Rn);             break;
	case 0x05: MOVWS0(Rm, Rn);             break;
	case 0x06: MOVLS0(Rm, Rn);             break;
	case 0x07: MULL(Rm, Rn);               break;
	case 0x08: CLRT();                       break;
	case 0x09: NOP();                           break;
	case 0x0a: STSMACH(Rn);                break;
	case 0x0b: RTS();                        break;
	case 0x0c: MOVBL0(Rm, Rn);             break;
	case 0x0d: MOVWL0(Rm, Rn);             break;
	case 0x0e: MOVLL0(Rm, Rn);             break;
	case 0x0f: MAC_L(Rm, Rn);              break;

	case 0x10: ILLEGAL();                       break;
	case 0x11: ILLEGAL();                       break;
	case 0x12: STCGBR(Rn);                 break;
	case 0x13: ILLEGAL();                       break;
	case 0x14: MOVBS0(Rm, Rn);             break;
	case 0x15: MOVWS0(Rm, Rn);             break;
	case 0x16: MOVLS0(Rm, Rn);             break;
	case 0x17: MULL(Rm, Rn);               break;
	case 0x18: SETT();                       break;
	case 0x19: DIV0U();                  break;
	case 0x1a: STSMACL(Rn);                break;
	case 0x1b: SLEEP();                  break;
	case 0x1c: MOVBL0(Rm, Rn);             break;
	case 0x1d: MOVWL0(Rm, Rn);             break;
	case 0x1e: MOVLL0(Rm, Rn);             break;
	case 0x1f: MAC_L(Rm, Rn);              break;

	case 0x20: ILLEGAL();                       break;
	case 0x21: ILLEGAL();                       break;
	case 0x22: STCVBR(Rn);                 break;
	case 0x23: BRAF(Rn);                   break;
	case 0x24: MOVBS0(Rm, Rn);             break;
	case 0x25: MOVWS0(Rm, Rn);             break;
	case 0x26: MOVLS0(Rm, Rn);             break;
	case 0x27: MULL(Rm, Rn);               break;
	case 0x28: CLRMAC();                 break;
	case 0x29: MOVT(Rn);                   break;
	case 0x2a: STSPR(Rn);                  break;
	case 0x2b: RTE();                        break;
	case 0x2c: MOVBL0(Rm, Rn);             break;
	case 0x2d: MOVWL0(Rm, Rn);             break;
	case 0x2e: MOVLL0(Rm, Rn);             break;
	case 0x2f: MAC_L(Rm, Rn);              break;

	case 0x30: ILLEGAL();                       break;
	case 0x31: ILLEGAL();                       break;
	case 0x32: ILLEGAL();                       break;
	case 0x33: ILLEGAL();                       break;
	case 0x34: MOVBS0(Rm, Rn);             break;
	case 0x35: MOVWS0(Rm, Rn);             break;
	case 0x36: MOVLS0(Rm, Rn);             break;
	case 0x37: MULL(Rm, Rn);               break;
	case 0x38: ILLEGAL();                       break;
	case 0x39: ILLEGAL();                       break;
	case 0x3a: ILLEGAL();                       break;
	case 0x3b: ILLEGAL();                       break;
	case 0x3c: MOVBL0(Rm, Rn);             break;
	case 0x3d: MOVWL0(Rm, Rn);             break;
	case 0x3e: MOVLL0(Rm, Rn);             break;
	case 0x3f: MAC_L(Rm, Rn);              break;
	}
}

// SH4 cases fall through to here too
void sh_common_execution::execute_one_4000(uint16_t opcode)
{
	// 0f always the same, others differ

	switch (opcode & 0x3F)
	{
	case 0x00: SHLL(Rn);                   break;
	case 0x01: SHLR(Rn);                   break;
	case 0x02: STSMMACH(Rn);               break;
	case 0x03: STCMSR(Rn);                 break;
	case 0x04: ROTL(Rn);                   break;
	case 0x05: ROTR(Rn);                   break;
	case 0x06: LDSMMACH(Rn);               break;
	case 0x07: LDCMSR(opcode);                 break;
	case 0x08: SHLL2(Rn);                  break;
	case 0x09: SHLR2(Rn);                  break;
	case 0x0a: LDSMACH(Rn);                break;
	case 0x0b: JSR(Rn);                    break;
	case 0x0c: ILLEGAL();                       break;
	case 0x0d: ILLEGAL();                       break;
	case 0x0e: LDCSR(opcode);                  break;
	case 0x0f: MAC_W(Rm, Rn);              break;

	case 0x10: DT(Rn);                     break;
	case 0x11: CMPPZ(Rn);                  break;
	case 0x12: STSMMACL(Rn);               break;
	case 0x13: STCMGBR(Rn);                break;
	case 0x14: ILLEGAL();                       break;
	case 0x15: CMPPL(Rn);                  break;
	case 0x16: LDSMMACL(Rn);               break;
	case 0x17: LDCMGBR(Rn);                break;
	case 0x18: SHLL8(Rn);                  break;
	case 0x19: SHLR8(Rn);                  break;
	case 0x1a: LDSMACL(Rn);                break;
	case 0x1b: TAS(Rn);                    break;
	case 0x1c: ILLEGAL();                       break;
	case 0x1d: ILLEGAL();                       break;
	case 0x1e: LDCGBR(Rn);                 break;
	case 0x1f: MAC_W(Rm, Rn);              break;

	case 0x20: SHAL(Rn);                   break;
	case 0x21: SHAR(Rn);                   break;
	case 0x22: STSMPR(Rn);                 break;
	case 0x23: STCMVBR(Rn);                break;
	case 0x24: ROTCL(Rn);                  break;
	case 0x25: ROTCR(Rn);                  break;
	case 0x26: LDSMPR(Rn);                 break;
	case 0x27: LDCMVBR(Rn);                break;
	case 0x28: SHLL16(Rn);                 break;
	case 0x29: SHLR16(Rn);                 break;
	case 0x2a: LDSPR(Rn);                  break;
	case 0x2b: JMP(Rn);                    break;
	case 0x2c: ILLEGAL();                       break;
	case 0x2d: ILLEGAL();                       break;
	case 0x2e: LDCVBR(Rn);                 break;
	case 0x2f: MAC_W(Rm, Rn);              break;

	case 0x30: ILLEGAL();                       break;
	case 0x31: ILLEGAL();                       break;
	case 0x32: ILLEGAL();                       break;
	case 0x33: ILLEGAL();                       break;
	case 0x34: ILLEGAL();                       break;
	case 0x35: ILLEGAL();                       break;
	case 0x36: ILLEGAL();                       break;
	case 0x37: ILLEGAL();                       break;
	case 0x38: ILLEGAL();                       break;
	case 0x39: ILLEGAL();                       break;
	case 0x3a: ILLEGAL();                       break;
	case 0x3b: ILLEGAL();                       break;
	case 0x3c: ILLEGAL();                       break;
	case 0x3d: ILLEGAL();                       break;
	case 0x3e: ILLEGAL();                       break;
	case 0x3f: MAC_W(Rm, Rn);              break;

	}
}

void sh_common_execution::execute_one(const uint16_t opcode)
{
	switch(opcode & 0xf000)
	{
		case 0x0000: execute_one_0000(opcode); break;
		case 0x1000: MOVLS4(Rm, opcode & 0x0f, Rn); break;
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
void cfunc_unimplemented(void *param) { ((sh_common_execution *)param)->func_unimplemented(); }
void cfunc_MAC_W(void *param) { ((sh_common_execution *)param)->func_MAC_W(); }
void cfunc_MAC_L(void *param) { ((sh_common_execution *)param)->func_MAC_L(); }
void cfunc_DIV1(void *param) { ((sh_common_execution *)param)->func_DIV1(); }
void cfunc_ADDV(void *param) { ((sh_common_execution *)param)->func_ADDV(); }
void cfunc_SUBV(void *param) { ((sh_common_execution *)param)->func_SUBV(); }
void cfunc_printf_probe(void *param) { ((sh_common_execution *)param)->func_printf_probe(); }

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

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    epc - compute the exception PC from a
    descriptor
-------------------------------------------------*/

uint32_t sh_common_execution::epc(const opcode_desc *desc)
{
	return (desc->flags & OPFLAG_IN_DELAY_SLOT) ? (desc->pc - 1) : desc->pc;
}

/*-------------------------------------------------
    alloc_handle - allocate a handle if not
    already allocated
-------------------------------------------------*/

void sh_common_execution::alloc_handle(uml::code_handle *&handleptr, const char *name)
{
	if (!handleptr)
		handleptr = m_drcuml->handle_alloc(name);
}

/*-------------------------------------------------
    load_fast_iregs - load any fast integer
    registers
-------------------------------------------------*/

void sh_common_execution::load_fast_iregs(drcuml_block &block)
{
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_regmap); regnum++)
	{
		if (m_regmap[regnum].is_int_register())
		{
			UML_MOV(block, uml::parameter::make_ireg(m_regmap[regnum].ireg()), mem(&m_sh2_state->r[regnum]));
		}
	}
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

void sh_common_execution::save_fast_iregs(drcuml_block &block)
{
	for (int regnum = 0; regnum < ARRAY_LENGTH(m_regmap); regnum++)
	{
		if (m_regmap[regnum].is_int_register())
		{
			UML_MOV(block, mem(&m_sh2_state->r[regnum]), uml::parameter::make_ireg(m_regmap[regnum].ireg()));
		}
	}
}


/*-------------------------------------------------
    log_desc_flags_to_string - generate a string
    representing the instruction description
    flags
-------------------------------------------------*/

const char *sh_common_execution::log_desc_flags_to_string(uint32_t flags)
{
	static char tempbuf[30];
	char *dest = tempbuf;

	/* branches */
	if (flags & OPFLAG_IS_UNCONDITIONAL_BRANCH)
		*dest++ = 'U';
	else if (flags & OPFLAG_IS_CONDITIONAL_BRANCH)
		*dest++ = 'C';
	else
		*dest++ = '.';

	/* intrablock branches */
	*dest++ = (flags & OPFLAG_INTRABLOCK_BRANCH) ? 'i' : '.';

	/* branch targets */
	*dest++ = (flags & OPFLAG_IS_BRANCH_TARGET) ? 'B' : '.';

	/* delay slots */
	*dest++ = (flags & OPFLAG_IN_DELAY_SLOT) ? 'D' : '.';

	/* exceptions */
	if (flags & OPFLAG_WILL_CAUSE_EXCEPTION)
		*dest++ = 'E';
	else if (flags & OPFLAG_CAN_CAUSE_EXCEPTION)
		*dest++ = 'e';
	else
		*dest++ = '.';

	/* read/write */
	if (flags & OPFLAG_READS_MEMORY)
		*dest++ = 'R';
	else if (flags & OPFLAG_WRITES_MEMORY)
		*dest++ = 'W';
	else
		*dest++ = '.';

	/* TLB validation */
	*dest++ = (flags & OPFLAG_VALIDATE_TLB) ? 'V' : '.';

	/* TLB modification */
	*dest++ = (flags & OPFLAG_MODIFIES_TRANSLATION) ? 'T' : '.';

	/* redispatch */
	*dest++ = (flags & OPFLAG_REDISPATCH) ? 'R' : '.';
	return tempbuf;
}


/*-------------------------------------------------
    log_register_list - log a list of GPR registers
-------------------------------------------------*/

void sh_common_execution::log_register_list(const char *string, const uint32_t *reglist, const uint32_t *regnostarlist)
{
	int count = 0;
	int regnum;

	/* skip if nothing */
	if (reglist[0] == 0 && reglist[1] == 0 && reglist[2] == 0)
		return;

	m_drcuml->log_printf("[%s:", string);

	for (regnum = 0; regnum < 16; regnum++)
	{
		if (reglist[0] & REGFLAG_R(regnum))
		{
			m_drcuml->log_printf("%sr%d", (count++ == 0) ? "" : ",", regnum);
			if (regnostarlist != nullptr && !(regnostarlist[0] & REGFLAG_R(regnum)))
				m_drcuml->log_printf("*");
		}
	}

	if (reglist[1] & REGFLAG_PR)
	{
		m_drcuml->log_printf("%spr", (count++ == 0) ? "" : ",");
		if (regnostarlist != nullptr && !(regnostarlist[1] & REGFLAG_PR))
			m_drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_SR)
	{
		m_drcuml->log_printf("%ssr", (count++ == 0) ? "" : ",");
		if (regnostarlist != nullptr && !(regnostarlist[1] & REGFLAG_SR))
			m_drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_MACL)
	{
		m_drcuml->log_printf("%smacl", (count++ == 0) ? "" : ",");
		if (regnostarlist != nullptr && !(regnostarlist[1] & REGFLAG_MACL))
			m_drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_MACH)
	{
		m_drcuml->log_printf("%smach", (count++ == 0) ? "" : ",");
		if (regnostarlist != nullptr && !(regnostarlist[1] & REGFLAG_MACH))
			m_drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_GBR)
	{
		m_drcuml->log_printf("%sgbr", (count++ == 0) ? "" : ",");
		if (regnostarlist != nullptr && !(regnostarlist[1] & REGFLAG_GBR))
			m_drcuml->log_printf("*");
	}

	if (reglist[1] & REGFLAG_VBR)
	{
		m_drcuml->log_printf("%svbr", (count++ == 0) ? "" : ",");
		if (regnostarlist != nullptr && !(regnostarlist[1] & REGFLAG_VBR))
			m_drcuml->log_printf("*");
	}

	m_drcuml->log_printf("] ");
}

/*-------------------------------------------------
    log_opcode_desc - log a list of descriptions
-------------------------------------------------*/

void sh_common_execution::log_opcode_desc(const opcode_desc *desclist, int indent)
{
	/* open the file, creating it if necessary */
	if (indent == 0)
		m_drcuml->log_printf("\nDescriptor list @ %08X\n", desclist->pc);

	/* output each descriptor */
	for ( ; desclist != nullptr; desclist = desclist->next())
	{
		std::ostringstream stream;

		/* disassemle the current instruction and output it to the log */
		if (m_drcuml->logging() || m_drcuml->logging_native())
		{
			if (desclist->flags & OPFLAG_VIRTUAL_NOOP)
				stream << "<virtual nop>";
			else
			{
				sh_disassembler sh2d(false);
				sh2d.dasm_one(stream, desclist->pc, desclist->opptr.w[0]);
			}
		}
		else
			stream << "???";
		m_drcuml->log_printf("%08X [%08X] t:%08X f:%s: %-30s", desclist->pc, desclist->physpc, desclist->targetpc, log_desc_flags_to_string(desclist->flags), stream.str().c_str());

		/* output register states */
		log_register_list("use", desclist->regin, nullptr);
		log_register_list("mod", desclist->regout, desclist->regreq);
		m_drcuml->log_printf("\n");

		/* if we have a delay slot, output it recursively */
		if (desclist->delay.first() != nullptr)
			log_opcode_desc(desclist->delay.first(), indent + 1);

		/* at the end of a sequence add a dividing line */
		if (desclist->flags & OPFLAG_END_SEQUENCE)
			m_drcuml->log_printf("-----\n");
	}
}

/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of an SH2 instruction
-------------------------------------------------*/

void sh_common_execution::log_add_disasm_comment(drcuml_block &block, uint32_t pc, uint32_t op)
{
	if (m_drcuml->logging())
	{
		sh_disassembler sh2d(false);
		std::ostringstream stream;
		sh2d.dasm_one(stream, pc, op);
		block.append_comment("%08X: %s", pc, stream.str().c_str());
	}
}


/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

void sh_common_execution::code_flush_cache()
{
	/* empty the transient cache contents */
	m_drcuml->reset();

	try
	{
		/* generate the entry point and out-of-cycles handlers */
		static_generate_nocode_handler();
		static_generate_out_of_cycles();
		static_generate_entry_point();

		/* add subroutines for memory accesses */
		static_generate_memory_accessor(1, false, "read8", m_read8);
		static_generate_memory_accessor(1, true,  "write8", m_write8);
		static_generate_memory_accessor(2, false, "read16", m_read16);
		static_generate_memory_accessor(2, true,  "write16", m_write16);
		static_generate_memory_accessor(4, false, "read32", m_read32);
		static_generate_memory_accessor(4, true,  "write32", m_write32);
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Unable to generate SH2 static code\n");
	}

	m_cache_dirty = false;
}

/* Execute cycles - returns number of cycles actually run */
void sh_common_execution::execute_run_drc()
{
	int execute_result;

	/* reset the cache if dirty */
	if (m_cache_dirty)
		code_flush_cache();

	/* execute */
	do
	{
		/* run as much as we can */
		execute_result = m_drcuml->execute(*m_entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
		{
			code_compile_block(0, m_sh2_state->pc);
		}
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
		{
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", m_sh2_state->pc);
		}
		else if (execute_result == EXECUTE_RESET_CACHE)
		{
			code_flush_cache();
		}
	} while (execute_result != EXECUTE_OUT_OF_CYCLES);
}


/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

void sh_common_execution::code_compile_block(uint8_t mode, offs_t pc)
{
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *desclist;
	bool override = false;

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	desclist = get_desclist(pc);

	if (m_drcuml->logging() || m_drcuml->logging_native())
		log_opcode_desc(desclist, 0);

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			drcuml_block &block(m_drcuml->begin_block(4096));

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != nullptr; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				uint32_t nextpc;

				/* add a code log entry */
				if (m_drcuml->logging())
					block.append_comment("-------------------------");                 // comment

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != nullptr; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != nullptr);

				/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
				if (override || !m_drcuml->hash_exists(mode, seqhead->pc))
					UML_HASH(block, mode, seqhead->pc);                                     // hash    mode,pc

				/* if we already have a hash, and this is the first sequence, assume that we */
				/* are recompiling due to being out of sync and allow future overrides */
				else if (seqhead == desclist)
				{
					override = true;
					UML_HASH(block, mode, seqhead->pc);                                     // hash    mode,pc
				}

				/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
				else
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc | 0x80000000
					UML_HASHJMP(block, 0, seqhead->pc, *m_nocode);
																							// hashjmp <mode>,seqhead->pc,nocode
					continue;
				}

				/* validate this code block if we're not pointing into ROM */
				if (m_program->get_write_ptr(seqhead->physpc) != nullptr)
					generate_checksum_block(block, compiler, seqhead, seqlast);

				/* label this instruction, if it may be jumped to locally */
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc | 0x80000000
				}

				/* iterate over instructions in the sequence and compile them */
				for (curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
				{
					generate_sequence_instruction(block, compiler, curdesc, 0xffffffff);
				}

				/* if we need to return to the start, do it */
				if (seqlast->flags & OPFLAG_RETURN_TO_START)
				{
					nextpc = pc;
				}
				/* otherwise we just go to the next instruction */
				else
				{
					nextpc = seqlast->pc + (seqlast->skipslots + 1) * 2;
				}

				/* count off cycles and go there */
				generate_update_cycles(block, compiler, nextpc, true);                // <subtract cycles>

				/* SH2 has no modes */
				if (seqlast->next() == nullptr || seqlast->next()->pc != nextpc)
				{
					UML_HASHJMP(block, 0, nextpc, *m_nocode);
				}
																							// hashjmp <mode>,nextpc,nocode
			}

			/* end the sequence */
			block.end();
			g_profiler.stop();
			succeeded = true;
		}
		catch (drcuml_block::abort_compilation &)
		{
			code_flush_cache();
		}
	}
}


/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

void sh_common_execution::static_generate_nocode_handler()
{
	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(10));

	/* generate a hash jump via the current mode and PC */
	alloc_handle(m_nocode, "nocode");
	UML_HANDLE(block, *m_nocode);                                    // handle  nocode
	UML_GETEXP(block, I0);                                  // getexp  i0
	UML_MOV(block, mem(&m_sh2_state->pc), I0);                              // mov     [pc],i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);                          // exit    EXECUTE_MISSING_CODE

	block.end();
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

void sh_common_execution::static_generate_out_of_cycles()
{
	/* begin generating */
	drcuml_block &block(m_drcuml->begin_block(10));

	/* generate a hash jump via the current mode and PC */
	alloc_handle(m_out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *m_out_of_cycles);                             // handle  out_of_cycles
	UML_GETEXP(block, I0);                                  // getexp  i0
	UML_MOV(block, mem(&m_sh2_state->pc), I0);                              // mov     <pc>,i0
	save_fast_iregs(block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);                         // exit    EXECUTE_OUT_OF_CYCLES

	block.end();
}

/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

void sh_common_execution::generate_checksum_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (m_drcuml->logging())
		block.append_comment("[Validation for %08X]", seqhead->pc);                // comment

	/* loose verify or single instruction: just compare and fail */
	if (!(m_drcoptions & SH2DRC_STRICT_VERIFY) || seqhead->next() == nullptr)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			const void *base = m_prptr(seqhead->physpc);

			UML_LOAD(block, I0, base, 0, SIZE_WORD, SCALE_x2);                          // load    i0,base,word
			UML_CMP(block, I0, seqhead->opptr.w[0]);                        // cmp     i0,*opptr
			UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));       // exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
		uint32_t sum = 0;
		const void *base = m_prptr(seqhead->physpc);

		UML_LOAD(block, I0, base, 0, SIZE_WORD, SCALE_x4);                              // load    i0,base,word
		sum += seqhead->opptr.w[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = m_prptr(curdesc->physpc);

				UML_LOAD(block, I1, base, 0, SIZE_WORD, SCALE_x2);                      // load    i1,*opptr,word
				UML_ADD(block, I0, I0, I1);                         // add     i0,i0,i1
				sum += curdesc->opptr.w[0];
			}
		UML_CMP(block, I0, sum);                                            // cmp     i0,sum
		UML_EXHc(block, COND_NE, *m_nocode, epc(seqhead));           // exne    nocode,seqhead->pc
	}
}



/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

void sh_common_execution::generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t ovrpc)
{
	offs_t expc;

	/* add an entry for the log */
	if (m_drcuml->logging() && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(block, desc->pc, desc->opptr.w[0]);

	/* set the PC map variable */
	expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 1 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, expc);                                             // mapvar  PC,expc

	/* accumulate total cycles */
	compiler.cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles);                             // mapvar  CYCLES,compiler.cycles

	/* if we want a probe, add it here */
	if (desc->pc == PROBE_ADDRESS)
	{
		UML_MOV(block, mem(&m_sh2_state->pc), desc->pc);                                // mov     [pc],desc->pc
		UML_CALLC(block, cfunc_printf_probe, this);                                  // callc   cfunc_printf_probe,sh2
	}

	/* if we are debugging, call the debugger */
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&m_sh2_state->pc), desc->pc);                                // mov     [pc],desc->pc
		save_fast_iregs(block);
		UML_DEBUG(block, desc->pc);                                         // debug   desc->pc
	}
	else    // not debug, see what other reasons there are for flushing the PC
	{
		if (m_drcoptions & SH2DRC_FLUSH_PC)  // always flush?
		{
			UML_MOV(block, mem(&m_sh2_state->pc), desc->pc);        // mov m_sh2_state->pc, desc->pc
		}
		else    // check for driver-selected flushes
		{
			int pcflush;

			for (pcflush = 0; pcflush < m_pcfsel; pcflush++)
			{
				if (desc->pc == m_pcflushes[pcflush])
				{
					UML_MOV(block, mem(&m_sh2_state->pc), desc->pc);        // mov m_sh2_state->pc, desc->pc
				}
			}
		}
	}


	/* if we hit an unmapped address, fatal error */
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, mem(&m_sh2_state->pc), desc->pc);                                // mov     [pc],desc->pc
		save_fast_iregs(block);
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);                             // exit    EXECUTE_UNMAPPED_CODE
	}

	/* if this is an invalid opcode, die */
	if (desc->flags & OPFLAG_INVALID_OPCODE)
	{
		fatalerror("SH2DRC: invalid opcode!\n");
	}

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	else if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(block, compiler, desc, ovrpc))
		{
			// handle an illegal op
			UML_MOV(block, mem(&m_sh2_state->pc), desc->pc);                            // mov     [pc],desc->pc
			UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);                  // mov     [arg0],opcode
			UML_CALLC(block, cfunc_unimplemented, this);                             // callc   cfunc_unimplemented
		}
	}
}

/*------------------------------------------------------------------
    generate_delay_slot
------------------------------------------------------------------*/

void sh_common_execution::generate_delay_slot(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t ovrpc)
{
	compiler_state compiler_temp(compiler);

	/* compile the delay slot using temporary compiler state */
	assert(desc->delay.first() != nullptr);
	generate_sequence_instruction(block, compiler_temp, desc->delay.first(), ovrpc);              // <next instruction>

	/* update the label */
	compiler.labelnum = compiler_temp.labelnum;
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
    generate_opcode - generate code for a specific
    opcode
-------------------------------------------------*/

bool sh_common_execution::generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint32_t ovrpc)
{
	uint32_t scratch, scratch2;
	int32_t disp;
	uint16_t opcode = desc->opptr.w[0];
	uint8_t opswitch = opcode >> 12;
	int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);

	//printf("generating %04x\n", opcode);

	switch (opswitch)
	{
		case  0:
			return generate_group_0(block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case  1:    // MOVLS4
			scratch = (opcode & 0x0f) * 4;
			UML_ADD(block, I0, R32(Rn), scratch);   // add r0, Rn, scratch
			UML_MOV(block, I1, R32(Rm));        // mov r1, Rm
			SETEA(0);                       // set ea for debug
			UML_CALLH(block, *m_write32);

			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 2, true);
			return true;

		case  2:
			return generate_group_2(block, compiler, desc, opcode, in_delay_slot, ovrpc);
		case  3:
			return generate_group_3(block, compiler, desc, opcode, ovrpc);
		case  4:
			return generate_group_4(block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case  5:    // MOVLL4
			scratch = (opcode & 0x0f) * 4;
			UML_ADD(block, I0, R32(Rm), scratch);       // add r0, Rm, scratch
			SETEA(0);                       // set ea for debug
			UML_CALLH(block, *m_read32);             // call read32
			UML_MOV(block, R32(Rn), I0);            // mov Rn, r0

			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 2, true);
			return true;

		case  6:
			return generate_group_6(block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case  7:    // ADDI
			scratch = opcode & 0xff;
			scratch2 = (uint32_t)(int32_t)(int16_t)(int8_t)scratch;
			UML_ADD(block, R32(Rn), R32(Rn), scratch2); // add Rn, Rn, scratch2
			return true;

		case  8:
			return generate_group_8(block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case  9:    // MOVWI
			if (ovrpc == 0xffffffff)
			{
				scratch = (desc->pc + 2) + ((opcode & 0xff) * 2) + 2;
			}
			else
			{
				scratch = (ovrpc + 2) + ((opcode & 0xff) * 2) + 2;
			}

			if (m_drcoptions & SH2DRC_STRICT_PCREL)
			{
				UML_MOV(block, I0, scratch);            // mov r0, scratch
				SETEA(0);                       // set ea for debug
				UML_CALLH(block, *m_read16);             // read16(r0, r1)
				UML_SEXT(block, R32(Rn), I0, SIZE_WORD);            // sext Rn, r0, WORD
			}
			else
			{
				scratch2 = (uint32_t)(int32_t)(int16_t) RW(scratch);
				UML_MOV(block, R32(Rn), scratch2);          // mov Rn, scratch2
			}

			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 2, true);
			return true;

		case 10:    // BRA
			disp = ((int32_t)opcode << 20) >> 20;
			m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;            // m_sh2_state->ea = pc+4 + disp*2 + 2

			generate_delay_slot(block, compiler, desc, m_sh2_state->ea-2);

			generate_update_cycles(block, compiler, m_sh2_state->ea, true);    // <subtract cycles>
			UML_HASHJMP(block, 0, m_sh2_state->ea, *m_nocode);   // hashjmp m_sh2_state->ea
			return true;

		case 11:    // BSR
			// panicstr @ 403da22 relies on the delay slot clobbering the PR set by a BSR, so
			// do this before running the delay slot
			UML_ADD(block, mem(&m_sh2_state->pr), desc->pc, 4); // add m_pr, desc->pc, #4 (skip the current insn & delay slot)

			disp = ((int32_t)opcode << 20) >> 20;
			m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;            // m_sh2_state->ea = pc+4 + disp*2 + 2

			generate_delay_slot(block, compiler, desc, m_sh2_state->ea-2);

			generate_update_cycles(block, compiler, m_sh2_state->ea, true);    // <subtract cycles>
			UML_HASHJMP(block, 0, m_sh2_state->ea, *m_nocode);   // hashjmp m_sh2_state->ea
			return true;

		case 12:
			return generate_group_12(block, compiler, desc, opcode, in_delay_slot, ovrpc);

		case 13:    // MOVLI
			if (ovrpc == 0xffffffff)
			{
				scratch = ((desc->pc + 4) & ~3) + ((opcode & 0xff) * 4);
			}
			else
			{
				scratch = ((ovrpc + 4) & ~3) + ((opcode & 0xff) * 4);
			}

			if (m_drcoptions & SH2DRC_STRICT_PCREL)
			{
				UML_MOV(block, I0, scratch);            // mov r0, scratch
				UML_CALLH(block, *m_read32);             // read32(r0, r1)
				UML_MOV(block, R32(Rn), I0);            // mov Rn, r0
			}
			else
			{
				scratch2 = RL(scratch);
				UML_MOV(block, R32(Rn), scratch2);          // mov Rn, scratch2
			}

			if (!in_delay_slot)
				generate_update_cycles(block, compiler, desc->pc + 2, true);
			return true;

		case 14:    // MOVI
			scratch = opcode & 0xff;
			scratch2 = (uint32_t)(int32_t)(int16_t)(int8_t)scratch;
			UML_MOV(block, R32(Rn), scratch2);
			return true;

		case 15:
			return generate_group_15(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	}

	return false;
}

bool sh_common_execution::generate_group_15(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	// no ops here on sh1/2
	return false;
}

bool sh_common_execution::generate_group_2(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
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
		UML_AND(block, I0, I0, ~(SH_Q|SH_M|SH_T));       // and r0, r0, ~(Q|M|T) (clear the Q,M, and T bits)

		UML_TEST(block, R32(Rn), 0x80000000);           // test Rn, #0x80000000
		UML_JMPc(block, COND_Z, compiler.labelnum);            // jz labelnum

		UML_OR(block, I0, I0, SH_Q);               // or r0, r0, Q
		UML_LABEL(block, compiler.labelnum++);             // labelnum:

		UML_TEST(block, R32(Rm), 0x80000000);           // test Rm, #0x80000000
		UML_JMPc(block, COND_Z, compiler.labelnum);            // jz labelnum

		UML_OR(block, I0, I0, SH_M);               // or r0, r0, M
		UML_LABEL(block, compiler.labelnum++);             // labelnum:

		UML_XOR(block, I1, R32(Rn), R32(Rm));           // xor r1, Rn, Rm
		UML_TEST(block, I1, 0x80000000);            // test r1, #0x80000000
		UML_JMPc(block, COND_Z, compiler.labelnum);            // jz labelnum

		UML_OR(block, I0, I0, SH_T);               // or r0, r0, T
		UML_LABEL(block, compiler.labelnum++);             // labelnum:
		UML_MOV(block, mem(&m_sh2_state->sr), I0);              // mov sr, r0
		return true;

	case  8: // TST(Rm, Rn);
		UML_AND(block, I0, mem(&m_sh2_state->sr), ~SH_T);  // and r0, sr, ~T (clear the T bit)
		UML_TEST(block, R32(Rm), R32(Rn));      // test Rm, Rn
		UML_JMPc(block, COND_NZ, compiler.labelnum);   // jnz compiler.labelnum

		UML_OR(block, I0, I0, SH_T);   // or r0, r0, T
		UML_LABEL(block, compiler.labelnum++);         // desc->pc:

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

		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~SH_T);   // and sr, sr, ~T (clear the T bit)

		UML_CMP(block, I1, 0);      // cmp r1, #0
		UML_JMPc(block, COND_Z, compiler.labelnum);    // jnz labelnum
		UML_CMP(block, I2, 0);      // cmp r2, #0
		UML_JMPc(block, COND_Z, compiler.labelnum);    // jnz labelnum
		UML_CMP(block, I3, 0);      // cmp r3, #0
		UML_JMPc(block, COND_Z, compiler.labelnum);    // jnz labelnum
		UML_CMP(block, I7, 0);      // cmp r7, #0
		UML_JMPc(block, COND_NZ, compiler.labelnum+1); // jnz labelnum

		UML_LABEL(block, compiler.labelnum++);     // labelnum:
		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), SH_T); // or sr, sr, T

		UML_LABEL(block, compiler.labelnum++);     // labelnum+1:
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


bool sh_common_execution::generate_group_3(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, uint32_t ovrpc)
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
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, SH_T); // rolins sr,i0,0,T
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
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, SH_T); // rolins sr,i0,0,T
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


bool sh_common_execution::generate_group_6(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
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
		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~SH_T);   // and sr, sr, ~T (clear the T bit)
		UML_CARRY(block, I0, 0);    // carry = T (T is bit 0 of SR)
		UML_SUBB(block, R32(Rn), 0, R32(Rm));   // subb Rn, #0, Rm

		UML_JMPc(block, COND_NC, compiler.labelnum);   // jnc labelnum

		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), SH_T); // or sr, sr, T

		UML_LABEL(block, compiler.labelnum++);     // labelnum:

		return true;
	}

	return false;
}

bool sh_common_execution::generate_group_8(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	int32_t disp;
	uint32_t udisp;
	uml::code_label templabel;

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
		UML_AND(block, I0, mem(&m_sh2_state->sr), ~SH_T);  // and r0, sr, ~T (clear the T bit)

		UML_SEXT(block, I1, opcode&0xff, SIZE_BYTE);    // sext r1, opcode&0xff, BYTE
		UML_CMP(block, I1, R32(0));         // cmp r1, R0
		UML_JMPc(block, COND_NZ, compiler.labelnum);   // jnz compiler.labelnum   (if negative)

		UML_OR(block, I0, I0, SH_T);   // or r0, r0, T

		UML_LABEL(block, compiler.labelnum++);         // labelnum:
		UML_MOV(block, mem(&m_sh2_state->sr), I0);      // mov m_sh2_state->sr, r0
		return true;

	case  9<< 8: // BT(opcode & 0xff);
		UML_TEST(block, mem(&m_sh2_state->sr), SH_T);      // test m_sh2_state->sr, T
		UML_JMPc(block, COND_Z, compiler.labelnum);    // jz compiler.labelnum

		disp = ((int32_t)opcode << 24) >> 24;
		m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;    // m_sh2_state->ea = destination

		generate_update_cycles(block, compiler, m_sh2_state->ea, true);    // <subtract cycles>
		UML_HASHJMP(block, 0, m_sh2_state->ea, *m_nocode);   // jmp m_sh2_state->ea

		UML_LABEL(block, compiler.labelnum++);         // labelnum:
		return true;

	case 11<< 8: // BF(opcode & 0xff);
		UML_TEST(block, mem(&m_sh2_state->sr), SH_T);      // test m_sh2_state->sr, T
		UML_JMPc(block, COND_NZ, compiler.labelnum);   // jnz compiler.labelnum

		disp = ((int32_t)opcode << 24) >> 24;
		m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;        // m_sh2_state->ea = destination

		generate_update_cycles(block, compiler, m_sh2_state->ea, true);    // <subtract cycles>
		UML_HASHJMP(block, 0, m_sh2_state->ea, *m_nocode);   // jmp m_sh2_state->ea

		UML_LABEL(block, compiler.labelnum++);         // labelnum:
		return true;

	case 13<< 8: // BTS(opcode & 0xff);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			UML_TEST(block, mem(&m_sh2_state->sr), SH_T);      // test m_sh2_state->sr, T
			UML_JMPc(block, COND_Z, compiler.labelnum);    // jz compiler.labelnum

			disp = ((int32_t)opcode << 24) >> 24;
			m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;        // m_sh2_state->ea = destination

			templabel = compiler.labelnum;         // save our label
			compiler.labelnum++;               // make sure the delay slot doesn't use it
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
			UML_TEST(block, mem(&m_sh2_state->sr), SH_T);      // test m_sh2_state->sr, T
			UML_JMPc(block, COND_NZ, compiler.labelnum);   // jnz compiler.labelnum

			disp = ((int32_t)opcode << 24) >> 24;
			m_sh2_state->ea = (desc->pc + 2) + disp * 2 + 2;        // m_sh2_state->ea = destination

			templabel = compiler.labelnum;         // save our label
			compiler.labelnum++;               // make sure the delay slot doesn't use it
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

bool sh_common_execution::generate_group_12_TRAPA(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	uint32_t scratch = (opcode & 0xff) * 4;
	UML_ADD(block, mem(&m_sh2_state->ea), mem(&m_sh2_state->vbr), scratch); // add ea, vbr, scratch

	UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
	UML_MOV(block, I0, R32(15));                // mov r0, R15
	UML_MOV(block, I1, mem(&m_sh2_state->sr));              // mov r1, sr
	UML_CALLH(block, *m_write32);                    // write32

	UML_SUB(block, R32(15), R32(15), 4);            // sub R15, R15, #4
	UML_MOV(block, I0, R32(15));                // mov r0, R15
	UML_MOV(block, I1, desc->pc + 2);             // mov r1, pc+2
	UML_CALLH(block, *m_write32);                    // write32

	UML_MOV(block, I0, mem(&m_sh2_state->ea));              // mov r0, ea
	UML_CALLH(block, *m_read32);                 // read32
	UML_HASHJMP(block, 0, I0, *m_nocode);        // jmp (r0)

	return true;
}

bool sh_common_execution::generate_group_12(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
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
		return generate_group_12_TRAPA(block, compiler, desc, opcode, in_delay_slot, ovrpc);

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

		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~SH_T);   // and sr, sr, ~T (clear the T bit)
		UML_AND(block, I0, R32(0), scratch);        // and r0, R0, scratch
		UML_CMP(block, I0, 0);          // cmp r0, #0
		UML_JMPc(block, COND_NZ, compiler.labelnum);       // jnz labelnum

		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), SH_T); // or sr, sr, T

		UML_LABEL(block, compiler.labelnum++);         // labelnum:
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
		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~SH_T);   // and sr, sr, ~T (clear the T bit)
		UML_ADD(block, I0, R32(0), mem(&m_sh2_state->gbr)); // add r0, R0, gbr
		UML_CALLH(block, *m_read8);              // read8

		UML_AND(block, I0, I0, opcode & 0xff);
		UML_CMP(block, I0, 0);          // cmp r0, #0
		UML_JMPc(block, COND_NZ, compiler.labelnum);       // jnz labelnum

		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), SH_T); // or sr, sr, T

		UML_LABEL(block, compiler.labelnum++);         // labelnum:
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

bool sh_common_execution::generate_group_0_RTE(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	generate_delay_slot(block, compiler, desc, 0xffffffff);

	UML_MOV(block, I0, R32(15));            // mov r0, R15
	UML_CALLH(block, *m_read32);             // call read32
	UML_MOV(block, mem(&m_sh2_state->pc), I0);          // mov pc, r0
	UML_ADD(block, R32(15), R32(15), 4);        // add R15, R15, #4

	UML_MOV(block, I0, R32(15));            // mov r0, R15
	UML_CALLH(block, *m_read32);             // call read32
	UML_MOV(block, mem(&m_sh2_state->sr), I0);          // mov sr, r0
	UML_ADD(block, R32(15), R32(15), 4);        // add R15, R15, #4

	compiler.checkints = true;
	UML_MOV(block, mem(&m_sh2_state->ea), mem(&m_sh2_state->pc));       // mov ea, pc
	generate_update_cycles(block, compiler, uml::mem(&m_sh2_state->ea), true);  // <subtract cycles>
	UML_HASHJMP(block, 0, mem(&m_sh2_state->pc), *m_nocode); // and jump to the "resume PC"

	return true;
}


bool sh_common_execution::generate_group_0(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 0x3F)
	{
	case 0x00:  // these are all illegal
	case 0x01:
	case 0x10:
	case 0x11:
	case 0x13:
	case 0x20:
	case 0x21:
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
		return false;

	case 0x09: // NOP();
		return true;

	case 0x02: // STCSR(Rn);
		UML_MOV(block, R32(Rn), mem(&m_sh2_state->sr));
		return true;

	case 0x03: // BSRF(Rn);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			UML_ADD(block, mem(&m_sh2_state->target), R32(Rn), 4);  // add target, Rm, #4
			UML_ADD(block, mem(&m_sh2_state->target), mem(&m_sh2_state->target), desc->pc); // add target, target, pc

			// 32x Cosmic Carnage @ 6002cb0 relies on the delay slot
			// clobbering the calculated PR, so do it first
			UML_ADD(block, mem(&m_sh2_state->pr), desc->pc, 4); // add m_pr, desc->pc, #4 (skip the current insn & delay slot)

			generate_delay_slot(block, compiler, desc, m_sh2_state->target);

			generate_update_cycles(block, compiler, uml::mem(&m_sh2_state->target), true);  // <subtract cycles>
			UML_HASHJMP(block, 0, mem(&m_sh2_state->target), *m_nocode); // jmp target
			return true;
		}
		break;

	case 0x04: // MOVBS0(Rm, Rn);
	case 0x14: // MOVBS0(Rm, Rn);
	case 0x24: // MOVBS0(Rm, Rn);
	case 0x34: // MOVBS0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rn));        // add r0, R0, Rn
		UML_AND(block, I1, R32(Rm), 0x000000ff);    // and r1, Rm, 0xff
		UML_CALLH(block, *m_write8);             // call write8

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x05: // MOVWS0(Rm, Rn);
	case 0x15: // MOVWS0(Rm, Rn);
	case 0x25: // MOVWS0(Rm, Rn);
	case 0x35: // MOVWS0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rn));        // add r0, R0, Rn
		UML_AND(block, I1, R32(Rm), 0x0000ffff);    // and r1, Rm, 0xffff
		UML_CALLH(block, *m_write16);                // call write16

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x06: // MOVLS0(Rm, Rn);
	case 0x16: // MOVLS0(Rm, Rn);
	case 0x26: // MOVLS0(Rm, Rn);
	case 0x36: // MOVLS0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rn));        // add r0, R0, Rn
		UML_MOV(block, I1, R32(Rm));            // mov r1, Rm
		UML_CALLH(block, *m_write32);                // call write32

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x07: // MULL(Rm, Rn);
	case 0x17: // MULL(Rm, Rn);
	case 0x27: // MULL(Rm, Rn);
	case 0x37: // MULL(Rm, Rn);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			UML_MULU(block, mem(&m_sh2_state->macl), mem(&m_sh2_state->ea), R32(Rn), R32(Rm));  // mulu macl, ea, Rn, Rm
			return true;
		}
		break;

	case 0x08: // CLRT();
		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~SH_T);   // and r0, sr, ~T (clear the T bit)
		return true;

	case 0x0a: // STSMACH(Rn);
		UML_MOV(block, R32(Rn), mem(&m_sh2_state->mach));       // mov Rn, mach
		return true;

	case 0x0b: // RTS();
		UML_MOV(block, mem(&m_sh2_state->target), mem(&m_sh2_state->pr));   // mov target, pr (in case of d-slot shenanigans)

		generate_delay_slot(block, compiler, desc, m_sh2_state->target);

		generate_update_cycles(block, compiler, uml::mem(&m_sh2_state->target), true);  // <subtract cycles>
		UML_HASHJMP(block, 0, mem(&m_sh2_state->target), *m_nocode);
		return true;

	case 0x0c: // MOVBL0(Rm, Rn);
	case 0x1c: // MOVBL0(Rm, Rn);
	case 0x2c: // MOVBL0(Rm, Rn);
	case 0x3c: // MOVBL0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rm));        // add r0, R0, Rm
		UML_CALLH(block, *m_read8);              // call read8
		UML_SEXT(block, R32(Rn), I0, SIZE_BYTE);        // sext Rn, r0, BYTE

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x0d: // MOVWL0(Rm, Rn);
	case 0x1d: // MOVWL0(Rm, Rn);
	case 0x2d: // MOVWL0(Rm, Rn);
	case 0x3d: // MOVWL0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rm));        // add r0, R0, Rm
		UML_CALLH(block, *m_read16);             // call read16
		UML_SEXT(block, R32(Rn), I0, SIZE_WORD);        // sext Rn, r0, WORD

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x0e: // MOVLL0(Rm, Rn);
	case 0x1e: // MOVLL0(Rm, Rn);
	case 0x2e: // MOVLL0(Rm, Rn);
	case 0x3e: // MOVLL0(Rm, Rn);
		UML_ADD(block, I0, R32(0), R32(Rm));        // add r0, R0, Rm
		UML_CALLH(block, *m_read32);             // call read32
		UML_MOV(block, R32(Rn), I0);            // mov Rn, r0

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x0f: // MAC_L(Rm, Rn);
	case 0x1f: // MAC_L(Rm, Rn);
	case 0x2f: // MAC_L(Rm, Rn);
	case 0x3f: // MAC_L(Rm, Rn);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			save_fast_iregs(block);
			UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
			UML_CALLC(block, cfunc_MAC_L, this);
			load_fast_iregs(block);
			return true;
		}
		break;

	case 0x12: // STCGBR(Rn);
		UML_MOV(block, R32(Rn), mem(&m_sh2_state->gbr));        // mov Rn, gbr
		return true;

	case 0x18: // SETT();
		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), SH_T); // or sr, sr, T
		return true;

	case 0x19: // DIV0U();
		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~(SH_M|SH_Q|SH_T)); // and sr, sr, ~(M|Q|T)
		return true;

	case 0x1a: // STSMACL(Rn);
		UML_MOV(block, R32(Rn), mem(&m_sh2_state->macl));       // mov Rn, macl
		return true;

	case 0x1b: // SLEEP();
		UML_MOV(block, I0, mem(&m_sh2_state->sleep_mode));                          // mov i0, sleep_mode
		UML_CMP(block, I0, 0x2);                                            // cmp i0, #2
		UML_JMPc(block, COND_E, compiler.labelnum);                        // beq labelnum
		// sleep mode != 2
		UML_MOV(block, mem(&m_sh2_state->sleep_mode), 0x1);                         // mov sleep_mode, #1
		generate_update_cycles(block, compiler, desc->pc, true);       // repeat this insn
		UML_JMP(block, compiler.labelnum+1);                               // jmp labelnum+1

		UML_LABEL(block, compiler.labelnum++);                             // labelnum:
		// sleep_mode == 2
		UML_MOV(block, mem(&m_sh2_state->sleep_mode), 0x0);                         // sleep_mode = 0
		generate_update_cycles(block, compiler, desc->pc+2, true);     // go to next insn

		UML_LABEL(block, compiler.labelnum++);                             // labelnum+1:
		return true;

	case 0x22: // STCVBR(Rn);
		UML_MOV(block, R32(Rn), mem(&m_sh2_state->vbr));        // mov Rn, vbr
		return true;

	case 0x23: // BRAF(Rn);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			UML_ADD(block, mem(&m_sh2_state->target), R32(Rn), desc->pc+4); // add target, Rn, pc+4

			generate_delay_slot(block, compiler, desc, m_sh2_state->target);

			generate_update_cycles(block, compiler, uml::mem(&m_sh2_state->target), true);  // <subtract cycles>
			UML_HASHJMP(block, 0, mem(&m_sh2_state->target), *m_nocode); // jmp target
			return true;
		}
		break;

	case 0x28: // CLRMAC();
		UML_MOV(block, mem(&m_sh2_state->macl), 0);     // mov macl, #0
		UML_MOV(block, mem(&m_sh2_state->mach), 0);     // mov mach, #0
		return true;

	case 0x29: // MOVT(Rn);
		UML_AND(block, R32(Rn), mem(&m_sh2_state->sr), SH_T);      // and Rn, sr, T
		return true;

	case 0x2a: // STSPR(Rn);
		UML_MOV(block, R32(Rn), mem(&m_sh2_state->pr));         // mov Rn, pr
		return true;

	case 0x2b: // RTE();
		return generate_group_0_RTE(block, compiler, desc, opcode, in_delay_slot, ovrpc);
	}

	return false;
}

bool sh_common_execution::generate_group_4_LDCSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	// needs to be different on SH2 / 4
	UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
	UML_AND(block, I0, I0, SH_FLAGS);  // and r0, r0, FLAGS
	UML_MOV(block, mem(&m_sh2_state->sr), I0);

	compiler.checkints = true;
	return true;
}

bool sh_common_execution::generate_group_4_LDCMSR(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
	SETEA(0);
	UML_CALLH(block, *m_read32);         // call read32
	UML_ADD(block, R32(Rn), R32(Rn), 4);    // add Rn, #4
	UML_MOV(block, mem(&m_sh2_state->sr), I0);      // mov sr, r0

	compiler.checkints = true;
	if (!in_delay_slot)
		generate_update_cycles(block, compiler, desc->pc + 2, true);
	return true;
}

bool sh_common_execution::generate_group_4(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, uint16_t opcode, int in_delay_slot, uint32_t ovrpc)
{
	switch (opcode & 0x3F)
	{
	case 0x00: // SHLL(Rn);
		UML_SHL(block, R32(Rn), R32(Rn), 1);        // shl Rn, Rn, 1
		UML_SETc(block, COND_C, I0);                    // set i0,C
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, SH_T); // rolins [sr],i0,0,T
		return true;

	case 0x01: // SHLR(Rn);
		UML_SHR(block, R32(Rn), R32(Rn), 1);        // shr Rn, Rn, 1
		UML_SETc(block, COND_C, I0);                    // set i0,C
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, SH_T); // rolins [sr],i0,0,T
		return true;

	case 0x04: // ROTL(Rn);
		UML_ROL(block, R32(Rn), R32(Rn), 1);        // rol Rn, Rn, 1
		UML_SETc(block, COND_C, I0);                    // set i0,C
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, SH_T); // rolins [sr],i0,0,T
		return true;

	case 0x05: // ROTR(Rn);
		UML_ROR(block, R32(Rn), R32(Rn), 1);        // ror Rn, Rn, 1
		UML_SETc(block, COND_C, I0);                    // set i0,C
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, SH_T); // rolins [sr],i0,0,T
		return true;

	case 0x02: // STSMMACH(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);    // sub Rn, Rn, #4
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_MOV(block, I1, mem(&m_sh2_state->mach));    // mov r1, mach
		SETEA(0);                   // set ea for debug
		UML_CALLH(block, *m_write32);            // call write32

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x03: // STCMSR(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);    // sub Rn, Rn, #4
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_MOV(block, I1, mem(&m_sh2_state->sr));      // mov r1, sr
		SETEA(0);                   // set ea for debug
		UML_CALLH(block, *m_write32);            // call write32

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x06: // LDSMMACH(Rn);
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *m_read32);         // call read32
		UML_ADD(block, R32(Rn), R32(Rn), 4);    // add Rn, #4
		UML_MOV(block, mem(&m_sh2_state->mach), I0);    // mov mach, r0

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x07: // LDCMSR(Rn);
		return generate_group_4_LDCMSR(block, compiler, desc, opcode, in_delay_slot, ovrpc);

	case 0x08: // SHLL2(Rn);
		UML_SHL(block, R32(Rn), R32(Rn), 2);
		return true;

	case 0x09: // SHLR2(Rn);
		UML_SHR(block, R32(Rn), R32(Rn), 2);
		return true;

	case 0x18: // SHLL8(Rn);
		UML_SHL(block, R32(Rn), R32(Rn), 8);
		return true;

	case 0x19: // SHLR8(Rn);
		UML_SHR(block, R32(Rn), R32(Rn), 8);
		return true;

	case 0x28: // SHLL16(Rn);
		UML_SHL(block, R32(Rn), R32(Rn), 16);
		return true;

	case 0x29: // SHLR16(Rn);
		UML_SHR(block, R32(Rn), R32(Rn), 16);
		return true;

	case 0x0a: // LDSMACH(Rn);
		UML_MOV(block, mem(&m_sh2_state->mach), R32(Rn));       // mov mach, Rn
		return true;

	case 0x0b: // JSR(Rn);
		UML_MOV(block, mem(&m_sh2_state->target), R32(Rn));     // mov target, Rn

		UML_ADD(block, mem(&m_sh2_state->pr), desc->pc, 4); // add m_pr, desc->pc, #4 (skip the current insn & delay slot)

		generate_delay_slot(block, compiler, desc, m_sh2_state->target-4);

		generate_update_cycles(block, compiler, uml::mem(&m_sh2_state->target), true);  // <subtract cycles>
		UML_HASHJMP(block, 0, mem(&m_sh2_state->target), *m_nocode); // and do the jump
		return true;

	case 0x0e: // LDCSR(Rn);
		return generate_group_4_LDCSR(block, compiler, desc, opcode, in_delay_slot, ovrpc);

	case 0x0f: // MAC_W(Rm, Rn);
	case 0x1f: // MAC_W(Rm, Rn);
	case 0x2f: // MAC_W(Rm, Rn);
	case 0x3f: // MAC_W(Rm, Rn);
		save_fast_iregs(block);
		UML_MOV(block, mem(&m_sh2_state->arg0), desc->opptr.w[0]);
		UML_CALLC(block, cfunc_MAC_W, this);
		load_fast_iregs(block);
		return true;

	case 0x10: // DT(Rn);
		if (m_cpu_type > CPU_TYPE_SH1)
		{
			UML_AND(block, I0, mem(&m_sh2_state->sr), ~SH_T);  // and r0, sr, ~T (clear the T bit)
			UML_SUB(block, R32(Rn), R32(Rn), 1);    // sub Rn, Rn, 1
			UML_JMPc(block, COND_NZ, compiler.labelnum);   // jz compiler.labelnum

			UML_OR(block, I0, I0, SH_T);   // or r0, r0, T
			UML_LABEL(block, compiler.labelnum++);         // desc->pc:

			UML_MOV(block, mem(&m_sh2_state->sr), I0);      // mov m_sh2_state->sr, r0
			return true;
		}
		break;

	case 0x11: // CMPPZ(Rn);
		UML_AND(block, I0, mem(&m_sh2_state->sr), ~SH_T);  // and r0, sr, ~T (clear the T bit)

		UML_CMP(block, R32(Rn), 0);     // cmp Rn, 0
		UML_JMPc(block, COND_S, compiler.labelnum);    // js compiler.labelnum    (if negative)

		UML_OR(block, I0, I0, SH_T);   // or r0, r0, T
		UML_LABEL(block, compiler.labelnum++);         // desc->pc:

		UML_MOV(block, mem(&m_sh2_state->sr), I0);      // mov m_sh2_state->sr, r0
		return true;

	case 0x15: // CMPPL(Rn);
		UML_AND(block, I0, mem(&m_sh2_state->sr), ~SH_T);  // and r0, sr, ~T (clear the T bit)

		UML_CMP(block, R32(Rn), 0);     // cmp Rn, 0

		UML_JMPc(block, COND_S, compiler.labelnum);    // js compiler.labelnum    (if negative)
		UML_JMPc(block, COND_Z, compiler.labelnum);    // jz compiler.labelnum    (if zero)

		UML_OR(block, I0, I0, SH_T);   // or r0, r0, T

		UML_LABEL(block, compiler.labelnum++);         // desc->pc:
		UML_MOV(block, mem(&m_sh2_state->sr), I0);      // mov m_sh2_state->sr, r0
		return true;

	case 0x12: // STSMMACL(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);    // sub Rn, Rn, #4
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_MOV(block, I1, mem(&m_sh2_state->macl));    // mov r1, macl
		SETEA(0);                   // set ea for debug
		UML_CALLH(block, *m_write32);            // call write32

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x13: // STCMGBR(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);    // sub Rn, Rn, #4
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_MOV(block, I1, mem(&m_sh2_state->gbr)); // mov r1, gbr
		SETEA(0);                   // set ea for debug
		UML_CALLH(block, *m_write32);            // call write32

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x16: // LDSMMACL(Rn);
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *m_read32);         // call read32
		UML_ADD(block, R32(Rn), R32(Rn), 4);    // add Rn, #4
		UML_MOV(block, mem(&m_sh2_state->macl), I0);    // mov macl, r0

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x17: // LDCMGBR(Rn);
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *m_read32);         // call read32
		UML_ADD(block, R32(Rn), R32(Rn), 4);    // add Rn, #4
		UML_MOV(block, mem(&m_sh2_state->gbr), I0); // mov gbr, r0

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x1a: // LDSMACL(Rn);
		UML_MOV(block, mem(&m_sh2_state->macl), R32(Rn));       // mov macl, Rn
		return true;

	case 0x1b: // TAS(Rn);
		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *m_read8);          // call read8

		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~SH_T);   // and sr, sr, ~T

		UML_CMP(block, I0, 0);      // cmp r0, #0
		UML_JMPc(block, COND_NZ, compiler.labelnum);   // jnz labelnum

		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), SH_T); // or sr, sr, T

		UML_LABEL(block, compiler.labelnum++);     // labelnum:

		UML_OR(block, I1, I0, 0x80);    // or r1, r0, #0x80

		UML_MOV(block, I0, R32(Rn));        // mov r0, Rn
		UML_CALLH(block, *m_write8);         // write the value back

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x1e: // LDCGBR(Rn);
		UML_MOV(block, mem(&m_sh2_state->gbr), R32(Rn));    // mov gbr, Rn
		return true;

	case 0x20: // SHAL(Rn);
		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~SH_T);   // and sr, sr, ~T
		UML_SHR(block, I0, R32(Rn), 31);        // shr r0, Rn, 31
		UML_AND(block, I0, I0, SH_T);      // and r0, r0, T
		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), I0);    // or sr, sr, r0
		UML_SHL(block, R32(Rn), R32(Rn), 1);        // shl Rn, Rn, 1
		return true;

	case 0x21: // SHAR(Rn);
		UML_AND(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), ~SH_T);   // and sr, sr, ~T
		UML_AND(block, I0, R32(Rn), SH_T);     // and r0, Rn, T
		UML_OR(block, mem(&m_sh2_state->sr), mem(&m_sh2_state->sr), I0);    // or sr, sr, r0
		UML_SAR(block, R32(Rn), R32(Rn), 1);        // sar Rn, Rn, 1
		return true;

	case 0x22: // STSMPR(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);        // sub Rn, Rn, 4
		UML_MOV(block, I0, R32(Rn));            // mov r0, Rn
		SETEA(0);
		UML_MOV(block, I1, mem(&m_sh2_state->pr));          // mov r1, pr
		UML_CALLH(block, *m_write32);                // call write32

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x23: // STCMVBR(Rn);
		UML_SUB(block, R32(Rn), R32(Rn), 4);        // sub Rn, Rn, 4
		UML_MOV(block, I0, R32(Rn));            // mov r0, Rn
		SETEA(0);
		UML_MOV(block, I1, mem(&m_sh2_state->vbr));     // mov r1, vbr
		UML_CALLH(block, *m_write32);                // call write32

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x24: // ROTCL(Rn);
		UML_CARRY(block, mem(&m_sh2_state->sr), 0);         // carry sr,0
		UML_ROLC(block, R32(Rn), R32(Rn), 1);           // rolc  Rn,Rn,1
		UML_SETc(block, COND_C, I0);                        // set   i0,C
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, SH_T); // rolins sr,i0,0,T
		return true;

	case 0x25: // ROTCR(Rn);
		UML_CARRY(block, mem(&m_sh2_state->sr), 0);         // carry sr,0
		UML_RORC(block, R32(Rn), R32(Rn), 1);           // rorc  Rn,Rn,1
		UML_SETc(block, COND_C, I0);                        // set   i0,C
		UML_ROLINS(block, mem(&m_sh2_state->sr), I0, 0, SH_T); // rolins sr,i0,0,T
		return true;

	case 0x26: // LDSMPR(Rn);
		UML_MOV(block, I0, R32(Rn));            // mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *m_read32);             // call read32
		UML_MOV(block, mem(&m_sh2_state->pr), I0);          // mov m_pr, r0
		UML_ADD(block, R32(Rn), R32(Rn), 4);        // add Rn, Rn, #4

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x27: // LDCMVBR(Rn);
		UML_MOV(block, I0, R32(Rn));            // mov r0, Rn
		SETEA(0);
		UML_CALLH(block, *m_read32);             // call read32
		UML_MOV(block, mem(&m_sh2_state->vbr), I0);     // mov m_sh2_state->vbr, r0
		UML_ADD(block, R32(Rn), R32(Rn), 4);        // add Rn, Rn, #4

		if (!in_delay_slot)
			generate_update_cycles(block, compiler, desc->pc + 2, true);
		return true;

	case 0x2a: // LDSPR(Rn);
		UML_MOV(block, mem(&m_sh2_state->pr), R32(Rn));         // mov m_pr, Rn
		return true;

	case 0x2b: // JMP(Rn);
		UML_MOV(block, mem(&m_sh2_state->target), R32(Rn));     // mov target, Rn

		generate_delay_slot(block, compiler, desc, m_sh2_state->target);

		generate_update_cycles(block, compiler, uml::mem(&m_sh2_state->target), true);  // <subtract cycles>
		UML_HASHJMP(block, 0, mem(&m_sh2_state->target), *m_nocode); // jmp (target)
		return true;

	case 0x2e: // LDCVBR(Rn);
		UML_MOV(block, mem(&m_sh2_state->vbr), R32(Rn));        //  mov vbr, Rn
		return true;

	case 0x0c:
	case 0x0d:
	case 0x14:
	case 0x1c:
	case 0x1d:
	case 0x2c:
	case 0x2d:
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:
		return false;
	}

	return false;
}


/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    sh2drc_set_options - configure DRC options
-------------------------------------------------*/

void sh_common_execution::sh2drc_set_options(uint32_t options)
{
	if (!allow_drc()) return;
	m_drcoptions = options;
}


/*-------------------------------------------------
    sh2drc_add_pcflush - add a new address where
    the PC must be flushed for speedups to work
-------------------------------------------------*/

void sh_common_execution::sh2drc_add_pcflush(offs_t address)
{
	if (!allow_drc()) return;

	if (m_pcfsel < ARRAY_LENGTH(m_pcflushes))
		m_pcflushes[m_pcfsel++] = address;
}
