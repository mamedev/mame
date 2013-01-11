/*****************************************************************************
 *
 *   sh4.c
 *   Portable Hitachi SH-4 (SH7750 family) emulator
 *
 *   By R. Belmont, based on sh2.c by Juergen Buchmueller, Mariusz Wojcieszek,
 *      Olivier Galibert, Sylvain Glaize, and James Forshaw.
 *
 *
 *   TODO: FPU
 *         DMA
 *         on-board peripherals
 *
 *   DONE: boot/reset setup
 *         64-bit data bus
 *         banked registers
 *         additional registers for supervisor mode
 *         FPU status and data registers
 *         state save for the new registers
 *         interrupts
 *         store queues
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "sh4.h"
#include "sh4regs.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"

#ifndef USE_SH4DRC

CPU_DISASSEMBLE( sh4 );
CPU_DISASSEMBLE( sh4be );

typedef const void (*sh4ophandler)(sh4_state*, const UINT16);

sh4ophandler master_ophandler_table[0x10000];
void sh4_build_optable(sh4_state* sh4);


/* Called for unimplemented opcodes */
const void TODO(sh4_state *sh4, const UINT16 opcode)
{
}

#if 0
int sign_of(int n)
{
	return(sh4->fr[n]>>31);
}

void zero(int n,int sign)
{
if (sign == 0)
	sh4->fr[n] = 0x00000000;
else
	sh4->fr[n] = 0x80000000;
if ((sh4->fpscr & PR) == 1)
	sh4->fr[n+1] = 0x00000000;
}

int data_type_of(int n)
{
UINT32 abs;

	abs = sh4->fr[n] & 0x7fffffff;
	if ((sh4->fpscr & PR) == 0) { /* Single-precision */
		if (abs < 0x00800000) {
			if (((sh4->fpscr & DN) == 1) || (abs == 0x00000000)) {
				if (sign_of(n) == 0) {
					zero(n, 0);
					return(SH4_FPU_PZERO);
				} else {
					zero(n, 1);
					return(SH4_FPU_NZERO);
				}
			} else
				return(SH4_FPU_DENORM);
		} else
			if (abs < 0x7f800000)
				return(SH4_FPU_NORM);
			else
				if (abs == 0x7f800000) {
					if (sign_of(n) == 0)
						return(SH4_FPU_PINF);
					else
						return(SH4_FPU_NINF);
				} else
					if (abs < 0x7fc00000)
						return(SH4_FPU_qNaN);
					else
						return(SH4_FPU_sNaN);
	} else { /* Double-precision */
		if (abs < 0x00100000) {
			if (((sh4->fpscr & DN) == 1) || ((abs == 0x00000000) && (sh4->fr[n+1] == 0x00000000))) {
				if(sign_of(n) == 0) {
					zero(n, 0);
					return(SH4_FPU_PZERO);
				} else {
					zero(n, 1);
					return(SH4_FPU_NZERO);
				}
			} else
				return(SH4_FPU_DENORM);
		} else
			if (abs < 0x7ff00000)
				return(SH4_FPU_NORM);
			else
				if ((abs == 0x7ff00000) && (sh4->fr[n+1] == 0x00000000)) {
					if (sign_of(n) == 0)
						return(SH4_FPU_PINF);
					else
						return(SH4_FPU_NINF);
				} else
					if (abs < 0x7ff80000)
						return(SH4_FPU_qNaN);
					else
						return(SH4_FPU_sNaN);
	}
	return(SH4_FPU_NORM);
}
#endif

const UINT8 RB(sh4_state *sh4, offs_t A)
{
	if (A >= 0xe0000000)
		return sh4->program->read_byte(A);

	return sh4->program->read_byte(A & AM);
}

const UINT16 RW(sh4_state *sh4, offs_t A)
{
	if (A >= 0xe0000000)
		return sh4->program->read_word(A);

	return sh4->program->read_word(A & AM);
}

const UINT32 RL(sh4_state *sh4, offs_t A)
{
	if (A >= 0xe0000000)
		return sh4->program->read_dword(A);

	return sh4->program->read_dword(A & AM);
}

const void WB(sh4_state *sh4, offs_t A, UINT8 V)
{
	if (A >= 0xe0000000)
	{
		sh4->program->write_byte(A,V);
		return;
	}

	sh4->program->write_byte(A & AM,V);
}

const void WW(sh4_state *sh4, offs_t A, UINT16 V)
{
	if (A >= 0xe0000000)
	{
		sh4->program->write_word(A,V);
		return;
	}

	sh4->program->write_word(A & AM,V);
}

const void WL(sh4_state *sh4, offs_t A, UINT32 V)
{
	if (A >= 0xe0000000)
	{
		sh4->program->write_dword(A,V);
		return;
	}

	sh4->program->write_dword(A & AM,V);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1100  1       -
 *  ADD     Rm,Rn
 */
const void ADD(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] += sh4->r[Rm];
}

/*  code                 cycles  t-bit
 *  0111 nnnn iiii iiii  1       -
 *  ADD     #imm,Rn
 */
const void ADDI(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] += (INT32)(INT16)(INT8)(opcode&0xff);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1110  1       carry
 *  ADDC    Rm,Rn
 */
const void ADDC(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;
	UINT32 tmp0, tmp1;

	tmp1 = sh4->r[n] + sh4->r[m];
	tmp0 = sh4->r[n];
	sh4->r[n] = tmp1 + (sh4->sr & T);
	if (tmp0 > tmp1)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
	if (tmp1 > sh4->r[n])
		sh4->sr |= T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1111  1       overflow
 *  ADDV    Rm,Rn
 */
const void ADDV(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;
	INT32 dest, src, ans;

	if ((INT32) sh4->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh4->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh4->r[n] += sh4->r[m];
	if ((INT32) sh4->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 0 || src == 2)
	{
		if (ans == 1)
			sh4->sr |= T;
		else
			sh4->sr &= ~T;
	}
	else
		sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1001  1       -
 *  AND     Rm,Rn
 */
const void AND(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] &= sh4->r[Rm];
}


/*  code                 cycles  t-bit
 *  1100 1001 iiii iiii  1       -
 *  AND     #imm,R0
 */
const void ANDI(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[0] &= (opcode&0xff);
}

/*  code                 cycles  t-bit
 *  1100 1101 iiii iiii  1       -
 *  AND.B   #imm,@(R0,GBR)
 */
const void ANDM(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 temp;

	sh4->ea = sh4->gbr + sh4->r[0];
	temp = (opcode&0xff) & RB(sh4,  sh4->ea );
	WB(sh4, sh4->ea, temp );
	sh4->sh4_icount -= 2;
}

/*  code                 cycles  t-bit
 *  1000 1011 dddd dddd  3/1     -
 *  BF      disp8
 */
const void BF(sh4_state *sh4, const UINT16 opcode)
{
	if ((sh4->sr & T) == 0)
	{
		INT32 disp = ((INT32)(opcode&0xff) << 24) >> 24;
		sh4->pc = sh4->ea = sh4->pc + disp * 2 + 2;
		sh4->sh4_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1111 dddd dddd  3/1     -
 *  BFS     disp8
 */
const void BFS(sh4_state *sh4, const UINT16 opcode)
{
	if ((sh4->sr & T) == 0)
	{
		INT32 disp = ((INT32)(opcode&0xff) << 24) >> 24;
		sh4->delay = sh4->pc;
		sh4->pc = sh4->ea = sh4->pc + disp * 2 + 2;
		sh4->sh4_icount--;
	}
}

/*  code                 cycles  t-bit
 *  1010 dddd dddd dddd  2       -
 *  BRA     disp12
 */
const void BRA(sh4_state *sh4, const UINT16 opcode)
{
	INT32 disp = ((INT32)(opcode&0xfff) << 20) >> 20;

#if BUSY_LOOP_HACKS
	if (disp == -2)
	{
		UINT32 next_opcode = RW(sh4,sh4->ppc & AM);
		/* BRA  $
		 * NOP
		 */
		if (next_opcode == 0x0009)
			sh4->sh4_icount %= 3;   /* cycles for BRA $ and NOP taken (3) */
	}
#endif
	sh4->delay = sh4->pc;
	sh4->pc = sh4->ea = sh4->pc + disp * 2 + 2;
	sh4->sh4_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0010 0011  2       -
 *  BRAF    Rm
 */
const void BRAF(sh4_state *sh4, const UINT16 opcode)
{
	sh4->delay = sh4->pc;
	sh4->pc += sh4->r[Rn] + 2;
	sh4->sh4_icount--;
}

/*  code                 cycles  t-bit
 *  1011 dddd dddd dddd  2       -
 *  BSR     disp12
 */
const void BSR(sh4_state *sh4, const UINT16 opcode)
{
	INT32 disp = ((INT32)(opcode&0xfff) << 20) >> 20;

	sh4->pr = sh4->pc + 2;
	sh4->delay = sh4->pc;
	sh4->pc = sh4->ea = sh4->pc + disp * 2 + 2;
	sh4->sh4_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0000 0011  2       -
 *  BSRF    Rm
 */
const void BSRF(sh4_state *sh4, const UINT16 opcode)
{
	sh4->pr = sh4->pc + 2;
	sh4->delay = sh4->pc;
	sh4->pc += sh4->r[Rn] + 2;
	sh4->sh4_icount--;
}

/*  code                 cycles  t-bit
 *  1000 1001 dddd dddd  3/1     -
 *  BT      disp8
 */
const void BT(sh4_state *sh4, const UINT16 opcode)
{
	if ((sh4->sr & T) != 0)
	{
		INT32 disp = ((INT32)(opcode&0xff) << 24) >> 24;
		sh4->pc = sh4->ea = sh4->pc + disp * 2 + 2;
		sh4->sh4_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1101 dddd dddd  2/1     -
 *  BTS     disp8
 */
const void BTS(sh4_state *sh4, const UINT16 opcode)
{
	if ((sh4->sr & T) != 0)
	{
		INT32 disp = ((INT32)(opcode&0xff) << 24) >> 24;
		sh4->delay = sh4->pc;
		sh4->pc = sh4->ea = sh4->pc + disp * 2 + 2;
		sh4->sh4_icount--;
	}
}

/*  code                 cycles  t-bit
 *  0000 0000 0010 1000  1       -
 *  CLRMAC
 */
const void CLRMAC(sh4_state *sh4, const UINT16 opcode)
{
	sh4->mach = 0;
	sh4->macl = 0;
}

/*  code                 cycles  t-bit
 *  0000 0000 0000 1000  1       -
 *  CLRT
 */
const void CLRT(sh4_state *sh4, const UINT16 opcode)
{
	sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0000  1       comparison result
 *  CMP_EQ  Rm,Rn
 */
const void CMPEQ(sh4_state *sh4, const UINT16 opcode)
{
	if (sh4->r[Rn] == sh4->r[Rm])
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0011  1       comparison result
 *  CMP_GE  Rm,Rn
 */
const void CMPGE(sh4_state *sh4, const UINT16 opcode)
{
	if ((INT32) sh4->r[Rn] >= (INT32) sh4->r[Rm])
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0111  1       comparison result
 *  CMP_GT  Rm,Rn
 */
const void CMPGT(sh4_state *sh4, const UINT16 opcode)
{
	if ((INT32) sh4->r[Rn] > (INT32) sh4->r[Rm])
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0110  1       comparison result
 *  CMP_HI  Rm,Rn
 */
const void CMPHI(sh4_state *sh4, const UINT16 opcode)
{
	if ((UINT32) sh4->r[Rn] > (UINT32) sh4->r[Rm])
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0010  1       comparison result
 *  CMP_HS  Rm,Rn
 */
const void CMPHS(sh4_state *sh4, const UINT16 opcode)
{
	if ((UINT32) sh4->r[Rn] >= (UINT32) sh4->r[Rm])
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}


/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0101  1       comparison result
 *  CMP_PL  Rn
 */
const void CMPPL(sh4_state *sh4, const UINT16 opcode)
{
	if ((INT32) sh4->r[Rn] > 0)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0001  1       comparison result
 *  CMP_PZ  Rn
 */
const void CMPPZ(sh4_state *sh4, const UINT16 opcode)
{
	if ((INT32) sh4->r[Rn] >= 0)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1100  1       comparison result
 * CMP_STR  Rm,Rn
 */
const void CMPSTR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 temp;
	INT32 HH, HL, LH, LL;
	temp = sh4->r[Rn] ^ sh4->r[Rm];
	HH = (temp >> 24) & 0xff;
	HL = (temp >> 16) & 0xff;
	LH = (temp >> 8) & 0xff;
	LL = temp & 0xff;
	if (HH && HL && LH && LL)
	sh4->sr &= ~T;
	else
	sh4->sr |= T;
	}


/*  code                 cycles  t-bit
 *  1000 1000 iiii iiii  1       comparison result
 *  CMP/EQ #imm,R0
 */
const void CMPIM(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 imm = (UINT32)(INT32)(INT16)(INT8)(opcode&0xff);

	if (sh4->r[0] == imm)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 0111  1       calculation result
 *  DIV0S   Rm,Rn
 */
const void DIV0S(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if ((sh4->r[n] & 0x80000000) == 0)
		sh4->sr &= ~Q;
	else
		sh4->sr |= Q;
	if ((sh4->r[m] & 0x80000000) == 0)
		sh4->sr &= ~M;
	else
		sh4->sr |= M;
	if ((sh4->r[m] ^ sh4->r[n]) & 0x80000000)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0000 0000 0001 1001  1       0
 *  DIV0U
 */
const void DIV0U(sh4_state *sh4, const UINT16 opcode)
{
	sh4->sr &= ~(M | Q | T);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0100  1       calculation result
 *  DIV1 Rm,Rn
 */
const void DIV1(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 tmp0;
	UINT32 old_q;

	old_q = sh4->sr & Q;
	if (0x80000000 & sh4->r[n])
		sh4->sr |= Q;
	else
		sh4->sr &= ~Q;

	sh4->r[n] = (sh4->r[n] << 1) | (sh4->sr & T);

	if (!old_q)
	{
		if (!(sh4->sr & M))
		{
			tmp0 = sh4->r[n];
			sh4->r[n] -= sh4->r[m];
			if(!(sh4->sr & Q))
				if(sh4->r[n] > tmp0)
					sh4->sr |= Q;
				else
					sh4->sr &= ~Q;
			else
				if(sh4->r[n] > tmp0)
					sh4->sr &= ~Q;
				else
					sh4->sr |= Q;
		}
		else
		{
			tmp0 = sh4->r[n];
			sh4->r[n] += sh4->r[m];
			if(!(sh4->sr & Q))
			{
				if(sh4->r[n] < tmp0)
					sh4->sr &= ~Q;
				else
					sh4->sr |= Q;
			}
			else
			{
				if(sh4->r[n] < tmp0)
					sh4->sr |= Q;
				else
					sh4->sr &= ~Q;
			}
		}
	}
	else
	{
		if (!(sh4->sr & M))
		{
			tmp0 = sh4->r[n];
			sh4->r[n] += sh4->r[m];
			if(!(sh4->sr & Q))
				if(sh4->r[n] < tmp0)
					sh4->sr |= Q;
				else
					sh4->sr &= ~Q;
			else
				if(sh4->r[n] < tmp0)
					sh4->sr &= ~Q;
				else
					sh4->sr |= Q;
		}
		else
		{
			tmp0 = sh4->r[n];
			sh4->r[n] -= sh4->r[m];
			if(!(sh4->sr & Q))
				if(sh4->r[n] > tmp0)
					sh4->sr &= ~Q;
				else
					sh4->sr |= Q;
			else
				if(sh4->r[n] > tmp0)
					sh4->sr |= Q;
				else
					sh4->sr &= ~Q;
		}
	}

	tmp0 = (sh4->sr & (Q | M));
	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  DMULS.L Rm,Rn */
const void DMULS(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) sh4->r[n];
	tempm = (INT32) sh4->r[m];
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	if ((INT32) (sh4->r[n] ^ sh4->r[m]) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
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
	sh4->mach = Res2;
	sh4->macl = Res0;
	sh4->sh4_icount--;
}

/*  DMULU.L Rm,Rn */
const void DMULU(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;

	RnL = sh4->r[n] & 0x0000ffff;
	RnH = (sh4->r[n] >> 16) & 0x0000ffff;
	RmL = sh4->r[m] & 0x0000ffff;
	RmH = (sh4->r[m] >> 16) & 0x0000ffff;
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
	sh4->mach = Res2;
	sh4->macl = Res0;
	sh4->sh4_icount--;
}

/*  DT      Rn */
const void DT(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n]--;
	if (sh4->r[n] == 0)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
#if BUSY_LOOP_HACKS
	{
		UINT32 next_opcode = RW(sh4,sh4->ppc & AM);
		/* DT   Rn
		 * BF   $-2
		 */
		if (next_opcode == 0x8bfd)
		{
			while (sh4->r[n] > 1 && sh4->sh4_icount > 4)
			{
				sh4->r[n]--;
				sh4->sh4_icount -= 4;   /* cycles for DT (1) and BF taken (3) */
			}
		}
	}
#endif
}

/*  EXTS.B  Rm,Rn */
const void EXTSB(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = ((INT32)sh4->r[Rm] << 24) >> 24;
}

/*  EXTS.W  Rm,Rn */
const void EXTSW(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = ((INT32)sh4->r[Rm] << 16) >> 16;
}

/*  EXTU.B  Rm,Rn */
const void EXTUB(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->r[Rm] & 0x000000ff;
}

/*  EXTU.W  Rm,Rn */
const void EXTUW(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->r[Rm] & 0x0000ffff;
}

/*  JMP     @Rm */
const void JMP(sh4_state *sh4, const UINT16 opcode)
{
	sh4->delay = sh4->pc;
	sh4->pc = sh4->ea = sh4->r[Rn];
}

/*  JSR     @Rm */
const void JSR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->delay = sh4->pc;
	sh4->pr = sh4->pc + 2;
	sh4->pc = sh4->ea = sh4->r[Rn];
	sh4->sh4_icount--;
}


/*  LDC     Rm,SR */
const void LDCSR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 reg;

	reg = sh4->r[Rn];
	if ((sh4->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank(sh4, (sh4->sr & sRB) >> 29);
	if ((sh4->r[Rn] & sRB) != (sh4->sr & sRB))
		sh4_change_register_bank(sh4, sh4->r[Rn] & sRB ? 1 : 0);
	sh4->sr = reg & FLAGS;
	sh4_exception_recompute(sh4);
}

/*  LDC     Rm,GBR */
const void LDCGBR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->gbr = sh4->r[Rn];
}

/*  LDC     Rm,VBR */
const void LDCVBR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->vbr = sh4->r[Rn];
}

/*  LDC.L   @Rm+,SR */
const void LDCMSR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 old;

	old = sh4->sr;
	sh4->ea = sh4->r[Rn];
	sh4->sr = RL(sh4, sh4->ea ) & FLAGS;
	if ((sh4->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank(sh4, (old & sRB) >> 29);
	if ((old & sRB) != (sh4->sr & sRB))
		sh4_change_register_bank(sh4, sh4->sr & sRB ? 1 : 0);
	sh4->r[Rn] += 4;
	sh4->sh4_icount -= 2;
	sh4_exception_recompute(sh4);
}

/*  LDC.L   @Rm+,GBR */
const void LDCMGBR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	sh4->gbr = RL(sh4, sh4->ea );
	sh4->r[Rn] += 4;
	sh4->sh4_icount -= 2;
}

/*  LDC.L   @Rm+,VBR */
const void LDCMVBR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	sh4->vbr = RL(sh4, sh4->ea );
	sh4->r[Rn] += 4;
	sh4->sh4_icount -= 2;
}

/*  LDS     Rm,MACH */
const void LDSMACH(sh4_state *sh4, const UINT16 opcode)
{
	sh4->mach = sh4->r[Rn];
}

/*  LDS     Rm,MACL */
const void LDSMACL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->macl = sh4->r[Rn];
}

/*  LDS     Rm,PR */
const void LDSPR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->pr = sh4->r[Rn];
}

/*  LDS.L   @Rm+,MACH */
const void LDSMMACH(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	sh4->mach = RL(sh4, sh4->ea );
	sh4->r[Rn] += 4;
}

/*  LDS.L   @Rm+,MACL */
const void LDSMMACL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	sh4->macl = RL(sh4, sh4->ea );
	sh4->r[Rn] += 4;
}

/*  LDS.L   @Rm+,PR */
const void LDSMPR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	sh4->pr = RL(sh4, sh4->ea );
	sh4->r[Rn] += 4;
}

/*  MAC.L   @Rm+,@Rn+ */
const void MAC_L(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) RL(sh4, sh4->r[n] );
	sh4->r[n] += 4;
	tempm = (INT32) RL(sh4, sh4->r[m] );
	sh4->r[m] += 4;
	if ((INT32) (tempn ^ tempm) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
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
	if (sh4->sr & S)
	{
		Res0 = sh4->macl + Res0;
		if (sh4->macl > Res0)
			Res2++;
		Res2 += (sh4->mach & 0x0000ffff);
		if (((INT32) Res2 < 0) && (Res2 < 0xffff8000))
		{
			Res2 = 0x00008000;
			Res0 = 0x00000000;
		}
		else if (((INT32) Res2 > 0) && (Res2 > 0x00007fff))
		{
			Res2 = 0x00007fff;
			Res0 = 0xffffffff;
		}
		sh4->mach = Res2;
		sh4->macl = Res0;
	}
	else
	{
		Res0 = sh4->macl + Res0;
		if (sh4->macl > Res0)
			Res2++;
		Res2 += sh4->mach;
		sh4->mach = Res2;
		sh4->macl = Res0;
	}
	sh4->sh4_icount -= 2;
}

/*  MAC.W   @Rm+,@Rn+ */
const void MAC_W(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	INT32 tempm, tempn, dest, src, ans;
	UINT32 templ;

	tempn = (INT32) RW(sh4, sh4->r[n] );
	sh4->r[n] += 2;
	tempm = (INT32) RW(sh4, sh4->r[m] );
	sh4->r[m] += 2;
	templ = sh4->macl;
	tempm = ((INT32) (short) tempn * (INT32) (short) tempm);
	if ((INT32) sh4->macl >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) tempm >= 0)
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
	sh4->macl += tempm;
	if ((INT32) sh4->macl >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (sh4->sr & S)
	{
		if (ans == 1)
			{
				if (src == 0)
					sh4->macl = 0x7fffffff;
				if (src == 2)
					sh4->macl = 0x80000000;
			}
	}
	else
	{
		sh4->mach += tempn;
		if (templ > sh4->macl)
			sh4->mach += 1;
	}
	sh4->sh4_icount -= 2;
}

/*  MOV     Rm,Rn */
const void MOV(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->r[Rm];
}

/*  MOV.B   Rm,@Rn */
const void MOVBS(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	WB(sh4, sh4->ea, sh4->r[Rm] & 0x000000ff);
}

/*  MOV.W   Rm,@Rn */
const void MOVWS(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	WW(sh4, sh4->ea, sh4->r[Rm] & 0x0000ffff);
}

/*  MOV.L   Rm,@Rn */
const void MOVLS(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	WL(sh4, sh4->ea, sh4->r[Rm] );
}

/*  MOV.B   @Rm,Rn */
const void MOVBL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rm];
	sh4->r[Rn] = (UINT32)(INT32)(INT16)(INT8) RB(sh4,  sh4->ea );
}

/*  MOV.W   @Rm,Rn */
const void MOVWL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rm];
	sh4->r[Rn] = (UINT32)(INT32)(INT16) RW(sh4, sh4->ea );
}

/*  MOV.L   @Rm,Rn */
const void MOVLL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rm];
	sh4->r[Rn] = RL(sh4, sh4->ea );
}

/*  MOV.B   Rm,@-Rn */
const void MOVBM(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 data = sh4->r[Rm] & 0x000000ff;

	sh4->r[Rn] -= 1;
	WB(sh4, sh4->r[Rn], data );
}

/*  MOV.W   Rm,@-Rn */
const void MOVWM(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 data = sh4->r[Rm] & 0x0000ffff;

	sh4->r[Rn] -= 2;
	WW(sh4, sh4->r[Rn], data );
}

/*  MOV.L   Rm,@-Rn */
const void MOVLM(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 data = sh4->r[Rm];

	sh4->r[Rn] -= 4;
	WL(sh4, sh4->r[Rn], data );
}

/*  MOV.B   @Rm+,Rn */
const void MOVBP(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	sh4->r[n] = (UINT32)(INT32)(INT16)(INT8) RB(sh4,  sh4->r[m] );
	if (n != m)
		sh4->r[m] += 1;
}

/*  MOV.W   @Rm+,Rn */
const void MOVWP(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	sh4->r[n] = (UINT32)(INT32)(INT16) RW(sh4, sh4->r[m] );
	if (n != m)
		sh4->r[m] += 2;
}

/*  MOV.L   @Rm+,Rn */
const void MOVLP(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	sh4->r[n] = RL(sh4, sh4->r[m] );
	if (n != m)
		sh4->r[m] += 4;
}

/*  MOV.B   Rm,@(R0,Rn) */
const void MOVBS0(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn] + sh4->r[0];
	WB(sh4, sh4->ea, sh4->r[Rm] & 0x000000ff );
}

/*  MOV.W   Rm,@(R0,Rn) */
const void MOVWS0(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn] + sh4->r[0];
	WW(sh4, sh4->ea, sh4->r[Rm] & 0x0000ffff );
}

/*  MOV.L   Rm,@(R0,Rn) */
const void MOVLS0(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn] + sh4->r[0];
	WL(sh4, sh4->ea, sh4->r[Rm] );
}

/*  MOV.B   @(R0,Rm),Rn */
const void MOVBL0(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rm] + sh4->r[0];
	sh4->r[Rn] = (UINT32)(INT32)(INT16)(INT8) RB(sh4,  sh4->ea );
}

/*  MOV.W   @(R0,Rm),Rn */
const void MOVWL0(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rm] + sh4->r[0];
	sh4->r[Rn] = (UINT32)(INT32)(INT16) RW(sh4, sh4->ea );
}

/*  MOV.L   @(R0,Rm),Rn */
const void MOVLL0(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rm] + sh4->r[0];
	sh4->r[Rn] = RL(sh4, sh4->ea );
}

/*  MOV     #imm,Rn */
const void MOVI(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = (UINT32)(INT32)(INT16)(INT8)(opcode&0xff);
}

/*  MOV.W   @(disp8,PC),Rn */
const void MOVWI(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	sh4->ea = sh4->pc + disp * 2 + 2;
	sh4->r[Rn] = (UINT32)(INT32)(INT16) RW(sh4, sh4->ea );
}

/*  MOV.L   @(disp8,PC),Rn */
const void MOVLI(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	sh4->ea = ((sh4->pc + 2) & ~3) + disp * 4;
	sh4->r[Rn] = RL(sh4, sh4->ea );
}

/*  MOV.B   @(disp8,GBR),R0 */
const void MOVBLG(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	sh4->ea = sh4->gbr + disp;
	sh4->r[0] = (UINT32)(INT32)(INT16)(INT8) RB(sh4,  sh4->ea );
}

/*  MOV.W   @(disp8,GBR),R0 */
const void MOVWLG(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	sh4->ea = sh4->gbr + disp * 2;
	sh4->r[0] = (INT32)(INT16) RW(sh4, sh4->ea );
}

/*  MOV.L   @(disp8,GBR),R0 */
const void MOVLLG(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	sh4->ea = sh4->gbr + disp * 4;
	sh4->r[0] = RL(sh4, sh4->ea );
}

/*  MOV.B   R0,@(disp8,GBR) */
const void MOVBSG(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	sh4->ea = sh4->gbr + disp;
	WB(sh4, sh4->ea, sh4->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp8,GBR) */
const void MOVWSG(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	sh4->ea = sh4->gbr + disp * 2;
	WW(sh4, sh4->ea, sh4->r[0] & 0x0000ffff );
}

/*  MOV.L   R0,@(disp8,GBR) */
const void MOVLSG(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	sh4->ea = sh4->gbr + disp * 4;
	WL(sh4, sh4->ea, sh4->r[0] );
}

/*  MOV.B   R0,@(disp4,Rm) */
const void MOVBS4(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	sh4->ea = sh4->r[Rm] + disp;
	WB(sh4, sh4->ea, sh4->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp4,Rm) */
const void MOVWS4(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	sh4->ea = sh4->r[Rm] + disp * 2;
	WW(sh4, sh4->ea, sh4->r[0] & 0x0000ffff );
}

/* MOV.L Rm,@(disp4,Rn) */
const void MOVLS4(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	sh4->ea = sh4->r[Rn] + disp * 4;
	WL(sh4, sh4->ea, sh4->r[Rm] );
}

/*  MOV.B   @(disp4,Rm),R0 */
const void MOVBL4(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	sh4->ea = sh4->r[Rm] + disp;
	sh4->r[0] = (UINT32)(INT32)(INT16)(INT8) RB(sh4,  sh4->ea );
}

/*  MOV.W   @(disp4,Rm),R0 */
const void MOVWL4(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	sh4->ea = sh4->r[Rm] + disp * 2;
	sh4->r[0] = (UINT32)(INT32)(INT16) RW(sh4, sh4->ea );
}

/*  MOV.L   @(disp4,Rm),Rn */
const void MOVLL4(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0x0f;
	sh4->ea = sh4->r[Rm] + disp * 4;
	sh4->r[Rn] = RL(sh4, sh4->ea );
}

/*  MOVA    @(disp8,PC),R0 */
const void MOVA(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 disp = opcode & 0xff;
	sh4->ea = ((sh4->pc + 2) & ~3) + disp * 4;
	sh4->r[0] = sh4->ea;
}

/*  MOVT    Rn */
const void MOVT(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->sr & T;
}

/*  MUL.L   Rm,Rn */
const void MULL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->macl = sh4->r[Rn] * sh4->r[Rm];
	sh4->sh4_icount--;
}

/*  MULS    Rm,Rn */
const void MULS(sh4_state *sh4, const UINT16 opcode)
{
	sh4->macl = (INT16) sh4->r[Rn] * (INT16) sh4->r[Rm];
}

/*  MULU    Rm,Rn */
const void MULU(sh4_state *sh4, const UINT16 opcode)
{
	sh4->macl = (UINT16) sh4->r[Rn] * (UINT16) sh4->r[Rm];
}

/*  NEG     Rm,Rn */
const void NEG(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = 0 - sh4->r[Rm];
}

/*  NEGC    Rm,Rn */
const void NEGC(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 temp;

	temp = sh4->r[Rm];
	sh4->r[Rn] = -temp - (sh4->sr & T);
	if (temp || (sh4->sr & T))
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  NOP */
const void NOP(sh4_state *sh4, const UINT16 opcode)
{
}

/*  NOT     Rm,Rn */
const void NOT(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = ~sh4->r[Rm];
}

/*  OR      Rm,Rn */
const void OR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] |= sh4->r[Rm];
}

/*  OR      #imm,R0 */
const void ORI(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[0] |= (opcode&0xff);
	sh4->sh4_icount -= 2;
}

/*  OR.B    #imm,@(R0,GBR) */
const void ORM(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 temp;

	sh4->ea = sh4->gbr + sh4->r[0];
	temp = RB(sh4,  sh4->ea );
	temp |= (opcode&0xff);
	WB(sh4, sh4->ea, temp );
}

/*  ROTCL   Rn */
const void ROTCL(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	UINT32 temp;

	temp = (sh4->r[n] >> 31) & T;
	sh4->r[n] = (sh4->r[n] << 1) | (sh4->sr & T);
	sh4->sr = (sh4->sr & ~T) | temp;
}

/*  ROTCR   Rn */
const void ROTCR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	UINT32 temp;
	temp = (sh4->sr & T) << 31;
	if (sh4->r[n] & T)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
	sh4->r[n] = (sh4->r[n] >> 1) | temp;
}

/*  ROTL    Rn */
const void ROTL(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->sr = (sh4->sr & ~T) | ((sh4->r[n] >> 31) & T);
	sh4->r[n] = (sh4->r[n] << 1) | (sh4->r[n] >> 31);
}

/*  ROTR    Rn */
const void ROTR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->sr = (sh4->sr & ~T) | (sh4->r[n] & T);
	sh4->r[n] = (sh4->r[n] >> 1) | (sh4->r[n] << 31);
}

/*  RTE */
const void RTE(sh4_state *sh4, const UINT16 opcode)
{
	sh4->delay = sh4->pc;
	sh4->pc = sh4->ea = sh4->spc;
	if ((sh4->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank(sh4, (sh4->sr & sRB) >> 29);
	if ((sh4->ssr & sRB) != (sh4->sr & sRB))
		sh4_change_register_bank(sh4, sh4->ssr & sRB ? 1 : 0);
	sh4->sr = sh4->ssr;
	sh4->sh4_icount--;
	sh4_exception_recompute(sh4);
}

/*  RTS */
const void RTS(sh4_state *sh4, const UINT16 opcode)
{
	sh4->delay = sh4->pc;
	sh4->pc = sh4->ea = sh4->pr;
	sh4->sh4_icount--;
}

/*  SETT */
const void SETT(sh4_state *sh4, const UINT16 opcode)
{
	sh4->sr |= T;
}

/*  SHAL    Rn      (same as SHLL) */
const void SHAL(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->sr = (sh4->sr & ~T) | ((sh4->r[n] >> 31) & T);
	sh4->r[n] <<= 1;
}

/*  SHAR    Rn */
const void SHAR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->sr = (sh4->sr & ~T) | (sh4->r[n] & T);
	sh4->r[n] = (UINT32)((INT32)sh4->r[n] >> 1);
}

/*  SHLL    Rn      (same as SHAL) */
const void SHLL(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->sr = (sh4->sr & ~T) | ((sh4->r[n] >> 31) & T);
	sh4->r[n] <<= 1;
}

/*  SHLL2   Rn */
const void SHLL2(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] <<= 2;
}

/*  SHLL8   Rn */
const void SHLL8(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] <<= 8;
}

/*  SHLL16  Rn */
const void SHLL16(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] <<= 16;
}

/*  SHLR    Rn */
const void SHLR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->sr = (sh4->sr & ~T) | (sh4->r[n] & T);
	sh4->r[n] >>= 1;
}

/*  SHLR2   Rn */
const void SHLR2(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] >>= 2;
}

/*  SHLR8   Rn */
const void SHLR8(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] >>= 8;
}

/*  SHLR16  Rn */
const void SHLR16(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] >>= 16;
}

/*  SLEEP */
const void SLEEP(sh4_state *sh4, const UINT16 opcode)
{
	/* 0 = normal mode */
	/* 1 = enters into power-down mode */
	/* 2 = go out the power-down mode after an exception */
	if(sh4->sleep_mode != 2)
		sh4->pc -= 2;
	sh4->sh4_icount -= 2;
	/* Wait_for_exception; */
	if(sh4->sleep_mode == 0)
		sh4->sleep_mode = 1;
	else if(sh4->sleep_mode == 2)
		sh4->sleep_mode = 0;
}

/*  STC     SR,Rn */
const void STCSR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->sr;
}

/*  STC     GBR,Rn */
const void STCGBR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->gbr;
}

/*  STC     VBR,Rn */
const void STCVBR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->vbr;
}

/*  STC.L   SR,@-Rn */
const void STCMSR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->sr );
	sh4->sh4_icount--;
}

/*  STC.L   GBR,@-Rn */
const void STCMGBR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->gbr );
	sh4->sh4_icount--;
}

/*  STC.L   VBR,@-Rn */
const void STCMVBR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->vbr );
	sh4->sh4_icount--;
}

/*  STS     MACH,Rn */
const void STSMACH(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->mach;
}

/*  STS     MACL,Rn */
const void STSMACL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->macl;
}

/*  STS     PR,Rn */
const void STSPR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->pr;
}

/*  STS.L   MACH,@-Rn */
const void STSMMACH(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->mach );
}

/*  STS.L   MACL,@-Rn */
const void STSMMACL(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->macl );
}

/*  STS.L   PR,@-Rn */
const void STSMPR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->pr );
}

/*  SUB     Rm,Rn */
const void SUB(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] -= sh4->r[Rm];
}

/*  SUBC    Rm,Rn */
const void SUBC(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 tmp0, tmp1;

	tmp1 = sh4->r[n] - sh4->r[m];
	tmp0 = sh4->r[n];
	sh4->r[n] = tmp1 - (sh4->sr & T);
	if (tmp0 < tmp1)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
	if (tmp1 < sh4->r[n])
		sh4->sr |= T;
}

/*  SUBV    Rm,Rn */
const void SUBV(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	INT32 dest, src, ans;

	if ((INT32) sh4->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh4->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh4->r[n] -= sh4->r[m];
	if ((INT32) sh4->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 1)
	{
		if (ans == 1)
			sh4->sr |= T;
		else
			sh4->sr &= ~T;
	}
	else
		sh4->sr &= ~T;
}

/*  SWAP.B  Rm,Rn */
const void SWAPB(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 temp0, temp1;

	temp0 = sh4->r[m] & 0xffff0000;
	temp1 = (sh4->r[m] & 0x000000ff) << 8;
	sh4->r[n] = (sh4->r[m] >> 8) & 0x000000ff;
	sh4->r[n] = sh4->r[n] | temp1 | temp0;
}

/*  SWAP.W  Rm,Rn */
const void SWAPW(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 temp;

	temp = (sh4->r[m] >> 16) & 0x0000ffff;
	sh4->r[n] = (sh4->r[m] << 16) | temp;
}

/*  TAS.B   @Rn */
const void TAS(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	UINT32 temp;
	sh4->ea = sh4->r[n];
	/* Bus Lock enable */
	temp = RB(sh4,  sh4->ea );
	if (temp == 0)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
	temp |= 0x80;
	/* Bus Lock disable */
	WB(sh4, sh4->ea, temp );
	sh4->sh4_icount -= 3;
}

/*  TRAPA   #imm */
const void TRAPA(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;

	if (sh4->cpu_type == CPU_TYPE_SH4)
	{
		sh4->m[TRA] = imm << 2;
	}
	else /* SH3 */
	{
		sh4->m_sh3internal_upper[SH3_TRA_ADDR] = imm << 2;
	}


	sh4->ssr = sh4->sr;
	sh4->spc = sh4->pc;
	sh4->sgr = sh4->r[15];

	sh4->sr |= MD;
	if ((sh4->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank(sh4, (sh4->sr & sRB) >> 29);
	if (!(sh4->sr & sRB))
		sh4_change_register_bank(sh4, 1);
	sh4->sr |= sRB;
	sh4->sr |= BL;
	sh4_exception_recompute(sh4);

	if (sh4->cpu_type == CPU_TYPE_SH4)
	{
		sh4->m[EXPEVT] = 0x00000160;
	}
	else /* SH3 */
	{
		sh4->m_sh3internal_upper[SH3_EXPEVT_ADDR] = 0x00000160;
	}

	sh4->pc = sh4->vbr + 0x00000100;

	sh4->sh4_icount -= 7;
}

/*  TST     Rm,Rn */
const void TST(sh4_state *sh4, const UINT16 opcode)
{
	if ((sh4->r[Rn] & sh4->r[Rm]) == 0)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  TST     #imm,R0 */
const void TSTI(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;

	if ((imm & sh4->r[0]) == 0)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
}

/*  TST.B   #imm,@(R0,GBR) */
const void TSTM(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;

	sh4->ea = sh4->gbr + sh4->r[0];
	if ((imm & RB(sh4,  sh4->ea )) == 0)
		sh4->sr |= T;
	else
		sh4->sr &= ~T;
	sh4->sh4_icount -= 2;
}

/*  XOR     Rm,Rn */
const void XOR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] ^= sh4->r[Rm];
}

/*  XOR     #imm,R0 */
const void XORI(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;
	sh4->r[0] ^= imm;
}

/*  XOR.B   #imm,@(R0,GBR) */
const void XORM(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 imm = opcode & 0xff;
	UINT32 temp;

	sh4->ea = sh4->gbr + sh4->r[0];
	temp = RB(sh4,  sh4->ea );
	temp ^= imm;
	WB(sh4, sh4->ea, temp );
	sh4->sh4_icount -= 2;
}

/*  XTRCT   Rm,Rn */
const void XTRCT(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	UINT32 temp;

	temp = (sh4->r[m] << 16) & 0xffff0000;
	sh4->r[n] = (sh4->r[n] >> 16) & 0x0000ffff;
	sh4->r[n] |= temp;
}

/*  STC     SSR,Rn */
const void STCSSR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->ssr;
}

/*  STC     SPC,Rn */
const void STCSPC(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->spc;
}

/*  STC     SGR,Rn */
const void STCSGR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->sgr;
}

/*  STS     FPUL,Rn */
const void STSFPUL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->fpul;
}

/*  STS     FPSCR,Rn */
const void STSFPSCR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->fpscr & 0x003FFFFF;
}

/*  STC     DBR,Rn */
const void STCDBR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->r[Rn] = sh4->dbr;
}

/*  STCRBANK   Rm_BANK,Rn */
const void STCRBANK(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm;

	sh4->r[Rn] = sh4->rbnk[sh4->sr&sRB ? 0 : 1][m & 7];
}

/*  STCMRBANK   Rm_BANK,@-Rn */
const void STCMRBANK(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->rbnk[sh4->sr&sRB ? 0 : 1][m & 7]);
	sh4->sh4_icount--;
}

/*  MOVCA.L     R0,@Rn */
const void MOVCAL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	WL(sh4, sh4->ea, sh4->r[0] );
}

const void CLRS(sh4_state *sh4, const UINT16 opcode)
{
	sh4->sr &= ~S;
}

const void SETS(sh4_state *sh4, const UINT16 opcode)
{
	sh4->sr |= S;
}

/*  STS.L   SGR,@-Rn */
const void STCMSGR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->sgr );
}

/*  STS.L   FPUL,@-Rn */
const void STSMFPUL(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->fpul );
}

/*  STS.L   FPSCR,@-Rn */
const void STSMFPSCR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->fpscr & 0x003FFFFF);
}

/*  STC.L   DBR,@-Rn */
const void STCMDBR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->dbr );
}

/*  STC.L   SSR,@-Rn */
const void STCMSSR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->ssr );
}

/*  STC.L   SPC,@-Rn */
const void STCMSPC(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	sh4->r[n] -= 4;
	sh4->ea = sh4->r[n];
	WL(sh4, sh4->ea, sh4->spc );
}

/*  LDS.L   @Rm+,FPUL */
const void LDSMFPUL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	sh4->fpul = RL(sh4, sh4->ea );
	sh4->r[Rn] += 4;
}

/*  LDS.L   @Rm+,FPSCR */
const void LDSMFPSCR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 s;

	s = sh4->fpscr;
	sh4->ea = sh4->r[Rn];
	sh4->fpscr = RL(sh4, sh4->ea );
	sh4->fpscr &= 0x003FFFFF;
	sh4->r[Rn] += 4;
	if ((s & FR) != (sh4->fpscr & FR))
		sh4_swap_fp_registers(sh4);
#ifdef LSB_FIRST
	if ((s & PR) != (sh4->fpscr & PR))
		sh4_swap_fp_couples(sh4);
#endif
	sh4->fpu_sz = (sh4->fpscr & SZ) ? 1 : 0;
	sh4->fpu_pr = (sh4->fpscr & PR) ? 1 : 0;
}

/*  LDC.L   @Rm+,DBR */
const void LDCMDBR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	sh4->dbr = RL(sh4, sh4->ea );
	sh4->r[Rn] += 4;
}

/*  LDC.L   @Rn+,Rm_BANK */
const void LDCMRBANK(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	sh4->ea = sh4->r[n];
	sh4->rbnk[sh4->sr&sRB ? 0 : 1][m & 7] = RL(sh4, sh4->ea );
	sh4->r[n] += 4;
}

/*  LDC.L   @Rm+,SSR */
const void LDCMSSR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	sh4->ssr = RL(sh4, sh4->ea );
	sh4->r[Rn] += 4;
}

/*  LDC.L   @Rm+,SPC */
const void LDCMSPC(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ea = sh4->r[Rn];
	sh4->spc = RL(sh4, sh4->ea );
	sh4->r[Rn] += 4;
}

/*  LDS     Rm,FPUL */
const void LDSFPUL(sh4_state *sh4, const UINT16 opcode)
{
	sh4->fpul = sh4->r[Rn];
}

/*  LDS     Rm,FPSCR */
const void LDSFPSCR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 s;

	s = sh4->fpscr;
	sh4->fpscr = sh4->r[Rn] & 0x003FFFFF;
	if ((s & FR) != (sh4->fpscr & FR))
		sh4_swap_fp_registers(sh4);
#ifdef LSB_FIRST
	if ((s & PR) != (sh4->fpscr & PR))
		sh4_swap_fp_couples(sh4);
#endif
	sh4->fpu_sz = (sh4->fpscr & SZ) ? 1 : 0;
	sh4->fpu_pr = (sh4->fpscr & PR) ? 1 : 0;
}

/*  LDC     Rm,DBR */
const void LDCDBR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->dbr = sh4->r[Rn];
}

/*  SHAD    Rm,Rn */
const void SHAD(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if ((sh4->r[m] & 0x80000000) == 0)
		sh4->r[n] = sh4->r[n] << (sh4->r[m] & 0x1F);
	else if ((sh4->r[m] & 0x1F) == 0) {
		if ((sh4->r[n] & 0x80000000) == 0)
			sh4->r[n] = 0;
		else
			sh4->r[n] = 0xFFFFFFFF;
	} else
		sh4->r[n]=(INT32)sh4->r[n] >> ((~sh4->r[m] & 0x1F)+1);
}

/*  SHLD    Rm,Rn */
const void SHLD(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if ((sh4->r[m] & 0x80000000) == 0)
		sh4->r[n] = sh4->r[n] << (sh4->r[m] & 0x1F);
	else if ((sh4->r[m] & 0x1F) == 0)
		sh4->r[n] = 0;
	else
		sh4->r[n] = sh4->r[n] >> ((~sh4->r[m] & 0x1F)+1);
}

/*  LDCRBANK   Rn,Rm_BANK */
const void LDCRBANK(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm;

	sh4->rbnk[sh4->sr&sRB ? 0 : 1][m & 7] = sh4->r[Rn];
}

/*  LDC     Rm,SSR */
const void LDCSSR(sh4_state *sh4, const UINT16 opcode)
{
	sh4->ssr = sh4->r[Rn];
}

/*  LDC     Rm,SPC */
const void LDCSPC(sh4_state *sh4, const UINT16 opcode)
{
	sh4->spc = sh4->r[Rn];
}

/*  PREF     @Rn */
const void PREFM(sh4_state *sh4, const UINT16 opcode)
{
	int a;
	UINT32 addr,dest,sq;

	addr = sh4->r[Rn]; // address
	if ((addr >= 0xE0000000) && (addr <= 0xE3FFFFFF))
	{
		if (sh4->sh4_mmu_enabled)
		{
			addr = addr & 0xFFFFFFE0;
			dest = sh4_getsqremap(sh4, addr); // good enough for naomi-gd rom, probably not much else

		}
		else
		{
			sq = (addr & 0x20) >> 5;
			dest = addr & 0x03FFFFE0;
			if (sq == 0)
			{
				if (sh4->cpu_type == CPU_TYPE_SH4)
				{
					dest |= (sh4->m[QACR0] & 0x1C) << 24;
				}
				else
				{
					fatalerror("sh4->cpu_type != CPU_TYPE_SH4 but access internal regs\n");
				}
			}
			else
			{
				if (sh4->cpu_type == CPU_TYPE_SH4)
				{
					dest |= (sh4->m[QACR1] & 0x1C) << 24;
				}
				else
				{
					fatalerror("sh4->cpu_type != CPU_TYPE_SH4 but access internal regs\n");
				}

			}
			addr = addr & 0xFFFFFFE0;
		}

		for (a = 0;a < 4;a++)
		{
			// shouldn't be causing a memory read, should store sq writes in registers.
			sh4->program->write_qword(dest, sh4->program->read_qword(addr));
			addr += 8;
			dest += 8;
		}
	}
}

/*****************************************************************************
 *  OPCODE DISPATCHERS
 *****************************************************************************/

















/*  FMOV.S  @Rm+,FRn PR=0 SZ=0 1111nnnnmmmm1001 */
/*  FMOV    @Rm+,DRn PR=0 SZ=1 1111nnn0mmmm1001 */
/*  FMOV    @Rm+,XDn PR=0 SZ=1 1111nnn1mmmm1001 */
/*  FMOV    @Rm+,XDn PR=1      1111nnn1mmmm1001 */
const void FMOVMRIFR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		sh4->ea = sh4->r[m];
		sh4->r[m] += 8;
		sh4->xf[n+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = RL(sh4, sh4->ea );
		sh4->xf[n+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] = RL(sh4, sh4->ea+4 );
	} else {              /* PR = 0 */
		if (sh4->fpu_sz) { /* SZ = 1 */
			if (n & 1) {
				n = n & 14;
				sh4->ea = sh4->r[m];
				sh4->xf[n] = RL(sh4, sh4->ea );
				sh4->r[m] += 4;
				sh4->xf[n+1] = RL(sh4, sh4->ea+4 );
				sh4->r[m] += 4;
			} else {
				sh4->ea = sh4->r[m];
				sh4->fr[n] = RL(sh4, sh4->ea );
				sh4->r[m] += 4;
				sh4->fr[n+1] = RL(sh4, sh4->ea+4 );
				sh4->r[m] += 4;
			}
		} else {              /* SZ = 0 */
			sh4->ea = sh4->r[m];
			sh4->fr[n] = RL(sh4, sh4->ea );
			sh4->r[m] += 4;
		}
	}
}

/*  FMOV.S  FRm,@Rn PR=0 SZ=0 1111nnnnmmmm1010 */
/*  FMOV    DRm,@Rn PR=0 SZ=1 1111nnnnmmm01010 */
/*  FMOV    XDm,@Rn PR=0 SZ=1 1111nnnnmmm11010 */
/*  FMOV    XDm,@Rn PR=1      1111nnnnmmm11010 */
const void FMOVFRMR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		m= m & 14;
		sh4->ea = sh4->r[n];
		WL(sh4, sh4->ea,sh4->xf[m+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] );
		WL(sh4, sh4->ea+4,sh4->xf[m+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] );
	} else {              /* PR = 0 */
		if (sh4->fpu_sz) { /* SZ = 1 */
			if (m & 1) {
				m= m & 14;
				sh4->ea = sh4->r[n];
				WL(sh4, sh4->ea,sh4->xf[m] );
				WL(sh4, sh4->ea+4,sh4->xf[m+1] );
			} else {
				sh4->ea = sh4->r[n];
				WL(sh4, sh4->ea,sh4->fr[m] );
				WL(sh4, sh4->ea+4,sh4->fr[m+1] );
			}
		} else {              /* SZ = 0 */
			sh4->ea = sh4->r[n];
			WL(sh4, sh4->ea,sh4->fr[m] );
		}
	}
}

/*  FMOV.S  FRm,@-Rn PR=0 SZ=0 1111nnnnmmmm1011 */
/*  FMOV    DRm,@-Rn PR=0 SZ=1 1111nnnnmmm01011 */
/*  FMOV    XDm,@-Rn PR=0 SZ=1 1111nnnnmmm11011 */
/*  FMOV    XDm,@-Rn PR=1      1111nnnnmmm11011 */
const void FMOVFRMDR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		m= m & 14;
		sh4->r[n] -= 8;
		sh4->ea = sh4->r[n];
		WL(sh4, sh4->ea,sh4->xf[m+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] );
		WL(sh4, sh4->ea+4,sh4->xf[m+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] );
	} else {              /* PR = 0 */
		if (sh4->fpu_sz) { /* SZ = 1 */
			if (m & 1) {
				m= m & 14;
				sh4->r[n] -= 8;
				sh4->ea = sh4->r[n];
				WL(sh4, sh4->ea,sh4->xf[m] );
				WL(sh4, sh4->ea+4,sh4->xf[m+1] );
			} else {
				sh4->r[n] -= 8;
				sh4->ea = sh4->r[n];
				WL(sh4, sh4->ea,sh4->fr[m] );
				WL(sh4, sh4->ea+4,sh4->fr[m+1] );
			}
		} else {              /* SZ = 0 */
			sh4->r[n] -= 4;
			sh4->ea = sh4->r[n];
			WL(sh4, sh4->ea,sh4->fr[m] );
		}
	}
}

/*  FMOV.S  FRm,@(R0,Rn) PR=0 SZ=0 1111nnnnmmmm0111 */
/*  FMOV    DRm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm00111 */
/*  FMOV    XDm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm10111 */
/*  FMOV    XDm,@(R0,Rn) PR=1      1111nnnnmmm10111 */
const void FMOVFRS0(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		m= m & 14;
		sh4->ea = sh4->r[0] + sh4->r[n];
		WL(sh4, sh4->ea,sh4->xf[m+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] );
		WL(sh4, sh4->ea+4,sh4->xf[m+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] );
	} else {              /* PR = 0 */
		if (sh4->fpu_sz) { /* SZ = 1 */
			if (m & 1) {
				m= m & 14;
				sh4->ea = sh4->r[0] + sh4->r[n];
				WL(sh4, sh4->ea,sh4->xf[m] );
				WL(sh4, sh4->ea+4,sh4->xf[m+1] );
			} else {
				sh4->ea = sh4->r[0] + sh4->r[n];
				WL(sh4, sh4->ea,sh4->fr[m] );
				WL(sh4, sh4->ea+4,sh4->fr[m+1] );
			}
		} else {              /* SZ = 0 */
			sh4->ea = sh4->r[0] + sh4->r[n];
			WL(sh4, sh4->ea,sh4->fr[m] );
		}
	}
}

/*  FMOV.S  @(R0,Rm),FRn PR=0 SZ=0 1111nnnnmmmm0110 */
/*  FMOV    @(R0,Rm),DRn PR=0 SZ=1 1111nnn0mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=0 SZ=1 1111nnn1mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=1      1111nnn1mmmm0110 */
const void FMOVS0FR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n= n & 14;
		sh4->ea = sh4->r[0] + sh4->r[m];
		sh4->xf[n+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = RL(sh4, sh4->ea );
		sh4->xf[n+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] = RL(sh4, sh4->ea+4 );
	} else {              /* PR = 0 */
		if (sh4->fpu_sz) { /* SZ = 1 */
			if (n & 1) {
				n= n & 14;
				sh4->ea = sh4->r[0] + sh4->r[m];
				sh4->xf[n] = RL(sh4, sh4->ea );
				sh4->xf[n+1] = RL(sh4, sh4->ea+4 );
			} else {
				sh4->ea = sh4->r[0] + sh4->r[m];
				sh4->fr[n] = RL(sh4, sh4->ea );
				sh4->fr[n+1] = RL(sh4, sh4->ea+4 );
			}
		} else {              /* SZ = 0 */
			sh4->ea = sh4->r[0] + sh4->r[m];
			sh4->fr[n] = RL(sh4, sh4->ea );
		}
	}
}

/*  FMOV.S  @Rm,FRn PR=0 SZ=0 1111nnnnmmmm1000 */
/*  FMOV    @Rm,DRn PR=0 SZ=1 1111nnn0mmmm1000 */
/*  FMOV    @Rm,XDn PR=0 SZ=1 1111nnn1mmmm1000 */
/*  FMOV    @Rm,XDn PR=1      1111nnn1mmmm1000 */
/*  FMOV    @Rm,DRn PR=1      1111nnn0mmmm1000 */
const void FMOVMRFR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		if (n & 1) {
			n= n & 14;
			sh4->ea = sh4->r[m];
			sh4->xf[n+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = RL(sh4, sh4->ea );
			sh4->xf[n+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] = RL(sh4, sh4->ea+4 );
		} else {
			n= n & 14;
			sh4->ea = sh4->r[m];
			sh4->fr[n+NATIVE_ENDIAN_VALUE_LE_BE(1,0)] = RL(sh4, sh4->ea );
			sh4->fr[n+NATIVE_ENDIAN_VALUE_LE_BE(0,1)] = RL(sh4, sh4->ea+4 );
		}
	} else {              /* PR = 0 */
		if (sh4->fpu_sz) { /* SZ = 1 */
			if (n & 1) {
				n= n & 14;
				sh4->ea = sh4->r[m];
				sh4->xf[n] = RL(sh4, sh4->ea );
				sh4->xf[n+1] = RL(sh4, sh4->ea+4 );
			} else {
				n= n & 14;
				sh4->ea = sh4->r[m];
				sh4->fr[n] = RL(sh4, sh4->ea );
				sh4->fr[n+1] = RL(sh4, sh4->ea+4 );
			}
		} else {              /* SZ = 0 */
			sh4->ea = sh4->r[m];
			sh4->fr[n] = RL(sh4, sh4->ea );
		}
	}
}

/*  FMOV    FRm,FRn PR=0 SZ=0 FRm -> FRn 1111nnnnmmmm1100 */
/*  FMOV    DRm,DRn PR=0 SZ=1 DRm -> DRn 1111nnn0mmm01100 */
/*  FMOV    XDm,DRn PR=1      XDm -> DRn 1111nnn0mmm11100 */
/*  FMOV    DRm,XDn PR=1      DRm -> XDn 1111nnn1mmm01100 */
/*  FMOV    XDm,XDn PR=1      XDm -> XDn 1111nnn1mmm11100 */
const void FMOVFR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if ((sh4->fpu_sz == 0) && (sh4->fpu_pr == 0)) /* SZ = 0 */
		sh4->fr[n] = sh4->fr[m];
	else { /* SZ = 1 or PR = 1 */
		if (m & 1) {
			if (n & 1) {
				sh4->xf[n & 14] = sh4->xf[m & 14];
				sh4->xf[n | 1] = sh4->xf[m | 1];
			} else {
				sh4->fr[n] = sh4->xf[m & 14];
				sh4->fr[n | 1] = sh4->xf[m | 1];
			}
		} else {
			if (n & 1) {
				sh4->xf[n & 14] = sh4->fr[m];
				sh4->xf[n | 1] = sh4->fr[m | 1]; // (a&14)+1 -> a|1
			} else {
				sh4->fr[n] = sh4->fr[m];
				sh4->fr[n | 1] = sh4->fr[m | 1];
			}
		}
	}
}

/*  FLDI1  FRn 1111nnnn10011101 */
const void FLDI1(sh4_state *sh4, const UINT16 opcode)
{
	sh4->fr[Rn] = 0x3F800000;
}

/*  FLDI0  FRn 1111nnnn10001101 */
const void FLDI0(sh4_state *sh4, const UINT16 opcode)
{
	sh4->fr[Rn] = 0;
}

/*  FLDS FRm,FPUL 1111mmmm00011101 */
const void  FLDS(sh4_state *sh4, const UINT16 opcode)
{
	sh4->fpul = sh4->fr[Rn];
}

/*  FSTS FPUL,FRn 1111nnnn00001101 */
const void  FSTS(sh4_state *sh4, const UINT16 opcode)
{
	sh4->fr[Rn] = sh4->fpul;
}

/* FRCHG 1111101111111101 */
const void FRCHG(sh4_state *sh4)
{
	sh4->fpscr ^= FR;
	sh4_swap_fp_registers(sh4);
}

/* FSCHG 1111001111111101 */
const void FSCHG(sh4_state *sh4)
{
	sh4->fpscr ^= SZ;
	sh4->fpu_sz = (sh4->fpscr & SZ) ? 1 : 0;
}

/* FTRC FRm,FPUL PR=0 1111mmmm00111101 */
/* FTRC DRm,FPUL PR=1 1111mmm000111101 */
const void FTRC(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		*((INT32 *)&sh4->fpul) = (INT32)FP_RFD(n);
	} else {              /* PR = 0 */
		/* read sh4->fr[n] as float -> truncate -> fpul(32) */
		*((INT32 *)&sh4->fpul) = (INT32)FP_RFS(n);
	}
}

/* FLOAT FPUL,FRn PR=0 1111nnnn00101101 */
/* FLOAT FPUL,DRn PR=1 1111nnn000101101 */
const void FLOAT(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		FP_RFD(n) = (double)*((INT32 *)&sh4->fpul);
	} else {              /* PR = 0 */
		FP_RFS(n) = (float)*((INT32 *)&sh4->fpul);
	}
}

/* FNEG FRn PR=0 1111nnnn01001101 */
/* FNEG DRn PR=1 1111nnn001001101 */
const void FNEG(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		FP_RFD(n) = -FP_RFD(n);
	} else {              /* PR = 0 */
		FP_RFS(n) = -FP_RFS(n);
	}
}

/* FABS FRn PR=0 1111nnnn01011101 */
/* FABS DRn PR=1 1111nnn001011101 */
const void FABS(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
#ifdef LSB_FIRST
		n = n | 1; // n & 14 + 1
		sh4->fr[n] = sh4->fr[n] & 0x7fffffff;
#else
		n = n & 14;
		sh4->fr[n] = sh4->fr[n] & 0x7fffffff;
#endif
	} else {              /* PR = 0 */
		sh4->fr[n] = sh4->fr[n] & 0x7fffffff;
	}
}

/* FCMP/EQ FRm,FRn PR=0 1111nnnnmmmm0100 */
/* FCMP/EQ DRm,DRn PR=1 1111nnn0mmm00100 */
const void FCMP_EQ(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) == FP_RFD(m))
			sh4->sr |= T;
		else
			sh4->sr &= ~T;
	} else {              /* PR = 0 */
		if (FP_RFS(n) == FP_RFS(m))
			sh4->sr |= T;
		else
			sh4->sr &= ~T;
	}
}

/* FCMP/GT FRm,FRn PR=0 1111nnnnmmmm0101 */
/* FCMP/GT DRm,DRn PR=1 1111nnn0mmm00101 */
const void FCMP_GT(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) > FP_RFD(m))
			sh4->sr |= T;
		else
			sh4->sr &= ~T;
	} else {              /* PR = 0 */
		if (FP_RFS(n) > FP_RFS(m))
			sh4->sr |= T;
		else
			sh4->sr &= ~T;
	}
}

/* FCNVDS DRm,FPUL PR=1 1111mmm010111101 */
const void FCNVDS(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		if (sh4->fpscr & RM)
			sh4->fr[n | NATIVE_ENDIAN_VALUE_LE_BE(0,1)] &= 0xe0000000; /* round toward zero*/
		*((float *)&sh4->fpul) = (float)FP_RFD(n);
	}
}

/* FCNVSD FPUL, DRn PR=1 1111nnn010101101 */
const void FCNVSD(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		FP_RFD(n) = (double)*((float *)&sh4->fpul);
	}
}

/* FADD FRm,FRn PR=0 1111nnnnmmmm0000 */
/* FADD DRm,DRn PR=1 1111nnn0mmm00000 */
const void FADD(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) + FP_RFD(m);
	} else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) + FP_RFS(m);
	}
}

/* FSUB FRm,FRn PR=0 1111nnnnmmmm0001 */
/* FSUB DRm,DRn PR=1 1111nnn0mmm00001 */
const void FSUB(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) - FP_RFD(m);
	} else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) - FP_RFS(m);
	}
}


/* FMUL FRm,FRn PR=0 1111nnnnmmmm0010 */
/* FMUL DRm,DRn PR=1 1111nnn0mmm00010 */
const void FMUL(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) * FP_RFD(m);
	} else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) * FP_RFS(m);
	}
}

/* FDIV FRm,FRn PR=0 1111nnnnmmmm0011 */
/* FDIV DRm,DRn PR=1 1111nnn0mmm00011 */
const void FDIV(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(m) == 0)
			return;
		FP_RFD(n) = FP_RFD(n) / FP_RFD(m);
	} else {              /* PR = 0 */
		if (FP_RFS(m) == 0)
			return;
		FP_RFS(n) = FP_RFS(n) / FP_RFS(m);
	}
}

/* FMAC FR0,FRm,FRn PR=0 1111nnnnmmmm1110 */
const void FMAC(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 m = Rm; UINT32 n = Rn;

	if (sh4->fpu_pr == 0) { /* PR = 0 */
		FP_RFS(n) = (FP_RFS(0) * FP_RFS(m)) + FP_RFS(n);
	}
}

/* FSQRT FRn PR=0 1111nnnn01101101 */
/* FSQRT DRn PR=1 1111nnnn01101101 */
const void FSQRT(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	if (sh4->fpu_pr) { /* PR = 1 */
		n = n & 14;
		if (FP_RFD(n) < 0)
			return;
		FP_RFD(n) = sqrtf(FP_RFD(n));
	} else {              /* PR = 0 */
		if (FP_RFS(n) < 0)
			return;
		FP_RFS(n) = sqrtf(FP_RFS(n));
	}
}

/* FSRRA FRn PR=0 1111nnnn01111101 */
const void FSRRA(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

	if (FP_RFS(n) < 0)
		return;
	FP_RFS(n) = 1.0 / sqrtf(FP_RFS(n));
}

/*  FSSCA FPUL,FRn PR=0 1111nnn011111101 */
const void FSSCA(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

float angle;

	angle = (((float)(sh4->fpul & 0xFFFF)) / 65536.0) * 2.0 * M_PI;
	FP_RFS(n) = sinf(angle);
	FP_RFS(n+1) = cosf(angle);
}

/* FIPR FVm,FVn PR=0 1111nnmm11101101 */
const void FIPR(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

UINT32 m;
float ml[4];
int a;

	m = (n & 3) << 2;
	n = n & 12;
	for (a = 0;a < 4;a++)
		ml[a] = FP_RFS(n+a) * FP_RFS(m+a);
	FP_RFS(n+3) = ml[0] + ml[1] + ml[2] + ml[3];
}

/* FTRV XMTRX,FVn PR=0 1111nn0111111101 */
const void FTRV(sh4_state *sh4, const UINT16 opcode)
{
	UINT32 n = Rn;

int i,j;
float sum[4];

	n = n & 12;
	for (i = 0;i < 4;i++) {
		sum[i] = 0;
		for (j=0;j < 4;j++)
			sum[i] += FP_XFS((j << 2) + i)*FP_RFS(n + j);
	}
	for (i = 0;i < 4;i++)
		FP_RFS(n + i) = sum[i];
}

const void op1111_0xf13(sh4_state *sh4, const UINT16 opcode)
{
	if (opcode & 0x100) {
			if (opcode & 0x200) {
				switch (opcode & 0xC00)
				{
					case 0x000:
						FSCHG(sh4);
						break;
					case 0x800:
						FRCHG(sh4);
						break;
					default:
						debugger_break(sh4->device->machine());
						break;
				}
			} else {
				FTRV(sh4, opcode);
			}
		} else {
			FSSCA(sh4, opcode);
		}
}

const void dbreak(sh4_state *sh4, const UINT16 opcode)
{
	debugger_break(sh4->device->machine());
}


sh4ophandler op1111_0x13_handlers[] =
{
	FSTS,       FLDS,       FLOAT,      FTRC,       FNEG,       FABS,       FSQRT,      FSRRA,      FLDI0,      FLDI1,      FCNVSD,     FCNVDS,     dbreak, dbreak, FIPR,   op1111_0xf13
};

const void op1111_0x13(sh4_state *sh4, UINT16 opcode)
{
	op1111_0x13_handlers[(opcode&0xf0)>>4](sh4, opcode);
}


/*****************************************************************************
 *  MAME CPU INTERFACE
 *****************************************************************************/

static CPU_RESET( common_sh4_reset )
{
	sh4_state *sh4 = get_safe_token(device);
	emu_timer *tsaved[4];
	emu_timer *tsave[5];
	UINT32 *m;
	int save_is_slave;
	int savecpu_clock, savebus_clock, savepm_clock;

	void (*f)(UINT32 data);
	device_irq_acknowledge_callback save_irqcallback;

	m = sh4->m;
	tsaved[0] = sh4->dma_timer[0];
	tsaved[1] = sh4->dma_timer[1];
	tsaved[2] = sh4->dma_timer[2];
	tsaved[3] = sh4->dma_timer[3];
	tsave[0] = sh4->refresh_timer;
	tsave[1] = sh4->rtc_timer;
	tsave[2] = sh4->timer[0];
	tsave[3] = sh4->timer[1];
	tsave[4] = sh4->timer[2];

	f = sh4->ftcsr_read_callback;
	save_irqcallback = sh4->irq_callback;
	save_is_slave = sh4->is_slave;
	savecpu_clock = sh4->cpu_clock;
	savebus_clock = sh4->bus_clock;
	savepm_clock = sh4->pm_clock;
	memset(sh4, 0, sizeof(*sh4));
	sh4->is_slave = save_is_slave;
	sh4->cpu_clock = savecpu_clock;
	sh4->bus_clock = savebus_clock;
	sh4->pm_clock = savepm_clock;
	sh4->ftcsr_read_callback = f;
	sh4->irq_callback = save_irqcallback;
	sh4->device = device;
	sh4->internal = &device->space(AS_PROGRAM);
	sh4->program = &device->space(AS_PROGRAM);
	sh4->direct = &sh4->program->direct();
	sh4->io = &device->space(AS_IO);

	sh4->dma_timer[0] = tsaved[0];
	sh4->dma_timer[1] = tsaved[1];
	sh4->dma_timer[2] = tsaved[2];
	sh4->dma_timer[3] = tsaved[3];
	sh4->refresh_timer = tsave[0];
	sh4->rtc_timer = tsave[1];
	sh4->timer[0] = tsave[2];
	sh4->timer[1] = tsave[3];
	sh4->timer[2] = tsave[4];
	sh4->m = m;
	memset(sh4->m, 0, 16384*4);
	sh4_default_exception_priorities(sh4);
	memset(sh4->exception_requesting, 0, sizeof(sh4->exception_requesting));

	sh4->rtc_timer->adjust(attotime::from_hz(128));


	sh4->pc = 0xa0000000;
	sh4->r[15] = RL(sh4,4);
	sh4->sr = 0x700000f0;
	sh4->fpscr = 0x00040001;
	sh4->fpu_sz = (sh4->fpscr & SZ) ? 1 : 0;
	sh4->fpu_pr = (sh4->fpscr & PR) ? 1 : 0;
	sh4->fpul = 0;
	sh4->dbr = 0;

	sh4->internal_irq_level = -1;
	sh4->irln = 15;
	sh4->sleep_mode = 0;

	sh4->sh4_mmu_enabled = 0;

	sh4_build_optable(sh4);
}

/*-------------------------------------------------
    sh3_reset - reset the processor
-------------------------------------------------*/

static CPU_RESET( sh3 )
{
	sh4_state *sh4 = get_safe_token(device);

	CPU_RESET_CALL(common_sh4_reset);

	sh4->cpu_type = CPU_TYPE_SH3;

	sh4->SH4_TCOR0 = 0xffffffff;
	sh4->SH4_TCNT0 = 0xffffffff;
	sh4->SH4_TCOR1 = 0xffffffff;
	sh4->SH4_TCNT1 = 0xffffffff;
	sh4->SH4_TCOR2 = 0xffffffff;
	sh4->SH4_TCNT2 = 0xffffffff;

}

static CPU_RESET( sh4 )
{
	sh4_state *sh4 = get_safe_token(device);

	CPU_RESET_CALL(common_sh4_reset);

	sh4->cpu_type = CPU_TYPE_SH4;

	sh4->m[RCR2] = 0x09;
	sh4->SH4_TCOR0 = 0xffffffff;
	sh4->SH4_TCNT0 = 0xffffffff;
	sh4->SH4_TCOR1 = 0xffffffff;
	sh4->SH4_TCNT1 = 0xffffffff;
	sh4->SH4_TCOR2 = 0xffffffff;
	sh4->SH4_TCNT2 = 0xffffffff;
}

/* These tables are combined into our main opcode jump table, master_ophandler_table in the RESET function */

sh4ophandler op1000_handler[] =
{
	MOVBS4,     MOVWS4,     NOP,        NOP,        MOVBL4,     MOVWL4,     NOP,        NOP,        CMPIM,      BT,         NOP,        BF,         NOP,        BTS,        NOP,        BFS
};

sh4ophandler op1100_handler[] =
{
	MOVBSG,     MOVWSG,     MOVLSG,     TRAPA,      MOVBLG,     MOVWLG,     MOVLLG,     MOVA,       TSTI,       ANDI,       XORI,       ORI,        TSTM,       ANDM,       XORM,       ORM
};

sh4ophandler op0000_handlers[] =
{
	NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,
	NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,
	STCSR,      STCGBR,     STCVBR,     STCSSR,     STCSPC,     NOP,        NOP,        NOP,        STCRBANK,   STCRBANK,   STCRBANK,   STCRBANK,   STCRBANK,   STCRBANK,   STCRBANK,   STCRBANK,
	BSRF,       NOP,        BRAF,       NOP,        NOP,        NOP,        NOP,        NOP,        PREFM,      TODO,       TODO,       TODO,       MOVCAL,     NOP,        NOP,        NOP,
	MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,     MOVBS0,
	MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,     MOVWS0,
	MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,     MOVLS0,
	MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,       MULL,
	CLRT,       SETT,       CLRMAC,     TODO,       CLRS,       SETS,       NOP,        NOP,        CLRT,       SETT,       CLRMAC,     TODO,       CLRS,       SETS,       NOP,        NOP,
	NOP,        DIV0U,      MOVT,       NOP,        NOP,        DIV0U,      MOVT,       NOP,        NOP,        DIV0U,      MOVT,       NOP,        NOP,        DIV0U,      MOVT,       NOP,
	STSMACH,    STSMACL,    STSPR,      STCSGR,     NOP,        STSFPUL,    STSFPSCR,   STCDBR,     STSMACH,    STSMACL,    STSPR,      STCSGR,     NOP,        STSFPUL,    STSFPSCR,   STCDBR,
	RTS,        SLEEP,      RTE,        NOP,        RTS,        SLEEP,      RTE,        NOP,        RTS,        SLEEP,      RTE,        NOP,        RTS,        SLEEP,      RTE,        NOP,
	MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,     MOVBL0,
	MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,     MOVWL0,
	MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,     MOVLL0,
	MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,      MAC_L,
};

sh4ophandler op0100_handlers[] =
{
	SHLL,       DT,         SHAL,       NOP,        SHLL,       DT,         SHAL,       NOP,        SHLL,       DT,         SHAL,       NOP,        SHLL,       DT,         SHAL,       NOP,
	SHLR,       CMPPZ,      SHAR,       NOP,        SHLR,       CMPPZ,      SHAR,       NOP,        SHLR,       CMPPZ,      SHAR,       NOP,        SHLR,       CMPPZ,      SHAR,       NOP,
	STSMMACH,   STSMMACL,   STSMPR,     STCMSGR,    NOP,        STSMFPUL,   STSMFPSCR,  NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        STCMDBR,
	STCMSR,     STCMGBR,    STCMVBR,    STCMSSR,    STCMSPC,    NOP,        NOP,        NOP,        STCMRBANK,  STCMRBANK,  STCMRBANK,  STCMRBANK,  STCMRBANK,  STCMRBANK,  STCMRBANK,  STCMRBANK,
	ROTL,       NOP,        ROTCL,      NOP,        ROTL,       NOP,        ROTCL,      NOP,        ROTL,       NOP,        ROTCL,      NOP,        ROTL,       NOP,        ROTCL,      NOP,
	ROTR,       CMPPL,      ROTCR,      NOP,        ROTR,       CMPPL,      ROTCR,      NOP,        ROTR,       CMPPL,      ROTCR,      NOP,        ROTR,       CMPPL,      ROTCR,      NOP,
	LDSMMACH,   LDSMMACL,   LDSMPR,     NOP,        NOP,        LDSMFPUL,   LDSMFPSCR,  NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        LDCMDBR,
	LDCMSR,     LDCMGBR,    LDCMVBR,    LDCMSSR,    LDCMSPC,    NOP,        NOP,        NOP,        LDCMRBANK,  LDCMRBANK,  LDCMRBANK,  LDCMRBANK,  LDCMRBANK,  LDCMRBANK,  LDCMRBANK,  LDCMRBANK,
	SHLL2,      SHLL8,      SHLL16,     NOP,        SHLL2,      SHLL8,      SHLL16,     NOP,        SHLL2,      SHLL8,      SHLL16,     NOP,        SHLL2,      SHLL8,      SHLL16,     NOP,
	SHLR2,      SHLR8,      SHLR16,     NOP,        SHLR2,      SHLR8,      SHLR16,     NOP,        SHLR2,      SHLR8,      SHLR16,     NOP,        SHLR2,      SHLR8,      SHLR16,     NOP,
	LDSMACH,    LDSMACL,    LDSPR,      NOP,        NOP,        LDSFPUL,    LDSFPSCR,   NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        NOP,        LDCDBR,
	JSR,        TAS,        JMP,        NOP,        JSR,        TAS,        JMP,        NOP,        JSR,        TAS,        JMP,        NOP,        JSR,        TAS,        JMP,        NOP,
	SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,       SHAD,
	SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,       SHLD,
	LDCSR,      LDCGBR,     LDCVBR,     LDCSSR,     LDCSPC,     NOP,        NOP,        NOP,        LDCRBANK,   LDCRBANK,   LDCRBANK,   LDCRBANK,   LDCRBANK,   LDCRBANK,   LDCRBANK,   LDCRBANK,
	MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,      MAC_W,
};


sh4ophandler upper4bits[] =
{
	0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,      /* j = 0x0000 - uses op0000_handlers*/
	MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4,     MOVLS4, /* j = 0x1000 */
	MOVBS,      MOVWS,      MOVLS,      NOP,        MOVBM,      MOVWM,      MOVLM,      DIV0S,      TST,        AND,        XOR,        OR,         CMPSTR,     XTRCT,      MULU,       MULS,   /* j = 0x2000 */
	CMPEQ,      NOP,        CMPHS,      CMPGE,      DIV1,       DMULU,      CMPHI,      CMPGT,      SUB,        NOP,        SUBC,       SUBV,       ADD,        DMULS,      ADDC,       ADDV,   /* j = 0x3000 */
	0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,      /* j = 0x4000 - uses op0100_handlers*/
	MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4,     MOVLL4, /* j = 0x5000 */
	MOVBL,      MOVWL,      MOVLL,      MOV,        MOVBP,      MOVWP,      MOVLP,      NOT,        SWAPB,      SWAPW,      NEGC,       NEG,        EXTUB,      EXTUW,      EXTSB,      EXTSW,  /* j = 0x6000 */
	ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,       ADDI,   /* j = 0x7000 */
	0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,      /* j = 0x8000 - uses op1000_handlers */
	MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,      MOVWI,  /* j = 0x9000 */
	BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,        BRA,    /* j = 0xa000 */
	BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,        BSR,    /* j = 0xb000 */
	0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,          0,      /* j = 0xc000 - uses op1100_handlers */
	MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,      MOVLI,  /* j = 0xd000 */
	MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,       MOVI,   /* j = 0xe000 */
	FADD,       FSUB,       FMUL,       FDIV,       FCMP_EQ,    FCMP_GT,    FMOVS0FR,   FMOVFRS0,   FMOVMRFR,   FMOVMRIFR,  FMOVFRMR,   FMOVFRMDR,  FMOVFR,     op1111_0x13,FMAC,       dbreak  /* j = 0xf000 */
};

void sh4_build_optable(sh4_state *sh4)
{
	int j,y,x,z;

	// combine our opcode handler tables into one larger table thus reducing level of indirection on all opcode handlers
	for (j = 0; j<0x10000;j+=0x1000)
	{
		for (y = 0; y<0x1000;y+=0x100)
		{
			for (x=0; x<0x100;x+=0x10)
			{
				for (z=0;z<0x10;z++)
				{
					master_ophandler_table[j+y+x+z] = upper4bits[(((j+z)&0xf000)>>8) + (z & 0xf)];
				}
			}
		}
	}

	j = 0x0000;
	//for (j = 0; j<0x10000;j+=0x1000)
	{
		for (y = 0; y<0x1000;y+=0x100)
		{
			for (x=0; x<0x100;x+=0x10)
			{
				for (z=0;z<0x10;z++)
				{
					master_ophandler_table[j+y+x+z] = op0000_handlers[((((j+y+x+z)&0xf0)>>4)) | ((((j+y+x+z)&0xf)<<4))];
				}
			}
		}
	}

	j = 0x4000;
	//for (j = 0; j<0x10000;j+=0x1000)
	{
		for (y = 0; y<0x1000;y+=0x100)
		{
			for (x=0; x<0x100;x+=0x10)
			{
				for (z=0;z<0x10;z++)
				{
					master_ophandler_table[j+y+x+z] = op0100_handlers[((((j+y+x+z)&0xf0)>>4)) | ((((j+y+x+z)&0xf)<<4))];
				}
			}
		}
	}


	j = 0x8000;
	//for (j = 0; j<0x10000;j+=0x1000)
	{
		for (y = 0; y<0x1000;y+=0x100)
		{
			for (x=0; x<0x100;x+=0x10)
			{
				for (z=0;z<0x10;z++)
				{
					master_ophandler_table[j+y+x+z] = op1000_handler[((((j+y+x+z)&0xf00)>>8))];
				}
			}
		}
	}

	j = 0xc000;
	//for (j = 0; j<0x10000;j+=0x1000)
	{
		for (y = 0; y<0x1000;y+=0x100)
		{
			for (x=0; x<0x100;x+=0x10)
			{
				for (z=0;z<0x10;z++)
				{
					master_ophandler_table[j+y+x+z] = op1100_handler[((((j+y+x+z)&0xf00)>>8))];
				}
			}
		}
	}


}


/* Execute cycles - returns number of cycles actually run */
static CPU_EXECUTE( sh4 )
{
	sh4_state *sh4 = get_safe_token(device);

	if (sh4->cpu_off)
	{
		sh4->sh4_icount = 0;
		return;
	}

	do
	{
		if (sh4->delay)
		{
			const UINT16 opcode = sh4->direct->read_decrypted_word((UINT32)(sh4->delay & AM), WORD2_XOR_LE(0));

			debugger_instruction_hook(device, (sh4->pc-2) & AM);

			sh4->delay = 0;
			sh4->ppc = sh4->pc;

			master_ophandler_table[opcode](sh4, opcode);

			if (sh4->test_irq && !sh4->delay)
			{
				sh4_check_pending_irq(sh4, "mame_sh4_execute");
			}
		}
		else
		{
			const UINT16  opcode = sh4->direct->read_decrypted_word((UINT32)(sh4->pc & AM), WORD2_XOR_LE(0));

			debugger_instruction_hook(device, sh4->pc & AM);

			sh4->pc += 2;
			sh4->ppc = sh4->pc;

			master_ophandler_table[opcode](sh4, opcode);

			if (sh4->test_irq && !sh4->delay)
			{
				sh4_check_pending_irq(sh4, "mame_sh4_execute");
			}
		}

		sh4->sh4_icount--;
	} while( sh4->sh4_icount > 0 );
}

static CPU_EXECUTE( sh4be )
{
	sh4_state *sh4 = get_safe_token(device);

	if (sh4->cpu_off)
	{
		sh4->sh4_icount = 0;
		return;
	}

	do
	{
		if (sh4->delay)
		{
			const UINT16 opcode = sh4->direct->read_decrypted_word((UINT32)(sh4->delay & AM), WORD_XOR_LE(6));

			debugger_instruction_hook(device, sh4->delay & AM);

			sh4->delay = 0;
			sh4->ppc = sh4->pc;

			master_ophandler_table[opcode](sh4, opcode);


			if (sh4->test_irq && !sh4->delay)
			{
				sh4_check_pending_irq(sh4, "mame_sh4_execute");
			}


		}
		else
		{
			const UINT16 opcode = sh4->direct->read_decrypted_word((UINT32)(sh4->pc & AM), WORD_XOR_LE(6));

			debugger_instruction_hook(device, sh4->pc & AM);

			sh4->pc += 2;
			sh4->ppc = sh4->pc;

			master_ophandler_table[opcode](sh4, opcode);

			if (sh4->test_irq && !sh4->delay)
			{
				sh4_check_pending_irq(sh4, "mame_sh4_execute");
			}
		}

		sh4->sh4_icount--;
	} while( sh4->sh4_icount > 0 );
}

static CPU_INIT( sh4 )
{
	const struct sh4_config *conf = (const struct sh4_config *)device->static_config();
	sh4_state *sh4 = get_safe_token(device);

	sh4_common_init(device);

	sh4_parse_configuration(sh4, conf);

	sh4->irq_callback = irqcallback;
	sh4->device = device;
	sh4->internal = &device->space(AS_PROGRAM);
	sh4->program = &device->space(AS_PROGRAM);
	sh4->io = &device->space(AS_IO);
	sh4_default_exception_priorities(sh4);
	sh4->irln = 15;
	sh4->test_irq = 0;

	device->save_item(NAME(sh4->pc));
	device->save_item(NAME(sh4->r[15]));
	device->save_item(NAME(sh4->sr));
	device->save_item(NAME(sh4->pr));
	device->save_item(NAME(sh4->gbr));
	device->save_item(NAME(sh4->vbr));
	device->save_item(NAME(sh4->mach));
	device->save_item(NAME(sh4->macl));
	device->save_item(NAME(sh4->spc));
	device->save_item(NAME(sh4->ssr));
	device->save_item(NAME(sh4->sgr));
	device->save_item(NAME(sh4->fpscr));
	device->save_item(NAME(sh4->r[ 0]));
	device->save_item(NAME(sh4->r[ 1]));
	device->save_item(NAME(sh4->r[ 2]));
	device->save_item(NAME(sh4->r[ 3]));
	device->save_item(NAME(sh4->r[ 4]));
	device->save_item(NAME(sh4->r[ 5]));
	device->save_item(NAME(sh4->r[ 6]));
	device->save_item(NAME(sh4->r[ 7]));
	device->save_item(NAME(sh4->r[ 8]));
	device->save_item(NAME(sh4->r[ 9]));
	device->save_item(NAME(sh4->r[10]));
	device->save_item(NAME(sh4->r[11]));
	device->save_item(NAME(sh4->r[12]));
	device->save_item(NAME(sh4->r[13]));
	device->save_item(NAME(sh4->r[14]));
	device->save_item(NAME(sh4->fr[ 0]));
	device->save_item(NAME(sh4->fr[ 1]));
	device->save_item(NAME(sh4->fr[ 2]));
	device->save_item(NAME(sh4->fr[ 3]));
	device->save_item(NAME(sh4->fr[ 4]));
	device->save_item(NAME(sh4->fr[ 5]));
	device->save_item(NAME(sh4->fr[ 6]));
	device->save_item(NAME(sh4->fr[ 7]));
	device->save_item(NAME(sh4->fr[ 8]));
	device->save_item(NAME(sh4->fr[ 9]));
	device->save_item(NAME(sh4->fr[10]));
	device->save_item(NAME(sh4->fr[11]));
	device->save_item(NAME(sh4->fr[12]));
	device->save_item(NAME(sh4->fr[13]));
	device->save_item(NAME(sh4->fr[14]));
	device->save_item(NAME(sh4->fr[15]));
	device->save_item(NAME(sh4->xf[ 0]));
	device->save_item(NAME(sh4->xf[ 1]));
	device->save_item(NAME(sh4->xf[ 2]));
	device->save_item(NAME(sh4->xf[ 3]));
	device->save_item(NAME(sh4->xf[ 4]));
	device->save_item(NAME(sh4->xf[ 5]));
	device->save_item(NAME(sh4->xf[ 6]));
	device->save_item(NAME(sh4->xf[ 7]));
	device->save_item(NAME(sh4->xf[ 8]));
	device->save_item(NAME(sh4->xf[ 9]));
	device->save_item(NAME(sh4->xf[10]));
	device->save_item(NAME(sh4->xf[11]));
	device->save_item(NAME(sh4->xf[12]));
	device->save_item(NAME(sh4->xf[13]));
	device->save_item(NAME(sh4->xf[14]));
	device->save_item(NAME(sh4->xf[15]));
	device->save_item(NAME(sh4->ea));
	device->save_item(NAME(sh4->fpul));
	device->save_item(NAME(sh4->dbr));
	device->save_item(NAME(sh4->exception_priority));
	device->save_item(NAME(sh4->exception_requesting));

	device->save_item(NAME(sh4->SH4_TSTR));
	device->save_item(NAME(sh4->SH4_TCNT0));
	device->save_item(NAME(sh4->SH4_TCNT1));
	device->save_item(NAME(sh4->SH4_TCNT2));
	device->save_item(NAME(sh4->SH4_TCR0));
	device->save_item(NAME(sh4->SH4_TCR1));
	device->save_item(NAME(sh4->SH4_TCR2));
	device->save_item(NAME(sh4->SH4_TCOR0));
	device->save_item(NAME(sh4->SH4_TCOR1));
	device->save_item(NAME(sh4->SH4_TCOR2));
	device->save_item(NAME(sh4->SH4_TOCR));
	device->save_item(NAME(sh4->SH4_TCPR2));

	device->save_item(NAME(sh4->SH4_IPRA));

	device->save_item(NAME(sh4->SH4_IPRC));



}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( sh4 )
{
	sh4_state *sh4 = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + SH4_IRL0:        sh4_set_irq_line(sh4, SH4_IRL0, info->i);       break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL1:        sh4_set_irq_line(sh4, SH4_IRL1, info->i);       break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL2:        sh4_set_irq_line(sh4, SH4_IRL2, info->i);       break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL3:        sh4_set_irq_line(sh4, SH4_IRL3, info->i);       break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRLn:        sh4_set_irq_line(sh4, SH4_IRLn, info->i);       break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:  sh4_set_irq_line(sh4, INPUT_LINE_NMI, info->i); break;

		case CPUINFO_INT_REGISTER + SH4_PC:
		case CPUINFO_INT_PC:                            sh4->pc = info->i; sh4->delay = 0;      break;
		case CPUINFO_INT_SP:                            sh4->r[15] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_PR:             sh4->pr = info->i;                      break;
		case CPUINFO_INT_REGISTER + SH4_SR:
			sh4->sr = info->i;
			sh4_exception_recompute(sh4);
			sh4_check_pending_irq(sh4, "sh4_set_info");
			break;
		case CPUINFO_INT_REGISTER + SH4_GBR:            sh4->gbr = info->i;                     break;
		case CPUINFO_INT_REGISTER + SH4_VBR:            sh4->vbr = info->i;                     break;
		case CPUINFO_INT_REGISTER + SH4_DBR:            sh4->dbr = info->i;                     break;
		case CPUINFO_INT_REGISTER + SH4_MACH:           sh4->mach = info->i;                        break;
		case CPUINFO_INT_REGISTER + SH4_MACL:           sh4->macl = info->i;                        break;
		case CPUINFO_INT_REGISTER + SH4_R0:             sh4->r[ 0] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R1:             sh4->r[ 1] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R2:             sh4->r[ 2] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R3:             sh4->r[ 3] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R4:             sh4->r[ 4] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R5:             sh4->r[ 5] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R6:             sh4->r[ 6] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R7:             sh4->r[ 7] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R8:             sh4->r[ 8] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R9:             sh4->r[ 9] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R10:            sh4->r[10] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R11:            sh4->r[11] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R12:            sh4->r[12] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R13:            sh4->r[13] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R14:            sh4->r[14] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_R15:            sh4->r[15] = info->i;                   break;
		case CPUINFO_INT_REGISTER + SH4_EA:             sh4->ea = info->i;                      break;
		case CPUINFO_STR_REGISTER + SH4_R0_BK0:         sh4->rbnk[0][0] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R1_BK0:         sh4->rbnk[0][1] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R2_BK0:         sh4->rbnk[0][2] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R3_BK0:         sh4->rbnk[0][3] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R4_BK0:         sh4->rbnk[0][4] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R5_BK0:         sh4->rbnk[0][5] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R6_BK0:         sh4->rbnk[0][6] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R7_BK0:         sh4->rbnk[0][7] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R0_BK1:         sh4->rbnk[1][0] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R1_BK1:         sh4->rbnk[1][1] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R2_BK1:         sh4->rbnk[1][2] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R3_BK1:         sh4->rbnk[1][3] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R4_BK1:         sh4->rbnk[1][4] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R5_BK1:         sh4->rbnk[1][5] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R6_BK1:         sh4->rbnk[1][6] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R7_BK1:         sh4->rbnk[1][7] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_SPC:            sh4->spc = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_SSR:            sh4->ssr = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_SGR:            sh4->sgr = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FPSCR:          sh4->fpscr = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FPUL:           sh4->fpul = info->i; break;
#ifdef LSB_FIRST
		case CPUINFO_STR_REGISTER + SH4_FR0:            sh4->fr[ 0 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR1:            sh4->fr[ 1 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR2:            sh4->fr[ 2 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR3:            sh4->fr[ 3 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR4:            sh4->fr[ 4 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR5:            sh4->fr[ 5 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR6:            sh4->fr[ 6 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR7:            sh4->fr[ 7 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR8:            sh4->fr[ 8 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR9:            sh4->fr[ 9 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR10:           sh4->fr[10 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR11:           sh4->fr[11 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR12:           sh4->fr[12 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR13:           sh4->fr[13 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR14:           sh4->fr[14 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR15:           sh4->fr[15 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF0:            sh4->xf[ 0 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF1:            sh4->xf[ 1 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF2:            sh4->xf[ 2 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF3:            sh4->xf[ 3 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF4:            sh4->xf[ 4 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF5:            sh4->xf[ 5 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF6:            sh4->xf[ 6 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF7:            sh4->xf[ 7 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF8:            sh4->xf[ 8 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF9:            sh4->xf[ 9 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF10:           sh4->xf[10 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF11:           sh4->xf[11 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF12:           sh4->xf[12 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF13:           sh4->xf[13 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF14:           sh4->xf[14 ^ sh4->fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF15:           sh4->xf[15 ^ sh4->fpu_pr] = info->i; break;
#else
		case CPUINFO_STR_REGISTER + SH4_FR0:            sh4->fr[ 0] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR1:            sh4->fr[ 1] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR2:            sh4->fr[ 2] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR3:            sh4->fr[ 3] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR4:            sh4->fr[ 4] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR5:            sh4->fr[ 5] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR6:            sh4->fr[ 6] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR7:            sh4->fr[ 7] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR8:            sh4->fr[ 8] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR9:            sh4->fr[ 9] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR10:           sh4->fr[10] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR11:           sh4->fr[11] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR12:           sh4->fr[12] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR13:           sh4->fr[13] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR14:           sh4->fr[14] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR15:           sh4->fr[15] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF0:            sh4->xf[ 0] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF1:            sh4->xf[ 1] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF2:            sh4->xf[ 2] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF3:            sh4->xf[ 3] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF4:            sh4->xf[ 4] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF5:            sh4->xf[ 5] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF6:            sh4->xf[ 6] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF7:            sh4->xf[ 7] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF8:            sh4->xf[ 8] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF9:            sh4->xf[ 9] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF10:           sh4->xf[10] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF11:           sh4->xf[11] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF12:           sh4->xf[12] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF13:           sh4->xf[13] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF14:           sh4->xf[14] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF15:           sh4->xf[15] = info->i; break;
#endif
	}
}

void sh4_set_ftcsr_callback(device_t *device, sh4_ftcsr_callback callback)
{
	sh4_state *sh4 = get_safe_token(device);
	sh4->ftcsr_read_callback = callback;
}


#if 0
/*When OC index mode is off (CCR.OIX = 0)*/
static ADDRESS_MAP_START( sh4_internal_map, AS_PROGRAM, 64, legacy_cpu_device )
	AM_RANGE(0x1C000000, 0x1C000FFF) AM_RAM AM_MIRROR(0x03FFD000)
	AM_RANGE(0x1C002000, 0x1C002FFF) AM_RAM AM_MIRROR(0x03FFD000)
	AM_RANGE(0xE0000000, 0xE000003F) AM_RAM AM_MIRROR(0x03FFFFC0)
ADDRESS_MAP_END
#endif

/*When OC index mode is on (CCR.OIX = 1)*/
static ADDRESS_MAP_START( sh4_internal_map, AS_PROGRAM, 64, legacy_cpu_device )
	AM_RANGE(0x1C000000, 0x1C000FFF) AM_RAM AM_MIRROR(0x01FFF000)
	AM_RANGE(0x1E000000, 0x1E000FFF) AM_RAM AM_MIRROR(0x01FFF000)
	AM_RANGE(0xE0000000, 0xE000003F) AM_RAM AM_MIRROR(0x03FFFFC0) // todo: store queues should be write only on DC's SH4, executing PREFM shouldn't cause an actual memory read access!
	AM_RANGE(0xF6000000, 0xF7FFFFFF) AM_READWRITE_LEGACY(sh4_tlb_r,sh4_tlb_w)
	AM_RANGE(0xFE000000, 0xFFFFFFFF) AM_READWRITE32_LEGACY(sh4_internal_r, sh4_internal_w, U64(0xffffffffffffffff))
ADDRESS_MAP_END

static ADDRESS_MAP_START( sh3_internal_map, AS_PROGRAM, 64, legacy_cpu_device )
	AM_RANGE(SH3_LOWER_REGBASE, SH3_LOWER_REGEND) AM_READWRITE32_LEGACY(sh3_internal_r, sh3_internal_w, U64(0xffffffffffffffff))
	AM_RANGE(SH3_UPPER_REGBASE, SH3_UPPER_REGEND) AM_READWRITE32_LEGACY(sh3_internal_high_r, sh3_internal_high_w, U64(0xffffffffffffffff))
ADDRESS_MAP_END


/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( sh4 )
{
	sh4_state *sh4 = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(sh4_state);        break;
		case CPUINFO_INT_INPUT_LINES:                   info->i = 5;                        break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:            info->i = 0;                        break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_LITTLE;                break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                        break;
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 1;                        break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 2;                        break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 2;                        break;
		case CPUINFO_INT_MIN_CYCLES:                    info->i = 1;                        break;
		case CPUINFO_INT_MAX_CYCLES:                    info->i = 4;                        break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:        info->i = 64;               break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:    info->i = 32;               break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:    info->i = 0;                break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:       info->i = 0;                break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:       info->i = 0;                break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:       info->i = 0;                break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:     info->i = 64;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:     info->i = 8;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:     info->i = 0;                    break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map64 = ADDRESS_MAP_NAME(sh4_internal_map); break;

		case CPUINFO_INT_INPUT_STATE + SH4_IRL0:        info->i = sh4->irq_line_state[SH4_IRL0]; break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL1:        info->i = sh4->irq_line_state[SH4_IRL1]; break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL2:        info->i = sh4->irq_line_state[SH4_IRL2]; break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL3:        info->i = sh4->irq_line_state[SH4_IRL3]; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:  info->i = sh4->nmi_line_state;          break;

		case CPUINFO_INT_PREVIOUSPC:                    info->i = sh4->ppc;                     break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SH4_PC:             info->i = (sh4->delay) ? (sh4->delay & AM) : (sh4->pc & AM); break;
		case CPUINFO_INT_SP:                            info->i = sh4->r[15];                   break;
		case CPUINFO_INT_REGISTER + SH4_PR:             info->i = sh4->pr;                      break;
		case CPUINFO_INT_REGISTER + SH4_SR:             info->i = sh4->sr;                      break;
		case CPUINFO_INT_REGISTER + SH4_GBR:            info->i = sh4->gbr;                     break;
		case CPUINFO_INT_REGISTER + SH4_VBR:            info->i = sh4->vbr;                     break;
		case CPUINFO_INT_REGISTER + SH4_DBR:            info->i = sh4->dbr;                     break;
		case CPUINFO_INT_REGISTER + SH4_MACH:           info->i = sh4->mach;                        break;
		case CPUINFO_INT_REGISTER + SH4_MACL:           info->i = sh4->macl;                        break;
		case CPUINFO_INT_REGISTER + SH4_R0:             info->i = sh4->r[ 0];                   break;
		case CPUINFO_INT_REGISTER + SH4_R1:             info->i = sh4->r[ 1];                   break;
		case CPUINFO_INT_REGISTER + SH4_R2:             info->i = sh4->r[ 2];                   break;
		case CPUINFO_INT_REGISTER + SH4_R3:             info->i = sh4->r[ 3];                   break;
		case CPUINFO_INT_REGISTER + SH4_R4:             info->i = sh4->r[ 4];                   break;
		case CPUINFO_INT_REGISTER + SH4_R5:             info->i = sh4->r[ 5];                   break;
		case CPUINFO_INT_REGISTER + SH4_R6:             info->i = sh4->r[ 6];                   break;
		case CPUINFO_INT_REGISTER + SH4_R7:             info->i = sh4->r[ 7];                   break;
		case CPUINFO_INT_REGISTER + SH4_R8:             info->i = sh4->r[ 8];                   break;
		case CPUINFO_INT_REGISTER + SH4_R9:             info->i = sh4->r[ 9];                   break;
		case CPUINFO_INT_REGISTER + SH4_R10:            info->i = sh4->r[10];                   break;
		case CPUINFO_INT_REGISTER + SH4_R11:            info->i = sh4->r[11];                   break;
		case CPUINFO_INT_REGISTER + SH4_R12:            info->i = sh4->r[12];                   break;
		case CPUINFO_INT_REGISTER + SH4_R13:            info->i = sh4->r[13];                   break;
		case CPUINFO_INT_REGISTER + SH4_R14:            info->i = sh4->r[14];                   break;
		case CPUINFO_INT_REGISTER + SH4_R15:            info->i = sh4->r[15];                   break;
		case CPUINFO_INT_REGISTER + SH4_EA:             info->i = sh4->ea;                      break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(sh4);         break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(sh4);                    break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(sh4);              break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(sh4);          break;
		case CPUINFO_FCT_BURN:                          info->burn = NULL;                      break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(sh4);          break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:           info->icount = &sh4->sh4_icount;                break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                      strcpy(info->s, "SH-4");                break;
		case CPUINFO_STR_FAMILY:                    strcpy(info->s, "Hitachi SH7750");      break;
		case CPUINFO_STR_VERSION:                   strcpy(info->s, "1.0");             break;
		case CPUINFO_STR_SOURCE_FILE:                   strcpy(info->s, __FILE__);              break;
		case CPUINFO_STR_CREDITS:                   strcpy(info->s, "Copyright R. Belmont"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%s%s%s%s%c%c%d%c%c",
					sh4->sr & MD ? "MD ":"   ",
					sh4->sr & sRB ? "RB ":"   ",
					sh4->sr & BL ? "BL ":"   ",
					sh4->sr & FD ? "FD ":"   ",
					sh4->sr & M ? 'M':'.',
					sh4->sr & Q ? 'Q':'.',
					(sh4->sr & I) >> 4,
					sh4->sr & S ? 'S':'.',
					sh4->sr & T ? 'T':'.');
			break;

		case CPUINFO_STR_REGISTER + SH4_PC:             sprintf(info->s, "PC  :%08X", sh4->pc); break;
		case CPUINFO_STR_REGISTER + SH4_SR:             sprintf(info->s, "SR  :%08X", sh4->sr); break;
		case CPUINFO_STR_REGISTER + SH4_PR:             sprintf(info->s, "PR  :%08X", sh4->pr); break;
		case CPUINFO_STR_REGISTER + SH4_GBR:            sprintf(info->s, "GBR :%08X", sh4->gbr); break;
		case CPUINFO_STR_REGISTER + SH4_VBR:            sprintf(info->s, "VBR :%08X", sh4->vbr); break;
		case CPUINFO_STR_REGISTER + SH4_DBR:            sprintf(info->s, "DBR :%08X", sh4->dbr); break;
		case CPUINFO_STR_REGISTER + SH4_MACH:           sprintf(info->s, "MACH:%08X", sh4->mach); break;
		case CPUINFO_STR_REGISTER + SH4_MACL:           sprintf(info->s, "MACL:%08X", sh4->macl); break;
		case CPUINFO_STR_REGISTER + SH4_R0:             sprintf(info->s, "R0  :%08X", sh4->r[ 0]); break;
		case CPUINFO_STR_REGISTER + SH4_R1:             sprintf(info->s, "R1  :%08X", sh4->r[ 1]); break;
		case CPUINFO_STR_REGISTER + SH4_R2:             sprintf(info->s, "R2  :%08X", sh4->r[ 2]); break;
		case CPUINFO_STR_REGISTER + SH4_R3:             sprintf(info->s, "R3  :%08X", sh4->r[ 3]); break;
		case CPUINFO_STR_REGISTER + SH4_R4:             sprintf(info->s, "R4  :%08X", sh4->r[ 4]); break;
		case CPUINFO_STR_REGISTER + SH4_R5:             sprintf(info->s, "R5  :%08X", sh4->r[ 5]); break;
		case CPUINFO_STR_REGISTER + SH4_R6:             sprintf(info->s, "R6  :%08X", sh4->r[ 6]); break;
		case CPUINFO_STR_REGISTER + SH4_R7:             sprintf(info->s, "R7  :%08X", sh4->r[ 7]); break;
		case CPUINFO_STR_REGISTER + SH4_R8:             sprintf(info->s, "R8  :%08X", sh4->r[ 8]); break;
		case CPUINFO_STR_REGISTER + SH4_R9:             sprintf(info->s, "R9  :%08X", sh4->r[ 9]); break;
		case CPUINFO_STR_REGISTER + SH4_R10:            sprintf(info->s, "R10 :%08X", sh4->r[10]); break;
		case CPUINFO_STR_REGISTER + SH4_R11:            sprintf(info->s, "R11 :%08X", sh4->r[11]); break;
		case CPUINFO_STR_REGISTER + SH4_R12:            sprintf(info->s, "R12 :%08X", sh4->r[12]); break;
		case CPUINFO_STR_REGISTER + SH4_R13:            sprintf(info->s, "R13 :%08X", sh4->r[13]); break;
		case CPUINFO_STR_REGISTER + SH4_R14:            sprintf(info->s, "R14 :%08X", sh4->r[14]); break;
		case CPUINFO_STR_REGISTER + SH4_R15:            sprintf(info->s, "R15 :%08X", sh4->r[15]); break;
		case CPUINFO_STR_REGISTER + SH4_EA:             sprintf(info->s, "EA  :%08X", sh4->ea);    break;
		case CPUINFO_STR_REGISTER + SH4_R0_BK0:         sprintf(info->s, "R0 BK 0  :%08X", sh4->rbnk[0][0]); break;
		case CPUINFO_STR_REGISTER + SH4_R1_BK0:         sprintf(info->s, "R1 BK 0 :%08X", sh4->rbnk[0][1]); break;
		case CPUINFO_STR_REGISTER + SH4_R2_BK0:         sprintf(info->s, "R2 BK 0 :%08X", sh4->rbnk[0][2]); break;
		case CPUINFO_STR_REGISTER + SH4_R3_BK0:         sprintf(info->s, "R3 BK 0 :%08X", sh4->rbnk[0][3]); break;
		case CPUINFO_STR_REGISTER + SH4_R4_BK0:         sprintf(info->s, "R4 BK 0 :%08X", sh4->rbnk[0][4]); break;
		case CPUINFO_STR_REGISTER + SH4_R5_BK0:         sprintf(info->s, "R5 BK 0 :%08X", sh4->rbnk[0][5]); break;
		case CPUINFO_STR_REGISTER + SH4_R6_BK0:         sprintf(info->s, "R6 BK 0 :%08X", sh4->rbnk[0][6]); break;
		case CPUINFO_STR_REGISTER + SH4_R7_BK0:         sprintf(info->s, "R7 BK 0 :%08X", sh4->rbnk[0][7]); break;
		case CPUINFO_STR_REGISTER + SH4_R0_BK1:         sprintf(info->s, "R0 BK 1 :%08X", sh4->rbnk[1][0]); break;
		case CPUINFO_STR_REGISTER + SH4_R1_BK1:         sprintf(info->s, "R1 BK 1 :%08X", sh4->rbnk[1][1]); break;
		case CPUINFO_STR_REGISTER + SH4_R2_BK1:         sprintf(info->s, "R2 BK 1 :%08X", sh4->rbnk[1][2]); break;
		case CPUINFO_STR_REGISTER + SH4_R3_BK1:         sprintf(info->s, "R3 BK 1 :%08X", sh4->rbnk[1][3]); break;
		case CPUINFO_STR_REGISTER + SH4_R4_BK1:         sprintf(info->s, "R4 BK 1 :%08X", sh4->rbnk[1][4]); break;
		case CPUINFO_STR_REGISTER + SH4_R5_BK1:         sprintf(info->s, "R5 BK 1 :%08X", sh4->rbnk[1][5]); break;
		case CPUINFO_STR_REGISTER + SH4_R6_BK1:         sprintf(info->s, "R6 BK 1 :%08X", sh4->rbnk[1][6]); break;
		case CPUINFO_STR_REGISTER + SH4_R7_BK1:         sprintf(info->s, "R7 BK 1 :%08X", sh4->rbnk[1][7]); break;
		case CPUINFO_STR_REGISTER + SH4_SPC:            sprintf(info->s, "SPC  :%08X", sh4->spc); break;
		case CPUINFO_STR_REGISTER + SH4_SSR:            sprintf(info->s, "SSR  :%08X", sh4->ssr); break;
		case CPUINFO_STR_REGISTER + SH4_SGR:            sprintf(info->s, "SGR  :%08X", sh4->sgr); break;
		case CPUINFO_STR_REGISTER + SH4_FPSCR:          sprintf(info->s, "FPSCR :%08X", sh4->fpscr); break;
		case CPUINFO_STR_REGISTER + SH4_FPUL:           sprintf(info->s, "FPUL :%08X", sh4->fpul); break;
#ifdef LSB_FIRST
		case CPUINFO_STR_REGISTER + SH4_FR0:            sprintf(info->s, "FR0  :%08X %f", FP_RS2( 0),(double)FP_RFS2( 0)); break;
		case CPUINFO_STR_REGISTER + SH4_FR1:            sprintf(info->s, "FR1  :%08X %f", FP_RS2( 1),(double)FP_RFS2( 1)); break;
		case CPUINFO_STR_REGISTER + SH4_FR2:            sprintf(info->s, "FR2  :%08X %f", FP_RS2( 2),(double)FP_RFS2( 2)); break;
		case CPUINFO_STR_REGISTER + SH4_FR3:            sprintf(info->s, "FR3  :%08X %f", FP_RS2( 3),(double)FP_RFS2( 3)); break;
		case CPUINFO_STR_REGISTER + SH4_FR4:            sprintf(info->s, "FR4  :%08X %f", FP_RS2( 4),(double)FP_RFS2( 4)); break;
		case CPUINFO_STR_REGISTER + SH4_FR5:            sprintf(info->s, "FR5  :%08X %f", FP_RS2( 5),(double)FP_RFS2( 5)); break;
		case CPUINFO_STR_REGISTER + SH4_FR6:            sprintf(info->s, "FR6  :%08X %f", FP_RS2( 6),(double)FP_RFS2( 6)); break;
		case CPUINFO_STR_REGISTER + SH4_FR7:            sprintf(info->s, "FR7  :%08X %f", FP_RS2( 7),(double)FP_RFS2( 7)); break;
		case CPUINFO_STR_REGISTER + SH4_FR8:            sprintf(info->s, "FR8  :%08X %f", FP_RS2( 8),(double)FP_RFS2( 8)); break;
		case CPUINFO_STR_REGISTER + SH4_FR9:            sprintf(info->s, "FR9  :%08X %f", FP_RS2( 9),(double)FP_RFS2( 9)); break;
		case CPUINFO_STR_REGISTER + SH4_FR10:           sprintf(info->s, "FR10 :%08X %f", FP_RS2(10),(double)FP_RFS2(10)); break;
		case CPUINFO_STR_REGISTER + SH4_FR11:           sprintf(info->s, "FR11 :%08X %f", FP_RS2(11),(double)FP_RFS2(11)); break;
		case CPUINFO_STR_REGISTER + SH4_FR12:           sprintf(info->s, "FR12 :%08X %f", FP_RS2(12),(double)FP_RFS2(12)); break;
		case CPUINFO_STR_REGISTER + SH4_FR13:           sprintf(info->s, "FR13 :%08X %f", FP_RS2(13),(double)FP_RFS2(13)); break;
		case CPUINFO_STR_REGISTER + SH4_FR14:           sprintf(info->s, "FR14 :%08X %f", FP_RS2(14),(double)FP_RFS2(14)); break;
		case CPUINFO_STR_REGISTER + SH4_FR15:           sprintf(info->s, "FR15 :%08X %f", FP_RS2(15),(double)FP_RFS2(15)); break;
		case CPUINFO_STR_REGISTER + SH4_XF0:            sprintf(info->s, "XF0  :%08X %f", FP_XS2( 0),(double)FP_XFS2( 0)); break;
		case CPUINFO_STR_REGISTER + SH4_XF1:            sprintf(info->s, "XF1  :%08X %f", FP_XS2( 1),(double)FP_XFS2( 1)); break;
		case CPUINFO_STR_REGISTER + SH4_XF2:            sprintf(info->s, "XF2  :%08X %f", FP_XS2( 2),(double)FP_XFS2( 2)); break;
		case CPUINFO_STR_REGISTER + SH4_XF3:            sprintf(info->s, "XF3  :%08X %f", FP_XS2( 3),(double)FP_XFS2( 3)); break;
		case CPUINFO_STR_REGISTER + SH4_XF4:            sprintf(info->s, "XF4  :%08X %f", FP_XS2( 4),(double)FP_XFS2( 4)); break;
		case CPUINFO_STR_REGISTER + SH4_XF5:            sprintf(info->s, "XF5  :%08X %f", FP_XS2( 5),(double)FP_XFS2( 5)); break;
		case CPUINFO_STR_REGISTER + SH4_XF6:            sprintf(info->s, "XF6  :%08X %f", FP_XS2( 6),(double)FP_XFS2( 6)); break;
		case CPUINFO_STR_REGISTER + SH4_XF7:            sprintf(info->s, "XF7  :%08X %f", FP_XS2( 7),(double)FP_XFS2( 7)); break;
		case CPUINFO_STR_REGISTER + SH4_XF8:            sprintf(info->s, "XF8  :%08X %f", FP_XS2( 8),(double)FP_XFS2( 8)); break;
		case CPUINFO_STR_REGISTER + SH4_XF9:            sprintf(info->s, "XF9  :%08X %f", FP_XS2( 9),(double)FP_XFS2( 9)); break;
		case CPUINFO_STR_REGISTER + SH4_XF10:           sprintf(info->s, "XF10 :%08X %f", FP_XS2(10),(double)FP_XFS2(10)); break;
		case CPUINFO_STR_REGISTER + SH4_XF11:           sprintf(info->s, "XF11 :%08X %f", FP_XS2(11),(double)FP_XFS2(11)); break;
		case CPUINFO_STR_REGISTER + SH4_XF12:           sprintf(info->s, "XF12 :%08X %f", FP_XS2(12),(double)FP_XFS2(12)); break;
		case CPUINFO_STR_REGISTER + SH4_XF13:           sprintf(info->s, "XF13 :%08X %f", FP_XS2(13),(double)FP_XFS2(13)); break;
		case CPUINFO_STR_REGISTER + SH4_XF14:           sprintf(info->s, "XF14 :%08X %f", FP_XS2(14),(double)FP_XFS2(14)); break;
		case CPUINFO_STR_REGISTER + SH4_XF15:           sprintf(info->s, "XF15 :%08X %f", FP_XS2(15),(double)FP_XFS2(15)); break;
#else
		case CPUINFO_STR_REGISTER + SH4_FR0:            sprintf(info->s, "FR0  :%08X %f", FP_RS( 0),(double)FP_RFS( 0)); break;
		case CPUINFO_STR_REGISTER + SH4_FR1:            sprintf(info->s, "FR1  :%08X %f", FP_RS( 1),(double)FP_RFS( 1)); break;
		case CPUINFO_STR_REGISTER + SH4_FR2:            sprintf(info->s, "FR2  :%08X %f", FP_RS( 2),(double)FP_RFS( 2)); break;
		case CPUINFO_STR_REGISTER + SH4_FR3:            sprintf(info->s, "FR3  :%08X %f", FP_RS( 3),(double)FP_RFS( 3)); break;
		case CPUINFO_STR_REGISTER + SH4_FR4:            sprintf(info->s, "FR4  :%08X %f", FP_RS( 4),(double)FP_RFS( 4)); break;
		case CPUINFO_STR_REGISTER + SH4_FR5:            sprintf(info->s, "FR5  :%08X %f", FP_RS( 5),(double)FP_RFS( 5)); break;
		case CPUINFO_STR_REGISTER + SH4_FR6:            sprintf(info->s, "FR6  :%08X %f", FP_RS( 6),(double)FP_RFS( 6)); break;
		case CPUINFO_STR_REGISTER + SH4_FR7:            sprintf(info->s, "FR7  :%08X %f", FP_RS( 7),(double)FP_RFS( 7)); break;
		case CPUINFO_STR_REGISTER + SH4_FR8:            sprintf(info->s, "FR8  :%08X %f", FP_RS( 8),(double)FP_RFS( 8)); break;
		case CPUINFO_STR_REGISTER + SH4_FR9:            sprintf(info->s, "FR9  :%08X %f", FP_RS( 9),(double)FP_RFS( 9)); break;
		case CPUINFO_STR_REGISTER + SH4_FR10:           sprintf(info->s, "FR10 :%08X %f", FP_RS(10),(double)FP_RFS(10)); break;
		case CPUINFO_STR_REGISTER + SH4_FR11:           sprintf(info->s, "FR11 :%08X %f", FP_RS(11),(double)FP_RFS(11)); break;
		case CPUINFO_STR_REGISTER + SH4_FR12:           sprintf(info->s, "FR12 :%08X %f", FP_RS(12),(double)FP_RFS(12)); break;
		case CPUINFO_STR_REGISTER + SH4_FR13:           sprintf(info->s, "FR13 :%08X %f", FP_RS(13),(double)FP_RFS(13)); break;
		case CPUINFO_STR_REGISTER + SH4_FR14:           sprintf(info->s, "FR14 :%08X %f", FP_RS(14),(double)FP_RFS(14)); break;
		case CPUINFO_STR_REGISTER + SH4_FR15:           sprintf(info->s, "FR15 :%08X %f", FP_RS(15),(double)FP_RFS(15)); break;
		case CPUINFO_STR_REGISTER + SH4_XF0:            sprintf(info->s, "XF0  :%08X %f", FP_XS( 0),(double)FP_XFS( 0)); break;
		case CPUINFO_STR_REGISTER + SH4_XF1:            sprintf(info->s, "XF1  :%08X %f", FP_XS( 1),(double)FP_XFS( 1)); break;
		case CPUINFO_STR_REGISTER + SH4_XF2:            sprintf(info->s, "XF2  :%08X %f", FP_XS( 2),(double)FP_XFS( 2)); break;
		case CPUINFO_STR_REGISTER + SH4_XF3:            sprintf(info->s, "XF3  :%08X %f", FP_XS( 3),(double)FP_XFS( 3)); break;
		case CPUINFO_STR_REGISTER + SH4_XF4:            sprintf(info->s, "XF4  :%08X %f", FP_XS( 4),(double)FP_XFS( 4)); break;
		case CPUINFO_STR_REGISTER + SH4_XF5:            sprintf(info->s, "XF5  :%08X %f", FP_XS( 5),(double)FP_XFS( 5)); break;
		case CPUINFO_STR_REGISTER + SH4_XF6:            sprintf(info->s, "XF6  :%08X %f", FP_XS( 6),(double)FP_XFS( 6)); break;
		case CPUINFO_STR_REGISTER + SH4_XF7:            sprintf(info->s, "XF7  :%08X %f", FP_XS( 7),(double)FP_XFS( 7)); break;
		case CPUINFO_STR_REGISTER + SH4_XF8:            sprintf(info->s, "XF8  :%08X %f", FP_XS( 8),(double)FP_XFS( 8)); break;
		case CPUINFO_STR_REGISTER + SH4_XF9:            sprintf(info->s, "XF9  :%08X %f", FP_XS( 9),(double)FP_XFS( 9)); break;
		case CPUINFO_STR_REGISTER + SH4_XF10:           sprintf(info->s, "XF10 :%08X %f", FP_XS(10),(double)FP_XFS(10)); break;
		case CPUINFO_STR_REGISTER + SH4_XF11:           sprintf(info->s, "XF11 :%08X %f", FP_XS(11),(double)FP_XFS(11)); break;
		case CPUINFO_STR_REGISTER + SH4_XF12:           sprintf(info->s, "XF12 :%08X %f", FP_XS(12),(double)FP_XFS(12)); break;
		case CPUINFO_STR_REGISTER + SH4_XF13:           sprintf(info->s, "XF13 :%08X %f", FP_XS(13),(double)FP_XFS(13)); break;
		case CPUINFO_STR_REGISTER + SH4_XF14:           sprintf(info->s, "XF14 :%08X %f", FP_XS(14),(double)FP_XFS(14)); break;
		case CPUINFO_STR_REGISTER + SH4_XF15:           sprintf(info->s, "XF15 :%08X %f", FP_XS(15),(double)FP_XFS(15)); break; //%01.2e
#endif
	}
}

CPU_GET_INFO( sh3 )
{
	switch (state)
	{
	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_FCT_RESET:                     info->reset = CPU_RESET_NAME(sh3);              break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME:                      strcpy(info->s, "SH-3");                break;
	case CPUINFO_STR_FAMILY:                    strcpy(info->s, "Hitachi SH7700");      break;

	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map64 = ADDRESS_MAP_NAME(sh3_internal_map); break;

	default:                                    CPU_GET_INFO_CALL(sh4);                 break;
	}
}

CPU_GET_INFO( sh3be )
{
	switch (state)
	{
	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_FCT_RESET:                     info->reset = CPU_RESET_NAME(sh3);              break;
	case CPUINFO_FCT_EXECUTE:                   info->execute = CPU_EXECUTE_NAME(sh4be);            break;
	case CPUINFO_FCT_DISASSEMBLE:               info->disassemble = CPU_DISASSEMBLE_NAME(sh4be);            break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME:                      strcpy(info->s, "SH-3");                break;
	case CPUINFO_STR_FAMILY:                    strcpy(info->s, "Hitachi SH7700");      break;

	case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map64 = ADDRESS_MAP_NAME(sh3_internal_map); break;

	case CPUINFO_INT_ENDIANNESS:                info->i = ENDIANNESS_BIG;               break;

	default:                                    CPU_GET_INFO_CALL(sh4);                 break;
	}
}

CPU_GET_INFO( sh4be )
{
	switch (state)
	{
	case CPUINFO_FCT_EXECUTE:                   info->execute = CPU_EXECUTE_NAME(sh4be);            break;
	case CPUINFO_FCT_DISASSEMBLE:               info->disassemble = CPU_DISASSEMBLE_NAME(sh4be);            break;
	case CPUINFO_INT_ENDIANNESS:                info->i = ENDIANNESS_BIG;               break;
	default:                                    CPU_GET_INFO_CALL(sh4);                 break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(SH3LE, sh3);
DEFINE_LEGACY_CPU_DEVICE(SH3BE, sh3be);
DEFINE_LEGACY_CPU_DEVICE(SH4LE, sh4);
DEFINE_LEGACY_CPU_DEVICE(SH4BE, sh4be);

#endif  // USE_SH4DRC
