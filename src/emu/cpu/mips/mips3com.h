/***************************************************************************

    mips3com.h

    Common MIPS III/IV definitions and functions

***************************************************************************/

#pragma once

#ifndef __MIPS3COM_H__
#define __MIPS3COM_H__

#include "mips3.h"
#include "cpu/vtlb.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* core parameters */
#define MIPS3_MIN_PAGE_SHIFT        12
#define MIPS3_MIN_PAGE_SIZE         (1 << MIPS3_MIN_PAGE_SHIFT)
#define MIPS3_MIN_PAGE_MASK         (MIPS3_MIN_PAGE_SIZE - 1)
#define MIPS3_MAX_PADDR_SHIFT       32
#define MIPS3_MAX_TLB_ENTRIES       48

/* cycle parameters */
#define MIPS3_COUNT_READ_CYCLES     250
#define MIPS3_CAUSE_READ_CYCLES     250

/* MIPS flavors */
enum mips3_flavor
{
	/* MIPS III variants */
	MIPS3_TYPE_MIPS_III,
	MIPS3_TYPE_VR4300,
	MIPS3_TYPE_R4600,
	MIPS3_TYPE_R4650,
	MIPS3_TYPE_R4700,

	/* MIPS IV variants */
	MIPS3_TYPE_MIPS_IV,
	MIPS3_TYPE_R5000,
	MIPS3_TYPE_QED5271,
	MIPS3_TYPE_RM7000
};

/* TLB bits */
#define TLB_GLOBAL              0x01
#define TLB_VALID               0x02
#define TLB_DIRTY               0x04
#define TLB_PRESENT             0x08

/* COP0 registers */
#define COP0_Index              0
#define COP0_Random             1
#define COP0_EntryLo            2
#define COP0_EntryLo0           2
#define COP0_EntryLo1           3
#define COP0_Context            4
#define COP0_PageMask           5
#define COP0_Wired              6
#define COP0_BadVAddr           8
#define COP0_Count              9
#define COP0_EntryHi            10
#define COP0_Compare            11
#define COP0_Status             12
#define COP0_Cause              13
#define COP0_EPC                14
#define COP0_PRId               15
#define COP0_Config             16
#define COP0_LLAddr             17
#define COP0_XContext           20
#define COP0_ECC                26
#define COP0_CacheErr           27
#define COP0_TagLo              28
#define COP0_TagHi              29
#define COP0_ErrorPC            30

/* Status register bits */
#define SR_IE                   0x00000001
#define SR_EXL                  0x00000002
#define SR_ERL                  0x00000004
#define SR_KSU_MASK             0x00000018
#define SR_KSU_KERNEL           0x00000000
#define SR_KSU_SUPERVISOR       0x00000008
#define SR_KSU_USER             0x00000010
#define SR_IMSW0                0x00000100
#define SR_IMSW1                0x00000200
#define SR_IMEX0                0x00000400
#define SR_IMEX1                0x00000800
#define SR_IMEX2                0x00001000
#define SR_IMEX3                0x00002000
#define SR_IMEX4                0x00004000
#define SR_IMEX5                0x00008000
#define SR_DE                   0x00010000
#define SR_CE                   0x00020000
#define SR_CH                   0x00040000
#define SR_SR                   0x00100000
#define SR_TS                   0x00200000
#define SR_BEV                  0x00400000
#define SR_ITS                  0x01000000  /* VR4300 only, Application Note doesn't give purpose */
#define SR_RE                   0x02000000
#define SR_FR                   0x04000000
#define SR_RP                   0x08000000
#define SR_COP0                 0x10000000
#define SR_COP1                 0x20000000
#define SR_COP2                 0x40000000
#define SR_COP3                 0x80000000

/* exception types */
#define EXCEPTION_INTERRUPT     0
#define EXCEPTION_TLBMOD        1
#define EXCEPTION_TLBLOAD       2
#define EXCEPTION_TLBSTORE      3
#define EXCEPTION_ADDRLOAD      4
#define EXCEPTION_ADDRSTORE     5
#define EXCEPTION_BUSINST       6
#define EXCEPTION_BUSDATA       7
#define EXCEPTION_SYSCALL       8
#define EXCEPTION_BREAK         9
#define EXCEPTION_INVALIDOP     10
#define EXCEPTION_BADCOP        11
#define EXCEPTION_OVERFLOW      12
#define EXCEPTION_TRAP          13
#define EXCEPTION_TLBLOAD_FILL  16
#define EXCEPTION_TLBSTORE_FILL 17
#define EXCEPTION_COUNT         18



/***************************************************************************
    HELPER MACROS
***************************************************************************/

#define REG_LO          32
#define REG_HI          33

#define RSREG           ((op >> 21) & 31)
#define RTREG           ((op >> 16) & 31)
#define RDREG           ((op >> 11) & 31)
#define SHIFT           ((op >> 6) & 31)

#define FRREG           ((op >> 21) & 31)
#define FTREG           ((op >> 16) & 31)
#define FSREG           ((op >> 11) & 31)
#define FDREG           ((op >> 6) & 31)

#define IS_SINGLE(o)    (((o) & (1 << 21)) == 0)
#define IS_DOUBLE(o)    (((o) & (1 << 21)) != 0)
#define IS_FLOAT(o)     (((o) & (1 << 23)) == 0)
#define IS_INTEGRAL(o)  (((o) & (1 << 23)) != 0)

#define SIMMVAL         ((INT16)op)
#define UIMMVAL         ((UINT16)op)
#define LIMMVAL         (op & 0x03ffffff)



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* MIPS3 TLB entry */
struct mips3_tlb_entry
{
	UINT64          page_mask;
	UINT64          entry_hi;
	UINT64          entry_lo[2];
};


/* forward declaration of implementation-specific state */
struct mips3imp_state;


/* MIPS3 state */
struct mips3_state
{
	/* core registers */
	UINT32          pc;
	int             icount;
	UINT64          r[35];

	/* COP registers */
	UINT64          cpr[3][32];
	UINT64          ccr[3][32];
	UINT32          llbit;

	/* internal stuff */
	mips3_flavor    flavor;
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device * device;
	address_space *program;
	direct_read_data *direct;
	UINT32          system_clock;
	UINT32          cpu_clock;
	UINT64          count_zero_time;
	UINT32          compare_armed;
	emu_timer *     compare_int_timer;

	/* derived info based on flavor */
	UINT32          pfnmask;
	UINT8           tlbentries;

	/* memory accesses */
	UINT8           bigendian;
	data_accessors  memory;

	/* cache memory */
	size_t          icache_size;
	size_t          dcache_size;

	/* MMU */
	vtlb_state *    vtlb;
	mips3_tlb_entry tlb[MIPS3_MAX_TLB_ENTRIES];

	/* for use by specific implementations */
	mips3imp_state *impstate;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void mips3com_init(mips3_state *mips, mips3_flavor flavor, int bigendian, legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback);
void mips3com_exit(mips3_state *mips);

void mips3com_reset(mips3_state *mips);
offs_t mips3com_dasm(mips3_state *mips, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
void mips3com_update_cycle_counting(mips3_state *mips);

void mips3com_asid_changed(mips3_state *mips);
int mips3com_translate_address(mips3_state *mips, address_spacenum space, int intention, offs_t *address);
void mips3com_tlbr(mips3_state *mips);
void mips3com_tlbwi(mips3_state *mips);
void mips3com_tlbwr(mips3_state *mips);
void mips3com_tlbp(mips3_state *mips);

void mips3com_set_info(mips3_state *mips, UINT32 state, cpuinfo *info);
void mips3com_get_info(mips3_state *mips, UINT32 state, cpuinfo *info);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    mips3com_set_irq_line - set or clear the given
    IRQ line
-------------------------------------------------*/

INLINE void mips3com_set_irq_line(mips3_state *mips, int irqline, int state)
{
	if (state != CLEAR_LINE)
		mips->cpr[0][COP0_Cause] |= 0x400 << irqline;
	else
		mips->cpr[0][COP0_Cause] &= ~(0x400 << irqline);
}

#endif /* __MIPS3COM_H__ */
