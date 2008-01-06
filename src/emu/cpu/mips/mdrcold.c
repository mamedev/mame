

/***************************************************************************
    CONFIGURATION
***************************************************************************/

#define STRIP_NOPS			1		/* 1 is faster */
#define USE_SSE				0		/* can't tell any speed difference here */



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_INSTRUCTIONS				((512 + 128) / 4)

/* recompiler flags */
#define RECOMPILE_UNIMPLEMENTED			0x0000
#define RECOMPILE_SUCCESSFUL			0x0001
#define RECOMPILE_SUCCESSFUL_CP(c,p)	(RECOMPILE_SUCCESSFUL | (((c) & 0xfff) << 8) | (((p) & 0xfff) << 20))
#define RECOMPILE_MAY_CAUSE_EXCEPTION	0x0002
#define RECOMPILE_END_OF_STRING			0x0004
#define RECOMPILE_CHECK_INTERRUPTS		0x0008
#define RECOMPILE_CHECK_SW_INTERRUPTS	0x0010
#define RECOMPILE_ADD_DISPATCH			0x0020

/* append_readwrite flags */
#define ARW_READ						0x0000
#define ARW_WRITE						0x0001
#define ARW_UNSIGNED					0x0000
#define ARW_SIGNED						0x0002
#define ARW_MASKED						0x0004



/***************************************************************************
    MACROS
***************************************************************************/

#define REGDISP(reg)		(offsetof(mips3_state, r[reg]) - offsetof(mips3_state, r[17]))
#define REGADDR(reg)		MBD(REG_ESI, REGDISP(reg))
#define REGADDRLO(reg)		MBD(REG_ESI, REGDISP(reg))
#define REGADDRHI(reg)		MBD(REG_ESI, REGDISP(reg)+4)

#define HIADDR				REGADDR(REG_HI)
#define HIADDRLO			REGADDRLO(REG_HI)
#define HIADDRHI			REGADDRHI(REG_HI)

#define LOADDR				REGADDR(REG_LO)
#define LOADDRLO			REGADDRLO(REG_LO)
#define LOADDRHI			REGADDRHI(REG_LO)

#define CPR0ADDR(reg)		MABS((UINT8 *)&mips3.core->cpr[0][reg])
#define CPR0ADDRLO(reg)		MABS((UINT8 *)&mips3.core->cpr[0][reg])
#define CPR0ADDRHI(reg)		MABS((UINT8 *)&mips3.core->cpr[0][reg] + 4)

#define CCR0ADDR(reg)		MABS((UINT8 *)&mips3.core->ccr[0][reg])
#define CCR0ADDRLO(reg)		MABS((UINT8 *)&mips3.core->ccr[0][reg])
#define CCR0ADDRHI(reg)		MABS((UINT8 *)&mips3.core->ccr[0][reg] + 4)

#define CPR1ADDR(reg)		MABS((UINT8 *)&mips3.core->cpr[1][reg])
#define CPR1ADDRLO(reg)		MABS((UINT8 *)&mips3.core->cpr[1][reg])
#define CPR1ADDRHI(reg)		MABS((UINT8 *)&mips3.core->cpr[1][reg] + 4)

#define CCR1ADDR(reg)		MABS((UINT8 *)&mips3.core->ccr[1][reg])
#define CCR1ADDRLO(reg)		MABS((UINT8 *)&mips3.core->ccr[1][reg])
#define CCR1ADDRHI(reg)		MABS((UINT8 *)&mips3.core->ccr[1][reg] + 4)

#define CPR2ADDR(reg)		MABS((UINT8 *)&mips3.core->cpr[2][reg])
#define CPR2ADDRLO(reg)		MABS((UINT8 *)&mips3.core->cpr[2][reg])
#define CPR2ADDRHI(reg)		MABS((UINT8 *)&mips3.core->cpr[2][reg] + 4)

#define CCR2ADDR(reg)		MABS((UINT8 *)&mips3.core->ccr[2][reg])
#define CCR2ADDRLO(reg)		MABS((UINT8 *)&mips3.core->ccr[2][reg])
#define CCR2ADDRHI(reg)		MABS((UINT8 *)&mips3.core->ccr[2][reg] + 4)

#define FPR32(reg)			(IS_FR0 ? &((float *)&mips3.core->cpr[1][0])[reg] : (float *)&mips3.core->cpr[1][reg])
#define FPR32ADDR(reg)		MABS(FPR32(reg))
#define FPR32ADDRLO(reg)	MABS(FPR32(reg))
#define FPR32ADDRHI(reg)	MABS((UINT8 *)FPR32(reg) + 4)

#define FPR64(reg)			(IS_FR0 ? (double *)&mips3.core->cpr[1][(reg)/2] : (double *)&mips3.core->cpr[1][reg])
#define FPR64ADDR(reg)		MABS(FPR64(reg))
#define FPR64ADDR(reg)		MABS(FPR64(reg))
#define FPR64ADDRLO(reg)	MABS(FPR64(reg))
#define FPR64ADDRHI(reg)	MABS((UINT8 *)FPR64(reg) + 4)

#define CF1ADDR(which)		MABS(&mips3.core->cf[1][(mips3.core->flavor < MIPS3_TYPE_MIPS_IV) ? 0 : (which)])

#define ICOUNTADDR			MABS(&mips3.core->icount)


/***************************************************************************
    USEFUL PRIMITIVES
***************************************************************************/

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

INLINE void emit_mov_m64_imm32(x86code **emitptr, DECLARE_MEMPARAMS, INT32 imm)
{
	emit_mov_m32_imm(emitptr, MEMPARAMS, imm);
	emit_mov_m32_imm(emitptr, base, index, scale, disp+4, (INT32)imm >> 31);
}

INLINE void emit_zero_m64(x86code **emitptr, DECLARE_MEMPARAMS)
{
	if (USE_SSE)
	{
		emit_xorpd_r128_r128(emitptr, REG_XMM0, REG_XMM0);
		emit_movsd_m64_r128(emitptr, MEMPARAMS, REG_XMM0);
	}
	else
		emit_mov_m64_imm32(emitptr, MEMPARAMS, 0);
}

INLINE void emit_mov_m64_m64(x86code **emitptr, DECLARE_MEMPARAMS, UINT8 base2, UINT8 index2, UINT8 scale2, INT32 disp2)
{
	if (USE_SSE)
	{
		emit_movsd_r128_m64(emitptr, REG_XMM0, base2, index2, scale2, disp2);
		emit_movsd_m64_r128(emitptr, MEMPARAMS, REG_XMM0);
	}
	else
	{
		emit_mov_r64_m64(emitptr, REG_EDX, REG_EAX, base2, index2, scale2, disp2);
		emit_mov_m64_r64(emitptr, MEMPARAMS, REG_EDX, REG_EAX);
	}
}

#define emit_save_pc_before_call(e)							\
do { 														\
	if (mips3.drcoptions & MIPS3DRC_FLUSH_PC)				\
		emit_mov_m32_r32(e, MABS(drc->pcptr), REG_EDI);		\
} while (0)




/***************************************************************************
    TYPEDEFS
***************************************************************************/

struct _mips3drc_data
{
	x86code *		generate_interrupt_exception;
	x86code *		generate_cop_exception;
	x86code *		generate_overflow_exception;
	x86code *		generate_invalidop_exception;
	x86code *		generate_syscall_exception;
	x86code *		generate_break_exception;
	x86code *		generate_trap_exception;
	x86code *		generate_tlbload_exception;
	x86code *		generate_tlbstore_exception;
	x86code *		handle_pc_tlb_mismatch;
	x86code *		read_and_translate_byte_signed;
	x86code *		read_and_translate_byte_unsigned;
	x86code *		read_and_translate_half_signed;
	x86code *		read_and_translate_half_unsigned;
	x86code *		read_and_translate_word;
	x86code *		read_and_translate_word_masked;
	x86code *		read_and_translate_double;
	x86code *		read_and_translate_double_masked;
	x86code *		write_and_translate_byte;
	x86code *		write_and_translate_half;
	x86code *		write_and_translate_word;
	x86code *		write_and_translate_word_masked;
	x86code *		write_and_translate_double;
	x86code *		write_and_translate_double_masked;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void drc_reset_callback(drc_core *drc);
static void drc_recompile_callback(drc_core *drc);
static void drc_entrygen_callback(drc_core *drc);

static UINT32 compile_one(drc_core *drc, UINT32 pc, UINT32 physpc);

static void append_generate_exception(drc_core *drc, UINT8 exception);
static void append_readwrite_and_translate(drc_core *drc, int size, UINT8 flags);
static void append_tlb_verify(drc_core *drc, UINT32 pc, void *target);
static void append_update_cycle_counting(drc_core *drc);
static void append_check_interrupts(drc_core *drc, int inline_generate);
static void append_check_sw_interrupts(drc_core *drc, int inline_generate);

static UINT32 recompile_instruction(drc_core *drc, UINT32 pc, UINT32 physpc);
static UINT32 recompile_special(drc_core *drc, UINT32 pc, UINT32 op);
static UINT32 recompile_regimm(drc_core *drc, UINT32 pc, UINT32 op);

static UINT32 recompile_cop0(drc_core *drc, UINT32 pc, UINT32 op);
static UINT32 recompile_cop1(drc_core *drc, UINT32 pc, UINT32 op);
static UINT32 recompile_cop1x(drc_core *drc, UINT32 pc, UINT32 op);



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static UINT64 dmult_temp1;
static UINT64 dmult_temp2;
static UINT32 jr_temp;

static UINT8 in_delay_slot = 0;



/***************************************************************************
    CORE RECOMPILER SYSTEMS
***************************************************************************/

/*-------------------------------------------------
    mips3drc_init - initialize the drc-specific
    state
-------------------------------------------------*/

static void mips3drc_init(void)
{
	drc_config drconfig;

	/* fill in the config */
	memset(&drconfig, 0, sizeof(drconfig));
	drconfig.cache_base       = mips3.cache;
	drconfig.cache_size       = CACHE_SIZE;
	drconfig.max_instructions = MAX_INSTRUCTIONS;
	drconfig.address_bits     = 32;
	drconfig.lsbs_to_ignore   = 2;
	drconfig.uses_fp          = 1;
	drconfig.uses_sse         = USE_SSE;
	drconfig.pc_in_memory     = 0;
	drconfig.icount_in_memory = 0;
	drconfig.pcptr            = (UINT32 *)&mips3.core->pc;
	drconfig.icountptr        = (UINT32 *)&mips3.core->icount;
	drconfig.esiptr           = NULL;
	drconfig.cb_reset         = drc_reset_callback;
	drconfig.cb_recompile     = drc_recompile_callback;
	drconfig.cb_entrygen      = drc_entrygen_callback;

	/* initialize the compiler */
	mips3.drc = drc_init(cpu_getactivecpu(), &drconfig);
	mips3.drcoptions = MIPS3DRC_FASTEST_OPTIONS;

	/* allocate our data out of the cache */
	mips3.drcdata = drc_alloc(mips3.drc, sizeof(*mips3.drcdata));
	memset(mips3.drcdata, 0, sizeof(*mips3.drcdata));
}


/*-------------------------------------------------
    mips3drc_exit - clean up the drc-specific
    state
-------------------------------------------------*/

static void mips3drc_exit(void)
{
	drc_exit(mips3.drc);
}



/***************************************************************************
    RECOMPILER CALLBACKS
***************************************************************************/

/*------------------------------------------------------------------
    drc_reset_callback
------------------------------------------------------------------*/

static void drc_reset_callback(drc_core *drc)
{
	if (LOG_CODE)
	{
		x86log_disasm_code_range(mips3.log, "entry_point:", (x86code *)drc->entry_point, drc->out_of_cycles);
		x86log_disasm_code_range(mips3.log, "out_of_cycles:", drc->out_of_cycles, drc->recompile);
		x86log_disasm_code_range(mips3.log, "recompile:", drc->recompile, drc->dispatch);
		x86log_disasm_code_range(mips3.log, "dispatch:", drc->dispatch, drc->flush);
		x86log_disasm_code_range(mips3.log, "flush:", drc->flush, drc->cache_top);
	}

	mips3.drcdata->generate_interrupt_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_INTERRUPT);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "generate_interrupt_exception:", mips3.drcdata->generate_interrupt_exception, drc->cache_top);

	mips3.drcdata->generate_cop_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_BADCOP);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "generate_cop_exception:", mips3.drcdata->generate_cop_exception, drc->cache_top);

	mips3.drcdata->generate_overflow_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_OVERFLOW);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "generate_overflow_exception:", mips3.drcdata->generate_overflow_exception, drc->cache_top);

	mips3.drcdata->generate_invalidop_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_INVALIDOP);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "generate_invalidop_exception:", mips3.drcdata->generate_invalidop_exception, drc->cache_top);

	mips3.drcdata->generate_syscall_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_SYSCALL);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "generate_syscall_exception:", mips3.drcdata->generate_syscall_exception, drc->cache_top);

	mips3.drcdata->generate_break_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_BREAK);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "generate_break_exception:", mips3.drcdata->generate_break_exception, drc->cache_top);

	mips3.drcdata->generate_trap_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_TRAP);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "generate_trap_exception:", mips3.drcdata->generate_trap_exception, drc->cache_top);

	mips3.drcdata->generate_tlbload_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_TLBLOAD);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "generate_tlbload_exception:", mips3.drcdata->generate_tlbload_exception, drc->cache_top);

	mips3.drcdata->generate_tlbstore_exception = drc->cache_top;
	append_generate_exception(drc, EXCEPTION_TLBSTORE);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "generate_tlbstore_exception:", mips3.drcdata->generate_tlbstore_exception, drc->cache_top);

	mips3.drcdata->handle_pc_tlb_mismatch = drc->cache_top;
	emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EDI);												// mov  eax,edi
	emit_shr_r32_imm(DRCTOP, REG_EAX, 12);													// shr  eax,12
	emit_mov_r32_m32(DRCTOP, REG_EBX, MISD(REG_EAX, 4, mips3.core->tlb_table));				// mov  ebx,tlb_table[eax*4]
	emit_test_r32_imm(DRCTOP, REG_EBX, 2);													// test ebx,2
	emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EDI);												// mov  eax,edi
	emit_jcc(DRCTOP, COND_NZ, mips3.drcdata->generate_tlbload_exception);					// jnz  generate_tlbload_exception
	emit_mov_m32_r32(DRCTOP, MABS(drc->pcptr), REG_EDI);									// mov  [pcptr],edi
	emit_jmp(DRCTOP, drc->recompile);														// jmp  recompile
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "handle_pc_tlb_mismatch:", mips3.drcdata->handle_pc_tlb_mismatch, drc->cache_top);

	mips3.drcdata->read_and_translate_byte_signed = drc->cache_top;
	append_readwrite_and_translate(drc, 1, ARW_READ | ARW_SIGNED);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "read_and_translate_byte_signed:", mips3.drcdata->read_and_translate_byte_signed, drc->cache_top);

	mips3.drcdata->read_and_translate_byte_unsigned = drc->cache_top;
	append_readwrite_and_translate(drc, 1, ARW_READ | ARW_UNSIGNED);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "read_and_translate_byte_unsigned:", mips3.drcdata->read_and_translate_byte_unsigned, drc->cache_top);

	mips3.drcdata->read_and_translate_half_signed = drc->cache_top;
	append_readwrite_and_translate(drc, 2, ARW_READ | ARW_SIGNED);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "read_and_translate_half_signed:", mips3.drcdata->read_and_translate_half_signed, drc->cache_top);

	mips3.drcdata->read_and_translate_half_unsigned = drc->cache_top;
	append_readwrite_and_translate(drc, 2, ARW_READ | ARW_UNSIGNED);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "read_and_translate_half_unsigned:", mips3.drcdata->read_and_translate_half_unsigned, drc->cache_top);

	mips3.drcdata->read_and_translate_word = drc->cache_top;
	append_readwrite_and_translate(drc, 4, ARW_READ);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "read_and_translate_word:", mips3.drcdata->read_and_translate_word, drc->cache_top);

	mips3.drcdata->read_and_translate_word_masked = drc->cache_top;
	append_readwrite_and_translate(drc, 4, ARW_READ | ARW_MASKED);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "read_and_translate_word_masked:", mips3.drcdata->read_and_translate_word_masked, drc->cache_top);

	mips3.drcdata->read_and_translate_double = drc->cache_top;
	append_readwrite_and_translate(drc, 8, ARW_READ);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "read_and_translate_double:", mips3.drcdata->read_and_translate_double, drc->cache_top);

	mips3.drcdata->read_and_translate_double_masked = drc->cache_top;
	append_readwrite_and_translate(drc, 8, ARW_READ | ARW_MASKED);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "read_and_translate_double_masked:", mips3.drcdata->read_and_translate_double_masked, drc->cache_top);

	mips3.drcdata->write_and_translate_byte = drc->cache_top;
	append_readwrite_and_translate(drc, 1, ARW_WRITE);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "write_and_translate_byte:", mips3.drcdata->write_and_translate_byte, drc->cache_top);

	mips3.drcdata->write_and_translate_half = drc->cache_top;
	append_readwrite_and_translate(drc, 2, ARW_WRITE);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "write_and_translate_half:", mips3.drcdata->write_and_translate_half, drc->cache_top);

	mips3.drcdata->write_and_translate_word = drc->cache_top;
	append_readwrite_and_translate(drc, 4, ARW_WRITE);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "write_and_translate_word:", mips3.drcdata->write_and_translate_word, drc->cache_top);

	mips3.drcdata->write_and_translate_word_masked = drc->cache_top;
	append_readwrite_and_translate(drc, 4, ARW_WRITE | ARW_MASKED);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "write_and_translate_word_masked:", mips3.drcdata->write_and_translate_word_masked, drc->cache_top);

	mips3.drcdata->write_and_translate_double = drc->cache_top;
	append_readwrite_and_translate(drc, 8, ARW_WRITE);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "write_and_translate_double:", mips3.drcdata->write_and_translate_double, drc->cache_top);

	mips3.drcdata->write_and_translate_double_masked = drc->cache_top;
	append_readwrite_and_translate(drc, 8, ARW_WRITE | ARW_MASKED);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "write_and_translate_double_masked:", mips3.drcdata->write_and_translate_double_masked, drc->cache_top);
}


/*------------------------------------------------------------------
    drc_recompile_callback
------------------------------------------------------------------*/

static void drc_recompile_callback(drc_core *drc)
{
	int remaining = MAX_INSTRUCTIONS;
	x86code *start = drc->cache_top;
	UINT32 pc = mips3.core->pc;
	UINT32 lastpc = pc;
	UINT32 physpc;

	(void)start;

	/* begin the sequence */
	drc_begin_sequence(drc, pc);

	/* make sure our FR flag matches the last time */
	emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_FR);								// test [COP0_Status],SR_FR
	emit_jcc(DRCTOP, IS_FR0 ? COND_NZ : COND_Z, drc->recompile);							// jnz/z recompile

	/* ensure that we are still mapped */
	append_tlb_verify(drc, pc, mips3.drcdata->handle_pc_tlb_mismatch);
	physpc = pc;
	if (!mips3com_translate_address(mips3.core, ADDRESS_SPACE_PROGRAM, &physpc))
	{
		emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EDI);											// mov  eax,edi
		emit_jmp(DRCTOP, mips3.drcdata->generate_tlbload_exception);						// jmp  tlbload_exception
		goto exit;
	}
	change_pc(physpc);

	/* loose verification case: one verification here only */
	if (!(mips3.drcoptions & MIPS3DRC_STRICT_VERIFY))
		drc_append_verify_code(drc, cpu_opptr(physpc), 4);

	/* loop until we hit an unconditional branch */
	while (--remaining != 0)
	{
		UINT32 result;

		/* if we crossed a page boundary, check the mapping */
		if ((pc ^ lastpc) & 0xfffff000)
		{
			physpc = pc;
			if (!mips3com_translate_address(mips3.core, ADDRESS_SPACE_PROGRAM, &physpc))
				break;
			append_tlb_verify(drc, pc, drc->dispatch);
			change_pc(physpc);
		}

		/* compile one instruction */
		physpc = (physpc & 0xfffff000) | (pc & 0xfff);
		result = compile_one(drc, pc, physpc);
		lastpc = pc;
		pc += (INT32)result >> 20;
		if (result & RECOMPILE_END_OF_STRING)
			break;
	}

	/* add dispatcher just in case */
	if (remaining == 0)
		drc_append_dispatcher(drc);

exit:
	/* end the sequence */
	drc_end_sequence(drc);

	/* log the generated code */
	if (LOG_CODE)
	{
		char label[60];
		physpc = mips3.core->pc;
		mips3com_translate_address(mips3.core, ADDRESS_SPACE_PROGRAM, &physpc);
		sprintf(label, "Code @ %08X (%08X physical)", mips3.core->pc, physpc);
		x86log_disasm_code_range(mips3.log, label, start, drc->cache_top);
	}
}


/*------------------------------------------------------------------
    drc_entrygen_callback
------------------------------------------------------------------*/

static void drc_entrygen_callback(drc_core *drc)
{
	emit_mov_r32_imm(DRCTOP, REG_ESI, (FPTR)&mips3.core->r[17]);
	append_check_interrupts(drc, 1);
}



/***************************************************************************
    RECOMPILER CORE
***************************************************************************/

/*------------------------------------------------------------------
    compile_one
------------------------------------------------------------------*/

static UINT32 compile_one(drc_core *drc, UINT32 pc, UINT32 physpc)
{
	int pcdelta, cycles, hotnum;
	UINT32 *opptr;
	UINT32 result;

	/* register this instruction */
	drc_register_code_at_cache_top(drc, pc);

	/* get a pointer to the current instruction */
	change_pc(physpc);
	opptr = cpu_opptr(physpc);

	/* emit debugging and self-modifying code checks */
	drc_append_call_debugger(drc);
	if (mips3.drcoptions & MIPS3DRC_STRICT_VERIFY)
		drc_append_verify_code(drc, opptr, 4);

	/* is this a hotspot? */
	for (hotnum = 0; hotnum < MIPS3_MAX_HOTSPOTS; hotnum++)
		if (pc == mips3.hotspot[hotnum].pc && *(UINT32 *)opptr == mips3.hotspot[hotnum].opcode)
			break;

	/* compile the instruction */
	result = recompile_instruction(drc, pc, physpc);

	/* handle the results */
	if (!(result & RECOMPILE_SUCCESSFUL))
		fatalerror("Unimplemented op %08X (%02X,%02X)", *opptr, *opptr >> 26, *opptr & 0x3f);

	pcdelta = (INT32)result >> 20;
	cycles = (result >> 8) & 0xfff;

	/* absorb any NOPs following */
	if (STRIP_NOPS && !Machine->debug_mode)
	{
		if (!(result & (RECOMPILE_END_OF_STRING | RECOMPILE_CHECK_INTERRUPTS | RECOMPILE_CHECK_SW_INTERRUPTS)))
			while ((((pc + pcdelta) ^ pc) & 0xfffff000) == 0 && pcdelta < 120 && opptr[pcdelta/4] == 0)
			{
				pcdelta += 4;
				cycles += 1;
			}
	}

	/* epilogue */
	if (hotnum != MIPS3_MAX_HOTSPOTS)
		cycles += mips3.hotspot[hotnum].cycles;
	drc_append_standard_epilogue(drc, cycles, pcdelta, 1);

	/* check interrupts */
	if (result & RECOMPILE_CHECK_INTERRUPTS)
		append_check_interrupts(drc, 0);
	if (result & RECOMPILE_CHECK_SW_INTERRUPTS)
		append_check_sw_interrupts(drc, 0);
	if (result & RECOMPILE_ADD_DISPATCH)
		drc_append_dispatcher(drc);

	return (result & 0xff) | ((cycles & 0xfff) << 8) | ((pcdelta & 0xfff) << 20);
}



/***************************************************************************
    COMMON ROUTINES
***************************************************************************/

/*------------------------------------------------------------------
    append_generate_exception
------------------------------------------------------------------*/

static void append_generate_exception(drc_core *drc, UINT8 exception)
{
	UINT32 offset = (exception >= EXCEPTION_TLBMOD && exception <= EXCEPTION_TLBSTORE) ? 0x00 : 0x180;
	emit_link link1, link2;

	/* assumes address is in EAX */
	if (exception == EXCEPTION_TLBLOAD || exception == EXCEPTION_TLBSTORE)
	{
		emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_BadVAddr), REG_EAX);							// mov  [BadVAddr],eax
		emit_and_m32_imm(DRCTOP, CPR0ADDR(COP0_EntryHi), 0x000000ff);						// and  [EntryHi],0x000000ff
		emit_and_r32_imm(DRCTOP, REG_EAX, 0xffffe000);										// and  eax,0xffffe000
		emit_or_m32_r32(DRCTOP, CPR0ADDR(COP0_EntryHi), REG_EAX);							// or   [EntryHi],eax
		emit_and_m32_imm(DRCTOP, CPR0ADDR(COP0_Context), 0xff800000);						// and  [Context],0xff800000
		emit_shr_r32_imm(DRCTOP, REG_EAX, 9);												// shr  eax,9
		emit_or_m32_r32(DRCTOP, CPR0ADDR(COP0_Context), REG_EAX);							// or   [Context],eax
	}

	emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_EPC), REG_EDI);									// mov  [COP0_EPC],edi
	emit_mov_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Cause));								// mov  eax,[COP0_Cause]
	emit_and_r32_imm(DRCTOP, REG_EAX, ~0x800000ff);											// and  eax,~0x800000ff
	if (exception)
		emit_or_r32_imm(DRCTOP, REG_EAX, exception << 2);									// or   eax,exception << 2
	emit_cmp_m32_imm(DRCTOP, MABS(&mips3.nextpc), ~0);										// cmp  [mips3.nextpc],~0
	emit_jcc_short_link(DRCTOP, COND_E, &link1);											// je   skip
	emit_mov_m32_imm(DRCTOP, MABS(&mips3.nextpc), ~0);										// mov  [mips3.nextpc],~0
	emit_sub_m32_imm(DRCTOP, CPR0ADDR(COP0_EPC), 4);										// sub  [COP0_EPC],4
	emit_or_r32_imm(DRCTOP, REG_EAX, 0x80000000);											// or   eax,0x80000000
	resolve_link(DRCTOP, &link1);														// skip:
	emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Cause), REG_EAX);								// mov  [COP0_Cause],eax
	emit_mov_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Status));								// mov  eax,[COP0_Status]
	emit_or_r32_imm(DRCTOP, REG_EAX, SR_EXL);												// or   eax,SR_EXL
	emit_test_r32_imm(DRCTOP, REG_EAX, SR_BEV);												// test eax,SR_BEV
	emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Status), REG_EAX);								// mov  [COP0_Status],eax
	emit_mov_r32_imm(DRCTOP, REG_EDI, 0xbfc00200 + offset);									// mov  edi,0xbfc00200+offset
	emit_jcc_short_link(DRCTOP, COND_NZ, &link2);											// jnz  skip2
	emit_mov_r32_imm(DRCTOP, REG_EDI, 0x80000000 + offset);									// mov  edi,0x80000000+offset
	resolve_link(DRCTOP, &link2);															// skip2:
	drc_append_dispatcher(drc);																// dispatch
}


/*------------------------------------------------------------------
    append_readwrite_and_translate
------------------------------------------------------------------*/

static void append_readwrite_and_translate(drc_core *drc, int size, UINT8 flags)
{
	emit_link link1 = { 0 }, link2 = { 0 }, link3 = { 0 };
	int ramnum;

	emit_mov_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 4));										// mov  eax,[esp+4]
	emit_mov_r32_r32(DRCTOP, REG_EBX, REG_EAX);												// mov  ebx,eax
	emit_shr_r32_imm(DRCTOP, REG_EBX, 12);													// shr  ebx,12
	emit_mov_r32_m32(DRCTOP, REG_EBX, MISD(REG_EBX, 4, mips3.core->tlb_table));				// mov  ebx,tlb_table[ebx*4]
	emit_and_r32_imm(DRCTOP, REG_EAX, 0xfff);												// and  eax,0xfff
	emit_shr_r32_imm(DRCTOP, REG_EBX, ((flags & ARW_WRITE) ? 1 : 2));						// shr  ebx,2/1 (read/write)
	emit_lea_r32_m32(DRCTOP, REG_EBX, MBISD(REG_EAX, REG_EBX, ((flags & ARW_WRITE) ? 2 : 4), 0));
																							// lea  ebx,[eax+ebx*4/2] (read/write)
	emit_jcc_near_link(DRCTOP, COND_C, &link1);												// jc   error
	for (ramnum = 0; ramnum < MIPS3_MAX_FASTRAM; ramnum++)
		if (!Machine->debug_mode && mips3.fastram[ramnum].base && (!(flags & ARW_WRITE) || !mips3.fastram[ramnum].readonly))
		{
			UINT32 fastbase = (UINT32)((UINT8 *)mips3.fastram[ramnum].base - mips3.fastram[ramnum].start);
			if (mips3.fastram[ramnum].end != 0xffffffff)
			{
				emit_cmp_r32_imm(DRCTOP, REG_EBX, mips3.fastram[ramnum].end);				// cmp  ebx,fastram_end
				emit_jcc_short_link(DRCTOP, COND_A, &link2);								// ja   notram
			}
			if (mips3.fastram[ramnum].start != 0x00000000)
			{
				emit_cmp_r32_imm(DRCTOP, REG_EBX, mips3.fastram[ramnum].start);				// cmp  ebx,fastram_start
				emit_jcc_short_link(DRCTOP, COND_B, &link3);								// jb   notram
			}

			if (!(flags & ARW_WRITE))
			{
				if (size == 1)
				{
					if (mips3.core->bigendian)
						emit_xor_r32_imm(DRCTOP, REG_EBX, 3);								// xor   ebx,3
					if (flags & ARW_SIGNED)
						emit_movsx_r32_m8(DRCTOP, REG_EAX, MBD(REG_EBX, fastbase));			// movsx eax,byte ptr [ebx+fastbase]
					else
						emit_movzx_r32_m8(DRCTOP, REG_EAX, MBD(REG_EBX, fastbase));			// movzx eax,byte ptr [ebx+fastbase]
				}
				else if (size == 2)
				{
					if (mips3.core->bigendian)
						emit_xor_r32_imm(DRCTOP, REG_EBX, 2);								// xor   ebx,2
					if (flags & ARW_SIGNED)
						emit_movsx_r32_m16(DRCTOP, REG_EAX, MBD(REG_EBX, fastbase));		// movsx eax,word ptr [ebx+fastbase]
					else
						emit_movzx_r32_m16(DRCTOP, REG_EAX, MBD(REG_EBX, fastbase));		// movzx eax,word ptr [ebx+fastbase]
				}
				else if (size == 4)
					emit_mov_r32_m32(DRCTOP, REG_EAX, MBD(REG_EBX, fastbase));				// mov   eax,[ebx+fastbase]
				else if (size == 8)
				{
					if (mips3.core->bigendian)
						emit_mov_r64_m64(DRCTOP, REG_EAX, REG_EDX, MBD(REG_EBX, (FPTR)fastbase));
																							// mov   eax:edx,[ebx+fastbase]
					else
						emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, MBD(REG_EBX, (FPTR)fastbase));
																							// mov   edx:eax,[ebx+fastbase]
				}
				emit_ret(DRCTOP);															// ret
			}
			else
			{
				if (size == 1)
				{
					emit_mov_r8_m8(DRCTOP, REG_AL, MBD(REG_ESP, 8));						// mov   al,[esp+8]
					if (mips3.core->bigendian)
						emit_xor_r32_imm(DRCTOP, REG_EBX, 3);								// xor   ebx,3
					emit_mov_m8_r8(DRCTOP, MBD(REG_EBX, fastbase), REG_AL);					// mov   [ebx+fastbase],al
				}
				else if (size == 2)
				{
					emit_mov_r16_m16(DRCTOP, REG_AX, MBD(REG_ESP, 8));						// mov   ax,[esp+8]
					if (mips3.core->bigendian)
						emit_xor_r32_imm(DRCTOP, REG_EBX, 2);								// xor   ebx,2
					emit_mov_m16_r16(DRCTOP, MBD(REG_EBX, fastbase), REG_AX);				// mov   [ebx+fastbase],ax
				}
				else if (size == 4)
				{
					if (!(flags & ARW_MASKED))
					{
						emit_mov_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 8));					// mov   eax,[esp+8]
						emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase), REG_EAX);			// mov   [ebx+fastbase],eax
					}
					else
					{
						emit_mov_r32_m32(DRCTOP, REG_ECX, MBD(REG_ESP, 12));				// mov   ecx,[esp+12]
						emit_mov_r32_r32(DRCTOP, REG_EAX, REG_ECX);							// mov   eax,ecx
						emit_and_r32_m32(DRCTOP, REG_ECX, MBD(REG_EBX, fastbase));			// and   ecx,[ebx+fastbase]
						emit_not_r32(DRCTOP, REG_EAX);										// not   eax
						emit_and_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 8));					// and   eax,[esp+8]
						emit_or_r32_r32(DRCTOP, REG_EAX, REG_ECX);							// or    eax,ecx
						emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase), REG_EAX);			// mov   [ebx+fastbase],eax
					}
				}
				else if (size == 8)
				{
					if (!(flags & ARW_MASKED))
					{
						if (mips3.core->bigendian)
						{
							emit_mov_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 8));				// mov   eax,[esp+8]
							emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase+4), REG_EAX);	// mov   [ebx+fastbase+4],eax
							emit_mov_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 12));			// mov   eax,[esp+12]
							emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase), REG_EAX);		// mov   [ebx+fastbase],eax
						}
						else
						{
							emit_mov_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 8));				// mov   eax,[esp+8]
							emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase), REG_EAX);		// mov   [ebx+fastbase],eax
							emit_mov_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 12));			// mov   eax,[esp+12]
							emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase+4), REG_EAX);	// mov   [ebx+fastbase+4],eax
						}
					}
					else
					{
						if (mips3.core->bigendian)
						{
							emit_mov_r32_m32(DRCTOP, REG_ECX, MBD(REG_ESP, 16));			// mov   ecx,[esp+16]
							emit_mov_r32_r32(DRCTOP, REG_EAX, REG_ECX);						// mov   eax,ecx
							emit_and_r32_m32(DRCTOP, REG_ECX, MBD(REG_EBX, fastbase+4));	// and   ecx,[ebx+fastbase+4]
							emit_not_r32(DRCTOP, REG_EAX);									// not   eax
							emit_and_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 8));				// and   eax,[esp+8]
							emit_or_r32_r32(DRCTOP, REG_EAX, REG_ECX);						// or    eax,ecx
							emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase+4), REG_EAX);	// mov   [ebx+fastbase+4],eax
							emit_mov_r32_m32(DRCTOP, REG_ECX, MBD(REG_ESP, 20));			// mov   ecx,[esp+20]
							emit_mov_r32_r32(DRCTOP, REG_EAX, REG_ECX);						// mov   eax,ecx
							emit_and_r32_m32(DRCTOP, REG_ECX, MBD(REG_EBX, fastbase));		// and   ecx,[ebx+fastbase]
							emit_not_r32(DRCTOP, REG_EAX);									// not   eax
							emit_and_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 12));			// and   eax,[esp+12]
							emit_or_r32_r32(DRCTOP, REG_EAX, REG_ECX);						// or    eax,ecx
							emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase), REG_EAX);		// mov   [ebx+fastbase],eax
						}
						else
						{
							emit_mov_r32_m32(DRCTOP, REG_ECX, MBD(REG_ESP, 16));			// mov   ecx,[esp+16]
							emit_mov_r32_r32(DRCTOP, REG_EAX, REG_ECX);						// mov   eax,ecx
							emit_and_r32_m32(DRCTOP, REG_ECX, MBD(REG_EBX, fastbase));		// and   ecx,[ebx+fastbase]
							emit_not_r32(DRCTOP, REG_EAX);									// not   eax
							emit_and_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 8));				// and   eax,[esp+8]
							emit_or_r32_r32(DRCTOP, REG_EAX, REG_ECX);						// or    eax,ecx
							emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase), REG_EAX);		// mov   [ebx+fastbase],eax
							emit_mov_r32_m32(DRCTOP, REG_ECX, MBD(REG_ESP, 20));			// mov   ecx,[esp+20]
							emit_mov_r32_r32(DRCTOP, REG_EAX, REG_ECX);						// mov   eax,ecx
							emit_and_r32_m32(DRCTOP, REG_ECX, MBD(REG_EBX, fastbase+4));	// and   ecx,[ebx+fastbase+4]
							emit_not_r32(DRCTOP, REG_EAX);									// not   eax
							emit_and_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 12));			// and   eax,[esp+12]
							emit_or_r32_r32(DRCTOP, REG_EAX, REG_ECX);						// or    eax,ecx
							emit_mov_m32_r32(DRCTOP, MBD(REG_EBX, fastbase+4), REG_EAX);	// mov   [ebx+fastbase+4],eax
						}
					}
				}
				emit_ret(DRCTOP);															// ret
			}

			if (mips3.fastram[ramnum].end != 0xffffffff)								// notram:
				resolve_link(DRCTOP, &link2);
			if (mips3.fastram[ramnum].start != 0x00000000)
				resolve_link(DRCTOP, &link3);
		}

	if (flags & ARW_WRITE)
	{
		emit_mov_m32_r32(DRCTOP, MBD(REG_ESP, 4), REG_EBX);									// mov  [esp+4],ebx
		if (size == 1)
			emit_jmp(DRCTOP, (x86code *)mips3.core->memory.writebyte);						// jmp  writebyte
		else if (size == 2)
			emit_jmp(DRCTOP, (x86code *)mips3.core->memory.writehalf);						// jmp  writehalf
		else if (size == 4)
		{
			if (!(flags & ARW_MASKED))
				emit_jmp(DRCTOP, (x86code *)mips3.core->memory.writeword);					// jmp  writeword
			else
				emit_jmp(DRCTOP, (x86code *)mips3.core->memory.writeword_masked);			// jmp  writeword_masked
		}
		else
		{
			if (!(flags & ARW_MASKED))
				emit_jmp(DRCTOP, (x86code *)mips3.core->memory.writedouble);				// jmp  writedouble
			else
				emit_jmp(DRCTOP, (x86code *)mips3.core->memory.writedouble_masked);			// jmp  writedouble_masked
		}
	}
	else
	{
		if (size == 1)
		{
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			emit_call(DRCTOP, (x86code *)mips3.core->memory.readbyte);						// call  readbyte
			if (flags & ARW_SIGNED)
				emit_movsx_r32_r8(DRCTOP, REG_EAX, REG_AL);									// movsx eax,al
			else
				emit_movzx_r32_r8(DRCTOP, REG_EAX, REG_AL);									// movzx eax,al
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_ret(DRCTOP);																// ret
		}
		else if (size == 2)
		{
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			emit_call(DRCTOP, (x86code *)mips3.core->memory.readhalf);						// call  readhalf
			if (flags & ARW_SIGNED)
				emit_movsx_r32_r16(DRCTOP, REG_EAX, REG_AX);								// movsx eax,ax
			else
				emit_movzx_r32_r16(DRCTOP, REG_EAX, REG_AX);								// movzx eax,ax
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_ret(DRCTOP);																// ret
		}
		else if (size == 4)
		{
			emit_mov_m32_r32(DRCTOP, MBD(REG_ESP, 4), REG_EBX);								// mov  [esp+4],ebx
			if (!(flags & ARW_MASKED))
				emit_jmp(DRCTOP, (x86code *)mips3.core->memory.readword);					// jmp  readword
			else
				emit_jmp(DRCTOP, (x86code *)mips3.core->memory.readword_masked);			// jmp  readword_masked
		}
		else
		{
			emit_mov_m32_r32(DRCTOP, MBD(REG_ESP, 4), REG_EBX);								// mov  [esp+4],ebx
			if (!(flags & ARW_MASKED))
				emit_jmp(DRCTOP, (x86code *)mips3.core->memory.readdouble);					// jmp  readdouble
			else
				emit_jmp(DRCTOP, (x86code *)mips3.core->memory.readdouble_masked);			// jmp  readdouble_masked
		}
	}
	{
		int valuebytes = (size == 8) ? 8 : 4;
		int stackbytes = 4 + ((flags & ARW_WRITE) ? 4 + valuebytes : 4) + ((flags & ARW_MASKED) ? valuebytes : 0);
		resolve_link(DRCTOP, &link1);													// error:
		emit_mov_r32_m32(DRCTOP, REG_EAX, MBD(REG_ESP, 4));									// mov  eax,[esp+4]
		emit_add_r32_imm(DRCTOP, REG_ESP, stackbytes);										// add  esp,stack_bytes
		emit_jmp(DRCTOP, (flags & ARW_WRITE) ? (x86code *)mips3.drcdata->generate_tlbstore_exception : (x86code *)mips3.drcdata->generate_tlbload_exception);
																							// jmp    generate_exception
	}
}


/*------------------------------------------------------------------
    append_tlb_verify
------------------------------------------------------------------*/

static void append_tlb_verify(drc_core *drc, UINT32 pc, void *target)
{
	/* addresses 0x80000000-0xbfffffff are direct-mapped; no checking needed */
	if (pc < 0x80000000 || pc >= 0xc0000000)
	{
		emit_cmp_m32_imm(DRCTOP, MABS(&mips3.core->tlb_table[pc >> 12]), mips3.core->tlb_table[pc >> 12]);
																							// cmp  tlbtable[pc >> 12],physpc & 0xfffff000
		emit_jcc(DRCTOP, COND_NE, target);													// jne  handle_pc_tlb_mismatch
	}
}


/*------------------------------------------------------------------
    append_update_cycle_counting
------------------------------------------------------------------*/

static void append_update_cycle_counting(drc_core *drc)
{
	emit_sub_r32_imm(DRCTOP, REG_ESP, 8);													// sub  esp,8
	emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);											// mov  [icount],ebp
	emit_push_imm(DRCTOP, (FPTR)mips3.core);												// push mips3.core
	emit_call(DRCTOP, (x86code *)mips3com_update_cycle_counting);							// call update_cycle_counting
	emit_add_r32_imm(DRCTOP, REG_ESP, 12);													// add  esp,12
	emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);											// mov  ebp,[icount]
}


/*------------------------------------------------------------------
    append_check_interrupts
------------------------------------------------------------------*/

static void append_check_interrupts(drc_core *drc, int inline_generate)
{
	emit_link link1, link2, link3;
	emit_mov_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Cause));								// mov  eax,[COP0_Cause]
	emit_and_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Status));								// and  eax,[COP0_Status]
	emit_and_r32_imm(DRCTOP, REG_EAX, 0xfc00);												// and  eax,0xfc00
	if (!inline_generate)
		emit_jcc_short_link(DRCTOP, COND_Z, &link1);										// jz   skip
	else
		emit_jcc_near_link(DRCTOP, COND_Z, &link1);											// jz   skip
	emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_IE);								// test [COP0_Status,SR_IE
	if (!inline_generate)
		emit_jcc_short_link(DRCTOP, COND_Z, &link2);										// jz   skip
	else
		emit_jcc_near_link(DRCTOP, COND_Z, &link2);											// jz   skip
	emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_EXL | SR_ERL);						// test [COP0_Status,SR_EXL | SR_ERL
	if (!inline_generate)
		emit_jcc(DRCTOP, COND_Z, mips3.drcdata->generate_interrupt_exception);				// jz   generate_interrupt_exception
	else
	{
		emit_jcc_near_link(DRCTOP, COND_NZ, &link3);										// jnz  skip
		append_generate_exception(drc, EXCEPTION_INTERRUPT);								// <generate exception>
		resolve_link(DRCTOP, &link3);													// skip:
	}
	resolve_link(DRCTOP, &link1);														// skip:
	resolve_link(DRCTOP, &link2);
}


/*------------------------------------------------------------------
    append_check_sw_interrupts
------------------------------------------------------------------*/

static void append_check_sw_interrupts(drc_core *drc, int inline_generate)
{
	emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Cause), 0x300);									// test [COP0_Cause],0x300
	emit_jcc(DRCTOP, COND_NZ, mips3.drcdata->generate_interrupt_exception);					// jnz  generate_interrupt_exception
}


/*------------------------------------------------------------------
    append_branch_or_dispatch
------------------------------------------------------------------*/

static void append_branch_or_dispatch(drc_core *drc, UINT32 newpc, int cycles)
{
	void *code = drc_get_code_at_pc(drc, newpc);
	emit_mov_r32_imm(DRCTOP, REG_EDI, newpc);												// mov  edi,newpc
	drc_append_standard_epilogue(drc, cycles, 0, 1);										// <epilogue>

	if (code)
		emit_jmp(DRCTOP, code);																// jmp  code
	else
		drc_append_tentative_fixed_dispatcher(drc, newpc);									// <dispatch>
}



/***************************************************************************
    CORE RECOMPILATION
***************************************************************************/

static void ddiv(INT64 *rs, INT64 *rt)
{
	if (*rt)
	{
		mips3.core->r[REG_LO] = *rs / *rt;
		mips3.core->r[REG_HI] = *rs % *rt;
	}
}

static void ddivu(UINT64 *rs, UINT64 *rt)
{
	if (*rt)
	{
		mips3.core->r[REG_LO] = *rs / *rt;
		mips3.core->r[REG_HI] = *rs % *rt;
	}
}


/*------------------------------------------------------------------
    recompile_delay_slot
------------------------------------------------------------------*/

static int recompile_delay_slot(drc_core *drc, UINT32 pc)
{
	UINT8 *saved_top;
	UINT32 result;
	UINT32 physpc;

#ifdef MAME_DEBUG
	if (Machine->debug_mode)
	{
		/* emit debugging */
		emit_mov_r32_imm(DRCTOP, REG_EDI, pc);
		drc_append_call_debugger(drc);
	}
#endif

	saved_top = drc->cache_top;

	/* recompile the instruction as-is */
	in_delay_slot = 1;
	physpc = pc;
	mips3com_translate_address(mips3.core, ADDRESS_SPACE_PROGRAM, &physpc);
	result = recompile_instruction(drc, pc, physpc);										// <next instruction>
	in_delay_slot = 0;

	/* if the instruction can cause an exception, recompile setting nextpc */
	if (result & RECOMPILE_MAY_CAUSE_EXCEPTION)
	{
		drc->cache_top = saved_top;
		emit_mov_m32_imm(DRCTOP, MABS(&mips3.nextpc), 0);									// bogus nextpc for exceptions
		result = recompile_instruction(drc, pc, physpc);									// <next instruction>
		emit_mov_m32_imm(DRCTOP, MABS(&mips3.nextpc), ~0);									// reset nextpc
	}

	return (result >> 8) & 0xfff;
}


/*------------------------------------------------------------------
    recompile_instruction
------------------------------------------------------------------*/

static UINT32 recompile_instruction(drc_core *drc, UINT32 pc, UINT32 physpc)
{
	emit_link link1, link2 = { 0 }, link3;
	UINT32 op = cpu_readop32(physpc);
	int cycles;

	if (LOG_CODE)
		log_add_disasm_comment(mips3.log, drc->cache_top, pc, op);

	switch (op >> 26)
	{
		case 0x00:	/* SPECIAL */
			return recompile_special(drc, pc, op);

		case 0x01:	/* REGIMM */
			return recompile_regimm(drc, pc, op);

		case 0x02:	/* J */
			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, (pc & 0xf0000000) | (LIMMVAL << 2), 1+cycles);	// <branch or dispatch>
			return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;

		case 0x03:	/* JAL */
			emit_mov_m64_imm32(DRCTOP, REGADDR(31), pc + 8);								// mov  [31],pc + 8
			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, (pc & 0xf0000000) | (LIMMVAL << 2), 1+cycles);	// <branch or dispatch>
			return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;

		case 0x04:	/* BEQ */
			if (RSREG == RTREG)
			{
				cycles = recompile_delay_slot(drc, pc + 4);									// <next instruction>
				append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);			// <branch or dispatch>
				return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
			}
			else if (RSREG == 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[rtreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// or   eax,[rtreg].hi
				emit_jcc_near_link(DRCTOP, COND_NZ, &link1);								// jnz  skip
			}
			else if (RTREG == 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// or   eax,[rsreg].hi
				emit_jcc_near_link(DRCTOP, COND_NZ, &link1);								// jnz  skip
			}
			else
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// cmp  eax,[rtreg].lo
				emit_jcc_near_link(DRCTOP, COND_NE, &link1);								// jne  skip
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));						// mov  eax,[rsreg].hi
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));						// cmp  eax,[rtreg].hi
				emit_jcc_near_link(DRCTOP, COND_NE, &link2);								// jne  skip
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			if (RSREG != 0 && RTREG != 0)
				resolve_link(DRCTOP, &link2);											// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x05:	/* BNE */
			if (RSREG == 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[rtreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// or   eax,[rtreg].hi
				emit_jcc_near_link(DRCTOP, COND_Z, &link1);									// jz   skip
			}
			else if (RTREG == 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// or   eax,[rsreg].hi
				emit_jcc_near_link(DRCTOP, COND_Z, &link1);									// jz   skip
			}
			else
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// cmp  eax,[rtreg].lo
				emit_jcc_short_link(DRCTOP, COND_NE, &link2);								// jne  takeit
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));						// mov  eax,[rsreg].hi
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));						// cmp  eax,[rtreg].hi
				emit_jcc_near_link(DRCTOP, COND_E, &link1);									// je   skip
				resolve_link(DRCTOP, &link2);											// takeit:
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x06:	/* BLEZ */
			if (RSREG == 0)
			{
				cycles = recompile_delay_slot(drc, pc + 4);									// <next instruction>
				append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);			// <branch or dispatch>
				return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
			}
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_G, &link1);									// jg   skip
				emit_jcc_short_link(DRCTOP, COND_L, &link2);								// jl   takeit
				emit_cmp_m32_imm(DRCTOP, REGADDRLO(RSREG), 0);								// cmp  [rsreg].lo,0
				emit_jcc_near_link(DRCTOP, COND_NE, &link3);								// jne  skip
				resolve_link(DRCTOP, &link2);											// takeit:
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			resolve_link(DRCTOP, &link3);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x07:	/* BGTZ */
			if (RSREG == 0)
				return RECOMPILE_SUCCESSFUL_CP(1,4);
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_L, &link1);									// jl   skip
				emit_jcc_short_link(DRCTOP, COND_G, &link2);								// jg   takeit
				emit_cmp_m32_imm(DRCTOP, REGADDRLO(RSREG), 0);								// cmp  [rsreg].lo,0
				emit_jcc_near_link(DRCTOP, COND_E, &link3);									// je   skip
				resolve_link(DRCTOP, &link2);											// takeit:
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			resolve_link(DRCTOP, &link3);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x08:	/* ADDI */
			if (RSREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_jcc(DRCTOP, COND_O, mips3.drcdata->generate_overflow_exception);		// jo   generate_overflow_exception
				if (RTREG != 0)
				{
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);				// mov  [rtreg],edx:eax
				}
			}
			else if (RTREG != 0)
				emit_mov_m64_imm32(DRCTOP, REGADDR(RTREG), SIMMVAL);						// mov  [rtreg],const
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x09:	/* ADDIU */
			if (RTREG != 0)
			{
				if (RSREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));					// mov  eax,[rsreg].lo
					emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);								// add  eax,SIMMVAL
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);				// mov  [rtreg],edx:eax
				}
				else
					emit_mov_m64_imm32(DRCTOP, REGADDR(RTREG), SIMMVAL);					// mov  [rtreg],const
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x0a:	/* SLTI */
			if (RTREG != 0)
			{
				if (RSREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));				// mov  edx:eax,[rsreg]
					emit_sub_r32_imm(DRCTOP, REG_EAX, SIMMVAL);								// sub  eax,[rtreg].lo
					emit_sbb_r32_imm(DRCTOP, REG_EDX, ((INT32)SIMMVAL >> 31));				// sbb  edx,[rtreg].lo
					emit_shr_r32_imm(DRCTOP, REG_EDX, 31);									// shr  edx,31
					emit_mov_m32_r32(DRCTOP, REGADDRLO(RTREG), REG_EDX);					// mov  [rdreg].lo,edx
					emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);							// mov  [rdreg].hi,0
				}
				else
				{
					emit_mov_m32_imm(DRCTOP, REGADDRLO(RTREG), (0 < SIMMVAL));				// mov  [rtreg].lo,const
					emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);							// mov  [rtreg].hi,sign-extend(const)
				}
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x0b:	/* SLTIU */
			if (RTREG != 0)
			{
				if (RSREG != 0)
				{
					emit_xor_r32_r32(DRCTOP, REG_ECX, REG_ECX);								// xor  ecx,ecx
					emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), ((INT32)SIMMVAL >> 31));		// cmp  [rsreg].hi,upper
					emit_jcc_short_link(DRCTOP, COND_B, &link1);							// jb   takeit
					emit_jcc_short_link(DRCTOP, COND_A, &link2);							// ja   skip
					emit_cmp_m32_imm(DRCTOP, REGADDRLO(RSREG), SIMMVAL);					// cmp  [rsreg].lo,lower
					emit_jcc_short_link(DRCTOP, COND_AE, &link3);							// jae  skip
					resolve_link(DRCTOP, &link1);										// takeit:
					emit_add_r32_imm(DRCTOP, REG_ECX, 1);									// add  ecx,1
					resolve_link(DRCTOP, &link2);										// skip:
					resolve_link(DRCTOP, &link3);										// skip:
					emit_mov_m32_r32(DRCTOP, REGADDRLO(RTREG), REG_ECX);					// mov  [rtreg].lo,ecx
					emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);							// mov  [rtreg].hi,sign-extend(const)
				}
				else
				{
					emit_mov_m32_imm(DRCTOP, REGADDRLO(RTREG), (0 < SIMMVAL));				// mov  [rtreg].lo,const
					emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);							// mov  [rtreg].hi,sign-extend(const)
				}
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x0c:	/* ANDI */
			if (RTREG != 0)
			{
				if (RSREG == RTREG)
				{
					emit_and_m32_imm(DRCTOP, REGADDRLO(RTREG), UIMMVAL);					// and  [rtreg],UIMMVAL
					emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);							// mov  [rtreg].hi,0
				}
				else if (RSREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));					// mov  eax,[rsreg].lo
					emit_and_r32_imm(DRCTOP, REG_EAX, UIMMVAL);								// and  eax,UIMMVAL
					emit_mov_m32_r32(DRCTOP, REGADDRLO(RTREG), REG_EAX);					// mov  [rtreg].lo,eax
					emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);							// mov  [rtreg].hi,0
				}
				else
					emit_mov_m64_imm32(DRCTOP, REGADDR(RTREG), 0);							// mov  [rtreg],0
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x0d:	/* ORI */
			if (RTREG != 0)
			{
				if (RSREG == RTREG)
					emit_or_m32_imm(DRCTOP, REGADDRLO(RTREG), UIMMVAL);						// or   [rtreg].lo,UIMMVAL
				else if (RSREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));				// mov  edx:eax,[rsreg]
					emit_or_r32_imm(DRCTOP, REG_EAX, UIMMVAL);								// or   eax,UIMMVAL
					emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);				// mov  [rtreg],edx:eax
				}
				else
					emit_mov_m64_imm32(DRCTOP, REGADDR(RTREG), UIMMVAL);					// mov  [rtreg],const
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x0e:	/* XORI */
			if (RTREG != 0)
			{
				if (RSREG == RTREG)
					emit_xor_m32_imm(DRCTOP, REGADDRLO(RTREG), UIMMVAL);					// xor  [rtreg].lo,UIMMVAL
				else if (RSREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));				// mov  edx:eax,[rsreg]
					emit_xor_r32_imm(DRCTOP, REG_EAX, UIMMVAL);								// xor  eax,UIMMVAL
					emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);				// mov  [rtreg],edx:eax
				}
				else
					emit_mov_m64_imm32(DRCTOP, REGADDR(RTREG), UIMMVAL);					// mov  [rtreg],const
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x0f:	/* LUI */
			if (RTREG != 0)
				emit_mov_m64_imm32(DRCTOP, REGADDR(RTREG), UIMMVAL << 16);					// mov  [rtreg],const << 16
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x10:	/* COP0 */
			return recompile_cop0(drc, pc, op) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x11:	/* COP1 */
			return recompile_cop1(drc, pc, op) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x12:	/* COP2 */
			emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);			// jmp  generate_invalidop_exception
			return RECOMPILE_SUCCESSFUL | RECOMPILE_MAY_CAUSE_EXCEPTION | RECOMPILE_END_OF_STRING;

		case 0x13:	/* COP1X - R5000 */
			if (mips3.core->flavor < MIPS3_TYPE_MIPS_IV)
			{
				emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);		// jmp  generate_invalidop_exception
				return RECOMPILE_SUCCESSFUL | RECOMPILE_MAY_CAUSE_EXCEPTION | RECOMPILE_END_OF_STRING;
			}
			return recompile_cop1x(drc, pc, op) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x14:	/* BEQL */
			if (RSREG == RTREG)
			{
				cycles = recompile_delay_slot(drc, pc + 4);									// <next instruction>
				append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);			// <branch or dispatch>
				return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
			}
			else if (RSREG == 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[rtreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// or   eax,[rtreg].hi
				emit_jcc_near_link(DRCTOP, COND_NZ, &link1);								// jnz  skip
			}
			else if (RTREG == 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// or   eax,[rsreg].hi
				emit_jcc_near_link(DRCTOP, COND_NZ, &link1);								// jnz  skip
			}
			else
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// cmp  eax,[rtreg].lo
				emit_jcc_near_link(DRCTOP, COND_NE, &link1);								// jne  skip
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));						// mov  eax,[rsreg].hi
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));						// cmp  eax,[rtreg].hi
				emit_jcc_near_link(DRCTOP, COND_NE, &link2);								// jne  skip
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			if (RSREG != 0 && RTREG != 0)
				resolve_link(DRCTOP, &link2);											// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,8);

		case 0x15:	/* BNEL */
			if (RSREG == 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[rtreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// or   eax,[rtreg].hi
				emit_jcc_near_link(DRCTOP, COND_Z, &link1);									// jz   skip
			}
			else if (RTREG == 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// or   eax,[rsreg].hi
				emit_jcc_near_link(DRCTOP, COND_Z, &link1);									// jz   skip
			}
			else
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// cmp  eax,[rtreg].lo
				emit_jcc_short_link(DRCTOP, COND_NE, &link2);								// jne  takeit
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));						// mov  eax,[rsreg].hi
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));						// cmp  eax,[rtreg].hi
				emit_jcc_near_link(DRCTOP, COND_E, &link1);									// je   skip
				resolve_link(DRCTOP, &link2);											// takeit:
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,8);

		case 0x16:	/* BLEZL */
			if (RSREG == 0)
			{
				cycles = recompile_delay_slot(drc, pc + 4);									// <next instruction>
				append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);			// <branch or dispatch>
				return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
			}
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_G, &link1);									// jg   skip
				emit_jcc_short_link(DRCTOP, COND_L, &link2);								// jl   takeit
				emit_cmp_m32_imm(DRCTOP, REGADDRLO(RSREG), 0);								// cmp  [rsreg].lo,0
				emit_jcc_near_link(DRCTOP, COND_NE, &link3);								// jne  skip
				resolve_link(DRCTOP, &link2);											// takeit:
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			resolve_link(DRCTOP, &link3);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,8);

		case 0x17:	/* BGTZL */
			if (RSREG == 0)
				return RECOMPILE_SUCCESSFUL_CP(1,8);
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_L, &link1);									// jl   skip
				emit_jcc_short_link(DRCTOP, COND_G, &link2);								// jg   takeit
				emit_cmp_m32_imm(DRCTOP, REGADDRLO(RSREG), 0);								// cmp  [rsreg].lo,0
				emit_jcc_near_link(DRCTOP, COND_E, &link3);									// je   skip
				resolve_link(DRCTOP, &link2);											// takeit:
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			resolve_link(DRCTOP, &link3);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,8);

		case 0x18:	/* DADDI */
			if (RSREG != 0)
			{
				emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));					// mov  edx:eax,[rsreg]
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_adc_r32_imm(DRCTOP, REG_EDX, (SIMMVAL < 0) ? -1 : 0);					// adc  edx,signext(SIMMVAL)
				emit_jcc(DRCTOP, COND_O, mips3.drcdata->generate_overflow_exception);		// jo   generate_overflow_exception
				if (RTREG != 0)
					emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);				// mov  [rtreg],edx:eax
			}
			else if (RTREG != 0)
				emit_mov_m64_imm32(DRCTOP, REGADDR(RTREG), SIMMVAL);						// mov  [rtreg],const
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x19:	/* DADDIU */
			if (RTREG != 0)
			{
				if (RSREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));				// mov  edx:eax,[rsreg]
					emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);								// add  eax,SIMMVAL
					emit_adc_r32_imm(DRCTOP, REG_EDX, (SIMMVAL < 0) ? -1 : 0);				// adc  edx,signext(SIMMVAL)
					emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);				// mov  [rtreg],edx:eax
				}
				else
					emit_mov_m64_imm32(DRCTOP, REGADDR(RTREG), SIMMVAL);					// mov  [rtreg],const
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x1a:	/* LDL */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			if (SIMMVAL)
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
			emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_EAX, 4, 0));							// lea  ecx,[eax*4]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~7);											// and  eax,~7
			emit_mov_r32_imm(DRCTOP, REG_EBX, -1);											// mov  ebx,-1
			emit_mov_r32_imm(DRCTOP, REG_EBP, -1);											// mov  ebp,-1
			if (!mips3.core->bigendian)
				emit_xor_r32_imm(DRCTOP, REG_ECX, 0x1c);									// xor  ecx,0x1c
			emit_shr_r32_cl(DRCTOP, REG_EBP);												// shr  ebp,cl
			emit_shrd_r32_r32_cl(DRCTOP, REG_EBX, REG_EBP);									// shrd ebx,ebp,cl
			emit_shr_r32_cl(DRCTOP, REG_EBP);												// shr  ebp,cl
			emit_not_r32(DRCTOP, REG_EBX);													// not  ebx
			emit_not_r32(DRCTOP, REG_EBP);													// not  ebp
			emit_push_r32(DRCTOP, REG_ECX);													// push ecx
			emit_push_r32(DRCTOP, REG_EBP);													// push ebp
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_double_masked);				// call read_and_translate_double_masked
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_pop_r32(DRCTOP, REG_ECX);													// pop  ecx

			if (RTREG != 0)
			{
				emit_mov_r32_imm(DRCTOP, REG_EBX, -1);										// mov  ebx,-1
				emit_mov_r32_imm(DRCTOP, REG_EBP, -1);										// mov  ebp,-1
				emit_shl_r32_cl(DRCTOP, REG_EBX);											// shl  ebx,cl
				emit_shld_r32_r32_cl(DRCTOP, REG_EBP, REG_EBX);								// shld ebp,ebx,cl
				emit_shl_r32_cl(DRCTOP, REG_EBX);											// shl  ebx,cl
				emit_not_r32(DRCTOP, REG_EBX);												// not  ebx
				emit_not_r32(DRCTOP, REG_EBP);												// not  ebp
				emit_shld_r32_r32_cl(DRCTOP, REG_EDX, REG_EAX);								// shld edx,eax,cl
				emit_shl_r32_cl(DRCTOP, REG_EAX);											// shl  eax,cl
				emit_shld_r32_r32_cl(DRCTOP, REG_EDX, REG_EAX);								// shld edx,eax,cl
				emit_shl_r32_cl(DRCTOP, REG_EAX);											// shl  eax,cl
				emit_and_r32_m32(DRCTOP, REG_EBX, REGADDRLO(RTREG));						// and  ebx,[rtreg].lo
				emit_and_r32_m32(DRCTOP, REG_EBP, REGADDRHI(RTREG));						// and  ebp,[rtreg].hi
				emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);									// or   eax,ebx
				emit_or_r32_r32(DRCTOP, REG_EDX, REG_EBP);									// or   edx,ebp
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x1b:	/* LDR */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			if (SIMMVAL)
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
			emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_EAX, 4, 0));							// lea  ecx,[eax*4]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~7);											// and  eax,~7
			emit_mov_r32_imm(DRCTOP, REG_EBX, -1);											// mov  ebx,-1
			emit_mov_r32_imm(DRCTOP, REG_EBP, -1);											// mov  ebp,-1
			if (mips3.core->bigendian)
				emit_xor_r32_imm(DRCTOP, REG_ECX, 0x1c);									// xor  ecx,0x1c
			emit_shl_r32_cl(DRCTOP, REG_EBX);												// shl  ebx,cl
			emit_shld_r32_r32_cl(DRCTOP, REG_EBP, REG_EBX);									// shld ebp,ebx,cl
			emit_shl_r32_cl(DRCTOP, REG_EBX);												// shl  ebx,cl
			emit_not_r32(DRCTOP, REG_EBX);													// not  ebx
			emit_not_r32(DRCTOP, REG_EBP);													// not  ebp
			emit_push_r32(DRCTOP, REG_ECX);													// push ecx
			emit_push_r32(DRCTOP, REG_EBP);													// push ebp
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_double_masked);				// call read_and_translate_double_masked
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_pop_r32(DRCTOP, REG_ECX);													// pop  ecx

			if (RTREG != 0)
			{
				emit_mov_r32_imm(DRCTOP, REG_EBX, -1);										// mov  ebx,-1
				emit_mov_r32_imm(DRCTOP, REG_EBP, -1);										// mov  ebp,-1
				emit_shr_r32_cl(DRCTOP, REG_EBP);											// shr  ebp,cl
				emit_shrd_r32_r32_cl(DRCTOP, REG_EBX, REG_EBP);								// shrd ebx,ebp,cl
				emit_shr_r32_cl(DRCTOP, REG_EBP);											// shr  ebp,cl
				emit_not_r32(DRCTOP, REG_EBX);												// not  ebx
				emit_not_r32(DRCTOP, REG_EBP);												// not  ebp
				emit_shrd_r32_r32_cl(DRCTOP, REG_EAX, REG_EDX);								// shld eax,edx,cl
				emit_shr_r32_cl(DRCTOP, REG_EDX);											// shl  edx,cl
				emit_shrd_r32_r32_cl(DRCTOP, REG_EAX, REG_EDX);								// shld eax,edx,cl
				emit_shr_r32_cl(DRCTOP, REG_EDX);											// shl  edx,cl
				emit_and_r32_m32(DRCTOP, REG_EBX, REGADDRLO(RTREG));						// and  ebx,[rtreg].lo
				emit_and_r32_m32(DRCTOP, REG_EBP, REGADDRHI(RTREG));						// and  ebp,[rtreg].hi
				emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);									// or   eax,ebx
				emit_or_r32_r32(DRCTOP, REG_EDX, REG_EBP);									// or   edx,ebp
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x1c:	/* IDT-specific opcodes: mad/madu/mul on R4640/4650, msub on RC32364 */
			switch (op & 0x1f)
			{
				case 0: /* MAD */
					if (RSREG != 0 && RTREG != 0)
					{
						emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));				// mov  eax,[rsreg]
						emit_mov_r32_m32(DRCTOP, REG_EDX, REGADDRLO(RTREG));				// mov  edx,[rtreg]
						emit_imul_r32(DRCTOP, REG_EDX);										// imul edx
						emit_add_r32_m32(DRCTOP, REG_EAX, LOADDRLO);						// add  eax,[lo]
						emit_adc_r32_m32(DRCTOP, REG_EDX, HIADDRLO);						// adc  edx,[hi]
						emit_mov_r32_r32(DRCTOP, REG_EBX, REG_EDX);							// mov  ebx,edx
						emit_cdq(DRCTOP);													// cdq
						emit_mov_m64_r64(DRCTOP, LOADDR, REG_EDX, REG_EAX);					// mov  [lo],edx:eax
						emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EBX);							// mov  eax,ebx
						emit_cdq(DRCTOP);													// cdq
						emit_mov_m64_r64(DRCTOP, HIADDR, REG_EDX, REG_EAX);					// mov  [hi],edx:eax
					}
					return RECOMPILE_SUCCESSFUL_CP(3,4);

				case 1: /* MADU */
					if (RSREG != 0 && RTREG != 0)
					{
						emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));				// mov  eax,[rsreg]
						emit_mov_r32_m32(DRCTOP, REG_EDX, REGADDRLO(RTREG));				// mov  edx,[rtreg]
						emit_mul_r32(DRCTOP, REG_EDX);										// mul  edx
						emit_add_r32_m32(DRCTOP, REG_EAX, LOADDRLO);						// add  eax,[lo]
						emit_adc_r32_m32(DRCTOP, REG_EDX, HIADDRLO);						// adc  edx,[hi]
						emit_mov_r32_r32(DRCTOP, REG_EBX, REG_EDX);							// mov  ebx,edx
						emit_cdq(DRCTOP);													// cdq
						emit_mov_m64_r64(DRCTOP, LOADDR, REG_EDX, REG_EAX);					// mov  [lo],edx:eax
						emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EBX);							// mov  eax,ebx
						emit_cdq(DRCTOP);													// cdq
						emit_mov_m64_r64(DRCTOP, HIADDR, REG_EDX, REG_EAX);					// mov  [hi],edx:eax
					}
					return RECOMPILE_SUCCESSFUL_CP(3,4);

				case 2: /* MUL */
					if (RDREG != 0)
					{
						if (RSREG != 0 && RTREG != 0)
						{
							emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));			// mov  eax,[rsreg].lo
							emit_imul_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));			// imul eax,[rtreg].lo
							emit_cdq(DRCTOP);												// cdq
							emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);		// mov  [rdreg],edx:eax
						}
						else
							emit_zero_m64(DRCTOP, REGADDR(RDREG));							// mov  [rdreg],0
					}
					return RECOMPILE_SUCCESSFUL_CP(3,4);
			}
			break;

		case 0x20:	/* LB */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_byte_signed);				// call read_and_translate_byte_signed
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			if (RTREG != 0)
			{
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x21:	/* LH */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_half_signed);				// call read_and_translate_half_signed
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			if (RTREG != 0)
			{
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x22:	/* LWL */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			if (SIMMVAL)
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
			emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_EAX, 8, 0));							// lea  ecx,[eax*8]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~3);											// and  eax,~3
			emit_mov_r32_imm(DRCTOP, REG_EBX, -1);											// mov  ebx,-1
			if (!mips3.core->bigendian)
				emit_xor_r32_imm(DRCTOP, REG_ECX, 0x18);									// xor  ecx,0x18
			emit_shr_r32_cl(DRCTOP, REG_EBX);												// shr  ebx,cl
			emit_not_r32(DRCTOP, REG_EBX);													// not  ebx
			emit_push_r32(DRCTOP, REG_ECX);													// push ecx
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_word_masked);				// call read_and_translate_word_masked
			emit_add_r32_imm(DRCTOP, REG_ESP, 8);											// add  esp,8
			emit_pop_r32(DRCTOP, REG_ECX);													// pop  ecx

			if (RTREG != 0)
			{
				emit_mov_r32_imm(DRCTOP, REG_EBX, -1);										// mov  ebx,-1
				emit_shl_r32_cl(DRCTOP, REG_EBX);											// shl  ebx,cl
				emit_not_r32(DRCTOP, REG_EBX);												// not  ebx
				emit_shl_r32_cl(DRCTOP, REG_EAX);											// shl  eax,cl
				emit_and_r32_m32(DRCTOP, REG_EBX, REGADDRLO(RTREG));						// and  ebx,[rtreg].lo
				emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);									// or   eax,ebx
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);											// add  esp,4
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x23:	/* LW */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_word);						// call read_and_translate_word
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			if (RTREG != 0)
			{
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x24:	/* LBU */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_byte_unsigned);				// call read_and_translate_byte_unsigned
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			if (RTREG != 0)
			{
				emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);								// mov  [rtreg].hi,0
				emit_mov_m32_r32(DRCTOP, REGADDRLO(RTREG), REG_EAX);						// mov  [rtreg].lo,eax
			}
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x25:	/* LHU */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_half_unsigned);				// call read_and_translate_half_unsigned
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			if (RTREG != 0)
			{
				emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);								// mov  [rtreg].hi,0
				emit_mov_m32_r32(DRCTOP, REGADDRLO(RTREG), REG_EAX);						// mov  [rtreg].lo,eax
			}
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x26:	/* LWR */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			if (SIMMVAL)
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
			emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_EAX, 8, 0));							// lea  ecx,[eax*8]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~3);											// and  eax,~3
			emit_mov_r32_imm(DRCTOP, REG_EBX, -1);											// mov  ebx,-1
			if (mips3.core->bigendian)
				emit_xor_r32_imm(DRCTOP, REG_ECX, 0x18);									// xor  ecx,0x18
			emit_shl_r32_cl(DRCTOP, REG_EBX);												// shl  ebx,cl
			emit_not_r32(DRCTOP, REG_EBX);													// not  ebx
			emit_push_r32(DRCTOP, REG_ECX);													// push ecx
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_word_masked);				// call read_and_translate_word_masked
			emit_add_r32_imm(DRCTOP, REG_ESP, 8);											// add  esp,8
			emit_pop_r32(DRCTOP, REG_ECX);													// pop  ecx

			if (RTREG != 0)
			{
				emit_mov_r32_imm(DRCTOP, REG_EBX, -1);										// mov  ebx,-1
				emit_shr_r32_cl(DRCTOP, REG_EBX);											// shr  ebx,cl
				emit_not_r32(DRCTOP, REG_EBX);												// not  ebx
				emit_shr_r32_cl(DRCTOP, REG_EAX);											// shr  eax,cl
				emit_and_r32_m32(DRCTOP, REG_EBX, REGADDRLO(RTREG));						// and  ebx,[rtreg].lo
				emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);									// or   eax,ebx
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			emit_add_r32_imm(DRCTOP, REG_ESP, 4);											// add  esp,4
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x27:	/* LWU */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_word);						// call read_and_translate_word
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			if (RTREG != 0)
			{
				emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);								// mov  [rtreg].hi,0
				emit_mov_m32_r32(DRCTOP, REGADDRLO(RTREG), REG_EAX);						// mov  [rtreg].lo,eax
			}
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x28:	/* SB */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			if (RTREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RTREG));									// push dword [rtreg].lo
			else
				emit_push_imm(DRCTOP, 0);													// push 0
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_byte);						// call writebyte
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x29:	/* SH */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			if (RTREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RTREG));									// push dword [rtreg].lo
			else
				emit_push_imm(DRCTOP, 0);													// push 0
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_half);						// call writehalf
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2a:	/* SWL */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			if (SIMMVAL)
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
			emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_EAX, 8, 0));							// lea  ecx,[eax*8]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~3);											// and  eax,~3
			emit_mov_r32_imm(DRCTOP, REG_EBX, -1);											// mov  ebx,-1
			if (!mips3.core->bigendian)
				emit_xor_r32_imm(DRCTOP, REG_ECX, 0x18);									// xor  ecx,0x18
			emit_shr_r32_cl(DRCTOP, REG_EBX);												// shr  ebx,cl
			emit_not_r32(DRCTOP, REG_EBX);													// not  ebx
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EDX, REGADDRLO(RTREG));						// mov  edx,[rtreg].lo
				emit_shr_r32_cl(DRCTOP, REG_EDX);											// shr  edx,cl
				emit_push_r32(DRCTOP, REG_EDX);												// push edx
			}
			else
				emit_push_imm(DRCTOP, 0);													// push 0
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_word_masked);				// call write_and_translate_word_masked
			emit_add_r32_imm(DRCTOP, REG_ESP, 16);											// add  esp,16
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2b:	/* SW */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			if (RTREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RTREG));									// push dword [rtreg].lo
			else
				emit_push_imm(DRCTOP, 0);													// push 0
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_word);						// call write_and_translate_word
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2c:	/* SDL */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 12);											// sub  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			if (SIMMVAL)
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
			emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_EAX, 4, 0));							// lea  ecx,[eax*4]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~7);											// and  eax,~7
			emit_mov_r32_imm(DRCTOP, REG_EBX, -1);											// mov  ebx,-1
			emit_mov_r32_imm(DRCTOP, REG_EBP, -1);											// mov  ebp,-1
			if (!mips3.core->bigendian)
				emit_xor_r32_imm(DRCTOP, REG_ECX, 0x1c);									// xor  ecx,0x1c
			emit_shr_r32_cl(DRCTOP, REG_EBP);												// shr  ebp,cl
			emit_shrd_r32_r32_cl(DRCTOP, REG_EBX, REG_EBP);									// shrd ebx,ebp,cl
			emit_shr_r32_cl(DRCTOP, REG_EBP);												// shr  ebp,cl
			emit_not_r32(DRCTOP, REG_EBP);													// not  ebp
			emit_not_r32(DRCTOP, REG_EBX);													// not  ebx
			emit_push_r32(DRCTOP, REG_EBP);													// push ebp
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EBX, REGADDRLO(RTREG));						// mov  ebx,[rtreg].lo
				emit_mov_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));						// mov  edx,[rtreg].hi
				emit_shrd_r32_r32_cl(DRCTOP, REG_EBX, REG_EDX);								// shrd ebx,edx,cl
				emit_shr_r32_cl(DRCTOP, REG_EDX);											// shr  edx,cl
				emit_shrd_r32_r32_cl(DRCTOP, REG_EBX, REG_EDX);								// shrd ebx,edx,cl
				emit_shr_r32_cl(DRCTOP, REG_EDX);											// shr  edx,cl
				emit_push_r32(DRCTOP, REG_EDX);												// push edx
				emit_push_r32(DRCTOP, REG_EBX);												// push ebx
			}
			else
			{
				emit_push_imm(DRCTOP, 0);													// push 0
				emit_push_imm(DRCTOP, 0);													// push 0
			}
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_double_masked);			// call write_and_translate_double_masked
			emit_add_r32_imm(DRCTOP, REG_ESP, 32);											// add  esp,32
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2d:	/* SDR */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 12);											// sub  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			if (SIMMVAL)
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
			emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_EAX, 4, 0));							// lea  ecx,[eax*4]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~7);											// and  eax,~7
			emit_mov_r32_imm(DRCTOP, REG_EBX, -1);											// mov  ebx,-1
			emit_mov_r32_imm(DRCTOP, REG_EBP, -1);											// mov  ebp,-1
			if (mips3.core->bigendian)
				emit_xor_r32_imm(DRCTOP, REG_ECX, 0x1c);									// xor  ecx,0x1c
			emit_shl_r32_cl(DRCTOP, REG_EBX);												// shl  ebx,cl
			emit_shld_r32_r32_cl(DRCTOP, REG_EBP, REG_EBX);									// shld ebp,ebx,cl
			emit_shl_r32_cl(DRCTOP, REG_EBX);												// shl  ebx,cl
			emit_not_r32(DRCTOP, REG_EBP);													// not  ebp
			emit_not_r32(DRCTOP, REG_EBX);													// not  ebx
			emit_push_r32(DRCTOP, REG_EBP);													// push ebp
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EBX, REGADDRLO(RTREG));						// mov  ebx,[rtreg].lo
				emit_mov_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));						// mov  edx,[rtreg].hi
				emit_shld_r32_r32_cl(DRCTOP, REG_EDX, REG_EBX);								// shld edx,ebx,cl
				emit_shl_r32_cl(DRCTOP, REG_EBX);											// shl  ebx,cl
				emit_shld_r32_r32_cl(DRCTOP, REG_EDX, REG_EBX);								// shld edx,ebx,cl
				emit_shl_r32_cl(DRCTOP, REG_EBX);											// shl  ebx,cl
				emit_push_r32(DRCTOP, REG_EDX);												// push edx
				emit_push_r32(DRCTOP, REG_EBX);												// push ebx
			}
			else
			{
				emit_push_imm(DRCTOP, 0);													// push 0
				emit_push_imm(DRCTOP, 0);													// push 0
			}
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_double_masked);			// call write_and_translate_double_masked
			emit_add_r32_imm(DRCTOP, REG_ESP, 32);											// add  esp,32
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2e:	/* SWR */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			if (SIMMVAL)
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
			emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_EAX, 8, 0));							// lea  ecx,[eax*8]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~3);											// and  eax,~3
			emit_mov_r32_imm(DRCTOP, REG_EBX, -1);											// mov  ebx,-1
			if (mips3.core->bigendian)
				emit_xor_r32_imm(DRCTOP, REG_ECX, 0x18);									// xor  ecx,0x18
			emit_shl_r32_cl(DRCTOP, REG_EBX);												// shl  ebx,cl
			emit_not_r32(DRCTOP, REG_EBX);													// not  ebx
			emit_push_r32(DRCTOP, REG_EBX);													// push ebx
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EDX, REGADDRLO(RTREG));						// mov  edx,[rtreg].lo
				emit_shl_r32_cl(DRCTOP, REG_EDX);											// shl  edx,cl
				emit_push_r32(DRCTOP, REG_EDX);												// push edx
			}
			else
				emit_push_imm(DRCTOP, 0);													// push 0
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_word_masked);				// call write_and_translate_word_masked
			emit_add_r32_imm(DRCTOP, REG_ESP, 16);											// add  esp,16
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2f:	/* CACHE */
			return RECOMPILE_SUCCESSFUL_CP(1,4);

//      case 0x30:  /* LL */        logerror("mips3 Unhandled op: LL\n");                                   break;

		case 0x31:	/* LWC1 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].los
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_word);						// call read_and_translate_word
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_m32_r32(DRCTOP, FPR32ADDR(RTREG), REG_EAX);							// mov  [rtreg],eax
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x32:	/* LWC2 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_word);						// call read_and_translate_word
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_m32_r32(DRCTOP, CPR2ADDR(RTREG), REG_EAX);								// mov  [rtreg],eax
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x33:	/* PREF */
			if (mips3.core->flavor >= MIPS3_TYPE_MIPS_IV)
				return RECOMPILE_SUCCESSFUL_CP(1,4);
			else
			{
				emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);		// jmp  generate_invalidop_exception
				return RECOMPILE_SUCCESSFUL | RECOMPILE_MAY_CAUSE_EXCEPTION | RECOMPILE_END_OF_STRING;
			}

//      case 0x34:  /* LLD */       logerror("mips3 Unhandled op: LLD\n");                                  break;

		case 0x35:	/* LDC1 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_double);					// call read_and_translate_double
			emit_mov_m64_r64(DRCTOP, FPR64ADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x36:	/* LDC2 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_double);					// call read_and_translate_word
			emit_mov_m64_r64(DRCTOP, CPR2ADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x37:	/* LD */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_double);					// call read_and_translate_double
			if (RTREG != 0)
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

//      case 0x38:  /* SC */        logerror("mips3 Unhandled op: SC\n");                                   break;

		case 0x39:	/* SWC1 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			emit_push_m32(DRCTOP, FPR32ADDR(RTREG));										// push dword [rtreg]
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_word);						// call write_and_translate_word
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x3a:	/* SWC2 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			emit_push_m32(DRCTOP, CPR2ADDR(RTREG));											// push dword [rtreg]
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_word);						// call write_and_translate_word
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

//      case 0x3b:  /* SWC3 */      invalid_instruction(op);                                                break;
//      case 0x3c:  /* SCD */       logerror("mips3 Unhandled op: SCD\n");                                  break;

		case 0x3d:	/* SDC1 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_push_m32(DRCTOP, FPR64ADDRHI(RTREG));										// push dword [rtreg].hi
			emit_push_m32(DRCTOP, FPR64ADDRLO(RTREG));										// push dword [rtreg].lo
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_double);					// call write_and_translate_double
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x3e:	/* SDC2 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_push_m32(DRCTOP, CPR2ADDRHI(RTREG));										// push dword [rtreg].hi
			emit_push_m32(DRCTOP, CPR2ADDRLO(RTREG));										// push dword [rtreg].lo
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_double);					// call write_and_translate_double
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x3f:	/* SD */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			if (RTREG != 0)
			{
				emit_push_m32(DRCTOP, REGADDRHI(RTREG));									// push   dword [rtreg].hi
				emit_push_m32(DRCTOP, REGADDRLO(RTREG));									// push   dword [rtreg].lo
			}
			else
			{
				emit_push_imm(DRCTOP, 0);													// push 0
				emit_push_imm(DRCTOP, 0);													// push 0
			}
			if (RSREG != 0 && SIMMVAL != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// add  eax,SIMMVAL
				emit_push_r32(DRCTOP, REG_EAX);												// push eax
			}
			else if (RSREG != 0)
				emit_push_m32(DRCTOP, REGADDRLO(RSREG));									// push [rsreg].lo
			else
				emit_push_imm(DRCTOP, SIMMVAL);												// push SIMMVAL
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_double);					// call write_and_translate_double
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

//      default:    /* ??? */       invalid_instruction(op);                                                break;
	}
	return RECOMPILE_UNIMPLEMENTED;
}


/*------------------------------------------------------------------
    recompile_special
------------------------------------------------------------------*/

static UINT32 recompile_special1(drc_core *drc, UINT32 pc, UINT32 op)
{
	emit_link link1, link2, link3;
	int cycles;

	switch (op & 63)
	{
		case 0x00:	/* SLL */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					if (SHIFT != 0)
						emit_shl_r32_imm(DRCTOP, REG_EAX, SHIFT);							// shl  eax,SHIFT
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x01:	/* MOVF - R5000*/
			if (mips3.core->flavor < MIPS3_TYPE_MIPS_IV)
			{
				emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);		// jmp  generate_invalidop_exception
				return RECOMPILE_SUCCESSFUL | RECOMPILE_MAY_CAUSE_EXCEPTION | RECOMPILE_END_OF_STRING;
			}
			emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);							// cmp  [cf[x]],0
			emit_jcc_short_link(DRCTOP, ((op >> 16) & 1) ? COND_Z : COND_NZ, &link1);		// jz/nz skip
			emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));						// mov  edx:eax,[rsreg]
			emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);						// mov  [rdreg],edx:eax
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x02:	/* SRL */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					if (SHIFT != 0)
						emit_shr_r32_imm(DRCTOP, REG_EAX, SHIFT);							// shr  eax,SHIFT
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x03:	/* SRA */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					if (SHIFT != 0)
						emit_sar_r32_imm(DRCTOP, REG_EAX, SHIFT);							// sar  eax,SHIFT
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x04:	/* SLLV */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					if (RSREG != 0)
					{
						emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RSREG));				// mov  ecx,[rsreg].lo
						emit_shl_r32_cl(DRCTOP, REG_EAX);									// shl  eax,cl
					}
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x06:	/* SRLV */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					if (RSREG != 0)
					{
						emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RSREG));				// mov  ecx,[rsreg].lo
						emit_shr_r32_cl(DRCTOP, REG_EAX);									// shr  eax,cl
					}
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x07:	/* SRAV */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					if (RSREG != 0)
					{
						emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RSREG));				// mov  ecx,[rsreg].lo
						emit_sar_r32_cl(DRCTOP, REG_EAX);									// sar  eax,cl
					}
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x08:	/* JR */
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			emit_mov_m32_r32(DRCTOP, MABS(&jr_temp), REG_EAX);								// mov  jr_temp,eax
			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			emit_mov_r32_m32(DRCTOP, REG_EDI, MABS(&jr_temp));								// mov  edi,jr_temp
			return RECOMPILE_SUCCESSFUL_CP(1+cycles,0) | RECOMPILE_END_OF_STRING | RECOMPILE_ADD_DISPATCH;

		case 0x09:	/* JALR */
			if (RDREG != 0)
				emit_mov_m64_imm32(DRCTOP, REGADDR(RDREG), pc + 8);							// mov  [rdreg],pc + 8
			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			emit_mov_r32_m32(DRCTOP, REG_EDI, REGADDRLO(RSREG));							// mov  edi,[rsreg].lo
			return RECOMPILE_SUCCESSFUL_CP(1+cycles,0) | RECOMPILE_END_OF_STRING | RECOMPILE_ADD_DISPATCH;

		case 0x0a:	/* MOVZ - R5000 */
			if (mips3.core->flavor < MIPS3_TYPE_MIPS_IV)
			{
				emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);		// jmp  generate_invalidop_exception
				return RECOMPILE_SUCCESSFUL | RECOMPILE_MAY_CAUSE_EXCEPTION | RECOMPILE_END_OF_STRING;
			}
			if (RDREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[rtreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// or   eax,[rtreg].hi
				emit_jcc_short_link(DRCTOP, COND_NZ, &link1);								// jnz  skip
				emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RSREG));
																							// mov  [rdreg],[rsreg]
				resolve_link(DRCTOP, &link1);											// skip:
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x0b:	/* MOVN - R5000 */
			if (mips3.core->flavor < MIPS3_TYPE_MIPS_IV)
			{
				emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);		// jmp  generate_invalidop_exception
				return RECOMPILE_SUCCESSFUL | RECOMPILE_MAY_CAUSE_EXCEPTION | RECOMPILE_END_OF_STRING;
			}
			if (RDREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[rtreg].lo
				emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// or   eax,[rtreg].hi
				emit_jcc_short_link(DRCTOP, COND_Z, &link1);								// jz   skip
				emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RSREG));
																							// mov  [rdreg],[rsreg]
				resolve_link(DRCTOP, &link1);											// skip:
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x0c:	/* SYSCALL */
			emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_syscall_exception);			// jmp  generate_syscall_exception
			return RECOMPILE_SUCCESSFUL | RECOMPILE_MAY_CAUSE_EXCEPTION | RECOMPILE_END_OF_STRING;

		case 0x0d:	/* BREAK */
			emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_break_exception);				// jmp  generate_break_exception
			return RECOMPILE_SUCCESSFUL | RECOMPILE_MAY_CAUSE_EXCEPTION | RECOMPILE_END_OF_STRING;

		case 0x0f:	/* SYNC */
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x10:	/* MFHI */
			if (RDREG != 0)
				emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), HIADDR);							// mov  [rdreg],[hi]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x11:	/* MTHI */
			emit_mov_m64_m64(DRCTOP, HIADDR, REGADDR(RSREG));								// mov  [hi],[rsreg]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x12:	/* MFLO */
			if (RDREG != 0)
				emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), LOADDR);							// mov  [rdreg],[lo]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x13:	/* MTLO */
			emit_mov_m64_m64(DRCTOP, LOADDR, REGADDR(RSREG));								// mov  [lo],[rsreg]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x14:	/* DSLLV */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RTREG));				// mov  edx:eax,[rtreg]
					if (RSREG != 0)
					{
						emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RSREG));				// mov  ecx,[rsreg].lo
						emit_test_r32_imm(DRCTOP, REG_ECX, 0x20);							// test ecx,0x20
						emit_jcc_short_link(DRCTOP, COND_Z, &link1);						// jz   skip
						emit_mov_r32_r32(DRCTOP, REG_EDX, REG_EAX);							// mov  edx,eax
						emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);							// xor  eax,eax
						resolve_link(DRCTOP, &link1);									// skip:
						emit_shld_r32_r32_cl(DRCTOP, REG_EDX, REG_EAX);						// shld edx,eax,cl
						emit_shl_r32_cl(DRCTOP, REG_EAX);									// shl  eax,cl
					}
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x16:	/* DSRLV */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RTREG));				// mov  edx:eax,[rtreg]
					if (RSREG != 0)
					{
						emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RSREG));				// mov  ecx,[rsreg].lo
						emit_test_r32_imm(DRCTOP, REG_ECX, 0x20);							// test ecx,0x20
						emit_jcc_short_link(DRCTOP, COND_Z, &link1);						// jz   skip
						emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EDX);							// mov  eax,edx
						emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);							// xor  edx,edx
						resolve_link(DRCTOP, &link1);									// skip:
						emit_shrd_r32_r32_cl(DRCTOP, REG_EAX, REG_EDX);						// shrd eax,edx,cl
						emit_shr_r32_cl(DRCTOP, REG_EDX);									// shr  edx,cl
					}
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x17:	/* DSRAV */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RTREG));				// mov  edx:eax,[rtreg]
					if (RSREG != 0)
					{
						emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RSREG));				// mov  ecx,[rsreg].lo
						emit_test_r32_imm(DRCTOP, REG_ECX, 0x20);							// test ecx,0x20
						emit_jcc_short_link(DRCTOP, COND_Z, &link1);						// jz   skip
						emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EDX);							// mov  eax,edx
						emit_cdq(DRCTOP);													// cdq
						resolve_link(DRCTOP, &link1);									// skip:
						emit_shrd_r32_r32_cl(DRCTOP, REG_EAX, REG_EDX);						// shrd eax,edx,cl
						emit_sar_r32_cl(DRCTOP, REG_EDX);									// sar  edx,cl
					}
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x18:	/* MULT */
			emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RTREG));							// mov  ecx,[rtreg].lo
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			emit_imul_r32(DRCTOP, REG_ECX);													// imul ecx
			emit_push_r32(DRCTOP, REG_EDX);													// push edx
			emit_cdq(DRCTOP);																// cdq
			emit_mov_m64_r64(DRCTOP, LOADDR, REG_EDX, REG_EAX);								// mov  [lo],edx:eax
			emit_pop_r32(DRCTOP, REG_EAX);													// pop  eax
			emit_cdq(DRCTOP);																// cdq
			emit_mov_m64_r64(DRCTOP, HIADDR, REG_EDX, REG_EAX);								// mov  [hi],edx:eax
			return RECOMPILE_SUCCESSFUL_CP(4,4);

		case 0x19:	/* MULTU */
			emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RTREG));							// mov  ecx,[rtreg].lo
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			emit_mul_r32(DRCTOP, REG_ECX);													// mul  ecx
			emit_push_r32(DRCTOP, REG_EDX);													// push edx
			emit_cdq(DRCTOP);																// cdq
			emit_mov_m64_r64(DRCTOP, LOADDR, REG_EDX, REG_EAX);								// mov  [lo],edx:eax
			emit_pop_r32(DRCTOP, REG_EAX);													// pop  eax
			emit_cdq(DRCTOP);																// cdq
			emit_mov_m64_r64(DRCTOP, HIADDR, REG_EDX, REG_EAX);								// mov  [hi],edx:eax
			return RECOMPILE_SUCCESSFUL_CP(4,4);

		case 0x1a:	/* DIV */
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RTREG));						// mov  ecx,[rtreg].lo
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_cdq(DRCTOP);															// cdq
				emit_cmp_r32_imm(DRCTOP, REG_ECX, 0);										// cmp  ecx,0
				emit_jcc_short_link(DRCTOP, COND_E, &link1);								// je   skip
				emit_idiv_r32(DRCTOP, REG_ECX);												// idiv ecx
				emit_push_r32(DRCTOP, REG_EDX);												// push edx
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, LOADDR, REG_EDX, REG_EAX);							// mov  [lo],edx:eax
				emit_pop_r32(DRCTOP, REG_EAX);												// pop  eax
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, HIADDR, REG_EDX, REG_EAX);							// mov  [hi],edx:eax
				resolve_link(DRCTOP, &link1);											// skip:
			}
			return RECOMPILE_SUCCESSFUL_CP(36,4);

		case 0x1b:	/* DIVU */
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_ECX, REGADDRLO(RTREG));						// mov  ecx,[rtreg].lo
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);									// xor  edx,edx
				emit_cmp_r32_imm(DRCTOP, REG_ECX, 0);										// cmp  ecx,0
				emit_jcc_short_link(DRCTOP, COND_E, &link1);								// je   skip
				emit_div_r32(DRCTOP, REG_ECX);												// div  ecx
				emit_push_r32(DRCTOP, REG_EDX);												// push edx
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, LOADDR, REG_EDX, REG_EAX);							// mov  [lo],edx:eax
				emit_pop_r32(DRCTOP, REG_EAX);												// pop  eax
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, HIADDR, REG_EDX, REG_EAX);							// mov  [hi],edx:eax
				resolve_link(DRCTOP, &link1);											// skip:
			}
			return RECOMPILE_SUCCESSFUL_CP(36,4);

		case 0x1c:	/* DMULT */
			emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));						// mov  edx:eax,[rsreg]
			emit_cmp_r32_imm(DRCTOP, REG_EDX, 0);											// cmp  edx,0
			emit_jcc_short_link(DRCTOP, COND_GE, &link1);									// jge  skip1
			emit_mov_r32_r32(DRCTOP, REG_ECX, REG_EDX);										// mov  ecx,edx
			emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);										// xor  edx,edx
			emit_neg_r32(DRCTOP, REG_EAX);													// neg  eax
			emit_sbb_r32_r32(DRCTOP, REG_EDX, REG_ECX);										// sbb  edx,ecx
			resolve_link(DRCTOP, &link1);												// skip1:
			emit_mov_m64_r64(DRCTOP, MABS(&dmult_temp1), REG_EDX, REG_EAX);					// mov  [dmult_temp1],edx:eax

			emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RTREG));						// mov  edx:eax,[rtreg]
			emit_cmp_r32_imm(DRCTOP, REG_EDX, 0);											// cmp  edx,0
			emit_jcc_short_link(DRCTOP, COND_GE, &link2);									// jge  skip2
			emit_mov_r32_r32(DRCTOP, REG_ECX, REG_EDX);										// mov  ecx,edx
			emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);										// xor  edx,edx
			emit_neg_r32(DRCTOP, REG_EAX);													// neg  eax
			emit_sbb_r32_r32(DRCTOP, REG_EDX, REG_ECX);										// sbb  edx,ecx
			resolve_link(DRCTOP, &link2);												// skip2:
			emit_mov_m64_r64(DRCTOP, MABS(&dmult_temp2), REG_EDX, REG_EAX);					// mov  [dmult_temp2],edx:eax

			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(LO(&dmult_temp1)));						// mov  eax,[dmult_temp1].lo
			emit_mul_m32(DRCTOP, MABS(LO(&dmult_temp2)));									// mul  [dmult_temp2].lo
			emit_mov_r32_r32(DRCTOP, REG_ECX, REG_EDX);										// mov  ecx,edx
			emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);										// xor  ebx,ebx
			emit_mov_m32_r32(DRCTOP, LOADDRLO, REG_EAX);									// mov  [lo].lo,eax

			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(HI(&dmult_temp1)));						// mov  eax,[dmult_temp1].hi
			emit_mul_m32(DRCTOP, MABS(LO(&dmult_temp2)));									// mul  [dmult_temp2].lo
			emit_add_r32_r32(DRCTOP, REG_ECX, REG_EAX);										// add  ecx,eax
			emit_adc_r32_r32(DRCTOP, REG_EBX, REG_EDX);										// adc  ebx,edx

			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(LO(&dmult_temp1)));						// mov  eax,[dmult_temp1].lo
			emit_mul_m32(DRCTOP, MABS(HI(&dmult_temp2)));									// mul  [dmult_temp2].hi
			emit_add_r32_r32(DRCTOP, REG_ECX, REG_EAX);										// add  ecx,eax
			emit_adc_r32_r32(DRCTOP, REG_EBX, REG_EDX);										// adc  ebx,edx
			emit_mov_m32_r32(DRCTOP, LOADDRHI, REG_ECX);									// mov  [lo].hi,ecx

			emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(HI(&dmult_temp1)));						// mov  eax,[dmult_temp1].hi
			emit_mul_m32(DRCTOP, MABS(HI(&dmult_temp2)));									// mul  [dmult_temp2].hi
			emit_add_r32_r32(DRCTOP, REG_EBX, REG_EAX);										// add  ebx,eax
			emit_adc_r32_imm(DRCTOP, REG_EDX, 0);											// adc  edx,0
			emit_mov_m32_r32(DRCTOP, HIADDRLO, REG_EBX);									// mov  [hi].lo,ebx
			emit_mov_m32_r32(DRCTOP, HIADDRHI, REG_EDX);									// mov  [hi].hi,edx

			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// mov  eax,[rsreg].hi
			emit_xor_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// xor  eax,[rtreg].hi
			emit_jcc_short_link(DRCTOP, COND_NS, &link3);									// jns  noflip
			emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);										// xor  eax,eax
			emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);										// xor  ebx,ebx
			emit_xor_r32_r32(DRCTOP, REG_ECX, REG_ECX);										// xor  ecx,ecx
			emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);										// xor  edx,edx
			emit_sub_r32_m32(DRCTOP, REG_EAX, LOADDRLO);									// sub  eax,[lo].lo
			emit_sbb_r32_m32(DRCTOP, REG_EBX, LOADDRHI);									// sbb  ebx,[lo].hi
			emit_sbb_r32_m32(DRCTOP, REG_ECX, HIADDRLO);									// sbb  ecx,[hi].lo
			emit_sbb_r32_m32(DRCTOP, REG_EDX, HIADDRHI);									// sbb  edx,[hi].hi
			emit_mov_m64_r64(DRCTOP, LOADDR, REG_EBX, REG_EAX);								// mov  [lo],ebx:eax
			emit_mov_m64_r64(DRCTOP, HIADDR, REG_EDX, REG_ECX);								// mov  [hi],edx:ecx
			resolve_link(DRCTOP, &link3);												// noflip:
			return RECOMPILE_SUCCESSFUL_CP(8,4);

		case 0x1d:	/* DMULTU */
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			emit_mul_m32(DRCTOP, REGADDRLO(RTREG));											// mul  [rtreg].lo
			emit_mov_r32_r32(DRCTOP, REG_ECX, REG_EDX);										// mov  ecx,edx
			emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);										// xor  ebx,ebx
			emit_mov_m32_r32(DRCTOP, LOADDRLO, REG_EAX);									// mov  [lo].lo,eax

			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// mov  eax,[rsreg].hi
			emit_mul_m32(DRCTOP, REGADDRLO(RTREG));											// mul  [rtreg].lo
			emit_add_r32_r32(DRCTOP, REG_ECX, REG_EAX);										// add  ecx,eax
			emit_adc_r32_r32(DRCTOP, REG_EBX, REG_EDX);										// adc  ebx,edx

			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			emit_mul_m32(DRCTOP, REGADDRHI(RTREG));											// mul  [rtreg].hi
			emit_add_r32_r32(DRCTOP, REG_ECX, REG_EAX);										// add  ecx,eax
			emit_adc_r32_r32(DRCTOP, REG_EBX, REG_EDX);										// adc  ebx,edx
			emit_mov_m32_r32(DRCTOP, LOADDRHI, REG_ECX);									// mov  [lo].hi,ecx

			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// mov  eax,[rsreg].hi
			emit_mul_m32(DRCTOP, REGADDRHI(RTREG));											// mul  [rtreg].hi
			emit_add_r32_r32(DRCTOP, REG_EBX, REG_EAX);										// add  ebx,eax
			emit_adc_r32_imm(DRCTOP, REG_EDX, 0);											// adc  edx,0
			emit_mov_m32_r32(DRCTOP, HIADDRLO, REG_EBX);									// mov  [hi].lo,ebx
			emit_mov_m32_r32(DRCTOP, HIADDRHI, REG_EDX);									// mov  [hi].hi,edx
			return RECOMPILE_SUCCESSFUL_CP(8,4);

		case 0x1e:	/* DDIV */
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			emit_push_imm(DRCTOP, (FPTR)&mips3.core->r[RTREG]);								// push [rtreg]
			emit_push_imm(DRCTOP, (FPTR)&mips3.core->r[RSREG]);								// push [rsreg]
			emit_call(DRCTOP, (x86code *)ddiv);												// call ddiv
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			return RECOMPILE_SUCCESSFUL_CP(68,4);

		case 0x1f:	/* DDIVU */
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			emit_push_imm(DRCTOP, (FPTR)&mips3.core->r[RTREG]);								// push [rtreg]
			emit_push_imm(DRCTOP, (FPTR)&mips3.core->r[RSREG]);								// push [rsreg]
			emit_call(DRCTOP, (x86code *)ddivu);											// call ddivu
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			return RECOMPILE_SUCCESSFUL_CP(68,4);
	}

	emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);					// jmp  generate_invalidop_exception
	return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
}

static UINT32 recompile_special2(drc_core *drc, UINT32 pc, UINT32 op)
{
	emit_link link1, link2, link3;

	switch (op & 63)
	{
		case 0x20:	/* ADD */
			if (RSREG != 0 && RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_add_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// add  eax,[rtreg].lo
				emit_jcc(DRCTOP, COND_O, mips3.drcdata->generate_overflow_exception);		// jo   generate_overflow_exception
				if (RDREG != 0)
				{
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
			}
			else if (RDREG != 0)
			{
				if (RSREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));					// mov  eax,[rsreg].lo
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x21:	/* ADDU */
			if (RDREG != 0)
			{
				if (RSREG != 0 && RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));					// mov  eax,[rsreg].lo
					emit_add_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// add  eax,[rtreg].lo
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RSREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));					// mov  eax,[rsreg].lo
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x22:	/* SUB */
			if (RSREG != 0 && RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_sub_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// sub  eax,[rtreg].lo
				emit_jcc(DRCTOP, COND_O, mips3.drcdata->generate_overflow_exception);		// jo   generate_overflow_exception
				if (RDREG != 0)
				{
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
			}
			else if (RDREG != 0)
			{
				if (RSREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));					// mov  eax,[rsreg].lo
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					emit_neg_r32(DRCTOP, REG_EAX);											// neg  eax
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x23:	/* SUBU */
			if (RDREG != 0)
			{
				if (RSREG != 0 && RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));					// mov  eax,[rsreg].lo
					emit_sub_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// sub  eax,[rtreg].lo
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RSREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));					// mov  eax,[rsreg].lo
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					emit_neg_r32(DRCTOP, REG_EAX);											// neg  eax
					emit_cdq(DRCTOP);														// cdq
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x24:	/* AND */
			if (RDREG != 0)
			{
				if (RSREG != 0 && RTREG != 0)
				{
					if (USE_SSE)
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, REGADDR(RSREG));				// movsd xmm0,[rsreg]
						emit_movsd_r128_m64(DRCTOP, REG_XMM1, REGADDR(RTREG));				// movsd xmm1,[rtreg]
						emit_andpd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);					// andpd xmm0,xmm1
						emit_movsd_m64_r128(DRCTOP, REGADDR(RDREG), REG_XMM0);				// mov  [rdreg],xmm0
					}
					else
					{
						emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));
																							// mov  edx:eax,[rsreg]
						emit_and_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));				// and  eax,[rtreg].lo
						emit_and_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));				// and  edx,[rtreg].hi
						emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);			// mov  [rdreg],edx:eax
					}
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x25:	/* OR */
			if (RDREG != 0)
			{
				if (RSREG != 0 && RTREG != 0)
				{
					if (USE_SSE)
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, REGADDR(RSREG));				// movsd xmm0,[rsreg]
						emit_movsd_r128_m64(DRCTOP, REG_XMM1, REGADDR(RTREG));				// movsd xmm1,[rtreg]
						emit_orpd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);					// orpd  xmm0,xmm1
						emit_movsd_m64_r128(DRCTOP, REGADDR(RDREG), REG_XMM0);				// mov  [rdreg],xmm0
					}
					else
					{
						emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));
																							// mov  edx:eax,[rsreg]
						emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// or   eax,[rtreg].lo
						emit_or_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));					// or   edx,[rtreg].hi
						emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);			// mov  [rdreg],edx:eax
					}
				}
				else if (RSREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RSREG));				// mov  [rdreg],[rsreg]
				else if (RTREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RTREG));				// mov  [rdreg],[rtreg]
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x26:	/* XOR */
			if (RDREG != 0)
			{
				if (RSREG != 0 && RTREG != 0)
				{
					if (USE_SSE)
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, REGADDR(RSREG));				// movsd xmm0,[rsreg]
						emit_movsd_r128_m64(DRCTOP, REG_XMM1, REGADDR(RTREG));				// movsd xmm1,[rtreg]
						emit_xorpd_r128_r128(DRCTOP, REG_XMM0, REG_XMM1);					// xorpd xmm0,xmm1
						emit_movsd_m64_r128(DRCTOP, REGADDR(RDREG), REG_XMM0);				// mov  [rdreg],xmm0
					}
					else
					{
						emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));
																							// mov  edx:eax,[rsreg]
						emit_xor_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));				// xor  eax,[rtreg].lo
						emit_xor_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));				// xor  edx,[rtreg].hi
						emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);			// mov  [rdreg],edx:eax
					}
				}
				else if (RSREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RSREG));				// mov  [rdreg],[rsreg]
				else if (RTREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RTREG));				// mov  [rdreg],[rtreg]
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x27:	/* NOR */
			if (RDREG != 0)
			{
				if (RSREG != 0 && RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));				// mov  edx:eax,[rsreg]
					emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// or   eax,[rtreg].lo
					emit_or_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));						// or   edx,[rtreg].hi
					emit_not_r32(DRCTOP, REG_EDX);											// not  edx
					emit_not_r32(DRCTOP, REG_EAX);											// not  eax
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RSREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));				// mov  edx:eax,[rsreg]
					emit_not_r32(DRCTOP, REG_EDX);											// not  edx
					emit_not_r32(DRCTOP, REG_EAX);											// not  eax
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RTREG));				// mov  edx:eax,[rtreg]
					emit_not_r32(DRCTOP, REG_EDX);											// not  edx
					emit_not_r32(DRCTOP, REG_EAX);											// not  eax
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
				{
					emit_mov_m32_imm(DRCTOP, REGADDRLO(RDREG), ~0);							// mov  [rtreg].lo,~0
					emit_mov_m32_imm(DRCTOP, REGADDRHI(RDREG), ~0);							// mov  [rtreg].hi,~0
				}
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2a:	/* SLT */
			if (RDREG != 0)
			{
				if (RSREG != 0)
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));	// mov  edx:eax,[rsreg]
				else
				{
					emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);								// xor  edx,edx
					emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);								// xor  eax,eax
				}
				if (RTREG != 0)
				{
					emit_sub_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// sub  eax,[rtreg].lo
					emit_sbb_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));					// sbb  edx,[rtreg].lo
				}
				emit_shr_r32_imm(DRCTOP, REG_EDX, 31);										// shr  edx,31
				emit_mov_m32_r32(DRCTOP, REGADDRLO(RDREG), REG_EDX);						// mov  [rdreg].lo,edx
				emit_mov_m32_imm(DRCTOP, REGADDRHI(RDREG), 0);								// mov  [rdreg].hi,0
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2b:	/* SLTU */
			if (RDREG != 0)
			{
				emit_xor_r32_r32(DRCTOP, REG_ECX, REG_ECX);									// xor  ecx,ecx
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));						// mov  eax,[rsreg].hi
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));						// cmp  eax,[rtreg].hi
				emit_jcc_short_link(DRCTOP, COND_B, &link1);								// jb   setit
				emit_jcc_short_link(DRCTOP, COND_A, &link2);								// ja   skipit
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));						// mov  eax,[rsreg].lo
				emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// cmp  eax,[rtreg].lo
				emit_jcc_short_link(DRCTOP, COND_AE, &link3);								// jae  skipit
				resolve_link(DRCTOP, &link1);											// setit:
				emit_add_r32_imm(DRCTOP, REG_ECX, 1);										// add  ecx,1
				resolve_link(DRCTOP, &link2);											// skipit:
				resolve_link(DRCTOP, &link3);											// skipit:
				emit_mov_m32_r32(DRCTOP, REGADDRLO(RDREG), REG_ECX);						// mov  [rdreg].lo,ecx
				emit_mov_m32_imm(DRCTOP, REGADDRHI(RDREG), 0);								// mov  [rdreg].hi,0
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2c:	/* DADD */
			if (RSREG != 0 && RTREG != 0)
			{
				emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));					// mov  edx:eax,[rsreg]
				emit_add_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// add  eax,[rtreg].lo
				emit_adc_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));						// adc  edx,[rtreg].hi
				emit_jcc(DRCTOP, COND_O, mips3.drcdata->generate_overflow_exception);		// jo   generate_overflow_exception
				if (RDREG != 0)
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
			}
			else if (RDREG != 0)
			{
				if (RSREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RSREG));				// mov  [rdreg],[rsreg]
				else if (RTREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RTREG));				// mov  [rdreg],[rtreg]
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x2d:	/* DADDU */
			if (RDREG != 0)
			{
				if (RSREG != 0 && RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));				// mov  edx:eax,[rsreg]
					emit_add_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// add  eax,[rtreg].lo
					emit_adc_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));					// adc  edx,[rtreg].hi
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RSREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RSREG));
																							// mov  [rdreg],[rsreg]
				else if (RTREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RTREG));
																							// mov  [rdreg],[rtreg]
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x2e:	/* DSUB */
			if (RSREG != 0 && RTREG != 0)
			{
				emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));					// mov  edx:eax,[rsreg]
				emit_sub_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// sub  eax,[rtreg].lo
				emit_sbb_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));						// sbb  edx,[rtreg].hi
				emit_jcc(DRCTOP, COND_O, mips3.drcdata->generate_overflow_exception);		// jo   generate_overflow_exception
				if (RDREG != 0)
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
			}
			else if (RDREG != 0)
			{
				if (RSREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RSREG));				// mov  [rdreg],[rsreg]
				else if (RTREG != 0)
				{
					emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);								// xor  eax,eax
					emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);								// xor  edx,edx
					emit_sub_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// sub  eax,[rtreg].lo
					emit_sbb_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));					// sbb  edx,[rtreg].hi
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x2f:	/* DSUBU */
			if (RDREG != 0)
			{
				if (RSREG != 0 && RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));				// mov  edx:eax,[rsreg]
					emit_sub_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// sub  eax,[rtreg].lo
					emit_sbb_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));					// sbb  edx,[rtreg].hi
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else if (RSREG != 0)
					emit_mov_m64_m64(DRCTOP, REGADDR(RDREG), REGADDR(RSREG));				// mov  [rdreg],[rsreg]
				else if (RTREG != 0)
				{
					emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);								// xor  eax,eax
					emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);								// xor  edx,edx
					emit_sub_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// sub  eax,[rtreg].lo
					emit_sbb_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));					// sbb  edx,[rtreg].hi
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x30:	/* TGE */
			if (RSREG != 0)
				emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));					// mov  edx:eax,[rsreg]
			else
			{
				emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);									// xor  edx,edx
				emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);									// xor  eax,eax
			}
			if (RTREG != 0)
			{
				emit_sub_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// sub  eax,[rtreg].lo
				emit_sbb_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));						// sbb  edx,[rtreg].hi
			}
			else
				emit_cmp_r32_imm(DRCTOP, REG_EDX, 0);										// cmp  edx,0
			emit_jcc(DRCTOP, COND_GE, mips3.drcdata->generate_trap_exception);				// jge  generate_trap_exception
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x31:	/* TGEU */
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// mov  eax,[rsreg].hi
			emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// cmp  eax,[rtreg].hi
			emit_jcc(DRCTOP, COND_A, mips3.drcdata->generate_trap_exception);				// ja   generate_trap_exception
			emit_jcc_short_link(DRCTOP, COND_B, &link1);									// jb   skipit
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));							// cmp  eax,[rtreg].lo
			emit_jcc(DRCTOP, COND_AE, mips3.drcdata->generate_trap_exception);				// jae  generate_trap_exception
			resolve_link(DRCTOP, &link1);												// skipit:
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x32:	/* TLT */
			if (RSREG != 0)
				emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));					// mov  edx:eax,[rsreg]
			else
			{
				emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);									// xor  edx,edx
				emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);									// xor  eax,eax
			}
			if (RTREG != 0)
			{
				emit_sub_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// sub  eax,[rtreg].lo
				emit_sbb_r32_m32(DRCTOP, REG_EDX, REGADDRHI(RTREG));						// sbb  edx,[rtreg].hi
			}
			else
				emit_cmp_r32_imm(DRCTOP, REG_EDX, 0);										// cmp  edx,0
			emit_jcc(DRCTOP, COND_L, mips3.drcdata->generate_trap_exception);				// jl   generate_trap_exception
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x33:	/* TLTU */
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// mov  eax,[rsreg].hi
			emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// cmp  eax,[rtreg].hi
			emit_jcc(DRCTOP, COND_B, mips3.drcdata->generate_trap_exception);				// jb   generate_trap_exception
			emit_jcc_short_link(DRCTOP, COND_A, &link1);									// ja   skipit
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));							// cmp  eax,[rtreg].lo
			emit_jcc(DRCTOP, COND_B, mips3.drcdata->generate_trap_exception);				// jb   generate_trap_exception
			resolve_link(DRCTOP, &link1);												// skipit:
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x34:	/* TEQ */
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// mov  eax,[rsreg].hi
			emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// cmp  eax,[rtreg].hi
			emit_jcc_short_link(DRCTOP, COND_NE, &link1);									// jne  skipit
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));							// cmp  eax,[rtreg].lo
			emit_jcc(DRCTOP, COND_E, mips3.drcdata->generate_trap_exception);				// je   generate_trap_exception
			resolve_link(DRCTOP, &link1);												// skipit:
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x36:	/* TNE */
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RSREG));							// mov  eax,[rsreg].hi
			emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));							// cmp  eax,[rtreg].hi
			emit_jcc_short_link(DRCTOP, COND_E, &link1);									// je   skipit
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg].lo
			emit_cmp_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));							// cmp  eax,[rtreg].lo
			emit_jcc(DRCTOP, COND_NE, mips3.drcdata->generate_trap_exception);				// jne  generate_trap_exception
			resolve_link(DRCTOP, &link1);												// skipit:
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x38:	/* DSLL */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RTREG));				// mov  edx:eax,[rtreg]
					if (SHIFT != 0)
					{
						emit_shld_r32_r32_imm(DRCTOP, REG_EDX, REG_EAX, SHIFT);				// shld edx,eax,SHIFT
						emit_shl_r32_imm(DRCTOP, REG_EAX, SHIFT);							// shl  eax,SHIFT
					}
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x3a:	/* DSRL */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RTREG));				// mov  edx:eax,[rtreg]
					if (SHIFT != 0)
					{
						emit_shrd_r32_r32_imm(DRCTOP, REG_EAX, REG_EDX, SHIFT);				// shrd eax,edx,SHIFT
						emit_shr_r32_imm(DRCTOP, REG_EDX, SHIFT);							// shr  edx,SHIFT
					}
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x3b:	/* DSRA */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RTREG));				// mov  edx:eax,[rtreg]
					if (SHIFT != 0)
					{
						emit_shrd_r32_r32_imm(DRCTOP, REG_EAX, REG_EDX, SHIFT);				// shrd eax,edx,SHIFT
						emit_sar_r32_imm(DRCTOP, REG_EDX, SHIFT);							// sar  edx,SHIFT
					}
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x3c:	/* DSLL32 */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					if (SHIFT != 0)
						emit_shl_r32_imm(DRCTOP, REG_EAX, SHIFT);							// shl  eax,SHIFT
					emit_mov_m32_imm(DRCTOP, REGADDRLO(RDREG), 0);							// mov  [rdreg].lo,0
					emit_mov_m32_r32(DRCTOP, REGADDRHI(RDREG), REG_EAX);					// mov  [rdreg].hi,eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x3e:	/* DSRL32 */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));					// mov  eax,[rtreg].hi
					if (SHIFT != 0)
						emit_shr_r32_imm(DRCTOP, REG_EAX, SHIFT);							// shr  eax,SHIFT
					emit_mov_m32_imm(DRCTOP, REGADDRHI(RDREG), 0);							// mov  [rdreg].hi,0
					emit_mov_m32_r32(DRCTOP, REGADDRLO(RDREG), REG_EAX);					// mov  [rdreg].lo,eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x3f:	/* DSRA32 */
			if (RDREG != 0)
			{
				if (RTREG != 0)
				{
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));					// mov  eax,[rtreg].hi
					emit_cdq(DRCTOP);														// cdq
					if (SHIFT != 0)
						emit_sar_r32_imm(DRCTOP, REG_EAX, SHIFT);							// sar  eax,SHIFT
					emit_mov_m64_r64(DRCTOP, REGADDR(RDREG), REG_EDX, REG_EAX);				// mov  [rdreg],edx:eax
				}
				else
					emit_zero_m64(DRCTOP, REGADDR(RDREG));
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);
	}

	emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);					// jmp  generate_invalidop_exception
	return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
}


static UINT32 recompile_special(drc_core *drc, UINT32 pc, UINT32 op)
{
	if (!(op & 32))
		return recompile_special1(drc, pc, op);
	else
		return recompile_special2(drc, pc, op);
}


/*------------------------------------------------------------------
    recompile_regimm
------------------------------------------------------------------*/

static UINT32 recompile_regimm(drc_core *drc, UINT32 pc, UINT32 op)
{
	emit_link link1;
	int cycles;

	switch (RTREG)
	{
		case 0x00:	/* BLTZ */
			if (RSREG == 0)
				return RECOMPILE_SUCCESSFUL_CP(1,4);
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_GE, &link1);								// jge  skip
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x01:	/* BGEZ */
			if (RSREG == 0)
			{
				cycles = recompile_delay_slot(drc, pc + 4);									// <next instruction>
				append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);			// <branch or dispatch>
				return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
			}
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_L, &link1);									// jl   skip
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x02:	/* BLTZL */
			if (RSREG == 0)
				return RECOMPILE_SUCCESSFUL_CP(1,4);
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_GE, &link1);								// jge  skip
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,8);

		case 0x03:	/* BGEZL */
			if (RSREG == 0)
			{
				cycles = recompile_delay_slot(drc, pc + 4);									// <next instruction>
				append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);			// <branch or dispatch>
				return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
			}
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_L, &link1);									// jl   skip
			}

			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,8);

		case 0x08:	/* TGEI */
			if (RSREG != 0)
			{
				emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));					// mov  edx:eax,[rsreg]
				emit_sub_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// sub  eax,[rtreg].lo
				emit_sbb_r32_imm(DRCTOP, REG_EDX, ((INT32)SIMMVAL >> 31));					// sbb  edx,[rtreg].lo
				emit_jcc(DRCTOP, COND_GE, mips3.drcdata->generate_trap_exception);			// jge  generate_trap_exception
			}
			else
			{
				if (0 >= SIMMVAL)
					emit_jmp(DRCTOP, mips3.drcdata->generate_trap_exception);				// jmp  generate_trap_exception
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x09:	/* TGEIU */
			if (RSREG != 0)
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), ((INT32)SIMMVAL >> 31));
																							// cmp  [rsreg].hi,upper
				emit_jcc(DRCTOP, COND_A, mips3.drcdata->generate_trap_exception);			// ja   generate_trap_exception
				emit_jcc_short_link(DRCTOP, COND_B, &link1);								// jb   skip
				emit_cmp_m32_imm(DRCTOP, REGADDRLO(RSREG), SIMMVAL);						// cmp  [rsreg].lo,lower
				emit_jcc(DRCTOP, COND_AE, mips3.drcdata->generate_trap_exception);			// jae  generate_trap_exception
				resolve_link(DRCTOP, &link1);											// skip:
			}
			else
			{
				if (0 >= SIMMVAL)
					emit_jmp(DRCTOP, mips3.drcdata->generate_trap_exception);				// jmp  generate_trap_exception
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x0a:	/* TLTI */
			if (RSREG != 0)
			{
				emit_mov_r64_m64(DRCTOP, REG_EDX, REG_EAX, REGADDR(RSREG));					// mov  edx:eax,[rsreg]
				emit_sub_r32_imm(DRCTOP, REG_EAX, SIMMVAL);									// sub  eax,[rtreg].lo
				emit_sbb_r32_imm(DRCTOP, REG_EDX, ((INT32)SIMMVAL >> 31));					// sbb  edx,[rtreg].lo
				emit_jcc(DRCTOP, COND_L, mips3.drcdata->generate_trap_exception);			// jl   generate_trap_exception
			}
			else
			{
				if (0 < SIMMVAL)
					emit_jmp(DRCTOP, mips3.drcdata->generate_trap_exception);				// jmp  generate_trap_exception
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x0b:	/* TLTIU */
			if (RSREG != 0)
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), ((INT32)SIMMVAL >> 31));
																							// cmp  [rsreg].hi,upper
				emit_jcc(DRCTOP, COND_B, mips3.drcdata->generate_trap_exception);			// jb   generate_trap_exception
				emit_jcc_short_link(DRCTOP, COND_A, &link1);								// ja   skip
				emit_cmp_m32_imm(DRCTOP, REGADDRLO(RSREG), SIMMVAL);						// cmp  [rsreg].lo,lower
				emit_jcc(DRCTOP, COND_B, mips3.drcdata->generate_trap_exception);			// jb   generate_trap_exception
				resolve_link(DRCTOP, &link1);											// skip:
			}
			else
			{
				emit_mov_m32_imm(DRCTOP, REGADDRLO(RTREG), (0 < SIMMVAL));					// mov  [rtreg].lo,const
				emit_mov_m32_imm(DRCTOP, REGADDRHI(RTREG), 0);								// mov  [rtreg].hi,sign-extend(const)
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x0c:	/* TEQI */
			if (RSREG != 0)
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), ((INT32)SIMMVAL >> 31));
																							// cmp  [rsreg].hi,upper
				emit_jcc_short_link(DRCTOP, COND_NE, &link1);								// jne  skip
				emit_cmp_m32_imm(DRCTOP, REGADDRLO(RSREG), SIMMVAL);						// cmp  [rsreg].lo,lower
				emit_jcc(DRCTOP, COND_E, mips3.drcdata->generate_trap_exception);			// je   generate_trap_exception
				resolve_link(DRCTOP, &link1);											// skip:
			}
			else
			{
				if (0 == SIMMVAL)
					emit_jmp(DRCTOP, mips3.drcdata->generate_trap_exception);				// jmp  generate_trap_exception
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x0e:	/* TNEI */
			if (RSREG != 0)
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), ((INT32)SIMMVAL >> 31));
																							// cmp  [rsreg].hi,upper
				emit_jcc_short_link(DRCTOP, COND_E, &link1);								// je   skip
				emit_cmp_m32_imm(DRCTOP, REGADDRLO(RSREG), SIMMVAL);						// cmp  [rsreg].lo,lower
				emit_jcc(DRCTOP, COND_NE, mips3.drcdata->generate_trap_exception);			// jne  generate_trap_exception
				resolve_link(DRCTOP, &link1);											// skip:
			}
			else
			{
				if (0 != SIMMVAL)
					emit_jmp(DRCTOP, mips3.drcdata->generate_trap_exception);				// jmp  generate_trap_exception
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_MAY_CAUSE_EXCEPTION;

		case 0x10:	/* BLTZAL */
			if (RSREG == 0)
				return RECOMPILE_SUCCESSFUL_CP(1,4);
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_GE, &link1);								// jge  skip
			}
			emit_mov_m64_imm32(DRCTOP, MABS(&mips3.core->r[31]), pc + 8);					// mov  [31],pc + 8
			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x11:	/* BGEZAL */
			if (RSREG == 0)
			{
				emit_mov_m64_imm32(DRCTOP, MABS(&mips3.core->r[31]), pc + 8);				// mov  [31],pc + 8
				cycles = recompile_delay_slot(drc, pc + 4);									// <next instruction>
				append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);			// <branch or dispatch>
				return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
			}
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_L, &link1);									// jl   skip
			}

			emit_mov_m64_imm32(DRCTOP, MABS(&mips3.core->r[31]), pc + 8);					// mov  [31],pc + 8
			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x12:	/* BLTZALL */
			if (RSREG == 0)
				return RECOMPILE_SUCCESSFUL_CP(1,4);
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_GE, &link1);								// jge  skip
			}

			emit_mov_m64_imm32(DRCTOP, MABS(&mips3.core->r[31]), pc + 8);					// mov  [31],pc + 8
			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,8);

		case 0x13:	/* BGEZALL */
			if (RSREG == 0)
			{
				emit_mov_m64_imm32(DRCTOP, MABS(&mips3.core->r[31]), pc + 8);				// mov  [31],pc + 8
				cycles = recompile_delay_slot(drc, pc + 4);									// <next instruction>
				append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);			// <branch or dispatch>
				return RECOMPILE_SUCCESSFUL_CP(0,0) | RECOMPILE_END_OF_STRING;
			}
			else
			{
				emit_cmp_m32_imm(DRCTOP, REGADDRHI(RSREG), 0);								// cmp  [rsreg].hi,0
				emit_jcc_near_link(DRCTOP, COND_L, &link1);									// jl   skip
			}

			emit_mov_m64_imm32(DRCTOP, MABS(&mips3.core->r[31]), pc + 8);					// mov  [31],pc + 8
			cycles = recompile_delay_slot(drc, pc + 4);										// <next instruction>
			append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);				// <branch or dispatch>
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,8);
	}

	emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);					// jmp  generate_invalidop_exception
	return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
}



/***************************************************************************
    COP0 RECOMPILATION
***************************************************************************/

/*------------------------------------------------------------------
    recompile_set_cop0_reg
------------------------------------------------------------------*/

static UINT32 recompile_set_cop0_reg(drc_core *drc, UINT8 reg)
{
	emit_link link1;

	switch (reg)
	{
		case COP0_Cause:
			emit_mov_r32_m32(DRCTOP, REG_EBX, CPR0ADDR(COP0_Cause));						// mov  ebx,[COP0_Cause]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~0xfc00);										// and  eax,~0xfc00
			emit_and_r32_imm(DRCTOP, REG_EBX, 0xfc00);										// and  ebx,0xfc00
			emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);										// or   eax,ebx
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Cause), REG_EAX);						// mov  [COP0_Cause],eax
			emit_and_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Status));						// and  eax,[COP0_Status]
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_CHECK_SW_INTERRUPTS;

		case COP0_Status:
			emit_mov_r32_m32(DRCTOP, REG_EBX, CPR0ADDR(COP0_Status));						// mov  ebx,[COP0_Status]
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Status), REG_EAX);						// mov  [COP0_Status],eax
			emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EAX);										// xor  ebx,eax
/*
#ifdef MAME_DEBUG
            _test_r32_imm(REG_EAX, 0xe0);                                                   // test eax,0xe0
            _jcc_short_link(COND_Z, &link1);                                                // jz   skip
            _sub_r32_imm(REG_ESP, 4);                                                       // sub  esp,4
            _push_r32(REG_EAX);                                                             // push eax
            _push_imm("System set 64-bit addressing mode, SR=%08X");                        // push <string>
            _call(mame_printf_debug);                                                       // call mame_printf_debug
            _call(mame_debug_break);                                                        // call mame_debug_break
            _add_r32_imm(REG_ESP, 12);                                                      // add esp,12
            _resolve_link(&link1);                                                          // skip:
#endif
*/
			emit_test_r32_imm(DRCTOP, REG_EBX, 0x8000);										// test ebx,0x8000
			emit_jcc_short_link(DRCTOP, COND_Z, &link1);									// jz   skip
			append_update_cycle_counting(drc);												// update cycle counting
			resolve_link(DRCTOP, &link1);												// skip:
			return RECOMPILE_SUCCESSFUL_CP(1,4) | RECOMPILE_CHECK_INTERRUPTS | RECOMPILE_END_OF_STRING | RECOMPILE_ADD_DISPATCH;

		case COP0_Count:
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Count), REG_EAX);						// mov  [COP0_Count],eax
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, (x86code *)activecpu_gettotalcycles64);						// call activecpu_gettotalcycles64
			emit_pop_r32(DRCTOP, REG_EBX);													// pop  ebx
			emit_add_r32_imm(DRCTOP, REG_ESP, 8);											// add  esp,8
			emit_sub_r32_r32(DRCTOP, REG_EAX, REG_EBX);										// sub  eax,ebx
			emit_sbb_r32_imm(DRCTOP, REG_EDX, 0);											// sbb  edx,0
			emit_sub_r32_r32(DRCTOP, REG_EAX, REG_EBX);										// sub  eax,ebx
			emit_sbb_r32_imm(DRCTOP, REG_EDX, 0);											// sbb  edx,0
			emit_mov_m64_r64(DRCTOP, MABS(&mips3.core->count_zero_time), REG_EDX, REG_EAX);	// mov  [mips3.core->count_zero_time],edx:eax
			append_update_cycle_counting(drc);												// update cycle counting
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case COP0_Compare:
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Compare), REG_EAX);						// mov  [COP0_Compare],eax
			emit_and_m32_imm(DRCTOP, CPR0ADDR(COP0_Cause), ~0x8000);						// and  [COP0_Cause],~0x8000
			append_update_cycle_counting(drc);												// update cycle counting
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case COP0_PRId:
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case COP0_Config:
			emit_mov_r32_m32(DRCTOP, REG_EBX, CPR0ADDR(COP0_Config));						// mov  ebx,[COP0_Cause]
			emit_and_r32_imm(DRCTOP, REG_EAX, 0x0007);										// and  eax,0x0007
			emit_and_r32_imm(DRCTOP, REG_EBX, ~0x0007);										// and  ebx,~0x0007
			emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);										// or   eax,ebx
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Config), REG_EAX);						// mov  [COP0_Cause],eax
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		default:
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(reg), REG_EAX);								// mov  [reg],eax
			return RECOMPILE_SUCCESSFUL_CP(1,4);
	}
	return RECOMPILE_UNIMPLEMENTED;
}


/*------------------------------------------------------------------
    recompile_get_cop0_reg
------------------------------------------------------------------*/

static UINT32 recompile_get_cop0_reg(drc_core *drc, UINT8 reg)
{
	emit_link link1;

	switch (reg)
	{
		case COP0_Count:
			emit_sub_r32_imm(DRCTOP, REG_EBP, 250);											// sub  ebp,24
			emit_jcc_short_link(DRCTOP, COND_NS, &link1);									// jns  notneg
			emit_xor_r32_r32(DRCTOP, REG_EBP, REG_EBP);										// xor  ebp,ebp
			resolve_link(DRCTOP, &link1);												// notneg:
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_sub_r32_imm(DRCTOP, REG_ESP, 12);											// sub  esp,12
			emit_call(DRCTOP, (x86code *)activecpu_gettotalcycles64);						// call activecpu_gettotalcycles64
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_sub_r32_m32(DRCTOP, REG_EAX, MABS(LO(&mips3.core->count_zero_time)));		// sub  eax,[mips3.core->count_zero_time+0]
			emit_sbb_r32_m32(DRCTOP, REG_EDX, MABS(HI(&mips3.core->count_zero_time)));		// sbb  edx,[mips3.core->count_zero_time+4]
			emit_shrd_r32_r32_imm(DRCTOP, REG_EAX, REG_EDX, 1);								// shrd eax,edx,1
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case COP0_Cause:
			emit_sub_r32_imm(DRCTOP, REG_EBP, 250);											// sub  ebp,24
			emit_jcc_short_link(DRCTOP, COND_NS, &link1);									// jns  notneg
			emit_xor_r32_r32(DRCTOP, REG_EBP, REG_EBP);										// xor  ebp,ebp
			resolve_link(DRCTOP, &link1);												// notneg:
			emit_mov_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(reg));								// mov  eax,[reg]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		default:
			emit_mov_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(reg));								// mov  eax,[reg]
			return RECOMPILE_SUCCESSFUL_CP(1,4);
	}
	return RECOMPILE_UNIMPLEMENTED;
}


/*------------------------------------------------------------------
    recompile_cop0
------------------------------------------------------------------*/

static UINT32 recompile_cop0(drc_core *drc, UINT32 pc, UINT32 op)
{
	emit_link checklink;
	UINT32 result;

	if (mips3.drcoptions & MIPS3DRC_STRICT_COP0)
	{
		emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_KSU_MASK);						// test [COP0_Status],SR_KSU_MASK
		emit_jcc_short_link(DRCTOP, COND_Z, &checklink);									// jz   okay
		emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_COP0);							// test [COP0_Status],SR_COP0
		emit_jcc(DRCTOP, COND_Z, mips3.drcdata->generate_cop_exception);					// jz   generate_cop_exception
		resolve_link(DRCTOP, &checklink);												// okay:
	}

	switch (RSREG)
	{
		case 0x00:	/* MFCz */
			result = RECOMPILE_SUCCESSFUL_CP(1,4);
			if (RTREG != 0)
			{
				result = recompile_get_cop0_reg(drc, RDREG);								// read cop0 reg
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			return result;

		case 0x01:	/* DMFCz */
			result = RECOMPILE_SUCCESSFUL_CP(1,4);
			if (RTREG != 0)
			{
				result = recompile_get_cop0_reg(drc, RDREG);								// read cop0 reg
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			return result;

		case 0x02:	/* CFCz */
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, CCR0ADDR(RDREG));							// mov  eax,[mips3.core->ccr[0][rdreg]]
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [rtreg],edx:eax
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x04:	/* MTCz */
			if (RTREG != 0)
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[mips3.core->r[RTREG]]
			else
				emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);									// xor  eax,eax
			result = recompile_set_cop0_reg(drc, RDREG);									// write cop0 reg
			return result;

		case 0x05:	/* DMTCz */
			if (RTREG != 0)
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[mips3.core->r[RTREG]]
			else
				emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);									// xor  eax,eax
			result = recompile_set_cop0_reg(drc, RDREG);									// write cop0 reg
			return result;

		case 0x06:	/* CTCz */
			if (RTREG != 0)
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[mips3.core->r[RTREG]]
			else
				emit_xor_r32_r32(DRCTOP, REG_EAX, REG_EAX);									// xor  eax,eax
			emit_mov_m32_r32(DRCTOP, CCR0ADDR(RDREG), REG_EAX);								// mov  [mips3.core->ccr[0][RDREG]],eax
			return RECOMPILE_SUCCESSFUL_CP(1,4);

//      case 0x08:  /* BC */
//          switch (RTREG)
//          {
//              case 0x00:  /* BCzF */  if (!mips3.core->cf[0][0]) ADDPC(SIMMVAL);                break;
//              case 0x01:  /* BCzF */  if (mips3.core->cf[0][0]) ADDPC(SIMMVAL);                 break;
//              case 0x02:  /* BCzFL */ invalid_instruction(op);                            break;
//              case 0x03:  /* BCzTL */ invalid_instruction(op);                            break;
//              default:    invalid_instruction(op);                                        break;
//          }
//          break;

		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:	/* COP */
			switch (op & 0x01ffffff)
			{
				case 0x01:	/* TLBR */
					emit_sub_r32_imm(DRCTOP, REG_ESP, 8);									// sub  esp,8
					emit_push_imm(DRCTOP, (FPTR)mips3.core);								// push mips3.core
					drc_append_save_call_restore(drc, (x86code *)mips3com_tlbr, 12);		// call tlbr
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x02:	/* TLBWI */
					emit_sub_r32_imm(DRCTOP, REG_ESP, 8);									// sub  esp,8
					emit_push_imm(DRCTOP, (FPTR)mips3.core);								// push mips3.core
					drc_append_save_call_restore(drc, (x86code *)mips3com_tlbwi, 12);		// call tlbwi
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x06:	/* TLBWR */
					emit_sub_r32_imm(DRCTOP, REG_ESP, 8);									// sub  esp,8
					emit_push_imm(DRCTOP, (FPTR)mips3.core);								// push mips3.core
					drc_append_save_call_restore(drc, (x86code *)mips3com_tlbwr, 12);		// call tlbwr
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x08:	/* TLBP */
					emit_sub_r32_imm(DRCTOP, REG_ESP, 8);									// sub  esp,8
					emit_push_imm(DRCTOP, (FPTR)mips3.core);								// push mips3.core
					drc_append_save_call_restore(drc, (x86code *)mips3com_tlbp, 12);		// call tlbp
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x10:	/* RFE */
					emit_jmp(DRCTOP, mips3.drcdata->generate_invalidop_exception);			// jmp  generate_invalidop_exception
					return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;

				case 0x18:	/* ERET */
					emit_mov_r32_m32(DRCTOP, REG_EDI, CPR0ADDR(COP0_EPC));					// mov  edi,[COP0_EPC]
					emit_and_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), ~SR_EXL);				// and  [COP0_Status],~SR_EXL
					return RECOMPILE_SUCCESSFUL_CP(1,0) | RECOMPILE_CHECK_INTERRUPTS | RECOMPILE_END_OF_STRING | RECOMPILE_ADD_DISPATCH;

				default:
					emit_jmp(DRCTOP, mips3.drcdata->generate_invalidop_exception);			// jmp  generate_invalidop_exception
					return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
			}
			break;

//      default:
//          _jmp(mips3.drcdata->generate_invalidop_exception);                              // jmp  generate_invalidop_exception
//          return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
	}
	return RECOMPILE_UNIMPLEMENTED;
}



/***************************************************************************
    COP1 RECOMPILATION
***************************************************************************/

/*------------------------------------------------------------------
    recompile_cop1
------------------------------------------------------------------*/

static UINT32 recompile_cop1(drc_core *drc, UINT32 pc, UINT32 op)
{
	emit_link link1;
	int cycles, i;

	if (mips3.drcoptions & MIPS3DRC_STRICT_COP1)
	{
		emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_COP1);							// test [COP0_Status],SR_COP1
		emit_jcc(DRCTOP, COND_Z, mips3.drcdata->generate_cop_exception);					// jz   generate_cop_exception
	}

	switch (RSREG)
	{
		case 0x00:	/* MFCz */
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(RDREG));						// mov  eax,[RDREG]
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [RTREG],edx:eax
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x01:	/* DMFCz */
			if (RTREG != 0)
				emit_mov_m64_m64(DRCTOP, REGADDR(RTREG), FPR64ADDR(RDREG));
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x02:	/* CFCz */
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, CCR1ADDR(RDREG));							// mov  eax,[mips3.core->ccr[1][RDREG]]
				if (RDREG == 31)
				{
					emit_and_r32_imm(DRCTOP, REG_EAX, ~0xfe800000);							// and  eax,~0xfe800000
					emit_xor_r32_r32(DRCTOP, REG_EBX, REG_EBX);								// xor  ebx,ebx
					emit_cmp_m8_imm(DRCTOP, MABS(&mips3.core->cf[1][0]), 0);				// cmp  [cf[0]],0
					emit_setcc_r8(DRCTOP, COND_NZ, REG_BL);									// setnz bl
					emit_shl_r32_imm(DRCTOP, REG_EBX, 23);									// shl  ebx,23
					emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);								// or   eax,ebx
					if (mips3.core->flavor >= MIPS3_TYPE_MIPS_IV)
						for (i = 1; i <= 7; i++)
						{
							emit_cmp_m8_imm(DRCTOP, MABS(&mips3.core->cf[1][i]), 0);		// cmp  [cf[i]],0
							emit_setcc_r8(DRCTOP, COND_NZ, REG_BL);							// setnz bl
							emit_shl_r32_imm(DRCTOP, REG_EBX, 24+i);						// shl  ebx,24+i
							emit_or_r32_r32(DRCTOP, REG_EAX, REG_EBX);						// or   eax,ebx
						}
				}
				emit_cdq(DRCTOP);															// cdq
				emit_mov_m64_r64(DRCTOP, REGADDR(RTREG), REG_EDX, REG_EAX);					// mov  [mips3.core->r[RTREG]],edx:eax
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x04:	/* MTCz */
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[mips3.core->r[RTREG]]
				emit_mov_m32_r32(DRCTOP, FPR32ADDR(RDREG), REG_EAX);						// mov  [mips3.core->cpr[1][RDREG]],eax
			}
			else
				emit_mov_m32_imm(DRCTOP, FPR32ADDR(RDREG), 0);								// mov  [mips3.core->cpr[1][RDREG]],0
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x05:	/* DMTCz */
			if (RTREG != 0)
				emit_mov_m64_m64(DRCTOP, FPR64ADDR(RDREG), REGADDR(RTREG));
			else
				emit_zero_m64(DRCTOP, FPR64ADDR(RDREG));									// mov  [mips3.core->cpr[1][RDREG]],0
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x06:	/* CTCz */
			if (RTREG != 0)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));						// mov  eax,[mips3.core->r[RTREG]]
				emit_mov_m32_r32(DRCTOP, CCR1ADDR(RDREG), REG_EAX);							// mov  [mips3.core->ccr[1][RDREG]],eax
			}
			else
				emit_mov_m32_imm(DRCTOP, CCR1ADDR(RDREG), 0);								// mov  [mips3.core->ccr[1][RDREG]],0
			if (RDREG == 31)
			{
				emit_mov_r32_m32(DRCTOP, REG_EAX, MABS(LO(&mips3.core->ccr[1][RDREG])));	// mov  eax,[mips3.core->ccr[1][RDREG]]
				emit_test_r32_imm(DRCTOP, REG_EAX, 1 << 23);								// test eax,1<<23
				emit_setcc_m8(DRCTOP, COND_NZ, MABS(&mips3.core->cf[1][0]));				// setnz [cf[0]]
				if (mips3.core->flavor >= MIPS3_TYPE_MIPS_IV)
					for (i = 1; i <= 7; i++)
					{
						emit_test_r32_imm(DRCTOP, REG_EAX, 1 << (24+i));					// test eax,1<<(24+i)
						emit_setcc_m8(DRCTOP, COND_NZ, MABS(&mips3.core->cf[1][i]));		// setnz [cf[i]]
					}
				emit_and_r32_imm(DRCTOP, REG_EAX, 3);										// and  eax,3
				emit_test_r32_imm(DRCTOP, REG_EAX, 1);										// test eax,1
				emit_jcc_near_link(DRCTOP, COND_Z, &link1);									// jz   skip
				emit_xor_r32_imm(DRCTOP, REG_EAX, 2);										// xor  eax,2
				resolve_link(DRCTOP, &link1);											// skip:
				drc_append_set_fp_rounding(drc, REG_EAX);									// set_rounding(EAX)
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x08:	/* BC */
			switch ((op >> 16) & 3)
			{
				case 0x00:	/* BCzF */
					emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);					// cmp  [cf[x]],0
					emit_jcc_near_link(DRCTOP, COND_NZ, &link1);							// jnz  link1
					cycles = recompile_delay_slot(drc, pc + 4);								// <next instruction>
					append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);		// <branch or dispatch>
					resolve_link(DRCTOP, &link1);										// skip:
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x01:	/* BCzT */
					emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);					// cmp  [cf[x]],0
					emit_jcc_near_link(DRCTOP, COND_Z, &link1);								// jz   link1
					cycles = recompile_delay_slot(drc, pc + 4);								// <next instruction>
					append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);		// <branch or dispatch>
					resolve_link(DRCTOP, &link1);										// skip:
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x02:	/* BCzFL */
					emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);					// cmp  [cf[x]],0
					emit_jcc_near_link(DRCTOP, COND_NZ, &link1);							// jnz  link1
					cycles = recompile_delay_slot(drc, pc + 4);								// <next instruction>
					append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);		// <branch or dispatch>
					resolve_link(DRCTOP, &link1);										// skip:
					return RECOMPILE_SUCCESSFUL_CP(1,8);

				case 0x03:	/* BCzTL */
					emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);					// cmp  [cf[x]],0
					emit_jcc_near_link(DRCTOP, COND_Z, &link1);								// jz   link1
					cycles = recompile_delay_slot(drc, pc + 4);								// <next instruction>
					append_branch_or_dispatch(drc, pc + 4 + (SIMMVAL << 2), 1+cycles);		// <branch or dispatch>
					resolve_link(DRCTOP, &link1);										// skip:
					return RECOMPILE_SUCCESSFUL_CP(1,8);
			}
			break;

		default:
			switch (op & 0x3f)
			{
				case 0x00:
					if (IS_SINGLE(op))	/* ADD.S */
					{
						if (USE_SSE)
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_addss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// addss xmm0,[ftreg]
							emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);		// movss [fdreg],xmm0
						}
						else
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_faddp(DRCTOP);												// faddp
							emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));						// fstp [fdreg]
						}
					}
					else				/* ADD.D */
					{
						if (USE_SSE)
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_addsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// addsd xmm0,[ftreg]
							emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);		// movsd [fdreg],xmm0
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_faddp(DRCTOP);												// faddp
							emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));						// fstp [fdreg]
						}
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x01:
					if (IS_SINGLE(op))	/* SUB.S */
					{
						if (USE_SSE)
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_subss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// subss xmm0,[ftreg]
							emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);		// movss [fdreg],xmm0
						}
						else
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_fsubp(DRCTOP);												// fsubp
							emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));						// fstp [fdreg]
						}
					}
					else				/* SUB.D */
					{
						if (USE_SSE)
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_subsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// subsd xmm0,[ftreg]
							emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);		// movsd [fdreg],xmm0
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_fsubp(DRCTOP);												// fsubp
							emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));						// fstp [fdreg]
						}
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x02:
					if (IS_SINGLE(op))	/* MUL.S */
					{
						if (USE_SSE)
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// mulss xmm0,[ftreg]
							emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);		// movss [fdreg],xmm0
						}
						else
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_fmulp(DRCTOP);												// fmulp
							emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));						// fstp [fdreg]
						}
					}
					else				/* MUL.D */
					{
						if (USE_SSE)
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// mulsd xmm0,[ftreg]
							emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);		// movsd [fdreg],xmm0
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_fmulp(DRCTOP);												// fmulp
							emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));						// fstp [fdreg]
						}
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x03:
					if (IS_SINGLE(op))	/* DIV.S */
					{
						if (USE_SSE)
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_divss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// divss xmm0,[ftreg]
							emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);		// movss [fdreg],xmm0
						}
						else
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_fdivp(DRCTOP);												// fdivp
							emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));						// fstp [fdreg]
						}
					}
					else				/* DIV.D */
					{
						if (USE_SSE)
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_divsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// divsd xmm0,[ftreg]
							emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);		// movsd [fdreg],xmm0
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_fdivp(DRCTOP);												// fdivp
							emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));						// fstp [fdreg]
						}
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x04:
					if (IS_SINGLE(op))	/* SQRT.S */
					{
						if (USE_SSE)
						{
							emit_sqrtss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// sqrtss xmm0,[fsreg]
							emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);		// movss [fdreg],xmm0
						}
						else
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
							emit_fsqrt(DRCTOP);												// fsqrt
							emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));						// fstp [fdreg]
						}
					}
					else				/* SQRT.D */
					{
						if (USE_SSE)
						{
							emit_sqrtsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// sqrtsd xmm0,[fsreg]
							emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);		// movsd [fdreg],xmm0
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
							emit_fsqrt(DRCTOP);												// fsqrt
							emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));						// fstp [fdreg]
						}
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x05:
					if (IS_SINGLE(op))	/* ABS.S */
					{
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
						emit_fabs(DRCTOP);													// fabs
						emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));							// fstp [fdreg]
					}
					else				/* ABS.D */
					{
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
						emit_fabs(DRCTOP);													// fabs
						emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));							// fstp [fdreg]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x06:
					if (IS_SINGLE(op))	/* MOV.S */
					{
						emit_mov_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(FSREG));				// mov  eax,[fsreg]
						emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);				// mov  [fdreg],eax
					}
					else				/* MOV.D */
						emit_mov_m64_m64(DRCTOP, FPR64ADDR(FDREG), FPR64ADDR(FSREG));
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x07:
					if (IS_SINGLE(op))	/* NEG.S */
					{
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
						emit_fchs(DRCTOP);													// fchs
						emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));							// fstp [fdreg]
					}
					else				/* NEG.D */
					{
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
						emit_fchs(DRCTOP);													// fchs
						emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));							// fstp [fdreg]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x08:
					drc_append_set_temp_fp_rounding(drc, FPRND_NEAR);
					if (IS_SINGLE(op))	/* ROUND.L.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* ROUND.L.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m64(DRCTOP, FPR64ADDR(FDREG));								// fistp [fdreg]
					drc_append_restore_fp_rounding(drc);
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x09:
					drc_append_set_temp_fp_rounding(drc, FPRND_CHOP);
					if (IS_SINGLE(op))	/* TRUNC.L.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* TRUNC.L.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m64(DRCTOP, FPR64ADDR(FDREG));								// fistp [fdreg]
					drc_append_restore_fp_rounding(drc);
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x0a:
					drc_append_set_temp_fp_rounding(drc, FPRND_UP);
					if (IS_SINGLE(op))	/* CEIL.L.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* CEIL.L.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m64(DRCTOP, FPR64ADDR(FDREG));								// fistp [fdreg]
					drc_append_restore_fp_rounding(drc);
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x0b:
					drc_append_set_temp_fp_rounding(drc, FPRND_DOWN);
					if (IS_SINGLE(op))	/* FLOOR.L.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* FLOOR.L.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m64(DRCTOP, FPR64ADDR(FDREG));								// fistp [fdreg]
					drc_append_restore_fp_rounding(drc);
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x0c:
					drc_append_set_temp_fp_rounding(drc, FPRND_NEAR);
					if (IS_SINGLE(op))	/* ROUND.W.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* ROUND.W.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m32(DRCTOP, FPR32ADDR(FDREG));								// fistp [fdreg]
					drc_append_restore_fp_rounding(drc);
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x0d:
					drc_append_set_temp_fp_rounding(drc, FPRND_CHOP);
					if (IS_SINGLE(op))	/* TRUNC.W.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* TRUNC.W.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m32(DRCTOP, FPR32ADDR(FDREG));								// fistp [fdreg]
					drc_append_restore_fp_rounding(drc);
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x0e:
					drc_append_set_temp_fp_rounding(drc, FPRND_UP);
					if (IS_SINGLE(op))	/* CEIL.W.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* CEIL.W.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m32(DRCTOP, FPR32ADDR(FDREG));								// fistp [fdreg]
					drc_append_restore_fp_rounding(drc);
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x0f:
					drc_append_set_temp_fp_rounding(drc, FPRND_DOWN);
					if (IS_SINGLE(op))	/* FLOOR.W.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* FLOOR.W.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m32(DRCTOP, FPR32ADDR(FDREG));								// fistp [fdreg]
					drc_append_restore_fp_rounding(drc);
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x11:	/* R5000 */
					if (mips3.core->flavor < MIPS3_TYPE_MIPS_IV)
					{
						emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);
																							// jmp  generate_invalidop_exception
						return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
					}
					emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);					// cmp  [cf[x]],0
					emit_jcc_short_link(DRCTOP, ((op >> 16) & 1) ? COND_Z : COND_NZ, &link1);
																							// jz/nz skip
					if (IS_SINGLE(op))	/* MOVT/F.S */
					{
						emit_mov_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(FSREG));				// mov  eax,[fsreg]
						emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);				// mov  [fdreg],eax
					}
					else				/* MOVT/F.D */
						emit_mov_m64_m64(DRCTOP, FPR64ADDR(FDREG), FPR64ADDR(FSREG));
					resolve_link(DRCTOP, &link1);										// skip:
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x12:	/* R5000 */
					if (mips3.core->flavor < MIPS3_TYPE_MIPS_IV)
					{
						emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);
																							// jmp  generate_invalidop_exception
						return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
					}
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));						// or   eax,[rtreg].hi
					emit_jcc_short_link(DRCTOP, COND_NZ, &link1);							// jnz  skip
					if (IS_SINGLE(op))	/* MOVZ.S */
					{
						emit_mov_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(FSREG));				// mov  eax,[fsreg]
						emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);				// mov  [fdreg],eax
					}
					else				/* MOVZ.D */
						emit_mov_m64_m64(DRCTOP, FPR64ADDR(FDREG), FPR64ADDR(FSREG));
					resolve_link(DRCTOP, &link1);										// skip:
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x13:	/* R5000 */
					if (mips3.core->flavor < MIPS3_TYPE_MIPS_IV)
					{
						emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);
																							// jmp  generate_invalidop_exception
						return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
					}
					emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));					// mov  eax,[rtreg].lo
					emit_or_r32_m32(DRCTOP, REG_EAX, REGADDRHI(RTREG));						// or   eax,[rtreg].hi
					emit_jcc_short_link(DRCTOP, COND_Z, &link1);							// jz   skip
					if (IS_SINGLE(op))	/* MOVN.S */
					{
						emit_mov_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(FSREG));				// mov  eax,[fsreg]
						emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);				// mov  [fdreg],eax
					}
					else				/* MOVN.D */
						emit_mov_m64_m64(DRCTOP, FPR64ADDR(FDREG), FPR64ADDR(FSREG));
					resolve_link(DRCTOP, &link1);										// skip:
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x15:	/* R5000 */
					if (mips3.core->flavor < MIPS3_TYPE_MIPS_IV)
					{
						emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);
																							// jmp  generate_invalidop_exception
						return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
					}
					emit_fld1(DRCTOP);														// fld1
					if (IS_SINGLE(op))	/* RECIP.S */
					{
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
						emit_fdivp(DRCTOP);													// fdivp
						emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));							// fstp [fdreg]
					}
					else				/* RECIP.D */
					{
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
						emit_fdivp(DRCTOP);													// fdivp
						emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));							// fstp [fdreg]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x16:	/* R5000 */
					if (mips3.core->flavor < MIPS3_TYPE_MIPS_IV)
					{
						emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);
																							// jmp  generate_invalidop_exception
						return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
					}
					emit_fld1(DRCTOP);														// fld1
					if (IS_SINGLE(op))	/* RSQRT.S */
					{
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
						emit_fsqrt(DRCTOP);													// fsqrt
						emit_fdivp(DRCTOP);													// fdivp
						emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));							// fstp [fdreg]
					}
					else				/* RSQRT.D */
					{
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
						emit_fsqrt(DRCTOP);													// fsqrt
						emit_fdivp(DRCTOP);													// fdivp
						emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));							// fstp [fdreg]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x20:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.S.W */
							emit_fild_m32(DRCTOP, FPR32ADDR(FSREG));						// fild [fsreg]
						else				/* CVT.S.L */
							emit_fild_m64(DRCTOP, FPR64ADDR(FSREG));						// fild [fsreg]
					}
					else					/* CVT.S.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));								// fstp [fdreg]
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x21:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.D.W */
							emit_fild_m32(DRCTOP, FPR32ADDR(FSREG));						// fild [fsreg]
						else				/* CVT.D.L */
							emit_fild_m64(DRCTOP, FPR64ADDR(FSREG));						// fild [fsreg]
					}
					else					/* CVT.D.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));								// fstp [fdreg]
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x24:
					if (IS_SINGLE(op))	/* CVT.W.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* CVT.W.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m32(DRCTOP, FPR32ADDR(FDREG));								// fistp [fdreg]
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x25:
					if (IS_SINGLE(op))	/* CVT.L.S */
						emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));								// fld  [fsreg]
					else				/* CVT.L.D */
						emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));								// fld  [fsreg]
					emit_fistp_m64(DRCTOP, FPR64ADDR(FDREG));								// fistp [fdreg]
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x30:
				case 0x38:
					emit_mov_m8_imm(DRCTOP, CF1ADDR((op >> 8) & 7), 0);						/* C.F.S/D */
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x31:
				case 0x39:
					emit_mov_m8_imm(DRCTOP, CF1ADDR((op >> 8) & 7), 0);						/* C.UN.S/D */
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x32:
				case 0x3a:
					if (USE_SSE)
					{
						if (IS_SINGLE(op))	/* C.EQ.S */
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_comiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// comiss xmm0,[ftreg]
						}
						else
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_comisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// comisd xmm0,[ftreg]
						}
						emit_setcc_m8(DRCTOP, COND_E, CF1ADDR((op >> 8) & 7)); 				// sete [cf[x]]
					}
					else
					{
						if (IS_SINGLE(op))	/* C.EQ.S */
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
						}
						emit_fcompp(DRCTOP);												// fcompp
						emit_fstsw_ax(DRCTOP);												// fnstsw ax
						emit_sahf(DRCTOP);													// sahf
						emit_setcc_m8(DRCTOP, COND_E, CF1ADDR((op >> 8) & 7)); 				// sete [cf[x]]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x33:
				case 0x3b:
					if (USE_SSE)
					{
						if (IS_SINGLE(op))	/* C.UEQ.S */
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_ucomiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// ucomiss xmm0,[ftreg]
						}
						else
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_ucomisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// ucomisd xmm0,[ftreg]
						}
						emit_setcc_m8(DRCTOP, COND_E, CF1ADDR((op >> 8) & 7)); 				// sete [cf[x]]
					}
					else
					{
						if (IS_SINGLE(op))	/* C.UEQ.S */
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
						}
						emit_fucompp(DRCTOP);												// fucompp
						emit_fstsw_ax(DRCTOP);												// fnstsw ax
						emit_sahf(DRCTOP);													// sahf
						emit_setcc_m8(DRCTOP, COND_E, CF1ADDR((op >> 8) & 7)); 				// sete [cf[x]]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x34:
				case 0x3c:
					if (USE_SSE)
					{
						if (IS_SINGLE(op))	/* C.OLT.S */
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_comiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// comiss xmm0,[ftreg]
						}
						else
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_comisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// comisd xmm0,[ftreg]
						}
						emit_setcc_m8(DRCTOP, COND_B, CF1ADDR((op >> 8) & 7)); 				// setb [cf[x]]
					}
					else
					{
						if (IS_SINGLE(op))	/* C.OLT.S */
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
						}
						emit_fcompp(DRCTOP);												// fcompp
						emit_fstsw_ax(DRCTOP);												// fnstsw ax
						emit_sahf(DRCTOP);													// sahf
						emit_setcc_m8(DRCTOP, COND_B, CF1ADDR((op >> 8) & 7)); 				// setb [cf[x]]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x35:
				case 0x3d:
					if (USE_SSE)
					{
						if (IS_SINGLE(op))	/* C.ULT.S */
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_ucomiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// ucomiss xmm0,[ftreg]
						}
						else
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_ucomisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// ucomisd xmm0,[ftreg]
						}
						emit_setcc_m8(DRCTOP, COND_B, CF1ADDR((op >> 8) & 7)); 				// setb [cf[x]]
					}
					else
					{
						if (IS_SINGLE(op))	/* C.ULT.S */
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
						}
						emit_fucompp(DRCTOP);												// fucompp
						emit_fstsw_ax(DRCTOP);												// fnstsw ax
						emit_sahf(DRCTOP);													// sahf
						emit_setcc_m8(DRCTOP, COND_B, CF1ADDR((op >> 8) & 7)); 				// setb [cf[x]]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x36:
				case 0x3e:
					if (USE_SSE)
					{
						if (IS_SINGLE(op))	/* C.OLE.S */
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_comiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// comiss xmm0,[ftreg]
						}
						else
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_comisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// comisd xmm0,[ftreg]
						}
						emit_setcc_m8(DRCTOP, COND_BE, CF1ADDR((op >> 8) & 7)); 			// setle [cf[x]]
					}
					else
					{
						if (IS_SINGLE(op))	/* C.OLE.S */
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
						}
						emit_fcompp(DRCTOP);												// fcompp
						emit_fstsw_ax(DRCTOP);												// fnstsw ax
						emit_sahf(DRCTOP);													// sahf
						emit_setcc_m8(DRCTOP, COND_BE, CF1ADDR((op >> 8) & 7)); 			// setbe [cf[x]]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);

				case 0x37:
				case 0x3f:
					if (USE_SSE)
					{
						if (IS_SINGLE(op))	/* C.ULE.S */
						{
							emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// movss xmm0,[fsreg]
							emit_ucomiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));		// ucomiss xmm0,[ftreg]
						}
						else
						{
							emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// movsd xmm0,[fsreg]
							emit_ucomisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));		// ucomisd xmm0,[ftreg]
						}
						emit_setcc_m8(DRCTOP, COND_BE, CF1ADDR((op >> 8) & 7)); 			// setl [cf[x]]
					}
					else
					{
						if (IS_SINGLE(op))	/* C.ULE.S */
						{
							emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));							// fld  [fsreg]
						}
						else
						{
							emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));							// fld  [ftreg]
							emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));							// fld  [fsreg]
						}
						emit_fucompp(DRCTOP);												// fucompp
						emit_fstsw_ax(DRCTOP);												// fnstsw ax
						emit_sahf(DRCTOP);													// sahf
						emit_setcc_m8(DRCTOP, COND_BE, CF1ADDR((op >> 8) & 7)); 			// setbe [cf[x]]
					}
					return RECOMPILE_SUCCESSFUL_CP(1,4);
			}
			break;
	}
	emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);					// jmp  generate_invalidop_exception
	return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
}



/***************************************************************************
    COP1X RECOMPILATION
***************************************************************************/

/*------------------------------------------------------------------
    recompile_cop1x
------------------------------------------------------------------*/

static UINT32 recompile_cop1x(drc_core *drc, UINT32 pc, UINT32 op)
{
	if (mips3.drcoptions & MIPS3DRC_STRICT_COP1)
	{
		emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_COP1);							// test [COP0_Status],SR_COP1
		emit_jcc(DRCTOP, COND_Z, mips3.drcdata->generate_cop_exception);					// jz   generate_cop_exception
	}

	switch (op & 0x3f)
	{
		case 0x00:		/* LWXC1 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg]
			emit_add_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));							// add  eax,[rtreg]
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_word);						// call read_and_translate_word
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);							// mov  [fdreg],eax
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x01:		/* LDXC1 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 8);											// sub  esp,8
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg]
			emit_add_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));							// add  eax,[rtreg]
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->read_and_translate_double);					// call read_and_translate_double
			emit_mov_m32_r32(DRCTOP, FPR64ADDRLO(FDREG), REG_EAX);							// mov  [fdreg].lo,eax
			emit_mov_m32_r32(DRCTOP, FPR64ADDRHI(FDREG), REG_EDX);							// mov  [fdreg].hi,edx
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x08:		/* SWXC1 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_sub_r32_imm(DRCTOP, REG_ESP, 4);											// sub  esp,4
			emit_push_m32(DRCTOP, FPR32ADDR(FSREG));										// push [fsreg]
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg]
			emit_add_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));							// add  eax,[rtreg]
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_word);						// call write_and_translate_word
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,8
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x09:		/* SDXC1 */
			emit_mov_m32_r32(DRCTOP, ICOUNTADDR, REG_EBP);									// mov  [icount],ebp
			emit_save_pc_before_call(DRCTOP);												// save pc
			emit_push_m32(DRCTOP, FPR64ADDRHI(FSREG));										// push [fsreg].hi
			emit_push_m32(DRCTOP, FPR64ADDRLO(FSREG));										// push [fsreg].lo
			emit_mov_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RSREG));							// mov  eax,[rsreg]
			emit_add_r32_m32(DRCTOP, REG_EAX, REGADDRLO(RTREG));							// add  eax,[rtreg]
			emit_push_r32(DRCTOP, REG_EAX);													// push eax
			emit_call(DRCTOP, mips3.drcdata->write_and_translate_double);					// call write_and_translate_double
			emit_add_r32_imm(DRCTOP, REG_ESP, 12);											// add  esp,12
			emit_mov_r32_m32(DRCTOP, REG_EBP, ICOUNTADDR);									// mov  ebp,[icount]
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x0f:		/* PREFX */
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x20:		/* MADD.S */
			if (USE_SSE)
			{
				emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));					// movss xmm0,[fsreg]
				emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));					// mulss xmm0,[ftreg]
				emit_addss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FRREG));					// addss xmm0,[frreg]
				emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);					// movss [fdreg],xmm0
			}
			else
			{
				emit_fld_m32(DRCTOP, FPR32ADDR(FRREG));										// fld  [frreg]
				emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));										// fld  [fsreg]
				emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));										// fld  [ftreg]
				emit_fmulp(DRCTOP);															// fmulp
				emit_faddp(DRCTOP);															// faddp
				emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));									// fstp [fdreg]
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x21:		/* MADD.D */
			if (USE_SSE)
			{
				emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));					// movsd xmm0,[fsreg]
				emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));					// mulsd xmm0,[ftreg]
				emit_addsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FRREG));					// addsd xmm0,[frreg]
				emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);					// movsd [fdreg],xmm0
			}
			else
			{
				emit_fld_m64(DRCTOP, FPR64ADDR(FRREG));										// fld  [frreg]
				emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));										// fld  [fsreg]
				emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));										// fld  [ftreg]
				emit_fmulp(DRCTOP);															// fmulp
				emit_faddp(DRCTOP);															// faddp
				emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));									// fstp [fdreg]
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x28:		/* MSUB.S */
			if (USE_SSE)
			{
				emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));					// movss xmm0,[fsreg]
				emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));					// mulss xmm0,[ftreg]
				emit_subss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FRREG));					// subss xmm0,[frreg]
				emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);					// movss [fdreg],xmm0
			}
			else
			{
				emit_fld_m32(DRCTOP, FPR32ADDR(FRREG));										// fld  [frreg]
				emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));										// fld  [fsreg]
				emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));										// fld  [ftreg]
				emit_fmulp(DRCTOP);															// fmulp
				emit_fsubrp(DRCTOP);														// fsubrp
				emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));									// fstp [fdreg]
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x29:		/* MSUB.D */
			if (USE_SSE)
			{
				emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));					// movsd xmm0,[fsreg]
				emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));					// mulsd xmm0,[ftreg]
				emit_subsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FRREG));					// subsd xmm0,[frreg]
				emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);					// movsd [fdreg],xmm0
			}
			else
			{
				emit_fld_m64(DRCTOP, FPR64ADDR(FRREG));										// fld  [frreg]
				emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));										// fld  [fsreg]
				emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));										// fld  [ftreg]
				emit_fmulp(DRCTOP);															// fmulp
				emit_fsubrp(DRCTOP);														// fsubrp
				emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));									// fstp [fdreg]
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x30:		/* NMADD.S */
			if (USE_SSE)
			{
				emit_xorps_r128_r128(DRCTOP, REG_XMM1, REG_XMM1);							// xorps xmm1,xmm1
				emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));					// movss xmm0,[fsreg]
				emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));					// mulss xmm0,[ftreg]
				emit_addss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FRREG));					// addss xmm0,[frreg]
				emit_subss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);							// subss xmm1,xmm0
				emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM1);					// movss [fdreg],xmm1
			}
			else
			{
				emit_fld_m32(DRCTOP, FPR32ADDR(FRREG));										// fld  [frreg]
				emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));										// fld  [fsreg]
				emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));										// fld  [ftreg]
				emit_fmulp(DRCTOP);															// fmulp
				emit_faddp(DRCTOP);															// faddp
				emit_fchs(DRCTOP);															// fchs
				emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));									// fstp [fdreg]
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x31:		/* NMADD.D */
			if (USE_SSE)
			{
				emit_xorps_r128_r128(DRCTOP, REG_XMM1, REG_XMM1);							// xorps xmm1,xmm1
				emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));					// movsd xmm0,[fsreg]
				emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));					// mulsd xmm0,[ftreg]
				emit_addsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FRREG));					// addsd xmm0,[frreg]
				emit_subss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);							// subss xmm1,xmm0
				emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM1);					// movsd [fdreg],xmm1
			}
			else
			{
				emit_fld_m64(DRCTOP, FPR64ADDR(FRREG));										// fld  [frreg]
				emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));										// fld  [fsreg]
				emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));										// fld  [ftreg]
				emit_fmulp(DRCTOP);															// fmulp
				emit_faddp(DRCTOP);															// faddp
				emit_fchs(DRCTOP);															// fchs
				emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));									// fstp [fdreg]
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x38:		/* NMSUB.S */
			if (USE_SSE)
			{
				emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));					// movss xmm0,[fsreg]
				emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));					// mulss xmm0,[ftreg]
				emit_movss_r128_m32(DRCTOP, REG_XMM1, FPR32ADDR(FRREG));					// movss xmm1,[frreg]
				emit_subss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);							// subss xmm1,xmm0
				emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM1);					// movss [fdreg],xmm1
			}
			else
			{
				emit_fld_m32(DRCTOP, FPR32ADDR(FRREG));										// fld  [frreg]
				emit_fld_m32(DRCTOP, FPR32ADDR(FSREG));										// fld  [fsreg]
				emit_fld_m32(DRCTOP, FPR32ADDR(FTREG));										// fld  [ftreg]
				emit_fmulp(DRCTOP);															// fmulp
				emit_fsubp(DRCTOP);															// fsubp
				emit_fstp_m32(DRCTOP, FPR32ADDR(FDREG));									// fstp [fdreg]
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x39:		/* NMSUB.D */
			if (USE_SSE)
			{
				emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));					// movsd xmm0,[fsreg]
				emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));					// mulsd xmm0,[ftreg]
				emit_movsd_r128_m64(DRCTOP, REG_XMM1, FPR64ADDR(FRREG));					// movsd xmm1,[frreg]
				emit_subss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);							// subss xmm1,xmm0
				emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM1);					// movsd [fdreg],xmm1
			}
			else
			{
				emit_fld_m64(DRCTOP, FPR64ADDR(FRREG));										// fld  [frreg]
				emit_fld_m64(DRCTOP, FPR64ADDR(FSREG));										// fld  [fsreg]
				emit_fld_m64(DRCTOP, FPR64ADDR(FTREG));										// fld  [ftreg]
				emit_fmulp(DRCTOP);															// fmulp
				emit_fsubp(DRCTOP);															// fsubrp
				emit_fstp_m64(DRCTOP, FPR64ADDR(FDREG));									// fstp [fdreg]
			}
			return RECOMPILE_SUCCESSFUL_CP(1,4);

		case 0x24:		/* MADD.W */
		case 0x25:		/* MADD.L */
		case 0x2c:		/* MSUB.W */
		case 0x2d:		/* MSUB.L */
		case 0x34:		/* NMADD.W */
		case 0x35:		/* NMADD.L */
		case 0x3c:		/* NMSUB.W */
		case 0x3d:		/* NMSUB.L */
		default:
			fprintf(stderr, "cop1x %X\n", op);
			break;
	}
	emit_jmp(DRCTOP, (void *)mips3.drcdata->generate_invalidop_exception);					// jmp  generate_invalidop_exception
	return RECOMPILE_SUCCESSFUL | RECOMPILE_END_OF_STRING;
}
