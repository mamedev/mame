// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/* IBM/Motorola PowerPC 4xx/6xx Emulator */

#include <setjmp.h>
#include "emu.h"
#include "debugger.h"

/* avoid including setjmp.h and defining jump buffer if not included from here */
#define PPC_H_INCLUDED_FROM_PPC_C
#include "ppc.h"

// PLL Configuration based on the table in MPC603EUM page 7-31
static const int mpc603e_pll_config[12][9] =
{
	// 16,  20,  25,  33,  40,  50,  60,  66,  75
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{ 0x2, 0x2, 0x2, 0x1, 0x1, 0x1,  -1, 0x0,  -1 },
	{  -1,  -1,  -1,  -1,  -1, 0xc,  -1, 0xc,  -1 },
	{ 0x5, 0x5, 0x5, 0x4, 0x4, 0x4,  -1,  -1,  -1 },
	{  -1,  -1,  -1, 0x6, 0x6,  -1,  -1,  -1,  -1 },
	{  -1,  -1, 0x8, 0x8,  -1,  -1,  -1,  -1,  -1 },
	{  -1, 0xe, 0xe,  -1,  -1,  -1,  -1,  -1,  -1 },
	{ 0xa, 0xa, 0xa,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
};

// PLL Configuration based on the table in MPC603E7VEC page 29
static const int mpc603ev_pll_config[12][9] =
{
	// 16,  20,  25,  33,  40,  50,  60,  66,  75
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	// 2:1
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1, 0x4, 0x4 },
	// 2.5:1
	{  -1,  -1,  -1,  -1,  -1, 0x6, 0x6, 0x6, 0x6 },
	// 3:1
	{  -1,  -1,  -1,  -1, 0x8, 0x8, 0x8, 0x8, 0x8 },
	// 3.5:1
	{  -1,  -1,  -1,  -1, 0xe, 0xe, 0xe, 0xe,  -1 },
	// 4:1
	{  -1,  -1,  -1, 0xa, 0xa, 0xa, 0xa,  -1,  -1 },
	// 4.5:1
	{  -1,  -1,  -1, 0x7, 0x7, 0x7,  -1,  -1,  -1 },
	// 5:1
	{  -1,  -1, 0xb, 0xb, 0xb,  -1,  -1,  -1,  -1 },
	// 5.5:1
	{  -1,  -1, 0x9, 0x9, 0x9,  -1,  -1,  -1,  -1 },
	// 6:1
	{  -1,  -1, 0xd, 0xd, 0xd,  -1,  -1,  -1,  -1 }
};

// PLL Configuration based on the table in MPC603E7TEC page 23
static const int mpc603r_pll_config[12][9] =
{
	// 16,  20,  25,  33,  40,  50,  60,  66,  75
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	{  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	// 2:1
	{  -1,  -1,  -1,  -1, 0x5, 0x5, 0x5, 0x5, 0x5 },
	// 2.5:1
	{  -1,  -1,  -1,  -1,  -1,  -1, 0x6, 0x6, 0x6 },
	// 3:1
	{  -1,  -1,  -1,  -1,  -1, 0x8, 0x8, 0x8, 0x8 },
	// 3.5:1
	{  -1,  -1,  -1,  -1,  -1, 0xe, 0xe, 0xe, 0xe },
	// 4:1
	{  -1,  -1,  -1,  -1, 0xa, 0xa, 0xa, 0xa, 0xa },
	// 4.5:1
	{  -1,  -1,  -1, 0x7, 0x7, 0x7, 0x7, 0x7,  -1 },
	// 5:1
	{  -1,  -1,  -1, 0xb, 0xb, 0xb, 0xb,  -1,  -1 },
	// 5.5:1
	{  -1,  -1,  -1, 0x9, 0x9, 0x9,  -1,  -1,  -1 },
	// 6:1
	{  -1,  -1, 0xd, 0xd, 0xd, 0xd,  -1,  -1,  -1 },
};


static void ppc603_exception(int exception);
static void ppc602_exception(int exception);
static void ppc403_exception(int exception);
static UINT8 ppc403_spu_r(UINT32 a);
static void ppc403_spu_w(UINT32 a, UINT8 d);

#define RD              ((op >> 21) & 0x1F)
#define RT              ((op >> 21) & 0x1f)
#define RS              ((op >> 21) & 0x1f)
#define RA              ((op >> 16) & 0x1f)
#define RB              ((op >> 11) & 0x1f)
#define RC              ((op >> 6) & 0x1f)

#define MB              ((op >> 6) & 0x1f)
#define ME              ((op >> 1) & 0x1f)
#define SH              ((op >> 11) & 0x1f)
#define BO              ((op >> 21) & 0x1f)
#define BI              ((op >> 16) & 0x1f)
#define CRFD            ((op >> 23) & 0x7)
#define CRFA            ((op >> 18) & 0x7)
#define FXM             ((op >> 12) & 0xff)
#define SPR             (((op >> 16) & 0x1f) | ((op >> 6) & 0x3e0))

#define SIMM16          (INT32)(INT16)(op & 0xffff)
#define UIMM16          (UINT32)(op & 0xffff)

#define RCBIT           (op & 0x1)
#define OEBIT           (op & 0x400)
#define AABIT           (op & 0x2)
#define LKBIT           (op & 0x1)

#define REG(x)          (m_r[x])
#define LR              (m_lr)
#define CTR             (m_ctr)
#define XER             (m_xer)
#define CR(x)           (m_cr[x])
#define MSR             (m_msr)
#define SRR0            (m_srr0)
#define SRR1            (m_srr1)
#define SRR2            (m_srr2)
#define SRR3            (m_srr3)
#define EVPR            (m_evpr)
#define EXIER           (m_exier)
#define EXISR           (m_exisr)
#define DEC             (m_dec)


// Stuff added for the 6xx
#define FPR(x)          (m_fpr[x])
#define FM              ((op >> 17) & 0xFF)
#define SPRF            (((op >> 6) & 0x3E0) | ((op >> 16) & 0x1F))


#define CHECK_SUPERVISOR()          \
	if((m_msr & 0x4000) != 0){    \
	}

#define CHECK_FPU_AVAILABLE()       \
	if((m_msr & 0x2000) == 0){    \
	}

static UINT32       ppc_field_xlat[256];



#define FPSCR_FX        0x80000000
#define FPSCR_FEX       0x40000000
#define FPSCR_VX        0x20000000
#define FPSCR_OX        0x10000000
#define FPSCR_UX        0x08000000
#define FPSCR_ZX        0x04000000
#define FPSCR_XX        0x02000000



#define BITMASK_0(n)    (UINT32)(((UINT64)1 << n) - 1)
#define CRBIT(x)        ((m_cr[x / 4] & (1 << (3 - (x % 4)))) ? 1 : 0)
#define _BIT(n)         (1 << (n))
#define GET_ROTATE_MASK(mb,me)      (ppc_rotate_mask[mb][me])
#define ADD_CA(r,a,b)       ((UINT32)r < (UINT32)a)
#define SUB_CA(r,a,b)       (!((UINT32)a < (UINT32)b))
#define ADD_OV(r,a,b)       ((~((a) ^ (b)) & ((a) ^ (r))) & 0x80000000)
#define SUB_OV(r,a,b)       (( ((a) ^ (b)) & ((a) ^ (r))) & 0x80000000)

#define XER_SO          0x80000000
#define XER_OV          0x40000000
#define XER_CA          0x20000000

#define MSR_AP          0x00800000  /* Access privilege state (PPC602) */
#define MSR_SA          0x00400000  /* Supervisor access mode (PPC602) */
#define MSR_POW         0x00040000  /* Power Management Enable */
#define MSR_WE          0x00040000
#define MSR_CE          0x00020000
#define MSR_ILE         0x00010000  /* Interrupt Little Endian Mode */
#define MSR_EE          0x00008000  /* External Interrupt Enable */
#define MSR_PR          0x00004000  /* Problem State */
#define MSR_FP          0x00002000  /* Floating Point Available */
#define MSR_ME          0x00001000  /* Machine Check Enable */
#define MSR_FE0         0x00000800
#define MSR_SE          0x00000400  /* Single Step Trace Enable */
#define MSR_BE          0x00000200  /* Branch Trace Enable */
#define MSR_DE          0x00000200
#define MSR_FE1         0x00000100
#define MSR_IP          0x00000040  /* Interrupt Prefix */
#define MSR_IR          0x00000020  /* Instruction Relocate */
#define MSR_DR          0x00000010  /* Data Relocate */
#define MSR_PE          0x00000008
#define MSR_PX          0x00000004
#define MSR_RI          0x00000002  /* Recoverable Interrupt Enable */
#define MSR_LE          0x00000001

#define TSR_ENW         0x80000000
#define TSR_WIS         0x40000000

#define BYTE_REVERSE16(x)   ((((x) >> 8) & 0xff) | (((x) << 8) & 0xff00))
#define BYTE_REVERSE32(x)   ((((x) >> 24) & 0xff) | (((x) >> 8) & 0xff00) | (((x) << 8) & 0xff0000) | (((x) << 24) & 0xff000000))


const device_type PPC403 = &device_creator<ppc403_device>;
const device_type PPC405 = &device_creator<ppc405_device>;
const device_type PPC601 = &device_creator<ppc601_device>;
const device_type PPC602 = &device_creator<ppc602_device>;
const device_type PPC603 = &device_creator<ppc603_device>;
const device_type PPC603E = &device_creator<ppc603e_device>;
const device_type PPC603R = &device_creator<ppc603r_device>;
const device_type PPC604 = &device_creator<ppc604_device>;
const device_type MPC8240 = &device_creator<mpc8240_device>;
const device_type PPC403GA = &device_creator<ppc403ga_device>;
const device_type PPC403GCX = &device_creator<ppc403gcx_device>;
const device_type PPC405GP = &device_creator<ppc405gp_device>;


struct PPC_OPCODE {
	int code;
	int subcode;
	void (* handler)(UINT32);
};



static UINT32 ppc_rotate_mask[32][32];

#define ROPCODE(pc)         memory_decrypted_read_dword(m_program, pc)
#define ROPCODE64(pc)       memory_decrypted_read_qword(m_program, DWORD_XOR_BE(pc))


ppc_device::ppc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int address_bits, powerpc_flavor flavor, UINT32 cap, UINT32 tb_divisor)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, __FILE__)
	, m_program_config("program", ENDIANNESS_BIG, address_bits, 32)
	, m_core(NULL)
	, c_bus_frequency(0)
	, m_bus_freq_multiplier(1)
	, m_flavor(flavor)
	, m_cap(cap)
	, m_tb_divisor(tb_divisor)
	, m_vtlb(NULL)
	, m_cache(CACHE_SIZE + sizeof(internal_ppc_state))
{
}

//ppc403_device::ppc403_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//  : ppc_device(mconfig, PPC403, "PPC403", tag, owner, clock, "ppc403", 32)
//{
//}
//
//ppc405_device::ppc405_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//  : ppc_device(mconfig, PPC405, "PPC405", tag, owner, clock, "ppc405", 32)
//{
//}

ppc603_device::ppc603_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC603, "PowerPC 603", tag, owner, clock, "ppc603", 64, PPC_MODEL_603, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4)
{
}

ppc603e_device::ppc603e_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC603E, "PowerPC 603e", tag, owner, clock, "ppc603e", 64, PPC_MODEL_603E, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4)
{
}

ppc603r_device::ppc603r_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC603R, "PowerPC 603R", tag, owner, clock, "ppc603r", 64, PPC_MODEL_603R, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4)
{
}

ppc602_device::ppc602_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC602, "PowerPC 602", tag, owner, clock, "ppc602", 64, PPC_MODEL_602, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4)
{
}

mpc8240_device::mpc8240_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, MPC8240, "PowerPC MPC8240", tag, owner, clock, "mpc8240", 64, PPC_MODEL_MPC8240, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4/* unknown */)
{
}

ppc601_device::ppc601_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC601, "PowerPC 601", tag, owner, clock, "ppc601", 64, PPC_MODEL_601, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_MFIOC | PPCCAP_601BAT, 0/* no TB */)
{
}

ppc604_device::ppc604_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc_device(mconfig, PPC604, "PowerPC 604", tag, owner, clock, "ppc604", 64, PPC_MODEL_604, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_604_MMU, 4)
{
}

ppc4xx_device::ppc4xx_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, powerpc_flavor flavor, UINT32 cap, UINT32 tb_divisor)
	: ppc_device(mconfig, type, name, tag, owner, clock, shortname, 64, flavor, cap, tb_divisor) // TODO address bits, ppccom has 31 address bits??
{
}

ppc403ga_device::ppc403ga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc4xx_device(mconfig, PPC403GA, "PowerPC 403GA", tag, owner, clock, "ppc403ga", PPC_MODEL_403GA, PPCCAP_4XX, 1)
{
}

ppc403gcx_device::ppc403gcx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc4xx_device(mconfig, PPC403GCX, "PowerPC 403GCX", tag, owner, clock, "ppc403gcx", PPC_MODEL_403GCX, PPCCAP_4XX, 1)
{
}

ppc405gp_device::ppc405gp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ppc4xx_device(mconfig, PPC405GP, "PowerPC 405GP", tag, owner, clock, "ppc405gp", PPC_MODEL_405GP, PPCCAP_4XX | PPCCAP_VEA, 1)
{
}

/*********************************************************************/

inline int ppc_device::IS_PPC602(void)
{
	return m_is602;
}

inline int ppc_device::IS_PPC603(void)
{
	return m_is603;
}

inline int ppc_device::IS_PPC403(void)
{
	return !IS_PPC602() && !IS_PPC603();
}

/*********************************************************************/


INLINE void SET_CR0(INT32 rd)
{
	if( rd < 0 ) {
		CR(0) = 0x8;
	} else if( rd > 0 ) {
		CR(0) = 0x4;
	} else {
		CR(0) = 0x2;
	}

	if( XER & XER_SO )
		CR(0) |= 0x1;
}

INLINE void SET_CR1(void)
{
	CR(1) = (ppc.fpscr >> 28) & 0xf;
}

INLINE void SET_ADD_OV(UINT32 rd, UINT32 ra, UINT32 rb)
{
	if( ADD_OV(rd, ra, rb) )
		XER |= XER_SO | XER_OV;
	else
		XER &= ~XER_OV;
}

INLINE void SET_SUB_OV(UINT32 rd, UINT32 ra, UINT32 rb)
{
	if( SUB_OV(rd, ra, rb) )
		XER |= XER_SO | XER_OV;
	else
		XER &= ~XER_OV;
}

INLINE void SET_ADD_CA(UINT32 rd, UINT32 ra, UINT32 rb)
{
	if( ADD_CA(rd, ra, rb) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;
}

INLINE void SET_SUB_CA(UINT32 rd, UINT32 ra, UINT32 rb)
{
	if( SUB_CA(rd, ra, rb) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;
}

INLINE UINT32 check_condition_code(UINT32 bo, UINT32 bi)
{
	UINT32 ctr_ok;
	UINT32 condition_ok;
	UINT32 bo0 = (bo & 0x10) ? 1 : 0;
	UINT32 bo1 = (bo & 0x08) ? 1 : 0;
	UINT32 bo2 = (bo & 0x04) ? 1 : 0;
	UINT32 bo3 = (bo & 0x02) ? 1 : 0;

	if( bo2 == 0 )
		--CTR;

	ctr_ok = bo2 | ((CTR != 0) ^ bo3);
	condition_ok = bo0 | (CRBIT(bi) ^ (~bo1 & 0x1));

	return ctr_ok && condition_ok;
}

INLINE UINT64 ppc_read_timebase(void)
{
	int cycles = ppc_tb_base_icount - ppc_icount;

	if (IS_PPC603() || IS_PPC602())
	{
		// timebase is incremented once every four core clock cycles, so adjust the cycles accordingly
		return ppc.tb + (cycles / 4);
	}
	else
	{
		// timebase is incremented once every core cycle on PPC403
		return ppc.tb + cycles;
	}
}

INLINE void ppc_write_timebase_l(UINT32 tbl)
{
	ppc_tb_base_icount = ppc_icount;

	ppc.tb &= ~0xffffffff;
	ppc.tb |= tbl;
}

INLINE void ppc_write_timebase_h(UINT32 tbh)
{
	ppc_tb_base_icount = ppc_icount;

	ppc.tb &= 0xffffffff;
	ppc.tb |= (UINT64)(tbh) << 32;
}

INLINE UINT32 read_decrementer(void)
{
	int cycles = ppc_dec_base_icount - ppc_icount;

	// decrementer is decremented once every four bus clock cycles, so adjust the cycles accordingly
	return DEC - (cycles / (bus_freq_multiplier * 2));
}

INLINE void write_decrementer(UINT32 value)
{
	ppc_dec_base_icount = ppc_icount + (ppc_dec_base_icount - ppc_icount) % (bus_freq_multiplier * 2);

	DEC = value;

	// check if decrementer exception occurs during execution
	if ((UINT32)(DEC - ppc_icount) > (UINT32)(DEC))
	{
		ppc_dec_trigger_cycle = ppc_icount - DEC;
	}
	else
	{
		ppc_dec_trigger_cycle = 0x7fffffff;
	}
}

/*********************************************************************/

INLINE void ppc_set_spr(int spr, UINT32 value)
{
	switch (spr)
	{
		case SPR_LR:        LR = value; return;
		case SPR_CTR:       CTR = value; return;
		case SPR_XER:       XER = value; return;
		case SPR_SRR0:      ppc.srr0 = value; return;
		case SPR_SRR1:      ppc.srr1 = value; return;
		case SPR_SPRG0:     ppc.sprg[0] = value; return;
		case SPR_SPRG1:     ppc.sprg[1] = value; return;
		case SPR_SPRG2:     ppc.sprg[2] = value; return;
		case SPR_SPRG3:     ppc.sprg[3] = value; return;
		case SPR_PVR:       return;
	}

	if(IS_PPC602() || IS_PPC603()) {
		switch(spr)
		{
			case SPR603E_DEC:
				if((value & 0x80000000) && !(DEC & 0x80000000))
				{
					/* trigger interrupt */
					if (IS_PPC602())
						ppc602_exception(EXCEPTION_DECREMENTER);
					if (IS_PPC603())
						ppc603_exception(EXCEPTION_DECREMENTER);
				}
				write_decrementer(value);
				return;

			case SPR603E_TBL_W:
			case SPR603E_TBL_R: // special 603e case
				ppc_write_timebase_l(value);
				return;

			case SPR603E_TBU_R:
			case SPR603E_TBU_W: // special 603e case
				ppc_write_timebase_h(value);
				return;

			case SPR603E_HID0:          ppc.hid0 = value; return;
			case SPR603E_HID1:          ppc.hid1 = value; return;
			case SPR603E_HID2:          ppc.hid2 = value; return;

			case SPR603E_DSISR:         ppc.dsisr = value; return;
			case SPR603E_DAR:           ppc.dar = value; return;
			case SPR603E_EAR:           ppc.ear = value; return;
			case SPR603E_DMISS:         ppc.dmiss = value; return;
			case SPR603E_DCMP:          ppc.dcmp = value; return;
			case SPR603E_HASH1:         ppc.hash1 = value; return;
			case SPR603E_HASH2:         ppc.hash2 = value; return;
			case SPR603E_IMISS:         ppc.imiss = value; return;
			case SPR603E_ICMP:          ppc.icmp = value; return;
			case SPR603E_RPA:           ppc.rpa = value; return;

			case SPR603E_IBAT0L:        ppc.ibat[0].l = value; return;
			case SPR603E_IBAT0U:        ppc.ibat[0].u = value; return;
			case SPR603E_IBAT1L:        ppc.ibat[1].l = value; return;
			case SPR603E_IBAT1U:        ppc.ibat[1].u = value; return;
			case SPR603E_IBAT2L:        ppc.ibat[2].l = value; return;
			case SPR603E_IBAT2U:        ppc.ibat[2].u = value; return;
			case SPR603E_IBAT3L:        ppc.ibat[3].l = value; return;
			case SPR603E_IBAT3U:        ppc.ibat[3].u = value; return;
			case SPR603E_DBAT0L:        ppc.dbat[0].l = value; return;
			case SPR603E_DBAT0U:        ppc.dbat[0].u = value; return;
			case SPR603E_DBAT1L:        ppc.dbat[1].l = value; return;
			case SPR603E_DBAT1U:        ppc.dbat[1].u = value; return;
			case SPR603E_DBAT2L:        ppc.dbat[2].l = value; return;
			case SPR603E_DBAT2U:        ppc.dbat[2].u = value; return;
			case SPR603E_DBAT3L:        ppc.dbat[3].l = value; return;
			case SPR603E_DBAT3U:        ppc.dbat[3].u = value; return;

			case SPR603E_SDR1:
				ppc.sdr1 = value;
				return;

			case SPR603E_IABR:          ppc.iabr = value; return;
		}
	}

	if (ppc.is602) {
		switch(spr)
		{
			case SPR602_LT:         ppc.lt = value; return;
			case SPR602_IBR:        ppc.ibr = value; return;
			case SPR602_SEBR:       ppc.sebr = value; return;
			case SPR602_SER:        ppc.ser = value; return;
			case SPR602_SP:         ppc.sp = value; return;
			case SPR602_TCR:        ppc.tcr = value; return;
		}
	}

	if (IS_PPC403()) {
		switch(spr)
		{
			case SPR403_TBHI:       ppc_write_timebase_h(value); return;
			case SPR403_TBLO:       ppc_write_timebase_l(value); return;

			case SPR403_TSR:
				ppc.tsr &= ~value; // 1 clears, 0 does nothing
				return;

			case SPR403_TCR:
				switch((value >> 24) & 0x3)
				{
					case 0: ppc.fit_bit = 1 << 8; break;
					case 1: ppc.fit_bit = 1 << 12; break;
					case 2: ppc.fit_bit = 1 << 16; break;
					case 3: ppc.fit_bit = 1 << 20; break;
				}
				switch((value >> 30) & 0x3)
				{
					case 0: ppc.wdt_bit = 1 << 16; break;
					case 1: ppc.wdt_bit = 1 << 20; break;
					case 2: ppc.wdt_bit = 1 << 24; break;
					case 3: ppc.wdt_bit = 1 << 28; break;
				}
				ppc.fit_int_enable = (value >> 23) & 0x1;
				ppc.wdt_int_enable = (value >> 27) & 0x1;
				ppc.tcr = value;

				if (!ppc.fit_int_enable)
				{
					ppc.interrupt_pending &= ~0x4;
				}
				return;

			case SPR403_ESR:        ppc.esr = value; return;
			case SPR403_ICCR:       ppc.iccr = value; return;
			case SPR403_DCCR:       ppc.dccr = value; return;
			case SPR403_EVPR:       EVPR = value & 0xffff0000; return;
			case SPR403_PIT:        ppc.pit = value; return;
			case SPR403_SGR:        ppc.sgr = value; return;
			case SPR403_DBSR:       ppc.dbsr = value; return;
			case SPR403_DCWR:       return;
			case SPR403_PID:        ppc.pid = value; return;
			case SPR403_PBL1:       ppc.pbl1 = value; return;
			case SPR403_PBU1:       ppc.pbu1 = value; return;
			case SPR403_PBL2:       ppc.pbl2 = value; return;
			case SPR403_PBU2:       ppc.pbu2 = value; return;
			case SPR403_SRR2:       ppc.srr2 = value; return;
			case SPR403_SRR3:       ppc.srr3 = value; return;
			case SPR403_DAC1:       ppc.dac1 = value; return;
			case SPR403_DAC2:       ppc.dac2 = value; return;
			case SPR403_IAC1:       ppc.iac1 = value; return;
			case SPR403_IAC2:       ppc.iac2 = value; return;
		}
	}

	fatalerror("ppc: set_spr: unknown spr %d (%03X)!\n", spr, spr);
}

INLINE UINT32 ppc_get_spr(int spr)
{
	switch(spr)
	{
		case SPR_LR:        return LR;
		case SPR_CTR:       return CTR;
		case SPR_XER:       return XER;
		case SPR_SRR0:      return ppc.srr0;
		case SPR_SRR1:      return ppc.srr1;
		case SPR_SPRG0:     return ppc.sprg[0];
		case SPR_SPRG1:     return ppc.sprg[1];
		case SPR_SPRG2:     return ppc.sprg[2];
		case SPR_SPRG3:     return ppc.sprg[3];
		case SPR_PVR:       return ppc.pvr;
	}

	if (IS_PPC403())
	{
		switch (spr)
		{
			case SPR403_TBLU:
			case SPR403_TBLO:       return (UINT32)(ppc_read_timebase());
			case SPR403_TBHU:
			case SPR403_TBHI:       return (UINT32)(ppc_read_timebase() >> 32);

			case SPR403_EVPR:       return EVPR;
			case SPR403_ESR:        return ppc.esr;
			case SPR403_TCR:        return ppc.tcr;
			case SPR403_ICCR:       return ppc.iccr;
			case SPR403_DCCR:       return ppc.dccr;
			case SPR403_PIT:        return ppc.pit;
			case SPR403_DBSR:       return ppc.dbsr;
			case SPR403_SGR:        return ppc.sgr;
			case SPR403_TSR:        return ppc.tsr;
			case SPR403_PBL1:       return ppc.pbl1;
			case SPR403_PBU1:       return ppc.pbu1;
			case SPR403_PBL2:       return ppc.pbl2;
			case SPR403_PBU2:       return ppc.pbu2;
			case SPR403_SRR2:       return ppc.srr2;
			case SPR403_SRR3:       return ppc.srr3;
			case SPR403_DAC1:       return ppc.dac1;
			case SPR403_DAC2:       return ppc.dac2;
			case SPR403_IAC1:       return ppc.iac1;
			case SPR403_IAC2:       return ppc.iac2;
		}
	}

	if (ppc.is602) {
		switch(spr)
		{
			case SPR602_LT:         return ppc.lt;
			case SPR602_IBR:        return ppc.ibr;
			case SPR602_ESASRR:     return ppc.esasrr;
			case SPR602_SEBR:       return ppc.sebr;
			case SPR602_SER:        return ppc.ser;
			case SPR602_SP:         return ppc.sp;
			case SPR602_TCR:        return ppc.tcr;
		}
	}

	if (IS_PPC603() || IS_PPC602())
	{
		switch (spr)
		{
			case SPR603E_TBL_R:
				fatalerror("ppc: get_spr: TBL_R\n");
				break;

			case SPR603E_TBU_R:
				fatalerror("ppc: get_spr: TBU_R\n");
				break;

			case SPR603E_TBL_W:     return (UINT32)(ppc_read_timebase());
			case SPR603E_TBU_W:     return (UINT32)(ppc_read_timebase() >> 32);
			case SPR603E_HID0:      return ppc.hid0;
			case SPR603E_HID1:      return ppc.hid1;
			case SPR603E_HID2:      return ppc.hid2;
			case SPR603E_DEC:       return read_decrementer();
			case SPR603E_SDR1:      return ppc.sdr1;
			case SPR603E_DSISR:     return ppc.dsisr;
			case SPR603E_DAR:       return ppc.dar;
			case SPR603E_EAR:       return ppc.ear;
			case SPR603E_DMISS:     return ppc.dmiss;
			case SPR603E_DCMP:      return ppc.dcmp;
			case SPR603E_HASH1:     return ppc.hash1;
			case SPR603E_HASH2:     return ppc.hash2;
			case SPR603E_IMISS:     return ppc.imiss;
			case SPR603E_ICMP:      return ppc.icmp;
			case SPR603E_RPA:       return ppc.rpa;
			case SPR603E_IBAT0L:    return ppc.ibat[0].l;
			case SPR603E_IBAT0U:    return ppc.ibat[0].u;
			case SPR603E_IBAT1L:    return ppc.ibat[1].l;
			case SPR603E_IBAT1U:    return ppc.ibat[1].u;
			case SPR603E_IBAT2L:    return ppc.ibat[2].l;
			case SPR603E_IBAT2U:    return ppc.ibat[2].u;
			case SPR603E_IBAT3L:    return ppc.ibat[3].l;
			case SPR603E_IBAT3U:    return ppc.ibat[3].u;
			case SPR603E_DBAT0L:    return ppc.dbat[0].l;
			case SPR603E_DBAT0U:    return ppc.dbat[0].u;
			case SPR603E_DBAT1L:    return ppc.dbat[1].l;
			case SPR603E_DBAT1U:    return ppc.dbat[1].u;
			case SPR603E_DBAT2L:    return ppc.dbat[2].l;
			case SPR603E_DBAT2U:    return ppc.dbat[2].u;
			case SPR603E_DBAT3L:    return ppc.dbat[3].l;
			case SPR603E_DBAT3U:    return ppc.dbat[3].u;
		}
	}

	fatalerror("ppc: get_spr: unknown spr %d (%03X)!\n", spr, spr);
	return 0;
}

static UINT8 ppc_read8_translated(address_space &space, offs_t address);
static UINT16 ppc_read16_translated(address_space &space, offs_t address);
static UINT32 ppc_read32_translated(address_space &space, offs_t address);
static UINT64 ppc_read64_translated(address_space &space, offs_t address);
static void ppc_write8_translated(address_space &space, offs_t address, UINT8 data);
static void ppc_write16_translated(address_space &space, offs_t address, UINT16 data);
static void ppc_write32_translated(address_space &space, offs_t address, UINT32 data);
static void ppc_write64_translated(address_space &space, offs_t address, UINT64 data);

INLINE void ppc_set_msr(UINT32 value)
{
	if( value & (MSR_ILE | MSR_LE) )
		fatalerror("ppc: set_msr: little_endian mode not supported!\n");

	MSR = value;

	if (IS_PPC603() || IS_PPC602())
	{
		if (!(MSR & MSR_DR))
		{
			ppc.read8 = memory_read_byte_64be;
			ppc.read16 = memory_read_word_64be;
			ppc.read32 = memory_read_dword_64be;
			ppc.read64 = memory_read_qword_64be;
			ppc.write8 = memory_write_byte_64be;
			ppc.write16 = memory_write_word_64be;
			ppc.write32 = memory_write_dword_64be;
			ppc.write64 = memory_write_qword_64be;
		}
		else
		{
			ppc.read8 = ppc_read8_translated;
			ppc.read16 = ppc_read16_translated;
			ppc.read32 = ppc_read32_translated;
			ppc.read64 = ppc_read64_translated;
			ppc.write8 = ppc_write8_translated;
			ppc.write16 = ppc_write16_translated;
			ppc.write32 = ppc_write32_translated;
			ppc.write64 = ppc_write64_translated;
		}
	}
}

INLINE UINT32 ppc_get_msr(void)
{
	return MSR;
}

INLINE void ppc_set_cr(UINT32 value)
{
	CR(0) = (value >> 28) & 0xf;
	CR(1) = (value >> 24) & 0xf;
	CR(2) = (value >> 20) & 0xf;
	CR(3) = (value >> 16) & 0xf;
	CR(4) = (value >> 12) & 0xf;
	CR(5) = (value >> 8) & 0xf;
	CR(6) = (value >> 4) & 0xf;
	CR(7) = (value >> 0) & 0xf;
}

INLINE UINT32 ppc_get_cr(void)
{
	return CR(0) << 28 | CR(1) << 24 | CR(2) << 20 | CR(3) << 16 | CR(4) << 12 | CR(5) << 8 | CR(6) << 4 | CR(7);
}

INLINE void ppc_exception(int exception_type)
{
	longjmp(ppc.exception_jmpbuf, exception_type);
}

/***********************************************************************/

#include "ppc_mem.inc"

#include "ppc403.inc"
#include "ppc602.inc"
#include "ppc603.inc"

/********************************************************************/

#include "ppc_ops.inc"
#include "ppc_ops.h"

/* Initialization and shutdown */

static void ppc_init(void)
{
	int i,j;

	memset(&ppc, 0, sizeof(ppc));

	for( i=0; i < 64; i++ ) {
		ppc.optable[i] = ppc_invalid;
	}
	for( i=0; i < 1024; i++ ) {
		ppc.optable19[i] = ppc_invalid;
		ppc.optable31[i] = ppc_invalid;
		ppc.optable59[i] = ppc_invalid;
		ppc.optable63[i] = ppc_invalid;
	}

	/* Fill the opcode tables */
	for( i=0; i < (sizeof(ppc_opcode_common) / sizeof(PPC_OPCODE)); i++ ) {
		switch(ppc_opcode_common[i].code)
		{
			case 19:
				ppc.optable19[ppc_opcode_common[i].subcode] = ppc_opcode_common[i].handler;
				break;

			case 31:
				ppc.optable31[ppc_opcode_common[i].subcode] = ppc_opcode_common[i].handler;
				break;

			case 59:
			case 63:
				break;

			default:
				ppc.optable[ppc_opcode_common[i].code] = ppc_opcode_common[i].handler;
		}

	}

	/* Calculate rotate mask table */
	for( i=0; i < 32; i++ ) {
		for( j=0; j < 32; j++ ) {
			UINT32 mask;
			int mb = i;
			int me = j;
			mask = ((UINT32)0xFFFFFFFF >> mb) ^ ((me >= 31) ? 0 : ((UINT32)0xFFFFFFFF >> (me + 1)));
			if( mb > me )
				mask = ~mask;

			ppc_rotate_mask[i][j] = mask;
		}
	}
}


// !!! probably should move this stuff elsewhere !!!
static CPU_INIT( ppc403 )
{
	const ppc_config *configdata = device->static_config();

	ppc_init();

	/* PPC403 specific opcodes */
	ppc.optable31[454] = ppc_dccci;
	ppc.optable31[486] = ppc_dcread;
	ppc.optable31[262] = ppc_icbt;
	ppc.optable31[966] = ppc_iccci;
	ppc.optable31[998] = ppc_icread;
	ppc.optable31[323] = ppc_mfdcr;
	ppc.optable31[451] = ppc_mtdcr;
	ppc.optable31[131] = ppc_wrtee;
	ppc.optable31[163] = ppc_wrteei;

	// !!! why is rfci here !!!
	ppc.optable19[51] = ppc_rfci;

	ppc.spu.rx_timer = device->machine().scheduler().timer_alloc(FUNC(ppc403_spu_rx_callback));
	ppc.spu.tx_timer = device->machine().scheduler().timer_alloc(FUNC(ppc403_spu_tx_callback));

	ppc.read8 = ppc403_read8;
	ppc.read16 = ppc403_read16;
	ppc.read32 = ppc403_read32;
	ppc.write8 = ppc403_write8;
	ppc.write16 = ppc403_write16;
	ppc.write32 = ppc403_write32;
	ppc.read16_unaligned = ppc403_read16_unaligned;
	ppc.read32_unaligned = ppc403_read32_unaligned;
	ppc.write16_unaligned = ppc403_write16_unaligned;
	ppc.write32_unaligned = ppc403_write32_unaligned;

	ppc.irq_callback = irqcallback;
	ppc.device = device;
	ppc.program = &device->space(AS_PROGRAM);

	ppc.pvr = configdata->pvr;
}

static CPU_EXIT( ppc403 )
{
}

static CPU_INIT( ppc405 )
{
	const ppc_config *configdata = device->static_config();

	ppc_init();

	/* PPC403 specific opcodes */
	ppc.optable31[454] = ppc_dccci;
	ppc.optable31[486] = ppc_dcread;
	ppc.optable31[262] = ppc_icbt;
	ppc.optable31[966] = ppc_iccci;
	ppc.optable31[998] = ppc_icread;
	ppc.optable31[323] = ppc_mfdcr;
	ppc.optable31[451] = ppc_mtdcr;
	ppc.optable31[131] = ppc_wrtee;
	ppc.optable31[163] = ppc_wrteei;

	// !!! why is rfci here !!!
	ppc.optable19[51] = ppc_rfci;

	ppc.spu.rx_timer = device->machine().scheduler().timer_alloc(FUNC(ppc403_spu_rx_callback));
	ppc.spu.tx_timer = device->machine().scheduler().timer_alloc(FUNC(ppc403_spu_tx_callback));

	ppc.read8 = ppc403_read8;
	ppc.read16 = ppc403_read16;
	ppc.read32 = ppc403_read32;
	ppc.write8 = ppc403_write8;
	ppc.write16 = ppc403_write16;
	ppc.write32 = ppc403_write32;
	ppc.read16_unaligned = ppc403_read16_unaligned;
	ppc.read32_unaligned = ppc403_read32_unaligned;
	ppc.write16_unaligned = ppc403_write16_unaligned;
	ppc.write32_unaligned = ppc403_write32_unaligned;

	ppc.irq_callback = irqcallback;
	ppc.device = device;
	ppc.program = &device->space(AS_PROGRAM);

	ppc.pvr = configdata->pvr;
}

static CPU_EXIT( ppc405 )
{
}

static CPU_INIT( ppc603 )
{
	const ppc_config *configdata = device->static_config();
	int pll_config = 0;
	float multiplier;
	int i ;

	ppc_init() ;

	ppc.optable[48] = ppc_lfs;
	ppc.optable[49] = ppc_lfsu;
	ppc.optable[50] = ppc_lfd;
	ppc.optable[51] = ppc_lfdu;
	ppc.optable[52] = ppc_stfs;
	ppc.optable[53] = ppc_stfsu;
	ppc.optable[54] = ppc_stfd;
	ppc.optable[55] = ppc_stfdu;
	ppc.optable31[631] = ppc_lfdux;
	ppc.optable31[599] = ppc_lfdx;
	ppc.optable31[567] = ppc_lfsux;
	ppc.optable31[535] = ppc_lfsx;
	ppc.optable31[595] = ppc_mfsr;
	ppc.optable31[659] = ppc_mfsrin;
	ppc.optable31[371] = ppc_mftb;
	ppc.optable31[210] = ppc_mtsr;
	ppc.optable31[242] = ppc_mtsrin;
	ppc.optable31[758] = ppc_dcba;
	ppc.optable31[759] = ppc_stfdux;
	ppc.optable31[727] = ppc_stfdx;
	ppc.optable31[983] = ppc_stfiwx;
	ppc.optable31[695] = ppc_stfsux;
	ppc.optable31[663] = ppc_stfsx;
	ppc.optable31[370] = ppc_tlbia;
	ppc.optable31[306] = ppc_tlbie;
	ppc.optable31[566] = ppc_tlbsync;
	ppc.optable31[310] = ppc_eciwx;
	ppc.optable31[438] = ppc_ecowx;

	ppc.optable63[264] = ppc_fabsx;
	ppc.optable63[21] = ppc_faddx;
	ppc.optable63[32] = ppc_fcmpo;
	ppc.optable63[0] = ppc_fcmpu;
	ppc.optable63[14] = ppc_fctiwx;
	ppc.optable63[15] = ppc_fctiwzx;
	ppc.optable63[18] = ppc_fdivx;
	ppc.optable63[72] = ppc_fmrx;
	ppc.optable63[136] = ppc_fnabsx;
	ppc.optable63[40] = ppc_fnegx;
	ppc.optable63[12] = ppc_frspx;
	ppc.optable63[26] = ppc_frsqrtex;
	ppc.optable63[22] = ppc_fsqrtx;
	ppc.optable63[20] = ppc_fsubx;
	ppc.optable63[583] = ppc_mffsx;
	ppc.optable63[70] = ppc_mtfsb0x;
	ppc.optable63[38] = ppc_mtfsb1x;
	ppc.optable63[711] = ppc_mtfsfx;
	ppc.optable63[134] = ppc_mtfsfix;
	ppc.optable63[64] = ppc_mcrfs;

	ppc.optable59[21] = ppc_faddsx;
	ppc.optable59[18] = ppc_fdivsx;
	ppc.optable59[24] = ppc_fresx;
	ppc.optable59[22] = ppc_fsqrtsx;
	ppc.optable59[20] = ppc_fsubsx;

	for(i = 0; i < 32; i++)
	{
		ppc.optable63[i * 32 | 29] = ppc_fmaddx;
		ppc.optable63[i * 32 | 28] = ppc_fmsubx;
		ppc.optable63[i * 32 | 25] = ppc_fmulx;
		ppc.optable63[i * 32 | 31] = ppc_fnmaddx;
		ppc.optable63[i * 32 | 30] = ppc_fnmsubx;
		ppc.optable63[i * 32 | 23] = ppc_fselx;

		ppc.optable59[i * 32 | 29] = ppc_fmaddsx;
		ppc.optable59[i * 32 | 28] = ppc_fmsubsx;
		ppc.optable59[i * 32 | 25] = ppc_fmulsx;
		ppc.optable59[i * 32 | 31] = ppc_fnmaddsx;
		ppc.optable59[i * 32 | 30] = ppc_fnmsubsx;
	}

	ppc.optable31[978] = ppc_tlbld;

	for(i = 0; i < 256; i++)
	{
		ppc_field_xlat[i] =
			((i & 0x80) ? 0xF0000000 : 0) |
			((i & 0x40) ? 0x0F000000 : 0) |
			((i & 0x20) ? 0x00F00000 : 0) |
			((i & 0x10) ? 0x000F0000 : 0) |
			((i & 0x08) ? 0x0000F000 : 0) |
			((i & 0x04) ? 0x00000F00 : 0) |
			((i & 0x02) ? 0x000000F0 : 0) |
			((i & 0x01) ? 0x0000000F : 0);
	}

	ppc.is603 = 1;

	ppc.read8 = memory_read_byte_64be;
	ppc.read16 = memory_read_word_64be;
	ppc.read32 = memory_read_dword_64be;
	ppc.read64 = memory_read_qword_64be;
	ppc.write8 = memory_write_byte_64be;
	ppc.write16 = memory_write_word_64be;
	ppc.write32 = memory_write_dword_64be;
	ppc.write64 = memory_write_qword_64be;
	ppc.read16_unaligned = ppc_read16_unaligned;
	ppc.read32_unaligned = ppc_read32_unaligned;
	ppc.read64_unaligned = ppc_read64_unaligned;
	ppc.write16_unaligned = ppc_write16_unaligned;
	ppc.write32_unaligned = ppc_write32_unaligned;
	ppc.write64_unaligned = ppc_write64_unaligned;

	ppc.irq_callback = irqcallback;
	ppc.device = device;
	ppc.program = &device->space(AS_PROGRAM);

	ppc.pvr = configdata->pvr;

	multiplier = (float)((configdata->bus_frequency_multiplier >> 4) & 0xf) +
					(float)(configdata->bus_frequency_multiplier & 0xf) / 10.0f;
	bus_freq_multiplier = (int)(multiplier * 2);

	switch(config->pvr)
	{
		case PPC_MODEL_603E:    pll_config = mpc603e_pll_config[bus_freq_multiplier-1][configdata->bus_frequency]; break;
		case PPC_MODEL_603EV:   pll_config = mpc603ev_pll_config[bus_freq_multiplier-1][configdata->bus_frequency]; break;
		case PPC_MODEL_603R:    pll_config = mpc603r_pll_config[bus_freq_multiplier-1][configdata->bus_frequency]; break;
		default: break;
	}

	if (pll_config == -1)
	{
		fatalerror("PPC: Invalid bus/multiplier combination (bus frequency = %d, multiplier = %1.1f)\n", config->bus_frequency, multiplier);
	}

	ppc.hid1 = pll_config << 28;
}

static CPU_EXIT( ppc603 )
{
}

static CPU_INIT( ppc602 )
{
	float multiplier;
	const ppc_config *configdata = device->static_config();

	int i ;

	ppc_init() ;

	ppc.optable[48] = ppc_lfs;
	ppc.optable[49] = ppc_lfsu;
	ppc.optable[50] = ppc_lfd;
	ppc.optable[51] = ppc_lfdu;
	ppc.optable[52] = ppc_stfs;
	ppc.optable[53] = ppc_stfsu;
	ppc.optable[54] = ppc_stfd;
	ppc.optable[55] = ppc_stfdu;
	ppc.optable31[631] = ppc_lfdux;
	ppc.optable31[599] = ppc_lfdx;
	ppc.optable31[567] = ppc_lfsux;
	ppc.optable31[535] = ppc_lfsx;
	ppc.optable31[595] = ppc_mfsr;
	ppc.optable31[659] = ppc_mfsrin;
	ppc.optable31[371] = ppc_mftb;
	ppc.optable31[210] = ppc_mtsr;
	ppc.optable31[242] = ppc_mtsrin;
	ppc.optable31[758] = ppc_dcba;
	ppc.optable31[759] = ppc_stfdux;
	ppc.optable31[727] = ppc_stfdx;
	ppc.optable31[983] = ppc_stfiwx;
	ppc.optable31[695] = ppc_stfsux;
	ppc.optable31[663] = ppc_stfsx;
	ppc.optable31[370] = ppc_tlbia;
	ppc.optable31[306] = ppc_tlbie;
	ppc.optable31[566] = ppc_tlbsync;
	ppc.optable31[310] = ppc_eciwx;
	ppc.optable31[438] = ppc_ecowx;

	ppc.optable63[264] = ppc_fabsx;
	ppc.optable63[21] = ppc_faddx;
	ppc.optable63[32] = ppc_fcmpo;
	ppc.optable63[0] = ppc_fcmpu;
	ppc.optable63[14] = ppc_fctiwx;
	ppc.optable63[15] = ppc_fctiwzx;
	ppc.optable63[18] = ppc_fdivx;
	ppc.optable63[72] = ppc_fmrx;
	ppc.optable63[136] = ppc_fnabsx;
	ppc.optable63[40] = ppc_fnegx;
	ppc.optable63[12] = ppc_frspx;
	ppc.optable63[26] = ppc_frsqrtex;
	ppc.optable63[22] = ppc_fsqrtx;
	ppc.optable63[20] = ppc_fsubx;
	ppc.optable63[583] = ppc_mffsx;
	ppc.optable63[70] = ppc_mtfsb0x;
	ppc.optable63[38] = ppc_mtfsb1x;
	ppc.optable63[711] = ppc_mtfsfx;
	ppc.optable63[134] = ppc_mtfsfix;
	ppc.optable63[64] = ppc_mcrfs;

	ppc.optable59[21] = ppc_faddsx;
	ppc.optable59[18] = ppc_fdivsx;
	ppc.optable59[24] = ppc_fresx;
	ppc.optable59[22] = ppc_fsqrtsx;
	ppc.optable59[20] = ppc_fsubsx;

	for(i = 0; i < 32; i++)
	{
		ppc.optable63[i * 32 | 29] = ppc_fmaddx;
		ppc.optable63[i * 32 | 28] = ppc_fmsubx;
		ppc.optable63[i * 32 | 25] = ppc_fmulx;
		ppc.optable63[i * 32 | 31] = ppc_fnmaddx;
		ppc.optable63[i * 32 | 30] = ppc_fnmsubx;
		ppc.optable63[i * 32 | 23] = ppc_fselx;

		ppc.optable59[i * 32 | 29] = ppc_fmaddsx;
		ppc.optable59[i * 32 | 28] = ppc_fmsubsx;
		ppc.optable59[i * 32 | 25] = ppc_fmulsx;
		ppc.optable59[i * 32 | 31] = ppc_fnmaddsx;
		ppc.optable59[i * 32 | 30] = ppc_fnmsubsx;
	}

	for(i = 0; i < 256; i++)
	{
		ppc_field_xlat[i] =
			((i & 0x80) ? 0xF0000000 : 0) |
			((i & 0x40) ? 0x0F000000 : 0) |
			((i & 0x20) ? 0x00F00000 : 0) |
			((i & 0x10) ? 0x000F0000 : 0) |
			((i & 0x08) ? 0x0000F000 : 0) |
			((i & 0x04) ? 0x00000F00 : 0) |
			((i & 0x02) ? 0x000000F0 : 0) |
			((i & 0x01) ? 0x0000000F : 0);
	}

	// PPC602 specific opcodes
	ppc.optable31[596] = ppc_esa;
	ppc.optable31[628] = ppc_dsa;
	ppc.optable31[1010] = ppc_tlbli;
	ppc.optable31[978] = ppc_tlbld;

	ppc.is602 = 1;

	ppc.read8 = memory_read_byte_64be;
	ppc.read16 = memory_read_word_64be;
	ppc.read32 = memory_read_dword_64be;
	ppc.read64 = memory_read_qword_64be;
	ppc.write8 = memory_write_byte_64be;
	ppc.write16 = memory_write_word_64be;
	ppc.write32 = memory_write_dword_64be;
	ppc.write64 = memory_write_qword_64be;
	ppc.read16_unaligned = ppc_read16_unaligned;
	ppc.read32_unaligned = ppc_read32_unaligned;
	ppc.read64_unaligned = ppc_read64_unaligned;
	ppc.write16_unaligned = ppc_write16_unaligned;
	ppc.write32_unaligned = ppc_write32_unaligned;
	ppc.write64_unaligned = ppc_write64_unaligned;

	ppc.irq_callback = irqcallback;
	ppc.device = device;
	ppc.program = &device->space(AS_PROGRAM);

	ppc.pvr = configdata->pvr;

	multiplier = (float)((configdata->bus_frequency_multiplier >> 4) & 0xf) +
					(float)(configdata->bus_frequency_multiplier & 0xf) / 10.0f;
	bus_freq_multiplier = (int)(multiplier * 2);
}

static CPU_EXIT( ppc602 )
{
}

static void mpc8240_tlbli(UINT32 op)
{
}

static void mpc8240_tlbld(UINT32 op)
{
}

static CPU_INIT( mpc8240 )
{
	float multiplier;
	const ppc_config *configdata = device->static_config();

	int i ;

	ppc_init();

	ppc.optable[48] = ppc_lfs;
	ppc.optable[49] = ppc_lfsu;
	ppc.optable[50] = ppc_lfd;
	ppc.optable[51] = ppc_lfdu;
	ppc.optable[52] = ppc_stfs;
	ppc.optable[53] = ppc_stfsu;
	ppc.optable[54] = ppc_stfd;
	ppc.optable[55] = ppc_stfdu;
	ppc.optable31[631] = ppc_lfdux;
	ppc.optable31[599] = ppc_lfdx;
	ppc.optable31[567] = ppc_lfsux;
	ppc.optable31[535] = ppc_lfsx;
	ppc.optable31[595] = ppc_mfsr;
	ppc.optable31[659] = ppc_mfsrin;
	ppc.optable31[371] = ppc_mftb;
	ppc.optable31[210] = ppc_mtsr;
	ppc.optable31[242] = ppc_mtsrin;
	ppc.optable31[758] = ppc_dcba;
	ppc.optable31[759] = ppc_stfdux;
	ppc.optable31[727] = ppc_stfdx;
	ppc.optable31[983] = ppc_stfiwx;
	ppc.optable31[695] = ppc_stfsux;
	ppc.optable31[663] = ppc_stfsx;
	ppc.optable31[370] = ppc_tlbia;
	ppc.optable31[306] = ppc_tlbie;
	ppc.optable31[566] = ppc_tlbsync;
	ppc.optable31[310] = ppc_eciwx;
	ppc.optable31[438] = ppc_ecowx;

	ppc.optable63[264] = ppc_fabsx;
	ppc.optable63[21] = ppc_faddx;
	ppc.optable63[32] = ppc_fcmpo;
	ppc.optable63[0] = ppc_fcmpu;
	ppc.optable63[14] = ppc_fctiwx;
	ppc.optable63[15] = ppc_fctiwzx;
	ppc.optable63[18] = ppc_fdivx;
	ppc.optable63[72] = ppc_fmrx;
	ppc.optable63[136] = ppc_fnabsx;
	ppc.optable63[40] = ppc_fnegx;
	ppc.optable63[12] = ppc_frspx;
	ppc.optable63[26] = ppc_frsqrtex;
	ppc.optable63[22] = ppc_fsqrtx;
	ppc.optable63[20] = ppc_fsubx;
	ppc.optable63[583] = ppc_mffsx;
	ppc.optable63[70] = ppc_mtfsb0x;
	ppc.optable63[38] = ppc_mtfsb1x;
	ppc.optable63[711] = ppc_mtfsfx;
	ppc.optable63[134] = ppc_mtfsfix;
	ppc.optable63[64] = ppc_mcrfs;

	ppc.optable59[21] = ppc_faddsx;
	ppc.optable59[18] = ppc_fdivsx;
	ppc.optable59[24] = ppc_fresx;
	ppc.optable59[22] = ppc_fsqrtsx;
	ppc.optable59[20] = ppc_fsubsx;

	for(i = 0; i < 32; i++)
	{
		ppc.optable63[i * 32 | 29] = ppc_fmaddx;
		ppc.optable63[i * 32 | 28] = ppc_fmsubx;
		ppc.optable63[i * 32 | 25] = ppc_fmulx;
		ppc.optable63[i * 32 | 31] = ppc_fnmaddx;
		ppc.optable63[i * 32 | 30] = ppc_fnmsubx;
		ppc.optable63[i * 32 | 23] = ppc_fselx;

		ppc.optable59[i * 32 | 29] = ppc_fmaddsx;
		ppc.optable59[i * 32 | 28] = ppc_fmsubsx;
		ppc.optable59[i * 32 | 25] = ppc_fmulsx;
		ppc.optable59[i * 32 | 31] = ppc_fnmaddsx;
		ppc.optable59[i * 32 | 30] = ppc_fnmsubsx;
	}

	for(i = 0; i < 256; i++)
	{
		ppc_field_xlat[i] =
			((i & 0x80) ? 0xF0000000 : 0) |
			((i & 0x40) ? 0x0F000000 : 0) |
			((i & 0x20) ? 0x00F00000 : 0) |
			((i & 0x10) ? 0x000F0000 : 0) |
			((i & 0x08) ? 0x0000F000 : 0) |
			((i & 0x04) ? 0x00000F00 : 0) |
			((i & 0x02) ? 0x000000F0 : 0) |
			((i & 0x01) ? 0x0000000F : 0);
	}

	// MPC8240 specific opcodes
	ppc.optable31[978] = mpc8240_tlbld;
	ppc.optable31[1010] = mpc8240_tlbli;

	ppc.is603 = 1;

	ppc.read8 = memory_read_byte_64be;
	ppc.read16 = memory_read_word_64be;
	ppc.read32 = memory_read_dword_64be;
	ppc.read64 = memory_read_qword_64be;
	ppc.write8 = memory_write_byte_64be;
	ppc.write16 = memory_write_word_64be;
	ppc.write32 = memory_write_dword_64be;
	ppc.write64 = memory_write_qword_64be;
	ppc.read16_unaligned = ppc_read16_unaligned;
	ppc.read32_unaligned = ppc_read32_unaligned;
	ppc.read64_unaligned = ppc_read64_unaligned;
	ppc.write16_unaligned = ppc_write16_unaligned;
	ppc.write32_unaligned = ppc_write32_unaligned;
	ppc.write64_unaligned = ppc_write64_unaligned;

	ppc.irq_callback = irqcallback;
	ppc.device = device;
	ppc.program = &device->space(AS_PROGRAM);

	ppc.pvr = configdata->pvr;

	multiplier = (float)((configdata->bus_frequency_multiplier >> 4) & 0xf) +
					(float)(configdata->bus_frequency_multiplier & 0xf) / 10.0f;
	bus_freq_multiplier = (int)(multiplier * 2);
}

static CPU_EXIT( mpc8240 )
{
}

static CPU_INIT( ppc601 )
{
	const ppc_config *configdata = device->static_config();
	float multiplier;
	int i ;

	ppc_init() ;

	ppc.optable[48] = ppc_lfs;
	ppc.optable[49] = ppc_lfsu;
	ppc.optable[50] = ppc_lfd;
	ppc.optable[51] = ppc_lfdu;
	ppc.optable[52] = ppc_stfs;
	ppc.optable[53] = ppc_stfsu;
	ppc.optable[54] = ppc_stfd;
	ppc.optable[55] = ppc_stfdu;
	ppc.optable31[631] = ppc_lfdux;
	ppc.optable31[599] = ppc_lfdx;
	ppc.optable31[567] = ppc_lfsux;
	ppc.optable31[535] = ppc_lfsx;
	ppc.optable31[595] = ppc_mfsr;
	ppc.optable31[659] = ppc_mfsrin;
	ppc.optable31[371] = ppc_mftb;
	ppc.optable31[210] = ppc_mtsr;
	ppc.optable31[242] = ppc_mtsrin;
	ppc.optable31[758] = ppc_dcba;
	ppc.optable31[759] = ppc_stfdux;
	ppc.optable31[727] = ppc_stfdx;
	ppc.optable31[983] = ppc_stfiwx;
	ppc.optable31[695] = ppc_stfsux;
	ppc.optable31[663] = ppc_stfsx;
	ppc.optable31[370] = ppc_tlbia;
	ppc.optable31[306] = ppc_tlbie;
	ppc.optable31[566] = ppc_tlbsync;
	ppc.optable31[310] = ppc_eciwx;
	ppc.optable31[438] = ppc_ecowx;

	ppc.optable63[264] = ppc_fabsx;
	ppc.optable63[21] = ppc_faddx;
	ppc.optable63[32] = ppc_fcmpo;
	ppc.optable63[0] = ppc_fcmpu;
	ppc.optable63[14] = ppc_fctiwx;
	ppc.optable63[15] = ppc_fctiwzx;
	ppc.optable63[18] = ppc_fdivx;
	ppc.optable63[72] = ppc_fmrx;
	ppc.optable63[136] = ppc_fnabsx;
	ppc.optable63[40] = ppc_fnegx;
	ppc.optable63[12] = ppc_frspx;
	ppc.optable63[26] = ppc_frsqrtex;
	ppc.optable63[22] = ppc_fsqrtx;
	ppc.optable63[20] = ppc_fsubx;
	ppc.optable63[583] = ppc_mffsx;
	ppc.optable63[70] = ppc_mtfsb0x;
	ppc.optable63[38] = ppc_mtfsb1x;
	ppc.optable63[711] = ppc_mtfsfx;
	ppc.optable63[134] = ppc_mtfsfix;
	ppc.optable63[64] = ppc_mcrfs;

	ppc.optable59[21] = ppc_faddsx;
	ppc.optable59[18] = ppc_fdivsx;
	ppc.optable59[24] = ppc_fresx;
	ppc.optable59[22] = ppc_fsqrtsx;
	ppc.optable59[20] = ppc_fsubsx;

	for(i = 0; i < 32; i++)
	{
		ppc.optable63[i * 32 | 29] = ppc_fmaddx;
		ppc.optable63[i * 32 | 28] = ppc_fmsubx;
		ppc.optable63[i * 32 | 25] = ppc_fmulx;
		ppc.optable63[i * 32 | 31] = ppc_fnmaddx;
		ppc.optable63[i * 32 | 30] = ppc_fnmsubx;
		ppc.optable63[i * 32 | 23] = ppc_fselx;

		ppc.optable59[i * 32 | 29] = ppc_fmaddsx;
		ppc.optable59[i * 32 | 28] = ppc_fmsubsx;
		ppc.optable59[i * 32 | 25] = ppc_fmulsx;
		ppc.optable59[i * 32 | 31] = ppc_fnmaddsx;
		ppc.optable59[i * 32 | 30] = ppc_fnmsubsx;
	}

	for(i = 0; i < 256; i++)
	{
		ppc_field_xlat[i] =
			((i & 0x80) ? 0xF0000000 : 0) |
			((i & 0x40) ? 0x0F000000 : 0) |
			((i & 0x20) ? 0x00F00000 : 0) |
			((i & 0x10) ? 0x000F0000 : 0) |
			((i & 0x08) ? 0x0000F000 : 0) |
			((i & 0x04) ? 0x00000F00 : 0) |
			((i & 0x02) ? 0x000000F0 : 0) |
			((i & 0x01) ? 0x0000000F : 0);
	}

	ppc.is603 = 1;

	ppc.read8 = memory_read_byte_64be;
	ppc.read16 = memory_read_word_64be;
	ppc.read32 = memory_read_dword_64be;
	ppc.read64 = memory_read_qword_64be;
	ppc.write8 = memory_write_byte_64be;
	ppc.write16 = memory_write_word_64be;
	ppc.write32 = memory_write_dword_64be;
	ppc.write64 = memory_write_qword_64be;
	ppc.read16_unaligned = ppc_read16_unaligned;
	ppc.read32_unaligned = ppc_read32_unaligned;
	ppc.read64_unaligned = ppc_read64_unaligned;
	ppc.write16_unaligned = ppc_write16_unaligned;
	ppc.write32_unaligned = ppc_write32_unaligned;
	ppc.write64_unaligned = ppc_write64_unaligned;

	ppc.irq_callback = irqcallback;
	ppc.device = device;
	ppc.program = &device->space(AS_PROGRAM);

	ppc.pvr = configdata->pvr;

	multiplier = (float)((configdata->bus_frequency_multiplier >> 4) & 0xf) +
					(float)(configdata->bus_frequency_multiplier & 0xf) / 10.0f;
	bus_freq_multiplier = (int)(multiplier * 2);

	ppc.hid1 = 0;
}

static CPU_EXIT( ppc601 )
{
}

static CPU_INIT( ppc604 )
{
	const ppc_config *configdata = device->static_config();
	float multiplier;
	int i ;

	ppc_init() ;

	ppc.optable[48] = ppc_lfs;
	ppc.optable[49] = ppc_lfsu;
	ppc.optable[50] = ppc_lfd;
	ppc.optable[51] = ppc_lfdu;
	ppc.optable[52] = ppc_stfs;
	ppc.optable[53] = ppc_stfsu;
	ppc.optable[54] = ppc_stfd;
	ppc.optable[55] = ppc_stfdu;
	ppc.optable31[631] = ppc_lfdux;
	ppc.optable31[599] = ppc_lfdx;
	ppc.optable31[567] = ppc_lfsux;
	ppc.optable31[535] = ppc_lfsx;
	ppc.optable31[595] = ppc_mfsr;
	ppc.optable31[659] = ppc_mfsrin;
	ppc.optable31[371] = ppc_mftb;
	ppc.optable31[210] = ppc_mtsr;
	ppc.optable31[242] = ppc_mtsrin;
	ppc.optable31[758] = ppc_dcba;
	ppc.optable31[759] = ppc_stfdux;
	ppc.optable31[727] = ppc_stfdx;
	ppc.optable31[983] = ppc_stfiwx;
	ppc.optable31[695] = ppc_stfsux;
	ppc.optable31[663] = ppc_stfsx;
	ppc.optable31[370] = ppc_tlbia;
	ppc.optable31[306] = ppc_tlbie;
	ppc.optable31[566] = ppc_tlbsync;
	ppc.optable31[310] = ppc_eciwx;
	ppc.optable31[438] = ppc_ecowx;

	ppc.optable63[264] = ppc_fabsx;
	ppc.optable63[21] = ppc_faddx;
	ppc.optable63[32] = ppc_fcmpo;
	ppc.optable63[0] = ppc_fcmpu;
	ppc.optable63[14] = ppc_fctiwx;
	ppc.optable63[15] = ppc_fctiwzx;
	ppc.optable63[18] = ppc_fdivx;
	ppc.optable63[72] = ppc_fmrx;
	ppc.optable63[136] = ppc_fnabsx;
	ppc.optable63[40] = ppc_fnegx;
	ppc.optable63[12] = ppc_frspx;
	ppc.optable63[26] = ppc_frsqrtex;
	ppc.optable63[22] = ppc_fsqrtx;
	ppc.optable63[20] = ppc_fsubx;
	ppc.optable63[583] = ppc_mffsx;
	ppc.optable63[70] = ppc_mtfsb0x;
	ppc.optable63[38] = ppc_mtfsb1x;
	ppc.optable63[711] = ppc_mtfsfx;
	ppc.optable63[134] = ppc_mtfsfix;
	ppc.optable63[64] = ppc_mcrfs;

	ppc.optable59[21] = ppc_faddsx;
	ppc.optable59[18] = ppc_fdivsx;
	ppc.optable59[24] = ppc_fresx;
	ppc.optable59[22] = ppc_fsqrtsx;
	ppc.optable59[20] = ppc_fsubsx;

	for(i = 0; i < 32; i++)
	{
		ppc.optable63[i * 32 | 29] = ppc_fmaddx;
		ppc.optable63[i * 32 | 28] = ppc_fmsubx;
		ppc.optable63[i * 32 | 25] = ppc_fmulx;
		ppc.optable63[i * 32 | 31] = ppc_fnmaddx;
		ppc.optable63[i * 32 | 30] = ppc_fnmsubx;
		ppc.optable63[i * 32 | 23] = ppc_fselx;

		ppc.optable59[i * 32 | 29] = ppc_fmaddsx;
		ppc.optable59[i * 32 | 28] = ppc_fmsubsx;
		ppc.optable59[i * 32 | 25] = ppc_fmulsx;
		ppc.optable59[i * 32 | 31] = ppc_fnmaddsx;
		ppc.optable59[i * 32 | 30] = ppc_fnmsubsx;
	}

	ppc.optable31[978] = ppc_tlbld;

	for(i = 0; i < 256; i++)
	{
		ppc_field_xlat[i] =
			((i & 0x80) ? 0xF0000000 : 0) |
			((i & 0x40) ? 0x0F000000 : 0) |
			((i & 0x20) ? 0x00F00000 : 0) |
			((i & 0x10) ? 0x000F0000 : 0) |
			((i & 0x08) ? 0x0000F000 : 0) |
			((i & 0x04) ? 0x00000F00 : 0) |
			((i & 0x02) ? 0x000000F0 : 0) |
			((i & 0x01) ? 0x0000000F : 0);
	}

	ppc.is603 = 1;

	ppc.read8 = memory_read_byte_64be;
	ppc.read16 = memory_read_word_64be;
	ppc.read32 = memory_read_dword_64be;
	ppc.read64 = memory_read_qword_64be;
	ppc.write8 = memory_write_byte_64be;
	ppc.write16 = memory_write_word_64be;
	ppc.write32 = memory_write_dword_64be;
	ppc.write64 = memory_write_qword_64be;
	ppc.read16_unaligned = ppc_read16_unaligned;
	ppc.read32_unaligned = ppc_read32_unaligned;
	ppc.read64_unaligned = ppc_read64_unaligned;
	ppc.write16_unaligned = ppc_write16_unaligned;
	ppc.write32_unaligned = ppc_write32_unaligned;
	ppc.write64_unaligned = ppc_write64_unaligned;

	ppc.irq_callback = irqcallback;
	ppc.device = device;
	ppc.program = &device->space(AS_PROGRAM);

	ppc.pvr = configdata->pvr;

	multiplier = (float)((configdata->bus_frequency_multiplier >> 4) & 0xf) +
					(float)(configdata->bus_frequency_multiplier & 0xf) / 10.0f;
	bus_freq_multiplier = (int)(multiplier * 2);

	ppc.hid1 = 0;
}

static CPU_EXIT( ppc604 )
{
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( ppc )
{
	switch (state)
	{
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + PPC_PC:             ppc.pc = info->i;                       break;
		case CPUINFO_INT_REGISTER + PPC_MSR:            ppc_set_msr(info->i);                   break;
		case CPUINFO_INT_REGISTER + PPC_CR:             ppc_set_cr(info->i);                    break;
		case CPUINFO_INT_REGISTER + PPC_LR:             LR = info->i;                           break;
		case CPUINFO_INT_REGISTER + PPC_CTR:            CTR = info->i;                          break;
		case CPUINFO_INT_REGISTER + PPC_XER:            XER = info->i;                          break;
		case CPUINFO_INT_REGISTER + PPC_SRR0:           SRR0 = info->i;                         break;
		case CPUINFO_INT_REGISTER + PPC_SRR1:           SRR1 = info->i;                         break;

		case CPUINFO_INT_REGISTER + PPC_R0:             ppc.r[0] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R1:             ppc.r[1] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R2:             ppc.r[2] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R3:             ppc.r[3] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R4:             ppc.r[4] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R5:             ppc.r[5] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R6:             ppc.r[6] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R7:             ppc.r[7] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R8:             ppc.r[8] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R9:             ppc.r[9] = info->i;                     break;
		case CPUINFO_INT_REGISTER + PPC_R10:            ppc.r[10] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R11:            ppc.r[11] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R12:            ppc.r[12] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R13:            ppc.r[13] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R14:            ppc.r[14] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R15:            ppc.r[15] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R16:            ppc.r[16] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R17:            ppc.r[17] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R18:            ppc.r[18] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R19:            ppc.r[19] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R20:            ppc.r[20] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R21:            ppc.r[21] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R22:            ppc.r[22] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R23:            ppc.r[23] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R24:            ppc.r[24] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R25:            ppc.r[25] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R26:            ppc.r[26] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R27:            ppc.r[27] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R28:            ppc.r[28] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R29:            ppc.r[29] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R30:            ppc.r[30] = info->i;                    break;
		case CPUINFO_INT_REGISTER + PPC_R31:            ppc.r[31] = info->i;                    break;
	}
}

static CPU_SET_INFO( ppc403 )
{
	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + 8)
	{
		ppc403_set_irq_line(state-CPUINFO_INT_INPUT_STATE, info->i);
		return;
	}
	switch(state)
	{
		case CPUINFO_INT_REGISTER + PPC_EXIER:      EXIER = info->i;                        break;
		case CPUINFO_INT_REGISTER + PPC_EXISR:      EXISR = info->i;                        break;
		default:                                    ppc_set_info(state, info);              break;
	}
}

static CPU_SET_INFO( ppc603 )
{
	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + 5)
	{
		ppc603_set_irq_line(state-CPUINFO_INT_INPUT_STATE, info->i);
		return;
	}
	switch(state)
	{
		case CPUINFO_INT_REGISTER + PPC_DEC:                write_decrementer(info->i);     break;
		case CPUINFO_INT_INPUT_STATE + PPC_INPUT_LINE_SMI:  ppc603_set_smi_line(info->i);   break;
		default:                                            ppc_set_info(state, info);      break;
	}
}

static CPU_GET_INFO( ppc )
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(ppc);                  break;
		case CPUINFO_INT_INPUT_LINES:                   info->i = 1;                            break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:            info->i = 0;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;                   break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                            break;
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 1;                            break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 4;                            break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 4;                            break;
		case CPUINFO_INT_MIN_CYCLES:                    info->i = 1;                            break;
		case CPUINFO_INT_MAX_CYCLES:                    info->i = 40;                           break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 32;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;                  break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;                   break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:   info->i = 0;                    break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:     info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:     info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:     info->i = 0;                    break;

		case CPUINFO_INT_INPUT_STATE:                   info->i = CLEAR_LINE;                   break;

		case CPUINFO_INT_PREVIOUSPC:                    /* not implemented */                   break;

		case CPUINFO_INT_PC:    /* intentional fallthrough */
		case CPUINFO_INT_REGISTER + PPC_PC:             info->i = ppc.pc;                       break;
		case CPUINFO_INT_REGISTER + PPC_MSR:            info->i = ppc_get_msr();                break;
		case CPUINFO_INT_REGISTER + PPC_CR:             info->i = ppc_get_cr();                 break;
		case CPUINFO_INT_REGISTER + PPC_LR:             info->i = LR;                           break;
		case CPUINFO_INT_REGISTER + PPC_CTR:            info->i = CTR;                          break;
		case CPUINFO_INT_REGISTER + PPC_XER:            info->i = XER;                          break;
		case CPUINFO_INT_REGISTER + PPC_SRR0:           info->i = SRR0;                         break;
		case CPUINFO_INT_REGISTER + PPC_SRR1:           info->i = SRR1;                         break;

		case CPUINFO_INT_REGISTER + PPC_R0:             info->i = ppc.r[0];                     break;
		case CPUINFO_INT_REGISTER + PPC_R1:             info->i = ppc.r[1];                     break;
		case CPUINFO_INT_REGISTER + PPC_R2:             info->i = ppc.r[2];                     break;
		case CPUINFO_INT_REGISTER + PPC_R3:             info->i = ppc.r[3];                     break;
		case CPUINFO_INT_REGISTER + PPC_R4:             info->i = ppc.r[4];                     break;
		case CPUINFO_INT_REGISTER + PPC_R5:             info->i = ppc.r[5];                     break;
		case CPUINFO_INT_REGISTER + PPC_R6:             info->i = ppc.r[6];                     break;
		case CPUINFO_INT_REGISTER + PPC_R7:             info->i = ppc.r[7];                     break;
		case CPUINFO_INT_REGISTER + PPC_R8:             info->i = ppc.r[8];                     break;
		case CPUINFO_INT_REGISTER + PPC_R9:             info->i = ppc.r[9];                     break;
		case CPUINFO_INT_REGISTER + PPC_R10:            info->i = ppc.r[10];                    break;
		case CPUINFO_INT_REGISTER + PPC_R11:            info->i = ppc.r[11];                    break;
		case CPUINFO_INT_REGISTER + PPC_R12:            info->i = ppc.r[12];                    break;
		case CPUINFO_INT_REGISTER + PPC_R13:            info->i = ppc.r[13];                    break;
		case CPUINFO_INT_REGISTER + PPC_R14:            info->i = ppc.r[14];                    break;
		case CPUINFO_INT_REGISTER + PPC_R15:            info->i = ppc.r[15];                    break;
		case CPUINFO_INT_REGISTER + PPC_R16:            info->i = ppc.r[16];                    break;
		case CPUINFO_INT_REGISTER + PPC_R17:            info->i = ppc.r[17];                    break;
		case CPUINFO_INT_REGISTER + PPC_R18:            info->i = ppc.r[18];                    break;
		case CPUINFO_INT_REGISTER + PPC_R19:            info->i = ppc.r[19];                    break;
		case CPUINFO_INT_REGISTER + PPC_R20:            info->i = ppc.r[20];                    break;
		case CPUINFO_INT_REGISTER + PPC_R21:            info->i = ppc.r[21];                    break;
		case CPUINFO_INT_REGISTER + PPC_R22:            info->i = ppc.r[22];                    break;
		case CPUINFO_INT_REGISTER + PPC_R23:            info->i = ppc.r[23];                    break;
		case CPUINFO_INT_REGISTER + PPC_R24:            info->i = ppc.r[24];                    break;
		case CPUINFO_INT_REGISTER + PPC_R25:            info->i = ppc.r[25];                    break;
		case CPUINFO_INT_REGISTER + PPC_R26:            info->i = ppc.r[26];                    break;
		case CPUINFO_INT_REGISTER + PPC_R27:            info->i = ppc.r[27];                    break;
		case CPUINFO_INT_REGISTER + PPC_R28:            info->i = ppc.r[28];                    break;
		case CPUINFO_INT_REGISTER + PPC_R29:            info->i = ppc.r[29];                    break;
		case CPUINFO_INT_REGISTER + PPC_R30:            info->i = ppc.r[30];                    break;
		case CPUINFO_INT_REGISTER + PPC_R31:            info->i = ppc.r[31];                    break;



		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_BURN:                          info->burn = NULL;                      break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(ppc);          break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:           info->icount = &ppc_icount;             break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PPC403");              break;
		case CPUINFO_STR_SHORTNAME:                          strcpy(info->s, "ppc403");              break;
		case CPUINFO_STR_FAMILY:                    strcpy(info->s, "PowerPC");             break;
		case CPUINFO_STR_VERSION:                   strcpy(info->s, "1.0");                 break;
		case CPUINFO_STR_SOURCE_FILE:                       strcpy(info->s, __FILE__);              break;
		case CPUINFO_STR_CREDITS:                   strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;

		case CPUINFO_STR_FLAGS:                         strcpy(info->s, " ");                   break;

		case CPUINFO_STR_REGISTER + PPC_PC:             sprintf(info->s, "PC: %08X", ppc.pc);   break;
		case CPUINFO_STR_REGISTER + PPC_MSR:            sprintf(info->s, "MSR: %08X", ppc_get_msr()); break;
		case CPUINFO_STR_REGISTER + PPC_CR:             sprintf(info->s, "CR: %08X", ppc_get_cr()); break;
		case CPUINFO_STR_REGISTER + PPC_LR:             sprintf(info->s, "LR: %08X", LR);       break;
		case CPUINFO_STR_REGISTER + PPC_CTR:            sprintf(info->s, "CTR: %08X", CTR);     break;
		case CPUINFO_STR_REGISTER + PPC_XER:            sprintf(info->s, "XER: %08X", XER);     break;
		case CPUINFO_STR_REGISTER + PPC_SRR0:           sprintf(info->s, "SRR0: %08X", SRR0);   break;
		case CPUINFO_STR_REGISTER + PPC_SRR1:           sprintf(info->s, "SRR1: %08X", SRR1);   break;

		case CPUINFO_STR_REGISTER + PPC_R0:             sprintf(info->s, "R0: %08X", ppc.r[0]); break;
		case CPUINFO_STR_REGISTER + PPC_R1:             sprintf(info->s, "R1: %08X", ppc.r[1]); break;
		case CPUINFO_STR_REGISTER + PPC_R2:             sprintf(info->s, "R2: %08X", ppc.r[2]); break;
		case CPUINFO_STR_REGISTER + PPC_R3:             sprintf(info->s, "R3: %08X", ppc.r[3]); break;
		case CPUINFO_STR_REGISTER + PPC_R4:             sprintf(info->s, "R4: %08X", ppc.r[4]); break;
		case CPUINFO_STR_REGISTER + PPC_R5:             sprintf(info->s, "R5: %08X", ppc.r[5]); break;
		case CPUINFO_STR_REGISTER + PPC_R6:             sprintf(info->s, "R6: %08X", ppc.r[6]); break;
		case CPUINFO_STR_REGISTER + PPC_R7:             sprintf(info->s, "R7: %08X", ppc.r[7]); break;
		case CPUINFO_STR_REGISTER + PPC_R8:             sprintf(info->s, "R8: %08X", ppc.r[8]); break;
		case CPUINFO_STR_REGISTER + PPC_R9:             sprintf(info->s, "R9: %08X", ppc.r[9]); break;
		case CPUINFO_STR_REGISTER + PPC_R10:            sprintf(info->s, "R10: %08X", ppc.r[10]); break;
		case CPUINFO_STR_REGISTER + PPC_R11:            sprintf(info->s, "R11: %08X", ppc.r[11]); break;
		case CPUINFO_STR_REGISTER + PPC_R12:            sprintf(info->s, "R12: %08X", ppc.r[12]); break;
		case CPUINFO_STR_REGISTER + PPC_R13:            sprintf(info->s, "R13: %08X", ppc.r[13]); break;
		case CPUINFO_STR_REGISTER + PPC_R14:            sprintf(info->s, "R14: %08X", ppc.r[14]); break;
		case CPUINFO_STR_REGISTER + PPC_R15:            sprintf(info->s, "R15: %08X", ppc.r[15]); break;
		case CPUINFO_STR_REGISTER + PPC_R16:            sprintf(info->s, "R16: %08X", ppc.r[16]); break;
		case CPUINFO_STR_REGISTER + PPC_R17:            sprintf(info->s, "R17: %08X", ppc.r[17]); break;
		case CPUINFO_STR_REGISTER + PPC_R18:            sprintf(info->s, "R18: %08X", ppc.r[18]); break;
		case CPUINFO_STR_REGISTER + PPC_R19:            sprintf(info->s, "R19: %08X", ppc.r[19]); break;
		case CPUINFO_STR_REGISTER + PPC_R20:            sprintf(info->s, "R20: %08X", ppc.r[20]); break;
		case CPUINFO_STR_REGISTER + PPC_R21:            sprintf(info->s, "R21: %08X", ppc.r[21]); break;
		case CPUINFO_STR_REGISTER + PPC_R22:            sprintf(info->s, "R22: %08X", ppc.r[22]); break;
		case CPUINFO_STR_REGISTER + PPC_R23:            sprintf(info->s, "R23: %08X", ppc.r[23]); break;
		case CPUINFO_STR_REGISTER + PPC_R24:            sprintf(info->s, "R24: %08X", ppc.r[24]); break;
		case CPUINFO_STR_REGISTER + PPC_R25:            sprintf(info->s, "R25: %08X", ppc.r[25]); break;
		case CPUINFO_STR_REGISTER + PPC_R26:            sprintf(info->s, "R26: %08X", ppc.r[26]); break;
		case CPUINFO_STR_REGISTER + PPC_R27:            sprintf(info->s, "R27: %08X", ppc.r[27]); break;
		case CPUINFO_STR_REGISTER + PPC_R28:            sprintf(info->s, "R28: %08X", ppc.r[28]); break;
		case CPUINFO_STR_REGISTER + PPC_R29:            sprintf(info->s, "R29: %08X", ppc.r[29]); break;
		case CPUINFO_STR_REGISTER + PPC_R30:            sprintf(info->s, "R30: %08X", ppc.r[30]); break;
		case CPUINFO_STR_REGISTER + PPC_R31:            sprintf(info->s, "R31: %08X", ppc.r[31]); break;
	}
}

CPU_GET_INFO( ppc403 )
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:                   info->i = 8;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;                   break;
		case CPUINFO_INT_REGISTER + PPC_EXIER:          info->i = EXIER;                        break;
		case CPUINFO_INT_REGISTER + PPC_EXISR:          info->i = EXISR;                        break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(ppc403);      break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(ppc403);             break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(ppc403);               break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(ppc403);             break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(ppc403);           break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PPC403");              break;
		case CPUINFO_STR_SHORTNAME:                     strcpy(info->s, "ppc403");              break;
		case CPUINFO_STR_REGISTER + PPC_EXIER:          sprintf(info->s, "EXIER: %08X", EXIER); break;
		case CPUINFO_STR_REGISTER + PPC_EXISR:          sprintf(info->s, "EXISR: %08X", EXISR); break;

		default:                                        CPU_GET_INFO_CALL(ppc);             break;
	}
}

CPU_GET_INFO( ppc405 )
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:                   info->i = 8;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;                   break;
		case CPUINFO_INT_REGISTER + PPC_EXIER:          info->i = EXIER;                        break;
		case CPUINFO_INT_REGISTER + PPC_EXISR:          info->i = EXISR;                        break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(ppc405);      break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(ppc405);             break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(ppc405);               break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(ppc405);             break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(ppc405);           break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PPC405");              break;
		case CPUINFO_STR_SHORTNAME:                     strcpy(info->s, "ppc405");              break;
		case CPUINFO_STR_REGISTER + PPC_EXIER:          sprintf(info->s, "EXIER: %08X", EXIER); break;
		case CPUINFO_STR_REGISTER + PPC_EXISR:          sprintf(info->s, "EXISR: %08X", EXISR); break;

		default:                                        CPU_GET_INFO_CALL(ppc);             break;
	}
}

CPU_GET_INFO( ppc603 )
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:                   info->i = 5;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;                   break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 64;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;                  break;
		case CPUINFO_INT_LOGADDR_WIDTH_PROGRAM: info->i = 32;                   break;
		case CPUINFO_INT_PAGE_SHIFT_PROGRAM:    info->i = 17;                   break;
		case CPUINFO_INT_REGISTER + PPC_DEC:            info->i = read_decrementer();           break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(ppc603);      break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(ppc603);             break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(ppc603);               break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(ppc603);             break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(ppc603);           break;
		case CPUINFO_FCT_READ:                          info->read = CPU_GET_READ_NAME(ppc);                    break;
		case CPUINFO_FCT_WRITE:                         info->write = CPU_GET_WRITE_NAME(ppc);              break;
		case CPUINFO_FCT_READOP:                        info->readop = CPU_GET_READOP_NAME(ppc);                break;
		case CPUINFO_FCT_TRANSLATE:                     info->translate = ppc_translate_address_cb; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PPC603");              break;
		case CPUINFO_STR_SHORTNAME:                     strcpy(info->s, "ppc603");              break;
		case CPUINFO_STR_REGISTER + PPC_DEC:            sprintf(info->s, "DEC: %08X", read_decrementer()); break;

		default:                                        CPU_GET_INFO_CALL(ppc);             break;
	}
}

static CPU_SET_INFO( ppc602 )
{
	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + 5)
	{
		ppc602_set_irq_line(state-CPUINFO_INT_INPUT_STATE, info->i);
		return;
	}
	switch(state)
	{
		case CPUINFO_INT_INPUT_STATE + PPC_INPUT_LINE_SMI:  ppc602_set_smi_line(info->i);       break;
		default:                                        ppc_set_info(state, info);              break;
	}
}

CPU_GET_INFO( ppc602 )
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:                   info->i = 5;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;                   break;
		case CPUINFO_INT_REGISTER + PPC_IBR:            info->i = ppc.ibr;                      break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 64;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;                  break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(ppc602);      break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(ppc602);             break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(ppc602);               break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(ppc602);             break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(ppc602);           break;
		case CPUINFO_FCT_READ:                          info->read = CPU_GET_READ_NAME(ppc);                    break;
		case CPUINFO_FCT_WRITE:                         info->write = CPU_GET_WRITE_NAME(ppc);              break;
		case CPUINFO_FCT_READOP:                        info->readop = CPU_GET_READOP_NAME(ppc);                break;
		case CPUINFO_FCT_TRANSLATE:                     info->translate = ppc_translate_address_cb; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PPC602");              break;
		case CPUINFO_STR_SHORTNAME:                     strcpy(info->s, "ppc602");              break;
		case CPUINFO_STR_REGISTER + PPC_IBR:            sprintf(info->s, "IBR: %08X", ppc.ibr); break;

		default:                                        CPU_GET_INFO_CALL(ppc);             break;
	}
}


static CPU_SET_INFO( mpc8240 )
{
	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + 5)
	{
		ppc603_set_irq_line(state-CPUINFO_INT_INPUT_STATE, info->i);
		return;
	}
	switch(state)
	{
		default:                                        ppc_set_info(state, info);              break;
	}
}

CPU_GET_INFO( mpc8240 )
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:                   info->i = 5;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;                   break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 64;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;                  break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(mpc8240);     break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(mpc8240);                break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(ppc603);               break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(mpc8240);                break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(ppc603);           break;
		case CPUINFO_FCT_READ:                          info->read = CPU_GET_READ_NAME(ppc);                    break;
		case CPUINFO_FCT_WRITE:                         info->write = CPU_GET_WRITE_NAME(ppc);              break;
		case CPUINFO_FCT_READOP:                        info->readop = CPU_GET_READOP_NAME(ppc);                break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "MPC8240");             break;
		case CPUINFO_STR_SHORTNAME:                     strcpy(info->s, "mpc8240");              break;

		default:                                        CPU_GET_INFO_CALL(ppc);             break;
	}
}

static CPU_SET_INFO( ppc601 )
{
	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + 5)
	{
		ppc603_set_irq_line(state-CPUINFO_INT_INPUT_STATE, info->i);
		return;
	}
	switch(state)
	{
		default:                                        ppc_set_info(state, info);              break;
	}
}

CPU_GET_INFO( ppc601 )
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:                   info->i = 5;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;                   break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 64;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;                  break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(ppc601);      break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(ppc601);             break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(ppc603);               break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(ppc601);             break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(ppc603);           break;
		case CPUINFO_FCT_READ:                          info->read = CPU_GET_READ_NAME(ppc);                    break;
		case CPUINFO_FCT_WRITE:                         info->write = CPU_GET_WRITE_NAME(ppc);              break;
		case CPUINFO_FCT_READOP:                        info->readop = CPU_GET_READOP_NAME(ppc);                break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PPC601");              break;
		case CPUINFO_STR_SHORTNAME:                     strcpy(info->s, "ppc601");              break;

		default:                                        CPU_GET_INFO_CALL(ppc);             break;
	}
}

static CPU_SET_INFO( ppc604 )
{
	if (state >= CPUINFO_INT_INPUT_STATE && state <= CPUINFO_INT_INPUT_STATE + 5)
	{
		ppc603_set_irq_line(state-CPUINFO_INT_INPUT_STATE, info->i);
		return;
	}
	switch(state)
	{
		default:                                        ppc_set_info(state, info);              break;
	}
}

CPU_GET_INFO( ppc604 )
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_LINES:                   info->i = 5;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_BIG;                   break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:    info->i = 64;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;                  break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(ppc604);      break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(ppc604);             break;
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(ppc603);               break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(ppc604);             break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(ppc603);           break;
		case CPUINFO_FCT_READ:                          info->read = CPU_GET_READ_NAME(ppc);                    break;
		case CPUINFO_FCT_WRITE:                         info->write = CPU_GET_WRITE_NAME(ppc);              break;
		case CPUINFO_FCT_READOP:                        info->readop = CPU_GET_READOP_NAME(ppc);                break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PPC604");              break;
		case CPUINFO_STR_SHORTNAME:                     strcpy(info->s, "ppc604");              break;

		default:                                        CPU_GET_INFO_CALL(ppc);             break;
	}
}

ppc403ga_device::ppc403ga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock)
	: ppc4xx_device(mconfig, type, tag, owner, clock, CPU_GET_INFO_NAME(ppc403ga))
{
}

const device_type PPC403GA = &legacy_device_creator<ppc403ga_device>;

ppc403gcx_device::ppc403gcx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock)
	: ppc4xx_device(mconfig, type, tag, owner, clock, CPU_GET_INFO_NAME(ppc403gcx))
{
}

const device_type PPC403GCX = &legacy_device_creator<ppc403gcx_device>;

ppc405gp_device::ppc405gp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock)
	: ppc4xx_device(mconfig, type, tag, owner, clock, CPU_GET_INFO_NAME(ppc405gp))
{
}

const device_type PPC405GP = &legacy_device_creator<ppc405gp_device>;

ppc4xx_device::ppc4xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock, cpu_get_info_func info)
	: legacy_cpu_device(mconfig, type, tag, owner, clock, info)
{
}

DEFINE_LEGACY_CPU_DEVICE(PPC601, ppc601);
DEFINE_LEGACY_CPU_DEVICE(PPC602, ppc602);
DEFINE_LEGACY_CPU_DEVICE(PPC603, ppc603);
DEFINE_LEGACY_CPU_DEVICE(PPC603E, ppc603e);
DEFINE_LEGACY_CPU_DEVICE(PPC603R, ppc603r);
DEFINE_LEGACY_CPU_DEVICE(PPC604, ppc604);
DEFINE_LEGACY_CPU_DEVICE(MPC8240, mpc8240);
