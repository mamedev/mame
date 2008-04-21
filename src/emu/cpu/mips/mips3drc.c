/***************************************************************************

    mips3drc.c

    Universal machine language-based MIPS III/IV emulator.
    See drcdesc.txt for an explanation of the universal DRC architecture.

    Copyright Aaron Giles
    Released for general use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include <stddef.h>
#include "cpuintrf.h"
#include "debugger.h"
#include "mips3com.h"
#include "mips3fe.h"
#include "deprecat.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

extern unsigned dasmmips3(char *buffer, unsigned pc, UINT32 op);



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_UML							(1)
#define LOG_NATIVE						(1)
#define SINGLE_INSTRUCTION_MODE			(1)
#define PRINTF_EXCEPTIONS				(1)

#define PROBE_ADDRESS					~0



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* map variables */
#define MAPVAR_PC						MVAR(0)
#define MAPVAR_CYCLES					MVAR(1)

/* size of the execution code cache */
#define CACHE_SIZE						(32 * 1024 * 1024)

/* compilation boundaries -- how far back/forward does the analysis extend? */
#define COMPILE_BACKWARDS_BYTES			128
#define COMPILE_FORWARDS_BYTES			512
#define COMPILE_MAX_INSTRUCTIONS		((COMPILE_BACKWARDS_BYTES/4) + (COMPILE_FORWARDS_BYTES/4))
#define COMPILE_MAX_SEQUENCE			64

/* exit codes */
#define EXECUTE_OUT_OF_CYCLES			0
#define EXECUTE_MISSING_CODE			1



/***************************************************************************
    MACROS
***************************************************************************/

#ifdef LSB_FIRST
#define LOPTR(x)				((UINT32 *)(x))
#else
#define LOPTR(x)				((UINT32 *)(x) + 1)
#endif

#define R32(reg)				((reg == 0) ? DRCUML_PTYPE_IMMEDIATE : DRCUML_PTYPE_MEMORY), ((reg) == 0) ? 0 : (FPTR)LOPTR(&mips3.core->r[reg])
#define LO32					R32(REG_LO)
#define HI32					R32(REG_HI)
#define CPR032(reg)				MEM(LOPTR(&mips3.core->cpr[0][reg]))
#define CCR032(reg)				MEM(LOPTR(&mips3.core->ccr[0][reg]))
#define FPR32(reg)				MEM(mips3.mode ? &((float *)&mips3.core->cpr[1][0])[reg] : (float *)&mips3.core->cpr[1][reg])
#define CCR132(reg)				MEM(LOPTR(&mips3.core->ccr[1][reg]))
#define CPR232(reg)				MEM(LOPTR(&mips3.core->cpr[2][reg]))
#define CCR232(reg)				MEM(LOPTR(&mips3.core->ccr[2][reg]))

#define R64(reg)				((reg == 0) ? DRCUML_PTYPE_IMMEDIATE : DRCUML_PTYPE_MEMORY), ((reg) == 0) ? 0 : (FPTR)&mips3.core->r[reg]
#define LO64					R64(REG_LO)
#define HI64					R64(REG_HI)
#define CPR064(reg)				MEM(&mips3.core->cpr[0][reg])
#define CCR064(reg)				MEM(&mips3.core->ccr[0][reg])
#define FPR64(reg)				MEM(mips3.mode ? (double *)&mips3.core->cpr[1][(reg)/2] : (double *)&mips3.core->cpr[1][reg])
#define CCR164(reg)				MEM(&mips3.core->ccr[1][reg])
#define CPR264(reg)				MEM(&mips3.core->cpr[2][reg])
#define CCR264(reg)				MEM(&mips3.core->ccr[2][reg])

#define FCCMASK(which)			fcc_mask[(mips3.core->flavor < MIPS3_TYPE_MIPS_IV) ? 0 : ((which) & 7)]



/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* fast RAM info */
typedef struct _fast_ram_info fast_ram_info;
struct _fast_ram_info
{
	offs_t				start;						/* start of the RAM block */
	offs_t				end;						/* end of the RAM block */
	UINT8				readonly;					/* TRUE if read-only */
	void *				base;						/* base in memory where the RAM lives */
};


/* hotspot info */
typedef struct _hotspot_info hotspot_info;
struct _hotspot_info
{
	offs_t				pc;							/* PC to consider */
	UINT32				opcode;						/* required opcode at that PC */
	UINT32				cycles;						/* number of cycles to eat when hit */
};


/* internal compiler state */
typedef struct _compiler_state compiler_state;
struct _compiler_state
{
	UINT32				cycles;						/* accumulated cycles */
	drcuml_codelabel	labelnum;					/* index for local labels */
};


/* MIPS3 registers */
typedef struct _mips3_regs mips3_regs;
struct _mips3_regs
{
	/* core state */
	drccache *			cache;						/* pointer to the DRC code cache */
	drcuml_state *		drcuml;						/* DRC UML generator state */
	drcfe_state *		drcfe;						/* pointer to the DRC front-end state */
	UINT32				drcoptions;					/* configurable DRC options */
	mips3_state *		core;						/* pointer to the core MIPS3 state */

	/* internal stuff */
	UINT32				mode;						/* current global mode */
	UINT8				cache_dirty;				/* true if we need to flush the cache */

	/* subroutines */
	drcuml_codehandle	entry;						/* entry point */
	drcuml_codehandle	nocode;						/* nocode exception handler */
	drcuml_codehandle	out_of_cycles;				/* out of cycles exception handler */
	drcuml_codehandle	exception[EXCEPTION_COUNT];	/* array of exception handlers */
	drcuml_codehandle	interrupt_norecover;		/* no-recover interrupt handler */
	drcuml_codehandle	tlb_mismatch;				/* tlb mismatch at current PC */
	drcuml_codehandle	read8;						/* read byte */
	drcuml_codehandle	write8;						/* write byte */
	drcuml_codehandle	read16;						/* read half */
	drcuml_codehandle	write16;					/* write half */
	drcuml_codehandle	read32;						/* read word */
	drcuml_codehandle	read32mask;					/* read word masked */
	drcuml_codehandle	write32;					/* write word */
	drcuml_codehandle	write32mask;				/* write word masked */
	drcuml_codehandle	read64;						/* read double */
	drcuml_codehandle	read64mask;					/* read double masked */
	drcuml_codehandle	write64;					/* write double */
	drcuml_codehandle	write64mask;				/* write double masked */
	
	/* parameters for subroutines */
	UINT64				numcycles;					/* return value from gettotalcycles */
	UINT32				tlbepc;						/* EPC for TLB faults */

	/* fast RAM */
	UINT32				fastram_select;
	fast_ram_info		fastram[MIPS3_MAX_FASTRAM];

	/* hotspots */
	UINT32				hotspot_select;
	hotspot_info		hotspot[MIPS3_MAX_HOTSPOTS];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void code_flush_cache(drcuml_state *drcuml);
static void code_compile_block(drcuml_state *drcuml, UINT8 mode, offs_t pc);

static void cfunc_printf_exception(void *param);
static void cfunc_get_cycles(void *param);
static void cfunc_printf_probe(void *param);

static void static_generate_entry_point(drcuml_state *drcuml);
static void static_generate_nocode_handler(drcuml_state *drcuml);
static void static_generate_out_of_cycles(drcuml_state *drcuml);
static void static_generate_exception(drcuml_state *drcuml, UINT8 exception, const char *name);
static void static_generate_memory_accessor(drcuml_state *drcuml, int size, int iswrite, int ismasked, const char *name, drcuml_codehandle *handle);

static void generate_update_cycles(drcuml_block *block, compiler_state *compiler, UINT32 nextpc, UINT32 *mempc);
static void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
static void generate_check_interrupts(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static void generate_delay_slot_and_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg);
static int generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_special(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_regimm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_idt(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_set_cop0_reg(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 reg);
static int generate_get_cop0_reg(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 reg);
static int generate_cop0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_cop1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_cop1x(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);

static int generate_load_word_partial(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int is_right);
static int generate_load_double_partial(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int is_right);
static int generate_store_word_partial(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int is_right);
static int generate_store_double_partial(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int is_right);

static void log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op);
static const char *log_desc_flags_to_string(UINT32 flags);
static void log_register_list(drcuml_state *drcuml, const char *string, UINT64 gprmask, UINT64 fprmask);
static void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent);



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static mips3_regs mips3;
static size_t allocatedsize;

/* bit indexes for various FCCs */
static const UINT32 fcc_mask[8] = { 1 << 23, 1 << 25, 1 << 26, 1 << 27, 1 << 28, 1 << 29, 1 << 30, 1 << 31 };

/* flag lookup table for SLT */
static const UINT64 slt_table[16] =
{
	/* .... */	0,
	/* ...C */	0,
	/* ..V. */	1,
	/* ..VC */	1,
	/* .Z.. */	0,
	/* .Z.C */	0,
	/* .ZV. */	1,
	/* .ZVC */	1,
	/* S... */	1,
	/* S..C */	1,
	/* S.V. */	0,
	/* S.VC */	0,
	/* SZ.. */	1,
	/* SZ.C */	1,
	/* SZV. */	0,
	/* SZVC */	0
};

/* flag lookup table for SLTU */
static const UINT64 sltu_table[16] =
{
	/* .... */	0,
	/* ...C */	1,
	/* ..V. */	0,
	/* ..VC */	1,
	/* .Z.. */	0,
	/* .Z.C */	1,
	/* .ZV. */	0,
	/* .ZVC */	1,
	/* S... */	0,
	/* S..C */	1,
	/* S.V. */	0,
	/* S.VC */	1,
	/* SZ.. */	0,
	/* SZ.C */	1,
	/* SZV. */	0,
	/* SZVC */	1
};

/* flag lookup table for C.EQ */
static const UINT32 c_eq_table[16] =
{
	/* .... */	0,
	/* ...C */	0,
	/* ..V. */	0,
	/* ..VC */	0,
	/* .Z.. */	~0,
	/* .Z.C */	~0,
	/* .ZV. */	~0,
	/* .ZVC */	~0,
	/* S... */	0,
	/* S..C */	0,
	/* S.V. */	0,
	/* S.VC */	0,
	/* SZ.. */	~0,
	/* SZ.C */	~0,
	/* SZV. */	~0,
	/* SZVC */	~0
};

/* flag lookup table for C.LT */
static const UINT32 c_lt_table[16] =
{
	/* .... */	0,
	/* ...C */	~0,
	/* ..V. */	0,
	/* ..VC */	~0,
	/* .Z.. */	0,
	/* .Z.C */	~0,
	/* .ZV. */	0,
	/* .ZVC */	~0,
	/* S... */	0,
	/* S..C */	~0,
	/* S.V. */	0,
	/* S.VC */	~0,
	/* SZ.. */	0,
	/* SZ.C */	~0,
	/* SZV. */	0,
	/* SZVC */	~0
};

/* flag lookup table for C.LE */
static const UINT32 c_le_table[16] =
{
	/* .... */	0,
	/* ...C */	~0,
	/* ..V. */	0,
	/* ..VC */	~0,
	/* .Z.. */	~0,
	/* .Z.C */	~0,
	/* .ZV. */	~0,
	/* .ZVC */	~0,
	/* S... */	0,
	/* S..C */	~0,
	/* S.V. */	0,
	/* S.VC */	~0,
	/* SZ.. */	~0,
	/* SZ.C */	~0,
	/* SZV. */	~0,
	/* SZVC */	~0
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    epc - compute the exception PC from a 
    descriptor
-------------------------------------------------*/

INLINE UINT32 epc(const opcode_desc *desc)
{
	return (desc->flags & OPFLAG_IN_DELAY_SLOT) ? (desc->pc - 3) : desc->pc;
}



/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    mips3_init - initialize the processor
-------------------------------------------------*/

static void mips3_init(mips3_flavor flavor, int bigendian, int index, int clock, const struct mips3_config *config, int (*irqcallback)(int))
{
	drcfe_config feconfig =
	{
		COMPILE_BACKWARDS_BYTES,	/* code window start offset = startpc - window_start */
		COMPILE_FORWARDS_BYTES,		/* code window end offset = startpc + window_end */
		COMPILE_MAX_SEQUENCE,		/* maximum instructions to include in a sequence */
		mips3fe_describe			/* callback to describe a single instruction */
	};
	UINT32 flags = 0;
	size_t extrasize;
	void *extramem;
	
	/* determine how much memory beyond the core size we need */
 	extrasize = mips3com_init(NULL, flavor, bigendian, index, clock, config, irqcallback, NULL);
 
	/* allocate enough space for the cache and the core */
	mips3.cache = drccache_alloc(CACHE_SIZE + sizeof(*mips3.core) + extrasize);
	if (mips3.cache == NULL)
		fatalerror("Unable to allocate cache of size %d", (UINT32)allocatedsize);
	
	/* allocate the core and the extra memory */
	mips3.core = drccache_memory_alloc(mips3.cache, sizeof(*mips3.core));
	extramem = drccache_memory_alloc(mips3.cache, extrasize);

	/* initialize the core */
	mips3com_init(mips3.core, flavor, bigendian, index, clock, config, irqcallback, extramem);
	
	/* initialize the UML generator */
	if (LOG_UML)
		flags |= DRCUML_OPTION_LOG_UML;
	if (LOG_NATIVE)
		flags |= DRCUML_OPTION_LOG_NATIVE;
	mips3.drcuml = drcuml_alloc(mips3.cache, flags | DRCUML_OPTION_USE_C, 2, 32, 2);
	if (mips3.drcuml == NULL)
		fatalerror("Error initializing the UML");
	
	/* initialize the front-end helper */
	if (Machine->debug_mode || SINGLE_INSTRUCTION_MODE)
		feconfig.max_sequence = 1;
	mips3.drcfe = drcfe_init(&feconfig, mips3.core);
	
	/* mark the cache dirty so it is updated on next execute */
	mips3.cache_dirty = TRUE;
}


/*-------------------------------------------------
    mips3_reset - reset the processor
-------------------------------------------------*/

static void mips3_reset(void)
{
	/* reset the common code and mark the cache dirty */
	mips3com_reset(mips3.core);
	mips3.cache_dirty = TRUE;
}


/*-------------------------------------------------
    mips3_execute - execute the CPU for the
    specified number of cycles
-------------------------------------------------*/

static int mips3_execute(int cycles)
{
	drcuml_state *drcuml = mips3.drcuml;
	int execute_result;
	
	/* reset the cache if dirty */
	if (mips3.cache_dirty)
		code_flush_cache(drcuml);
	mips3.cache_dirty = FALSE;

	/* execute */
	mips3.core->icount = cycles;
	do
	{
		/* run as much as we can */
		execute_result = drcuml_execute(drcuml, mips3.entry);
		
		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
			code_compile_block(drcuml, mips3.mode, mips3.core->pc);
			
	} while (execute_result != EXECUTE_OUT_OF_CYCLES);

	/* return the number of cycles executed */
	return cycles - mips3.core->icount;
}


/*-------------------------------------------------
    mips3_exit - cleanup from execution
-------------------------------------------------*/

static void mips3_exit(void)
{
	/* clean up the DRC */
	drcfe_exit(mips3.drcfe);
	drcuml_free(mips3.drcuml);
	drccache_free(mips3.cache);
}


/*-------------------------------------------------
    mips3_get_context - return a copy of the
    current context
-------------------------------------------------*/

static void mips3_get_context(void *dst)
{
	if (dst != NULL)
		*(mips3_regs *)dst = mips3;
}


/*-------------------------------------------------
    mips3_set_context - copy the current context
    into the global state
-------------------------------------------------*/

static void mips3_set_context(void *src)
{
	if (src != NULL)
		mips3 = *(mips3_regs *)src;
}


/*-------------------------------------------------
    mips3_translate - perform virtual-to-physical
    address translation
-------------------------------------------------*/

static int mips3_translate(int space, offs_t *address)
{
	return mips3com_translate_address(mips3.core, space, address);
}


/*-------------------------------------------------
    mips3_dasm - disassemble an instruction
-------------------------------------------------*/

#ifdef ENABLE_DEBUGGER
static offs_t mips3_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	return mips3com_dasm(mips3.core, buffer, pc, oprom, opram);
}
#endif /* ENABLE_DEBUGGER */




/***************************************************************************
    COMMON UTILITIES
***************************************************************************/

/*-------------------------------------------------
    fastram_ptr - return a pointer to the base of
    memory containing the given address, if it
    is within a fast RAM range; returns NULL
    otherwise
-------------------------------------------------*/

#ifdef UNUSED_CODE
static void *fastram_ptr(offs_t addr, int size, int iswrite)
{
	int ramnum;

	/* only direct maps are supported */
	if (addr < 0x80000000 || addr >= 0xc0000000)
		return NULL;
	addr &= 0x1fffffff;

	/* adjust address for endianness */
	if (mips3.core->bigendian)
	{
		if (size == 1)
			addr ^= 3;
		else if (size == 2)
			addr ^= 2;
	}

	/* search for fastram */
	for (ramnum = 0; ramnum < MIPS3_MAX_FASTRAM; ramnum++)
		if (mips3.fastram[ramnum].base != NULL && (!iswrite || !mips3.fastram[ramnum].readonly) &&
			addr >= mips3.fastram[ramnum].start && addr <= mips3.fastram[ramnum].end)
		{
			return (UINT8 *)mips3.fastram[ramnum].base + (addr - mips3.fastram[ramnum].start);
		}

	return NULL;
}
#endif



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

static void code_flush_cache(drcuml_state *drcuml)
{
	/* empty the transient cache contents */
	drcuml_reset(drcuml);

	static_generate_nocode_handler(drcuml);
	static_generate_out_of_cycles(drcuml);
	
	/* append exception handlers for various types */
	static_generate_exception(drcuml, EXCEPTION_INTERRUPT, "exception_interrupt");
	static_generate_exception(drcuml, EXCEPTION_TLBMOD,    "exception_tlbmod");
	static_generate_exception(drcuml, EXCEPTION_TLBLOAD,   "exception_tlbload");
	static_generate_exception(drcuml, EXCEPTION_TLBSTORE,  "exception_tlbstore");
	static_generate_exception(drcuml, EXCEPTION_SYSCALL,   "exception_syscall");
	static_generate_exception(drcuml, EXCEPTION_BREAK,     "exception_break");
	static_generate_exception(drcuml, EXCEPTION_INVALIDOP, "exception_invalidop");
	static_generate_exception(drcuml, EXCEPTION_BADCOP,    "exception_badcop");
	static_generate_exception(drcuml, EXCEPTION_OVERFLOW,  "exception_overflow");
	static_generate_exception(drcuml, EXCEPTION_TRAP,      "exception_trap");
	
	/* add subroutines for memory accesses */
	static_generate_memory_accessor(drcuml, 1, FALSE, FALSE, "read8",       &mips3.read8);
	static_generate_memory_accessor(drcuml, 1, TRUE,  FALSE, "write8",      &mips3.write8);
	static_generate_memory_accessor(drcuml, 2, FALSE, FALSE, "read16",      &mips3.read16);
	static_generate_memory_accessor(drcuml, 2, TRUE,  FALSE, "write16",     &mips3.write16);
	static_generate_memory_accessor(drcuml, 4, FALSE, FALSE, "read32",      &mips3.read32);
	static_generate_memory_accessor(drcuml, 4, FALSE, TRUE,  "read32mask",  &mips3.read32mask);
	static_generate_memory_accessor(drcuml, 4, TRUE,  FALSE, "write32",     &mips3.write32);
	static_generate_memory_accessor(drcuml, 4, TRUE,  TRUE,  "write32mask", &mips3.write32mask);
	static_generate_memory_accessor(drcuml, 8, FALSE, FALSE, "read64",      &mips3.read64);
	static_generate_memory_accessor(drcuml, 8, FALSE, TRUE,  "read64mask",  &mips3.read64mask);
	static_generate_memory_accessor(drcuml, 8, TRUE,  FALSE, "write64",     &mips3.write64);
	static_generate_memory_accessor(drcuml, 8, TRUE,  TRUE,  "write64mask", &mips3.write64mask);

	/* generate the entry point and out-of-cycles handlers */
	static_generate_entry_point(drcuml);
}


/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

static void code_compile_block(drcuml_state *drcuml, UINT8 mode, offs_t pc)
{
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *desclist;
	int override = FALSE;
	drcuml_block *block;
	jmp_buf errorbuf;
	
	/* get a description of this sequence */
	desclist = drcfe_describe_code(mips3.drcfe, pc);
	if (LOG_UML)
		log_opcode_desc(drcuml, desclist, 0);

	/* if we get an error back, flush the cache and try again */
	if (setjmp(errorbuf) != 0)
		code_flush_cache(drcuml);
	
	/* start the block */
	block = drcuml_block_begin(drcuml, 4096, &errorbuf);

	/* loop until we get through all instruction sequences */
	for (seqhead = desclist; seqhead != NULL; seqhead = seqlast->next)
	{
		const opcode_desc *curdesc;
		UINT32 nextpc;

		/* add a code log entry */
		if (LOG_UML)
			UML_COMMENT(block, "-------------------------");						// comment

		/* label this instruction, since it may be jumped to locally */
		UML_LABEL(block, seqhead->pc | 0x80000000);									// label   seqhead->pc | 0x80000000

		/* determine the last instruction in this sequence */
		for (seqlast = seqhead; seqlast != NULL; seqlast = seqlast->next)
			if (seqlast->flags & OPFLAG_END_SEQUENCE)
				break;
		assert(seqlast != NULL);

		/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
		if (override || !drcuml_hash_exists(drcuml, mode, seqhead->pc))
			UML_HASH(block, mode, seqhead->pc);										// hash    mode,pc

		/* if we already have a hash, and this is the first sequence, assume that we */
		/* are recompiling due to being out of sync and allow future overrides */
		else if (seqhead == desclist)
		{
			override = TRUE;
			UML_HASH(block, mode, seqhead->pc);										// hash    mode,pc
		}
			
		/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
		else
		{
			UML_HASHJMP(block, IMM(mips3.mode), IMM(seqhead->pc), mips3.nocode);	// hashjmp <mode>,seqhead->pc,nocode
			continue;
		}
		
		/* validate this code block if we're not pointing into ROM */
		if (memory_get_write_ptr(cpu_getactivecpu(), ADDRESS_SPACE_PROGRAM, seqhead->physpc) != NULL)
			generate_checksum_block(block, &compiler, seqhead, seqlast);

		/* iterate over instructions in the sequence and compile them */
		for (curdesc = seqhead; curdesc != seqlast->next; curdesc = curdesc->next)
			generate_sequence_instruction(block, &compiler, curdesc);
		
		/* if this is a likely branch, the fall through case needs to skip the next instruction */
		if ((seqlast->flags & OPFLAG_IS_CONDITIONAL_BRANCH) && seqlast->skipslots > 0 && seqlast->next != NULL && seqlast->next->pc != seqlast->pc + 8)
			nextpc = seqlast->pc + 8;

		/* if we need a redispatch, do it now */
		else if (seqlast->flags & OPFLAG_REDISPATCH)
			nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;

 		/* if we need to return to the start, do it */
		else if (seqlast->flags & OPFLAG_RETURN_TO_START)
			nextpc = pc;

		/* otherwise we just go to the next instruction */
		else
			nextpc = seqlast->pc + 4;
		
		/* count off cycles and go there */
		generate_update_cycles(block, &compiler, nextpc, NULL);						// <subtract cycles>
		if (seqlast->next == NULL || seqlast->next->pc != nextpc)
			UML_HASHJMP(block, IMM(mips3.mode), IMM(nextpc), mips3.nocode);			// hashjmp <mode>,nextpc,nocode
	}

	/* end the sequence */
	drcuml_block_end(block);
}



/***************************************************************************
    C FUNCTION CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    cfunc_printf_exception - log any exceptions that
    aren't interrupts
-------------------------------------------------*/

static void cfunc_printf_exception(void *param)
{
	printf("Exception: EPC=%08X Cause=%08X BadVAddr=%08X Jmp=%08X\n", (UINT32)mips3.core->cpr[0][COP0_EPC], (UINT32)mips3.core->cpr[0][COP0_Cause], (UINT32)mips3.core->cpr[0][COP0_BadVAddr], mips3.core->pc);
}


/*-------------------------------------------------
    cfunc_get_cycles - compute the total number
    of cycles executed so far
-------------------------------------------------*/

static void cfunc_get_cycles(void *param)
{
	UINT64 *dest = param;
	*dest = activecpu_gettotalcycles();
}


/*-------------------------------------------------
    cfunc_printf_probe - print the current CPU 
    state and return
-------------------------------------------------*/

static void cfunc_printf_probe(void *param)
{
	UINT32 pc = (UINT32)(FPTR)param;

	printf(" PC=%08X          r1=%08X%08X  r2=%08X%08X  r3=%08X%08X\n",
		pc,
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



/***************************************************************************
    STATIC CODEGEN
***************************************************************************/

/*-------------------------------------------------
    static_generate_entry_point - generate a 
    static entry point
-------------------------------------------------*/

static void static_generate_entry_point(drcuml_state *drcuml)
{
	drcuml_codelabel skip = 1;
	drcuml_block *block;
	jmp_buf errorbuf;
	
	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_entry_point");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 10, &errorbuf);
	
	mips3.entry = UML_HANDLE(block, "entry");										// handle  block,name

	/* check for interrupts */
	UML_AND(block, IREG(0), CPR032(COP0_Cause), CPR032(COP0_Status));				// and     i0,[Cause],[Status]
	UML_ANDf(block, IREG(0), IREG(0), IMM(0xfc00), FLAGS_Z);						// and     i0,i0,0xfc00,Z
	UML_JMPc(block, IF_Z, skip);													// jmp     skip,Z
	UML_TEST(block, CPR032(COP0_Status), IMM(SR_IE));								// test    [Status],SR_IE
	UML_JMPc(block, IF_Z, skip);													// jmp     skip,Z
	UML_TEST(block, CPR032(COP0_Status), IMM(SR_EXL | SR_ERL));						// test    [Status],SR_EXL | SR_ERL
	UML_JMPc(block, IF_NZ, skip);													// jmp     skip,NZ
	UML_MOV(block, IREG(0), MEM(&mips3.core->pc));									// mov     i0,pc
	UML_CALLH(block, mips3.interrupt_norecover);									// callh   interrupt_norecover
	UML_LABEL(block, skip);														// skip:

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, MEM(&mips3.mode), MEM(&mips3.core->pc), mips3.nocode);		// hashjmp <mode>,<pc>,nocode
	
	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_nocode_handler - generate an 
    exception handler for "out of code"
-------------------------------------------------*/

static void static_generate_nocode_handler(drcuml_state *drcuml)
{
	drcuml_block *block;
	jmp_buf errorbuf;
	
	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_nocode_handler");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 10, &errorbuf);
	
	/* generate a hash jump via the current mode and PC */
	mips3.nocode = UML_HANDLE(block, "nocode");										// handle  block,name
	UML_GETEXP(block, IREG(0));														// getexp  i0
	UML_MOV(block, MEM(&mips3.core->pc), IREG(0));									// mov     [pc],i0
	UML_EXIT(block, IMM(EXECUTE_MISSING_CODE));										// exit    EXECUTE_MISSING_CODE
	
	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

static void static_generate_out_of_cycles(drcuml_state *drcuml)
{
	drcuml_block *block;
	jmp_buf errorbuf;
	
	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_out_of_cycles");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 10, &errorbuf);
	
	/* generate a hash jump via the current mode and PC */
	mips3.out_of_cycles = UML_HANDLE(block, "out_of_cycles");						// handle  block,name
	UML_GETEXP(block, IREG(0));														// getexp  i0
	UML_MOV(block, MEM(&mips3.core->pc), IREG(0));									// mov     <pc>,i0
	UML_EXIT(block, IMM(EXECUTE_OUT_OF_CYCLES));									// exit    EXECUTE_OUT_OF_CYCLES
	
	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_exception - generate a static
    exception handler
-------------------------------------------------*/

static void static_generate_exception(drcuml_state *drcuml, UINT8 exception, const char *name)
{
	UINT32 offset = (exception >= EXCEPTION_TLBMOD && exception <= EXCEPTION_TLBSTORE) ? 0x00 : 0x180;
	drcuml_codelabel next = 1;
	drcuml_block *block;
	jmp_buf errorbuf;
	
	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_exception");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 1024, &errorbuf);
	
	/* add a global entry for this */
	mips3.exception[exception] = UML_HANDLE(block, name);							// handle  name

	/* exception parameter is expected to be the fault address in this case */
	if (exception == EXCEPTION_TLBLOAD || exception == EXCEPTION_TLBSTORE)
	{
		/* set BadVAddr to the fault address */
		UML_GETEXP(block, CPR032(COP0_BadVAddr));									// mov     [BadVAddr],exp

		/* set the upper bits of EntryHi to the fault page */
		UML_AND(block, IREG(0), IREG(0), IMM(0xffffe000));							// and     i0,i0,0xffffe000
		UML_AND(block, IREG(1), CPR032(COP0_EntryHi), IMM(0x000000ff));				// and     i1,[EntryHi],0x000000ff
		UML_OR(block, CPR032(COP0_EntryHi), IREG(0), IREG(1));						// or      [EntryHi],i0,i1
		
		/* set the lower bits of Context to the fault page */
		UML_SHR(block, IREG(0), IREG(0), IMM(9));									// shr     i0,i0,9
		UML_AND(block, IREG(1), CPR032(COP0_Context), IMM(0xff800000));				// and     i1,[Context],0xff800000
		UML_OR(block, CPR032(COP0_Context), IREG(0), IREG(1));						// or      [Context],i0,i1
	}

	/* set the EPC and Cause registers */
	UML_RECOVER(block, IREG(0), MAPVAR_PC);											// recover i0,PC
	UML_RECOVER(block, IREG(1), MAPVAR_CYCLES);										// recover i1,CYCLES

	if (exception == EXCEPTION_INTERRUPT)
		mips3.interrupt_norecover = UML_HANDLE(block, "exception_interrupt_norecover");	// handle  exception_interrupt_norecover

	UML_AND(block, IREG(2), CPR032(COP0_Cause), IMM(~0x800000ff));					// and     i2,[Cause],~0x800000ff
	UML_TEST(block, IREG(0), IMM(1));												// test    i0,1
	UML_JMPc(block, IF_Z, next);													// jz      <next>
	UML_OR(block, IREG(2), IREG(2), IMM(0x80000000));								// or      i2,i2,0x80000000
	UML_SUB(block, IREG(0), IREG(0), IMM(1));										// sub     i0,i0,1
	UML_LABEL(block, next);														// <next>:
	UML_MOV(block, CPR032(COP0_EPC), IREG(0));										// mov     [EPC],i0
	UML_OR(block, CPR032(COP0_Cause), IREG(2), IMM(exception << 2));				// or      [Cause],i2,exception << 2
	
	/* set EXL in the SR */
	UML_OR(block, CPR032(COP0_Status), CPR032(COP0_Status), IMM(SR_EXL));			// or      [Status],[Status],SR_EXL
	
	/* optionally print exceptions */
	if (PRINTF_EXCEPTIONS && exception != EXCEPTION_INTERRUPT)
		UML_CALLC(block, cfunc_printf_exception, NULL);								// callc   cfunc_printf_exception,NULL
	
	/* choose our target PC */
	UML_MOV(block, IREG(0), IMM(0xbfc00200 + offset));								// mov     i0,0xbfc00200 + offset
	UML_TEST(block, CPR032(COP0_Status), IMM(SR_BEV));								// test    [Status],SR_BEV
	UML_MOVc(block, IF_Z, IREG(0), IMM(0x80000000 + offset));						// mov     i0,0x80000000 + offset,z
	
	/* adjust cycles */
	UML_SUBf(block, MEM(&mips3.core->icount), MEM(&mips3.core->icount), IREG(1), FLAGS_S); // sub icount,icount,cycles,S
	UML_EXHc(block, IF_S, mips3.out_of_cycles, IREG(0));							// exh     out_of_cycles,i0
	
	UML_HASHJMP(block, MEM(&mips3.mode), IREG(0), mips3.nocode);					// hashjmp <mode>,i0,nocode
	
	drcuml_block_end(block);
}


/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

static void static_generate_memory_accessor(drcuml_state *drcuml, int size, int iswrite, int ismasked, const char *name, drcuml_codehandle *handle)
{
	/* on entry, address is in I0; data for writes is in I1; mask for accesses is in I2 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I3 */
	drcuml_codehandle exception_target = mips3.exception[iswrite ? EXCEPTION_TLBSTORE : EXCEPTION_TLBLOAD];
	drcuml_block *block;
	jmp_buf errorbuf;
	
	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_exception");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 1024, &errorbuf);

	/* add a global entry for this */
	*handle = UML_HANDLE(block, name);												// handle  block,name

	/* general case: assume paging and perform a translation */
	UML_SHR(block, IREG(3), IREG(0), IMM(12));										// shr     i3,i0,12
	UML_LOAD4(block, IREG(3), mips3.core->tlb_table, IREG(3));						// load4   i3,[tlb_table],i3
	UML_TEST(block, IREG(3), IMM(iswrite ? 1 : 2));									// test    i3,iswrite ? 1 : 2
	UML_EXHc(block, IF_NZ, exception_target, IREG(0));								// exnz    exception_target,i0
	UML_AND(block, IREG(0), IREG(0), IMM(0xfff));									// and     i0,i0,0xfff
	UML_AND(block, IREG(3), IREG(3), IMM(~0xfff));									// and     i3,i3,~0xfff
	UML_OR(block, IREG(0), IREG(0), IREG(3));										// or      i0,i0,i3

	switch (size)
	{
		case 1:
			if (iswrite)
				UML_WRITE1(block, PROGRAM, IREG(0), IREG(1));						// write1  i0,i1
			else
				UML_DREAD1U(block, IREG(0), PROGRAM, IREG(0));						// dread1u i0,i0
			break;
		
		case 2:
			if (iswrite)
				UML_WRITE2(block, PROGRAM, IREG(0), IREG(1));						// write2  i0,i1
			else
				UML_DREAD2U(block, IREG(0), PROGRAM, IREG(0));						// dread2u i0,i0
			break;
		
		case 4:
			if (iswrite)
			{
				if (!ismasked)
					UML_WRITE4(block, PROGRAM, IREG(0), IREG(1));					// write4  PROGRAM,i0,i1
				else
					UML_WRIT4M(block, PROGRAM, IREG(0), IREG(2), IREG(1));			// writ4m  PROGRAM,i0,i2,i1
			}
			else
			{
				if (!ismasked)
					UML_DREAD4U(block, IREG(0), PROGRAM, IREG(0));					// dread4u i0,PROGRAM,i0
				else
					UML_DREAD4M(block, IREG(0), PROGRAM, IREG(0), IREG(2));			// dread4m i0,PROGRAM,i0,i2
			}
			break;
		
		case 8:
			if (iswrite)
			{
				if (!ismasked)
					UML_DWRITE8(block, PROGRAM, IREG(0), IREG(1));					// dwrite8 PROGRAM,i0,i1
				else
					UML_DWRIT8M(block, PROGRAM, IREG(0), IREG(2), IREG(1));			// dwrit8m PROGRAM,i0,i2,i1
			}
			else
			{
				if (!ismasked)
					UML_DREAD8(block, IREG(0), PROGRAM, IREG(0));					// dread8  i0,PROGRAM,i0
				else
					UML_DREAD8M(block, IREG(0), PROGRAM, IREG(0), IREG(2));			// dread8m i0,PROGRAM,i0,i2
			}
			break;
	}
	UML_RET(block);																	// ret
	
	drcuml_block_end(block);
}



/***************************************************************************
    CODE GENERATION
***************************************************************************/

/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/

static void generate_update_cycles(drcuml_block *block, compiler_state *compiler, UINT32 nextpc, UINT32 *mempc)
{
	if (compiler->cycles > 0)
	{
		UML_SUBf(block, MEM(&mips3.core->icount), MEM(&mips3.core->icount), 
					MAPVAR_CYCLES, FLAGS_S);										// sub     icount,cycles,S
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);										// mapvar  cycles,0
		if (mempc == NULL)
			UML_EXHc(block, IF_S, mips3.out_of_cycles, IMM(nextpc));				// exh     out_of_cycles,nextpc
		else
			UML_EXHc(block, IF_S, mips3.out_of_cycles, MEM(mempc));					// exh     out_of_cycles,<mempc>
	}
	compiler->cycles = 0;
}


/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

static void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (LOG_UML)
		UML_COMMENT(block, "[Validation for %08X]", seqhead->pc);					// comment
	for (curdesc = seqhead; curdesc != seqlast->next; curdesc = curdesc->next)
	{
		UML_CMP(block, MEM(curdesc->opptr.l), IMM(*curdesc->opptr.l));				// cmp     [opptr],*opptr
		UML_EXHc(block, IF_NE, mips3.nocode, IMM(epc(seqhead)));					// exne    nocode,seqhead->pc
	}
}


/*------------------------------------------------------------------
    generate_check_interrupts
------------------------------------------------------------------*/

static void generate_check_interrupts(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	drcuml_codelabel skip;

	UML_AND(block, IREG(0), CPR032(COP0_Cause), CPR032(COP0_Status));				// and     i0,[Cause],[Status]
	UML_ANDf(block, IREG(0), IREG(0), IMM(0xfc00), FLAGS_Z);						// and     i0,i0,0xfc00,Z
	UML_JMPc(block, IF_Z, skip = compiler->labelnum++);								// jmp     skip,Z
	UML_TEST(block, CPR032(COP0_Status), IMM(SR_IE));								// test    [Status],SR_IE
	UML_JMPc(block, IF_Z, skip);													// jmp     skip,Z
	UML_TEST(block, CPR032(COP0_Status), IMM(SR_EXL | SR_ERL));						// test    [Status],SR_EXL | SR_ERL
	UML_EXHc(block, IF_Z, mips3.exception[EXCEPTION_INTERRUPT], IMM(0));			// exh     interrupt,0,Z
	UML_LABEL(block, skip);														// skip:
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

static void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	offs_t expc;
	int hotnum;

	/* add an entry for the log */
	if (LOG_UML)
		log_add_disasm_comment(block, desc->pc, *desc->opptr.l);

	/* set the PC map variable */
	expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 3 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, expc);												// mapvar  PC,expc

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* is this a hotspot? */
	for (hotnum = 0; hotnum < MIPS3_MAX_HOTSPOTS; hotnum++)
		if (desc->pc == mips3.hotspot[hotnum].pc && *desc->opptr.l == mips3.hotspot[hotnum].opcode)
		{
			compiler->cycles += mips3.hotspot[hotnum].cycles;
			break;
		}
	
	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);								// mapvar  CYCLES,compiler->cycles

	/* if we want a probe, add it here */
	if (desc->pc == PROBE_ADDRESS)
		UML_CALLC(block, cfunc_printf_probe, desc->pc);								// callc   cfunc_printf_probe,desc->pc

	/* if we are debugging, call the debugger */
	if (Machine->debug_mode)
	{
		UML_MOV(block, MEM(&mips3.core->pc), IMM(desc->pc));						// mov     [pc],desc->pc
		UML_DEBUG(block, IMM(desc->pc));											// debug   desc->pc
	}

	/* validate our TLB entry at this PC; if we fail, we need to recompile */
	if ((desc->flags & OPFLAG_VALIDATE_TLB) && (desc->pc < 0x80000000 || desc->pc >= 0xc0000000))
	{
		UML_CMP(block, MEM(&mips3.core->tlb_table[desc->pc >> 12]), 
		               IMM(mips3.core->tlb_table[desc->pc >> 12]));					// cmp     [tlbentry],*tlbentry
		UML_EXHc(block, IF_NE, mips3.tlb_mismatch, IMM(0));							// exh     tlb_mismatch,0,NE
	}

	/* if this is an invalid opcode, generate the exception now */
	if (desc->flags & OPFLAG_INVALID_OPCODE)
		UML_EXH(block, mips3.exception[EXCEPTION_INVALIDOP], IMM(0));				// exh     invalidop,0

	/* otherwise, it's a regular instruction */
	else
	{
		/* compile the instruction */
		if (!generate_opcode(block, compiler, desc))
			fatalerror("Unimplemented op %08X (%02X,%02X)", *desc->opptr.l, *desc->opptr.l >> 26, *desc->opptr.l & 0x3f);
	}
}


/*------------------------------------------------------------------
    generate_delay_slot_and_branch
------------------------------------------------------------------*/

static void generate_delay_slot_and_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg)
{
	compiler_state compiler_temp = *compiler;
	UINT32 op = *desc->opptr.l;

	/* set the link if needed */
	if (linkreg != 0)
		UML_DMOV(block, R64(linkreg), IMM((INT32)(desc->pc + 8)));					// mov     <linkreg>,desc->pc + 8

	/* compile the delay slot using temporary compiler state */
	assert(desc->delay != NULL);
	generate_sequence_instruction(block, &compiler_temp, desc->delay);				// <next instruction>

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(block, &compiler_temp, desc->targetpc, NULL);		// <subtract cycles>
		if (desc->flags & OPFLAG_INTRABLOCK_BRANCH)
			UML_JMP(block, desc->targetpc | 0x80000000);							// jmp     desc->targetpc | 0x80000000
		else
			UML_HASHJMP(block, IMM(mips3.mode), IMM(desc->targetpc), mips3.nocode);	// hashjmp <mode>,desc->targetpc,nocode
	}
	else
	{
		generate_update_cycles(block, &compiler_temp, 0, LOPTR(&mips3.core->r[RSREG]));	// <subtract cycles>
		UML_HASHJMP(block, IMM(mips3.mode), R32(RSREG), mips3.nocode);				// hashjmp <mode>,<rsreg>,nocode
	}
	
	/* update the label */
	compiler->labelnum = compiler_temp.labelnum;
}


/*-------------------------------------------------
    generate_opcode - generate code for a specific
    opcode
-------------------------------------------------*/

static int generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT8 opswitch = op >> 26;
	drcuml_codelabel skip;

	switch (opswitch)
	{
		/* ----- sub-groups ----- */

		case 0x00:	/* SPECIAL */
			return generate_special(block, compiler, desc);

		case 0x01:	/* REGIMM */
			return generate_regimm(block, compiler, desc);

		case 0x1c:	/* IDT-specific */
			return generate_idt(block, compiler, desc);


		/* ----- jumps and branches ----- */

		case 0x02:	/* J */
			generate_delay_slot_and_branch(block, compiler, desc, 0);				// <next instruction + hashjmp>
			return TRUE;

		case 0x03:	/* JAL */
			generate_delay_slot_and_branch(block, compiler, desc, 31);				// <next instruction + hashjmp>
			return TRUE;

		case 0x04:	/* BEQ */
		case 0x14:	/* BEQL */
			UML_DCMP(block, R64(RSREG), R64(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_JMPc(block, IF_NE, skip = compiler->labelnum++);					// jmp     skip,NE
			generate_delay_slot_and_branch(block, compiler, desc, 0);				// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;
		
		case 0x05:	/* BNE */
		case 0x15:	/* BNEL */
			UML_DCMP(block, R64(RSREG), R64(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_JMPc(block, IF_E, skip = compiler->labelnum++);						// jmp     skip,E
			generate_delay_slot_and_branch(block, compiler, desc, 0);				// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;
		
		case 0x06:	/* BLEZ */
		case 0x16:	/* BLEZL */
			UML_DCMP(block, R64(RSREG), IMM(0));									// dcmp    <rsreg>,0
			UML_JMPc(block, IF_G, skip = compiler->labelnum++);						// jmp     skip,G
			generate_delay_slot_and_branch(block, compiler, desc, 0);				// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;
		
		case 0x07:	/* BGTZ */
		case 0x17:	/* BGTZL */
			UML_DCMP(block, R64(RSREG), IMM(0));									// dcmp    <rsreg>,0
			UML_JMPc(block, IF_LE, skip = compiler->labelnum++);					// jmp     skip,LE
			generate_delay_slot_and_branch(block, compiler, desc, 0);				// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;
		

		/* ----- immediate arithmetic ----- */

		case 0x0f:	/* LUI */
			if (RTREG != 0)
				UML_DSEXT4(block, R64(RTREG), IMM(SIMMVAL << 16));					// dsext4  <rtreg>,SIMMVAL << 16
			return TRUE;

		case 0x08:	/* ADDI */
			UML_ADDf(block, IREG(0), R32(RSREG), IMM(SIMMVAL), FLAGS_V);			// add     i0,<rsreg>,SIMMVAL,V
			UML_EXHc(block, IF_V, mips3.exception[EXCEPTION_OVERFLOW], IMM(0));		// exh    overflow,0
			if (RTREG != 0)
				UML_DSEXT4(block, R64(RTREG), IREG(0));								// dsext4  <rtreg>,i0
			return TRUE;

		case 0x09:	/* ADDIU */
			if (RTREG != 0)
			{
				UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));					// add     i0,<rsreg>,SIMMVAL,V
				UML_DSEXT4(block, R64(RTREG), IREG(0));								// dsext4  <rtreg>,i0
			}
			return TRUE;

		case 0x18:	/* DADDI */
			UML_DADDf(block, IREG(0), R64(RSREG), IMM(SIMMVAL), FLAGS_V);			// dadd    i0,<rsreg>,SIMMVAL,V
			UML_EXHc(block, IF_V, mips3.exception[EXCEPTION_OVERFLOW], IMM(0));		// exh    overflow,0
			if (RTREG != 0)
				UML_DMOV(block, R64(RTREG), IREG(0));								// dmov    <rtreg>,i0
			return TRUE;

		case 0x19:	/* DADDIU */
			if (RTREG != 0)
				UML_DADD(block, R64(RTREG), R64(RSREG), IMM(SIMMVAL));				// dadd    <rtreg>,<rsreg>,SIMMVAL
			return TRUE;

		case 0x0c:	/* ANDI */
			if (RTREG != 0)
				UML_DAND(block, R64(RTREG), R64(RSREG), IMM(UIMMVAL));				// dand    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0d:	/* ORI */
			if (RTREG != 0)
				UML_DOR(block, R64(RTREG), R64(RSREG), IMM(UIMMVAL));				// dor     <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0e:	/* XORI */
			if (RTREG != 0)
				UML_DXOR(block, R64(RTREG), R64(RSREG), IMM(UIMMVAL));				// dxor    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0a:	/* SLTI */
			if (RTREG != 0)
			{
				UML_DCMP(block, R64(RSREG), IMM(SIMMVAL));							// dcmp    <rsreg>,SIMMVAL
				UML_DFLAGS(block, R64(RTREG), (UINT64)~0, slt_table);				// dflags  <rtreg>,~0,slt_table
			}
			return TRUE;

		case 0x0b:	/* SLTIU */
			if (RTREG != 0)
			{
				UML_DCMP(block, R64(RSREG), IMM(SIMMVAL));							// dcmp    <rsreg>,SIMMVAL
				UML_DFLAGS(block, R64(RTREG), (UINT64)~0, sltu_table);				// dflags  <rtreg>,~0,sltu_table
			}
			return TRUE;


		/* ----- memory load operations ----- */

		case 0x20:	/* LB */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read8);											// callh   read8
			if (RTREG != 0)
				UML_DSEXT1(block, R64(RTREG), IREG(0));								// dsext1  <rtreg>,i0
			return TRUE;
		
		case 0x21:	/* LH */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read16);											// callh   read16
			if (RTREG != 0)
				UML_DSEXT2(block, R64(RTREG), IREG(0));								// dsext2  <rtreg>,i0
			return TRUE;
		
		case 0x23:	/* LW */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read32);											// callh   read32
			if (RTREG != 0)
				UML_DSEXT4(block, R64(RTREG), IREG(0));								// dsext4  <rtreg>,i0
			return TRUE;
		
		case 0x24:	/* LBU */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read8);											// callh   read8
			if (RTREG != 0)
				UML_DZEXT1(block, R64(RTREG), IREG(0));								// dzext1  <rtreg>,i0
			return TRUE;
		
		case 0x25:	/* LHU */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read16);											// callh   read16
			if (RTREG != 0)
				UML_DZEXT2(block, R64(RTREG), IREG(0));								// dzext2  <rtreg>,i0
			return TRUE;
		
		case 0x27:	/* LWU */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read32);											// callh   read32
			if (RTREG != 0)
				UML_DZEXT4(block, R64(RTREG), IREG(0));								// dzext4  <rtreg>,i0
			return TRUE;
		
		case 0x37:	/* LD */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read64);											// callh   read64
			if (RTREG != 0)
				UML_DMOV(block, R64(RTREG), IREG(0));								// dmov    <rtreg>,i0
			return TRUE;
		
		case 0x31:	/* LWC1 */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read32);											// callh   read32
			UML_MOV(block, FPR32(RTREG), IREG(0));									// mov     <cpr1_rt>,i0
			return TRUE;

		case 0x35:	/* LDC1 */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read64);											// callh   read64
			UML_DMOV(block, FPR64(RTREG), IREG(0));									// dmov    <cpr1_rt>,i0
			return TRUE;

		case 0x32:	/* LWC2 */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read32);											// callh   read32
			UML_DZEXT4(block, CPR264(RTREG), IREG(0));								// dzext4  <cpr2_rt>,i0
			return TRUE;

		case 0x36:	/* LDC2 */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, mips3.read64);											// callh   read64
			UML_DMOV(block, CPR264(RTREG), IREG(0));								// dmov    <cpr2_rt>,i0
			return TRUE;

		case 0x1a:	/* LDL */
		case 0x1b:	/* LDR */
			return generate_load_double_partial(block, compiler, desc, opswitch & 1);

		case 0x22:	/* LWL */
		case 0x26:	/* LWR */
			return generate_load_word_partial(block, compiler, desc, opswitch & 4);


		/* ----- memory store operations ----- */

		case 0x28:	/* SB */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, IREG(1), R32(RTREG));									// mov     i1,<rtreg>
			UML_MOV(block, MEM(&mips3.tlbepc), IMM(0));								// mov     [tlbepc],0
			UML_CALLH(block, mips3.write8);											// callh   write8
			return TRUE;
		
		case 0x29:	/* SH */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, IREG(1), R32(RTREG));									// mov     i1,<rtreg>
			UML_MOV(block, MEM(&mips3.tlbepc), IMM(0));								// mov     [tlbepc],0
			UML_CALLH(block, mips3.write16);										// callh   write16
			return TRUE;
		
		case 0x2b:	/* SW */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, IREG(1), R32(RTREG));									// mov     i1,<rtreg>
			UML_MOV(block, MEM(&mips3.tlbepc), IMM(0));								// mov     [tlbepc],desc->pc
			UML_CALLH(block, mips3.write32);										// callh   write32
			return TRUE;
		
		case 0x3f:	/* SD */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_DMOV(block, IREG(1), R64(RTREG));									// dmov    i1,<rtreg>
			UML_CALLH(block, mips3.write64);										// callh   write64
			return TRUE;
		
		case 0x39:	/* SWC1 */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, IREG(1), FPR32(RTREG));									// mov     i1,<cpr1_rt>
			UML_CALLH(block, mips3.write32);										// callh   write32
			return TRUE;
		
		case 0x3d:	/* SDC1 */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_DMOV(block, IREG(1), FPR64(RTREG));									// dmov    i1,<cpr1_rt>
			UML_CALLH(block, mips3.write64);										// callh   write64
			return TRUE;

		case 0x3a:	/* SWC2 */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, IREG(1), CPR232(RTREG));									// mov     i1,<cpr2_rt>
			UML_CALLH(block, mips3.write32);										// callh   write32
			return TRUE;

		case 0x3e:	/* SDC2 */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_DMOV(block, IREG(1), CPR264(RTREG));								// dmov    i1,<cpr2_rt>
			UML_CALLH(block, mips3.write64);										// callh   write64
			return TRUE;
		
		case 0x2a:	/* SWL */
		case 0x2e:	/* SWR */
			return generate_store_word_partial(block, compiler, desc, opswitch & 4);

		case 0x2c:	/* SDL */
		case 0x2d:	/* SDR */
			return generate_store_double_partial(block, compiler, desc, opswitch & 1);


		/* ----- effective no-ops ----- */

		case 0x2f:	/* CACHE */
		case 0x33:	/* PREF */
			return TRUE;


		/* ----- coprocessor instructions ----- */

		case 0x10:	/* COP0 */
			return generate_cop0(block, compiler, desc);

		case 0x11:	/* COP1 */
			return generate_cop1(block, compiler, desc);

		case 0x13:	/* COP1X - R5000 */
			return generate_cop1x(block, compiler, desc);

		case 0x12:	/* COP2 */
			UML_EXH(block, mips3.exception[EXCEPTION_INVALIDOP], IMM(0));				// exh     invalidop,0
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
    generate_special - compile opcodes in the
    'SPECIAL' group
-------------------------------------------------*/

static int generate_special(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT8 opswitch = op & 63;

	switch (opswitch)
	{
		/* ----- shift instructions ----- */

		case 0x00:	/* SLL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, IREG(0), R32(RTREG), IMM(SHIFT));					// shl     i0,<rtreg>,<shift>
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			}
			return TRUE;
		
		case 0x02:	/* SRL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, IREG(0), R32(RTREG), IMM(SHIFT));					// shr     i0,<rtreg>,<shift>
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			}
			return TRUE;
		
		case 0x03:	/* SRA - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, IREG(0), R32(RTREG), IMM(SHIFT));					// sar     i0,<rtreg>,<shift>
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			}
			return TRUE;
		
		case 0x04:	/* SLLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, IREG(0), R32(RTREG), R32(RSREG));					// shl     i0,<rtreg>,<rsreg>
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			}
			return TRUE;

		case 0x06:	/* SRLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, IREG(0), R32(RTREG), R32(RSREG));					// shr     i0,<rtreg>,<rsreg>
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			}
			return TRUE;

		case 0x07:	/* SRAV - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, IREG(0), R32(RTREG), R32(RSREG));					// sar     i0,<rtreg>,<rsreg>
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			}
			return TRUE;

		case 0x14:	/* DSLLV - MIPS III */
			if (RDREG != 0)
				UML_DSHL(block, R64(RDREG), R64(RTREG), R64(RSREG));				// dshl    <rdreg>,<rtreg>,<rsreg>
			return TRUE;

		case 0x16:	/* DSRLV - MIPS III */
			if (RDREG != 0)
				UML_DSHR(block, R64(RDREG), R64(RTREG), R64(RSREG));				// dshr    <rdreg>,<rtreg>,<rsreg>
			return TRUE;

		case 0x17:	/* DSRAV - MIPS III */
			if (RDREG != 0)
				UML_DSAR(block, R64(RDREG), R64(RTREG), R64(RSREG));				// dsar    <rdreg>,<rtreg>,<rsreg>
			return TRUE;

		case 0x38:	/* DSLL - MIPS III */
			if (RDREG != 0)
				UML_DSHL(block, R64(RDREG), R64(RTREG), IMM(SHIFT));				// dshl    <rdreg>,<rtreg>,<shift>
			return TRUE;
		
		case 0x3a:	/* DSRL - MIPS III */
			if (RDREG != 0)
				UML_DSHR(block, R64(RDREG), R64(RTREG), IMM(SHIFT));				// dshr    <rdreg>,<rtreg>,<shift>
			return TRUE;
		
		case 0x3b:	/* DSRA - MIPS III */
			if (RDREG != 0)
				UML_DSAR(block, R64(RDREG), R64(RTREG), IMM(SHIFT));				// dsar    <rdreg>,<rtreg>,<shift>
			return TRUE;
		
		case 0x3c:	/* DSLL32 - MIPS III */
			if (RDREG != 0)
				UML_DSHL(block, R64(RDREG), R64(RTREG), IMM(SHIFT + 32));			// dshl    <rdreg>,<rtreg>,<shift>+32
			return TRUE;
		
		case 0x3e:	/* DSRL32 - MIPS III */
			if (RDREG != 0)
				UML_DSHR(block, R64(RDREG), R64(RTREG), IMM(SHIFT + 32));			// dshr    <rdreg>,<rtreg>,<shift>+32
			return TRUE;
		
		case 0x3f:	/* DSRA32 - MIPS III */
			if (RDREG != 0)
				UML_DSAR(block, R64(RDREG), R64(RTREG), IMM(SHIFT + 32));			// dsar    <rdreg>,<rtreg>,<shift>+32
			return TRUE;
		

		/* ----- basic arithmetic ----- */

		case 0x20:	/* ADD - MIPS I */
			UML_ADDf(block, IREG(0), R32(RSREG), R32(RTREG), FLAGS_V);				// add     i0,<rsreg>,<rtreg>,V
			UML_EXHc(block, IF_V, mips3.exception[EXCEPTION_OVERFLOW], IMM(0));		// exh     overflow,0,V
			if (RDREG != 0)
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			return TRUE;

		case 0x21:	/* ADDU - MIPS I */
			if (RDREG != 0)
			{
				UML_ADD(block, IREG(0), R32(RSREG), R32(RTREG));					// add     i0,<rsreg>,<rtreg>
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			}
			return TRUE;

		case 0x2c:	/* DADD - MIPS III */
			UML_DADDf(block, IREG(0), R64(RSREG), R64(RTREG), FLAGS_V);				// dadd    i0,<rsreg>,<rtreg>,V
			UML_EXHc(block, IF_V, mips3.exception[EXCEPTION_OVERFLOW], IMM(0));		// exh     overflow,0,V
			if (RDREG != 0)
				UML_DMOV(block, R64(RDREG), IREG(0));								// dmov    <rdreg>,i0
			return TRUE;

		case 0x2d:	/* DADDU - MIPS III */
			if (RDREG != 0)
				UML_DADD(block, R64(RDREG), R64(RSREG), R64(RTREG));				// dadd    <rdreg>,<rsreg>,<rtreg>
			return TRUE;

		case 0x22:	/* SUB - MIPS I */
			UML_SUBf(block, IREG(0), R32(RSREG), R32(RTREG), FLAGS_V);				// sub     i0,<rsreg>,<rtreg>,V
			UML_EXHc(block, IF_V, mips3.exception[EXCEPTION_OVERFLOW], IMM(0));		// exh     overflow,0,V
			if (RDREG != 0)
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			return TRUE;

		case 0x23:	/* SUBU - MIPS I */
			if (RDREG != 0)
			{
				UML_SUB(block, IREG(0), R32(RSREG), R32(RTREG));					// sub     i0,<rsreg>,<rtreg>
				UML_DSEXT4(block, R64(RDREG), IREG(0));								// dsext4  <rdreg>,i0
			}
			return TRUE;

		case 0x2e:	/* DSUB - MIPS III */
			UML_DSUBf(block, IREG(0), R64(RSREG), R64(RTREG), FLAGS_V);				// dsub    i0,<rsreg>,<rtreg>,V
			UML_EXHc(block, IF_V, mips3.exception[EXCEPTION_OVERFLOW], IMM(0));		// exh     overflow,0,V
			if (RDREG != 0)
				UML_DMOV(block, R64(RDREG), IREG(0));								// dmov    <rdreg>,i0
			return TRUE;

		case 0x2f:	/* DSUBU - MIPS III */
			if (RDREG != 0)
				UML_DSUB(block, R64(RDREG), R64(RSREG), R64(RTREG));				// dsub    <rdreg>,<rsreg>,<rtreg>
			return TRUE;

		case 0x18:	/* MULT - MIPS I */
			UML_MULS(block, IREG(0), IREG(1), R32(RSREG), R32(RTREG));				// muls    i0,i1,<rsreg>,<rtreg>
			UML_DSEXT4(block, LO64, IREG(0));										// dsext4  lo,i0
			UML_DSEXT4(block, HI64, IREG(1));										// dsext4  hi,i1
			return TRUE;

		case 0x19:	/* MULTU - MIPS I */
			UML_MULU(block, IREG(0), IREG(1), R32(RSREG), R32(RTREG));				// mulu    i0,i1,<rsreg>,<rtreg>
			UML_DSEXT4(block, LO64, IREG(0));										// dsext4  lo,i0
			UML_DSEXT4(block, HI64, IREG(1));										// dsext4  hi,i1
			return TRUE;

		case 0x1c:	/* DMULT - MIPS III */
			UML_DMULS(block, LO64, HI64, R64(RSREG), R64(RTREG));					// dmuls   lo,hi,<rsreg>,<rtreg>
			return TRUE;

		case 0x1d:	/* DMULTU - MIPS III */
			UML_DMULU(block, LO64, HI64, R64(RSREG), R64(RTREG));					// dmulu   lo,hi,<rsreg>,<rtreg>
			return TRUE;

		case 0x1a:	/* DIV - MIPS I */
			UML_DIVS(block, IREG(0), IREG(1), R32(RSREG), R32(RTREG));				// divs    i0,i1,<rsreg>,<rtreg>
			UML_DSEXT4(block, LO64, IREG(0));										// dsext4  lo,i0
			UML_DSEXT4(block, HI64, IREG(1));										// dsext4  hi,i1
			return TRUE;

		case 0x1b:	/* DIVU - MIPS I */
			UML_DIVU(block, IREG(0), IREG(1), R32(RSREG), R32(RTREG));				// divu    i0,i1,<rsreg>,<rtreg>
			UML_DSEXT4(block, LO64, IREG(0));										// dsext4  lo,i0
			UML_DSEXT4(block, HI64, IREG(1));										// dsext4  hi,i1
			return TRUE;

		case 0x1e:	/* DDIV - MIPS III */
			UML_DDIVS(block, LO64, HI64, R64(RSREG), R64(RTREG));					// ddivs    lo,hi,<rsreg>,<rtreg>
			return TRUE;

		case 0x1f:	/* DDIVU - MIPS III */
			UML_DDIVU(block, LO64, HI64, R64(RSREG), R64(RTREG));					// ddivu    lo,hi,<rsreg>,<rtreg>
			return TRUE;


		/* ----- basic logical ops ----- */

		case 0x24:	/* AND - MIPS I */
			if (RDREG != 0)
				UML_DAND(block, R64(RDREG), R64(RSREG), R64(RTREG));				// dand     <rdreg>,<rsreg>,<rtreg>
			return TRUE;
		
		case 0x25:	/* OR - MIPS I */
			if (RDREG != 0)
				UML_DOR(block, R64(RDREG), R64(RSREG), R64(RTREG));					// dor      <rdreg>,<rsreg>,<rtreg>
			return TRUE;

		case 0x26:	/* XOR - MIPS I */
			if (RDREG != 0)
				UML_DXOR(block, R64(RDREG), R64(RSREG), R64(RTREG));				// dxor     <rdreg>,<rsreg>,<rtreg>
			return TRUE;

		case 0x27:	/* NOR - MIPS I */
			if (RDREG != 0)
			{
				UML_DOR(block, IREG(0), R64(RSREG), R64(RTREG));					// dor      i0,<rsreg>,<rtreg>
				UML_DXOR(block, R64(RDREG), IREG(0), IMM((UINT64)~0));				// dxor     <rdreg>,i0,~0
			}
			return TRUE;


		/* ----- basic comparisons ----- */

		case 0x2a:	/* SLT - MIPS I */
			if (RDREG != 0)
			{
				UML_DCMP(block, R64(RSREG), R64(RTREG));							// dcmp    <rsreg>,<rtreg>
				UML_DFLAGS(block, R64(RDREG), (UINT64)~0, slt_table);				// dflags  <rdreg>,~0,slt_table
			}
			return TRUE;
		
		case 0x2b:	/* SLTU - MIPS I */
			if (RDREG != 0)
			{
				UML_DCMP(block, R64(RSREG), R64(RTREG));							// dcmp    <rsreg>,<rtreg>
				UML_DFLAGS(block, R64(RDREG), (UINT64)~0, sltu_table);				// dflags  <rdreg>,~0,sltu_table
			}
			return TRUE;


		/* ----- conditional traps ----- */

		case 0x30:	/* TGE - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, IF_GE, mips3.exception[EXCEPTION_TRAP], IMM(0));		// exh     trap,0,GE
			return TRUE;
		
		case 0x31:	/* TGEU - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, IF_AE, mips3.exception[EXCEPTION_TRAP], IMM(0));		// exh     trap,0,AE
			return TRUE;
		
		case 0x32:	/* TLT - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, IF_L, mips3.exception[EXCEPTION_TRAP], IMM(0));			// exh     trap,0,LT
			return TRUE;
		
		case 0x33:	/* TLTU - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, IF_B, mips3.exception[EXCEPTION_TRAP], IMM(0));			// exh     trap,0,B
			return TRUE;
		
		case 0x34:	/* TEQ - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, IF_E, mips3.exception[EXCEPTION_TRAP], IMM(0));			// exh     trap,0,E
			return TRUE;
		
		case 0x36:	/* TNE - MIPS II */
			UML_DCMP(block, R64(RSREG), R64(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_EXHc(block, IF_NE, mips3.exception[EXCEPTION_TRAP], IMM(0));		// exh     trap,0,NE
			return TRUE;


		/* ----- conditional moves ----- */

		case 0x0a:	/* MOVZ - MIPS IV */
			if (RDREG != 0)
			{
				UML_DCMP(block, R64(RTREG), IMM(0));								// dcmp    <rtreg>,0
				UML_DMOVc(block, IF_Z, R64(RDREG), R64(RSREG));						// mov     <rdreg>,<rsreg>,Z
			}
			return TRUE;
		
		case 0x0b:	/* MOVN - MIPS IV */
			if (RDREG != 0)
			{
				UML_DCMP(block, R64(RTREG), IMM(0));								// dcmp    <rtreg>,0
				UML_DMOVc(block, IF_NZ, R64(RDREG), R64(RSREG));					// mov     <rdreg>,<rsreg>,NZ
			}
			return TRUE;
		
		case 0x01:	/* MOVF/MOVT - MIPS IV */
			if (RDREG != 0)
			{
				UML_TEST(block, CCR132(31), IMM(fcc_mask[(op >> 18) & 7]));			// test    ccr31,fcc_mask[x]
				UML_DMOVc(block, ((op >> 16) & 1) ? IF_NZ : IF_Z, 
				                 R64(RDREG), R64(RSREG));							// mov     <rdreg>,<rsreg>,NZ/Z
			}
			return TRUE;


		/* ----- jumps and branches ----- */

		case 0x08:	/* JR - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 0);				// <next instruction + hashjmp>
			return TRUE;

		case 0x09:	/* JALR - MIPS I */
			generate_delay_slot_and_branch(block, compiler, desc, 31);				// <next instruction + hashjmp>
			return TRUE;


		/* ----- system calls ----- */

		case 0x0c:	/* SYSCALL - MIPS I */
			UML_EXH(block, mips3.exception[EXCEPTION_SYSCALL], IMM(0));				// exh     syscall,0
			return TRUE;

		case 0x0d:	/* BREAK - MIPS I */
			UML_EXH(block, mips3.exception[EXCEPTION_BREAK], IMM(0));				// exh     break,0
			return TRUE;


		/* ----- effective no-ops ----- */

		case 0x0f:	/* SYNC - MIPS II */
			return TRUE;


		/* ----- hi/lo register access ----- */

		case 0x10:	/* MFHI - MIPS I */
			if (RDREG != 0)
				UML_DMOV(block, R64(RDREG), HI64);									// dmov    <rdreg>,hi
			return TRUE;

		case 0x11:	/* MTHI - MIPS I */
			if (RDREG != 0)
				UML_DMOV(block, HI64, R64(RDREG));									// dmov    hi,<rdreg>
			return TRUE;

		case 0x12:	/* MFLO - MIPS I */
			if (RDREG != 0)
				UML_DMOV(block, R64(RDREG), LO64);									// dmov    <rdreg>,lo
			return TRUE;

		case 0x13:	/* MTLO - MIPS I */
			if (RDREG != 0)
				UML_DMOV(block, LO64, R64(RDREG));									// dmov    lo,<rdreg>
			return TRUE;
	}
	return FALSE;
}


/*-------------------------------------------------
    generate_regimm - compile opcodes in the
    'REGIMM' group
-------------------------------------------------*/

static int generate_regimm(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT8 opswitch = RTREG;
	drcuml_codelabel skip;

	switch (opswitch)
	{
		case 0x00:	/* BLTZ */
		case 0x02:	/* BLTZL */
		case 0x10:	/* BLTZAL */
		case 0x12:	/* BLTZALL */
			if (RSREG != 0)
			{
				UML_DCMP(block, R64(RSREG), IMM(0));								// dcmp    <rsreg>,0
				UML_JMPc(block, IF_GE, skip = compiler->labelnum++);				// jmp     skip,GE
				generate_delay_slot_and_branch(block, compiler, desc, 
											(opswitch & 0x10) ? 31 : 0);			// <next instruction + hashjmp>
				UML_LABEL(block, skip);											// skip:
			}
			return TRUE;
		
		case 0x01:	/* BGEZ */
		case 0x03:	/* BGEZL */
		case 0x11:	/* BGEZAL */
		case 0x13:	/* BGEZALL */
			if (RSREG != 0)
			{
				UML_DCMP(block, R64(RSREG), IMM(0));								// dcmp    <rsreg>,0
				UML_JMPc(block, IF_L, skip = compiler->labelnum++);					// jmp     skip,L
				generate_delay_slot_and_branch(block, compiler, desc, 
											(opswitch & 0x10) ? 31 : 0);			// <next instruction + hashjmp>
				UML_LABEL(block, skip);											// skip:
			}
			return TRUE;

		case 0x08:	/* TGEI */
			UML_DCMP(block, R64(RSREG), IMM(SIMMVAL));								// dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, IF_GE, mips3.exception[EXCEPTION_TRAP], IMM(0));		// exh     trap,0,GE
			return TRUE;
		
		case 0x09:	/* TGEIU */
			UML_DCMP(block, R64(RSREG), IMM(SIMMVAL));								// dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, IF_AE, mips3.exception[EXCEPTION_TRAP], IMM(0));		// exh     trap,0,AE
			return TRUE;
		
		case 0x0a:	/* TLTI */
			UML_DCMP(block, R64(RSREG), IMM(SIMMVAL));								// dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, IF_L, mips3.exception[EXCEPTION_TRAP], IMM(0));			// exh     trap,0,L
			return TRUE;
		
		case 0x0b:	/* TLTIU */
			UML_DCMP(block, R64(RSREG), IMM(SIMMVAL));								// dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, IF_B, mips3.exception[EXCEPTION_TRAP], IMM(0));			// exh     trap,0,B
			return TRUE;
		
		case 0x0c:	/* TEQI */
			UML_DCMP(block, R64(RSREG), IMM(SIMMVAL));								// dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, IF_E, mips3.exception[EXCEPTION_TRAP], IMM(0));			// exh     trap,0,E
			return TRUE;
		
		case 0x0e:	/* TNEI */
			UML_DCMP(block, R64(RSREG), IMM(SIMMVAL));								// dcmp    <rsreg>,SIMMVAL
			UML_EXHc(block, IF_NE, mips3.exception[EXCEPTION_TRAP], IMM(0));		// exh     trap,0,NE
			return TRUE;
	}
	return FALSE;
}


/*-------------------------------------------------
    generate_idt - compile opcodes in the IDT-
    specific group
-------------------------------------------------*/

static int generate_idt(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
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


/*-------------------------------------------------
    generate_set_cop0_reg - generate code to
    handle special COP0 registers
-------------------------------------------------*/

static int generate_set_cop0_reg(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 reg)
{
	switch (reg)
	{
		case COP0_Cause:
			UML_AND(block, IREG(0), IREG(0), IMM(~0xfc00));							// and     i0,i0,~0xfc00
			UML_AND(block, IREG(1), CPR032(COP0_Cause), IMM(0xfc00));				// and     i1,[Cause],0xfc00
			UML_OR(block, CPR032(COP0_Cause), IREG(0), IREG(1));					// or      [Cause],i0,i1
			UML_AND(block, IREG(0), IREG(0), CPR032(COP0_Status));					// and     i0,i0,[Status]
			UML_TEST(block, IREG(0), IMM(0x300));									// test    i0,0x300
			UML_EXHc(block, IF_NZ, mips3.exception[EXCEPTION_INTERRUPT], IMM(0));	// exh     interrupt,0,NZ
			return TRUE;

		case COP0_Status:
			generate_update_cycles(block, compiler, desc->pc, NULL);				// <subtract cycles>
			UML_MOV(block, IREG(1), CPR032(COP0_Status));							// mov     i1,[Status]
			UML_MOV(block, CPR032(COP0_Status), IREG(0));							// mov     [Status],i0
			UML_XOR(block, IREG(0), IREG(0), IREG(1));								// xor     i0,i0,i1
			UML_TEST(block, IREG(0), IMM(0x8000));									// test    i0,0x8000
			UML_CALLCc(block, IF_NZ, mips3com_update_cycle_counting, mips3.core);	// callc  mips3com_update_cycle_counting,mips.core,NZ
			generate_check_interrupts(block, compiler, desc);						// <check interrupts>
			return TRUE;

		case COP0_Count:
			generate_update_cycles(block, compiler, desc->pc, NULL);				// <subtract cycles>
			UML_MOV(block, CPR032(COP0_Count), IREG(0));							// mov     [Count],i0
			UML_CALLC(block, cfunc_get_cycles, &mips3.numcycles);					// callc   cfunc_get_cycles,&mips3.numcycles
			UML_DZEXT4(block, IREG(0), IREG(0));									// dzext4  i0,i0
			UML_DADD(block, IREG(0), IREG(0), IREG(0));								// dadd    i0,i0,i0
			UML_DSUB(block, MEM(&mips3.core->count_zero_time), 
			                MEM(&mips3.numcycles), IREG(0));						// dsub    [count_zero_time],[mips3.numcycles],i0
			UML_CALLC(block, mips3com_update_cycle_counting, mips3.core);			// callc   mips3com_update_cycle_counting,mips.core
			return TRUE;

		case COP0_Compare:
			generate_update_cycles(block, compiler, desc->pc, NULL);				// <subtract cycles>
			UML_MOV(block, CPR032(COP0_Compare), IREG(0));							// mov     [Compare],i0
			UML_AND(block, CPR032(COP0_Cause), CPR032(COP0_Cause), IMM(~0x8000));	// and    [Cause],[Cause],~0x8000
			UML_CALLC(block, mips3com_update_cycle_counting, mips3.core);			// callc   mips3com_update_cycle_counting,mips.core
			return TRUE;

		case COP0_PRId:
			return TRUE;

		case COP0_Config:
			UML_AND(block, IREG(0), IREG(0), IMM(0x0007));							// and     i0,i0,0x0007
			UML_AND(block, IREG(1), CPR032(COP0_Config), IMM(~0x0007));				// and     i1,[Config],~0x0007
			UML_OR(block, CPR032(COP0_Config), IREG(0), IREG(1));					// or      [Config],i0,i1
			return TRUE;

		default:
			UML_MOV(block, CPR032(reg), IREG(0));									// mov     cpr0[reg],i0
			return TRUE;
	}
}


/*-------------------------------------------------
    generate_get_cop0_reg - generate code to
    read special COP0 registers
-------------------------------------------------*/

static int generate_get_cop0_reg(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 reg)
{
	drcuml_codelabel link1, link2;

	switch (reg)
	{
		case COP0_Count:
			compiler->cycles += MIPS3_COUNT_READ_CYCLES;
			generate_update_cycles(block, compiler, desc->pc, NULL);				// <subtract cycles>
			UML_CALLC(block, cfunc_get_cycles, &mips3.numcycles);					// callc   cfunc_get_cycles,&mips3.numcycles
			UML_DSUB(block, IREG(0), MEM(&mips3.numcycles), 
			                         MEM(&mips3.core->count_zero_time));			// dsub    i0,[numcycles],[count_zero_time]
			UML_DSHR(block, IREG(0), IREG(0), IMM(1));								// dshr    i0,i0,1
			UML_DSEXT4(block, IREG(0), IREG(0));									// dsext4  i0,i0
			return TRUE;

		case COP0_Cause:
			compiler->cycles += MIPS3_CAUSE_READ_CYCLES;
			UML_DSEXT4(block, IREG(0), CPR032(COP0_Cause));							// dsext4  i0,[Cause]
			return TRUE;

		case COP0_Random:
			generate_update_cycles(block, compiler, desc->pc, NULL);				// <subtract cycles>
			UML_CALLC(block, cfunc_get_cycles, &mips3.numcycles);					// callc   cfunc_get_cycles,&mips3.numcycles
			UML_DSUB(block, IREG(0), MEM(&mips3.numcycles), 
			                         MEM(&mips3.core->count_zero_time));			// dsub    i0,[numcycles],[count_zero_time]
			UML_AND(block, IREG(1), CPR032(COP0_Wired), IMM(0x3f));					// and     i1,[Wired],0x3f
			UML_SUBf(block, IREG(2), IMM(48), IREG(1), FLAGS_ALL);					// sub     i2,48,i1,ALL
			UML_JMPc(block, IF_BE, link1 = compiler->labelnum++);					// jmp     link1,BE
			UML_DZEXT4(block, IREG(2), IREG(2));									// dzext4  i2,i2
			UML_DDIVU(block, IREG(0), IREG(2), IREG(0), IREG(2));					// ddivu   i0,i2,i0,i2
			UML_ADD(block, IREG(0), IREG(2), IREG(1));								// add     i0,i2,i1
			UML_DAND(block, IREG(0), IREG(0), IMM(0x3f));							// dand    i0,i0,0x3f
			UML_JMP(block, link2 = compiler->labelnum++);							// jmp     link2
			UML_LABEL(block, link1);											// link1:
			UML_DSEXT4(block, IREG(0), IMM(47));									// dsext4  i0,47
			UML_LABEL(block, link2);											// link2:
			return TRUE;

		default:
			UML_DSEXT4(block, IREG(0), CPR032(reg));								// dsext4  i0,cpr0[reg]
			return TRUE;
	}
}


/*-------------------------------------------------
    generate_cop0 - compile COP0 opcodes
-------------------------------------------------*/

static int generate_cop0(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT8 opswitch = RSREG;

	/* generate an exception if COP0 is disabled or we are not in kernel mode */
	if (mips3.drcoptions & MIPS3DRC_STRICT_COP0)
	{
		drcuml_codelabel okay;

		UML_TEST(block, CPR032(COP0_Status), IMM(SR_KSU_MASK));						// test    [Status],SR_KSU_MASK
		UML_JMPc(block, IF_Z, okay = compiler->labelnum++);							// jmp     okay,Z
		UML_TEST(block, CPR032(COP0_Status), IMM(SR_COP0));							// test    [Status],SR_COP0
		UML_EXHc(block, IF_Z, mips3.exception[EXCEPTION_BADCOP], IMM(0));			// exh     cop,0,Z
		UML_LABEL(block, okay);													// okay:
	}

	switch (opswitch)
	{
		case 0x00:	/* MFCz */
			if (RTREG != 0)
			{
				generate_get_cop0_reg(block, compiler, desc, RDREG);				// <get cop0 reg>
				UML_DSEXT4(block, R64(RTREG), IREG(0));								// dsext4  <rtreg>,i0
			}
			return TRUE;

		case 0x01:	/* DMFCz */
			if (RTREG != 0)
			{
				generate_get_cop0_reg(block, compiler, desc, RDREG);				// <get cop0 reg>
				UML_DMOV(block, R64(RTREG), IREG(0));								// dmov    <rtreg>,i0
			}
			return TRUE;

		case 0x02:	/* CFCz */
			if (RTREG != 0)
				UML_DSEXT4(block, R32(RTREG), CCR032(RDREG));						// dsext4  <rtreg>,ccr0[rdreg]
			return TRUE;

		case 0x04:	/* MTCz */
			UML_DSEXT4(block, IREG(0), R32(RTREG));									// dsext4  i0,<rtreg>
			generate_set_cop0_reg(block, compiler, desc, RDREG);					// <set cop0 reg>
			return TRUE;
		
		case 0x05:	/* DMTCz */
			UML_DMOV(block, IREG(0), R64(RTREG));									// dmov    i0,<rtreg>
			generate_set_cop0_reg(block, compiler, desc, RDREG);					// <set cop0 reg>
			return TRUE;

		case 0x06:	/* CTCz */
			UML_DSEXT4(block, CCR064(RDREG), R32(RTREG));							// dsext4  ccr0[rdreg],<rtreg>
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
					UML_CALLC(block, mips3com_tlbr, mips3.core);					// callc   mips3com_tlbr,mips3.core
					return TRUE;

				case 0x02:	/* TLBWI */
					UML_CALLC(block, mips3com_tlbwi, mips3.core);					// callc   mips3com_tlbwi,mips3.core
					return TRUE;

				case 0x06:	/* TLBWR */
					UML_CALLC(block, mips3com_tlbwr, mips3.core);					// callc   mips3com_tlbwr,mips3.core
					return TRUE;

				case 0x08:	/* TLBP */
					UML_CALLC(block, mips3com_tlbp, mips3.core);					// callc   mips3com_tlbp,mips3.core
					return TRUE;

				case 0x18:	/* ERET */
					UML_AND(block, CPR032(COP0_Status), CPR032(COP0_Status), 
					               IMM(~SR_EXL));									// and     [Status],[Status],~SR_EXL
					generate_update_cycles(block, compiler, 0, 
									LOPTR(&mips3.core->cpr[0][COP0_EPC]));			// <subtract cycles>
					UML_HASHJMP(block, IMM(mips3.mode), CPR032(COP0_EPC), 
									mips3.nocode);									// hashjmp <mode>,[EPC],nocode
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
    generate_cop1 - compile COP1 opcodes
-------------------------------------------------*/

static int generate_cop1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	drcuml_codelabel skip;
	int condition;

	/* generate an exception if COP1 is disabled */
	if (mips3.drcoptions & MIPS3DRC_STRICT_COP1)
	{
		UML_TEST(block, CPR032(COP0_Status), IMM(SR_COP1));							// test    [Status],SR_COP1
		UML_EXHc(block, IF_Z, mips3.exception[EXCEPTION_BADCOP], IMM(0));			// exh     cop,0,Z
	}

	switch (RSREG)
	{
		case 0x00:	/* MFCz */
			if (RTREG != 0)
				UML_DSEXT4(block, R64(RTREG), FPR32(RDREG));						// dsext4  <rtreg>,fpr[rdreg]
			return TRUE;

		case 0x01:	/* DMFCz */
			if (RTREG != 0)
				UML_DMOV(block, R64(RTREG), FPR64(RDREG));							// dmov    <rtreg>,fpr[rdreg]
			return TRUE;

		case 0x02:	/* CFCz */
			if (RTREG != 0)
				UML_DSEXT4(block, R64(RTREG), CCR132(RDREG));						// dsext4  <rtreg>,ccr132[rdreg]
			return TRUE;

		case 0x04:	/* MTCz */
			UML_MOV(block, FPR32(RDREG), R32(RTREG));								// mov     fpr[rdreg],<rtreg>
			return TRUE;

		case 0x05:	/* DMTCz */
			UML_DMOV(block, FPR64(RDREG), R64(RTREG));								// dmov    fpr[rdreg],<rtreg>
			return TRUE;

		case 0x06:	/* CTCz */
			UML_DSEXT4(block, CCR164(RDREG), R32(RTREG));							// dsext4  ccr1[rdreg],<rtreg>
			return TRUE;

		case 0x08:	/* BC */
			switch ((op >> 16) & 3)
			{
				case 0x00:	/* BCzF */
				case 0x02:	/* BCzFL */
					UML_TEST(block, CCR132(31), IMM(FCCMASK(op >> 18)));			// test    ccr1[31],fccmask[which]
					UML_JMPc(block, IF_NZ, skip = compiler->labelnum++);			// jmp     skip,NZ
					generate_delay_slot_and_branch(block, compiler, desc, 0);		// <next instruction + hashjmp>
					UML_LABEL(block, skip);										// skip:
					return TRUE;

				case 0x01:	/* BCzT */
				case 0x03:	/* BCzTL */
					UML_TEST(block, CCR132(31), IMM(FCCMASK(op >> 18)));			// test    ccr1[31],fccmask[which]
					UML_JMPc(block, IF_Z, skip = compiler->labelnum++);				// jmp     skip,Z
					generate_delay_slot_and_branch(block, compiler, desc, 0);		// <next instruction + hashjmp>
					UML_LABEL(block, skip);										// skip:
					return TRUE;
			}
			break;

		default:
			switch (op & 0x3f)
			{
				case 0x00:
					if (IS_SINGLE(op))	/* ADD.S */
						UML_FSADD(block, FPR32(FDREG), FPR32(FSREG), FPR32(FTREG));	// fsadd   <fdreg>,<fsreg>,<ftreg>
					else				/* ADD.D */
						UML_FDADD(block, FPR64(FDREG), FPR64(FSREG), FPR64(FTREG));	// fdadd   <fdreg>,<fsreg>,<ftreg>
					return TRUE;

				case 0x01:
					if (IS_SINGLE(op))	/* SUB.S */
						UML_FSSUB(block, FPR32(FDREG), FPR32(FSREG), FPR32(FTREG));	// fssub   <fdreg>,<fsreg>,<ftreg>
					else				/* SUB.D */
						UML_FDSUB(block, FPR64(FDREG), FPR64(FSREG), FPR64(FTREG));	// fdsub   <fdreg>,<fsreg>,<ftreg>
					return TRUE;

				case 0x02:
					if (IS_SINGLE(op))	/* MUL.S */
						UML_FSMUL(block, FPR32(FDREG), FPR32(FSREG), FPR32(FTREG));	// fsmul   <fdreg>,<fsreg>,<ftreg>
					else				/* MUL.D */
						UML_FDMUL(block, FPR64(FDREG), FPR64(FSREG), FPR64(FTREG));	// fdmul   <fdreg>,<fsreg>,<ftreg>
					return TRUE;

				case 0x03:
					if (IS_SINGLE(op))	/* DIV.S */
						UML_FSDIV(block, FPR32(FDREG), FPR32(FSREG), FPR32(FTREG));	// fsdiv   <fdreg>,<fsreg>,<ftreg>
					else				/* DIV.D */
						UML_FDDIV(block, FPR64(FDREG), FPR64(FSREG), FPR64(FTREG));	// fddiv   <fdreg>,<fsreg>,<ftreg>
					return TRUE;

				case 0x04:
					if (IS_SINGLE(op))	/* SQRT.S */
						UML_FSSQRT(block, FPR32(FDREG), FPR32(FSREG));				// fssqrt  <fdreg>,<fsreg>
					else				/* SQRT.D */
						UML_FDSQRT(block, FPR64(FDREG), FPR64(FSREG));				// fdsqrt  <fdreg>,<fsreg>
					return TRUE;

				case 0x05:
					if (IS_SINGLE(op))	/* ABS.S */
						UML_FSABS(block, FPR32(FDREG), FPR32(FSREG));				// fsabs   <fdreg>,<fsreg>
					else				/* ABS.D */
						UML_FDABS(block, FPR64(FDREG), FPR64(FSREG));				// fdabs   <fdreg>,<fsreg>
					return TRUE;

				case 0x06:
					if (IS_SINGLE(op))	/* MOV.S */
						UML_FSMOV(block, FPR32(FDREG), FPR32(FSREG));				// fsmov   <fdreg>,<fsreg>
					else				/* MOV.D */
						UML_FDMOV(block, FPR64(FDREG), FPR64(FSREG));				// fdmov   <fdreg>,<fsreg>
					return TRUE;

				case 0x07:
					if (IS_SINGLE(op))	/* NEG.S */
						UML_FSNEG(block, FPR32(FDREG), FPR32(FSREG));				// fsneg   <fdreg>,<fsreg>
					else				/* NEG.D */
						UML_FDNEG(block, FPR64(FDREG), FPR64(FSREG));				// fdneg   <fdreg>,<fsreg>
					return TRUE;

				case 0x08:
					if (IS_SINGLE(op))	/* ROUND.L.S */
						UML_FSTOI8R(block, FPR64(FDREG), FPR32(FSREG));				// fstoi8r <fdreg>,<fsreg>
					else				/* ROUND.L.D */
						UML_FDTOI8R(block, FPR64(FDREG), FPR64(FSREG));				// fdtoi8r <fdreg>,<fsreg>
					return TRUE;

				case 0x09:
					if (IS_SINGLE(op))	/* TRUNC.L.S */
						UML_FSTOI8T(block, FPR64(FDREG), FPR32(FSREG));				// fstoi8t <fdreg>,<fsreg>
					else				/* TRUNC.L.D */
						UML_FDTOI8T(block, FPR64(FDREG), FPR64(FSREG));				// fdtoi8t <fdreg>,<fsreg>
					return TRUE;

				case 0x0a:
					if (IS_SINGLE(op))	/* CEIL.L.S */
						UML_FSTOI8C(block, FPR64(FDREG), FPR32(FSREG));				// fstoi8c <fdreg>,<fsreg>
					else				/* CEIL.L.D */
						UML_FDTOI8C(block, FPR64(FDREG), FPR64(FSREG));				// fdtoi8c <fdreg>,<fsreg>
					return TRUE;

				case 0x0b:
					if (IS_SINGLE(op))	/* FLOOR.L.S */
						UML_FSTOI8F(block, FPR64(FDREG), FPR32(FSREG));				// fstoi8f <fdreg>,<fsreg>
					else				/* FLOOR.L.D */
						UML_FDTOI8F(block, FPR64(FDREG), FPR64(FSREG));				// fdtoi8f <fdreg>,<fsreg>
					return TRUE;

				case 0x0c:
					if (IS_SINGLE(op))	/* ROUND.W.S */
						UML_FSTOI4R(block, FPR32(FDREG), FPR32(FSREG));				// fstoi4r <fdreg>,<fsreg>
					else				/* ROUND.W.D */
						UML_FDTOI4R(block, FPR32(FDREG), FPR64(FSREG));				// fdtoi4r <fdreg>,<fsreg>
					return TRUE;

				case 0x0d:
					if (IS_SINGLE(op))	/* TRUNC.W.S */
						UML_FSTOI4T(block, FPR32(FDREG), FPR32(FSREG));				// fstoi4t <fdreg>,<fsreg>
					else				/* TRUNC.W.D */
						UML_FDTOI4T(block, FPR32(FDREG), FPR64(FSREG));				// fdtoi4t <fdreg>,<fsreg>
					return TRUE;

				case 0x0e:
					if (IS_SINGLE(op))	/* CEIL.W.S */
						UML_FSTOI4C(block, FPR32(FDREG), FPR32(FSREG));				// fstoi4c <fdreg>,<fsreg>
					else				/* CEIL.W.D */
						UML_FDTOI4C(block, FPR32(FDREG), FPR64(FSREG));				// fdtoi4c <fdreg>,<fsreg>
					return TRUE;

				case 0x0f:
					if (IS_SINGLE(op))	/* FLOOR.W.S */
						UML_FSTOI4F(block, FPR32(FDREG), FPR32(FSREG));				// fstoi4f <fdreg>,<fsreg>
					else				/* FLOOR.W.D */
						UML_FDTOI4F(block, FPR32(FDREG), FPR64(FSREG));				// fdtoi4f <fdreg>,<fsreg>
					return TRUE;

				case 0x11:	/* R5000 */
					condition = ((op >> 16) & 1) ? IF_NZ : IF_Z;
					UML_TEST(block, CCR132(31), IMM(FCCMASK(op >> 18)));			// test    ccr31,fccmask[op]
					if (IS_SINGLE(op))	/* MOVT/F.S */
						UML_FSMOVc(block, condition, FPR32(FDREG), FPR32(FSREG));	// fsmov   <fdreg>,<fsreg>,condition
					else				/* MOVT/F.D */
						UML_FDMOVc(block, condition, FPR64(FDREG), FPR64(FSREG));	// fdmov   <fdreg>,<fsreg>,condition
					return TRUE;

				case 0x12:	/* R5000 */
					UML_DCMP(block, R64(RTREG), IMM(0));							// dcmp    <rtreg>,0
					if (IS_SINGLE(op))	/* MOVZ.S */
						UML_FSMOVc(block, IF_Z, FPR32(FDREG), FPR32(FSREG));		// fsmov   <fdreg>,<fsreg>,Z
					else				/* MOVZ.D */
						UML_FDMOVc(block, IF_Z, FPR64(FDREG), FPR64(FSREG));		// fdmov   <fdreg>,<fsreg>,Z
					return TRUE;

				case 0x13:	/* R5000 */
					UML_DCMP(block, R64(RTREG), IMM(0));							// dcmp    <rtreg>,0
					if (IS_SINGLE(op))	/* MOVZ.S */
						UML_FSMOVc(block, IF_NZ, FPR32(FDREG), FPR32(FSREG));		// fsmov   <fdreg>,<fsreg>,NZ
					else				/* MOVZ.D */
						UML_FDMOVc(block, IF_NZ, FPR64(FDREG), FPR64(FSREG));		// fdmov   <fdreg>,<fsreg>,NZ
					return TRUE;

				case 0x15:	/* R5000 */
					if (IS_SINGLE(op))	/* RECIP.S */
						UML_FSRECIP(block, FPR32(FDREG), FPR32(FSREG));				// fsrecip <fdreg>,<fsreg>
					else				/* RECIP.D */
						UML_FDRECIP(block, FPR64(FDREG), FPR64(FSREG));				// fdrecip <fdreg>,<fsreg>
					return TRUE;

				case 0x16:	/* R5000 */
					if (IS_SINGLE(op))	/* RSQRT.S */
						UML_FSRSQRT(block, FPR32(FDREG), FPR32(FSREG));				// fsrsqrt <fdreg>,<fsreg>
					else				/* RSQRT.D */
						UML_FDRSQRT(block, FPR64(FDREG), FPR64(FSREG));				// fdrsqrt <fdreg>,<fsreg>
					return TRUE;

				case 0x20:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.S.W */
							UML_FSFRI4(block, FPR32(FDREG), FPR32(FSREG));			// fsfri4  <fdreg>,<fsreg>
						else				/* CVT.S.L */
							UML_FSFRI8(block, FPR32(FDREG), FPR64(FSREG));			// fsfri8  <fdreg>,<fsreg>
					}
					else					/* CVT.S.D */
						UML_FSFRFD(block, FPR32(FDREG), FPR64(FSREG));				// fsfrfd  <fdreg>,<fsreg>
					return TRUE;

				case 0x21:
					if (IS_INTEGRAL(op))
					{
						if (IS_SINGLE(op))	/* CVT.D.W */
							UML_FDFRI4(block, FPR64(FDREG), FPR32(FSREG));			// fdfri4  <fdreg>,<fsreg>
						else				/* CVT.D.L */
							UML_FDFRI8(block, FPR64(FDREG), FPR64(FSREG));			// fdfri8  <fdreg>,<fsreg>
					}
					else					/* CVT.D.S */
						UML_FDFRFS(block, FPR64(FDREG), FPR32(FSREG));				// fdfrfs  <fdreg>,<fsreg>
					return TRUE;

				case 0x24:
					if (IS_SINGLE(op))	/* CVT.W.S */
						UML_FSTOI4(block, FPR32(FDREG), FPR32(FSREG));				// fstoi4  <fdreg>,<fsreg>
					else				/* CVT.W.D */
						UML_FDTOI4(block, FPR32(FDREG), FPR64(FSREG));				// fdtoi4  <fdreg>,<fsreg>
					return TRUE;

				case 0x25:
					if (IS_SINGLE(op))	/* CVT.L.S */
						UML_FSTOI8(block, FPR64(FDREG), FPR32(FSREG));				// fstoi8  <fdreg>,<fsreg>
					else				/* CVT.L.D */
						UML_FDTOI8(block, FPR64(FDREG), FPR64(FSREG));				// fdtoi8  <fdreg>,<fsreg>
					return TRUE;

				case 0x30:
				case 0x38:				/* C.F.S/D */
					UML_AND(block, CCR132(31), CCR132(31), IMM(~FCCMASK(op >> 8)));	// and     ccr31,ccr31,~fccmask[op]
					return TRUE;

				case 0x31:
				case 0x39:				/* C.F.S/D */
					UML_AND(block, CCR132(31), CCR132(31), IMM(~FCCMASK(op >> 8)));	// and     ccr31,ccr31,~fccmask[op]
					return TRUE;

				case 0x32:
				case 0x3a:
					if (IS_SINGLE(op))	/* C.EQ.S */
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));				// fscmp   <fsreg>,<ftreg>
					else				/* C.EQ.D */
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));				// fdcmp   <fsreg>,<ftreg>
					UML_FLAGS(block, CCR132(31), FCCMASK(op >> 8), c_eq_table);		// flags   ccr31,fccmask[op],c_eq_table
					return TRUE;

				case 0x33:
				case 0x3b:
					if (IS_SINGLE(op))	/* C.UEQ.S */
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));				// fscmp   <fsreg>,<ftreg>
					else				/* C.UEQ.D */
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));				// fdcmp   <fsreg>,<ftreg>
					UML_FLAGS(block, CCR132(31), FCCMASK(op >> 8), c_eq_table);		// flags   ccr31,fccmask[op],c_eq_table
					return TRUE;

				case 0x34:
				case 0x3c:
					if (IS_SINGLE(op))	/* C.OLT.S */
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));				// fscmp   <fsreg>,<ftreg>
					else				/* C.OLT.D */
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));				// fdcmp   <fsreg>,<ftreg>
					UML_FLAGS(block, CCR132(31), FCCMASK(op >> 8), c_lt_table);		// flags   ccr31,fccmask[op],c_lt_table
					return TRUE;

				case 0x35:
				case 0x3d:
					if (IS_SINGLE(op))	/* C.ULT.S */
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));				// fscmp   <fsreg>,<ftreg>
					else				/* C.ULT.D */
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));				// fdcmp   <fsreg>,<ftreg>
					UML_FLAGS(block, CCR132(31), FCCMASK(op >> 8), c_lt_table);		// flags   ccr31,fccmask[op],c_lt_table
					return TRUE;

				case 0x36:
				case 0x3e:
					if (IS_SINGLE(op))	/* C.OLE.S */
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));				// fscmp   <fsreg>,<ftreg>
					else				/* C.OLE.D */
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));				// fdcmp   <fsreg>,<ftreg>
					UML_FLAGS(block, CCR132(31), FCCMASK(op >> 8), c_le_table);		// flags   ccr31,fccmask[op],c_le_table
					return TRUE;

				case 0x37:
				case 0x3f:
					if (IS_SINGLE(op))	/* C.ULE.S */
						UML_FSCMP(block, FPR32(FSREG), FPR32(FTREG));				// fscmp   <fsreg>,<ftreg>
					else				/* C.ULE.D */
						UML_FDCMP(block, FPR64(FSREG), FPR64(FTREG));				// fdcmp   <fsreg>,<ftreg>
					UML_FLAGS(block, CCR132(31), FCCMASK(op >> 8), c_le_table);		// flags   ccr31,fccmask[op],c_le_table
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
    generate_cop1x - compile COP1X opcodes
-------------------------------------------------*/

static int generate_cop1x(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;

	if (mips3.drcoptions & MIPS3DRC_STRICT_COP1)
	{
		UML_TEST(block, CPR032(COP0_Status), IMM(SR_COP1));							// test    [Status],SR_COP1
		UML_EXHc(block, IF_Z, mips3.exception[EXCEPTION_BADCOP], IMM(0));			// exh     cop,0,Z
	}

	switch (op & 0x3f)
	{
		case 0x00:		/* LWXC1 */
			UML_ADD(block, IREG(0), R32(RSREG), R32(RTREG));						// add     i0,<rsreg>,<rtreg>
			UML_CALLH(block, mips3.read32);											// callh   read32
			UML_MOV(block, FPR32(RDREG), IREG(0));									// mov     <cpr1_rd>,i0
			return TRUE;

		case 0x01:		/* LDXC1 */
			UML_ADD(block, IREG(0), R32(RSREG), R32(RTREG));						// add     i0,<rsreg>,<rtreg>
			UML_CALLH(block, mips3.read64);											// callh   read64
			UML_DMOV(block, FPR64(RDREG), IREG(0));									// dmov    <cpr1_rd>,i0
			return TRUE;

		case 0x08:		/* SWXC1 */
			UML_ADD(block, IREG(0), R32(RSREG), R32(RTREG));						// add     i0,<rsreg>,<rtreg>
			UML_MOV(block, IREG(1), FPR32(RTREG));									// mov     i1,<cpr1_rt>
			UML_CALLH(block, mips3.write32);										// callh   write32
			return TRUE;

		case 0x09:		/* SDXC1 */
			UML_ADD(block, IREG(0), R32(RSREG), R32(RTREG));						// add     i0,<rsreg>,<rtreg>
			UML_DMOV(block, IREG(1), FPR64(RTREG));									// dmov    i1,<cpr1_rt>
			UML_CALLH(block, mips3.write64);										// callh   write64
			return TRUE;

		case 0x0f:		/* PREFX */
			return TRUE;

		case 0x20:		/* MADD.S */
			UML_FSMUL(block, FREG(0), FPR32(FSREG), FPR32(FTREG));					// fsmul   f0,<fsreg>,<ftreg>
			UML_FSADD(block, FPR32(FDREG), FREG(0), FPR32(FRREG));					// fsadd   <fdreg>,f0,<frreg>
			return TRUE;

		case 0x21:		/* MADD.D */
			UML_FDMUL(block, FREG(0), FPR64(FSREG), FPR64(FTREG));					// fdmul   f0,<fsreg>,<ftreg>
			UML_FDADD(block, FPR64(FDREG), FREG(0), FPR64(FRREG));					// fdadd   <fdreg>,f0,<frreg>
			return TRUE;

		case 0x28:		/* MSUB.S */
			UML_FSMUL(block, FREG(0), FPR32(FSREG), FPR32(FTREG));					// fsmul   f0,<fsreg>,<ftreg>
			UML_FSSUB(block, FPR32(FDREG), FREG(0), FPR32(FRREG));					// fssub   <fdreg>,f0,<frreg>
			return TRUE;

		case 0x29:		/* MSUB.D */
			UML_FDMUL(block, FREG(0), FPR64(FSREG), FPR64(FTREG));					// fdmul   f0,<fsreg>,<ftreg>
			UML_FDSUB(block, FPR64(FDREG), FREG(0), FPR64(FRREG));					// fdsub   <fdreg>,f0,<frreg>
			return TRUE;

		case 0x30:		/* NMADD.S */
			UML_FSMUL(block, FREG(0), FPR32(FSREG), FPR32(FTREG));					// fsmul   f0,<fsreg>,<ftreg>
			UML_FSADD(block, FREG(0), FREG(0), FPR32(FRREG));						// fsadd   f0,f0,<frreg>
			UML_FSNEG(block, FPR32(FDREG), FREG(0));								// fsneg   <fdreg>,f0
			return TRUE;

		case 0x31:		/* NMADD.D */
			UML_FDMUL(block, FREG(0), FPR64(FSREG), FPR64(FTREG));					// fdmul   f0,<fsreg>,<ftreg>
			UML_FDADD(block, FREG(0), FREG(0), FPR64(FRREG));						// fdadd   f0,f0,<frreg>
			UML_FDNEG(block, FPR64(FDREG), FREG(0));								// fdneg   <fdreg>,f0
			return TRUE;

		case 0x38:		/* NMSUB.S */
			UML_FSMUL(block, FREG(0), FPR32(FSREG), FPR32(FTREG));					// fsmul   f0,<fsreg>,<ftreg>
			UML_FSSUB(block, FPR32(FDREG), FPR32(FRREG), FREG(0));					// fssub   <fdreg>,<frreg>,f0
			return TRUE;

		case 0x39:		/* NMSUB.D */
			UML_FDMUL(block, FREG(0), FPR64(FSREG), FPR64(FTREG));					// fdmul   f0,<fsreg>,<ftreg>
			UML_FDSUB(block, FPR64(FDREG), FPR64(FRREG), FREG(0));					// fdsub   <fdreg>,<frreg>,f0
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
    generate_load_word_partial - compile a lwl/lwr
    instruction

    lwl   rT,SIMM(rS)
    lwr   rT,SIMM(rS)
-------------------------------------------------*/

static int generate_load_word_partial(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int is_right)
{
	UINT32 op = *desc->opptr.l;

	/* compute the address and mask values */
	UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));								// add     i0,<rsreg>,SIMMVAL
	UML_SHL(block, IREG(4), IREG(0), IMM(3));										// shl     i4,i0,3
	if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
		UML_XOR(block, IREG(4), IREG(4), IMM(0x18));								// xor     i4,i4,0x18
	UML_AND(block, IREG(0), IREG(0), IMM(~3));										// and     i0,~3
	if (!is_right)
		UML_SHR(block, IREG(2), IMM(~0), IREG(4));									// shr     i2,~0,i4
	else
		UML_SHL(block, IREG(2), IMM(~0), IREG(4));									// shl     i2,~0,i4
	UML_CALLH(block, mips3.read32mask);												// callh   read32mask
	
	/* handle writing back the register */
	if (RTREG != 0)
	{
		/* recompute the mask */
		if (!is_right)
			UML_SHL(block, IREG(2), IMM(~0), IREG(4));								// shl     i2,~0,i4
		else
			UML_SHR(block, IREG(2), IMM(~0), IREG(4));								// shr     i2,~0,i4

		/* apply the shifts and masks */
		UML_XOR(block, IREG(2), IREG(2), IMM(~0));									// xor     i2,i2,~0
		if (!is_right)
			UML_SHL(block, IREG(0), IREG(0), IREG(4));								// shl     i0,i0,i4
		else
			UML_SHR(block, IREG(0), IREG(0), IREG(4));								// shr     i0,i0,i4
		UML_AND(block, IREG(2), IREG(2), R32(RTREG));								// and     i2,i2,<rtreg>
		UML_OR(block, IREG(0), IREG(0), IREG(2));									// or      i0,i0,i2
		UML_DSEXT4(block, R64(RTREG), IREG(0));										// dsext4  <rtreg>,i0
	}
	return TRUE;
}


/*-------------------------------------------------
    generate_load_double_partial - compile a
    ldl/ldr instruction

    ldl   rT,SIMM(rS)
    ldr   rT,SIMM(rS)
-------------------------------------------------*/

static int generate_load_double_partial(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int is_right)
{
	UINT32 op = *desc->opptr.l;
	
	/* compute the address and mask values */
	UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));								// add     i0,<rsreg>,SIMMVAL
	UML_SHL(block, IREG(4), IREG(0), IMM(3));										// shl     i4,i0,3
	if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
		UML_XOR(block, IREG(4), IREG(4), IMM(0x38));								// xor     i4,i4,0x38
	UML_AND(block, IREG(0), IREG(0), IMM(~7));										// and     i0,~7
	if (!is_right)
		UML_DSHR(block, IREG(2), IMM((UINT64)~0), IREG(4));							// dshr    i2,~0,i4
	else
		UML_DSHL(block, IREG(2), IMM((UINT64)~0), IREG(4));							// dshl    i2,~0,i4
	UML_CALLH(block, mips3.read64mask);												// callh   read64mask
	
	/* handle writing back the register */
	if (RTREG != 0)
	{
		/* recompute the mask */
		if (!is_right)
			UML_DSHL(block, IREG(2), IMM((UINT64)~0), IREG(4));						// dshl    i2,~0,i4
		else
			UML_DSHR(block, IREG(2), IMM((UINT64)~0), IREG(4));						// dshr    i2,~0,i4

		/* apply the shifts and masks */
		UML_DXOR(block, IREG(2), IREG(2), IMM((UINT64)~0));							// dxor    i2,i2,~0
		if (!is_right)
			UML_DSHL(block, IREG(0), IREG(0), IREG(4));								// dshl    i0,i0,i4
		else
			UML_DSHR(block, IREG(0), IREG(0), IREG(4));								// dshr    i0,i0,i4
		UML_DAND(block, IREG(2), IREG(2), R64(RTREG));								// dand    i2,i2,<rtreg>
		UML_DOR(block, R64(RTREG), IREG(0), IREG(2));								// dor     <rtreg>,i0,i2
	}
	return TRUE;
}


/*-------------------------------------------------
    generate_store_word_partial - compile a swl/swr
    instruction

    swl   rT,SIMM(rS)
    swr   rT,SIMM(rS)
-------------------------------------------------*/

static int generate_store_word_partial(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int is_right)
{
	UINT32 op = *desc->opptr.l;

	/* compute the address and mask values */
	UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));								// add     i0,<rsreg>,SIMMVAL
	UML_SHL(block, IREG(4), IREG(0), IMM(3));										// shl     i4,i0,3
	UML_MOV(block, IREG(1), R32(RTREG));											// mov     i1,<rtreg>
	if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
		UML_XOR(block, IREG(4), IREG(4), IMM(0x18));								// xor     i4,i4,0x18
	UML_AND(block, IREG(0), IREG(0), IMM(~3));										// and     i0,i0,~3
	if (!is_right)
	{
		UML_SHR(block, IREG(2), IMM(~0), IREG(4));									// shr     i2,~0,i4
		UML_SHR(block, IREG(1), IREG(1), IREG(4));									// shr     i1,i1,i4
	}
	else
	{
		UML_SHL(block, IREG(2), IMM(~0), IREG(4));									// shl     i2,~0,i4
		UML_SHL(block, IREG(1), IREG(1), IREG(4));									// shl     i1,i1,i4
	}
	UML_CALLH(block, mips3.write32mask);											// callh   write32mask

	return TRUE;
}


/*-------------------------------------------------
    generate_store_double_partial - compile a
    sdl/sdr instruction

    sdl   rT,SIMM(rS)
    sdr   rT,SIMM(rS)
-------------------------------------------------*/

static int generate_store_double_partial(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int is_right)
{
	UINT32 op = *desc->opptr.l;

	/* compute the address and mask values */
	UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));								// add     i0,<rsreg>,SIMMVAL
	UML_SHL(block, IREG(4), IREG(0), IMM(3));										// shl     i4,i0,3
	UML_DMOV(block, IREG(1), R64(RTREG));											// dmov    i1,<rtreg>
	if ((!mips3.core->bigendian && !is_right) || (mips3.core->bigendian && is_right))
		UML_XOR(block, IREG(4), IREG(4), IMM(0x38));								// xor     i4,i4,0x38
	UML_AND(block, IREG(0), IREG(0), IMM(~7));										// and     i0,i0,~7
	if (!is_right)
	{
		UML_DSHR(block, IREG(2), IMM((UINT64)~0), IREG(4));							// dshr    i2,~0,i4
		UML_DSHR(block, IREG(1), IREG(1), IREG(4));									// dshr    i1,i1,i4
	}
	else
	{
		UML_DSHL(block, IREG(2), IMM((UINT64)~0), IREG(4));							// dshl    i2,~0,i4
		UML_DSHL(block, IREG(1), IREG(1), IREG(4));									// dshl    i1,i1,i4
	}
	UML_CALLH(block, mips3.write64mask);											// callh   write64mask

	return TRUE;
}



/***************************************************************************
    CODE LOGGING HELPERS
***************************************************************************/

/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of a MIPS instruction
-------------------------------------------------*/

static void log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op)
{
#if (LOG_UML)
	char buffer[100];
	dasmmips3(buffer, pc, op);
	UML_COMMENT(block, "%08X: %s", pc, buffer);										// comment
#endif
}


/*-------------------------------------------------
    log_desc_flags_to_string - generate a string
    representing the instruction description
    flags
-------------------------------------------------*/

static const char *log_desc_flags_to_string(UINT32 flags)
{
	static char tempbuf[30];
	char *dest = tempbuf;

	/* branches */
	if (flags & OPFLAG_IS_UNCONDITIONAL_BRANCH)
		*dest++ = 'U';
	else if (flags & OPFLAG_IS_CONDITIONAL_BRANCH)
		*dest++ = 'C';
	else
		*dest++ = '.';

	/* intrablock branches */
	*dest++ = (flags & OPFLAG_INTRABLOCK_BRANCH) ? 'i' : '.';

	/* branch targets */
	*dest++ = (flags & OPFLAG_IS_BRANCH_TARGET) ? 'B' : '.';

	/* delay slots */
	*dest++ = (flags & OPFLAG_IN_DELAY_SLOT) ? 'D' : '.';

	/* exceptions */
	if (flags & OPFLAG_WILL_CAUSE_EXCEPTION)
		*dest++ = 'E';
	else if (flags & OPFLAG_CAN_CAUSE_EXCEPTION)
		*dest++ = 'e';
	else
		*dest++ = '.';

	/* read/write */
	if (flags & OPFLAG_READS_MEMORY)
		*dest++ = 'R';
	else if (flags & OPFLAG_WRITES_MEMORY)
		*dest++ = 'W';
	else
		*dest++ = '.';

	/* TLB validation */
	*dest++ = (flags & OPFLAG_VALIDATE_TLB) ? 'V' : '.';

	/* TLB modification */
	*dest++ = (flags & OPFLAG_MODIFIES_TRANSLATION) ? 'T' : '.';

	/* redispatch */
	*dest++ = (flags & OPFLAG_REDISPATCH) ? 'R' : '.';
	return tempbuf;
}


/*-------------------------------------------------
    log_register_list - log a list of GPR registers
-------------------------------------------------*/

static void log_register_list(drcuml_state *drcuml, const char *string, UINT64 gprmask, UINT64 fprmask)
{
	int count = 0;
	int regnum;

	/* skip if nothing */
	if ((gprmask & ~1) == 0 && fprmask == 0)
		return;

	drcuml_log_printf(drcuml, "[%s:", string);

	for (regnum = 1; regnum < 32; regnum++)
		if (gprmask & ((UINT64)1 << regnum))
			drcuml_log_printf(drcuml, "%sr%d", (count++ == 0) ? "" : ",", regnum);
	if (gprmask & ((UINT64)1 << REG_LO))
		drcuml_log_printf(drcuml, "%slo", (count++ == 0) ? "" : ",");
	if (gprmask & ((UINT64)1 << REG_HI))
		drcuml_log_printf(drcuml, "%shi", (count++ == 0) ? "" : ",");

	for (regnum = 0; regnum < 32; regnum++)
		if (fprmask & ((UINT64)1 << regnum))
			drcuml_log_printf(drcuml, "%sfpr%d", (count++ == 0) ? "" : ",", regnum);
	if (fprmask & REGFLAG_FCC)
		drcuml_log_printf(drcuml, "%sfcc", (count++ == 0) ? "" : ",");
	drcuml_log_printf(drcuml, "] ");
}


/*-------------------------------------------------
    log_opcode_desc - log a list of descriptions
-------------------------------------------------*/

static void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent)
{
	/* open the file, creating it if necessary */
	if (indent == 0)
		drcuml_log_printf(drcuml, "\nDescriptor list @ %08X\n", desclist->pc);

	/* output each descriptor */
	for ( ; desclist != NULL; desclist = desclist->next)
	{
		char buffer[100];

		/* disassemle the current instruction and output it to the log */
#if LOG_UML
		dasmmips3(buffer, desclist->pc, *desclist->opptr.l);
#else
		strcpy(buffer, "???");
#endif
		drcuml_log_printf(drcuml, "%08X [%08X] t:%08X f:%s: %-30s", desclist->pc, desclist->physpc, desclist->targetpc, log_desc_flags_to_string(desclist->flags), buffer);

		/* output register states */
		log_register_list(drcuml, "use", desclist->gpr.used, desclist->fpr.used);
		log_register_list(drcuml, "mod", desclist->gpr.modified, desclist->fpr.modified);
		log_register_list(drcuml, "lrd", desclist->gpr.liveread, desclist->fpr.liveread);
		log_register_list(drcuml, "lwr", desclist->gpr.livewrite, desclist->fpr.livewrite);
		drcuml_log_printf(drcuml, "\n");

		/* if we have a delay slot, output it recursively */
		if (desclist->delay != NULL)
			log_opcode_desc(drcuml, desclist->delay, indent + 1);

		/* at the end of a sequence add a dividing line */
		if (desclist->flags & OPFLAG_END_SEQUENCE)
			drcuml_log_printf(drcuml, "-----\n");
	}
}



/***************************************************************************
    GENERIC GET/SET INFO
***************************************************************************/

static void mips3_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_MIPS3_DRC_OPTIONS:				mips3.drcoptions = info->i;				break;

		case CPUINFO_INT_MIPS3_FASTRAM_SELECT:			if (info->i >= 0 && info->i < MIPS3_MAX_FASTRAM) mips3.fastram_select = info->i; mips3.cache_dirty = TRUE; break;
		case CPUINFO_INT_MIPS3_FASTRAM_START:			mips3.fastram[mips3.fastram_select].start = info->i; mips3.cache_dirty = TRUE; break;
		case CPUINFO_INT_MIPS3_FASTRAM_END:				mips3.fastram[mips3.fastram_select].end = info->i; mips3.cache_dirty = TRUE; break;
		case CPUINFO_INT_MIPS3_FASTRAM_READONLY:		mips3.fastram[mips3.fastram_select].readonly = info->i; mips3.cache_dirty = TRUE; break;

		case CPUINFO_INT_MIPS3_HOTSPOT_SELECT:			if (info->i >= 0 && info->i < MIPS3_MAX_HOTSPOTS) mips3.hotspot_select = info->i; mips3.cache_dirty = TRUE; break;
		case CPUINFO_INT_MIPS3_HOTSPOT_PC:				mips3.hotspot[mips3.hotspot_select].pc = info->i; mips3.cache_dirty = TRUE; break;
		case CPUINFO_INT_MIPS3_HOTSPOT_OPCODE:			mips3.hotspot[mips3.hotspot_select].opcode = info->i; mips3.cache_dirty = TRUE; break;
		case CPUINFO_INT_MIPS3_HOTSPOT_CYCLES:			mips3.hotspot[mips3.hotspot_select].cycles = info->i; mips3.cache_dirty = TRUE; break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_MIPS3_FASTRAM_BASE:			mips3.fastram[mips3.fastram_select].base = info->p;	break;

		/* --- everything else is handled generically --- */
		default:										mips3com_set_info(mips3.core, state, info);	break;
	}
}


static void mips3_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(mips3);				break;
		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = mips3_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = mips3_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = mips3_set_context;	break;
		case CPUINFO_PTR_INIT:							/* provided per-CPU */					break;
		case CPUINFO_PTR_RESET:							info->reset = mips3_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = mips3_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = mips3_execute;			break;
#ifdef ENABLE_DEBUGGER
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = mips3_dasm;			break;
#endif /* ENABLE_DEBUGGER */
		case CPUINFO_PTR_TRANSLATE:						info->translate = mips3_translate;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;

		/* --- everything else is handled generically --- */
		default:										mips3com_get_info(mips3.core, state, info); break;
	}
}



/***************************************************************************
    R4600 VARIANTS
***************************************************************************/

#if (HAS_R4600)
static void r4600be_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_R4600, TRUE, index, clock, config, irqcallback);
}

static void r4600le_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_R4600, FALSE, index, clock, config, irqcallback);
}

void r4600be_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = r4600be_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R4600 (big)");			break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}

void r4600le_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = r4600le_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R4600 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}
#endif



/***************************************************************************
    R4650 VARIANTS
***************************************************************************/

#if (HAS_R4650)
static void r4650be_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_R4650, TRUE, index, clock, config, irqcallback);
}

static void r4650le_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_R4650, FALSE, index, clock, config, irqcallback);
}

void r4650be_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = r4650be_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "IDT R4650 (big)");		break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}

void r4650le_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = r4650le_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "IDT R4650 (little)");	break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}
#endif



/***************************************************************************
    R4700 VARIANTS
***************************************************************************/

#if (HAS_R4700)
static void r4700be_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_R4700, TRUE, index, clock, config, irqcallback);
}

static void r4700le_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_R4700, FALSE, index, clock, config, irqcallback);
}

void r4700be_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = r4700be_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R4700 (big)");			break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}


void r4700le_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = r4700le_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R4700 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}
#endif



/***************************************************************************
    R5000 VARIANTS
***************************************************************************/

#if (HAS_R5000)
static void r5000be_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_R5000, TRUE, index, clock, config, irqcallback);
}

static void r5000le_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_R5000, FALSE, index, clock, config, irqcallback);
}

void r5000be_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = r5000be_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R5000 (big)");			break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}

void r5000le_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = r5000le_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "R5000 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}
#endif



/***************************************************************************
    QED5271 VARIANTS
***************************************************************************/

#if (HAS_QED5271)
static void qed5271be_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_QED5271, TRUE, index, clock, config, irqcallback);
}

static void qed5271le_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_QED5271, FALSE, index, clock, config, irqcallback);
}

void qed5271be_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = qed5271be_init;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "QED5271 (big)");		break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}

void qed5271le_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = qed5271le_init;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "QED5271 (little)");	break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}
#endif



/***************************************************************************
    RM7000 VARIANTS
***************************************************************************/

#if (HAS_RM7000)
static void rm7000be_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_RM7000, TRUE, index, clock, config, irqcallback);
}

static void rm7000le_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	mips3_init(MIPS3_TYPE_RM7000, FALSE, index, clock, config, irqcallback);
}

void rm7000be_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = rm7000be_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "RM7000 (big)");		break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}

void rm7000le_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = rm7000le_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "RM7000 (little)");		break;

		/* --- everything else is handled generically --- */
		default:										mips3_get_info(state, info);			break;
	}
}
#endif



/***************************************************************************
    DISASSEMBLERS
***************************************************************************/

#if !defined(MAME_DEBUG) && (LOG_UML)
#include "mips3dsm.c"
#endif

