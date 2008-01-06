/***************************************************************************

    x64drc.h

    x64 Dynamic recompiler support routines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __X64DRC_H__
#define __X64DRC_H__

#include "cpuintrf.h"
#include "x86emit.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* ABI-specific configuration */
#define REG_DRC				REG_RBP			/* pointer to DRC base */

#ifdef X64_WINDOWS_ABI

#define REG_P1				REG_RCX			/* 1st function parameter */
#define REG_P2				REG_RDX			/* 2nd function parameter */
#define REG_P3				REG_R8			/* 3rd function parameter */
#define REG_P4				REG_R9			/* 4th function parameter */
#define REG_V5				REG_R10			/* volatile register 5 */
#define REG_V6				REG_R11			/* volatile register 6 */

#define NUM_NVREG			7				/* number of non-volatile registers */
#define REG_NV0				REG_RBX			/* non-volatile reg 0 */
#define REG_NV1				REG_RDI			/* non-volatile reg 1 */
#define REG_NV2				REG_RSI			/* non-volatile reg 2 */
#define REG_NV3				REG_R12			/* non-volatile reg 3 */
#define REG_NV4				REG_R13			/* non-volatile reg 4 */
#define REG_NV5				REG_R14			/* non-volatile reg 5 */
#define REG_NV6				REG_R15			/* non-volatile reg 6 */

#else

#define REG_P1				REG_RDI			/* 1st function parameter */
#define REG_P2				REG_RSI			/* 2nd function parameter */
#define REG_P3				REG_RDX			/* 3rd function parameter */
#define REG_P4				REG_RCX			/* 4th function parameter */
#define REG_V5				REG_R10			/* volatile register 5 */
#define REG_V6				REG_R11			/* volatile register 6 */

#define NUM_NVREG			5				/* number of non-volatile registers */
#define REG_NV0				REG_RBX			/* non-volatile reg 0 */
#define REG_NV1				REG_R12			/* non-volatile reg 1 */
#define REG_NV2				REG_R13			/* non-volatile reg 2 */
#define REG_NV3				REG_R14			/* non-volatile reg 3 */
#define REG_NV4				REG_R15			/* non-volatile reg 4 */
#define REG_NV5				REG_NONE		/* non-volatile reg 5 */
#define REG_NV6				REG_NONE		/* non-volatile reg 6 */

#endif



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* PC and pointer pair */
typedef struct _pc_ptr_pair pc_ptr_pair;
struct _pc_ptr_pair
{
	UINT32			pc;
	x86code *		target;
	x86code *		end;
};


/* core interface structure for the drc common code */
typedef struct _drc_core drc_core;
struct _drc_core
{
	/* cache parameters */
	x86code *		cache_base;					/* base pointer to the compiler cache */
	x86code *		cache_top;					/* current top of cache */
	x86code *		cache_danger;				/* high water mark for the end */
	x86code *		cache_end;					/* end of cache memory */
	size_t			cache_size;					/* cache allocated size */

	/* hash table lookups */
	x86code ***		lookup_l1;					/* level 1 lookup */
	x86code **		lookup_l2_recompile;		/* level 2 lookup populated with recompile pointers */
	UINT8			l1bits;						/* number of bits in level 1 lookup */
	UINT8			l2bits;						/* number of bits in level 2 lookup */
	UINT8			l1shift;					/* shift to go from PC to level 1 lookup */
	UINT32			l2mask;						/* mask to go from PC to level 2 lookup */
	UINT8			l2scale;					/* scale to get from masked PC value to final level 2 lookup */

	/* entry point for calling from C code */
	void 			(*entry_point)(void *);		/* pointer to asm entry point */

	/* base pointer for memory accesses */
	void *			baseptr;					/* pointer to base; all cache must be accessible from here */

	/* internal subroutines generated in the cache */
	x86code *		exit_point;					/* exit out of the DRC engine */
	x86code *		recompile;					/* pointer to recompile jump point */
	x86code *		dispatch;					/* pointer to dispatch jump point */
	x86code *		flush;						/* pointer to flush jump point */

	/* pointers to external C code */
	x86code *		mame_debug_hook;			/* pointer to mame_debug_hook function */
	x86code *		recompile_code;				/* pointer to recompile_code function */
	x86code *		drc_cache_reset;			/* pointer to drc_cache_reset function */

	/* pointers to the PC */
	UINT32 *		pcptr;						/* pointer to where the PC is stored */

	/* save areas for the MXCSR register */
	UINT32			mxcsr_curr;					/* current SSE control word */
	UINT32			mxcsr_save;					/* saved SSE control word */
	UINT32			mxcsr_values[4];			/* array of values for different modes */

	/* internal lists of sequences and tentative branches */
	pc_ptr_pair *	sequence_list;				/* PC/pointer sets for the current instruction sequence */
	UINT32			sequence_count;				/* number of instructions in the current sequence */
	UINT32			sequence_count_max;			/* max number of instructions in the current sequence */
	pc_ptr_pair *	tentative_list;				/* PC/pointer sets for tentative branches */
	UINT32			tentative_count;			/* number of tentative branches */
	UINT32			tentative_count_max;		/* max number of tentative branches */

	/* CPU-specific callbacks */
	void 			(*cb_reset)(struct _drc_core *drc);		/* callback when the cache is reset */
	void 			(*cb_recompile)(struct _drc_core *drc);	/* callback when code needs to be recompiled */
	void 			(*cb_entrygen)(struct _drc_core *drc);	/* callback before generating the dispatcher on entry */
};


/* configuration structure for the drc common code */
typedef struct _drc_config drc_config;
struct _drc_config
{
	void *			cache_base;					/* base of cache */
	UINT32			cache_size;					/* size of cache */
	UINT32			max_instructions;			/* maximum instructions per sequence */
	UINT8			address_bits;				/* number of live address bits in the PC */
	UINT8			lsbs_to_ignore;				/* number of LSBs to ignore on the PC */

	void *			baseptr;					/* pointer to base; all cache must be accessible from here */
	UINT32 *		pcptr;						/* pointer to where the PC is stored */

	void 			(*cb_reset)(drc_core *drc);		/* callback when the cache is reset */
	void 			(*cb_recompile)(drc_core *drc);	/* callback when code needs to be recompiled */
	void 			(*cb_entrygen)(drc_core *drc);	/* callback before generating the dispatcher on entry */
};



/***************************************************************************
    MACROS
***************************************************************************/

/* use this macro in emit_* instructions to make them shorter */
#define DRCTOP					&drc->cache_top

/* memory references to DRC-relative data */
#define MDRC(x)							MBD(REG_DRC, drcrel32(drc->baseptr, (x)))
#define MDRCD(x, disp)					MBD(REG_DRC, drcrel32(drc->baseptr, (x)) + (disp))
#define MDRCISD(x, index, scale, disp)	MBISD(REG_DRC, (index), (scale), drcrel32(drc->baseptr, (x)) + (disp))



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* init/shutdown */
drc_core *drc_init(UINT8 cpunum, drc_config *config);
void *drc_alloc(drc_core *drc, size_t amount);
void drc_cache_reset(drc_core *drc);
void drc_execute(drc_core *drc);
void drc_exit(drc_core *drc);

/* code management */
void drc_begin_sequence(drc_core *drc, UINT32 pc);
int drc_add_entry_point(drc_core *drc, UINT32 pc, int override);
void drc_end_sequence(drc_core *drc);
void drc_register_code_at_cache_top(drc_core *drc, UINT32 pc);
x86code *drc_get_code_at_pc(drc_core *drc, UINT32 pc);
void drc_invalidate_code_range(drc_core *drc, UINT32 startpc, UINT32 endpc);

/* standard appendages */
void drc_append_dispatcher(drc_core *drc);
void drc_append_fixed_dispatcher(drc_core *drc, UINT32 newpc, int loadpc);
void drc_append_tentative_fixed_dispatcher(drc_core *drc, UINT32 newpc, int loadpc);
void drc_append_call_debugger(drc_core *drc);
void drc_append_set_sse_rounding(drc_core *drc, UINT8 regindex);
void drc_append_set_temp_sse_rounding(drc_core *drc, UINT8 rounding);
void drc_append_restore_sse_rounding(drc_core *drc);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE INT32 drcrel32(void *base, void *ptr)
{
	INT64 delta = (UINT8 *)ptr - (UINT8 *)base;
	assert((INT32)delta == delta);
	return (INT32)delta;
}

INLINE void *drcrelptr(drc_core *drc, INT32 offset)
{
	return (UINT8 *)drc + offset;
}

#endif	/* __X64DRC_H__ */
