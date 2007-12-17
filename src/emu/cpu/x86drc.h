/***************************************************************************

    x86drc.h

    x86 Dynamic recompiler support routines.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __X86DRC_H__
#define __X86DRC_H__

#include "cpuintrf.h"
#include "x86emit.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* PC and pointer pair */
typedef struct _pc_ptr_pair pc_ptr_pair;
struct _pc_ptr_pair
{
	UINT32		pc;
	x86code *	target;
};


/* core interface structure for the drc common code */
typedef struct _drc_core drc_core;
struct _drc_core
{
	UINT8 *		cache_base;				/* base pointer to the compiler cache */
	UINT8 *		cache_top;				/* current top of cache */
	UINT8 *		cache_danger;			/* high water mark for the end */
	UINT8 *		cache_end;				/* end of cache memory */
	size_t		cache_size;				/* cache allocated size */
	UINT8		cache_allocated;		/* did the DRC core allocate the cache? */

	x86code ***	lookup_l1;				/* level 1 lookup */
	x86code **	lookup_l2_recompile;	/* level 2 lookup populated with recompile pointers */
	UINT8		l1bits;					/* number of bits in level 1 lookup */
	UINT8		l2bits;					/* number of bits in level 2 lookup */
	UINT8		l1shift;				/* shift to go from PC to level 1 lookup */
	UINT32		l2mask;					/* mask to go from PC to level 2 lookup */
	UINT8		l2scale;				/* scale to get from masked PC value to final level 2 lookup */

	void 		(*entry_point)(void);	/* pointer to asm entry point */
	x86code *	out_of_cycles;			/* pointer to out of cycles jump point */
	x86code *	recompile;				/* pointer to recompile jump point */
	x86code *	dispatch;				/* pointer to dispatch jump point */
	x86code *	flush;					/* pointer to flush jump point */

	UINT32 *	pcptr;					/* pointer to where the PC is stored */
	UINT32 *	icountptr;				/* pointer to where the icount is stored */
	UINT32 *	esiptr;					/* pointer to where the volatile data in ESI is stored */
	UINT8		pc_in_memory;			/* true if the PC is stored in memory */
	UINT8		icount_in_memory;		/* true if the icount is stored in memory */

	UINT8		uses_fp;				/* true if we need the FP unit */
	UINT8		uses_sse;				/* true if we need the SSE unit */
	UINT16		fpcw_curr;				/* current FPU control word */
	UINT32		mxcsr_curr;				/* current SSE control word */
	UINT16		fpcw_save;				/* saved FPU control word */
	UINT32		mxcsr_save;				/* saved SSE control word */

	pc_ptr_pair *sequence_list;			/* PC/pointer sets for the current instruction sequence */
	UINT32		sequence_count;			/* number of instructions in the current sequence */
	UINT32		sequence_count_max;		/* max number of instructions in the current sequence */
	pc_ptr_pair *tentative_list;		/* PC/pointer sets for tentative branches */
	UINT32		tentative_count;		/* number of tentative branches */
	UINT32		tentative_count_max;	/* max number of tentative branches */

	void 		(*cb_reset)(struct _drc_core *drc);		/* callback when the cache is reset */
	void 		(*cb_recompile)(struct _drc_core *drc);	/* callback when code needs to be recompiled */
	void 		(*cb_entrygen)(struct _drc_core *drc);	/* callback before generating the dispatcher on entry */
};


/* configuration structure for the drc common code */
typedef struct _drc_config drc_config;
struct _drc_config
{
	UINT8 *		cache_base;				/* base pointer to the compiler cache */
	UINT32		cache_size;				/* size of cache to allocate */
	UINT32		max_instructions;		/* maximum instructions per sequence */
	UINT8		address_bits;			/* number of live address bits in the PC */
	UINT8		lsbs_to_ignore;			/* number of LSBs to ignore on the PC */
	UINT8		uses_fp;				/* true if we need the FP unit */
	UINT8		uses_sse;				/* true if we need the SSE unit */
	UINT8		pc_in_memory;			/* true if the PC is stored in memory */
	UINT8		icount_in_memory;		/* true if the icount is stored in memory */

	UINT32 *	pcptr;					/* pointer to where the PC is stored */
	UINT32 *	icountptr;				/* pointer to where the icount is stored */
	UINT32 *	esiptr;					/* pointer to where the volatile data in ESI is stored */

	void 		(*cb_reset)(drc_core *drc);		/* callback when the cache is reset */
	void 		(*cb_recompile)(drc_core *drc);	/* callback when code needs to be recompiled */
	void 		(*cb_entrygen)(drc_core *drc);	/* callback before generating the dispatcher on entry */
};


/* structure to hold link data to be filled in later */
typedef struct _link_info link_info;
struct _link_info
{
	UINT8 		size;
	UINT8 *		target;
};



/***************************************************************************
    MACROS
***************************************************************************/

/* use this macro in emit_* instructions to make them shorter */
#define DRCTOP					&drc->cache_top



/***************************************************************************
    HELPER MACROS
***************************************************************************/

/* useful macros for accessing hi/lo portions of 64-bit values */
#define LO(x)		(&(((UINT32 *)(UINT32)(x))[0]))
#define HI(x)		(&(((UINT32 *)(UINT32)(x))[1]))



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* features */
#define CPUID_FEATURES_MMX		(1 << 23)
#define CPUID_FEATURES_SSE		(1 << 26)
#define CPUID_FEATURES_SSE2		(1 << 25)
#define CPUID_FEATURES_CMOV		(1 << 15)
#define CPUID_FEATURES_TSC		(1 << 4)



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
void drc_end_sequence(drc_core *drc);
void drc_register_code_at_cache_top(drc_core *drc, UINT32 pc);
void *drc_get_code_at_pc(drc_core *drc, UINT32 pc);

/* standard appendages */
void drc_append_dispatcher(drc_core *drc);
void drc_append_fixed_dispatcher(drc_core *drc, UINT32 newpc);
void drc_append_tentative_fixed_dispatcher(drc_core *drc, UINT32 newpc);
void drc_append_call_debugger(drc_core *drc);
void drc_append_standard_epilogue(drc_core *drc, INT32 cycles, INT32 pcdelta, int allow_exit);
void drc_append_save_volatiles(drc_core *drc);
void drc_append_restore_volatiles(drc_core *drc);
void drc_append_save_call_restore(drc_core *drc, x86code *target, UINT32 stackadj);
void drc_append_verify_code(drc_core *drc, void *code, UINT8 length);

void drc_append_set_fp_rounding(drc_core *drc, UINT8 regindex);
void drc_append_set_temp_fp_rounding(drc_core *drc, UINT8 rounding);
void drc_append_restore_fp_rounding(drc_core *drc);

void drc_append_set_sse_rounding(drc_core *drc, UINT8 regindex);
void drc_append_set_temp_sse_rounding(drc_core *drc, UINT8 rounding);
void drc_append_restore_sse_rounding(drc_core *drc);

/* disassembling drc code */
void drc_dasm(FILE *f, const void *begin, const void *end);

/* x86 CPU features */
UINT32 drc_x86_get_features(void);


#endif	/* __X86DRC_H__ */
