// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/* PowerPC common opcodes */

// it really seems like this should be elsewhere - like maybe the floating point checks can hang out someplace else
#include <math.h>

#define USE_SSE2            0
#define COMPILE_FPU         0

/* recompiler flags */
#define RECOMPILE_UNIMPLEMENTED         0x0000
#define RECOMPILE_SUCCESSFUL            0x0001
#define RECOMPILE_SUCCESSFUL_CP(c,p)    (RECOMPILE_SUCCESSFUL | (((c) & 0xff) << 16) | (((p) & 0xff) << 24))
#define RECOMPILE_END_OF_STRING         0x0002
#define RECOMPILE_ADD_DISPATCH          0x0004

INLINE void emit_mov_r64_m64(x86code **emitptr, UINT8 reghi, UINT8 reglo, DECLARE_MEMPARAMS)
{
	emit_mov_r32_m32(emitptr, reglo, MEMPARAMS);
	emit_mov_r32_m32(emitptr, reghi, base, index, scale, disp+4);
}

INLINE void emit_mov_m64_r64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 reghi, UINT8 reglo)
{
	emit_mov_m32_r32(emitptr, MEMPARAMS, reglo);
	emit_mov_m32_r32(emitptr, base, index, scale, disp+4, reghi);
}


static UINT32 compile_one(drc_core *drc, UINT32 pc);

static void append_generate_exception(drc_core *drc, UINT8 exception);
static void append_check_interrupts(drc_core *drc, int inline_generate);
static UINT32 recompile_instruction(drc_core *drc, UINT32 pc, UINT32 *opptr);

static UINT32 temp_ppc_pc;

static void ppcdrc_init(void)
{
	drc_config drconfig;

	/* fill in the config */
	memset(&drconfig, 0, sizeof(drconfig));
	drconfig.cache_size       = CACHE_SIZE;
	drconfig.max_instructions = MAX_INSTRUCTIONS;
	drconfig.address_bits     = 32;
	drconfig.lsbs_to_ignore   = 2;
	drconfig.uses_fp          = 1;
	drconfig.uses_sse         = USE_SSE2;
	drconfig.pc_in_memory     = 0;
	drconfig.icount_in_memory = 0;
	drconfig.pcptr            = (UINT32 *)&ppc.pc;
	drconfig.icountptr        = (UINT32 *)&ppc_icount;
	drconfig.esiptr           = NULL;
	drconfig.cb_reset         = CPU_RESET_NAME(ppcdrc);
	drconfig.cb_recompile     = ppcdrc_recompile;
	drconfig.cb_entrygen      = ppcdrc_entrygen;

	/* initialize the compiler */
	ppc.drc = drc_init(cpunum_get_active(), &drconfig);
	ppc.drcoptions = 0;
}

static void ppcdrc_reset(drc_core *drc)
{
	code_log_reset();

	code_log("entry_point:", (x86code *)drc->entry_point, drc->out_of_cycles);
	code_log("out_of_cycles:", drc->out_of_cycles, drc->recompile);
	code_log("recompile:", drc->recompile, drc->dispatch);
	code_log("dispatch:", drc->dispatch, drc->flush);
	code_log("flush:", drc->flush, drc->cache_top);

	ppc.invoke_exception_handler = drc->cache_top;
	drc_append_restore_volatiles(drc);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 4));
	emit_mov_r32_m32(DRCTOP, REG_ESP, MABS(&ppc.host_esp));
	emit_mov_m32_r32(DRCTOP, MABS(&SRR0), REG_EDI);     /* save return address */
	emit_jmp_r32(DRCTOP, REG_EAX);
	code_log("invoke_exception_handler:", ppc.invoke_exception_handler, drc->cache_top);

	ppc.generate_interrupt_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_IRQ);
	code_log("generate_interrupt_exception:", ppc.generate_interrupt_exception, drc->cache_top);

	ppc.generate_syscall_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_SYSTEM_CALL);
	code_log("generate_syscall_exception:", ppc.generate_syscall_exception, drc->cache_top);

	ppc.generate_decrementer_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_DECREMENTER);
	code_log("generate_decrementer_exception:", ppc.generate_decrementer_exception, drc->cache_top);

	ppc.generate_trap_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_TRAP);
	code_log("generate_trap_exception:", ppc.generate_trap_exception, drc->cache_top);

	ppc.generate_dsi_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_DSI);
	code_log("generate_dsi_exception:", ppc.generate_dsi_exception, drc->cache_top);

	ppc.generate_isi_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_ISI);
	code_log("generate_isi_exception:", ppc.generate_isi_exception, drc->cache_top);

	if (!ppc.is603 && !ppc.is602)
	{
		ppc.generate_fit_exception = drc->cache_top;
		append_generate_exception(drc, EXCEPTION_FIXED_INTERVAL_TIMER);
		code_log("generate_fit_exception:", ppc.generate_fit_exception, drc->cache_top);
	}
}

static CPU_EXIT( ppcdrc )
{
	drc_exit(ppc.drc);
}

static UINT32 *ppcdrc_getopptr(UINT32 address)
{
	UINT32 *result;
	UINT32 offset = 0;

	if (ppc.is603 || ppc.is602)
	{
		if (MSR & MSR_IR)
		{
			if (!ppc_translate_address(&address, PPC_TRANSLATE_CODE | PPC_TRANSLATE_READ | PPC_TRANSLATE_NOEXCEPTION))
				return NULL;
		}
		address = DWORD_XOR_BE(address);
		offset = (address & 0x07) / sizeof(*result);
		address &= ~0x07;
	}

	result = (UINT32 *) memory_decrypted_read_ptr(ppc.core->program, address);
	if (result)
		result += offset;
	return result;
}

static void ppcdrc_recompile(drc_core *drc)
{
	int remaining = MAX_INSTRUCTIONS;
	x86code *start = drc->cache_top;
	UINT32 pc = ppc.pc;
	UINT32 *opptr;

	(void)start;

	/* begin the sequence */
	drc_begin_sequence(drc, pc);
	code_log_reset();

	/* loose verification case: one verification here only */
	if (!(ppc.drcoptions & PPCDRC_OPTIONS_CHECK_SELFMOD_CODE))
	{
		opptr = ppcdrc_getopptr(pc);
		if (opptr)
			drc_append_verify_code(drc, opptr, 4);
	}

	/* loop until we hit an unconditional branch */
	while (--remaining != 0)
	{
		UINT32 result;

		/* compile one instruction */
		result = compile_one(drc, pc);
		pc += (INT8)(result >> 24);
		if (result & RECOMPILE_END_OF_STRING)
			break;

		/* do not recompile across MMU page boundaries */
		if ((pc & 0x0FFF) == 0)
		{
			remaining = 0;
			break;
		}
	}

	/* add dispatcher just in case */
	if (remaining == 0)
		drc_append_dispatcher(drc);

	/* end the sequence */
	drc_end_sequence(drc);

if (0)
{
	char label[40];
	sprintf(label, "Code @ %08X", ppc.pc);
	code_log(label, start, drc->cache_top);
}
}

static void update_counters(drc_core *drc)
{
	emit_link link1;

	/* decrementer */
	if (ppc.is603 || ppc.is602)
	{
		emit_cmp_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_dec_trigger_cycle));
		emit_jcc_short_link(DRCTOP, COND_NZ, &link1);
		emit_or_m32_imm(DRCTOP, MABS(&ppc.exception_pending), 0x2);
		resolve_link(DRCTOP, &link1);
	}

	/* FIT */
	if (!ppc.is603 && !ppc.is602)
	{
		emit_link link2;
		emit_cmp_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_fit_trigger_cycle));
		emit_jcc_short_link(DRCTOP, COND_NZ, &link1);
		emit_cmp_r32_m32(DRCTOP, REG_EBP, MABS(&ppc.fit_int_enable));
		emit_jcc_short_link(DRCTOP, COND_Z, &link2);
		emit_or_m32_imm(DRCTOP, MABS(&ppc.exception_pending), 0x4);
		resolve_link(DRCTOP, &link1);
		resolve_link(DRCTOP, &link2);
	}
}

static void ppcdrc_entrygen(drc_core *drc)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc.host_esp), REG_ESP);
	append_check_interrupts(drc, 0);
}

static UINT32 compile_one(drc_core *drc, UINT32 pc)
{
	int pcdelta, cycles;
	UINT32 *opptr;
	UINT32 result;

	/* register this instruction */
	drc_register_code_at_cache_top(drc, pc);

	/* get a pointer to the current instruction */
	opptr = ppcdrc_getopptr(pc);

	//log_symbol(drc, ~2);
	//log_symbol(drc, pc);

	/* emit debugging call */
	drc_append_call_debugger(drc);

	/* null opptr?  if legit, we need to generate an ISI exception */
	if (!opptr)
	{
		/* first check to see if the code is up to date; if not, recompile */
		emit_push_imm(DRCTOP, pc);
		emit_call(DRCTOP, (x86code *)ppcdrc_getopptr);
		emit_add_r32_imm(DRCTOP, REG_ESP, 4);
		emit_cmp_r32_imm(DRCTOP, REG_EAX, 0);
		emit_jcc(DRCTOP, COND_NZ, drc->recompile);

		/* code is up to date; do the exception */
		emit_mov_m32_r32(DRCTOP, MABS(&SRR0), REG_EDI);     /* save return address */
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.generate_isi_exception));
		emit_jmp_r32(DRCTOP, REG_EAX);
		return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
	}

	/* emit self-modifying code checks */
	if (ppc.drcoptions & PPCDRC_OPTIONS_CHECK_SELFMOD_CODE)
	{
		drc_append_verify_code(drc, opptr, 4);
	}

	/* compile the instruction */
	result = recompile_instruction(drc, pc, opptr);

	/* handle the results */
	if (!(result & RECOMPILE_SUCCESSFUL))
		fatalerror("Unimplemented op %08X\n", *opptr);

	pcdelta = (INT8)(result >> 24);
	cycles = (INT8)(result >> 16);

	/* epilogue */
	update_counters(drc);
	drc_append_standard_epilogue(drc, cycles, pcdelta, 1);

	if (result & RECOMPILE_ADD_DISPATCH)
		drc_append_dispatcher(drc);

	return (result & 0xffff) | ((UINT8)cycles << 16) | ((UINT8)pcdelta << 24);
}

static UINT32 recompile_instruction(drc_core *drc, UINT32 pc, UINT32 *opptr)
{
	UINT32 opcode;
	temp_ppc_pc = pc;

	opcode = *opptr;

	code_log_add_entry(pc, opcode, drc->cache_top);

	if (opcode != 0) {      // this is a little workaround for VF3
		switch(opcode >> 26)
		{
			case 19:    return ppc.optable19[(opcode >> 1) & 0x3ff](drc, opcode);
			case 31:    return ppc.optable31[(opcode >> 1) & 0x3ff](drc, opcode);
			case 59:    return ppc.optable59[(opcode >> 1) & 0x3ff](drc, opcode);
			case 63:    return ppc.optable63[(opcode >> 1) & 0x3ff](drc, opcode);
			default:    return ppc.optable[opcode >> 26](drc, opcode);
		}
	}
	return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
}


static const UINT32 exception_vector[32] =
{
	0x0000, 0x0500, 0x0900, 0x0700, 0x0c00, 0x1400, 0x0300, 0x0400,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x1000, 0x1010, 0x1020
};

static void append_generate_exception(drc_core *drc, UINT8 exception)
{
	emit_link link1, link2, link3;

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.msr));
	emit_and_r32_imm(DRCTOP, REG_EAX, 0xff73);
	emit_mov_m32_r32(DRCTOP, MABS(&SRR1), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.msr));

	// Clear POW, EE, PR, FP, FE0, SE, BE, FE1, IR, DR, RI
	emit_and_r32_imm(DRCTOP, REG_EAX, ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI));
	// Set LE to ILE
	emit_and_r32_imm(DRCTOP, REG_EAX, ~MSR_LE);     // clear LE first
	emit_test_r32_imm(DRCTOP, REG_EAX, MSR_ILE);
	emit_jcc_short_link(DRCTOP, COND_Z, &link1);    // if Z == 0, bit == 1
	emit_or_r32_imm(DRCTOP, REG_EAX, MSR_LE);       // set LE
	resolve_link(DRCTOP, &link1);
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)ppc_set_msr);
	emit_pop_r32(DRCTOP, REG_EDX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	if (ppc.is603)
	{
		emit_mov_r32_imm(DRCTOP, REG_EDI, exception_vector[exception]);     // first move the exception handler offset
		emit_test_r32_imm(DRCTOP, REG_EDX, MSR_IP);                         // test if the base should be 0xfff0 or EVPR
		emit_jcc_short_link(DRCTOP, COND_Z, &link2);                        // if Z == 1, bit == 0 means base == 0x00000000
		emit_or_r32_imm(DRCTOP, REG_EDI, 0xfff00000);                       // else base == 0xfff00000
		resolve_link(DRCTOP, &link2);
	}
	else if (ppc.is602)
	{
		emit_mov_r32_imm(DRCTOP, REG_EDI, exception_vector[exception]);     // first move the exception handler offset
		emit_test_r32_imm(DRCTOP, REG_EDX, MSR_IP);                         // test if the base should be 0xfff0 or IBR
		emit_jcc_short_link(DRCTOP, COND_NZ, &link2);                       // if Z == 0, bit == 1 means base == 0xfff00000
		emit_or_r32_m32(DRCTOP, REG_EDI, MABS(&ppc.ibr));                       // else base == IBR
		emit_jmp_short_link(DRCTOP, &link3);
		resolve_link(DRCTOP, &link2);
		emit_or_r32_imm(DRCTOP, REG_EDI, 0xfff00000);
		resolve_link(DRCTOP, &link3);
	}
	else
	{
		emit_mov_r32_imm(DRCTOP, REG_EDI, exception_vector[exception]);     // first move the exception handler offset
		emit_test_r32_imm(DRCTOP, REG_EDX, MSR_IP);                         // test if the base should be 0xfff0 or EVPR
		emit_jcc_short_link(DRCTOP, COND_NZ, &link2);                       // if Z == 0, bit == 1 means base == 0xfff00000
		emit_or_r32_m32(DRCTOP, REG_EDI, MABS(&EVPR));                          // else base == EVPR
		emit_jmp_short_link(DRCTOP, &link3);
		resolve_link(DRCTOP, &link2);
		emit_or_r32_imm(DRCTOP, REG_EDI, 0xfff00000);
		resolve_link(DRCTOP, &link3);
	}

	if (exception == EXCEPTION_IRQ)
	{
		emit_and_m32_imm(DRCTOP, MABS(&ppc.exception_pending), ~0x1);       // clear pending irq
	}
	if (exception == EXCEPTION_DECREMENTER)
	{
		emit_and_m32_imm(DRCTOP, MABS(&ppc.exception_pending), ~0x2);       // clear pending decrementer exception
	}
	if (exception == EXCEPTION_FIXED_INTERVAL_TIMER)
	{
		emit_and_m32_imm(DRCTOP, MABS(&ppc.exception_pending), ~0x4);       // clear pending fit exception
	}

	drc_append_dispatcher(drc);
}

static void append_check_interrupts(drc_core *drc, int inline_generate)
{
	if (ppc.is602 || ppc.is603)
	{
		emit_link link1, link2, link3, link4;
		emit_test_m32_imm(DRCTOP, MABS(&ppc.msr), MSR_EE);      /* no interrupt if external interrupts are not enabled */
		emit_jcc_short_link(DRCTOP, COND_Z, &link1);        /* ZF = 1 if bit == 0 */

		/* else check if any interrupt are pending */
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.exception_pending));
		emit_cmp_r32_imm(DRCTOP, REG_EAX, 0);
		emit_jcc_short_link(DRCTOP, COND_Z, &link2);        /* reg == 0, no exceptions are pending */

		/* else handle the first pending exception */
		emit_test_r32_imm(DRCTOP, REG_EAX, 0x1);            /* is it a IRQ? */
		emit_jcc_short_link(DRCTOP, COND_Z, &link3);

		emit_mov_m32_r32(DRCTOP, MABS(&SRR0), REG_EDI);     /* save return address */
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.generate_interrupt_exception));
		emit_jmp_r32(DRCTOP, REG_EAX);
		resolve_link(DRCTOP, &link3);

		emit_test_r32_imm(DRCTOP, REG_EAX, 0x2);            /* is it a decrementer exception */
		emit_jcc_short_link(DRCTOP, COND_Z, &link4);
		emit_mov_m32_r32(DRCTOP, MABS(&SRR0), REG_EDI);     /* save return address */
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.generate_decrementer_exception));
		emit_jmp_r32(DRCTOP, REG_EAX);
		resolve_link(DRCTOP, &link4);

		resolve_link(DRCTOP, &link1);
		resolve_link(DRCTOP, &link2);
	}
	else
	{
		emit_link link1, link2, link3, link4, link5, link6;
		emit_test_m32_imm(DRCTOP, MABS(&ppc.msr), MSR_EE);      /* no interrupt if external interrupts are not enabled */
		emit_jcc_short_link(DRCTOP, COND_Z, &link1);        /* ZF = 1 if bit == 0 */

		/* else check if any interrupt are pending */
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.exception_pending));
		emit_cmp_r32_imm(DRCTOP, REG_EAX, 0);
		emit_jcc_short_link(DRCTOP, COND_Z, &link2);        /* reg == 0, no exceptions are pending */

		/* else handle the first pending exception */
		emit_test_r32_imm(DRCTOP, REG_EAX, 0x1);            /* is it a IRQ? */
		emit_jcc_short_link(DRCTOP, COND_Z, &link3);

		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.exisr));
		emit_and_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.exier));
		emit_cmp_r32_imm(DRCTOP, REG_EAX, 0);
		emit_jcc_short_link(DRCTOP, COND_Z, &link4);

		emit_mov_m32_r32(DRCTOP, MABS(&SRR0), REG_EDI);     /* save return address */
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.generate_interrupt_exception));
		emit_jmp_r32(DRCTOP, REG_EAX);

		/* check if it's FIT exception */
		resolve_link(DRCTOP, &link3);
		emit_test_r32_imm(DRCTOP, REG_EAX, 0x4);
		emit_jcc_short_link(DRCTOP, COND_Z, &link5);

		// check if FIT interrupts are enabled
		emit_test_m32_imm(DRCTOP, MABS(&ppc.fit_int_enable), 0x1);
		emit_jcc_short_link(DRCTOP, COND_Z, &link6);

		// calculate the next trigger cycle for FIT
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc_fit_trigger_cycle));
		emit_sub_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.fit_bit));
		emit_mov_m32_r32(DRCTOP, MABS(&ppc_fit_trigger_cycle), REG_EAX);

		emit_mov_m32_r32(DRCTOP, MABS(&SRR0), REG_EDI);     /* save return address */
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.generate_fit_exception));
		emit_jmp_r32(DRCTOP, REG_EAX);

		resolve_link(DRCTOP, &link1);
		resolve_link(DRCTOP, &link2);
		resolve_link(DRCTOP, &link4);
		resolve_link(DRCTOP, &link5);
		resolve_link(DRCTOP, &link6);
	}
}

static void append_branch_or_dispatch(drc_core *drc, UINT32 newpc, int cycles)
{
	void *code = drc_get_code_at_pc(drc, newpc);
	emit_mov_r32_imm(DRCTOP, REG_EDI, newpc);

	update_counters(drc);
	append_check_interrupts(drc, 0);

	drc_append_standard_epilogue(drc, cycles, 0, 1);


	if (code)
		emit_jmp(DRCTOP, code);
	else
		drc_append_tentative_fixed_dispatcher(drc, newpc);
}

/*
// this table translates x86 SF and ZF flags to PPC CR values
static const UINT8 condition_table[4] =
{
    0x4,    // x86 SF == 0, ZF == 0   -->   PPC GT (positive)
    0x2,    // x86 SF == 0, ZF == 1   -->   PPC EQ (zero)
    0x8,    // x86 SF == 1, ZF == 0   -->   PPC LT (negative)
    0x0,    // x86 SF == 1, ZF == 1 (impossible)
};
*/

// expects the result value in EDX!!!
static void append_set_cr0(drc_core *drc)
{
	emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_cmp_r32_imm(DRCTOP, REG_EDX, 0);
/*
    _lahf();

    _shr_r32_imm(REG_EAX, 14);

    _add_r32_imm(REG_EAX, &condition_table);
    _mov_r8_m8bd(REG_BL, REG_EAX, 0);
*/
	emit_setcc_r8(DRCTOP, COND_Z, REG_AL);
	emit_setcc_r8(DRCTOP, COND_L, REG_AH);
	emit_setcc_r8(DRCTOP, COND_G, REG_BL);
	emit_shl_r8_imm(DRCTOP, REG_AL, 1);
	emit_shl_r8_imm(DRCTOP, REG_AH, 3);
	emit_shl_r8_imm(DRCTOP, REG_BL, 2);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AH);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AL);

	emit_bt_m32_imm(DRCTOP, MABS(&XER), 31);        // set XER SO bit to carry
	emit_adc_r32_imm(DRCTOP, REG_EBX, 0);       // effectively sets bit 0 to carry

	emit_mov_m8_r8(DRCTOP, MABS(&ppc.cr[0]), REG_BL);
}

#ifdef UNUSED_FUNCTION
static void append_set_cr1(drc_core *drc)
{
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.fpscr));
	emit_shr_r32_imm(DRCTOP, REG_EAX, 28);
	emit_and_r32_imm(DRCTOP, REG_EAX, 0xf);
	emit_mov_m8_r8(DRCTOP, MABS(&ppc.cr[1]), REG_AL);
}
#endif

static UINT32 recompile_addx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);

	if (OEBIT) {
		osd_printf_debug("recompile_addx: OE bit set !\n");
		return RECOMPILE_UNIMPLEMENTED;
	}
	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_addcx(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBX, MABS(&XER));
	emit_and_r32_imm(DRCTOP, REG_EBX, ~0x20000000);     // clear carry
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);

	emit_setcc_r8(DRCTOP, COND_C, REG_AL);      // carry to AL
	emit_shl_r32_imm(DRCTOP, REG_EAX, 29);      // shift to carry bit
	emit_or_r32_r32(DRCTOP, REG_EBX, REG_EAX);
	emit_mov_m32_r32(DRCTOP, MABS(&XER), REG_EBX);

	if (OEBIT) {
		osd_printf_debug("recompile_addcx: OE bit set !\n");
		return RECOMPILE_UNIMPLEMENTED;
	}
	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_addex(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_addex);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_addi(drc_core *drc, UINT32 op)
{
	if (RA == 0)
	{
		emit_mov_m32_imm(DRCTOP, MABS(&REG(RT)), SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_addic(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBX, MABS(&XER));
	emit_and_r32_imm(DRCTOP, REG_EBX, ~0x20000000); // clear carry bit
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);

	emit_setcc_r8(DRCTOP, COND_C, REG_AL);          // carry to AL
	emit_shl_r32_imm(DRCTOP, REG_EAX, 29);          // shift to carry bit
	emit_or_r32_r32(DRCTOP, REG_EBX, REG_EAX);
	emit_mov_m32_r32(DRCTOP, MABS(&XER), REG_EBX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_addic_rc(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBX, MABS(&XER));
	emit_and_r32_imm(DRCTOP, REG_EBX, ~0x20000000); // clear carry bit
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);

	emit_setcc_r8(DRCTOP, COND_C, REG_AL);          // carry to AL
	emit_shl_r32_imm(DRCTOP, REG_EAX, 29);          // shift to carry bit
	emit_or_r32_r32(DRCTOP, REG_EBX, REG_EAX);
	emit_mov_m32_r32(DRCTOP, MABS(&XER), REG_EBX);

	append_set_cr0(drc);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_addis(drc_core *drc, UINT32 op)
{
	if (RA == 0)
	{
		emit_mov_m32_imm(DRCTOP, MABS(&REG(RT)), UIMM16 << 16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, UIMM16 << 16);
		emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_addmex(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_addmex);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_addzex(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_addzex);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_andx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_and_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_andcx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	emit_not_r32(DRCTOP, REG_EAX);
	emit_and_r32_r32(DRCTOP, REG_EDX, REG_EAX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_andi_rc(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_and_r32_imm(DRCTOP, REG_EDX, UIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	append_set_cr0(drc);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_andis_rc(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_and_r32_imm(DRCTOP, REG_EDX, UIMM16 << 16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	append_set_cr0(drc);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_bx(drc_core *drc, UINT32 op)
{
	UINT32 newpc;
	INT32 li = op & 0x3fffffc;
	if( li & 0x2000000 )
		li |= 0xfc000000;

	if( AABIT ) {
		newpc = li;
	} else {
		newpc = temp_ppc_pc + li;
	}

	if( LKBIT ) {
		emit_mov_m32_imm(DRCTOP, MABS(&LR), temp_ppc_pc + 4);
	}

	append_branch_or_dispatch(drc, newpc, 1);

	return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
}

static UINT32 recompile_bcx(drc_core *drc, UINT32 op)
{
	emit_link link1 = {0}, link2 = {0};
	int do_link1 = 0, do_link2 = 0;
	UINT32 newpc;

	if( AABIT ) {
		newpc = SIMM16 & ~0x3;
	} else {
		newpc = temp_ppc_pc + (SIMM16 & ~0x3);
	}

	if( LKBIT ) {
		emit_mov_m32_imm(DRCTOP, MABS(&LR), temp_ppc_pc + 4);
	}

	if (BO == 20)       /* condition is always true, so the basic block ends here */
	{
		append_branch_or_dispatch(drc, newpc, 1);

		return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
	}
	else
	{
		// if BO[2] == 0, update CTR and check CTR condition
		if ((BO & 0x4) == 0)
		{
			do_link1 = 1;
			//_dec_m32abs(&CTR);
			emit_sub_m32_imm(DRCTOP, MABS(&CTR), 1);

			// if BO[1] == 0, branch if CTR != 0
			if ((BO & 0x2) == 0)
			{
				emit_jcc_near_link(DRCTOP, COND_Z, &link1);
			}
			else
			{
				emit_jcc_near_link(DRCTOP, COND_NZ, &link1);
			}
		}

		// if BO[0] == 0, check condition
		if ((BO & 0x10) == 0)
		{
			do_link2 = 1;
			emit_movzx_r32_m8(DRCTOP, REG_EAX, MABS(&ppc.cr[(BI)/4]));
			emit_test_r32_imm(DRCTOP, REG_EAX, 1 << (3 - ((BI) & 0x3)));    // test if condition register bit is set

			// if BO[3] == 0, branch if condition == FALSE (bit zero)
			if ((BO & 0x8) == 0)
			{
				emit_jcc_near_link(DRCTOP, COND_NZ, &link2);            // bit not zero, skip branch
			}
			// if BO[3] == 1, branch if condition == TRUE (bit not zero)
			else
			{
				emit_jcc_near_link(DRCTOP, COND_Z, &link2);             // bit zero, skip branch
			}
		}

		// take the branch
		append_branch_or_dispatch(drc, newpc, 1);

		// skip the branch
		if (do_link1) {
			resolve_link(DRCTOP, &link1);
		}
		if (do_link2) {
			resolve_link(DRCTOP, &link2);
		}

		return RECOMPILE_SUCCESSFUL_CP(1,4);
	}
}

static UINT32 recompile_bcctrx(drc_core *drc, UINT32 op)
{
	emit_link link1 = {0} ,link2 = {0};
	int do_link1 = 0, do_link2 = 0;

	if (BO == 20)       /* condition is always true, so the basic block ends here */
	{
		emit_mov_r32_m32(DRCTOP, REG_EDI, MABS(&CTR));                  // mov  edi, CTR

		if( LKBIT ) {
			emit_mov_m32_imm(DRCTOP, MABS(&LR), temp_ppc_pc + 4);
		}
		return RECOMPILE_SUCCESSFUL_CP(1,0) | RECOMPILE_END_OF_STRING | RECOMPILE_ADD_DISPATCH;
	}
	else
	{
		// if BO[2] == 0, update CTR and check CTR condition
		if ((BO & 0x4) == 0)
		{
			do_link1 = 1;
			//_dec_m32abs(&CTR);
			emit_sub_m32_imm(DRCTOP, MABS(&CTR), 1);

			// if BO[1] == 0, branch if CTR != 0
			if ((BO & 0x2) == 0)
			{
				emit_jcc_near_link(DRCTOP, COND_Z, &link1);
			}
			else
			{
				emit_jcc_near_link(DRCTOP, COND_NZ, &link1);
			}
		}

		// if BO[0] == 0, check condition
		if ((BO & 0x10) == 0)
		{
			do_link2 = 1;
			emit_movzx_r32_m8(DRCTOP, REG_EAX, MABS(&ppc.cr[(BI)/4]));
			emit_test_r32_imm(DRCTOP, REG_EAX, 1 << (3 - ((BI) & 0x3)));    // test if condition register bit is set

			// if BO[3] == 0, branch if condition == FALSE (bit zero)
			if ((BO & 0x8) == 0)
			{
				emit_jcc_near_link(DRCTOP, COND_NZ, &link2);            // bit not zero, skip branch
			}
			// if BO[3] == 1, branch if condition == TRUE (bit not zero)
			else
			{
				emit_jcc_near_link(DRCTOP, COND_Z, &link2);             // bit zero, skip branch
			}
		}

		// take the branch
		emit_mov_r32_m32(DRCTOP, REG_EDI, MABS(&CTR));      // mov  edi, CTR
		if( LKBIT ) {
			emit_mov_m32_imm(DRCTOP, MABS(&LR), temp_ppc_pc + 4);
		}
		append_check_interrupts(drc, 0);
		drc_append_standard_epilogue(drc, 1, 0, 1);
		drc_append_dispatcher(drc);

		// skip the branch
		if (do_link1) {
			resolve_link(DRCTOP, &link1);
		}
		if (do_link2) {
			resolve_link(DRCTOP, &link2);
		}
		if( LKBIT ) {
			emit_mov_m32_imm(DRCTOP, MABS(&LR), temp_ppc_pc + 4);
		}

		return RECOMPILE_SUCCESSFUL_CP(1,4);
	}
}

static UINT32 recompile_bclrx(drc_core *drc, UINT32 op)
{
	emit_link link1 = {0}, link2 = {0};
	int do_link1 = 0, do_link2 = 0;

	if (BO == 20)       /* condition is always true, so the basic block ends here */
	{
		emit_mov_r32_m32(DRCTOP, REG_EDI, MABS(&LR));                   // mov  edi, LR

		if( LKBIT ) {
			emit_mov_m32_imm(DRCTOP, MABS(&LR), temp_ppc_pc + 4);
		}
		return RECOMPILE_SUCCESSFUL_CP(1,0) | RECOMPILE_END_OF_STRING | RECOMPILE_ADD_DISPATCH;
	}
	else
	{
		// if BO[2] == 0, update CTR and check CTR condition
		if ((BO & 0x4) == 0)
		{
			do_link1 = 1;
			//_dec_m32abs(&CTR);
			emit_sub_m32_imm(DRCTOP, MABS(&CTR), 1);

			// if BO[1] == 0, branch if CTR != 0
			if ((BO & 0x2) == 0)
			{
				emit_jcc_near_link(DRCTOP, COND_Z, &link1);
			}
			else
			{
				emit_jcc_near_link(DRCTOP, COND_NZ, &link1);
			}
		}

		// if BO[0] == 0, check condition
		if ((BO & 0x10) == 0)
		{
			do_link2 = 1;
			emit_movzx_r32_m8(DRCTOP, REG_EAX, MABS(&ppc.cr[(BI)/4]));
			emit_test_r32_imm(DRCTOP, REG_EAX, 1 << (3 - ((BI) & 0x3)));    // test if condition register bit is set

			// if BO[3] == 0, branch if condition == FALSE (bit zero)
			if ((BO & 0x8) == 0)
			{
				emit_jcc_near_link(DRCTOP, COND_NZ, &link2);            // bit not zero, skip branch
			}
			// if BO[3] == 1, branch if condition == TRUE (bit not zero)
			else
			{
				emit_jcc_near_link(DRCTOP, COND_Z, &link2);             // bit zero, skip branch
			}
		}

		// take the branch
		emit_mov_r32_m32(DRCTOP, REG_EDI, MABS(&LR));       // mov  edi, LR
		if( LKBIT ) {
			emit_mov_m32_imm(DRCTOP, MABS(&LR), temp_ppc_pc + 4);
		}
		append_check_interrupts(drc, 0);
		drc_append_standard_epilogue(drc, 1, 0, 1);
		drc_append_dispatcher(drc);

		// skip the branch
		if (do_link1) {
			resolve_link(DRCTOP, &link1);
		}
		if (do_link2) {
			resolve_link(DRCTOP, &link2);
		}
		if( LKBIT ) {
			emit_mov_m32_imm(DRCTOP, MABS(&LR), temp_ppc_pc + 4);
		}

		return RECOMPILE_SUCCESSFUL_CP(1,4);
	}
}

static UINT32 recompile_cmp(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);
	emit_mov_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RA)));
	emit_cmp_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RB)));

	emit_setcc_r8(DRCTOP, COND_Z, REG_AL);
	emit_setcc_r8(DRCTOP, COND_L, REG_AH);
	emit_setcc_r8(DRCTOP, COND_G, REG_BL);
	emit_shl_r8_imm(DRCTOP, REG_AL, 1);
	emit_shl_r8_imm(DRCTOP, REG_AH, 3);
	emit_shl_r8_imm(DRCTOP, REG_BL, 2);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AH);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AL);

	emit_bt_m32_imm(DRCTOP, MABS(&XER), 31);        // set XER SO bit to carry
	emit_adc_r32_imm(DRCTOP, REG_EBX, 0);       // effectively sets bit 0 to carry
	emit_mov_m8_r8(DRCTOP, MABS(&ppc.cr[CRFD]), REG_BL);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_cmpi(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);
	emit_mov_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RA)));
	emit_cmp_r32_imm(DRCTOP, REG_ECX, SIMM16);

	emit_setcc_r8(DRCTOP, COND_Z, REG_AL);
	emit_setcc_r8(DRCTOP, COND_L, REG_AH);
	emit_setcc_r8(DRCTOP, COND_G, REG_BL);
	emit_shl_r8_imm(DRCTOP, REG_AL, 1);
	emit_shl_r8_imm(DRCTOP, REG_AH, 3);
	emit_shl_r8_imm(DRCTOP, REG_BL, 2);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AH);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AL);

	emit_bt_m32_imm(DRCTOP, MABS(&XER), 31);        // set XER SO bit to carry
	emit_adc_r32_imm(DRCTOP, REG_EBX, 0);       // effectively sets bit 0 to carry
	emit_mov_m8_r8(DRCTOP, MABS(&ppc.cr[CRFD]), REG_BL);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_cmpl(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);
	emit_mov_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RA)));
	emit_cmp_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RB)));

	emit_setcc_r8(DRCTOP, COND_Z, REG_AL);
	emit_setcc_r8(DRCTOP, COND_B, REG_AH);
	emit_setcc_r8(DRCTOP, COND_A, REG_BL);
	emit_shl_r8_imm(DRCTOP, REG_AL, 1);
	emit_shl_r8_imm(DRCTOP, REG_AH, 3);
	emit_shl_r8_imm(DRCTOP, REG_BL, 2);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AH);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AL);

	emit_bt_m32_imm(DRCTOP, MABS(&XER), 31);        // set XER SO bit to carry
	emit_adc_r32_imm(DRCTOP, REG_EBX, 0);       // effectively sets bit 0 to carry
	emit_mov_m8_r8(DRCTOP, MABS(&ppc.cr[CRFD]), REG_BL);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_cmpli(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);
	emit_mov_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RA)));
	emit_cmp_r32_imm(DRCTOP, REG_ECX, UIMM16);

	emit_setcc_r8(DRCTOP, COND_Z, REG_AL);
	emit_setcc_r8(DRCTOP, COND_B, REG_AH);
	emit_setcc_r8(DRCTOP, COND_A, REG_BL);
	emit_shl_r8_imm(DRCTOP, REG_AL, 1);
	emit_shl_r8_imm(DRCTOP, REG_AH, 3);
	emit_shl_r8_imm(DRCTOP, REG_BL, 2);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AH);
	emit_or_r8_r8(DRCTOP, REG_BL, REG_AL);

	emit_bt_m32_imm(DRCTOP, MABS(&XER), 31);        // set XER SO bit to carry
	emit_adc_r32_imm(DRCTOP, REG_EBX, 0);       // effectively sets bit 0 to carry
	emit_mov_m8_r8(DRCTOP, MABS(&ppc.cr[CRFD]), REG_BL);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_cntlzw(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);
	emit_mov_r32_imm(DRCTOP, REG_EDX, 31);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RT)));
	emit_bsr_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_setcc_r8(DRCTOP, COND_Z, REG_BL);          // if all zeros, set BL to 1, so result becomes 32
	emit_sub_r32_r32(DRCTOP, REG_EDX, REG_EAX);
	emit_add_r32_r32(DRCTOP, REG_EDX, REG_EBX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_crand(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_crand);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_crandc(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_crandc);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_creqv(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_creqv);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_crnand(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_crnand);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_crnor(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_crnor);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_cror(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_cror);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_crorc(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_crorc);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_crxor(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_crxor);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dcbf(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dcbi(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dcbst(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dcbt(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dcbtst(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dcbz(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_divwx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_divwx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_divwux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_divwux);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_eieio(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_eqvx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_xor_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_not_r32(DRCTOP, REG_EDX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_extsbx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_movsx_r32_r8(DRCTOP, REG_EDX, REG_DL);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_extshx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_movsx_r32_r16(DRCTOP, REG_EDX, REG_DX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_icbi(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_isync(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lbz(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (x86code *)READ8);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movzx_r32_r8(DRCTOP, REG_EAX, REG_AL);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lbzu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ8);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movzx_r32_r8(DRCTOP, REG_EAX, REG_AL);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lbzux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ8);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movzx_r32_r8(DRCTOP, REG_EAX, REG_AL);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lbzx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)READ8);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movzx_r32_r8(DRCTOP, REG_EAX, REG_AL);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lha(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (x86code *)READ16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movsx_r32_r16(DRCTOP, REG_EAX, REG_AX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lhau(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movsx_r32_r16(DRCTOP, REG_EAX, REG_AX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lhaux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movsx_r32_r16(DRCTOP, REG_EAX, REG_AX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lhax(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)READ16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movsx_r32_r16(DRCTOP, REG_EAX, REG_AX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lhbrx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)READ16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_imm(DRCTOP, REG_ECX, 8);
	emit_rol_r16_cl(DRCTOP, REG_AX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lhz(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (x86code *)READ16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lhzu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lhzux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lhzx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)READ16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lmw(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_lmw);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lswi(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_lswi);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lswx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_lswx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lwarx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_lwarx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lwbrx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)READ32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_bswap_r32(DRCTOP, REG_EAX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lwz(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (x86code *)READ32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lwzu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lwzux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lwzx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)READ32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mcrf(drc_core *drc, UINT32 op)
{
	emit_mov_r8_m8(DRCTOP, REG_AL, MABS(&ppc.cr[RA >> 2]));
	emit_mov_m8_r8(DRCTOP, MABS(&ppc.cr[RT >> 2]), REG_AL);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mcrxr(drc_core *drc, UINT32 op)
{
	osd_printf_debug("PPCDRC: recompile mcrxr\n");
	return RECOMPILE_UNIMPLEMENTED;
}

static UINT32 recompile_mfcr(drc_core *drc, UINT32 op)
{
	int i;
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);

	// generate code for each condition register
	for (i=0; i < 8; i++)
	{
		emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);
		emit_mov_r8_m8(DRCTOP, REG_DL, MABS(&ppc.cr[i]));
		emit_shl_r32_imm(DRCTOP, REG_EDX, ((7-i) * 4));
		emit_or_r32_r32(DRCTOP, REG_EAX, REG_EDX);
	}
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mfmsr(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.msr));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mfspr(drc_core *drc, UINT32 op)
{
	if (SPR == SPR_LR)          // optimized case, LR
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&LR));
	}
	else if(SPR == SPR_CTR)     // optimized case, CTR
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&CTR));
	}
	else
	{
		emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
		emit_push_imm(DRCTOP, SPR);
		emit_call(DRCTOP, (x86code *)ppc_get_spr);
		emit_add_r32_imm(DRCTOP, REG_ESP, 4);
		emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	}
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtcrf(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mtcrf);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtmsr(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)ppc_set_msr);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtspr(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	if (SPR == SPR_LR)          // optimized case, LR
	{
		emit_mov_m32_r32(DRCTOP, MABS(&LR), REG_EAX);
	}
	else if(SPR == SPR_CTR)     // optimized case, CTR
	{
		emit_mov_m32_r32(DRCTOP, MABS(&CTR), REG_EAX);
	}
	else
	{
		emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
		emit_push_r32(DRCTOP, REG_EAX);
		emit_push_imm(DRCTOP, SPR);
		emit_call(DRCTOP, (x86code *)ppc_set_spr);
		emit_add_r32_imm(DRCTOP, REG_ESP, 8);
		emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mulhwx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_mov_r32_m32(DRCTOP, REG_EBX, MABS(&REG(RB)));
	emit_imul_r32(DRCTOP, REG_EBX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mulhwux(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_mov_r32_m32(DRCTOP, REG_EBX, MABS(&REG(RB)));
	emit_mul_r32(DRCTOP, REG_EBX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mulli(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_mov_r32_imm(DRCTOP, REG_EBX, SIMM16);
	emit_imul_r32(DRCTOP, REG_EBX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EAX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mullwx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_mov_r32_m32(DRCTOP, REG_EBX, MABS(&REG(RB)));
	emit_mul_r32(DRCTOP, REG_EBX);
	emit_mov_r32_r32(DRCTOP, REG_EDX, REG_EAX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);

	if (OEBIT) {
		osd_printf_debug("recompile_mullwx: OEBIT set!\n");
		return RECOMPILE_UNIMPLEMENTED;
	}

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_nandx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_and_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_not_r32(DRCTOP, REG_EDX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_negx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_neg_r32(DRCTOP, REG_EDX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_norx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_or_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_not_r32(DRCTOP, REG_EDX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_orx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_or_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_orcx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_not_r32(DRCTOP, REG_EDX);
	emit_or_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_ori(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_or_r32_imm(DRCTOP, REG_EAX, UIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EAX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_oris(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_or_r32_imm(DRCTOP, REG_EAX, UIMM16 << 16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EAX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_rfi(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDI, MABS(&ppc.srr0)); /* get saved PC from SRR0 */
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&ppc.srr1)); /* get saved MSR from SRR1 */

	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)ppc_set_msr);      /* set MSR */
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,0) | RECOMPILE_END_OF_STRING | RECOMPILE_ADD_DISPATCH;
}

static UINT32 recompile_rlwimix(drc_core *drc, UINT32 op)
{
	UINT32 mask = GET_ROTATE_MASK(MB, ME);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_rol_r32_imm(DRCTOP, REG_EDX, (SH));
	emit_and_r32_imm(DRCTOP, REG_EDX, mask);
	emit_and_r32_imm(DRCTOP, REG_EAX, ~mask);
	emit_or_r32_r32(DRCTOP, REG_EDX, REG_EAX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_rlwinmx(drc_core *drc, UINT32 op)
{
	UINT32 mask = GET_ROTATE_MASK(MB, ME);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_rol_r32_imm(DRCTOP, REG_EDX, (SH));
	emit_and_r32_imm(DRCTOP, REG_EDX, mask);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_rlwnmx(drc_core *drc, UINT32 op)
{
	UINT32 mask = GET_ROTATE_MASK(MB, ME);

	emit_mov_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RB)));      // x86 rotate instruction use only 5 bits, so no need to mask this
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_rol_r32_cl(DRCTOP, REG_EDX);
	emit_and_r32_imm(DRCTOP, REG_EDX, mask);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_sc(drc_core *drc, UINT32 op)
{
	emit_mov_m32_imm(DRCTOP, MABS(&SRR0), temp_ppc_pc + 4);
	emit_jmp(DRCTOP, ppc.generate_syscall_exception);
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_slwx(drc_core *drc, UINT32 op)
{
#if USE_SSE2
	emit_mov_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RB)));
	emit_and_r32_imm(DRCTOP, REG_ECX, 0x3f);
	emit_movd_r128_m32(DRCTOP, REG_XMM0, MABS(&REG(RS)));
	emit_movd_r128_r32(DRCTOP, REG_XMM1, REG_ECX);
	emit_psllq_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movd_r32_r128(DRCTOP, REG_EDX, REG_XMM0);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}
#else
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_slwx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_srawx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_srawx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_srawix(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_srawix);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_srwx(drc_core *drc, UINT32 op)
{
#if USE_SSE2
	emit_mov_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RB)));
	emit_and_r32_imm(DRCTOP, REG_ECX, 0x3f);
	emit_movd_r128_m32(DRCTOP, REG_XMM0, MABS(&REG(RS)));
	emit_movd_r128_r32(DRCTOP, REG_XMM1, REG_ECX);
	emit_psrlq_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movd_r32_r128(DRCTOP, REG_EDX, REG_XMM0);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}
#else
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_srwx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stb(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_movzx_r32_r8(DRCTOP, REG_EAX, REG_AL);
	emit_push_r32(DRCTOP, REG_EAX);

	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (x86code *)WRITE8);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stbu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_movzx_r32_r8(DRCTOP, REG_EAX, REG_AL);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)WRITE8);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stbux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_movzx_r32_r8(DRCTOP, REG_EAX, REG_AL);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)WRITE8);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stbx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_movzx_r32_r8(DRCTOP, REG_EAX, REG_AL);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)WRITE8);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_sth(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_movzx_r32_r16(DRCTOP, REG_EAX, REG_AX);
	emit_push_r32(DRCTOP, REG_EAX);

	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
		emit_push_r32(DRCTOP, REG_EDX);
	}
	emit_call(DRCTOP, (x86code *)WRITE16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_sthbrx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_movzx_r32_r16(DRCTOP, REG_EAX, REG_AX);
	emit_mov_r32_imm(DRCTOP, REG_ECX, 8);
	emit_rol_r16_cl(DRCTOP, REG_AX);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)WRITE16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_sthu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_movzx_r32_r16(DRCTOP, REG_EAX, REG_AX);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)WRITE16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_sthux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_movzx_r32_r16(DRCTOP, REG_EAX, REG_AX);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)WRITE16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_sthx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_movzx_r32_r16(DRCTOP, REG_EAX, REG_AX);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)WRITE16);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stmw(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stmw);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stswi(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stswi);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stswx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stswx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stw(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_m32(DRCTOP, MABS(&REG(RS)));
	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stwbrx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RS)));
	emit_bswap_r32(DRCTOP, REG_EAX);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stwcx_rc(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stwcx_rc);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stwu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_m32(DRCTOP, MABS(&REG(RS)));

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EAX);
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stwux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_m32(DRCTOP, MABS(&REG(RS)));

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EAX);
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stwx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_m32(DRCTOP, MABS(&REG(RS)));

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_subfx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_sub_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);

	if (OEBIT) {
		osd_printf_debug("recompile_subfx: OEBIT set !\n");
		return RECOMPILE_UNIMPLEMENTED;
	}
	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_subfcx(drc_core *drc, UINT32 op)
{
	if (OEBIT)
	{
		emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
		emit_push_imm(DRCTOP, op);
		emit_call(DRCTOP, (x86code *)ppc_subfcx);
		emit_add_r32_imm(DRCTOP, REG_ESP, 4);
		emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
	}
	else
	{
		emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
		emit_mov_r32_m32(DRCTOP, REG_EBX, MABS(&XER));
		emit_and_r32_imm(DRCTOP, REG_EBX, ~0x20000000);     // clear carry
		emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
		emit_sub_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
		emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);
		emit_setcc_r8(DRCTOP, COND_NC, REG_AL);             // subtract carry is inverse
		emit_shl_r32_imm(DRCTOP, REG_EAX, 29);              // move carry to correct location in XER
		emit_or_r32_r32(DRCTOP, REG_EBX, REG_EAX);          // insert carry to XER
		emit_mov_m32_r32(DRCTOP, MABS(&XER), REG_EBX);

		//if (OEBIT) {
		//  osd_printf_debug("recompile_subfcx: OEBIT set !\n");
		//  return RECOMPILE_UNIMPLEMENTED;
		//}
		if (RCBIT) {
			append_set_cr0(drc);
		}
	}
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_subfex(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBX, MABS(&XER));
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_r32_m32(DRCTOP, REG_ECX, MABS(&REG(RA)));
	emit_bt_r32_imm(DRCTOP, REG_EBX, 29);               // XER carry to carry flag
	emit_cmc(DRCTOP);                                   // invert carry
	emit_adc_r32_imm(DRCTOP, REG_ECX, 0);
	emit_sub_r32_r32(DRCTOP, REG_EDX, REG_ECX);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);
	emit_setcc_r8(DRCTOP, COND_NC, REG_AL);             // subtract carry is inverse
	emit_and_r32_imm(DRCTOP, REG_EBX, ~0x20000000);     // clear carry
	emit_shl_r32_imm(DRCTOP, REG_EAX, 29);              // move carry to correct location in XER
	emit_or_r32_r32(DRCTOP, REG_EBX, REG_EAX);          // insert carry to XER
	emit_mov_m32_r32(DRCTOP, MABS(&XER), REG_EBX);

	if (OEBIT) {
		osd_printf_debug("recompile_subfex: OEBIT set !\n");
		return RECOMPILE_UNIMPLEMENTED;
	}
	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_subfic(drc_core *drc, UINT32 op)
{
	emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBX, MABS(&XER));
	emit_and_r32_imm(DRCTOP, REG_EBX, ~0x20000000);     // clear carry
	emit_mov_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_sub_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RT)), REG_EDX);
	emit_setcc_r8(DRCTOP, COND_NC, REG_AL);             // subtract carry is inverse
	emit_shl_r32_imm(DRCTOP, REG_EAX, 29);              // move carry to correct location in XER
	emit_or_r32_r32(DRCTOP, REG_EBX, REG_EAX);          // insert carry to XER
	emit_mov_m32_r32(DRCTOP, MABS(&XER), REG_EBX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_subfmex(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_subfmex);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_subfzex(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_subfzex);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_sync(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_tw(drc_core *drc, UINT32 op)
{
	emit_link link1 = {0}, link2 = {0}, link3 = {0}, link4 = {0}, link5 = {0}, link6 = {0};
	int do_link1 = 0;
	int do_link2 = 0;
	int do_link3 = 0;
	int do_link4 = 0;
	int do_link5 = 0;

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_cmp_r32_r32(DRCTOP, REG_EAX, REG_EDX);

	if (RT & 0x10) {
		emit_jcc_near_link(DRCTOP, COND_L, &link1);     // less than = signed <
		do_link1 = 1;
	}
	if (RT & 0x08) {
		emit_jcc_near_link(DRCTOP, COND_G, &link2);     // greater = signed >
		do_link2 = 1;
	}
	if (RT & 0x04) {
		emit_jcc_near_link(DRCTOP, COND_E, &link3);     // equal
		do_link3 = 1;
	}
	if (RT & 0x02) {
		emit_jcc_near_link(DRCTOP, COND_B, &link4);     // below = unsigned <
		do_link4 = 1;
	}
	if (RT & 0x01) {
		emit_jcc_near_link(DRCTOP, COND_A, &link5);     // above = unsigned >
		do_link5 = 1;
	}
	emit_jmp_near_link(DRCTOP, &link6);

	if (do_link1) {
		resolve_link(DRCTOP, &link1);
	}
	if (do_link2) {
		resolve_link(DRCTOP, &link2);
	}
	if (do_link3) {
		resolve_link(DRCTOP, &link3);
	}
	if (do_link4) {
		resolve_link(DRCTOP, &link4);
	}
	if (do_link5) {
		resolve_link(DRCTOP, &link5);
	}
	// generate exception
	emit_mov_m32_imm(DRCTOP, MABS(&SRR0), temp_ppc_pc + 4);
	emit_jmp(DRCTOP, ppc.generate_trap_exception);

	// no exception
	resolve_link(DRCTOP, &link6);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_twi(drc_core *drc, UINT32 op)
{
	emit_link link1 = {0}, link2 = {0}, link3 = {0}, link4 = {0}, link5 = {0}, link6 = {0};
	int do_link1 = 0;
	int do_link2 = 0;
	int do_link3 = 0;
	int do_link4 = 0;
	int do_link5 = 0;

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_mov_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_cmp_r32_r32(DRCTOP, REG_EAX, REG_EDX);

	if (RT & 0x10) {
		emit_jcc_near_link(DRCTOP, COND_L, &link1);     // less than = signed <
		do_link1 = 1;
	}
	if (RT & 0x08) {
		emit_jcc_near_link(DRCTOP, COND_G, &link2);     // greater = signed >
		do_link2 = 1;
	}
	if (RT & 0x04) {
		emit_jcc_near_link(DRCTOP, COND_E, &link3);     // equal
		do_link3 = 1;
	}
	if (RT & 0x02) {
		emit_jcc_near_link(DRCTOP, COND_B, &link4);     // below = unsigned <
		do_link4 = 1;
	}
	if (RT & 0x01) {
		emit_jcc_near_link(DRCTOP, COND_A, &link5);     // above = unsigned >
		do_link5 = 1;
	}
	emit_jmp_near_link(DRCTOP, &link6);

	if (do_link1) {
		resolve_link(DRCTOP, &link1);
	}
	if (do_link2) {
		resolve_link(DRCTOP, &link2);
	}
	if (do_link3) {
		resolve_link(DRCTOP, &link3);
	}
	if (do_link4) {
		resolve_link(DRCTOP, &link4);
	}
	if (do_link5) {
		resolve_link(DRCTOP, &link5);
	}
	// generate exception
	emit_mov_m32_imm(DRCTOP, MABS(&SRR0), temp_ppc_pc + 4);
	emit_jmp(DRCTOP, ppc.generate_trap_exception);

	// no exception
	resolve_link(DRCTOP, &link6);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_xorx(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_xor_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	if (RCBIT) {
		append_set_cr0(drc);
	}

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_xori(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_xor_r32_imm(DRCTOP, REG_EDX, UIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_xoris(drc_core *drc, UINT32 op)
{
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RS)));
	emit_xor_r32_imm(DRCTOP, REG_EDX, UIMM16 << 16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dccci(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dcread(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_icbt(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_iccci(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_icread(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_rfci(drc_core *drc, UINT32 op)
{
	osd_printf_debug("PPCDRC: recompile rfci\n");
	return RECOMPILE_UNIMPLEMENTED;
}

static UINT32 recompile_mfdcr(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mfdcr);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtdcr(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mtdcr);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_wrtee(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_wrtee);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_wrteei(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_wrteei);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}
#endif



static UINT32 recompile_invalid(drc_core *drc, UINT32 op)
{
	osd_printf_debug("PPCDRC: Invalid opcode %08X PC : %X\n", op, ppc.pc);
	return RECOMPILE_UNIMPLEMENTED;
}



/* PowerPC 60x Recompilers */

static UINT32 recompile_lfs(drc_core *drc,UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
#if USE_SSE2
	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (genf*)READ32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movd_r128_r32(DRCTOP, REG_XMM0, REG_EAX);
	emit_cvtss2sd_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);        // convert float to double
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM1);
#else
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_lfs);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
#endif
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lfsu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
#if USE_SSE2
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movd_r128_r32(DRCTOP, REG_XMM0, REG_EAX);
	emit_cvtss2sd_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);        // convert float to double
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM1);
#else
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_lfsu);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
#endif
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lfd(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (x86code *)READ64);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m64_r64(DRCTOP, MABS(&FPR(RT)), REG_EDX, REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lfdu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EDX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ64);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m64_r64(DRCTOP, MABS(&FPR(RT)), REG_EDX, REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stfs(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
#if USE_SSE2
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RT)));
	emit_cvtsd2ss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);        // convert double to float
	emit_movd_r32_r128(DRCTOP, REG_EAX, REG_XMM1);
	emit_push_r32(DRCTOP, REG_EAX);
	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
#else
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stfs);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
#endif
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stfsu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
#if USE_SSE2
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RT)));
	emit_cvtsd2ss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);        // convert double to float
	emit_movd_r32_r128(DRCTOP, REG_EAX, REG_XMM1);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EAX);
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
#else
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stfsu);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
#endif
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stfd(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, MABS(&FPR(RT)));
	emit_push_r32(DRCTOP, REG_EDX);
	emit_push_r32(DRCTOP, REG_EAX);
	if (RA == 0)
	{
		emit_push_imm(DRCTOP, SIMM16);
	}
	else
	{
		emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
		emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
		emit_push_r32(DRCTOP, REG_EAX);
	}
	emit_call(DRCTOP, (x86code *)WRITE64);
	emit_add_r32_imm(DRCTOP, REG_ESP, 12);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stfdu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, MABS(&FPR(RT)));
	emit_push_r32(DRCTOP, REG_EDX);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_add_r32_imm(DRCTOP, REG_EAX, SIMM16);
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EAX);
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE64);
	emit_add_r32_imm(DRCTOP, REG_ESP, 12);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lfdux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ64);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m64_r64(DRCTOP, MABS(&FPR(RT)), REG_EDX, REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lfdx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)READ64);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_m64_r64(DRCTOP, MABS(&FPR(RT)), REG_EDX, REG_EAX);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lfsux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
#if USE_SSE2
	emit_mov_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EDX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EDX);
	emit_push_r32(DRCTOP, REG_EDX);
	emit_call(DRCTOP, (x86code *)READ32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movd_r128_r32(DRCTOP, REG_XMM0, REG_EAX);
	emit_cvtss2sd_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);        // convert float to double
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM1);
#else
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_lfsux);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
#endif
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_lfsx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
#if USE_SSE2
	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)READ32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_movd_r128_r32(DRCTOP, REG_XMM0, REG_EAX);
	emit_cvtss2sd_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);        // convert float to double
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM1);
#else
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_lfsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
#endif
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mfsr(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mfsr);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mfsrin(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mfsrin);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mftb(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mftb);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtsr(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mtsr);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtsrin(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mtsrin);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dcba(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stfdux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, MABS(&FPR(RT)));
	emit_push_r32(DRCTOP, REG_EDX);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EAX);
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE64);
	emit_add_r32_imm(DRCTOP, REG_ESP, 12);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stfdx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, MABS(&FPR(RT)));
	emit_push_r32(DRCTOP, REG_EDX);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE64);
	emit_add_r32_imm(DRCTOP, REG_ESP, 12);

	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stfdx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stfiwx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
#if USE_SSE2
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RT)));
	emit_movd_r32_r128(DRCTOP, REG_EAX, REG_XMM0);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
#else
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stfiwx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
#endif
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stfsux(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
#if USE_SSE2
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RT)));
	emit_cvtsd2ss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);        // convert double to float
	emit_movd_r32_r128(DRCTOP, REG_EAX, REG_XMM1);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	emit_mov_m32_r32(DRCTOP, MABS(&REG(RA)), REG_EAX);
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
#else
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stfsux);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
#endif
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_stfsx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
#if USE_SSE2
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RT)));
	emit_cvtsd2ss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);        // convert double to float
	emit_movd_r32_r128(DRCTOP, REG_EAX, REG_XMM1);
	emit_push_r32(DRCTOP, REG_EAX);

	emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RB)));
	if (RA != 0)
	{
		emit_add_r32_m32(DRCTOP, REG_EAX, MABS(&REG(RA)));
	}
	emit_push_r32(DRCTOP, REG_EAX);
	emit_call(DRCTOP, (x86code *)WRITE32);
	emit_add_r32_imm(DRCTOP, REG_ESP, 8);
#else
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_stfsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
#endif
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_tlbia(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_tlbie(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_tlbsync(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_eciwx(drc_core *drc, UINT32 op)
{
	osd_printf_debug("PPCDRC: eciwx unimplemented\n");
	return RECOMPILE_UNIMPLEMENTED;
}

static UINT32 recompile_ecowx(drc_core *drc, UINT32 op)
{
	osd_printf_debug("PPCDRC: ecowx unimplemented\n");
	return RECOMPILE_UNIMPLEMENTED;
}

static UINT32 recompile_fabsx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fabsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, MABS(&FPR(RB)));
	emit_and_r32_imm(DRCTOP, REG_EDX, 0x7fffffff);
	emit_mov_m64_r64(DRCTOP, MABS(&FPR(RT)), REG_EDX, REG_EAX);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_faddx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_faddx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RB)));
	emit_addsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fcmpo(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fcmpo);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fcmpu(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fcmpu);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fctiwx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fctiwx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fctiwzx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fctiwzx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fdivx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fdivx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RB)));
	emit_divsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fmrx(drc_core *drc, UINT32 op)
{
#if USE_SSE2
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RB)));
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#else
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fmrx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fnabsx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fnabsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, MABS(&FPR(RB)));
	emit_or_r32_imm(DRCTOP, REG_EDX, 0x80000000);
	emit_mov_m64_r64(DRCTOP, MABS(&FPR(RT)), REG_EDX, REG_EAX);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fnegx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fnegx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, MABS(&FPR(RB)));
	emit_xor_r32_imm(DRCTOP, REG_EDX, 0x80000000);
	emit_mov_m64_r64(DRCTOP, MABS(&FPR(RT)), REG_EDX, REG_EAX);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_frspx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_frspx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
/*
    _movq_r128_m64(REG_XMM0, MABS(&FPR(RB)));
    _movq_m64abs_r128(&FPR(RT), REG_XMM0);

    if (RCBIT) {
        append_set_cr1(drc);
    }
*/
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_frsqrtex(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_frsqrtex);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fsqrtx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fsqrtx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fsubx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fsubx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RB)));
	emit_subsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mffsx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mffsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtfsb0x(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mtfsb0x);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtfsb1x(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mtfsb1x);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtfsfx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mtfsfx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mtfsfix(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mtfsfix);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_mcrfs(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_mcrfs);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_faddsx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_faddsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RB)));
	emit_addsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fdivsx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fdivsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RB)));
	emit_divsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fresx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fresx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fsqrtsx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fsqrtsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fsubsx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fsubsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RB)));
	emit_subsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fmaddx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fmaddx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RC)));
	emit_movq_r128_m64(DRCTOP, REG_XMM2, MABS(&FPR(RB)));
	emit_mulsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_addsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM2);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fmsubx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fmsubx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RC)));
	emit_movq_r128_m64(DRCTOP, REG_XMM2, MABS(&FPR(RB)));
	emit_mulsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_subsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM2);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fmulx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fmulx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RC)));
	emit_mulsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fnmaddx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fnmaddx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fnmsubx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fnmsubx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fselx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fselx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fmaddsx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fmaddsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RC)));
	emit_movq_r128_m64(DRCTOP, REG_XMM2, MABS(&FPR(RB)));
	emit_mulsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_addsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM2);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fmsubsx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fmsubsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RC)));
	emit_movq_r128_m64(DRCTOP, REG_XMM2, MABS(&FPR(RB)));
	emit_mulsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_subsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM2);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fmulsx(drc_core *drc, UINT32 op)
{
#if !COMPILE_FPU || !USE_SSE2
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fmulsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));
#else
	emit_movq_r128_m64(DRCTOP, REG_XMM0, MABS(&FPR(RA)));
	emit_movq_r128_m64(DRCTOP, REG_XMM1, MABS(&FPR(RC)));
	emit_mulsd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);
	emit_movq_m64_r128(DRCTOP, MABS(&FPR(RT)), REG_XMM0);

	if (RCBIT) {
		append_set_cr1(drc);
	}
#endif

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fnmaddsx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fnmaddsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_fnmsubsx(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_fnmsubsx);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}
#endif

// PPC602

static UINT32 recompile_esa(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_esa);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_dsa(drc_core *drc, UINT32 op)
{
	emit_mov_m32_r32(DRCTOP, MABS(&ppc_icount), REG_EBP);
	emit_push_imm(DRCTOP, op);
	emit_call(DRCTOP, (x86code *)ppc_dsa);
	emit_add_r32_imm(DRCTOP, REG_ESP, 4);
	emit_mov_r32_m32(DRCTOP, REG_EBP, MABS(&ppc_icount));

	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_tlbli(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}

static UINT32 recompile_tlbld(drc_core *drc, UINT32 op)
{
	return RECOMPILE_SUCCESSFUL_CP(1,4);
}
