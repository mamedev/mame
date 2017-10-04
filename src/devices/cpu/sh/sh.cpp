
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

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1100  1       -
 *  ADD     Rm,Rn
 */
void sh_common_execution::SH2ADD(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] += m_sh2_state->r[m];
}

/*  code                 cycles  t-bit
 *  0111 nnnn iiii iiii  1       -
 *  ADD     #imm,Rn
 */
void sh_common_execution::SH2ADDI(uint32_t i, uint32_t n)
{
	m_sh2_state->r[n] += (int32_t)(int16_t)(int8_t)i;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1110  1       carry
 *  ADDC    Rm,Rn
 */
void sh_common_execution::SH2ADDC(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2ADDV(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2AND(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] &= m_sh2_state->r[m];
}

/*  code                 cycles  t-bit
 *  1100 1001 iiii iiii  1       -
 *  AND     #imm,R0
 */
void sh_common_execution::SH2ANDI(uint32_t i)
{
	m_sh2_state->r[0] &= i;
}

/*  code                 cycles  t-bit
 *  1100 1101 iiii iiii  1       -
 *  AND.B   #imm,@(R0,GBR)
 */
void sh_common_execution::SH2ANDM(uint32_t i)
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
void sh_common_execution::SH2BF(uint32_t d)
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
void sh_common_execution::SH2BFS(uint32_t d)
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
void sh_common_execution::SH2BRA(uint32_t d)
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
void sh_common_execution::SH2BRAF(uint32_t m)
{
	m_sh2_state->m_delay = m_sh2_state->pc + m_sh2_state->r[m] + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  1011 dddd dddd dddd  2       -
 *  BSR     disp12
 */
void sh_common_execution::SH2BSR(uint32_t d)
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
void sh_common_execution::SH2BSRF(uint32_t m)
{
	m_sh2_state->pr = m_sh2_state->pc + 2;
	m_sh2_state->m_delay = m_sh2_state->pc + m_sh2_state->r[m] + 2;
	m_sh2_state->icount--;
}

/*  code                 cycles  t-bit
 *  1000 1001 dddd dddd  3/1     -
 *  BT      disp8
 */
void sh_common_execution::SH2BT(uint32_t d)
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
void sh_common_execution::SH2BTS(uint32_t d)
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
void sh_common_execution::SH2CLRMAC()
{
	m_sh2_state->mach = 0;
	m_sh2_state->macl = 0;
}

/*  code                 cycles  t-bit
 *  0000 0000 0000 1000  1       -
 *  CLRT
 */
void sh_common_execution::SH2CLRT()
{
	m_sh2_state->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0000  1       comparison result
 *  CMP_EQ  Rm,Rn
 */
void sh_common_execution::SH2CMPEQ(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2CMPGE(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2CMPGT(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2CMPHI(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2CMPHS(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2CMPPL(uint32_t n)
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
void sh_common_execution::SH2CMPPZ(uint32_t n)
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
void sh_common_execution::SH2CMPSTR(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2CMPIM(uint32_t i)
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
void sh_common_execution::SH2DIV0S(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2DIV0U()
{
	m_sh2_state->sr &= ~(M | Q | T);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0100  1       calculation result
 *  DIV1 Rm,Rn
 */
void sh_common_execution::SH2DIV1(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2DMULS(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2DMULU(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2DT(uint32_t n)
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
void sh_common_execution::SH2EXTSB(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = ((int32_t)m_sh2_state->r[m] << 24) >> 24;
}

/*  EXTS.W  Rm,Rn */
void sh_common_execution::SH2EXTSW(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = ((int32_t)m_sh2_state->r[m] << 16) >> 16;
}

/*  EXTU.B  Rm,Rn */
void sh_common_execution::SH2EXTUB(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->r[m] & 0x000000ff;
}

/*  EXTU.W  Rm,Rn */
void sh_common_execution::SH2EXTUW(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->r[m] & 0x0000ffff;
}

/*  JMP     @Rm */
void sh_common_execution::SH2JMP(uint32_t m)
{
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->icount--; // not in SH4 implementation?
}

/*  JSR     @Rm */
void sh_common_execution::SH2JSR(uint32_t m)
{
	m_sh2_state->pr = m_sh2_state->pc + 2;
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->icount--;
}

/*  LDC     Rm,GBR */
void sh_common_execution::SH2LDCGBR(uint32_t m)
{
	m_sh2_state->gbr = m_sh2_state->r[m];
}

/*  LDC     Rm,VBR */
void sh_common_execution::SH2LDCVBR(uint32_t m)
{
	m_sh2_state->vbr = m_sh2_state->r[m];
}

/*  LDC.L   @Rm+,GBR */
void sh_common_execution::SH2LDCMGBR(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->gbr = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
	m_sh2_state->icount -= 2;
}

/*  LDC.L   @Rm+,VBR */
void sh_common_execution::SH2LDCMVBR(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->vbr = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
	m_sh2_state->icount -= 2;
}

/*  LDS     Rm,MACH */
void sh_common_execution::SH2LDSMACH(uint32_t m)
{
	m_sh2_state->mach = m_sh2_state->r[m];
}

/*  LDS     Rm,MACL */
void sh_common_execution::SH2LDSMACL(uint32_t m)
{
	m_sh2_state->macl = m_sh2_state->r[m];
}

/*  LDS     Rm,PR */
void sh_common_execution::SH2LDSPR(uint32_t m)
{
	m_sh2_state->pr = m_sh2_state->r[m];
}

/*  LDS.L   @Rm+,MACH */
void sh_common_execution::SH2LDSMMACH(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->mach = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
}

/*  LDS.L   @Rm+,MACL */
void sh_common_execution::SH2LDSMMACL(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->macl = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
}

/*  LDS.L   @Rm+,PR */
void sh_common_execution::SH2LDSMPR(uint32_t m)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->pr = RL( m_sh2_state->ea );
	m_sh2_state->r[m] += 4;
}

/*  MAC.L   @Rm+,@Rn+ */
void sh_common_execution::SH2MAC_L(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2MAC_W(uint32_t m, uint32_t n)
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
void sh_common_execution::SH2MOV(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->r[m];
}

/*  MOV.B   Rm,@Rn */
void sh_common_execution::SH2MOVBS(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n];
	WB( m_sh2_state->ea, m_sh2_state->r[m] & 0x000000ff);
}

/*  MOV.W   Rm,@Rn */
void sh_common_execution::SH2MOVWS(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n];
	WW( m_sh2_state->ea, m_sh2_state->r[m] & 0x0000ffff);
}

/*  MOV.L   Rm,@Rn */
void sh_common_execution::SH2MOVLS(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->r[m] );
}

/*  MOV.B   @Rm,Rn */
void sh_common_execution::SH2MOVBL(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->ea );
}

/*  MOV.W   @Rm,Rn */
void sh_common_execution::SH2MOVWL(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @Rm,Rn */
void sh_common_execution::SH2MOVLL(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m];
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOV.B   Rm,@-Rn */
void sh_common_execution::SH2MOVBM(uint32_t m, uint32_t n)
{
	/* SMG : bug fix, was reading m_sh2_state->r[n] */
	uint32_t data = m_sh2_state->r[m] & 0x000000ff;

	m_sh2_state->r[n] -= 1;
	WB( m_sh2_state->r[n], data );
}

/*  MOV.W   Rm,@-Rn */
void sh_common_execution::SH2MOVWM(uint32_t m, uint32_t n)
{
	uint32_t data = m_sh2_state->r[m] & 0x0000ffff;

	m_sh2_state->r[n] -= 2;
	WW( m_sh2_state->r[n], data );
}

/*  MOV.L   Rm,@-Rn */
void sh_common_execution::SH2MOVLM(uint32_t m, uint32_t n)
{
	uint32_t data = m_sh2_state->r[m];

	m_sh2_state->r[n] -= 4;
	WL( m_sh2_state->r[n], data );
}

/*  MOV.B   @Rm+,Rn */
void sh_common_execution::SH2MOVBP(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->r[m] );
	if (n != m)
		m_sh2_state->r[m] += 1;
}

/*  MOV.W   @Rm+,Rn */
void sh_common_execution::SH2MOVWP(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->r[m] );
	if (n != m)
		m_sh2_state->r[m] += 2;
}

/*  MOV.L   @Rm+,Rn */
void sh_common_execution::SH2MOVLP(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = RL( m_sh2_state->r[m] );
	if (n != m)
		m_sh2_state->r[m] += 4;
}

/*  MOV.B   Rm,@(R0,Rn) */
void sh_common_execution::SH2MOVBS0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n] + m_sh2_state->r[0];
	WB( m_sh2_state->ea, m_sh2_state->r[m] & 0x000000ff );
}

/*  MOV.W   Rm,@(R0,Rn) */
void sh_common_execution::SH2MOVWS0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n] + m_sh2_state->r[0];
	WW( m_sh2_state->ea, m_sh2_state->r[m] & 0x0000ffff );
}

/*  MOV.L   Rm,@(R0,Rn) */
void sh_common_execution::SH2MOVLS0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[n] + m_sh2_state->r[0];
	WL( m_sh2_state->ea, m_sh2_state->r[m] );
}

/*  MOV.B   @(R0,Rm),Rn */
void sh_common_execution::SH2MOVBL0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m] + m_sh2_state->r[0];
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->ea );
}

/*  MOV.W   @(R0,Rm),Rn */
void sh_common_execution::SH2MOVWL0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m] + m_sh2_state->r[0];
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @(R0,Rm),Rn */
void sh_common_execution::SH2MOVLL0(uint32_t m, uint32_t n)
{
	m_sh2_state->ea = m_sh2_state->r[m] + m_sh2_state->r[0];
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOV     #imm,Rn */
void sh_common_execution::SH2MOVI(uint32_t i, uint32_t n)
{
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t)(int8_t) i;
}

/*  MOV.W   @(disp8,PC),Rn */
void sh_common_execution::SH2MOVWI(uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->pc + disp * 2 + 2;
	m_sh2_state->r[n] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @(disp8,PC),Rn */
void sh_common_execution::SH2MOVLI(uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = ((m_sh2_state->pc + 2) & ~3) + disp * 4;
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOV.B   @(disp8,GBR),R0 */
void sh_common_execution::SH2MOVBLG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp;
	m_sh2_state->r[0] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->ea );
}

/*  MOV.W   @(disp8,GBR),R0 */
void sh_common_execution::SH2MOVWLG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 2;
	m_sh2_state->r[0] = (int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @(disp8,GBR),R0 */
void sh_common_execution::SH2MOVLLG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 4;
	m_sh2_state->r[0] = RL( m_sh2_state->ea );
}

/*  MOV.B   R0,@(disp8,GBR) */
void sh_common_execution::SH2MOVBSG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp;
	WB( m_sh2_state->ea, m_sh2_state->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp8,GBR) */
void sh_common_execution::SH2MOVWSG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 2;
	WW( m_sh2_state->ea, m_sh2_state->r[0] & 0x0000ffff );
}

/*  MOV.L   R0,@(disp8,GBR) */
void sh_common_execution::SH2MOVLSG(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = m_sh2_state->gbr + disp * 4;
	WL( m_sh2_state->ea, m_sh2_state->r[0] );
}

/*  MOV.B   R0,@(disp4,Rn) */
void sh_common_execution::SH2MOVBS4(uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[n] + disp;
	WB( m_sh2_state->ea, m_sh2_state->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp4,Rn) */
void sh_common_execution::SH2MOVWS4(uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[n] + disp * 2;
	WW( m_sh2_state->ea, m_sh2_state->r[0] & 0x0000ffff );
}

/* MOV.L Rm,@(disp4,Rn) */
void sh_common_execution::SH2MOVLS4(uint32_t m, uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[n] + disp * 4;
	WL( m_sh2_state->ea, m_sh2_state->r[m] );
}

/*  MOV.B   @(disp4,Rm),R0 */
void sh_common_execution::SH2MOVBL4(uint32_t m, uint32_t d)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[m] + disp;
	m_sh2_state->r[0] = (uint32_t)(int32_t)(int16_t)(int8_t) RB( m_sh2_state->ea );
}

/*  MOV.W   @(disp4,Rm),R0 */
void sh_common_execution::SH2MOVWL4(uint32_t m, uint32_t d)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[m] + disp * 2;
	m_sh2_state->r[0] = (uint32_t)(int32_t)(int16_t) RW( m_sh2_state->ea );
}

/*  MOV.L   @(disp4,Rm),Rn */
void sh_common_execution::SH2MOVLL4(uint32_t m, uint32_t d, uint32_t n)
{
	uint32_t disp = d & 0x0f;
	m_sh2_state->ea = m_sh2_state->r[m] + disp * 4;
	m_sh2_state->r[n] = RL( m_sh2_state->ea );
}

/*  MOVA    @(disp8,PC),R0 */
void sh_common_execution::SH2MOVA(uint32_t d)
{
	uint32_t disp = d & 0xff;
	m_sh2_state->ea = ((m_sh2_state->pc + 2) & ~3) + disp * 4;
	m_sh2_state->r[0] = m_sh2_state->ea;
}

/*  MOVT    Rn */
void sh_common_execution::SH2MOVT(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->sr & T;
}

/*  MUL.L   Rm,Rn */
void sh_common_execution::SH2MULL(uint32_t m, uint32_t n)
{
	m_sh2_state->macl = m_sh2_state->r[n] * m_sh2_state->r[m];
	m_sh2_state->icount--;
}

/*  MUL.L   Rm,Rn */
void sh_common_execution::MULL(const uint16_t opcode)
{
	m_sh2_state->macl = m_sh2_state->r[Rn] * m_sh2_state->r[Rm];
	m_sh2_state->icount--;
}

/*  MULS    Rm,Rn */
void sh_common_execution::SH2MULS(uint32_t m, uint32_t n)
{
	m_sh2_state->macl = (int16_t) m_sh2_state->r[n] * (int16_t) m_sh2_state->r[m];
}

/*  MULS    Rm,Rn */
void sh_common_execution::MULS(const uint16_t opcode)
{
	m_sh2_state->macl = (int16_t) m_sh2_state->r[Rn] * (int16_t) m_sh2_state->r[Rm];
}

/*  MULU    Rm,Rn */
void sh_common_execution::SH2MULU(uint32_t m, uint32_t n)
{
	m_sh2_state->macl = (uint16_t) m_sh2_state->r[n] * (uint16_t) m_sh2_state->r[m];
}

/*  MULU    Rm,Rn */
void sh_common_execution::MULU(const uint16_t opcode)
{
	m_sh2_state->macl = (uint16_t) m_sh2_state->r[Rn] * (uint16_t) m_sh2_state->r[Rm];
}

/*  NEG     Rm,Rn */
void sh_common_execution::SH2NEG(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = 0 - m_sh2_state->r[m];
}

/*  NEG     Rm,Rn */
void sh_common_execution::NEG(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = 0 - m_sh2_state->r[Rm];
}

/*  NEGC    Rm,Rn */
void sh_common_execution::SH2NEGC(uint32_t m, uint32_t n)
{
	uint32_t temp;

	temp = m_sh2_state->r[m];
	m_sh2_state->r[n] = -temp - (m_sh2_state->sr & T);
	if (temp || (m_sh2_state->sr & T))
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  NEGC    Rm,Rn */
void sh_common_execution::NEGC(const uint16_t opcode)
{
	uint32_t temp;

	temp = m_sh2_state->r[Rm];
	m_sh2_state->r[Rn] = -temp - (m_sh2_state->sr & T);
	if (temp || (m_sh2_state->sr & T))
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  NOP */
void sh_common_execution::SH2NOP(void)
{
}

/*  NOP */
void sh_common_execution::NOP(const uint16_t opcode)
{
}

/*  NOT     Rm,Rn */
void sh_common_execution::SH2NOT(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] = ~m_sh2_state->r[m];
}

/*  NOT     Rm,Rn */
void sh_common_execution::NOT(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = ~m_sh2_state->r[Rm];
}

/*  OR      Rm,Rn */
void sh_common_execution::SH2OR(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] |= m_sh2_state->r[m];
}

/*  OR      Rm,Rn */
void sh_common_execution::OR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] |= m_sh2_state->r[Rm];
}

/*  OR      #imm,R0 */
void sh_common_execution::SH2ORI(uint32_t i)
{
	m_sh2_state->r[0] |= i;
}

/*  OR      #imm,R0 */
void sh_common_execution::ORI(const uint16_t opcode)
{
	m_sh2_state->r[0] |= (opcode&0xff);
	m_sh2_state->icount -= 2;
}

/*  OR.B    #imm,@(R0,GBR) */
void sh_common_execution::SH2ORM(uint32_t i)
{
	uint32_t temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = RB( m_sh2_state->ea );
	temp |= i;
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 2;
}

/*  OR.B    #imm,@(R0,GBR) */
void sh_common_execution::ORM(const uint16_t opcode)
{
	uint32_t temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = RB( m_sh2_state->ea );
	temp |= (opcode&0xff);
	WB(m_sh2_state->ea, temp );
}

/*  ROTCL   Rn */
void sh_common_execution::SH2ROTCL(uint32_t n)
{
	uint32_t temp;

	temp = (m_sh2_state->r[n] >> 31) & T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->sr & T);
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | temp;
}

/*  ROTCL   Rn */
void sh_common_execution::ROTCL(const uint16_t opcode)
{
	uint32_t n = Rn;

	uint32_t temp;

	temp = (m_sh2_state->r[n] >> 31) & T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->sr & T);
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | temp;
}

/*  ROTCR   Rn */
void sh_common_execution::SH2ROTCR(uint32_t n)
{
	uint32_t temp;
	temp = (m_sh2_state->sr & T) << 31;
	if (m_sh2_state->r[n] & T)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 1) | temp;
}

/*  ROTCR   Rn */
void sh_common_execution::ROTCR(const uint16_t opcode)
{
	uint32_t n = Rn;

	uint32_t temp;
	temp = (m_sh2_state->sr & T) << 31;
	if (m_sh2_state->r[n] & T)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 1) | temp;
}

/*  ROTL    Rn */
void sh_common_execution::SH2ROTL(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->r[n] >> 31);
}

/*  ROTL    Rn */
void sh_common_execution::ROTL(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] = (m_sh2_state->r[n] << 1) | (m_sh2_state->r[n] >> 31);
}

/*  ROTR    Rn */
void sh_common_execution::SH2ROTR(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 1) | (m_sh2_state->r[n] << 31);
}

/*  ROTR    Rn */
void sh_common_execution::ROTR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 1) | (m_sh2_state->r[n] << 31);
}

/*  RTS */
void sh_common_execution::SH2RTS()
{
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->pr;
	m_sh2_state->icount--;
}

/*  RTS */
void sh_common_execution::RTS(const uint16_t opcode)
{
	m_sh2_state->m_delay = m_sh2_state->ea = m_sh2_state->pr;
	m_sh2_state->icount--;
}

/*  SETT */
void sh_common_execution::SH2SETT()
{
	m_sh2_state->sr |= T;
}

/*  SETT */
void sh_common_execution::SETT(const uint16_t opcode)
{
	m_sh2_state->sr |= T;
}

/*  SHAL    Rn      (same as SHLL) */
void sh_common_execution::SH2SHAL(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] <<= 1;
}

/*  SHAL    Rn      (same as SHLL) */
void sh_common_execution::SHAL(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] <<= 1;
}

/*  SHAR    Rn */
void sh_common_execution::SH2SHAR(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] = (uint32_t)((int32_t)m_sh2_state->r[n] >> 1);
}

/*  SHAR    Rn */
void sh_common_execution::SHAR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] = (uint32_t)((int32_t)m_sh2_state->r[n] >> 1);
}

/*  SHLL    Rn      (same as SHAL) */
void sh_common_execution::SH2SHLL(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] <<= 1;
}

/*  SHLL    Rn      (same as SHAL) */
void sh_common_execution::SHLL(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->sr = (m_sh2_state->sr & ~T) | ((m_sh2_state->r[n] >> 31) & T);
	m_sh2_state->r[n] <<= 1;
}

/*  SHLL2   Rn */
void sh_common_execution::SH2SHLL2(uint32_t n)
{
	m_sh2_state->r[n] <<= 2;
}

/*  SHLL2   Rn */
void sh_common_execution::SHLL2(const uint16_t opcode)
{
	m_sh2_state->r[Rn] <<= 2;
}

/*  SHLL8   Rn */
void sh_common_execution::SH2SHLL8(uint32_t n)
{
	m_sh2_state->r[n] <<= 8;
}

/*  SHLL8   Rn */
void sh_common_execution::SHLL8(const uint16_t opcode)
{
	m_sh2_state->r[Rn] <<= 8;
}

/*  SHLL16  Rn */
void sh_common_execution::SH2SHLL16(uint32_t n)
{
	m_sh2_state->r[n] <<= 16;
}

/*  SHLL16  Rn */
void sh_common_execution::SHLL16(const uint16_t opcode)
{
	m_sh2_state->r[Rn] <<= 16;
}

/*  SHLR    Rn */
void sh_common_execution::SH2SHLR(uint32_t n)
{
	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] >>= 1;
}

/*  SHLR    Rn */
void sh_common_execution::SHLR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->sr = (m_sh2_state->sr & ~T) | (m_sh2_state->r[n] & T);
	m_sh2_state->r[n] >>= 1;
}

/*  SHLR2   Rn */
void sh_common_execution::SH2SHLR2(uint32_t n)
{
	m_sh2_state->r[n] >>= 2;
}

/*  SHLR2   Rn */
void sh_common_execution::SHLR2(const uint16_t opcode)
{
	m_sh2_state->r[Rn] >>= 2;
}

/*  SHLR8   Rn */
void sh_common_execution::SH2SHLR8(uint32_t n)
{
	m_sh2_state->r[n] >>= 8;
}

/*  SHLR8   Rn */
void sh_common_execution::SHLR8(const uint16_t opcode)
{
	m_sh2_state->r[Rn] >>= 8;
}

/*  SHLR16  Rn */
void sh_common_execution::SH2SHLR16(uint32_t n)
{
	m_sh2_state->r[n] >>= 16;
}

/*  SHLR16  Rn */
void sh_common_execution::SHLR16(const uint16_t opcode)
{
	m_sh2_state->r[Rn] >>= 16;
}

/*  STC     SR,Rn */
void sh_common_execution::SH2STCSR(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->sr;
}

/*  STC     SR,Rn */
void sh_common_execution::STCSR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->sr;
}

/*  STC     GBR,Rn */
void sh_common_execution::SH2STCGBR(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->gbr;
}

/*  STC     GBR,Rn */
void sh_common_execution::STCGBR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->gbr;
}

/*  STC     VBR,Rn */
void sh_common_execution::SH2STCVBR(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->vbr;
}

/*  STC     VBR,Rn */
void sh_common_execution::STCVBR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->vbr;
}

/*  STC.L   SR,@-Rn */
void sh_common_execution::SH2STCMSR(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->sr );
	m_sh2_state->icount--;
}

/*  STC.L   SR,@-Rn */
void sh_common_execution::STCMSR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->sr );
	m_sh2_state->icount--;
}

/*  STC.L   GBR,@-Rn */
void sh_common_execution::SH2STCMGBR(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->gbr );
	m_sh2_state->icount--;
}

/*  STC.L   GBR,@-Rn */
void sh_common_execution::STCMGBR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->gbr );
	m_sh2_state->icount--;
}

/*  STC.L   VBR,@-Rn */
void sh_common_execution::SH2STCMVBR(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->vbr );
	m_sh2_state->icount--;
}

/*  STC.L   VBR,@-Rn */
void sh_common_execution::STCMVBR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->vbr );
	m_sh2_state->icount--;
}

/*  STS     MACH,Rn */
void sh_common_execution::SH2STSMACH(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->mach;
}

/*  STS     MACH,Rn */
void sh_common_execution::STSMACH(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->mach;
}

/*  STS     MACL,Rn */
void sh_common_execution::SH2STSMACL(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->macl;
}

/*  STS     MACL,Rn */
void sh_common_execution::STSMACL(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->macl;
}

/*  STS     PR,Rn */
void sh_common_execution::SH2STSPR(uint32_t n)
{
	m_sh2_state->r[n] = m_sh2_state->pr;
}

/*  STS     PR,Rn */
void sh_common_execution::STSPR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] = m_sh2_state->pr;
}

/*  STS.L   MACH,@-Rn */
void sh_common_execution::SH2STSMMACH(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->mach );
}

/*  STS.L   MACH,@-Rn */
void sh_common_execution::STSMMACH(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->mach );
}

/*  STS.L   MACL,@-Rn */
void sh_common_execution::SH2STSMMACL(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->macl );
}

/*  STS.L   MACL,@-Rn */
void sh_common_execution::STSMMACL(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->macl );
}

/*  STS.L   PR,@-Rn */
void sh_common_execution::SH2STSMPR(uint32_t n)
{
	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL( m_sh2_state->ea, m_sh2_state->pr );
}

/*  STS.L   PR,@-Rn */
void sh_common_execution::STSMPR(const uint16_t opcode)
{
	uint32_t n = Rn;

	m_sh2_state->r[n] -= 4;
	m_sh2_state->ea = m_sh2_state->r[n];
	WL(m_sh2_state->ea, m_sh2_state->pr );
}

/*  SUB     Rm,Rn */
void sh_common_execution::SH2SUB(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] -= m_sh2_state->r[m];
}

/*  SUB     Rm,Rn */
void sh_common_execution::SUB(const uint16_t opcode)
{
	m_sh2_state->r[Rn] -= m_sh2_state->r[Rm];
}

/*  SUBC    Rm,Rn */
void sh_common_execution::SH2SUBC(uint32_t m, uint32_t n)
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

/*  SUBC    Rm,Rn */
void sh_common_execution::SUBC(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

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
void sh_common_execution::SH2SUBV(uint32_t m, uint32_t n)
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

/*  SUBV    Rm,Rn */
void sh_common_execution::SUBV(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

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
void sh_common_execution::SH2SWAPB(uint32_t m, uint32_t n)
{
	uint32_t temp0, temp1;

	temp0 = m_sh2_state->r[m] & 0xffff0000;
	temp1 = (m_sh2_state->r[m] & 0x000000ff) << 8;
	m_sh2_state->r[n] = (m_sh2_state->r[m] >> 8) & 0x000000ff;
	m_sh2_state->r[n] = m_sh2_state->r[n] | temp1 | temp0;
}

/*  SWAP.B  Rm,Rn */
void sh_common_execution::SWAPB(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	uint32_t temp0, temp1;

	temp0 = m_sh2_state->r[m] & 0xffff0000;
	temp1 = (m_sh2_state->r[m] & 0x000000ff) << 8;
	m_sh2_state->r[n] = (m_sh2_state->r[m] >> 8) & 0x000000ff;
	m_sh2_state->r[n] = m_sh2_state->r[n] | temp1 | temp0;
}

/*  SWAP.W  Rm,Rn */
void sh_common_execution::SH2SWAPW(uint32_t m, uint32_t n)
{
	uint32_t temp;

	temp = (m_sh2_state->r[m] >> 16) & 0x0000ffff;
	m_sh2_state->r[n] = (m_sh2_state->r[m] << 16) | temp;
}

/*  SWAP.W  Rm,Rn */
void sh_common_execution::SWAPW(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	uint32_t temp;

	temp = (m_sh2_state->r[m] >> 16) & 0x0000ffff;
	m_sh2_state->r[n] = (m_sh2_state->r[m] << 16) | temp;
}

/*  TAS.B   @Rn */
void sh_common_execution::SH2TAS(uint32_t n)
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

/*  TAS.B   @Rn */
void sh_common_execution::TAS(const uint16_t opcode)
{
	uint32_t n = Rn;

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
	WB(m_sh2_state->ea, temp );
	m_sh2_state->icount -= 3;
}

/*  TST     Rm,Rn */
void sh_common_execution::SH2TST(uint32_t m, uint32_t n)
{
	if ((m_sh2_state->r[n] & m_sh2_state->r[m]) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  TST     Rm,Rn */
void sh_common_execution::TST(const uint16_t opcode)
{
	if ((m_sh2_state->r[Rn] & m_sh2_state->r[Rm]) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  TST     #imm,R0 */
void sh_common_execution::SH2TSTI(uint32_t i)
{
	uint32_t imm = i & 0xff;

	if ((imm & m_sh2_state->r[0]) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  TST     #imm,R0 */
void sh_common_execution::TSTI(const uint16_t opcode)
{
	uint32_t imm = opcode & 0xff;

	if ((imm & m_sh2_state->r[0]) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
}

/*  TST.B   #imm,@(R0,GBR) */
void sh_common_execution::SH2TSTM(uint32_t i)
{
	uint32_t imm = i & 0xff;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	if ((imm & RB( m_sh2_state->ea )) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	m_sh2_state->icount -= 2;
}

/*  TST.B   #imm,@(R0,GBR) */
void sh_common_execution::TSTM(const uint16_t opcode)
{
	uint32_t imm = opcode & 0xff;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	if ((imm & RB( m_sh2_state->ea )) == 0)
		m_sh2_state->sr |= T;
	else
		m_sh2_state->sr &= ~T;
	m_sh2_state->icount -= 2;
}

/*  XOR     Rm,Rn */
void sh_common_execution::SH2XOR(uint32_t m, uint32_t n)
{
	m_sh2_state->r[n] ^= m_sh2_state->r[m];
}

/*  XOR     Rm,Rn */
void sh_common_execution::XOR(const uint16_t opcode)
{
	m_sh2_state->r[Rn] ^= m_sh2_state->r[Rm];
}

/*  XOR     #imm,R0 */
void sh_common_execution::SH2XORI(uint32_t i)
{
	uint32_t imm = i & 0xff;
	m_sh2_state->r[0] ^= imm;
}

/*  XOR     #imm,R0 */
void sh_common_execution::XORI(const uint16_t opcode)
{
	uint32_t imm = opcode & 0xff;
	m_sh2_state->r[0] ^= imm;
}

/*  XOR.B   #imm,@(R0,GBR) */
void sh_common_execution::SH2XORM(uint32_t i)
{
	uint32_t imm = i & 0xff;
	uint32_t temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = RB( m_sh2_state->ea );
	temp ^= imm;
	WB( m_sh2_state->ea, temp );
	m_sh2_state->icount -= 2;
}


/*  XOR.B   #imm,@(R0,GBR) */
void sh_common_execution::XORM(const uint16_t opcode)
{
	uint32_t imm = opcode & 0xff;
	uint32_t temp;

	m_sh2_state->ea = m_sh2_state->gbr + m_sh2_state->r[0];
	temp = RB( m_sh2_state->ea );
	temp ^= imm;
	WB(m_sh2_state->ea, temp );
	m_sh2_state->icount -= 2;
}

/*  XTRCT   Rm,Rn */
void sh_common_execution::SH2XTRCT(uint32_t m, uint32_t n)
{
	uint32_t temp;

	temp = (m_sh2_state->r[m] << 16) & 0xffff0000;
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 16) & 0x0000ffff;
	m_sh2_state->r[n] |= temp;
}

/*  XTRCT   Rm,Rn */
void sh_common_execution::XTRCT(const uint16_t opcode)
{
	uint32_t m = Rm; uint32_t n = Rn;

	uint32_t temp;

	temp = (m_sh2_state->r[m] << 16) & 0xffff0000;
	m_sh2_state->r[n] = (m_sh2_state->r[n] >> 16) & 0x0000ffff;
	m_sh2_state->r[n] |= temp;
}


/* SH4 only */

/*  MOVCA.L     R0,@Rn */
void sh_common_execution::MOVCAL(const uint16_t opcode)
{
	m_sh2_state->ea = m_sh2_state->r[Rn];
	WL(m_sh2_state->ea, m_sh2_state->r[0] );
}

void sh_common_execution::CLRS(const uint16_t opcode)
{
	m_sh2_state->sr &= ~S;
}

void sh_common_execution::SETS(const uint16_t opcode)
{
	m_sh2_state->sr |= S;
}

// SH2

/*  ILLEGAL */
void sh_common_execution::SH2ILLEGAL()
{
	//logerror("Illegal opcode at %08x\n", m_sh2_state->pc - 2);
	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->sr );     /* push SR onto stack */
	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->pc - 2 ); /* push PC onto stack */

	/* fetch PC */
	m_sh2_state->pc = RL( m_sh2_state->vbr + 4 * 4 );

	/* TODO: timing is a guess */
	m_sh2_state->icount -= 5;
}

/*  SLEEP */
void sh_common_execution::SH2SLEEP()
{
	if(m_sh2_state->sleep_mode != 2)
		m_sh2_state->pc -= 2;
	m_sh2_state->icount -= 2;
	/* Wait_for_exception; */
	if(m_sh2_state->sleep_mode == 0)
		m_sh2_state->sleep_mode = 1;
	else if(m_sh2_state->sleep_mode == 2)
		m_sh2_state->sleep_mode = 0;
}

/*  TRAPA   #imm */
void sh_common_execution::SH2TRAPA(uint32_t i)
{
	uint32_t imm = i & 0xff;

	m_sh2_state->ea = m_sh2_state->vbr + imm * 4;

	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->sr );
	m_sh2_state->r[15] -= 4;
	WL( m_sh2_state->r[15], m_sh2_state->pc );

	m_sh2_state->pc = RL( m_sh2_state->ea );

	m_sh2_state->icount -= 7;
}

#undef Rn
#undef Rm

#undef T
#undef S
#undef I
#undef Q
#undef M

