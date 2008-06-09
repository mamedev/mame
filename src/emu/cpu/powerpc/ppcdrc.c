/***************************************************************************

    ppcdrc.c

    Universal machine language-based PowerPC emulator.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Future improvements/changes:

    * Timing for long instructions (stmw/lmw/mul/div/stswi)

    * Eliminate unnecessary flag calculations using information from the
        frontend

***************************************************************************/

#include <stddef.h>
#include "cpuintrf.h"
#include "debugger.h"
#include "ppccom.h"
#include "ppcfe.h"
#include "deprecat.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

extern offs_t ppc_dasm_one(char *buffer, UINT32 pc, UINT32 op);



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define FORCE_C_BACKEND					(0)
#define LOG_UML							(0)
#define LOG_NATIVE						(0)

#define DISABLE_FAST_REGISTERS			(0)
#define SINGLE_INSTRUCTION_MODE			(0)

#define PRINTF_EXCEPTIONS				(0)
#define PRINTF_MMU						(0)

#define PROBE_ADDRESS					~0



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* map variables */
#define MAPVAR_PC						MVAR(0)
#define MAPVAR_CYCLES					MVAR(1)
#define MAPVAR_DSISR					MVAR(2)

/* mode bits */
#define MODE_LITTLE_ENDIAN				0x01
#define MODE_DATA_TRANSLATION			0x02		/* OEA */
#define MODE_PROTECTION					0x02		/* 4XX */
#define MODE_USER						0x04

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
#define EXECUTE_UNMAPPED_CODE			2
#define EXECUTE_RESET_CACHE				3



/***************************************************************************
    MACROS
***************************************************************************/

#define R32(reg)				ppc->impstate->regmap[reg].type, ppc->impstate->regmap[reg].value
#define R32Z(reg)				(((reg) == 0) ? DRCUML_PTYPE_IMMEDIATE : ppc->impstate->regmap[reg].type), (((reg) == 0) ? 0 : ppc->impstate->regmap[reg].value)
#define F64(reg)				ppc->impstate->fdregmap[reg].type, ppc->impstate->fdregmap[reg].value
#define CR32					MEM(&ppc->cr)
#define FPSCR32					MEM(&ppc->fpscr)
#define MSR32					MEM(&ppc->msr)
#define SR32(reg)				MEM(&ppc->sr[reg])
#define SPR32(reg)				MEM(&ppc->spr[reg])

#define CRMASK(reg)				(0xf0000000 >> ((reg) * 4))

/* DSISR values for various addressing types */
#define DSISR_IMM(op)	((0) |										/* bits 15-16: cleared */			\
						 ((((op) >> (31- 5)) & 0x01) << (31-17)) |	/* bit  17:    opcode bit 5 */		\
						 ((((op) >> (31- 4)) & 0x0f) << (31-21)) | 	/* bits 18-21: opcode bits 1-4 */	\
						 ((((op) >> (31-10)) & 0x1f) << (31-26)) |	/* bits 22-26: opcode bits 6-10 */	\
						 (0))										/* bits 27-31: undefined */

#define DSISR_IMMU(op)	((0) |										/* bits 15-16: cleared */			\
						 ((((op) >> (31- 5)) & 0x01) << (31-17)) |	/* bit  17:    opcode bit 5 */		\
						 ((((op) >> (31- 4)) & 0x0f) << (31-21)) | 	/* bits 18-21: opcode bits 1-4 */	\
						 ((((op) >> (31-10)) & 0x1f) << (31-26)) |	/* bits 22-26: opcode bits 6-10 */	\
						 ((((op) >> (31-15)) & 0x1f) << (31-31)))	/* bits 27-31: opcode bits 11-15 */

#define DSISR_IDX(op)	(((((op) >> (31-30)) & 0x03) << (31-16)) |	/* bits 15-16: opcode bits 29-30 */	\
						 ((((op) >> (31-25)) & 0x01) << (31-17)) |	/* bit  17:    opcode bit 25 */		\
						 ((((op) >> (31-24)) & 0x0f) << (31-21)) | 	/* bits 18-21: opcode bits 21-24 */	\
						 ((((op) >> (31-10)) & 0x1f) << (31-26)) |	/* bits 22-26: opcode bits 6-10 */	\
						 (0))										/* bits 27-31: undefined */

#define DSISR_IDXU(op)	(((((op) >> (31-30)) & 0x03) << (31-16)) |	/* bits 15-16: opcode bits 29-30 */	\
						 ((((op) >> (31-25)) & 0x01) << (31-17)) |	/* bit  17:    opcode bit 25 */		\
						 ((((op) >> (31-24)) & 0x0f) << (31-21)) | 	/* bits 18-21: opcode bits 21-24 */	\
						 ((((op) >> (31-10)) & 0x1f) << (31-26)) |	/* bits 22-26: opcode bits 6-10 */	\
						 ((((op) >> (31-15)) & 0x1f) << (31-31)))	/* bits 27-31: opcode bits 11-15 */



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
	UINT8				checkints;					/* need to check interrupts before next instruction */
	UINT8				checksoftints;				/* need to check software interrupts before next instruction */
	drcuml_codelabel	labelnum;					/* index for local labels */
};


/* PowerPC implementation state */
struct _ppcimp_state
{
	/* core state */
	drccache *			cache;						/* pointer to the DRC code cache */
	drcuml_state *		drcuml;						/* DRC UML generator state */
	drcfe_state *		drcfe;						/* pointer to the DRC front-end state */
	UINT32				drcoptions;					/* configurable DRC options */

	/* parameters for subroutines */
	UINT64				numcycles;					/* return value from gettotalcycles */
	UINT32				mode;						/* current global mode */
	UINT32				arg0;						/* print_debug argument 1 */
	UINT32				arg1;						/* print_debug argument 2 */
	UINT32				updateaddr;					/* update address storage */
	UINT32				swcount;					/* counter for sw instructions */
	UINT32				swreg;						/* reg index for sw instructions */
	UINT32				tempaddr;					/* temporary address storage */
	drcuml_ireg			tempdata;					/* temporary data storage */
	double				fp0;						/* floating point 0 */

	/* tables */
	UINT8				fpmode[4];					/* FPU mode table */
	UINT32				sz_cr_table[32];			/* SZ CR table */
	UINT32				cmp_cr_table[32];			/* CMP CR table */
	UINT32				cmpl_cr_table[32];			/* CMPL CR table */
	UINT32				fcmp_cr_table[32];			/* FCMP CR table */

	/* internal stuff */
	UINT8				cache_dirty;				/* true if we need to flush the cache */

	/* register mappings */
	drcuml_parameter	regmap[32];					/* parameter to register mappings for all 32 integer registers */
	drcuml_parameter	fsregmap[32];				/* parameter to register mappings for all 32 floating point registers */
	drcuml_parameter	fdregmap[32];				/* parameter to register mappings for all 32 floating point registers */

	/* subroutines */
	drcuml_codehandle *	entry;						/* entry point */
	drcuml_codehandle *	nocode;						/* nocode exception handler */
	drcuml_codehandle *	out_of_cycles;				/* out of cycles exception handler */
	drcuml_codehandle *	tlb_mismatch;				/* tlb mismatch handler */
	drcuml_codehandle *	swap_tgpr;					/* swap TGPR handler */
	drcuml_codehandle *	lsw[4][32];					/* lsw entries */
	drcuml_codehandle *	stsw[4][32];				/* stsw entries */
	drcuml_codehandle *	read8[4];					/* read byte */
	drcuml_codehandle *	write8[4];					/* write byte */
	drcuml_codehandle *	read16[4];					/* read half */
	drcuml_codehandle *	read16mask[4];				/* read half */
	drcuml_codehandle *	write16[4];					/* write half */
	drcuml_codehandle *	write16mask[4];				/* write half */
	drcuml_codehandle *	read32[4];					/* read word */
	drcuml_codehandle *	read32align[4];				/* read word aligned */
	drcuml_codehandle *	read32mask[4];				/* read word */
	drcuml_codehandle *	write32[4];					/* write word */
	drcuml_codehandle *	write32align[4];			/* write word aligned */
	drcuml_codehandle *	write32mask[4];				/* write word */
	drcuml_codehandle *	read64[4];					/* read double */
	drcuml_codehandle *	read64mask[4];				/* read double */
	drcuml_codehandle *	write64[4];					/* write double */
	drcuml_codehandle *	write64mask[4];				/* write double */
	drcuml_codehandle *	exception[EXCEPTION_COUNT];	/* array of exception handlers */
	drcuml_codehandle *	exception_norecover[EXCEPTION_COUNT];	/* array of exception handlers */

	/* fast RAM */
	UINT32				fastram_select;
	fast_ram_info		fastram[PPC_MAX_FASTRAM];

	/* hotspots */
	UINT32				hotspot_select;
	hotspot_info		hotspot[PPC_MAX_HOTSPOTS];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void code_flush_cache(drcuml_state *drcuml);
static void code_compile_block(drcuml_state *drcuml, UINT8 mode, offs_t pc);

static void cfunc_printf_exception(void *param);
static void cfunc_printf_probe(void *param);

static void static_generate_entry_point(drcuml_state *drcuml);
static void static_generate_nocode_handler(drcuml_state *drcuml);
static void static_generate_out_of_cycles(drcuml_state *drcuml);
static void static_generate_tlb_mismatch(drcuml_state *drcuml);
static void static_generate_exception(drcuml_state *drcuml, UINT8 exception, int recover, const char *name);
static void static_generate_memory_accessor(drcuml_state *drcuml, int mode, int size, int iswrite, int ismasked, const char *name, drcuml_codehandle **handleptr, drcuml_codehandle *masked);
static void static_generate_swap_tgpr(drcuml_state *drcuml);
static void static_generate_lsw_entries(drcuml_state *drcuml, int mode);
static void static_generate_stsw_entries(drcuml_state *drcuml, int mode);

static void generate_update_mode(drcuml_block *block);
static void generate_update_cycles(drcuml_block *block, compiler_state *compiler, drcuml_ptype ptype, UINT64 pvalue, int allow_exception);
static void generate_checksum_block(drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
static void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_instruction_13(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_instruction_1f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_instruction_3b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_instruction_3f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);

static void log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op);
static const char *log_desc_flags_to_string(UINT32 flags);
static void log_register_list(drcuml_state *drcuml, const char *string, UINT64 gprmask, UINT64 fprmask);
static void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent);



/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

static powerpc_state *ppc;


/* lookup table for FP modes */
static const UINT8 fpmode_source[4] =
{
	DRCUML_FMOD_ROUND,
	DRCUML_FMOD_TRUNC,
	DRCUML_FMOD_CEIL,
	DRCUML_FMOD_FLOOR
};

/* flag lookup table for SZ */
static const UINT32 sz_cr_table_source[32] =
{
	/* ..... */	0x40000000,
	/* ....C */	0x42000000,
	/* ...V. */	0x4c000000,
	/* ...VC */	0x4e000000,
	/* ..Z.. */	0x20000000,
	/* ..Z.C */	0x22000000,
	/* ..ZV. */	0x2c000000,
	/* ..ZVC */	0x2e000000,
	/* .S... */	0x80000000,
	/* .S..C */	0x82000000,
	/* .S.V. */	0x8c000000,
	/* .S.VC */	0x8e000000,
	/* .SZ.. */	0x20000000,
	/* .SZ.C */	0x22000000,
	/* .SZV. */	0x2c000000,
	/* .SZVC */	0x2e000000,
	/* U.... */	0x40000000,
	/* U...C */	0x42000000,
	/* U..V. */	0x4c000000,
	/* U..VC */	0x4e000000,
	/* U.Z.. */	0x20000000,
	/* U.Z.C */	0x22000000,
	/* U.ZV. */	0x2c000000,
	/* U.ZVC */	0x2e000000,
	/* US... */	0x80000000,
	/* US..C */	0x82000000,
	/* US.V. */	0x8c000000,
	/* US.VC */	0x8e000000,
	/* USZ.. */	0x20000000,
	/* USZ.C */	0x22000000,
	/* USZV. */	0x2c000000,
	/* USZVC */	0x2e000000
};

/* flag lookup table for CMP */
static const UINT32 cmp_cr_table_source[32] =
{
	/* ..... */	0x44444444,
	/* ....C */	0x44444444,
	/* ...V. */	0x88888888,
	/* ...VC */	0x88888888,
	/* ..Z.. */	0x22222222,
	/* ..Z.C */	0x22222222,
	/* ..ZV. */	0x22222222,
	/* ..ZVC */	0x22222222,
	/* .S... */	0x88888888,
	/* .S..C */	0x88888888,
	/* .S.V. */	0x44444444,
	/* .S.VC */	0x44444444,
	/* .SZ.. */	0x22222222,
	/* .SZ.C */	0x22222222,
	/* .SZV. */	0x22222222,
	/* .SZVC */	0x22222222,
	/* U.... */	0x44444444,
	/* U...C */	0x44444444,
	/* U..V. */	0x88888888,
	/* U..VC */	0x88888888,
	/* U.Z.. */	0x22222222,
	/* U.Z.C */	0x22222222,
	/* U.ZV. */	0x22222222,
	/* U.ZVC */	0x22222222,
	/* US... */	0x88888888,
	/* US..C */	0x88888888,
	/* US.V. */	0x44444444,
	/* US.VC */	0x44444444,
	/* USZ.. */	0x22222222,
	/* USZ.C */	0x22222222,
	/* USZV. */	0x22222222,
	/* USZVC */	0x22222222
};

/* flag lookup table for CMPL */
static const UINT32 cmpl_cr_table_source[32] =
{
	/* ..... */	0x44444444,
	/* ....C */	0x88888888,
	/* ...V. */	0x44444444,
	/* ...VC */	0x88888888,
	/* ..Z.. */	0x22222222,
	/* ..Z.C */	0x22222222,
	/* ..ZV. */	0x22222222,
	/* ..ZVC */	0x22222222,
	/* .S... */	0x44444444,
	/* .S..C */	0x88888888,
	/* .S.V. */	0x44444444,
	/* .S.VC */	0x88888888,
	/* .SZ.. */	0x22222222,
	/* .SZ.C */	0x22222222,
	/* .SZV. */	0x22222222,
	/* .SZVC */	0x22222222,
	/* U.... */	0x44444444,
	/* U...C */	0x88888888,
	/* U..V. */	0x44444444,
	/* U..VC */	0x88888888,
	/* U.Z.. */	0x22222222,
	/* U.Z.C */	0x22222222,
	/* U.ZV. */	0x22222222,
	/* U.ZVC */	0x22222222,
	/* US... */	0x44444444,
	/* US..C */	0x88888888,
	/* US.V. */	0x44444444,
	/* US.VC */	0x88888888,
	/* USZ.. */	0x22222222,
	/* USZ.C */	0x22222222,
	/* USZV. */	0x22222222,
	/* USZVC */	0x22222222
};

/* flag lookup table for FCMP */
static const UINT32 fcmp_cr_table_source[32] =
{
	/* ..... */	0x44444444,
	/* ....C */	0x88888888,
	/* ...V. */	0x44444444,
	/* ...VC */	0x88888888,
	/* ..Z.. */	0x22222222,
	/* ..Z.C */	0xaaaaaaaa,
	/* ..ZV. */	0x22222222,
	/* ..ZVC */	0xaaaaaaaa,
	/* .S... */	0x44444444,
	/* .S..C */	0x88888888,
	/* .S.V. */	0x44444444,
	/* .S.VC */	0x88888888,
	/* .SZ.. */	0x22222222,
	/* .SZ.C */	0xaaaaaaaa,
	/* .SZV. */	0x22222222,
	/* .SZVC */	0xaaaaaaaa,
	/* U.... */	0x55555555,
	/* U...C */	0x99999999,
	/* U..V. */	0x55555555,
	/* U..VC */	0x99999999,
	/* U.Z.. */	0x33333333,
	/* U.Z.C */	0xbbbbbbbb,
	/* U.ZV. */	0x33333333,
	/* U.ZVC */	0xbbbbbbbb,
	/* US... */	0x55555555,
	/* US..C */	0x99999999,
	/* US.V. */	0x55555555,
	/* US.VC */	0x99999999,
	/* USZ.. */	0x33333333,
	/* USZ.C */	0xbbbbbbbb,
	/* USZV. */	0x33333333,
	/* USZVC */	0xbbbbbbbb
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    alloc_handle - allocate a handle if not
    already allocated
-------------------------------------------------*/

INLINE void alloc_handle(drcuml_state *drcuml, drcuml_codehandle **handleptr, const char *name)
{
	if (*handleptr == NULL)
		*handleptr = drcuml_handle_alloc(drcuml, name);
}


/*-------------------------------------------------
    load_fast_iregs - load any fast integer
    registers
-------------------------------------------------*/

INLINE void load_fast_iregs(drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(ppc->impstate->regmap); regnum++)
		if (ppc->impstate->regmap[regnum].type == DRCUML_PTYPE_INT_REGISTER)
			UML_MOV(block, IREG(ppc->impstate->regmap[regnum].value - DRCUML_REG_I0), MEM(&ppc->r[regnum]));
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

INLINE void save_fast_iregs(drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(ppc->impstate->regmap); regnum++)
		if (ppc->impstate->regmap[regnum].type == DRCUML_PTYPE_INT_REGISTER)
			UML_MOV(block, MEM(&ppc->r[regnum]), IREG(ppc->impstate->regmap[regnum].value - DRCUML_REG_I0));
}


/*-------------------------------------------------
    compute_rlw_mask - compute the 32-bit mask
    for an rlw* instruction
-------------------------------------------------*/

INLINE UINT32 compute_rlw_mask(UINT8 mb, UINT8 me)
{
	if (mb <= me)
		return (0xffffffff >> mb) & (0xffffffff << (31 - me));
	else
		return (0xffffffff >> mb) | (0xffffffff << (31 - me));
}


/*-------------------------------------------------
    compute_crf_mask - compute the 32-bit mask
    for a mtcrf/mfcrf instruction
-------------------------------------------------*/

INLINE UINT32 compute_crf_mask(UINT8 crm)
{
	UINT32 mask = 0;
	if (crm & 0x80) mask |= 0xf0000000;
	if (crm & 0x40) mask |= 0x0f000000;
	if (crm & 0x20) mask |= 0x00f00000;
	if (crm & 0x10) mask |= 0x000f0000;
	if (crm & 0x08) mask |= 0x0000f000;
	if (crm & 0x04) mask |= 0x00000f00;
	if (crm & 0x02) mask |= 0x000000f0;
	if (crm & 0x01) mask |= 0x0000000f;
	return mask;
}


/*-------------------------------------------------
    compute_spr - compute the SPR index from the
    SPR field of an opcode
-------------------------------------------------*/

INLINE UINT32 compute_spr(UINT32 spr)
{
	return ((spr >> 5) | (spr << 5)) & 0x3ff;
}



/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    ppcdrc_init - initialize the processor
-------------------------------------------------*/

static void ppcdrc_init(powerpc_flavor flavor, UINT8 cap, int tb_divisor, int clock, const powerpc_config *config, int (*irqcallback)(int))
{
	drcfe_config feconfig =
	{
		COMPILE_BACKWARDS_BYTES,	/* code window start offset = startpc - window_start */
		COMPILE_FORWARDS_BYTES,		/* code window end offset = startpc + window_end */
		COMPILE_MAX_SEQUENCE,		/* maximum instructions to include in a sequence */
		ppcfe_describe				/* callback to describe a single instruction */
	};
	drcbe_info beinfo;
	UINT32 flags = 0;
	size_t extrasize;
	drccache *cache;
	void *extramem;
	int regnum;

	/* determine how much memory beyond the core size we need */
 	extrasize = ppccom_init(NULL, flavor, cap, tb_divisor, clock, config, irqcallback, NULL);

	/* allocate enough space for the cache and the core */
	cache = drccache_alloc(CACHE_SIZE + sizeof(*ppc) + extrasize);
	if (cache == NULL)
		fatalerror("Unable to allocate cache of size %d", (UINT32)(CACHE_SIZE + sizeof(*ppc) + extrasize));

	/* allocate the core from the near cache, and the extra memory from the full cache */
	ppc = drccache_memory_alloc_near(cache, sizeof(*ppc));
	memset(ppc, 0, sizeof(*ppc));
	extramem = drccache_memory_alloc(cache, extrasize);
	memset(extramem, 0, extrasize);

	/* initialize the core */
	ppccom_init(ppc, flavor, cap, tb_divisor, clock, config, irqcallback, extramem);

	/* allocate the implementation-specific state from the full cache */
	ppc->impstate = drccache_memory_alloc_near(cache, sizeof(*ppc->impstate));
	memset(ppc->impstate, 0, sizeof(*ppc->impstate));
	ppc->impstate->cache = cache;

	/* initialize the UML generator */
	if (FORCE_C_BACKEND)
		flags |= DRCUML_OPTION_USE_C;
	if (LOG_UML)
		flags |= DRCUML_OPTION_LOG_UML;
	if (LOG_NATIVE)
		flags |= DRCUML_OPTION_LOG_NATIVE;
	ppc->impstate->drcuml = drcuml_alloc(cache, flags, 8, 32, 2);
	if (ppc->impstate->drcuml == NULL)
		fatalerror("Error initializing the UML");
	
	/* add symbols for our stuff */
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->pc, sizeof(ppc->pc), "pc");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->icount, sizeof(ppc->icount), "icount");
	for (regnum = 0; regnum < 32; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		drcuml_symbol_add(ppc->impstate->drcuml, &ppc->r[regnum], sizeof(ppc->r[regnum]), buf);
		sprintf(buf, "fpr%d", regnum);
		drcuml_symbol_add(ppc->impstate->drcuml, &ppc->f[regnum], sizeof(ppc->r[regnum]), buf);
	}
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->cr, sizeof(ppc->cr), "cr");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->fpscr, sizeof(ppc->fpscr), "fpscr");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->msr, sizeof(ppc->msr), "msr");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->sr, sizeof(ppc->sr), "sr");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->spr[SPR_XER], sizeof(ppc->spr[SPR_XER]), "xer");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->spr[SPR_LR], sizeof(ppc->spr[SPR_LR]), "lr");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->spr[SPR_CTR], sizeof(ppc->spr[SPR_CTR]), "ctr");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->spr, sizeof(ppc->spr), "spr");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->dcr, sizeof(ppc->dcr), "dcr");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->param0, sizeof(ppc->param0), "param0");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->param1, sizeof(ppc->param1), "param1");
	drcuml_symbol_add(ppc->impstate->drcuml, &ppc->irq_pending, sizeof(ppc->irq_pending), "irq_pending");

	/* initialize the front-end helper */
	if (SINGLE_INSTRUCTION_MODE)
		feconfig.max_sequence = 1;
	ppc->impstate->drcfe = drcfe_init(&feconfig, ppc);

	/* initialize the implementation state tables */
	memcpy(ppc->impstate->fpmode, fpmode_source, sizeof(fpmode_source));
	memcpy(ppc->impstate->sz_cr_table, sz_cr_table_source, sizeof(sz_cr_table_source));
	memcpy(ppc->impstate->cmp_cr_table, cmp_cr_table_source, sizeof(cmp_cr_table_source));
	memcpy(ppc->impstate->cmpl_cr_table, cmpl_cr_table_source, sizeof(cmpl_cr_table_source));
	memcpy(ppc->impstate->fcmp_cr_table, fcmp_cr_table_source, sizeof(fcmp_cr_table_source));

	/* compute the register parameters */
	for (regnum = 0; regnum < 32; regnum++)
	{
		ppc->impstate->regmap[regnum].type = DRCUML_PTYPE_MEMORY;
		ppc->impstate->regmap[regnum].value = (FPTR)&ppc->r[regnum];
		ppc->impstate->fsregmap[regnum].type = DRCUML_PTYPE_MEMORY;
		ppc->impstate->fsregmap[regnum].value = (FPTR)((UINT32 *)&ppc->f[regnum] + BYTE_XOR_LE(0));
		ppc->impstate->fdregmap[regnum].type = DRCUML_PTYPE_MEMORY;
		ppc->impstate->fdregmap[regnum].value = (FPTR)&ppc->f[regnum];
	}

	/* if we have registers to spare, assign r0, r1, r2 to leftovers */
	if (!DISABLE_FAST_REGISTERS)
	{
		drcuml_get_backend_info(ppc->impstate->drcuml, &beinfo);
		if (beinfo.direct_iregs > 5)
		{
			ppc->impstate->regmap[0].type = DRCUML_PTYPE_INT_REGISTER;
			ppc->impstate->regmap[0].value = DRCUML_REG_I5;
		}
		if (beinfo.direct_iregs > 6)
		{
			ppc->impstate->regmap[1].type = DRCUML_PTYPE_INT_REGISTER;
			ppc->impstate->regmap[1].value = DRCUML_REG_I6;
		}
		if (beinfo.direct_iregs > 7)
		{
			ppc->impstate->regmap[2].type = DRCUML_PTYPE_INT_REGISTER;
			ppc->impstate->regmap[2].value = DRCUML_REG_I7;
		}
	}

	/* mark the cache dirty so it is updated on next execute */
	ppc->impstate->cache_dirty = TRUE;
}


/*-------------------------------------------------
    ppcdrc_reset - reset the processor
-------------------------------------------------*/

static void ppcdrc_reset(void)
{
	/* reset the common code and mark the cache dirty */
	ppccom_reset(ppc);
	ppc->impstate->mode = 0;
	ppc->impstate->cache_dirty = TRUE;
}


/*-------------------------------------------------
    ppcdrc_execute - execute the CPU for the
    specified number of cycles
-------------------------------------------------*/

static int ppcdrc_execute(int cycles)
{
	drcuml_state *drcuml = ppc->impstate->drcuml;
	int execute_result;

	/* reset the cache if dirty */
	if (ppc->impstate->cache_dirty)
		code_flush_cache(drcuml);
	ppc->impstate->cache_dirty = FALSE;

	/* execute */
	ppc->icount = cycles;
	do
	{
		/* run as much as we can */
		execute_result = drcuml_execute(drcuml, ppc->impstate->entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
			code_compile_block(drcuml, ppc->impstate->mode, ppc->pc);
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", ppc->pc);
		else if (execute_result == EXECUTE_RESET_CACHE)
			code_flush_cache(drcuml);

	} while (execute_result != EXECUTE_OUT_OF_CYCLES);

	/* return the number of cycles executed */
	return cycles - ppc->icount;
}


/*-------------------------------------------------
    ppcdrc_exit - cleanup from execution
-------------------------------------------------*/

static void ppcdrc_exit(void)
{
	/* clean up the DRC */
	drcfe_exit(ppc->impstate->drcfe);
	drcuml_free(ppc->impstate->drcuml);
	drccache_free(ppc->impstate->cache);
}


/*-------------------------------------------------
    ppcdrc_get_context - return a copy of the
    current context
-------------------------------------------------*/

static void ppcdrc_get_context(void *dst)
{
	if (dst != NULL)
		*(powerpc_state **)dst = ppc;
}


/*-------------------------------------------------
    ppcdrc_set_context - copy the current context
    into the global state
-------------------------------------------------*/

static void ppcdrc_set_context(void *src)
{
	if (src != NULL)
		ppc = *(powerpc_state **)src;
}


/*-------------------------------------------------
    ppcdrc_translate - perform virtual-to-physical
    address translation
-------------------------------------------------*/

static int ppcdrc_translate(int space, int intention, offs_t *address)
{
	return ppccom_translate_address(ppc, space, intention, address);
}


/*-------------------------------------------------
    ppcdrc_dasm - disassemble an instruction
-------------------------------------------------*/

#ifdef ENABLE_DEBUGGER
static offs_t ppcdrc_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	return ppccom_dasm(ppc, buffer, pc, oprom, opram);
}
#endif /* ENABLE_DEBUGGER */


/*-------------------------------------------------
    ppcdrc_set_info - set information about a given
    CPU instance
-------------------------------------------------*/

static void ppcdrc_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PPC_DRC_OPTIONS:				ppc->impstate->drcoptions = info->i;				break;

		case CPUINFO_INT_PPC_FASTRAM_SELECT:			if (info->i >= 0 && info->i < PPC_MAX_FASTRAM) ppc->impstate->fastram_select = info->i; ppc->impstate->cache_dirty = TRUE; break;
		case CPUINFO_INT_PPC_FASTRAM_START:				ppc->impstate->fastram[ppc->impstate->fastram_select].start = info->i; ppc->impstate->cache_dirty = TRUE; break;
		case CPUINFO_INT_PPC_FASTRAM_END:				ppc->impstate->fastram[ppc->impstate->fastram_select].end = info->i; ppc->impstate->cache_dirty = TRUE; break;
		case CPUINFO_INT_PPC_FASTRAM_READONLY:			ppc->impstate->fastram[ppc->impstate->fastram_select].readonly = info->i; ppc->impstate->cache_dirty = TRUE; break;

		case CPUINFO_INT_PPC_HOTSPOT_SELECT:			if (info->i >= 0 && info->i < PPC_MAX_HOTSPOTS) ppc->impstate->hotspot_select = info->i; ppc->impstate->cache_dirty = TRUE; break;
		case CPUINFO_INT_PPC_HOTSPOT_PC:				ppc->impstate->hotspot[ppc->impstate->hotspot_select].pc = info->i; ppc->impstate->cache_dirty = TRUE; break;
		case CPUINFO_INT_PPC_HOTSPOT_OPCODE:			ppc->impstate->hotspot[ppc->impstate->hotspot_select].opcode = info->i; ppc->impstate->cache_dirty = TRUE; break;
		case CPUINFO_INT_PPC_HOTSPOT_CYCLES:			ppc->impstate->hotspot[ppc->impstate->hotspot_select].cycles = info->i; ppc->impstate->cache_dirty = TRUE; break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_PPC_FASTRAM_BASE:				ppc->impstate->fastram[ppc->impstate->fastram_select].base = info->p;	break;

		/* --- everything else is handled generically --- */
		default:										ppccom_set_info(ppc, state, info);	break;
	}
}


/*-------------------------------------------------
    ppcdrc_get_info - return information about a given
    CPU instance
-------------------------------------------------*/

static void ppcdrc_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(ppc);					break;
		case CPUINFO_INT_PREVIOUSPC:					/* not implemented */					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = ppcdrc_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = ppcdrc_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = ppcdrc_set_context;	break;
		case CPUINFO_PTR_INIT:							/* provided per-CPU */					break;
		case CPUINFO_PTR_RESET:							info->reset = ppcdrc_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = ppcdrc_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = ppcdrc_execute;			break;
#ifdef ENABLE_DEBUGGER
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = ppcdrc_dasm;		break;
#endif /* ENABLE_DEBUGGER */
		case CPUINFO_PTR_TRANSLATE:						info->translate = ppcdrc_translate;		break;
		case CPUINFO_PTR_CONTEXT:						info->p = ppc;							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;

		/* --- everything else is handled generically --- */
		default:										ppccom_get_info(ppc, state, info); break;
	}
}



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

static void code_flush_cache(drcuml_state *drcuml)
{
	int mode;

	/* empty the transient cache contents */
	drcuml_reset(drcuml);

	/* generate the entry point and out-of-cycles handlers */
	static_generate_entry_point(drcuml);
	static_generate_nocode_handler(drcuml);
	static_generate_out_of_cycles(drcuml);
	static_generate_tlb_mismatch(drcuml);
	if (ppc->cap & PPCCAP_603_MMU)
		static_generate_swap_tgpr(drcuml);

	/* append exception handlers for various types */
	static_generate_exception(drcuml, EXCEPTION_RESET,     TRUE,  "exception_reset");
	static_generate_exception(drcuml, EXCEPTION_MACHCHECK, TRUE,  "exception_machine_check");
	static_generate_exception(drcuml, EXCEPTION_DSI,       TRUE,  "exception_dsi");
	static_generate_exception(drcuml, EXCEPTION_ISI,       TRUE,  "exception_isi");
	static_generate_exception(drcuml, EXCEPTION_EI,        TRUE,  "exception_ei");
	static_generate_exception(drcuml, EXCEPTION_EI,        FALSE, "exception_ei_norecover");
	static_generate_exception(drcuml, EXCEPTION_ALIGN,     TRUE,  "exception_align");
	static_generate_exception(drcuml, EXCEPTION_PROGRAM,   TRUE,  "exception_program");
	static_generate_exception(drcuml, EXCEPTION_NOFPU,     TRUE,  "exception_fpu_unavailable");
	static_generate_exception(drcuml, EXCEPTION_DECREMENT, TRUE,  "exception_decrementer");
	static_generate_exception(drcuml, EXCEPTION_SYSCALL,   TRUE,  "exception_syscall");
	static_generate_exception(drcuml, EXCEPTION_TRACE,     TRUE,  "exception_trace");
	static_generate_exception(drcuml, EXCEPTION_FPASSIST,  TRUE,  "exception_floating_point_assist");
	if (ppc->cap & PPCCAP_603_MMU)
	{
		static_generate_exception(drcuml, EXCEPTION_ITLBMISS,  TRUE,  "exception_itlb_miss");
		static_generate_exception(drcuml, EXCEPTION_DTLBMISSL, TRUE,  "exception_dtlb_miss_load");
		static_generate_exception(drcuml, EXCEPTION_DTLBMISSS, TRUE,  "exception_dtlb_miss_store");
	}

	/* add subroutines for memory accesses */
	for (mode = 0; mode < 4; mode++)
	{
		static_generate_memory_accessor(drcuml, mode, 1, FALSE, FALSE, "read8",       &ppc->impstate->read8[mode],       NULL);
		static_generate_memory_accessor(drcuml, mode, 1, TRUE,  FALSE, "write8",      &ppc->impstate->write8[mode],      NULL);
		static_generate_memory_accessor(drcuml, mode, 2, FALSE, TRUE,  "read16mask",  &ppc->impstate->read16mask[mode],  NULL);
		static_generate_memory_accessor(drcuml, mode, 2, FALSE, FALSE, "read16",      &ppc->impstate->read16[mode],      ppc->impstate->read16mask[mode]);
		static_generate_memory_accessor(drcuml, mode, 2, TRUE,  TRUE,  "write16mask", &ppc->impstate->write16mask[mode], NULL);
		static_generate_memory_accessor(drcuml, mode, 2, TRUE,  FALSE, "write16",     &ppc->impstate->write16[mode],     ppc->impstate->write16mask[mode]);
		static_generate_memory_accessor(drcuml, mode, 4, FALSE, TRUE,  "read32mask",  &ppc->impstate->read32mask[mode],  NULL);
		static_generate_memory_accessor(drcuml, mode, 4, FALSE, FALSE, "read32align", &ppc->impstate->read32align[mode], NULL);
		static_generate_memory_accessor(drcuml, mode, 4, FALSE, FALSE, "read32",      &ppc->impstate->read32[mode],      ppc->impstate->read32mask[mode]);
		static_generate_memory_accessor(drcuml, mode, 4, TRUE,  TRUE,  "write32mask", &ppc->impstate->write32mask[mode], NULL);
		static_generate_memory_accessor(drcuml, mode, 4, TRUE,  FALSE, "write32align",&ppc->impstate->write32align[mode],NULL);
		static_generate_memory_accessor(drcuml, mode, 4, TRUE,  FALSE, "write32",     &ppc->impstate->write32[mode],     ppc->impstate->write32mask[mode]);
		static_generate_memory_accessor(drcuml, mode, 8, FALSE, TRUE,  "read64mask",  &ppc->impstate->read64mask[mode],  NULL);
		static_generate_memory_accessor(drcuml, mode, 8, FALSE, FALSE, "read64",      &ppc->impstate->read64[mode],      ppc->impstate->read64mask[mode]);
		static_generate_memory_accessor(drcuml, mode, 8, TRUE,  TRUE,  "write64mask", &ppc->impstate->write64mask[mode], NULL);
		static_generate_memory_accessor(drcuml, mode, 8, TRUE,  FALSE, "write64",     &ppc->impstate->write64[mode],     ppc->impstate->write64mask[mode]);
		static_generate_lsw_entries(drcuml, mode);
		static_generate_stsw_entries(drcuml, mode);
	}
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
	desclist = drcfe_describe_code(ppc->impstate->drcfe, pc);
	if (LOG_UML || LOG_NATIVE)
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
			UML_COMMENT(block, "-------------------------");								// comment

		/* determine the last instruction in this sequence */
		for (seqlast = seqhead; seqlast != NULL; seqlast = seqlast->next)
			if (seqlast->flags & OPFLAG_END_SEQUENCE)
				break;
		assert(seqlast != NULL);

		/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
		if (override || !drcuml_hash_exists(drcuml, mode, seqhead->pc))
			UML_HASH(block, mode, seqhead->pc);												// hash    mode,pc

		/* if we already have a hash, and this is the first sequence, assume that we */
		/* are recompiling due to being out of sync and allow future overrides */
		else if (seqhead == desclist)
		{
			override = TRUE;
			UML_HASH(block, mode, seqhead->pc);												// hash    mode,pc
		}

		/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
		else
		{
			UML_LABEL(block, seqhead->pc | 0x80000000);										// label   seqhead->pc | 0x80000000
			UML_HASHJMP(block, IMM(ppc->impstate->mode), IMM(seqhead->pc), ppc->impstate->nocode);
																							// hashjmp <mode>,seqhead->pc,nocode
			continue;
		}

		/* validate this code block if we're not pointing into ROM */
		if (memory_get_write_ptr(cpu_getactivecpu(), ADDRESS_SPACE_PROGRAM, seqhead->physpc) != NULL)
			generate_checksum_block(block, &compiler, seqhead, seqlast);					// <checksum>

		/* label this instruction, if it may be jumped to locally */
		if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
			UML_LABEL(block, seqhead->pc | 0x80000000);										// label   seqhead->pc | 0x80000000

		/* iterate over instructions in the sequence and compile them */
		for (curdesc = seqhead; curdesc != seqlast->next; curdesc = curdesc->next)
			generate_sequence_instruction(block, &compiler, curdesc);						// <instruction>

		/* if we need to return to the start, do it */
		if (seqlast->flags & OPFLAG_RETURN_TO_START)
			nextpc = pc;

		/* otherwise we just go to the next instruction */
		else
			nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;

		/* count off cycles and go there */
		generate_update_cycles(block, &compiler, IMM(nextpc), TRUE);						// <subtract cycles>

		/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
		if (seqlast->flags & OPFLAG_CAN_CHANGE_MODES)
			UML_HASHJMP(block, MEM(&ppc->impstate->mode), IMM(nextpc), ppc->impstate->nocode);// hashjmp <mode>,nextpc,nocode
		else if (seqlast->next == NULL || seqlast->next->pc != nextpc)
			UML_HASHJMP(block, IMM(ppc->impstate->mode), IMM(nextpc), ppc->impstate->nocode);// hashjmp <mode>,nextpc,nocode
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
	printf("Exception: type=%2d EPC=%08X MSR=%08X\n", ppc->param0, ppc->spr[SPROEA_SRR0], ppc->spr[SPROEA_SRR1]);
	cfunc_printf_probe((void *)(FPTR)ppc->spr[SPROEA_SRR0]);
}


/*-------------------------------------------------
    cfunc_printf_debug - generic printf for
    debugging
-------------------------------------------------*/

static void cfunc_printf_debug(void *param)
{
	const char *format = param;
	printf(format, ppc->impstate->arg0, ppc->impstate->arg1);
}


/*-------------------------------------------------
    cfunc_printf_probe - print the current CPU
    state and return
-------------------------------------------------*/

static void cfunc_printf_probe(void *param)
{
	UINT32 pc = (UINT32)(FPTR)param;

	printf(" PC=%08X\n", pc);
	printf(" r0=%08X  r1=%08X  r2=%08X  r3=%08X\n",
		ppc->r[0], ppc->r[1], ppc->r[2], ppc->r[3]);
	printf(" r4=%08X  r5=%08X  r6=%08X  r7=%08X\n",
		ppc->r[4], ppc->r[5], ppc->r[6], ppc->r[7]);
	printf(" r8=%08X  r9=%08X r10=%08X r11=%08X\n",
		ppc->r[8], ppc->r[9], ppc->r[10], ppc->r[11]);
	printf("r12=%08X r13=%08X r14=%08X r15=%08X\n",
		ppc->r[12], ppc->r[13], ppc->r[14], ppc->r[15]);
	printf("r16=%08X r17=%08X r18=%08X r19=%08X\n",
		ppc->r[16], ppc->r[17], ppc->r[18], ppc->r[19]);
	printf("r20=%08X r21=%08X r22=%08X r23=%08X\n",
		ppc->r[20], ppc->r[21], ppc->r[22], ppc->r[23]);
	printf("r24=%08X r25=%08X r26=%08X r27=%08X\n",
		ppc->r[24], ppc->r[25], ppc->r[26], ppc->r[27]);
	printf("r28=%08X r29=%08X r30=%08X r31=%08X\n",
		ppc->r[28], ppc->r[29], ppc->r[30], ppc->r[31]);
}


/*-------------------------------------------------
    cfunc_unimplemented - handler for
    unimplemented opcdes
-------------------------------------------------*/

static void cfunc_unimplemented(void *param)
{
	UINT32 opcode = (FPTR)param;
	fatalerror("PC=%08X: Unimplemented op %08X", ppc->pc, opcode);
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
	block = drcuml_block_begin(drcuml, 20, &errorbuf);

	/* forward references */
	alloc_handle(drcuml, &ppc->impstate->nocode, "nocode");
	alloc_handle(drcuml, &ppc->impstate->exception_norecover[EXCEPTION_EI], "exception_ei_norecover");

	alloc_handle(drcuml, &ppc->impstate->entry, "entry");
	UML_HANDLE(block, ppc->impstate->entry);												// handle  entry

	/* reset the FPU mode */
	UML_AND(block, IREG(0), FPSCR32, IMM(3));												// and     i0,fpscr,3
	UML_LOAD(block, IREG(0), &ppc->impstate->fpmode[0], IREG(0), BYTE);						// load    i0,fpmode,i0,byte
	UML_SETFMOD(block, IREG(0));															// setfmod i0

	/* load fast integer registers */
	load_fast_iregs(block);																	// <load fastregs>

	/* check for interrupts */
	UML_TEST(block, MEM(&ppc->irq_pending), IMM(~0));										// test    [irq_pending],0
	UML_JMPc(block, IF_Z, skip);															// jmp     skip,Z
	UML_TEST(block, MSR32, IMM(MSR_EE));													// test    msr,MSR_EE
	UML_JMPc(block, IF_Z, skip);															// jmp     skip,Z
	UML_MOV(block, IREG(0), MEM(&ppc->pc));													// mov     i0,pc
	UML_MOV(block, IREG(1), IMM(0));														// mov     i1,0
	UML_CALLH(block, ppc->impstate->exception_norecover[EXCEPTION_EI]);						// callh   exception_norecover
	UML_LABEL(block, skip);																// skip:

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, MEM(&ppc->impstate->mode), MEM(&ppc->pc), ppc->impstate->nocode);	// hashjmp <mode>,<pc>,nocode

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
	alloc_handle(drcuml, &ppc->impstate->nocode, "nocode");
	UML_HANDLE(block, ppc->impstate->nocode);												// handle  nocode
	UML_GETEXP(block, IREG(0));																// getexp  i0
	UML_MOV(block, MEM(&ppc->pc), IREG(0));													// mov     [pc],i0
	save_fast_iregs(block);																	// <save fastregs>
	UML_EXIT(block, IMM(EXECUTE_MISSING_CODE));												// exit    EXECUTE_MISSING_CODE

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
	alloc_handle(drcuml, &ppc->impstate->out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, ppc->impstate->out_of_cycles);										// handle  out_of_cycles
	UML_GETEXP(block, IREG(0));																// getexp  i0
	UML_MOV(block, MEM(&ppc->pc), IREG(0));													// mov     <pc>,i0
	save_fast_iregs(block);																	// <save fastregs>
	UML_EXIT(block, IMM(EXECUTE_OUT_OF_CYCLES));											// exit    EXECUTE_OUT_OF_CYCLES

	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_tlb_mismatch - generate a
    TLB mismatch handler
-------------------------------------------------*/

static void static_generate_tlb_mismatch(drcuml_state *drcuml)
{
	drcuml_block *block;
	jmp_buf errorbuf;
	int isi, label = 1;

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_tlb_mismatch");

	/* forward references */
	alloc_handle(drcuml, &ppc->impstate->exception[EXCEPTION_ISI], "exception_isi");
	if (ppc->cap & PPCCAP_603_MMU)
		alloc_handle(drcuml, &ppc->impstate->exception[EXCEPTION_ITLBMISS], "exception_itlb_miss");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 20, &errorbuf);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &ppc->impstate->tlb_mismatch, "tlb_mismatch");
	UML_HANDLE(block, ppc->impstate->tlb_mismatch);											// handle  tlb_mismatch
	UML_RECOVER(block, IREG(0), MAPVAR_PC);													// recover i0,PC
	UML_MOV(block, MEM(&ppc->param0), IREG(0));												// mov     [param0],i0
	UML_MOV(block, MEM(&ppc->param1), IMM(TRANSLATE_FETCH));								// mov     [param1],TRANSLATE_FETCH
	UML_CALLC(block, ppccom_tlb_fill, ppc);													// callc   tlbfill,ppc
	UML_CMP(block, MEM(&ppc->param0), IMM(1));												// cmp     [param0],1
	UML_JMPc(block, IF_A, isi = label++);													// jmp     isi,A
	save_fast_iregs(block);																	// <save fastregs>
	UML_EXIT(block, IMM(EXECUTE_MISSING_CODE));												// exit    EXECUTE_MISSING_CODE
	UML_LABEL(block, isi);																// isi:
	if (!(ppc->cap & PPCCAP_603_MMU))
	{
		UML_MOV(block, SPR32(SPROEA_DSISR), MEM(&ppc->param0));								// mov     [dsisr],[param0]
		UML_EXH(block, ppc->impstate->exception[EXCEPTION_ISI], IREG(0));					// exh     isi,i0
	}
	else
	{
		UML_MOV(block, SPR32(SPR603_IMISS), IREG(0));										// mov     [imiss],i0
		UML_MOV(block, SPR32(SPR603_ICMP), MEM(&ppc->mmu603_cmp));							// mov     [icmp],[mmu603_cmp]
		UML_MOV(block, SPR32(SPR603_HASH1), MEM(&ppc->mmu603_hash[0]));						// mov     [hash1],[mmu603_hash][0]
		UML_MOV(block, SPR32(SPR603_HASH2), MEM(&ppc->mmu603_hash[1]));						// mov     [hash2],[mmu603_hash][1]
		UML_EXH(block, ppc->impstate->exception[EXCEPTION_ITLBMISS], IREG(0));				// exh     itlbmiss,i0
	}

	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_exception - generate a static
    exception handler
-------------------------------------------------*/

static void static_generate_exception(drcuml_state *drcuml, UINT8 exception, int recover, const char *name)
{
	drcuml_codehandle **exception_handle = recover ? &ppc->impstate->exception[exception] : &ppc->impstate->exception_norecover[exception];
	UINT32 vector = exception << 8;
	drcuml_codelabel label = 1;
	drcuml_block *block;
	jmp_buf errorbuf;

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_exception");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 1024, &errorbuf);

	/* add a global entry for this */
	alloc_handle(drcuml, exception_handle, name);
	UML_HANDLE(block, *exception_handle);													// handle  name

	/* exception parameter is expected to be the fault address in this case */
	if (exception == EXCEPTION_ISI || exception == EXCEPTION_DSI)
	{
		UML_GETEXP(block, IREG(0));															// getexp  i0
		UML_MOV(block, SPR32(SPROEA_DAR), IREG(0));											// mov     [dar],i0
	}

	/* fetch the PC and uncounted cycles */
	if (recover)
	{
		UML_RECOVER(block, IREG(0), MAPVAR_PC);												// recover i0,PC
		UML_RECOVER(block, IREG(1), MAPVAR_CYCLES);											// recover i1,CYCLES
	}

	/* OEA handling of SRR exceptions */
	if (ppc->cap & PPCCAP_OEA)
	{
		UINT32 msrandmask = MSROEA_POW | MSR_EE | MSR_PR | MSROEA_FP | MSROEA_FE0 | MSROEA_SE | MSROEA_BE | MSROEA_FE1 | MSROEA_IR | MSROEA_DR | MSROEA_RI | MSR_LE;
		UINT32 msrormask = 0;

		/* check registers to see the real source of our exception (EI exceptions only) */
		UML_MOV(block, IREG(3), IMM(vector));												// mov     i3,vector
		if (exception == EXCEPTION_EI)
		{
			drcuml_codelabel not_decrementer;

			UML_TEST(block, MEM(&ppc->irq_pending), IMM(0x01));								// test    [irq_pending],0x01
			UML_JMPc(block, IF_NZ, not_decrementer = label++);								// jmp     not_decrementer,nz
			UML_MOV(block, IREG(3), IMM(EXCEPTION_DECREMENT << 8));							// mov     i3,EXCEPTION_DECREMENT << 8
			UML_AND(block, MEM(&ppc->irq_pending), MEM(&ppc->irq_pending), IMM(~0x02));		// and     [irq_pending],[irq_pending],~0x02
			UML_LABEL(block, not_decrementer);											// not_decrementer:
		}

		/* exception PC goes into SRR0 */
		UML_MOV(block, SPR32(SPROEA_SRR0), IREG(0));										// mov     [srr0],i0

		/* MSR bits go into SRR1, along with some exception-specific data */
		UML_AND(block, SPR32(SPROEA_SRR1), MSR32, IMM(0x87c0ffff));							// and     [srr1],[msr],0x87c0ffff
		if (exception == EXCEPTION_PROGRAM)
		{
			UML_GETEXP(block, IREG(1));														// getexp  i1
			UML_OR(block, SPR32(SPROEA_SRR1), SPR32(SPROEA_SRR1), IREG(1));					// or      [srr1],[srr1],i1
		}
		if (ppc->cap & PPCCAP_603_MMU)
		{
			if (exception == EXCEPTION_ITLBMISS)
				UML_OR(block, SPR32(SPROEA_SRR1), SPR32(SPROEA_SRR1), IMM(0x00040000));		// or      [srr1],0x00040000
			else if (exception == EXCEPTION_DTLBMISSL)
				UML_OR(block, SPR32(SPROEA_SRR1), SPR32(SPROEA_SRR1), IMM(0x00010000));		// or      [srr1],0x00010000
			if (exception == EXCEPTION_ITLBMISS || exception == EXCEPTION_DTLBMISSL || exception == EXCEPTION_DTLBMISSS)
				UML_ROLINS(block, SPR32(SPROEA_SRR1), CR32, IMM(0), IMM(CRMASK(0)));		// rolins  [srr1],[cr],0,crmask(0)
		}

		/* update MSR */
		if (ppc->cap & PPCCAP_603_MMU)
		{
			if (exception == EXCEPTION_ITLBMISS || exception == EXCEPTION_DTLBMISSL || exception == EXCEPTION_DTLBMISSS)
				msrormask |= MSR603_TGPR;
			else
				msrandmask |= MSR603_TGPR;
			UML_MOV(block, IREG(0), MSR32);													// mov     i0,[msr]
		}
		UML_AND(block, IREG(2), MSR32, IMM(~msrandmask));									// and     i2,[msr],~andmask
		UML_OR(block, IREG(2), IREG(2), IMM(msrormask));									// or      i2,i2,ormask
		UML_ROLINS(block, IREG(2), IREG(2), IMM(16), IMM(MSR_LE));							// rolins  i2,u2,16,MSR_LE
		UML_MOV(block, MSR32, IREG(2));														// mov     [msr],i2
		if (ppc->cap & PPCCAP_603_MMU)
		{
			UML_XOR(block, IREG(0), IREG(0), IREG(2));										// xor     i0,i0,i2
			UML_TEST(block, IREG(0), IMM(MSR603_TGPR));										// test    i0,tgpr
			UML_CALLHc(block, IF_NZ, ppc->impstate->swap_tgpr);								// callh   swap_tgpr,nz
		}
		generate_update_mode(block);														// <update mode>

		/* determine our target PC */
		UML_MOV(block, IREG(0), IMM(0x00000000));											// mov     i0,0x00000000
		UML_TEST(block, MSR32, IMM(MSROEA_IP));												// test    [msr],IP
		UML_MOVc(block, IF_NZ, IREG(0), IMM(0xfff00000));									// mov     i0,0xfff00000,nz
		UML_OR(block, IREG(0), IREG(0), IREG(3));											// or      i0,i0,i3
	}

	/* 4XX handling of exceptions */
	if (ppc->cap & PPCCAP_4XX)
	{
		/* check registers to see the real source of our exception (EI exceptions only) */
		UML_MOV(block, IREG(3), IMM(vector));												// mov     i3,vector
		if (exception == EXCEPTION_EI)
		{
			drcuml_codelabel notwdog, common;

			UML_TEST(block, SPR32(SPR4XX_TSR), IMM(PPC4XX_TSR_PIS));						// test    [tsr],PIS
			UML_MOVc(block, IF_NZ, IREG(3), IMM(0x1000));									// mov     i3,0x1000,NZ
			UML_TEST(block, SPR32(SPR4XX_TSR), IMM(PPC4XX_TSR_FIS));						// test    [tsr],FIS
			UML_MOVc(block, IF_NZ, IREG(3), IMM(0x1010));									// mov     i3,0x1010,NZ
			UML_TEST(block, SPR32(DCR4XX_EXISR), SPR32(DCR4XX_EXIER));						// test    [exisr],[exier]
			UML_MOVc(block, IF_NZ, IREG(3), IMM(vector));									// mov     i3,vector,NZ
			UML_TEST(block, SPR32(SPR4XX_TSR), IMM(PPC4XX_TSR_WIS));						// test    [tsr],WIS
			UML_JMPc(block, IF_Z, notwdog = label++);										// jz      notwdog
			UML_MOV(block, IREG(3), IMM(0x1020));											// mov     i3,0x1020

			/* exception PC goes into SRR2, MSR goes to SRR3 */
			UML_MOV(block, SPR32(SPR4XX_SRR2), IREG(0));									// mov     [srr2],i0
			UML_MOV(block, SPR32(SPR4XX_SRR3), MSR32);										// mov     [srr3],[msr]
			UML_AND(block, IREG(2), MSR32, IMM(~(MSR4XX_WE | MSR_PR | MSR4XX_CE | MSR_EE | MSR4XX_DE | MSR4XX_PE)));
			UML_JMP(block, common = label++);												// jmp     common

			/* exception PC goes into SRR0, MSR goes to SRR1 */
			UML_LABEL(block, notwdog);													// notwdog:
			UML_MOV(block, SPR32(SPR4XX_SRR0), IREG(0));									// mov     [srr0],i0
			UML_MOV(block, SPR32(SPR4XX_SRR1), MSR32);										// mov     [srr1],[msr]
			UML_AND(block, IREG(2), MSR32, IMM(~(MSR4XX_WE | MSR_PR | MSR_EE | MSR4XX_PE)));// and     i2,[msr],~(bunch-o-flags)
			UML_LABEL(block, common);													// common:
		}
		else
		{
			/* exception PC goes into SRR0, MSR goes to SRR1 */
			UML_MOV(block, SPR32(SPR4XX_SRR0), IREG(0));									// mov     [srr0],i0
			UML_MOV(block, SPR32(SPR4XX_SRR1), MSR32);										// mov     [srr1],[msr]
			UML_AND(block, IREG(2), MSR32, IMM(~(MSR4XX_WE | MSR_PR | MSR_EE | MSR4XX_PE)));// and     i2,[msr],~(bunch-o-flags)
		}

		/* finish updating MSR */
		UML_ROLINS(block, IREG(2), IREG(2), IMM(16), IMM(MSR_LE));							// rolins  i2,u2,16,MSR_LE
		UML_MOV(block, MSR32, IREG(2));														// mov     [msr],i2
		generate_update_mode(block);														// <update mode>

		/* program exception flags go to ESR */
		if (exception == EXCEPTION_PROGRAM)
		{
			UML_GETEXP(block, IREG(1));														// getexp  i1
			UML_SHL(block, SPR32(SPR4XX_ESR), IREG(1), IMM(8));								// shl     [esr],i1,8
		}

		/* determine our target PC */
		UML_ROLINS(block, IREG(3), SPR32(SPR4XX_EVPR), IMM(0), IMM(0xffff0000));			// rolins  i3,[evpr],0,0xffff0000
		UML_MOV(block, IREG(0), IREG(3));													// mov     i0,i3
	}

	/* optionally print exceptions */
	if ((PRINTF_EXCEPTIONS && exception != EXCEPTION_EI && exception != EXCEPTION_SYSCALL) ||
		(PRINTF_MMU && (exception == EXCEPTION_ISI || exception == EXCEPTION_DSI)))
	{
		UML_MOV(block, MEM(&ppc->param0), IMM(exception));									// mov     [param0],exception
		UML_CALLC(block, cfunc_printf_exception, NULL);										// callc   cfunc_printf_exception,NULL
	}

	/* adjust cycles */
	UML_SUB(block, MEM(&ppc->icount), MEM(&ppc->icount), IREG(1));			 				// sub     icount,icount,cycles
	UML_EXHc(block, IF_S, ppc->impstate->out_of_cycles, IREG(0));							// exh     out_of_cycles,i0
	UML_HASHJMP(block, MEM(&ppc->impstate->mode), IREG(0), ppc->impstate->nocode);			// hashjmp <mode>,i0,nocode

	drcuml_block_end(block);
}


/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

static void static_generate_memory_accessor(drcuml_state *drcuml, int mode, int size, int iswrite, int ismasked, const char *name, drcuml_codehandle **handleptr, drcuml_codehandle *masked)
{
	/* on entry, address is in I0; data for writes is in I1; masks are in I2 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I3 */
	int fastxor = BYTE8_XOR_BE(0) >> (activecpu_databus_width(ADDRESS_SPACE_PROGRAM) < 64);
	drcuml_block *block;
	jmp_buf errorbuf;
	int tlbreturn = 0;
	int unaligned = 0;
	int alignex = 0;
	int tlbmiss = 0;
	int label = 1;
	int ramnum;

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_exception");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 1024, &errorbuf);

	/* add a global entry for this */
	alloc_handle(drcuml, handleptr, name);
	UML_HANDLE(block, *handleptr);															// handle  *handleptr

	/* check for unaligned accesses and break into two */
	if (!ismasked && size != 1)
	{
		/* in little-endian mode, anything misaligned generates an exception */
		if ((mode & MODE_LITTLE_ENDIAN) || masked == NULL || !(ppc->cap & PPCCAP_MISALIGNED))
		{
			UML_TEST(block, IREG(0), IMM(size - 1));										// test    i0,size-1
			UML_JMPc(block, IF_NZ, alignex = label++);										// jmp     alignex,nz
		}

		/* in big-endian mode, it's more complicated */
		else
		{
			/* 8-byte accesses must be word-aligned */
			if (size == 8)
			{
				UML_TEST(block, IREG(0), IMM(3));											// test    i0,3
				UML_JMPc(block, IF_NZ, alignex = label++);									// jmp     alignex,nz
			}

			/* unaligned 2 and 4 byte accesses need to be broken up */
			else
			{
				UML_TEST(block, IREG(0), IMM(size - 1));									// test    i0,size-1
				UML_JMPc(block, IF_NZ, unaligned = label++);								// jmp     unaligned,nz
			}
		}
	}

	/* general case: assume paging and perform a translation */
	if (((ppc->cap & PPCCAP_OEA) && (mode & MODE_DATA_TRANSLATION)) || (iswrite && (ppc->cap & PPCCAP_4XX) && (mode & MODE_PROTECTION)))
	{
		UML_SHR(block, IREG(3), IREG(0), IMM(12));											// shr     i3,i0,12
		UML_LOAD(block, IREG(3), ppc->tlb_table, IREG(3), DWORD);							// load    i3,[tlb_table],i3,dword
		UML_TEST(block, IREG(3), IMM(iswrite ? TLB_WRITE : TLB_READ));						// test    i3,iswrite ? TLB_WRITE : TLB_READ
		UML_JMPc(block, IF_Z, tlbmiss = label++);											// jmp     tlbmiss,z
		UML_LABEL(block, tlbreturn = label++);											// tlbreturn:
		UML_ROLINS(block, IREG(0), IREG(3), IMM(0), IMM(0xfffff000));						// rolins  i0,i3,0,0xfffff000
	}
	UML_XOR(block, IREG(0), IREG(0), IMM((mode & MODE_LITTLE_ENDIAN) ? (8 - size) : 0));	// xor     i0,i0,8-size

	for (ramnum = 0; ramnum < PPC_MAX_FASTRAM; ramnum++)
		if (!Machine->debug_mode && ppc->impstate->fastram[ramnum].base != NULL && (!iswrite || !ppc->impstate->fastram[ramnum].readonly))
		{
			void *fastbase = (UINT8 *)ppc->impstate->fastram[ramnum].base - ppc->impstate->fastram[ramnum].start;
			UINT32 skip = label++;

			if (ppc->impstate->fastram[ramnum].end != 0xffffffff)
			{
				UML_CMP(block, IREG(0), IMM(ppc->impstate->fastram[ramnum].end));			// cmp     i0,end
				UML_JMPc(block, IF_A, skip);												// ja      skip
			}
			if (ppc->impstate->fastram[ramnum].start != 0x00000000)
			{
				UML_CMP(block, IREG(0), IMM(ppc->impstate->fastram[ramnum].start));			// cmp     i0,fastram_start
				UML_JMPc(block, IF_B, skip);												// jb      skip
			}

			if (!iswrite)
			{
				if (size == 1)
				{
					UML_XOR(block, IREG(0), IREG(0), IMM(fastxor >> 0));					// xor     i0,i0,fastxor >> 0
					UML_LOAD(block, IREG(0), fastbase, IREG(0), BYTE);						// load    i0,fastbase,i0,byte
				}
				else if (size == 2)
				{
					UML_SHR(block, IREG(0), IREG(0), IMM(1));								// shr     i0,i0,1
					UML_XOR(block, IREG(0), IREG(0), IMM(fastxor >> 1));					// xor     i0,i0,fastxor >> 1
					UML_LOAD(block, IREG(0), fastbase, IREG(0), WORD);						// load    i0,fastbase,i0,word
				}
				else if (size == 4)
				{
					UML_SHR(block, IREG(0), IREG(0), IMM(2));								// shr     i0,i0,2
					UML_XOR(block, IREG(0), IREG(0), IMM(fastxor >> 2));					// xor     i0,i0,fastxor >> 2
					UML_LOAD(block, IREG(0), fastbase, IREG(0), DWORD);						// load    i0,fastbase,i0,dword
				}
				else if (size == 8)
				{
					UML_SHR(block, IREG(0), IREG(0), IMM(3));								// shr     i0,i0,3
					UML_DLOAD(block, IREG(0), fastbase, IREG(0), QWORD);					// dload   i0,fastbase,i0,qword
				}
				UML_RET(block);																// ret
			}
			else
			{
				if (size == 1)
				{
					UML_XOR(block, IREG(0), IREG(0), IMM(fastxor >> 0));					// xor     i0,i0,fastxor >> 0
					UML_STORE(block, fastbase, IREG(0), IREG(1), BYTE);						// store   fastbase,i0,i1,byte
				}
				else if (size == 2)
				{
					UML_SHR(block, IREG(0), IREG(0), IMM(1));								// shr     i0,i0,1
					UML_XOR(block, IREG(0), IREG(0), IMM(fastxor >> 1));					// xor     i0,i0,fastxor >> 1
					UML_STORE(block, fastbase, IREG(0), IREG(1), WORD);						// store   fastbase,i0,i1,word
				}
				else if (size == 4)
				{
					UML_SHR(block, IREG(0), IREG(0), IMM(2));								// shr     i0,i0,2
					UML_XOR(block, IREG(0), IREG(0), IMM(fastxor >> 2));					// xor     i0,i0,fastxor >> 2
					if (ismasked)
					{
						UML_LOAD(block, IREG(3), fastbase, IREG(0), DWORD);					// load    i3,fastbase,i0,dword
						UML_AND(block, IREG(1), IREG(1), IREG(2));							// and     i1,i1,i2
						UML_XOR(block, IREG(2), IREG(2), IMM(0xffffffff));					// xor     i2,i2,0xfffffffff
						UML_AND(block, IREG(3), IREG(3), IREG(2));							// and     i3,i3,i2
						UML_OR(block, IREG(1), IREG(1), IREG(3));							// or      i1,i1,i3
					}
					UML_STORE(block, fastbase, IREG(0), IREG(1), DWORD);					// store   fastbase,i0,i1,dword
				}
				else if (size == 8)
				{
					UML_SHR(block, IREG(0), IREG(0), IMM(3));								// shr     i0,i0,3
					if (ismasked)
					{
						UML_DLOAD(block, IREG(3), fastbase, IREG(0), QWORD);				// dload   i3,fastbase,i0,qword
						UML_DAND(block, IREG(1), IREG(1), IREG(2));							// dand    i1,i1,i2
						UML_DXOR(block, IREG(2), IREG(2), IMM(U64(0xffffffffffffffff)));	// dxor    i2,i2,0xfffffffffffffffff
						UML_DAND(block, IREG(3), IREG(3), IREG(2));							// dand    i3,i3,i2
						UML_DOR(block, IREG(1), IREG(1), IREG(3));							// dor     i1,i1,i3
					}
					UML_DSTORE(block, fastbase, IREG(0), IREG(1), QWORD);					// dstore  fastbase,i0,i1,qword
				}
				UML_RET(block);																// ret
			}

			UML_LABEL(block, skip);														// skip:
		}

	switch (size)
	{
		case 1:
			if (iswrite)
				UML_WRITE(block, IREG(0), IREG(1), PROGRAM_BYTE);							// write   i0,i1,program_byte
			else
				UML_READ(block, IREG(0), IREG(0), PROGRAM_BYTE);							// read    i0,i0,program_byte
			break;

		case 2:
			if (iswrite)
			{
				if (!ismasked)
					UML_WRITE(block, IREG(0), IREG(1), PROGRAM_WORD);						// write   i0,i1,program_word
				else
					UML_WRITEM(block, IREG(0), IREG(2), IREG(1), PROGRAM_WORD);				// writem  i0,i2,i1,program_word
			}
			else
			{
				if (!ismasked)
					UML_READ(block, IREG(0), IREG(0), PROGRAM_WORD);						// read    i0,i0,program_word
				else
					UML_READM(block, IREG(0), IREG(0), IREG(2), PROGRAM_WORD);				// readm   i0,i0,i2,program_word
			}
			break;

		case 4:
			if (iswrite)
			{
				if (!ismasked)
					UML_WRITE(block, IREG(0), IREG(1), PROGRAM_DWORD);						// write   i0,i1,program_dword
				else
					UML_WRITEM(block, IREG(0), IREG(2), IREG(1), PROGRAM_DWORD);			// writem  i0,i2,i1,program_dword
			}
			else
			{
				if (!ismasked)
					UML_READ(block, IREG(0), IREG(0), PROGRAM_DWORD);						// read    i0,i0,program_dword
				else
					UML_READM(block, IREG(0), IREG(0), IREG(2), PROGRAM_DWORD);				// readm   i0,i0,i2,program_dword
			}
			break;

		case 8:
			if (iswrite)
			{
				if (!ismasked)
					UML_DWRITE(block, IREG(0), IREG(1), PROGRAM_QWORD);						// dwrite  i0,i1,program_qword
				else
					UML_DWRITEM(block, IREG(0), IREG(2), IREG(1), PROGRAM_QWORD);			// dwritem i0,i2,i1,program_qword
			}
			else
			{
				if (!ismasked)
					UML_DREAD(block, IREG(0), IREG(0), PROGRAM_QWORD);						// dread   i0,i0,program_qword
				else
					UML_DREADM(block, IREG(0), IREG(0), IREG(2), PROGRAM_QWORD);			// dreadm  i0,i0,i2,program_qword
			}
			break;
	}
	UML_RET(block);																			// ret

	/* handle unaligned accesses */
	if (unaligned != 0)
	{
		UML_LABEL(block, unaligned);													// unaligned:
		if (size == 2)
		{
			if (iswrite)
			{
				UML_MOV(block, MEM(&ppc->impstate->tempaddr), IREG(0));						// mov     [tempaddr],i0
				UML_MOV(block, MEM(&ppc->impstate->tempdata.w.l), IREG(1));					// mov     [tempdata],i1
				UML_SUB(block, IREG(0), IREG(0), IMM(1));									// sub     i0,i0,1
				UML_SHR(block, IREG(1), IREG(1), IMM(8));									// shr     i1,i1,8
				UML_MOV(block, IREG(2), IMM(0x00ff));										// mov     i2,0x00ff
				UML_CALLH(block, masked);													// callh   masked
				UML_ADD(block, IREG(0), MEM(&ppc->impstate->tempaddr), IMM(1));				// add     i0,[tempaddr],1
				UML_SHL(block, IREG(1), MEM(&ppc->impstate->tempdata.w.l), IMM(8));			// shl     i1,[tempdata],8
				UML_MOV(block, IREG(2), IMM(0xff00));										// mov     i2,0xff00
				UML_CALLH(block, masked);													// callh   masked
			}
			else
			{
				UML_MOV(block, MEM(&ppc->impstate->tempaddr), IREG(0));						// mov     [tempaddr],i0
				UML_SUB(block, IREG(0), IREG(0), IMM(1));									// sub     i0,i0,1
				UML_MOV(block, IREG(2), IMM(0x00ff));										// mov     i2,0x00ff
				UML_CALLH(block, masked);													// callh   masked
				UML_SHL(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0), IMM(8));			// shl     [tempdata],i0,8
				UML_ADD(block, IREG(0), MEM(&ppc->impstate->tempaddr), IMM(1));				// add     i0,[tempaddr],1
				UML_MOV(block, IREG(2), IMM(0xff00));										// mov     i2,0xff00
				UML_CALLH(block, masked);													// callh   masked
				UML_SHR(block, IREG(0), IREG(0), IMM(8));									// shr     i0,i0,8
				UML_OR(block, IREG(0), IREG(0), MEM(&ppc->impstate->tempdata.w.l));			// or      i0,i0,[tempdata]
			}
		}
		else if (size == 4)
		{
			int offs2, offs3;
			if (iswrite)
			{
				UML_MOV(block, MEM(&ppc->impstate->tempaddr), IREG(0));						// mov     [tempaddr],i0
				UML_MOV(block, MEM(&ppc->impstate->tempdata.w.l), IREG(1));					// mov     [tempdata],i1
				UML_TEST(block, IREG(0), IMM(2));											// test    i0,i0,2
				UML_JMPc(block, IF_NZ, offs2 = label++);									// jnz     offs2
				UML_SUB(block, IREG(0), IREG(0), IMM(1));									// sub     i0,i0,1
				UML_SHR(block, IREG(1), IREG(1), IMM(8));									// shr     i1,i1,8
				UML_MOV(block, IREG(2), IMM(0x00ffffff));									// mov     i2,0x00ffffff
				UML_CALLH(block, masked);													// callh   masked
				UML_ADD(block, IREG(0), MEM(&ppc->impstate->tempaddr), IMM(3));				// add     i0,[tempaddr],3
				UML_SHL(block, IREG(1), MEM(&ppc->impstate->tempdata.w.l), IMM(24));		// shl     i1,[tempdata],24
				UML_MOV(block, IREG(2), IMM(0xff000000));									// mov     i2,0xff000000
				UML_CALLH(block, masked);													// callh   masked
				UML_RET(block);																// ret
				UML_LABEL(block, offs2);												// offs2:
				UML_TEST(block, IREG(0), IMM(1));											// test    i0,i0,1
				UML_JMPc(block, IF_NZ, offs3 = label++);									// jnz     offs3
				UML_SUB(block, IREG(0), IREG(0), IMM(2));									// sub     i0,i0,2
				UML_SHR(block, IREG(1), IREG(1), IMM(16));									// shr     i1,i1,16
				UML_MOV(block, IREG(2), IMM(0x0000ffff));									// mov     i2,0x0000ffff
				UML_CALLH(block, masked);													// callh   masked
				UML_ADD(block, IREG(0), MEM(&ppc->impstate->tempaddr), IMM(2));				// add     i0,[tempaddr],2
				UML_SHL(block, IREG(1), MEM(&ppc->impstate->tempdata.w.l), IMM(16));		// shl     i1,[tempdata],16
				UML_MOV(block, IREG(2), IMM(0xffff0000));									// mov     i2,0xffff0000
				UML_CALLH(block, masked);													// callh   masked
				UML_RET(block);																// ret
				UML_LABEL(block, offs3);												// offs3:
				UML_SUB(block, IREG(0), IREG(0), IMM(3));									// sub     i0,i0,3
				UML_SHR(block, IREG(1), IREG(1), IMM(24));									// shr     i1,i1,24
				UML_MOV(block, IREG(2), IMM(0x000000ff));									// mov     i2,0x000000ff
				UML_CALLH(block, masked);													// callh   masked
				UML_ADD(block, IREG(0), MEM(&ppc->impstate->tempaddr), IMM(1));				// add     i0,[tempaddr],1
				UML_SHL(block, IREG(1), MEM(&ppc->impstate->tempdata.w.l), IMM(8));			// shl     i1,[tempdata],8
				UML_MOV(block, IREG(2), IMM(0xffffff00));									// mov     i2,0xffffff00
				UML_CALLH(block, masked);													// callh   masked
			}
			else
			{
				UML_MOV(block, MEM(&ppc->impstate->tempaddr), IREG(0));						// mov     [tempaddr],i0
				UML_TEST(block, IREG(0), IMM(2));											// test    i0,i0,2
				UML_JMPc(block, IF_NZ, offs2 = label++);									// jnz     offs2
				UML_SUB(block, IREG(0), IREG(0), IMM(1));									// sub     i0,i0,1
				UML_MOV(block, IREG(2), IMM(0x00ffffff));									// mov     i2,0x00ffffff
				UML_CALLH(block, masked);													// callh   masked
				UML_SHL(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0), IMM(8));			// shl     [tempdata],i0,8
				UML_ADD(block, IREG(0), MEM(&ppc->impstate->tempaddr), IMM(3));				// add     i0,[tempaddr],3
				UML_MOV(block, IREG(2), IMM(0xff000000));									// mov     i2,0xff000000
				UML_CALLH(block, masked);													// callh   masked
				UML_SHR(block, IREG(0), IREG(0), IMM(24));									// shr     i0,i0,24
				UML_OR(block, IREG(0), IREG(0), MEM(&ppc->impstate->tempdata.w.l));			// or      i0,i0,[tempdata]
				UML_RET(block);																// ret
				UML_LABEL(block, offs2);												// offs2:
				UML_TEST(block, IREG(0), IMM(1));											// test    i0,i0,1
				UML_JMPc(block, IF_NZ, offs3 = label++);									// jnz     offs3
				UML_SUB(block, IREG(0), IREG(0), IMM(2));									// sub     i0,i0,2
				UML_MOV(block, IREG(2), IMM(0x0000ffff));									// mov     i2,0x0000ffff
				UML_CALLH(block, masked);													// callh   masked
				UML_SHL(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0), IMM(16));		// shl     [tempdata],i0,16
				UML_ADD(block, IREG(0), MEM(&ppc->impstate->tempaddr), IMM(2));				// add     i0,[tempaddr],2
				UML_MOV(block, IREG(2), IMM(0xffff0000));									// mov     i2,0xffff0000
				UML_CALLH(block, masked);													// callh   masked
				UML_SHR(block, IREG(0), IREG(0), IMM(16));									// shr     i0,i0,16
				UML_OR(block, IREG(0), IREG(0), MEM(&ppc->impstate->tempdata.w.l));			// or      i0,i0,[tempdata]
				UML_RET(block);																// ret
				UML_LABEL(block, offs3);												// offs3:
				UML_SUB(block, IREG(0), IREG(0), IMM(3));									// sub     i0,i0,3
				UML_MOV(block, IREG(2), IMM(0x000000ff));									// mov     i2,0x000000ff
				UML_CALLH(block, masked);													// callh   masked
				UML_SHL(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0), IMM(24));		// shl     [tempdata],i0,24
				UML_ADD(block, IREG(0), MEM(&ppc->impstate->tempaddr), IMM(1));				// add     i0,[tempaddr],1
				UML_MOV(block, IREG(2), IMM(0xffffff00));									// mov     i2,0xffffff00
				UML_CALLH(block, masked);													// callh   masked
				UML_SHR(block, IREG(0), IREG(0), IMM(8));									// shr     i0,i0,8
				UML_OR(block, IREG(0), IREG(0), MEM(&ppc->impstate->tempdata.w.l));			// or      i0,i0,[tempdata]
			}
		}
		UML_RET(block);																		// ret
	}

	/* handle an alignment exception */
	if (alignex != 0)
	{
		UML_LABEL(block, alignex);														// alignex:
		UML_RECOVER(block, SPR32(SPROEA_DSISR), MAPVAR_DSISR);								// recover [dsisr],DSISR
		UML_EXH(block, ppc->impstate->exception[EXCEPTION_ALIGN], IREG(0));					// exh     align,i0
	}

	/* handle a TLB miss */
	if (tlbmiss != 0)
	{
		int dsi;

		UML_LABEL(block, tlbmiss);														// tlbmiss:
		UML_MOV(block, MEM(&ppc->param0), IREG(0));											// mov     [param0],i0
		UML_MOV(block, MEM(&ppc->param1), IMM(iswrite ? TRANSLATE_WRITE : TRANSLATE_READ));	// mov     [param1],TRANSLATE_READ/WRITE
		UML_CALLC(block, ppccom_tlb_fill, ppc);												// callc   tlbfill,ppc
		UML_CMP(block, MEM(&ppc->param0), IMM(1));											// cmp     [param0],1
		UML_JMPc(block, IF_A, dsi = label++);												// jmp     dsi,A
		UML_SHR(block, IREG(3), IREG(0), IMM(12));											// shr     i3,i0,12
		UML_LOAD(block, IREG(3), ppc->tlb_table, IREG(3), DWORD);							// load    i3,[tlb_table],i3,dword
		UML_JMP(block, tlbreturn);															// jmp     tlbreturn
		UML_LABEL(block, dsi);															// dsi:
		
		/* 4XX case: protection exception */
		if (ppc->cap & PPCCAP_4XX)
		{
			UML_MOV(block, SPR32(SPR4XX_DEAR), IREG(0));									// mov     [dear],i0
			UML_EXH(block, ppc->impstate->exception[EXCEPTION_DSI], IREG(0));				// exh     dsi,i0
		}
		
		/* 603 case: TLBMISS exception */
		else if (ppc->cap & PPCCAP_603_MMU)
		{
			UML_MOV(block, SPR32(SPR603_DMISS), IREG(0));									// mov     [dmiss],i0
			UML_MOV(block, SPR32(SPR603_DCMP), MEM(&ppc->mmu603_cmp));						// mov     [dcmp],[mmu603_cmp]
			UML_MOV(block, SPR32(SPR603_HASH1), MEM(&ppc->mmu603_hash[0]));					// mov     [hash1],[mmu603_hash][0]
			UML_MOV(block, SPR32(SPR603_HASH2), MEM(&ppc->mmu603_hash[1]));					// mov     [hash2],[mmu603_hash][1]
			if (iswrite)
				UML_EXH(block, ppc->impstate->exception[EXCEPTION_DTLBMISSS], IREG(0));		// exh     dtlbmisss,i0
			else
				UML_EXH(block, ppc->impstate->exception[EXCEPTION_DTLBMISSL], IREG(0));		// exh     dtlbmissl,i0
		}
		
		/* general case: DSI exception */
		else
		{
			UML_MOV(block, SPR32(SPROEA_DSISR), MEM(&ppc->param0));							// mov     [dsisr],[param0]
			UML_EXH(block, ppc->impstate->exception[EXCEPTION_DSI], IREG(0));				// exh     dsi,i0
		}
	}

	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_swap_tgpr - generate a
    subroutine to swap GPR0-3 with TGPR0-3
-------------------------------------------------*/

static void static_generate_swap_tgpr(drcuml_state *drcuml)
{
	drcuml_block *block;
	jmp_buf errorbuf;
	int regnum;

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_swap_tgpr");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 30, &errorbuf);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &ppc->impstate->swap_tgpr, "swap_tgpr");
	UML_HANDLE(block, ppc->impstate->swap_tgpr);											// handle  swap_tgpr
	for (regnum = 0; regnum < 4; regnum++)
	{
		UML_MOV(block, IREG(1), R32(regnum));												// mov     i1,r[regnum]
		UML_MOV(block, R32(regnum), MEM(&ppc->mmu603_r[regnum]));							// mov     r[regnum],mmu603_r[regnum]
		UML_MOV(block, MEM(&ppc->mmu603_r[regnum]), IREG(1));								// mov     mmu603_r[regnum],i1
	}
	UML_RET(block);																			// ret

	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_lsw_entries - generate a
    subroutine to perform LSWI/LSWX; one handle
    for each possible register
-------------------------------------------------*/

static void static_generate_lsw_entries(drcuml_state *drcuml, int mode)
{
	drcuml_block *block;
	jmp_buf errorbuf;
	int regnum;

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_lsw_entries");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 32 * 30, &errorbuf);

	/* iterate over all possible registers */
	for (regnum = 0; regnum < 32; regnum++)
	{
		char temp[20];
		
		/* allocate a handle */
		sprintf(temp, "lsw%d", regnum);
		alloc_handle(drcuml, &ppc->impstate->lsw[mode][regnum], temp);
		UML_HANDLE(block, ppc->impstate->lsw[mode][regnum]);								// handle  lsw<regnum>
		UML_LABEL(block, regnum);														// regnum:
		UML_ADD(block, IREG(0), MEM(&ppc->impstate->updateaddr), IMM(0));					// add     i0,[updateaddr],0
		UML_CALLH(block, ppc->impstate->read8[mode]);										// callh   read8
		UML_ROLAND(block, R32(regnum), IREG(0), IMM(24), IMM(0xff000000));					// roland  reg,i0,24,0xff000000
		UML_SUB(block, MEM(&ppc->impstate->swcount), MEM(&ppc->impstate->swcount), IMM(1));	// sub     [swcount],[swcount],1
		UML_RETc(block, IF_Z);																// ret     z
		UML_ADD(block, IREG(0), MEM(&ppc->impstate->updateaddr), IMM(1));					// add     i0,[updateaddr],1
		UML_CALLH(block, ppc->impstate->read8[mode]);										// callh   read8
		UML_ROLAND(block, IREG(0), IREG(0), IMM(16), IMM(0x00ff0000));						// roland  i0,i0,16,0x00ff0000
		UML_OR(block, R32(regnum), R32(regnum), IREG(0));									// or      reg,i0
		UML_SUB(block, MEM(&ppc->impstate->swcount), MEM(&ppc->impstate->swcount), IMM(1));	// sub     [swcount],[swcount],1
		UML_RETc(block, IF_Z);																// ret     z
		UML_ADD(block, IREG(0), MEM(&ppc->impstate->updateaddr), IMM(2));					// add     i0,[updateaddr],2
		UML_CALLH(block, ppc->impstate->read8[mode]);										// callh   read8
		UML_ROLAND(block, IREG(0), IREG(0), IMM(8), IMM(0x0000ff00));						// roland  i0,i0,8,0x0000ff00
		UML_OR(block, R32(regnum), R32(regnum), IREG(0));									// or      reg,i0
		UML_SUB(block, MEM(&ppc->impstate->swcount), MEM(&ppc->impstate->swcount), IMM(1));	// sub     [swcount],[swcount],1
		UML_RETc(block, IF_Z);																// ret     z
		UML_ADD(block, IREG(0), MEM(&ppc->impstate->updateaddr), IMM(3));					// add     i0,[updateaddr],3
		UML_ADD(block, MEM(&ppc->impstate->updateaddr), IREG(0), IMM(1));					// add     [updateaddr],i0,1
		UML_CALLH(block, ppc->impstate->read8[mode]);										// callh   read8
		UML_ROLAND(block, IREG(0), IREG(0), IMM(0), IMM(0x000000ff));						// roland  i0,i0,0,0x000000ff
		UML_OR(block, R32(regnum), R32(regnum), IREG(0));									// or      reg,i0
		UML_SUB(block, MEM(&ppc->impstate->swcount), MEM(&ppc->impstate->swcount), IMM(1));	// sub     [swcount],[swcount],1
		UML_RETc(block, IF_Z);																// ret     z
		UML_JMP(block, (regnum + 1) % 32);													// jmp     nextreg
	}

	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_stsw_entries - generate a
    subroutine to perform LSWI/LSWX; one handle
    for each possible register
-------------------------------------------------*/

static void static_generate_stsw_entries(drcuml_state *drcuml, int mode)
{
	drcuml_block *block;
	jmp_buf errorbuf;
	int regnum;

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_stsw_entries");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 32 * 30, &errorbuf);

	/* iterate over all possible registers */
	for (regnum = 0; regnum < 32; regnum++)
	{
		char temp[20];
		
		/* allocate a handle */
		sprintf(temp, "stsw%d", regnum);
		alloc_handle(drcuml, &ppc->impstate->stsw[mode][regnum], temp);
		UML_HANDLE(block, ppc->impstate->stsw[mode][regnum]);								// handle  stsw<regnum>
		UML_LABEL(block, regnum);														// regnum:
		UML_ADD(block, IREG(0), MEM(&ppc->impstate->updateaddr), IMM(0));					// add     i0,[updateaddr],0
		UML_ROLAND(block, IREG(1), R32(regnum), IMM(8), IMM(0xff));							// roland  i1,regnum,8,0xff
		UML_CALLH(block, ppc->impstate->write8[mode]);										// callh   write8
		UML_SUB(block, MEM(&ppc->impstate->swcount), MEM(&ppc->impstate->swcount), IMM(1));	// sub     [swcount],[swcount],1
		UML_RETc(block, IF_Z);																// ret     z
		UML_ADD(block, IREG(0), MEM(&ppc->impstate->updateaddr), IMM(1));					// add     i0,[updateaddr],1
		UML_ROLAND(block, IREG(1), R32(regnum), IMM(16), IMM(0xff));						// roland  i1,regnum,16,0xff
		UML_CALLH(block, ppc->impstate->write8[mode]);										// callh   write8
		UML_SUB(block, MEM(&ppc->impstate->swcount), MEM(&ppc->impstate->swcount), IMM(1));	// sub     [swcount],[swcount],1
		UML_RETc(block, IF_Z);																// ret     z
		UML_ADD(block, IREG(0), MEM(&ppc->impstate->updateaddr), IMM(2));					// add     i0,[updateaddr],2
		UML_ROLAND(block, IREG(1), R32(regnum), IMM(24), IMM(0xff));						// roland  i1,regnum,24,0xff
		UML_CALLH(block, ppc->impstate->write8[mode]);										// callh   write8
		UML_SUB(block, MEM(&ppc->impstate->swcount), MEM(&ppc->impstate->swcount), IMM(1));	// sub     [swcount],[swcount],1
		UML_RETc(block, IF_Z);																// ret     z
		UML_ADD(block, IREG(0), MEM(&ppc->impstate->updateaddr), IMM(3));					// add     i0,[updateaddr],3
		UML_ADD(block, MEM(&ppc->impstate->updateaddr), IREG(0), IMM(1));					// add     [updateaddr],i0,1
		UML_ROLAND(block, IREG(1), R32(regnum), IMM(0), IMM(0xff));							// roland  i1,regnum,0,0xff
		UML_CALLH(block, ppc->impstate->write8[mode]);										// callh   write8
		UML_SUB(block, MEM(&ppc->impstate->swcount), MEM(&ppc->impstate->swcount), IMM(1));	// sub     [swcount],[swcount],1
		UML_RETc(block, IF_Z);																// ret     z
		UML_JMP(block, (regnum + 1) % 32);													// jmp     nextreg
	}

	drcuml_block_end(block);
}


/***************************************************************************
    CODE GENERATION
***************************************************************************/

/*-------------------------------------------------
    generate_update_mode - update the mode based
    on the MSR
-------------------------------------------------*/

static void generate_update_mode(drcuml_block *block)
{
	/* LE in bit 0 of mode */
	UML_AND(block, IREG(0), MSR32, IMM(MSR_LE));											// and     i0,msr,MSR_LE
	
	/* DR (OEA) or PE (4XX) in bit 1 of mode */
	if (ppc->cap & PPCCAP_OEA)
		UML_ROLAND(block, IREG(1), MSR32, IMM(29), IMM(0x02));								// roland  i1,[msr],29,0x02
	else if (ppc->cap & PPCCAP_4XX)
		UML_ROLAND(block, IREG(1), MSR32, IMM(30), IMM(0x02));								// roland  i1,[msr],30,0x02
	UML_OR(block, IREG(0), IREG(0), IREG(1));												// or      i0,i0,i1
	
	/* PR in bit 2 of mode */
	UML_ROLAND(block, IREG(1), MSR32, IMM(20), IMM(0x04));									// roland  i1,[msr],20,0x04
	UML_OR(block, MEM(&ppc->impstate->mode), IREG(0), IREG(1));								// or      [mode],i0,i1
}


/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/

static void generate_update_cycles(drcuml_block *block, compiler_state *compiler, drcuml_ptype ptype, UINT64 pvalue, int allow_exception)
{
	/* check full interrupts if pending */
	if (compiler->checkints)
	{
		drcuml_codelabel skip;

		compiler->checkints = FALSE;
		UML_TEST(block, MEM(&ppc->irq_pending), IMM(~0));									// test    [irq_pending],0
		UML_JMPc(block, IF_Z, skip = compiler->labelnum++);									// jmp     skip,Z
		UML_TEST(block, MSR32, IMM(MSR_EE));												// test    [msr],MSR_EE
		UML_JMPc(block, IF_Z, skip);														// jmp     skip,Z
		UML_MOV(block, IREG(0), PARAM(ptype, pvalue));										// mov     i0,nextpc
		UML_MOV(block, IREG(1), IMM(compiler->cycles));										// mov     i1,cycles
		UML_CALLH(block, ppc->impstate->exception_norecover[EXCEPTION_EI]);					// callh   interrupt_norecover
		UML_LABEL(block, skip);															// skip:
	}

	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, MEM(&ppc->icount), MEM(&ppc->icount), MAPVAR_CYCLES);				// sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);												// mapvar  cycles,0
		if (allow_exception)
			UML_EXHc(block, IF_S, ppc->impstate->out_of_cycles, PARAM(ptype, pvalue));		// exh     out_of_cycles,nextpc
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
		UML_COMMENT(block, "[Validation for %08X]", seqhead->pc);							// comment

	/* loose verify or single instruction: just compare and fail */
	if (!(ppc->impstate->drcoptions & PPCDRC_STRICT_VERIFY) || seqhead->next == NULL)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			UML_LOAD(block, IREG(0), seqhead->opptr.l, IMM(0), DWORD);						// load    i0,*opptr,dword
			UML_CMP(block, IREG(0), IMM(*seqhead->opptr.l));								// cmp     i0,*opptr
			UML_EXHc(block, IF_NE, ppc->impstate->nocode, IMM(seqhead->pc));				// exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
#if 0
		for (curdesc = seqhead->next; curdesc != seqlast->next; curdesc = curdesc->next)
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				UML_LOAD(block, IREG(0), curdesc->opptr.l, IMM(0), DWORD);					// load    i0,*opptr,dword
				UML_CMP(block, IREG(0), IMM(*curdesc->opptr.l));							// cmp     i0,*opptr
				UML_EXHc(block, IF_NE, ppc->impstate->nocode, IMM(seqhead->pc));			// exne    nocode,seqhead->pc
			}
#else
		UINT32 sum = 0;
		UML_LOAD(block, IREG(0), seqhead->opptr.l, IMM(0), DWORD);							// load    i0,*opptr,dword
		sum += *seqhead->opptr.l;
		for (curdesc = seqhead->next; curdesc != seqlast->next; curdesc = curdesc->next)
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				UML_LOAD(block, IREG(1), curdesc->opptr.l, IMM(0), DWORD);					// load    i1,*opptr,dword
				UML_ADD(block, IREG(0), IREG(0), IREG(1));									// add     i0,i0,i1
				sum += *curdesc->opptr.l;
			}
		UML_CMP(block, IREG(0), IMM(sum));													// cmp     i0,sum
		UML_EXHc(block, IF_NE, ppc->impstate->nocode, IMM(seqhead->pc));					// exne    nocode,seqhead->pc
#endif
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

static void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	int hotnum;

	/* add an entry for the log */
	if (LOG_UML && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(block, desc->pc, *desc->opptr.l);

	/* set the PC map variable */
	UML_MAPVAR(block, MAPVAR_PC, desc->pc);													// mapvar  PC,desc->pc

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* is this a hotspot? */
	for (hotnum = 0; hotnum < PPC_MAX_HOTSPOTS; hotnum++)
		if (ppc->impstate->hotspot[hotnum].pc != 0 && desc->pc == ppc->impstate->hotspot[hotnum].pc && *desc->opptr.l == ppc->impstate->hotspot[hotnum].opcode)
		{
			compiler->cycles += ppc->impstate->hotspot[hotnum].cycles;
			break;
		}

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);										// mapvar  CYCLES,compiler->cycles

	/* if we want a probe, add it here */
	if (desc->pc == PROBE_ADDRESS)
		UML_CALLC(block, cfunc_printf_probe, desc->pc);										// callc   cfunc_printf_probe,desc->pc

	/* if we are debugging, call the debugger */
	if (Machine->debug_mode)
	{
		UML_MOV(block, MEM(&ppc->pc), IMM(desc->pc));										// mov     [pc],desc->pc
		save_fast_iregs(block);																// <save fastregs>
		UML_DEBUG(block, IMM(desc->pc));													// debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, MEM(&ppc->pc), IMM(desc->pc));										// mov     [pc],desc->pc
		save_fast_iregs(block);																// <save fastregs>
		UML_EXIT(block, IMM(EXECUTE_UNMAPPED_CODE));										// exit    EXECUTE_UNMAPPED_CODE
	}

	/* if we hit a compiler page fault, it's just like a TLB mismatch */
	if (desc->flags & OPFLAG_COMPILER_PAGE_FAULT)
	{
		if (PRINTF_MMU)
		{
			UML_MOV(block, MEM(&ppc->impstate->arg0), IMM(desc->pc));						// mov     [arg0],desc->pc
			UML_CALLC(block, cfunc_printf_debug, "Compiler page fault @ %08X\n");			// callc   printf_debug
		}
		UML_EXH(block, ppc->impstate->tlb_mismatch, IMM(0));								// exh     tlb_mismatch,0
	}

	/* validate our TLB entry at this PC; if we fail, we need to handle it */
	if ((desc->flags & OPFLAG_VALIDATE_TLB) && (ppc->impstate->mode & MODE_DATA_TRANSLATION))
	{
		/* if we currently have a valid TLB read entry, we just verify */
		if (ppc->tlb_table[desc->pc >> 12] != 0)
		{
			if (PRINTF_MMU)
			{
				UML_MOV(block, MEM(&ppc->impstate->arg0), IMM(desc->pc));					// mov     [arg0],desc->pc
				UML_CALLC(block, cfunc_printf_debug, "Checking TLB at @ %08X\n");			// callc   printf_debug
			}
			UML_LOAD(block, IREG(0), &ppc->tlb_table[desc->pc >> 12], IMM(0), DWORD);		// load    i0,tlb_table[desc->pc >> 12],dword
			UML_CMP(block, IREG(0), IMM(ppc->tlb_table[desc->pc >> 12]));					// cmp     i0,*tlbentry
			UML_EXHc(block, IF_NE, ppc->impstate->tlb_mismatch, IMM(0));					// exh     tlb_mismatch,0,NE
		}

		/* otherwise, we generate an unconditional exception */
		else
		{
			if (PRINTF_MMU)
			{
				UML_MOV(block, MEM(&ppc->impstate->arg0), IMM(desc->pc));						// mov     [arg0],desc->pc
				UML_CALLC(block, cfunc_printf_debug, "No valid TLB @ %08X\n");				// callc   printf_debug
			}
			UML_EXH(block, ppc->impstate->tlb_mismatch, IMM(0));										// exh     tlb_mismatch,0
		}
	}

	/* if this is an invalid opcode, generate the exception now */
	if (desc->flags & OPFLAG_INVALID_OPCODE)
		UML_EXH(block, ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x80000));						// exh    exception_program,0x80000

	/* if this is a privileged opcode in user mode, generate the exception */
	else if ((desc->flags & OPFLAG_PRIVILEGED) && (ppc->impstate->mode & MODE_USER))
		UML_EXH(block, ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x40000));						// exh    exception_program,0x40000

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	else if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(block, compiler, desc))
		{
			UML_MOV(block, MEM(&ppc->pc), IMM(desc->pc));								// mov     [pc],desc->pc
			UML_CALLC(block, cfunc_unimplemented, (void *)(FPTR)*desc->opptr.l);			// callc   cfunc_unimplemented
		}
	}
}


/*------------------------------------------------------------------
    generate_compute_flags - compute CR0 and/or XER flags
------------------------------------------------------------------*/

static void generate_compute_flags(drcuml_block *block, int updatecr, UINT32 xermask, int invertcarry)
{
	int xerflags = ((xermask & XER_OV) ? DRCUML_FLAG_V : 0) | ((xermask & XER_CA) ? DRCUML_FLAG_C : 0);
	UINT32 xermask_so = xermask | ((xermask & XER_OV) ? XER_SO : 0);

	/* easy case: nothing to do */
	if (!updatecr && xermask == 0)
		return;

	/* semi-easy case: crfield only */
	if (xermask == 0)
	{
		UML_MOV(block, IREG(0), CR32);														// mov     i0,cr32
		UML_GETFLGS(block, IREG(1), DRCUML_FLAG_S | DRCUML_FLAG_Z);							// getflgs i1,sz
		UML_LOAD(block, IREG(1), ppc->impstate->sz_cr_table, IREG(1), DWORD);				// load    i1,sz_cr_table,i1,dword
		UML_ROLINS(block, IREG(0), IREG(1), IMM(0), IMM(CRMASK(0)));						// rolins  i0,i1,0,crmask(field)
		UML_AND(block, IREG(1), SPR32(SPR_XER), IMM(XER_SO));								// and     i1,[xer],XER_SO
		UML_SHR(block, IREG(1), IREG(1), IMM(3));											// shr     i1,i1,3
		UML_OR(block, CR32, IREG(0), IREG(1));												// or      [cr],i0,i1
		return;
	}

	/* semi-easy case: xer only */
	if (!updatecr)
	{
		if (xermask & XER_OV)
		{
			UML_MOV(block, IREG(0), SPR32(SPR_XER));										// mov     i0,[xer]
			UML_GETFLGS(block, IREG(1), xerflags);											// getflgs i1,xerflags
			if (invertcarry && (xermask & XER_CA))
				UML_XOR(block, IREG(1), IREG(1), IMM(DRCUML_FLAG_C));						// xor     i1,i1,FLAG_C
			UML_ROLINS(block, IREG(0), IREG(1), IMM(29), IMM(xermask));						// rolins  i0,i1,29,crmask(xermask)
			UML_AND(block, IREG(1), SPR32(SPR_XER), IMM(XER_OV));							// and     i1,[xer],XER_OV
			UML_SHL(block, IREG(1), IREG(1), IMM(1));										// shl     i1,i1,1
			UML_OR(block, SPR32(SPR_XER), IREG(0), IREG(1));								// or      [xer],i0,i1
		}
		else
		{
			UML_SETc(block, invertcarry ? IF_NC : IF_C, IREG(0));							// setc    i0,nc/c
			UML_ROLINS(block, SPR32(SPR_XER), IREG(0), IMM(29), IMM(XER_CA));				// rolins  [xer],i0,29,XER_CA
		}
		return;
	}

	/* tricky case: both */
	UML_GETFLGS(block, IREG(0), DRCUML_FLAG_S | DRCUML_FLAG_Z | xerflags);					// getflgs i0,sz|xerflags
	if (invertcarry && (xermask & XER_CA))
		UML_XOR(block, IREG(0), IREG(0), IMM(DRCUML_FLAG_C));								// xor     i0,i0,FLAG_C
	UML_LOAD(block, IREG(0), ppc->impstate->sz_cr_table, IREG(0), DWORD);					// load    i0,sz_cr_table,i0,dword
	UML_AND(block, IREG(1), SPR32(SPR_XER), IMM(~xermask));									// and     i1,[xer],~xermask
	UML_SHL(block, IREG(2), IREG(0), IMM(4));												// shl     i2,i0,4
	if (xermask_so != (XER_CA | XER_OV | XER_SO))
		UML_AND(block, IREG(2), IREG(2), IMM(xermask_so));									// and     i2,i2,xermask
	UML_OR(block, IREG(1), IREG(1), IREG(2));												// or      i1,i1,i2
	UML_MOV(block, SPR32(SPR_XER), IREG(1));												// mov     [xer],i1
	UML_SHR(block, IREG(1), IREG(1), IMM(3));												// shr     i1,i1,3
	UML_OR(block, IREG(0), IREG(0), IREG(1));												// or      i0,i0,i1
	UML_ROLINS(block, CR32, IREG(0), IMM(0), IMM(CRMASK(0)));								// rolins  [cr],i0,0,crmask(0)
}


/*-------------------------------------------------
    generate_branch - generate an unconditional
    branch
-------------------------------------------------*/

static void generate_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int source, UINT8 link)
{
	compiler_state compiler_temp = *compiler;
	UINT32 *srcptr = &ppc->spr[source];

	/* set the link if needed */
	if (link)
	{
		if (desc->targetpc == BRANCH_TARGET_DYNAMIC && source == SPR_LR)
		{
			UML_MOV(block, MEM(&ppc->impstate->tempaddr), MEM(srcptr));						// mov     [tempaddr],[lr]
			srcptr = &ppc->impstate->tempaddr;
		}
		UML_MOV(block, SPR32(SPR_LR), IMM(desc->pc + 4));									// mov     [lr],desc->pc + 4
	}

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(block, &compiler_temp, IMM(desc->targetpc), TRUE);			// <subtract cycles>
		if (desc->flags & OPFLAG_INTRABLOCK_BRANCH)
			UML_JMP(block, desc->targetpc | 0x80000000);									// jmp     desc->targetpc | 0x80000000
		else
			UML_HASHJMP(block, IMM(ppc->impstate->mode), IMM(desc->targetpc), ppc->impstate->nocode);
																							// hashjmp <mode>,desc->targetpc,nocode
	}
	else
	{
		generate_update_cycles(block, &compiler_temp, MEM(srcptr), TRUE);					// <subtract cycles>
		UML_HASHJMP(block, IMM(ppc->impstate->mode), MEM(srcptr), ppc->impstate->nocode);	// hashjmp <mode>,<rsreg>,nocode
	}

	/* update the label */
	compiler->labelnum = compiler_temp.labelnum;

	/* reset the mapvar to the current cycles */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);										// mapvar  CYCLES,compiler->cycles
}


/*-------------------------------------------------
    generate_branch_bo - generate a conditional
    branch based on the BO and BI fields
-------------------------------------------------*/

static void generate_branch_bo(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 bo, UINT32 bi, int source, int link)
{
	int skip = compiler->labelnum++;

	if (!(bo & 0x04))
	{
		UML_SUB(block, SPR32(SPR_CTR), SPR32(SPR_CTR), IMM(1));								// sub   [ctr],[ctr],1
		UML_JMPc(block, (bo & 0x02) ? IF_NZ : IF_Z, skip);									// jmp   skip,nz/z
	}
	if (!(bo & 0x10))
	{
		UML_TEST(block, CR32, IMM(0x80000000 >> bi));										// test  cr32,0x80000000 >> bi
		UML_JMPc(block, (bo & 0x08) ? IF_Z : IF_NZ, skip);									// jmp   skip,z/nz
	}
	generate_branch(block, compiler, desc, source, link);									// <branch>
	UML_LABEL(block, skip);																// skip:
}


/*-------------------------------------------------
    generate_opcode - generate code for a specific
    opcode
-------------------------------------------------*/

static int generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT32 opswitch = op >> 26;
	int regnum;

	switch (opswitch)
	{
		case 0x02:	/* TDI - 64-bit only */
		case 0x1e:	/* 0x1e group - 64-bit only */
		case 0x3a:	/* 0x3a group - 64-bit only */
		case 0x3e:	/* 0x3e group - 64-bit only */
			return FALSE;

		case 0x03:	/* TWI */
			UML_CMP(block, R32(G_RA(op)), IMM((INT16)G_SIMM(op)));							// cmp     ra,simm
			if (G_TO(op) & 0x10)
				UML_EXHc(block, IF_L, ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,l
			if (G_TO(op) & 0x08)
				UML_EXHc(block, IF_G, ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,g
			if (G_TO(op) & 0x04)
				UML_EXHc(block, IF_E, ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,e
			if (G_TO(op) & 0x02)
				UML_EXHc(block, IF_B,  ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,b
			if (G_TO(op) & 0x01)
				UML_EXHc(block, IF_A,  ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,a
			return TRUE;

		case 0x07:	/* MULLI */
			UML_MULS(block, R32(G_RD(op)), R32(G_RD(op)), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));
																							// muls    rd,rd,ra,simm
			return TRUE;

		case 0x0e:	/* ADDI */
			UML_ADD(block, R32(G_RD(op)), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));			// add     rd,ra,simm
			return TRUE;

		case 0x0f:	/* ADDIS */
			UML_ADD(block, R32(G_RD(op)), R32Z(G_RA(op)), IMM(G_SIMM(op) << 16));			// add     rd,ra,simm << 16
			return TRUE;

		case 0x0a:	/* CMPLI */
			UML_CMP(block, R32(G_RA(op)), IMM(G_UIMM(op)));									// cmp     ra,uimm
			UML_GETFLGS(block, IREG(0), DRCUML_FLAG_Z | DRCUML_FLAG_C);						// getflgs i0,zc
			UML_LOAD(block, IREG(0), ppc->impstate->cmpl_cr_table, IREG(0), DWORD);			// load    i0,cmpl_cr_table,i0,dword
			UML_ROLINS(block, IREG(0), SPR32(SPR_XER), IMM(32 - (3 + 4 * G_CRFD(op))), IMM(CRMASK(G_CRFD(op)) & 0x11111111));
																							// rolins  i0,[xer],32-(3+4*crfd),0x11111111 & crmask[cr]
			UML_ROLINS(block, CR32, IREG(0), IMM(0), IMM(CRMASK(G_CRFD(op))));				// rolins  cr,i0,0,crmask[cr]
			return TRUE;

		case 0x0b:	/* CMPI */
			UML_CMP(block, R32(G_RA(op)), IMM((INT16)G_SIMM(op)));							// cmp     ra,uimm
			UML_GETFLGS(block, IREG(0), DRCUML_FLAG_Z | DRCUML_FLAG_C | DRCUML_FLAG_S);		// getflgs i0,zcs
			UML_LOAD(block, IREG(0), ppc->impstate->cmp_cr_table, IREG(0), DWORD);			// load    i0,cmp_cr_table,i0,dword
			UML_ROLINS(block, IREG(0), SPR32(SPR_XER), IMM(32 - (3 + 4 * G_CRFD(op))), IMM(CRMASK(G_CRFD(op)) & 0x11111111));
																							// rolins  i0,[xer],32-(3+4*crfd),0x11111111 & crmask[cr]
			UML_ROLINS(block, CR32, IREG(0), IMM(0), IMM(CRMASK(G_CRFD(op))));				// rolins  cr,i0,0,crmask[cr]
			return TRUE;

		case 0x08:	/* SUBFIC */
			UML_SUB(block, R32(G_RD(op)), IMM((INT16)G_SIMM(op)), R32(G_RA(op)));			// sub     rd,simm,ra
			generate_compute_flags(block, FALSE, XER_CA, TRUE);								// <update flags>
			return TRUE;

		case 0x0c:	/* ADDIC */
			UML_ADD(block, R32(G_RD(op)), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));			// add     rd,ra,simm
			generate_compute_flags(block, FALSE, XER_CA, FALSE);							// <update flags>
			return TRUE;

		case 0x0d:	/* ADDIC. */
			UML_ADD(block, R32(G_RD(op)), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));			// add     rd,ra,simm
			generate_compute_flags(block, TRUE, XER_CA, FALSE);								// <update flags>
			return TRUE;

		case 0x10:	/* BCx */
			generate_branch_bo(block, compiler, desc, G_BO(op), G_BI(op), 0, op & M_LK);	// <branch conditional>
			return TRUE;

		case 0x11:	/* SC */
			UML_MAPVAR(block, MAPVAR_PC, desc->pc + 4);										// mapvar  PC,desc->pc+4
			UML_EXH(block, ppc->impstate->exception[EXCEPTION_SYSCALL], IMM(0));			// exh     syscall,0
			return TRUE;

		case 0x12:	/* Bx */
			generate_branch(block, compiler, desc, 0, op & M_LK);							// <branch>
			return TRUE;

		case 0x13:	/* 0x13 group */
			return generate_instruction_13(block, compiler, desc);							// <group13>

		case 0x14:	/* RLWIMIx */
			UML_ROLINS(block, R32(G_RA(op)), R32(G_RS(op)), IMM(G_SH(op)), IMM(compute_rlw_mask(G_MB(op), G_ME(op))));
																							// rolins ra,rs,sh,mask
			if (op & M_RC)
				generate_compute_flags(block, TRUE, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x15:	/* RLWINMx */
			UML_ROLAND(block, R32(G_RA(op)), R32(G_RS(op)), IMM(G_SH(op)), IMM(compute_rlw_mask(G_MB(op), G_ME(op))));
																							// roland ra,rs,sh,mask
			if (op & M_RC)
				generate_compute_flags(block, TRUE, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x17:	/* RLWNMx */
			UML_ROLAND(block, R32(G_RA(op)), R32(G_RS(op)), R32(G_RB(op)), IMM(compute_rlw_mask(G_MB(op), G_ME(op))));
																							// roland ra,rs,rb,mask
			if (op & M_RC)
				generate_compute_flags(block, TRUE, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x18:	/* ORI */
			UML_OR(block, R32(G_RA(op)), R32(G_RS(op)), IMM(G_UIMM(op)));					// or      ra,rs,uimm
			return TRUE;

		case 0x19:	/* ORIS */
			UML_OR(block, R32(G_RA(op)), R32(G_RS(op)), IMM(G_UIMM(op) << 16));				// or      ra,rs,uimm << 16
			return TRUE;

		case 0x1a:	/* XORI */
			UML_XOR(block, R32(G_RA(op)), R32(G_RS(op)), IMM(G_UIMM(op)));					// xor     ra,rs,uimm
			return TRUE;

		case 0x1b:	/* XORIS */
			UML_XOR(block, R32(G_RA(op)), R32(G_RS(op)), IMM(G_UIMM(op) << 16));			// xor     ra,rs,uimm << 16
			return TRUE;

		case 0x1c:	/* ANDI. */
			UML_AND(block, R32(G_RA(op)), R32(G_RS(op)), IMM(G_UIMM(op)));					// and     ra,rs,uimm
			generate_compute_flags(block, TRUE, 0, FALSE);									// <update flags>
			return TRUE;

		case 0x1d:	/* ANDIS. */
			UML_AND(block, R32(G_RA(op)), R32(G_RS(op)), IMM(G_UIMM(op) << 16));			// and  ra,rs,uimm << 16
			generate_compute_flags(block, TRUE, 0, FALSE);									// <update flags>
			return TRUE;

		case 0x1f:	/* 0x1f group */
			return generate_instruction_1f(block, compiler, desc);							// <group1f>

		case 0x22:	/* LBZ */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->read8[ppc->impstate->mode & 3]);				// callh   read8
			UML_AND(block, R32(G_RD(op)), IREG(0), IMM(0xff));								// and     rd,i0,0xff
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x28:	/* LHZ */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->read16[ppc->impstate->mode & 3]);				// callh   read16
			UML_AND(block, R32(G_RD(op)), IREG(0), IMM(0xffff));							// and     rd,i0,0xffff
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x2a:	/* LHA */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->read16[ppc->impstate->mode & 3]);				// callh   read16
			UML_SEXT(block, R32(G_RD(op)), IREG(0), WORD);									// sext    rd,i0,word
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x20:	/* LWZ */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->read32[ppc->impstate->mode & 3]);				// callh   read32
			UML_MOV(block, R32(G_RD(op)), IREG(0));											// mov     rd,i0
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x23:	/* LBZU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->read8[ppc->impstate->mode & 3]);				// callh   read8
			UML_AND(block, R32(G_RD(op)), IREG(0), IMM(0xff));								// and     rd,i0,0xff
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x29:	/* LHZU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->read16[ppc->impstate->mode & 3]);				// callh   read16
			UML_AND(block, R32(G_RD(op)), IREG(0), IMM(0xffff));							// and     rd,i0,0xffff
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x2b:	/* LHAU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->read16[ppc->impstate->mode & 3]);				// callh   read16
			UML_SEXT(block, R32(G_RD(op)), IREG(0), WORD);									// sext    rd,i0,word
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x21:	/* LWZU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->read32[ppc->impstate->mode & 3]);				// callh   read32
			UML_MOV(block, R32(G_RD(op)), IREG(0));											// mov     rd,i0
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x26:	/* STB */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_AND(block, IREG(1), R32(G_RS(op)), IMM(0xff));								// and     i1,rs,0xff
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->write8[ppc->impstate->mode & 3]);				// callh   write8
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x2c:	/* STH */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_AND(block, IREG(1), R32(G_RS(op)), IMM(0xffff));							// and     i1,rs,0xffff
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->write16[ppc->impstate->mode & 3]);				// callh   write16
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x24:	/* STW */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_MOV(block, IREG(1), R32(G_RS(op)));											// mov     i1,rs
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x27:	/* STBU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_AND(block, IREG(1), R32(G_RS(op)), IMM(0xff));								// and     i1,rs,0xff
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->write8[ppc->impstate->mode & 3]);				// callh   write8
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x2d:	/* STHU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_AND(block, IREG(1), R32(G_RS(op)), IMM(0xffff));							// and     i1,rs,0xffff
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->write16[ppc->impstate->mode & 3]);				// callh   write16
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x25:	/* STWU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_MOV(block, IREG(1), R32(G_RS(op)));											// mov     i1,rs
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x2e:	/* LMW */
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			for (regnum = G_RD(op); regnum < 32; regnum++)
			{
				UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op) + 4 * (regnum - G_RD(op))));
																							// add     i0,ra,simm + 4*(regnum-rd)
				UML_CALLH(block, ppc->impstate->read32align[ppc->impstate->mode & 3]);		// callh   read32align
				UML_MOV(block, R32(regnum), IREG(0));										// mov     regnum,i0
			}
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x2f:	/* STMW */
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			for (regnum = G_RS(op); regnum < 32; regnum++)
			{
				UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op) + 4 * (regnum - G_RS(op))));
																							// add     i0,ra,simm + 4*(regnum-rs)
				UML_MOV(block, IREG(1), R32(regnum));										// mov     i1,regnum
				UML_CALLH(block, ppc->impstate->write32align[ppc->impstate->mode & 3]);		// callh   write32align
			}
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x30:	/* LFS */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->read32[ppc->impstate->mode & 3]);				// callh   read32
			UML_MOV(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0));						// mov     [tempdata],i0
			UML_FDFRFLT(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.w.l), DWORD);	// fdfrflt fd,[tempdata],dword
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x32:	/* LFD */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->read64[ppc->impstate->mode & 3]);				// callh   read64
			UML_DMOV(block, MEM(&ppc->impstate->tempdata.d), IREG(0));						// dmov    [tempdata],i0
			UML_FDMOV(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.d));				// fdmov   fd,[tempdata]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x31:	/* LFSU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->read32[ppc->impstate->mode & 3]);				// callh   read32
			UML_MOV(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0));						// mov     [tempdata],i0
			UML_FDFRFLT(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.w.l), DWORD);	// fdfrflt fd,[tempdata],dword
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x33:	/* LFDU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->read64[ppc->impstate->mode & 3]);				// callh   read64
			UML_DMOV(block, MEM(&ppc->impstate->tempdata.d), IREG(0));						// dmov    [tempdata],i0
			UML_FDMOV(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.d));				// fdmov   fd,[tempdata]
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x34:	/* STFS */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_FSFRFLT(block, MEM(&ppc->impstate->tempdata.w.l), F64(G_RS(op)), QWORD);	// fsfrflt [tempdata],rs,qword
			UML_MOV(block, IREG(1), MEM(&ppc->impstate->tempdata.w.l));						// mov     i1,[tempdata]
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x36:	/* STFD */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), IMM((INT16)G_SIMM(op)));				// add     i0,ra,simm
			UML_FDMOV(block, MEM(&ppc->impstate->tempdata.d), F64(G_RS(op)));				// fdmov   [tempdata],rs
			UML_DMOV(block, IREG(1), MEM(&ppc->impstate->tempdata.d));						// dmov    i1,[tempdata]
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMM(op));									// mapvar  dsisr,DSISR_IMM(op)
			UML_CALLH(block, ppc->impstate->write64[ppc->impstate->mode & 3]);				// callh   write64
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x35:	/* STFSU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_FSFRFLT(block, MEM(&ppc->impstate->tempdata.w.l), F64(G_RS(op)), QWORD);	// fsfrflt [tempdata],rs,qword
			UML_MOV(block, IREG(1), MEM(&ppc->impstate->tempdata.w.l));						// mov     i1,[tempdata]
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x37:	/* STFDU */
			UML_ADD(block, IREG(0), R32(G_RA(op)), IMM((INT16)G_SIMM(op)));					// add     i0,ra,simm
			UML_FDMOV(block, MEM(&ppc->impstate->tempdata.d), F64(G_RS(op)));				// fdmov   [tempdata],rs
			UML_DMOV(block, IREG(1), MEM(&ppc->impstate->tempdata.d));						// dmov    i1,[tempdata]
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IMMU(op));								// mapvar  dsisr,DSISR_IMMU(op)
			UML_CALLH(block, ppc->impstate->write64[ppc->impstate->mode & 3]);				// callh   write64
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x3b:	/* 0x3b group */
			return generate_instruction_3b(block, compiler, desc);							// <group3b>

		case 0x3f:	/* 0x3f group */
			return generate_instruction_3f(block, compiler, desc);							// <group3f>
	}

	return FALSE;
}


/*-------------------------------------------------
    generate_instruction_13 - compile opcodes in
    the 0x13 group
-------------------------------------------------*/

static int generate_instruction_13(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT32 opswitch = (op >> 1) & 0x3ff;

	switch (opswitch)
	{
		case 0x010:	/* BCLRx */
			generate_branch_bo(block, compiler, desc, G_BO(op), G_BI(op), SPR_LR, op & M_LK);// <branch conditional>
			return TRUE;

		case 0x210:	/* BCCTRx */
			generate_branch_bo(block, compiler, desc, G_BO(op), G_BI(op), SPR_CTR, op & M_LK);
																							// <branch conditional>
			return TRUE;

		case 0x000:	/* MCRF */
			UML_ROLINS(block, CR32, CR32, IMM(((G_CRFS(op) - G_CRFD(op)) & 7) * 4), IMM(CRMASK(G_CRFD(op))));
																							// rolins  cr,cr,4*(crfs-crfd),crmask[crfd]
			return TRUE;

		case 0x101:	/* CRAND */
			UML_SHL(block, IREG(0), CR32, IMM(G_CRBA(op)));									// shl     i0,cr,crba(op)
			UML_SHL(block, IREG(1), CR32, IMM(G_CRBB(op)));									// shl     i1,cr,crbb(op)
			UML_AND(block, IREG(0), IREG(0), IREG(1));										// and     i0,i1
			UML_ROLINS(block, CR32, IREG(0), IMM(32 - G_CRBD(op)), IMM(0x80000000 >> G_CRBD(op)));
																							// rolins  cr,i0,32-crbd,1<<crbd
			return TRUE;

		case 0x081:	/* CRANDC */
			UML_SHL(block, IREG(0), CR32, IMM(G_CRBA(op)));									// shl     i0,cr,crba(op)
			UML_SHL(block, IREG(1), CR32, IMM(G_CRBB(op)));									// shl     i1,cr,crbb(op)
			UML_XOR(block, IREG(1), IREG(1), IMM(~0));										// xor     i1,~0
			UML_AND(block, IREG(0), IREG(0), IREG(1));										// and     i0,i1
			UML_ROLINS(block, CR32, IREG(0), IMM(32 - G_CRBD(op)), IMM(0x80000000 >> G_CRBD(op)));
																							// rolins  cr,i0,32-crbd,1<<crbd
			return TRUE;

		case 0x0e1:	/* CRNAND */
			UML_SHL(block, IREG(0), CR32, IMM(G_CRBA(op)));									// shl     i0,cr,crba(op)
			UML_SHL(block, IREG(1), CR32, IMM(G_CRBB(op)));									// shl     i1,cr,crbb(op)
			UML_AND(block, IREG(0), IREG(0), IREG(1));										// and     i0,i1
			UML_XOR(block, IREG(0), IREG(0), IMM(~0));										// xor     i0,~0
			UML_ROLINS(block, CR32, IREG(0), IMM(32 - G_CRBD(op)), IMM(0x80000000 >> G_CRBD(op)));
																							// rolins  cr,i0,32-crbd,1<<crbd
			return TRUE;

		case 0x1c1:	/* CROR */
			UML_SHL(block, IREG(0), CR32, IMM(G_CRBA(op)));									// shl     i0,cr,crba(op)
			UML_SHL(block, IREG(1), CR32, IMM(G_CRBB(op)));									// shl     i1,cr,crbb(op)
			UML_OR(block, IREG(0), IREG(0), IREG(1));										// or      i0,i1
			UML_ROLINS(block, CR32, IREG(0), IMM(32 - G_CRBD(op)), IMM(0x80000000 >> G_CRBD(op)));
																							// rolins  cr,i0,32-crbd,1<<crbd
			return TRUE;

		case 0x1a1:	/* CRORC */
			UML_SHL(block, IREG(0), CR32, IMM(G_CRBA(op)));									// shl     i0,cr,crba(op)
			UML_SHL(block, IREG(1), CR32, IMM(G_CRBB(op)));									// shl     i1,cr,crbb(op)
			UML_XOR(block, IREG(1), IREG(1), IMM(~0));										// xor     i1,~0
			UML_OR(block, IREG(0), IREG(0), IREG(1));										// or      i0,i1
			UML_ROLINS(block, CR32, IREG(0), IMM(32 - G_CRBD(op)), IMM(0x80000000 >> G_CRBD(op)));
																							// rolins  cr,i0,32-crbd,1<<crbd
			return TRUE;

		case 0x021:	/* CRNOR */
			UML_SHL(block, IREG(0), CR32, IMM(G_CRBA(op)));									// shl     i0,cr,crba(op)
			UML_SHL(block, IREG(1), CR32, IMM(G_CRBB(op)));									// shl     i1,cr,crbb(op)
			UML_OR(block, IREG(0), IREG(0), IREG(1));										// or      i0,i1
			UML_XOR(block, IREG(0), IREG(0), IMM(~0));										// xor     i0,~0
			UML_ROLINS(block, CR32, IREG(0), IMM(32 - G_CRBD(op)), IMM(0x80000000 >> G_CRBD(op)));
																							// rolins  cr,i0,32-crbd,1<<crbd
			return TRUE;

		case 0x0c1:	/* CRXOR */
			UML_SHL(block, IREG(0), CR32, IMM(G_CRBA(op)));									// shl     i0,cr,crba(op)
			UML_SHL(block, IREG(1), CR32, IMM(G_CRBB(op)));									// shl     i1,cr,crbb(op)
			UML_XOR(block, IREG(0), IREG(0), IREG(1));										// xor     i0,i1
			UML_ROLINS(block, CR32, IREG(0), IMM(32 - G_CRBD(op)), IMM(0x80000000 >> G_CRBD(op)));
																							// rolins  cr,i0,32-crbd,1<<crbd
			return TRUE;

		case 0x121:	/* CREQV */
			UML_SHL(block, IREG(0), CR32, IMM(G_CRBA(op)));									// shl     i0,cr,crba(op)
			UML_SHL(block, IREG(1), CR32, IMM(G_CRBB(op)));									// shl     i1,cr,crbb(op)
			UML_XOR(block, IREG(0), IREG(0), IREG(1));										// xor     i0,i1
			UML_XOR(block, IREG(0), IREG(0), IMM(~0));										// xor     i0,~0
			UML_ROLINS(block, CR32, IREG(0), IMM(32 - G_CRBD(op)), IMM(0x80000000 >> G_CRBD(op)));
																							// rolins  cr,i0,32-crbd,1<<crbd
			return TRUE;

		case 0x032:	/* RFI */
			if (ppc->cap & PPCCAP_OEA)
			{
				if (!(ppc->cap & PPCCAP_603_MMU))
					UML_ROLINS(block, MSR32, SPR32(SPROEA_SRR1), IMM(0), IMM(0x87c0ffff));	// rolins  [msr],[srr1],0,0x87c0ffff
				else
				{
					UML_MOV(block, IREG(0), MSR32);											// mov     i0,[msr]
					UML_ROLINS(block, MSR32, SPR32(SPROEA_SRR1), IMM(0), IMM(0x87c0ffff |  MSR603_TGPR));
																							// rolins  [msr],[srr1],0,0x87c0ffff | MSR603_TGPR
					UML_XOR(block, IREG(0), IREG(0), MSR32);								// xor     i0,i0,[msr]
					UML_TEST(block, IREG(0), IMM(MSR603_TGPR));								// test    i0,tgpr
					UML_CALLHc(block, IF_NZ, ppc->impstate->swap_tgpr);						// callh   swap_tgpr,nz
				}
			}
			else if (ppc->cap & PPCCAP_4XX)
				UML_MOV(block, MSR32, SPR32(SPR4XX_SRR1));									// mov     [msr],[srr1]
			generate_update_mode(block);													// <update mode>
			compiler->checkints = TRUE;
			generate_update_cycles(block, compiler, SPR32(SPROEA_SRR0), TRUE);				// <subtract cycles>
			UML_HASHJMP(block, MEM(&ppc->impstate->mode), SPR32(SPROEA_SRR0), ppc->impstate->nocode);
																							// hashjmp mode,[srr0],nocode
			return TRUE;

		case 0x033:	/* RFCI */
			assert(ppc->cap & PPCCAP_4XX);
			UML_MOV(block, MSR32, SPR32(SPR4XX_SRR3));										// mov     [msr],[srr3]
			generate_update_mode(block);													// <update mode>
			compiler->checkints = TRUE;
			generate_update_cycles(block, compiler, SPR32(SPR4XX_SRR2), TRUE);				// <subtract cycles>
			UML_HASHJMP(block, MEM(&ppc->impstate->mode), SPR32(SPR4XX_SRR2), ppc->impstate->nocode);
																							// hashjmp mode,[srr2],nocode
			return TRUE;

		case 0x096:	/* ISYNC */
			/* effective no-op */
			return TRUE;
	}

	return FALSE;
}


/*-------------------------------------------------
    generate_instruction_1f - compile opcodes in
    the 0x1f group
-------------------------------------------------*/

static int generate_instruction_1f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT32 opswitch = (op >> 1) & 0x3ff;
	int item;

	switch (opswitch)
	{
		case 0x009:	/* MULHDUx - 64-bit only */
		case 0x015:	/* LDX - 64-bit only */
		case 0x01b:	/* SLDx - 64-bit only */
		case 0x035:	/* LDUX - 64-bit only */
		case 0x03a:	/* CNTLZDx - 64-bit only */
		case 0x044:	/* TD - 64-bit only */
		case 0x049:	/* MULHDx - 64-bit only */
		case 0x054:	/* LDARX - 64-bit only */
		case 0x095:	/* STDX - 64-bit only */
		case 0x0b5:	/* STDUX - 64-bit only */
		case 0x0d6:	/* STDCX. - 64-bit only */
		case 0x0e9:	/* MULLD - 64-bit only */
		case 0x2e9:	/* MULLDO - 64-bit only */
		case 0x155:	/* LWAX - 64-bit only */
		case 0x175:	/* LWAUX - 64-bit only */
		case 0x33a: /* SRADIx - 64-bit only */
		case 0x33b: /* SRADIx - 64-bit only */
		case 0x1b2:	/* SLBIE - 64-bit only */
		case 0x1c9:	/* DIVDUx - 64-bit only */
		case 0x3c9:	/* DIVDUOx - 64-bit only */
		case 0x1e9:	/* DIVDx - 64-bit only */
		case 0x3e9:	/* DIVDOx - 64-bit only */
		case 0x1f2:	/* SLBIA - 64-bit only */
		case 0x21b:	/* SRDx - 64-bit only */
		case 0x31a:	/* SRADx - 64-bit only */
		case 0x3da:	/* EXTSW - 64-bit only */
			return FALSE;

		case 0x004:	/* TW */
			UML_CMP(block, R32(G_RA(op)), R32(G_RB(op)));									// cmp     ra,rb
			if (G_TO(op) & 0x10)
				UML_EXHc(block, IF_L, ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,l
			if (G_TO(op) & 0x08)
				UML_EXHc(block, IF_G, ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,g
			if (G_TO(op) & 0x04)
				UML_EXHc(block, IF_E, ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,e
			if (G_TO(op) & 0x02)
				UML_EXHc(block, IF_B,  ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,b
			if (G_TO(op) & 0x01)
				UML_EXHc(block, IF_A,  ppc->impstate->exception[EXCEPTION_PROGRAM], IMM(0x20000));// exh program,0x20000,a
			return TRUE;

		case 0x10a:	/* ADDx */
		case 0x30a:	/* ADDOx */
			UML_ADD(block, R32(G_RD(op)), R32(G_RA(op)), R32(G_RB(op)));					// add     rd,ra,rb
			generate_compute_flags(block, op & M_RC, ((op & M_OE) ? XER_OV : 0), FALSE);	// <update flags>
			return TRUE;

		case 0x00a:	/* ADDCx */
		case 0x20a:	/* ADDCOx */
			UML_ADD(block, R32(G_RD(op)), R32(G_RA(op)), R32(G_RB(op)));					// add     rd,ra,rb
			generate_compute_flags(block, op & M_RC, XER_CA | ((op & M_OE) ? XER_OV : 0), FALSE);// <update flags>
			return TRUE;

		case 0x08a:	/* ADDEx */
		case 0x28a:	/* ADDEOx */
			UML_CARRY(block, SPR32(SPR_XER), IMM(29));										// carry   [xer],XER_CA
			UML_ADDC(block, R32(G_RD(op)), R32(G_RA(op)), R32(G_RB(op)));					// addc    rd,ra,rb
			generate_compute_flags(block, op & M_RC, XER_CA | ((op & M_OE) ? XER_OV : 0), FALSE);// <update flags>
			return TRUE;

		case 0x0ca:	/* ADDZEx */
		case 0x2ca:	/* ADDZEOx */
			UML_CARRY(block, SPR32(SPR_XER), IMM(29));										// carry   [xer],XER_CA
			UML_ADDC(block, R32(G_RD(op)), R32(G_RA(op)), IMM(0));							// addc    rd,ra,0
			generate_compute_flags(block, op & M_RC, XER_CA | ((op & M_OE) ? XER_OV : 0), FALSE);// <update flags>
			return TRUE;

		case 0x0ea:	/* ADDMEx */
		case 0x2ea:	/* ADDMEOx */
			UML_CARRY(block, SPR32(SPR_XER), IMM(29));										// carry   [xer],XER_CA
			UML_ADDC(block, R32(G_RD(op)), R32(G_RA(op)), IMM(-1));							// addc    rd,ra,-1
			generate_compute_flags(block, op & M_RC, XER_CA | ((op & M_OE) ? XER_OV : 0), FALSE);// <update flags>
			return TRUE;

		case 0x028:	/* SUBFx */
		case 0x228:	/* SUBFOx */
			UML_SUB(block, R32(G_RD(op)), R32(G_RB(op)), R32(G_RA(op)));					// sub     rd,rb,ra
			generate_compute_flags(block, op & M_RC, (op & M_OE) ? XER_OV : 0, TRUE);		// <update flags>
			return TRUE;

		case 0x008:	/* SUBFCx */
		case 0x208:	/* SUBFCOx */
			UML_SUB(block, R32(G_RD(op)), R32(G_RB(op)), R32(G_RA(op)));					// sub     rd,rb,ra
			generate_compute_flags(block, op & M_RC, XER_CA | ((op & M_OE) ? XER_OV : 0), TRUE);// <update flags>
			return TRUE;

		case 0x088:	/* SUBFEx */
		case 0x288:	/* SUBFEOx */
			UML_XOR(block, IREG(0), SPR32(SPR_XER), IMM(XER_CA));							// xor     i0,[xer],XER_CA
			UML_CARRY(block, IREG(0), IMM(29));												// carry   i0,XER_CA
			UML_SUBB(block, R32(G_RD(op)), R32(G_RB(op)), R32(G_RA(op)));					// subc    rd,rb,ra
			generate_compute_flags(block, op & M_RC, XER_CA | ((op & M_OE) ? XER_OV : 0), TRUE);// <update flags>
			return TRUE;

		case 0x0c8:	/* SUBFZEx */
		case 0x2c8:	/* SUBFZEOx */
			UML_XOR(block, IREG(0), SPR32(SPR_XER), IMM(XER_CA));							// xor     i0,[xer],XER_CA
			UML_CARRY(block, IREG(0), IMM(29));												// carry   i0,XER_CA
			UML_SUBB(block, R32(G_RD(op)), IMM(0), R32(G_RA(op)));							// subc    rd,0,ra
			generate_compute_flags(block, op & M_RC, XER_CA | ((op & M_OE) ? XER_OV : 0), TRUE);// <update flags>
			return TRUE;

		case 0x0e8:	/* SUBFMEx */
		case 0x2e8:	/* SUBFMEOx */
			UML_XOR(block, IREG(0), SPR32(SPR_XER), IMM(XER_CA));							// xor     i0,[xer],XER_CA
			UML_CARRY(block, IREG(0), IMM(29));												// carry   i0,XER_CA
			UML_SUBB(block, R32(G_RD(op)), IMM(-1), R32(G_RA(op)));							// subc    rd,-1,ra
			generate_compute_flags(block, op & M_RC, XER_CA | ((op & M_OE) ? XER_OV : 0), TRUE);// <update flags>
			return TRUE;

		case 0x068:	/* NEGx */
		case 0x268:	/* NEGOx */
			UML_SUB(block, R32(G_RD(op)), IMM(0), R32(G_RA(op)));							// sub     rd,0,ra
			generate_compute_flags(block, op & M_RC, (op & M_OE) ? XER_OV : 0, TRUE);		// <update flags>
			return TRUE;

		case 0x000:	/* CMP */
			UML_CMP(block, R32(G_RA(op)), R32(G_RB(op)));									// cmp     ra,rb
			UML_GETFLGS(block, IREG(0), DRCUML_FLAG_Z | DRCUML_FLAG_C | DRCUML_FLAG_S);		// getflgs i0,zcs
			UML_LOAD(block, IREG(0), ppc->impstate->cmp_cr_table, IREG(0), DWORD);			// load    i0,cmp_cr_table,i0,dword
			UML_ROLINS(block, IREG(0), SPR32(SPR_XER), IMM(32 - (3 + 4 * G_CRFD(op))), IMM(CRMASK(G_CRFD(op)) & 0x11111111));
																							// rolins  i0,[xer],32-(3+4*crfd),0x11111111 & crmask[cr]
			UML_ROLINS(block, CR32, IREG(0), IMM(0), IMM(CRMASK(G_CRFD(op))));				// rolins  cr,i0,0,crmask[cr]
			return TRUE;

		case 0x020:	/* CMPL */
			UML_CMP(block, R32(G_RA(op)), R32(G_RB(op)));									// cmp     ra,rb
			UML_GETFLGS(block, IREG(0), DRCUML_FLAG_Z | DRCUML_FLAG_C);						// getflgs i0,zc
			UML_LOAD(block, IREG(0), ppc->impstate->cmpl_cr_table, IREG(0), DWORD);			// load    i0,cmpl_cr_table,i0,dword
			UML_ROLINS(block, IREG(0), SPR32(SPR_XER), IMM(32 - (3 + 4 * G_CRFD(op))), IMM(CRMASK(G_CRFD(op)) & 0x11111111));
																							// rolins  i0,[xer],32-(3+4*crfd),0x11111111 & crmask[cr]
			UML_ROLINS(block, CR32, IREG(0), IMM(0), IMM(CRMASK(G_CRFD(op))));				// rolins  cr,i0,0,crmask[cr]
			return TRUE;

		case 0x00b:	/* MULHWUx */
			UML_MULU(block, IREG(0), R32(G_RD(op)), R32(G_RA(op)), R32(G_RB(op)));			// mulu    i0,rd,ra,rb
			if (op & M_RC)
			{
				UML_TEST(block, R32(G_RD(op)), IMM(~0));									// test    rd,~0
				generate_compute_flags(block, op & M_RC, 0, FALSE);							// <update flags>
			}
			return TRUE;

		case 0x04b:	/* MULHWx */
			UML_MULS(block, IREG(0), R32(G_RD(op)), R32(G_RA(op)), R32(G_RB(op)));			// muls    i0,rd,ra,rb
			if (op & M_RC)
			{
				UML_TEST(block, R32(G_RD(op)), IMM(~0));									// test    rd,~0
				generate_compute_flags(block, op & M_RC, 0, FALSE);							// <update flags>
			}
			return TRUE;

		case 0x0eb:	/* MULLWx */
		case 0x2eb:	/* MULLWOx */
			UML_MULS(block, R32(G_RD(op)), R32(G_RD(op)), R32(G_RA(op)), R32(G_RB(op)));	// muls    rd,rd,ra,rb
			generate_compute_flags(block, op & M_RC, ((op & M_OE) ? XER_OV : 0), FALSE);	// <update flags>
			return TRUE;

		case 0x1cb:	/* DIVWUx */
		case 0x3cb:	/* DIVWUOx */
			UML_DIVU(block, R32(G_RD(op)), R32(G_RD(op)), R32(G_RA(op)), R32(G_RB(op)));	// divu    rd,rd,ra,rb
			generate_compute_flags(block, op & M_RC, ((op & M_OE) ? XER_OV : 0), FALSE);	// <update flags>
			return TRUE;

		case 0x1eb:	/* DIVWx */
		case 0x3eb:	/* DIVWOx */
			UML_DIVS(block, R32(G_RD(op)), R32(G_RD(op)), R32(G_RA(op)), R32(G_RB(op)));	// divs    rd,rd,ra,rb
			generate_compute_flags(block, op & M_RC, ((op & M_OE) ? XER_OV : 0), FALSE);	// <update flags>
			return TRUE;

		case 0x01c:	/* ANDx */
			UML_AND(block, R32(G_RA(op)), R32(G_RS(op)), R32(G_RB(op)));					// and     ra,rs,rb
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x03c:	/* ANDCx */
			UML_XOR(block, IREG(0), R32(G_RB(op)), IMM(~0));								// xor     i0,rb,~0
			UML_AND(block, R32(G_RA(op)), R32(G_RS(op)), IREG(0));							// and     ra,rs,i0
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x1dc:	/* NANDx */
			UML_AND(block, IREG(0), R32(G_RS(op)), R32(G_RB(op)));							// and     i0,rs,rb
			UML_XOR(block, R32(G_RA(op)), IREG(0), IMM(~0));								// xor     ra,i0,~0
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x1bc:	/* ORx */
			UML_OR(block, R32(G_RA(op)), R32(G_RS(op)), R32(G_RB(op)));						// or      ra,rs,rb
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x19c:	/* ORCx */
			UML_XOR(block, IREG(0), R32(G_RB(op)), IMM(~0));								// xor     i0,rb,~0
			UML_OR(block, R32(G_RA(op)), R32(G_RS(op)), IREG(0));							// or      ra,rs,i0
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x07c:	/* NORx */
			UML_OR(block, IREG(0), R32(G_RS(op)), R32(G_RB(op)));							// or      i0,rs,rb
			UML_XOR(block, R32(G_RA(op)), IREG(0), IMM(~0));								// xor     ra,i0,~0
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x13c:	/* XORx */
			UML_XOR(block, R32(G_RA(op)), R32(G_RS(op)), R32(G_RB(op)));					// xor     ra,rs,rb
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x11c:	/* EQVx */
			UML_XOR(block, IREG(0), R32(G_RS(op)), R32(G_RB(op)));							// xor     i0,rs,rb
			UML_XOR(block, R32(G_RA(op)), IREG(0), IMM(~0));								// xor     ra,i0,~0
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x018:	/* SLWx */
			UML_SHL(block, R32(G_RA(op)), R32(G_RS(op)), R32(G_RB(op)));					// shl     ra,rs,rb
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x218:	/* SRWx */
			UML_MOV(block, IREG(0), R32(G_RS(op)));											// mov     i0,rs
			UML_TEST(block, R32(G_RB(op)), IMM(0x20));										// test    rb,0x20
			UML_MOVc(block, IF_NZ, IREG(0), IMM(0));										// mov     i0,0,nz
			UML_SHR(block, R32(G_RA(op)), IREG(0), R32(G_RB(op)));							// shr     ra,i0,rb
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x318:	/* SRAWx */
			UML_MOV(block, IREG(2), R32(G_RB(op)));											// mov     i2,rb
			UML_TEST(block, IREG(2), IMM(0x20));											// test    i2,0x20
			UML_MOVc(block, IF_NZ, IREG(2), IMM(31));										// mov     i2,31,nz
			UML_SHL(block, IREG(1), IMM(0xffffffff), IREG(2));								// shl     i1,0xffffffff,i2
			UML_XOR(block, IREG(1), IREG(1), IMM(~0));										// xor     i1,i1,~0
			UML_AND(block, IREG(0), R32(G_RS(op)), IREG(1));								// and     i0,rs,i1
			UML_SAR(block, IREG(1), R32(G_RS(op)), IMM(31));								// sar     i1,rs,31
			UML_TEST(block, IREG(0), IREG(1));												// test    i0,i1
			UML_SETc(block, IF_NZ, IREG(0));												// set     i0,nz
			UML_ROLINS(block, SPR32(SPR_XER), IREG(0), IMM(29), IMM(XER_CA));				// rolins  [xer],i0,29,XER_CA
			UML_SAR(block, R32(G_RA(op)), R32(G_RS(op)), IREG(2));							// sar     ra,rs,i2
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x338:	/* SRAWIx */
			UML_AND(block, IREG(0), R32(G_RS(op)), IMM(~(0xffffffff << (G_SH(op) & 31))));	// and     i0,rs,~(0xffffffff << (sh & 31))
			UML_SAR(block, IREG(1), R32(G_RS(op)), IMM(31));								// sar     i1,rs,31
			UML_TEST(block, IREG(0), IREG(1));												// test    i0,i1
			UML_SETc(block, IF_NZ, IREG(0));												// set     i0,nz
			UML_ROLINS(block, SPR32(SPR_XER), IREG(0), IMM(29), IMM(XER_CA));				// rolins  [xer],i0,29,XER_CA
			UML_SAR(block, R32(G_RA(op)), R32(G_RS(op)), IMM(G_SH(op)));					// sar     ra,rs,sh
			generate_compute_flags(block, op & M_RC, 0, FALSE);								// <update flags>
			return TRUE;

		case 0x01a:	/* CNTLZWx */
			UML_LZCNT(block, R32(G_RA(op)), R32(G_RS(op)));									// lzcnt   ra,rs
			if (op & M_RC)
				generate_compute_flags(block, op & M_RC, 0, FALSE);							// <update flags>
			return TRUE;

		case 0x3ba:	/* EXTSBx */
			UML_SEXT(block, R32(G_RA(op)), R32(G_RS(op)), BYTE);							// sext    ra,rs,byte
			if (op & M_RC)
				generate_compute_flags(block, op & M_RC, 0, FALSE);							// <update flags>
			return TRUE;

		case 0x39a:	/* EXTSHx */
			UML_SEXT(block, R32(G_RA(op)), R32(G_RS(op)), WORD);							// sext    ra,rs,word
			if (op & M_RC)
				generate_compute_flags(block, op & M_RC, 0, FALSE);							// <update flags>
			return TRUE;

		case 0x057:	/* LBZX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read8[ppc->impstate->mode & 3]);				// callh   read8
			UML_AND(block, R32(G_RD(op)), IREG(0), IMM(0xff));								// and     rd,i0,0xff
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x117:	/* LHZX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read16[ppc->impstate->mode & 3]);				// callh   read16
			UML_AND(block, R32(G_RD(op)), IREG(0), IMM(0xffff));							// and     rd,i0,0xffff
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x157:	/* LHAX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read16[ppc->impstate->mode & 3]);				// callh   read16
			UML_SEXT(block, R32(G_RD(op)), IREG(0), WORD);									// sext    rd,i0,word
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x017:	/* LWZX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read32[ppc->impstate->mode & 3]);				// callh   read32
			UML_MOV(block, R32(G_RD(op)), IREG(0));											// mov     rd,i0
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x217:	/* LFSX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read32[ppc->impstate->mode & 3]);				// callh   read32
			UML_MOV(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0));						// mov     [tempdata],i0
			UML_FDFRFLT(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.w.l), DWORD);	// fdfrflt fd,[tempdata],dword
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x257:	/* LFDX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read64[ppc->impstate->mode & 3]);				// callh   read64
			UML_DMOV(block, MEM(&ppc->impstate->tempdata.d), IREG(0));						// dmov    [tempdata],i0
			UML_FDMOV(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.d));				// fdmov   fd,[tempdata]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x316:	/* LHBRX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read16[ppc->impstate->mode & 3]);				// callh   read16
			UML_BSWAP(block, IREG(0), IREG(0));												// bswap   i0,i0
			UML_SHR(block, R32(G_RD(op)), IREG(0), IMM(16));								// shr     rd,i0,16
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x216:	/* LWBRX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read32align[ppc->impstate->mode & 3]);			// callh   read32align
			UML_BSWAP(block, R32(G_RD(op)), IREG(0));										// bswap   rd,i0
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x077:	/* LBZUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDXU(op));								// mapvar  dsisr,DSISR_IDXU(op)
			UML_CALLH(block, ppc->impstate->read8[ppc->impstate->mode & 3]);				// callh   read8
			UML_AND(block, R32(G_RD(op)), IREG(0), IMM(0xff));								// and     rd,i0,0xff
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x137:	/* LHZUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDXU(op));								// mapvar  dsisr,DSISR_IDXU(op)
			UML_CALLH(block, ppc->impstate->read16[ppc->impstate->mode & 3]);				// callh   read16
			UML_AND(block, R32(G_RD(op)), IREG(0), IMM(0xffff));							// and     rd,i0,0xffff
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x177:	/* LHAUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDXU(op));								// mapvar  dsisr,DSISR_IDXU(op)
			UML_CALLH(block, ppc->impstate->read16[ppc->impstate->mode & 3]);				// callh   read16
			UML_SEXT(block, R32(G_RD(op)), IREG(0), WORD);									// sext    rd,i0,word
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x037:	/* LWZUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDXU(op));								// mapvar  dsisr,DSISR_IDXU(op)
			UML_CALLH(block, ppc->impstate->read32[ppc->impstate->mode & 3]);				// callh   read32
			UML_MOV(block, R32(G_RD(op)), IREG(0));											// mov     rd,i0
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x237:	/* LFSUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read32[ppc->impstate->mode & 3]);				// callh   read32
			UML_MOV(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0));						// mov     [tempdata],i0
			UML_FDFRFLT(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.w.l), DWORD);	// fdfrflt fd,[tempdata],dword
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x277:	/* LFDUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read64[ppc->impstate->mode & 3]);				// callh   read64
			UML_DMOV(block, MEM(&ppc->impstate->tempdata.d), IREG(0));						// dmov    [tempdata],i0
			UML_FDMOV(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.d));				// fdmov   fd,[tempdata]
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x014:	/* LWARX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->read32align[ppc->impstate->mode & 3]);			// callh   read32align
			UML_MOV(block, R32(G_RD(op)), IREG(0));											// mov     rd,i0
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x255:	/* LSWI */
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), R32Z(G_RA(op)));				// mov     [updateaddr],ra
			UML_MOV(block, MEM(&ppc->impstate->swcount), IMM(((G_NB(op) - 1) & 0x1f) + 1));	// mov     [swcount],G_NB
			UML_CALLH(block, ppc->impstate->lsw[ppc->impstate->mode & 3][G_RD(op)]);		// call    lsw[rd]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x215:	/* LSWX */
			UML_ADD(block, MEM(&ppc->impstate->updateaddr), R32Z(G_RA(op)), R32(G_RB(op)));	// add     [updateaddr],ra,rb
			UML_AND(block, MEM(&ppc->impstate->swcount), SPR32(SPR_XER), IMM(0x7f));		// and     [swcount],[xer],0x7f
			UML_CALLHc(block, IF_NZ, ppc->impstate->lsw[ppc->impstate->mode & 3][G_RD(op)]);// call    lsw[rd],nz
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x136:	/* ECIWX */
			/* not implemented */
			return FALSE;

		case 0x0d7:	/* STBX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_AND(block, IREG(1), R32(G_RS(op)), IMM(0xff));								// and     i1,rs,0xff
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write8[ppc->impstate->mode & 3]);				// callh   write8
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x197:	/* STHX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_AND(block, IREG(1), R32(G_RS(op)), IMM(0xffff));							// and     i1,rs
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write16[ppc->impstate->mode & 3]);				// callh   write16
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x097:	/* STWX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MOV(block, IREG(1), R32(G_RS(op)));											// mov     i1,rs
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x297:	/* STFSX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_FSFRFLT(block, MEM(&ppc->impstate->tempdata.w.l), F64(G_RS(op)), QWORD);	// fsfrflt [tempdata],rs,qword
			UML_MOV(block, IREG(1), MEM(&ppc->impstate->tempdata.w.l));						// mov     i1,[tempdata]
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x3d7:	/* STFIWX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_FDMOV(block, MEM(&ppc->impstate->tempdata.d), F64(G_RS(op)));				// fdmov   [tempdata],rs
			UML_MOV(block, IREG(1), MEM(&ppc->impstate->tempdata.w.l));						// mov     i1,[tempdata.lo]
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x2d7:	/* STFDX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_FDMOV(block, MEM(&ppc->impstate->tempdata.d), F64(G_RS(op)));				// fdmov   [tempdata],rs
			UML_DMOV(block, IREG(1), MEM(&ppc->impstate->tempdata.d));						// dmov    i1,[tempdata]
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write64[ppc->impstate->mode & 3]);				// callh   write64
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x396:	/* STHBRX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_BSWAP(block, IREG(1), R32(G_RS(op)));										// bswap   i1,rs
			UML_SHR(block, IREG(1), IREG(1), IMM(16));										// shr     i1,i1,16
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write16[ppc->impstate->mode & 3]);				// callh   write16
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x296:	/* STWBRX */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_BSWAP(block, IREG(1), R32(G_RS(op)));										// bswap   i1,rs
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x0f7:	/* STBUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_AND(block, IREG(1), R32(G_RS(op)), IMM(0xff));								// and     i1,rs,0xff
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write8[ppc->impstate->mode & 3]);				// callh   write8
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x1b7:	/* STHUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_AND(block, IREG(1), R32(G_RS(op)), IMM(0xffff));							// and     i1,rs,0xffff
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write16[ppc->impstate->mode & 3]);				// callh   write16
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x0b7:	/* STWUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MOV(block, IREG(1), R32(G_RS(op)));											// mov     i1,rs
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDXU(op));								// mapvar  dsisr,DSISR_IDXU(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x2b7:	/* STFSUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_FSFRFLT(block, MEM(&ppc->impstate->tempdata.w.l), F64(G_RS(op)), QWORD);	// fsfrflt [tempdata],rs,qword
			UML_MOV(block, IREG(1), MEM(&ppc->impstate->tempdata.w.l));						// mov     i1,[tempdata]
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write32[ppc->impstate->mode & 3]);				// callh   write32
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x2f7:	/* STFDUX */
			UML_ADD(block, IREG(0), R32(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_FDMOV(block, MEM(&ppc->impstate->tempdata.d), F64(G_RS(op)));				// fdmov   [tempdata],rs
			UML_DMOV(block, IREG(1), MEM(&ppc->impstate->tempdata.d));						// dmov    i1,[tempdata]
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), IREG(0));						// mov     [updateaddr],i0
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write64[ppc->impstate->mode & 3]);				// callh   write64
			UML_MOV(block, R32(G_RA(op)), MEM(&ppc->impstate->updateaddr));					// mov     ra,[updateaddr]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x096:	/* STWCX. */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_MOV(block, IREG(1), R32(G_RS(op)));											// mov     i1,rs
			UML_MAPVAR(block, MAPVAR_DSISR, DSISR_IDX(op));									// mapvar  dsisr,DSISR_IDX(op)
			UML_CALLH(block, ppc->impstate->write32align[ppc->impstate->mode & 3]);			// callh   write32align
			UML_AND(block, IREG(0), CR32, IMM(~CRMASK(0)));									// and     i0,cr,~crmask(0)
			UML_ROLINS(block, IREG(0), SPR32(SPR_XER), IMM(29), IMM(0x10000000));			// rolins  i0,[xer],29,0x10000000
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			UML_CMP(block, IREG(0), IREG(0));												// cmp     i0,i0
			generate_compute_flags(block, TRUE, 0, FALSE);									// <update flags>
			return TRUE;

		case 0x2d5:	/* STSWI */
			UML_MOV(block, MEM(&ppc->impstate->updateaddr), R32Z(G_RA(op)));				// mov     [updateaddr],ra
			UML_MOV(block, MEM(&ppc->impstate->swcount), IMM(((G_NB(op) - 1) & 0x1f) + 1));	// mov     [swcount],G_NB
			UML_CALLH(block, ppc->impstate->stsw[ppc->impstate->mode & 3][G_RD(op)]);		// call    stsw[rd]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x295:	/* STSWX */
			UML_ADD(block, MEM(&ppc->impstate->updateaddr), R32Z(G_RA(op)), R32(G_RB(op)));	// add     [updateaddr],ra,rb
			UML_AND(block, MEM(&ppc->impstate->swcount), SPR32(SPR_XER), IMM(0x7f));		// and     [swcount],[xer],0x7f
			UML_CALLHc(block, IF_NZ, ppc->impstate->stsw[ppc->impstate->mode & 3][G_RD(op)]);// call   stsw[rd]
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x1b6:	/* ECOWX */
			/* not implemented */
			return FALSE;

		case 0x036:	/* DCBST */
		case 0x056:	/* DCBF */
		case 0x0f6:	/* DCBTST */
		case 0x116:	/* DCBT */
		case 0x3d6:	/* ICBI */
		case 0x256:	/* SYNC */
		case 0x356:	/* EIEIO */
		case 0x1d6:	/* DCBI */
		case 0x236:	/* TLBSYNC */
			/* effective no-ops */
			return TRUE;

		case 0x3f6:	/* DCBZ */
			UML_ADD(block, IREG(0), R32Z(G_RA(op)), R32(G_RB(op)));							// add     i0,ra,rb
			UML_AND(block, MEM(&ppc->impstate->tempaddr), IREG(0), IMM(~(ppc->cache_line_size - 1)));
																							// and     [tempaddr],i0,~(cache_line_size - 1)
			for (item = 0; item < ppc->cache_line_size / 8; item++)
			{
				UML_ADD(block, IREG(0), MEM(&ppc->impstate->tempaddr), IMM(8 * item));		// add     i0,[tempaddr],8*item
				UML_DMOV(block, IREG(1), IMM(0));											// dmov    i1,0
				UML_CALLH(block, ppc->impstate->write64[ppc->impstate->mode & 3]);			// callh   write64
			}
			return TRUE;

		case 0x132:	/* TLBIE */
			UML_MOV(block, MEM(&ppc->param0), R32(G_RB(op)));								// mov     [param0],rb
			UML_CALLC(block, ppccom_execute_tlbie, ppc);									// callc   ppccom_execute_tlbie,ppc
			return TRUE;

		case 0x172:	/* TLBIA */
			UML_CALLC(block, ppccom_execute_tlbia, ppc);									// callc   ppccom_execute_tlbia,ppc
			return TRUE;

		case 0x3d2:	/* TLBLD */
			assert(ppc->cap & PPCCAP_603_MMU);
			UML_MOV(block, MEM(&ppc->param0), R32(G_RB(op)));								// mov     [param0],rb
			UML_MOV(block, MEM(&ppc->param1), IMM(0));										// mov     [param1],0
			UML_CALLC(block, ppccom_execute_tlbl, ppc);										// callc   ppccom_execute_tlbl,ppc
			return TRUE;

		case 0x3f2:	/* TLBLI */
			assert(ppc->cap & PPCCAP_603_MMU);
			UML_MOV(block, MEM(&ppc->param0), R32(G_RB(op)));								// mov     [param0],rb
			UML_MOV(block, MEM(&ppc->param1), IMM(1));										// mov     [param1],1
			UML_CALLC(block, ppccom_execute_tlbl, ppc);										// callc   ppccom_execute_tlbl,ppc
			return TRUE;

		case 0x013:	/* MFCR */
			UML_MOV(block, R32(G_RD(op)), CR32);											// mov     rd,cr
			return TRUE;

		case 0x053:	/* MFMSR */
			UML_MOV(block, R32(G_RD(op)), MSR32);											// mov     rd,msr
			return TRUE;

		case 0x153:	/* MFSPR */
		{
			UINT32 spr = compute_spr(G_SPR(op));
			if (spr == SPR_LR || spr == SPR_CTR || spr == SPR_XER || (spr >= SPROEA_SPRG0 && spr <= SPROEA_SPRG3))
				UML_MOV(block, R32(G_RD(op)), SPR32(spr));									// mov     rd,spr
			else if (spr == SPROEA_PVR)
				UML_MOV(block, R32(G_RD(op)), IMM(ppc->flavor));							// mov     rd,flavor
			else
			{
				generate_update_cycles(block, compiler, IMM(desc->pc), TRUE);				// <update cycles>
				UML_MOV(block, MEM(&ppc->param0), IMM(spr));								// mov     [param0],spr
				UML_CALLC(block, ppccom_execute_mfspr, ppc);								// callc   ppccom_execute_mfspr,ppc
				UML_MOV(block, R32(G_RD(op)), MEM(&ppc->param1));							// mov     rd,[param1]
			}
			return TRUE;
		}

		case 0x253:	/* MFSR */
			UML_MOV(block, R32(G_RD(op)), SR32(G_SR(op)));									// mov     rd,sr
			return TRUE;

		case 0x293:	/* MFSRIN */
			UML_SHR(block, IREG(0), R32(G_RB(op)), IMM(28));								// shr     i0,G_RB,28
			UML_LOAD(block, R32(G_RD(op)), &ppc->sr[0], IREG(0), DWORD);					// load    rd,sr,i0,dword
			return TRUE;

		case 0x173:	/* MFTB */
		{
			UINT32 tbr = compute_spr(G_SPR(op));
			if (tbr != SPRVEA_TBL_R && tbr != SPRVEA_TBU_R)
				return FALSE;
			generate_update_cycles(block, compiler, IMM(desc->pc), TRUE);					// <update cycles>
			UML_MOV(block, MEM(&ppc->param0), IMM(tbr));									// mov     [param0],tbr
			UML_CALLC(block, ppccom_execute_mftb, ppc);										// callc   ppccom_execute_mftb,ppc
			UML_MOV(block, R32(G_RD(op)), MEM(&ppc->param1));								// mov     rd,[param1]
			return TRUE;
		}

		case 0x090:	/* MTCRF */
			UML_ROLINS(block, CR32, R32(G_RS(op)), IMM(0), IMM(compute_crf_mask(G_CRM(op))));// rolins cr,rs,0,G_CRM
			return TRUE;

		case 0x092:	/* MTMSR */
			if (ppc->cap & PPCCAP_603_MMU)
				UML_XOR(block, IREG(0), MSR32, R32(G_RS(op)));								// xor     i0,msr32,rs
			UML_MOV(block, MSR32, R32(G_RS(op)));											// mov     msr,rs
			if (ppc->cap & PPCCAP_603_MMU)
			{
				UML_TEST(block, IREG(0), IMM(MSR603_TGPR));									// test    i0,tgpr
				UML_CALLHc(block, IF_NZ, ppc->impstate->swap_tgpr);							// callh   swap_tgpr,nz
			}
			generate_update_mode(block);													// <update mode>
			return TRUE;

		case 0x1d3:	/* MTSPR */
		{
			UINT32 spr = compute_spr(G_SPR(op));
			if (spr == SPR_LR || spr == SPR_CTR || spr == SPR_XER || (spr >= SPROEA_SPRG0 && spr <= SPROEA_SPRG3))
				UML_MOV(block, SPR32(spr), R32(G_RS(op)));									// mov     spr,rs
			else if (spr == SPROEA_PVR)
				;																			// read only
			else
			{
				generate_update_cycles(block, compiler, IMM(desc->pc), TRUE);				// <update cycles>
				UML_MOV(block, MEM(&ppc->param0), IMM(spr));								// mov     [param0],spr
				UML_MOV(block, MEM(&ppc->param1), R32(G_RS(op)));							// mov     [param1],rs
				UML_CALLC(block, ppccom_execute_mtspr, ppc);								// callc   ppccom_execute_mtspr,ppc
				compiler->checkints = TRUE;
				generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);			// <update cycles>
			}
			return TRUE;
		}

		case 0x0d2:	/* MTSR */
			UML_MOV(block, SR32(G_SR(op)), R32(G_RS(op)));									// mov     sr[G_SR],rs
			UML_CALLC(block, ppccom_tlb_flush, ppc);										// callc   ppccom_tlb_flush,ppc
			return TRUE;

		case 0x0f2:	/* MTSRIN */
			UML_SHR(block, IREG(0), R32(G_RB(op)), IMM(28));								// shr     i0,G_RB,28
			UML_STORE(block, &ppc->sr[0], IREG(0), R32(G_RS(op)), DWORD);					// store   sr,i0,rs,dword
			UML_CALLC(block, ppccom_tlb_flush, ppc);										// callc   ppccom_tlb_flush,ppc
			return TRUE;

		case 0x200:	/* MCRXR */
			UML_ROLINS(block, CR32, SPR32(SPR_XER), IMM(32 - 4 * G_CRFD(op)), IMM(CRMASK(G_CRFD(op))));
																							// rolins  cr,[xer],32-4*crfd,crmask(crfd)
			UML_AND(block, SPR32(SPR_XER), SPR32(SPR_XER), IMM(~0xf0000000));				// and     [xer],[xer],~0xf0000000
			return TRUE;

		case 0x106:	/* ICBT */
		case 0x1c6:	/* DCCCI */
		case 0x3c6:	/* ICCCI */
			assert(ppc->cap & PPCCAP_4XX);
			/* effective no-nop */
			return TRUE;

		case 0x1e6:	/* DCREAD */
		case 0x3e6:	/* ICREAD */
			assert(ppc->cap & PPCCAP_4XX);
			UML_MOV(block, R32(G_RT(op)), IMM(0));											// mov     rt,0
			return TRUE;

		case 0x143:	/* MFDCR */
		{
			UINT32 spr = compute_spr(G_SPR(op));
			assert(ppc->cap & PPCCAP_4XX);
			generate_update_cycles(block, compiler, IMM(desc->pc), TRUE);					// <update cycles>
			UML_MOV(block, MEM(&ppc->param0), IMM(spr));									// mov     [param0],spr
			UML_CALLC(block, ppccom_execute_mfdcr, ppc);									// callc   ppccom_execute_mfdcr,ppc
			UML_MOV(block, R32(G_RD(op)), MEM(&ppc->param1));								// mov     rd,[param1]
			return TRUE;
		}

		case 0x1c3:	/* MTDCR */
		{
			UINT32 spr = compute_spr(G_SPR(op));
			assert(ppc->cap & PPCCAP_4XX);
			generate_update_cycles(block, compiler, IMM(desc->pc), TRUE);					// <update cycles>
			UML_MOV(block, MEM(&ppc->param0), IMM(spr));									// mov     [param0],spr
			UML_MOV(block, MEM(&ppc->param1), R32(G_RS(op)));								// mov     [param1],rs
			UML_CALLC(block, ppccom_execute_mtdcr, ppc);									// callc   ppccom_execute_mtdcr,ppc
			compiler->checkints = TRUE;
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;
		}

		case 0x083:	/* WRTEE */
			assert(ppc->cap & PPCCAP_4XX);
			UML_ROLINS(block, MSR32, R32(G_RS(op)), IMM(0), IMM(MSR_EE));					// rolins  msr,rs,0,MSR_EE
			compiler->checkints = TRUE;
			generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);				// <update cycles>
			return TRUE;

		case 0x0a3:	/* WRTEEI */
			assert(ppc->cap & PPCCAP_4XX);
			if (op & MSR_EE)
			{
				UML_OR(block, MSR32, MSR32, IMM(MSR_EE));									// or      msr,msr,MSR_EE
				compiler->checkints = TRUE;
				generate_update_cycles(block, compiler, IMM(desc->pc + 4), TRUE);			// <update cycles>
			}
			else
				UML_AND(block, MSR32, MSR32, IMM(~MSR_EE));									// and     msr,msr,~MSR_EE
			return TRUE;
	}

	return FALSE;
}


/*-------------------------------------------------
    generate_instruction_3b - compile opcodes in
    the 0x3b group
-------------------------------------------------*/

static int generate_instruction_3b(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT32 opswitch = (op >> 1) & 0x1f;

	switch (opswitch)
	{
		case 0x15:	/* FADDSx */
			UML_FDADD(block, FREG(0), F64(G_RA(op)), F64(G_RB(op)));						// fdadd   f0,ra,rb
			UML_FDRNDS(block, F64(G_RD(op)), FREG(0));										// fdrnds  rd,f0
			return TRUE;

		case 0x14:	/* FSUBSx */
			UML_FDSUB(block, FREG(0), F64(G_RA(op)), F64(G_RB(op)));						// fdsub   f0,ra,rb
			UML_FDRNDS(block, F64(G_RD(op)), FREG(0));										// fdrnds  rd,f0
			return TRUE;

		case 0x19:	/* FMULSx */
			UML_FDMUL(block, FREG(0), F64(G_RA(op)), F64(G_RB(op)));						// fdmul   f0,ra,rb
			UML_FDRNDS(block, F64(G_RD(op)), FREG(0));										// fdrnds  rd,f0
			return TRUE;

		case 0x12:	/* FDIVSx */
			UML_FDDIV(block, FREG(0), F64(G_RA(op)), F64(G_RB(op)));						// fddiv   f0,ra,rb
			UML_FDRNDS(block, F64(G_RD(op)), FREG(0));										// fdrnds  rd,f0
			return TRUE;

		case 0x16:	/* FSQRTSx */
			UML_FDSQRT(block, FREG(0), F64(G_RB(op)));										// fdsqrt  f0,rb
			UML_FDRNDS(block, F64(G_RD(op)), FREG(0));										// fdrnds  rd,f0
			return TRUE;

		case 0x18:	/* FRESx */
			UML_FSFRFLT(block, FREG(0), F64(G_RB(op)), QWORD);								// fsfrlt  f0,rb,qword
			UML_FSRECIP(block, FREG(0), FREG(0));											// fsrecip f0,f0
			UML_FDFRFLT(block, F64(G_RD(op)), FREG(0), DWORD);								// fdfrflt rd,f0,dword
			return TRUE;

		case 0x1d:	/* FMADDSx */
			UML_FDMUL(block, FREG(0), F64(G_RA(op)), F64(G_REGC(op)));						// fdmul   f0,ra,rc
			UML_FDADD(block, FREG(0), FREG(0), F64(G_RB(op)));								// fdadd   f0,f0,rb
			UML_FDRNDS(block, F64(G_RD(op)), FREG(0));										// fdrnds  rd,f0
			return TRUE;

		case 0x1c:	/* FMSUBSx */
			UML_FDMUL(block, FREG(0), F64(G_RA(op)), F64(G_REGC(op)));						// fdmul   f0,ra,rc
			UML_FDSUB(block, FREG(0), FREG(0), F64(G_RB(op)));								// fdsub   f0,f0,rb
			UML_FDRNDS(block, F64(G_RD(op)), FREG(0));										// fdrnds  rd,f0
			return TRUE;

		case 0x1f:	/* FNMADDSx */
			UML_FDMUL(block, FREG(0), F64(G_RA(op)), F64(G_REGC(op)));						// fdmul   f0,ra,rc
			UML_FDADD(block, FREG(0), FREG(0), F64(G_RB(op)));								// fdadd   f0,f0,rb
			UML_FDNEG(block, FREG(0), FREG(0));												// fdneg   f0,f0
			UML_FDRNDS(block, F64(G_RD(op)), FREG(0));										// fdrnds  rd,f0
			return TRUE;

		case 0x1e:	/* FNMSUBSx */
			UML_FDMUL(block, FREG(0), F64(G_RA(op)), F64(G_REGC(op)));						// fdmul   f0,ra,rc
			UML_FDSUB(block, FREG(0), F64(G_RB(op)), FREG(0));								// fdsub   f0,rb,f0
			UML_FDRNDS(block, F64(G_RD(op)), FREG(0));										// fdrnds  rd,f0
			return TRUE;
	}

	return FALSE;
}



/*-------------------------------------------------
    generate_instruction_3f - compile opcodes in
    the 0x3f group
-------------------------------------------------*/

static int generate_instruction_3f(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = *desc->opptr.l;
	UINT32 opswitch = (op >> 1) & 0x3ff;

	if (opswitch & 0x10)
	{
		opswitch &= 0x1f;
		switch (opswitch)
		{
			case 0x15:	/* FADDx */
				UML_FDADD(block, F64(G_RD(op)), F64(G_RA(op)), F64(G_RB(op)));				// fdadd   rd,ra,rb
				return TRUE;

			case 0x14:	/* FSUBx */
				UML_FDSUB(block, F64(G_RD(op)), F64(G_RA(op)), F64(G_RB(op)));				// fdsub   rd,ra,rb
				return TRUE;

			case 0x19:	/* FMULx */
				UML_FDMUL(block, F64(G_RD(op)), F64(G_RA(op)), F64(G_RB(op)));				// fdmul   rd,ra,rb
				return TRUE;

			case 0x12:	/* FDIVx */
				UML_FDDIV(block, F64(G_RD(op)), F64(G_RA(op)), F64(G_RB(op)));				// fddiv   rd,ra,rb
				return TRUE;

			case 0x16:	/* FSQRTx */
				UML_FDSQRT(block, F64(G_RD(op)), F64(G_RB(op)));							// fdsqrt  rd,rb
				return TRUE;

			case 0x1a:	/* FRSQRTEx */
				UML_FDRSQRT(block, F64(G_RD(op)), F64(G_RB(op)));							// fdrsqrt rd,rb
				return TRUE;

			case 0x17:	/* FSELx */
				UML_FDCMP(block, F64(G_RA(op)), MEM(&ppc->impstate->fp0));					// fdcmp   f0,ra,[fp0]
				UML_FDMOVc(block, IF_AE, F64(G_RD(op)), F64(G_REGC(op)));					// fdmov   rd,rc,AE
				UML_FDMOVc(block, IF_B, F64(G_RD(op)), F64(G_RB(op)));						// fdmov   rd,rb,B
				return TRUE;

			case 0x1d:	/* FMADDx */
				UML_FDMUL(block, FREG(0), F64(G_RA(op)), F64(G_REGC(op)));					// fdmul   f0,ra,rc
				UML_FDADD(block, F64(G_RD(op)), FREG(0), F64(G_RB(op)));					// fdadd   rd,f0,rb
				return TRUE;

			case 0x1f:	/* FNMADDx */
				UML_FDMUL(block, FREG(0), F64(G_RA(op)), F64(G_REGC(op)));					// fdmul   f0,ra,rc
				UML_FDADD(block, FREG(0), FREG(0), F64(G_RB(op)));							// fdadd   f0,f0,rb
				UML_FDNEG(block, F64(G_RD(op)), FREG(0));									// fdneg   rd,f0
				return TRUE;

			case 0x1c:	/* FMSUBx */
				UML_FDMUL(block, FREG(0), F64(G_RA(op)), F64(G_REGC(op)));					// fdmul   f0,ra,rc
				UML_FDSUB(block, F64(G_RD(op)), FREG(0), F64(G_RB(op)));					// fdsub   rd,f0,rb
				return TRUE;

			case 0x1e:	/* FNMSUBx */
				UML_FDMUL(block, FREG(0), F64(G_RA(op)), F64(G_REGC(op)));					// fdmul   f0,ra,rc
				UML_FDSUB(block, F64(G_RD(op)), F64(G_RB(op)), FREG(0));					// fdsub   rd,rb,f0
				return TRUE;
		}
	}
	else
	{
		switch (opswitch)
		{
			case 0x32e:	/* FCTIDx - 64-bit only */
			case 0x32f:	/* FCTIDZx - 64-bit only */
			case 0x34e:	/* FCFIDx - 64-bit only */
				return FALSE;

			case 0x000:	/* FCMPU */
			case 0x020:	/* FCMPO */
				UML_FDCMP(block, F64(G_RA(op)), F64(G_RB(op)));								// fdcmp   ra,rb
				UML_GETFLGS(block, IREG(0), DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_U);	// getflgs i0,czu
				UML_LOAD(block, IREG(0), ppc->impstate->fcmp_cr_table, IREG(0), DWORD);		// load    i0,fcmp_cr_table,i0,dword
				UML_ROLINS(block, CR32, IREG(0), IMM(0), IMM(CRMASK(G_CRFD(op))));			// rolins  cr,i0,0,crmask[cr]
				return TRUE;

			case 0x00c:	/* FRSPx */
				UML_FDRNDS(block, F64(G_RD(op)), F64(G_RB(op)));							// fdrnds  rd,rb
				return TRUE;

			case 0x00e:	/* FCTIWx */
				UML_FDTOINT(block, IREG(0), F64(G_RB(op)), DWORD, DEFAULT);					// fdtoint i0,rb,dword,default
				UML_DAND(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0), IMM(0xffffffff));// dand    i0,i0,0xffffffff
				UML_FDMOV(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.w.l));			// fdmovr  rd,i0
				return TRUE;

			case 0x00f:	/* FCTIWZx */
				UML_FDTOINT(block, IREG(0), F64(G_RB(op)), DWORD, TRUNC);					// fdtoint i0,rb,dword,default
				UML_DAND(block, MEM(&ppc->impstate->tempdata.w.l), IREG(0), IMM(0xffffffff));// dand    i0,i0,0xffffffff
				UML_FDMOV(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.w.l));			// fdmovr  rd,i0
				return TRUE;

			case 0x028:	/* FNEGx */
				UML_FDNEG(block, F64(G_RD(op)), F64(G_RB(op)));								// fdneg   rd,rb
				return TRUE;

			case 0x048:	/* FMRx */
				UML_FDMOV(block, F64(G_RD(op)), F64(G_RB(op)));								// fdmov   rd,rb
				return TRUE;

			case 0x088:	/* FNABSx */
				UML_FDABS(block, FREG(0), F64(G_RB(op)));									// fdabs   f0,rb
				UML_FDNEG(block, F64(G_RD(op)), FREG(0));									// fdneg   rd,f0
				return TRUE;

			case 0x108:	/* FABSx */
				UML_FDABS(block, F64(G_RD(op)), F64(G_RB(op)));								// fdabs   rd,rb
				return TRUE;

			case 0x046:	/* MTFSB0x */
				UML_AND(block, FPSCR32, FPSCR32, IMM(~(0x80000000 >> G_CRBD(op))));			// and     fpscr32,fpscr32,~(0x80000000 >> G_CRBD)
				return TRUE;

			case 0x026:	/* MTFSB1x */
				UML_OR(block, FPSCR32, FPSCR32, IMM(0x80000000 >> G_CRBD(op)));				// or      fpscr32,fpscr32,(0x80000000 >> G_CRBD)
				return TRUE;

			case 0x040:	/* MCRFS */
				UML_ROLINS(block, CR32, FPSCR32, IMM(((G_CRFS(op) - G_CRFD(op)) & 7) * 4), IMM(CRMASK(G_CRFD(op))));
																							// rolins  cr,fpscr,shift,crmask[crfd]
				UML_AND(block, FPSCR32, FPSCR32, IMM(~CRMASK(G_CRFS(op))));					// and     fpscr,fpscr,~crmask[crfs]
				return TRUE;

			case 0x247:	/* MFFSx */
				UML_MOV(block, MEM(&ppc->impstate->tempdata.w.l), FPSCR32);					// mov     [tempdata],fpscr
				UML_FSMOV(block, F64(G_RD(op)), MEM(&ppc->impstate->tempdata.d));			// fsmov   rd,fpscr
				return TRUE;

			case 0x2c7:	/* MTFSFx */
				UML_FDMOV(block, MEM(&ppc->impstate->tempdata.d), F64(G_RB(op)));			// fdmov   [tempdata],fb
				UML_ROLINS(block, FPSCR32, MEM(&ppc->impstate->tempdata.w.l), IMM(0), IMM(compute_crf_mask(G_FM(op))));
																							// rolins  fpscr,rb,0,crf_mask
				return TRUE;

			case 0x086:	/* MTFSFIx */
				UML_ROLINS(block, FPSCR32, IMM(G_IMM(op)), IMM(28 - 4 * G_CRFD(op)), IMM(CRMASK(G_CRFD(op))));
																							// rolins  fpscr,rb,0,crf_mask
				return TRUE;
		}
	}

	return FALSE;
}



/***************************************************************************
    CODE LOGGING HELPERS
***************************************************************************/

/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of a PowerPC instruction
-------------------------------------------------*/

static void log_add_disasm_comment(drcuml_block *block, UINT32 pc, UINT32 op)
{
#if (LOG_UML)
	char buffer[100];
	ppc_dasm_one(buffer, pc, op);
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

	for (regnum = 0; regnum < 32; regnum++)
		if (gprmask & ((UINT64)1 << regnum))
			drcuml_log_printf(drcuml, "%sr%d", (count++ == 0) ? "" : ",", regnum);
	for (regnum = 0; regnum < 8; regnum++)
		if (gprmask & ((UINT64)1 << (32 + regnum)))
			drcuml_log_printf(drcuml, "%scr%d", (count++ == 0) ? "" : ",", regnum);
	if (gprmask & ((UINT64)1 << 40))
		drcuml_log_printf(drcuml, "%sxer", (count++ == 0) ? "" : ",");
	if (gprmask & ((UINT64)1 << 41))
		drcuml_log_printf(drcuml, "%sctr", (count++ == 0) ? "" : ",");
	if (gprmask & ((UINT64)1 << 42))
		drcuml_log_printf(drcuml, "%slr", (count++ == 0) ? "" : ",");

	for (regnum = 0; regnum < 32; regnum++)
		if (fprmask & ((UINT64)1 << regnum))
			drcuml_log_printf(drcuml, "%sfpr%d", (count++ == 0) ? "" : ",", regnum);
	if (fprmask & ((UINT64)1 << 32))
		drcuml_log_printf(drcuml, "%sfpscr", (count++ == 0) ? "" : ",");
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
#if (LOG_UML || LOG_NATIVE)
		if (desclist->flags & OPFLAG_VIRTUAL_NOOP)
			strcpy(buffer, "<virtual nop>");
		else
			ppc_dasm_one(buffer, desclist->pc, *desclist->opptr.l);
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
    PPC 4XX VARIANTS
***************************************************************************/

#if (HAS_PPC403GA || HAS_PPC403GCX)

/*-------------------------------------------------
    ppcdrc4xx_get_info - PowerPC 4XX-specific
    information getter
-------------------------------------------------*/

static void ppcdrc4xx_get_info(UINT32 state, cpuinfo *info)
{
	ppcdrc_get_info(state, info);
	ppc4xx_get_info(ppc, state, info);
}


/*-------------------------------------------------
    ppcdrc4xx_set_info - PowerPC 4XX-specific
    information setter
-------------------------------------------------*/

static void ppcdrc4xx_set_info(UINT32 state, cpuinfo *info)
{
	ppcdrc_set_info(state, info);
	ppc4xx_set_info(ppc, state, info);
}

#endif


#if (HAS_PPC403GA)

/*-------------------------------------------------
    ppc403ga_init - PowerPC 403GA-specific
    initialization
-------------------------------------------------*/

static void ppc403ga_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	ppcdrc_init(PPC_MODEL_403GA, PPCCAP_4XX, 1, clock, config, irqcallback);
}


/*-------------------------------------------------
    ppc403ga_get_info - PowerPC 403GA-specific
    information getter
-------------------------------------------------*/

void ppc403ga_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = ppc403ga_init;				break;
		case CPUINFO_PTR_SET_INFO:						info->setinfo = ppcdrc4xx_set_info;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PowerPC 403GA");		break;

		/* --- everything else is handled generically --- */
		default:										ppcdrc4xx_get_info(state, info);		break;
	}
}

#endif


#if (HAS_PPC403GCX)

/*-------------------------------------------------
    ppc403gcx_init - PowerPC 403GCX-specific
    initialization
-------------------------------------------------*/

static void ppc403gcx_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	ppcdrc_init(PPC_MODEL_403GCX, PPCCAP_4XX, 1, clock, config, irqcallback);
}


/*-------------------------------------------------
    ppc403gcx_get_info - PowerPC 403GCX-specific
    information getter
-------------------------------------------------*/

void ppc403gcx_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = ppc403gcx_init;			break;
		case CPUINFO_PTR_SET_INFO:						info->setinfo = ppcdrc4xx_set_info;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PowerPC 403GCX");		break;

		/* --- everything else is handled generically --- */
		default:										ppcdrc4xx_get_info(state, info);		break;
	}
}

#endif



/***************************************************************************
    PPC 6XX VARIANTS
***************************************************************************/

#if (HAS_PPC601)

/*-------------------------------------------------
    ppc601_init - PowerPC 601-specific
    initialization
-------------------------------------------------*/

static void ppc601_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	ppcdrc_init(PPC_MODEL_601, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED, 0/* no TB */, clock, config, irqcallback);
}


/*-------------------------------------------------
    ppc601_get_info - PowerPC 601-specific
    information getter
-------------------------------------------------*/

void ppc601_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = ppc601_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PowerPC 601");			break;

		/* --- everything else is handled generically --- */
		default:										ppcdrc_get_info(state, info);			break;
	}
}

#endif


#if (HAS_PPC602)

/*-------------------------------------------------
    ppc602_init - PowerPC 602-specific
    initialization
-------------------------------------------------*/

static void ppc602_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	ppcdrc_init(PPC_MODEL_602, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED, 1/* unknown */, clock, config, irqcallback);
}


/*-------------------------------------------------
    ppc602_get_info - PowerPC 602-specific
    information getter
-------------------------------------------------*/

void ppc602_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = ppc602_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PowerPC 602");			break;

		/* --- everything else is handled generically --- */
		default:										ppcdrc_get_info(state, info);			break;
	}
}

#endif


#if (HAS_PPC603)

/*-------------------------------------------------
    ppc603_init - PowerPC 603-specific
    initialization
-------------------------------------------------*/

static void ppc603_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	ppcdrc_init(PPC_MODEL_603, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, clock, config, irqcallback);
}


/*-------------------------------------------------
    ppc603_get_info - PowerPC 603-specific
    information getter
-------------------------------------------------*/

void ppc603_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = ppc603_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PowerPC 603");			break;

		/* --- everything else is handled generically --- */
		default:										ppcdrc_get_info(state, info);			break;
	}
}

#endif


#if (HAS_PPC603E)

/*-------------------------------------------------
    ppc603e_init - PowerPC 603e-specific
    initialization
-------------------------------------------------*/

static void ppc603e_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	ppcdrc_init(PPC_MODEL_603E, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, clock, config, irqcallback);
}


/*-------------------------------------------------
    ppc603e_get_info - PowerPC 603e-specific
    information getter
-------------------------------------------------*/

void ppc603e_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = ppc603e_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PowerPC 603e");		break;

		/* --- everything else is handled generically --- */
		default:										ppcdrc_get_info(state, info);			break;
	}
}

#endif


#if (HAS_PPC603R)

/*-------------------------------------------------
    ppc603r_init - PowerPC 603r-specific
    initialization
-------------------------------------------------*/

static void ppc603r_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	ppcdrc_init(PPC_MODEL_603R, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED | PPCCAP_603_MMU, 4, clock, config, irqcallback);
}


/*-------------------------------------------------
    ppc603r_get_info - PowerPC 603r-specific
    information getter
-------------------------------------------------*/

void ppc603r_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = ppc603r_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PowerPC 603R");		break;

		/* --- everything else is handled generically --- */
		default:										ppcdrc_get_info(state, info);			break;
	}
}

#endif


#if (HAS_PPC604)

/*-------------------------------------------------
    ppc604_init - PowerPC 604-specific
    initialization
-------------------------------------------------*/

static void ppc604_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	ppcdrc_init(PPC_MODEL_604, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED, 4, clock, config, irqcallback);
}


/*-------------------------------------------------
    ppc604_get_info - PowerPC 604-specific
    information getter
-------------------------------------------------*/

void ppc604_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = ppc604_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PowerPC 604");			break;

		/* --- everything else is handled generically --- */
		default:										ppcdrc_get_info(state, info);			break;
	}
}

#endif



/***************************************************************************
    PPC 6XX VARIANTS
***************************************************************************/

#if (HAS_MPC8240)

/*-------------------------------------------------
    mpc8240_init - PowerPC MPC8240-specific
    initialization
-------------------------------------------------*/

static void mpc8240_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	ppcdrc_init(PPC_MODEL_MPC8240, PPCCAP_OEA | PPCCAP_VEA | PPCCAP_FPU | PPCCAP_MISALIGNED, 4/* unknown */, clock, config, irqcallback);
}


/*-------------------------------------------------
    mpc8240_get_info - PowerPC MPC8240-specific
    information getter
-------------------------------------------------*/

void mpc8240_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INIT:							info->init = mpc8240_init;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PowerPC MPC8240");		break;

		/* --- everything else is handled generically --- */
		default:										ppcdrc_get_info(state, info);			break;
	}
}

#endif



/***************************************************************************
    DISASSEMBLERS
***************************************************************************/

#if !defined(MAME_DEBUG) && (LOG_UML || LOG_NATIVE)
#include "ppc_dasm.c"
#endif
