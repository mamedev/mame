/***************************************************************************

    x86drc.c

    x86 Dynamic recompiler support routines.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "cpuintrf.h"
#include "x86drc.h"
#include "debugger.h"

#define LOG_DISPATCHES				0
#define BREAK_ON_MODIFIED_CODE		0




static const UINT16 fp_control[4] = { 0x023f, 0x063f, 0x0a3f, 0x0e3f };
static const UINT32 sse_control[4] = { 0x9fc0, 0xbfc0, 0xdfc0, 0xffc0 };


static void append_entry_point(drc_core *drc);
static void append_recompile(drc_core *drc);
static void append_flush(drc_core *drc);
static void append_out_of_cycles(drc_core *drc);

#if LOG_DISPATCHES
static void log_dispatch(drc_core *drc);
#endif



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
	UINT8 cache_allocated = FALSE;
	drc_core *drc = NULL;

	/* allocate memory */
	if (config->cache_base == NULL)
	{
		config->cache_base = osd_alloc_executable(config->cache_size);
		if (config->cache_base == NULL)
			goto error;
		cache_allocated = TRUE;
	}

	/* the drc structure lives at the start of the cache */
	drc = (drc_core *)config->cache_base;
	memset(drc, 0, sizeof(*drc));

	/* copy in relevant data from the config */
	drc->pcptr        = config->pcptr;
	drc->icountptr    = config->icountptr;
	drc->esiptr       = config->esiptr;
	drc->cb_reset     = config->cb_reset;
	drc->cb_recompile = config->cb_recompile;
	drc->cb_entrygen  = config->cb_entrygen;
	drc->uses_fp      = config->uses_fp;
	drc->uses_sse     = config->uses_sse;
	drc->pc_in_memory = config->pc_in_memory;
	drc->icount_in_memory = config->icount_in_memory;
	drc->fpcw_curr    = fp_control[0];
	drc->mxcsr_curr   = sse_control[0];

	/* configure cache */
	drc->cache_base = (UINT8 *)config->cache_base + sizeof(*drc);
	drc->cache_size = config->cache_size - sizeof(*drc);
	drc->cache_end = drc->cache_base + drc->cache_size;
	drc->cache_danger = drc->cache_end - 65536;
	drc->cache_allocated = cache_allocated;

	/* compute shifts and masks */
	drc->l1bits = effective_address_bits/2;
	drc->l2bits = effective_address_bits - drc->l1bits;
	drc->l1shift = config->lsbs_to_ignore + drc->l2bits;
	drc->l2mask = ((1 << drc->l2bits) - 1) << config->lsbs_to_ignore;
	drc->l2scale = 4 >> config->lsbs_to_ignore;

	/* allocate lookup tables */
	drc->lookup_l1 = malloc(sizeof(*drc->lookup_l1) * (1 << drc->l1bits));
	drc->lookup_l2_recompile = malloc(sizeof(*drc->lookup_l2_recompile) * (1 << drc->l2bits));
	if (drc->lookup_l1 == NULL || drc->lookup_l2_recompile == NULL)
		goto error;
	memset(drc->lookup_l1, 0, sizeof(*drc->lookup_l1) * (1 << drc->l1bits));
	memset(drc->lookup_l2_recompile, 0, sizeof(*drc->lookup_l2_recompile) * (1 << drc->l2bits));

	/* allocate the sequence and tentative lists */
	drc->sequence_count_max = config->max_instructions;
	drc->sequence_list = malloc(drc->sequence_count_max * sizeof(*drc->sequence_list));
	if (drc->sequence_list == NULL)
		goto error;

	drc->tentative_count_max = config->max_instructions;
	drc->tentative_list = malloc(drc->tentative_count_max * sizeof(*drc->tentative_list));
	if (!drc->tentative_list)
		return NULL;

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
	drc->entry_point = (void (*)(void))(UINT32)drc->cache_top;
	append_entry_point(drc);
	drc->out_of_cycles = drc->cache_top;
	append_out_of_cycles(drc);

	/* append an INT 3 before the recompile so that BREAK_ON_MODIFIED_CODE works */
	emit_int_3(DRCTOP);
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
			memcpy(drc->lookup_l1[i], drc->lookup_l2_recompile, sizeof(*drc->lookup_l2_recompile) * (1 << drc->l2bits));
	}

	/* call back to the host */
	if (drc->cb_reset)
		(*drc->cb_reset)(drc);
}


/*------------------------------------------------------------------
    drc_execute
------------------------------------------------------------------*/

void drc_execute(drc_core *drc)
{
	(*drc->entry_point)();
}


/*------------------------------------------------------------------
    drc_exit
------------------------------------------------------------------*/

void drc_exit(drc_core *drc)
{
	int i;

	/* free all the l2 tables allocated */
	for (i = 0; i < (1 << drc->l1bits); i++)
		if (drc->lookup_l1[i] != drc->lookup_l2_recompile)
			free(drc->lookup_l1[i]);

	/* free the l1 table */
	if (drc->lookup_l1)
		free(drc->lookup_l1);

	/* free the default l2 table */
	if (drc->lookup_l2_recompile)
		free(drc->lookup_l2_recompile);

	/* free the lists */
	if (drc->sequence_list)
		free(drc->sequence_list);
	if (drc->tentative_list)
		free(drc->tentative_list);

	/* and the drc itself */
	if (drc->cache_allocated)
		osd_free_executable(drc, drc->cache_size + sizeof(*drc));
}


/*------------------------------------------------------------------
    drc_begin_sequence
------------------------------------------------------------------*/

void drc_begin_sequence(drc_core *drc, UINT32 pc)
{
	UINT32 l1index = pc >> drc->l1shift;
	UINT32 l2index = ((pc & drc->l2mask) * drc->l2scale) / 4;

	/* reset the sequence and tentative counts */
	drc->sequence_count = 0;
	drc->tentative_count = 0;

	/* allocate memory if necessary */
	if (drc->lookup_l1[l1index] == drc->lookup_l2_recompile)
	{
		/* create a new copy of the recompile table */
		drc->lookup_l1[l1index] = malloc_or_die(sizeof(*drc->lookup_l2_recompile) * (1 << drc->l2bits));

		memcpy(drc->lookup_l1[l1index], drc->lookup_l2_recompile, sizeof(*drc->lookup_l2_recompile) * (1 << drc->l2bits));
	}

	/* nuke any previous link to this instruction */
	if (drc->lookup_l1[l1index][l2index] != drc->recompile)
	{
		UINT8 *cache_save = drc->cache_top;
		drc->cache_top = drc->lookup_l1[l1index][l2index];
		emit_jmp(DRCTOP, drc->dispatch);
		drc->cache_top = cache_save;
	}

	/* note the current location for this instruction */
	drc->lookup_l1[l1index][l2index] = drc->cache_top;
}


/*------------------------------------------------------------------
    drc_end_sequence
------------------------------------------------------------------*/

void drc_end_sequence(drc_core *drc)
{
	int i, j;

	/* fix up any internal links */
	for (i = 0; i < drc->tentative_count; i++)
		for (j = 0; j < drc->sequence_count; j++)
			if (drc->tentative_list[i].pc == drc->sequence_list[j].pc)
			{
				UINT8 *cache_save = drc->cache_top;
				drc->cache_top = drc->tentative_list[i].target;
				emit_jmp(DRCTOP, drc->sequence_list[j].target);
				drc->cache_top = cache_save;
				break;
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

void *drc_get_code_at_pc(drc_core *drc, UINT32 pc)
{
	UINT32 l1index = pc >> drc->l1shift;
	UINT32 l2index = ((pc & drc->l2mask) * drc->l2scale) / 4;
	return (drc->lookup_l1[l1index][l2index] != drc->recompile) ? drc->lookup_l1[l1index][l2index] : NULL;
}


/*------------------------------------------------------------------
    drc_append_verify_code
------------------------------------------------------------------*/

void drc_append_verify_code(drc_core *drc, void *code, UINT8 length)
{
#if BREAK_ON_MODIFIED_CODE
	x86code *recompile = drc->recompile - 1;
#else
	x86code *recompile = drc->recompile;
#endif

	if (length > 8)
	{
		UINT32 *codeptr = code, sum = 0;
		void *target;
		int i;

		for (i = 0; i < length / 4; i++)
		{
			sum = (sum >> 1) | (sum << 31);
			sum += *codeptr++;
		}

		emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);								// xor  eax,eax
		emit_mov_r32_imm(DRCTOP, REG_EBX, (FPTR)code);							// mov  ebx,code
		emit_mov_r32_imm(DRCTOP, REG_ECX, length / 4);							// mov  ecx,length / 4
		target = drc->cache_top;											// target:
		emit_ror_r32_imm(DRCTOP, REG_EAX, 1);									// ror  eax,1
		emit_add_r32_m32(DRCTOP, REG_EAX, MBD(REG_EBX, 0));						// add  eax,[ebx]
		emit_sub_r32_imm(DRCTOP, REG_ECX, 1);									// sub  ecx,1
		emit_lea_r32_m32(DRCTOP, REG_EBX, MBD(REG_EBX, 4));						// lea  ebx,[ebx+4]
		emit_jcc(DRCTOP, COND_NZ, target);										// jnz  target
		emit_cmp_r32_imm(DRCTOP, REG_EAX, sum);									// cmp  eax,sum
		emit_jcc(DRCTOP, COND_NE, recompile);									// jne  recompile
	}
	else if (length >= 12)
	{
		emit_cmp_m32_imm(DRCTOP, MABS(code), *(UINT32 *)code);					// cmp  [pc],opcode
		emit_jcc(DRCTOP, COND_NE, recompile);									// jne  recompile
		emit_cmp_m32_imm(DRCTOP, MABS((UINT8 *)code + 4), ((UINT32 *)code)[1]);	// cmp  [pc+4],opcode+4
		emit_jcc(DRCTOP, COND_NE, recompile);									// jne  recompile
		emit_cmp_m32_imm(DRCTOP, MABS((UINT8 *)code + 8), ((UINT32 *)code)[2]);	// cmp  [pc+8],opcode+8
		emit_jcc(DRCTOP, COND_NE, recompile);									// jne  recompile
	}
	else if (length >= 8)
	{
		emit_cmp_m32_imm(DRCTOP, MABS(code), *(UINT32 *)code);					// cmp  [pc],opcode
		emit_jcc(DRCTOP, COND_NE, recompile);									// jne  recompile
		emit_cmp_m32_imm(DRCTOP, MABS((UINT8 *)code + 4), ((UINT32 *)code)[1]);	// cmp  [pc+4],opcode+4
		emit_jcc(DRCTOP, COND_NE, recompile);									// jne  recompile
	}
	else if (length >= 4)
	{
		emit_cmp_m32_imm(DRCTOP, MABS(code), *(UINT32 *)code);					// cmp  [pc],opcode
		emit_jcc(DRCTOP, COND_NE, recompile);									// jne  recompile
	}
	else if (length >= 2)
	{
		emit_cmp_m16_imm(DRCTOP, MABS(code), *(UINT16 *)code);					// cmp  [pc],opcode
		emit_jcc(DRCTOP, COND_NE, recompile);									// jne  recompile
	}
	else
	{
		emit_cmp_m8_imm(DRCTOP, MABS(code), *(UINT8 *)code);					// cmp  [pc],opcode
		emit_jcc(DRCTOP, COND_NE, recompile);									// jne  recompile
	}
}


/*------------------------------------------------------------------
    drc_append_call_debugger
------------------------------------------------------------------*/

void drc_append_call_debugger(drc_core *drc)
{
#ifdef MAME_DEBUG
	if (Machine->debug_mode)
	{
		emit_link link;
		emit_cmp_m32_imm(DRCTOP, MABS(&Machine->debug_mode), 0);					// cmp  [Machine->debug_mode],0
		emit_jcc_short_link(DRCTOP, COND_E, &link);								// je   skip
		emit_sub_r32_imm(DRCTOP, REG_ESP, 12);									// align stack
		drc_append_save_call_restore(drc, (x86code *)mame_debug_hook, 12);			// save volatiles
		resolve_link(DRCTOP, &link);
	}
#endif
}


/*------------------------------------------------------------------
    drc_append_save_volatiles
------------------------------------------------------------------*/

void drc_append_save_volatiles(drc_core *drc)
{
	if (drc->icountptr && !drc->icount_in_memory)
		emit_mov_m32_r32(DRCTOP, MABS(drc->icountptr), REG_EBP);
	if (drc->pcptr && !drc->pc_in_memory)
		emit_mov_m32_r32(DRCTOP, MABS(drc->pcptr), REG_EDI);
	if (drc->esiptr)
		emit_mov_m32_r32(DRCTOP, MABS(drc->esiptr), REG_ESI);
}


/*------------------------------------------------------------------
    drc_append_restore_volatiles
------------------------------------------------------------------*/

void drc_append_restore_volatiles(drc_core *drc)
{
	if (drc->icountptr && !drc->icount_in_memory)
		emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(drc->icountptr));
	if (drc->pcptr && !drc->pc_in_memory)
		emit_mov_r32_m32(DRCTOP, REG_EDI, MABS(drc->pcptr));
	if (drc->esiptr)
		emit_mov_r32_m32(DRCTOP, REG_ESI, MABS(drc->esiptr));
}


/*------------------------------------------------------------------
    drc_append_save_call_restore
------------------------------------------------------------------*/

void drc_append_save_call_restore(drc_core *drc, x86code *target, UINT32 stackadj)
{
	drc_append_save_volatiles(drc);												// save volatiles
	emit_call(DRCTOP, target);													// call target
	drc_append_restore_volatiles(drc);											// restore volatiles
	if (stackadj)
		emit_add_r32_imm(DRCTOP, REG_ESP, stackadj);							// adjust stack
}


/*------------------------------------------------------------------
    drc_append_standard_epilogue
------------------------------------------------------------------*/

void drc_append_standard_epilogue(drc_core *drc, INT32 cycles, INT32 pcdelta, int allow_exit)
{
	if (pcdelta != 0 && drc->pc_in_memory)
		emit_add_m32_imm(DRCTOP, MABS(drc->pcptr), pcdelta);						// add  [pc],pcdelta
	if (cycles != 0)
	{
		if (drc->icount_in_memory)
			emit_sub_m32_imm(DRCTOP, MABS(drc->icountptr), cycles);				// sub  [icount],cycles
		else
			emit_sub_r32_imm(DRCTOP, REG_EBP, cycles);					// sub  ebp,cycles
	}
	if (pcdelta != 0 && !drc->pc_in_memory)
		emit_lea_r32_m32(DRCTOP, REG_EDI, MBD(REG_EDI, pcdelta));					// lea  edi,[edi+pcdelta]
	if (allow_exit && cycles != 0)
		emit_jcc(DRCTOP, COND_S, drc->out_of_cycles);							// js   out_of_cycles
}


/*------------------------------------------------------------------
    drc_append_dispatcher
------------------------------------------------------------------*/

void drc_append_dispatcher(drc_core *drc)
{
#if LOG_DISPATCHES
	emit_sub_r32_imm(DRCTOP, REG_ESP, 8);										// align stack
	emit_push_imm(DRCTOP, drc);													// push drc
	drc_append_save_call_restore(drc, (x86code *)log_dispatch, 12);				// call log_dispatch
#endif
	if (drc->pc_in_memory)
		emit_mov_r32_m32(DRCTOP, REG_EDI, MABS(drc->pcptr));						// mov  edi,[pc]
	emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EDI);									// mov  eax,edi
	emit_shr_r32_imm(DRCTOP, REG_EAX, drc->l1shift);							// shr  eax,l1shift
	emit_mov_r32_r32(DRCTOP, REG_EDX, REG_EDI);									// mov  edx,edi
	emit_mov_r32_m32(DRCTOP, REG_EAX, MISD(REG_EAX, 4, (FPTR)drc->lookup_l1));		// mov  eax,[eax*4 + l1lookup]
	emit_and_r32_imm(DRCTOP, REG_EDX, drc->l2mask);								// and  edx,l2mask
	emit_jmp_m32(DRCTOP, MBISD(REG_EAX, REG_EDX, drc->l2scale, 0));				// jmp  [eax+edx*l2scale]
}


/*------------------------------------------------------------------
    drc_append_fixed_dispatcher
------------------------------------------------------------------*/

void drc_append_fixed_dispatcher(drc_core *drc, UINT32 newpc)
{
	x86code **base = drc->lookup_l1[newpc >> drc->l1shift];
	if (base == drc->lookup_l2_recompile)
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&drc->lookup_l1[newpc >> drc->l1shift]));
																				// mov eax,[(newpc >> l1shift)*4 + l1lookup]
		emit_jmp_m32(DRCTOP, MBD(REG_EAX, (newpc & drc->l2mask) * drc->l2scale));
																				// jmp  [eax+(newpc & l2mask)*l2scale]
	}
	else
		emit_jmp_m32(DRCTOP, MABS((UINT8 *)base + (newpc & drc->l2mask) * drc->l2scale));
																				// jmp  [eax+(newpc & l2mask)*l2scale]
}


/*------------------------------------------------------------------
    drc_append_tentative_fixed_dispatcher
------------------------------------------------------------------*/

void drc_append_tentative_fixed_dispatcher(drc_core *drc, UINT32 newpc)
{
	pc_ptr_pair *pair = &drc->tentative_list[drc->tentative_count++];
	assert_always(drc->tentative_count <= drc->tentative_count_max, "drc_append_tentative_fixed_dispatcher: too many tentative branches!");

	pair->target = drc->cache_top;
	pair->pc = newpc;
	drc_append_fixed_dispatcher(drc, newpc);
}


/*------------------------------------------------------------------
    drc_append_set_fp_rounding
------------------------------------------------------------------*/

void drc_append_set_fp_rounding(drc_core *drc, UINT8 regindex)
{
	emit_fldcw_m16(DRCTOP, MISD(regindex, 2, (INT32)&fp_control[0]));				// fldcw [fp_control + reg*2]
	emit_fstcw_m16(DRCTOP, MABS(&drc->fpcw_curr));									// fnstcw [fpcw_curr]
}



/*------------------------------------------------------------------
    drc_append_set_temp_fp_rounding
------------------------------------------------------------------*/

void drc_append_set_temp_fp_rounding(drc_core *drc, UINT8 rounding)
{
	emit_fldcw_m16(DRCTOP, MABS(&fp_control[rounding]));							// fldcw [fp_control]
}



/*------------------------------------------------------------------
    drc_append_restore_fp_rounding
------------------------------------------------------------------*/

void drc_append_restore_fp_rounding(drc_core *drc)
{
	emit_fldcw_m16(DRCTOP, MABS(&drc->fpcw_curr));									// fldcw [fpcw_curr]
}



/*------------------------------------------------------------------
    drc_append_set_sse_rounding
------------------------------------------------------------------*/

void drc_append_set_sse_rounding(drc_core *drc, UINT8 regindex)
{
	emit_ldmxcsr_m32(DRCTOP, MISD(regindex, 4, (INT32)&sse_control[0]));			// ldmxcsr [sse_control + reg*2]
	emit_stmxcsr_m32(DRCTOP, MABS(&drc->mxcsr_curr));								// stmxcsr [mxcsr_curr]
}



/*------------------------------------------------------------------
    drc_append_set_temp_sse_rounding
------------------------------------------------------------------*/

void drc_append_set_temp_sse_rounding(drc_core *drc, UINT8 rounding)
{
	emit_ldmxcsr_m32(DRCTOP, MABS(&sse_control[rounding]));						// ldmxcsr [sse_control]
}



/*------------------------------------------------------------------
    drc_append_restore_sse_rounding
------------------------------------------------------------------*/

void drc_append_restore_sse_rounding(drc_core *drc)
{
	emit_ldmxcsr_m32(DRCTOP, MABS(&drc->mxcsr_curr));								// ldmxcsr [mxcsr_curr]
}



/*------------------------------------------------------------------
    drc_dasm

    An attempt to make a disassembler for DRC code; currently limited
    by the functionality of DasmI386
------------------------------------------------------------------*/

void drc_dasm(FILE *f, const void *begin, const void *end)
{
	extern int i386_dasm_one(char *buffer, UINT32 eip, UINT8 *oprom, int mode);

	char buffer[256];
	const UINT8 *begin_ptr = (const UINT8 *) begin;
	const UINT8 *end_ptr = (const UINT8 *) end;
	UINT32 pc = (UINT32) begin;
	int length;

	while(begin_ptr < end_ptr)
	{
#if defined(MAME_DEBUG) && HAS_I386
		length = i386_dasm_one(buffer, pc, (UINT8 *) begin_ptr, 32) & DASMFLAG_LENGTHMASK;
#else
		sprintf(buffer, "%02X", *begin_ptr);
		length = 1;
#endif

		fprintf(f, "%08X:\t%s\n", (unsigned) pc, buffer);
		begin_ptr += length;
		pc += length;
	}
}




/***************************************************************************
    INTERNAL CODEGEN
***************************************************************************/

/*------------------------------------------------------------------
    append_entry_point
------------------------------------------------------------------*/

static void append_entry_point(drc_core *drc)
{
	emit_pushad(DRCTOP);														// pushad
	if (drc->uses_fp)
	{
		emit_fstcw_m16(DRCTOP, MABS(&drc->fpcw_save));								// fstcw [fpcw_save]
		emit_fldcw_m16(DRCTOP, MABS(&drc->fpcw_curr));								// fldcw [fpcw_curr]
	}
	if (drc->uses_sse)
	{
		emit_stmxcsr_m32(DRCTOP, MABS(&drc->mxcsr_save));							// stmxcsr [mxcsr_save]
		emit_ldmxcsr_m32(DRCTOP, MABS(&drc->mxcsr_curr));							// ldmxcsr [mxcsr_curr]
	}
	drc_append_restore_volatiles(drc);											// load volatiles
	if (drc->cb_entrygen)
		(*drc->cb_entrygen)(drc);												// additional entry point duties
	drc_append_dispatcher(drc);													// dispatch
}


/*------------------------------------------------------------------
    recompile_code
------------------------------------------------------------------*/

static void recompile_code(drc_core *drc)
{
	if (drc->cache_top >= drc->cache_danger)
		drc_cache_reset(drc);
	(*drc->cb_recompile)(drc);
}


/*------------------------------------------------------------------
    append_recompile
------------------------------------------------------------------*/

static void append_recompile(drc_core *drc)
{
	emit_sub_r32_imm(DRCTOP, REG_ESP, 8);										// align stack
	emit_push_imm(DRCTOP, (FPTR)drc);													// push drc
	drc_append_save_call_restore(drc, (x86code *)recompile_code, 12);			// call recompile_code
	drc_append_dispatcher(drc);													// dispatch
}


/*------------------------------------------------------------------
    append_flush
------------------------------------------------------------------*/

static void append_flush(drc_core *drc)
{
	emit_sub_r32_imm(DRCTOP, REG_ESP, 8);										// align stack
	emit_push_imm(DRCTOP, (FPTR)drc);													// push drc
	drc_append_save_call_restore(drc, (x86code *)drc_cache_reset, 12);			// call drc_cache_reset
	drc_append_dispatcher(drc);													// dispatch
}


/*------------------------------------------------------------------
    append_out_of_cycles
------------------------------------------------------------------*/

static void append_out_of_cycles(drc_core *drc)
{
	drc_append_save_volatiles(drc);												// save volatiles
	if (drc->uses_fp)
	{
		emit_fclex(DRCTOP);														// fnclex
		emit_fldcw_m16(DRCTOP, MABS(&drc->fpcw_save));								// fldcw [fpcw_save]
	}
	if (drc->uses_sse)
		emit_ldmxcsr_m32(DRCTOP, MABS(&drc->mxcsr_save));							// ldmxcsr [mxcsr_save]
	emit_popad(DRCTOP);															// popad
	emit_ret(DRCTOP);															// ret
}



/*------------------------------------------------------------------
    drc_x86_get_features()
------------------------------------------------------------------*/
UINT32 drc_x86_get_features(void)
{
	UINT32 features = 0;
#ifdef _MSC_VER
	__asm
	{
		mov eax, 1
		xor ebx, ebx
		xor ecx, ecx
		xor edx, edx
		__asm _emit 0Fh __asm _emit 0A2h	// cpuid
		mov features, edx
	}
#else /* !_MSC_VER */
	__asm__
	(
		"pushl %%ebx         ; "
		"movl $1,%%eax       ; "
		"xorl %%ebx,%%ebx    ; "
		"xorl %%ecx,%%ecx    ; "
		"xorl %%edx,%%edx    ; "
		"cpuid               ; "
		"movl %%edx,%0       ; "
		"popl %%ebx          ; "
	: "=&a" (features)		/* result has to go in eax */
	: 				/* no inputs */
	: "%ecx", "%edx"	/* clobbers ebx, ecx and edx */
	);
#endif /* MSC_VER */
	return features;
}



/*------------------------------------------------------------------
    log_dispatch
------------------------------------------------------------------*/

#if LOG_DISPATCHES
static void log_dispatch(drc_core *drc)
{
	if (input_code_pressed(KEYCODE_D))
		logerror("Disp:%08X\n", *drc->pcptr);
}
#endif
