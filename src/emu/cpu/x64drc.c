/***************************************************************************

    x64drc.c

    x64 Dynamic recompiler support routines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Windows x64 conventions:

        Register        Status          Use
        --------        ------          ---
        RAX             Volatile        Return value register
        RCX             Volatile        First integer argument
        RDX             Volatile        Second integer argument
        R8              Volatile        Third integer argument
        R9              Volatile        Fourth integer argument
        R10:R11         Volatile        Must be preserved as needed by caller; used in syscall/sysret instructions
        R12:R15         Nonvolatile     Must be preserved by callee
        RDI             Nonvolatile     Must be preserved by callee
        RSI             Nonvolatile     Must be preserved by callee
        RBX             Nonvolatile     Must be preserved by callee
        RBP             Nonvolatile     May be used as a frame pointer; must be preserved by callee
        RSP             Nonvolatile     Stack pointer

        XMM0            Volatile        First FP argument
        XMM1            Volatile        Second FP argument
        XMM2            Volatile        Third FP argument
        XMM3            Volatile        Fourth FP argument
        XMM4:XMM5       Volatile        Must be preserved as needed by caller
        XMM6:XMM15      Nonvolatile     Must be preserved as needed by callee.


    Linux/MacOS x64 conventions:

        Register        Status          Use
        --------        ------          ---
        RAX             Volatile        Return value register
        RDI             Volatile        First integer argument
        RSI             Volatile        Second integer argument
        RDX             Volatile        Third integer argument
        RCX             Volatile        Fourth integer argument
        R8              Volatile        Fifth integer argument
        R9              Volatile        Sixth integer argument
        R10:R11         Volatile        Must be preserved as needed by caller
        R12:R15         Nonvolatile     Must be preserved by callee
        RBX             Nonvolatile     Must be preserved by callee
        RBP             Nonvolatile     Must be preserved by callee
        RSP             Nonvolatile     Stack pointer

        XMM0            Volatile        First FP argument
        XMM1            Volatile        Second FP argument
        XMM2            Volatile        Third FP argument
        XMM3            Volatile        Fourth FP argument
        XMM4            Volatile        Fifth FP argument
        XMM5            Volatile        Sixth FP argument
        XMM6:XMM15      Volatile        Must be preserved as needed by caller.

***************************************************************************/

#include "cpuintrf.h"
#include "x64drc.h"
#include "debugger.h"

#ifdef ENABLE_DEBUGGER
#include "deprecat.h"
#endif



/***************************************************************************
    MACROS
***************************************************************************/

#define MXCSR_VALUE(x)		(0x1fc0 | ((x) << 14))



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void append_entry_point(drc_core *drc);
static void append_exit_point(drc_core *drc);
static void recompile_code(drc_core *drc);
static void append_recompile(drc_core *drc);
static void append_flush(drc_core *drc);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    allocate_l2_table - allocate an L2 table
    if necessary
-------------------------------------------------*/

INLINE void allocate_l2_table(drc_core *drc, UINT32 l1index)
{
	if (drc->lookup_l1[l1index] == drc->lookup_l2_recompile)
	{
		/* create a new copy of the recompile table */
		drc->lookup_l1[l1index] = drc_alloc(drc, sizeof(*drc->lookup_l2_recompile) << drc->l2bits);
		memcpy(drc->lookup_l1[l1index], drc->lookup_l2_recompile, sizeof(*drc->lookup_l2_recompile) << drc->l2bits);
	}
}


/***************************************************************************
    EXTERNAL INTERFACES
***************************************************************************/

/*-------------------------------------------------
    drc_init - initialize the DRC core
-------------------------------------------------*/

drc_core *drc_init(UINT8 cpunum, drc_config *config)
{
	int address_bits = config->address_bits;
	int effective_address_bits = address_bits - config->lsbs_to_ignore;
	drc_core *drc;
	int i;

	/* the drc structure lives at the start of the cache */
	drc = (drc_core *)config->cache_base;
	memset(drc, 0, sizeof(*drc));

	/* copy in relevant data from the config */
	drc->baseptr      = config->baseptr;
	drc->pcptr        = config->pcptr;
	drc->cb_reset     = config->cb_reset;
	drc->cb_recompile = config->cb_recompile;
	drc->cb_entrygen  = config->cb_entrygen;
	drc->mxcsr_curr   = MXCSR_VALUE(FPRND_NEAR);

	/* configure cache */
	drc->cache_base = (UINT8 *)config->cache_base + sizeof(*drc);
	drc->cache_size = config->cache_size - sizeof(*drc);
	drc->cache_end = drc->cache_base + drc->cache_size;
	drc->cache_danger = drc->cache_end - 65536;

	/* compute shifts and masks */
	drc->l1bits = effective_address_bits / 2;
	drc->l2bits = effective_address_bits - drc->l1bits;
	drc->l1shift = config->lsbs_to_ignore + drc->l2bits;
	drc->l2mask = ((1 << drc->l2bits) - 1) << config->lsbs_to_ignore;
	drc->l2scale = sizeof(x86code *) >> config->lsbs_to_ignore;

	/* configure the MXCSR states */
	for (i = 0; i < 4; i++)
		drc->mxcsr_values[i] = MXCSR_VALUE(i);

	/* allocate lookup tables out of the cache */
	drc->lookup_l1 = drc_alloc(drc, sizeof(*drc->lookup_l1) << drc->l1bits);
	drc->lookup_l2_recompile = drc_alloc(drc, sizeof(*drc->lookup_l2_recompile) << drc->l2bits);
	if (drc->lookup_l1 == NULL || drc->lookup_l2_recompile == NULL)
		goto error;
	memset(drc->lookup_l1, 0, sizeof(*drc->lookup_l1) << drc->l1bits);
	memset(drc->lookup_l2_recompile, 0, sizeof(*drc->lookup_l2_recompile) << drc->l2bits);

	/* allocate the sequence and tentative lists */
	drc->sequence_count_max = config->max_instructions * 2;
	drc->sequence_list = malloc(drc->sequence_count_max * sizeof(*drc->sequence_list));
	if (drc->sequence_list == NULL)
		goto error;

	drc->tentative_count_max = config->max_instructions * 2;
	drc->tentative_list = malloc(drc->tentative_count_max * sizeof(*drc->tentative_list));
	if (drc->tentative_list == NULL)
		goto error;

	/* get pointers to external C functions */
#ifdef ENABLE_DEBUGGER
	drc->mame_debug_hook = (x86code *)mame_debug_hook;
#endif
	drc->recompile_code = (x86code *)recompile_code;
	drc->drc_cache_reset = (x86code *)drc_cache_reset;
	return drc;

error:
	if (drc != NULL)
		drc_exit(drc);
	return NULL;
}


/*-------------------------------------------------
    drc_alloc - allocate memory from the top of
    the DRC cache
-------------------------------------------------*/

void *drc_alloc(drc_core *drc, size_t amount)
{
	/* if we don't have enough space, reset the cache */
	if (drc->cache_top >= drc->cache_danger - amount)
		drc_cache_reset(drc);

	/* if we still don't have enough space, fail */
	if (drc->cache_top >= drc->cache_danger - amount)
		return NULL;

	/* adjust the end and danger values downward */
	drc->cache_end -= amount;
	drc->cache_danger -= amount;
	return drc->cache_end;
}


/*-------------------------------------------------
    drc_cache_reset - reset the DRC cache
-------------------------------------------------*/

void drc_cache_reset(drc_core *drc)
{
	int i;

	/* reset the cache and add the basics */
	drc->cache_top = drc->cache_base;

	/* append the core entry points to the fresh cache */
	drc->entry_point = (void (*)(void *))(FPTR)drc->cache_top;
	append_entry_point(drc);
	drc->exit_point = drc->cache_top;
	append_exit_point(drc);
	drc->recompile = drc->cache_top;
	append_recompile(drc);
	drc->dispatch = drc->cache_top;
	drc_append_dispatcher(drc);
	drc->flush = drc->cache_top;
	append_flush(drc);

	/* populate the recompile table */
	for (i = 0; i < (1 << drc->l2bits); i++)
		drc->lookup_l2_recompile[i] = drc->recompile;

	/* reset all the l1 tables */
	for (i = 0; i < (1 << drc->l1bits); i++)
	{
		/* point NULL entries to the generic recompile table */
		if (drc->lookup_l1[i] == NULL)
			drc->lookup_l1[i] = drc->lookup_l2_recompile;

		/* reset allocated tables to point all entries back to the recompiler */
		else if (drc->lookup_l1[i] != drc->lookup_l2_recompile)
			memcpy(drc->lookup_l1[i], drc->lookup_l2_recompile, sizeof(*drc->lookup_l2_recompile) << drc->l2bits);
	}

	/* call back to the host */
	if (drc->cb_reset != NULL)
		(*drc->cb_reset)(drc);
}


/*-------------------------------------------------
    drc_execute - execute generated code
-------------------------------------------------*/

void drc_execute(drc_core *drc)
{
	(*drc->entry_point)(drc->baseptr);
}


/*-------------------------------------------------
    drc_exit - release resources allocated by
    the DRC core
-------------------------------------------------*/

void drc_exit(drc_core *drc)
{
	/* free the lists */
	if (drc->sequence_list != NULL)
		free(drc->sequence_list);
	if (drc->tentative_list != NULL)
		free(drc->tentative_list);
}


/*-------------------------------------------------
    drc_begin_sequence - begin code generation
    for a particular PC
-------------------------------------------------*/

void drc_begin_sequence(drc_core *drc, UINT32 pc)
{
	/* reset the sequence and tentative counts */
	drc->sequence_count = 0;
	drc->tentative_count = 0;
}


/*-------------------------------------------------
    drc_add_entry_point - add a new external
    entry point for a given PC
-------------------------------------------------*/

int drc_add_entry_point(drc_core *drc, UINT32 pc, int override)
{
	UINT32 l1index = pc >> drc->l1shift;
	UINT32 l2index = ((pc & drc->l2mask) * drc->l2scale) / sizeof(x86code *);
	int was_occupied;

	/* allocate memory if necessary */
	allocate_l2_table(drc, l1index);

	/* in case anybody was jumping directly to this code entry, replace
       the code with a jmp to the new code */
    was_occupied = (drc->lookup_l1[l1index][l2index] != drc->recompile);
	if (was_occupied && override)
	{
		x86code *dest = drc->lookup_l1[l1index][l2index];
		emit_jmp(&dest, drc->cache_top);
	}

	/* note the current location for this instruction */
	if (!was_occupied || override)
		drc->lookup_l1[l1index][l2index] = drc->cache_top;
	return was_occupied;
}


/*------------------------------------------------------------------
    drc_end_sequence
------------------------------------------------------------------*/

void drc_end_sequence(drc_core *drc)
{
	int i, j;

	/* fix up any internal links */
	for (i = 0; i < drc->tentative_count; i++)
	{
		for (j = 0; j < drc->sequence_count; j++)
			if (drc->tentative_list[i].pc == drc->sequence_list[j].pc)
			{
				x86code *dest = drc->tentative_list[i].target;
				emit_jmp(&dest, drc->sequence_list[j].target);
				while (dest < drc->tentative_list[i].end)
					emit_int_3(&dest);
				break;
			}
	}
}


/*------------------------------------------------------------------
    drc_register_code_at_cache_top
------------------------------------------------------------------*/

void drc_register_code_at_cache_top(drc_core *drc, UINT32 pc)
{
	pc_ptr_pair *pair = &drc->sequence_list[drc->sequence_count++];
	assert_always(drc->sequence_count <= drc->sequence_count_max, "drc_register_code_at_cache_top: too many instructions!");

	pair->target = drc->cache_top;
	pair->pc = pc;
}


/*------------------------------------------------------------------
    drc_get_code_at_pc
------------------------------------------------------------------*/

x86code *drc_get_code_at_pc(drc_core *drc, UINT32 pc)
{
	UINT32 l1index = pc >> drc->l1shift;
	UINT32 l2index = ((pc & drc->l2mask) * drc->l2scale) / sizeof(x86code *);
	x86code *codeptr = drc->lookup_l1[l1index][l2index];
	return (codeptr != drc->recompile) ? codeptr : NULL;
}


/*------------------------------------------------------------------
    drc_invalidate_code_range
------------------------------------------------------------------*/

void drc_invalidate_code_range(drc_core *drc, UINT32 startpc, UINT32 endpc)
{
	UINT32 startl1index = startpc >> drc->l1shift;
	UINT32 startl2index = ((startpc & drc->l2mask) * drc->l2scale) / sizeof(x86code *);
	UINT32 endl1index = endpc >> drc->l1shift;
	UINT32 endl2index = ((endpc & drc->l2mask) * drc->l2scale) / sizeof(x86code *);
	UINT32 curl1;

	/* iterate over l1 bunches */
	for (curl1 = startl1index; curl1 <= endl1index; curl1++)
	{
		x86code **l2table = drc->lookup_l1[curl1];
		UINT32 l2start, l2end, curl2;

		/* if this is already empty, skip it */
		if (l2table == drc->lookup_l2_recompile)
			continue;

		/* determine start/stop for this range */
		l2start = (curl1 == startl1index) ? startl2index : 0;
		l2end = (curl1 == endl1index) ? endl2index : ((1 << drc->l2bits) - 1);

		/* invalidate any code entries here */
		for (curl2 = l2start; curl2 <= l2end; curl2++)
			if (l2table[curl2] != drc->recompile)
			{
				/* overwrite code there with a jump to the recompiler in case we ever get there */
				x86code *dest = l2table[curl2];
//              emit_int_3(&dest);
				emit_jmp(&dest, drc->recompile);
				l2table[curl2] = drc->recompile;
			}
	}
}


/*------------------------------------------------------------------
    drc_append_call_debugger
------------------------------------------------------------------*/

void drc_append_call_debugger(drc_core *drc)
{
#ifdef ENABLE_DEBUGGER
	if (Machine->debug_mode)
		emit_call_m64(DRCTOP, MDRC(&drc->mame_debug_hook));							// call mame_debug_hook
#endif
}


/*------------------------------------------------------------------
    drc_append_dispatcher
------------------------------------------------------------------*/

void drc_append_dispatcher(drc_core *drc)
{
	/* target PC is in P1 on entry; we must keep P1 intact in case we jump to the recompile callback */
	emit_mov_r32_r32(DRCTOP, REG_P2, REG_P1);										// mov  p2,p1
	emit_mov_r32_r32(DRCTOP, REG_EAX, REG_P1);										// mov  eax,p1
	emit_shr_r32_imm(DRCTOP, REG_P2, drc->l1shift);									// shr  p2,l1shift
	emit_mov_r64_m64(DRCTOP, REG_P2, MDRCISD(drc->lookup_l1, REG_P2, 8, 0));		// mov  p2,[rbp+p2*8+l1lookup]
	emit_and_r32_imm(DRCTOP, REG_EAX, drc->l2mask);									// and  eax,l2mask
	emit_jmp_m64(DRCTOP, MBISD(REG_P2, REG_EAX, drc->l2scale, 0));					// jmp  [p2 + eax*l2scale]
}


/*------------------------------------------------------------------
    drc_append_fixed_dispatcher
------------------------------------------------------------------*/

void drc_append_fixed_dispatcher(drc_core *drc, UINT32 pc, int loadpc)
{
	UINT32 l1index = pc >> drc->l1shift;
	UINT32 l2index = ((pc & drc->l2mask) * drc->l2scale) / sizeof(x86code *);

	/* make sure we have an L2 table for this entry */
	allocate_l2_table(drc, l1index);

	/* emit a jump through the table entry */
	if (loadpc)
		emit_mov_r32_imm(DRCTOP, REG_P1, pc);										// mov  p1,pc
	emit_jmp_m64(DRCTOP, MDRC(&drc->lookup_l1[l1index][l2index]));					// jmp  [lookup[l1index][l2index]]
}


/*------------------------------------------------------------------
    drc_append_tentative_fixed_dispatcher
------------------------------------------------------------------*/

void drc_append_tentative_fixed_dispatcher(drc_core *drc, UINT32 newpc, int loadpc)
{
	pc_ptr_pair *pair = &drc->tentative_list[drc->tentative_count++];
	assert_always(drc->tentative_count <= drc->tentative_count_max, "drc_append_tentative_fixed_dispatcher: too many tentative branches!");

	pair->target = drc->cache_top;
	pair->pc = newpc;
	drc_append_fixed_dispatcher(drc, newpc, loadpc);
	pair->end = drc->cache_top;
}


/*------------------------------------------------------------------
    drc_append_set_sse_rounding
------------------------------------------------------------------*/

void drc_append_set_sse_rounding(drc_core *drc, UINT8 regindex)
{
	emit_ldmxcsr_m32(DRCTOP, MDRCISD(&drc->mxcsr_values[0], regindex, 4, 0));		// ldmxcsr [mxcsr_values + reg*4]
	emit_stmxcsr_m32(DRCTOP, MDRC(&drc->mxcsr_curr));								// stmxcsr [mxcsr_curr]
}



/*------------------------------------------------------------------
    drc_append_set_temp_sse_rounding
------------------------------------------------------------------*/

void drc_append_set_temp_sse_rounding(drc_core *drc, UINT8 rounding)
{
	emit_ldmxcsr_m32(DRCTOP, MDRC(&drc->mxcsr_values[rounding]));					// ldmxcsr mxcsr_values[rounding]
}



/*------------------------------------------------------------------
    drc_append_restore_sse_rounding
------------------------------------------------------------------*/

void drc_append_restore_sse_rounding(drc_core *drc)
{
	emit_ldmxcsr_m32(DRCTOP, MDRC(&drc->mxcsr_curr));								// ldmxcsr [mxcsr_curr]
}



/***************************************************************************
    INTERNAL CODEGEN
***************************************************************************/

/*------------------------------------------------------------------
    append_entry_point
------------------------------------------------------------------*/

static void append_entry_point(drc_core *drc)
{
	int pushbytes = 8 * (6 + (REG_NV5 != REG_NONE) + (REG_NV6 != REG_NONE));

	/* save non-volatile registers */
	emit_push_r64(DRCTOP, REG_DRC);													// push drc
	emit_push_r64(DRCTOP, REG_NV0);													// push nv0
	emit_push_r64(DRCTOP, REG_NV1); 												// push nv1
	emit_push_r64(DRCTOP, REG_NV2);													// push nv2
	emit_push_r64(DRCTOP, REG_NV3);													// push nv3
	emit_push_r64(DRCTOP, REG_NV4);													// push nv4
	if (REG_NV5 != REG_NONE) emit_push_r64(DRCTOP, REG_NV5);						// push nv5
	if (REG_NV6 != REG_NONE) emit_push_r64(DRCTOP, REG_NV6);						// push nv6

	/* stack frame:
        0x40/0x48 bytes of scratch space/alignment
        0x20 bytes for register parameter spilling in Windows ABI
    */
	emit_sub_r64_imm(DRCTOP, REG_RSP, 0x68 + (pushbytes % 16));						// sub  rsp,0x68/0x70

	/* parameter 1 is the baseptr; copy to RBP */
	emit_mov_r64_r64(DRCTOP, REG_DRC, REG_P1);										// mov  rbp,p1

	/* update the MXCSR register */
	emit_stmxcsr_m32(DRCTOP, MDRC(&drc->mxcsr_save));								// stmxcsr [mxcsr_save]
	emit_ldmxcsr_m32(DRCTOP, MDRC(&drc->mxcsr_curr));								// ldmxcsr [mxcsr_curr]

	/* continue performing entry generation duties */
	if (drc->cb_entrygen != NULL)
		(*drc->cb_entrygen)(drc);													// additional entry point duties

	/* reload the PC into P1 and dispatch */
	emit_mov_r32_m32(DRCTOP, REG_P1, MDRC(drc->pcptr));								// mov  p1,[pc]
	drc_append_dispatcher(drc);														// dispatch
}


/*------------------------------------------------------------------
    append_exit_point
------------------------------------------------------------------*/

static void append_exit_point(drc_core *drc)
{
	int pushbytes = 8 * (6 + (REG_NV5 != REG_NONE) + (REG_NV6 != REG_NONE));

	/* on exit, P1 must contain the final PC */
	emit_mov_m32_r32(DRCTOP, MDRC(drc->pcptr), REG_P1);								// mov  [pc],p1

	/* restore the MXCSR state */
	emit_ldmxcsr_m32(DRCTOP, MDRC(&drc->mxcsr_save));								// ldmxcsr [mxcsr_save]

	/* add 0x18 to RSP to get us back to the original stack pointer */
	emit_add_r64_imm(DRCTOP, REG_RSP, 0x68 + (pushbytes % 16));						// add  rsp,0x68/0x70

	/* save non-volatile registers */
	if (REG_NV6 != REG_NONE) emit_pop_r64(DRCTOP, REG_NV6);							// pop nv6
	if (REG_NV5 != REG_NONE) emit_pop_r64(DRCTOP, REG_NV5);							// pop nv5
	emit_pop_r64(DRCTOP, REG_NV4);													// pop nv4
	emit_pop_r64(DRCTOP, REG_NV3);													// pop nv3
	emit_pop_r64(DRCTOP, REG_NV2);													// pop nv2
	emit_pop_r64(DRCTOP, REG_NV1);													// pop nv1
	emit_pop_r64(DRCTOP, REG_NV0);													// pop nv0
	emit_pop_r64(DRCTOP, REG_DRC);													// pop drc
	emit_ret(DRCTOP);																// ret
}


/*------------------------------------------------------------------
    recompile_code
------------------------------------------------------------------*/

static void recompile_code(drc_core *drc)
{
	/* if we're above the danger line, flush the cache before recompiling */
	if (drc->cache_top >= drc->cache_danger)
		drc_cache_reset(drc);

	/* call the recompile callback */
	(*drc->cb_recompile)(drc);
}


/*------------------------------------------------------------------
    append_recompile
------------------------------------------------------------------*/

static void append_recompile(drc_core *drc)
{
	/* on entry, P1 must contain the PC that needs recompilation */
	emit_mov_m32_r32(DRCTOP, MDRC(drc->pcptr), REG_P1);								// mov  [pc],p1
	emit_lea_r64_m64(DRCTOP, REG_P1, MDRC(drc));									// lea  p1,drc
	emit_call_m64(DRCTOP, MDRC(&drc->recompile_code));								// call recompile_code
	emit_mov_r32_m32(DRCTOP, REG_P1, MDRC(drc->pcptr));								// mov  p1,[pc]
	drc_append_dispatcher(drc);														// dispatch
}


/*------------------------------------------------------------------
    append_flush
------------------------------------------------------------------*/

static void append_flush(drc_core *drc)
{
	/* on entry, P1 must contain the PC that needs recompilation */
	emit_mov_m32_r32(DRCTOP, MDRC(drc->pcptr), REG_P1);								// mov  [pc],p1
	emit_lea_r64_m64(DRCTOP, REG_P1, MDRC(drc));									// lea  p1,drc
	emit_call_m64(DRCTOP, MDRC(&drc->drc_cache_reset));								// call drc_cache_reset
	emit_mov_r32_m32(DRCTOP, REG_P1, MDRC(drc->pcptr));								// mov  p1,[pc]
	drc_append_dispatcher(drc);														// dispatch
}
