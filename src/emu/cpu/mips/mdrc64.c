/***************************************************************************

    mdrc64.c

    x64 MIPS III recompiler.

    Copyright Aaron Giles
    Released for general use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Conventions/rules:

        * Instructions are grouped into sequences by mips3fe; each
          sequence of instructions is guaranteed to not contain any
          branches to any instructions within the sequence apart from
          the first.

        * Because of the above rule, assumptions are made during the
          execution of a sequence in order to optimize the generated
          code.

        * Because some state may be live and not flushed to memory at
          the time an exception is generated, all exception-generating
          instructions must jump to stub code that is appended to each
          group of instructions which cleans up live registers before
          running the exception code.

        * Page faults are trickier because they are detected in a
          subroutine. In order to handle this, special markers are added
          to the code consisting of one qword EXCEPTION_ADDRESS_COOKIE
          followed by the return address of the call. When a page fault
          needs to be generated, the common code scans forward from
          the return address for this cookie/address pair and jumps to
          the code immediately following.

        * Convention: jumps to non-constant addresses have their target
          PC stored at [sp+SPOFFS_NEXTPC]

****************************************************************************

    Future improvements:
        * problem:
            lui  r3,$b000
            lw   r3,$100(r3)
          if exception happens, then r3 is trashed

        * register allocator can use non-volatile registers if the
            lifetime is only for this one instruction (only for
            registers where (gpr.used & ~gpr.modified)

        * spec says if registers aren't sign-extended, then 32-bit ops have
            undefined results; this can simplify our code

        * track which results are 32-bit sign extended versus 64-bit and
            only promote when necessary

        * register aliasing - move from one to another either doesn't do
            anything and tracks both, or else shifts ownership to new
            target

        * generic drc:
            * common code to load registers prior to instruction
                - each register has a callback that generates the load
            * generate code
            * common code to write registers afterwards
            * common register allocation routines (64 integer, 64 FPU)
            * common OOB mechanism
            * common self-mod code detection (via write bitmask)
            * common code validation routines

****************************************************************************

    Temporary stack usage:
        [esp+32].b  = storage for CL in LWL/LWR/SWL/SWR
        [esp+36].l  = storage for new target PC in JR/JALR

***************************************************************************/

#include "mips3fe.h"
#include "deprecat.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define DISABLE_FIXED_RAM_LOAD			(0)
#define DISABLE_FIXED_RAM_STORE			(0)
#define FLUSH_AFTER_EACH_INSTRUCTION	(0)
#define PRINTF_EXCEPTIONS				(0)
#define PROBE_ADDRESS					(0)
#define LOG_HOTSPOTS					(0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* extra "conditions" used to augment the basic conditions for OOB handlers */
#define COND_NONE						16
#define COND_NO_JUMP					17

/* 64-bit cookie that is scanned for during exception processing */
#define EXCEPTION_ADDRESS_COOKIE		U64(0xcc90cc90cc90cc90)

/* append_readwrite flags */
#define ARW_READ						0x0000
#define ARW_WRITE						0x0001
#define ARW_UNSIGNED					0x0000
#define ARW_SIGNED						0x0002
#define ARW_MASKED						0x0004
#define ARW_CACHED						0x0008
#define ARW_UNCACHED					0x0010

/* compiler_register_allocate flags */
#define ALLOC_SRC						0x0001
#define ALLOC_DST						0x0002
#define ALLOC_DIRTY						0x0004

/* x86 register tracking flags */
#define X86REG_LIVE						0x80		/* register is live and contains something */
#define X86REG_MIPSMASK					0x3f		/* low 6 bits indicate which MIPS register we hold */

/* MIPS register tracking flags */
#define MIPSREG_CONSTANT				0x8000		/* register is constant; low 10 bits select which one */
#define MIPSREG_DIRTY					0x4000		/* register has been modified and needs to be flushed */
#define MIPSREG_LIVE					0x2000		/* register is live in an x86 register */
#define MIPSREG_X86MASK					0x000f		/* low 4 bits indicate which x86 register holds us */



/***************************************************************************
    MACROS
***************************************************************************/

#define SPOFFS_SAVECL			32
#define SPOFFS_NEXTPC			36

#define REGADDR(reg)			MDRC(&mips3.core->r[reg])
#define HIADDR					MDRC(&mips3.core->r[REG_HI])
#define LOADDR					MDRC(&mips3.core->r[REG_LO])
#define CPR0ADDR(reg)			MDRC(&mips3.core->cpr[0][reg])
#define CCR0ADDR(reg)			MDRC(&mips3.core->ccr[0][reg])
#define CPR1ADDR(reg)			MDRC(&mips3.core->cpr[1][reg])
#define CCR1ADDR(reg)			MDRC(&mips3.core->ccr[1][reg])
#define CPR2ADDR(reg)			MDRC(&mips3.core->cpr[2][reg])
#define CCR2ADDR(reg)			MDRC(&mips3.core->ccr[2][reg])
#define FPR32(reg)				(IS_FR0 ? &((float *)&mips3.core->cpr[1][0])[reg] : (float *)&mips3.core->cpr[1][reg])
#define FPR32ADDR(reg)			MDRC(FPR32(reg))
#define FPR64(reg)				(IS_FR0 ? (double *)&mips3.core->cpr[1][(reg)/2] : (double *)&mips3.core->cpr[1][reg])
#define FPR64ADDR(reg)			MDRC(FPR64(reg))
#define CF1ADDR(which)			MDRC(&mips3.core->cf[1][(mips3.core->flavor < MIPS3_TYPE_MIPS_IV) ? 0 : (which)])
#define PCADDR					MDRC(&mips3.core->pc)
#define ICOUNTADDR				MDRC(&mips3.core->icount)
#define COREADDR				MDRC(mips3.core)

/* macros for querying/accessing the tracking flags */
#define ISCONST(x)				(((x) & MIPSREG_CONSTANT) != 0)
#define ISDIRTY(x)				(((x) & MIPSREG_DIRTY) != 0)
#define ISLIVE(x)				(((x) & MIPSREG_LIVE) != 0)
#define CONSTVAL(x)				((INT32)((x) >> 32))
#define X86REG(x)				((x) & MIPSREG_X86MASK)

/* macro for defining a small positivec constant inline */
#define MAKE_CONST(x)			(MIPSREG_CONSTANT | ((INT64)(x) << 32))
#define MAKE_DESTREG(x)			(MIPSREG_DIRTY | ((x) & MIPSREG_X86MASK))



/***************************************************************************
    TYPEDEFS
***************************************************************************/

typedef struct _readwrite_handlers readwrite_handlers;
struct _readwrite_handlers
{
	x86code *		read_byte_signed;
	x86code *		read_byte_unsigned;
	x86code *		read_half_signed;
	x86code *		read_half_unsigned;
	x86code *		read_word_signed;
	x86code *		read_word_unsigned;
	x86code *		read_word_masked;
	x86code *		read_double;
	x86code *		read_double_masked;
	x86code *		write_byte;
	x86code *		write_half;
	x86code *		write_word;
	x86code *		write_word_masked;
	x86code *		write_double;
	x86code *		write_double_masked;
};


typedef INT64 		mipsreg_state;
typedef UINT8		x86reg_state;


typedef struct _compiler_state compiler_state;
struct _compiler_state
{
	UINT32			cycles;					/* accumulated cycles */
	x86reg_state	x86regs[REG_MAX];		/* tracking what is in the x86 registers */
	mipsreg_state	mipsregs[35];			/* tracking what is in the MIPS registers */
};


typedef struct _oob_handler oob_handler;

typedef void (*oob_callback_func)(drc_core *drc, oob_handler *oob, void *param);

struct _oob_handler
{
	oob_callback_func		callback;			/* pointer to callback function */
	void *				param;				/* callback parameter */
	const opcode_desc *	desc;				/* pointer to description of relevant instruction */
	emit_link			link;				/* link to source of the branch */
	compiler_state		compiler;			/* state of the compiler at this time */
};


struct _mips3drc_data
{
	/* misc data */
	UINT64				abs64mask[2];
	UINT32				abs32mask[4];

	/* C functions */
	x86code *			activecpu_gettotalcycles;
	x86code *			mips3com_update_cycle_counting;
	x86code *			mips3com_tlbr;
	x86code *			mips3com_tlbwi;
	x86code *			mips3com_tlbwr;
	x86code *			mips3com_tlbp;

	/* internal functions */
	x86code *			generate_interrupt_exception;
	x86code *			generate_cop_exception;
	x86code *			generate_overflow_exception;
	x86code *			generate_invalidop_exception;
	x86code *			generate_syscall_exception;
	x86code *			generate_break_exception;
	x86code *			generate_trap_exception;
	x86code *			generate_tlbload_exception;
	x86code *			generate_tlbstore_exception;
	x86code *			handle_pc_tlb_mismatch;
	x86code *			find_exception_handler;
	x86code *			explode_ccr31;
	x86code *			recover_ccr31;

	/* read/write handlers */
	readwrite_handlers 	general;
	readwrite_handlers 	cached;
	readwrite_handlers 	uncached;

	/* out of bounds handlers */
	oob_handler			ooblist[COMPILE_MAX_INSTRUCTIONS*2];
	int					oobcount;

	/* C functions */
	x86code *			execute_c_version;

	/* hotspot tracking */
#if LOG_HOTSPOTS
	struct
	{
		offs_t			pc;
		UINT32			opcode;
		UINT64			count;
	} hotspot[100000];
	int					next_hotspot;
#endif
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void drc_reset_callback(drc_core *drc);
static void drc_recompile_callback(drc_core *drc);
static void drc_entrygen_callback(drc_core *drc);

static void compile_one(drc_core *drc, compiler_state *compiler, const opcode_desc *desc);

static void oob_exception_cleanup(drc_core *drc, oob_handler *oob, void *param);
static void oob_interrupt_cleanup(drc_core *drc, oob_handler *oob, void *param);

static void compiler_register_set_constant(compiler_state *compiler, UINT8 mipsreg, UINT64 constval);
static mipsreg_state compiler_register_allocate(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT8 mipsreg, UINT8 flags);
static void compiler_register_flush(drc_core *drc, compiler_state *compiler, UINT8 mipsreg, int free);
static void compiler_register_flush_all(drc_core *drc, compiler_state *compiler);

static void probe_printf(void);

static void generate_read_write_handlers(drc_core *drc, readwrite_handlers *handlers, const char *type, UINT8 flags);
static void append_generate_exception(drc_core *drc, UINT8 exception);
static void append_readwrite_and_translate(drc_core *drc, int size, UINT8 flags, UINT32 ptroffs);
static void append_handle_pc_tlb_mismatch(drc_core *drc);
static void append_find_exception_handler(drc_core *drc);
static void append_explode_ccr31(drc_core *drc);
static void append_recover_ccr31(drc_core *drc);
static void append_check_interrupts(drc_core *drc, compiler_state *compiler, const opcode_desc *desc);
//static void append_check_sw_interrupts(drc_core *drc, int inline_generate);

static void emit_update_cycles_pc_and_flush(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, offs_t destpc);

static int compile_instruction(drc_core *drc, compiler_state *compiler, const opcode_desc *desc);
static int compile_special(drc_core *drc, compiler_state *compiler, const opcode_desc *desc);
static int compile_regimm(drc_core *drc, compiler_state *compiler, const opcode_desc *desc);
static int compile_idt(drc_core *drc, compiler_state *compiler, const opcode_desc *desc);
static int compile_cop0(drc_core *drc, compiler_state *compiler, const opcode_desc *desc);
static int compile_cop1(drc_core *drc, compiler_state *compiler, const opcode_desc *desc);
static int compile_cop1x(drc_core *drc, compiler_state *compiler, const opcode_desc *desc);

static int compile_shift(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int shift_type, int shift_variable);
static int compile_dshift(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int shift_type, int shift_variable, int shift_const_plus32);
static int compile_movzn(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_movn);
static int compile_trapcc(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int condition, int immediate);
static int compile_branchcc(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int condition);
static int compile_branchcz(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int condition, int link);
static int compile_add(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int suppress_exceptions, int immediate);
static int compile_dadd(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int suppress_exceptions, int immediate);
static int compile_sub(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int suppress_exceptions);
static int compile_dsub(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int suppress_exceptions);
static int compile_mult(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned);
static int compile_dmult(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned);
static int compile_div(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned);
static int compile_ddiv(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned);
static int compile_logical(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int operation, int immediate);
static int compile_slt(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned, int immediate);
static int compile_load(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, int is_unsigned);
static int compile_load_cop(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, void *dest);
static int compile_loadx_cop(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, void *dest);
static int compile_load_word_partial(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_right);
static int compile_load_double_partial(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_right);
static int compile_store(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size);
static int compile_store_cop(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, void *src);
static int compile_storex_cop(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, void *src);
static int compile_store_word_partial(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_right);
static int compile_store_double_partial(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_right);



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static const UINT8 nvreg[] = { REG_NV0, REG_NV1, REG_NV2, REG_NV3, REG_NV4, REG_NV5, REG_NV6 };

static const char *const x86regname[] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15"};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    oob_request_callback - request an out-of-
    bounds callback for later
-------------------------------------------------*/

INLINE void oob_request_callback(drc_core *drc, UINT8 condition, oob_callback_func callback, const compiler_state *compiler, const opcode_desc *desc, void *param)
{
	oob_handler *oob = &mips3.drcdata->ooblist[mips3.drcdata->oobcount++];

	/* fill in the next handler */
	oob->callback = callback;
	oob->desc = desc;
	oob->compiler = *compiler;
	oob->param = param;

	/* emit an appropriate jump */
	if (condition == COND_NO_JUMP)
	{
		oob->link.size = 0;
		oob->link.target = drc->cache_top;
	}
	else if (condition == COND_NONE)
		emit_jmp_near_link(DRCTOP, &oob->link);												// jmp  <target>
	else
		emit_jcc_near_link(DRCTOP, condition, &oob->link);									// jcc  <target>
}


/*-------------------------------------------------
    compiler_state_reset - reset the compiler
    state
-------------------------------------------------*/

INLINE void compiler_state_reset(compiler_state *compiler)
{
	memset(compiler, 0, sizeof(*compiler));
	compiler_register_set_constant(compiler, 0, 0);
}



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
	drconfig.max_instructions = (512 + 128) / 4;
	drconfig.address_bits     = 32;
	drconfig.lsbs_to_ignore   = 2;
	drconfig.baseptr	      = &mips3.core->r[17];
	drconfig.pcptr            = (UINT32 *)&mips3.core->pc;
	drconfig.cb_reset         = drc_reset_callback;
	drconfig.cb_recompile     = drc_recompile_callback;
	drconfig.cb_entrygen      = drc_entrygen_callback;

	/* initialize the compiler */
	mips3.drc = drc_init(cpu_getactivecpu(), &drconfig);
	mips3.drcoptions = MIPS3DRC_FASTEST_OPTIONS;

	/* allocate our data out of the cache */
	mips3.drcdata = drc_alloc(mips3.drc, sizeof(*mips3.drcdata) + 16);
	mips3.drcdata = (mips3drc_data *)((((FPTR)mips3.drcdata + 15) >> 4) << 4);
	memset(mips3.drcdata, 0, sizeof(*mips3.drcdata));

	/* set up constants */
	mips3.drcdata->abs64mask[0] = U64(0x7fffffffffffffff);
	mips3.drcdata->abs32mask[0] = 0x7fffffff;

	/* get pointers to C functions */
	mips3.drcdata->activecpu_gettotalcycles = (x86code *)activecpu_gettotalcycles;
	mips3.drcdata->mips3com_update_cycle_counting = (x86code *)mips3com_update_cycle_counting;
	mips3.drcdata->mips3com_tlbr = (x86code *)mips3com_tlbr;
	mips3.drcdata->mips3com_tlbwi = (x86code *)mips3com_tlbwi;
	mips3.drcdata->mips3com_tlbwr = (x86code *)mips3com_tlbwr;
	mips3.drcdata->mips3com_tlbp = (x86code *)mips3com_tlbp;

#if COMPARE_AGAINST_C
	mips3.drcdata->execute_c_version = (x86code *)execute_c_version;
#endif
}


/*-------------------------------------------------
    mips3drc_exit - clean up the drc-specific
    state
-------------------------------------------------*/

static void mips3drc_exit(void)
{
	printf("Final code size = %d\n", (int)(mips3.drc->cache_top - mips3.drc->cache_base));
#if LOG_HOTSPOTS
	/* print top hotspots */
	{
		int maxspot = 0;
		int hotnum;
		int count;

		for (count = 0; count < 10; count++)
		{
			for (hotnum = 1; hotnum < mips3.drcdata->next_hotspot; hotnum++)
				if (mips3.drcdata->hotspot[hotnum].count > mips3.drcdata->hotspot[maxspot].count)
					maxspot = hotnum;
			printf("%08X%08X hits: add_speedup(0x%08X, 0x%08X);\n",
					(UINT32)(mips3.drcdata->hotspot[maxspot].count >> 32),
					(UINT32)mips3.drcdata->hotspot[maxspot].count,
					mips3.drcdata->hotspot[maxspot].pc,
					mips3.drcdata->hotspot[maxspot].opcode);
			mips3.drcdata->hotspot[maxspot].count = 0;
		}
	}
#endif
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
		x86log_disasm_code_range(mips3.log, "entry_point:", (x86code *)drc->entry_point, drc->exit_point);
		x86log_disasm_code_range(mips3.log, "exit_point:", drc->exit_point, drc->recompile);
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
	append_handle_pc_tlb_mismatch(drc);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "handle_pc_tlb_mismatch:", mips3.drcdata->handle_pc_tlb_mismatch, drc->cache_top);

	mips3.drcdata->find_exception_handler = drc->cache_top;
	append_find_exception_handler(drc);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "find_exception_handler:", mips3.drcdata->find_exception_handler, drc->cache_top);

	mips3.drcdata->explode_ccr31 = drc->cache_top;
	append_explode_ccr31(drc);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "explode_ccr31:", mips3.drcdata->explode_ccr31, drc->cache_top);

	mips3.drcdata->recover_ccr31 = drc->cache_top;
	append_recover_ccr31(drc);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, "recover_ccr31:", mips3.drcdata->recover_ccr31, drc->cache_top);

	generate_read_write_handlers(drc, &mips3.drcdata->general, "general", 0);
	generate_read_write_handlers(drc, &mips3.drcdata->cached, "cached", ARW_CACHED);
	generate_read_write_handlers(drc, &mips3.drcdata->uncached, "uncached", ARW_UNCACHED);
}


/*------------------------------------------------------------------
    drc_recompile_callback
------------------------------------------------------------------*/

static void drc_recompile_callback(drc_core *drc)
{
	int compiled_last_sequence = FALSE;
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *desclist;
	void *start = drc->cache_top;
	int override = FALSE;
	int oobnum;

	(void)start;

	/* begin the sequence */
	drc_begin_sequence(drc, mips3.core->pc);
	mips3.drcdata->oobcount = 0;

	/* get a description of this sequence */
	desclist = drcfe_describe_code(mips3.drcfe, mips3.core->pc);
	if (LOG_CODE)
		log_opcode_desc(mips3.log, desclist, 0);

	/* loop until we get through all instruction sequences */
	for (seqhead = desclist; seqhead != NULL; seqhead = seqlast->next)
	{
		const opcode_desc *curdesc;
		compiler_state compiler;

		/* determine the last instruction in this sequence */
		for (seqlast = seqhead; seqlast != NULL; seqlast = seqlast->next)
			if (seqlast->flags & OPFLAG_END_SEQUENCE)
				break;
		assert(seqlast != NULL);

		/* add this as an entry point */
		if (drc_add_entry_point(drc, seqhead->pc, override) && !override)
		{
			/* if this is the first sequence, it is a recompile request; allow overrides */
			if (seqhead == desclist)
			{
				override = TRUE;
				drc_add_entry_point(drc, seqhead->pc, override);
			}

			/* otherwise, just emit a jump to existing code */
			else
			{
				if (compiled_last_sequence)
					drc_append_fixed_dispatcher(drc, seqhead->pc, TRUE);
				compiled_last_sequence = FALSE;
				continue;
			}
		}
		compiled_last_sequence = TRUE;

		/* add a code log entry */
		if (LOG_CODE)
			x86log_add_comment(mips3.log, drc->cache_top, "-------------------------");

		/* validate this code block if we're not pointing into ROM */
		/* note that we assume P1 still contains the PC */
		if (memory_get_write_ptr(cpu_getactivecpu(), ADDRESS_SPACE_PROGRAM, seqhead->physpc) != NULL)
		{
			if (LOG_CODE)
				x86log_add_comment(mips3.log, drc->cache_top, "[Validation for %08X]", seqhead->pc);
			emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)seqhead->opptr.v);						// mov  rax,seqhead->opptr
			for (curdesc = seqhead; curdesc != seqlast->next; curdesc = curdesc->next)
			{
				emit_cmp_m32_imm(DRCTOP, MBD(REG_RAX, curdesc->opptr.b - seqhead->opptr.b), *curdesc->opptr.l);
																								// cmp  [code],val
				emit_jcc(DRCTOP, COND_NE, mips3.drc->recompile);								// jne  recompile
			}
		}

		/* initialize the compiler state */
		compiler_state_reset(&compiler);

#if LOG_HOTSPOTS
		/* add hotspot tracking */
		{
			int hotnum;

			drc_register_code_at_cache_top(drc, seqhead->pc);

			for (hotnum = 0; hotnum < mips3.drcdata->next_hotspot; hotnum++)
				if (mips3.drcdata->hotspot[hotnum].pc == seqhead->pc && mips3.drcdata->hotspot[hotnum].opcode == *seqhead->opptr.l)
					break;
			if (hotnum == mips3.drcdata->next_hotspot)
			{
				assert(hotnum < ARRAY_LENGTH(mips3.drcdata->hotspot));
				mips3.drcdata->hotspot[hotnum].pc = seqhead->pc;
				mips3.drcdata->hotspot[hotnum].opcode = *seqhead->opptr.l;
				mips3.drcdata->hotspot[hotnum].count = 0;
				mips3.drcdata->next_hotspot++;
			}
			emit_add_m64_imm(DRCTOP, MDRC(&mips3.drcdata->hotspot[hotnum].count), 1);
		}
#endif

		/* iterate over instructions in the sequence and compile them */
		for (curdesc = seqhead; curdesc != seqlast->next; curdesc = curdesc->next)
			compile_one(drc, &compiler, curdesc);

		/* at the end of the sequence; update the PC in memory and check cycle counts */
		if (!(seqlast->flags & (OPFLAG_IS_UNCONDITIONAL_BRANCH | OPFLAG_WILL_CAUSE_EXCEPTION)))
		{
			UINT32 nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;
			emit_update_cycles_pc_and_flush(drc, &compiler, NULL, nextpc);
		}

		/* if this is a likely branch, the fall through case needs to skip the next instruction */
		if ((seqlast->flags & OPFLAG_IS_CONDITIONAL_BRANCH) && seqlast->skipslots > 0)
			if (seqlast->next != NULL && seqlast->next->pc != seqlast->pc + 8)
			{
				compiler_register_flush_all(drc, &compiler);
				drc_append_tentative_fixed_dispatcher(drc, seqlast->pc + 8, TRUE);			// jmp  <pc+8>
			}

		/* if we need a redispatch, do it now */
		if (seqlast->flags & OPFLAG_REDISPATCH)
		{
			UINT32 nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;
			compiler_register_flush_all(drc, &compiler);
 			drc_append_tentative_fixed_dispatcher(drc, nextpc, TRUE);						// jmp  <nextpc>
 		}

 		/* if we need to return to the start, do it */
		if (seqlast->flags & OPFLAG_RETURN_TO_START)
		{
			compiler_register_flush_all(drc, &compiler);
 			drc_append_tentative_fixed_dispatcher(drc, mips3.core->pc, FALSE);				// jmp  <startpc>
		}
	}

	/* end the sequence */
	drc_end_sequence(drc);

	/* run the out-of-bounds handlers */
	for (oobnum = 0; oobnum < mips3.drcdata->oobcount; oobnum++)
	{
		oob_handler *oob = &mips3.drcdata->ooblist[oobnum];
		(*oob->callback)(drc, oob, oob->param);
	}

	/* log the generated code */
	if (LOG_CODE)
	{
		char label[60];
		sprintf(label, "Code @ %08X (%08X physical)", desclist->pc, desclist->physpc);
		x86log_disasm_code_range(mips3.log, label, start, drc->cache_top);
	}
}


/*------------------------------------------------------------------
    drc_entrygen_callback
------------------------------------------------------------------*/

static void drc_entrygen_callback(drc_core *drc)
{
	append_check_interrupts(drc, NULL, NULL);
}



/***************************************************************************
    RECOMPILER CORE
***************************************************************************/

/*------------------------------------------------------------------
    compile_one
------------------------------------------------------------------*/

static void compile_one(drc_core *drc, compiler_state *compiler, const opcode_desc *desc)
{
	int hotnum;

	/* register this instruction */
	if (!(desc->flags & OPFLAG_IN_DELAY_SLOT))
		drc_register_code_at_cache_top(drc, desc->pc);

	/* add an entry for the log */
	if (LOG_CODE)
		log_add_disasm_comment(mips3.log, drc->cache_top, desc->pc, *desc->opptr.l);

	/* if we want a probe, add it here */
	if (desc->pc == PROBE_ADDRESS)
	{
		emit_mov_m32_imm(DRCTOP, PCADDR, desc->pc);											// mov  [pc],desc->pc
		compiler_register_flush_all(drc, compiler);
		emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)probe_printf);
		emit_call_r64(DRCTOP, REG_RAX);
	}

	/* if we are debugging, call the debugger */
	if (Machine->debug_mode || COMPARE_AGAINST_C)
	{
		emit_mov_m32_imm(DRCTOP, PCADDR, desc->pc);											// mov  [pc],desc->pc
		if (COMPARE_AGAINST_C && !(desc->flags & OPFLAG_IN_DELAY_SLOT))
			emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->execute_c_version));
		if (Machine->debug_mode)
			drc_append_call_debugger(drc);													// <call debugger>
	}

	/* validate our TLB entry at this PC; if we fail, we need to recompile */
	if ((desc->flags & OPFLAG_VALIDATE_TLB) && (desc->pc < 0x80000000 || desc->pc >= 0xc0000000))
	{
		emit_cmp_m32_imm(DRCTOP, MDRC(&mips3.core->tlb_table[desc->pc >> 12]), mips3.core->tlb_table[desc->pc >> 12]);
																							// cmp  tlb_table[pc>>12],<original value>
		oob_request_callback(drc, COND_NE, oob_exception_cleanup, compiler, desc, mips3.drcdata->handle_pc_tlb_mismatch);
																							// jne  handle_pc_tlb_mismatch
	}

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* is this a hotspot? */
	for (hotnum = 0; hotnum < MIPS3_MAX_HOTSPOTS; hotnum++)
		if (desc->pc == mips3.hotspot[hotnum].pc && *desc->opptr.l == mips3.hotspot[hotnum].opcode)
		{
			compiler->cycles += mips3.hotspot[hotnum].cycles;
			break;
		}

	/* if this is an invalid opcode, generate the exception now */
	if (desc->flags & OPFLAG_INVALID_OPCODE)
		oob_request_callback(drc, COND_NONE, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_invalidop_exception);

	/* otherwise, it's a regular instruction */
	else
	{
		/* compile the instruction */
		if (!compile_instruction(drc, compiler, desc))
			fatalerror("Unimplemented op %08X (%02X,%02X)", *desc->opptr.l, *desc->opptr.l >> 26, *desc->opptr.l & 0x3f);

		/* flush */
		if (FLUSH_AFTER_EACH_INSTRUCTION)
			compiler_register_flush_all(drc, compiler);
	}
}



/***************************************************************************
    OUT-OF-BOUNDS CODEGEN
***************************************************************************/

/*-------------------------------------------------
    oob_exception_cleanup - out-of-bounds code
    generator for cleaning up and generating an
    exception with the EPC pointing to the
    current instruction
-------------------------------------------------*/

static void oob_exception_cleanup(drc_core *drc, oob_handler *oob, void *param)
{
	/* if this is non-jumping error, emit a unique signature */
	if (oob->link.size == 0)
	{
		/* align to an 8-byte boundary */
		while (((UINT64)drc->cache_top & 7) != 0)
			emit_int_3(DRCTOP);

		/* logging */
		if (LOG_CODE)
		{
			x86log_add_comment(mips3.log, drc->cache_top, "[Exception path for %08X]", oob->desc->pc);
			x86log_mark_as_data(mips3.log, drc->cache_top, drc->cache_top + 15, 8);
		}

		/* output a pair of qwords */
		emit_qword(DRCTOP, EXCEPTION_ADDRESS_COOKIE);
		emit_qword(DRCTOP, (UINT64)oob->link.target);
	}

	/* otherwise, just resolve the link */
	else
	{
		resolve_link(DRCTOP, &oob->link);
		if (LOG_CODE)
			x86log_add_comment(mips3.log, drc->cache_top, "[Exception path for %08X]", oob->desc->pc);
	}

	/* adjust for cycles */
	if (oob->compiler.cycles != 0)
		emit_sub_m32_imm(DRCTOP, ICOUNTADDR, oob->compiler.cycles);							// sub  [icount],cycles

	/* update the PC with the instruction PC */
	if (!(oob->desc->flags & OPFLAG_IN_DELAY_SLOT))
		emit_mov_r32_imm(DRCTOP, REG_P1, oob->desc->pc);									// mov  p1,pc
	else
		emit_mov_r32_imm(DRCTOP, REG_P1, oob->desc->pc | 1);								// mov  p1,pc | 1

	/* flush any dirty registers */
	compiler_register_flush_all(drc, &oob->compiler);										// <flush registers>

	/* jump to the exception generator */
	emit_jmp(DRCTOP, param);																// jmp  <generate exception>
}


/*-------------------------------------------------
    oob_interrupt_cleanup - out-of-bounds code
    generator for cleaning up and generating an
    interrupt with the EPC pointing to the
    following instruction
-------------------------------------------------*/

static void oob_interrupt_cleanup(drc_core *drc, oob_handler *oob, void *param)
{
	/* resolve the link */
	resolve_link(DRCTOP, &oob->link);

	/* add a code log entry */
	if (LOG_CODE)
		x86log_add_comment(mips3.log, drc->cache_top, "[Interrupt path for %08X]", oob->desc->pc);

	/* adjust for cycles */
	if (oob->compiler.cycles != 0)
		emit_sub_m32_imm(DRCTOP, ICOUNTADDR, oob->compiler.cycles);							// sub  [icount],cycles

	/* update the PC with the following instruction PC */
	if (!(oob->desc->flags & OPFLAG_IN_DELAY_SLOT))
	{
		assert(!(oob->desc->flags & OPFLAG_IS_BRANCH));
		emit_mov_r32_imm(DRCTOP, REG_P1, oob->desc->pc + 4);								// mov  p1,pc + 4
	}
	else
	{
		assert(oob->desc->branch != NULL);
		if (oob->desc->branch->targetpc != BRANCH_TARGET_DYNAMIC)
			emit_mov_r32_imm(DRCTOP, REG_P1, oob->desc->branch->targetpc);					// mov  p1,<targetpc>
		else
			emit_mov_r32_m32(DRCTOP, REG_P1, MBD(REG_RSP, SPOFFS_NEXTPC));					// mov  p1,[rsp + nextpc]
	}

	/* flush any dirty registers */
	compiler_register_flush_all(drc, &oob->compiler);										// <flush registers>

	/* jump to the exception generator */
	emit_jmp(DRCTOP, param);																// jmp  <generate exception>
}



/***************************************************************************
    REGISTER ALLOCATION ROUTINES
***************************************************************************/

/*-------------------------------------------------
    compiler_register_set_constant - set a MIPS
    register to a constant value
-------------------------------------------------*/

static void compiler_register_set_constant(compiler_state *compiler, UINT8 mipsreg, UINT64 constval)
{
	mipsreg_state reg = compiler->mipsregs[mipsreg];

	assert((INT32)constval == (INT64)constval);

	/* if the register was previously live, free it */
	if (ISLIVE(reg))
	{
		assert(compiler->x86regs[X86REG(reg)] & X86REG_LIVE);
		compiler->x86regs[X86REG(reg)] = 0;
	}

	/* set up the constant */
	compiler->mipsregs[mipsreg] = MAKE_CONST(constval) | ((mipsreg == 0) ? 0 : MIPSREG_DIRTY);
}


/*-------------------------------------------------
    compiler_register_allocate - allocate an x86
    register to hold a MIPS register
-------------------------------------------------*/

static mipsreg_state compiler_register_allocate(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT8 mipsreg, UINT8 flags)
{
	UINT8 regindex, regnum = 0;
	UINT64 livemask = 0;

	/* register 0 is always constant */
	if (mipsreg == 0)
		return MAKE_CONST(0);

	/* first see if we are already allocated */
	if (ISLIVE(compiler->mipsregs[mipsreg]))
	{
		/* if we're not a source, make sure we are marked dirty */
		if (flags & ALLOC_DIRTY)
			compiler->mipsregs[mipsreg] |= MIPSREG_DIRTY;
		return compiler->mipsregs[mipsreg];
	}

	/* if we just want a source register, return a constant directly */
	if (flags == ALLOC_SRC && ISCONST(compiler->mipsregs[mipsreg]))
		return compiler->mipsregs[mipsreg];

	/* first look for a free entry */
	for (regindex = 0; regindex < NUM_NVREG; regindex++)
	{
		regnum = nvreg[regindex];
		if (!(compiler->x86regs[regnum] & X86REG_LIVE))
			goto allocateit;
		else
			livemask |= (UINT64)1 << (compiler->x86regs[regnum] & X86REG_MIPSMASK);
	}

	/* can't have anything we need for this instruction, however */
	livemask &= ~desc->gpr.used & ~desc->gpr.modified;
	desc = desc->next;

	/* next loop over the registers, figuring out who has the longest until being used */
	while (desc != NULL && livemask != 0)
	{
		/* if this instruction uses the last remaining register, we have to pick from among those */
		if ((livemask & ~desc->gpr.used) == 0)
			break;
		livemask &= ~desc->gpr.used;

		/* if this instruction modifies any of the registers, we want those */
		if ((livemask & desc->gpr.modified) != 0)
		{
			livemask &= desc->gpr.modified;
			break;
		}

		/* advance to the next instruction; stop if we hit end-of-sequence */
		if (desc->flags & OPFLAG_END_SEQUENCE)
			break;
		desc = desc->next;
	}

	/* pick the first one */
	for (regindex = 0; regindex < NUM_NVREG; regindex++)
	{
		regnum = nvreg[regindex];
		if (livemask & ((UINT64)1 << (compiler->x86regs[regnum] & X86REG_MIPSMASK)))
			break;
	}
	assert(regindex != NUM_NVREG);

	/* flush it */
	compiler_register_flush(drc, compiler, compiler->x86regs[regnum] & X86REG_MIPSMASK, TRUE);

allocateit:
	/* load the contents if we are a source */
	if (LOG_CODE)
		x86log_add_comment(mips3.log, drc->cache_top, "allocate r%d (%s)", mipsreg, x86regname[regnum]);
	if (flags & ALLOC_SRC)
	{
		mipsreg_state state = compiler->mipsregs[mipsreg];
		if (ISCONST(state))
			emit_mov_r64_imm(DRCTOP, regnum, CONSTVAL(state));								// mov  <x86reg>,constant
		else
			emit_mov_r64_m64(DRCTOP, regnum, REGADDR(mipsreg));								// mov  <x86reg>,[reg]
	}

	/* mark it allocated */
	assert(!(compiler->x86regs[regnum] & X86REG_LIVE));
	compiler->x86regs[regnum] = X86REG_LIVE | mipsreg;
	compiler->mipsregs[mipsreg] = MIPSREG_LIVE | regnum | ((flags & ALLOC_DIRTY) ? MIPSREG_DIRTY : 0);
	return compiler->mipsregs[mipsreg];
}


/*-------------------------------------------------
    compiler_register_flush - flush and optionally
    free a single MIPS register
-------------------------------------------------*/

static void compiler_register_flush(drc_core *drc, compiler_state *compiler, UINT8 mipsreg, int free)
{
	mipsreg_state reg = compiler->mipsregs[mipsreg];

	assert(ISCONST(reg) || ISLIVE(reg));

	/* flush the value if dirty */
	if (ISDIRTY(reg))
	{
		if (LOG_CODE)
			x86log_add_comment(mips3.log, drc->cache_top, "flush r%d (%s)", mipsreg, ISCONST(reg) ? "constant" : x86regname[X86REG(reg)]);

		/* flush a constant value */
		if (ISCONST(reg))
			emit_mov_m64_imm(DRCTOP, REGADDR(mipsreg), CONSTVAL(reg));							// mov  [reg],constant

		/* flush a live value */
		else if (ISLIVE(reg))
			emit_mov_m64_r64(DRCTOP, REGADDR(mipsreg), X86REG(reg));							// mov  [reg],<x86reg>
	}

	/* clear the dirty flag */
	if (!free)
		compiler->mipsregs[mipsreg] &= ~MIPSREG_DIRTY;
	else
	{
		if (ISLIVE(reg))
			compiler->x86regs[X86REG(reg)] = 0;
		compiler->mipsregs[mipsreg] = 0;
	}
}


/*-------------------------------------------------
    compiler_register_flush_all - flush all dirty
    registers
-------------------------------------------------*/

static void compiler_register_flush_all(drc_core *drc, compiler_state *compiler)
{
	int mipsreg;

	/* flush all dirty registers */
	for (mipsreg = 1; mipsreg < 35; mipsreg++)
		if (ISDIRTY(compiler->mipsregs[mipsreg]))
			compiler_register_flush(drc, compiler, mipsreg, FALSE);
}



/***************************************************************************
    COMPILER CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    probe_printf - print the current CPU state
    and return
-------------------------------------------------*/

static void probe_printf(void)
{
	printf(" PC=%08X          r1=%08X%08X  r2=%08X%08X  r3=%08X%08X\n",
		mips3.core->pc,
		(UINT32)(mips3.core->r[1] >> 32), (UINT32)mips3.core->r[1],
		(UINT32)(mips3.core->r[2] >> 32), (UINT32)mips3.core->r[2],
		(UINT32)(mips3.core->r[3] >> 32), (UINT32)mips3.core->r[3]);
	printf(" r4=%08X%08X  r5=%08X%08X  r6=%08X%08X  r7=%08X%08X\n",
		(UINT32)(mips3.core->r[4] >> 32), (UINT32)mips3.core->r[4],
		(UINT32)(mips3.core->r[5] >> 32), (UINT32)mips3.core->r[5],
		(UINT32)(mips3.core->r[6] >> 32), (UINT32)mips3.core->r[6],
		(UINT32)(mips3.core->r[7] >> 32), (UINT32)mips3.core->r[7]);
	printf(" r8=%08X%08X  r9=%08X%08X r10=%08X%08X r11=%08X%08X\n",
		(UINT32)(mips3.core->r[8] >> 32), (UINT32)mips3.core->r[8],
		(UINT32)(mips3.core->r[9] >> 32), (UINT32)mips3.core->r[9],
		(UINT32)(mips3.core->r[10] >> 32), (UINT32)mips3.core->r[10],
		(UINT32)(mips3.core->r[11] >> 32), (UINT32)mips3.core->r[11]);
	printf("r12=%08X%08X r13=%08X%08X r14=%08X%08X r15=%08X%08X\n",
		(UINT32)(mips3.core->r[12] >> 32), (UINT32)mips3.core->r[12],
		(UINT32)(mips3.core->r[13] >> 32), (UINT32)mips3.core->r[13],
		(UINT32)(mips3.core->r[14] >> 32), (UINT32)mips3.core->r[14],
		(UINT32)(mips3.core->r[15] >> 32), (UINT32)mips3.core->r[15]);
	printf("r16=%08X%08X r17=%08X%08X r18=%08X%08X r19=%08X%08X\n",
		(UINT32)(mips3.core->r[16] >> 32), (UINT32)mips3.core->r[16],
		(UINT32)(mips3.core->r[17] >> 32), (UINT32)mips3.core->r[17],
		(UINT32)(mips3.core->r[18] >> 32), (UINT32)mips3.core->r[18],
		(UINT32)(mips3.core->r[19] >> 32), (UINT32)mips3.core->r[19]);
	printf("r20=%08X%08X r21=%08X%08X r22=%08X%08X r23=%08X%08X\n",
		(UINT32)(mips3.core->r[20] >> 32), (UINT32)mips3.core->r[20],
		(UINT32)(mips3.core->r[21] >> 32), (UINT32)mips3.core->r[21],
		(UINT32)(mips3.core->r[22] >> 32), (UINT32)mips3.core->r[22],
		(UINT32)(mips3.core->r[23] >> 32), (UINT32)mips3.core->r[23]);
	printf("r24=%08X%08X r25=%08X%08X r26=%08X%08X r27=%08X%08X\n",
		(UINT32)(mips3.core->r[24] >> 32), (UINT32)mips3.core->r[24],
		(UINT32)(mips3.core->r[25] >> 32), (UINT32)mips3.core->r[25],
		(UINT32)(mips3.core->r[26] >> 32), (UINT32)mips3.core->r[26],
		(UINT32)(mips3.core->r[27] >> 32), (UINT32)mips3.core->r[27]);
	printf("r28=%08X%08X r29=%08X%08X r30=%08X%08X r31=%08X%08X\n",
		(UINT32)(mips3.core->r[28] >> 32), (UINT32)mips3.core->r[28],
		(UINT32)(mips3.core->r[29] >> 32), (UINT32)mips3.core->r[29],
		(UINT32)(mips3.core->r[30] >> 32), (UINT32)mips3.core->r[30],
		(UINT32)(mips3.core->r[31] >> 32), (UINT32)mips3.core->r[31]);
	printf(" hi=%08X%08X  lo=%08X%08X\n",
		(UINT32)(mips3.core->r[REG_HI] >> 32), (UINT32)mips3.core->r[REG_HI],
		(UINT32)(mips3.core->r[REG_LO] >> 32), (UINT32)mips3.core->r[REG_LO]);
}


/*-------------------------------------------------
    exception_trap - log any exceptions that
    aren't interrupts
-------------------------------------------------*/

static void exception_trap(void)
{
	if ((mips3.core->cpr[0][COP0_Cause] & 0xff) != 0)
		printf("Exception: EPC=%08X Cause=%08X BadVAddr=%08X Jmp=%08X\n", (UINT32)mips3.core->cpr[0][COP0_EPC], (UINT32)mips3.core->cpr[0][COP0_Cause], (UINT32)mips3.core->cpr[0][COP0_BadVAddr], mips3.core->pc);
}



/***************************************************************************
    COMMON ROUTINES
***************************************************************************/

/*------------------------------------------------------------------
    generate_read_write_handlers
------------------------------------------------------------------*/

static void generate_read_write_handlers(drc_core *drc, readwrite_handlers *handlers, const char *type, UINT8 flags)
{
	char buffer[100];

	handlers->read_byte_signed = drc->cache_top;
	append_readwrite_and_translate(drc, 1, ARW_READ | ARW_SIGNED | flags, offsetof(readwrite_handlers, read_byte_signed));
	sprintf(buffer, "%s.read_byte_signed:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->read_byte_signed, drc->cache_top);

	handlers->read_byte_unsigned = drc->cache_top;
	append_readwrite_and_translate(drc, 1, ARW_READ | ARW_UNSIGNED | flags, offsetof(readwrite_handlers, read_byte_unsigned));
	sprintf(buffer, "%s.read_byte_unsigned:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->read_byte_unsigned, drc->cache_top);

	handlers->read_half_signed = drc->cache_top;
	append_readwrite_and_translate(drc, 2, ARW_READ | ARW_SIGNED | flags, offsetof(readwrite_handlers, read_half_signed));
	sprintf(buffer, "%s.read_half_signed:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->read_half_signed, drc->cache_top);

	handlers->read_half_unsigned = drc->cache_top;
	append_readwrite_and_translate(drc, 2, ARW_READ | ARW_UNSIGNED | flags, offsetof(readwrite_handlers, read_half_unsigned));
	sprintf(buffer, "%s.read_half_unsigned:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->read_half_unsigned, drc->cache_top);

	handlers->read_word_signed = drc->cache_top;
	append_readwrite_and_translate(drc, 4, ARW_READ | ARW_SIGNED | flags, offsetof(readwrite_handlers, read_word_signed));
	sprintf(buffer, "%s.read_word_signed:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->read_word_signed, drc->cache_top);

	handlers->read_word_unsigned = drc->cache_top;
	append_readwrite_and_translate(drc, 4, ARW_READ | ARW_UNSIGNED | flags, offsetof(readwrite_handlers, read_word_unsigned));
	sprintf(buffer, "%s.read_word_unsigned:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->read_word_unsigned, drc->cache_top);

	handlers->read_word_masked = drc->cache_top;
	append_readwrite_and_translate(drc, 4, ARW_READ | ARW_UNSIGNED | ARW_MASKED | flags, offsetof(readwrite_handlers, read_word_masked));
	sprintf(buffer, "%s.read_word_masked:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->read_word_masked, drc->cache_top);

	handlers->read_double = drc->cache_top;
	append_readwrite_and_translate(drc, 8, ARW_READ | flags, offsetof(readwrite_handlers, read_double));
	sprintf(buffer, "%s.read_double:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->read_double, drc->cache_top);

	handlers->read_double_masked = drc->cache_top;
	append_readwrite_and_translate(drc, 8, ARW_READ | ARW_MASKED | flags, offsetof(readwrite_handlers, read_double_masked));
	sprintf(buffer, "%s.read_double_masked:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->read_double_masked, drc->cache_top);

	handlers->write_byte = drc->cache_top;
	append_readwrite_and_translate(drc, 1, ARW_WRITE | flags, offsetof(readwrite_handlers, write_byte));
	sprintf(buffer, "%s.write_byte:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->write_byte, drc->cache_top);

	handlers->write_half = drc->cache_top;
	append_readwrite_and_translate(drc, 2, ARW_WRITE | flags, offsetof(readwrite_handlers, write_half));
	sprintf(buffer, "%s.write_half:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->write_half, drc->cache_top);

	handlers->write_word = drc->cache_top;
	append_readwrite_and_translate(drc, 4, ARW_WRITE | flags, offsetof(readwrite_handlers, write_word));
	sprintf(buffer, "%s.write_word:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->write_word, drc->cache_top);

	handlers->write_word_masked = drc->cache_top;
	append_readwrite_and_translate(drc, 4, ARW_WRITE | ARW_MASKED | flags, offsetof(readwrite_handlers, write_word_masked));
	sprintf(buffer, "%s.write_word_masked:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->write_word_masked, drc->cache_top);

	handlers->write_double = drc->cache_top;
	append_readwrite_and_translate(drc, 8, ARW_WRITE | flags, offsetof(readwrite_handlers, write_double));
	sprintf(buffer, "%s.write_double:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->write_double, drc->cache_top);

	handlers->write_double_masked = drc->cache_top;
	append_readwrite_and_translate(drc, 8, ARW_WRITE | ARW_MASKED | flags, offsetof(readwrite_handlers, write_double_masked));
	sprintf(buffer, "%s.write_double_masked:", type);
	if (LOG_CODE)
		x86log_disasm_code_range(mips3.log, buffer, handlers->write_double_masked, drc->cache_top);
}


/*------------------------------------------------------------------
    append_generate_exception
------------------------------------------------------------------*/

static void append_generate_exception(drc_core *drc, UINT8 exception)
{
	UINT32 offset = (exception >= EXCEPTION_TLBMOD && exception <= EXCEPTION_TLBSTORE) ? 0x00 : 0x180;
	emit_link link1, link2;

	if (exception == EXCEPTION_TLBLOAD || exception == EXCEPTION_TLBSTORE)
	{
		/* assumes failed address is in P2 */
		emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_BadVAddr), REG_P2);							// mov  [BadVAddr],p2
		emit_and_m32_imm(DRCTOP, CPR0ADDR(COP0_EntryHi), 0x000000ff);						// and  [EntryHi],0x000000ff
		emit_and_r32_imm(DRCTOP, REG_P2, 0xffffe000);										// and  p2,0xffffe000
		emit_or_m32_r32(DRCTOP, CPR0ADDR(COP0_EntryHi), REG_P2);							// or   [EntryHi],p2
		emit_and_m32_imm(DRCTOP, CPR0ADDR(COP0_Context), 0xff800000);						// and  [Context],0xff800000
		emit_shr_r32_imm(DRCTOP, REG_P2, 9);												// shr  p2,9
		emit_or_m32_r32(DRCTOP, CPR0ADDR(COP0_Context), REG_P2);							// or   [Context],p2
	}

	/* on entry, exception PC must be in REG_P1, and low bit set if in a delay slot */
	emit_mov_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Cause));								// mov  eax,[Cause]
	emit_and_r32_imm(DRCTOP, REG_EAX, ~0x800000ff);											// and  eax,~0x800000ff
	emit_test_r32_imm(DRCTOP, REG_P1, 1);													// test p1,1
	emit_jcc_short_link(DRCTOP, COND_Z, &link1);											// jz   skip
	emit_or_r32_imm(DRCTOP, REG_EAX, 0x80000000);											// or   eax,0x80000000
	emit_sub_r32_imm(DRCTOP, REG_P1, 5);													// sub  p1,5

	resolve_link(DRCTOP, &link1);														// skip:
	emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_EPC), REG_P1);									// mov  [EPC],p1
	if (exception)
		emit_or_r32_imm(DRCTOP, REG_EAX, exception << 2);									// or   eax,exception << 2
	emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Cause), REG_EAX);								// mov  [Cause],eax
	emit_mov_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Status));								// mov  eax,[SR]
	emit_or_r32_imm(DRCTOP, REG_EAX, SR_EXL);												// or   eax,SR_EXL
	emit_test_r32_imm(DRCTOP, REG_EAX, SR_BEV);												// test eax,SR_BEV
	emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Status), REG_EAX);								// mov  [SR],eax
	emit_mov_r32_imm(DRCTOP, REG_P1, 0xbfc00200 + offset);									// mov  p1,0xbfc00200+offset
	emit_jcc_short_link(DRCTOP, COND_NZ, &link2);											// jnz  skip2
	emit_mov_r32_imm(DRCTOP, REG_P1, 0x80000000 + offset);									// mov  p1,0x80000000+offset

	resolve_link(DRCTOP, &link2);														// skip2:
	if (PRINTF_EXCEPTIONS)
	{
		emit_mov_m32_r32(DRCTOP, PCADDR, REG_P1);											// mov  [PC],p1
		emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)exception_trap);							// mov  rax,exception_trap
		emit_call_r64(DRCTOP, REG_RAX);														// call rax
		emit_mov_r32_m32(DRCTOP, REG_P1, PCADDR);											// mov  p1,[PC]
	}
	drc_append_dispatcher(drc);																// dispatch
}


/*------------------------------------------------------------------
    append_handle_pc_tlb_mismatch
------------------------------------------------------------------*/

static void append_handle_pc_tlb_mismatch(drc_core *drc)
{
#if 0
	emit_mov_r32_r32(DRCTOP, REG_EAX, REG_R15D);											// mov  eax,edi
	emit_shr_r32_imm(DRCTOP, REG_EAX, 12);													// shr  eax,12
	emit_mov_r32_m32(DRCTOP, REG_EDX, MISD(REG_EAX, 4, &mips3.core->tlb_table));			// mov  edx,tlb_table[eax*4]
	emit_test_r32_imm(DRCTOP, REG_EDX, 2);													// test edx,2
	emit_mov_r32_r32(DRCTOP, REG_EAX, REG_R15D);											// mov  eax,edi
	emit_jcc(DRCTOP, COND_NZ, mips3.drcdata->generate_tlbload_exception);					// jnz  generate_tlbload_exception
	emit_mov_m32bd_r32(DRCTOP, drc->pcptr, REG_R15D);										// mov  [pcptr],edi
	emit_jmp(DRCTOP, drc->recompile);														// call recompile
#endif
}


/*------------------------------------------------------------------
    append_find_exception_handler
------------------------------------------------------------------*/

static void append_find_exception_handler(drc_core *drc)
{
	x86code *loop;

	emit_mov_r64_m64(DRCTOP, REG_RAX, MBD(REG_RSP, 0));										// mov  rax,[rsp]
	emit_mov_r64_imm(DRCTOP, REG_V5, EXCEPTION_ADDRESS_COOKIE);								// mov  v5,EXCEPTION_ADDRESS_COOKIE
	emit_mov_r64_r64(DRCTOP, REG_V6, REG_RAX);												// mov  v6,rax
	emit_and_r64_imm(DRCTOP, REG_RAX, ~7);													// and  rax,~7
	emit_add_r64_imm(DRCTOP, REG_RSP, 8);													// add  rsp,8
	emit_add_r64_imm(DRCTOP, REG_RAX, 8);													// add  rax,8
	loop = drc->cache_top;																// loop:
	emit_cmp_m64_r64(DRCTOP, MBD(REG_RAX, 0), REG_V5);										// cmp  [rax],v5
	emit_lea_r64_m64(DRCTOP, REG_RAX, MBD(REG_RAX, 8));										// lea  rax,[rax+8]
	emit_jcc(DRCTOP, COND_NE, loop);														// jne  loop
	emit_cmp_m64_r64(DRCTOP, MBD(REG_RAX, 0), REG_V6);										// cmp  [rax],v6
	emit_lea_r64_m64(DRCTOP, REG_RAX, MBD(REG_RAX, 8));										// lea  rax,[rax+8]
	emit_jcc(DRCTOP, COND_NE, loop);														// jne  loop
	emit_jmp_r64(DRCTOP, REG_RAX);															// jmp  rax
}


/*------------------------------------------------------------------
    append_direct_ram_read
------------------------------------------------------------------*/

static void append_direct_ram_read(drc_core *drc, int size, UINT8 flags, void *rambase, offs_t ramstart)
{
	void *adjbase = (UINT8 *)rambase - ramstart;
	INT64 adjdisp = (UINT8 *)adjbase - (UINT8 *)drc->baseptr;
	int raxrel = ((INT32)adjdisp != adjdisp);

	/* if we can't fit the delta to adjbase within an INT32, load RAX with the full address here */
	if (raxrel)
		emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)adjbase);

	/* fast RAM byte read */
	if (size == 1)
	{
		/* adjust for endianness */
		if (mips3.core->bigendian)
			emit_xor_r32_imm(DRCTOP, REG_P1, 3);											// xor   p1,3

		/* read rax-relative */
		if (raxrel)
		{
			if (flags & ARW_SIGNED)
				emit_movsx_r64_m8(DRCTOP, REG_RAX, MBISD(REG_RAX, REG_P1, 1, 0));			// movsx rax,byte ptr [p1 + rax]
			else
				emit_movzx_r64_m8(DRCTOP, REG_RAX, MBISD(REG_RAX, REG_P1, 1, 0));			// movzx rax,byte ptr [p1 + rax]
		}

		/* read drc-relative */
		else
		{
			if (flags & ARW_SIGNED)
				emit_movsx_r64_m8(DRCTOP, REG_RAX, MDRCISD(adjbase, REG_P1, 1, 0));			// movsx rax,byte ptr [p1 + adjbase]
			else
				emit_movzx_r64_m8(DRCTOP, REG_RAX, MDRCISD(adjbase, REG_P1, 1, 0));			// movzx rax,byte ptr [p1 + adjbase]
		}
	}

	/* fast RAM halfword read */
	else if (size == 2)
	{
		/* adjust for endianness */
		if (mips3.core->bigendian)
			emit_xor_r32_imm(DRCTOP, REG_P1, 2);											// xor   p1,2

		/* read rax-relative */
		if (raxrel)
		{
			if (flags & ARW_SIGNED)
				emit_movsx_r64_m16(DRCTOP, REG_RAX, MBISD(REG_RAX, REG_P1, 1, 0));			// movsx rax,word ptr [p1 + rax]
			else
				emit_movzx_r64_m16(DRCTOP, REG_EAX, MBISD(REG_RAX, REG_P1, 1, 0));			// movzx rax,word ptr [p1 + rax]
		}

		/* read drc-relative */
		else
		{
			if (flags & ARW_SIGNED)
				emit_movsx_r64_m16(DRCTOP, REG_RAX, MDRCISD(adjbase, REG_P1, 1, 0));		// movsx rax,word ptr [p1 + adjbase]
			else
				emit_movzx_r64_m16(DRCTOP, REG_EAX, MDRCISD(adjbase, REG_P1, 1, 0));		// movzx rax,word ptr [p1 + adjbase]
		}
	}

	/* fast RAM dword read */
	else if (size == 4)
	{
		/* read rax-relative */
		if (raxrel)
		{
			if (flags & ARW_SIGNED)
				emit_movsxd_r64_m32(DRCTOP, REG_RAX, MBISD(REG_RAX, REG_P1, 1, 0));			// movsxd rax,dword ptr [p1 + rax]
			else
				emit_mov_r32_m32(DRCTOP, REG_EAX, MBISD(REG_RAX, REG_P1, 1, 0));			// mov   eax,word ptr [p1 + rax]
		}

		/* read drc-relative */
		else
		{
			if (flags & ARW_SIGNED)
				emit_movsxd_r64_m32(DRCTOP, REG_RAX, MDRCISD(adjbase, REG_P1, 1, 0));		// movsxd rax,dword ptr [p1 + adjbase]
			else
				emit_mov_r32_m32(DRCTOP, REG_EAX, MDRCISD(adjbase, REG_P1, 1, 0));			// mov   eax,word ptr [p1 + adjbase]
		}
	}

	/* fast RAM qword read */
	else if (size == 8)
	{
		/* read rax-relative */
		if (raxrel)
			emit_mov_r64_m64(DRCTOP, REG_RAX, MBISD(REG_RAX, REG_P1, 1, 0));				// mov   rax,[p1 + rax]

		/* read drc-relative */
		else
			emit_mov_r64_m64(DRCTOP, REG_RAX, MDRCISD(adjbase, REG_P1, 1, 0));				// mov   rax,[p1 + adjbase]

		/* adjust for endianness */
		if (mips3.core->bigendian)
			emit_ror_r64_imm(DRCTOP, REG_RAX, 32);											// ror   rax,32
	}
}


/*------------------------------------------------------------------
    append_direct_ram_write
------------------------------------------------------------------*/

static void append_direct_ram_write(drc_core *drc, int size, UINT8 flags, void *rambase, offs_t ramstart)
{
	void *adjbase = (UINT8 *)rambase - ramstart;
	INT64 adjdisp = (UINT8 *)adjbase - (UINT8 *)drc->baseptr;
	int raxrel = ((INT32)adjdisp != adjdisp);

	/* if we can't fit the delta to adjbase within an INT32, load RAX with the full address here */
	if (raxrel)
		emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)adjbase);

	/* fast RAM byte write */
	if (size == 1)
	{
		/* adjust for endianness */
		if (mips3.core->bigendian)
			emit_xor_r32_imm(DRCTOP, REG_P1, 3);											// xor   p1,3

		/* write either rax-relative or drc-relative */
		if (raxrel)
			emit_mov_m8_r8(DRCTOP, MBISD(REG_RAX, REG_P1, 1, 0), REG_P2);					// mov   [p1 + rax],p2
		else
			emit_mov_m8_r8(DRCTOP, MDRCISD(adjbase, REG_P1, 1, 0), REG_P2);					// mov   [p1 + adjbase],p2
	}

	/* fast RAM halfword write */
	else if (size == 2)
	{
		/* adjust for endianness */
		if (mips3.core->bigendian)
			emit_xor_r32_imm(DRCTOP, REG_P1, 2);											// xor   p1,2

		/* write either rax-relative or drc-relative */
		if (raxrel)
			emit_mov_m16_r16(DRCTOP, MBISD(REG_RAX, REG_P1, 1, 0), REG_P2);					// mov   [p1 + rax],p2
		else
			emit_mov_m16_r16(DRCTOP, MDRCISD(adjbase, REG_P1, 1, 0), REG_P2);				// mov   [p1 + adjbase],p2
	}

	/* fast RAM dword write */
	else if (size == 4)
	{
		/* unmasked */
		if (!(flags & ARW_MASKED))
		{
			if (raxrel)
				emit_mov_m32_r32(DRCTOP, MBISD(REG_RAX, REG_P1, 1, 0), REG_P2);				// mov   [p1 + rax],p2
			else
				emit_mov_m32_r32(DRCTOP, MDRCISD(adjbase, REG_P1, 1, 0), REG_P2);			// mov   [p1 + adjbase],p2
		}

		/* masked */
		else
		{
			emit_mov_r32_r32(DRCTOP, REG_P4, REG_P3);										// mov   p4,p3
			if (raxrel)
				emit_and_r32_m32(DRCTOP, REG_P3, MBISD(REG_RAX, REG_P1, 1, 0));				// and   p3,[p1 + rax]
			else
				emit_and_r32_m32(DRCTOP, REG_P3, MDRCISD(adjbase, REG_P1, 1, 0));			// and   p3,[p1 + adjbase]
			emit_not_r32(DRCTOP, REG_P4);													// not   p4
			emit_and_r32_r32(DRCTOP, REG_P4, REG_P2);										// and   p4,p2
			emit_or_r32_r32(DRCTOP, REG_P4, REG_P3);										// or    p4,p3
			if (raxrel)
				emit_mov_m32_r32(DRCTOP, MBISD(REG_RAX, REG_P1, 1, 0), REG_P4);				// mov   [p1 + rax],p4
			else
				emit_mov_m32_r32(DRCTOP, MDRCISD(adjbase, REG_P1, 1, 0), REG_P4);			// mov   [p1 + adjbase],p4
		}
	}

	/* fast RAM qword write */
	else if (size == 8)
	{
		/* adjust for endianness */
		if (mips3.core->bigendian)
			emit_ror_r64_imm(DRCTOP, REG_P2, 32);											// ror  p2,32

		/* unmasked */
		if (!(flags & ARW_MASKED))
		{
			if (raxrel)
				emit_mov_m64_r64(DRCTOP, MBISD(REG_RAX, REG_P1, 1, 0), REG_P2);				// mov   [p1 + rax],p2
			else
				emit_mov_m64_r64(DRCTOP, MDRCISD(adjbase, REG_P1, 1, 0), REG_P2);			// mov   [p1 + adjbase],p2
		}

		/* masked */
		else
		{
			if (mips3.core->bigendian)
				emit_ror_r64_imm(DRCTOP, REG_P3, 32);										// ror   p3,32
			emit_mov_r64_r64(DRCTOP, REG_P4, REG_P3);										// mov   p4,p3
			if (raxrel)
				emit_and_r64_m64(DRCTOP, REG_P3, MBISD(REG_RAX, REG_P1, 1, 0));				// and   p3,[p1 + rax]
			else
				emit_and_r64_m64(DRCTOP, REG_P3, MDRCISD(adjbase, REG_P1, 1, 0));			// and   p3,[p1 + adjbase]
			emit_not_r64(DRCTOP, REG_P4);													// not   p4
			emit_and_r64_r64(DRCTOP, REG_P4, REG_P2);										// and   p4,p2
			emit_or_r64_r64(DRCTOP, REG_P4, REG_P3);										// or    p4,p3
			if (raxrel)
				emit_mov_m64_r64(DRCTOP, MBISD(REG_RAX, REG_P1, 1, 0), REG_P4);				// mov   [p1 + rax],p4
			else
				emit_mov_m64_r64(DRCTOP, MDRCISD(adjbase, REG_P1, 1, 0), REG_P4);			// mov   [p1 + adjbase],p4
		}
	}
}


/*------------------------------------------------------------------
    append_readwrite_and_translate
------------------------------------------------------------------*/

static void append_readwrite_and_translate(drc_core *drc, int size, UINT8 flags, UINT32 ptroffs)
{
	UINT32 base = ((flags & ARW_CACHED) ? 0x80000000 : (flags & ARW_UNCACHED) ? 0xa0000000 : 0);
	emit_link page_fault = { 0 }, handle_kernel = { 0 }, wrong_handler = { 0 };
	x86code *continue_mapped = NULL;
	int ramnum;

	/* if the base is 0, this is a general case which may be paged; perform the translation first */
	/* the result of the translation produces the physical address inline in P1 */
	/* if a page fault occurs, we have to keep an original copy of the address in EAX */
	/* note that we use RAX and P4 as scratch registers */
	if (base == 0)
	{
		emit_test_r32_r32(DRCTOP, REG_P1, REG_P1);											// test p1,p1
		emit_mov_r32_r32(DRCTOP, REG_P4, REG_P1);											// mov  p4,p1d
		emit_jcc_near_link(DRCTOP, COND_S, &handle_kernel);									// js   handle_kernel
		continue_mapped = drc->cache_top;												// continue_mapped:
		emit_mov_r32_r32(DRCTOP, REG_V5, REG_P1);											// mov  v5,p1
		emit_shr_r32_imm(DRCTOP, REG_P1, 12);												// shr  p1,12
		emit_mov_r32_m32(DRCTOP, REG_P1, MDRCISD(mips3.core->tlb_table, REG_P1, 4, 0));		// mov  p1,tlb_table[p1*4]
		emit_and_r32_imm(DRCTOP, REG_P4, 0xfff);											// and  p4,0xfff
		emit_shr_r32_imm(DRCTOP, REG_P1, (flags & ARW_WRITE) ? 1 : 2);						// shr  p1,2/1 (read/write)
		emit_lea_r32_m32(DRCTOP, REG_P1, MBISD(REG_P4, REG_P1, (flags & ARW_WRITE) ? 2 : 4, 0));
																							// lea  p1,[p4 + p1 * 4/2 (read/write)]
		emit_jcc_near_link(DRCTOP, COND_C, &page_fault);									// jc   page_fault
	}

	/* if the base is non-zero, this is a cached/uncached direct-mapped case; no translation necessary */
	/* just subtract the base from the address to get the physcial address */
	else
		emit_sub_r32_imm(DRCTOP, REG_P1, base);												// sub  p1,base

	/* iterate over all fast RAM ranges looking for matches that can be handled directly */
	for (ramnum = 0; ramnum < MIPS3_MAX_FASTRAM; ramnum++)
	{
		const fast_ram_info *raminfo = &mips3.fastram[ramnum];
		if (!COMPARE_AGAINST_C && !Machine->debug_mode && raminfo->base != NULL && (!(flags & ARW_WRITE) || !raminfo->readonly))
		{
			emit_link notram1 = { 0 }, notram2 = { 0 };

			/* if we have an end, check it first */
			if (raminfo->end != 0xffffffff)
			{
				emit_cmp_r32_imm(DRCTOP, REG_P1, raminfo->end);								// cmp  p1,fastram_end
				emit_jcc_short_link(DRCTOP, COND_A, &notram1);								// ja   notram1
			}

			/* if we have a non-0 start, check it next */
			if (raminfo->start != 0x00000000)
			{
				emit_cmp_r32_imm(DRCTOP, REG_P1, raminfo->start);							// cmp  p1,fastram_start
				emit_jcc_short_link(DRCTOP, COND_B, &notram2);								// jb   notram2
			}

			/* handle a fast RAM read */
			if (!(flags & ARW_WRITE))
			{
				append_direct_ram_read(drc, size, flags, raminfo->base, raminfo->start);	// <direct read>
				emit_ret(DRCTOP);															// ret
			}

			/* handle a fast RAM write */
			else
			{
				append_direct_ram_write(drc, size, flags, raminfo->base, raminfo->start);	// <direct write>
				emit_ret(DRCTOP);															// ret
			}

			/* resolve links that skipped over us */
			if (raminfo->end != 0xffffffff)
				resolve_link(DRCTOP, &notram1);											// notram1:
			if (raminfo->start != 0x00000000)
				resolve_link(DRCTOP, &notram2);											// notram2:
		}
	}

	/* failed to find it in fast RAM; make sure we are within our expected range */
	if (base != 0)
	{
		emit_cmp_r32_imm(DRCTOP, REG_P1, 0x20000000);										// cmp  p1,0x20000000
		emit_jcc_short_link(DRCTOP, COND_AE, &wrong_handler);								// jae  wrong_handler
	}

	/* handle a write */
	if (flags & ARW_WRITE)
	{
		if (size == 1)
			emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.writebyte));						// jmp  writebyte
		else if (size == 2)
			emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.writehalf));						// jmp  writehalf
		else if (size == 4)
		{
			if (!(flags & ARW_MASKED))
				emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.writeword));					// jmp  writeword
			else
				emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.writeword_masked));			// jmp  writeword_masked
		}
		else
		{
			if (!(flags & ARW_MASKED))
				emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.writedouble));				// jmp  writedouble
			else
				emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.writedouble_masked));			// jmp  writedouble_masked
		}
	}

	/* handle a read */
	else
	{
		if (size == 1)
		{
			emit_sub_r64_imm(DRCTOP, REG_RSP, 0x28);										// sub   rsp,0x28
			emit_call_m64(DRCTOP, MDRC(&mips3.core->memory.readbyte));						// call  readbyte
			if (flags & ARW_SIGNED)
				emit_movsx_r64_r8(DRCTOP, REG_RAX, REG_AL);									// movsx rax,al
			else
				emit_movzx_r64_r8(DRCTOP, REG_RAX, REG_AL);									// movzx rax,al
			emit_add_r64_imm(DRCTOP, REG_RSP, 0x28);										// add   rsp,0x28
			emit_ret(DRCTOP);																// ret
		}
		else if (size == 2)
		{
			emit_sub_r64_imm(DRCTOP, REG_RSP, 0x28);										// sub   rsp,0x28
			emit_call_m64(DRCTOP, MDRC(&mips3.core->memory.readhalf));						// call  readhalf
			if (flags & ARW_SIGNED)
				emit_movsx_r64_r16(DRCTOP, REG_RAX, REG_AX);								// movsx rax,ax
			else
				emit_movzx_r64_r16(DRCTOP, REG_RAX, REG_AX);								// movzx rax,ax
			emit_add_r64_imm(DRCTOP, REG_RSP, 0x28);										// add   rsp,0x28
			emit_ret(DRCTOP);																// ret
		}
		else if (size == 4)
		{
			if (!(flags & ARW_MASKED))
			{
				if (!(flags & ARW_SIGNED))
					emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.readword));				// jmp  readword
				else
				{
					emit_sub_r64_imm(DRCTOP, REG_RSP, 0x28);								// sub  rsp,0x28
					emit_call_m64(DRCTOP, MDRC(&mips3.core->memory.readword));				// call readword
					emit_movsxd_r64_r32(DRCTOP, REG_RAX, REG_EAX);							// movsxd rax,eax
					emit_add_r64_imm(DRCTOP, REG_RSP, 0x28);								// add  rsp,0x28
					emit_ret(DRCTOP);														// ret
				}
			}
			else
				emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.readword_masked));			// jmp  readword_masked
		}
		else
		{
			if (!(flags & ARW_MASKED))
				emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.readdouble));					// jmp  readdouble
			else
				emit_jmp_m64(DRCTOP, MDRC(&mips3.core->memory.readdouble_masked));			// jmp  readdouble_masked
		}
	}

	/* handle wrong handlers */
	if (base == 0)
	{
		resolve_link(DRCTOP, &handle_kernel);											// handle_kernel:
		emit_cmp_r32_imm(DRCTOP, REG_P1, 0xc0000000);										// cmp  p1,0xc0000000
		emit_jcc(DRCTOP, COND_AE, continue_mapped);											// jae  continue_mapped
		emit_test_r32_imm(DRCTOP, REG_P1, 0x20000000);										// test p1,0x20000000
		emit_cmovcc_r64_m64(DRCTOP, COND_Z, REG_RAX, MDRCD(&mips3.drcdata->cached, ptroffs));
																							// mov  rax,<cached version>
		emit_cmovcc_r64_m64(DRCTOP, COND_NZ, REG_RAX, MDRCD(&mips3.drcdata->uncached, ptroffs));
																							// mov  rax,<uncached version>
		emit_mov_r64_m64(DRCTOP, REG_P4, MBD(REG_RSP, 0));									// mov  p4,[rsp]
		emit_push_r64(DRCTOP, REG_RAX);														// push rax
		emit_sub_r64_r64(DRCTOP, REG_RAX, REG_P4);											// sub  rax,p4
		emit_mov_m32_r32(DRCTOP, MBD(REG_P4, -4), REG_EAX);									// mov  [p4-4],eax
		emit_ret(DRCTOP);																	// ret
	}
	else
	{
		resolve_link(DRCTOP, &wrong_handler);											// wrong_handler:
		emit_mov_r64_m64(DRCTOP, REG_RAX, MDRCD(&mips3.drcdata->general, ptroffs));
																							// mov  rax,<general version>
		emit_mov_r64_m64(DRCTOP, REG_P4, MBD(REG_RSP, 0));									// mov  p4,[rsp]
		emit_push_r64(DRCTOP, REG_RAX);														// push rax
		emit_sub_r64_r64(DRCTOP, REG_RAX, REG_P4);											// sub  rax,p4
		emit_mov_m32_r32(DRCTOP, MBD(REG_P4, -4), REG_EAX);									// mov  [p4-4],eax
		emit_add_r32_imm(DRCTOP, REG_P1, base);												// add  p1,base
		emit_ret(DRCTOP);																	// ret
	}

	/* handle page faults */
	if (base == 0)
	{
		resolve_link(DRCTOP, &page_fault);												// page_fault:
		emit_mov_r32_r32(DRCTOP, REG_P2, REG_V5);											// mov  p2,v5
		emit_jmp(DRCTOP, mips3.drcdata->find_exception_handler);							// jmp  find_exception_handler
	}
}


/*------------------------------------------------------------------
    append_explode_ccr31
------------------------------------------------------------------*/

static void append_explode_ccr31(drc_core *drc)
{
	emit_link link1;
	int i;

	/* new CCR31 value in EAX */
	emit_test_r32_imm(DRCTOP, REG_EAX, 1 << 23);											// test eax,1 << 23
	emit_mov_m32_r32(DRCTOP, CCR1ADDR(31), REG_EAX);										// mov  ccr[31],eax
	emit_setcc_m8(DRCTOP, COND_NZ, CF1ADDR(0));												// setnz fcc[0]
	if (mips3.core->flavor >= MIPS3_TYPE_MIPS_IV)
		for (i = 1; i <= 7; i++)
		{
			emit_test_r32_imm(DRCTOP, REG_EAX, 1 << (24 + i));								// test eax,1 << (24+i)
			emit_setcc_m8(DRCTOP, COND_NZ, CF1ADDR(i));										// setnz fcc[i]
		}
	emit_and_r32_imm(DRCTOP, REG_EAX, 3);													// and  eax,3
	emit_test_r32_imm(DRCTOP, REG_EAX, 1);													// test eax,1
	emit_jcc_near_link(DRCTOP, COND_Z, &link1);												// jz   skip
	emit_xor_r32_imm(DRCTOP, REG_EAX, 2);													// xor  eax,2
	resolve_link(DRCTOP, &link1);														// skip:
	drc_append_set_sse_rounding(drc, REG_EAX);												// set_rounding(EAX)
	emit_ret(DRCTOP);																		// ret
}


/*------------------------------------------------------------------
    append_recover_ccr31
------------------------------------------------------------------*/

static void append_recover_ccr31(drc_core *drc)
{
	int i;

	emit_movsxd_r64_m32(DRCTOP, REG_RAX, CCR1ADDR(31));										// movsxd rax,ccr1[rd]
	emit_and_r64_imm(DRCTOP, REG_RAX, ~0xfe800000);											// and  rax,~0xfe800000
	emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);												// xor  edx,edx
	emit_cmp_m8_imm(DRCTOP, CF1ADDR(0), 0);													// cmp  fcc[0],0
	emit_setcc_r8(DRCTOP, COND_NZ, REG_DL);													// setnz dl
	emit_shl_r32_imm(DRCTOP, REG_EDX, 23);													// shl  edx,23
	emit_or_r64_r64(DRCTOP, REG_RAX, REG_RDX);												// or   rax,rdx
	if (mips3.core->flavor >= MIPS3_TYPE_MIPS_IV)
		for (i = 1; i <= 7; i++)
		{
			emit_cmp_m8_imm(DRCTOP, CF1ADDR(i), 0);											// cmp  fcc[i],0
			emit_setcc_r8(DRCTOP, COND_NZ, REG_DL);											// setnz dl
			emit_shl_r32_imm(DRCTOP, REG_EDX, 24+i);										// shl  edx,24+i
			emit_or_r64_r64(DRCTOP, REG_RAX, REG_RDX);										// or   rax,rdx
		}
	emit_ret(DRCTOP);																		// ret
}


/*------------------------------------------------------------------
    append_check_interrupts
------------------------------------------------------------------*/

static void append_check_interrupts(drc_core *drc, compiler_state *compiler, const opcode_desc *desc)
{
	emit_link link1, link2, link3 = { 0 };

	emit_mov_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Cause));								// mov  eax,[Cause]
	emit_and_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Status));								// and  eax,[Status]
	emit_and_r32_imm(DRCTOP, REG_EAX, 0xfc00);												// and  eax,0xfc00
	emit_jcc_short_link(DRCTOP, COND_Z, &link1);											// jz   skip
	emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_IE);								// test [Status],SR_IE
	emit_jcc_short_link(DRCTOP, COND_Z, &link2);											// jz   skip
	emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_EXL | SR_ERL);						// test [Status],SR_EXL | SR_ERL
	if (desc == NULL)
	{
		emit_mov_r32_m32(DRCTOP, REG_P1, MDRC(drc->pcptr));									// mov  p1,[pc]
		emit_jcc_short_link(DRCTOP, COND_NZ, &link3);										// jnz  skip
		emit_jmp_m64(DRCTOP, MDRC(&mips3.drcdata->generate_interrupt_exception));			// jmp  generate_interrupt_exception
	}
	else
		oob_request_callback(drc, COND_Z, oob_interrupt_cleanup, compiler, desc, mips3.drcdata->generate_interrupt_exception);
	resolve_link(DRCTOP, &link1);														// skip:
	resolve_link(DRCTOP, &link2);
	if (desc == NULL)
		resolve_link(DRCTOP, &link3);
}



/***************************************************************************
    RECOMPILATION HELPERS
***************************************************************************/

/*-------------------------------------------------
    emit_load_register_value - load a register
    either as a constant or as a register
-------------------------------------------------*/

static void emit_load_register_value(drc_core *drc, compiler_state *compiler, UINT8 dstreg, mipsreg_state regstate)
{
	/* must be live or constant */
	assert(ISCONST(regstate) || ISLIVE(regstate));

	/* if the register is constant, do an immediate load */
	if (ISCONST(regstate))
	{
		INT32 mipsval = CONSTVAL(regstate);
		if (mipsval != 0)
			emit_mov_r64_imm(DRCTOP, dstreg, mipsval);
		else
			emit_xor_r32_r32(DRCTOP, dstreg, dstreg);
	}

	/* otherwise, load from the allocated register, but only if different */
	else
	{
		UINT8 srcreg = X86REG(regstate);
		if (dstreg != srcreg)
			emit_mov_r64_r64(DRCTOP, dstreg, srcreg);
	}
}


/*-------------------------------------------------
    emit_set_constant_value - set a constant
    value; if it is too large, allocate a
    register and populate it
-------------------------------------------------*/

static void emit_set_constant_value(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT8 mipsreg, UINT64 constval)
{
	/* ignore reigster 0 destination */
	if (mipsreg == 0)
		return;

	/* if we fit in 32-bits, just continue the constant charade */
	if ((INT32)constval == (INT64)constval)
		compiler_register_set_constant(compiler, mipsreg, constval);

	/* otherwise, allocate a new register and move the result into it */
	else
	{
		mipsreg_state resultreg = compiler_register_allocate(drc, compiler, desc, mipsreg, ALLOC_DST | ALLOC_DIRTY);
		emit_mov_r64_imm(DRCTOP, X86REG(resultreg), constval);								// mov  <rt>,result
	}
}


/*------------------------------------------------------------------
    emit_flush_cycles_before_instruction
------------------------------------------------------------------*/

static void emit_flush_cycles_before_instruction(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT32 cycles_to_leave)
{
	/* if we have cycles, subtract them now */
	if (compiler->cycles - cycles_to_leave != 0)
		emit_sub_m32_imm(DRCTOP, ICOUNTADDR, compiler->cycles - cycles_to_leave);			// sub  [icount],cycles

	/* clear out compiler counts */
	compiler->cycles = cycles_to_leave;
}


/*------------------------------------------------------------------
    emit_update_cycles_pc_and_flush
------------------------------------------------------------------*/

static void emit_update_cycles_pc_and_flush(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, offs_t destpc)
{
	/* if we have cycles, subtract them now */
	emit_sub_m32_imm(DRCTOP, ICOUNTADDR, compiler->cycles);									// sub  [icount],cycles

	/* clear out compiler counts */
	compiler->cycles = 0;

	/* if we have an opcode descriptor, make this an oob */
	if (desc != NULL)
		oob_request_callback(drc, COND_S, oob_interrupt_cleanup, compiler, desc, mips3.drc->exit_point);

	/* otherwise, do it inline */
	else
	{
		/* load the target PC into P1 */
		if (destpc != BRANCH_TARGET_DYNAMIC)
			emit_mov_r32_imm(DRCTOP, REG_P1, destpc);										// mov  p1,destpc
		else
			emit_mov_r32_m32(DRCTOP, REG_P1, MBD(REG_RSP, SPOFFS_NEXTPC));					// mov  p1,[esp+nextpc]

		/* flush any live registers */
		compiler_register_flush_all(drc, compiler);

		/* if we subtracted cycles and went negative, time to exit */
		emit_jcc(DRCTOP, COND_S, mips3.drc->exit_point);									// js   exit_point
	}
}


/*------------------------------------------------------------------
    emit_delay_slot_and_branch
------------------------------------------------------------------*/

static void emit_delay_slot_and_branch(drc_core *drc, const compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg)
{
	compiler_state compiler_temp = *compiler;

	/* set the link if needed */
	if (linkreg != 0)
		compiler_register_set_constant(&compiler_temp, linkreg, (INT32)(desc->pc + 8));		// <link> = pc + 8

	/* compile the delay slot using temporary compiler state */
	assert(desc->delay != NULL);
	compile_one(drc, &compiler_temp, desc->delay);											// <next instruction>

	/* update the cycles and load the new PC in case we need to stop here */
	emit_update_cycles_pc_and_flush(drc, &compiler_temp, NULL, desc->targetpc);

	/* otherwise, append a dispatcher */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
		drc_append_tentative_fixed_dispatcher(drc, desc->targetpc, FALSE);					// <redispatch>
	else
		drc_append_dispatcher(drc);															// <redispatch>
}


/*------------------------------------------------------------------
    emit_fixed_byte_read
------------------------------------------------------------------*/

static int emit_fixed_byte_read(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT8 destreg, int is_unsigned, offs_t address)
{
	void *fastram = fastram_ptr(address, 1, FALSE);

	/* if we can't do fast RAM, we need to make the call */
	if (fastram == NULL)
	{
		emit_mov_r32_imm(DRCTOP, REG_P1, address);											// mov  p1,address
		if (is_unsigned)
			emit_call(DRCTOP, mips3.drcdata->general.read_byte_unsigned);					// call read_byte_unsigned
		else
			emit_call(DRCTOP, mips3.drcdata->general.read_byte_signed);						// call read_byte_signed

		/* handle exceptions */
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbload_exception);
		if (destreg != REG_RAX)
			emit_mov_r64_r64(DRCTOP, destreg, REG_RAX);										// mov  <rt>,rax
	}

	/* if we can do fastram, emit the right form */
	else
	{
		INT64 fastdisp = (UINT8 *)fastram - (UINT8 *)drc->baseptr;
		int raxrel = ((INT32)fastdisp != fastdisp);

		/* if we can't fit the delta to fastram within an INT32, load RAX with the full address here */
		if (raxrel)
		{
			emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);								// mov  rax,fastram
			if (is_unsigned)
				emit_movzx_r64_m8(DRCTOP, destreg, MBD(REG_RAX, 0));						// movzx destreg,byte ptr [rax]
			else
				emit_movsx_r64_m8(DRCTOP, destreg, MBD(REG_RAX, 0));						// movsx destreg,byte ptr [rax]
		}

		/* read drc-relative */
		else
		{
			if (is_unsigned)
				emit_movzx_r64_m8(DRCTOP, destreg, MDRC(fastram));							// movzx destreg,byte ptr [p1 + adjbase]
			else
				emit_movsx_r64_m8(DRCTOP, destreg, MDRC(fastram));							// movsx destreg,byte ptr [p1 + adjbase]
		}
	}
	return (fastram != NULL);
}


/*------------------------------------------------------------------
    emit_fixed_half_read
------------------------------------------------------------------*/

static int emit_fixed_half_read(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT8 destreg, int is_unsigned, offs_t address)
{
	void *fastram = fastram_ptr(address & ~1, 2, FALSE);

	/* if we can't do fast RAM, we need to make the call */
	if (fastram == NULL)
	{
		emit_mov_r32_imm(DRCTOP, REG_P1, address & ~1);										// mov  p1,address & ~1
		if (is_unsigned)
			emit_call(DRCTOP, mips3.drcdata->general.read_half_unsigned);					// call read_half_unsigned
		else
			emit_call(DRCTOP, mips3.drcdata->general.read_half_signed);						// call read_half_signed

		/* handle exceptions */
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbload_exception);
		if (destreg != REG_RAX)
			emit_mov_r64_r64(DRCTOP, destreg, REG_RAX);										// mov  <rt>,rax
	}

	/* if we can do fastram, emit the right form */
	else
	{
		INT64 fastdisp = (UINT8 *)fastram - (UINT8 *)drc->baseptr;
		int raxrel = ((INT32)fastdisp != fastdisp);

		/* read rax-relative */
		if (raxrel)
		{
			emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);								// mov  rax,fastram
			if (is_unsigned)
				emit_movzx_r64_m16(DRCTOP, destreg, MBD(REG_RAX, 0));						// movzx destreg,word ptr [rax]
			else
				emit_movsx_r64_m16(DRCTOP, destreg, MBD(REG_RAX, 0));						// movsx destreg,word ptr [rax]
		}

		/* read drc-relative */
		else
		{
			if (is_unsigned)
				emit_movzx_r64_m16(DRCTOP, destreg, MDRC(fastram));							// movzx destreg,word ptr [p1 + adjbase]
			else
				emit_movsx_r64_m16(DRCTOP, destreg, MDRC(fastram));							// movsx destreg,word ptr [p1 + adjbase]
		}
	}
	return (fastram != NULL);
}


/*------------------------------------------------------------------
    emit_fixed_word_read
------------------------------------------------------------------*/

static int emit_fixed_word_read(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT8 destreg, int is_unsigned, offs_t address, UINT32 mask)
{
	void *fastram = fastram_ptr(address & ~3, 4, FALSE);

	/* if we can't do fast RAM, we need to make the call */
	if (fastram == NULL)
	{
		emit_mov_r32_imm(DRCTOP, REG_P1, address & ~3);										// mov  p1,address & ~3
		if (mask != 0)
		{
			emit_mov_r32_imm(DRCTOP, REG_P2, (INT32)mask);									// mov  p2,mask
			emit_call(DRCTOP, mips3.drcdata->general.read_word_masked);						// call read_word_masked
		}
		else if (is_unsigned)
			emit_call(DRCTOP, mips3.drcdata->general.read_word_unsigned);					// call read_word_unsigned
		else
			emit_call(DRCTOP, mips3.drcdata->general.read_word_signed);						// call read_word_signed

		/* handle exceptions */
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbload_exception);
		if (destreg != REG_RAX)
			emit_mov_r64_r64(DRCTOP, destreg, REG_RAX);										// mov  <rt>,rax
	}

	/* if we can do fastram, emit the right form */
	else
	{
		INT64 fastdisp = (UINT8 *)fastram - (UINT8 *)drc->baseptr;
		int raxrel = ((INT32)fastdisp != fastdisp);

		/* if we can't fit the delta to fastram within an INT32, load RAX with the full address here */
		if (raxrel)
		{
			emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);								// mov  rax,fastram
			if (is_unsigned)
				emit_mov_r32_m32(DRCTOP, destreg, MBD(REG_RAX, 0));							// mov   destreg,word ptr [rax]
			else
				emit_movsxd_r64_m32(DRCTOP, destreg, MBD(REG_RAX, 0));						// movsxd destreg,dword ptr [rax]
		}

		/* read drc-relative */
		else
		{
			if (is_unsigned)
				emit_mov_r32_m32(DRCTOP, destreg, MDRC(fastram));							// mov   destreg,word ptr [p1 + adjbase]
			else
				emit_movsxd_r64_m32(DRCTOP, destreg, MDRC(fastram));						// movsxd destreg,dword ptr [p1 + adjbase]
		}
	}
	return (fastram != NULL);
}


/*------------------------------------------------------------------
    emit_fixed_double_read
------------------------------------------------------------------*/

static int emit_fixed_double_read(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT8 destreg, offs_t address, UINT64 mask)
{
	void *fastram = fastram_ptr(address & ~7, 8, FALSE);

	/* if we can't do fast RAM, we need to make the call */
	if (fastram == NULL)
	{
		emit_mov_r32_imm(DRCTOP, REG_P1, address & ~7);										// mov  p1,address & ~7
		if (mask != 0)
		{
			emit_mov_r64_imm(DRCTOP, REG_P2, mask);											// mov  p2,mask
			emit_call(DRCTOP, mips3.drcdata->general.read_double_masked);					// call read_double_masked
		}
		else
			emit_call(DRCTOP, mips3.drcdata->general.read_double);							// call read_double

		/* handle exceptions */
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbload_exception);
		if (destreg != REG_RAX)
			emit_mov_r64_r64(DRCTOP, destreg, REG_RAX);										// mov  <rt>,rax
	}

	/* if we can do fastram, emit the right form */
	else
	{
		INT64 fastdisp = (UINT8 *)fastram - (UINT8 *)drc->baseptr;
		int raxrel = ((INT32)fastdisp != fastdisp);

		/* if we can't fit the delta to fastram within an INT32, load RAX with the full address here */
		if (raxrel)
		{
			emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);								// mov  rax,fastram
			emit_mov_r64_m64(DRCTOP, destreg, MBD(REG_RAX, 0));								// mov  destreg,[rax]
		}

		/* read drc-relative */
		else
			emit_mov_r64_m64(DRCTOP, destreg, MDRC(fastram));								// mov  destreg,[p1 + adjbase]

		/* adjust for endianness */
		if (mips3.core->bigendian)
			emit_ror_r64_imm(DRCTOP, destreg, 32);											// ror  destreg,32
	}
	return (fastram != NULL);
}


/*------------------------------------------------------------------
    emit_fixed_byte_write
------------------------------------------------------------------*/

static int emit_fixed_byte_write(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, mipsreg_state srcreg, offs_t address)
{
	void *fastram = fastram_ptr(address, 1, TRUE);

	assert(ISCONST(srcreg) || X86REG(srcreg != REG_P1));

	/* if we can't do fast RAM, we need to make the call */
	if (fastram == NULL)
	{
		emit_mov_r32_imm(DRCTOP, REG_P1, address);											// mov  p1,address
		if (ISCONST(srcreg))
			emit_mov_r64_imm(DRCTOP, REG_P2, CONSTVAL(srcreg));								// mov  p2,<srcreg>
		else if (X86REG(srcreg) != REG_P2)
			emit_mov_r64_r64(DRCTOP, REG_P2, X86REG(srcreg));								// mov  p2,<srcreg>
		emit_call(DRCTOP, mips3.drcdata->general.write_byte);								// call write_byte

		/* handle exceptions */
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbstore_exception);
	}

	/* if we can do fastram, emit the right form */
	else
	{
		INT64 fastdisp = (UINT8 *)fastram - (UINT8 *)drc->baseptr;
		int raxrel = ((INT32)fastdisp != fastdisp);

		/* if we can't fit the delta to fastram within an INT32, load RAX with the full address here */
		if (raxrel)
		{
			emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);								// mov  rax,fastram
			if (ISCONST(srcreg))
				emit_mov_m8_imm(DRCTOP, MBD(REG_RAX, 0), CONSTVAL(srcreg));					// mov  [rax],constval
			else
				emit_mov_m8_r8(DRCTOP, MBD(REG_RAX, 0), X86REG(srcreg));					// mov  [rax],<srcreg>
		}

		/* write drc-relative */
		else
		{
			if (ISCONST(srcreg))
				emit_mov_m8_imm(DRCTOP, MDRC(fastram), CONSTVAL(srcreg));					// mov  [fastram],constval
			else
				emit_mov_m8_r8(DRCTOP, MDRC(fastram), X86REG(srcreg));						// mov  [fastram],srcreg
		}
	}
	return (fastram != NULL);
}


/*------------------------------------------------------------------
    emit_fixed_half_write
------------------------------------------------------------------*/

static int emit_fixed_half_write(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, mipsreg_state srcreg, offs_t address)
{
	void *fastram = fastram_ptr(address & ~1, 2, TRUE);

	assert(ISCONST(srcreg) || X86REG(srcreg) != REG_P1);

	/* if we can't do fast RAM, we need to make the call */
	if (fastram == NULL)
	{
		emit_mov_r32_imm(DRCTOP, REG_P1, address & ~1);										// mov  p1,address & ~1
		if (ISCONST(srcreg))
			emit_mov_r64_imm(DRCTOP, REG_P2, CONSTVAL(srcreg));								// mov  p2,constval
		else if (X86REG(srcreg) != REG_P2)
			emit_mov_r64_r64(DRCTOP, REG_P2, X86REG(srcreg));								// mov  p2,srcreg
		emit_call(DRCTOP, mips3.drcdata->general.write_half);								// call write_half

		/* handle exceptions */
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbstore_exception);
	}

	/* if we can do fastram, emit the right form */
	else
	{
		INT64 fastdisp = (UINT8 *)fastram - (UINT8 *)drc->baseptr;
		int raxrel = ((INT32)fastdisp != fastdisp);

		/* if we can't fit the delta to fastram within an INT32, load RAX with the full address here */
		if (raxrel)
		{
			emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);								// mov  rax,fastram
			if (ISCONST(srcreg))
				emit_mov_m16_imm(DRCTOP, MBD(REG_RAX, 0), CONSTVAL(srcreg));				// mov  [rax],constval
			else
				emit_mov_m16_r16(DRCTOP, MBD(REG_RAX, 0), X86REG(srcreg));					// mov  [rax],srcreg
		}

		/* write drc-relative */
		else
		{
			if (ISCONST(srcreg))
				emit_mov_m16_imm(DRCTOP, MDRC(fastram), CONSTVAL(srcreg));					// mov  [fastram],constval
			else
				emit_mov_m16_r16(DRCTOP, MDRC(fastram), X86REG(srcreg));					// mov  [fastram],srcreg
		}
	}
	return (fastram != NULL);
}


/*------------------------------------------------------------------
    emit_fixed_word_write
------------------------------------------------------------------*/

static int emit_fixed_word_write(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, mipsreg_state srcreg, offs_t address, UINT32 mask)
{
	void *fastram = fastram_ptr(address & ~3, 4, TRUE);

	assert(ISCONST(srcreg) || X86REG(srcreg) != REG_P1);
	assert(ISCONST(srcreg) || X86REG(srcreg) != REG_P3);

	/* if we can't do fast RAM, we need to make the call */
	if (fastram == NULL)
	{
		emit_mov_r32_imm(DRCTOP, REG_P1, address & ~3);										// mov  p1,address & ~3
		if (ISCONST(srcreg))
			emit_mov_r64_imm(DRCTOP, REG_P2, CONSTVAL(srcreg));								// mov  p2,constval
		else if (X86REG(srcreg) != REG_P2)
			emit_mov_r64_r64(DRCTOP, REG_P2, X86REG(srcreg));								// mov  p2,srcreg
		if (mask == 0)
			emit_call(DRCTOP, mips3.drcdata->general.write_word);							// call write_word
		else
		{
			emit_mov_r32_imm(DRCTOP, REG_P3, mask);											// mov  p3,mask
			emit_call(DRCTOP, mips3.drcdata->general.write_word_masked);					// call write_word_masked
		}

		/* handle exceptions */
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbstore_exception);
	}

	/* if we can do fastram, emit the right form */
	else
	{
		INT64 fastdisp = (UINT8 *)fastram - (UINT8 *)drc->baseptr;
		int raxrel = ((INT32)fastdisp != fastdisp);

		/* non-masked case */
		if (mask == 0)
		{
			/* if we can't fit the delta to fastram within an INT32, load RAX with the full address here */
			if (raxrel)
			{
				emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);							// mov  rax,fastram
				if (ISCONST(srcreg))
					emit_mov_m32_imm(DRCTOP, MBD(REG_RAX, 0), CONSTVAL(srcreg));			// mov  [rax],constval
				else
					emit_mov_m32_r32(DRCTOP, MBD(REG_RAX, 0), X86REG(srcreg));				// mov  [rax],srcreg
			}

			/* write drc-relative */
			else
			{
				if (ISCONST(srcreg))
					emit_mov_m32_imm(DRCTOP, MDRC(fastram), CONSTVAL(srcreg));				// mov  [fastram],constval
				else
					emit_mov_m32_r32(DRCTOP, MDRC(fastram), X86REG(srcreg));				// mov  [fastram],srcreg
			}
		}

		/* masked case */
		else
		{
			if (raxrel)
				emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);							// mov  rax,fastram
			else
				emit_lea_r64_m64(DRCTOP, REG_RAX, MDRC(fastram));							// lea   rax,[fastram]

			/* always write rax-relative */
			if (ISCONST(srcreg))
			{
				emit_mov_r32_m32(DRCTOP, REG_P2, MBD(REG_RAX, 0));							// mov  p2,[rax]
				emit_and_r32_imm(DRCTOP, REG_P2, mask);										// and  p2,mask
				emit_or_r32_imm(DRCTOP, REG_P2, CONSTVAL(srcreg) & ~mask);					// or   p2,constval & ~mask
				emit_mov_m32_r32(DRCTOP, MBD(REG_RAX, 0), REG_P2);							// mov  [rax],p2
			}
			else
			{
				emit_mov_r32_m32(DRCTOP, REG_P3, MBD(REG_RAX, 0));							// mov  p3,[rax]
				if (X86REG(srcreg) != REG_P2)
					emit_mov_r32_r32(DRCTOP, REG_P2, X86REG(srcreg));						// mov  p2,srcreg
				emit_and_r32_imm(DRCTOP, REG_P3, mask);										// and  p3,mask
				emit_and_r32_imm(DRCTOP, REG_P2, ~mask);									// and  p2,~mask
				emit_or_r32_r32(DRCTOP, REG_P3, REG_P2);									// or   p3,p2
				emit_mov_m32_r32(DRCTOP, MBD(REG_RAX, 0), REG_P3);							// mov  [rax],p3
			}
		}
	}
	return (fastram != NULL);
}


/*------------------------------------------------------------------
    emit_fixed_double_write
------------------------------------------------------------------*/

static int emit_fixed_double_write(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, mipsreg_state srcreg, offs_t address, UINT64 mask)
{
	void *fastram = fastram_ptr(address & ~7, 8, TRUE);

	assert(ISCONST(srcreg) || X86REG(srcreg) != REG_P1);
	assert(ISCONST(srcreg) || X86REG(srcreg) != REG_P3);
	assert((INT32)CONSTVAL(srcreg) == (INT64)CONSTVAL(srcreg));

	/* if we can't do fast RAM, we need to make the call */
	if (fastram == NULL)
	{
		emit_mov_r32_imm(DRCTOP, REG_P1, address & ~7);										// mov  p1,address & ~7
		if (ISCONST(srcreg))
			emit_mov_r64_imm(DRCTOP, REG_P2, CONSTVAL(srcreg));								// mov  p2,constval
		else if (X86REG(srcreg) != REG_P2)
			emit_mov_r64_r64(DRCTOP, REG_P2, X86REG(srcreg));								// mov  p2,srcreg
		if (mask == 0)
			emit_call(DRCTOP, mips3.drcdata->general.write_double);							// call write_double
		else
		{
			emit_mov_r64_imm(DRCTOP, REG_P3, mask);											// mov  p3,mask
			emit_call(DRCTOP, mips3.drcdata->general.write_double_masked);					// call write_double_masked
		}

		/* handle exceptions */
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbstore_exception);
	}

	/* if we can do fastram, emit the right form */
	else
	{
		INT64 fastdisp = (UINT8 *)fastram - (UINT8 *)drc->baseptr;
		int raxrel = ((INT32)fastdisp != fastdisp);
		INT64 constval = CONSTVAL(srcreg);

		/* adjust values for endianness */
		if (mips3.core->bigendian)
		{
			if (ISCONST(srcreg))
				constval = (constval << 32) | (constval >> 32);
			else
			{
				emit_mov_r64_r64(DRCTOP, REG_P2, X86REG(srcreg));							// mov  p2,srcreg
				emit_ror_r64_imm(DRCTOP, REG_P2, 32);										// ror  p2,32
				srcreg = REG_P2;
			}
			mask = (mask << 32) | (mask >> 32);
		}

		/* non-masked case */
		if (mask == 0)
		{
			/* if we can't fit the delta to fastram within an INT32, load RAX with the full address here */
			if (raxrel)
			{
				emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);							// mov  rax,fastram
				if (ISCONST(srcreg))
					emit_mov_m64_imm(DRCTOP, MBD(REG_RAX, 0), constval);					// mov  [rax],constval
				else
					emit_mov_m64_r64(DRCTOP, MBD(REG_RAX, 0), X86REG(srcreg));				// mov  [rax],srcreg
			}

			/* write drc-relative */
			else
			{
				if (ISCONST(srcreg))
					emit_mov_m64_imm(DRCTOP, MDRC(fastram), constval);						// mov  [fastram],constval
				else
					emit_mov_m64_r64(DRCTOP, MDRC(fastram), X86REG(srcreg));						// mov  [fastram],srcreg
			}
		}

		/* masked case */
		else
		{
			if (raxrel)
				emit_mov_r64_imm(DRCTOP, REG_RAX, (UINT64)fastram);							// mov  rax,fastram
			else
				emit_lea_r64_m64(DRCTOP, REG_RAX, MDRC(fastram));							// lea  rax,[fastram]
			emit_mov_r64_imm(DRCTOP, REG_P4, mask);											// mov  p4,mask

			/* always write rax-relative */
			if (ISCONST(srcreg))
			{
				emit_and_r64_m64(DRCTOP, REG_P4, MBD(REG_RAX, 0));							// and  p4,[rax]
				emit_mov_r64_imm(DRCTOP, REG_P3, constval & ~mask);							// mov  p3,constval & ~mask
				emit_or_r64_r64(DRCTOP, REG_P4, REG_P3);									// or   p4,p3
				emit_mov_m64_r64(DRCTOP, MBD(REG_RAX, 0), REG_P4);							// mov  [rax],p4
			}
			else
			{
				emit_mov_r64_m64(DRCTOP, REG_P3, MBD(REG_RAX, 0));							// mov  p3,[rax]
				if (X86REG(srcreg) != REG_P2)
					emit_mov_r64_r64(DRCTOP, REG_P2, X86REG(srcreg));						// mov  p2,srcreg
				emit_and_r64_r64(DRCTOP, REG_P3, REG_P4);									// and  p3,p4
				emit_not_r64(DRCTOP, REG_P4);												// not  p4
				emit_and_r64_r64(DRCTOP, REG_P2, REG_P4);									// and  p2,p4
				emit_or_r64_r64(DRCTOP, REG_P3, REG_P2);									// or   p3,p2
				emit_mov_m64_r64(DRCTOP, MBD(REG_RAX, 0), REG_P3);							// mov  [rax],p3
			}
		}
	}
	return (fastram != NULL);
}



/***************************************************************************
    CORE RECOMPILATION
***************************************************************************/

/*-------------------------------------------------
    compile_instruction - master switch statement
    for compiling a given opcode
-------------------------------------------------*/

static int compile_instruction(drc_core *drc, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT8 opswitch = op >> 26;

	switch (opswitch)
	{
		/* ----- sub-groups ----- */

		case 0x00:	/* SPECIAL */
			return compile_special(drc, compiler, desc);

		case 0x01:	/* REGIMM */
			return compile_regimm(drc, compiler, desc);

		case 0x1c:	/* IDT-specific */
			return compile_idt(drc, compiler, desc);


		/* ----- jumps and branches ----- */

		case 0x02:	/* J */
		case 0x03:	/* JAL */
			emit_delay_slot_and_branch(drc, compiler, desc, (opswitch == 0x02) ? 0 : 31);	// <next instruction + redispatch>
			return TRUE;

		case 0x04:	/* BEQ */
		case 0x14:	/* BEQL */
		case 0x05:	/* BNE */
		case 0x15:	/* BNEL */
			return compile_branchcc(drc, compiler, desc, (opswitch & 1) ? COND_NE : COND_E);

		case 0x06:	/* BLEZ */
		case 0x16:	/* BLEZL */
		case 0x07:	/* BGTZ */
		case 0x17:	/* BGTZL */
			return compile_branchcz(drc, compiler, desc, (opswitch & 1) ? COND_G : COND_LE, FALSE);


		/* ----- immediate arithmetic ----- */

		case 0x0f:	/* LUI */
			if (RTREG != 0)
				emit_set_constant_value(drc, compiler, desc, RTREG, SIMMVAL << 16);
			return TRUE;

		case 0x08:	/* ADDI */
		case 0x09:	/* ADDIU */
			return compile_add(drc, compiler, desc, opswitch & 1, TRUE);

		case 0x18:	/* DADDI */
		case 0x19:	/* DADDIU */
			return compile_dadd(drc, compiler, desc, opswitch & 1, TRUE);

		case 0x0c:	/* ANDI */
		case 0x0d:	/* ORI */
		case 0x0e:	/* XORI */
			return compile_logical(drc, compiler, desc, opswitch & 3, TRUE);

		case 0x0a:	/* SLTI */
		case 0x0b:	/* SLTIU */
			return compile_slt(drc, compiler, desc, opswitch & 1, TRUE);


		/* ----- memory load operations ----- */

		case 0x20:	/* LB */
		case 0x21:	/* LH */
		case 0x23:	/* LW */
		case 0x24:	/* LBU */
		case 0x25:	/* LHU */
		case 0x27:	/* LWU */
			return compile_load(drc, compiler, desc, (opswitch & 3) + 1, opswitch & 4);

		case 0x37:	/* LD */
			return compile_load(drc, compiler, desc, 8, FALSE);

		case 0x31:	/* LWC1 */
			return compile_load_cop(drc, compiler, desc, 4, FPR32(RTREG));

		case 0x35:	/* LDC1 */
			return compile_load_cop(drc, compiler, desc, 8, FPR64(RTREG));

		case 0x32:	/* LWC2 */
		case 0x36:	/* LDC2 */
			return compile_load_cop(drc, compiler, desc, (opswitch & 4) ? 8 : 4, &mips3.core->cpr[2][RTREG]);

		case 0x1a:	/* LDL */
		case 0x1b:	/* LDR */
			return compile_load_double_partial(drc, compiler, desc, opswitch & 1);

		case 0x22:	/* LWL */
		case 0x26:	/* LWR */
			return compile_load_word_partial(drc, compiler, desc, opswitch & 4);


		/* ----- memory store operations ----- */

		case 0x28:	/* SB */
		case 0x29:	/* SH */
		case 0x2b:	/* SW */
			return compile_store(drc, compiler, desc, (opswitch & 3) + 1);

		case 0x3f:	/* SD */
			return compile_store(drc, compiler, desc, 8);

		case 0x39:	/* SWC1 */
			return compile_store_cop(drc, compiler, desc, 4, FPR32(RTREG));

		case 0x3d:	/* SDC1 */
			return compile_store_cop(drc, compiler, desc, 8, FPR64(RTREG));

		case 0x3a:	/* SWC2 */
			return compile_store_cop(drc, compiler, desc, (opswitch & 4) ? 8 : 4, &mips3.core->cpr[2][RTREG]);

		case 0x2a:	/* SWL */
		case 0x2e:	/* SWR */
			return compile_store_word_partial(drc, compiler, desc, opswitch & 4);

		case 0x2c:	/* SDL */
		case 0x2d:	/* SDR */
			return compile_store_double_partial(drc, compiler, desc, opswitch & 1);


		/* ----- effective no-ops ----- */

		case 0x2f:	/* CACHE */
		case 0x33:	/* PREF */
			return TRUE;


		/* ----- coprocessor instructions ----- */

		case 0x10:	/* COP0 */
			return compile_cop0(drc, compiler, desc);

		case 0x11:	/* COP1 */
			return compile_cop1(drc, compiler, desc);

		case 0x13:	/* COP1X - R5000 */
			return compile_cop1x(drc, compiler, desc);

		case 0x12:	/* COP2 */
			oob_request_callback(drc, COND_NONE, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_invalidop_exception);
																							// jmp  generate_invalidop_exception
			return TRUE;


		/* ----- unimplemented/illegal instructions ----- */

//      case 0x30:  /* LL */        logerror("mips3 Unhandled op: LL\n");                                   break;
//      case 0x34:  /* LLD */       logerror("mips3 Unhandled op: LLD\n");                                  break;
//      case 0x38:  /* SC */        logerror("mips3 Unhandled op: SC\n");                                   break;
//      case 0x3c:  /* SCD */       logerror("mips3 Unhandled op: SCD\n");                                  break;
//      case 0x3b:  /* SWC3 */      invalid_instruction(op);                                                break;
//      default:    /* ??? */       invalid_instruction(op);                                                break;
	}

	return FALSE;
}


/*-------------------------------------------------
    compile_special - compile opcodes in the
    'SPECIAL' group
-------------------------------------------------*/

static int compile_special(drc_core *drc, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT8 opswitch = op & 63;
	emit_link link1 = { 0 };

	switch (opswitch)
	{
		/* ----- shift instructions ----- */

		case 0x00:	/* SLL - MIPS I */
		case 0x02:	/* SRL - MIPS I */
		case 0x03:	/* SRA - MIPS I */
		case 0x04:	/* SLLV - MIPS I */
		case 0x06:	/* SRLV - MIPS I */
		case 0x07:	/* SRAV - MIPS I */
			return compile_shift(drc, compiler, desc, opswitch & 0x03, opswitch & 0x04);

		case 0x14:	/* DSLLV - MIPS III */
		case 0x16:	/* DSRLV - MIPS III */
		case 0x17:	/* DSRAV - MIPS III */
		case 0x38:	/* DSLL - MIPS III */
		case 0x3a:	/* DSRL - MIPS III */
		case 0x3b:	/* DSRA - MIPS III */
		case 0x3c:	/* DSLL32 - MIPS III */
		case 0x3e:	/* DSRL32 - MIPS III */
		case 0x3f:	/* DSRA32 - MIPS III */
			return compile_dshift(drc, compiler, desc, opswitch & 0x03, ~opswitch & 0x20, opswitch & 0x04);


		/* ----- basic arithmetic ----- */

		case 0x20:	/* ADD - MIPS I */
		case 0x21:	/* ADDU - MIPS I */
			return compile_add(drc, compiler, desc, opswitch & 1, FALSE);

		case 0x2c:	/* DADD - MIPS III */
		case 0x2d:	/* DADDU - MIPS III */
			return compile_dadd(drc, compiler, desc, opswitch & 1, FALSE);

		case 0x22:	/* SUB - MIPS I */
		case 0x23:	/* SUBU - MIPS I */
			return compile_sub(drc, compiler, desc, opswitch & 1);

		case 0x2e:	/* DSUB - MIPS III */
		case 0x2f:	/* DSUBU - MIPS III */
			return compile_dsub(drc, compiler, desc, opswitch & 1);

		case 0x18:	/* MULT - MIPS I */
		case 0x19:	/* MULTU - MIPS I */
			return compile_mult(drc, compiler, desc, opswitch & 1);

		case 0x1c:	/* DMULT - MIPS III */
		case 0x1d:	/* DMULTU - MIPS III */
			return compile_dmult(drc, compiler, desc, opswitch & 1);

		case 0x1a:	/* DIV - MIPS I */
		case 0x1b:	/* DIVU - MIPS I */
			return compile_div(drc, compiler, desc, opswitch & 1);

		case 0x1e:	/* DDIV - MIPS III */
		case 0x1f:	/* DDIVU - MIPS III */
			return compile_ddiv(drc, compiler, desc, opswitch & 1);


		/* ----- basic logical ops ----- */

		case 0x24:	/* AND - MIPS I */
		case 0x25:	/* OR - MIPS I */
		case 0x26:	/* XOR - MIPS I */
		case 0x27:	/* NOR - MIPS I */
			return compile_logical(drc, compiler, desc, opswitch & 3, FALSE);


		/* ----- basic comparisons ----- */

		case 0x2a:	/* SLT - MIPS I */
		case 0x2b:	/* SLTU - MIPS I */
			return compile_slt(drc, compiler, desc, opswitch & 1, FALSE);


		/* ----- conditional traps ----- */

		case 0x30:	/* TGE - MIPS II */
		case 0x31:	/* TGEU - MIPS II */
		case 0x32:	/* TLT - MIPS II */
		case 0x33:	/* TLTU - MIPS II */
		case 0x34:	/* TEQ - MIPS II */
		case 0x36:	/* TNE - MIPS II */
			return compile_trapcc(drc, compiler, desc, opswitch & 7, FALSE);


		/* ----- conditional moves ----- */

		case 0x0a:	/* MOVZ - MIPS IV */
		case 0x0b:	/* MOVN - MIPS IV */
			return compile_movzn(drc, compiler, desc, opswitch & 1);

		case 0x01:	/* MOVF - MIPS IV */
			if (RDREG != 0)
			{
				mipsreg_state rs, rd;

				/* allocate source registers */
				rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
				rd = compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_SRC | ALLOC_DST | ALLOC_DIRTY);

				emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);						// cmp  fcc[x],0
				emit_jcc_short_link(DRCTOP, ((op >> 16) & 1) ? COND_Z : COND_NZ, &link1);	// jz/nz skip
				emit_load_register_value(drc, compiler, X86REG(rd), rs);					// mov  <rd>,<rs>
			}
			resolve_link(DRCTOP, &link1);												// skip:
			return TRUE;


		/* ----- jumps and branches ----- */

		case 0x08:	/* JR - MIPS I */
		case 0x09:	/* JALR - MIPS I */
			{
				mipsreg_state rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
				if (ISCONST(rs))
					emit_mov_m32_imm(DRCTOP, MBD(REG_RSP, SPOFFS_NEXTPC), CONSTVAL(rs));	// mov  [esp+nextpc],rs
				else
					emit_mov_m32_r32(DRCTOP, MBD(REG_RSP, SPOFFS_NEXTPC), X86REG(rs));		// mov  [esp+nextpc],rs
				emit_delay_slot_and_branch(drc, compiler, desc, (opswitch == 0x09) ? RDREG : 0);
																							// <next instruction + redispatch>
			}
			return TRUE;


		/* ----- system calls ----- */

		case 0x0c:	/* SYSCALL - MIPS I */
			oob_request_callback(drc, COND_NONE, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_syscall_exception);
																							// jmp  generate_syscall_exception
			return TRUE;

		case 0x0d:	/* BREAK - MIPS I */
			oob_request_callback(drc, COND_NONE, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_break_exception);
																							// jmp  generate_break_exception
			return TRUE;


		/* ----- effective no-ops ----- */

		case 0x0f:	/* SYNC - MIPS II */
			return TRUE;


		/* ----- hi/lo register access ----- */

		case 0x10:	/* MFHI - MIPS I */
			if (RDREG != 0)
			{
				mipsreg_state hi = compiler_register_allocate(drc, compiler, desc, REG_HI, ALLOC_SRC);
				mipsreg_state rd = compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_DST | ALLOC_DIRTY);
				emit_load_register_value(drc, compiler, X86REG(rd), hi);					// mov  <rd>,<hi>
			}
			return TRUE;

		case 0x11:	/* MTHI - MIPS I */
			if (RSREG != 0)
			{
				mipsreg_state rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
				mipsreg_state hi = compiler_register_allocate(drc, compiler, desc, REG_HI, ALLOC_DST | ALLOC_DIRTY);
				emit_load_register_value(drc, compiler, X86REG(hi), rs);					// mov  hi,<rs>
			}
			return TRUE;

		case 0x12:	/* MFLO - MIPS I */
			if (RDREG != 0)
			{
				mipsreg_state lo = compiler_register_allocate(drc, compiler, desc, REG_LO, ALLOC_SRC);
				mipsreg_state rd = compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_DST | ALLOC_DIRTY);
				emit_load_register_value(drc, compiler, X86REG(rd), lo);					// mov  <rd>,<lo>
			}
			return TRUE;

		case 0x13:	/* MTLO - MIPS I */
			if (RSREG != 0)
			{
				mipsreg_state rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
				mipsreg_state lo = compiler_register_allocate(drc, compiler, desc, REG_LO, ALLOC_DST | ALLOC_DIRTY);
				emit_load_register_value(drc, compiler, X86REG(lo), rs);					// mov  lo,<rs>
			}
			return TRUE;
	}
	return FALSE;
}


/*-------------------------------------------------
    compile_regimm - compile opcodes in the
    'REGIMM' group
-------------------------------------------------*/

static int compile_regimm(drc_core *drc, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT8 opswitch = RTREG;

	switch (opswitch)
	{
		case 0x00:	/* BLTZ */
		case 0x01:	/* BGEZ */
		case 0x02:	/* BLTZL */
		case 0x03:	/* BGEZL */
		case 0x10:	/* BLTZAL */
		case 0x11:	/* BGEZAL */
		case 0x12:	/* BLTZALL */
		case 0x13:	/* BGEZALL */
			return compile_branchcz(drc, compiler, desc, (opswitch & 1) ? COND_GE : COND_L, opswitch & 0x10);

		case 0x08:	/* TGEI */
		case 0x09:	/* TGEIU */
		case 0x0a:	/* TLTI */
		case 0x0b:	/* TLTIU */
		case 0x0c:	/* TEQI */
		case 0x0e:	/* TNEI */
			return compile_trapcc(drc, compiler, desc, opswitch & 7, TRUE);
	}
	return FALSE;
}


/*-------------------------------------------------
    compile_idt - compile opcodes in the IDT-
    specific group
-------------------------------------------------*/

static int compile_idt(drc_core *drc, compiler_state *compiler, const opcode_desc *desc)
{
	fatalerror("Unimplemented IDT instructions");
#if 0
	UINT32 op = *desc->opptr.l;
	UINT8 opswitch = op & 0x1f;

	switch (opswitch)
	{
		case 0: /* MAD */
			if (RSREG != 0 && RTREG != 0)
			{
				emit_mov_r32_m32(REG_EAX, REGADDR(RSREG));							// mov  eax,[rsreg]
				emit_mov_r32_m32(REG_EDX, REGADDR(RTREG));							// mov  edx,[rtreg]
				emit_imul_r32(DRCTOP, REG_EDX);										// imul edx
				emit_add_r32_m32(DRCTOP, LOADDR);									// add  eax,[lo]
				emit_adc_r32_m32(DRCTOP, HIADDR);									// adc  edx,[hi]
				emit_mov_r32_r32(DRCTOP, REG_EBX, REG_EDX);							// mov  ebx,edx
				emit_cdq(DRCTOP);													// cdq
				emit_mov_m64_r64(DRCTOP, LOADDR, REG_EAX);							// mov  [lo],edx:eax
				emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EBX);							// mov  eax,ebx
				emit_cdq(DRCTOP);													// cdq
				emit_mov_m64_r64(DRCTOP, HIADDR, REG_EAX);							// mov  [hi],edx:eax
			}
			return compile_SUCCESSFUL_CP(3,4);

		case 1: /* MADU */
			if (RSREG != 0 && RTREG != 0)
			{
				emit_mov_r32_m32(REG_EAX, REGADDR(RSREG));							// mov  eax,[rsreg]
				emit_mov_r32_m32(REG_EDX, REGADDR(RTREG));							// mov  edx,[rtreg]
				emit_mul_r32(DRCTOP, REG_EDX);										// mul  edx
				emit_add_r32_m32(DRCTOP, LOADDR);									// add  eax,[lo]
				emit_adc_r32_m32(DRCTOP, HIADDR);									// adc  edx,[hi]
				emit_mov_r32_r32(DRCTOP, REG_EBX, REG_EDX);							// mov  ebx,edx
				emit_cdq(DRCTOP);													// cdq
				emit_mov_m64_r64(DRCTOP, LOADDR, REG_EDX, REG_EAX);					// mov  [lo],edx:eax
				emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EBX);							// mov  eax,ebx
				emit_cdq(DRCTOP);													// cdq
				emit_mov_m64_r64(DRCTOP, HIADDR, REG_EDX, REG_EAX);					// mov  [hi],edx:eax
			}
			return compile_SUCCESSFUL_CP(3,4);

		case 2: /* MUL */
			if (RDREG != 0)
			{
				if (RSREG != 0 && RTREG != 0)
				{
					emit_mov_r32_m32(REG_EAX, REGADDR(RSREG));						// mov  eax,[rsreg]
					emit_imul_r32_m32(REG_EAX, REGADDR(RTREG));						// imul eax,[rtreg]
					emit_cdq(DRCTOP);												// cdq
					emit_mov_m64_r64(REG_ESI, REGADDR(RDREG), REG_EDX, REG_EAX);	// mov  [rd],edx:eax
				}
				else
					emit_zero_m64(REG_ESI, REGADDR(RDREG));
			}
			return compile_SUCCESSFUL_CP(3,4);
	}
#endif
	return FALSE;
}



/***************************************************************************
    COP0 RECOMPILATION
***************************************************************************/

/*-------------------------------------------------
    compile_set_cop0_reg - generate code to
    handle special COP0 registers
-------------------------------------------------*/

static int compile_set_cop0_reg(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT8 reg)
{
	emit_link link1;

	switch (reg)
	{
		case COP0_Cause:
			emit_mov_r32_m32(DRCTOP, REG_EDX, CPR0ADDR(COP0_Cause));						// mov  edx,[Cause]
			emit_and_r32_imm(DRCTOP, REG_EAX, ~0xfc00);										// and  eax,~0xfc00
			emit_and_r32_imm(DRCTOP, REG_EDX, 0xfc00);										// and  edx,0xfc00
			emit_or_r32_r32(DRCTOP, REG_EAX, REG_EDX);										// or   eax,edx
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Cause), REG_EAX);						// mov  [Cause],eax
			emit_and_r32_m32(DRCTOP, REG_EAX, CPR0ADDR(COP0_Status));						// and  eax,[SR]
			emit_test_r32_imm(DRCTOP, REG_EAX, 0x300);										// test eax,0x300
			oob_request_callback(drc, COND_NZ, oob_interrupt_cleanup, compiler, desc, mips3.drcdata->generate_interrupt_exception);
																							// jnz  generate_interrupt_exception
			return TRUE;

		case COP0_Status:
			emit_flush_cycles_before_instruction(drc, compiler, desc, 1);
			emit_mov_r32_m32(DRCTOP, REG_EDX, CPR0ADDR(COP0_Status));						// mov  edx,[Status]
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Status), REG_EAX);						// mov  [Status],eax
			emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EAX);										// xor  edx,eax
			emit_test_r32_imm(DRCTOP, REG_EDX, SR_IMEX5);										// test edx,0x8000
			emit_jcc_short_link(DRCTOP, COND_Z, &link1);									// jz   skip
			emit_lea_r64_m64(DRCTOP, REG_P1, COREADDR);										// lea  p1,[mips3.core]
			emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->mips3com_update_cycle_counting));	// call mips3com_update_cycle_counting
			resolve_link(DRCTOP, &link1);												// skip:
			append_check_interrupts(drc, compiler, desc);									// <check interrupts>
			return TRUE;

		case COP0_Count:
			emit_flush_cycles_before_instruction(drc, compiler, desc, 1);
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Count), REG_EAX);						// mov  [Count],eax
			emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->activecpu_gettotalcycles));			// call activecpu_gettotalcycles
			emit_mov_r32_m32(DRCTOP, REG_EDX, CPR0ADDR(COP0_Count));						// mov  edx,[Count]
			emit_sub_r64_r64(DRCTOP, REG_RAX, REG_RDX);										// sub  rax,rdx
			emit_sub_r64_r64(DRCTOP, REG_RAX, REG_RDX);										// sub  rax,rdx
			emit_mov_m64_r64(DRCTOP, MDRC(&mips3.core->count_zero_time), REG_RAX);			// mov  [count_zero_time],rax
			emit_lea_r64_m64(DRCTOP, REG_P1, COREADDR);										// lea  p1,[mips3.core]
			emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->mips3com_update_cycle_counting));	// call mips3com_update_cycle_counting
			return TRUE;

		case COP0_Compare:
			emit_flush_cycles_before_instruction(drc, compiler, desc, 1);
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Compare), REG_EAX);						// mov  [Compare],eax
			emit_and_m32_imm(DRCTOP, CPR0ADDR(COP0_Cause), ~0x8000);						// and  [Cause],~0x8000
			emit_lea_r64_m64(DRCTOP, REG_P1, COREADDR);										// lea  p1,[mips3.core]
			emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->mips3com_update_cycle_counting));	// call mips3com_update_cycle_counting
			return TRUE;

		case COP0_PRId:
			return TRUE;

		case COP0_Config:
			emit_mov_r32_m32(DRCTOP, REG_EDX, CPR0ADDR(COP0_Config));						// mov  edx,[Config]
			emit_and_r32_imm(DRCTOP, REG_EAX, 0x0007);										// and  eax,0x0007
			emit_and_r32_imm(DRCTOP, REG_EDX, ~0x0007);										// and  edx,~0x0007
			emit_or_r32_r32(DRCTOP, REG_EAX, REG_EDX);										// or   eax,edx
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(COP0_Config), REG_EAX);						// mov  [Config],eax
			return TRUE;

		default:
			emit_mov_m32_r32(DRCTOP, CPR0ADDR(reg), REG_EAX);								// mov  cpr0[reg],eax
			return TRUE;
	}
}


/*-------------------------------------------------
    compile_get_cop0_reg - generate code to
    read special COP0 registers
-------------------------------------------------*/

static int compile_get_cop0_reg(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, UINT8 reg)
{
	emit_link link1, link2;

	switch (reg)
	{
		case COP0_Count:
			compiler->cycles += MIPS3_COUNT_READ_CYCLES;
			emit_flush_cycles_before_instruction(drc, compiler, desc, MIPS3_COUNT_READ_CYCLES);
			emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->activecpu_gettotalcycles));			// call activecpu_gettotalcycles
			emit_sub_r64_m64(DRCTOP, REG_RAX, MDRC(&mips3.core->count_zero_time));			// sub  rax,[count_zero_time]
			emit_shr_r64_imm(DRCTOP, REG_RAX, 1);											// shr  rax,1
			emit_movsxd_r64_r32(DRCTOP, REG_RAX, REG_EAX);									// movsxd rax,eax
			return TRUE;

		case COP0_Cause:
			compiler->cycles += MIPS3_CAUSE_READ_CYCLES;
			emit_movsxd_r64_m32(DRCTOP, REG_RAX, CPR0ADDR(COP0_Cause));						// movsxd rax,[Cause]
			return TRUE;

		case COP0_Random:
			emit_flush_cycles_before_instruction(drc, compiler, desc, 1);
			emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->activecpu_gettotalcycles));			// call activecpu_gettotalcycles
			emit_mov_r32_m32(DRCTOP, REG_ECX, CPR0ADDR(COP0_Wired));						// mov  ecx,[Wired]
			emit_mov_r32_imm(DRCTOP, REG_R8D, 48);											// mov  r8d,48
			emit_and_r32_imm(DRCTOP, REG_ECX, 0x3f);										// and  ecx,0x3f
			emit_sub_r64_m64(DRCTOP, REG_RAX, MDRC(&mips3.core->count_zero_time));			// sub  rax,[count_zero_time]
			emit_sub_r32_r32(DRCTOP, REG_R8D, REG_ECX);										// sub  r8d,ecx
			emit_jcc_short_link(DRCTOP, COND_BE, &link1);									// jbe  link1
			emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);										// xor  edx,edx
			emit_div_r64(DRCTOP, REG_R8);													// div  r8
			emit_mov_r32_r32(DRCTOP, REG_EAX, REG_EDX);										// mov  eax,edx
			emit_add_r32_r32(DRCTOP, REG_EAX, REG_ECX);										// add  eax,ecx
			emit_and_r32_imm(DRCTOP, REG_EAX, 0x3f);										// and  eax,0x3f
			emit_jmp_short_link(DRCTOP, &link2);											// jmp  link2
			resolve_link(DRCTOP, &link1);												// link1:
			emit_mov_r32_imm(DRCTOP, REG_EAX, 47);											// mov  eax,47
			resolve_link(DRCTOP, &link2);												// link2:
			return TRUE;

		default:
			emit_movsxd_r64_m32(DRCTOP, REG_RAX, CPR0ADDR(reg));							// movsxd rax,cpr0[reg]
			return TRUE;
	}
}


/*-------------------------------------------------
    compile_cop0 - compile COP0 opcodes
-------------------------------------------------*/

static int compile_cop0(drc_core *drc, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT8 opswitch = RSREG;

	/* generate an exception if COP0 is disabled or we are not in kernel mode */
	if (mips3.drcoptions & MIPS3DRC_STRICT_COP0)
	{
		emit_link checklink;

		emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_KSU_MASK);						// test [SR],SR_KSU_MASK
		emit_jcc_short_link(DRCTOP, COND_Z, &checklink);									// jz   okay
		emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_COP0);							// test [SR],SR_COP0
		oob_request_callback(drc, COND_Z, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_cop_exception);
																							// jz   generate_cop_exception
		resolve_link(DRCTOP, &checklink);												// okay:
	}

	switch (opswitch)
	{
		case 0x00:	/* MFCz */
		case 0x01:	/* DMFCz */
			if (RTREG != 0)
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_DST | ALLOC_DIRTY);
				compile_get_cop0_reg(drc, compiler, desc, RDREG);							// mov  eax,cpr0[rd])
				if (!(opswitch & 1))
					emit_movsxd_r64_r32(DRCTOP, X86REG(rt), REG_EAX);						// movsxd <rt>,eax
				else
					emit_mov_r64_r64(DRCTOP, X86REG(rt), REG_RAX);							// mov  <rt>,rax
			}
			return TRUE;

		case 0x02:	/* CFCz */
			if (RTREG != 0)
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_DST | ALLOC_DIRTY);
				emit_movsxd_r64_m32(DRCTOP, X86REG(rt), CCR0ADDR(RDREG));					// movsxd <rt>,ccr1[rd]
			}
			return TRUE;

		case 0x04:	/* MTCz */
		case 0x05:	/* DMTCz */
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);
				emit_load_register_value(drc, compiler, REG_EAX, rt);						// mov  eax,<rt>
				compile_set_cop0_reg(drc, compiler, desc, RDREG);							// mov  cpr0[rd],rax
			}
			return TRUE;

		case 0x06:	/* CTCz */
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);
				emit_load_register_value(drc, compiler, REG_EAX, rt);						// mov  eax,<rt>
				emit_mov_m32_r32(DRCTOP, CCR0ADDR(RDREG), REG_EAX);							// mov  ccr1[rd],eax
			}
			return TRUE;

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
					emit_lea_r64_m64(DRCTOP, REG_P1, COREADDR);								// lea  p1,[mips3.core]
					emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->mips3com_tlbr));				// call mips3com_tlbr
					return TRUE;

				case 0x02:	/* TLBWI */
					emit_lea_r64_m64(DRCTOP, REG_P1, COREADDR);								// lea  p1,[mips3.core]
					emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->mips3com_tlbwi));			// call mips3com_tlbwi
					return TRUE;

				case 0x06:	/* TLBWR */
					emit_lea_r64_m64(DRCTOP, REG_P1, COREADDR);								// lea  p1,[mips3.core]
					emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->mips3com_tlbwr));			// call mips3com_tlbwr
					return TRUE;

				case 0x08:	/* TLBP */
					emit_lea_r64_m64(DRCTOP, REG_P1, COREADDR);								// lea  p1,[mips3.core]
					emit_call_m64(DRCTOP, MDRC(&mips3.drcdata->mips3com_tlbp));				// call mips3com_tlbp
					return TRUE;

				case 0x18:	/* ERET */
					emit_mov_r32_m32(DRCTOP, REG_P1, CPR0ADDR(COP0_EPC));					// mov  p1,[EPC]
					emit_and_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), ~SR_EXL);				// and  [SR],~SR_EXL
					compiler_register_flush_all(drc, compiler);								// <flush registers>
					drc_append_dispatcher(drc);												// <dispatch>
					return TRUE;

				case 0x20:	/* WAIT */
					return TRUE;
			}
			break;
	}

	return FALSE;
}



/***************************************************************************
    COP1 RECOMPILATION
***************************************************************************/

/*-------------------------------------------------
    compile_cop1 - compile COP1 opcodes
-------------------------------------------------*/

static int compile_cop1(drc_core *drc, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	emit_link link1;

	/* generate an exception if COP1 is disabled */
	if (mips3.drcoptions & MIPS3DRC_STRICT_COP1)
	{
		emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_COP1);							// test [mips3.core->cpr[0][COP0_Status]],SR_COP1
		oob_request_callback(drc, COND_Z, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_cop_exception);
																							// jz   generate_cop_exception
	}

	switch (RSREG)
	{
		case 0x00:	/* MFCz */
			if (RTREG != 0)
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_DST | ALLOC_DIRTY);
				emit_movsxd_r64_m32(DRCTOP, X86REG(rt), FPR32ADDR(RDREG));						// movsxd <rt>,cpr[rd]
			}
			return TRUE;

		case 0x01:	/* DMFCz */
			if (RTREG != 0)
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_DST | ALLOC_DIRTY);
				emit_mov_r64_m64(DRCTOP, X86REG(rt), FPR64ADDR(RDREG));							// mov  <rt>,cpr[rd]
			}
			return TRUE;

		case 0x02:	/* CFCz */
			if (RTREG != 0)
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_DST | ALLOC_DIRTY);
				if (RDREG != 31)
					emit_movsxd_r64_m32(DRCTOP, X86REG(rt), CCR1ADDR(RDREG));					// movsxd <rt>,ccr1[rd]
				else
				{
					emit_call(DRCTOP, mips3.drcdata->recover_ccr31);							// call recover_ccr31
					emit_mov_r64_r64(DRCTOP, X86REG(rt), REG_RAX);								// mov  [X86REG(rt)],rax
				}
			}
			return TRUE;

		case 0x04:	/* MTCz */
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);
				UINT8 tempreg = ISCONST(rt) ? REG_EAX : X86REG(rt);
				emit_load_register_value(drc, compiler, tempreg, rt);							// mov  <tempreg>,<rt>
				emit_mov_m32_r32(DRCTOP, FPR32ADDR(RDREG), tempreg);							// mov  cpr1[rd],<tempreg>
			}
			return TRUE;

		case 0x05:	/* DMTCz */
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);
				UINT8 tempreg = ISCONST(rt) ? REG_EAX : X86REG(rt);
				emit_load_register_value(drc, compiler, tempreg, rt);							// mov  <tempreg>,<rt>
				emit_mov_m64_r64(DRCTOP, FPR64ADDR(RDREG), tempreg);							// mov  cpr1[rd],<tempreg>
			}
			return TRUE;

		case 0x06:	/* CTCz */
			{
				mipsreg_state rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);
				emit_load_register_value(drc, compiler, REG_EAX, rt);							// mov  eax,<rt>
				if (RDREG != 31)
					emit_mov_m32_r32(DRCTOP, CCR1ADDR(RDREG), REG_EAX);							// mov  ccr1[rd],eax
				else
					emit_call(DRCTOP, mips3.drcdata->explode_ccr31);							// call explode_ccr31
			}
			return TRUE;

		case 0x08:	/* BC */
			switch ((op >> 16) & 3)
			{
				case 0x00:	/* BCzF */
				case 0x02:	/* BCzFL */
					emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);					// cmp  fcc[which],0
					emit_jcc_near_link(DRCTOP, COND_NZ, &link1);							// jnz  link1
					emit_delay_slot_and_branch(drc, compiler, desc, 0);						// <next instruction + redispatch>
					resolve_link(DRCTOP, &link1);										// skip:
					return TRUE;

				case 0x01:	/* BCzT */
				case 0x03:	/* BCzTL */
					emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);					// cmp  fcc[which],0
					emit_jcc_near_link(DRCTOP, COND_Z, &link1);								// jz  link1
					emit_delay_slot_and_branch(drc, compiler, desc, 0);						// <next instruction + redispatch>
					resolve_link(DRCTOP, &link1);										// skip:
					return TRUE;
			}
			break;

		default:
			switch (op & 0x3f)
			{
				case 0x00:
					if (IS_SINGLE(op))	/* ADD.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_addss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// addss xmm0,[ftreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* ADD.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_addsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// addsd xmm0,[ftreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x01:
					if (IS_SINGLE(op))	/* SUB.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_subss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// subss xmm0,[ftreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* SUB.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_subsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// subsd xmm0,[ftreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x02:
					if (IS_SINGLE(op))	/* MUL.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// mulss xmm0,[ftreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* MUL.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// mulsd xmm0,[ftreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x03:
					if (IS_SINGLE(op))	/* DIV.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_divss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// divss xmm0,[ftreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* DIV.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_divsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// divsd xmm0,[ftreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x04:
					if (IS_SINGLE(op))	/* SQRT.S */
					{
						emit_sqrtss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// sqrtss xmm0,[fsreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* SQRT.D */
					{
						emit_sqrtsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// sqrtsd xmm0,[fsreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x05:
					if (IS_SINGLE(op))	/* ABS.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_andps_r128_m128(DRCTOP, REG_XMM0, MDRC(mips3.drcdata->abs32mask));
																							// andps xmm0,[abs32mask]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* ABS.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_andps_r128_m128(DRCTOP, REG_XMM0, MDRC(mips3.drcdata->abs64mask));
																							// andpd xmm0,[abs64mask]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x06:
					if (IS_SINGLE(op))	/* MOV.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* MOV.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x07:
					if (IS_SINGLE(op))	/* NEG.S */
					{
						emit_xorps_r128_r128(DRCTOP, REG_XMM0, REG_XMM0);					// xorps xmm0,xmm0
						emit_subss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// subss xmm0,[fsreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* NEG.D */
					{
						emit_xorps_r128_r128(DRCTOP, REG_XMM0, REG_XMM0);					// xorps xmm0,xmm0
						emit_subsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// subsd xmm0,[fsreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x08:
					/* FUTURE: On Penryn, we can use ROUNDPS/ROUNDPD */
					drc_append_set_temp_sse_rounding(drc, FPRND_NEAR);						// ldmxcsr [round]
					if (IS_SINGLE(op))	/* ROUND.L.S */
						emit_cvtss2si_r64_m32(DRCTOP, REG_RAX, FPR32ADDR(FSREG));			// cvtss2si rax,[fsreg]
					else				/* ROUND.L.D */
						emit_cvtsd2si_r64_m64(DRCTOP, REG_RAX, FPR64ADDR(FSREG));			// cvtsd2si rax,[fsreg]
					emit_mov_m64_r64(DRCTOP, FPR64ADDR(FDREG), REG_RAX);					// mov  [fdreg],rax
					drc_append_restore_sse_rounding(drc);									// ldmxcsr [mode]
					return TRUE;

				case 0x09:
					if (IS_SINGLE(op))	/* TRUNC.L.S */
						emit_cvttss2si_r64_m32(DRCTOP, REG_RAX, FPR32ADDR(FSREG));			// cvttss2si rax,[fsreg]
					else				/* TRUNC.L.D */
						emit_cvttsd2si_r64_m64(DRCTOP, REG_RAX, FPR64ADDR(FSREG));			// cvttsd2si rax,[fsreg]
					emit_mov_m64_r64(DRCTOP, FPR64ADDR(FDREG), REG_RAX);					// mov  [fdreg],rax
					return TRUE;

				case 0x0a:
					/* FUTURE: On Penryn, we can use ROUNDPS/ROUNDPD */
					drc_append_set_temp_sse_rounding(drc, FPRND_UP);						// ldmxcsr [round]
					if (IS_SINGLE(op))	/* CEIL.L.S */
						emit_cvtss2si_r64_m32(DRCTOP, REG_RAX, FPR32ADDR(FSREG));			// cvtss2si rax,[fsreg]
					else				/* CEIL.L.D */
						emit_cvtsd2si_r64_m64(DRCTOP, REG_RAX, FPR64ADDR(FSREG));			// cvtsd2si rax,[fsreg]
					emit_mov_m64_r64(DRCTOP, FPR64ADDR(FDREG), REG_RAX);					// mov  [fdreg],rax
					drc_append_restore_sse_rounding(drc);									// ldmxcsr [mode]
					return TRUE;

				case 0x0b:
					/* FUTURE: On Penryn, we can use ROUNDPS/ROUNDPD */
					drc_append_set_temp_sse_rounding(drc, FPRND_DOWN);						// ldmxcsr [round]
					if (IS_SINGLE(op))	/* FLOOR.L.S */
						emit_cvtss2si_r64_m32(DRCTOP, REG_RAX, FPR32ADDR(FSREG));			// cvtss2si rax,[fsreg]
					else				/* FLOOR.L.D */
						emit_cvtsd2si_r64_m64(DRCTOP, REG_RAX, FPR64ADDR(FSREG));			// cvtsd2si rax,[fsreg]
					emit_mov_m64_r64(DRCTOP, FPR64ADDR(FDREG), REG_RAX);					// mov  [fdreg],rax
					drc_append_restore_sse_rounding(drc);									// ldmxcsr [mode]
					return TRUE;

				case 0x0c:
					/* FUTURE: On Penryn, we can use ROUNDPS/ROUNDPD */
					drc_append_set_temp_sse_rounding(drc, FPRND_NEAR);						// ldmxcsr [round]
					if (IS_SINGLE(op))	/* ROUND.W.S */
						emit_cvtss2si_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(FSREG));			// cvtss2si eax,[fsreg]
					else				/* ROUND.W.D */
						emit_cvtsd2si_r32_m64(DRCTOP, REG_EAX, FPR64ADDR(FSREG));			// cvtsd2si eax,[fsreg]
					emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);					// mov  [fdreg],eax
					drc_append_restore_sse_rounding(drc);									// ldmxcsr [mode]
					return TRUE;

				case 0x0d:
					if (IS_SINGLE(op))	/* TRUNC.W.S */
						emit_cvttss2si_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(FSREG));			// cvttss2si eax,[fsreg]
					else				/* TRUNC.W.D */
						emit_cvttsd2si_r32_m64(DRCTOP, REG_EAX, FPR64ADDR(FSREG));			// cvttsd2si eax,[fsreg]
					emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);					// mov  [fdreg],eax
					return TRUE;

				case 0x0e:
					/* FUTURE: On Penryn, we can use ROUNDPS/ROUNDPD */
					drc_append_set_temp_sse_rounding(drc, FPRND_UP);						// ldmxcsr [round]
					if (IS_SINGLE(op))	/* CEIL.W.S */
						emit_cvtss2si_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(FSREG));			// cvtss2si eax,[fsreg]
					else				/* CEIL.W.D */
						emit_cvtsd2si_r32_m64(DRCTOP, REG_EAX, FPR64ADDR(FSREG));			// cvtsd2si eax,[fsreg]
					emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);					// mov  [fdreg],eax
					drc_append_restore_sse_rounding(drc);									// ldmxcsr [mode]
					return TRUE;

				case 0x0f:
					/* FUTURE: On Penryn, we can use ROUNDPS/ROUNDPD */
					drc_append_set_temp_sse_rounding(drc, FPRND_DOWN);						// ldmxcsr [round]
					if (IS_SINGLE(op))	/* FLOOR.W.S */
						emit_cvtss2si_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(FSREG));			// cvtss2si eax,[fsreg]
					else				/* FLOOR.W.D */
						emit_cvtsd2si_r32_m64(DRCTOP, REG_EAX, FPR64ADDR(FSREG));			// cvtsd2si eax,[fsreg]
					emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);					// mov  [fdreg],eax
					drc_append_restore_sse_rounding(drc);									// ldmxcsr [mode]
					return TRUE;

				case 0x11:	/* R5000 */
					emit_cmp_m8_imm(DRCTOP, CF1ADDR((op >> 18) & 7), 0);					// cmp  fcc[which],0
					emit_jcc_short_link(DRCTOP, ((op >> 16) & 1) ? COND_Z : COND_NZ, &link1);
																							// jz/nz skip
					if (IS_SINGLE(op))	/* MOVT/F.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* MOVT/F.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					resolve_link(DRCTOP, &link1);										// skip:
					return TRUE;

				case 0x12:	/* R5000 */
					emit_cmp_m64_imm(DRCTOP, REGADDR(RTREG), 0);							// cmp  [X86REG(rt)],0
					emit_jcc_short_link(DRCTOP, COND_NZ, &link1);							// jnz  skip
					if (IS_SINGLE(op))	/* MOVZ.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* MOVZ.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					resolve_link(DRCTOP, &link1);										// skip:
					return TRUE;

				case 0x13:	/* R5000 */
					emit_cmp_m64_imm(DRCTOP, REGADDR(RTREG), 0);							// cmp  [X86REG(rt)],0
					emit_jcc_short_link(DRCTOP, COND_Z, &link1);							// jz  skip
					if (IS_SINGLE(op))	/* MOVN.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* MOVN.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					resolve_link(DRCTOP, &link1);										// skip:
					return TRUE;

				case 0x15:	/* R5000 */
					if (IS_SINGLE(op))	/* RECIP.S */
					{
						emit_rcpss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// rcpss xmm0,[fsreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* RECIP.D */
					{
						emit_cvtsd2ss_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// cvtsd2ss xmm0,[fsreg]
						emit_rcpss_r128_r128(DRCTOP, REG_XMM0, REG_XMM0);					// rcpss xmm0,xmm0
						emit_cvtss2sd_r128_r128(DRCTOP, REG_XMM0, REG_XMM0);				// cvtss2sd xmm0,xmm0
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x16:	/* R5000 */
					if (IS_SINGLE(op))	/* RSQRT.S */
					{
						emit_rsqrtss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// rsqrtss xmm0,[fsreg]
						emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);			// movss [fdreg],xmm0
					}
					else				/* RSQRT.D */
					{
						emit_cvtsd2ss_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// cvtss2sd xmm0,[fsreg]
						emit_rsqrtss_r128_r128(DRCTOP, REG_XMM0, REG_XMM0);					// rsqrtss xmm0,xmm0
						emit_cvtss2sd_r128_r128(DRCTOP, REG_XMM0, REG_XMM0);				// cvtsd2ss xmm0,xmm0
						emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);			// movsd [fdreg],xmm0
					}
					return TRUE;

				case 0x20:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.S.W */
							emit_cvtsi2ss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// cvtsi2ss xmm0,[fsreg]
						else				/* CVT.S.L */
							emit_cvtsi2ss_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// cvtsi2ss xmm0,[fsreg]
					}
					else					/* CVT.S.D */
						emit_cvtsd2ss_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// cvtsd2ss xmm0,[fsreg]
					emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);				// movss [fdreg],xmm0
					return TRUE;

				case 0x21:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.D.W */
							emit_cvtsi2sd_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));		// cvtsi2sd xmm0,[fsreg]
						else				/* CVT.D.L */
							emit_cvtsi2sd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));		// cvtsi2sd xmm0,[fsreg]
					}
					else					/* CVT.D.S */
						emit_cvtss2sd_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// cvtss2sd xmm0,[fsreg]
					emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);				// movsd [fdreg],xmm0
					return TRUE;

				case 0x24:
					if (IS_SINGLE(op))	/* CVT.W.S */
						emit_cvtss2si_r32_m32(DRCTOP, REG_EAX, FPR32ADDR(FSREG));			// cvtss2si eax,[fsreg]
					else				/* CVT.W.D */
						emit_cvtsd2si_r32_m64(DRCTOP, REG_EAX, FPR64ADDR(FSREG));			// cvtsd2si eax,[fsreg]
					emit_mov_m32_r32(DRCTOP, FPR32ADDR(FDREG), REG_EAX);					// mov  [fdreg],eax
					return TRUE;

				case 0x25:
					if (IS_SINGLE(op))	/* CVT.L.S */
						emit_cvtss2si_r64_m32(DRCTOP, REG_RAX, FPR32ADDR(FSREG));			// cvtss2si rax,[fsreg]
					else				/* CVT.L.D */
						emit_cvtsd2si_r64_m64(DRCTOP, REG_RAX, FPR64ADDR(FSREG));			// cvtsd2si rax,[fsreg]
					emit_mov_m64_r64(DRCTOP, FPR32ADDR(FDREG), REG_RAX);					// mov  [fdreg],rax
					return TRUE;

				case 0x30:
				case 0x38:
					emit_mov_m8_imm(DRCTOP, CF1ADDR((op >> 8) & 7), 0);						/* C.F.S/D */
					return TRUE;

				case 0x31:
				case 0x39:
					emit_mov_m8_imm(DRCTOP, CF1ADDR((op >> 8) & 7), 0);						/* C.F.S/D */
					return TRUE;

				case 0x32:
				case 0x3a:
					if (IS_SINGLE(op))	/* C.EQ.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_comiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// comiss xmm0,[ftreg]
					}
					else				/* C.EQ.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_comisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// comisd xmm0,[ftreg]
					}
					emit_setcc_m8(DRCTOP, COND_E, CF1ADDR((op >> 8) & 7));					// sete fcc[x]
					return TRUE;

				case 0x33:
				case 0x3b:
					if (IS_SINGLE(op))	/* C.UEQ.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_ucomiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// ucomiss xmm0,[ftreg]
					}
					else				/* C.UEQ.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_ucomisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// ucomisd xmm0,[ftreg]
					}
					emit_setcc_m8(DRCTOP, COND_E, CF1ADDR((op >> 8) & 7));					// sete fcc[x]
					return TRUE;

				case 0x34:
				case 0x3c:
					if (IS_SINGLE(op))	/* C.OLT.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_comiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// comiss xmm0,[ftreg]
					}
					else				/* C.OLT.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_comisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// comisd xmm0,[ftreg]
					}
					emit_setcc_m8(DRCTOP, COND_B, CF1ADDR((op >> 8) & 7));					// setb fcc[x]
					return TRUE;

				case 0x35:
				case 0x3d:
					if (IS_SINGLE(op))	/* C.ULT.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_ucomiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// ucomiss xmm0,[ftreg]
					}
					else				/* C.ULT.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_ucomisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// ucomisd xmm0,[ftreg]
					}
					emit_setcc_m8(DRCTOP, COND_B, CF1ADDR((op >> 8) & 7));					// setb fcc[x]
					return TRUE;

				case 0x36:
				case 0x3e:
					if (IS_SINGLE(op))	/* C.OLE.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_comiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// comiss xmm0,[ftreg]
					}
					else				/* C.OLE.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_comisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// comisd xmm0,[ftreg]
					}
					emit_setcc_m8(DRCTOP, COND_BE, CF1ADDR((op >> 8) & 7));					// setbe fcc[x]
					return TRUE;

				case 0x37:
				case 0x3f:
					if (IS_SINGLE(op))	/* C.ULE.S */
					{
						emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));			// movss xmm0,[fsreg]
						emit_ucomiss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));			// ucomiss xmm0,[ftreg]
					}
					else				/* C.ULE.D */
					{
						emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));			// movsd xmm0,[fsreg]
						emit_ucomisd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));			// ucomisd xmm0,[ftreg]
					}
					emit_setcc_m8(DRCTOP, COND_BE, CF1ADDR((op >> 8) & 7));					// setbe fcc[x]
					return TRUE;
			}
			break;
	}
	return FALSE;
}



/***************************************************************************
    COP1X RECOMPILATION
***************************************************************************/

/*-------------------------------------------------
    compile_cop1x - compile COP1X opcodes
-------------------------------------------------*/

static int compile_cop1x(drc_core *drc, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;

	if (mips3.drcoptions & MIPS3DRC_STRICT_COP1)
	{
		emit_test_m32_imm(DRCTOP, CPR0ADDR(COP0_Status), SR_COP1);							// test [mips3.core->cpr[0][COP0_Status]],SR_COP1
		oob_request_callback(drc, COND_Z, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_cop_exception);
																							// jz   generate_cop_exception
	}

	switch (op & 0x3f)
	{
		case 0x00:		/* LWXC1 */
			return compile_loadx_cop(drc, compiler, desc, 4, FPR32(FDREG));

		case 0x01:		/* LDXC1 */
			return compile_loadx_cop(drc, compiler, desc, 8, FPR64(FDREG));

		case 0x08:		/* SWXC1 */
			return compile_storex_cop(drc, compiler, desc, 4, FPR32(FDREG));

		case 0x09:		/* SDXC1 */
			return compile_storex_cop(drc, compiler, desc, 8, FPR64(FDREG));

		case 0x0f:		/* PREFX */
			return TRUE;

		case 0x20:		/* MADD.S */
			emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));						// movss xmm0,[fsreg]
			emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));						// mulss xmm0,[ftreg]
			emit_addss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FRREG));						// addss xmm0,[frreg]
			emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);						// movss [fdreg],xmm0
			return TRUE;

		case 0x21:		/* MADD.D */
			emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));						// movsd xmm0,[fsreg]
			emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));						// mulsd xmm0,[ftreg]
			emit_addsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FRREG));						// addsd xmm0,[frreg]
			emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);						// movsd [fdreg],xmm0
			return TRUE;

		case 0x28:		/* MSUB.S */
			emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));						// movss xmm0,[fsreg]
			emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));						// mulss xmm0,[ftreg]
			emit_subss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FRREG));						// subss xmm0,[frreg]
			emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM0);						// movss [fdreg],xmm0
			return TRUE;

		case 0x29:		/* MSUB.D */
			emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));						// movsd xmm0,[fsreg]
			emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));						// mulsd xmm0,[ftreg]
			emit_subsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FRREG));						// subsd xmm0,[frreg]
			emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM0);						// movsd [fdreg],xmm0
			return TRUE;

		case 0x30:		/* NMADD.S */
			emit_xorps_r128_r128(DRCTOP, REG_XMM1, REG_XMM1);								// xorps xmm1,xmm1
			emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));						// movss xmm0,[fsreg]
			emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));						// mulss xmm0,[ftreg]
			emit_addss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FRREG));						// addss xmm0,[frreg]
			emit_subss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);								// subss xmm1,xmm0
			emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM1);						// movss [fdreg],xmm1
			return TRUE;

		case 0x31:		/* NMADD.D */
			emit_xorps_r128_r128(DRCTOP, REG_XMM1, REG_XMM1);								// xorps xmm1,xmm1
			emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));						// movsd xmm0,[fsreg]
			emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));						// mulsd xmm0,[ftreg]
			emit_addsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FRREG));						// addsd xmm0,[frreg]
			emit_subsd_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);								// subsd xmm1,xmm0
			emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM1);						// movsd [fdreg],xmm1
			return TRUE;

		case 0x38:		/* NMSUB.S */
			emit_movss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FSREG));						// movss xmm0,[fsreg]
			emit_mulss_r128_m32(DRCTOP, REG_XMM0, FPR32ADDR(FTREG));						// mulss xmm0,[ftreg]
			emit_movss_r128_m32(DRCTOP, REG_XMM1, FPR32ADDR(FRREG));						// movss xmm1,[frreg]
			emit_subss_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);								// subss xmm1,xmm0
			emit_movss_m32_r128(DRCTOP, FPR32ADDR(FDREG), REG_XMM1);						// movss [fdreg],xmm1
			return TRUE;

		case 0x39:		/* NMSUB.D */
			emit_movsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FSREG));						// movsd xmm0,[fsreg]
			emit_mulsd_r128_m64(DRCTOP, REG_XMM0, FPR64ADDR(FTREG));						// mulsd xmm0,[ftreg]
			emit_movsd_r128_m64(DRCTOP, REG_XMM1, FPR64ADDR(FRREG));						// movsd xmm1,[frreg]
			emit_subsd_r128_r128(DRCTOP, REG_XMM1, REG_XMM0);								// subsd xmm1,xmm0
			emit_movsd_m64_r128(DRCTOP, FPR64ADDR(FDREG), REG_XMM1);						// movsd [fdreg],xmm1
			return TRUE;

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
	return FALSE;
}



/***************************************************************************
    INDIVIDUAL INSTRUCTION COMPILATION
***************************************************************************/

/*-------------------------------------------------
    compile_shift - compile a 32-bit
    shift, both fixed and variable counts

    0: sll/sllv rD,rT,rS
    2: srl/srlv rD,rT,rS
    3: sra/srav rD,rT,rS
-------------------------------------------------*/

static int compile_shift(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int shift_type, int shift_variable)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rd, rs, rt;

	assert(shift_type == 0 || shift_type == 2 || shift_type == 3);

	/* if the destination is r0, bail */
	if (RDREG == 0)
		return TRUE;

	/* allocate source registers */
	rs = shift_variable ? compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC) : MAKE_CONST(SHIFT);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if rS and rT are both constant, we can compute the constant result */
	if (ISCONST(rs) && ISCONST(rt))
	{
		if (shift_type == 0)
			emit_set_constant_value(drc, compiler, desc, RDREG, (INT32)((UINT32)CONSTVAL(rt) << CONSTVAL(rs)));
		else if (shift_type == 2)
			emit_set_constant_value(drc, compiler, desc, RDREG, (INT32)((UINT32)CONSTVAL(rt) >> CONSTVAL(rs)));
		else
			emit_set_constant_value(drc, compiler, desc, RDREG, (INT32)((INT32)CONSTVAL(rt) >> CONSTVAL(rs)));
	}

	/* otherwise, handle generically */
	else
	{
		/* allocate the register and load rT into it */
		/* if rS is non-const, must load it first in case rD == rS */
		rd = compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_DST | ALLOC_DIRTY);
		if (!ISCONST(rs))
			emit_load_register_value(drc, compiler, REG_ECX, rs);							// mov  ecx,<rs>
		emit_load_register_value(drc, compiler, X86REG(rd), rt);							// mov  <rd>,<rt>

		/* if rs is constant, use immediate forms of the shift */
		if (ISCONST(rs))
		{
			if (shift_type == 0)
				emit_shl_r32_imm(DRCTOP, X86REG(rd), CONSTVAL(rs) & 31);					// shl  <rd>,<rs>
			else if (shift_type == 2)
				emit_shr_r32_imm(DRCTOP, X86REG(rd), CONSTVAL(rs) & 31);					// shr  <rd>,<rs>
			else
				emit_sar_r32_imm(DRCTOP, X86REG(rd), CONSTVAL(rs) & 31);					// sar  <rd>,<rs>
		}

		/* otherwise, load rs into cl so we can do a variable shift */
		else
		{
			if (shift_type == 0)
				emit_shl_r32_cl(DRCTOP, X86REG(rd));										// shl  <rd>,cl
			else if (shift_type == 2)
				emit_shr_r32_cl(DRCTOP, X86REG(rd));										// shr  <rd>,cl
			else
				emit_sar_r32_cl(DRCTOP, X86REG(rd));										// sar  <rd>,cl
		}
		emit_movsxd_r64_r32(DRCTOP, X86REG(rd), X86REG(rd));								// movsxd <rd>,<rd>
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_dshift - compile a 64-bit
    shift, both fixed and variable counts

    0: dsll/dsll32/dsllv rD,rT,rS
    2: dsrl/dsrl32/dsrlv rD,rT,rS
    3: dsra/dsra32/dsrav rD,rT,rS
-------------------------------------------------*/

static int compile_dshift(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int shift_type, int shift_variable, int shift_const_plus32)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rd, rs, rt;

	assert(shift_type == 0 || shift_type == 2 || shift_type == 3);

	/* if the destination is r0, bail */
	if (RDREG == 0)
		return TRUE;

	/* allocate source registers */
	rs = shift_variable ? compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC) : MAKE_CONST(SHIFT + (shift_const_plus32 ? 32 : 0));
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if rS and rT are both constant, we can compute the constant result */
	if (ISCONST(rs) && ISCONST(rt))
	{
		if (shift_type == 0)
			emit_set_constant_value(drc, compiler, desc, RDREG, (UINT64)CONSTVAL(rt) << CONSTVAL(rs));
		else if (shift_type == 2)
			emit_set_constant_value(drc, compiler, desc, RDREG, (UINT64)CONSTVAL(rt) >> CONSTVAL(rs));
		else
			emit_set_constant_value(drc, compiler, desc, RDREG, (INT64)CONSTVAL(rt) >> CONSTVAL(rs));
	}

	/* otherwise, handle generically */
	else
	{
		/* allocate the register and load rT into it */
		/* if rS is non-const, must load it first in case rD == rS */
		rd = compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_DST | ALLOC_DIRTY);
		if (!ISCONST(rs))
			emit_load_register_value(drc, compiler, REG_ECX, rs);							// mov  ecx,<rs>
		emit_load_register_value(drc, compiler, X86REG(rd), rt);							// mov  <rd>,<rt>

		/* if rs is constant, use immediate forms of the shift */
		if (ISCONST(rs))
		{
			if (shift_type == 0)
				emit_shl_r64_imm(DRCTOP, X86REG(rd), CONSTVAL(rs) & 63);					// shl  <rd>,<rs>
			else if (shift_type == 2)
				emit_shr_r64_imm(DRCTOP, X86REG(rd), CONSTVAL(rs) & 63);					// shr  <rd>,<rs>
			else
				emit_sar_r64_imm(DRCTOP, X86REG(rd), CONSTVAL(rs) & 63);					// sar  <rd>,<rs>
		}

		/* otherwise, load rs into cl so we can do a variable shift */
		else
		{
			if (shift_type == 0)
				emit_shl_r64_cl(DRCTOP, X86REG(rd));										// shl  <rd>,cl
			else if (shift_type == 2)
				emit_shr_r64_cl(DRCTOP, X86REG(rd));										// shr  <rd>,cl
			else
				emit_sar_r64_cl(DRCTOP, X86REG(rd));										// sar  <rd>,cl
		}
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_movzn - compile a conditional
    move based on compare against 0

    movz/movn rD,rS,rT
-------------------------------------------------*/

static int compile_movzn(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_movn)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rd, rs, rt;
	emit_link skip = { 0 };

	/* if the destination is r0, bail */
	if (RDREG == 0)
		return TRUE;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);
	rd = compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_SRC | ALLOC_DST | ALLOC_DIRTY);

	/* if rT is constant and fails the condition, we're done */
	if (ISCONST(rt))
	{
		INT32 rtval = CONSTVAL(rt);
		if ((is_movn && rtval == 0) || (!is_movn && rtval != 0))
			return TRUE;
	}

	/* if rT is not constant, check the condition */
	else
	{
		emit_test_r64_r64(DRCTOP, X86REG(rt), X86REG(rt));									// test <rt>,<rt>
		emit_jcc_short_link(DRCTOP, is_movn ? COND_Z : COND_NZ, &skip);						// jz/nz skip
	}

	/* load rS into rD */
	emit_load_register_value(drc, compiler, X86REG(rd), rs);								// mov  <rd>,<rs>
	if (!ISCONST(rt))
		resolve_link(DRCTOP, &skip);													// skip:

	return TRUE;
}


/*-------------------------------------------------
    compile_trapcc - compile a conditional
    trap comparing two registers

    0/1: tge/tgeu rS,rT / tgei/tgeiu rS,SIMM
    2/3: tlt/tltu rS,rT / tlti/tltiu rS,SIMM
      4: teq      rS,rT / teqi       rS,SIMM
      6: tne      rS,rT / tnei       rS,SIMM
-------------------------------------------------*/

static int compile_trapcc(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int condition, int immediate)
{
	static const UINT8 condtype[] = { COND_GE, COND_AE, COND_L, COND_B, COND_E, COND_NONE, COND_NE, COND_NONE };
	UINT32 op = *desc->opptr.l;
	mipsreg_state rs, rt;

	assert(condition < ARRAY_LENGTH(condtype) && condtype[condition] != COND_NONE);

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = immediate ? MAKE_CONST(SIMMVAL) : compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		INT32 rsval = CONSTVAL(rs);
		INT32 rtval = CONSTVAL(rt);
		int result = FALSE;

		/* determine the fixed answer */
		switch (condition)
		{
			case 0:		result = ((INT64)rsval >= (INT64)rtval);		break;
			case 1:		result = ((UINT64)rsval >= (UINT64)rtval);		break;
			case 2:		result = ((INT64)rsval < (INT64)rtval);			break;
			case 3:		result = ((UINT64)rsval < (UINT64)rtval);		break;
			case 4:		result = (rsval == rtval);						break;
			case 6:		result = (rsval != rtval);						break;
		}

		/* if we're not trapping, just return; otherwise, always trap */
		if (!result)
			return TRUE;
		condition = COND_NONE;
	}

	/* if rT is constant do an immediate compare */
	else if (ISCONST(rt))
	{
		emit_cmp_r64_imm(DRCTOP, X86REG(rs), CONSTVAL(rt));									// cmp  <rs>,<rt>
		condition = condtype[condition];
	}

	/* if rS is constant do an immediate compare with opposite condition */
	else if (ISCONST(rs))
	{
		emit_cmp_r64_imm(DRCTOP, X86REG(rt), CONSTVAL(rs));									// cmp  <rt>,<rs>
		condition = condtype[condition] ^ 1;
	}

	/* otherwise, compare the two registers with a normal condition */
	else
	{
		emit_cmp_r64_imm(DRCTOP, X86REG(rs), X86REG(rt));									// cmp  <rs>,<rt>
		condition = condtype[condition];
	}

	/* generate an OOB request if the trap is taken */
	oob_request_callback(drc, condition, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_trap_exception);
	return TRUE;
}


/*-------------------------------------------------
    compile_branchcc - compile a conditional
    branch comparing two registers

    beq rS,rT,dest
    bne rS,rT,dest
-------------------------------------------------*/

static int compile_branchcc(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int condition)
{
	UINT32 op = *desc->opptr.l;
	emit_link skip = { 0 };
	mipsreg_state rs, rt;

	assert(condition == COND_E || condition == COND_NE);

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		INT32 rsval = CONSTVAL(rs);
		INT32 rtval = CONSTVAL(rt);
		if ((condition == COND_E && rsval != rtval) || (condition == COND_NE && rsval == rtval))
			return TRUE;
	}

	/* if rT is constant do an immediate compare */
	else if (ISCONST(rt))
	{
		emit_cmp_r64_imm(DRCTOP, X86REG(rs), CONSTVAL(rt));									// cmp  <rs>,<rt>
		emit_jcc_near_link(DRCTOP, condition ^ 1, &skip);									// jncc skip
	}

	/* if rS is constant do an immediate compare */
	else if (ISCONST(rs))
	{
		emit_cmp_r64_imm(DRCTOP, X86REG(rt), CONSTVAL(rs));									// cmp  <rt>,<rs>
		emit_jcc_near_link(DRCTOP, condition ^ 1, &skip);									// jncc skip
	}

	/* otherwise, compare the two registers with a normal condition */
	else
	{
		emit_cmp_r64_r64(DRCTOP, X86REG(rs), X86REG(rt));									// cmp  <rs>,<rt>
		emit_jcc_near_link(DRCTOP, condition ^ 1, &skip);									// jncc skip
	}

	/* append the branch logic */
	emit_delay_slot_and_branch(drc, compiler, desc, 0);										// <next instruction + redispatch>

	if (!(ISCONST(rs) && ISCONST(rt)))
		resolve_link(DRCTOP, &skip);													// skip:
	return TRUE;
}


/*-------------------------------------------------
    compile_branchcz - compile a conditional
    branch comparing one register against 0

    blez    rS,dest
    blezl   rS,dest
    bgtz    rS,dest
    bgtzl   rS,dest
    bltz    rS,dest
    bltzl   rS,dest
    bltzal  rS,dest
    bltzall rS,dest
    bgez    rS,dest
    bgezl   rS,dest
    bgezal  rS,dest
    bgezall rS,dest
-------------------------------------------------*/

static int compile_branchcz(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int condition, int link)
{
	UINT32 op = *desc->opptr.l;
	emit_link skip = { 0 };
	mipsreg_state rs;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);

	/* if the register is constant, we know the answer up front */
	if (ISCONST(rs))
	{
		INT32 rsval = CONSTVAL(rs);
		if ((condition == COND_LE && rsval > 0) || (condition == COND_G && rsval <= 0) ||
			(condition == COND_L && rsval >= 0) || (condition == COND_GE && rsval < 0))
			return TRUE;
	}

	/* otherwise, perform the compare */
	else
	{
		emit_cmp_r64_imm(DRCTOP, X86REG(rs), 0);											// cmp  <rs>,0
		emit_jcc_near_link(DRCTOP, condition ^ 1, &skip);									// jncc skip
	}

	/* append the branch logic */
	emit_delay_slot_and_branch(drc, compiler, desc, link ? 31 : 0);							// <next instruction + redispatch>

	if (!ISCONST(rs))
		resolve_link(DRCTOP, &skip);													// skip:
	return TRUE;
}


/*-------------------------------------------------
    compile_add - compile an add with or without
    overflow

    add  rD,rS,rT / addi  rT,rS,SIMM
    addu rD,rS,rT / addiu rT,rS,SIMM
-------------------------------------------------*/

static int compile_add(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int suppress_exceptions, int immediate)
{
	UINT32 op = *desc->opptr.l;
	UINT8 destreg = immediate ? RTREG : RDREG;
	mipsreg_state rd, rs, rt;

	/* note that we still must perform this operation even if destreg == 0
       in the overflow case */
	if (destreg == 0 && suppress_exceptions)
		return TRUE;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = immediate ? MAKE_CONST(SIMMVAL) : compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		INT64 result = (INT64)CONSTVAL(rs) + (INT64)CONSTVAL(rt);

		/* if we overflow, always generate an exception */
		if (!suppress_exceptions && (INT32)result != result)
			oob_request_callback(drc, COND_NONE, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_overflow_exception);

		/* else just set the new constant value */
		else if (destreg != 0)
			emit_set_constant_value(drc, compiler, desc, destreg, (INT32)result);
	}

	/* otherwise, handle generically */
	else
	{
		rd = compiler_register_allocate(drc, compiler, desc, destreg, ALLOC_DST);

		/* perform the operating in EAX so we don't trash rD if an exception happens */
		if (!suppress_exceptions)
		{
			emit_load_register_value(drc, compiler, REG_EAX, rs);							// mov  eax,<rs>
			if (!ISCONST(rt))
				emit_add_r32_r32(DRCTOP, REG_EAX, X86REG(rt));								// add  eax,<rt>
			else if (CONSTVAL(rt) != 0)
				emit_add_r32_imm(DRCTOP, REG_EAX, CONSTVAL(rt));							// add  eax,<rt>
			oob_request_callback(drc, COND_O, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_overflow_exception);
																							// jo   generate_overflow_exception
			if (destreg != 0)
				emit_movsxd_r64_r32(DRCTOP, X86REG(rd), REG_EAX);							// movsxd <rd>,eax
		}
		else
		{
			if (ISCONST(rs) && CONSTVAL(rs) == 0)
				emit_movsxd_r64_r32(DRCTOP, X86REG(rd), X86REG(rt));						// movsxd <rd>,<rt>
			else if (ISCONST(rt) && CONSTVAL(rt) == 0)
				emit_movsxd_r64_r32(DRCTOP, X86REG(rd), X86REG(rs));						// movsxd <rd>,<rs>
			else if (ISCONST(rs))
			{
				emit_lea_r32_m32(DRCTOP, X86REG(rd), MBD(X86REG(rt), CONSTVAL(rs)));		// lea  <rd>,[<rt> + <rs>]
				emit_movsxd_r64_r32(DRCTOP, X86REG(rd), X86REG(rd));						// movsxd <rd>,<rd>
			}
			else if (ISCONST(rt))
			{
				emit_lea_r32_m32(DRCTOP, X86REG(rd), MBD(X86REG(rs), CONSTVAL(rt)));		// lea  <rd>,[<rs> + <rt>]
				emit_movsxd_r64_r32(DRCTOP, X86REG(rd), X86REG(rd));						// movsxd <rd>,<rd>
			}
			else
			{
				emit_lea_r32_m32(DRCTOP, X86REG(rd), MBISD(X86REG(rs), X86REG(rt), 1, 0));	// lea  <rd>,[<rs> + <rt>]
				emit_movsxd_r64_r32(DRCTOP, X86REG(rd), X86REG(rd));						// movsxd <rd>,<rd>
			}
		}

		/* only mark it dirty after the exception risk is over */
		compiler_register_allocate(drc, compiler, desc, destreg, ALLOC_DIRTY);
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_dadd - compile an add with or without
    overflow

    dadd  rD,rS,rT
    daddu rD,rS,rT
-------------------------------------------------*/

static int compile_dadd(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int suppress_exceptions, int immediate)
{
	UINT32 op = *desc->opptr.l;
	UINT8 destreg = immediate ? RTREG : RDREG;
	mipsreg_state rd, rs, rt;

	/* note that we still must perform this operation even if destreg == 0
       in the overflow case */
	if (destreg == 0 && suppress_exceptions)
		return TRUE;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = immediate ? MAKE_CONST(SIMMVAL) : compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		INT64 result = (INT64)CONSTVAL(rs) + (INT64)CONSTVAL(rt);

		/* if we overflow, always generate an exception */
		if (!suppress_exceptions && (INT32)result != result)
			oob_request_callback(drc, COND_NONE, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_overflow_exception);

		/* else just set the new constant value */
		else if (destreg != 0)
			emit_set_constant_value(drc, compiler, desc, destreg, result);
	}

	/* otherwise, handle generically */
	else
	{
		rd = compiler_register_allocate(drc, compiler, desc, destreg, ALLOC_DST);

		/* perform the operating in EAX so we don't trash RTREG if an exception happens */
		if (!suppress_exceptions)
		{
			emit_load_register_value(drc, compiler, REG_RAX, rs);							// mov  rax,<rs>
			if (!ISCONST(rt))
				emit_add_r64_r64(DRCTOP, REG_RAX, X86REG(rt));								// add  rax,<rt>
			else if (CONSTVAL(rt) != 0)
				emit_add_r64_imm(DRCTOP, REG_RAX, CONSTVAL(rt));							// add  rax,<rt>
			oob_request_callback(drc, COND_O, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_overflow_exception);
																							// jo   generate_overflow_exception
			if (destreg != 0)
				emit_mov_r64_r64(DRCTOP, X86REG(rd), REG_RAX);								// mov  <rd>,rax
		}
		else
		{
			if (ISCONST(rs))
				emit_lea_r64_m64(DRCTOP, X86REG(rd), MBD(X86REG(rt), CONSTVAL(rs)));		// lea  <rd>,[<rt> + <rs>]
			else if (ISCONST(rt))
				emit_lea_r64_m64(DRCTOP, X86REG(rd), MBD(X86REG(rs), CONSTVAL(rt)));		// lea  <rd>,[<rs> + <rt>]
			else
				emit_lea_r64_m64(DRCTOP, X86REG(rd), MBISD(X86REG(rs), X86REG(rt), 1, 0));	// lea  <rd>,[<rs> + <rt>]
		}

		/* only mark it dirty after the exception risk is over */
		compiler_register_allocate(drc, compiler, desc, destreg, ALLOC_DIRTY);
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_sub - compile an add with or without
    overflow

    sub  rD,rS,rT
    subu rD,rS,rT
-------------------------------------------------*/

static int compile_sub(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int suppress_exceptions)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rd, rs, rt;

	/* note that we still must perform this operation even if RDREG == 0
       in the overflow case */
	if (RDREG == 0 && suppress_exceptions)
		return TRUE;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		INT64 result = (INT64)CONSTVAL(rs) - (INT64)CONSTVAL(rt);

		/* if we overflow, always generate an exception */
		if (!suppress_exceptions && (INT32)result != result)
			oob_request_callback(drc, COND_NONE, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_overflow_exception);

		/* else just set the new constant value */
		else if (RDREG != 0)
			emit_set_constant_value(drc, compiler, desc, RDREG, (INT32)result);
	}

	/* if rS == rT, the result is 0 */
	else if (RSREG == RTREG)
	{
		if (RDREG != 0)
			emit_set_constant_value(drc, compiler, desc, RDREG, 0);
	}

	/* otherwise, handle generically */
	else
	{
		rd = compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_DST);

		/* perform the operating in EAX so we don't trash RTREG if an exception happens */
		if (!suppress_exceptions)
		{
			emit_load_register_value(drc, compiler, REG_EAX, rs);							// mov  eax,<rs>
			if (!ISCONST(rt))
				emit_sub_r32_r32(DRCTOP, REG_EAX, X86REG(rt));								// sub  eax,<rt>
			else if (CONSTVAL(rt) != 0)
				emit_sub_r32_imm(DRCTOP, REG_EAX, CONSTVAL(rt));							// sub  eax,<rt>
			if (!ISCONST(rt) || CONSTVAL(rt) != 0)
				oob_request_callback(drc, COND_O, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_overflow_exception);
																							// jo   generate_overflow_exception
			if (RDREG != 0)
				emit_movsxd_r64_r32(DRCTOP, X86REG(rd), REG_EAX);							// movsxd <rd>,eax
		}
		else
		{
			/* if rD and rT aren't the same, it is pretty basic */
			if (X86REG(rd) != X86REG(rt))
			{
				emit_load_register_value(drc, compiler, X86REG(rd), rs);					// mov  <rd>,<rs>
				if (!ISCONST(rt))
					emit_sub_r32_r32(DRCTOP, X86REG(rd), X86REG(rt));						// sub  <rd>,<rt>
				else if (CONSTVAL(rt) != 0)
					emit_sub_r32_imm(DRCTOP, X86REG(rd), CONSTVAL(rt));						// sub  <rd>,<rt>
			}

			/* otherwise, we negate rT and add rS */
			else
			{
				emit_neg_r32(DRCTOP, X86REG(rd));											// neg  <rd>
				if (!ISCONST(rs))
					emit_add_r32_r32(DRCTOP, X86REG(rd), X86REG(rs));						// add  <rd>,<rs>
				else if (CONSTVAL(rs) != 0)
					emit_add_r32_imm(DRCTOP, X86REG(rd), CONSTVAL(rs));						// add  <rd>,<rs>
			}
			emit_movsxd_r64_r32(DRCTOP, X86REG(rd), X86REG(rd));							// movsxd <rd>,<rd>
		}

		/* only mark it dirty after the exception risk is over */
		compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_DIRTY);
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_dsub - compile an add with or without
    overflow

    dsub  rD,rS,rT
    dsubu rD,rS,rT
-------------------------------------------------*/

static int compile_dsub(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int suppress_exceptions)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rd, rs, rt;

	/* note that we still must perform this operation even if RDREG == 0
       in the overflow case */
	if (RDREG == 0 && suppress_exceptions)
		return TRUE;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		INT64 result = (INT64)CONSTVAL(rs) - (INT64)CONSTVAL(rt);

		/* if we overflow, always generate an exception */
		if (!suppress_exceptions && (INT32)result != result)
			oob_request_callback(drc, COND_NONE, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_overflow_exception);

		/* else just set the new constant value */
		else if (RDREG != 0)
			emit_set_constant_value(drc, compiler, desc, RDREG, result);
	}

	/* if rS == rT, the result is 0 */
	else if (RSREG == RTREG)
	{
		if (RDREG != 0)
			emit_set_constant_value(drc, compiler, desc, RDREG, 0);
	}

	/* otherwise, handle generically */
	else
	{
		rd = compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_DST);

		/* perform the operating in EAX so we don't trash RTREG if an exception happens */
		if (!suppress_exceptions)
		{
			emit_load_register_value(drc, compiler, REG_EAX, rs);						// mov  eax,<rs>
			if (!ISCONST(rt))
				emit_sub_r64_r64(DRCTOP, REG_RAX, X86REG(rt));								// sub  rax,<rt>
			else if (CONSTVAL(rt) != 0)
				emit_sub_r64_imm(DRCTOP, REG_RAX, CONSTVAL(rt));							// sub  rax,<rt>
			if (!ISCONST(rt) || CONSTVAL(rt) != 0)
				oob_request_callback(drc, COND_O, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_overflow_exception);
																							// jo   generate_overflow_exception
			if (RDREG != 0)
				emit_mov_r64_r64(DRCTOP, X86REG(rd), REG_RAX);								// mov  <rd>,rax
		}
		else
		{
			/* if rD and rT aren't the same, it is pretty basic */
			if (X86REG(rd) != X86REG(rt))
			{
				emit_load_register_value(drc, compiler, X86REG(rd), rs);					// mov  <rd>,<rs>
				if (!ISCONST(rt))
					emit_sub_r64_r64(DRCTOP, X86REG(rd), X86REG(rt));						// sub  <rd>,<rt>
				else if (CONSTVAL(rt) != 0)
					emit_sub_r64_imm(DRCTOP, X86REG(rd), CONSTVAL(rt));						// sub  <rd>,<rt>
			}

			/* otherwise, we negate rT and add rS */
			else
			{
				emit_neg_r64(DRCTOP, X86REG(rd));											// neg  <rd>
				if (!ISCONST(rs))
					emit_add_r64_r64(DRCTOP, X86REG(rd), X86REG(rs));						// add  <rd>,<rs>
				else if (CONSTVAL(rs) != 0)
					emit_add_r64_imm(DRCTOP, X86REG(rd), CONSTVAL(rs));						// add  <rd>,<rs>
			}
		}

		/* only mark it dirty after the exception risk is over */
		compiler_register_allocate(drc, compiler, desc, RDREG, ALLOC_DIRTY);
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_mult - compile a 32-bit multiply
    instruction

    mult  rS,rT
    multu rS,rT
-------------------------------------------------*/

static int compile_mult(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rs, rt, lo, hi;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		UINT64 result = is_unsigned ? ((UINT64)CONSTVAL(rs) * (UINT64)CONSTVAL(rt)) : ((INT64)CONSTVAL(rs) * (INT64)CONSTVAL(rt));
		emit_set_constant_value(drc, compiler, desc, REG_LO, (INT32)result);
		emit_set_constant_value(drc, compiler, desc, REG_HI, (INT32)(result >> 32));
	}

	/* otherwise, handle generically */
	else
	{
		UINT8 mulreg = ISCONST(rt) ? REG_ECX : X86REG(rt);

		hi = compiler_register_allocate(drc, compiler, desc, REG_HI, ALLOC_DST | ALLOC_DIRTY);
		lo = compiler_register_allocate(drc, compiler, desc, REG_LO, ALLOC_DST | ALLOC_DIRTY);

		emit_load_register_value(drc, compiler, REG_EAX, rs);								// mov  eax,<rs>
		emit_load_register_value(drc, compiler, mulreg, rt);								// mov  <mulreg>,<rt>
		if (is_unsigned)
			emit_mul_r32(DRCTOP, mulreg);													// mul  <mulreg>
		else
			emit_imul_r32(DRCTOP, mulreg);													// imul <mulreg>
		emit_movsxd_r64_r32(DRCTOP, X86REG(hi), REG_EDX);									// movsxd <hireg>,edx
		emit_movsxd_r64_r32(DRCTOP, X86REG(lo), REG_EAX);									// movsxd <loreg>,eax
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_dmult - compile a 64-bit multiply
    instruction

    dmult  rS,rT
    dmultu rS,rT
-------------------------------------------------*/

static int compile_dmult(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rs, rt, lo, hi;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		UINT64 result = is_unsigned ? ((UINT64)CONSTVAL(rs) * (UINT64)CONSTVAL(rt)) : ((INT64)CONSTVAL(rs) * (INT64)CONSTVAL(rt));
		emit_set_constant_value(drc, compiler, desc, REG_LO, (INT64)result);
		emit_set_constant_value(drc, compiler, desc, REG_HI, (INT64)(result >> 63));
	}

	/* otherwise, handle generically */
	else
	{
		UINT8 mulreg = ISCONST(rt) ? REG_ECX : X86REG(rt);

		hi = compiler_register_allocate(drc, compiler, desc, REG_HI, ALLOC_DST | ALLOC_DIRTY);
		lo = compiler_register_allocate(drc, compiler, desc, REG_LO, ALLOC_DST | ALLOC_DIRTY);

		emit_load_register_value(drc, compiler, REG_RAX, rs);								// mov  rax,<rs>
		emit_load_register_value(drc, compiler, mulreg, rt);								// mov  <mulreg>,<rt>
		if (is_unsigned)
			emit_mul_r64(DRCTOP, mulreg);													// mul  <mulreg>
		else
			emit_imul_r64(DRCTOP, mulreg);													// imul <mulreg>
		emit_mov_r64_r64(DRCTOP, X86REG(hi), REG_RDX);										// mov  <hi>,rdx
		emit_mov_r64_r64(DRCTOP, X86REG(lo), REG_RAX);										// mov  <lo>,rax
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_div - compile a 32-bit divide
    instruction

    div   rS,rT
    divu  rS,rT
-------------------------------------------------*/

static int compile_div(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rs, rt, lo, hi;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if rT is constant and 0, just skip */
	if (ISCONST(rt) && CONSTVAL(rt) == 0)
		return TRUE;

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		UINT32 result = is_unsigned ? ((UINT32)CONSTVAL(rs) / (UINT32)CONSTVAL(rt)) : ((INT32)CONSTVAL(rs) / (INT32)CONSTVAL(rt));
		UINT32 remainder = is_unsigned ? ((UINT32)CONSTVAL(rs) % (UINT32)CONSTVAL(rt)) : ((INT32)CONSTVAL(rs) % (INT32)CONSTVAL(rt));

		emit_set_constant_value(drc, compiler, desc, REG_LO, (INT32)result);
		emit_set_constant_value(drc, compiler, desc, REG_HI, (INT32)remainder);
	}

	/* otherwise, handle generically */
	else
	{
		UINT8 divreg = ISCONST(rt) ? REG_ECX : X86REG(rt);
		emit_link skip = { 0 };

		hi = compiler_register_allocate(drc, compiler, desc, REG_HI, ALLOC_DST | ALLOC_DIRTY);
		lo = compiler_register_allocate(drc, compiler, desc, REG_LO, ALLOC_DST | ALLOC_DIRTY);

		emit_load_register_value(drc, compiler, REG_RAX, rs);								// mov  rax,<rs>
		emit_load_register_value(drc, compiler, divreg, rt);								// mov  <divreg>,<rt>
		if (!ISCONST(rt))
		{
			emit_test_r32_r32(DRCTOP, divreg, divreg);										// test <divreg>,<divreg>
			emit_jcc_short_link(DRCTOP, COND_Z, &skip);										// jz   skip
		}
		if (is_unsigned)
		{
			emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);										// xor  edx,edx
			emit_div_r32(DRCTOP, divreg);													// div  <divreg>
		}
		else
		{
			emit_cdq(DRCTOP);																// cdq
			emit_idiv_r32(DRCTOP, divreg);													// idiv <divreg>
		}
		emit_movsxd_r64_r32(DRCTOP, X86REG(hi), REG_EDX);									// movsxd <hi>,edx
		emit_movsxd_r64_r32(DRCTOP, X86REG(lo), REG_EAX);									// movsxd <lo>,eax
		if (!ISCONST(rt))
			resolve_link(DRCTOP, &skip);												// skip:
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_ddiv - compile a 64-bit divide
    instruction

    ddiv   rS,rT
    ddivu  rS,rT
-------------------------------------------------*/

static int compile_ddiv(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rs, rt, lo, hi;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if rT is constant and 0, just skip */
	if (ISCONST(rt) && CONSTVAL(rt) == 0)
		return TRUE;

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		UINT32 result = is_unsigned ? ((UINT32)CONSTVAL(rs) / (UINT32)CONSTVAL(rt)) : ((INT32)CONSTVAL(rs) / (INT32)CONSTVAL(rt));
		UINT32 remainder = is_unsigned ? ((UINT32)CONSTVAL(rs) % (UINT32)CONSTVAL(rt)) : ((INT32)CONSTVAL(rs) % (INT32)CONSTVAL(rt));

		emit_set_constant_value(drc, compiler, desc, REG_LO, (INT32)result);
		emit_set_constant_value(drc, compiler, desc, REG_HI, (INT32)remainder);
	}

	/* if one is a constant, load the constant into EAX and multiple from there */
	else
	{
		UINT8 divreg = ISCONST(rt) ? REG_ECX : X86REG(rt);
		emit_link skip = { 0 };

		hi = compiler_register_allocate(drc, compiler, desc, REG_HI, ALLOC_DST | ALLOC_DIRTY);
		lo = compiler_register_allocate(drc, compiler, desc, REG_LO, ALLOC_DST | ALLOC_DIRTY);

		emit_load_register_value(drc, compiler, REG_RAX, rs);								// mov  rax,<rs>
		emit_load_register_value(drc, compiler, divreg, rt);								// mov  <divreg>,<rt>
		emit_test_r64_r64(DRCTOP, divreg, divreg);											// test <divreg>,<divreg>
		emit_jcc_short_link(DRCTOP, COND_Z, &skip);											// jz   skip
		if (is_unsigned)
		{
			emit_xor_r32_r32(DRCTOP, REG_EDX, REG_EDX);										// xor  edx,edx
			emit_div_r64(DRCTOP, divreg);													// div  <divreg>
		}
		else
		{
			emit_cqo(DRCTOP);																// cqo
			emit_idiv_r64(DRCTOP, divreg);													// idiv <divreg>
		}
		emit_mov_r64_r64(DRCTOP, X86REG(hi), REG_EDX);										// mov  <hi>,edx
		emit_mov_r64_r64(DRCTOP, X86REG(lo), REG_EAX);										// mov  <loreg>,eax
		resolve_link(DRCTOP, &skip);													// skip:
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_logical - compile a logical
    operation between two registers

    0: and rD,rS,rT / and rT,rS,UIMM
    1: or  rD,rS,rT / or  rT,rS,UIMM
    2: xor rD,rS,rT / xor rT,rS,UIMM
    3: nor rD,rS,rT
-------------------------------------------------*/

static int compile_logical(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int operation, int immediate)
{
	UINT32 op = *desc->opptr.l;
	UINT8 destreg = immediate ? RTREG : RDREG;
	mipsreg_state rd, rs, rt;

	assert(operation >= 0 && operation <= 3);
	assert(!immediate || operation != 3);

	/* if the destination is r0, bail */
	if (destreg == 0)
		return TRUE;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = immediate ? MAKE_CONST(UIMMVAL) : compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if both constant, we know the answer up front */
	if (ISCONST(rs) && ISCONST(rt))
	{
		UINT64 result = (operation == 0) ? (CONSTVAL(rs) & CONSTVAL(rt)) : (operation == 1) ? (CONSTVAL(rs) | CONSTVAL(rt)) : (operation == 2) ? (CONSTVAL(rs) ^ CONSTVAL(rt)) : ~(CONSTVAL(rs) | CONSTVAL(rt));
		emit_set_constant_value(drc, compiler, desc, destreg, result);
	}

	/* an AND against 0 is constant as well */
	else if (operation == 0 && ISCONST(rt) && CONSTVAL(rt) == 0)
		emit_set_constant_value(drc, compiler, desc, destreg, 0);

	/* else do the operation in code */
	else
	{
		rd = compiler_register_allocate(drc, compiler, desc, destreg, ALLOC_DST | ALLOC_DIRTY);

		/* if rD matches rT, or if rS is const and rT isn't, then swap rT and rS */
		if (X86REG(rd) == X86REG(rt) || (ISCONST(rs) && !ISCONST(rt)))
		{
			mipsreg_state temp = rs;
			rs = rt;
			rt = temp;
		}

		/* load rS into rD */
		emit_load_register_value(drc, compiler, X86REG(rd), rs);							// mov  <rd>,<rs>

		/* if rD does not match rT, we can work directly in rD */
		if (!ISCONST(rt))
		{
			if (operation == 0)
				emit_and_r64_r64(DRCTOP, X86REG(rd), X86REG(rt));							// and  <rd>,<rt>
			else if (operation == 1)
				emit_or_r64_r64(DRCTOP, X86REG(rd), X86REG(rt));							// or   <rd>,<rt>
			else if (operation == 2)
				emit_xor_r64_r64(DRCTOP, X86REG(rd), X86REG(rt));							// xor  <rd>,<rt>
			else if (operation == 3)
			{
				emit_or_r64_r64(DRCTOP, X86REG(rd), X86REG(rt));							// or   <rd>,<rt>
				emit_not_r64(DRCTOP, X86REG(rd));											// not  <rd>
			}
		}
		else
		{
			if (operation == 0)
				emit_and_r64_imm(DRCTOP, X86REG(rd), CONSTVAL(rt));							// and  <rd>,CONSTVAL(rt)
			else if (operation == 1 && CONSTVAL(rt) != 0)
				emit_or_r64_imm(DRCTOP, X86REG(rd), CONSTVAL(rt));							// or   <rd>,CONSTVAL(rt)
			else if (operation == 2 && CONSTVAL(rt) != 0)
				emit_xor_r64_imm(DRCTOP, X86REG(rd), CONSTVAL(rt));							// xor  <rd>,CONSTVAL(rt)
			else if (operation == 3)
			{
				if (CONSTVAL(rt) != 0)
					emit_or_r64_imm(DRCTOP, X86REG(rd), CONSTVAL(rt));						// or   <rd>,CONSTVAL(rt)
				emit_not_r64(DRCTOP, X86REG(rd));											// not  <rd>
			}
		}
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_slt - compile a set if less
    than btween two registers

    slti  rD,rS,rT / slti  rT,rS,SIMM
    sltiu rD,rS,rT / sltiu rT,rS,SIMM
-------------------------------------------------*/

static int compile_slt(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_unsigned, int immediate)
{
	UINT32 op = *desc->opptr.l;
	UINT8 destreg = immediate ? RTREG : RDREG;
	mipsreg_state rd, rs, rt;

	/* if the destination is r0, bail */
	if (destreg == 0)
		return TRUE;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = immediate ? MAKE_CONST(SIMMVAL) : compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if the source is constant, compute as constant */
	if (ISCONST(rs) && ISCONST(rt))
	{
		INT64 result = is_unsigned ? ((UINT32)CONSTVAL(rs) < (UINT32)CONSTVAL(rt)) : ((INT32)CONSTVAL(rs) < (INT32)CONSTVAL(rt));
		emit_set_constant_value(drc, compiler, desc, destreg, result);
	}

	/* else do the operation in code */
	else
	{
		UINT8 rsreg = ISCONST(rs) ? REG_RAX : X86REG(rs);

		rd = compiler_register_allocate(drc, compiler, desc, destreg, ALLOC_DST | ALLOC_DIRTY);

		emit_load_register_value(drc, compiler, rsreg, rs);									// mov  <rsreg>,<rs>
		if (ISCONST(rt))
			emit_cmp_r64_imm(DRCTOP, rsreg, CONSTVAL(rt));									// cmp  <rsreg>,<rt>
		else
			emit_cmp_r64_r64(DRCTOP, rsreg, X86REG(rt));									// cmp  <rsreg>,<rt>
		emit_setcc_r8(DRCTOP, is_unsigned ? COND_B : COND_L, X86REG(rd));					// setb/l <rd>
		emit_movzx_r32_r8(DRCTOP, X86REG(rd), X86REG(rd));									// movzx <rd>,<rd>
	}
	return TRUE;
}


/*-------------------------------------------------
    compile_load - compile a memory load
    instruction

    lb   rT,SIMM(rS)
    lbu  rT,SIMM(rS)
    lh   rT,SIMM(rS)
    lhu  rT,SIMM(rS)
    lw   rT,SIMM(rS)
    lwu  rT,SIMM(rS)
    ld   rT,SIMM(rS)
-------------------------------------------------*/

static int compile_load(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, int is_unsigned)
{
	UINT32 op = *desc->opptr.l;
	mipsreg_state rs, rt;
	int was_fixed = FALSE;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_DST);

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && !DISABLE_FIXED_RAM_LOAD)
	{
		UINT8 destreg = (RTREG == 0) ? REG_RAX : X86REG(rt);

		/* perform a fixed read if we can */
		if (size == 1)
			was_fixed = emit_fixed_byte_read(drc, compiler, desc, destreg, is_unsigned, CONSTVAL(rs) + SIMMVAL);
		else if (size == 2)
			was_fixed = emit_fixed_half_read(drc, compiler, desc, destreg, is_unsigned, CONSTVAL(rs) + SIMMVAL);
		else if (size == 4)
			was_fixed = emit_fixed_word_read(drc, compiler, desc, destreg, is_unsigned, CONSTVAL(rs) + SIMMVAL, 0);
		else
			was_fixed = emit_fixed_double_read(drc, compiler, desc, destreg, CONSTVAL(rs) + SIMMVAL, 0);
	}

	/* otherwise, we just need to call the appropriate subroutine */
	else
	{
		if (ISCONST(rs))
			emit_mov_r32_imm(DRCTOP, REG_P1, CONSTVAL(rs) + SIMMVAL);						// mov  p1,<rs> + SIMMVAL
		else
			emit_lea_r32_m32(DRCTOP, REG_P1, MBD(X86REG(rs), SIMMVAL));						// lea  p1,[<rs> + SIMMVAL]
		if (size == 1 && is_unsigned)
			emit_call(DRCTOP, mips3.drcdata->general.read_byte_unsigned);					// call general.read_byte_unsigned
		else if (size == 1 && !is_unsigned)
			emit_call(DRCTOP, mips3.drcdata->general.read_byte_signed);						// call general.read_byte_signed
		else if (size == 2 && is_unsigned)
			emit_call(DRCTOP, mips3.drcdata->general.read_half_unsigned);					// call general.read_half_unsigned
		else if (size == 2 && !is_unsigned)
			emit_call(DRCTOP, mips3.drcdata->general.read_half_signed);						// call general.read_half_signed
		else if (size == 4 && is_unsigned)
			emit_call(DRCTOP, mips3.drcdata->general.read_word_unsigned);					// call general.read_word_unsigned
		else if (size == 4 && !is_unsigned)
			emit_call(DRCTOP, mips3.drcdata->general.read_word_signed);						// call general.read_word_signed
		else
			emit_call(DRCTOP, mips3.drcdata->general.read_double);							// call general.read_double
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbload_exception);
		if (RTREG != 0)
			emit_mov_r64_r64(DRCTOP, X86REG(rt), REG_RAX);									// mov  <rt>,rax
	}

	/* only mark dirty after the exception risk is over */
	compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_DIRTY);

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}


/*-------------------------------------------------
    compile_load_cop - compile a COP memory load
    instruction

    lwc1 rT,SIMM(rs)
    lwc2 rT,SIMM(rs)
    ldc1 rT,SIMM(rs)
    ldc2 rT,SIMM(rs)
-------------------------------------------------*/

static int compile_load_cop(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, void *dest)
{
	UINT32 op = *desc->opptr.l;
	int was_fixed = FALSE;
	mipsreg_state rs;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && !DISABLE_FIXED_RAM_LOAD)
	{
		/* perform a fixed read if we can */
		if (size == 4)
			was_fixed = emit_fixed_word_read(drc, compiler, desc, REG_RAX, FALSE, CONSTVAL(rs) + SIMMVAL, 0);
		else
			was_fixed = emit_fixed_double_read(drc, compiler, desc, REG_RAX, CONSTVAL(rs) + SIMMVAL, 0);
	}

	/* otherwise, we just need to call the appropriate subroutine */
	else
	{
		if (ISCONST(rs))
			emit_mov_r32_imm(DRCTOP, REG_P1, CONSTVAL(rs) + SIMMVAL);						// mov  p1,<rs> + SIMMVAL
		else
			emit_lea_r32_m32(DRCTOP, REG_P1, MBD(X86REG(rs), SIMMVAL));						// lea  p1,[<rs> + SIMMVAL]
		if (size == 4)
			emit_call(DRCTOP, mips3.drcdata->general.read_word_unsigned);					// call general.read_word_unsigned
		else
			emit_call(DRCTOP, mips3.drcdata->general.read_double);							// call general.read_double
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbload_exception);
	}

	/* complete the load by moving to the destination address */
	if (size == 4)
		emit_mov_m32_r32(DRCTOP, MDRC(dest), REG_EAX);										// mov  [dest],eax
	else
		emit_mov_m64_r64(DRCTOP, MDRC(dest), REG_RAX);										// mov  [dest],rax

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}


/*-------------------------------------------------
    compile_loadx_cop - compile a COP indexed
    memory load instruction

    lwxc1 fD,rS,rT
    ldxc1 fD,rS,rT
-------------------------------------------------*/

static int compile_loadx_cop(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, void *dest)
{
	UINT32 op = *desc->opptr.l;
	int was_fixed = FALSE;
	mipsreg_state rs, rt;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && ISCONST(rt) && !DISABLE_FIXED_RAM_LOAD)
	{
		/* perform a fixed read if we can */
		if (size == 4)
			was_fixed = emit_fixed_word_read(drc, compiler, desc, REG_RAX, FALSE, CONSTVAL(rs) + CONSTVAL(rt), 0);
		else
			was_fixed = emit_fixed_double_read(drc, compiler, desc, REG_RAX, CONSTVAL(rs) + CONSTVAL(rt), 0);
	}

	/* otherwise, we just need to call the appropriate subroutine */
	else
	{
		if (ISCONST(rs) && ISCONST(rt))
			emit_mov_r32_imm(DRCTOP, REG_P1, CONSTVAL(rs) + CONSTVAL(rt));					// mov  p1,<rs> + <rt>
		else if (ISCONST(rs))
			emit_lea_r32_m32(DRCTOP, REG_P1, MBD(X86REG(rt), CONSTVAL(rs)));				// lea  p1,[<rt> + <rs>]
		else if (ISCONST(rt))
			emit_lea_r32_m32(DRCTOP, REG_P1, MBD(X86REG(rs), CONSTVAL(rt)));				// lea  p1,[<rs> + <rt>]
		else
			emit_lea_r32_m32(DRCTOP, REG_P1, MBISD(X86REG(rs), X86REG(rt), 1, 0));			// lea  p1,[<rs> + <rt>]
		if (size == 4)
			emit_call(DRCTOP, mips3.drcdata->general.read_word_unsigned);					// call general.read_word_unsigned
		else
			emit_call(DRCTOP, mips3.drcdata->general.read_double);							// call general.read_double
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbload_exception);
	}

	/* complete the load by moving to the destination address */
	if (size == 4)
		emit_mov_m32_r32(DRCTOP, MDRC(dest), REG_EAX);										// mov  [dest],eax
	else
		emit_mov_m64_r64(DRCTOP, MDRC(dest), REG_RAX);										// mov  [dest],rax

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}


/*-------------------------------------------------
    compile_load_word_partial - compile a lwl/lwr
    instruction

    lwl   rT,SIMM(rS)
    lwr   rT,SIMM(rS)
-------------------------------------------------*/

static int compile_load_word_partial(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_right)
{
	UINT32 op = *desc->opptr.l;
	int was_fixed = FALSE;
	mipsreg_state rs, rt;

	/* allocate source/dest registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC | ALLOC_DST | ALLOC_DIRTY);

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && !DISABLE_FIXED_RAM_LOAD)
	{
		INT32 addr = CONSTVAL(rs) + SIMMVAL;
		int shift = (addr & 3) * 8;

		/* shift the opposite amount depending on right/left and endianness */
		if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
			shift ^= 0x18;

		/* read the double value into rax, handling exceptions */
		was_fixed = emit_fixed_word_read(drc, compiler, desc, REG_RAX, FALSE, addr, is_right ? (~((UINT32)-1 << shift)) : (~((UINT32)-1 >> shift)));

		/* handle writing back the register */
		if (RTREG != 0)
		{
			UINT32 mask = is_right ? (~((UINT32)-1 >> shift)) : (~((UINT32)-1 << shift));

			/* AND rT with the mask */
			emit_and_r32_imm(DRCTOP, X86REG(rt), mask);										// and  <rt>,mask

			/* shift and merge rax into the X86REG(rt) */
			if (!is_right)
				emit_shl_r32_imm(DRCTOP, REG_EAX, shift);									// shl  eax,shift
			else
				emit_shr_r32_imm(DRCTOP, REG_EAX, shift);									// shr  eax,shift
			emit_or_r32_r32(DRCTOP, X86REG(rt), REG_EAX);									// or   <rtdst>,eax
			emit_movsxd_r64_r32(DRCTOP, X86REG(rt), X86REG(rt));							// movsxd <rt>,<rt>
		}
	}

	/* otherwise, we need to compute everything at runtime */
	else
	{
		/* compute the address and mask values */
		if (ISCONST(rs))
			emit_mov_r32_imm(DRCTOP, REG_EAX, CONSTVAL(rs) + SIMMVAL);						// mov  eax,<rs> + SIMMVAL
		else
			emit_lea_r32_m32(DRCTOP, REG_EAX, MBD(X86REG(rs), SIMMVAL));					// lea  eax,[<rs> + SIMMVAL]
		emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_RAX, 8, 0));								// lea  ecx,[rax*8]
		emit_or_r32_imm(DRCTOP, REG_P2, -1);												// or   p2,-1
		if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
			emit_xor_r32_imm(DRCTOP, REG_ECX, 0x18);										// xor  ecx,0x18
		emit_and_r32_imm(DRCTOP, REG_EAX, ~3);												// and  eax,~3
		if (!is_right)
			emit_shr_r32_cl(DRCTOP, REG_P2);												// shr  p2,cl
		else
			emit_shl_r32_cl(DRCTOP, REG_P2);												// shl  p2,cl
		emit_mov_m8_r8(DRCTOP, MBD(REG_RSP, SPOFFS_SAVECL), REG_CL);						// mov  [sp+savecl],cl
		emit_not_r32(DRCTOP, REG_P2);														// not  p2
		emit_mov_r32_r32(DRCTOP, REG_P1, REG_EAX);											// mov  p1,eax
		emit_call(DRCTOP, mips3.drcdata->general.read_word_masked);							// call general.read_word_masked
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbload_exception);

		/* handle writing back the register */
		if (RTREG != 0)
		{
			/* recompute the mask */
			emit_mov_r8_m8(DRCTOP, REG_CL, MBD(REG_RSP, SPOFFS_SAVECL));					// mov  cl,[sp+savecl]
			emit_or_r32_imm(DRCTOP, REG_P2, -1);											// or   p2,-1
			if (!is_right)
				emit_shl_r32_cl(DRCTOP, REG_P2);											// shl  p2,cl
			else
				emit_shr_r32_cl(DRCTOP, REG_P2);											// shr  p2,cl

			/* apply the shifts and masks */
			emit_not_r32(DRCTOP, REG_P2);													// not  p2
			if (!is_right)
				emit_shl_r32_cl(DRCTOP, REG_RAX);											// shl  rax,cl
			else
				emit_shr_r32_cl(DRCTOP, REG_RAX);											// shr  rax,cl
			emit_and_r32_r32(DRCTOP, X86REG(rt), REG_P2);									// and  <rt>,p2
			emit_or_r32_r32(DRCTOP, X86REG(rt), REG_RAX);									// or   <rt>,rax
			emit_movsxd_r64_r32(DRCTOP, X86REG(rt), X86REG(rt));							// movsxd <rt>,<rt>
		}
	}

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}


/*-------------------------------------------------
    compile_load_double_partial - compile a
    ldl/ldr instruction

    ldl   rT,SIMM(rS)
    ldr   rT,SIMM(rS)
-------------------------------------------------*/

static int compile_load_double_partial(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_right)
{
	UINT32 op = *desc->opptr.l;
	int was_fixed = FALSE;
	mipsreg_state rs, rt;

	/* allocate source/dest registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC | ALLOC_DST | ALLOC_DIRTY);

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && !DISABLE_FIXED_RAM_LOAD)
	{
		INT32 addr = CONSTVAL(rs) + SIMMVAL;
		int shift = (addr & 7) * 8;

		/* shift the opposite amount depending on right/left and endianness */
		if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
			shift ^= 0x38;

		/* read the double value into rax, handling exceptions */
		was_fixed = emit_fixed_double_read(drc, compiler, desc, REG_RAX, addr, is_right ? (~((UINT64)-1 << shift)) : (~((UINT64)-1 >> shift)));

		/* handle writing back the register */
		if (RTREG != 0)
		{
			UINT64 mask = is_right ? (~((UINT64)-1 >> shift)) : (~((UINT64)-1 << shift));

			/* AND rT with the mask, using 32 bits if we can */
			if ((INT32)mask == (INT64)mask)
				emit_and_r64_imm(DRCTOP, X86REG(rt), (INT32)mask);							// and  <rt>,mask
			else
			{
				emit_mov_r64_imm(DRCTOP, REG_P2, mask);										// mov  p2,mask
				emit_and_r64_r64(DRCTOP, X86REG(rt), REG_P2);								// and  <rt>,p2
			}

			/* shift and merge rax into the X86REG(rt) */
			if (!is_right)
				emit_shl_r64_imm(DRCTOP, REG_RAX, shift);									// shl  rax,shift
			else
				emit_shr_r64_imm(DRCTOP, REG_RAX, shift);									// shr  rax,shift
			emit_or_r64_r64(DRCTOP, X86REG(rt), REG_RAX);									// or   <rt>,rax
		}
	}

	/* otherwise, we need to compute everything at runtime */
	else
	{
		/* compute the address and mask values */
		if (ISCONST(rs))
			emit_mov_r32_imm(DRCTOP, REG_EAX, CONSTVAL(rs) + SIMMVAL);						// mov  eax,<rs> + SIMMVAL
		else
			emit_lea_r32_m32(DRCTOP, REG_EAX, MBD(X86REG(rs), SIMMVAL));					// lea  eax,[<rs> + SIMMVAL]
		emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_RAX, 8, 0));								// lea  ecx,[rax*8]
		emit_or_r64_imm(DRCTOP, REG_P2, -1);												// or   p2,-1
		if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
			emit_xor_r32_imm(DRCTOP, REG_ECX, 0x38);										// xor  ecx,0x38
		emit_and_r32_imm(DRCTOP, REG_EAX, ~7);												// and  eax,~7
		if (!is_right)
			emit_shr_r64_cl(DRCTOP, REG_P2);												// shr  p2,cl
		else
			emit_shl_r64_cl(DRCTOP, REG_P2);												// shl  p2,cl
		emit_mov_m8_r8(DRCTOP, MBD(REG_RSP, SPOFFS_SAVECL), REG_CL);						// mov  [sp+savecl],cl
		emit_not_r64(DRCTOP, REG_P2);														// not  p2
		emit_mov_r32_r32(DRCTOP, REG_P1, REG_EAX);											// mov  p1,eax
		emit_call(DRCTOP, mips3.drcdata->general.read_double_masked);						// call general.read_double_masked
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbload_exception);

		/* handle writing back the register */
		if (RTREG != 0)
		{
			/* recompute the mask */
			emit_mov_r8_m8(DRCTOP, REG_CL, MBD(REG_RSP, SPOFFS_SAVECL));					// mov  cl,[sp+savecl]
			emit_or_r64_imm(DRCTOP, REG_P2, -1);											// or   p2,-1
			if (!is_right)
				emit_shl_r64_cl(DRCTOP, REG_P2);											// shl  p2,cl
			else
				emit_shr_r64_cl(DRCTOP, REG_P2);											// shr  p2,cl

			/* apply the shifts and masks */
			emit_not_r64(DRCTOP, REG_P2);													// not  p2
			if (!is_right)
				emit_shl_r64_cl(DRCTOP, REG_RAX);											// shl  rax,cl
			else
				emit_shr_r64_cl(DRCTOP, REG_RAX);											// shr  rax,cl
			emit_and_r64_r64(DRCTOP, X86REG(rt), REG_P2);									// and  <rt>,p2
			emit_or_r64_r64(DRCTOP, X86REG(rt), REG_RAX);									// or   <rt>,rax
		}
	}

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}


/*-------------------------------------------------
    compile_store - compile a memory store
    instruction

    sb   rT,SIMM(rS)
    sh   rT,SIMM(rS)
    sw   rT,SIMM(rS)
    sd   rT,SIMM(rS)
-------------------------------------------------*/

static int compile_store(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size)
{
	UINT32 op = *desc->opptr.l;
	int was_fixed = FALSE;
	mipsreg_state rs, rt;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && !DISABLE_FIXED_RAM_STORE)
	{
		/* perform a fixed write if we can */
		if (size == 1)
			was_fixed = emit_fixed_byte_write(drc, compiler, desc, rt, CONSTVAL(rs) + SIMMVAL);
		else if (size == 2)
			was_fixed = emit_fixed_half_write(drc, compiler, desc, rt, CONSTVAL(rs) + SIMMVAL);
		else if (size == 4)
			was_fixed = emit_fixed_word_write(drc, compiler, desc, rt, CONSTVAL(rs) + SIMMVAL, 0);
		else
			was_fixed = emit_fixed_double_write(drc, compiler, desc, rt, CONSTVAL(rs) + SIMMVAL, 0);
	}

	/* otherwise, we just need to call the appropriate subroutine */
	else
	{
		if (ISCONST(rs))
			emit_mov_r32_imm(DRCTOP, REG_P1, CONSTVAL(rs) + SIMMVAL);						// mov  p1,<rs> + SIMMVAL
		else
			emit_lea_r32_m32(DRCTOP, REG_P1, MBD(X86REG(rs), SIMMVAL));						// lea  p1,[<rs> + SIMMVAL]
		emit_load_register_value(drc, compiler, REG_P2, rt);								// mov  p2,<rt>
		if (size == 1)
			emit_call(DRCTOP, mips3.drcdata->general.write_byte);							// call general.write_byte
		else if (size == 2)
			emit_call(DRCTOP, mips3.drcdata->general.write_half);							// call general.write_half
		else if (size == 4)
			emit_call(DRCTOP, mips3.drcdata->general.write_word);							// call general.write_word
		else if (size == 8)
			emit_call(DRCTOP, mips3.drcdata->general.write_double);							// call general.write_double
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbstore_exception);
	}

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}


/*-------------------------------------------------
    compile_store_cop - compile a COP memory
    store instruction

    swc1 rT,SIMM(rs)
    swc2 rT,SIMM(rs)
    sdc1 rT,SIMM(rs)
    sdc2 rT,SIMM(rs)
-------------------------------------------------*/

static int compile_store_cop(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, void *src)
{
	UINT32 op = *desc->opptr.l;
	int was_fixed = FALSE;
	mipsreg_state rs;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);

	/* load the value into P2 */
	if (size == 4)
		emit_mov_r32_m32(DRCTOP, REG_P2, MDRC(src));										// mov  p2,[src]
	else
		emit_mov_r64_m64(DRCTOP, REG_P2, MDRC(src));										// mov  p2,[src]

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && !DISABLE_FIXED_RAM_STORE)
	{
		/* perform a fixed write if we can */
		if (size == 4)
			was_fixed = emit_fixed_word_write(drc, compiler, desc, REG_P2, CONSTVAL(rs) + SIMMVAL, 0);
		else
			was_fixed = emit_fixed_double_write(drc, compiler, desc, REG_P2, CONSTVAL(rs) + SIMMVAL, 0);
	}

	/* otherwise, we just need to call the appropriate subroutine */
	else
	{
		if (ISCONST(rs))
			emit_mov_r32_imm(DRCTOP, REG_P1, CONSTVAL(rs) + SIMMVAL);						// mov  p1,<rs> + SIMMVAL
		else
			emit_lea_r32_m32(DRCTOP, REG_P1, MBD(X86REG(rs), SIMMVAL));						// lea  p1,[<rs> + SIMMVAL]
		if (size == 4)
			emit_call(DRCTOP, mips3.drcdata->general.write_word);							// call general.write_word
		else if (size == 8)
			emit_call(DRCTOP, mips3.drcdata->general.write_double);							// call general.write_double
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbstore_exception);
	}

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}


/*-------------------------------------------------
    compile_storex_cop - compile a COP indexed
    memory store instruction

    swxc1 fD,rS,rT
    sdxc1 fD,rS,rT
-------------------------------------------------*/

static int compile_storex_cop(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int size, void *src)
{
	UINT32 op = *desc->opptr.l;
	int was_fixed = FALSE;
	mipsreg_state rs, rt;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* load the value into P2 */
	if (size == 4)
		emit_mov_r32_m32(DRCTOP, REG_P2, MDRC(src));										// mov  p2,[src]
	else
		emit_mov_r64_m64(DRCTOP, REG_P2, MDRC(src));										// mov  p2,[src]

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && ISCONST(rt) && !DISABLE_FIXED_RAM_STORE)
	{
		/* perform a fixed write if we can */
		if (size == 4)
			was_fixed = emit_fixed_word_write(drc, compiler, desc, REG_P2, CONSTVAL(rs) + CONSTVAL(rt), 0);
		else
			was_fixed = emit_fixed_double_write(drc, compiler, desc, REG_P2, CONSTVAL(rs) + CONSTVAL(rt), 0);
	}

	/* otherwise, we just need to call the appropriate subroutine */
	else
	{
		if (ISCONST(rs) && ISCONST(rt))
			emit_mov_r32_imm(DRCTOP, REG_P1, CONSTVAL(rs) + CONSTVAL(rt));					// mov  p1,<rs> + <rt>
		else if (ISCONST(rs))
			emit_lea_r32_m32(DRCTOP, REG_P1, MBD(X86REG(rt), CONSTVAL(rs)));				// lea  p1,[<rt> + <rs>]
		else if (ISCONST(rt))
			emit_lea_r32_m32(DRCTOP, REG_P1, MBD(X86REG(rs), CONSTVAL(rt)));				// lea  p1,[<rs> + <rt>]
		else
			emit_lea_r32_m32(DRCTOP, REG_P1, MBISD(X86REG(rs), X86REG(rt), 1, 0));			// lea  p1,[<rs> + <rt>]
		if (size == 4)
			emit_call(DRCTOP, mips3.drcdata->general.write_word);							// call general.write_word
		else if (size == 8)
			emit_call(DRCTOP, mips3.drcdata->general.write_double);							// call general.write_double
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbstore_exception);
	}

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}


/*-------------------------------------------------
    compile_store_word_partial - compile a swl/swr
    instruction

    swl   rT,SIMM(rS)
    swr   rT,SIMM(rS)
-------------------------------------------------*/

static int compile_store_word_partial(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_right)
{
	UINT32 op = *desc->opptr.l;
	int was_fixed = FALSE;
	mipsreg_state rs, rt;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && !DISABLE_FIXED_RAM_STORE)
	{
		INT32 addr = CONSTVAL(rs) + SIMMVAL;
		int shift = (addr & 3) * 8;
		UINT32 mask;

		/* shift the opposite amount depending on right/left and endianness */
		if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
			shift ^= 0x18;

		/* if rT is not constant, load and shift it */
		if (!ISCONST(rt))
		{
			emit_mov_r32_r32(DRCTOP, REG_EAX, X86REG(rt));									// mov  eax,X86REG(rt)
			emit_shr_r32_imm(DRCTOP, REG_EAX, shift);										// shr  eax,shift
			rt = MAKE_DESTREG(REG_EAX);
		}

		/* compute shifted value and mask */
		if (!is_right)
		{
			if (ISCONST(rt))
				rt = MAKE_CONST((UINT32)CONSTVAL(rt) >> shift);
			mask = ~((UINT64)-1 >> shift);
		}
		else
		{
			if (ISCONST(rt))
				rt = MAKE_CONST((UINT32)CONSTVAL(rt) << shift);
			mask = ~((UINT64)-1 << shift);
		}

		/* perform a fixed write */
		was_fixed = emit_fixed_word_write(drc, compiler, desc, rt, addr & ~3, mask);
	}

	/* otherwise, we need to compute everything at runtime */
	else
	{
		/* compute the address and mask values */
		emit_lea_r32_m32(DRCTOP, REG_EAX, MBD(X86REG(rs), SIMMVAL));						// lea  eax,[<rs> + SIMMVAL]
		emit_or_r32_imm(DRCTOP, REG_P3, -1);												// or   p3,-1
		emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_RAX, 8, 0));								// lea  ecx,[rax*8]
		emit_load_register_value(drc, compiler, REG_P2, rt);								// mov  p2,<rt>
		if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
			emit_xor_r32_imm(DRCTOP, REG_ECX, 0x18);										// xor  ecx,0x18
		emit_and_r32_imm(DRCTOP, REG_EAX, ~3);												// and  eax,~3
		if (!is_right)
		{
			emit_shr_r32_cl(DRCTOP, REG_P3);												// shr  p3,cl
			emit_shr_r32_cl(DRCTOP, REG_P2);												// shr  p2,cl
		}
		else
		{
			emit_shl_r32_cl(DRCTOP, REG_P3);												// shl  p3,cl
			emit_shl_r32_cl(DRCTOP, REG_P2);												// shl  p2,cl
		}
		emit_mov_r32_r32(DRCTOP, REG_P1, REG_EAX);											// mov  p1,eax
		emit_not_r32(DRCTOP, REG_P3);														// not  p3

		/* perform the write */
		emit_call(DRCTOP, mips3.drcdata->general.write_word_masked);						// call general.write_word_masked
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbstore_exception);
	}

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}


/*-------------------------------------------------
    compile_store_double_partial - compile a
    sdl/sdr instruction

    sdl   rT,SIMM(rS)
    sdr   rT,SIMM(rS)
-------------------------------------------------*/

static int compile_store_double_partial(drc_core *drc, compiler_state *compiler, const opcode_desc *desc, int is_right)
{
	UINT32 op = *desc->opptr.l;
	int was_fixed = FALSE;
	mipsreg_state rs, rt;

	/* allocate source registers */
	rs = compiler_register_allocate(drc, compiler, desc, RSREG, ALLOC_SRC);
	rt = compiler_register_allocate(drc, compiler, desc, RTREG, ALLOC_SRC);

	/* if rS is constant, we know the address at compile time; this simplifies the code */
	if (ISCONST(rs) && !DISABLE_FIXED_RAM_STORE)
	{
		INT32 addr = CONSTVAL(rs) + SIMMVAL;
		int shift = (addr & 7) * 8;
		UINT64 mask;

		/* shift the opposite amount depending on right/left and endianness */
		if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
			shift ^= 0x38;

		/* if rT is not constant, load and shift it */
		if (!ISCONST(rt))
		{
			emit_mov_r64_r64(DRCTOP, REG_RAX, X86REG(rt));										// mov  rax,X86REG(rt)
			emit_shr_r64_imm(DRCTOP, REG_RAX, shift);										// shr  rax,shift
			rt = MAKE_DESTREG(REG_RAX);
		}

		/* compute shifted value and mask */
		if (!is_right)
		{
			if (ISCONST(rt))
				rt = MAKE_CONST((UINT64)CONSTVAL(rt) >> shift);
			mask = ~((UINT64)-1 >> shift);
		}
		else
		{
			if (ISCONST(rt))
				rt = MAKE_CONST((UINT64)CONSTVAL(rt) << shift);
			mask = ~((UINT64)-1 << shift);
		}

		/* perform a fixed write */
		was_fixed = emit_fixed_double_write(drc, compiler, desc, rt, addr & ~7, mask);
	}

	/* otherwise, we need to compute everything at runtime */
	else
	{
		/* compute the address and mask values */
		emit_lea_r32_m32(DRCTOP, REG_EAX, MBD(X86REG(rs), SIMMVAL));				// lea  eax,[<rs> + SIMMVAL]
		emit_or_r64_imm(DRCTOP, REG_P3, -1);										// or   p3,-1
		emit_lea_r32_m32(DRCTOP, REG_ECX, MISD(REG_RAX, 8, 0));						// lea  ecx,[rax*8]
		emit_load_register_value(drc, compiler, REG_P2, rt);						// mov  p2,<rt>
		if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
			emit_xor_r32_imm(DRCTOP, REG_ECX, 0x38);								// xor  ecx,0x38
		emit_and_r32_imm(DRCTOP, REG_EAX, ~7);										// and  eax,~7
		if (!is_right)
		{
			emit_shr_r64_cl(DRCTOP, REG_P3);										// shr  p3,cl
			emit_shr_r64_cl(DRCTOP, REG_P2);										// shr  p2,cl
		}
		else
		{
			emit_shl_r64_cl(DRCTOP, REG_P3);										// shl  p3,cl
			emit_shl_r64_cl(DRCTOP, REG_P2);										// shl  p2,cl
		}
		emit_mov_r32_r32(DRCTOP, REG_P1, REG_EAX);									// mov  p1,eax
		emit_not_r64(DRCTOP, REG_P3);												// not  p3

		/* perform the write */
		emit_call(DRCTOP, mips3.drcdata->general.write_double_masked);				// call general.write_double_masked
		oob_request_callback(drc, COND_NO_JUMP, oob_exception_cleanup, compiler, desc, mips3.drcdata->generate_tlbstore_exception);
	}

	/* update cycle counts */
	if (!was_fixed)
		emit_update_cycles_pc_and_flush(drc, compiler, desc, 0);
	return TRUE;
}
