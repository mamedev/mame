/*****************************************************************************
 *
 *   cp1610.c
 *   Portable CP1610 emulator (General Instrument CP1610)
 *
 *   Copyright Frank Palazzolo, all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     palazzol@comcast.net
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on Juergen Buchmueller's F8 emulation,
 *  and the 'General Instruments CP1610' data sheets.
 *  Special thanks to Joe Zbiciak for his GPL'd CP1610 emulator
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "cp1610.h"

#define S  0x80
#define Z  0x40
#define OV 0x20
#define C  0x10

typedef struct _cp1610_state cp1610_state;
struct _cp1610_state
{
	UINT16	r[8];	/* registers */
	UINT8	flags;	/* flags */
	int 	intr_enabled;
	//int       (*reset_callback)(cp1610_state *cpustate);
	device_irq_acknowledge_callback irq_callback;
	UINT16	intr_vector;
	int 	reset_state;
	int		intr_state;
	int		intrm_state;
	int 	reset_pending;
	int		intr_pending;
	int		intrm_pending;
	int		mask_interrupts;
	legacy_cpu_device *device;
	address_space *program;
	int icount;
};

INLINE cp1610_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == CP1610);
	return (cp1610_state *)downcast<legacy_cpu_device *>(device)->token();
}

#define cp1610_readop(A) cpustate->program->read_word((A)<<1)
#define cp1610_readmem16(A) cpustate->program->read_word((A)<<1)
#define cp1610_writemem16(A,B) cpustate->program->write_word((A)<<1,B)

/* clear all flags */
#define CLR_SZOC                \
	cpustate->flags &= ~(S|Z|C|OV)

/* clear sign and zero flags */
#define CLR_SZ              	\
	cpustate->flags &= ~(S|Z)

/* clear sign,zero, and carry flags */
#define CLR_SZC             	\
	cpustate->flags &= ~(S|Z|C)

/* set sign and zero flags */
#define SET_SZ(n)               \
	if (n == 0) 				\
		cpustate->flags |= Z;		\
	else						\
	if (n & 0x8000)				\
		cpustate->flags |= S

/* set sign zero, and carry flags */
#define SET_SZC(n,m)            \
	if (n == 0) 				\
		cpustate->flags |= Z;		\
	else						\
	if (n & 0x8000)				\
		cpustate->flags |= S;		\
	if ((n + m) & 0x10000)		\
		cpustate->flags |= C

/* set carry and overflow flags */
#define SET_COV(n,m,qq)         \
  { unsigned int pp = n + m;	\
	if (pp & 0x10000)			\
		cpustate->flags |= C;		\
	if (qq)						\
	{							\
		if ((n^pp)&(~(m^n))&0x8000)	\
			cpustate->flags |= OV; \
		if (m == 0x8000)		\
			cpustate->flags ^= OV;	\
	}							\
	else						\
	{							\
		if ((n^pp)&(~(m^n))&0x8000)	\
			cpustate->flags |= OV;		\
	}							\
  }

#if 0
#define SET_COV(n,m)         \
	if ((n&0x7fff)+(m&0x7fff) > 0x7fff)	\
	{							\
		if (!(cpustate->flags & C))\
			cpustate->flags |= OV;	\
	}							\
	else						\
	{							\
		if (cpustate->flags & C)	\
			cpustate->flags |= OV;	\
	}
#endif

/***********************************
 *  illegal opcodes
 ***********************************/
static void cp1610_illegal(cp1610_state *cpustate)
{
	logerror("cp1610 illegal opcode at 0x%04x\n", cpustate->r[7]);
}

/***************************************************
 *  S Z C OV 0 000 000 000
 *  - - - -  HLT
 ***************************************************/
static void cp1610_hlt(cp1610_state *cpustate)
{
	/* TBD */
	cpustate->icount -= 4;
}

/***************************************************
 *  S Z C OV 0 000 000 010
 *  - - - -  EIS
 ***************************************************/
static void cp1610_eis(cp1610_state *cpustate)
{
	cpustate->mask_interrupts = 1;
	cpustate->intr_enabled = 1;
	cpustate->icount -= 4;
}

/***************************************************
 *  S Z C OV 0 000 000 011
 *  - - - -  DIS
 ***************************************************/
static void cp1610_dis(cp1610_state *cpustate)
{
	cpustate->mask_interrupts = 1;
	cpustate->intr_enabled = 0;
	cpustate->icount -= 4;
}

/***************************************************
 *  S Z C OV 0 000 000 101
 *  - - - -  TCI
 ***************************************************/
static void cp1610_tci(cp1610_state *cpustate)
{
	/* TBD */
	cpustate->mask_interrupts = 1;
	cpustate->icount -= 4;
}

/***************************************************
 *  S Z C OV 0 000 000 110
 *  - - 0 -  CLRC
 ***************************************************/
static void cp1610_clrc(cp1610_state *cpustate)
{
	cpustate->mask_interrupts = 1;
	cpustate->flags &= ~C;
	cpustate->icount -= 4;
}

/***************************************************
 *  S Z C OV 0 000 000 111
 *  - - 1 -  SETC
 ***************************************************/
static void cp1610_setc(cp1610_state *cpustate)
{
	cpustate->mask_interrupts = 1;
	cpustate->flags |= C;
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 000 001 ddd
 *  x x - -  INCR  Rd
 ***************************************************/
static void cp1610_incr(cp1610_state *cpustate, int n)
{
	cpustate->r[n]++;
	CLR_SZ;
	SET_SZ(cpustate->r[n]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 000 010 ddd
 *  x x - -  DECR  Rd
 ***************************************************/
static void cp1610_decr(cp1610_state *cpustate, int n)
{
	cpustate->r[n]--;
	CLR_SZ;
	SET_SZ(cpustate->r[n]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 000 011 ddd
 *  x x - -  COMR  Rd
 ***************************************************/
static void cp1610_comr(cp1610_state *cpustate, int n)
{
	cpustate->r[n] ^= 0xffff;
	CLR_SZ;
	SET_SZ(cpustate->r[n]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 000 100 ddd
 *  x x x x  NEGR  Rd
 ***************************************************/
static void cp1610_negr(cp1610_state *cpustate, int n)
{
	UINT32 temp;
	CLR_SZOC;
	temp = (cpustate->r[n] ^ 0xffff) + 1;
	SET_COV(0,temp,1);
	cpustate->r[n] = temp&0xffff;
	SET_SZ(cpustate->r[n]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 000 101 ddd
 *  x x x x  ADCR  Rd
 ***************************************************/
static void cp1610_adcr(cp1610_state *cpustate, int n)
{
	UINT16 offset = 0;
	if (cpustate->flags & C)
		offset = 1;
	CLR_SZOC;
	SET_COV(cpustate->r[n],offset,0);
	cpustate->r[n] += offset;
	SET_SZ(cpustate->r[n]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 000 110 0dd
 *  - - - -  GSWD  Rd
 ***************************************************/
static void cp1610_gswd(cp1610_state *cpustate, int n)
{
	cpustate->r[n] = (cpustate->flags << 8) + cpustate->flags;
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 000 110 10x
 *  - - - -  NOP
 ***************************************************/
static void cp1610_nop(cp1610_state *cpustate)
{
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 000 110 11x
 *  - - - -  SIN
 ***************************************************/
static void cp1610_sin(cp1610_state *cpustate)
{
	/* TBD */
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 000 111 sss
 *  x x x x  RSWD Rs
 ***************************************************/
static void cp1610_rswd(cp1610_state *cpustate, int n)
{
	cpustate->flags = cpustate->r[n];
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 001 000 0rr
 *  x x - -  SWAP Rr,1
 ***************************************************/
static void cp1610_swap(cp1610_state *cpustate, int r)
{
	UINT8 temp;
	cpustate->mask_interrupts = 1;
	CLR_SZ;
	temp = cpustate->r[r] >> 8;
	cpustate->r[r] = (cpustate->r[r] << 8) | temp;
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 001 000 1rr
 *  x x - -  SWAP Rr,2
 ***************************************************/
static void cp1610_dswap(cp1610_state *cpustate, int r)
{
	/* This instruction was not officially supported by GI */
	UINT16 temp;
	cpustate->mask_interrupts = 1;
	CLR_SZ;
	temp = cpustate->r[r] & 0xff;
	cpustate->r[r] = (temp << 8) | temp;
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 0 001 001 0rr
 *  x x - -  SLL Rr,1
 ***************************************************/
static void cp1610_sll_1(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZ;
	cpustate->r[r] <<= 1;
	SET_SZ(cpustate->r[r]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 001 001 1rr
 *  x x - -  SLL Rr,2
 ***************************************************/
static void cp1610_sll_2(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZ;
	cpustate->r[r] <<= 2;
	SET_SZ(cpustate->r[r]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 0 001 010 0rr
 *  x x x -  RLC Rr,1
 ***************************************************/
static void cp1610_rlc_1(cp1610_state *cpustate, int r)
{
	UINT16 offset = 0;
	cpustate->mask_interrupts = 1;
	if (cpustate->flags & C)
		offset = 1;
	CLR_SZC;
	if (cpustate->r[r] & 0x8000)
		cpustate->flags |= C;
	cpustate->r[r] = (cpustate->r[r] << 1) + offset;
	SET_SZ(cpustate->r[r]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 001 010 1rr
 *  x x x x  RLC Rr,2
 ***************************************************/
static void cp1610_rlc_2(cp1610_state *cpustate, int r)
{
	UINT16 offset = 0;
	cpustate->mask_interrupts = 1;
	switch(cpustate->flags & (C | OV))
	{
		case 0:
			offset = 0;
		break;
		case OV:
			offset = 1;
		break;
		case C:
			offset = 2;
		break;
		case (C | OV):
			offset = 3;
		break;
	}

	CLR_SZOC;
	if (cpustate->r[r] & 0x8000)
		cpustate->flags |= C;
	if (cpustate->r[r] & 0x4000)
		cpustate->flags |= OV;
	cpustate->r[r] <<= 2;
	cpustate->r[r] += offset;
	SET_SZ(cpustate->r[r]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 0 001 011 0rr
 *  x x x -  SLLC Rr,1
 ***************************************************/
static void cp1610_sllc_1(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZC;
	if (cpustate->r[r] & 0x8000)
		cpustate->flags |= C;
	cpustate->r[r] <<= 1;
	SET_SZ(cpustate->r[r]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 001 011 1rr
 *  x x x x  SLLC Rr,2
 ***************************************************/
static void cp1610_sllc_2(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZOC;
	if (cpustate->r[r] & 0x8000)
		cpustate->flags |= C;
	if (cpustate->r[r] & 0x4000)
		cpustate->flags |= OV;
	cpustate->r[r] <<= 2;
	SET_SZ(cpustate->r[r]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 0 001 100 0rr
 *  x x - -  SLR Rr,1
 ***************************************************/
static void cp1610_slr_1(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZ;
	cpustate->r[r] >>= 1;
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 001 100 1rr
 *  x x - -  SLR Rr,2
 ***************************************************/
static void cp1610_slr_2(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZ;
	cpustate->r[r] >>= 2;
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 0 001 101 0rr
 *  x x - -  SAR Rr,1
 ***************************************************/
static void cp1610_sar_1(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZ;
	cpustate->r[r] = (UINT16)(((INT16)(cpustate->r[r])) >> 1);
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 001 101 1rr
 *  x x - -  SAR Rr,2
 ***************************************************/
static void cp1610_sar_2(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZ;
	cpustate->r[r] = (UINT16)(((INT16)(cpustate->r[r])) >> 2);
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 0 001 110 0rr
 *  x x x -  RRC Rr,1
 ***************************************************/
static void cp1610_rrc_1(cp1610_state *cpustate, int r)
{
	UINT16 offset = 0;
	cpustate->mask_interrupts = 1;
	if (cpustate->flags & C)
		offset = 0x8000;
	CLR_SZC;
	if (cpustate->r[r] & 1)
		cpustate->flags |= C;
	cpustate->r[r] >>= 1;
	cpustate->r[r] += offset;
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 001 110 1rr
 *  x x x x  RRC Rr,2
 ***************************************************/
static void cp1610_rrc_2(cp1610_state *cpustate, int r)
{
	UINT16 offset = 0;
	cpustate->mask_interrupts = 1;
	if (cpustate->flags & C)
		offset |= 0x4000;
	if (cpustate->flags & OV)
		offset |= 0x8000;
	CLR_SZOC;
	if (cpustate->r[r] & 1)
		cpustate->flags |= C;
	if (cpustate->r[r] & 2)
		cpustate->flags |= OV;
	cpustate->r[r] >>= 2;
	cpustate->r[r] += offset;
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 0 001 111 0rr
 *  x x x -  SARC Rr,1
 ***************************************************/
static void cp1610_sarc_1(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZC;
	if (cpustate->r[r] & 1)
		cpustate->flags |= C;
	cpustate->r[r] = (UINT16)(((INT16)cpustate->r[r]) >> 1);
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 001 111 1rr
 *  x x x x  SARC Rr,2
 ***************************************************/
static void cp1610_sarc_2(cp1610_state *cpustate, int r)
{
	cpustate->mask_interrupts = 1;
	CLR_SZOC;
	if (cpustate->r[r] & 1)
		cpustate->flags |= C;
	if (cpustate->r[r] & 2)
		cpustate->flags |= OV;
	cpustate->r[r] = (UINT16)(((INT16)cpustate->r[r]) >> 2);
	SET_SZ(cpustate->r[r]);
	/* S flag is set on bit 7 not bit 15 */
	cpustate->flags &= ~S;
	if (cpustate->r[r] & 0x80)
		cpustate->flags |= S;
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 010 sss sss
 *  x x - -  TSTR Rs
 ***************************************************/
static void cp1610_tstr(cp1610_state *cpustate, int n)
{
	CLR_SZ;
	SET_SZ(cpustate->r[n]);
	cpustate->icount -= 6;
	if (n > 5)
		cpustate->icount -= 1;
}

/***************************************************
 *  S Z C OV 0 010 sss ddd      (sss != ddd)
 *  x x - -  MOVR Rs,Rd
 ***************************************************/
static void cp1610_movr(cp1610_state *cpustate, int s, int d)
{
	CLR_SZ;
	cpustate->r[d] = cpustate->r[s];
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 6;
	if (d > 5)
		cpustate->icount -= 1;
}

/***************************************************
 *  S Z C OV 0 011 sss ddd
 *  x x x x  ADDR Rs, Rd
 ***************************************************/
static void cp1610_addr(cp1610_state *cpustate, int s, int d)
{
	CLR_SZOC;
	SET_COV(cpustate->r[s],cpustate->r[d],0);
	cpustate->r[d] += cpustate->r[s];
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 100 sss ddd
 *  x x x x  SUBR Rs, Rd
 ***************************************************/
static void cp1610_subr(cp1610_state *cpustate, int s, int d)
{
	CLR_SZOC;
	SET_COV(cpustate->r[d],(UINT32)((cpustate->r[s]^0xffff)+1),1);
	cpustate->r[d] -= cpustate->r[s];
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 101 sss ddd
 *  x x x x  CMPR Rs, Rd
 ***************************************************/
static void cp1610_cmpr(cp1610_state *cpustate, int s, int d)
{
	UINT16 temp;
	CLR_SZOC;
	SET_COV(cpustate->r[d],(UINT32)((cpustate->r[s]^0xffff)+1),1);
	temp = cpustate->r[d] - cpustate->r[s];
	SET_SZ(temp);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 110 sss ddd
 *  x x - -  ANDR Rs, Rd
 ***************************************************/
static void cp1610_andr(cp1610_state *cpustate, int s, int d)
{
	CLR_SZ;
	cpustate->r[d] &= cpustate->r[s];
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 111 sss ddd  (sss != ddd)
 *  x x - -  XORR Rs, Rd
 ***************************************************/
static void cp1610_xorr(cp1610_state *cpustate, int s, int d)
{
	CLR_SZ;
	cpustate->r[d] ^= cpustate->r[s];
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 0 111 ddd ddd
 *  x x - -  CLRR Rd
 ***************************************************/
static void cp1610_clrr(cp1610_state *cpustate, int d)
{
	CLR_SZ;
	cpustate->r[d] = 0;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 6;
}

/***************************************************
 *  S Z C OV 1 000 s00 000 p ppp ppp ppp ppp ppp
 *  - - - -  B ADDR
 ***************************************************/
static void cp1610_b(cp1610_state *cpustate, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	cpustate->r[7] += (offset ^ dir);
	cpustate->icount -= 9;
}

/***************************************************
 *  S Z C OV 1 000 s01 000 p ppp ppp ppp ppp ppp
 *  - - - -  NOPP
 ***************************************************/
static void cp1610_nopp(cp1610_state *cpustate, int dir)
{
	cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	cpustate->icount -= 7;
}

/***************************************************
 *  S Z C OV 1 000 s00 001 p ppp ppp ppp ppp ppp
 *  - - - -  BC ADDR
 ***************************************************/
static void cp1610_bc(cp1610_state *cpustate, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & C)
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s01 001 p ppp ppp ppp ppp ppp
 *  - - - -  BNC ADDR
 ***************************************************/
static void cp1610_bnc(cp1610_state *cpustate, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (!(cpustate->flags & C))
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s00 010 p ppp ppp ppp ppp ppp
 *  - - - -  BOV ADDR
 ***************************************************/
static void cp1610_bov(cp1610_state *cpustate, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & OV)
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s01 010 p ppp ppp ppp ppp ppp
 *  - - - -  BNOV ADDR
 ***************************************************/
static void cp1610_bnov(cp1610_state *cpustate, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (!(cpustate->flags & OV))
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s00 011 p ppp ppp ppp ppp ppp
 *  - - - -  BPL ADDR
 ***************************************************/
static void cp1610_bpl(cp1610_state *cpustate, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (!(cpustate->flags & S))
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s01 011 p ppp ppp ppp ppp ppp
 *  - - - -  BMI ADDR
 ***************************************************/
static void cp1610_bmi(cp1610_state *cpustate, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & S)
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s00 100 p ppp ppp ppp ppp ppp
 *  - - - -  BZE ADDR
 ***************************************************/
static void cp1610_bze(cp1610_state *cpustate, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & Z)
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s01 100 p ppp ppp ppp ppp ppp
 *  - - - -  BNZE ADDR
 ***************************************************/
static void cp1610_bnze(cp1610_state *cpustate, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (!(cpustate->flags & Z))
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s00 101 p ppp ppp ppp ppp ppp
 *  - - - -  BLT ADDR
 ***************************************************/
static void cp1610_blt(cp1610_state *cpustate, int dir)
{
	int condition1 = 0;
	int condition2 = 0;
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & S) condition1 = 1;
	if (cpustate->flags & OV) condition2 = 1;
	if (condition1 ^ condition2)
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s01 101 p ppp ppp ppp ppp ppp
 *  - - - -  BGE ADDR
 ***************************************************/
static void cp1610_bge(cp1610_state *cpustate, int dir)
{
	int condition1 = 0;
	int condition2 = 0;
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & S) condition1 = 1;
	if (cpustate->flags & OV) condition2 = 1;
	if (!(condition1 ^ condition2))
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s00 110 p ppp ppp ppp ppp ppp
 *  - - - -  BLE ADDR
 ***************************************************/
static void cp1610_ble(cp1610_state *cpustate, int dir)
{
	int condition1 = 0;
	int condition2 = 0;
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & S) condition1 = 1;
	if (cpustate->flags & OV) condition2 = 1;
	if ((cpustate->flags & Z) || (condition1 ^ condition2))
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s01 110 p ppp ppp ppp ppp ppp
 *  - - - -  BGT ADDR
 ***************************************************/
static void cp1610_bgt(cp1610_state *cpustate, int dir)
{
	int condition1 = 0;
	int condition2 = 0;
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & S) condition1 = 1;
	if (cpustate->flags & OV) condition2 = 1;
	if (!((cpustate->flags & Z) || (condition1 ^ condition2)))
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s00 111 p ppp ppp ppp ppp ppp
 *  - - - -  BUSC ADDR
 ***************************************************/
static void cp1610_busc(cp1610_state *cpustate, int dir)
{
	int condition1 = 0;
	int condition2 = 0;
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & C) condition1 = 1;
	if (cpustate->flags & S) condition2 = 1;
	if (condition1 ^ condition2)
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s01 111 p ppp ppp ppp ppp ppp
 *  - - - -  BESC ADDR
 ***************************************************/
static void cp1610_besc(cp1610_state *cpustate, int dir)
{
	int condition1 = 0;
	int condition2 = 0;
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	if (cpustate->flags & C) condition1 = 1;
	if (cpustate->flags & S) condition2 = 1;
	if (!(condition1 ^ condition2))
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 000 s1e e p ppp ppp ppp ppp ppp
 *  - - - -  BEXT ADDR, eeee
 ***************************************************/
static void cp1610_bext(cp1610_state *cpustate, int ext, int dir)
{
	UINT16 offset = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	/* TBD */
	if (0)
	{
		cpustate->r[7] += (offset ^ dir);
		cpustate->icount -= 9;
	}
	else
	{
		cpustate->icount -= 7;
	}
}

/***************************************************
 *  S Z C OV 1 001 000 sss a aaa aaa aaa aaa aaa
 *  - - - -  MVO Rs, ADDR
 ***************************************************/
static void cp1610_mvo(cp1610_state *cpustate, int s)
{
	UINT16 addr;
	cpustate->mask_interrupts = 1;
	addr = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	cp1610_writemem16(addr,cpustate->r[s]);
	cpustate->icount -= 11;
}

/***************************************************
 *  S Z C OV 1 001 mmm sss  (mmm = 0xx)
 *  - - - -  MVO@ Rs, Rm
 ***************************************************/
static void cp1610_mvoat(cp1610_state *cpustate, int s, int m)
{
	cpustate->mask_interrupts = 1;
	cp1610_writemem16(cpustate->r[m],cpustate->r[s]);
	cpustate->icount -= 9;
}

/***************************************************
 *  S Z C OV 1 001 mmm sss  (m = 10x or 110)
 *  - - - -  MVO@ Rs, Rm
 ***************************************************/
static void cp1610_mvoat_i(cp1610_state *cpustate, int s, int m)
{
	cpustate->mask_interrupts = 1;
	cp1610_writemem16(cpustate->r[m],cpustate->r[s]);
	cpustate->r[m]++;
	cpustate->icount -= 9;
}

/***************************************************
 *  S Z C OV 1 001 111 sss I III III III III III
 *  - - - -  MVOI Rs, II
 ***************************************************/
static void cp1610_mvoi(cp1610_state *cpustate, int s)
{
	cpustate->mask_interrupts = 1;
	cp1610_writemem16(cpustate->r[7],cpustate->r[s]);
	cpustate->r[7]++;
	cpustate->icount -= 9;
}

/***************************************************
 *  S Z C OV 1 010 000 ddd a aaa aaa aaa aaa aaa
 *  - - - -  MVI ADDR, Rd
 ***************************************************/
static void cp1610_mvi(cp1610_state *cpustate, int d)
{
	UINT16 addr = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	cpustate->r[d] = cp1610_readmem16(addr);
	cpustate->icount -= 10;
}

/***************************************************
 *  S Z C OV 1 010 mmm ddd  (mmm = 0xx)
 *  - - - -  MVI@ Rm, Rd
 ***************************************************/
static void cp1610_mviat(cp1610_state *cpustate, int m, int d)
{
	cpustate->r[d] = cp1610_readmem16(cpustate->r[m]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 010 mmm ddd  (mmm = 10x)
 *  - - - -  MVI@ Rm, Rd
 ***************************************************/
static void cp1610_mviat_i(cp1610_state *cpustate, int m, int d)
{
	UINT16 temp = cp1610_readmem16(cpustate->r[m]);
	cpustate->r[m]++;
	cpustate->r[d] = temp;
	cpustate->icount -= 8;

	//cpustate->r[d] = cp1610_readmem16(cpustate->r[m]);
	//cpustate->r[m]++;
	//cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 010 110 ddd
 *  - - - -  PULR Rd
 ***************************************************/
static void cp1610_pulr(cp1610_state *cpustate, int d)
{
	cpustate->r[6]--;
	cpustate->r[d] = cp1610_readmem16(cpustate->r[6]);
	cpustate->icount -= 11;
}

/***************************************************
 *  S Z C OV 1 010 111 ddd I III III III III III
 *  - - - -  MVII II, Rd
 ***************************************************/
static void cp1610_mvii(cp1610_state *cpustate, int d)
{
	UINT16 temp = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	cpustate->r[d] = temp;
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 011 000 ddd a aaa aaa aaa aaa aaa
 *  x x x x  ADD ADDR, Rd
 ***************************************************/
static void cp1610_add(cp1610_state *cpustate, int d)
{
	UINT16 addr = cp1610_readop(cpustate->r[7]);
	UINT16 data = cp1610_readmem16(addr);
	cpustate->r[7]++;
	CLR_SZOC;
	SET_COV(cpustate->r[d],data,0);
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 10;
}

/***************************************************
 *  S Z C OV 1 011 mmm ddd  (mmm = 0xx)
 *  x x x x  ADD@ Rm, Rd
 ***************************************************/
static void cp1610_addat(cp1610_state *cpustate, int m, int d)
{
	UINT16 data = cp1610_readmem16(cpustate->r[m]);
	CLR_SZOC;
	SET_COV(cpustate->r[d],data,0);
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 011 mmm ddd  (mmm = 10x)
 *  x x x x  ADD@ Rm, Rd
 ***************************************************/
static void cp1610_addat_i(cp1610_state *cpustate, int m, int d)
{
	UINT16 data = cp1610_readmem16(cpustate->r[m]);
	cpustate->r[m]++;
	CLR_SZOC;
	SET_COV(cpustate->r[d],data,0);
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 011 mmm ddd  (mmm = 110)
 *  x x x x  ADD@ Rm, Rd
 ***************************************************/
static void cp1610_addat_d(cp1610_state *cpustate, int m, int d)
{
	UINT16 data;
	cpustate->r[m]--;
	data = cp1610_readmem16(cpustate->r[m]);
	CLR_SZOC;
	SET_COV(cpustate->r[d],data,0);
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 11;
}

/***************************************************
 *  S Z C OV 1 011 111 ddd I III III III III III
 *  x x x x  ADDI II, Rd
 ***************************************************/
static void cp1610_addi(cp1610_state *cpustate, int d)
{
	UINT16 data;
	data = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	CLR_SZOC;
	SET_COV(cpustate->r[d],data,0);
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 100 000 ddd a aaa aaa aaa aaa aaa
 *  x x x x  SUB ADDR, Rd
 ***************************************************/
static void cp1610_sub(cp1610_state *cpustate, int d)
{
	UINT16 addr = cp1610_readop(cpustate->r[7]);
	UINT32 data = cp1610_readmem16(addr);
	cpustate->r[7]++;
	CLR_SZOC;
	data = (data ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 10;
}

/***************************************************
 *  S Z C OV 1 100 mmm ddd  (mmm = 0xx)
 *  x x x x  SUB@ Rm, Rd
 ***************************************************/
static void cp1610_subat(cp1610_state *cpustate, int m, int d)
{
	UINT32 data = cp1610_readmem16(cpustate->r[m]);
	CLR_SZOC;
	data = (data ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 100 mmm ddd  (mmm = 10x)
 *  x x x x  SUB@ Rm, Rd
 ***************************************************/
static void cp1610_subat_i(cp1610_state *cpustate, int m, int d)
{
	UINT32 data = cp1610_readmem16(cpustate->r[m]);
	cpustate->r[m]++;
	CLR_SZOC;
	data = (data ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 100 mmm ddd  (mmm = 110)
 *  x x x x  SUB@ Rm, Rd
 ***************************************************/
static void cp1610_subat_d(cp1610_state *cpustate, int m, int d)
{
	UINT32 data;
	cpustate->r[m]--;
	data = cp1610_readmem16(cpustate->r[m]);
	CLR_SZOC;
	data = (data ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 11;
}

/***************************************************
 *  S Z C OV 1 100 111 ddd I III III III III III
 *  x x x x  SUBI II, Rd
 ***************************************************/
static void cp1610_subi(cp1610_state *cpustate, int d)
{
	UINT32 data;
	data = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	data = (data ^ 0xffff) + 1;
	CLR_SZOC;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	cpustate->r[d] += data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 101 000 ddd a aaa aaa aaa aaa aaa
 *  x x x x  CMP ADDR, Rd
 ***************************************************/
static void cp1610_cmp(cp1610_state *cpustate, int d)
{
	UINT16 addr = cp1610_readop(cpustate->r[7]);
	UINT32 data = cp1610_readmem16(addr);
	UINT16 res;
	cpustate->r[7]++;
	CLR_SZOC;
	data = (data ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	res = cpustate->r[d] + data;
	SET_SZ(res);
	cpustate->icount -= 10;
}

/***************************************************
 *  S Z C OV 1 101 mmm ddd  (mmm = 0xx)
 *  x x x x  CMP@ Rm, Rd
 ***************************************************/
static void cp1610_cmpat(cp1610_state *cpustate, int m, int d)
{
	UINT32 data = cp1610_readmem16(cpustate->r[m]);
	UINT16 res;
	CLR_SZOC;
	data = (data ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	res = cpustate->r[d] + data;
	SET_SZ(res);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 101 mmm ddd  (mmm = 10x)
 *  x x x x  CMP@ Rm, Rd
 ***************************************************/
static void cp1610_cmpat_i(cp1610_state *cpustate, int m, int d)
{
	UINT32 data = cp1610_readmem16(cpustate->r[m]);
	UINT16 res;
	cpustate->r[m]++;
	CLR_SZOC;
	data = (data ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	res = cpustate->r[d] + data;
	SET_SZ(res);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 101 mmm ddd  (mmm = 110)
 *  x x x x  CMP@ Rm, Rd
 ***************************************************/
static void cp1610_cmpat_d(cp1610_state *cpustate, int m, int d)
{
	UINT32 data;
	UINT16 res;
	cpustate->r[m]--;
	data = cp1610_readmem16(cpustate->r[m]);
	CLR_SZOC;
	data = (data ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	res = cpustate->r[d] + data;
	SET_SZ(res);
	cpustate->icount -= 11;
}

/***************************************************
 *  S Z C OV 1 101 111 ddd I III III III III III
 *  x x x x  CMPI II, Rd
 ***************************************************/
static void cp1610_cmpi(cp1610_state *cpustate, int d)
{
	UINT32 data;
	UINT16 res;
	data = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	data = (data ^ 0xffff) + 1;
	CLR_SZOC;
	SET_COV(cpustate->r[d],data,1);
	data &= 0xffff;
	res = cpustate->r[d] + data;
	SET_SZ(res);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 110 000 ddd a aaa aaa aaa aaa aaa
 *  x x - -  AND ADDR, Rd
 ***************************************************/
static void cp1610_and(cp1610_state *cpustate, int d)
{
	UINT16 addr = cp1610_readop(cpustate->r[7]);
	UINT16 data = cp1610_readmem16(addr);
	cpustate->r[7]++;
	CLR_SZ;
	cpustate->r[d] &= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 10;
}

/***************************************************
 *  S Z C OV 1 110 mmm ddd  (mmm = 0xx)
 *  x x - -  AND@ Rm, Rd
 ***************************************************/
static void cp1610_andat(cp1610_state *cpustate, int m, int d)
{
	UINT16 data = cp1610_readmem16(cpustate->r[m]);
	CLR_SZ;
	cpustate->r[d] &= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 110 mmm ddd  (mmm = 10x)
 *  x x - -  AND@ Rm, Rd
 ***************************************************/
static void cp1610_andat_i(cp1610_state *cpustate, int m, int d)
{
	UINT16 data = cp1610_readmem16(cpustate->r[m]);
	cpustate->r[m]++;
	CLR_SZ;
	cpustate->r[d] &= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 110 mmm ddd  (mmm = 110)
 *  x x - -  AND@ Rm, Rd
 ***************************************************/
static void cp1610_andat_d(cp1610_state *cpustate, int m, int d)
{
	UINT16 data;
	cpustate->r[m]--;
	data = cp1610_readmem16(cpustate->r[m]);
	CLR_SZ;
	cpustate->r[d] &= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 11;
}

/***************************************************
 *  S Z C OV 1 110 111 ddd I III III III III III
 *  x x - -  AND II, Rd
 ***************************************************/
static void cp1610_andi(cp1610_state *cpustate, int d)
{
	UINT16 data;
	data = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	CLR_SZ;
	cpustate->r[d] &= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 111 000 ddd a aaa aaa aaa aaa aaa
 *  x x - -  XOR ADDR, Rd
 ***************************************************/
static void cp1610_xor(cp1610_state *cpustate, int d)
{
	UINT16 addr = cp1610_readop(cpustate->r[7]);
	UINT16 data = cp1610_readmem16(addr);
	cpustate->r[7]++;
	CLR_SZ;
	cpustate->r[d] ^= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 10;
}

/***************************************************
 *  S Z C OV 1 111 mmm ddd  (mmm = 0xx)
 *  x x - -  XOR@ Rm, Rd
 ***************************************************/
static void cp1610_xorat(cp1610_state *cpustate, int m, int d)
{
	UINT16 data = cp1610_readmem16(cpustate->r[m]);
	CLR_SZ;
	cpustate->r[d] ^= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 111 mmm ddd  (mmm = 10x)
 *  x x - -  XOR@ Rm, Rd
 ***************************************************/
static void cp1610_xorat_i(cp1610_state *cpustate, int m, int d)
{
	UINT16 data = cp1610_readmem16(cpustate->r[m]);
	cpustate->r[m]++;
	CLR_SZ;
	cpustate->r[d] ^= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

/***************************************************
 *  S Z C OV 1 111 mmm ddd  (mmm = 110)
 *  x x - -  XOR@ Rm, Rd
 ***************************************************/
static void cp1610_xorat_d(cp1610_state *cpustate, int m, int d)
{
	UINT16 data;
	cpustate->r[m]--;
	data = cp1610_readmem16(cpustate->r[m]);
	CLR_SZ;
	cpustate->r[d] ^= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 11;
}

/***************************************************
 *  S Z C OV 1 111 111 ddd I III III III III III
 *  x x - -  XOR II, Rd
 ***************************************************/
static void cp1610_xori(cp1610_state *cpustate, int d)
{
	UINT16 data;
	data = cp1610_readop(cpustate->r[7]);
	cpustate->r[7]++;
	CLR_SZ;
	cpustate->r[d] ^= data;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 8;
}

static CPU_RESET( cp1610 )
{
	/* This is how we set the reset vector */
	device->set_input_line(CP1610_RESET, PULSE_LINE);
}

/***************************************************
 *  S Z C OV 0x001  1 010 mmm ddd   (mmm = 0xx)
 *  - - - -  SDBD, MVI@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_mviat(cp1610_state *cpustate, int r, int d)
{
	cpustate->r[d] = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[d] |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 010 mmm ddd   (mmm = 10x)
 *  - - - -  SDBD, MVI@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_mviat_i(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]++;
	cpustate->r[d] = temp;
	temp = (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[r]++;
	cpustate->r[d] |= temp;
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 010 mmm ddd   (mmm = 101)
 *  - - - -  SDBD, MVI@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_mviat_d(cp1610_state *cpustate, int r, int d)
{
	cpustate->r[r]--;
	cpustate->r[d] = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]--;
	cpustate->r[d] |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->icount -= 17;
}

/************************************************************************
 *  S Z C OV 0x001  1 010 111 ddd  x xxx xLL LLL LLL  x xxx xUU UUU UUU
 *  - - - -  SDBD, MVII I-I, Rd
 ************************************************************************/
static void cp1610_sdbd_mvii(cp1610_state *cpustate, int d)
{
	UINT16 addr;
	addr = cp1610_readop(cpustate->r[7]) & 0xff;
	cpustate->r[7]++;
	addr |= (cp1610_readop(cpustate->r[7]) << 8);
	cpustate->r[7]++;
	cpustate->r[d] = addr;
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 011 mmm ddd   (mmm = 0xx)
 *  x x x x  SDBD, ADD@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_addat(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	CLR_SZOC;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	SET_COV(cpustate->r[d],temp,0);
	cpustate->r[d] += temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 011 mmm ddd   (mmm = 10x)
 *  x x x x  SDBD, ADD@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_addat_i(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	CLR_SZOC;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]++;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[r]++;
	SET_COV(cpustate->r[d],temp,0);
	cpustate->r[d] += temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 011 mmm ddd   (mmm = 101)
 *  x x x x  SDBD, ADD@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_addat_d(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	CLR_SZOC;
	cpustate->r[r]--;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]--;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	SET_COV(cpustate->r[d],temp,0);
	cpustate->r[d] += temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 17;
}

/************************************************************************
 *  S Z C OV 0x001  1 011 111 ddd  x xxx xLL LLL LLL  x xxx xUU UUU UUU
 *  x x x x  SDBD, ADDI I-I, Rd
 ************************************************************************/
static void cp1610_sdbd_addi(cp1610_state *cpustate, int d)
{
	UINT16 addr;
	UINT16 temp;
	CLR_SZOC;
	addr = cp1610_readop(cpustate->r[7]) & 0xff;
	cpustate->r[7]++;
	addr |= (cp1610_readop(cpustate->r[7]) << 8);
	cpustate->r[7]++;
	temp = addr;
	SET_COV(cpustate->r[d],temp,0);
	cpustate->r[d] += temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 100 mmm ddd   (mmm = 0xx)
 *  x x x x  SDBD, SUB@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_subat(cp1610_state *cpustate, int r, int d)
{
	UINT32 temp;
	CLR_SZOC;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	temp = (temp ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],temp,1);
	temp &= 0xffff;
	cpustate->r[d] += temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 100 mmm ddd   (mmm = 10x)
 *  x x x x  SDBD, SUB@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_subat_i(cp1610_state *cpustate, int r, int d)
{
	UINT32 temp;
	CLR_SZOC;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]++;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[r]++;
	temp = (temp ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],temp,1);
	temp &= 0xffff;
	cpustate->r[d] += temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 100 mmm ddd   (mmm = 101)
 *  x x x x  SDBD, SUB@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_subat_d(cp1610_state *cpustate, int r, int d)
{
	UINT32 temp;
	CLR_SZOC;
	cpustate->r[r]--;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]--;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	temp = (temp ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],temp,1);
	temp &= 0xffff;
	cpustate->r[d] += temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 17;
}

/************************************************************************
 *  S Z C OV 0x001  1 100 111 ddd  x xxx xLL LLL LLL  x xxx xUU UUU UUU
 *  x x x x  SDBD, SUBI I-I, Rd
 ************************************************************************/
static void cp1610_sdbd_subi(cp1610_state *cpustate, int d)
{
	UINT16 addr;
	UINT32 temp;
	CLR_SZOC;
	addr = cp1610_readop(cpustate->r[7]) & 0xff;
	cpustate->r[7]++;
	addr |= (cp1610_readop(cpustate->r[7]) << 8);
	cpustate->r[7]++;
	temp = addr;
	temp = (temp ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],temp,1);
	temp &= 0xffff;
	cpustate->r[d] += temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 101 mmm ddd   (mmm = 0xx)
 *  x x x x  SDBD, CMP@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_cmpat(cp1610_state *cpustate, int r, int d)
{
	UINT32 temp;
	UINT16 temp2;
	CLR_SZOC;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	temp = (temp ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],temp,1);
	temp &= 0xffff;
	temp2 = cpustate->r[d] + temp;
	SET_SZ(temp2);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 101 mmm ddd   (mmm = 10x)
 *  x x x x  SDBD, CMP@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_cmpat_i(cp1610_state *cpustate, int r, int d)
{
	UINT32 temp;
	UINT16 temp2;
	CLR_SZOC;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]++;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[r]++;
	temp = (temp ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],temp,1);
	temp &= 0xffff;
	temp2 = cpustate->r[d] + temp;
	SET_SZ(temp2);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 101 mmm ddd   (mmm = 101)
 *  x x x x  SDBD, CMP@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_cmpat_d(cp1610_state *cpustate, int r, int d)
{
	UINT32 temp;
	UINT16 temp2;
	CLR_SZOC;
	cpustate->r[r]--;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]--;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	temp = (temp ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],temp,1);
	temp &= 0xffff;
	temp2 = cpustate->r[d] + temp;
	SET_SZ(temp2);
	cpustate->icount -= 17;
}

/************************************************************************
 *  S Z C OV 0x001  1 101 111 ddd  x xxx xLL LLL LLL  x xxx xUU UUU UUU
 *  x x x x  SDBD, CMPI I-I, Rd
 ************************************************************************/
static void cp1610_sdbd_cmpi(cp1610_state *cpustate, int d)
{
	UINT16 addr;
	UINT32 temp;
	UINT16 temp2;
	CLR_SZOC;
	addr = cp1610_readop(cpustate->r[7]) & 0xff;
	cpustate->r[7]++;
	addr |= (cp1610_readop(cpustate->r[7]) << 8);
	cpustate->r[7]++;
	temp = addr;
	temp = (temp ^ 0xffff) + 1;
	SET_COV(cpustate->r[d],temp,1);
	temp &= 0xffff;
	temp2 = cpustate->r[d] + temp;
	SET_SZ(temp2);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 110 mmm ddd   (mmm = 0xx)
 *  x x - -  SDBD, AND@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_andat(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	CLR_SZ;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[d] &= temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 110 mmm ddd   (mmm = 10x)
 *  x x - -  SDBD, AND@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_andat_i(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	CLR_SZ;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]++;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[r]++;
	cpustate->r[d] &= temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 110 mmm ddd   (mmm = 101)
 *  x x - -  SDBD, AND@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_andat_d(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	CLR_SZ;
	cpustate->r[r]--;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]--;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[d] &= temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 17;
}

/************************************************************************
 *  S Z C OV 0x001  1 110 111 ddd  x xxx xLL LLL LLL  x xxx xUU UUU UUU
 *  x x - -  SDBD, ANDI I-I, Rd
 ************************************************************************/
static void cp1610_sdbd_andi(cp1610_state *cpustate, int d)
{
	UINT16 addr;
	CLR_SZ;
	addr = cp1610_readop(cpustate->r[7]) & 0xff;
	cpustate->r[7]++;
	addr |= (cp1610_readop(cpustate->r[7]) << 8);
	cpustate->r[7]++;
	cpustate->r[d] &= addr;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 111 mmm ddd   (mmm = 0xx)
 *  x x - -  SDBD, XOR@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_xorat(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	CLR_SZ;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[d] ^= temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 111 mmm ddd   (mmm = 10x)
 *  x x - -  SDBD, XOR@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_xorat_i(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	CLR_SZ;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]++;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[r]++;
	cpustate->r[d] ^= temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV 0x001  1 111 mmm ddd   (mmm = 101)
 *  x x - -  SDBD, XOR@ Rm, Rd
 ***************************************************/
static void cp1610_sdbd_xorat_d(cp1610_state *cpustate, int r, int d)
{
	UINT16 temp;
	CLR_SZ;
	cpustate->r[r]--;
	temp = cp1610_readmem16(cpustate->r[r]) & 0xff;
	cpustate->r[r]--;
	temp |= (cp1610_readmem16(cpustate->r[r]) << 8);
	cpustate->r[d] ^= temp;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 17;
}

/************************************************************************
 *  S Z C OV 0x001  1 111 111 ddd  x xxx xLL LLL LLL  x xxx xUU UUU UUU
 *  x x - -  SDBD, XORI I-I, Rd
 ************************************************************************/
static void cp1610_sdbd_xori(cp1610_state *cpustate, int d)
{
	UINT16 addr;
	CLR_SZ;
	addr = cp1610_readop(cpustate->r[7]) & 0xff;
	cpustate->r[7]++;
	addr |= (cp1610_readop(cpustate->r[7]) << 8);
	cpustate->r[7]++;
	cpustate->r[d] ^= addr;
	SET_SZ(cpustate->r[d]);
	cpustate->icount -= 14;
}

/***************************************************
 *  S Z C OV b baa aaa a00  x xxx xxa aaa aaa aaa
 *  - - - -  JSR R1bb, ADDR
 ***************************************************/
static void cp1610_jsr(cp1610_state *cpustate, int r, UINT16 addr)
{
	cpustate->r[r] = cpustate->r[7];
	cpustate->r[7] = addr;
}

/***************************************************
 *  S Z C OV b baa aaa a01  x xxx xxa aaa aaa aaa
 *  - - - -  JSRE R1bb, ADDR
 ***************************************************/
static void cp1610_jsre(cp1610_state *cpustate, int r, UINT16 addr)
{
	cpustate->r[r] = cpustate->r[7];
	cpustate->r[7] = addr;
	cpustate->intr_enabled = 1;
}

/***************************************************
 *  S Z C OV b baa aaa a10  x xxx xxa aaa aaa aaa
 *  - - - -  JSRD R1bb, ADDR
 ***************************************************/
static void cp1610_jsrd(cp1610_state *cpustate, int r, UINT16 addr)
{
	cpustate->r[r] = cpustate->r[7];
	cpustate->r[7] = addr;
	cpustate->intr_enabled = 0;
}

/***************************************************
 *  S Z C OV 1 1aa aaa a00  x xxx xxa aaa aaa aaa
 *  - - - -  J ADDR
 ***************************************************/
static void cp1610_j(cp1610_state *cpustate, UINT16 addr)
{
	cpustate->r[7] = addr;
}

/***************************************************
 *  S Z C OV 1 1aa aaa a01  x xxx xxa aaa aaa aaa
 *  - - - -  JE ADDR
 ***************************************************/
static void cp1610_je(cp1610_state *cpustate, UINT16 addr)
{
	cpustate->r[7] = addr;
	cpustate->intr_enabled = 1;
}

/***************************************************
 *  S Z C OV 1 1aa aaa a10  x xxx xxa aaa aaa aaa
 *  - - - -  JD ADDR
 ***************************************************/
static void cp1610_jd(cp1610_state *cpustate, UINT16 addr)
{
	cpustate->r[7] = addr;
	cpustate->intr_enabled = 0;
}

static void cp1610_do_sdbd(cp1610_state *cpustate)
{
	UINT16 sdbdtype, dest;

	/* Even though SDBD is uninterruptable, we don't need to set the mask bit,
     * because we already treat the SDBD prefixed instructions as uninterruptable
     */
	//cpustate->mask_interrupts = 1;

	sdbdtype = cp1610_readop(cpustate->r[7]);
	dest = sdbdtype & 0x07;
	cpustate->r[7]++;

	switch (sdbdtype & 0x3f8)
	{
	case 0x240: /* Not supporting SDBD MVO@ or SDBD MVOI for now */
	case 0x248:
	case 0x250:
	case 0x258:
	case 0x260:
	case 0x268:
	case 0x270:
	case 0x278: /* 1 001 xxx xxx */ cp1610_illegal(cpustate);				break;

	case 0x280: /* 1 010 000 xxx */ cp1610_sdbd_mviat(cpustate, 0,dest);		break;
	case 0x288: /* 1 010 001 xxx */ cp1610_sdbd_mviat(cpustate, 1,dest);		break;
	case 0x290: /* 1 010 010 xxx */ cp1610_sdbd_mviat(cpustate, 2,dest);		break;
	case 0x298: /* 1 010 011 xxx */ cp1610_sdbd_mviat(cpustate, 3,dest);		break;
	case 0x2a0: /* 1 010 100 xxx */ cp1610_sdbd_mviat_i(cpustate, 4,dest);	break;
	case 0x2a8: /* 1 010 101 xxx */ cp1610_sdbd_mviat_i(cpustate, 5,dest);	break;
	case 0x2b0: /* 1 010 110 xxx */ cp1610_sdbd_mviat_d(cpustate, 6,dest);	break; /* ??? */
	case 0x2b8: /* 1 010 111 xxx */ cp1610_sdbd_mvii(cpustate, dest);			break;

	case 0x2c0: /* 1 011 000 xxx */ cp1610_sdbd_addat(cpustate, 0,dest);		break;
	case 0x2c8: /* 1 011 001 xxx */ cp1610_sdbd_addat(cpustate, 1,dest);		break;
	case 0x2d0: /* 1 011 010 xxx */ cp1610_sdbd_addat(cpustate, 2,dest);		break;
	case 0x2d8: /* 1 011 011 xxx */ cp1610_sdbd_addat(cpustate, 3,dest);		break;
	case 0x2e0: /* 1 011 100 xxx */ cp1610_sdbd_addat_i(cpustate, 4,dest);	break;
	case 0x2e8: /* 1 011 101 xxx */ cp1610_sdbd_addat_i(cpustate, 5,dest);	break;
	case 0x2f0: /* 1 011 110 xxx */ cp1610_sdbd_addat_d(cpustate, 6,dest);	break; /* ??? */
	case 0x2f8: /* 1 011 111 xxx */ cp1610_sdbd_addi(cpustate, dest);			break;

	case 0x300: /* 1 100 000 xxx */ cp1610_sdbd_subat(cpustate, 0,dest);		break;
	case 0x308: /* 1 100 001 xxx */ cp1610_sdbd_subat(cpustate, 1,dest);		break;
	case 0x310: /* 1 100 010 xxx */ cp1610_sdbd_subat(cpustate, 2,dest);		break;
	case 0x318: /* 1 100 011 xxx */ cp1610_sdbd_subat(cpustate, 3,dest);		break;
	case 0x320: /* 1 100 100 xxx */ cp1610_sdbd_subat_i(cpustate, 4,dest);	break;
	case 0x328: /* 1 100 101 xxx */ cp1610_sdbd_subat_i(cpustate, 5,dest);	break;
	case 0x330: /* 1 100 110 xxx */ cp1610_sdbd_subat_d(cpustate, 6,dest);	break; /* ??? */
	case 0x338: /* 1 100 111 xxx */ cp1610_sdbd_subi(cpustate, dest);			break;

	case 0x340: /* 1 101 000 xxx */ cp1610_sdbd_cmpat(cpustate, 0,dest);		break;
	case 0x348: /* 1 101 001 xxx */ cp1610_sdbd_cmpat(cpustate, 1,dest);		break;
	case 0x350: /* 1 101 010 xxx */ cp1610_sdbd_cmpat(cpustate, 2,dest);		break;
	case 0x358: /* 1 101 011 xxx */ cp1610_sdbd_cmpat(cpustate, 3,dest);		break;
	case 0x360: /* 1 101 100 xxx */ cp1610_sdbd_cmpat_i(cpustate, 4,dest);	break;
	case 0x368: /* 1 101 101 xxx */ cp1610_sdbd_cmpat_i(cpustate, 5,dest);	break;
	case 0x370: /* 1 101 110 xxx */ cp1610_sdbd_cmpat_d(cpustate, 6,dest);	break; /* ??? */
	case 0x378: /* 1 101 111 xxx */ cp1610_sdbd_cmpi(cpustate, dest);			break;

	case 0x380: /* 1 110 000 xxx */ cp1610_sdbd_andat(cpustate, 0,dest);		break;
	case 0x388: /* 1 110 001 xxx */ cp1610_sdbd_andat(cpustate, 1,dest);		break;
	case 0x390: /* 1 110 010 xxx */ cp1610_sdbd_andat(cpustate, 2,dest);		break;
	case 0x398: /* 1 110 011 xxx */ cp1610_sdbd_andat(cpustate, 3,dest);		break;
	case 0x3a0: /* 1 110 100 xxx */ cp1610_sdbd_andat_i(cpustate, 4,dest);	break;
	case 0x3a8: /* 1 110 101 xxx */ cp1610_sdbd_andat_i(cpustate, 5,dest);	break;
	case 0x3b0: /* 1 110 110 xxx */ cp1610_sdbd_andat_d(cpustate, 6,dest);	break; /* ??? */
	case 0x3b8: /* 1 110 111 xxx */ cp1610_sdbd_andi(cpustate, dest);			break;

	case 0x3c0: /* 1 110 000 xxx */ cp1610_sdbd_xorat(cpustate, 0,dest);		break;
	case 0x3c8: /* 1 110 001 xxx */ cp1610_sdbd_xorat(cpustate, 1,dest);		break;
	case 0x3d0: /* 1 110 010 xxx */ cp1610_sdbd_xorat(cpustate, 2,dest);		break;
	case 0x3d8: /* 1 110 011 xxx */ cp1610_sdbd_xorat(cpustate, 3,dest);		break;
	case 0x3e0: /* 1 110 100 xxx */ cp1610_sdbd_xorat_i(cpustate, 4,dest);	break;
	case 0x3e8: /* 1 110 101 xxx */ cp1610_sdbd_xorat_i(cpustate, 5,dest);	break;
	case 0x3f0: /* 1 110 110 xxx */ cp1610_sdbd_xorat_d(cpustate, 6,dest);	break; /* ??? */
	case 0x3f8: /* 1 110 111 xxx */ cp1610_sdbd_xori(cpustate, dest);			break;
	default:						cp1610_illegal(cpustate); break;
	}
}

static void cp1610_do_jumps(cp1610_state *cpustate)
{
	UINT16 jumptype, arg1, arg2, addr;

	arg1 = cp1610_readop(cpustate->r[7]);
    cpustate->r[7]++;

	arg2 = cp1610_readop(cpustate->r[7]);
    cpustate->r[7]++;

    /*logerror("jumps: pc = 0x%04x, arg1 = 0x%04x, arg2 = 0x%04x\n",cpustate->r[7]-1,arg1,arg2);*/
	jumptype = arg1 & 0x303;
	addr = ((arg1 & 0x0fc) << 8) | (arg2 & 0x3ff);

    switch( jumptype )
    {
	case 0x000: /* 0 0xx xxx x00 */	cp1610_jsr(cpustate, 4,addr);		break;
	case 0x001: /* 0 0xx xxx x01 */	cp1610_jsre(cpustate, 4,addr);	break;
	case 0x002: /* 0 0xx xxx x10 */	cp1610_jsrd(cpustate, 4,addr);	break;
	case 0x003: /* 0 0xx xxx x11 */	cp1610_illegal(cpustate);		break;

	case 0x100: /* 0 1xx xxx x00 */	cp1610_jsr(cpustate, 5,addr);		break;
	case 0x101: /* 0 1xx xxx x01 */	cp1610_jsre(cpustate, 5,addr);	break;
	case 0x102: /* 0 1xx xxx x10 */	cp1610_jsrd(cpustate, 5,addr);	break;
	case 0x103: /* 0 1xx xxx x11 */	cp1610_illegal(cpustate);		break;

	case 0x200: /* 1 0xx xxx x00 */	cp1610_jsr(cpustate, 6,addr);		break;
	case 0x201: /* 1 0xx xxx x01 */	cp1610_jsre(cpustate, 6,addr);	break;
	case 0x202: /* 1 0xx xxx x10 */	cp1610_jsrd(cpustate, 6,addr);	break;
	case 0x203: /* 1 0xx xxx x11 */	cp1610_illegal(cpustate);		break;

	case 0x300: /* 1 1xx xxx x00 */	cp1610_j(cpustate, addr);			break;
	case 0x301: /* 1 1xx xxx x01 */	cp1610_je(cpustate, addr);		break;
	case 0x302: /* 1 1xx xxx x10 */	cp1610_jd(cpustate, addr);		break;
	case 0x303: /* 1 1xx xxx x11 */	cp1610_illegal(cpustate);		break;
	}

	cpustate->icount -= 12;
}

/* Execute cycles - returns number of cycles actually run */
static CPU_EXECUTE( cp1610 )
{
	cp1610_state *cpustate = get_safe_token(device);
	UINT16 opcode;

    do
    {
        debugger_instruction_hook(device, cpustate->r[7]);

		cpustate->mask_interrupts = 0;

        opcode = cp1610_readop(cpustate->r[7]);
        cpustate->r[7]++;
#if 0
		logerror("PC:0x%04x, opcode = 0x%03x, ",cpustate->r[7]-1,opcode);
		logerror("R0:0x%04x, ",cpustate->r[0]);
		logerror("R1:0x%04x, ",cpustate->r[1]);
		logerror("R2:0x%04x, ",cpustate->r[2]);
		logerror("R3:0x%04x, ",cpustate->r[3]);
		logerror("R4:0x%04x, ",cpustate->r[4]);
		logerror("R5:0x%04x, ",cpustate->r[5]);
		logerror("R6:0x%04x\n",cpustate->r[6]);
#endif

		switch( opcode )
        {
		/* opcode  bitmask */
		case 0x000: /* 0 000 000 000 */	cp1610_hlt(cpustate);		break; /* TBD */
		case 0x001: /* 0 000 000 001 */	cp1610_do_sdbd(cpustate);	break;
		case 0x002: /* 0 000 000 010 */	cp1610_eis(cpustate);		break; /* TBD */
		case 0x003: /* 0 000 000 011 */	cp1610_dis(cpustate);		break; /* TBD */
		case 0x004: /* 0 000 000 100 */	cp1610_do_jumps(cpustate);	break;
		case 0x005: /* 0 000 000 101 */	cp1610_tci(cpustate);		break; /* TBD */
		case 0x006: /* 0 000 000 110 */	cp1610_clrc(cpustate);		break;
		case 0x007: /* 0 000 000 111 */	cp1610_setc(cpustate);		break;

		case 0x008: /* 0 000 001 000 */	cp1610_incr(cpustate, 0);		break;
		case 0x009: /* 0 000 001 001 */	cp1610_incr(cpustate, 1);		break;
		case 0x00a: /* 0 000 001 010 */	cp1610_incr(cpustate, 2);		break;
		case 0x00b: /* 0 000 001 011 */	cp1610_incr(cpustate, 3);		break;
		case 0x00c: /* 0 000 001 100 */	cp1610_incr(cpustate, 4);		break;
		case 0x00d: /* 0 000 001 101 */	cp1610_incr(cpustate, 5);		break;
		case 0x00e: /* 0 000 001 110 */	cp1610_incr(cpustate, 6);		break;
		case 0x00f: /* 0 000 001 111 */	cp1610_incr(cpustate, 7);		break;

		case 0x010: /* 0 000 010 000 */	cp1610_decr(cpustate, 0);		break;
		case 0x011: /* 0 000 010 001 */	cp1610_decr(cpustate, 1);		break;
		case 0x012: /* 0 000 010 010 */	cp1610_decr(cpustate, 2);		break;
		case 0x013: /* 0 000 010 011 */	cp1610_decr(cpustate, 3);		break;
		case 0x014: /* 0 000 010 100 */	cp1610_decr(cpustate, 4);		break;
		case 0x015: /* 0 000 010 101 */	cp1610_decr(cpustate, 5);		break;
		case 0x016: /* 0 000 010 110 */	cp1610_decr(cpustate, 6);		break;
		case 0x017: /* 0 000 010 111 */	cp1610_decr(cpustate, 7);		break;

		case 0x018: /* 0 000 011 000 */	cp1610_comr(cpustate, 0);		break;
		case 0x019: /* 0 000 011 001 */	cp1610_comr(cpustate, 1);		break;
		case 0x01a: /* 0 000 011 010 */	cp1610_comr(cpustate, 2);		break;
		case 0x01b: /* 0 000 011 011 */	cp1610_comr(cpustate, 3);		break;
		case 0x01c: /* 0 000 011 100 */	cp1610_comr(cpustate, 4);		break;
		case 0x01d: /* 0 000 011 101 */	cp1610_comr(cpustate, 5);		break;
		case 0x01e: /* 0 000 011 110 */	cp1610_comr(cpustate, 6);		break;
		case 0x01f: /* 0 000 011 111 */	cp1610_comr(cpustate, 7);		break;

		case 0x020: /* 0 000 100 000 */	cp1610_negr(cpustate, 0);		break;
		case 0x021: /* 0 000 100 001 */	cp1610_negr(cpustate, 1);		break;
		case 0x022: /* 0 000 100 010 */	cp1610_negr(cpustate, 2);		break;
		case 0x023: /* 0 000 100 011 */	cp1610_negr(cpustate, 3);		break;
		case 0x024: /* 0 000 100 100 */	cp1610_negr(cpustate, 4);		break;
		case 0x025: /* 0 000 100 101 */	cp1610_negr(cpustate, 5);		break;
		case 0x026: /* 0 000 100 110 */	cp1610_negr(cpustate, 6);		break;
		case 0x027: /* 0 000 100 111 */	cp1610_negr(cpustate, 7);		break;

		case 0x028: /* 0 000 101 000 */	cp1610_adcr(cpustate, 0);		break;
		case 0x029: /* 0 000 101 001 */	cp1610_adcr(cpustate, 1);		break;
		case 0x02a: /* 0 000 101 010 */	cp1610_adcr(cpustate, 2);		break;
		case 0x02b: /* 0 000 101 011 */	cp1610_adcr(cpustate, 3);		break;
		case 0x02c: /* 0 000 101 100 */	cp1610_adcr(cpustate, 4);		break;
		case 0x02d: /* 0 000 101 101 */	cp1610_adcr(cpustate, 5);		break;
		case 0x02e: /* 0 000 101 110 */	cp1610_adcr(cpustate, 6);		break;
		case 0x02f: /* 0 000 101 111 */	cp1610_adcr(cpustate, 7);		break;

		case 0x030: /* 0 000 110 000 */ cp1610_gswd(cpustate, 0);		break;
		case 0x031: /* 0 000 110 001 */ cp1610_gswd(cpustate, 1);		break;
		case 0x032: /* 0 000 110 010 */ cp1610_gswd(cpustate, 2);		break;
		case 0x033: /* 0 000 110 011 */ cp1610_gswd(cpustate, 3);		break;
		case 0x034: /* 0 000 110 100 */ cp1610_nop(cpustate);		break;
		case 0x035: /* 0 000 110 101 */ cp1610_nop(cpustate);		break;
		case 0x036: /* 0 000 110 110 */ cp1610_sin(cpustate);		break; /* TBD */
		case 0x037: /* 0 000 110 111 */ cp1610_sin(cpustate);		break; /* TBD */

		case 0x038: /* 0 000 111 000 */	cp1610_rswd(cpustate, 0);		break;
		case 0x039: /* 0 000 111 001 */	cp1610_rswd(cpustate, 1);		break;
		case 0x03a: /* 0 000 111 010 */	cp1610_rswd(cpustate, 2);		break;
		case 0x03b: /* 0 000 111 011 */	cp1610_rswd(cpustate, 3);		break;
		case 0x03c: /* 0 000 111 100 */	cp1610_rswd(cpustate, 4);		break;
		case 0x03d: /* 0 000 111 101 */	cp1610_rswd(cpustate, 5);		break;
		case 0x03e: /* 0 000 111 110 */	cp1610_rswd(cpustate, 6);		break;
		case 0x03f: /* 0 000 111 111 */	cp1610_rswd(cpustate, 7);		break;

		case 0x040: /* 0 001 000 000 */ cp1610_swap(cpustate, 0);		break;
		case 0x041: /* 0 001 000 001 */ cp1610_swap(cpustate, 1);		break;
		case 0x042: /* 0 001 000 010 */ cp1610_swap(cpustate, 2);		break;
		case 0x043: /* 0 001 000 011 */ cp1610_swap(cpustate, 3);		break;
		case 0x044: /* 0 001 000 100 */ cp1610_dswap(cpustate, 0);	break;
		case 0x045: /* 0 001 000 101 */ cp1610_dswap(cpustate, 1);	break;
		case 0x046: /* 0 001 000 110 */ cp1610_dswap(cpustate, 2);	break;
		case 0x047: /* 0 001 000 111 */ cp1610_dswap(cpustate, 3);	break;

		case 0x048: /* 0 001 001 000 */ cp1610_sll_1(cpustate, 0);	break;
		case 0x049: /* 0 001 001 001 */ cp1610_sll_1(cpustate, 1);	break;
		case 0x04a: /* 0 001 001 010 */ cp1610_sll_1(cpustate, 2);	break;
		case 0x04b: /* 0 001 001 011 */ cp1610_sll_1(cpustate, 3);	break;
		case 0x04c: /* 0 001 001 100 */ cp1610_sll_2(cpustate, 0);	break;
		case 0x04d: /* 0 001 001 101 */ cp1610_sll_2(cpustate, 1);	break;
		case 0x04e: /* 0 001 001 110 */ cp1610_sll_2(cpustate, 2);	break;
		case 0x04f: /* 0 001 001 111 */ cp1610_sll_2(cpustate, 3);	break;

		case 0x050: /* 0 001 010 000 */ cp1610_rlc_1(cpustate, 0);	break;
		case 0x051: /* 0 001 010 001 */ cp1610_rlc_1(cpustate, 1);	break;
		case 0x052: /* 0 001 010 010 */ cp1610_rlc_1(cpustate, 2);	break;
		case 0x053: /* 0 001 010 011 */ cp1610_rlc_1(cpustate, 3);	break;
		case 0x054: /* 0 001 010 100 */ cp1610_rlc_2(cpustate, 0);	break;
		case 0x055: /* 0 001 010 101 */ cp1610_rlc_2(cpustate, 1);	break;
		case 0x056: /* 0 001 010 110 */ cp1610_rlc_2(cpustate, 2);	break;
		case 0x057: /* 0 001 010 111 */ cp1610_rlc_2(cpustate, 3);	break;

		case 0x058: /* 0 001 011 000 */ cp1610_sllc_1(cpustate, 0);	break;
		case 0x059: /* 0 001 011 001 */ cp1610_sllc_1(cpustate, 1);	break;
		case 0x05a: /* 0 001 011 010 */ cp1610_sllc_1(cpustate, 2);	break;
		case 0x05b: /* 0 001 011 011 */ cp1610_sllc_1(cpustate, 3);	break;
		case 0x05c: /* 0 001 011 100 */ cp1610_sllc_2(cpustate, 0);	break;
		case 0x05d: /* 0 001 011 101 */ cp1610_sllc_2(cpustate, 1);	break;
		case 0x05e: /* 0 001 011 110 */ cp1610_sllc_2(cpustate, 2);	break;
		case 0x05f: /* 0 001 011 111 */ cp1610_sllc_2(cpustate, 3);	break;

		case 0x060: /* 0 001 100 000 */ cp1610_slr_1(cpustate, 0);	break;
		case 0x061: /* 0 001 100 001 */ cp1610_slr_1(cpustate, 1);	break;
		case 0x062: /* 0 001 100 010 */ cp1610_slr_1(cpustate, 2);	break;
		case 0x063: /* 0 001 100 011 */ cp1610_slr_1(cpustate, 3);	break;
		case 0x064: /* 0 001 100 100 */ cp1610_slr_2(cpustate, 0);	break;
		case 0x065: /* 0 001 100 101 */ cp1610_slr_2(cpustate, 1);	break;
		case 0x066: /* 0 001 100 110 */ cp1610_slr_2(cpustate, 2);	break;
		case 0x067: /* 0 001 100 111 */ cp1610_slr_2(cpustate, 3);	break;

		case 0x068: /* 0 001 101 000 */ cp1610_sar_1(cpustate, 0);	break;
		case 0x069: /* 0 001 101 001 */ cp1610_sar_1(cpustate, 1);	break;
		case 0x06a: /* 0 001 101 010 */ cp1610_sar_1(cpustate, 2);	break;
		case 0x06b: /* 0 001 101 011 */ cp1610_sar_1(cpustate, 3);	break;
		case 0x06c: /* 0 001 101 100 */ cp1610_sar_2(cpustate, 0);	break;
		case 0x06d: /* 0 001 101 101 */ cp1610_sar_2(cpustate, 1);	break;
		case 0x06e: /* 0 001 101 110 */ cp1610_sar_2(cpustate, 2);	break;
		case 0x06f: /* 0 001 101 111 */ cp1610_sar_2(cpustate, 3);	break;

		case 0x070: /* 0 001 110 000 */ cp1610_rrc_1(cpustate, 0);	break;
		case 0x071: /* 0 001 110 001 */ cp1610_rrc_1(cpustate, 1);	break;
		case 0x072: /* 0 001 110 010 */ cp1610_rrc_1(cpustate, 2);	break;
		case 0x073: /* 0 001 110 011 */ cp1610_rrc_1(cpustate, 3);	break;
		case 0x074: /* 0 001 110 100 */ cp1610_rrc_2(cpustate, 0);	break;
		case 0x075: /* 0 001 110 101 */ cp1610_rrc_2(cpustate, 1);	break;
		case 0x076: /* 0 001 110 110 */ cp1610_rrc_2(cpustate, 2);	break;
		case 0x077: /* 0 001 110 111 */ cp1610_rrc_2(cpustate, 3);	break;

		case 0x078: /* 0 001 111 000 */ cp1610_sarc_1(cpustate, 0);	break;
		case 0x079: /* 0 001 111 001 */ cp1610_sarc_1(cpustate, 1);	break;
		case 0x07a: /* 0 001 111 010 */ cp1610_sarc_1(cpustate, 2);	break;
		case 0x07b: /* 0 001 111 011 */ cp1610_sarc_1(cpustate, 3);	break;
		case 0x07c: /* 0 001 111 100 */ cp1610_sarc_2(cpustate, 0);	break;
		case 0x07d: /* 0 001 111 101 */ cp1610_sarc_2(cpustate, 1);	break;
		case 0x07e: /* 0 001 111 110 */ cp1610_sarc_2(cpustate, 2);	break;
		case 0x07f: /* 0 001 111 111 */ cp1610_sarc_2(cpustate, 3);	break;

		case 0x080: /* 0 010 000 000 */	cp1610_tstr(cpustate, 0);		break;
		case 0x081: /* 0 010 000 001 */	cp1610_movr(cpustate, 0,1);	break;
		case 0x082: /* 0 010 000 010 */	cp1610_movr(cpustate, 0,2);	break;
		case 0x083: /* 0 010 000 011 */	cp1610_movr(cpustate, 0,3);	break;
		case 0x084: /* 0 010 000 100 */	cp1610_movr(cpustate, 0,4);	break;
		case 0x085: /* 0 010 000 101 */	cp1610_movr(cpustate, 0,5);	break;
		case 0x086: /* 0 010 000 110 */	cp1610_movr(cpustate, 0,6);	break;
		case 0x087: /* 0 010 000 111 */	cp1610_movr(cpustate, 0,7);	break; /* jr */

		case 0x088: /* 0 010 001 000 */	cp1610_movr(cpustate, 1,0);	break;
		case 0x089: /* 0 010 001 001 */	cp1610_tstr(cpustate, 1);		break;
		case 0x08a: /* 0 010 001 010 */	cp1610_movr(cpustate, 1,2);	break;
		case 0x08b: /* 0 010 001 011 */	cp1610_movr(cpustate, 1,3);	break;
		case 0x08c: /* 0 010 001 100 */	cp1610_movr(cpustate, 1,4);	break;
		case 0x08d: /* 0 010 001 101 */	cp1610_movr(cpustate, 1,5);	break;
		case 0x08e: /* 0 010 001 110 */	cp1610_movr(cpustate, 1,6);	break;
		case 0x08f: /* 0 010 001 111 */	cp1610_movr(cpustate, 1,7);	break; /* jr */

		case 0x090: /* 0 010 010 000 */	cp1610_movr(cpustate, 2,0);	break;
		case 0x091: /* 0 010 010 001 */	cp1610_movr(cpustate, 2,1);	break;
		case 0x092: /* 0 010 010 010 */	cp1610_tstr(cpustate, 2);		break;
		case 0x093: /* 0 010 010 011 */	cp1610_movr(cpustate, 2,3);	break;
		case 0x094: /* 0 010 010 100 */	cp1610_movr(cpustate, 2,4);	break;
		case 0x095: /* 0 010 010 101 */	cp1610_movr(cpustate, 2,5);	break;
		case 0x096: /* 0 010 010 110 */	cp1610_movr(cpustate, 2,6);	break;
		case 0x097: /* 0 010 010 111 */	cp1610_movr(cpustate, 2,7);	break; /* jr */

		case 0x098: /* 0 010 011 000 */	cp1610_movr(cpustate, 3,0);	break;
		case 0x099: /* 0 010 011 001 */	cp1610_movr(cpustate, 3,1);	break;
		case 0x09a: /* 0 010 011 010 */	cp1610_movr(cpustate, 3,2);	break;
		case 0x09b: /* 0 010 011 011 */	cp1610_tstr(cpustate, 3);		break;
		case 0x09c: /* 0 010 011 100 */	cp1610_movr(cpustate, 3,4);	break;
		case 0x09d: /* 0 010 011 101 */	cp1610_movr(cpustate, 3,5);	break;
		case 0x09e: /* 0 010 011 110 */	cp1610_movr(cpustate, 3,6);	break;
		case 0x09f: /* 0 010 011 111 */	cp1610_movr(cpustate, 3,7);	break; /* jr */

		case 0x0a0: /* 0 010 100 000 */	cp1610_movr(cpustate, 4,0);	break;
		case 0x0a1: /* 0 010 100 001 */	cp1610_movr(cpustate, 4,1);	break;
		case 0x0a2: /* 0 010 100 010 */	cp1610_movr(cpustate, 4,2);	break;
		case 0x0a3: /* 0 010 100 011 */	cp1610_movr(cpustate, 4,3);	break;
		case 0x0a4: /* 0 010 100 100 */	cp1610_tstr(cpustate, 4);		break;
		case 0x0a5: /* 0 010 100 101 */	cp1610_movr(cpustate, 4,5);	break;
		case 0x0a6: /* 0 010 100 110 */	cp1610_movr(cpustate, 4,6);	break;
		case 0x0a7: /* 0 010 100 111 */	cp1610_movr(cpustate, 4,7);	break; /* jr */

		case 0x0a8: /* 0 010 101 000 */	cp1610_movr(cpustate, 5,0);	break;
		case 0x0a9: /* 0 010 101 001 */	cp1610_movr(cpustate, 5,1);	break;
		case 0x0aa: /* 0 010 101 010 */	cp1610_movr(cpustate, 5,2);	break;
		case 0x0ab: /* 0 010 101 011 */	cp1610_movr(cpustate, 5,3);	break;
		case 0x0ac: /* 0 010 101 100 */	cp1610_movr(cpustate, 5,4);	break;
		case 0x0ad: /* 0 010 101 101 */	cp1610_tstr(cpustate, 5);		break;
		case 0x0ae: /* 0 010 101 110 */	cp1610_movr(cpustate, 5,6);	break;
		case 0x0af: /* 0 010 101 111 */	cp1610_movr(cpustate, 5,7);	break; /* jr */

		case 0x0b0: /* 0 010 110 000 */	cp1610_movr(cpustate, 6,0);	break;
		case 0x0b1: /* 0 010 110 001 */	cp1610_movr(cpustate, 6,1);	break;
		case 0x0b2: /* 0 010 110 010 */	cp1610_movr(cpustate, 6,2);	break;
		case 0x0b3: /* 0 010 110 011 */	cp1610_movr(cpustate, 6,3);	break;
		case 0x0b4: /* 0 010 110 100 */	cp1610_movr(cpustate, 6,4);	break;
		case 0x0b5: /* 0 010 110 101 */	cp1610_movr(cpustate, 6,5);	break;
		case 0x0b6: /* 0 010 110 110 */	cp1610_tstr(cpustate, 6);		break;
		case 0x0b7: /* 0 010 110 111 */	cp1610_movr(cpustate, 6,7);	break; /* jr */

		case 0x0b8: /* 0 010 111 000 */	cp1610_movr(cpustate, 7,0);	break;
		case 0x0b9: /* 0 010 111 001 */	cp1610_movr(cpustate, 7,1);	break;
		case 0x0ba: /* 0 010 111 010 */	cp1610_movr(cpustate, 7,2);	break;
		case 0x0bb: /* 0 010 111 011 */	cp1610_movr(cpustate, 7,3);	break;
		case 0x0bc: /* 0 010 111 100 */	cp1610_movr(cpustate, 7,4);	break;
		case 0x0bd: /* 0 010 111 101 */	cp1610_movr(cpustate, 7,5);	break;
		case 0x0be: /* 0 010 111 110 */	cp1610_movr(cpustate, 7,6);	break;
		case 0x0bf: /* 0 010 111 111 */	cp1610_tstr(cpustate, 7);		break;

		case 0x0c0: /* 0 011 000 000 */	cp1610_addr(cpustate, 0,0);	break;
		case 0x0c1: /* 0 011 000 001 */	cp1610_addr(cpustate, 0,1);	break;
		case 0x0c2: /* 0 011 000 010 */	cp1610_addr(cpustate, 0,2);	break;
		case 0x0c3: /* 0 011 000 011 */	cp1610_addr(cpustate, 0,3);	break;
		case 0x0c4: /* 0 011 000 100 */	cp1610_addr(cpustate, 0,4);	break;
		case 0x0c5: /* 0 011 000 101 */	cp1610_addr(cpustate, 0,5);	break;
		case 0x0c6: /* 0 011 000 110 */	cp1610_addr(cpustate, 0,6);	break;
		case 0x0c7: /* 0 011 000 111 */	cp1610_addr(cpustate, 0,7);	break;

		case 0x0c8: /* 0 011 001 000 */	cp1610_addr(cpustate, 1,0);	break;
		case 0x0c9: /* 0 011 001 001 */	cp1610_addr(cpustate, 1,1);	break;
		case 0x0ca: /* 0 011 001 010 */	cp1610_addr(cpustate, 1,2);	break;
		case 0x0cb: /* 0 011 001 011 */	cp1610_addr(cpustate, 1,3);	break;
		case 0x0cc: /* 0 011 001 100 */	cp1610_addr(cpustate, 1,4);	break;
		case 0x0cd: /* 0 011 001 101 */	cp1610_addr(cpustate, 1,5);	break;
		case 0x0ce: /* 0 011 001 110 */	cp1610_addr(cpustate, 1,6);	break;
		case 0x0cf: /* 0 011 001 111 */	cp1610_addr(cpustate, 1,7);	break;

		case 0x0d0: /* 0 011 010 000 */	cp1610_addr(cpustate, 2,0);	break;
		case 0x0d1: /* 0 011 010 001 */	cp1610_addr(cpustate, 2,1);	break;
		case 0x0d2: /* 0 011 010 010 */	cp1610_addr(cpustate, 2,2);	break;
		case 0x0d3: /* 0 011 010 011 */	cp1610_addr(cpustate, 2,3);	break;
		case 0x0d4: /* 0 011 010 100 */	cp1610_addr(cpustate, 2,4);	break;
		case 0x0d5: /* 0 011 010 101 */	cp1610_addr(cpustate, 2,5);	break;
		case 0x0d6: /* 0 011 010 110 */	cp1610_addr(cpustate, 2,6);	break;
		case 0x0d7: /* 0 011 010 111 */	cp1610_addr(cpustate, 2,7);	break;

		case 0x0d8: /* 0 011 011 000 */	cp1610_addr(cpustate, 3,0);	break;
		case 0x0d9: /* 0 011 011 001 */	cp1610_addr(cpustate, 3,1);	break;
		case 0x0da: /* 0 011 011 010 */	cp1610_addr(cpustate, 3,2);	break;
		case 0x0db: /* 0 011 011 011 */	cp1610_addr(cpustate, 3,3);	break;
		case 0x0dc: /* 0 011 011 100 */	cp1610_addr(cpustate, 3,4);	break;
		case 0x0dd: /* 0 011 011 101 */	cp1610_addr(cpustate, 3,5);	break;
		case 0x0de: /* 0 011 011 110 */	cp1610_addr(cpustate, 3,6);	break;
		case 0x0df: /* 0 011 011 111 */	cp1610_addr(cpustate, 3,7);	break;

		case 0x0e0: /* 0 011 100 000 */	cp1610_addr(cpustate, 4,0);	break;
		case 0x0e1: /* 0 011 100 001 */	cp1610_addr(cpustate, 4,1);	break;
		case 0x0e2: /* 0 011 100 010 */	cp1610_addr(cpustate, 4,2);	break;
		case 0x0e3: /* 0 011 100 011 */	cp1610_addr(cpustate, 4,3);	break;
		case 0x0e4: /* 0 011 100 100 */	cp1610_addr(cpustate, 4,4);	break;
		case 0x0e5: /* 0 011 100 101 */	cp1610_addr(cpustate, 4,5);	break;
		case 0x0e6: /* 0 011 100 110 */	cp1610_addr(cpustate, 4,6);	break;
		case 0x0e7: /* 0 011 100 111 */	cp1610_addr(cpustate, 4,7);	break;

		case 0x0e8: /* 0 011 101 000 */	cp1610_addr(cpustate, 5,0);	break;
		case 0x0e9: /* 0 011 101 001 */	cp1610_addr(cpustate, 5,1);	break;
		case 0x0ea: /* 0 011 101 010 */	cp1610_addr(cpustate, 5,2);	break;
		case 0x0eb: /* 0 011 101 011 */	cp1610_addr(cpustate, 5,3);	break;
		case 0x0ec: /* 0 011 101 100 */	cp1610_addr(cpustate, 5,4);	break;
		case 0x0ed: /* 0 011 101 101 */	cp1610_addr(cpustate, 5,5);	break;
		case 0x0ee: /* 0 011 101 110 */	cp1610_addr(cpustate, 5,6);	break;
		case 0x0ef: /* 0 011 101 111 */	cp1610_addr(cpustate, 5,7);	break;

		case 0x0f0: /* 0 011 110 000 */	cp1610_addr(cpustate, 6,0);	break;
		case 0x0f1: /* 0 011 110 001 */	cp1610_addr(cpustate, 6,1);	break;
		case 0x0f2: /* 0 011 110 010 */	cp1610_addr(cpustate, 6,2);	break;
		case 0x0f3: /* 0 011 110 011 */	cp1610_addr(cpustate, 6,3);	break;
		case 0x0f4: /* 0 011 110 100 */	cp1610_addr(cpustate, 6,4);	break;
		case 0x0f5: /* 0 011 110 101 */	cp1610_addr(cpustate, 6,5);	break;
		case 0x0f6: /* 0 011 110 110 */	cp1610_addr(cpustate, 6,6);	break;
		case 0x0f7: /* 0 011 110 111 */	cp1610_addr(cpustate, 6,7);	break;

		case 0x0f8: /* 0 011 111 000 */	cp1610_addr(cpustate, 7,0);	break;
		case 0x0f9: /* 0 011 111 001 */	cp1610_addr(cpustate, 7,1);	break;
		case 0x0fa: /* 0 011 111 010 */	cp1610_addr(cpustate, 7,2);	break;
		case 0x0fb: /* 0 011 111 011 */	cp1610_addr(cpustate, 7,3);	break;
		case 0x0fc: /* 0 011 111 100 */	cp1610_addr(cpustate, 7,4);	break;
		case 0x0fd: /* 0 011 111 101 */	cp1610_addr(cpustate, 7,5);	break;
		case 0x0fe: /* 0 011 111 110 */	cp1610_addr(cpustate, 7,6);	break;
		case 0x0ff: /* 0 011 111 111 */	cp1610_addr(cpustate, 7,7);	break;

		case 0x100: /* 0 100 000 000 */	cp1610_subr(cpustate, 0,0);	break;
		case 0x101: /* 0 100 000 001 */	cp1610_subr(cpustate, 0,1);	break;
		case 0x102: /* 0 100 000 010 */	cp1610_subr(cpustate, 0,2);	break;
		case 0x103: /* 0 100 000 011 */	cp1610_subr(cpustate, 0,3);	break;
		case 0x104: /* 0 100 000 100 */	cp1610_subr(cpustate, 0,4);	break;
		case 0x105: /* 0 100 000 101 */	cp1610_subr(cpustate, 0,5);	break;
		case 0x106: /* 0 100 000 110 */	cp1610_subr(cpustate, 0,6);	break;
		case 0x107: /* 0 100 000 111 */	cp1610_subr(cpustate, 0,7);	break;

		case 0x108: /* 0 100 001 000 */	cp1610_subr(cpustate, 1,0);	break;
		case 0x109: /* 0 100 001 001 */	cp1610_subr(cpustate, 1,1);	break;
		case 0x10a: /* 0 100 001 010 */	cp1610_subr(cpustate, 1,2);	break;
		case 0x10b: /* 0 100 001 011 */	cp1610_subr(cpustate, 1,3);	break;
		case 0x10c: /* 0 100 001 100 */	cp1610_subr(cpustate, 1,4);	break;
		case 0x10d: /* 0 100 001 101 */	cp1610_subr(cpustate, 1,5);	break;
		case 0x10e: /* 0 100 001 110 */	cp1610_subr(cpustate, 1,6);	break;
		case 0x10f: /* 0 100 001 111 */	cp1610_subr(cpustate, 1,7);	break;

		case 0x110: /* 0 100 010 000 */	cp1610_subr(cpustate, 2,0);	break;
		case 0x111: /* 0 100 010 001 */	cp1610_subr(cpustate, 2,1);	break;
		case 0x112: /* 0 100 010 010 */	cp1610_subr(cpustate, 2,2);	break;
		case 0x113: /* 0 100 010 011 */	cp1610_subr(cpustate, 2,3);	break;
		case 0x114: /* 0 100 010 100 */	cp1610_subr(cpustate, 2,4);	break;
		case 0x115: /* 0 100 010 101 */	cp1610_subr(cpustate, 2,5);	break;
		case 0x116: /* 0 100 010 110 */	cp1610_subr(cpustate, 2,6);	break;
		case 0x117: /* 0 100 010 111 */	cp1610_subr(cpustate, 2,7);	break;

		case 0x118: /* 0 100 011 000 */	cp1610_subr(cpustate, 3,0);	break;
		case 0x119: /* 0 100 011 001 */	cp1610_subr(cpustate, 3,1);	break;
		case 0x11a: /* 0 100 011 010 */	cp1610_subr(cpustate, 3,2);	break;
		case 0x11b: /* 0 100 011 011 */	cp1610_subr(cpustate, 3,3);	break;
		case 0x11c: /* 0 100 011 100 */	cp1610_subr(cpustate, 3,4);	break;
		case 0x11d: /* 0 100 011 101 */	cp1610_subr(cpustate, 3,5);	break;
		case 0x11e: /* 0 100 011 110 */	cp1610_subr(cpustate, 3,6);	break;
		case 0x11f: /* 0 100 011 111 */	cp1610_subr(cpustate, 3,7);	break;

		case 0x120: /* 0 100 100 000 */	cp1610_subr(cpustate, 4,0);	break;
		case 0x121: /* 0 100 100 001 */	cp1610_subr(cpustate, 4,1);	break;
		case 0x122: /* 0 100 100 010 */	cp1610_subr(cpustate, 4,2);	break;
		case 0x123: /* 0 100 100 011 */	cp1610_subr(cpustate, 4,3);	break;
		case 0x124: /* 0 100 100 100 */	cp1610_subr(cpustate, 4,4);	break;
		case 0x125: /* 0 100 100 101 */	cp1610_subr(cpustate, 4,5);	break;
		case 0x126: /* 0 100 100 110 */	cp1610_subr(cpustate, 4,6);	break;
		case 0x127: /* 0 100 100 111 */	cp1610_subr(cpustate, 4,7);	break;

		case 0x128: /* 0 100 101 000 */	cp1610_subr(cpustate, 5,0);	break;
		case 0x129: /* 0 100 101 001 */	cp1610_subr(cpustate, 5,1);	break;
		case 0x12a: /* 0 100 101 010 */	cp1610_subr(cpustate, 5,2);	break;
		case 0x12b: /* 0 100 101 011 */	cp1610_subr(cpustate, 5,3);	break;
		case 0x12c: /* 0 100 101 100 */	cp1610_subr(cpustate, 5,4);	break;
		case 0x12d: /* 0 100 101 101 */	cp1610_subr(cpustate, 5,5);	break;
		case 0x12e: /* 0 100 101 110 */	cp1610_subr(cpustate, 5,6);	break;
		case 0x12f: /* 0 100 101 111 */	cp1610_subr(cpustate, 5,7);	break;

		case 0x130: /* 0 100 110 000 */	cp1610_subr(cpustate, 6,0);	break;
		case 0x131: /* 0 100 110 001 */	cp1610_subr(cpustate, 6,1);	break;
		case 0x132: /* 0 100 110 010 */	cp1610_subr(cpustate, 6,2);	break;
		case 0x133: /* 0 100 110 011 */	cp1610_subr(cpustate, 6,3);	break;
		case 0x134: /* 0 100 110 100 */	cp1610_subr(cpustate, 6,4);	break;
		case 0x135: /* 0 100 110 101 */	cp1610_subr(cpustate, 6,5);	break;
		case 0x136: /* 0 100 110 110 */	cp1610_subr(cpustate, 6,6);	break;
		case 0x137: /* 0 100 110 111 */	cp1610_subr(cpustate, 6,7);	break;

		case 0x138: /* 0 100 111 000 */	cp1610_subr(cpustate, 7,0);	break;
		case 0x139: /* 0 100 111 001 */	cp1610_subr(cpustate, 7,1);	break;
		case 0x13a: /* 0 100 111 010 */	cp1610_subr(cpustate, 7,2);	break;
		case 0x13b: /* 0 100 111 011 */	cp1610_subr(cpustate, 7,3);	break;
		case 0x13c: /* 0 100 111 100 */	cp1610_subr(cpustate, 7,4);	break;
		case 0x13d: /* 0 100 111 101 */	cp1610_subr(cpustate, 7,5);	break;
		case 0x13e: /* 0 100 111 110 */	cp1610_subr(cpustate, 7,6);	break;
		case 0x13f: /* 0 100 111 111 */	cp1610_subr(cpustate, 7,7);	break;

		case 0x140: /* 0 101 000 000 */	cp1610_cmpr(cpustate, 0,0);	break;
		case 0x141: /* 0 101 000 001 */	cp1610_cmpr(cpustate, 0,1);	break;
		case 0x142: /* 0 101 000 010 */	cp1610_cmpr(cpustate, 0,2);	break;
		case 0x143: /* 0 101 000 011 */	cp1610_cmpr(cpustate, 0,3);	break;
		case 0x144: /* 0 101 000 100 */	cp1610_cmpr(cpustate, 0,4);	break;
		case 0x145: /* 0 101 000 101 */	cp1610_cmpr(cpustate, 0,5);	break;
		case 0x146: /* 0 101 000 110 */	cp1610_cmpr(cpustate, 0,6);	break;
		case 0x147: /* 0 101 000 111 */	cp1610_cmpr(cpustate, 0,7);	break;

		case 0x148: /* 0 101 001 000 */	cp1610_cmpr(cpustate, 1,0);	break;
		case 0x149: /* 0 101 001 001 */	cp1610_cmpr(cpustate, 1,1);	break;
		case 0x14a: /* 0 101 001 010 */	cp1610_cmpr(cpustate, 1,2);	break;
		case 0x14b: /* 0 101 001 011 */	cp1610_cmpr(cpustate, 1,3);	break;
		case 0x14c: /* 0 101 001 100 */	cp1610_cmpr(cpustate, 1,4);	break;
		case 0x14d: /* 0 101 001 101 */	cp1610_cmpr(cpustate, 1,5);	break;
		case 0x14e: /* 0 101 001 110 */	cp1610_cmpr(cpustate, 1,6);	break;
		case 0x14f: /* 0 101 001 111 */	cp1610_cmpr(cpustate, 1,7);	break;

		case 0x150: /* 0 101 010 000 */	cp1610_cmpr(cpustate, 2,0);	break;
		case 0x151: /* 0 101 010 001 */	cp1610_cmpr(cpustate, 2,1);	break;
		case 0x152: /* 0 101 010 010 */	cp1610_cmpr(cpustate, 2,2);	break;
		case 0x153: /* 0 101 010 011 */	cp1610_cmpr(cpustate, 2,3);	break;
		case 0x154: /* 0 101 010 100 */	cp1610_cmpr(cpustate, 2,4);	break;
		case 0x155: /* 0 101 010 101 */	cp1610_cmpr(cpustate, 2,5);	break;
		case 0x156: /* 0 101 010 110 */	cp1610_cmpr(cpustate, 2,6);	break;
		case 0x157: /* 0 101 010 111 */	cp1610_cmpr(cpustate, 2,7);	break;

		case 0x158: /* 0 101 011 000 */	cp1610_cmpr(cpustate, 3,0);	break;
		case 0x159: /* 0 101 011 001 */	cp1610_cmpr(cpustate, 3,1);	break;
		case 0x15a: /* 0 101 011 010 */	cp1610_cmpr(cpustate, 3,2);	break;
		case 0x15b: /* 0 101 011 011 */	cp1610_cmpr(cpustate, 3,3);	break;
		case 0x15c: /* 0 101 011 100 */	cp1610_cmpr(cpustate, 3,4);	break;
		case 0x15d: /* 0 101 011 101 */	cp1610_cmpr(cpustate, 3,5);	break;
		case 0x15e: /* 0 101 011 110 */	cp1610_cmpr(cpustate, 3,6);	break;
		case 0x15f: /* 0 101 011 111 */	cp1610_cmpr(cpustate, 3,7);	break;

		case 0x160: /* 0 101 100 000 */	cp1610_cmpr(cpustate, 4,0);	break;
		case 0x161: /* 0 101 100 001 */	cp1610_cmpr(cpustate, 4,1);	break;
		case 0x162: /* 0 101 100 010 */	cp1610_cmpr(cpustate, 4,2);	break;
		case 0x163: /* 0 101 100 011 */	cp1610_cmpr(cpustate, 4,3);	break;
		case 0x164: /* 0 101 100 100 */	cp1610_cmpr(cpustate, 4,4);	break;
		case 0x165: /* 0 101 100 101 */	cp1610_cmpr(cpustate, 4,5);	break;
		case 0x166: /* 0 101 100 110 */	cp1610_cmpr(cpustate, 4,6);	break;
		case 0x167: /* 0 101 100 111 */	cp1610_cmpr(cpustate, 4,7);	break;

		case 0x168: /* 0 101 101 000 */	cp1610_cmpr(cpustate, 5,0);	break;
		case 0x169: /* 0 101 101 001 */	cp1610_cmpr(cpustate, 5,1);	break;
		case 0x16a: /* 0 101 101 010 */	cp1610_cmpr(cpustate, 5,2);	break;
		case 0x16b: /* 0 101 101 011 */	cp1610_cmpr(cpustate, 5,3);	break;
		case 0x16c: /* 0 101 101 100 */	cp1610_cmpr(cpustate, 5,4);	break;
		case 0x16d: /* 0 101 101 101 */	cp1610_cmpr(cpustate, 5,5);	break;
		case 0x16e: /* 0 101 101 110 */	cp1610_cmpr(cpustate, 5,6);	break;
		case 0x16f: /* 0 101 101 111 */	cp1610_cmpr(cpustate, 5,7);	break;

		case 0x170: /* 0 101 110 000 */	cp1610_cmpr(cpustate, 6,0);	break;
		case 0x171: /* 0 101 110 001 */	cp1610_cmpr(cpustate, 6,1);	break;
		case 0x172: /* 0 101 110 010 */	cp1610_cmpr(cpustate, 6,2);	break;
		case 0x173: /* 0 101 110 011 */	cp1610_cmpr(cpustate, 6,3);	break;
		case 0x174: /* 0 101 110 100 */	cp1610_cmpr(cpustate, 6,4);	break;
		case 0x175: /* 0 101 110 101 */	cp1610_cmpr(cpustate, 6,5);	break;
		case 0x176: /* 0 101 110 110 */	cp1610_cmpr(cpustate, 6,6);	break;
		case 0x177: /* 0 101 110 111 */	cp1610_cmpr(cpustate, 6,7);	break;

		case 0x178: /* 0 101 111 000 */	cp1610_cmpr(cpustate, 7,0);	break;
		case 0x179: /* 0 101 111 001 */	cp1610_cmpr(cpustate, 7,1);	break;
		case 0x17a: /* 0 101 111 010 */	cp1610_cmpr(cpustate, 7,2);	break;
		case 0x17b: /* 0 101 111 011 */	cp1610_cmpr(cpustate, 7,3);	break;
		case 0x17c: /* 0 101 111 100 */	cp1610_cmpr(cpustate, 7,4);	break;
		case 0x17d: /* 0 101 111 101 */	cp1610_cmpr(cpustate, 7,5);	break;
		case 0x17e: /* 0 101 111 110 */	cp1610_cmpr(cpustate, 7,6);	break;
		case 0x17f: /* 0 101 111 111 */	cp1610_cmpr(cpustate, 7,7);	break;

		case 0x180: /* 0 110 000 000 */	cp1610_andr(cpustate, 0,0);	break;
		case 0x181: /* 0 110 000 001 */	cp1610_andr(cpustate, 0,1);	break;
		case 0x182: /* 0 110 000 010 */	cp1610_andr(cpustate, 0,2);	break;
		case 0x183: /* 0 110 000 011 */	cp1610_andr(cpustate, 0,3);	break;
		case 0x184: /* 0 110 000 100 */	cp1610_andr(cpustate, 0,4);	break;
		case 0x185: /* 0 110 000 101 */	cp1610_andr(cpustate, 0,5);	break;
		case 0x186: /* 0 110 000 110 */	cp1610_andr(cpustate, 0,6);	break;
		case 0x187: /* 0 110 000 111 */	cp1610_andr(cpustate, 0,7);	break;

		case 0x188: /* 0 110 001 000 */	cp1610_andr(cpustate, 1,0);	break;
		case 0x189: /* 0 110 001 001 */	cp1610_andr(cpustate, 1,1);	break;
		case 0x18a: /* 0 110 001 010 */	cp1610_andr(cpustate, 1,2);	break;
		case 0x18b: /* 0 110 001 011 */	cp1610_andr(cpustate, 1,3);	break;
		case 0x18c: /* 0 110 001 100 */	cp1610_andr(cpustate, 1,4);	break;
		case 0x18d: /* 0 110 001 101 */	cp1610_andr(cpustate, 1,5);	break;
		case 0x18e: /* 0 110 001 110 */	cp1610_andr(cpustate, 1,6);	break;
		case 0x18f: /* 0 110 001 111 */	cp1610_andr(cpustate, 1,7);	break;

		case 0x190: /* 0 110 010 000 */	cp1610_andr(cpustate, 2,0);	break;
		case 0x191: /* 0 110 010 001 */	cp1610_andr(cpustate, 2,1);	break;
		case 0x192: /* 0 110 010 010 */	cp1610_andr(cpustate, 2,2);	break;
		case 0x193: /* 0 110 010 011 */	cp1610_andr(cpustate, 2,3);	break;
		case 0x194: /* 0 110 010 100 */	cp1610_andr(cpustate, 2,4);	break;
		case 0x195: /* 0 110 010 101 */	cp1610_andr(cpustate, 2,5);	break;
		case 0x196: /* 0 110 010 110 */	cp1610_andr(cpustate, 2,6);	break;
		case 0x197: /* 0 110 010 111 */	cp1610_andr(cpustate, 2,7);	break;

		case 0x198: /* 0 110 011 000 */	cp1610_andr(cpustate, 3,0);	break;
		case 0x199: /* 0 110 011 001 */	cp1610_andr(cpustate, 3,1);	break;
		case 0x19a: /* 0 110 011 010 */	cp1610_andr(cpustate, 3,2);	break;
		case 0x19b: /* 0 110 011 011 */	cp1610_andr(cpustate, 3,3);	break;
		case 0x19c: /* 0 110 011 100 */	cp1610_andr(cpustate, 3,4);	break;
		case 0x19d: /* 0 110 011 101 */	cp1610_andr(cpustate, 3,5);	break;
		case 0x19e: /* 0 110 011 110 */	cp1610_andr(cpustate, 3,6);	break;
		case 0x19f: /* 0 110 011 111 */	cp1610_andr(cpustate, 3,7);	break;

		case 0x1a0: /* 0 110 100 000 */	cp1610_andr(cpustate, 4,0);	break;
		case 0x1a1: /* 0 110 100 001 */	cp1610_andr(cpustate, 4,1);	break;
		case 0x1a2: /* 0 110 100 010 */	cp1610_andr(cpustate, 4,2);	break;
		case 0x1a3: /* 0 110 100 011 */	cp1610_andr(cpustate, 4,3);	break;
		case 0x1a4: /* 0 110 100 100 */	cp1610_andr(cpustate, 4,4);	break;
		case 0x1a5: /* 0 110 100 101 */	cp1610_andr(cpustate, 4,5);	break;
		case 0x1a6: /* 0 110 100 110 */	cp1610_andr(cpustate, 4,6);	break;
		case 0x1a7: /* 0 110 100 111 */	cp1610_andr(cpustate, 4,7);	break;

		case 0x1a8: /* 0 110 101 000 */	cp1610_andr(cpustate, 5,0);	break;
		case 0x1a9: /* 0 110 101 001 */	cp1610_andr(cpustate, 5,1);	break;
		case 0x1aa: /* 0 110 101 010 */	cp1610_andr(cpustate, 5,2);	break;
		case 0x1ab: /* 0 110 101 011 */	cp1610_andr(cpustate, 5,3);	break;
		case 0x1ac: /* 0 110 101 100 */	cp1610_andr(cpustate, 5,4);	break;
		case 0x1ad: /* 0 110 101 101 */	cp1610_andr(cpustate, 5,5);	break;
		case 0x1ae: /* 0 110 101 110 */	cp1610_andr(cpustate, 5,6);	break;
		case 0x1af: /* 0 110 101 111 */	cp1610_andr(cpustate, 5,7);	break;

		case 0x1b0: /* 0 110 110 000 */	cp1610_andr(cpustate, 6,0);	break;
		case 0x1b1: /* 0 110 110 001 */	cp1610_andr(cpustate, 6,1);	break;
		case 0x1b2: /* 0 110 110 010 */	cp1610_andr(cpustate, 6,2);	break;
		case 0x1b3: /* 0 110 110 011 */	cp1610_andr(cpustate, 6,3);	break;
		case 0x1b4: /* 0 110 110 100 */	cp1610_andr(cpustate, 6,4);	break;
		case 0x1b5: /* 0 110 110 101 */	cp1610_andr(cpustate, 6,5);	break;
		case 0x1b6: /* 0 110 110 110 */	cp1610_andr(cpustate, 6,6);	break;
		case 0x1b7: /* 0 110 110 111 */	cp1610_andr(cpustate, 6,7);	break;

		case 0x1b8: /* 0 110 111 000 */	cp1610_andr(cpustate, 7,0);	break;
		case 0x1b9: /* 0 110 111 001 */	cp1610_andr(cpustate, 7,1);	break;
		case 0x1ba: /* 0 110 111 010 */	cp1610_andr(cpustate, 7,2);	break;
		case 0x1bb: /* 0 110 111 011 */	cp1610_andr(cpustate, 7,3);	break;
		case 0x1bc: /* 0 110 111 100 */	cp1610_andr(cpustate, 7,4);	break;
		case 0x1bd: /* 0 110 111 101 */	cp1610_andr(cpustate, 7,5);	break;
		case 0x1be: /* 0 110 111 110 */	cp1610_andr(cpustate, 7,6);	break;
		case 0x1bf: /* 0 110 111 111 */	cp1610_andr(cpustate, 7,7);	break;

		case 0x1c0: /* 0 111 000 000 */	cp1610_clrr(cpustate, 0);		break;
		case 0x1c1: /* 0 111 000 001 */	cp1610_xorr(cpustate, 0,1);	break;
		case 0x1c2: /* 0 111 000 010 */	cp1610_xorr(cpustate, 0,2);	break;
		case 0x1c3: /* 0 111 000 011 */	cp1610_xorr(cpustate, 0,3);	break;
		case 0x1c4: /* 0 111 000 100 */	cp1610_xorr(cpustate, 0,4);	break;
		case 0x1c5: /* 0 111 000 101 */	cp1610_xorr(cpustate, 0,5);	break;
		case 0x1c6: /* 0 111 000 110 */	cp1610_xorr(cpustate, 0,6);	break;
		case 0x1c7: /* 0 111 000 111 */	cp1610_xorr(cpustate, 0,7);	break;

		case 0x1c8: /* 0 111 001 000 */	cp1610_xorr(cpustate, 1,0);	break;
		case 0x1c9: /* 0 111 001 001 */	cp1610_clrr(cpustate, 1);		break;
		case 0x1ca: /* 0 111 001 010 */	cp1610_xorr(cpustate, 1,2);	break;
		case 0x1cb: /* 0 111 001 011 */	cp1610_xorr(cpustate, 1,3);	break;
		case 0x1cc: /* 0 111 001 100 */	cp1610_xorr(cpustate, 1,4);	break;
		case 0x1cd: /* 0 111 001 101 */	cp1610_xorr(cpustate, 1,5);	break;
		case 0x1ce: /* 0 111 001 110 */	cp1610_xorr(cpustate, 1,6);	break;
		case 0x1cf: /* 0 111 001 111 */	cp1610_xorr(cpustate, 1,7);	break;

		case 0x1d0: /* 0 111 010 000 */	cp1610_xorr(cpustate, 2,0);	break;
		case 0x1d1: /* 0 111 010 001 */	cp1610_xorr(cpustate, 2,1);	break;
		case 0x1d2: /* 0 111 010 010 */	cp1610_clrr(cpustate, 2);		break;
		case 0x1d3: /* 0 111 010 011 */	cp1610_xorr(cpustate, 2,3);	break;
		case 0x1d4: /* 0 111 010 100 */	cp1610_xorr(cpustate, 2,4);	break;
		case 0x1d5: /* 0 111 010 101 */	cp1610_xorr(cpustate, 2,5);	break;
		case 0x1d6: /* 0 111 010 110 */	cp1610_xorr(cpustate, 2,6);	break;
		case 0x1d7: /* 0 111 010 111 */	cp1610_xorr(cpustate, 2,7);	break;

		case 0x1d8: /* 0 111 011 000 */	cp1610_xorr(cpustate, 3,0);	break;
		case 0x1d9: /* 0 111 011 001 */	cp1610_xorr(cpustate, 3,1);	break;
		case 0x1da: /* 0 111 011 010 */	cp1610_xorr(cpustate, 3,2);	break;
		case 0x1db: /* 0 111 011 011 */	cp1610_clrr(cpustate, 3);		break;
		case 0x1dc: /* 0 111 011 100 */	cp1610_xorr(cpustate, 3,4);	break;
		case 0x1dd: /* 0 111 011 101 */	cp1610_xorr(cpustate, 3,5);	break;
		case 0x1de: /* 0 111 011 110 */	cp1610_xorr(cpustate, 3,6);	break;
		case 0x1df: /* 0 111 011 111 */	cp1610_xorr(cpustate, 3,7);	break;

		case 0x1e0: /* 0 111 100 000 */	cp1610_xorr(cpustate, 4,0);	break;
		case 0x1e1: /* 0 111 100 001 */	cp1610_xorr(cpustate, 4,1);	break;
		case 0x1e2: /* 0 111 100 010 */	cp1610_xorr(cpustate, 4,2);	break;
		case 0x1e3: /* 0 111 100 011 */	cp1610_xorr(cpustate, 4,3);	break;
		case 0x1e4: /* 0 111 100 100 */	cp1610_clrr(cpustate, 4);		break;
		case 0x1e5: /* 0 111 100 101 */	cp1610_xorr(cpustate, 4,5);	break;
		case 0x1e6: /* 0 111 100 110 */	cp1610_xorr(cpustate, 4,6);	break;
		case 0x1e7: /* 0 111 100 111 */	cp1610_xorr(cpustate, 4,7);	break;

		case 0x1e8: /* 0 111 101 000 */	cp1610_xorr(cpustate, 5,0);	break;
		case 0x1e9: /* 0 111 101 001 */	cp1610_xorr(cpustate, 5,1);	break;
		case 0x1ea: /* 0 111 101 010 */	cp1610_xorr(cpustate, 5,2);	break;
		case 0x1eb: /* 0 111 101 011 */	cp1610_xorr(cpustate, 5,3);	break;
		case 0x1ec: /* 0 111 101 100 */	cp1610_xorr(cpustate, 5,4);	break;
		case 0x1ed: /* 0 111 101 101 */	cp1610_clrr(cpustate, 5);		break;
		case 0x1ee: /* 0 111 101 110 */	cp1610_xorr(cpustate, 5,6);	break;
		case 0x1ef: /* 0 111 101 111 */	cp1610_xorr(cpustate, 5,7);	break;

		case 0x1f0: /* 0 111 110 000 */	cp1610_xorr(cpustate, 6,0);	break;
		case 0x1f1: /* 0 111 110 001 */	cp1610_xorr(cpustate, 6,1);	break;
		case 0x1f2: /* 0 111 110 010 */	cp1610_xorr(cpustate, 6,2);	break;
		case 0x1f3: /* 0 111 110 011 */	cp1610_xorr(cpustate, 6,3);	break;
		case 0x1f4: /* 0 111 110 100 */	cp1610_xorr(cpustate, 6,4);	break;
		case 0x1f5: /* 0 111 110 101 */	cp1610_xorr(cpustate, 6,5);	break;
		case 0x1f6: /* 0 111 110 110 */	cp1610_clrr(cpustate, 6);		break;
		case 0x1f7: /* 0 111 110 111 */	cp1610_xorr(cpustate, 6,7);	break;

		case 0x1f8: /* 0 111 111 000 */	cp1610_xorr(cpustate, 7,0);	break;
		case 0x1f9: /* 0 111 111 001 */	cp1610_xorr(cpustate, 7,1);	break;
		case 0x1fa: /* 0 111 111 010 */	cp1610_xorr(cpustate, 7,2);	break;
		case 0x1fb: /* 0 111 111 011 */	cp1610_xorr(cpustate, 7,3);	break;
		case 0x1fc: /* 0 111 111 100 */	cp1610_xorr(cpustate, 7,4);	break;
		case 0x1fd: /* 0 111 111 101 */	cp1610_xorr(cpustate, 7,5);	break;
		case 0x1fe: /* 0 111 111 110 */	cp1610_xorr(cpustate, 7,6);	break;
		case 0x1ff: /* 0 110 111 111 */	cp1610_clrr(cpustate, 7);		break;

		case 0x200: /* 1 000 000 000 */ cp1610_b(cpustate, 0);		break;
		case 0x201: /* 1 000 000 001 */ cp1610_bc(cpustate, 0);		break; /* aka BLGE */
		case 0x202: /* 1 000 000 010 */ cp1610_bov(cpustate, 0);		break;
		case 0x203:	/* 1 000 000 011 */ cp1610_bpl(cpustate, 0);		break;
		case 0x204:	/* 1 000 000 100 */ cp1610_bze(cpustate, 0);		break; /* aka BEQ */
		case 0x205:	/* 1 000 000 101 */ cp1610_blt(cpustate, 0);		break;
		case 0x206:	/* 1 000 000 110 */ cp1610_ble(cpustate, 0);		break;
		case 0x207:	/* 1 000 000 111 */ cp1610_busc(cpustate, 0);		break;

		case 0x208:	/* 1 000 001 000 */ cp1610_nopp(cpustate, 0);		break;
		case 0x209:	/* 1 000 001 001 */ cp1610_bnc(cpustate, 0);		break; /* aka BLLT */
		case 0x20a:	/* 1 000 001 010 */ cp1610_bnov(cpustate, 0);		break;
		case 0x20b:	/* 1 000 001 011 */ cp1610_bmi(cpustate, 0);		break;
		case 0x20c:	/* 1 000 001 100 */ cp1610_bnze(cpustate, 0);		break; /* aka BNEQ */
		case 0x20d:	/* 1 000 001 101 */ cp1610_bge(cpustate, 0);		break;
		case 0x20e:	/* 1 000 001 110 */ cp1610_bgt(cpustate, 0);		break;
		case 0x20f:	/* 1 000 001 111 */ cp1610_besc(cpustate, 0);		break;

		case 0x210: /* 1 000 010 000 */ cp1610_bext(cpustate, 0,0);	break;
		case 0x211: /* 1 000 010 001 */ cp1610_bext(cpustate, 1,0);	break;
		case 0x212: /* 1 000 010 010 */ cp1610_bext(cpustate, 2,0);	break;
		case 0x213:	/* 1 000 010 011 */ cp1610_bext(cpustate, 3,0);	break;
		case 0x214:	/* 1 000 010 100 */ cp1610_bext(cpustate, 4,0);	break;
		case 0x215:	/* 1 000 010 101 */ cp1610_bext(cpustate, 5,0);	break;
		case 0x216:	/* 1 000 010 110 */ cp1610_bext(cpustate, 6,0);	break;
		case 0x217:	/* 1 000 010 111 */ cp1610_bext(cpustate, 7,0);	break;

		case 0x218:	/* 1 000 011 000 */ cp1610_bext(cpustate, 8,0);	break;
		case 0x219:	/* 1 000 011 001 */ cp1610_bext(cpustate, 9,0);	break;
		case 0x21a:	/* 1 000 011 010 */ cp1610_bext(cpustate, 10,0);	break;
		case 0x21b:	/* 1 000 011 011 */ cp1610_bext(cpustate, 11,0);	break;
		case 0x21c:	/* 1 000 011 100 */ cp1610_bext(cpustate, 12,0);	break;
		case 0x21d:	/* 1 000 011 101 */ cp1610_bext(cpustate, 13,0);	break;
		case 0x21e:	/* 1 000 011 110 */ cp1610_bext(cpustate, 14,0);	break;
		case 0x21f:	/* 1 000 011 111 */ cp1610_bext(cpustate, 15,0);	break;

		case 0x220:	/* 1 000 100 000 */ cp1610_b(cpustate, 0xffff);		break;
		case 0x221:	/* 1 000 100 001 */ cp1610_bc(cpustate, 0xffff);		break; /* aka BLGE */
		case 0x222:	/* 1 000 100 010 */ cp1610_bov(cpustate, 0xffff);		break;
		case 0x223:	/* 1 000 100 011 */ cp1610_bpl(cpustate, 0xffff);		break;
		case 0x224:	/* 1 000 100 100 */ cp1610_bze(cpustate, 0xffff);		break; /* aka BEQ */
		case 0x225:	/* 1 000 100 101 */ cp1610_blt(cpustate, 0xffff);		break;
		case 0x226:	/* 1 000 100 110 */ cp1610_ble(cpustate, 0xffff);		break;
		case 0x227:	/* 1 000 100 111 */ cp1610_busc(cpustate, 0xffff);	break;

		case 0x228:	/* 1 000 101 000 */ cp1610_nopp(cpustate, 0xffff);	break;
		case 0x229:	/* 1 000 101 001 */ cp1610_bnc(cpustate, 0xffff);		break; /* aka BLLT */
		case 0x22a:	/* 1 000 101 010 */ cp1610_bnov(cpustate, 0xffff);	break;
		case 0x22b:	/* 1 000 101 011 */ cp1610_bmi(cpustate, 0xffff);		break;
		case 0x22c:	/* 1 000 101 100 */ cp1610_bnze(cpustate, 0xffff);	break; /* aka BNEQ */
		case 0x22d:	/* 1 000 101 101 */ cp1610_bge(cpustate, 0xffff);		break;
		case 0x22e:	/* 1 000 101 110 */ cp1610_bgt(cpustate, 0xffff);		break;
		case 0x22f:	/* 1 000 101 111 */ cp1610_besc(cpustate, 0xffff);	break;

		case 0x230:	/* 1 000 110 000 */ cp1610_bext(cpustate, 0,0xffff);	break;
		case 0x231:	/* 1 000 110 001 */ cp1610_bext(cpustate, 1,0xffff);	break;
		case 0x232:	/* 1 000 110 010 */ cp1610_bext(cpustate, 2,0xffff);	break;
		case 0x233:	/* 1 000 110 011 */ cp1610_bext(cpustate, 3,0xffff);	break;
		case 0x234:	/* 1 000 110 100 */ cp1610_bext(cpustate, 4,0xffff);	break;
		case 0x235:	/* 1 000 110 101 */ cp1610_bext(cpustate, 5,0xffff);	break;
		case 0x236:	/* 1 000 110 110 */ cp1610_bext(cpustate, 6,0xffff);	break;
		case 0x237:	/* 1 000 110 111 */ cp1610_bext(cpustate, 7,0xffff);	break;

		case 0x238:	/* 1 000 111 000 */ cp1610_bext(cpustate, 8,0xffff);	break;
		case 0x239:	/* 1 000 111 001 */ cp1610_bext(cpustate, 9,0xffff);	break;
		case 0x23a:	/* 1 000 111 010 */ cp1610_bext(cpustate, 10,0xffff);	break;
		case 0x23b:	/* 1 000 111 011 */ cp1610_bext(cpustate, 11,0xffff);	break;
		case 0x23c:	/* 1 000 111 100 */ cp1610_bext(cpustate, 12,0xffff);	break;
		case 0x23d:	/* 1 000 111 101 */ cp1610_bext(cpustate, 13,0xffff);	break;
		case 0x23e:	/* 1 000 111 110 */ cp1610_bext(cpustate, 14,0xffff);	break;
		case 0x23f:	/* 1 000 111 111 */ cp1610_bext(cpustate, 15,0xffff);	break;

		case 0x240: /* 1 001 000 000 */ cp1610_mvo(cpustate, 0);			break;
		case 0x241: /* 1 001 000 001 */ cp1610_mvo(cpustate, 1);			break;
		case 0x242: /* 1 001 000 010 */ cp1610_mvo(cpustate, 2);			break;
		case 0x243: /* 1 001 000 011 */ cp1610_mvo(cpustate, 3);			break;
		case 0x244: /* 1 001 000 100 */ cp1610_mvo(cpustate, 4);			break;
		case 0x245: /* 1 001 000 101 */ cp1610_mvo(cpustate, 5);			break;
		case 0x246: /* 1 001 000 110 */ cp1610_mvo(cpustate, 6);			break;
		case 0x247: /* 1 001 000 111 */ cp1610_mvo(cpustate, 7);			break;

		case 0x248: /* 1 001 001 000 */ cp1610_mvoat(cpustate, 0,1);		break;
		case 0x249: /* 1 001 001 001 */ cp1610_mvoat(cpustate, 1,1);		break;
		case 0x24a: /* 1 001 001 010 */ cp1610_mvoat(cpustate, 2,1);		break;
		case 0x24b: /* 1 001 001 011 */ cp1610_mvoat(cpustate, 3,1);		break;
		case 0x24c: /* 1 001 001 100 */ cp1610_mvoat(cpustate, 4,1);		break;
		case 0x24d: /* 1 001 001 101 */ cp1610_mvoat(cpustate, 5,1);		break;
		case 0x24e: /* 1 001 001 110 */ cp1610_mvoat(cpustate, 6,1);		break;
		case 0x24f: /* 1 001 001 111 */ cp1610_mvoat(cpustate, 7,1);		break;

		case 0x250: /* 1 001 010 000 */ cp1610_mvoat(cpustate, 0,2);		break;
		case 0x251: /* 1 001 010 001 */ cp1610_mvoat(cpustate, 1,2);		break;
		case 0x252: /* 1 001 010 010 */ cp1610_mvoat(cpustate, 2,2);		break;
		case 0x253: /* 1 001 010 011 */ cp1610_mvoat(cpustate, 3,2);		break;
		case 0x254: /* 1 001 010 100 */ cp1610_mvoat(cpustate, 4,2);		break;
		case 0x255: /* 1 001 010 101 */ cp1610_mvoat(cpustate, 5,2);		break;
		case 0x256: /* 1 001 010 110 */ cp1610_mvoat(cpustate, 6,2);		break;
		case 0x257: /* 1 001 010 111 */ cp1610_mvoat(cpustate, 7,2);		break;

		case 0x258: /* 1 001 011 000 */ cp1610_mvoat(cpustate, 0,3);		break;
		case 0x259: /* 1 001 011 001 */ cp1610_mvoat(cpustate, 1,3);		break;
		case 0x25a: /* 1 001 011 010 */ cp1610_mvoat(cpustate, 2,3);		break;
		case 0x25b: /* 1 001 011 011 */ cp1610_mvoat(cpustate, 3,3);		break;
		case 0x25c: /* 1 001 011 100 */ cp1610_mvoat(cpustate, 4,3);		break;
		case 0x25d: /* 1 001 011 101 */ cp1610_mvoat(cpustate, 5,3);		break;
		case 0x25e: /* 1 001 011 110 */ cp1610_mvoat(cpustate, 6,3);		break;
		case 0x25f: /* 1 001 011 111 */ cp1610_mvoat(cpustate, 7,3);		break;

		case 0x260: /* 1 001 100 000 */ cp1610_mvoat_i(cpustate, 0,4);	break;
		case 0x261: /* 1 001 100 001 */ cp1610_mvoat_i(cpustate, 1,4);	break;
		case 0x262: /* 1 001 100 010 */ cp1610_mvoat_i(cpustate, 2,4);	break;
		case 0x263: /* 1 001 100 011 */ cp1610_mvoat_i(cpustate, 3,4);	break;
		case 0x264: /* 1 001 100 100 */ cp1610_mvoat_i(cpustate, 4,4);	break;
		case 0x265: /* 1 001 100 101 */ cp1610_mvoat_i(cpustate, 5,4);	break;
		case 0x266: /* 1 001 100 110 */ cp1610_mvoat_i(cpustate, 6,4);	break;
		case 0x267: /* 1 001 100 111 */ cp1610_mvoat_i(cpustate, 7,4);	break;

		case 0x268: /* 1 001 101 000 */ cp1610_mvoat_i(cpustate, 0,5);	break;
		case 0x269: /* 1 001 101 001 */ cp1610_mvoat_i(cpustate, 1,5);	break;
		case 0x26a: /* 1 001 101 010 */ cp1610_mvoat_i(cpustate, 2,5);	break;
		case 0x26b: /* 1 001 101 011 */ cp1610_mvoat_i(cpustate, 3,5);	break;
		case 0x26c: /* 1 001 101 100 */ cp1610_mvoat_i(cpustate, 4,5);	break;
		case 0x26d: /* 1 001 101 101 */ cp1610_mvoat_i(cpustate, 5,5);	break;
		case 0x26e: /* 1 001 101 110 */ cp1610_mvoat_i(cpustate, 6,5);	break;
		case 0x26f: /* 1 001 101 111 */ cp1610_mvoat_i(cpustate, 7,5);	break;

		case 0x270: /* 1 001 110 000 */ cp1610_mvoat_i(cpustate, 0,6);	break; /* pshr */
		case 0x271: /* 1 001 110 001 */ cp1610_mvoat_i(cpustate, 1,6);	break; /* pshr */
		case 0x272: /* 1 001 110 010 */ cp1610_mvoat_i(cpustate, 2,6);	break; /* pshr */
		case 0x273: /* 1 001 110 011 */ cp1610_mvoat_i(cpustate, 3,6);	break; /* pshr */
		case 0x274: /* 1 001 110 100 */ cp1610_mvoat_i(cpustate, 4,6);	break; /* pshr */
		case 0x275: /* 1 001 110 101 */ cp1610_mvoat_i(cpustate, 5,6);	break; /* pshr */
		case 0x276: /* 1 001 110 110 */ cp1610_mvoat_i(cpustate, 6,6);	break; /* pshr */
		case 0x277: /* 1 001 110 111 */ cp1610_mvoat_i(cpustate, 7,6);	break; /* pshr */

		case 0x278: /* 1 001 111 000 */ cp1610_mvoi(cpustate, 0);			break;
		case 0x279: /* 1 001 111 001 */ cp1610_mvoi(cpustate, 1);			break;
		case 0x27a: /* 1 001 111 010 */ cp1610_mvoi(cpustate, 2);			break;
		case 0x27b: /* 1 001 111 011 */ cp1610_mvoi(cpustate, 3);			break;
		case 0x27c: /* 1 001 111 100 */ cp1610_mvoi(cpustate, 4);			break;
		case 0x27d: /* 1 001 111 101 */ cp1610_mvoi(cpustate, 5);			break;
		case 0x27e: /* 1 001 111 110 */ cp1610_mvoi(cpustate, 6);			break;
		case 0x27f: /* 1 001 111 111 */ cp1610_mvoi(cpustate, 7);			break;

		case 0x280: /* 1 010 000 000 */ cp1610_mvi(cpustate, 0);			break;
		case 0x281: /* 1 010 000 001 */ cp1610_mvi(cpustate, 1);			break;
		case 0x282: /* 1 010 000 010 */ cp1610_mvi(cpustate, 2);			break;
		case 0x283: /* 1 010 000 011 */ cp1610_mvi(cpustate, 3);			break;
		case 0x284: /* 1 010 000 100 */ cp1610_mvi(cpustate, 4);			break;
		case 0x285: /* 1 010 000 101 */ cp1610_mvi(cpustate, 5);			break;
		case 0x286: /* 1 010 000 110 */ cp1610_mvi(cpustate, 6);			break;
		case 0x287: /* 1 010 000 111 */ cp1610_mvi(cpustate, 7);			break;

		case 0x288: /* 1 010 001 000 */ cp1610_mviat(cpustate, 1,0);		break;
		case 0x289: /* 1 010 001 001 */ cp1610_mviat(cpustate, 1,1);		break;
		case 0x28a: /* 1 010 001 010 */ cp1610_mviat(cpustate, 1,2);		break;
		case 0x28b: /* 1 010 001 011 */ cp1610_mviat(cpustate, 1,3);		break;
		case 0x28c: /* 1 010 001 100 */ cp1610_mviat(cpustate, 1,4);		break;
		case 0x28d: /* 1 010 001 101 */ cp1610_mviat(cpustate, 1,5);		break;
		case 0x28e: /* 1 010 001 110 */ cp1610_mviat(cpustate, 1,6);		break;
		case 0x28f: /* 1 010 001 111 */ cp1610_mviat(cpustate, 1,7);		break;

		case 0x290: /* 1 010 010 000 */ cp1610_mviat(cpustate, 2,0);		break;
		case 0x291: /* 1 010 010 001 */ cp1610_mviat(cpustate, 2,1);		break;
		case 0x292: /* 1 010 010 010 */ cp1610_mviat(cpustate, 2,2);		break;
		case 0x293: /* 1 010 010 011 */ cp1610_mviat(cpustate, 2,3);		break;
		case 0x294: /* 1 010 010 100 */ cp1610_mviat(cpustate, 2,4);		break;
		case 0x295: /* 1 010 010 101 */ cp1610_mviat(cpustate, 2,5);		break;
		case 0x296: /* 1 010 010 110 */ cp1610_mviat(cpustate, 2,6);		break;
		case 0x297: /* 1 010 010 111 */ cp1610_mviat(cpustate, 2,7);		break;

		case 0x298: /* 1 010 011 000 */ cp1610_mviat(cpustate, 3,0);		break;
		case 0x299: /* 1 010 011 001 */ cp1610_mviat(cpustate, 3,1);		break;
		case 0x29a: /* 1 010 011 010 */ cp1610_mviat(cpustate, 3,2);		break;
		case 0x29b: /* 1 010 011 011 */ cp1610_mviat(cpustate, 3,3);		break;
		case 0x29c: /* 1 010 011 100 */ cp1610_mviat(cpustate, 3,4);		break;
		case 0x29d: /* 1 010 011 101 */ cp1610_mviat(cpustate, 3,5);		break;
		case 0x29e: /* 1 010 011 110 */ cp1610_mviat(cpustate, 3,6);		break;
		case 0x29f: /* 1 010 011 111 */ cp1610_mviat(cpustate, 3,7);		break;

		case 0x2a0: /* 1 010 100 000 */ cp1610_mviat_i(cpustate, 4,0);	break;
		case 0x2a1: /* 1 010 100 001 */ cp1610_mviat_i(cpustate, 4,1);	break;
		case 0x2a2: /* 1 010 100 010 */ cp1610_mviat_i(cpustate, 4,2);	break;
		case 0x2a3: /* 1 010 100 011 */ cp1610_mviat_i(cpustate, 4,3);	break;
		case 0x2a4: /* 1 010 100 100 */ cp1610_mviat_i(cpustate, 4,4);	break;
		case 0x2a5: /* 1 010 100 101 */ cp1610_mviat_i(cpustate, 4,5);	break;
		case 0x2a6: /* 1 010 100 110 */ cp1610_mviat_i(cpustate, 4,6);	break;
		case 0x2a7: /* 1 010 100 111 */ cp1610_mviat_i(cpustate, 4,7);	break;

		case 0x2a8: /* 1 010 101 000 */ cp1610_mviat_i(cpustate, 5,0);	break;
		case 0x2a9: /* 1 010 101 001 */ cp1610_mviat_i(cpustate, 5,1);	break;
		case 0x2aa: /* 1 010 101 010 */ cp1610_mviat_i(cpustate, 5,2);	break;
		case 0x2ab: /* 1 010 101 011 */ cp1610_mviat_i(cpustate, 5,3);	break;
		case 0x2ac: /* 1 010 101 100 */ cp1610_mviat_i(cpustate, 5,4);	break;
		case 0x2ad: /* 1 010 101 101 */ cp1610_mviat_i(cpustate, 5,5);	break;
		case 0x2ae: /* 1 010 101 110 */ cp1610_mviat_i(cpustate, 5,6);	break;
		case 0x2af: /* 1 010 101 111 */ cp1610_mviat_i(cpustate, 5,7);	break;

		case 0x2b0: /* 1 010 110 000 */ cp1610_pulr(cpustate, 0);			break;
		case 0x2b1: /* 1 010 110 001 */ cp1610_pulr(cpustate, 1);			break;
		case 0x2b2: /* 1 010 110 010 */ cp1610_pulr(cpustate, 2);			break;
		case 0x2b3: /* 1 010 110 011 */ cp1610_pulr(cpustate, 3);			break;
		case 0x2b4: /* 1 010 110 100 */ cp1610_pulr(cpustate, 4);			break;
		case 0x2b5: /* 1 010 110 101 */ cp1610_pulr(cpustate, 5);			break;
		case 0x2b6: /* 1 010 110 110 */ cp1610_pulr(cpustate, 6);			break;
		case 0x2b7: /* 1 010 110 111 */ cp1610_pulr(cpustate, 7);			break;

		case 0x2b8: /* 1 010 111 000 */ cp1610_mvii(cpustate, 0);			break;
		case 0x2b9: /* 1 010 111 001 */ cp1610_mvii(cpustate, 1);			break;
		case 0x2ba: /* 1 010 111 010 */ cp1610_mvii(cpustate, 2);			break;
		case 0x2bb: /* 1 010 111 011 */ cp1610_mvii(cpustate, 3);			break;
		case 0x2bc: /* 1 010 111 100 */ cp1610_mvii(cpustate, 4);			break;
		case 0x2bd: /* 1 010 111 101 */ cp1610_mvii(cpustate, 5);			break;
		case 0x2be: /* 1 010 111 110 */ cp1610_mvii(cpustate, 6);			break;
		case 0x2bf: /* 1 010 111 111 */ cp1610_mvii(cpustate, 7);			break;

		case 0x2c0: /* 1 011 000 000 */ cp1610_add(cpustate, 0);			break;
		case 0x2c1: /* 1 011 000 001 */ cp1610_add(cpustate, 1);			break;
		case 0x2c2: /* 1 011 000 010 */ cp1610_add(cpustate, 2);			break;
		case 0x2c3: /* 1 011 000 011 */ cp1610_add(cpustate, 3);			break;
		case 0x2c4: /* 1 011 000 100 */ cp1610_add(cpustate, 4);			break;
		case 0x2c5: /* 1 011 000 101 */ cp1610_add(cpustate, 5);			break;
		case 0x2c6: /* 1 011 000 110 */ cp1610_add(cpustate, 6);			break;
		case 0x2c7: /* 1 011 000 111 */ cp1610_add(cpustate, 7);			break;

		case 0x2c8: /* 1 011 001 000 */ cp1610_addat(cpustate, 1,0);		break;
		case 0x2c9: /* 1 011 001 001 */ cp1610_addat(cpustate, 1,1);		break;
		case 0x2ca: /* 1 011 001 010 */ cp1610_addat(cpustate, 1,2);		break;
		case 0x2cb: /* 1 011 001 011 */ cp1610_addat(cpustate, 1,3);		break;
		case 0x2cc: /* 1 011 001 100 */ cp1610_addat(cpustate, 1,4);		break;
		case 0x2cd: /* 1 011 001 101 */ cp1610_addat(cpustate, 1,5);		break;
		case 0x2ce: /* 1 011 001 110 */ cp1610_addat(cpustate, 1,6);		break;
		case 0x2cf: /* 1 011 001 111 */ cp1610_addat(cpustate, 1,7);		break;

		case 0x2d0: /* 1 011 010 000 */ cp1610_addat(cpustate, 2,0);		break;
		case 0x2d1: /* 1 011 010 001 */ cp1610_addat(cpustate, 2,1);		break;
		case 0x2d2: /* 1 011 010 010 */ cp1610_addat(cpustate, 2,2);		break;
		case 0x2d3: /* 1 011 010 011 */ cp1610_addat(cpustate, 2,3);		break;
		case 0x2d4: /* 1 011 010 100 */ cp1610_addat(cpustate, 2,4);		break;
		case 0x2d5: /* 1 011 010 101 */ cp1610_addat(cpustate, 2,5);		break;
		case 0x2d6: /* 1 011 010 110 */ cp1610_addat(cpustate, 2,6);		break;
		case 0x2d7: /* 1 011 010 111 */ cp1610_addat(cpustate, 2,7);		break;

		case 0x2d8: /* 1 011 011 000 */ cp1610_addat(cpustate, 3,0);		break;
		case 0x2d9: /* 1 011 011 001 */ cp1610_addat(cpustate, 3,1);		break;
		case 0x2da: /* 1 011 011 010 */ cp1610_addat(cpustate, 3,2);		break;
		case 0x2db: /* 1 011 011 011 */ cp1610_addat(cpustate, 3,3);		break;
		case 0x2dc: /* 1 011 011 100 */ cp1610_addat(cpustate, 3,4);		break;
		case 0x2dd: /* 1 011 011 101 */ cp1610_addat(cpustate, 3,5);		break;
		case 0x2de: /* 1 011 011 110 */ cp1610_addat(cpustate, 3,6);		break;
		case 0x2df: /* 1 011 011 111 */ cp1610_addat(cpustate, 3,7);		break;

		case 0x2e0: /* 1 011 100 000 */ cp1610_addat_i(cpustate, 4,0);	break;
		case 0x2e1: /* 1 011 100 001 */ cp1610_addat_i(cpustate, 4,1);	break;
		case 0x2e2: /* 1 011 100 010 */ cp1610_addat_i(cpustate, 4,2);	break;
		case 0x2e3: /* 1 011 100 011 */ cp1610_addat_i(cpustate, 4,3);	break;
		case 0x2e4: /* 1 011 100 100 */ cp1610_addat_i(cpustate, 4,4);	break;
		case 0x2e5: /* 1 011 100 101 */ cp1610_addat_i(cpustate, 4,5);	break;
		case 0x2e6: /* 1 011 100 110 */ cp1610_addat_i(cpustate, 4,6);	break;
		case 0x2e7: /* 1 011 100 111 */ cp1610_addat_i(cpustate, 4,7);	break;

		case 0x2e8: /* 1 011 101 000 */ cp1610_addat_i(cpustate, 5,0);	break;
		case 0x2e9: /* 1 011 101 001 */ cp1610_addat_i(cpustate, 5,1);	break;
		case 0x2ea: /* 1 011 101 010 */ cp1610_addat_i(cpustate, 5,2);	break;
		case 0x2eb: /* 1 011 101 011 */ cp1610_addat_i(cpustate, 5,3);	break;
		case 0x2ec: /* 1 011 101 100 */ cp1610_addat_i(cpustate, 5,4);	break;
		case 0x2ed: /* 1 011 101 101 */ cp1610_addat_i(cpustate, 5,5);	break;
		case 0x2ee: /* 1 011 101 110 */ cp1610_addat_i(cpustate, 5,6);	break;
		case 0x2ef: /* 1 011 101 111 */ cp1610_addat_i(cpustate, 5,7);	break;

		case 0x2f0: /* 1 011 110 000 */ cp1610_addat_d(cpustate, 6,0);	break;
		case 0x2f1: /* 1 011 110 001 */ cp1610_addat_d(cpustate, 6,1);	break;
		case 0x2f2: /* 1 011 110 010 */ cp1610_addat_d(cpustate, 6,2);	break;
		case 0x2f3: /* 1 011 110 011 */ cp1610_addat_d(cpustate, 6,3);	break;
		case 0x2f4: /* 1 011 110 100 */ cp1610_addat_d(cpustate, 6,4);	break;
		case 0x2f5: /* 1 011 110 101 */ cp1610_addat_d(cpustate, 6,5);	break;
		case 0x2f6: /* 1 011 110 110 */ cp1610_addat_d(cpustate, 6,6);	break;
		case 0x2f7: /* 1 011 110 111 */ cp1610_addat_d(cpustate, 6,7);	break;

		case 0x2f8: /* 1 011 111 000 */ cp1610_addi(cpustate, 0);			break;
		case 0x2f9: /* 1 011 111 001 */ cp1610_addi(cpustate, 1);			break;
		case 0x2fa: /* 1 011 111 010 */ cp1610_addi(cpustate, 2);			break;
		case 0x2fb: /* 1 011 111 011 */ cp1610_addi(cpustate, 3);			break;
		case 0x2fc: /* 1 011 111 100 */ cp1610_addi(cpustate, 4);			break;
		case 0x2fd: /* 1 011 111 101 */ cp1610_addi(cpustate, 5);			break;
		case 0x2fe: /* 1 011 111 110 */ cp1610_addi(cpustate, 6);			break;
		case 0x2ff: /* 1 011 111 111 */ cp1610_addi(cpustate, 7);			break;

		case 0x300: /* 1 100 000 000 */ cp1610_sub(cpustate, 0);			break;
		case 0x301: /* 1 100 000 001 */ cp1610_sub(cpustate, 1);			break;
		case 0x302: /* 1 100 000 010 */ cp1610_sub(cpustate, 2);			break;
		case 0x303: /* 1 100 000 011 */ cp1610_sub(cpustate, 3);			break;
		case 0x304: /* 1 100 000 100 */ cp1610_sub(cpustate, 4);			break;
		case 0x305: /* 1 100 000 101 */ cp1610_sub(cpustate, 5);			break;
		case 0x306: /* 1 100 000 110 */ cp1610_sub(cpustate, 6);			break;
		case 0x307: /* 1 100 000 111 */ cp1610_sub(cpustate, 7);			break;

		case 0x308: /* 1 100 001 000 */ cp1610_subat(cpustate, 1,0);		break;
		case 0x309: /* 1 100 001 001 */ cp1610_subat(cpustate, 1,1);		break;
		case 0x30a: /* 1 100 001 010 */ cp1610_subat(cpustate, 1,2);		break;
		case 0x30b: /* 1 100 001 011 */ cp1610_subat(cpustate, 1,3);		break;
		case 0x30c: /* 1 100 001 100 */ cp1610_subat(cpustate, 1,4);		break;
		case 0x30d: /* 1 100 001 101 */ cp1610_subat(cpustate, 1,5);		break;
		case 0x30e: /* 1 100 001 110 */ cp1610_subat(cpustate, 1,6);		break;
		case 0x30f: /* 1 100 001 111 */ cp1610_subat(cpustate, 1,7);		break;

		case 0x310: /* 1 100 010 000 */ cp1610_subat(cpustate, 2,0);		break;
		case 0x311: /* 1 100 010 001 */ cp1610_subat(cpustate, 2,1);		break;
		case 0x312: /* 1 100 010 010 */ cp1610_subat(cpustate, 2,2);		break;
		case 0x313: /* 1 100 010 011 */ cp1610_subat(cpustate, 2,3);		break;
		case 0x314: /* 1 100 010 100 */ cp1610_subat(cpustate, 2,4);		break;
		case 0x315: /* 1 100 010 101 */ cp1610_subat(cpustate, 2,5);		break;
		case 0x316: /* 1 100 010 110 */ cp1610_subat(cpustate, 2,6);		break;
		case 0x317: /* 1 100 010 111 */ cp1610_subat(cpustate, 2,7);		break;

		case 0x318: /* 1 100 011 000 */ cp1610_subat(cpustate, 3,0);		break;
		case 0x319: /* 1 100 011 001 */ cp1610_subat(cpustate, 3,1);		break;
		case 0x31a: /* 1 100 011 010 */ cp1610_subat(cpustate, 3,2);		break;
		case 0x31b: /* 1 100 011 011 */ cp1610_subat(cpustate, 3,3);		break;
		case 0x31c: /* 1 100 011 100 */ cp1610_subat(cpustate, 3,4);		break;
		case 0x31d: /* 1 100 011 101 */ cp1610_subat(cpustate, 3,5);		break;
		case 0x31e: /* 1 100 011 110 */ cp1610_subat(cpustate, 3,6);		break;
		case 0x31f: /* 1 100 011 111 */ cp1610_subat(cpustate, 3,7);		break;

		case 0x320: /* 1 100 100 000 */ cp1610_subat_i(cpustate, 4,0);	break;
		case 0x321: /* 1 100 100 001 */ cp1610_subat_i(cpustate, 4,1);	break;
		case 0x322: /* 1 100 100 010 */ cp1610_subat_i(cpustate, 4,2);	break;
		case 0x323: /* 1 100 100 011 */ cp1610_subat_i(cpustate, 4,3);	break;
		case 0x324: /* 1 100 100 100 */ cp1610_subat_i(cpustate, 4,4);	break;
		case 0x325: /* 1 100 100 101 */ cp1610_subat_i(cpustate, 4,5);	break;
		case 0x326: /* 1 100 100 110 */ cp1610_subat_i(cpustate, 4,6);	break;
		case 0x327: /* 1 100 100 111 */ cp1610_subat_i(cpustate, 4,7);	break;

		case 0x328: /* 1 100 101 000 */ cp1610_subat_i(cpustate, 5,0);	break;
		case 0x329: /* 1 100 101 001 */ cp1610_subat_i(cpustate, 5,1);	break;
		case 0x32a: /* 1 100 101 010 */ cp1610_subat_i(cpustate, 5,2);	break;
		case 0x32b: /* 1 100 101 011 */ cp1610_subat_i(cpustate, 5,3);	break;
		case 0x32c: /* 1 100 101 100 */ cp1610_subat_i(cpustate, 5,4);	break;
		case 0x32d: /* 1 100 101 101 */ cp1610_subat_i(cpustate, 5,5);	break;
		case 0x32e: /* 1 100 101 110 */ cp1610_subat_i(cpustate, 5,6);	break;
		case 0x32f: /* 1 100 101 111 */ cp1610_subat_i(cpustate, 5,7);	break;

		case 0x330: /* 1 100 110 000 */ cp1610_subat_d(cpustate, 6,0);	break;
		case 0x331: /* 1 100 110 001 */ cp1610_subat_d(cpustate, 6,1);	break;
		case 0x332: /* 1 100 110 010 */ cp1610_subat_d(cpustate, 6,2);	break;
		case 0x333: /* 1 100 110 011 */ cp1610_subat_d(cpustate, 6,3);	break;
		case 0x334: /* 1 100 110 100 */ cp1610_subat_d(cpustate, 6,4);	break;
		case 0x335: /* 1 100 110 101 */ cp1610_subat_d(cpustate, 6,5);	break;
		case 0x336: /* 1 100 110 110 */ cp1610_subat_d(cpustate, 6,6);	break;
		case 0x337: /* 1 100 110 111 */ cp1610_subat_d(cpustate, 6,7);	break;

		case 0x338: /* 1 100 111 000 */ cp1610_subi(cpustate, 0);			break;
		case 0x339: /* 1 100 111 001 */ cp1610_subi(cpustate, 1);			break;
		case 0x33a: /* 1 100 111 010 */ cp1610_subi(cpustate, 2);			break;
		case 0x33b: /* 1 100 111 011 */ cp1610_subi(cpustate, 3);			break;
		case 0x33c: /* 1 100 111 100 */ cp1610_subi(cpustate, 4);			break;
		case 0x33d: /* 1 100 111 101 */ cp1610_subi(cpustate, 5);			break;
		case 0x33e: /* 1 100 111 110 */ cp1610_subi(cpustate, 6);			break;
		case 0x33f: /* 1 100 111 111 */ cp1610_subi(cpustate, 7);			break;

		case 0x340: /* 1 101 000 000 */ cp1610_cmp(cpustate, 0);			break;
		case 0x341: /* 1 101 000 001 */ cp1610_cmp(cpustate, 1);			break;
		case 0x342: /* 1 101 000 010 */ cp1610_cmp(cpustate, 2);			break;
		case 0x343: /* 1 101 000 011 */ cp1610_cmp(cpustate, 3);			break;
		case 0x344: /* 1 101 000 100 */ cp1610_cmp(cpustate, 4);			break;
		case 0x345: /* 1 101 000 101 */ cp1610_cmp(cpustate, 5);			break;
		case 0x346: /* 1 101 000 110 */ cp1610_cmp(cpustate, 6);			break;
		case 0x347: /* 1 101 000 111 */ cp1610_cmp(cpustate, 7);			break;

		case 0x348: /* 1 101 001 000 */ cp1610_cmpat(cpustate, 1,0);		break;
		case 0x349: /* 1 101 001 001 */ cp1610_cmpat(cpustate, 1,1);		break;
		case 0x34a: /* 1 101 001 010 */ cp1610_cmpat(cpustate, 1,2);		break;
		case 0x34b: /* 1 101 001 011 */ cp1610_cmpat(cpustate, 1,3);		break;
		case 0x34c: /* 1 101 001 100 */ cp1610_cmpat(cpustate, 1,4);		break;
		case 0x34d: /* 1 101 001 101 */ cp1610_cmpat(cpustate, 1,5);		break;
		case 0x34e: /* 1 101 001 110 */ cp1610_cmpat(cpustate, 1,6);		break;
		case 0x34f: /* 1 101 001 111 */ cp1610_cmpat(cpustate, 1,7);		break;

		case 0x350: /* 1 101 010 000 */ cp1610_cmpat(cpustate, 2,0);		break;
		case 0x351: /* 1 101 010 001 */ cp1610_cmpat(cpustate, 2,1);		break;
		case 0x352: /* 1 101 010 010 */ cp1610_cmpat(cpustate, 2,2);		break;
		case 0x353: /* 1 101 010 011 */ cp1610_cmpat(cpustate, 2,3);		break;
		case 0x354: /* 1 101 010 100 */ cp1610_cmpat(cpustate, 2,4);		break;
		case 0x355: /* 1 101 010 101 */ cp1610_cmpat(cpustate, 2,5);		break;
		case 0x356: /* 1 101 010 110 */ cp1610_cmpat(cpustate, 2,6);		break;
		case 0x357: /* 1 101 010 111 */ cp1610_cmpat(cpustate, 2,7);		break;

		case 0x358: /* 1 101 011 000 */ cp1610_cmpat(cpustate, 3,0);		break;
		case 0x359: /* 1 101 011 001 */ cp1610_cmpat(cpustate, 3,1);		break;
		case 0x35a: /* 1 101 011 010 */ cp1610_cmpat(cpustate, 3,2);		break;
		case 0x35b: /* 1 101 011 011 */ cp1610_cmpat(cpustate, 3,3);		break;
		case 0x35c: /* 1 101 011 100 */ cp1610_cmpat(cpustate, 3,4);		break;
		case 0x35d: /* 1 101 011 101 */ cp1610_cmpat(cpustate, 3,5);		break;
		case 0x35e: /* 1 101 011 110 */ cp1610_cmpat(cpustate, 3,6);		break;
		case 0x35f: /* 1 101 011 111 */ cp1610_cmpat(cpustate, 3,7);		break;

		case 0x360: /* 1 101 100 000 */ cp1610_cmpat_i(cpustate, 4,0);	break;
		case 0x361: /* 1 101 100 001 */ cp1610_cmpat_i(cpustate, 4,1);	break;
		case 0x362: /* 1 101 100 010 */ cp1610_cmpat_i(cpustate, 4,2);	break;
		case 0x363: /* 1 101 100 011 */ cp1610_cmpat_i(cpustate, 4,3);	break;
		case 0x364: /* 1 101 100 100 */ cp1610_cmpat_i(cpustate, 4,4);	break;
		case 0x365: /* 1 101 100 101 */ cp1610_cmpat_i(cpustate, 4,5);	break;
		case 0x366: /* 1 101 100 110 */ cp1610_cmpat_i(cpustate, 4,6);	break;
		case 0x367: /* 1 101 100 111 */ cp1610_cmpat_i(cpustate, 4,7);	break;

		case 0x368: /* 1 101 101 000 */ cp1610_cmpat_i(cpustate, 5,0);	break;
		case 0x369: /* 1 101 101 001 */ cp1610_cmpat_i(cpustate, 5,1);	break;
		case 0x36a: /* 1 101 101 010 */ cp1610_cmpat_i(cpustate, 5,2);	break;
		case 0x36b: /* 1 101 101 011 */ cp1610_cmpat_i(cpustate, 5,3);	break;
		case 0x36c: /* 1 101 101 100 */ cp1610_cmpat_i(cpustate, 5,4);	break;
		case 0x36d: /* 1 101 101 101 */ cp1610_cmpat_i(cpustate, 5,5);	break;
		case 0x36e: /* 1 101 101 110 */ cp1610_cmpat_i(cpustate, 5,6);	break;
		case 0x36f: /* 1 101 101 111 */ cp1610_cmpat_i(cpustate, 5,7);	break;

		case 0x370: /* 1 101 110 000 */ cp1610_cmpat_d(cpustate, 6,0);	break;
		case 0x371: /* 1 101 110 001 */ cp1610_cmpat_d(cpustate, 6,1);	break;
		case 0x372: /* 1 101 110 010 */ cp1610_cmpat_d(cpustate, 6,2);	break;
		case 0x373: /* 1 101 110 011 */ cp1610_cmpat_d(cpustate, 6,3);	break;
		case 0x374: /* 1 101 110 100 */ cp1610_cmpat_d(cpustate, 6,4);	break;
		case 0x375: /* 1 101 110 101 */ cp1610_cmpat_d(cpustate, 6,5);	break;
		case 0x376: /* 1 101 110 110 */ cp1610_cmpat_d(cpustate, 6,6);	break;
		case 0x377: /* 1 101 110 111 */ cp1610_cmpat_d(cpustate, 6,7);	break;

		case 0x378: /* 1 101 111 000 */ cp1610_cmpi(cpustate, 0);			break;
		case 0x379: /* 1 101 111 001 */ cp1610_cmpi(cpustate, 1);			break;
		case 0x37a: /* 1 101 111 010 */ cp1610_cmpi(cpustate, 2);			break;
		case 0x37b: /* 1 101 111 011 */ cp1610_cmpi(cpustate, 3);			break;
		case 0x37c: /* 1 101 111 100 */ cp1610_cmpi(cpustate, 4);			break;
		case 0x37d: /* 1 101 111 101 */ cp1610_cmpi(cpustate, 5);			break;
		case 0x37e: /* 1 101 111 110 */ cp1610_cmpi(cpustate, 6);			break;
		case 0x37f: /* 1 101 111 111 */ cp1610_cmpi(cpustate, 7);			break;

		case 0x380: /* 1 110 000 000 */ cp1610_and(cpustate, 0);			break;
		case 0x381: /* 1 110 000 001 */ cp1610_and(cpustate, 1);			break;
		case 0x382: /* 1 110 000 010 */ cp1610_and(cpustate, 2);			break;
		case 0x383: /* 1 110 000 011 */ cp1610_and(cpustate, 3);			break;
		case 0x384: /* 1 110 000 100 */ cp1610_and(cpustate, 4);			break;
		case 0x385: /* 1 110 000 101 */ cp1610_and(cpustate, 5);			break;
		case 0x386: /* 1 110 000 110 */ cp1610_and(cpustate, 6);			break;
		case 0x387: /* 1 110 000 111 */ cp1610_and(cpustate, 7);			break;

		case 0x388: /* 1 110 001 000 */ cp1610_andat(cpustate, 1,0);		break;
		case 0x389: /* 1 110 001 001 */ cp1610_andat(cpustate, 1,1);		break;
		case 0x38a: /* 1 110 001 010 */ cp1610_andat(cpustate, 1,2);		break;
		case 0x38b: /* 1 110 001 011 */ cp1610_andat(cpustate, 1,3);		break;
		case 0x38c: /* 1 110 001 100 */ cp1610_andat(cpustate, 1,4);		break;
		case 0x38d: /* 1 110 001 101 */ cp1610_andat(cpustate, 1,5);		break;
		case 0x38e: /* 1 110 001 110 */ cp1610_andat(cpustate, 1,6);		break;
		case 0x38f: /* 1 110 001 111 */ cp1610_andat(cpustate, 1,7);		break;

		case 0x390: /* 1 110 010 000 */ cp1610_andat(cpustate, 2,0);		break;
		case 0x391: /* 1 110 010 001 */ cp1610_andat(cpustate, 2,1);		break;
		case 0x392: /* 1 110 010 010 */ cp1610_andat(cpustate, 2,2);		break;
		case 0x393: /* 1 110 010 011 */ cp1610_andat(cpustate, 2,3);		break;
		case 0x394: /* 1 110 010 100 */ cp1610_andat(cpustate, 2,4);		break;
		case 0x395: /* 1 110 010 101 */ cp1610_andat(cpustate, 2,5);		break;
		case 0x396: /* 1 110 010 110 */ cp1610_andat(cpustate, 2,6);		break;
		case 0x397: /* 1 110 010 111 */ cp1610_andat(cpustate, 2,7);		break;

		case 0x398: /* 1 110 011 000 */ cp1610_andat(cpustate, 3,0);		break;
		case 0x399: /* 1 110 011 001 */ cp1610_andat(cpustate, 3,1);		break;
		case 0x39a: /* 1 110 011 010 */ cp1610_andat(cpustate, 3,2);		break;
		case 0x39b: /* 1 110 011 011 */ cp1610_andat(cpustate, 3,3);		break;
		case 0x39c: /* 1 110 011 100 */ cp1610_andat(cpustate, 3,4);		break;
		case 0x39d: /* 1 110 011 101 */ cp1610_andat(cpustate, 3,5);		break;
		case 0x39e: /* 1 110 011 110 */ cp1610_andat(cpustate, 3,6);		break;
		case 0x39f: /* 1 110 011 111 */ cp1610_andat(cpustate, 3,7);		break;

		case 0x3a0: /* 1 110 100 000 */ cp1610_andat_i(cpustate, 4,0);	break;
		case 0x3a1: /* 1 110 100 001 */ cp1610_andat_i(cpustate, 4,1);	break;
		case 0x3a2: /* 1 110 100 010 */ cp1610_andat_i(cpustate, 4,2);	break;
		case 0x3a3: /* 1 110 100 011 */ cp1610_andat_i(cpustate, 4,3);	break;
		case 0x3a4: /* 1 110 100 100 */ cp1610_andat_i(cpustate, 4,4);	break;
		case 0x3a5: /* 1 110 100 101 */ cp1610_andat_i(cpustate, 4,5);	break;
		case 0x3a6: /* 1 110 100 110 */ cp1610_andat_i(cpustate, 4,6);	break;
		case 0x3a7: /* 1 110 100 111 */ cp1610_andat_i(cpustate, 4,7);	break;

		case 0x3a8: /* 1 110 101 000 */ cp1610_andat_i(cpustate, 5,0);	break;
		case 0x3a9: /* 1 110 101 001 */ cp1610_andat_i(cpustate, 5,1);	break;
		case 0x3aa: /* 1 110 101 010 */ cp1610_andat_i(cpustate, 5,2);	break;
		case 0x3ab: /* 1 110 101 011 */ cp1610_andat_i(cpustate, 5,3);	break;
		case 0x3ac: /* 1 110 101 100 */ cp1610_andat_i(cpustate, 5,4);	break;
		case 0x3ad: /* 1 110 101 101 */ cp1610_andat_i(cpustate, 5,5);	break;
		case 0x3ae: /* 1 110 101 110 */ cp1610_andat_i(cpustate, 5,6);	break;
		case 0x3af: /* 1 110 101 111 */ cp1610_andat_i(cpustate, 5,7);	break;

		case 0x3b0: /* 1 110 110 000 */ cp1610_andat_d(cpustate, 6,0);	break;
		case 0x3b1: /* 1 110 110 001 */ cp1610_andat_d(cpustate, 6,1);	break;
		case 0x3b2: /* 1 110 110 010 */ cp1610_andat_d(cpustate, 6,2);	break;
		case 0x3b3: /* 1 110 110 011 */ cp1610_andat_d(cpustate, 6,3);	break;
		case 0x3b4: /* 1 110 110 100 */ cp1610_andat_d(cpustate, 6,4);	break;
		case 0x3b5: /* 1 110 110 101 */ cp1610_andat_d(cpustate, 6,5);	break;
		case 0x3b6: /* 1 110 110 110 */ cp1610_andat_d(cpustate, 6,6);	break;
		case 0x3b7: /* 1 110 110 111 */ cp1610_andat_d(cpustate, 6,7);	break;

		case 0x3b8: /* 1 110 111 000 */ cp1610_andi(cpustate, 0);			break;
		case 0x3b9: /* 1 110 111 001 */ cp1610_andi(cpustate, 1);			break;
		case 0x3ba: /* 1 110 111 010 */ cp1610_andi(cpustate, 2);			break;
		case 0x3bb: /* 1 110 111 011 */ cp1610_andi(cpustate, 3);			break;
		case 0x3bc: /* 1 110 111 100 */ cp1610_andi(cpustate, 4);			break;
		case 0x3bd: /* 1 110 111 101 */ cp1610_andi(cpustate, 5);			break;
		case 0x3be: /* 1 110 111 110 */ cp1610_andi(cpustate, 6);			break;
		case 0x3bf: /* 1 110 111 111 */ cp1610_andi(cpustate, 7);			break;

		case 0x3c0: /* 1 111 000 000 */ cp1610_xor(cpustate, 0);			break;
		case 0x3c1: /* 1 111 000 001 */ cp1610_xor(cpustate, 1);			break;
		case 0x3c2: /* 1 111 000 010 */ cp1610_xor(cpustate, 2);			break;
		case 0x3c3: /* 1 111 000 011 */ cp1610_xor(cpustate, 3);			break;
		case 0x3c4: /* 1 111 000 100 */ cp1610_xor(cpustate, 4);			break;
		case 0x3c5: /* 1 111 000 101 */ cp1610_xor(cpustate, 5);			break;
		case 0x3c6: /* 1 111 000 110 */ cp1610_xor(cpustate, 6);			break;
		case 0x3c7: /* 1 111 000 111 */ cp1610_xor(cpustate, 7);			break;

		case 0x3c8: /* 1 111 001 000 */ cp1610_xorat(cpustate, 1,0);		break;
		case 0x3c9: /* 1 111 001 001 */ cp1610_xorat(cpustate, 1,1);		break;
		case 0x3ca: /* 1 111 001 010 */ cp1610_xorat(cpustate, 1,2);		break;
		case 0x3cb: /* 1 111 001 011 */ cp1610_xorat(cpustate, 1,3);		break;
		case 0x3cc: /* 1 111 001 100 */ cp1610_xorat(cpustate, 1,4);		break;
		case 0x3cd: /* 1 111 001 101 */ cp1610_xorat(cpustate, 1,5);		break;
		case 0x3ce: /* 1 111 001 110 */ cp1610_xorat(cpustate, 1,6);		break;
		case 0x3cf: /* 1 111 001 111 */ cp1610_xorat(cpustate, 1,7);		break;

		case 0x3d0: /* 1 111 010 000 */ cp1610_xorat(cpustate, 2,0);		break;
		case 0x3d1: /* 1 111 010 001 */ cp1610_xorat(cpustate, 2,1);		break;
		case 0x3d2: /* 1 111 010 010 */ cp1610_xorat(cpustate, 2,2);		break;
		case 0x3d3: /* 1 111 010 011 */ cp1610_xorat(cpustate, 2,3);		break;
		case 0x3d4: /* 1 111 010 100 */ cp1610_xorat(cpustate, 2,4);		break;
		case 0x3d5: /* 1 111 010 101 */ cp1610_xorat(cpustate, 2,5);		break;
		case 0x3d6: /* 1 111 010 110 */ cp1610_xorat(cpustate, 2,6);		break;
		case 0x3d7: /* 1 111 010 111 */ cp1610_xorat(cpustate, 2,7);		break;

		case 0x3d8: /* 1 111 011 000 */ cp1610_xorat(cpustate, 3,0);		break;
		case 0x3d9: /* 1 111 011 001 */ cp1610_xorat(cpustate, 3,1);		break;
		case 0x3da: /* 1 111 011 010 */ cp1610_xorat(cpustate, 3,2);		break;
		case 0x3db: /* 1 111 011 011 */ cp1610_xorat(cpustate, 3,3);		break;
		case 0x3dc: /* 1 111 011 100 */ cp1610_xorat(cpustate, 3,4);		break;
		case 0x3dd: /* 1 111 011 101 */ cp1610_xorat(cpustate, 3,5);		break;
		case 0x3de: /* 1 111 011 110 */ cp1610_xorat(cpustate, 3,6);		break;
		case 0x3df: /* 1 111 011 111 */ cp1610_xorat(cpustate, 3,7);		break;

		case 0x3e0: /* 1 111 100 000 */ cp1610_xorat_i(cpustate, 4,0);	break;
		case 0x3e1: /* 1 111 100 001 */ cp1610_xorat_i(cpustate, 4,1);	break;
		case 0x3e2: /* 1 111 100 010 */ cp1610_xorat_i(cpustate, 4,2);	break;
		case 0x3e3: /* 1 111 100 011 */ cp1610_xorat_i(cpustate, 4,3);	break;
		case 0x3e4: /* 1 111 100 100 */ cp1610_xorat_i(cpustate, 4,4);	break;
		case 0x3e5: /* 1 111 100 101 */ cp1610_xorat_i(cpustate, 4,5);	break;
		case 0x3e6: /* 1 111 100 110 */ cp1610_xorat_i(cpustate, 4,6);	break;
		case 0x3e7: /* 1 111 100 111 */ cp1610_xorat_i(cpustate, 4,7);	break;

		case 0x3e8: /* 1 111 101 000 */ cp1610_xorat_i(cpustate, 5,0);	break;
		case 0x3e9: /* 1 111 101 001 */ cp1610_xorat_i(cpustate, 5,1);	break;
		case 0x3ea: /* 1 111 101 010 */ cp1610_xorat_i(cpustate, 5,2);	break;
		case 0x3eb: /* 1 111 101 011 */ cp1610_xorat_i(cpustate, 5,3);	break;
		case 0x3ec: /* 1 111 101 100 */ cp1610_xorat_i(cpustate, 5,4);	break;
		case 0x3ed: /* 1 111 101 101 */ cp1610_xorat_i(cpustate, 5,5);	break;
		case 0x3ee: /* 1 111 101 110 */ cp1610_xorat_i(cpustate, 5,6);	break;
		case 0x3ef: /* 1 111 101 111 */ cp1610_xorat_i(cpustate, 5,7);	break;

		case 0x3f0: /* 1 111 110 000 */ cp1610_xorat_d(cpustate, 6,0);	break;
		case 0x3f1: /* 1 111 110 001 */ cp1610_xorat_d(cpustate, 6,1);	break;
		case 0x3f2: /* 1 111 110 010 */ cp1610_xorat_d(cpustate, 6,2);	break;
		case 0x3f3: /* 1 111 110 011 */ cp1610_xorat_d(cpustate, 6,3);	break;
		case 0x3f4: /* 1 111 110 100 */ cp1610_xorat_d(cpustate, 6,4);	break;
		case 0x3f5: /* 1 111 110 101 */ cp1610_xorat_d(cpustate, 6,5);	break;
		case 0x3f6: /* 1 111 110 110 */ cp1610_xorat_d(cpustate, 6,6);	break;
		case 0x3f7: /* 1 111 110 111 */ cp1610_xorat_d(cpustate, 6,7);	break;

		case 0x3f8: /* 1 111 111 000 */ cp1610_xori(cpustate, 0);			break;
		case 0x3f9: /* 1 111 111 001 */ cp1610_xori(cpustate, 1);			break;
		case 0x3fa: /* 1 111 111 010 */ cp1610_xori(cpustate, 2);			break;
		case 0x3fb: /* 1 111 111 011 */ cp1610_xori(cpustate, 3);			break;
		case 0x3fc: /* 1 111 111 100 */ cp1610_xori(cpustate, 4);			break;
		case 0x3fd: /* 1 111 111 101 */ cp1610_xori(cpustate, 5);			break;
		case 0x3fe: /* 1 111 111 110 */ cp1610_xori(cpustate, 6);			break;
		case 0x3ff: /* 1 111 111 111 */ cp1610_xori(cpustate, 7);			break;
        }

        if (cpustate->mask_interrupts == 0)
        {
			if (cpustate->intr_pending == 1)
			{
				/* PSHR R7 */
				cp1610_writemem16(cpustate->r[6],cpustate->r[7]);
				cpustate->r[6]++;
				cpustate->icount -= 9;
				cpustate->intr_pending = 0;
				cpustate->r[7] = cpustate->irq_callback(cpustate->device, CP1610_INT_INTR);
			}
			if ((cpustate->intrm_pending == 1) && (cpustate->intr_enabled))
			{
				/* PSHR R7 */
				cp1610_writemem16(cpustate->r[6],cpustate->r[7]);
				cpustate->r[6]++;
				cpustate->icount -= 9;
				cpustate->intrm_pending = 0;
				cpustate->r[7] = cpustate->irq_callback(cpustate->device, CP1610_INT_INTRM);
			}
			if (cpustate->reset_pending == 1)
			{
				cpustate->reset_pending = 0;
				cpustate->r[7] = cpustate->irq_callback(cpustate->device, CP1610_RESET);
			}
		}

	} while( cpustate->icount > 0 );
}

static CPU_INIT( cp1610 )
{
	cp1610_state *cpustate = get_safe_token(device);
	cpustate->intr_enabled = 0;
	cpustate->reset_pending = 0;
	cpustate->intr_pending = 0;
	cpustate->intrm_pending = 0;
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);

	device->save_item(NAME(cpustate->r));
	device->save_item(NAME(cpustate->flags));
	device->save_item(NAME(cpustate->intr_enabled));
	device->save_item(NAME(cpustate->intr_vector));
	device->save_item(NAME(cpustate->reset_state));
	device->save_item(NAME(cpustate->intr_state));
	device->save_item(NAME(cpustate->intrm_state));
	device->save_item(NAME(cpustate->reset_pending));
	device->save_item(NAME(cpustate->intr_pending));
	device->save_item(NAME(cpustate->intrm_pending));
	device->save_item(NAME(cpustate->mask_interrupts));
}

static void cp1610_set_irq_line(cp1610_state *cpustate, UINT32 irqline, int state)
{
	switch(irqline)
	{
		case CP1610_INT_INTRM:
			cpustate->intrm_pending = (state == ASSERT_LINE);
			cpustate->intrm_state = state;
			break;
		case CP1610_RESET:
			if (state == ASSERT_LINE)
				cpustate->reset_pending = 1;
			cpustate->reset_state = state;
			break;
		case CP1610_INT_INTR:
			if (state == ASSERT_LINE)
				cpustate->intr_pending = 1;
			cpustate->intr_state = state;
			break;
	}
}

static CPU_SET_INFO( cp1610 )
{
	cp1610_state *cpustate = get_safe_token(device);
	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_PREVIOUSPC:	break;	/* TODO? */
	case CPUINFO_INT_INPUT_STATE + CP1610_INT_INTRM:
	case CPUINFO_INT_INPUT_STATE + CP1610_RESET:
	case CPUINFO_INT_INPUT_STATE + CP1610_INT_INTR:
			cp1610_set_irq_line(cpustate, state-CPUINFO_INT_INPUT_STATE, info->i);		break;

	case CPUINFO_INT_REGISTER + CP1610_R0: cpustate->r[0] = info->i;			break;
	case CPUINFO_INT_REGISTER + CP1610_R1: cpustate->r[1] = info->i;			break;
	case CPUINFO_INT_REGISTER + CP1610_R2: cpustate->r[2] = info->i;			break;
	case CPUINFO_INT_REGISTER + CP1610_R3: cpustate->r[3] = info->i;			break;
	case CPUINFO_INT_REGISTER + CP1610_R4: cpustate->r[4] = info->i;			break;
	case CPUINFO_INT_REGISTER + CP1610_R5: cpustate->r[5] = info->i;			break;
	case CPUINFO_INT_SP:
	case CPUINFO_INT_REGISTER + CP1610_R6: cpustate->r[6] = info->i;			break;
	case CPUINFO_INT_PC:
	case CPUINFO_INT_REGISTER + CP1610_R7: cpustate->r[7] = info->i;			break;
	}
	return;
}

CPU_GET_INFO( cp1610 )
{
	cp1610_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(cp1610_state);	break;
	case CPUINFO_INT_INPUT_LINES:						info->i = 2;			break;
	case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;			break;
	case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;	break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;			break;
	case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;			break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;			break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 8;			break;
	case CPUINFO_INT_MIN_CYCLES:					info->i = 1;			break;
	case CPUINFO_INT_MAX_CYCLES:					info->i = 7;			break;

	case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;	break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;	break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -1;	break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;	break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;	break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;	break;
	case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;	break;
	case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;	break;
	case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;	break;

	case CPUINFO_INT_PREVIOUSPC:		info->i = 0;	/* TODO??? */		break;

	case CPUINFO_INT_INPUT_STATE + CP1610_INT_INTRM:	info->i = cpustate->intrm_state;	break;
	case CPUINFO_INT_INPUT_STATE + CP1610_RESET:		info->i = cpustate->reset_state;	break;
	case CPUINFO_INT_INPUT_STATE + CP1610_INT_INTR:	info->i = cpustate->intr_state;	break;

	case CPUINFO_INT_REGISTER + CP1610_R0: info->i = cpustate->r[0];			break;
	case CPUINFO_INT_REGISTER + CP1610_R1: info->i = cpustate->r[1];			break;
	case CPUINFO_INT_REGISTER + CP1610_R2: info->i = cpustate->r[2];			break;
	case CPUINFO_INT_REGISTER + CP1610_R3: info->i = cpustate->r[3];			break;
	case CPUINFO_INT_REGISTER + CP1610_R4: info->i = cpustate->r[4];			break;
	case CPUINFO_INT_REGISTER + CP1610_R5: info->i = cpustate->r[5];			break;
	case CPUINFO_INT_SP:
	case CPUINFO_INT_REGISTER + CP1610_R6: info->i = cpustate->r[6];			break;
	case CPUINFO_INT_PC:
	case CPUINFO_INT_REGISTER + CP1610_R7: info->i = cpustate->r[7];			break;

	/* --- the following bits of info are returned as pointers to data or functions --- */
	case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(cp1610);		break;
	case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(cp1610);				break;
	case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(cp1610);				break;
	case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(cp1610);			break;
	case CPUINFO_FCT_BURN:							info->burn = NULL;						break;

	case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(cp1610);		break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;			break;

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	case CPUINFO_STR_NAME:			strcpy(info->s, "CP1610");		break;
	case CPUINFO_STR_FAMILY:	strcpy(info->s, "");				break;
	case CPUINFO_STR_VERSION:	strcpy(info->s, "1.0");			break;
	case CPUINFO_STR_SOURCE_FILE:		strcpy(info->s, __FILE__);		break;
	case CPUINFO_STR_CREDITS:	strcpy(info->s,
									"Copyright Frank Palazzolo, all rights reserved.");
									break;
    case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c",
				cpustate->flags & 0x80 ? 'S':'.',
				cpustate->flags & 0x40 ? 'Z':'.',
				cpustate->flags & 0x20 ? 'V':'.',
				cpustate->flags & 0x10 ? 'C':'.');
			break;
	case CPUINFO_STR_REGISTER+CP1610_R0: sprintf(info->s, "R0:%04X", cpustate->r[0]); break;
	case CPUINFO_STR_REGISTER+CP1610_R1: sprintf(info->s, "R1:%04X", cpustate->r[1]); break;
	case CPUINFO_STR_REGISTER+CP1610_R2: sprintf(info->s, "R2:%04X", cpustate->r[2]); break;
	case CPUINFO_STR_REGISTER+CP1610_R3: sprintf(info->s, "R3:%04X", cpustate->r[3]); break;
	case CPUINFO_STR_REGISTER+CP1610_R4: sprintf(info->s, "R4:%04X", cpustate->r[4]); break;
	case CPUINFO_STR_REGISTER+CP1610_R5: sprintf(info->s, "R5:%04X", cpustate->r[5]); break;
	case CPUINFO_STR_REGISTER+CP1610_R6: sprintf(info->s, "R6:%04X", cpustate->r[6]); break;
	case CPUINFO_STR_REGISTER+CP1610_R7: sprintf(info->s, "R7:%04X", cpustate->r[7]); break;
	}

	return;
}

DEFINE_LEGACY_CPU_DEVICE(CP1610, cp1610);
