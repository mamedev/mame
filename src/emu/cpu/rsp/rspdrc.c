/***************************************************************************

    rspdrc.c

    Universal machine language-based Nintendo/SGI RSP emulator.
    Written by Harmony of the MESS team.

    Copyright the MESS team.
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Future improvements/changes:

    * Confer with Aaron Giles about adding a memory hash-based caching
      system and static recompilation for maximum overhead minimization

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "profiler.h"
#include "rsp.h"
#include "rspfe.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

CPU_DISASSEMBLE( rsp );

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define FORCE_C_BACKEND					(0)
#define LOG_UML							(0)
#define LOG_NATIVE						(0)

#define SINGLE_INSTRUCTION_MODE			(0)

#define DRC_LSV							(1)
#define DRC_LLV							(1)
#define DRC_LDV							(1)
#define DRC_LQV							(1)
#define DRC_LPV							(1)
#define DRC_LUV							(0)

#define DRC_SSV							(1)
#define DRC_SLV							(1)
#define DRC_SDV							(1)
#define DRC_SQV							(0)
#define DRC_SPV							(0) // Todo

#define DRC_VMUDL						(0)
#define DRC_VMUDM						(0)
#define DRC_VMADM						(0)
#define DRC_VMADN						(0)
#define DRC_VMADH						(0)
#define DRC_VADD						(0)
#define DRC_VAND						(0)
#define DRC_VNAND						(0)
#define DRC_VOR							(0)
#define DRC_VNOR						(0)
#define DRC_VXOR						(0)
#define DRC_VNXOR						(0)


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
#define EXECUTE_UNMAPPED_CODE			2
#define EXECUTE_RESET_CACHE				3



/***************************************************************************
    MACROS
***************************************************************************/

#define R32(reg)				rsp->impstate->regmap[reg].type, rsp->impstate->regmap[reg].value
#define VB(reg, el)				rsp->impstate->regmap[16*reg+(15-el)+34].type, rsp->impstate->regmap[16*reg+(15-el)+34].value
#define VS(reg, el)				rsp->impstate->regmap[8*reg+(7-el)+546].type, rsp->impstate->regmap[8*reg+(7-el)+546].value
#define VSX(reg, el)			rsp->impstate->regmap[8*reg+el+546].type, rsp->impstate->regmap[8*reg+el+546].value
#define VL(reg, el)				rsp->impstate->regmap[4*reg+(3-el)+802].type, rsp->impstate->regmap[4*reg+(3-el)+802].value
#define VLX(reg, el)			rsp->impstate->regmap[4*reg+el+802].type, rsp->impstate->regmap[4*reg+el+802].value
#define VFLAG(reg)				rsp->impstate->regmap[930+reg].type, rsp->impstate->regmap[930+reg].value
#define VACCUML(reg)			rsp->impstate->regmap[934+reg].type, rsp->impstate->regmap[934+reg].value
#define VACCUMHH(reg)			rsp->impstate->regmap[942+reg].type, rsp->impstate->regmap[942+reg].value
#define VACCUMHM(reg)			rsp->impstate->regmap[950+reg].type, rsp->impstate->regmap[950+reg].value
#define VACCUMHL(reg)			rsp->impstate->regmap[958+reg].type, rsp->impstate->regmap[958+reg].value
#define VACCUMHZ(reg)			rsp->impstate->regmap[966+reg].type, rsp->impstate->regmap[966+reg].value
#define VACCUMWMH(reg)			rsp->impstate->regmap[974+reg].type, rsp->impstate->regmap[974+reg].value
#define VACCUMWZL(reg)			rsp->impstate->regmap[982+reg].type, rsp->impstate->regmap[982+reg].value
#define VRES(reg)				rsp->impstate->regmap[990+reg].type, rsp->impstate->regmap[990+reg].value



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


/* internal compiler state */
typedef struct _compiler_state compiler_state;
struct _compiler_state
{
	UINT32				cycles;						/* accumulated cycles */
	UINT8				checkints;					/* need to check interrupts before next instruction */
	UINT8				checksoftints;				/* need to check software interrupts before next instruction */
	drcuml_codelabel	labelnum;					/* index for local labels */
};

struct _rspimp_state
{
	/* core state */
	drccache *			cache;						/* pointer to the DRC code cache */
	drcuml_state *		drcuml;						/* DRC UML generator state */
	drcfe_state *		drcfe;						/* pointer to the DRC front-end state */
	UINT32				drcoptions;					/* configurable DRC options */

	/* internal stuff */
	UINT8				cache_dirty;				/* true if we need to flush the cache */
	UINT32				jmpdest;					/* destination jump target */

	/* parameters for subroutines */
	UINT64				numcycles;					/* return value from gettotalcycles */
	const char *		format;						/* format string for print_debug */
	UINT32				arg0;						/* print_debug argument 1 */
	UINT32				arg1;						/* print_debug argument 2 */
	UINT64				arg64;						/* print_debug 64-bit argument */
	UINT32				vres[8];					/* used for temporary vector results */

	/* register mappings */
	drcuml_parameter	regmap[998];				/* parameter to register mappings for all 32 integer registers, vector registers, flags, accumulators and temps */

	/* subroutines */
	drcuml_codehandle *	entry;						/* entry point */
	drcuml_codehandle *	nocode;						/* nocode exception handler */
	drcuml_codehandle *	out_of_cycles;				/* out of cycles exception handler */
	drcuml_codehandle *	read8;						/* read byte */
	drcuml_codehandle *	write8;						/* write byte */
	drcuml_codehandle *	read16;						/* read half */
	drcuml_codehandle *	write16;					/* write half */
	drcuml_codehandle *	read32;						/* read word */
	drcuml_codehandle *	write32;					/* write word */

	/* fast RAM */
	void*				dmem;
	void*				imem;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void code_flush_cache(rsp_state *rsp);
static void code_compile_block(rsp_state *rsp, offs_t pc);

static void cfunc_unimplemented(void *param);
static void cfunc_set_cop0_reg(void *param);
static void cfunc_get_cop0_reg(void *param);
static void cfunc_mfc2(void *param);
static void cfunc_cfc2(void *param);
static void cfunc_mtc2(void *param);
static void cfunc_ctc2(void *param);
//static void cfunc_swc2(void *param);
//static void cfunc_lwc2(void *param);
static void cfunc_sp_set_status_cb(void *param);

static void cfunc_rsp_lbv(void *param);
#if !(DRC_LSV)
static void cfunc_rsp_lsv(void *param);
#endif
#if !(DRC_LLV)
static void cfunc_rsp_llv(void *param);
#endif
#if !(DRC_LDV)
static void cfunc_rsp_ldv(void *param);
#endif
#if !(DRC_LQV)
static void cfunc_rsp_lqv(void *param);
#endif
static void cfunc_rsp_lrv(void *param);
#if !(DRC_LPV)
static void cfunc_rsp_lpv(void *param);
#endif
#if !(DRC_LUV)
static void cfunc_rsp_luv(void *param);
#endif
static void cfunc_rsp_lhv(void *param);
static void cfunc_rsp_lfv(void *param);
static void cfunc_rsp_lwv(void *param);
static void cfunc_rsp_ltv(void *param);

static void cfunc_rsp_sbv(void *param);
#if !(DRC_SSV)
static void cfunc_rsp_ssv(void *param);
#endif
#if !(DRC_SLV)
static void cfunc_rsp_slv(void *param);
#endif
#if !(DRC_SDV)
static void cfunc_rsp_sdv(void *param);
#endif
#if !(DRC_SQV)
static void cfunc_rsp_sqv(void *param);
#endif
static void cfunc_rsp_srv(void *param);
#if !(DRC_SPV)
static void cfunc_rsp_spv(void *param);
#endif
static void cfunc_rsp_suv(void *param);
static void cfunc_rsp_shv(void *param);
static void cfunc_rsp_sfv(void *param);
static void cfunc_rsp_swv(void *param);
static void cfunc_rsp_stv(void *param);

static void static_generate_entry_point(rsp_state *rsp);
static void static_generate_nocode_handler(rsp_state *rsp);
static void static_generate_out_of_cycles(rsp_state *rsp);
static void static_generate_memory_accessor(rsp_state *rsp, int size, int iswrite, const char *name, drcuml_codehandle **handleptr);

#if (DRC_VMADN)
static void generate_saturate_accum_unsigned(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int accum);
#endif
#if (DRC_VMADH) || (DRC_VMADM)
static void generate_saturate_accum_signed(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int accum);
#endif
#if (DRC_VMUDL)
static int generate_vmudl(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vmudl
#endif
#if (DRC_VMADM)
static int generate_vmadm(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vmadm
#endif
#if (DRC_VMADN)
static int generate_vmadn(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vmadn
#endif
#if (DRC_VMADH)
static int generate_vmadh(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vmadh
#endif
#if (DRC_VADD)
static int generate_vadd(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vadd
#endif
#if (DRC_VAND)
static int generate_vand(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vand
#endif
#if (DRC_VNAND)
static int generate_vnand(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vand
#endif
#if (DRC_VOR)
static int generate_vor(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vand
#endif
#if (DRC_VNOR)
static int generate_vnor(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vand
#endif
#if (DRC_VXOR)
static int generate_vxor(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vand
#endif
#if (DRC_VNXOR)
static int generate_vnxor(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);	// vand
#endif
static int generate_lwc2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_swc2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static void generate_update_cycles(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, drcuml_ptype ptype, UINT64 pvalue, int allow_exception);
static void generate_checksum_block(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
static void generate_sequence_instruction(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static void generate_delay_slot_and_branch(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg);
static int generate_opcode(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_special(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_regimm(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_cop0(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_cop2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);

static void log_add_disasm_comment(rsp_state *rsp, drcuml_block *block, UINT32 pc, UINT32 op);

/***************************************************************************
    HELPFUL DEFINES
***************************************************************************/

#define VDREG						((op >> 6) & 0x1f)
#define VS1REG						((op >> 11) & 0x1f)
#define VS2REG						((op >> 16) & 0x1f)
#define EL							((op >> 21) & 0xf)

#define R_VREG_B(reg, offset)		rsp->v[(reg)].b[15 - (offset)]
#define R_VREG_S(reg, offset)		(INT16)rsp->v[(reg)].s[7 - (offset)]
#define R_VREG_S_X(reg, offset)		(INT16)rsp->v[reg].s[offset]
#define R_VREG_L(reg, offset)		rsp->v[(reg)].l[3 - (offset)]
#define R_VREG_D(reg, offset)		rsp->v[(reg)].d[1 - (offset)]

#define W_VREG_B(reg, offset, val)	(rsp->v[(reg)].b[15 - (offset)] = val)
#define W_VREG_S(reg, offset, val)	(rsp->v[(reg)].s[7 - (offset)] = val)
#define W_VREG_S_X(reg, offset, val) (rsp->v[reg].s[offset] = val)
#define W_VREG_L(reg, offset, val)	(rsp->v[(reg)].l[3 - (offset)] = val)
#define W_VREG_D(reg, offset, val)	(rsp->v[(reg)].d[1 - (offset)] = val)

#define VEC_EL_1(x,z)				(vector_elements_1[(x)][(z)])
#define VEC_EL_2(x,z)				(vector_elements_2[(x)][(z)])

#define ACCUM(x)					(rsp->accum[x])

#define W_ACCUM_H(x, y)				(ACCUM(x).h.high = (INT16)((INT64)(y)))
#define W_ACCUM_M(x, y)				(ACCUM(x).h.mid  = (INT16)((INT64)(y)))
#define W_ACCUM_L(x, y)				(ACCUM(x).h.low  = (INT16)((INT64)(y)))

#define CARRY_FLAG(x)				((rsp->flag[0] & (1 << (x))) ? 1 : 0)
#define CLEAR_CARRY_FLAGS()			{ rsp->flag[0] &= ~0xff; }
#define SET_CARRY_FLAG(x)			{ rsp->flag[0] |= (1 << (x)); }
#define CLEAR_CARRY_FLAG(x)			{ rsp->flag[0] &= ~(1 << (x)); }

#define COMPARE_FLAG(x)				((rsp->flag[1] & (1 << (x))) ? 1 : 0)
#define CLEAR_COMPARE_FLAGS()		{ rsp->flag[1] &= ~0xff; }
#define SET_COMPARE_FLAG(x)			{ rsp->flag[1] |= (1 << (x)); }
#define CLEAR_COMPARE_FLAG(x)		{ rsp->flag[1] &= ~(1 << (x)); }

#define ZERO_FLAG(x)				((rsp->flag[0] & (0x100 << (x))) ? 1 : 0)
#define CLEAR_ZERO_FLAGS()			{ rsp->flag[0] &= ~0xff00; }
#define SET_ZERO_FLAG(x)			{ rsp->flag[0] |= (0x100 << (x)); }
#define CLEAR_ZERO_FLAG(x)			{ rsp->flag[0] &= ~(0x100 << (x)); }

INLINE rsp_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_RSP);
	return *(rsp_state **)device->token;
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    epc - compute the exception PC from a
    descriptor
-------------------------------------------------*/

INLINE UINT32 epc(const opcode_desc *desc)
{
	return ((desc->flags & OPFLAG_IN_DELAY_SLOT) ? (desc->pc - 3) : desc->pc) | 0x1000;
}


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

INLINE void load_fast_iregs(rsp_state *rsp, drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(rsp->impstate->regmap); regnum++)
		if (rsp->impstate->regmap[regnum].type == DRCUML_PTYPE_INT_REGISTER)
			UML_MOV(block, IREG(rsp->impstate->regmap[regnum].value - DRCUML_REG_I0), MEM(&rsp->r[regnum]));
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

INLINE void save_fast_iregs(rsp_state *rsp, drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(rsp->impstate->regmap); regnum++)
		if (rsp->impstate->regmap[regnum].type == DRCUML_PTYPE_INT_REGISTER)
			UML_MOV(block, MEM(&rsp->r[regnum]), IREG(rsp->impstate->regmap[regnum].value - DRCUML_REG_I0));
}

/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

/* Legacy.  Needed for vector cfuncs. */
INLINE UINT8 READ8(rsp_state *rsp, UINT32 address)
{
	UINT8* dmem = (UINT8*)rsp->impstate->dmem;
	return dmem[BYTE4_XOR_BE(address & 0x00000fff)];
}

/* Legacy.  Needed for vector cfuncs. */
INLINE UINT16 READ16(rsp_state *rsp, UINT32 address)
{
	UINT8* dmem = (UINT8*)rsp->impstate->dmem;
	UINT16 ret = 0;
	address &= 0x00000fff;
	ret  = (UINT16)dmem[BYTE4_XOR_BE(address+0)] << 8;
	ret |= (UINT16)dmem[BYTE4_XOR_BE(address+1)];
	return ret;
}

/* Legacy.  Needed for vector cfuncs. */
INLINE UINT32 READ32(rsp_state *rsp, UINT32 address)
{
	UINT8* dmem = (UINT8*)rsp->impstate->dmem;
	UINT32 ret = 0;
	address &= 0x00000fff;
	ret  = (UINT32)dmem[BYTE4_XOR_BE(address+0)] << 24;
	ret |= (UINT32)dmem[BYTE4_XOR_BE(address+1)] << 16;
	ret |= (UINT32)dmem[BYTE4_XOR_BE(address+2)] <<  8;
	ret |= (UINT32)dmem[BYTE4_XOR_BE(address+3)];
	return ret;
}

/* Legacy.  Needed for vector cfuncs. */
INLINE UINT64 READ64(rsp_state *rsp, UINT32 address)
{
	UINT8* dmem = (UINT8*)rsp->impstate->dmem;
	UINT64 ret = 0;
	address &= 0x00000fff;
	ret  = (UINT64)dmem[BYTE4_XOR_BE(address+0)] << 56;
	ret |= (UINT64)dmem[BYTE4_XOR_BE(address+1)] << 48;
	ret |= (UINT64)dmem[BYTE4_XOR_BE(address+2)] << 40;
	ret |= (UINT64)dmem[BYTE4_XOR_BE(address+3)] << 32;
	ret |= (UINT64)dmem[BYTE4_XOR_BE(address+4)] << 24;
	ret |= (UINT64)dmem[BYTE4_XOR_BE(address+5)] << 16;
	ret |= (UINT64)dmem[BYTE4_XOR_BE(address+6)] <<  8;
	ret |= (UINT64)dmem[BYTE4_XOR_BE(address+7)];
	return ret;
}

/* Legacy.  Needed for vector cfuncs. */
INLINE void WRITE8(rsp_state *rsp, UINT32 address, UINT8 data)
{
	UINT8* dmem = (UINT8*)rsp->impstate->dmem;
	address &= 0x00000fff;
	dmem[BYTE4_XOR_BE(address)] = data;
}

/* Legacy.  Needed for vector cfuncs. */
INLINE void WRITE16(rsp_state *rsp, UINT32 address, UINT16 data)
{
	UINT8* dmem = (UINT8*)rsp->impstate->dmem;
	address &= 0x00000fff;
	dmem[BYTE4_XOR_BE(address+0)] = (UINT8)(data >> 8);
	dmem[BYTE4_XOR_BE(address+1)] = (UINT8)(data >> 0);
}

/* Legacy.  Needed for vector cfuncs. */
INLINE void WRITE32(rsp_state *rsp, UINT32 address, UINT32 data)
{
	UINT8* dmem = (UINT8*)rsp->impstate->dmem;
	address &= 0x00000fff;
	dmem[BYTE4_XOR_BE(address+0)] = (UINT8)(data >> 24);
	dmem[BYTE4_XOR_BE(address+1)] = (UINT8)(data >> 16);
	dmem[BYTE4_XOR_BE(address+2)] = (UINT8)(data >>  8);
	dmem[BYTE4_XOR_BE(address+3)] = (UINT8)(data >>  0);
}

/* Legacy.  Needed for vector cfuncs. */
INLINE void WRITE64(rsp_state *rsp, UINT32 address, UINT64 data)
{
	UINT8* dmem = (UINT8*)rsp->impstate->dmem;
	address &= 0x00000fff;
	dmem[BYTE4_XOR_BE(address+0)] = (UINT8)(data >> 56);
	dmem[BYTE4_XOR_BE(address+1)] = (UINT8)(data >> 48);
	dmem[BYTE4_XOR_BE(address+2)] = (UINT8)(data >> 40);
	dmem[BYTE4_XOR_BE(address+3)] = (UINT8)(data >> 32);
	dmem[BYTE4_XOR_BE(address+4)] = (UINT8)(data >> 24);
	dmem[BYTE4_XOR_BE(address+5)] = (UINT8)(data >> 16);
	dmem[BYTE4_XOR_BE(address+6)] = (UINT8)(data >>  8);
	dmem[BYTE4_XOR_BE(address+7)] = (UINT8)(data >>  0);
}

/*****************************************************************************/

/*-------------------------------------------------
    rspdrc_set_options - configure DRC options
-------------------------------------------------*/

void rspdrc_set_options(running_device *device, UINT32 options)
{
	rsp_state *rsp = get_safe_token(device);
	rsp->impstate->drcoptions = options;
}


/*-------------------------------------------------
    rspdrc_add_imem - register an imem region
-------------------------------------------------*/

void rspdrc_add_imem(running_device *device, void *base)
{
	rsp_state *rsp = get_safe_token(device);
	rsp->impstate->imem = base;
}


/*-------------------------------------------------
    rspdrc_add_dmem - register a dmem region
-------------------------------------------------*/

void rspdrc_add_dmem(running_device *device, void *base)
{
	rsp_state *rsp = get_safe_token(device);
	rsp->impstate->dmem = base;
}


/*-------------------------------------------------
    cfunc_printf_debug - generic printf for
    debugging
-------------------------------------------------*/

//static void cfunc_printf_debug(void *param)
//{
//  rsp_state *rsp = (rsp_state *)param;
//  printf(rsp->impstate->format, rsp->impstate->arg0, rsp->impstate->arg1);
//  logerror(rsp->impstate->format, rsp->impstate->arg0, rsp->impstate->arg1);
//}


/*-------------------------------------------------
    cfunc_printf_debug64 - generic printf for
    debugging 64-bit values
-------------------------------------------------*/

//static void cfunc_printf_debug64(void *param)
//{
//  rsp_state *rsp = (rsp_state *)param;
//  printf(rsp->impstate->format, (UINT32)(rsp->impstate->arg64 >> 32), (UINT32)(rsp->impstate->arg64 & 0x00000000ffffffff));
//  logerror(rsp->impstate->format, (UINT32)(rsp->impstate->arg64 >> 32), (UINT32)(rsp->impstate->arg64 & 0x00000000ffffffff));
//}


static void cfunc_get_cop0_reg(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int reg = rsp->impstate->arg0;
	int dest = rsp->impstate->arg1;

	if (reg >= 0 && reg < 8)
	{
		if(dest)
		{
			rsp->r[dest] = (rsp->config->sp_reg_r)(rsp->device, reg, 0x00000000);
		}
	}
	else if (reg >= 8 && reg < 16)
	{
		if(dest)
		{
			rsp->r[dest] = (rsp->config->dp_reg_r)(rsp->device, reg - 8, 0x00000000);
		}
	}
	else
	{
		fatalerror("RSP: cfunc_get_cop0_reg: %d", reg);
	}
}

static void cfunc_set_cop0_reg(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int reg = rsp->impstate->arg0;
	UINT32 data = rsp->impstate->arg1;

	if (reg >= 0 && reg < 8)
	{
		(rsp->config->sp_reg_w)(rsp->device, reg, data, 0x00000000);
	}
	else if (reg >= 8 && reg < 16)
	{
		(rsp->config->dp_reg_w)(rsp->device, reg - 8, data, 0x00000000);
	}
	else
	{
		fatalerror("RSP: set_cop0_reg: %d, %08X\n", reg, data);
	}
}

static void cfunc_unimplemented_opcode(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	if ((rsp->device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		char string[200];
		rsp_dasm_one(string, rsp->ppc, op);
		mame_printf_debug("%08X: %s\n", rsp->ppc, string);
	}

	fatalerror("RSP: unknown opcode %02X (%08X) at %08X\n", op >> 26, op, rsp->ppc);
}

static void unimplemented_opcode(rsp_state *rsp, UINT32 op)
{
	if ((rsp->device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		char string[200];
		rsp_dasm_one(string, rsp->ppc, op);
		mame_printf_debug("%08X: %s\n", rsp->ppc, string);
	}

	fatalerror("RSP: unknown opcode %02X (%08X) at %08X\n", op >> 26, op, rsp->ppc);
}

/*****************************************************************************/

/* Legacy.  Going forward, this will be transitioned into unrolled opcode decodes. */
static const int vector_elements_1[16][8] =
{
	{ 0, 1, 2, 3, 4, 5, 6, 7 },		// none
	{ 0, 1, 2, 3, 4, 5, 6 ,7 },		// ???
	{ 1, 3, 5, 7, 0, 2, 4, 6 },		// 0q
	{ 0, 2, 4, 6, 1, 3, 5, 7 },		// 1q
	{ 1, 2, 3, 5, 6, 7, 0, 4 },		// 0h
	{ 0, 2, 3, 4, 6, 7, 1, 5 },		// 1h
	{ 0, 1, 3, 4, 5, 7, 2, 6 },		// 2h
	{ 0, 1, 2, 4, 5, 6, 3, 7 },		// 3h
	{ 1, 2, 3, 4, 5, 6, 7, 0 },		// 0
	{ 0, 2, 3, 4, 5, 6, 7, 1 },		// 1
	{ 0, 1, 3, 4, 5, 6, 7, 2 },		// 2
	{ 0, 1, 2, 4, 5, 6, 7, 3 },		// 3
	{ 0, 1, 2, 3, 5, 6, 7, 4 },		// 4
	{ 0, 1, 2, 3, 4, 6, 7, 5 },		// 5
	{ 0, 1, 2, 3, 4, 5, 7, 6 },		// 6
	{ 0, 1, 2, 3, 4, 5, 6, 7 },		// 7
};

/* Legacy.  Going forward, this will be transitioned into unrolled opcode decodes. */
static const int vector_elements_2[16][8] =
{
	{ 0, 1, 2, 3, 4, 5, 6, 7 },		// none
	{ 0, 1, 2, 3, 4, 5, 6, 7 },		// ???
	{ 0, 0, 2, 2, 4, 4, 6, 6 },		// 0q
	{ 1, 1, 3, 3, 5, 5, 7, 7 },		// 1q
	{ 0, 0, 0, 0, 4, 4, 4, 4 },		// 0h
	{ 1, 1, 1, 1, 5, 5, 5, 5 },		// 1h
	{ 2, 2, 2, 2, 6, 6, 6, 6 },		// 2h
	{ 3, 3, 3, 3, 7, 7, 7, 7 },		// 3h
	{ 0, 0, 0, 0, 0, 0, 0, 0 },		// 0
	{ 1, 1, 1, 1, 1, 1, 1, 1 },		// 1
	{ 2, 2, 2, 2, 2, 2, 2, 2 },		// 2
	{ 3, 3, 3, 3, 3, 3, 3, 3 },		// 3
	{ 4, 4, 4, 4, 4, 4, 4, 4 },		// 4
	{ 5, 5, 5, 5, 5, 5, 5, 5 },		// 5
	{ 6, 6, 6, 6, 6, 6, 6, 6 },		// 6
	{ 7, 7, 7, 7, 7, 7, 7, 7 },		// 7
};

static void rspcom_init(rsp_state *rsp, running_device *device, cpu_irq_callback irqcallback)
{
	int regIdx = 0;
    int accumIdx;

	memset(rsp, 0, sizeof(*rsp));

	rsp->config = (const rsp_config *)device->baseconfig().static_config;
	rsp->irq_callback = irqcallback;
	rsp->device = device;
	rsp->program = device->space(AS_PROGRAM);

#if 1
    // Inaccurate.  RSP registers power on to a random state...
	for(regIdx = 0; regIdx < 32; regIdx++ )
	{
		rsp->r[regIdx] = 0;
		rsp->v[regIdx].d[0] = 0;
		rsp->v[regIdx].d[1] = 0;
	}
	rsp->flag[0] = 0;
	rsp->flag[1] = 0;
	rsp->flag[2] = 0;
	rsp->flag[3] = 0;
	rsp->square_root_res = 0;
	rsp->square_root_high = 0;
	rsp->reciprocal_res = 0;
	rsp->reciprocal_high = 0;
#endif

    // ...except for the accumulators.
    for(accumIdx = 0; accumIdx < 8; accumIdx++ )
    {
        rsp->accum[accumIdx].l = 0;
    }

	rsp->sr = RSP_STATUS_HALT;
    rsp->step_count = 0;
}

static CPU_INIT( rsp )
{
	drcfe_config feconfig =
	{
		COMPILE_BACKWARDS_BYTES,	/* code window start offset = startpc - window_start */
		COMPILE_FORWARDS_BYTES,		/* code window end offset = startpc + window_end */
		COMPILE_MAX_SEQUENCE,		/* maximum instructions to include in a sequence */
		rspfe_describe				/* callback to describe a single instruction */
	};
	rsp_state *rsp;
	drccache *cache;
	UINT32 flags = 0;
	int regnum;
	int elnum;

	/* allocate enough space for the cache and the core */
	cache = (drccache *)drccache_alloc(CACHE_SIZE + sizeof(*rsp));
	if (cache == NULL)
	{
		fatalerror("Unable to allocate cache of size %d", (UINT32)(CACHE_SIZE + sizeof(*rsp)));
	}

	/* allocate the core memory */
	*(rsp_state **)device->token = rsp = (rsp_state *)drccache_memory_alloc_near(cache, sizeof(*rsp));
	memset(rsp, 0, sizeof(*rsp));

	rspcom_init(rsp, device, irqcallback);

	/* allocate the implementation-specific state from the full cache */
	rsp->impstate = (rspimp_state *)drccache_memory_alloc_near(cache, sizeof(*rsp->impstate));
	memset(rsp->impstate, 0, sizeof(*rsp->impstate));
	rsp->impstate->cache = cache;

	/* initialize the UML generator */
	if (FORCE_C_BACKEND)
	{
		flags |= DRCUML_OPTION_USE_C;
	}
	if (LOG_UML)
	{
		flags |= DRCUML_OPTION_LOG_UML;
	}
	if (LOG_NATIVE)
	{
		flags |= DRCUML_OPTION_LOG_NATIVE;
	}
	rsp->impstate->drcuml = drcuml_alloc(device, cache, flags, 8, 32, 2);
	if (rsp->impstate->drcuml == NULL)
	{
		fatalerror("Error initializing the UML");
	}

	/* add symbols for our stuff */
	drcuml_symbol_add(rsp->impstate->drcuml, &rsp->pc, sizeof(rsp->pc), "pc");
	drcuml_symbol_add(rsp->impstate->drcuml, &rsp->icount, sizeof(rsp->icount), "icount");
	for (regnum = 0; regnum < 32; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		drcuml_symbol_add(rsp->impstate->drcuml, &rsp->r[regnum], sizeof(rsp->r[regnum]), buf);
	}
	drcuml_symbol_add(rsp->impstate->drcuml, &rsp->impstate->arg0, sizeof(rsp->impstate->arg0), "arg0");
	drcuml_symbol_add(rsp->impstate->drcuml, &rsp->impstate->arg1, sizeof(rsp->impstate->arg1), "arg1");
	drcuml_symbol_add(rsp->impstate->drcuml, &rsp->impstate->arg64, sizeof(rsp->impstate->arg1), "arg64");
	drcuml_symbol_add(rsp->impstate->drcuml, &rsp->impstate->numcycles, sizeof(rsp->impstate->numcycles), "numcycles");

	/* initialize the front-end helper */
	if (SINGLE_INSTRUCTION_MODE)
	{
		feconfig.max_sequence = 1;
	}
	rsp->impstate->drcfe = drcfe_init(device, &feconfig, rsp);

	/* compute the register parameters */
	for (regnum = 0; regnum < 34; regnum++)
	{
		rsp->impstate->regmap[regnum].type = (regnum == 0) ? DRCUML_PTYPE_IMMEDIATE : DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[regnum].value = (regnum == 0) ? 0 : (FPTR)&rsp->r[regnum];
	}
	for (regnum = 0; regnum < 32; regnum++)
	{
		for(elnum = 0; elnum < 16; elnum++)
		{
			rsp->impstate->regmap[16*regnum+elnum+34].type = DRCUML_PTYPE_MEMORY;
			rsp->impstate->regmap[16*regnum+elnum+34].value = (FPTR)&rsp->v[regnum].b[15-elnum];
		}
		for(elnum = 0; elnum < 8; elnum++)
		{
			rsp->impstate->regmap[8*regnum+elnum+546].type = DRCUML_PTYPE_MEMORY;
			rsp->impstate->regmap[8*regnum+elnum+546].value = (FPTR)&rsp->v[regnum].s[7-elnum];
		}
		for(elnum = 0; elnum < 4; elnum++)
		{
			rsp->impstate->regmap[4*regnum+elnum+802].type = DRCUML_PTYPE_MEMORY;
			rsp->impstate->regmap[4*regnum+elnum+802].value = (FPTR)&rsp->v[regnum].l[3-elnum];
		}
	}
	for (regnum = 0; regnum < 4; regnum++)
	{
		rsp->impstate->regmap[930+regnum].type = DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[930+regnum].value = (FPTR)&rsp->flag[regnum];
	}
	for (regnum = 0; regnum < 8; regnum++)
	{
		rsp->impstate->regmap[934+regnum].type = DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[934+regnum].value = (FPTR)&rsp->accum[regnum].l;
		rsp->impstate->regmap[942+regnum].type = DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[942+regnum].value = (FPTR)&rsp->accum[regnum].h.high;
		rsp->impstate->regmap[950+regnum].type = DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[950+regnum].value = (FPTR)&rsp->accum[regnum].h.mid;
		rsp->impstate->regmap[958+regnum].type = DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[958+regnum].value = (FPTR)&rsp->accum[regnum].h.low;
		rsp->impstate->regmap[966+regnum].type = DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[966+regnum].value = (FPTR)&rsp->accum[regnum].h.z;
		rsp->impstate->regmap[974+regnum].type = DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[974+regnum].value = (FPTR)&rsp->accum[regnum].w.mh;
		rsp->impstate->regmap[982+regnum].type = DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[982+regnum].value = (FPTR)&rsp->accum[regnum].w.zl;
		rsp->impstate->regmap[990+regnum].type = DRCUML_PTYPE_MEMORY;
		rsp->impstate->regmap[990+regnum].value = (FPTR)&rsp->impstate->vres[regnum];
	}

	/* mark the cache dirty so it is updated on next execute */
	rsp->impstate->cache_dirty = TRUE;
}

static CPU_EXIT( rsp )
{
	rsp_state *rsp = get_safe_token(device);

	/* clean up the DRC */
	drcfe_exit(rsp->impstate->drcfe);
	drcuml_free(rsp->impstate->drcuml);
	drccache_free(rsp->impstate->cache);
}


static CPU_RESET( rsp )
{
	rsp_state *rsp = get_safe_token(device);
	rsp->nextpc = ~0;
}


static void cfunc_rsp_lbv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 00000 | IIII | Offset |
	// --------------------------------------------------
	//
	// Load 1 byte to vector byte index

	ea = (base) ? rsp->r[base] + offset : offset;
	W_VREG_B(dest, index, READ8(rsp, ea));
}

#if !(DRC_LSV)
static void cfunc_rsp_lsv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 00001 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads 2 bytes starting from vector byte index

	ea = (base) ? rsp->r[base] + (offset * 2) : (offset * 2);

	W_VREG_S(dest, index >> 1, READ16(rsp, ea));
}
#endif

#if !(DRC_LLV)
static void cfunc_rsp_llv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 00010 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads 4 bytes starting from vector byte index

	ea = (base) ? rsp->r[base] + (offset * 4) : (offset * 4);

	W_VREG_L(dest, index >> 2, READ32(rsp, ea));
}
#endif

#if !(DRC_LDV)
static void cfunc_rsp_ldv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 00011 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads 8 bytes starting from vector byte index

	ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);

	index >>= 2;
	W_VREG_L(dest, index, READ32(rsp, ea));
	W_VREG_L(dest, index + 1, READ32(rsp, ea + 4));
}
#endif

#if !(DRC_LQV)
static void cfunc_rsp_lqv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int i = 0;
	int end = 0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 00100 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads up to 16 bytes starting from vector byte index

	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	end = index + (16 - (ea & 0xf));
	if (end > 16) end = 16;

	for (i=index; i < end; i++)
	{
		W_VREG_B(dest, i, READ8(rsp, ea));
		ea++;
	}
}
#endif

static void cfunc_rsp_lrv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int i = 0;
	int end = 0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 00101 | IIII | Offset |
	// --------------------------------------------------
	//
	// Stores up to 16 bytes starting from right side until 16-byte boundary

	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	index = 16 - ((ea & 0xf) - index);
	end = 16;
	ea &= ~0xf;

	for (i=index; i < end; i++)
	{
		W_VREG_B(dest, i, READ8(rsp, ea));
		ea++;
	}
}

#if !(DRC_LPV)
static void cfunc_rsp_lpv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int i = 0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 00110 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads a byte as the upper 8 bits of each element

	ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);

	for (i=0; i < 8; i++)
	{
		W_VREG_S(dest, i, READ8(rsp, ea + (((16-index) + i) & 0xf)) << 8);
	}
}
#endif

#if !(DRC_LUV)
static void cfunc_rsp_luv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int i = 0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 00111 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads a byte as the bits 14-7 of each element

	ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);

	for (i=0; i < 8; i++)
	{
		W_VREG_S(dest, i, READ8(rsp, ea + (((16-index) + i) & 0xf)) << 7);
	}
}
#endif

static void cfunc_rsp_lhv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int i = 0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 01000 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads a byte as the bits 14-7 of each element, with 2-byte stride

	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	for (i=0; i < 8; i++)
	{
		W_VREG_S(dest, i, READ8(rsp, ea + (((16-index) + (i<<1)) & 0xf)) << 7);
	}
}

static void cfunc_rsp_lfv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int i = 0;
	int end = 0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 01001 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads a byte as the bits 14-7 of upper or lower quad, with 4-byte stride

	fatalerror("RSP: LFV\n");

	if (index & 0x7)	fatalerror("RSP: LFV: index = %d at %08X\n", index, rsp->ppc);

	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	// not sure what happens if 16-byte boundary is crossed...
	if ((ea & 0xf) > 0)	fatalerror("RSP: LFV: 16-byte boundary crossing at %08X, recheck this!\n", rsp->ppc);

	end = (index >> 1) + 4;

	for (i=index >> 1; i < end; i++)
	{
		W_VREG_S(dest, i, READ8(rsp, ea) << 7);
		ea += 4;
	}
}

static void cfunc_rsp_lwv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int i = 0;
	int end = 0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 01010 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads the full 128-bit vector starting from vector byte index and wrapping to index 0
	// after byte index 15

	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	// not sure what happens if 16-byte boundary is crossed...
	if ((ea & 0xf) > 0) fatalerror("RSP: LWV: 16-byte boundary crossing at %08X, recheck this!\n", rsp->ppc);

	end = (16 - index) + 16;

	for (i=(16 - index); i < end; i++)
	{
		W_VREG_B(dest, i & 0xf, READ8(rsp, ea));
		ea += 4;
	}
}

static void cfunc_rsp_ltv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int i = 0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);

	// 31       25      20      15      10     6        0
	// --------------------------------------------------
	// | 110010 | BBBBB | TTTTT | 01011 | IIII | Offset |
	// --------------------------------------------------
	//
	// Loads one element to maximum of 8 vectors, while incrementing element index

	// FIXME: has a small problem with odd indices

	int element;
	int vs = dest;
	int ve = dest + 8;
	if (ve > 32)
		ve = 32;

	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	element = 7 - (index >> 1);

	if (index & 1)	fatalerror("RSP: LTV: index = %d\n", index);

	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	ea = ((ea + 8) & ~0xf) + (index & 1);
	for (i=vs; i < ve; i++)
	{
		element = ((8 - (index >> 1) + (i-vs)) << 1);
		W_VREG_B(i, (element & 0xf), READ8(rsp, ea));
		W_VREG_B(i, ((element+1) & 0xf), READ8(rsp, ea+1));

		ea += 2;
	}
}

static int generate_lwc2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	int loopdest;
	UINT32 op = desc->opptr.l[0];
	int dest = (op >> 16) & 0x1f;
	//int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	//int skip;
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:		/* LBV */
			//UML_ADD(block, IREG(0), R32(RSREG), IMM(offset));
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lbv, rsp);
			return TRUE;
		case 0x01:		/* LSV */
#if (DRC_LSV)
			offset <<= 1;
			index >>= 1;

			index = 7 - index;
			UML_ADD(block, IREG(0), R32(RSREG), IMM(offset));						// add     i0,<rsreg>,offset
			UML_CALLH(block, rsp->impstate->read16);								// callh   read32
			UML_STORE(block, &rsp->v[dest].s[index], IMM(0), IREG(0), WORD);		// store   v[dest][index],i0,word
			return TRUE;
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lsv, rsp);
			return TRUE;
#endif
		case 0x02:		/* LLV */
#if (DRC_LLV)
			offset <<= 2;
			index >>= 2;

			UML_ADD(block, IREG(0), R32(RSREG), IMM(offset));						// add     i0,<rsreg>,offset
			UML_CALLH(block, rsp->impstate->read32);								// callh   read32
			UML_MOV(block, VLX(dest, index), IREG(0));								// mov     v[dest][index].i0
			return TRUE;
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_llv, rsp);
			return TRUE;
#endif
		case 0x03:		/* LDV */
#if (DRC_LDV)
			offset <<= 3;
			index >>= 2;

			UML_ADD(block, IREG(0), R32(RSREG), IMM(offset));						// add     i0,<rsreg>,offset
			UML_CALLH(block, rsp->impstate->read32);								// callh   read32
			UML_MOV(block, VLX(dest, index), IREG(0));								// mov     v[dest][index],i0

			UML_ADD(block, IREG(0), R32(RSREG), IMM(offset));						// add     i0,<rsreg>,offset
			UML_ADD(block, IREG(0), IREG(0), IMM(4));								// add     i0,i0,4
			UML_CALLH(block, rsp->impstate->read32);								// callh   read32
			UML_MOV(block, VLX(dest, index+1), IREG(0));							// mov     v[dest][index+1],i0
			return TRUE;
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ldv, rsp);
			return TRUE;
#endif
		case 0x04:		/* LQV */
#if (DRC_LQV)
			offset <<= 4;

			UML_ADD(block, MEM(&rsp->impstate->arg0), R32(RSREG), IMM(offset));		// mov     arg0,<rsreg>,offset
			UML_MOV(block, IREG(3), IMM(index));									// mov     i3,index
			UML_MOV(block, IREG(2), MEM(&rsp->impstate->arg0));						// mov     i2,arg0
			UML_AND(block, IREG(2), IREG(2), IMM(0x0000000f));						// and     i2,i2,0x0000000f
			UML_SUB(block, IREG(2), IMM(16), IREG(2));								// sub     i2,16,i2
			UML_ADD(block, IREG(2), IREG(2), IREG(3));								// add     i2,i2,i3
			UML_CMP(block, IREG(2), IMM(16));										// cmp     i2,16
			UML_TEST(block, IREG(2), IMM(0xfffffff0));								// test    i2,0xfffffff0
			UML_MOVc(block, IF_NZ, IREG(2), IMM(16));								// mov     NZ,i2,16

		UML_LABEL(block, loopdest = compiler->labelnum++);							// loopdest:
			UML_MOV(block, IREG(0), MEM(&rsp->impstate->arg0));						// mov     i0,arg0
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SUB(block, IREG(1), IMM(15), IREG(3));								// sub     i1,15,i3
			UML_STORE(block, &rsp->v[dest].b[0], IREG(1), IREG(0), BYTE);			// store   v[dest][0]+i1,i0

			UML_ADD(block, MEM(&rsp->impstate->arg0), MEM(&rsp->impstate->arg0), IMM(1));
			UML_AND(block, MEM(&rsp->impstate->arg0), MEM(&rsp->impstate->arg0), IMM(0x00000fff));
			UML_ADD(block, IREG(3), IREG(3), IMM(1));								// add     i3,i3,1
			UML_CMP(block, IREG(3), IREG(2));										// cmp     i3,i2
			UML_JMPc(block, IF_L, loopdest);										// jmp     L,loopdest
			return TRUE;

#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lqv, rsp);
			return TRUE;
#endif
		case 0x05:		/* LRV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lrv, rsp);
			return TRUE;
		case 0x06:		/* LPV */
#if (DRC_LPV)
			offset <<= 3;

			UML_ADD(block, IREG(2), R32(RSREG), IMM(offset));						// add     i2,<rsreg>,offset
			UML_SUB(block, IREG(3), IMM(16), IMM(index));							// sub     i3,16,index

			UML_AND(block, IREG(1), IREG(3), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));						// and     i0,i0,0x00000fff
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(8));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[7], IMM(0), IREG(0), WORD);			// store   v[dest][7],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(1));								// add     i1,i3,1
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));						// and     i0,i0,0x00000fff
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(8));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[6], IMM(0), IREG(0), WORD);			// store   v[dest][6],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(2));								// add     i1,i3,2
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));						// and     i0,i0,0x00000fff
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(8));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[5], IMM(0), IREG(0), WORD);			// store   v[dest][5],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(3));								// add     i1,i3,3
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));						// and     i0,i0,0x00000fff
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(8));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[4], IMM(0), IREG(0), WORD);			// store   v[dest][4],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(4));								// add     i1,i3,4
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));						// and     i0,i0,0x00000fff
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(8));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[3], IMM(0), IREG(0), WORD);			// store   v[dest][3],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(5));								// add     i1,i3,5
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));						// and     i0,i0,0x00000fff
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(8));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[2], IMM(0), IREG(0), WORD);			// store   v[dest][2],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(6));								// add     i1,i3,6
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));						// and     i0,i0,0x00000fff
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(8));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[1], IMM(0), IREG(0), WORD);			// store   v[dest][1],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(7));								// add     i1,i3,7
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));						// and     i0,i0,0x00000fff
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(8));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[0], IMM(0), IREG(0), WORD);			// store   v[dest][0],i0,word
			return TRUE;
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lpv, rsp);
			return TRUE;
#endif
		case 0x07:		/* LUV */
#if (DRC_LUV)
	/*
    ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);

    for (i=0; i < 8; i++)
    {
        W_VREG_S(dest, i, READ8(rsp, ea + (((16-index) + i) & 0xf)) << 7);
    }
    */
			offset <<= 3;

			UML_ADD(block, IREG(2), R32(RSREG), IMM(offset));						// add     i2,<rsreg>,offset
			UML_SUB(block, IREG(3), IMM(16), IMM(index));							// sub     i3,16,index

			UML_AND(block, IREG(1), IREG(3), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(7));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[7], IMM(0), IREG(0), WORD);			// store   v[dest][7],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(1));								// add     i1,i3,1
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(7));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[6], IMM(0), IREG(0), WORD);			// store   v[dest][6],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(2));								// add     i1,i3,2
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(7));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[5], IMM(0), IREG(0), WORD);			// store   v[dest][5],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(3));								// add     i1,i3,3
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(7));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[4], IMM(0), IREG(0), WORD);			// store   v[dest][4],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(4));								// add     i1,i3,4
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(7));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[3], IMM(0), IREG(0), WORD);			// store   v[dest][3],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(5));								// add     i1,i3,5
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(7));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[2], IMM(0), IREG(0), WORD);			// store   v[dest][2],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(6));								// add     i1,i3,6
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(7));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[1], IMM(0), IREG(0), WORD);			// store   v[dest][1],i0,word

			UML_ADD(block, IREG(1), IREG(3), IMM(7));								// add     i1,i3,7
			UML_AND(block, IREG(1), IREG(1), IMM(0x0000000f));						// and     i1,i1,0x0000000f
			UML_ADD(block, IREG(0), IREG(1), IREG(2));								// add     i0,i1,i2
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,bytexor
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);			// load    i0,dmem,i0,byte
			UML_SHL(block, IREG(0), IREG(0), IMM(7));								// shl     i0,i0,8
			UML_STORE(block, &rsp->v[dest].s[0], IMM(0), IREG(0), WORD);			// store   v[dest][0],i0,word
			return TRUE;
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_luv, rsp);
			return TRUE;
#endif
		case 0x08:		/* LHV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lhv, rsp);
			return TRUE;
		case 0x09:		/* LFV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lfv, rsp);
			return TRUE;
		case 0x0a:		/* LWV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lwv, rsp);
			return TRUE;
		case 0x0b:		/* LTV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ltv, rsp);
			return TRUE;

		default:
			return FALSE;
	}
}

static void cfunc_rsp_sbv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 1 byte from vector byte index

			ea = (base) ? rsp->r[base] + offset : offset;
			WRITE8(rsp, ea, R_VREG_B(dest, index));
}

#if !(DRC_SSV)
static void cfunc_rsp_ssv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 2 bytes starting from vector byte index

			ea = (base) ? rsp->r[base] + (offset * 2) : (offset * 2);

			WRITE16(rsp, ea, R_VREG_S(dest, index >> 1));
}
#endif

#if !(DRC_SLV)
static void cfunc_rsp_slv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 4 bytes starting from vector byte index

			ea = (base) ? rsp->r[base] + (offset * 4) : (offset * 4);

			WRITE32(rsp, ea, R_VREG_L(dest, index >> 2));
}
#endif

#if !(DRC_SDV)
static void cfunc_rsp_sdv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int end = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores 8 bytes starting from vector byte index
			ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);

			end = index + 8;

			index >>= 2;
			WRITE32(rsp, ea, R_VREG_L(dest, index));
			WRITE32(rsp, ea + 4, R_VREG_L(dest, index + 1));
}
#endif

#if !(DRC_SQV)
static void cfunc_rsp_sqv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int i = 0;
	int end = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00100 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from vector byte index until 16-byte boundary

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			end = index + (16 - (ea & 0xf));

			for (i=index; i < end; i++)
			{
				WRITE8(rsp, ea, R_VREG_B(dest, i & 0xf));
				ea++;
			}
}
#endif

static void cfunc_rsp_srv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int o;
	int i = 0;
	int end = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00101 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores up to 16 bytes starting from right side until 16-byte boundary

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			end = index + (ea & 0xf);
			o = (16 - (ea & 0xf)) & 0xf;
			ea &= ~0xf;

			for (i=index; i < end; i++)
			{
				WRITE8(rsp, ea, R_VREG_B(dest, ((i + o) & 0xf)));
				ea++;
			}
}

#if !(DRC_SPV)
static void cfunc_rsp_spv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int i = 0;
	int end = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00110 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores upper 8 bits of each element

			ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					WRITE8(rsp, ea, R_VREG_B(dest, ((i & 0xf) << 1)));
				}
				else
				{
					WRITE8(rsp, ea, R_VREG_S(dest, (i & 0x7)) >> 7);
				}
				ea++;
			}
}
#endif

static void cfunc_rsp_suv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int i = 0;
	int end = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 00111 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of each element

			ea = (base) ? rsp->r[base] + (offset * 8) : (offset * 8);
			end = index + 8;

			for (i=index; i < end; i++)
			{
				if ((i & 0xf) < 8)
				{
					WRITE8(rsp, ea, R_VREG_S(dest, (i & 0x7)) >> 7);
				}
				else
				{
					WRITE8(rsp, ea, R_VREG_B(dest, ((i & 0x7) << 1)));
				}
				ea++;
			}
}

static void cfunc_rsp_shv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int i = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01000 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of each element, with 2-byte stride

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			for (i=0; i < 8; i++)
			{
				UINT8 d = ((R_VREG_B(dest, ((index + (i << 1) + 0) & 0xf))) << 1) |
						  ((R_VREG_B(dest, ((index + (i << 1) + 1) & 0xf))) >> 7);

				WRITE8(rsp, ea, d);
				ea += 2;
			}
}

static void cfunc_rsp_sfv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int i = 0;
	int end = 0;
	int eaoffset = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01001 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores bits 14-7 of upper or lower quad, with 4-byte stride

			// FIXME: only works for index 0 and index 8

			if (index & 0x7)	mame_printf_debug("RSP: SFV: index = %d at %08X\n", index, rsp->ppc);

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = (index >> 1) + 4;

			for (i=index >> 1; i < end; i++)
			{
				WRITE8(rsp, ea + (eaoffset & 0xf), R_VREG_S(dest, i) >> 7);
				eaoffset += 4;
			}
}

static void cfunc_rsp_swv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int i = 0;
	int end = 0;
	int eaoffset = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01010 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores the full 128-bit vector starting from vector byte index and wrapping to index 0
			// after byte index 15

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			eaoffset = ea & 0xf;
			ea &= ~0xf;

			end = index + 16;

			for (i=index; i < end; i++)
			{
				WRITE8(rsp, ea + (eaoffset & 0xf), R_VREG_B(dest, i & 0xf));
				eaoffset++;
			}
}

static void cfunc_rsp_stv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	UINT32 ea = 0;
	int i = 0;
	int dest = (op >> 16) & 0x1f;
	int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	int element;
	int eaoffset = 0;
	int vs = dest;
	int ve = dest + 8;
	if (ve > 32)
		ve = 32;

	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}
			// 31       25      20      15      10     6        0
			// --------------------------------------------------
			// | 111010 | BBBBB | TTTTT | 01011 | IIII | Offset |
			// --------------------------------------------------
			//
			// Stores one element from maximum of 8 vectors, while incrementing element index

			element = 8 - (index >> 1);
			if (index & 0x1)	fatalerror("RSP: STV: index = %d at %08X\n", index, rsp->ppc);

			ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

			if (ea & 0x1)		fatalerror("RSP: STV: ea = %08X at %08X\n", ea, rsp->ppc);

			eaoffset = (ea & 0xf) + (element * 2);
			ea &= ~0xf;

			for (i=vs; i < ve; i++)
			{
				WRITE16(rsp, ea + (eaoffset & 0xf), R_VREG_S(i, element & 0x7));
				eaoffset += 2;
				element++;
			}
}

static int generate_swc2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
//  int loopdest;
	UINT32 op = desc->opptr.l[0];
	int dest = (op >> 16) & 0x1f;
	//int base = (op >> 21) & 0x1f;
	int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	//int skip;
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:		/* SBV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sbv, rsp);
			return TRUE;
		case 0x01:		/* SSV */
#if (DRC_SSV)
			offset <<= 1;
			index >>= 1;

			index = 7 - index;
			UML_ADD(block, IREG(0), R32(RSREG), IMM(offset));				// add     i0,<rsreg>,offset
			UML_LOAD(block, IREG(1), &rsp->v[dest].s[index], IMM(0), WORD);	// load    i1,v[dest].b[0],0,byte
			UML_CALLH(block, rsp->impstate->write16);						// callh   read32
			return TRUE;
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ssv, rsp);
			return TRUE;
#endif
		case 0x02:		/* SLV */
#if (DRC_SLV)
			offset <<= 2;
			index >>= 2;

			UML_ADD(block, IREG(0), R32(RSREG), IMM(offset));							// add     i0,<rsreg>,offset
			UML_MOV(block, IREG(1), VLX(dest, index));									// mov     i1,v[dest].l[index]
			UML_CALLH(block, rsp->impstate->write32);									// callh   read32
			return TRUE;
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_slv, rsp);
			return TRUE;
#endif
		case 0x03:		/* SDV */
#if (DRC_SDV)
			offset <<= 3;
			index >>= 2;

			UML_ADD(block, IREG(0), R32(RSREG), IMM(offset));							// add     i0,<rsreg>,offset
			UML_MOV(block, IREG(1), VLX(dest, index));									// mov     i1,v[dest].l[index]
			UML_CALLH(block, rsp->impstate->write32);									// callh   write32

			UML_ADD(block, IREG(0), R32(RSREG), IMM(offset));							// add     i0,<rsreg>,offset
			UML_ADD(block, IREG(0), IREG(0), IMM(4));									// add     i0,i0,4
			UML_MOV(block, IREG(1), VLX(dest, index+1));								// mov     i1,v[dest].l[index+1]
			UML_CALLH(block, rsp->impstate->write32);									// callh   write32
			return TRUE;
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sdv, rsp);
			return TRUE;
#endif
		case 0x04:		/* SQV */
#if (DRC_SQV)
			offset <<= 4;

			UML_ADD(block, MEM(&rsp->impstate->arg0), R32(RSREG), IMM(offset));		// mov     arg0,<rsreg>,offset
			UML_MOV(block, IREG(3), IMM(index));									// mov     i3,index
			UML_MOV(block, IREG(2), MEM(&rsp->impstate->arg0));						// mov     i2,arg0
			UML_AND(block, IREG(2), IREG(2), IMM(0x0000000f));						// and     i2,i2,0x0000000f
			UML_SUB(block, IREG(2), IMM(16), IREG(2));								// sub     i2,16,i2
			UML_ADD(block, IREG(2), IREG(2), IREG(3));								// add     i2,i2,i3
			UML_CMP(block, IREG(2), IMM(16));										// cmp     i2,16
			UML_TEST(block, IREG(2), IMM(0xfffffff0));								// test    i2,0xfffffff0
			UML_MOVc(block, IF_NZ, IREG(2), IMM(16));								// mov     NZ,i2,16

		UML_LABEL(block, loopdest = compiler->labelnum++);							// loopdest:
			UML_MOV(block, IREG(0), MEM(&rsp->impstate->arg0));						// mov     i0,arg0
			UML_XOR(block, IREG(0), IREG(0), IMM(BYTE4_XOR_BE(0)));					// xor     i0,i0,byte4xor
			UML_SUB(block, IREG(1), IMM(15), IREG(3));								// sub     i1,15,i3
			UML_LOAD(block, IREG(1), &rsp->v[dest].b[0], IREG(1), BYTE);			// load    i1,v[dest].b[0],i1,byte
			UML_STORE(block, rsp->impstate->dmem, IREG(0), IREG(1), BYTE);			// store   dmem,i0,i1,byte

			UML_ADD(block, MEM(&rsp->impstate->arg0), MEM(&rsp->impstate->arg0), IMM(1));
			UML_ADD(block, IREG(3), IREG(3), IMM(1));								// add     i3,i3,1
			UML_CMP(block, IREG(3), IREG(2));										// cmp     i3,i2
			UML_JMPc(block, IF_L, loopdest);										// jmp     L,loopdest
			return TRUE;
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sqv, rsp);
#endif
			return TRUE;
		case 0x05:		/* SRV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_srv, rsp);
			return TRUE;
		case 0x06:		/* SPV */
#if (DRC_SPV)
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_spv, rsp);
#endif
			return TRUE;
		case 0x07:		/* SUV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_suv, rsp);
			return TRUE;
		case 0x08:		/* SHV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_shv, rsp);
			return TRUE;
		case 0x09:		/* SFV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sfv, rsp);
			return TRUE;
		case 0x0a:		/* SWV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_swv, rsp);
			return TRUE;
		case 0x0b:		/* STV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_stv, rsp);
			return TRUE;

		default:
			unimplemented_opcode(rsp, op);
			return FALSE;
	}

	return TRUE;
}

#if (DRC_VMADN)
static void generate_saturate_accum_unsigned(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int accum)
{
	/*
    int skip, skip2;

    UML_CMP(block, VACCUMWMH(accum), IMM(-32768));
    UML_JMPc(block, IF_GE, skip = compiler->labelnum++);
    UML_MOV(block, IREG(0), IMM(0));
    UML_JMP(block, skip2 = compiler->labelnum++);

    UML_LABEL(block, skip);
    UML_CMP(block, VACCUMWMH(accum), IMM(32767));
    UML_JMPc(block, IF_L, skip = compiler->labelnum++);
    UML_MOV(block, IREG(0), IMM(0x0000ffff));
    UML_JMP(block, skip2);

    UML_LABEL(block, skip);
    UML_SEXT(block, IREG(0), VACCUMHL(accum), WORD);
    UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff));
    UML_LABEL(block, skip2);
    */
	UML_SEXT(block, IREG(0), VACCUMHL(accum), WORD);
	UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff));
	UML_CMP(block, VACCUMWMH(accum), IMM(-32768));
	UML_MOVc(block, IF_L, IREG(0), IMM(0));
	UML_CMP(block, VACCUMWMH(accum), IMM(32767));
	UML_MOVc(block, IF_GE, IREG(0), IMM(0x0000ffff));
}
#endif

INLINE UINT16 SATURATE_ACCUM_UNSIGNED(rsp_state *rsp, int accum)
{
	if (ACCUM(accum).w.mh < -32768)
	{
		return 0;
	}
	else if(ACCUM(accum).w.mh > 32767)
	{
		return 0xffff;
	}
	else
	{
		return ACCUM(accum).h.low;
	}
}

#if (DRC_VMADH) || (DRC_VMADM)
static void generate_saturate_accum_signed(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int accum)
{
	int skip, skip2;

	UML_CMP(block, VACCUMWMH(accum), IMM(-32768));
	UML_JMPc(block, IF_GE, skip = compiler->labelnum++);
	UML_MOV(block, IREG(0), IMM(0x00008000));
	UML_JMP(block, skip2 = compiler->labelnum++);

	UML_LABEL(block, skip);
	UML_CMP(block, VACCUMWMH(accum), IMM(32767));
	UML_JMPc(block, IF_L, skip = compiler->labelnum++);
	UML_MOV(block, IREG(0), IMM(0x00007fff));
	UML_JMP(block, skip2);

	UML_LABEL(block, skip);
	UML_SEXT(block, IREG(0), VACCUMHM(accum), WORD);
	UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff));
	UML_LABEL(block, skip2);
}
#endif

INLINE UINT16 SATURATE_ACCUM_SIGNED(rsp_state *rsp, int accum)
{
	if (ACCUM(accum).w.mh < -32768)
	{
		return 0x8000;
	}
	if(ACCUM(accum).w.mh > 32767)
	{
		return 0x7fff;
	}
	return ACCUM(accum).h.mid;
}

#define WRITEBACK_RESULT()					\
	do {									\
		W_VREG_S_X(VDREG, 7, vres[0]);			\
		W_VREG_S_X(VDREG, 6, vres[1]);			\
		W_VREG_S_X(VDREG, 5, vres[2]);			\
		W_VREG_S_X(VDREG, 4, vres[3]);			\
		W_VREG_S_X(VDREG, 3, vres[4]);			\
		W_VREG_S_X(VDREG, 2, vres[5]);			\
		W_VREG_S_X(VDREG, 1, vres[6]);			\
		W_VREG_S_X(VDREG, 0, vres[7]);			\
	} while(0)

#if 0
static float float_round(float input)
{
    INT32 integer = (INT32)input;
    float fraction = input - (float)integer;
    float output = 0.0f;
    if( fraction >= 0.5f )
    {
        output = (float)( integer + 1 );
    }
    else
    {
        output = (float)integer;
    }
    return output;
}
#endif

#define RSP_VMULF(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		INT16 v1, v2; \
		v1 = R_VREG_S_X(VS1REG, I0); v2 = R_VREG_S_X(VS2REG, E20); \
		if (v1 == -32768 && v2 == -32768) { ACCUM(E10).l = 0x0000800080000000LL; vres[E10] = 0x7fff; } \
		else { ACCUM(E10).l = ((INT64)((INT32)v1 * (INT32)v2) * 0x20000 + 0x80000000); vres[E10] = ACCUM(E10).h.mid; } \
		v1 = R_VREG_S_X(VS1REG, I1); v2 = R_VREG_S_X(VS2REG, E21); \
		if (v1 == -32768 && v2 == -32768) { ACCUM(E11).l = 0x0000800080000000LL; vres[E11] = 0x7fff; } \
		else { ACCUM(E11).l = ((INT64)((INT32)v1 * (INT32)v2) * 0x20000 + 0x80000000); vres[E11] = ACCUM(E11).h.mid; } \
		v1 = R_VREG_S_X(VS1REG, I2); v2 = R_VREG_S_X(VS2REG, E22); \
		if (v1 == -32768 && v2 == -32768) { ACCUM(E12).l = 0x0000800080000000LL; vres[E12] = 0x7fff; } \
		else { ACCUM(E12).l = ((INT64)((INT32)v1 * (INT32)v2) * 0x20000 + 0x80000000); vres[E12] = ACCUM(E12).h.mid; } \
		v1 = R_VREG_S_X(VS1REG, I3); v2 = R_VREG_S_X(VS2REG, E23); \
		if (v1 == -32768 && v2 == -32768) { ACCUM(E13).l = 0x0000800080000000LL; vres[E13] = 0x7fff; } \
		else { ACCUM(E13).l = ((INT64)((INT32)v1 * (INT32)v2) * 0x20000 + 0x80000000); vres[E13] = ACCUM(E13).h.mid; } \
		v1 = R_VREG_S_X(VS1REG, I4); v2 = R_VREG_S_X(VS2REG, E24); \
		if (v1 == -32768 && v2 == -32768) { ACCUM(E14).l = 0x0000800080000000LL; vres[E14] = 0x7fff; } \
		else { ACCUM(E14).l = ((INT64)((INT32)v1 * (INT32)v2) * 0x20000 + 0x80000000); vres[E14] = ACCUM(E14).h.mid; } \
		v1 = R_VREG_S_X(VS1REG, I5); v2 = R_VREG_S_X(VS2REG, E25); \
		if (v1 == -32768 && v2 == -32768) { ACCUM(E15).l = 0x0000800080000000LL; vres[E15] = 0x7fff; } \
		else { ACCUM(E15).l = ((INT64)((INT32)v1 * (INT32)v2) * 0x20000 + 0x80000000); vres[E15] = ACCUM(E15).h.mid; } \
		v1 = R_VREG_S_X(VS1REG, I6); v2 = R_VREG_S_X(VS2REG, E26); \
		if (v1 == -32768 && v2 == -32768) { ACCUM(E16).l = 0x0000800080000000LL; vres[E16] = 0x7fff; } \
		else { ACCUM(E16).l = ((INT64)((INT32)v1 * (INT32)v2) * 0x20000 + 0x80000000); vres[E16] = ACCUM(E16).h.mid; } \
		v1 = R_VREG_S_X(VS1REG, I7); v2 = R_VREG_S_X(VS2REG, E27); \
		if (v1 == -32768 && v2 == -32768) { ACCUM(E17).l = 0x0000800080000000LL; vres[E17] = 0x7fff; } \
		else { ACCUM(E17).l = ((INT64)((INT32)v1 * (INT32)v2) * 0x20000 + 0x80000000); vres[E17] = ACCUM(E17).h.mid; } \
	}

INLINE void cfunc_rsp_vmulf(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };
	//int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000000 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by signed integer * 2

	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMULF(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMULF(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMULF(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMULF(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMULF(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMULF(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMULF(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMULF(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMULF(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMULF(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMULF(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMULF(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMULF(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMULF(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMULF(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMULF(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
	}

	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vmulu(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000001 |
	// ------------------------------------------------------
	//

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT32 s1 = (INT32)(INT16)R_VREG_S(VS1REG, del);
		INT32 s2 = (INT32)(INT16)R_VREG_S(VS2REG, sel);
		INT64 r = s1 * s2;
		r += 0x4000;	// rounding ?

		ACCUM(del).l = r << 17;

		if (r < 0)
		{
			vres[del] = 0;
		}
		else if ((ACCUM(del).h.high ^ ACCUM(del).h.mid) < 0)
		{
			vres[del] = -1;

		}
		else
		{
			vres[del] = ACCUM(del).h.mid;
		}
	}
	WRITEBACK_RESULT();
}

#if (DRC_VMUDL)
/*------------------------------------------------------------------
    generate_vmudl
------------------------------------------------------------------*/

#define RSP_VMUDL_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(1), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E20), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DAND(block, IREG(0), IREG(0), IMM(0x00000000ffff0000)); \
		UML_DMOV(block, VACCUML(E10), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E21), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DAND(block, IREG(0), IREG(0), IMM(0x00000000ffff0000)); \
		UML_DMOV(block, VACCUML(E11), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E22), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DAND(block, IREG(0), IREG(0), IMM(0x00000000ffff0000)); \
		UML_DMOV(block, VACCUML(E12), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E23), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DAND(block, IREG(0), IREG(0), IMM(0x00000000ffff0000)); \
		UML_DMOV(block, VACCUML(E13), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E24), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DAND(block, IREG(0), IREG(0), IMM(0x00000000ffff0000)); \
		UML_DMOV(block, VACCUML(E14), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E25), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DAND(block, IREG(0), IREG(0), IMM(0x00000000ffff0000)); \
		UML_DMOV(block, VACCUML(E15), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E26), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DAND(block, IREG(0), IREG(0), IMM(0x00000000ffff0000)); \
		UML_DMOV(block, VACCUML(E16), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E27), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DAND(block, IREG(0), IREG(0), IMM(0x00000000ffff0000)); \
		UML_DMOV(block, VACCUML(E17), IREG(0)); \
 \
		UML_SEXT(block, IREG(0), VACCUMHL(E10), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHL(E11), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHL(E12), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHL(E13), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHL(E14), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHL(E15), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHL(E16), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHL(E17), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vmudl(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)	// vmudl
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by unsigned fraction
	// The result is added into accumulator
	// The middle slice of accumulator is stored into destination element
	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMUDL_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMUDL_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMUDL_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMUDL_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMUDL_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMUDL_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMUDL_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMUDL_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMUDL_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMUDL_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMUDL_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMUDL_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMUDL_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMUDL_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMUDL_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMUDL_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VMUDL(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	{ \
		ACCUM(E10).l = (((UINT32)(UINT16)R_VREG_S(VS1REG, E10) * (UINT32)(UINT16)R_VREG_S(VS2REG, E20)) & 0xffff0000); \
		ACCUM(E11).l = (((UINT32)(UINT16)R_VREG_S(VS1REG, E11) * (UINT32)(UINT16)R_VREG_S(VS2REG, E21)) & 0xffff0000); \
		ACCUM(E12).l = (((UINT32)(UINT16)R_VREG_S(VS1REG, E12) * (UINT32)(UINT16)R_VREG_S(VS2REG, E22)) & 0xffff0000); \
		ACCUM(E13).l = (((UINT32)(UINT16)R_VREG_S(VS1REG, E13) * (UINT32)(UINT16)R_VREG_S(VS2REG, E23)) & 0xffff0000); \
		ACCUM(E14).l = (((UINT32)(UINT16)R_VREG_S(VS1REG, E14) * (UINT32)(UINT16)R_VREG_S(VS2REG, E24)) & 0xffff0000); \
		ACCUM(E15).l = (((UINT32)(UINT16)R_VREG_S(VS1REG, E15) * (UINT32)(UINT16)R_VREG_S(VS2REG, E25)) & 0xffff0000); \
		ACCUM(E16).l = (((UINT32)(UINT16)R_VREG_S(VS1REG, E16) * (UINT32)(UINT16)R_VREG_S(VS2REG, E26)) & 0xffff0000); \
		ACCUM(E17).l = (((UINT32)(UINT16)R_VREG_S(VS1REG, E17) * (UINT32)(UINT16)R_VREG_S(VS2REG, E27)) & 0xffff0000); \
		vres[E10] = ACCUM(E10).h.low; \
		vres[E11] = ACCUM(E11).h.low; \
		vres[E12] = ACCUM(E12).h.low; \
		vres[E13] = ACCUM(E13).h.low; \
		vres[E14] = ACCUM(E14).h.low; \
		vres[E15] = ACCUM(E15).h.low; \
		vres[E16] = ACCUM(E16).h.low; \
		vres[E17] = ACCUM(E17).h.low; \
		W_VREG_S_X(VDREG, 7, vres[0]);			\
		W_VREG_S_X(VDREG, 6, vres[1]);			\
		W_VREG_S_X(VDREG, 5, vres[2]);			\
		W_VREG_S_X(VDREG, 4, vres[3]);			\
		W_VREG_S_X(VDREG, 3, vres[4]);			\
		W_VREG_S_X(VDREG, 2, vres[5]);			\
		W_VREG_S_X(VDREG, 1, vres[6]);			\
		W_VREG_S_X(VDREG, 0, vres[7]);			\
	}

INLINE void cfunc_rsp_vmudl(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001101 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by unsigned fraction
	// The result is added into accumulator
	// The middle slice of accumulator is stored into destination element

	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMUDL(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMUDL(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMUDL(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMUDL(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMUDL(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMUDL(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMUDL(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMUDL(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMUDL(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMUDL(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMUDL(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMUDL(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMUDL(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMUDL(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMUDL(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMUDL(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
}
#endif

#if (DRC_VMUDM)
/*------------------------------------------------------------------
    generate_vmudm
------------------------------------------------------------------*/

#define RSP_VMUDM_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(1), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E20), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DMOV(block, VACCUML(E10), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E21), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DMOV(block, VACCUML(E11), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E22), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DMOV(block, VACCUML(E12), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E23), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DMOV(block, VACCUML(E13), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E24), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DMOV(block, VACCUML(E14), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E25), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DMOV(block, VACCUML(E15), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E26), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DMOV(block, VACCUML(E16), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E27), WORD); \
		UML_AND(block, IREG(0), IREG(0), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DMOV(block, VACCUML(E17), IREG(0)); \
 \
		UML_SEXT(block, IREG(0), VACCUMHM(E10), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHM(E11), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHM(E12), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHM(E13), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHM(E14), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHM(E15), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHM(E16), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		UML_SEXT(block, IREG(0), VACCUMHM(E17), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vmudm(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)	// vmudl
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by unsigned fraction
	// The result is stored into accumulator
	// The middle slice of accumulator is stored into destination element
	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMUDM_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMUDM_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMUDM_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMUDM_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMUDM_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMUDM_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMUDM_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMUDM_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMUDM_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMUDM_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMUDM_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMUDM_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMUDM_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMUDM_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMUDM_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMUDM_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VMUDM(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		ACCUM(E10).l = (INT64)((INT32)(INT16)R_VREG_S_X(VS1REG, I0) * (UINT16)R_VREG_S_X(VS2REG, E20)) << 16; \
		ACCUM(E11).l = (INT64)((INT32)(INT16)R_VREG_S_X(VS1REG, I1) * (UINT16)R_VREG_S_X(VS2REG, E21)) << 16; \
		ACCUM(E12).l = (INT64)((INT32)(INT16)R_VREG_S_X(VS1REG, I2) * (UINT16)R_VREG_S_X(VS2REG, E22)) << 16; \
		ACCUM(E13).l = (INT64)((INT32)(INT16)R_VREG_S_X(VS1REG, I3) * (UINT16)R_VREG_S_X(VS2REG, E23)) << 16; \
		ACCUM(E14).l = (INT64)((INT32)(INT16)R_VREG_S_X(VS1REG, I4) * (UINT16)R_VREG_S_X(VS2REG, E24)) << 16; \
		ACCUM(E15).l = (INT64)((INT32)(INT16)R_VREG_S_X(VS1REG, I5) * (UINT16)R_VREG_S_X(VS2REG, E25)) << 16; \
		ACCUM(E16).l = (INT64)((INT32)(INT16)R_VREG_S_X(VS1REG, I6) * (UINT16)R_VREG_S_X(VS2REG, E26)) << 16; \
		ACCUM(E17).l = (INT64)((INT32)(INT16)R_VREG_S_X(VS1REG, I7) * (UINT16)R_VREG_S_X(VS2REG, E27)) << 16; \
		vres[E10] = ACCUM(E10).h.mid; \
		vres[E11] = ACCUM(E11).h.mid; \
		vres[E12] = ACCUM(E12).h.mid; \
		vres[E13] = ACCUM(E13).h.mid; \
		vres[E14] = ACCUM(E14).h.mid; \
		vres[E15] = ACCUM(E15).h.mid; \
		vres[E16] = ACCUM(E16).h.mid; \
		vres[E17] = ACCUM(E17).h.mid; \
		W_VREG_S_X(VDREG, 7, vres[0]);			\
		W_VREG_S_X(VDREG, 6, vres[1]);			\
		W_VREG_S_X(VDREG, 5, vres[2]);			\
		W_VREG_S_X(VDREG, 4, vres[3]);			\
		W_VREG_S_X(VDREG, 3, vres[4]);			\
		W_VREG_S_X(VDREG, 2, vres[5]);			\
		W_VREG_S_X(VDREG, 1, vres[6]);			\
		W_VREG_S_X(VDREG, 0, vres[7]);			\
	}

INLINE void cfunc_rsp_vmudm(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };
	//int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000101 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by unsigned fraction
	// The result is stored into accumulator
	// The middle slice of accumulator is stored into destination element

	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMUDM(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMUDM(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMUDM(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMUDM(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMUDM(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMUDM(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMUDM(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMUDM(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMUDM(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMUDM(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMUDM(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMUDM(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMUDM(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMUDM(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMUDM(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMUDM(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
	}

	/*
    for (i=0; i < 8; i++)
    {
        int del = VEC_EL_1(EL, i);
        int sel = VEC_EL_2(EL, del);
        INT32 s1 = (INT32)(INT16)R_VREG_S(VS1REG, del);
        INT32 s2 = (UINT16)R_VREG_S(VS2REG, sel);   // not sign-extended
        INT32 r =  s1 * s2;

        W_ACCUM_H(del, (r < 0) ? 0xffff : 0);       // sign-extend to 48-bit
        W_ACCUM_M(del, (INT16)(r >> 16));
        W_ACCUM_L(del, (UINT16)(r));

        vres[del] = ACCUM(del).h.mid;
    }
    WRITEBACK_RESULT();
    */
}
#endif

#define RSP_VMUDN(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	{ \
		ACCUM(E10).l = ((UINT16)R_VREG_S(VS1REG, E10) * (INT64)(INT16)R_VREG_S(VS2REG, E20)) << 16; \
		ACCUM(E11).l = ((UINT16)R_VREG_S(VS1REG, E11) * (INT64)(INT16)R_VREG_S(VS2REG, E21)) << 16; \
		ACCUM(E12).l = ((UINT16)R_VREG_S(VS1REG, E12) * (INT64)(INT16)R_VREG_S(VS2REG, E22)) << 16; \
		ACCUM(E13).l = ((UINT16)R_VREG_S(VS1REG, E13) * (INT64)(INT16)R_VREG_S(VS2REG, E23)) << 16; \
		ACCUM(E14).l = ((UINT16)R_VREG_S(VS1REG, E14) * (INT64)(INT16)R_VREG_S(VS2REG, E24)) << 16; \
		ACCUM(E15).l = ((UINT16)R_VREG_S(VS1REG, E15) * (INT64)(INT16)R_VREG_S(VS2REG, E25)) << 16; \
		ACCUM(E16).l = ((UINT16)R_VREG_S(VS1REG, E16) * (INT64)(INT16)R_VREG_S(VS2REG, E26)) << 16; \
		ACCUM(E17).l = ((UINT16)R_VREG_S(VS1REG, E17) * (INT64)(INT16)R_VREG_S(VS2REG, E27)) << 16; \
		W_VREG_S(VDREG, E10, ACCUM(E10).h.low);			\
		W_VREG_S(VDREG, E11, ACCUM(E11).h.low);			\
		W_VREG_S(VDREG, E12, ACCUM(E12).h.low);			\
		W_VREG_S(VDREG, E13, ACCUM(E13).h.low);			\
		W_VREG_S(VDREG, E14, ACCUM(E14).h.low);			\
		W_VREG_S(VDREG, E15, ACCUM(E15).h.low);			\
		W_VREG_S(VDREG, E16, ACCUM(E16).h.low);			\
		W_VREG_S(VDREG, E17, ACCUM(E17).h.low);			\
	}

INLINE void cfunc_rsp_vmudn(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000110 |
	// ------------------------------------------------------
	//
	// Multiplies unsigned fraction by signed integer
	// The result is stored into accumulator
	// The low slice of accumulator is stored into destination element

	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMUDN(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMUDN(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMUDN(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMUDN(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMUDN(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMUDN(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMUDN(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMUDN(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMUDN(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMUDN(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMUDN(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMUDN(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMUDN(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMUDN(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMUDN(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMUDN(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
}

INLINE void cfunc_rsp_vmudh(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000111 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by signed integer
	// The result is stored into highest 32 bits of accumulator, the low slice is zero
	// The highest 32 bits of accumulator is saturated into destination element

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT32 s1 = (INT32)(INT16)R_VREG_S(VS1REG, del);
		INT32 s2 = (INT32)(INT16)R_VREG_S(VS2REG, sel);
		INT32 r = s1 * s2;

		W_ACCUM_H(del, (INT16)(r >> 16));
		W_ACCUM_M(del, (UINT16)(r));
		W_ACCUM_L(del, 0);

		if (r < -32768) r = -32768;
		if (r >  32767)	r = 32767;
		vres[del] = (INT16)(r);
	}
	WRITEBACK_RESULT();
}

#define RSP_VMACF(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	{ \
		ACCUM(E10).l += (INT64)((INT32)(INT16)R_VREG_S(VS1REG, E10) * (INT32)(INT16)R_VREG_S(VS2REG, E20)) << 17; \
		ACCUM(E11).l += (INT64)((INT32)(INT16)R_VREG_S(VS1REG, E11) * (INT32)(INT16)R_VREG_S(VS2REG, E21)) << 17; \
		ACCUM(E12).l += (INT64)((INT32)(INT16)R_VREG_S(VS1REG, E12) * (INT32)(INT16)R_VREG_S(VS2REG, E22)) << 17; \
		ACCUM(E13).l += (INT64)((INT32)(INT16)R_VREG_S(VS1REG, E13) * (INT32)(INT16)R_VREG_S(VS2REG, E23)) << 17; \
		ACCUM(E14).l += (INT64)((INT32)(INT16)R_VREG_S(VS1REG, E14) * (INT32)(INT16)R_VREG_S(VS2REG, E24)) << 17; \
		ACCUM(E15).l += (INT64)((INT32)(INT16)R_VREG_S(VS1REG, E15) * (INT32)(INT16)R_VREG_S(VS2REG, E25)) << 17; \
		ACCUM(E16).l += (INT64)((INT32)(INT16)R_VREG_S(VS1REG, E16) * (INT32)(INT16)R_VREG_S(VS2REG, E26)) << 17; \
		ACCUM(E17).l += (INT64)((INT32)(INT16)R_VREG_S(VS1REG, E17) * (INT32)(INT16)R_VREG_S(VS2REG, E27)) << 17; \
		vres[E10] = SATURATE_ACCUM_SIGNED(rsp, E10); \
		vres[E11] = SATURATE_ACCUM_SIGNED(rsp, E11); \
		vres[E12] = SATURATE_ACCUM_SIGNED(rsp, E12); \
		vres[E13] = SATURATE_ACCUM_SIGNED(rsp, E13); \
		vres[E14] = SATURATE_ACCUM_SIGNED(rsp, E14); \
		vres[E15] = SATURATE_ACCUM_SIGNED(rsp, E15); \
		vres[E16] = SATURATE_ACCUM_SIGNED(rsp, E16); \
		vres[E17] = SATURATE_ACCUM_SIGNED(rsp, E17); \
		W_VREG_S_X(VDREG, 7, vres[0]);			\
		W_VREG_S_X(VDREG, 6, vres[1]);			\
		W_VREG_S_X(VDREG, 5, vres[2]);			\
		W_VREG_S_X(VDREG, 4, vres[3]);			\
		W_VREG_S_X(VDREG, 3, vres[4]);			\
		W_VREG_S_X(VDREG, 2, vres[5]);			\
		W_VREG_S_X(VDREG, 1, vres[6]);			\
		W_VREG_S_X(VDREG, 0, vres[7]);			\
	}

INLINE void cfunc_rsp_vmacf(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	//int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001000 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by signed integer * 2
	// The result is added to accumulator

	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMACF(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMACF(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMACF(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMACF(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMACF(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMACF(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMACF(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMACF(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMACF(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMACF(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMACF(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMACF(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMACF(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMACF(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMACF(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMACF(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
#if 0
	for (i=0; i < 8; i++)
	{
		UINT16 res;
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT32 s1 = (INT32)(INT16)R_VREG_S(VS1REG, del);
		INT32 s2 = (INT32)(INT16)R_VREG_S(VS2REG, sel);
		INT32 r = s1 * s2;

		ACCUM(del).l += (INT64)(r) << 17;
		res = SATURATE_ACCUM_SIGNED(rsp, del);

		vres[del] = res;
	}
	WRITEBACK_RESULT();
#endif
}

INLINE void cfunc_rsp_vmacu(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001001 |
	// ------------------------------------------------------
	//

	for (i=0; i < 8; i++)
	{
		UINT16 res;
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT32 s1 = (INT32)(INT16)R_VREG_S(VS1REG, del);
		INT32 s2 = (INT32)(INT16)R_VREG_S(VS2REG, sel);
		INT32 r1 = s1 * s2;
		UINT32 r2 = (UINT16)ACCUM(del).h.low + ((UINT16)(r1) * 2);
		UINT32 r3 = (UINT16)ACCUM(del).h.mid + (UINT16)((r1 >> 16) * 2) + (UINT16)(r2 >> 16);

		W_ACCUM_L(del, (UINT16)(r2));
		W_ACCUM_M(del, (UINT16)(r3));
		W_ACCUM_H(del, (UINT16)ACCUM(del).h.high + (UINT16)(r3 >> 16) + (UINT16)(r1 >> 31));

		if (ACCUM(del).h.high < 0)
		{
			res = 0;
		}
		else
		{
			if (ACCUM(del).h.high != 0)
			{
				res = 0xffff;
			}
			else
			{
				if (ACCUM(del).h.mid < 0)
				{
					res = 0xffff;
				}
				else
				{
					res = ACCUM(del).h.mid;
				}
			}
		}

		vres[del] = res;
	}
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vmadl(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001100 |
	// ------------------------------------------------------
	//
	// Multiplies unsigned fraction by unsigned fraction
	// Adds the higher 16 bits of the 32-bit result to accumulator
	// The low slice of accumulator is stored into destination element

	for (i=0; i < 8; i++)
	{
		UINT16 res;
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		UINT32 s1 = (UINT32)(UINT16)R_VREG_S(VS1REG, del);
		UINT32 s2 = (UINT32)(UINT16)R_VREG_S(VS2REG, sel);
		UINT32 r1 = s1 * s2;
		UINT32 r2 = (UINT16)ACCUM(del).h.low + (r1 >> 16);
		UINT32 r3 = (UINT16)ACCUM(del).h.mid + (r2 >> 16);

		W_ACCUM_L(del, (UINT16)(r2));
		W_ACCUM_M(del, (UINT16)(r3));
		W_ACCUM_H(del, ACCUM(del).h.high + (INT16)(r3 >> 16));

		res = SATURATE_ACCUM_UNSIGNED(rsp, del);

		vres[del] = res;
	}
	WRITEBACK_RESULT();
}

#if (DRC_VMADM)
/*------------------------------------------------------------------
    generate_vmadm
------------------------------------------------------------------*/

#define RSP_VMADM_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(0), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E20), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E10), VACCUML(E10), IREG(0)); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E21), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E11), VACCUML(E11), IREG(0)); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E22), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E12), VACCUML(E12), IREG(0)); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E23), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E13), VACCUML(E13), IREG(0)); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E24), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E14), VACCUML(E14), IREG(0)); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E25), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E15), VACCUML(E15), IREG(0)); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E26), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E16), VACCUML(E16), IREG(0)); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E27), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E17), VACCUML(E17), IREG(0)); \
 \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E10); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E11); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E12); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E13); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E14); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E15); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E16); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E17); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vmadm(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)	// vmadm
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by unsigned fraction
	// The result is added into accumulator
	// The middle slice of accumulator is stored into destination element
	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMADM_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMADM_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMADM_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMADM_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMADM_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMADM_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMADM_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMADM_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMADM_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMADM_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMADM_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMADM_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMADM_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMADM_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMADM_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMADM_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VMADM(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	{ \
		ACCUM(E10).l += ((INT64)(INT16)R_VREG_S(VS1REG, E10) * (UINT16)R_VREG_S(VS2REG, E20)) << 16; \
		ACCUM(E11).l += ((INT64)(INT16)R_VREG_S(VS1REG, E11) * (UINT16)R_VREG_S(VS2REG, E21)) << 16; \
		ACCUM(E12).l += ((INT64)(INT16)R_VREG_S(VS1REG, E12) * (UINT16)R_VREG_S(VS2REG, E22)) << 16; \
		ACCUM(E13).l += ((INT64)(INT16)R_VREG_S(VS1REG, E13) * (UINT16)R_VREG_S(VS2REG, E23)) << 16; \
		ACCUM(E14).l += ((INT64)(INT16)R_VREG_S(VS1REG, E14) * (UINT16)R_VREG_S(VS2REG, E24)) << 16; \
		ACCUM(E15).l += ((INT64)(INT16)R_VREG_S(VS1REG, E15) * (UINT16)R_VREG_S(VS2REG, E25)) << 16; \
		ACCUM(E16).l += ((INT64)(INT16)R_VREG_S(VS1REG, E16) * (UINT16)R_VREG_S(VS2REG, E26)) << 16; \
		ACCUM(E17).l += ((INT64)(INT16)R_VREG_S(VS1REG, E17) * (UINT16)R_VREG_S(VS2REG, E27)) << 16; \
		vres[E10] = SATURATE_ACCUM_SIGNED(rsp, E10); \
		vres[E11] = SATURATE_ACCUM_SIGNED(rsp, E11); \
		vres[E12] = SATURATE_ACCUM_SIGNED(rsp, E12); \
		vres[E13] = SATURATE_ACCUM_SIGNED(rsp, E13); \
		vres[E14] = SATURATE_ACCUM_SIGNED(rsp, E14); \
		vres[E15] = SATURATE_ACCUM_SIGNED(rsp, E15); \
		vres[E16] = SATURATE_ACCUM_SIGNED(rsp, E16); \
		vres[E17] = SATURATE_ACCUM_SIGNED(rsp, E17); \
		W_VREG_S_X(VDREG, 7, vres[0]);			\
		W_VREG_S_X(VDREG, 6, vres[1]);			\
		W_VREG_S_X(VDREG, 5, vres[2]);			\
		W_VREG_S_X(VDREG, 4, vres[3]);			\
		W_VREG_S_X(VDREG, 3, vres[4]);			\
		W_VREG_S_X(VDREG, 2, vres[5]);			\
		W_VREG_S_X(VDREG, 1, vres[6]);			\
		W_VREG_S_X(VDREG, 0, vres[7]);			\
	}

INLINE void cfunc_rsp_vmadm(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001101 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by unsigned fraction
	// The result is added into accumulator
	// The middle slice of accumulator is stored into destination element

	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMADM(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMADM(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMADM(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMADM(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMADM(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMADM(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMADM(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMADM(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMADM(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMADM(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMADM(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMADM(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMADM(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMADM(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMADM(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMADM(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
}
#endif

#if (DRC_VMADN)
/*------------------------------------------------------------------
    generate_vmadn
------------------------------------------------------------------*/

#define RSP_VMADN_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(1), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E20), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E10), VACCUML(E10), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E21), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E11), VACCUML(E11), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E22), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E12), VACCUML(E12), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E23), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E13), VACCUML(E13), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E24), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E14), VACCUML(E14), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E25), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E15), VACCUML(E15), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E26), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E16), VACCUML(E16), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E27), WORD); \
		UML_AND(block, IREG(1), IREG(1), IMM(0x0000ffff)); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_DSEXT(block, IREG(0), IREG(0), DWORD); \
		UML_DSHL(block, IREG(0), IREG(0), IMM(16)); \
		UML_DADD(block, VACCUML(E17), VACCUML(E17), IREG(0)); \
 \
		generate_saturate_accum_unsigned(rsp, block, compiler, desc, E10); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_unsigned(rsp, block, compiler, desc, E11); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_unsigned(rsp, block, compiler, desc, E12); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_unsigned(rsp, block, compiler, desc, E13); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_unsigned(rsp, block, compiler, desc, E14); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_unsigned(rsp, block, compiler, desc, E15); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_unsigned(rsp, block, compiler, desc, E16); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_unsigned(rsp, block, compiler, desc, E17); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vmadn(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)	// vmadn
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
	// ------------------------------------------------------
	//
	// Multiplies unsigned fraction by signed integer
	// The result is added into accumulator
	// The low slice of accumulator is stored into destination element
	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMADN_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMADN_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMADN_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMADN_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMADN_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMADN_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMADN_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMADN_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMADN_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMADN_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMADN_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMADN_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMADN_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMADN_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMADN_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMADN_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VMADN(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		ACCUM(E10).l += ((UINT16)R_VREG_S_X(VS1REG, I0) * (INT64)R_VREG_S_X(VS2REG, E20)) << 16; \
		ACCUM(E11).l += ((UINT16)R_VREG_S_X(VS1REG, I1) * (INT64)R_VREG_S_X(VS2REG, E21)) << 16; \
		ACCUM(E12).l += ((UINT16)R_VREG_S_X(VS1REG, I2) * (INT64)R_VREG_S_X(VS2REG, E22)) << 16; \
		ACCUM(E13).l += ((UINT16)R_VREG_S_X(VS1REG, I3) * (INT64)R_VREG_S_X(VS2REG, E23)) << 16; \
		ACCUM(E14).l += ((UINT16)R_VREG_S_X(VS1REG, I4) * (INT64)R_VREG_S_X(VS2REG, E24)) << 16; \
		ACCUM(E15).l += ((UINT16)R_VREG_S_X(VS1REG, I5) * (INT64)R_VREG_S_X(VS2REG, E25)) << 16; \
		ACCUM(E16).l += ((UINT16)R_VREG_S_X(VS1REG, I6) * (INT64)R_VREG_S_X(VS2REG, E26)) << 16; \
		ACCUM(E17).l += ((UINT16)R_VREG_S_X(VS1REG, I7) * (INT64)R_VREG_S_X(VS2REG, E27)) << 16; \
		vres[E10] = SATURATE_ACCUM_UNSIGNED(rsp, E10); \
		vres[E11] = SATURATE_ACCUM_UNSIGNED(rsp, E11); \
		vres[E12] = SATURATE_ACCUM_UNSIGNED(rsp, E12); \
		vres[E13] = SATURATE_ACCUM_UNSIGNED(rsp, E13); \
		vres[E14] = SATURATE_ACCUM_UNSIGNED(rsp, E14); \
		vres[E15] = SATURATE_ACCUM_UNSIGNED(rsp, E15); \
		vres[E16] = SATURATE_ACCUM_UNSIGNED(rsp, E16); \
		vres[E17] = SATURATE_ACCUM_UNSIGNED(rsp, E17); \
		W_VREG_S_X(VDREG, 7, vres[0]);			\
		W_VREG_S_X(VDREG, 6, vres[1]);			\
		W_VREG_S_X(VDREG, 5, vres[2]);			\
		W_VREG_S_X(VDREG, 4, vres[3]);			\
		W_VREG_S_X(VDREG, 3, vres[4]);			\
		W_VREG_S_X(VDREG, 2, vres[5]);			\
		W_VREG_S_X(VDREG, 1, vres[6]);			\
		W_VREG_S_X(VDREG, 0, vres[7]);			\
	}

INLINE void cfunc_rsp_vmadn(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
	// ------------------------------------------------------
	//
	// Multiplies unsigned fraction by signed integer
	// The result is added into accumulator
	// The low slice of accumulator is stored into destination element
	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMADN(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMADN(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMADN(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMADN(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMADN(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMADN(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMADN(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMADN(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMADN(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMADN(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMADN(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMADN(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMADN(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMADN(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMADN(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMADN(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
	}
}
#endif

#if (DRC_VMADH)
/*------------------------------------------------------------------
    generate_vmadh
------------------------------------------------------------------*/

#define RSP_VMADH_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(1), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E20), WORD); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_ADD(block, VACCUMWMH(E10), VACCUMWMH(E10), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E21), WORD); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_ADD(block, VACCUMWMH(E11), VACCUMWMH(E11), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E22), WORD); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_ADD(block, VACCUMWMH(E12), VACCUMWMH(E12), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E23), WORD); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_ADD(block, VACCUMWMH(E13), VACCUMWMH(E13), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E24), WORD); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_ADD(block, VACCUMWMH(E14), VACCUMWMH(E14), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E25), WORD); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_ADD(block, VACCUMWMH(E15), VACCUMWMH(E15), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E26), WORD); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_ADD(block, VACCUMWMH(E16), VACCUMWMH(E16), IREG(0)); \
 \
		UML_SEXT(block, IREG(1), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(0), VS(VS2REG, E27), WORD); \
		UML_MULS(block, IREG(0), IREG(1), IREG(0), IREG(1)); \
		UML_ADD(block, VACCUMWMH(E17), VACCUMWMH(E17), IREG(0)); \
 \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E10); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E11); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E12); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E13); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E14); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E15); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E16); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		generate_saturate_accum_signed(rsp, block, compiler, desc, E17); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vmadh(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)	// vmadh
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
	// ------------------------------------------------------
	//
	// Multiplies unsigned fraction by signed integer
	// The result is added into accumulator
	// The low slice of accumulator is stored into destination element
	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMADH_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMADH_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMADH_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMADH_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMADH_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMADH_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMADH_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMADH_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMADH_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMADH_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMADH_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMADH_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMADH_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMADH_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMADH_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMADH_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VMADH(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		ACCUM(E10).w.mh += (INT32)R_VREG_S_X(VS1REG, I0) * (INT32)R_VREG_S_X(VS2REG, E20); \
		ACCUM(E11).w.mh += (INT32)R_VREG_S_X(VS1REG, I1) * (INT32)R_VREG_S_X(VS2REG, E21); \
		ACCUM(E12).w.mh += (INT32)R_VREG_S_X(VS1REG, I2) * (INT32)R_VREG_S_X(VS2REG, E22); \
		ACCUM(E13).w.mh += (INT32)R_VREG_S_X(VS1REG, I3) * (INT32)R_VREG_S_X(VS2REG, E23); \
		ACCUM(E14).w.mh += (INT32)R_VREG_S_X(VS1REG, I4) * (INT32)R_VREG_S_X(VS2REG, E24); \
		ACCUM(E15).w.mh += (INT32)R_VREG_S_X(VS1REG, I5) * (INT32)R_VREG_S_X(VS2REG, E25); \
		ACCUM(E16).w.mh += (INT32)R_VREG_S_X(VS1REG, I6) * (INT32)R_VREG_S_X(VS2REG, E26); \
		ACCUM(E17).w.mh += (INT32)R_VREG_S_X(VS1REG, I7) * (INT32)R_VREG_S_X(VS2REG, E27); \
		vres[E10] = SATURATE_ACCUM_SIGNED(rsp, E10); \
		vres[E11] = SATURATE_ACCUM_SIGNED(rsp, E11); \
		vres[E12] = SATURATE_ACCUM_SIGNED(rsp, E12); \
		vres[E13] = SATURATE_ACCUM_SIGNED(rsp, E13); \
		vres[E14] = SATURATE_ACCUM_SIGNED(rsp, E14); \
		vres[E15] = SATURATE_ACCUM_SIGNED(rsp, E15); \
		vres[E16] = SATURATE_ACCUM_SIGNED(rsp, E16); \
		vres[E17] = SATURATE_ACCUM_SIGNED(rsp, E17); \
		W_VREG_S_X(VDREG, 7, vres[0]);			\
		W_VREG_S_X(VDREG, 6, vres[1]);			\
		W_VREG_S_X(VDREG, 5, vres[2]);			\
		W_VREG_S_X(VDREG, 4, vres[3]);			\
		W_VREG_S_X(VDREG, 3, vres[4]);			\
		W_VREG_S_X(VDREG, 2, vres[5]);			\
		W_VREG_S_X(VDREG, 1, vres[6]);			\
		W_VREG_S_X(VDREG, 0, vres[7]);			\
	}

INLINE void cfunc_rsp_vmadh(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001111 |
	// ------------------------------------------------------
	//
	// Multiplies signed integer by signed integer
	// The result is added into highest 32 bits of accumulator, the low slice is zero
	// The highest 32 bits of accumulator is saturated into destination element

	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMADH(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMADH(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMADH(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMADH(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMADH(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMADH(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMADH(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMADH(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMADH(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMADH(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMADH(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMADH(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMADH(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMADH(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMADH(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMADH(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
	}
}
#endif

#if (DRC_VADD)
/*------------------------------------------------------------------
    generate_vadd
------------------------------------------------------------------*/

#define RSP_VADD_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		int inrange, outofrange; \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E20), WORD); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_SEXT(block, IREG(1), VFLAG(0), WORD); \
		UML_SHL(block, IREG(2), IMM(1), IMM(E10)); \
		UML_AND(block, IREG(1), IREG(1), IREG(2)); \
		UML_SHR(block, IREG(1), IREG(1), IMM(E10)); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E10].h.low, IMM(0), IREG(0), WORD); \
		UML_CMP(block, IREG(0), IMM(32767)); \
		UML_JMPc(block, IF_LE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(32767)); \
		UML_JMP(block, outofrange = compiler->labelnum++); \
 \
	UML_LABEL(block, inrange); \
 \
		UML_CMP(block, IREG(0), IMM(-32768)); \
		UML_JMPc(block, IF_G, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(-32768)); \
 \
	UML_LABEL(block, inrange); \
	UML_LABEL(block, outofrange); \
 \
		UML_MOV(block, VRES(E10), IREG(0)); \
 \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E21), WORD); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_SEXT(block, IREG(1), VFLAG(0), WORD); \
		UML_SHL(block, IREG(2), IMM(1), IMM(E11)); \
		UML_AND(block, IREG(1), IREG(1), IREG(2)); \
		UML_SHR(block, IREG(1), IREG(1), IMM(E11)); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E11].h.low, IMM(0), IREG(0), WORD); \
		UML_CMP(block, IREG(0), IMM(32767)); \
		UML_JMPc(block, IF_LE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(32767)); \
		UML_JMP(block, outofrange = compiler->labelnum++); \
 \
	UML_LABEL(block, inrange); \
 \
		UML_CMP(block, IREG(0), IMM(-32768)); \
		UML_JMPc(block, IF_GE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(-32768)); \
 \
	UML_LABEL(block, inrange); \
	UML_LABEL(block, outofrange); \
 \
		UML_MOV(block, VRES(E11), IREG(0)); \
 \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E22), WORD); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_SEXT(block, IREG(1), VFLAG(0), WORD); \
		UML_SHL(block, IREG(2), IMM(1), IMM(E12)); \
		UML_AND(block, IREG(1), IREG(1), IREG(2)); \
		UML_SHR(block, IREG(1), IREG(1), IMM(E12)); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E12].h.low, IMM(0), IREG(0), WORD); \
		UML_CMP(block, IREG(0), IMM(32767)); \
		UML_JMPc(block, IF_LE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(32767)); \
		UML_JMP(block, outofrange = compiler->labelnum++); \
 \
	UML_LABEL(block, inrange); \
 \
		UML_CMP(block, IREG(0), IMM(-32768)); \
		UML_JMPc(block, IF_GE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(-32768)); \
 \
	UML_LABEL(block, inrange); \
	UML_LABEL(block, outofrange); \
 \
		UML_MOV(block, VRES(E12), IREG(0)); \
 \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E23), WORD); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_SEXT(block, IREG(1), VFLAG(0), WORD); \
		UML_SHL(block, IREG(2), IMM(1), IMM(E13)); \
		UML_AND(block, IREG(1), IREG(1), IREG(2)); \
		UML_SHR(block, IREG(1), IREG(1), IMM(E13)); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E13].h.low, IMM(0), IREG(0), WORD); \
		UML_CMP(block, IREG(0), IMM(32767)); \
		UML_JMPc(block, IF_LE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(32767)); \
		UML_JMP(block, outofrange = compiler->labelnum++); \
 \
	UML_LABEL(block, inrange); \
 \
		UML_CMP(block, IREG(0), IMM(-32768)); \
		UML_JMPc(block, IF_GE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(-32768)); \
 \
	UML_LABEL(block, inrange); \
	UML_LABEL(block, outofrange); \
 \
		UML_MOV(block, VRES(E13), IREG(0)); \
 \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E24), WORD); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_SEXT(block, IREG(1), VFLAG(0), WORD); \
		UML_SHL(block, IREG(2), IMM(1), IMM(E14)); \
		UML_AND(block, IREG(1), IREG(1), IREG(2)); \
		UML_SHR(block, IREG(1), IREG(1), IMM(E14)); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E14].h.low, IMM(0), IREG(0), WORD); \
		UML_CMP(block, IREG(0), IMM(32767)); \
		UML_JMPc(block, IF_LE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(32767)); \
		UML_JMP(block, outofrange = compiler->labelnum++); \
 \
	UML_LABEL(block, inrange); \
 \
		UML_CMP(block, IREG(0), IMM(-32768)); \
		UML_JMPc(block, IF_GE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(-32768)); \
 \
	UML_LABEL(block, inrange); \
	UML_LABEL(block, outofrange); \
 \
		UML_MOV(block, VRES(E14), IREG(0)); \
 \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E25), WORD); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_SEXT(block, IREG(1), VFLAG(0), WORD); \
		UML_SHL(block, IREG(2), IMM(1), IMM(E15)); \
		UML_AND(block, IREG(1), IREG(1), IREG(2)); \
		UML_SHR(block, IREG(1), IREG(1), IMM(E15)); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E15].h.low, IMM(0), IREG(0), WORD); \
		UML_CMP(block, IREG(0), IMM(32767)); \
		UML_JMPc(block, IF_LE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(32767)); \
		UML_JMP(block, outofrange = compiler->labelnum++); \
 \
	UML_LABEL(block, inrange); \
 \
		UML_CMP(block, IREG(0), IMM(-32768)); \
		UML_JMPc(block, IF_GE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(-32768)); \
 \
	UML_LABEL(block, inrange); \
	UML_LABEL(block, outofrange); \
 \
		UML_MOV(block, VRES(E15), IREG(0)); \
 \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E26), WORD); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_SEXT(block, IREG(1), VFLAG(0), WORD); \
		UML_SHL(block, IREG(2), IMM(1), IMM(E16)); \
		UML_AND(block, IREG(1), IREG(1), IREG(2)); \
		UML_SHR(block, IREG(1), IREG(1), IMM(E16)); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E16].h.low, IMM(0), IREG(0), WORD); \
		UML_CMP(block, IREG(0), IMM(32767)); \
		UML_JMPc(block, IF_LE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(32767)); \
		UML_JMP(block, outofrange = compiler->labelnum++); \
 \
	UML_LABEL(block, inrange); \
 \
		UML_CMP(block, IREG(0), IMM(-32768)); \
		UML_JMPc(block, IF_GE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(-32768)); \
 \
	UML_LABEL(block, inrange); \
	UML_LABEL(block, outofrange); \
 \
		UML_MOV(block, VRES(E16), IREG(0)); \
 \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E27), WORD); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_SEXT(block, IREG(1), VFLAG(0), WORD); \
		UML_SHL(block, IREG(2), IMM(1), IMM(E17)); \
		UML_AND(block, IREG(1), IREG(1), IREG(2)); \
		UML_SHR(block, IREG(1), IREG(1), IMM(E17)); \
		UML_ADD(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E17].h.low, IMM(0), IREG(0), WORD); \
		UML_CMP(block, IREG(0), IMM(32767)); \
		UML_JMPc(block, IF_LE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(32767)); \
		UML_JMP(block, outofrange = compiler->labelnum++); \
 \
	UML_LABEL(block, inrange); \
 \
		UML_CMP(block, IREG(0), IMM(-32768)); \
		UML_JMPc(block, IF_GE, inrange = compiler->labelnum++); \
 \
		UML_MOV(block, IREG(0), IMM(-32768)); \
 \
	UML_LABEL(block, inrange); \
	UML_LABEL(block, outofrange); \
 \
		UML_MOV(block, VRES(E17), IREG(0)); \
 \
 \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), VRES(E10), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), VRES(E11), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), VRES(E12), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), VRES(E13), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), VRES(E14), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), VRES(E15), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), VRES(E16), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), VRES(E17), WORD); \
	}

static int generate_vadd(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)	// vadd
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 001110 |
	// ------------------------------------------------------
	//
	// Multiplies unsigned fraction by signed integer
	// The result is added into accumulator
	// The low slice of accumulator is stored into destination element
	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VADD_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VADD_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VADD_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VADD_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VADD_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VADD_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VADD_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VADD_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VADD_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VADD_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VADD_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VADD_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VADD_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VADD_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VADD_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VADD_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VADD(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{																						\
		INT32 r;																			\
		ACCUM(E10).h.low = (INT16)(r = (INT32)R_VREG_S_X(VS1REG, I0) + (INT32)R_VREG_S_X(VS2REG, E20) + ((rsp->flag[0] & (1 << E10)) >> E10)); \
		vres[E10] = (r > 32767) ? 32767 : ((r < -32768) ? -32768 : ACCUM(E10).h.low);		\
		ACCUM(E11).h.low = (INT16)(r = (INT32)R_VREG_S_X(VS1REG, I1) + (INT32)R_VREG_S_X(VS2REG, E21) + ((rsp->flag[0] & (1 << E11)) >> E11)); \
		vres[E11] = (r > 32767) ? 32767 : ((r < -32768) ? -32768 : ACCUM(E11).h.low);		\
		ACCUM(E12).h.low = (INT16)(r = (INT32)R_VREG_S_X(VS1REG, I2) + (INT32)R_VREG_S_X(VS2REG, E22) + ((rsp->flag[0] & (1 << E12)) >> E12)); \
		vres[E12] = (r > 32767) ? 32767 : ((r < -32768) ? -32768 : ACCUM(E12).h.low);		\
		ACCUM(E13).h.low = (INT16)(r = (INT32)R_VREG_S_X(VS1REG, I3) + (INT32)R_VREG_S_X(VS2REG, E23) + ((rsp->flag[0] & (1 << E13)) >> E13)); \
		vres[E13] = (r > 32767) ? 32767 : ((r < -32768) ? -32768 : ACCUM(E13).h.low);		\
		ACCUM(E14).h.low = (INT16)(r = (INT32)R_VREG_S_X(VS1REG, I4) + (INT32)R_VREG_S_X(VS2REG, E24) + ((rsp->flag[0] & (1 << E14)) >> E14)); \
		vres[E14] = (r > 32767) ? 32767 : ((r < -32768) ? -32768 : ACCUM(E14).h.low);		\
		ACCUM(E15).h.low = (INT16)(r = (INT32)R_VREG_S_X(VS1REG, I5) + (INT32)R_VREG_S_X(VS2REG, E25) + ((rsp->flag[0] & (1 << E15)) >> E15)); \
		vres[E15] = (r > 32767) ? 32767 : ((r < -32768) ? -32768 : ACCUM(E15).h.low);		\
		ACCUM(E16).h.low = (INT16)(r = (INT32)R_VREG_S_X(VS1REG, I6) + (INT32)R_VREG_S_X(VS2REG, E26) + ((rsp->flag[0] & (1 << E16)) >> E16)); \
		vres[E16] = (r > 32767) ? 32767 : ((r < -32768) ? -32768 : ACCUM(E16).h.low);		\
		ACCUM(E17).h.low = (INT16)(r = (INT32)R_VREG_S_X(VS1REG, I7) + (INT32)R_VREG_S_X(VS2REG, E27) + ((rsp->flag[0] & (1 << E17)) >> E17)); \
		vres[E17] = (r > 32767) ? 32767 : ((r < -32768) ? -32768 : ACCUM(E17).h.low);		\
	}

INLINE void cfunc_rsp_vadd(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };
	//int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010000 |
	// ------------------------------------------------------
	//
	// Adds two vector registers and carry flag, the result is saturated to 32767

	// TODO: check VS2REG == VDREG
	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VADD(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VADD(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VADD(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VADD(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VADD(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VADD(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VADD(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VADD(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VADD(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VADD(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VADD(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VADD(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VADD(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VADD(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VADD(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VADD(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
	}
	rsp->flag[0] = 0;
	WRITEBACK_RESULT();
}
#endif

INLINE void cfunc_rsp_vsub(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010001 |
	// ------------------------------------------------------
	//
	// Subtracts two vector registers and carry flag, the result is saturated to -32768

	// TODO: check VS2REG == VDREG

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT32 s1 = (INT32)(INT16)R_VREG_S(VS1REG, del);
		INT32 s2 = (INT32)(INT16)R_VREG_S(VS2REG, sel);
		INT32 r = s1 - s2 - CARRY_FLAG(del);

		W_ACCUM_L(del, (INT16)(r));

		if (r > 32767) r = 32767;
		if (r < -32768) r = -32768;

		vres[del] = (INT16)(r);
	}
	rsp->flag[0] = 0;
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vabs(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010011 |
	// ------------------------------------------------------
	//
	// Changes the sign of source register 2 if source register 1 is negative and stores
	// the result to destination register

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT16 s1 = (INT16)R_VREG_S(VS1REG, del);
		INT16 s2 = (INT16)R_VREG_S(VS2REG, sel);

		if (s1 < 0)
		{
			if (s2 == -32768)
			{
				vres[del] = 32767;
			}
			else
			{
				vres[del] = -s2;
			}
		}
		else if (s1 > 0)
		{
			vres[del] = s2;
		}
		else
		{
			vres[del] = 0;
		}

		W_ACCUM_L(del, vres[del]);
	}
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vaddc(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010100 |
	// ------------------------------------------------------
	//
	// Adds two vector registers, the carry out is stored into carry register

	// TODO: check VS2REG = VDREG

	rsp->flag[0] = 0;

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT32 s1 = (UINT32)(UINT16)R_VREG_S(VS1REG, del);
		INT32 s2 = (UINT32)(UINT16)R_VREG_S(VS2REG, sel);
		INT32 r = s1 + s2;

		vres[del] = (INT16)(r);
		W_ACCUM_L(del, (INT16)(r));

		if (r & 0xffff0000)
		{
			SET_CARRY_FLAG(del);
		}
	}
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vsubc(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 010101 |
	// ------------------------------------------------------
	//
	// Subtracts two vector registers, the carry out is stored into carry register

	// TODO: check VS2REG = VDREG

	rsp->flag[0] = 0;

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT32 s1 = (UINT32)(UINT16)R_VREG_S(VS1REG, del);
		INT32 s2 = (UINT32)(UINT16)R_VREG_S(VS2REG, sel);
		INT32 r = s1 - s2;

		vres[del] = (INT16)(r);
		W_ACCUM_L(del, (UINT16)(r));

		if ((UINT16)(r) != 0)
		{
			SET_ZERO_FLAG(del);
		}
		if (r & 0xffff0000)
		{
			SET_CARRY_FLAG(del);
		}
	}
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vsaw(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 011101 |
	// ------------------------------------------------------
	//
	// Stores high, middle or low slice of accumulator to destination vector

	switch (EL)
	{
		case 0x08:		// VSAWH
		{
			W_VREG_S_X(VDREG, 7, ACCUM(0).h.high);
			W_VREG_S_X(VDREG, 6, ACCUM(1).h.high);
			W_VREG_S_X(VDREG, 5, ACCUM(2).h.high);
			W_VREG_S_X(VDREG, 4, ACCUM(3).h.high);
			W_VREG_S_X(VDREG, 3, ACCUM(4).h.high);
			W_VREG_S_X(VDREG, 2, ACCUM(5).h.high);
			W_VREG_S_X(VDREG, 1, ACCUM(6).h.high);
			W_VREG_S_X(VDREG, 0, ACCUM(7).h.high);
			break;
		}
		case 0x09:		// VSAWM
		{
			W_VREG_S_X(VDREG, 7, ACCUM(0).h.mid);
			W_VREG_S_X(VDREG, 6, ACCUM(1).h.mid);
			W_VREG_S_X(VDREG, 5, ACCUM(2).h.mid);
			W_VREG_S_X(VDREG, 4, ACCUM(3).h.mid);
			W_VREG_S_X(VDREG, 3, ACCUM(4).h.mid);
			W_VREG_S_X(VDREG, 2, ACCUM(5).h.mid);
			W_VREG_S_X(VDREG, 1, ACCUM(6).h.mid);
			W_VREG_S_X(VDREG, 0, ACCUM(7).h.mid);
			break;
		}
		case 0x0a:		// VSAWL
		{
			W_VREG_S_X(VDREG, 7, ACCUM(0).h.low);
			W_VREG_S_X(VDREG, 6, ACCUM(1).h.low);
			W_VREG_S_X(VDREG, 5, ACCUM(2).h.low);
			W_VREG_S_X(VDREG, 4, ACCUM(3).h.low);
			W_VREG_S_X(VDREG, 3, ACCUM(4).h.low);
			W_VREG_S_X(VDREG, 2, ACCUM(5).h.low);
			W_VREG_S_X(VDREG, 1, ACCUM(6).h.low);
			W_VREG_S_X(VDREG, 0, ACCUM(7).h.low);
			break;
		}
		default:	fatalerror("RSP: VSAW: el = %d\n", EL);
	}
}

#define RSP_VLT(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		rsp->flag[1] = 0; \
 \
		if (R_VREG_S_X(VS1REG, I0) > R_VREG_S_X(VS2REG, E20)) \
		{ \
			ACCUM(E16).h.low = vres[E10] = R_VREG_S_X(VS2REG, E20); \
		} \
		else if (R_VREG_S_X(VS1REG, I0) < R_VREG_S_X(VS2REG, E20)) \
		{ \
			ACCUM(E10).h.low = vres[E10] = R_VREG_S_X(VS1REG, I0); \
			SET_COMPARE_FLAG(E10); \
		} \
		else if (R_VREG_S_X(VS1REG, I0) == R_VREG_S_X(VS2REG, E20)) \
		{ \
			ACCUM(E10).h.low = vres[E10] = R_VREG_S_X(VS1REG, I0); \
			if (ZERO_FLAG(E10) && CARRY_FLAG(E10)) SET_COMPARE_FLAG(E10); \
		} \
		if (R_VREG_S_X(VS1REG, I1) > R_VREG_S_X(VS2REG, E21)) \
		{ \
			ACCUM(E16).h.low = vres[E11] = R_VREG_S_X(VS2REG, E21); \
		} \
		else if (R_VREG_S_X(VS1REG, I1) < R_VREG_S_X(VS2REG, E21)) \
		{ \
			ACCUM(E11).h.low = vres[E11] = R_VREG_S_X(VS1REG, I1); \
			SET_COMPARE_FLAG(E11); \
		} \
		else if (R_VREG_S_X(VS1REG, I1) == R_VREG_S_X(VS2REG, E21)) \
		{ \
			ACCUM(E11).h.low = vres[E11] = R_VREG_S_X(VS1REG, I1); \
			if (ZERO_FLAG(E11) && CARRY_FLAG(E11)) SET_COMPARE_FLAG(E11); \
		} \
		if (R_VREG_S_X(VS1REG, I2) > R_VREG_S_X(VS2REG, E22)) \
		{ \
			ACCUM(E12).h.low = vres[E12] = R_VREG_S_X(VS2REG, E22); \
		} \
		else if (R_VREG_S_X(VS1REG, I2) < R_VREG_S_X(VS2REG, E22)) \
		{ \
			ACCUM(E12).h.low = vres[E12] = R_VREG_S_X(VS1REG, I2); \
			SET_COMPARE_FLAG(E12); \
		} \
		else if (R_VREG_S_X(VS1REG, I2) == R_VREG_S_X(VS2REG, E22)) \
		{ \
			ACCUM(E12).h.low = vres[E12] = R_VREG_S_X(VS1REG, I2); \
			if (ZERO_FLAG(E12) && CARRY_FLAG(E12)) SET_COMPARE_FLAG(E12); \
		} \
		if (R_VREG_S_X(VS1REG, I3) > R_VREG_S_X(VS2REG, E23)) \
		{ \
			ACCUM(E13).h.low = vres[E13] = R_VREG_S_X(VS2REG, E23); \
		} \
		else if (R_VREG_S_X(VS1REG, I3) < R_VREG_S_X(VS2REG, E23)) \
		{ \
			ACCUM(E13).h.low = vres[E13] = R_VREG_S_X(VS1REG, I3); \
			SET_COMPARE_FLAG(E13); \
		} \
		else if (R_VREG_S_X(VS1REG, I3) == R_VREG_S_X(VS2REG, E23)) \
		{ \
			ACCUM(E13).h.low = vres[E13] = R_VREG_S_X(VS1REG, I3); \
			if (ZERO_FLAG(E10) && CARRY_FLAG(E13)) SET_COMPARE_FLAG(E13); \
		} \
		if (R_VREG_S_X(VS1REG, I4) > R_VREG_S_X(VS2REG, E24)) \
		{ \
			ACCUM(E14).h.low = vres[E14] = R_VREG_S_X(VS2REG, E24); \
		} \
		else if (R_VREG_S_X(VS1REG, I4) < R_VREG_S_X(VS2REG, E24)) \
		{ \
			ACCUM(E14).h.low = vres[E14] = R_VREG_S_X(VS1REG, I4); \
			SET_COMPARE_FLAG(E14); \
		} \
		else if (R_VREG_S_X(VS1REG, I4) == R_VREG_S_X(VS2REG, E24)) \
		{ \
			ACCUM(E14).h.low = vres[E14] = R_VREG_S_X(VS1REG, I4); \
			if (ZERO_FLAG(E14) && CARRY_FLAG(E14)) SET_COMPARE_FLAG(E14); \
		} \
		if (R_VREG_S_X(VS1REG, I5) > R_VREG_S_X(VS2REG, E25)) \
		{ \
			ACCUM(E15).h.low = vres[5] = R_VREG_S_X(VS2REG, E25); \
		} \
		else if (R_VREG_S_X(VS1REG, I5) < R_VREG_S_X(VS2REG, E25)) \
		{ \
			ACCUM(E15).h.low = vres[E15] = R_VREG_S_X(VS1REG, I5); \
			SET_COMPARE_FLAG(E15); \
		} \
		else if (R_VREG_S_X(VS1REG, I5) == R_VREG_S_X(VS2REG, E25)) \
		{ \
			ACCUM(E15).h.low = vres[E15] = R_VREG_S_X(VS1REG, I5); \
			if (ZERO_FLAG(E15) && CARRY_FLAG(E15)) SET_COMPARE_FLAG(E15); \
		} \
		if (R_VREG_S_X(VS1REG, I6) > R_VREG_S_X(VS2REG, E26)) \
		{ \
			ACCUM(E16).h.low = vres[E16] = R_VREG_S_X(VS2REG, E26); \
		} \
		else if (R_VREG_S_X(VS1REG, I6) < R_VREG_S_X(VS2REG, E26)) \
		{ \
			ACCUM(E16).h.low = vres[E16] = R_VREG_S_X(VS1REG, I6); \
			SET_COMPARE_FLAG(E16); \
		} \
		else if (R_VREG_S_X(VS1REG, I6) == R_VREG_S_X(VS2REG, E26)) \
		{ \
			ACCUM(E16).h.low = vres[E16] = R_VREG_S_X(VS1REG, I6); \
			if (ZERO_FLAG(E16) && CARRY_FLAG(E16)) SET_COMPARE_FLAG(E16); \
		} \
		if (R_VREG_S_X(VS1REG, I7) > R_VREG_S_X(VS2REG, E27)) \
		{ \
			ACCUM(E17).h.low = vres[E17] = R_VREG_S_X(VS2REG, E27); \
		} \
		else if (R_VREG_S_X(VS1REG, I7) < R_VREG_S_X(VS2REG, E27)) \
		{ \
			ACCUM(E17).h.low = vres[E17] = R_VREG_S_X(VS1REG, I7); \
			SET_COMPARE_FLAG(E17); \
		} \
		else if (R_VREG_S_X(VS1REG, I7) == R_VREG_S_X(VS2REG, E27)) \
		{ \
			ACCUM(E17).h.low = vres[E17] = R_VREG_S_X(VS1REG, I7); \
			if (ZERO_FLAG(E17) && CARRY_FLAG(E17)) SET_COMPARE_FLAG(E17); \
		} \
		rsp->flag[0] = 0; \
		W_VREG_S_X(VDREG, 7, vres[0]);			\
		W_VREG_S_X(VDREG, 6, vres[1]);			\
		W_VREG_S_X(VDREG, 5, vres[2]);			\
		W_VREG_S_X(VDREG, 4, vres[3]);			\
		W_VREG_S_X(VDREG, 3, vres[4]);			\
		W_VREG_S_X(VDREG, 2, vres[5]);			\
		W_VREG_S_X(VDREG, 1, vres[6]);			\
		W_VREG_S_X(VDREG, 0, vres[7]);			\
	}

INLINE void cfunc_rsp_vlt(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };
	//int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100000 |
	// ------------------------------------------------------
	//
	// Sets compare flags if elements in VS1 are less than VS2
	// Moves the element in VS2 to destination vector

	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VLT(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VLT(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VLT(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VLT(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VLT(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VLT(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VLT(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VLT(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VLT(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VLT(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VLT(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VLT(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VLT(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VLT(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VLT(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VLT(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
	}

}

INLINE void cfunc_rsp_veq(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100001 |
	// ------------------------------------------------------
	//
	// Sets compare flags if elements in VS1 are equal with VS2
	// Moves the element in VS2 to destination vector

	rsp->flag[1] = 0;

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);

		vres[del] = R_VREG_S(VS2REG, sel);
		W_ACCUM_L(del, vres[del]);

		if (R_VREG_S(VS1REG, del) == R_VREG_S(VS2REG, sel))
		{
			if (ZERO_FLAG(del) == 0)
			{
				SET_COMPARE_FLAG(del);
			}
		}
	}

	rsp->flag[0] = 0;
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vne(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100010 |
	// ------------------------------------------------------
	//
	// Sets compare flags if elements in VS1 are not equal with VS2
	// Moves the element in VS2 to destination vector

	rsp->flag[1] = 0;

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);

		vres[del] = R_VREG_S(VS1REG, del);
		W_ACCUM_L(del, vres[del]);

		if (R_VREG_S(VS1REG, del) != R_VREG_S(VS2REG, sel))
		{
			SET_COMPARE_FLAG(del);
		}
		else
		{
			if (ZERO_FLAG(del) != 0)
			{
				SET_COMPARE_FLAG(del);
			}
		}
	}

	rsp->flag[0] = 0;
	WRITEBACK_RESULT();
}

#define RSP_VGE(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		rsp->flag[1] = 0; \
 \
		if (R_VREG_S_X(VS1REG, I0) > R_VREG_S_X(VS2REG, E20)) \
		{ \
			ACCUM(E10).h.low = vres[E10] = R_VREG_S_X(VS1REG, I0); \
			SET_COMPARE_FLAG(E10); \
		} \
		else if (R_VREG_S_X(VS1REG, I0) == R_VREG_S_X(VS2REG, E20)) \
		{ \
			if (!ZERO_FLAG(E10) || !CARRY_FLAG(E10)) \
			{ \
				ACCUM(E10).h.low = vres[E10] = R_VREG_S_X(VS1REG, I0); \
				SET_COMPARE_FLAG(E10); \
			} \
			else \
			{ \
				ACCUM(E10).h.low = vres[E10] = R_VREG_S_X(VS2REG, E20); \
			} \
		} \
		else \
		{ \
			ACCUM(E10).h.low = vres[E10] = R_VREG_S_X(VS2REG, E20); \
		} \
		if (R_VREG_S_X(VS1REG, I1) > R_VREG_S_X(VS2REG, E21)) \
		{ \
			ACCUM(E11).h.low = vres[E11] = R_VREG_S_X(VS1REG, I1); \
			SET_COMPARE_FLAG(E11); \
		} \
		else if (R_VREG_S_X(VS1REG, I1) == R_VREG_S_X(VS2REG, E21)) \
		{ \
			if (!ZERO_FLAG(E11) || !CARRY_FLAG(E11)) \
			{ \
				ACCUM(E11).h.low = vres[E11] = R_VREG_S_X(VS1REG, I1); \
				SET_COMPARE_FLAG(E11); \
			} \
			else \
			{ \
				ACCUM(E11).h.low = vres[E11] = R_VREG_S_X(VS2REG, E21); \
			} \
		} \
		else \
		{ \
			ACCUM(E11).h.low = vres[E11] = R_VREG_S_X(VS2REG, E21); \
		} \
		if (R_VREG_S_X(VS1REG, I2) > R_VREG_S_X(VS2REG, E22)) \
		{ \
			ACCUM(E12).h.low = vres[E12] = R_VREG_S_X(VS1REG, I2); \
			SET_COMPARE_FLAG(E12); \
		} \
		else if (R_VREG_S_X(VS1REG, I2) == R_VREG_S_X(VS2REG, E22)) \
		{ \
			if (!ZERO_FLAG(E12) || !CARRY_FLAG(E12)) \
			{ \
				ACCUM(E12).h.low = vres[E12] = R_VREG_S_X(VS1REG, I2); \
				SET_COMPARE_FLAG(E12); \
			} \
			else \
			{ \
				ACCUM(E12).h.low = vres[E12] = R_VREG_S_X(VS2REG, E22); \
			} \
		} \
		else \
		{ \
			ACCUM(E12).h.low = vres[E12] = R_VREG_S_X(VS2REG, E22); \
		} \
		if (R_VREG_S_X(VS1REG, I3) > R_VREG_S_X(VS2REG, E23)) \
		{ \
			ACCUM(E13).h.low = vres[E13] = R_VREG_S_X(VS1REG, I3); \
			SET_COMPARE_FLAG(E13); \
		} \
		else if (R_VREG_S_X(VS1REG, I3) == R_VREG_S_X(VS2REG, E23)) \
		{ \
			if (!ZERO_FLAG(E13) || !CARRY_FLAG(E13)) \
			{ \
				ACCUM(E13).h.low = vres[E13] = R_VREG_S_X(VS1REG, I3); \
				SET_COMPARE_FLAG(E13); \
			} \
			else \
			{ \
				ACCUM(E13).h.low = vres[E13] = R_VREG_S_X(VS2REG, E23); \
			} \
		} \
		else \
		{ \
			ACCUM(E13).h.low = vres[E13] = R_VREG_S_X(VS2REG, E23); \
		} \
		if (R_VREG_S_X(VS1REG, I4) > R_VREG_S_X(VS2REG, E24)) \
		{ \
			ACCUM(E14).h.low = vres[E14] = R_VREG_S_X(VS1REG, I4); \
			SET_COMPARE_FLAG(E14); \
		} \
		else if (R_VREG_S_X(VS1REG, I4) == R_VREG_S_X(VS2REG, E24)) \
		{ \
			if (!ZERO_FLAG(E14) || !CARRY_FLAG(E14)) \
			{ \
				ACCUM(E14).h.low = vres[E14] = R_VREG_S_X(VS1REG, I4); \
				SET_COMPARE_FLAG(E14); \
			} \
			else \
			{ \
				ACCUM(E14).h.low = vres[E14] = R_VREG_S_X(VS2REG, E24); \
			} \
		} \
		else \
		{ \
			ACCUM(E14).h.low = vres[E14] = R_VREG_S_X(VS2REG, E24); \
		} \
		if (R_VREG_S_X(VS1REG, I5) > R_VREG_S_X(VS2REG, E25)) \
		{ \
			ACCUM(E15).h.low = vres[E15] = R_VREG_S_X(VS1REG, I5); \
			SET_COMPARE_FLAG(E15); \
		} \
		else if (R_VREG_S_X(VS1REG, I5) == R_VREG_S_X(VS2REG, E25)) \
		{ \
			if (!ZERO_FLAG(E15) || !CARRY_FLAG(E15)) \
			{ \
				ACCUM(E15).h.low = vres[E15] = R_VREG_S_X(VS1REG, I5); \
				SET_COMPARE_FLAG(E15); \
			} \
			else \
			{ \
				ACCUM(E15).h.low = vres[E15] = R_VREG_S_X(VS2REG, E25); \
			} \
		} \
		else \
		{ \
			ACCUM(E15).h.low = vres[E15] = R_VREG_S_X(VS2REG, E25); \
		} \
		if (R_VREG_S_X(VS1REG, I6) > R_VREG_S_X(VS2REG, E26)) \
		{ \
			ACCUM(E16).h.low = vres[E16] = R_VREG_S_X(VS1REG, I6); \
			SET_COMPARE_FLAG(E16); \
		} \
		else if (R_VREG_S_X(VS1REG, I6) == R_VREG_S_X(VS2REG, E26)) \
		{ \
			if (!ZERO_FLAG(E16) || !CARRY_FLAG(E16)) \
			{ \
				ACCUM(E16).h.low = vres[E16] = R_VREG_S_X(VS1REG, I6); \
				SET_COMPARE_FLAG(E16); \
			} \
			else \
			{ \
				ACCUM(E16).h.low = vres[E16] = R_VREG_S_X(VS2REG, E26); \
			} \
		} \
		else \
		{ \
			ACCUM(E16).h.low = vres[E16] = R_VREG_S_X(VS2REG, E26); \
		} \
		if (R_VREG_S_X(VS1REG, I7) > R_VREG_S_X(VS2REG, E27)) \
		{ \
			ACCUM(E17).h.low = vres[E17] = R_VREG_S_X(VS1REG, I7); \
			SET_COMPARE_FLAG(E17); \
		} \
		else if (R_VREG_S_X(VS1REG, I7) == R_VREG_S_X(VS2REG, E27)) \
		{ \
			if (!ZERO_FLAG(E17) || !CARRY_FLAG(E17)) \
			{ \
				ACCUM(E17).h.low = vres[E17] = R_VREG_S_X(VS1REG, I7); \
				SET_COMPARE_FLAG(E17); \
			} \
			else \
			{ \
				ACCUM(E17).h.low = vres[E17] = R_VREG_S_X(VS2REG, E27); \
			} \
		} \
		else \
		{ \
			ACCUM(E17).h.low = vres[E17] = R_VREG_S_X(VS2REG, E27); \
		} \
		rsp->flag[0] = 0; \
		W_VREG_S_X(VDREG, 7, vres[0]);			\
		W_VREG_S_X(VDREG, 6, vres[1]);			\
		W_VREG_S_X(VDREG, 5, vres[2]);			\
		W_VREG_S_X(VDREG, 4, vres[3]);			\
		W_VREG_S_X(VDREG, 3, vres[4]);			\
		W_VREG_S_X(VDREG, 2, vres[5]);			\
		W_VREG_S_X(VDREG, 1, vres[6]);			\
		W_VREG_S_X(VDREG, 0, vres[7]);			\
	}

INLINE void cfunc_rsp_vge(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };
	//int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100011 |
	// ------------------------------------------------------
	//
	// Sets compare flags if elements in VS1 are greater or equal with VS2
	// Moves the element in VS2 to destination vector

#if 1
	switch(EL)
	{
		case 0:    /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VGE(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VGE(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VGE(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			break;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VGE(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			break;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VGE(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			break;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VGE(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			break;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VGE(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			break;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VGE(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			break;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VGE(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			break;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VGE(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			break;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VGE(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			break;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VGE(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			break;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VGE(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			break;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VGE(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			break;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VGE(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			break;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VGE(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			break;
	}
#else
	rsp->flag[1] = 0;

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);

		if (R_VREG_S(VS1REG, del) == R_VREG_S(VS2REG, sel))
		{
			if (ZERO_FLAG(del) == 0 || CARRY_FLAG(del) == 0)
			{
				SET_COMPARE_FLAG(del);
			}
		}
		else if (R_VREG_S(VS1REG, del) > R_VREG_S(VS2REG, sel))
		{
			SET_COMPARE_FLAG(del);
		}

		if (COMPARE_FLAG(del))
		{
			vres[del] = R_VREG_S(VS1REG, del);
		}
		else
		{
			vres[del] = R_VREG_S(VS2REG, sel);
		}

		W_ACCUM_L(del, vres[del]);
	}

	rsp->flag[0] = 0;
	WRITEBACK_RESULT();
#endif
}

INLINE void cfunc_rsp_vcl(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100100 |
	// ------------------------------------------------------
	//
	// Vector clip low

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT16 s1 = R_VREG_S(VS1REG, del);
		INT16 s2 = R_VREG_S(VS2REG, sel);

		if (CARRY_FLAG(del) != 0)
		{
			if (ZERO_FLAG(del) != 0)
			{
				if (COMPARE_FLAG(del) != 0)
				{
					W_ACCUM_L(del, -(UINT16)s2);
				}
				else
				{
					W_ACCUM_L(del, s1);
				}
			}
			else
			{
				if (rsp->flag[2] & (1 << (del)))
				{
					if (((UINT32)(INT16)(s1) + (UINT32)(INT16)(s2)) > 0x10000)
					{
						W_ACCUM_L(del, s1);
						CLEAR_COMPARE_FLAG(del);
					}
					else
					{
						W_ACCUM_L(del, -((UINT16)s2));
						SET_COMPARE_FLAG(del);
					}
				}
				else
				{
					if (((UINT32)(INT16)(s1) + (UINT32)(INT16)(s2)) != 0)
					{
						W_ACCUM_L(del, s1);
						CLEAR_COMPARE_FLAG(del);
					}
					else
					{
						W_ACCUM_L(del, -((UINT16)s2));
						SET_COMPARE_FLAG(del);
					}
				}
			}
		}
		else
		{
			if (ZERO_FLAG(del) != 0)
			{
				if (rsp->flag[1] & (0x100 << del))
				{
					W_ACCUM_L(del, s2);
				}
				else
				{
					W_ACCUM_L(del, s1);
				}
			}
			else
			{
				if (((INT32)(UINT16)s1 - (INT32)(UINT16)s2) >= 0)
				{
					W_ACCUM_L(del, s2);
					rsp->flag[1] |= (0x100 << del);
				}
				else
				{
					W_ACCUM_L(del, s1);
					rsp->flag[1] &= ~(0x100 << del);
				}
			}
		}

		vres[del] = ACCUM(del).h.low;
	}
	rsp->flag[0] = 0;
	rsp->flag[2] = 0;
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vch(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100101 |
	// ------------------------------------------------------
	//
	// Vector clip high

	rsp->flag[0] = 0;
	rsp->flag[1] = 0;
	rsp->flag[2] = 0;

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT16 s1 = R_VREG_S(VS1REG, del);
		INT16 s2 = R_VREG_S(VS2REG, sel);

		if ((s1 ^ s2) < 0)
		{
			SET_CARRY_FLAG(del);
			if (s2 < 0)
			{
				rsp->flag[1] |= (0x100 << del);
			}

			if (s1 + s2 <= 0)
			{
				if (s1 + s2 == -1)
				{
					rsp->flag[2] |= (1 << (del));
				}
				SET_COMPARE_FLAG(del);
				vres[del] = -((UINT16)s2);
			}
			else
			{
				vres[del] = s1;
			}

			if (s1 + s2 != 0)
			{
				if (s1 != ~s2)
				{
					SET_ZERO_FLAG(del);
				}
			}
		}
		else
		{
			if (s2 < 0)
			{
				SET_COMPARE_FLAG(del);
			}
			if (s1 - s2 >= 0)
			{
				rsp->flag[1] |= (0x100 << del);
				vres[del] = s2;
			}
			else
			{
				vres[del] = s1;
			}

			if ((s1 - s2) != 0)
			{
				if (s1 != ~s2)
				{
					SET_ZERO_FLAG(del);
				}
			}
		}

		W_ACCUM_L(del, vres[del]);
	}
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vcr(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100110 |
	// ------------------------------------------------------
	//
	// Vector clip reverse

	rsp->flag[0] = 0;
	rsp->flag[1] = 0;
	rsp->flag[2] = 0;

	for (i=0; i < 8; i++)
	{
		int del = VEC_EL_1(EL, i);
		int sel = VEC_EL_2(EL, del);
		INT16 s1 = R_VREG_S(VS1REG, del);
		INT16 s2 = R_VREG_S(VS2REG, sel);

		if ((INT16)(s1 ^ s2) < 0)
		{
			if (s2 < 0)
			{
				rsp->flag[1] |= (0x100 << del);
			}
			if ((s1 + s2) <= 0)
			{
				W_ACCUM_L(del, ~((UINT16)s2));
				SET_COMPARE_FLAG(del);
			}
			else
			{
				W_ACCUM_L(del, s1);
			}
		}
		else
		{
			if (s2 < 0)
			{
				SET_COMPARE_FLAG(del);
			}
			if ((s1 - s2) >= 0)
			{
				W_ACCUM_L(del, s2);
				rsp->flag[1] |= (0x100 << del);
			}
			else
			{
				W_ACCUM_L(del, s1);
			}
		}

		vres[del] = ACCUM(del).h.low;
	}
	WRITEBACK_RESULT();
}

#define RSP_VMRG(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	if (COMPARE_FLAG(E10))					\
	{										\
		vres[E10] = R_VREG_S(VS1REG, E10);	\
	}										\
	else									\
	{										\
		vres[E10] = R_VREG_S(VS2REG, E20);	\
	}										\
	if (COMPARE_FLAG(E11))					\
	{										\
		vres[E11] = R_VREG_S(VS1REG, E11);	\
	}										\
	else									\
	{										\
		vres[E11] = R_VREG_S(VS2REG, E21);	\
	}										\
	if (COMPARE_FLAG(E12))					\
	{										\
		vres[E12] = R_VREG_S(VS1REG, E12);	\
	}										\
	else									\
	{										\
		vres[E12] = R_VREG_S(VS2REG, E22);	\
	}										\
	if (COMPARE_FLAG(E13))					\
	{										\
		vres[E13] = R_VREG_S(VS1REG, E13);	\
	}										\
	else									\
	{										\
		vres[E13] = R_VREG_S(VS2REG, E23);	\
	}										\
	if (COMPARE_FLAG(E14))					\
	{										\
		vres[E14] = R_VREG_S(VS1REG, E14);	\
	}										\
	else									\
	{										\
		vres[E14] = R_VREG_S(VS2REG, E24);	\
	}										\
	if (COMPARE_FLAG(E15))					\
	{										\
		vres[E15] = R_VREG_S(VS1REG, E15);	\
	}										\
	else									\
	{										\
		vres[E15] = R_VREG_S(VS2REG, E25);	\
	}										\
	if (COMPARE_FLAG(E16))					\
	{										\
		vres[E16] = R_VREG_S(VS1REG, E16);	\
	}										\
	else									\
	{										\
		vres[E16] = R_VREG_S(VS2REG, E26);	\
	}										\
	if (COMPARE_FLAG(E17))					\
	{										\
		vres[E17] = R_VREG_S(VS1REG, E17);	\
	}										\
	else									\
	{										\
		vres[E17] = R_VREG_S(VS2REG, E27);	\
	}										\
	W_ACCUM_L(E10, vres[E10]);				\
	W_ACCUM_L(E11, vres[E11]);				\
	W_ACCUM_L(E12, vres[E12]);				\
	W_ACCUM_L(E13, vres[E13]);				\
	W_ACCUM_L(E14, vres[E14]);				\
	W_ACCUM_L(E15, vres[E15]);				\
	W_ACCUM_L(E16, vres[E16]);				\
	W_ACCUM_L(E17, vres[E17]);				\

INLINE void cfunc_rsp_vmrg(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 100111 |
	// ------------------------------------------------------
	//
	// Merges two vectors according to compare flags

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VMRG(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1: /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VMRG(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2: /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VMRG(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3: /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VMRG(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4: /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VMRG(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5: /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VMRG(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6: /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VMRG(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7: /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VMRG(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8: /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VMRG(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9: /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VMRG(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:/* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VMRG(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:/* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VMRG(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:/* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VMRG(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:/* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VMRG(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:/* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VMRG(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:/* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VMRG(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
	WRITEBACK_RESULT();
}

#if (DRC_VAND)
#define RSP_VAND_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(0), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E20), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E10].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E21), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E11].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E22), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E12].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E23), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E13].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E24), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E14].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E25), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E15].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E26), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E16].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E27), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E17].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_LOAD(block, IREG(0), &rsp->accum[E10].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E11].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E12].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E13].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E14].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E15].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E16].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E17].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vand(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
	// ------------------------------------------------------
	//
	// Bitwise AND of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VAND_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VAND_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VAND_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VAND_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VAND_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VAND_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VAND_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VAND_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VAND_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VAND_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VAND_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VAND_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VAND_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VAND_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VAND_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VAND_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VAND(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	vres[E10] = R_VREG_S(VS1REG, E10) & R_VREG_S(VS2REG, E20);	\
	vres[E11] = R_VREG_S(VS1REG, E11) & R_VREG_S(VS2REG, E21);	\
	vres[E12] = R_VREG_S(VS1REG, E12) & R_VREG_S(VS2REG, E22);	\
	vres[E13] = R_VREG_S(VS1REG, E13) & R_VREG_S(VS2REG, E23);	\
	vres[E14] = R_VREG_S(VS1REG, E14) & R_VREG_S(VS2REG, E24);	\
	vres[E15] = R_VREG_S(VS1REG, E15) & R_VREG_S(VS2REG, E25);	\
	vres[E16] = R_VREG_S(VS1REG, E16) & R_VREG_S(VS2REG, E26);	\
	vres[E17] = R_VREG_S(VS1REG, E17) & R_VREG_S(VS2REG, E27);	\
	W_ACCUM_L(E10, vres[E10]);				\
	W_ACCUM_L(E11, vres[E11]);				\
	W_ACCUM_L(E12, vres[E12]);				\
	W_ACCUM_L(E13, vres[E13]);				\
	W_ACCUM_L(E14, vres[E14]);				\
	W_ACCUM_L(E15, vres[E15]);				\
	W_ACCUM_L(E16, vres[E16]);				\
	W_ACCUM_L(E17, vres[E17]);				\

INLINE void cfunc_rsp_vand(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
	// ------------------------------------------------------
	//
	// Bitwise AND of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VAND(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1: /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VAND(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2: /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VAND(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3: /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VAND(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4: /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VAND(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5: /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VAND(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6: /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VAND(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7: /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VAND(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8: /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VAND(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9: /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VAND(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:/* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VAND(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:/* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VAND(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:/* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VAND(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:/* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VAND(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:/* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VAND(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:/* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VAND(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
	WRITEBACK_RESULT();
}
#endif

#if (DRC_VNAND)
#define RSP_VNAND_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(0), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E20), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E10].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E21), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E11].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E22), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E12].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E23), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E13].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E24), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E14].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E25), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E15].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E26), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E16].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E27), WORD); \
		UML_AND(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E17].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_LOAD(block, IREG(0), &rsp->accum[E10].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E11].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E12].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E13].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E14].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E15].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E16].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E17].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vnand(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
	// ------------------------------------------------------
	//
	// Bitwise NOT AND of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VNAND_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VNAND_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VNAND_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VNAND_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VNAND_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VNAND_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VNAND_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VNAND_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VNAND_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VNAND_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VNAND_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VNAND_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VNAND_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VNAND_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VNAND_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VNAND_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VNAND(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	vres[E10] = ~(R_VREG_S(VS1REG, E10) & R_VREG_S(VS2REG, E20));	\
	vres[E11] = ~(R_VREG_S(VS1REG, E11) & R_VREG_S(VS2REG, E21));	\
	vres[E12] = ~(R_VREG_S(VS1REG, E12) & R_VREG_S(VS2REG, E22));	\
	vres[E13] = ~(R_VREG_S(VS1REG, E13) & R_VREG_S(VS2REG, E23));	\
	vres[E14] = ~(R_VREG_S(VS1REG, E14) & R_VREG_S(VS2REG, E24));	\
	vres[E15] = ~(R_VREG_S(VS1REG, E15) & R_VREG_S(VS2REG, E25));	\
	vres[E16] = ~(R_VREG_S(VS1REG, E16) & R_VREG_S(VS2REG, E26));	\
	vres[E17] = ~(R_VREG_S(VS1REG, E17) & R_VREG_S(VS2REG, E27));	\
	W_ACCUM_L(E10, vres[E10]);				\
	W_ACCUM_L(E11, vres[E11]);				\
	W_ACCUM_L(E12, vres[E12]);				\
	W_ACCUM_L(E13, vres[E13]);				\
	W_ACCUM_L(E14, vres[E14]);				\
	W_ACCUM_L(E15, vres[E15]);				\
	W_ACCUM_L(E16, vres[E16]);				\
	W_ACCUM_L(E17, vres[E17]);				\

INLINE void cfunc_rsp_vnand(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101001 |
	// ------------------------------------------------------
	//
	// Bitwise NOT AND of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VNAND(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1: /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VNAND(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2: /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VNAND(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3: /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VNAND(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4: /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VNAND(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5: /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VNAND(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6: /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VNAND(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7: /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VNAND(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8: /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VNAND(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9: /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VNAND(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:/* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VNAND(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:/* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VNAND(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:/* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VNAND(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:/* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VNAND(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:/* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VNAND(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:/* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VNAND(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
	WRITEBACK_RESULT();
}
#endif

#if (DRC_VOR)
#define RSP_VOR_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(0), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E20), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E10].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E21), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E11].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E22), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E12].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E23), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E13].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E24), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E14].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E25), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E15].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E26), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E16].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E27), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E17].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_LOAD(block, IREG(0), &rsp->accum[E10].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E11].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E12].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E13].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E14].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E15].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E16].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E17].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vor(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
	// ------------------------------------------------------
	//
	// Bitwise OR of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VOR_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VOR_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VOR_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VOR_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VOR_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VOR_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VOR_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VOR_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VOR_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VOR_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VOR_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VOR_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VOR_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VOR(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	vres[E10] = R_VREG_S(VS1REG, E10) | R_VREG_S(VS2REG, E20);	\
	vres[E11] = R_VREG_S(VS1REG, E11) | R_VREG_S(VS2REG, E21);	\
	vres[E12] = R_VREG_S(VS1REG, E12) | R_VREG_S(VS2REG, E22);	\
	vres[E13] = R_VREG_S(VS1REG, E13) | R_VREG_S(VS2REG, E23);	\
	vres[E14] = R_VREG_S(VS1REG, E14) | R_VREG_S(VS2REG, E24);	\
	vres[E15] = R_VREG_S(VS1REG, E15) | R_VREG_S(VS2REG, E25);	\
	vres[E16] = R_VREG_S(VS1REG, E16) | R_VREG_S(VS2REG, E26);	\
	vres[E17] = R_VREG_S(VS1REG, E17) | R_VREG_S(VS2REG, E27);	\
	W_ACCUM_L(E10, vres[E10]);									\
	W_ACCUM_L(E11, vres[E11]);									\
	W_ACCUM_L(E12, vres[E12]);									\
	W_ACCUM_L(E13, vres[E13]);									\
	W_ACCUM_L(E14, vres[E14]);									\
	W_ACCUM_L(E15, vres[E15]);									\
	W_ACCUM_L(E16, vres[E16]);									\
	W_ACCUM_L(E17, vres[E17]);									\

INLINE void cfunc_rsp_vor(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101010 |
	// ------------------------------------------------------
	//
	// Bitwise OR of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VOR(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1: /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VOR(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2: /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VOR(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3: /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VOR(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4: /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VOR(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5: /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VOR(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6: /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VOR(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7: /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VOR(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8: /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VOR(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9: /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VOR(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:/* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VOR(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:/* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VOR(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:/* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VOR(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:/* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VOR(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:/* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VOR(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:/* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VOR(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
	WRITEBACK_RESULT();
}
#endif

#if (DRC_VNOR)
#define RSP_VNOR_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(0), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E20), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E10].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E21), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E11].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E22), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E12].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E23), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E13].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E24), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E14].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E25), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E15].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E26), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E16].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E27), WORD); \
		UML_OR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E17].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_LOAD(block, IREG(0), &rsp->accum[E10].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E11].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E12].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E13].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E14].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E15].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E16].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E17].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vnor(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
	// ------------------------------------------------------
	//
	// Bitwise NOT OR of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VNOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VNOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VNOR_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VNOR_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VNOR_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VNOR_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VNOR_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VNOR_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VNOR_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VNOR_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VNOR_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VNOR_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VNOR_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VNOR_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VNOR_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VNOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VNOR(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	vres[E10] = ~(R_VREG_S(VS1REG, E10) | R_VREG_S(VS2REG, E20));	\
	vres[E11] = ~(R_VREG_S(VS1REG, E11) | R_VREG_S(VS2REG, E21));	\
	vres[E12] = ~(R_VREG_S(VS1REG, E12) | R_VREG_S(VS2REG, E22));	\
	vres[E13] = ~(R_VREG_S(VS1REG, E13) | R_VREG_S(VS2REG, E23));	\
	vres[E14] = ~(R_VREG_S(VS1REG, E14) | R_VREG_S(VS2REG, E24));	\
	vres[E15] = ~(R_VREG_S(VS1REG, E15) | R_VREG_S(VS2REG, E25));	\
	vres[E16] = ~(R_VREG_S(VS1REG, E16) | R_VREG_S(VS2REG, E26));	\
	vres[E17] = ~(R_VREG_S(VS1REG, E17) | R_VREG_S(VS2REG, E27));	\
	W_ACCUM_L(E10, vres[E10]);										\
	W_ACCUM_L(E11, vres[E11]);										\
	W_ACCUM_L(E12, vres[E12]);										\
	W_ACCUM_L(E13, vres[E13]);										\
	W_ACCUM_L(E14, vres[E14]);										\
	W_ACCUM_L(E15, vres[E15]);										\
	W_ACCUM_L(E16, vres[E16]);										\
	W_ACCUM_L(E17, vres[E17]);										\

INLINE void cfunc_rsp_vnor(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101011 |
	// ------------------------------------------------------
	//
	// Bitwise NOT OR of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VNOR(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1: /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VNOR(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2: /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VNOR(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3: /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VNOR(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4: /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VNOR(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5: /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VNOR(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6: /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VNOR(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7: /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VNOR(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8: /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VNOR(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9: /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VNOR(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:/* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VNOR(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:/* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VNOR(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:/* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VNOR(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:/* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VNOR(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:/* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VNOR(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:/* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VNOR(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
	WRITEBACK_RESULT();
}
#endif

#if (DRC_VXOR)
#define RSP_VXOR_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(0), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E20), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E10].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E21), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E11].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E22), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E12].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E23), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E13].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E24), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E14].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E25), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E15].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E26), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E16].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E27), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_STORE(block, &rsp->accum[E17].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_LOAD(block, IREG(0), &rsp->accum[E10].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E11].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E12].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E13].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E14].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E15].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E16].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E17].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vxor(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
	// ------------------------------------------------------
	//
	// Bitwise XOR of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VXOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VXOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VXOR_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VXOR_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VXOR_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VXOR_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VXOR_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VXOR_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VXOR_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VXOR_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VXOR_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VXOR_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VXOR_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VXOR_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VXOR_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VXOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VXOR(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	vres[E10] = R_VREG_S(VS1REG, E10) ^ R_VREG_S(VS2REG, E20);	\
	vres[E11] = R_VREG_S(VS1REG, E11) ^ R_VREG_S(VS2REG, E21);	\
	vres[E12] = R_VREG_S(VS1REG, E12) ^ R_VREG_S(VS2REG, E22);	\
	vres[E13] = R_VREG_S(VS1REG, E13) ^ R_VREG_S(VS2REG, E23);	\
	vres[E14] = R_VREG_S(VS1REG, E14) ^ R_VREG_S(VS2REG, E24);	\
	vres[E15] = R_VREG_S(VS1REG, E15) ^ R_VREG_S(VS2REG, E25);	\
	vres[E16] = R_VREG_S(VS1REG, E16) ^ R_VREG_S(VS2REG, E26);	\
	vres[E17] = R_VREG_S(VS1REG, E17) ^ R_VREG_S(VS2REG, E27);	\
	W_ACCUM_L(E10, vres[E10]);									\
	W_ACCUM_L(E11, vres[E11]);									\
	W_ACCUM_L(E12, vres[E12]);									\
	W_ACCUM_L(E13, vres[E13]);									\
	W_ACCUM_L(E14, vres[E14]);									\
	W_ACCUM_L(E15, vres[E15]);									\
	W_ACCUM_L(E16, vres[E16]);									\
	W_ACCUM_L(E17, vres[E17]);									\

INLINE void cfunc_rsp_vxor(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101100 |
	// ------------------------------------------------------
	//
	// Bitwise XOR of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VXOR(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1: /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VXOR(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2: /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VXOR(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3: /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VXOR(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4: /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VXOR(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5: /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VXOR(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6: /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VXOR(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7: /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VXOR(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8: /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VXOR(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9: /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VXOR(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:/* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VXOR(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:/* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VXOR(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:/* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VXOR(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:/* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VXOR(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:/* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VXOR(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:/* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VXOR(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
	WRITEBACK_RESULT();
}
#endif

#if (DRC_VNXOR)
#define RSP_VNXOR_DRC(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27, I0, I1, I2, I3, I4, I5, I6, I7) \
	{ \
		UML_SEXT(block, IREG(0), VS(VS1REG, I0), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E20), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E10].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I1), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E21), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E11].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I2), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E22), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E12].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I3), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E23), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E13].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I4), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E24), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E14].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I5), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E25), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E15].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I6), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E26), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E16].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_SEXT(block, IREG(0), VS(VS1REG, I7), WORD); \
		UML_SEXT(block, IREG(1), VS(VS2REG, E27), WORD); \
		UML_XOR(block, IREG(0), IREG(0), IREG(1)); \
		UML_XOR(block, IREG(0), IREG(0), IMM(0xffffffff)); \
		UML_STORE(block, &rsp->accum[E17].h.low, IMM(0), IREG(0), WORD); \
 \
		UML_LOAD(block, IREG(0), &rsp->accum[E10].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I0], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E11].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I1], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E12].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I2], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E13].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I3], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E14].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I4], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E15].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I5], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E16].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I6], IMM(0), IREG(0), WORD); \
		UML_LOAD(block, IREG(0), &rsp->accum[E17].h.low, IMM(0), WORD); \
		UML_STORE(block, &rsp->v[VDREG].s[I7], IMM(0), IREG(0), WORD); \
	}

static int generate_vnxor(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101000 |
	// ------------------------------------------------------
	//
	// Bitwise NOT XOR of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VNXOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 1:    /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VNXOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
		case 2:    /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VNXOR_DRC(1, 3, 5, 7, 0, 2, 4, 6, 7, 5, 3, 1, 7, 5, 3, 1, 6, 4, 2, 0, 7, 5, 3, 1);
			return TRUE;
		case 3:    /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VNXOR_DRC(0, 2, 4, 6, 1, 3, 5, 7, 6, 4, 2, 0, 6, 4, 2, 0, 7, 5, 3, 1, 6, 4, 2, 0);
			return TRUE;
		case 4:    /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VNXOR_DRC(1, 2, 3, 5, 6, 7, 0, 4, 7, 7, 7, 3, 3, 3, 7, 3, 6, 5, 4, 2, 1, 0, 7, 3);
			return TRUE;
		case 5:    /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VNXOR_DRC(0, 2, 3, 4, 6, 7, 1, 5, 6, 6, 6, 2, 2, 2, 6, 2, 7, 5, 4, 3, 1, 0, 6, 2);
			return TRUE;
		case 6:    /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VNXOR_DRC(0, 1, 3, 4, 5, 7, 2, 6, 5, 5, 5, 1, 1, 1, 5, 1, 7, 6, 4, 3, 2, 0, 5, 1);
			return TRUE;
		case 7:    /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VNXOR_DRC(0, 1, 2, 4, 5, 6, 3, 7, 4, 4, 4, 0, 0, 0, 4, 0, 7, 6, 5, 3, 2, 1, 4, 0);
			return TRUE;
		case 8:    /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VNXOR_DRC(1, 2, 3, 4, 5, 6, 7, 0, 7, 7, 7, 7, 7, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0, 7);
			return TRUE;
		case 9:    /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VNXOR_DRC(0, 2, 3, 4, 5, 6, 7, 1, 6, 6, 6, 6, 6, 6, 6, 6, 7, 5, 4, 3, 2, 1, 0, 6);
			return TRUE;
		case 10:   /* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VNXOR_DRC(0, 1, 3, 4, 5, 6, 7, 2, 5, 5, 5, 5, 5, 5, 5, 5, 7, 6, 4, 3, 2, 1, 0, 5);
			return TRUE;
		case 11:   /* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VNXOR_DRC(0, 1, 2, 4, 5, 6, 7, 3, 4, 4, 4, 4, 4, 4, 4, 4, 7, 6, 5, 3, 2, 1, 0, 4);
			return TRUE;
		case 12:   /* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VNXOR_DRC(0, 1, 2, 3, 5, 6, 7, 4, 3, 3, 3, 3, 3, 3, 3, 3, 7, 6, 5, 4, 2, 1, 0, 3);
			return TRUE;
		case 13:   /* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VNXOR_DRC(0, 1, 2, 3, 4, 6, 7, 5, 2, 2, 2, 2, 2, 2, 2, 2, 7, 6, 5, 4, 3, 1, 0, 2);
			return TRUE;
		case 14:   /* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VNXOR_DRC(0, 1, 2, 3, 4, 5, 7, 6, 1, 1, 1, 1, 1, 1, 1, 1, 7, 6, 5, 4, 3, 2, 0, 1);
			return TRUE;
		case 15:   /* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VNOR_DRC(0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 7, 6, 5, 4, 3, 2, 1, 0);
			return TRUE;
	}

	return TRUE;
}
#else
#define RSP_VNXOR(E10, E11, E12, E13, E14, E15, E16, E17, E20, E21, E22, E23, E24, E25, E26, E27) \
	vres[E10] = ~(R_VREG_S(VS1REG, E10) ^ R_VREG_S(VS2REG, E20));	\
	vres[E11] = ~(R_VREG_S(VS1REG, E11) ^ R_VREG_S(VS2REG, E21));	\
	vres[E12] = ~(R_VREG_S(VS1REG, E12) ^ R_VREG_S(VS2REG, E22));	\
	vres[E13] = ~(R_VREG_S(VS1REG, E13) ^ R_VREG_S(VS2REG, E23));	\
	vres[E14] = ~(R_VREG_S(VS1REG, E14) ^ R_VREG_S(VS2REG, E24));	\
	vres[E15] = ~(R_VREG_S(VS1REG, E15) ^ R_VREG_S(VS2REG, E25));	\
	vres[E16] = ~(R_VREG_S(VS1REG, E16) ^ R_VREG_S(VS2REG, E26));	\
	vres[E17] = ~(R_VREG_S(VS1REG, E17) ^ R_VREG_S(VS2REG, E27));	\
	W_ACCUM_L(E10, vres[E10]);										\
	W_ACCUM_L(E11, vres[E11]);										\
	W_ACCUM_L(E12, vres[E12]);										\
	W_ACCUM_L(E13, vres[E13]);										\
	W_ACCUM_L(E14, vres[E14]);										\
	W_ACCUM_L(E15, vres[E15]);										\
	W_ACCUM_L(E16, vres[E16]);										\
	W_ACCUM_L(E17, vres[E17]);										\

INLINE void cfunc_rsp_vnxor(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 101101 |
	// ------------------------------------------------------
	//
	// Bitwise NOT XOR of two vector registers

	switch(EL)
	{
		case 0: /* 0, 1, 2, 3, 4, 5, 6, 7 - none */		/* 0, 1, 2, 3, 4, 5, 6, 7 - none */
			RSP_VNXOR(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 1: /* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */		/* 0, 1, 2, 3, 4, 5, 6, 7 - ???  */
			RSP_VNXOR(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);
			break;
		case 2: /* 1, 3, 5, 7, 0, 2, 4, 6 - 0q   */		/* 0, 0, 2, 2, 4, 4, 6, 6 - 0q   */
			RSP_VNXOR(1, 3, 5, 7, 0, 2, 4, 6, 0, 2, 4, 6, 0, 2, 4, 6);
			break;
		case 3: /* 0, 2, 4, 6, 1, 3, 5, 7 - 1q   */		/* 1, 1, 3, 3, 5, 5, 7, 7 - 1q   */
			RSP_VNXOR(0, 2, 4, 6, 1, 3, 5, 7, 1, 3, 5, 7, 1, 3, 5, 7);
			break;
		case 4: /* 1, 2, 3, 5, 6, 7, 0, 4 - 0h   */		/* 0, 0, 0, 0, 4, 4, 4, 4 - 0h   */
			RSP_VNXOR(1, 2, 3, 5, 6, 7, 0, 4, 0, 0, 0, 4, 4, 4, 0, 4);
			break;
		case 5: /* 0, 2, 3, 4, 6, 7, 1, 5 - 1h   */		/* 1, 1, 1, 1, 5, 5, 5, 5 - 1h   */
			RSP_VNXOR(0, 2, 3, 4, 6, 7, 1, 5, 1, 1, 1, 5, 5, 5, 1, 5);
			break;
		case 6: /* 0, 1, 3, 4, 5, 7, 2, 6 - 2h   */		/* 2, 2, 2, 2, 6, 6, 6, 6 - 2h   */
			RSP_VNXOR(0, 1, 3, 4, 5, 7, 2, 6, 2, 2, 2, 6, 6, 6, 2, 6);
			break;
		case 7: /* 0, 1, 2, 4, 5, 6, 3, 7 - 3h   */		/* 3, 3, 3, 3, 7, 7, 7, 7 - 3h   */
			RSP_VNXOR(0, 1, 2, 4, 5, 6, 3, 7, 3, 3, 3, 7, 7, 7, 3, 7);
			break;
		case 8: /* 1, 2, 3, 4, 5, 6, 7, 0 - 0    */		/*  0, 0, 0, 0, 0, 0, 0, 0 - 0    */
			RSP_VNXOR(1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case 9: /* 0, 2, 3, 4, 5, 6, 7, 1 - 1    */		/*  1, 1, 1, 1, 1, 1, 1, 1 - 0    */
			RSP_VNXOR(0, 2, 3, 4, 5, 6, 7, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case 10:/* 0, 1, 3, 4, 5, 6, 7, 2 - 2    */		/*  2, 2, 2, 2, 2, 2, 2, 2 - 0    */
			RSP_VNXOR(0, 1, 3, 4, 5, 6, 7, 2, 2, 2, 2, 2, 2, 2, 2, 2);
			break;
		case 11:/* 0, 1, 2, 4, 5, 6, 7, 3 - 3    */		/*  3, 3, 3, 3, 3, 3, 3, 3 - 0    */
			RSP_VNXOR(0, 1, 2, 4, 5, 6, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3);
			break;
		case 12:/* 0, 1, 2, 3, 5, 6, 7, 4 - 4    */		/*  4, 4, 4, 4, 4, 4, 4, 4 - 0    */
			RSP_VNXOR(0, 1, 2, 3, 5, 6, 7, 4, 4, 4, 4, 4, 4, 4, 4, 4);
			break;
		case 13:/* 0, 1, 2, 3, 4, 6, 7, 5 - 5    */		/*  5, 5, 5, 5, 5, 5, 5, 5 - 0    */
			RSP_VNXOR(0, 1, 2, 3, 4, 6, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5);
			break;
		case 14:/* 0, 1, 2, 3, 4, 5, 7, 6 - 6    */		/*  6, 6, 6, 6, 6, 6, 6, 6 - 0    */
			RSP_VNXOR(0, 1, 2, 3, 4, 5, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6);
			break;
		case 15:/* 0, 1, 2, 3, 4, 5, 6, 7 - 7    */		/*  7, 7, 7, 7, 7, 7, 7, 7 - 0    */
			RSP_VNXOR(0, 1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7);
			break;
	}
	WRITEBACK_RESULT();
}
#endif

INLINE void cfunc_rsp_vrcp(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110000 |
	// ------------------------------------------------------
	//
	// Calculates reciprocal

	int del = 7 - (VS1REG & 7);
	int sel = 7 - (EL & 7);
	INT32 rec;

	rec = (INT16)(R_VREG_S_X(VS2REG, sel));

	if (rec == 0)
	{
		// divide by zero -> overflow
		rec = 0x7fffffff;
	}
	else
	{
		int sign = 0;
		int exp = 0;
		int mantissa = 0;

		if (rec < 0)
		{
			rec = -rec;	// rec = MINUS rec
			sign = 1;
		}

		// restrict to 10-bit mantissa
		for (i = 15; i >= 0; i--)
		{
			if (rec & (1 << i))
			{
				exp = i;
				mantissa = (rec << (15 - i)) >> 6;
				break;
			}
		}

		if (mantissa == 0x200)
		{
			rec = 0x7fffffff;
		}
		else
		{
			rec = 0xffffffffU / mantissa;

			//
			// simulate rounding error
			//
			// This has been verified on the real hardware.
			//
			// I was able to replicate this exact behaviour by using a five-round
			// Newton reciprocal method using floorf() on intermediate results
			// to force the use of IEEE 754 32bit floats.
			// However, for the sake of portability, we'll use integer arithmetic.
			//
			if (rec & 0x800)
				rec += 1;

			rec <<= 8;
		}

		// restrict result to 17 significant bits
		rec &= 0x7fffc000;

		rec >>= exp;

		if (sign)
		{
			rec = ~rec;	// rec = BITWISE NOT rec
		}
	}

	for (i=0; i < 8; i++)
	{
		int element = VEC_EL_2(EL, i);
		W_ACCUM_L(i, R_VREG_S_X(VS2REG, element));
	}

	rsp->reciprocal_res = rec;

	W_VREG_S_X(VDREG, del, (UINT16)(rsp->reciprocal_res));			// store low part
}

INLINE void cfunc_rsp_vrcpl(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110001 |
	// ------------------------------------------------------
	//
	// Calculates reciprocal low part

	int del = 7 - (VS1REG & 7);
	int sel = 7 - VEC_EL_2(EL, 7-del);
	INT32 rec;

	rec = ((UINT16)(R_VREG_S_X(VS2REG, sel)) | ((UINT32)(rsp->reciprocal_high) << 16));

	if (rec == 0)
	{
		// divide by zero -> overflow
		rec = 0x7fffffff;
	}
	else
	{
		int negative = 0;
		if (rec < 0)
		{
			if (((UINT32)(rec & 0xffff0000) == 0xffff0000) && ((INT16)(rec & 0xffff) < 0))
			{
				rec = ~rec+1;
			}
			else
			{
				rec = ~rec;
			}
			negative = 1;
		}
		for (i = 31; i > 0; i--)
		{
			if (rec & (1 << i))
			{
				rec &= ((0xffc00000) >> (31 - i));
				i = 0;
			}
		}
		rec = (0x7fffffff / rec);
		for (i = 31; i > 0; i--)
		{
			if (rec & (1 << i))
			{
				rec &= ((0xffff8000) >> (31 - i));
				i = 0;
			}
		}
		if (negative)
		{
			rec = ~rec;
		}
	}

	for (i=0; i < 8; i++)
	{
		int element = VEC_EL_2(EL, i);
		W_ACCUM_L(i, R_VREG_S_X(VS2REG, element));
	}

	rsp->reciprocal_res = rec;

	W_VREG_S_X(VDREG, del, (UINT16)(rsp->reciprocal_res));			// store low part
}

INLINE void cfunc_rsp_vrcph(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110010 |
	// ------------------------------------------------------
	//
	// Calculates reciprocal high part

	int del = 7 - (VS1REG & 7);
	int sel = 7 - VEC_EL_2(EL, 7-del);

	rsp->reciprocal_high = R_VREG_S_X(VS2REG, sel);

	W_ACCUM_L(0, R_VREG_S_X(VS2REG, VEC_EL_2(EL, 0)));
	W_ACCUM_L(1, R_VREG_S_X(VS2REG, VEC_EL_2(EL, 1)));
	W_ACCUM_L(2, R_VREG_S_X(VS2REG, VEC_EL_2(EL, 2)));
	W_ACCUM_L(3, R_VREG_S_X(VS2REG, VEC_EL_2(EL, 3)));
	W_ACCUM_L(4, R_VREG_S_X(VS2REG, VEC_EL_2(EL, 4)));
	W_ACCUM_L(5, R_VREG_S_X(VS2REG, VEC_EL_2(EL, 5)));
	W_ACCUM_L(6, R_VREG_S_X(VS2REG, VEC_EL_2(EL, 6)));
	W_ACCUM_L(7, R_VREG_S_X(VS2REG, VEC_EL_2(EL, 7)));

	W_VREG_S_X(VDREG, del, (INT16)(rsp->reciprocal_res >> 16));	// store high part
}

INLINE void cfunc_rsp_vmov(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110011 |
	// ------------------------------------------------------
	//
	// Moves element from vector to destination vector

	int element = 7 - (VS1REG & 7);
	W_VREG_S_X(VDREG, element, R_VREG_S_X(VS2REG, 7 - VEC_EL_2(EL, element)));
}

INLINE void cfunc_rsp_vrsql(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110101 |
	// ------------------------------------------------------
	//
	// Calculates reciprocal square-root low part

	int del = 7 - (VS1REG & 7);
	int sel = 7 - VEC_EL_2(EL, 7 - del);
	INT32 sqr;

	sqr = (UINT16)(R_VREG_S_X(VS2REG, sel)) | ((UINT32)(rsp->square_root_high) << 16);

	if (sqr == 0)
	{
		// square root on 0 -> overflow
		sqr = 0x7fffffff;
	}
	else if (sqr == 0xffff8000)
	{
		// overflow ?
		sqr = 0xffff8000;
	}
	else
	{
		int negative = 0;
		if (sqr < 0)
		{
			if (((UINT32)(sqr & 0xffff0000) == 0xffff0000) && ((INT16)(sqr & 0xffff) < 0))
			{
				sqr = ~sqr+1;
			}
			else
			{
				sqr = ~sqr;
			}
			negative = 1;
		}
		for (i = 31; i > 0; i--)
		{
			if (sqr & (1 << i))
			{
				sqr &= (0xff800000 >> (31 - i));
				i = 0;
			}
		}
		sqr = (INT32)(0x7fffffff / sqrt((double)sqr));
		for (i = 31; i > 0; i--)
		{
			if (sqr & (1 << i))
			{
				sqr &= (0xffff8000 >> (31 - i));
				i = 0;
			}
		}
		if (negative)
		{
			sqr = ~sqr;
		}
	}

	W_ACCUM_L(0, R_VREG_S(VS2REG, VEC_EL_2(EL, 0)));
	W_ACCUM_L(1, R_VREG_S(VS2REG, VEC_EL_2(EL, 1)));
	W_ACCUM_L(2, R_VREG_S(VS2REG, VEC_EL_2(EL, 2)));
	W_ACCUM_L(3, R_VREG_S(VS2REG, VEC_EL_2(EL, 3)));
	W_ACCUM_L(4, R_VREG_S(VS2REG, VEC_EL_2(EL, 4)));
	W_ACCUM_L(5, R_VREG_S(VS2REG, VEC_EL_2(EL, 5)));
	W_ACCUM_L(6, R_VREG_S(VS2REG, VEC_EL_2(EL, 6)));
	W_ACCUM_L(7, R_VREG_S(VS2REG, VEC_EL_2(EL, 7)));

	rsp->square_root_res = sqr;

	W_VREG_S_X(VDREG, del, (UINT16)(rsp->square_root_res));			// store low part
}

INLINE void cfunc_rsp_vrsqh(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	int i;
	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | ?FFFF | DDDDD | 110110 |
	// ------------------------------------------------------
	//
	// Calculates reciprocal square-root high part

	int del = (VS1REG & 7);
	int sel = VEC_EL_2(EL, del);

	rsp->square_root_high = R_VREG_S(VS2REG, sel);

	for (i=0; i < 8; i++)
	{
		int element = VEC_EL_2(EL, i);
		W_ACCUM_L(i, R_VREG_S(VS2REG, element));		// perhaps accumulator is used to store the intermediate values ?
	}

	W_VREG_S(VDREG, del, (INT16)(rsp->square_root_res >> 16));	// store high part
}
static void cfunc_sp_set_status_cb(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	(rsp->config->sp_set_status)(rsp->device, rsp->impstate->arg0);
}

static CPU_EXECUTE( rsp )
{
	rsp_state *rsp = get_safe_token(device);
	drcuml_state *drcuml = rsp->impstate->drcuml;
	int execute_result;

	/* reset the cache if dirty */
	if (rsp->impstate->cache_dirty)
		code_flush_cache(rsp);
	rsp->impstate->cache_dirty = FALSE;

	/* execute */
	rsp->icount = cycles;
	do
	{
		if( rsp->sr & ( RSP_STATUS_HALT | RSP_STATUS_BROKE ) )
		{
			rsp->icount = MIN(rsp->icount, 0);
			break;
		}

		/* run as much as we can */
		execute_result = drcuml_execute(drcuml, rsp->impstate->entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
		{
			code_compile_block(rsp, rsp->pc);
		}
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
		{
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", rsp->pc);
		}
		else if (execute_result == EXECUTE_RESET_CACHE)
		{
			code_flush_cache(rsp);
		}
	} while (execute_result != EXECUTE_OUT_OF_CYCLES);

	/* return the number of cycles executed */
	return cycles - rsp->icount;
}

/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    rspdrc_flush_drc_cache - outward-facing
    accessor to code_flush_cache
-------------------------------------------------*/

void rspdrc_flush_drc_cache(running_device *device)
{
	rsp_state *rsp = get_safe_token(device);
	rsp->impstate->cache_dirty = TRUE;
}

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

static void code_flush_cache(rsp_state *rsp)
{
	/* empty the transient cache contents */
	drcuml_reset(rsp->impstate->drcuml);

	/* generate the entry point and out-of-cycles handlers */
	static_generate_entry_point(rsp);
	static_generate_nocode_handler(rsp);
	static_generate_out_of_cycles(rsp);

	/* add subroutines for memory accesses */
	static_generate_memory_accessor(rsp, 1, FALSE, "read8",       &rsp->impstate->read8);
	static_generate_memory_accessor(rsp, 1, TRUE,  "write8",      &rsp->impstate->write8);
	static_generate_memory_accessor(rsp, 2, FALSE, "read16",      &rsp->impstate->read16);
	static_generate_memory_accessor(rsp, 2, TRUE,  "write16",     &rsp->impstate->write16);
	static_generate_memory_accessor(rsp, 4, FALSE, "read32",      &rsp->impstate->read32);
	static_generate_memory_accessor(rsp, 4, TRUE,  "write32",     &rsp->impstate->write32);
}


/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

static void code_compile_block(rsp_state *rsp, offs_t pc)
{
	drcuml_state *drcuml = rsp->impstate->drcuml;
	compiler_state compiler = { 0 };
	const opcode_desc *seqhead, *seqlast;
	const opcode_desc *desclist;
	int override = FALSE;
	drcuml_block *block;
	jmp_buf errorbuf;

	profiler_mark_start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	desclist = drcfe_describe_code(rsp->impstate->drcfe, pc);

	/* if we get an error back, flush the cache and try again */
	if (setjmp(errorbuf) != 0)
	{
		code_flush_cache(rsp);
	}

	/* start the block */
	block = drcuml_block_begin(drcuml, 8192, &errorbuf);

	/* loop until we get through all instruction sequences */
	for (seqhead = desclist; seqhead != NULL; seqhead = seqlast->next)
	{
		const opcode_desc *curdesc;
		UINT32 nextpc;

		/* add a code log entry */
		if (LOG_UML)
			UML_COMMENT(block, "-------------------------");						// comment

		/* determine the last instruction in this sequence */
		for (seqlast = seqhead; seqlast != NULL; seqlast = seqlast->next)
			if (seqlast->flags & OPFLAG_END_SEQUENCE)
				break;
		assert(seqlast != NULL);

		/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
		if (override || !drcuml_hash_exists(drcuml, 0, seqhead->pc))
			UML_HASH(block, 0, seqhead->pc);										// hash    mode,pc

		/* if we already have a hash, and this is the first sequence, assume that we */
		/* are recompiling due to being out of sync and allow future overrides */
		else if (seqhead == desclist)
		{
			override = TRUE;
			UML_HASH(block, 0, seqhead->pc);										// hash    mode,pc
		}

		/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
		else
		{
			UML_LABEL(block, seqhead->pc | 0x80000000);								// label   seqhead->pc
			UML_HASHJMP(block, IMM(0), IMM(seqhead->pc), rsp->impstate->nocode);
																					// hashjmp <0>,seqhead->pc,nocode
			continue;
		}

		/* validate this code block if we're not pointing into ROM */
		if (memory_get_write_ptr(rsp->program, seqhead->physpc) != NULL)
			generate_checksum_block(rsp, block, &compiler, seqhead, seqlast);

		/* label this instruction, if it may be jumped to locally */
		if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
			UML_LABEL(block, seqhead->pc | 0x80000000);								// label   seqhead->pc

		/* iterate over instructions in the sequence and compile them */
		for (curdesc = seqhead; curdesc != seqlast->next; curdesc = curdesc->next)
			generate_sequence_instruction(rsp, block, &compiler, curdesc);

		/* if we need to return to the start, do it */
		if (seqlast->flags & OPFLAG_RETURN_TO_START)
			nextpc = pc;

		/* otherwise we just go to the next instruction */
		else
			nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;

		/* count off cycles and go there */
		generate_update_cycles(rsp, block, &compiler, IMM(nextpc), TRUE);			// <subtract cycles>

		/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
		if (seqlast->next == NULL || seqlast->next->pc != nextpc)
			UML_HASHJMP(block, IMM(0), IMM(nextpc), rsp->impstate->nocode);			// hashjmp <mode>,nextpc,nocode
	}

	/* end the sequence */
	drcuml_block_end(block);
	profiler_mark_end();
}

/***************************************************************************
    C FUNCTION CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    cfunc_unimplemented - handler for
    unimplemented opcdes
-------------------------------------------------*/

static void cfunc_unimplemented(void *param)
{
	rsp_state *rsp = (rsp_state *)param;
	UINT32 opcode = rsp->impstate->arg0;
	fatalerror("PC=%08X: Unimplemented op %08X (%02X,%02X)", rsp->pc, opcode, opcode >> 26, opcode & 0x3f);
}


/*-------------------------------------------------
    cfunc_fatalerror - a generic fatalerror call
-------------------------------------------------*/

//static void cfunc_fatalerror(void *param)
//{
	//fatalerror("fatalerror");
//}


/***************************************************************************
    STATIC CODEGEN
***************************************************************************/

/*-------------------------------------------------
    ferate_entry_point - generate a
    static entry point
-------------------------------------------------*/

static void static_generate_entry_point(rsp_state *rsp)
{
	drcuml_state *drcuml = rsp->impstate->drcuml;
	drcuml_block *block;
	jmp_buf errorbuf;

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Unrecoverable error in static_generate_entry_point");

	/* begin generating */
	block = drcuml_block_begin(drcuml, 20, &errorbuf);

	/* forward references */
	alloc_handle(drcuml, &rsp->impstate->nocode, "nocode");

	alloc_handle(drcuml, &rsp->impstate->entry, "entry");
	UML_HANDLE(block, rsp->impstate->entry);										// handle  entry

	/* load fast integer registers */
	load_fast_iregs(rsp, block);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, IMM(0), MEM(&rsp->pc), rsp->impstate->nocode);
																					// hashjmp <mode>,<pc>,nocode
	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

static void static_generate_nocode_handler(rsp_state *rsp)
{
	drcuml_state *drcuml = rsp->impstate->drcuml;
	drcuml_block *block;
	jmp_buf errorbuf;

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
	{
		fatalerror("Unrecoverable error in static_generate_nocode_handler");
	}

	/* begin generating */
	block = drcuml_block_begin(drcuml, 10, &errorbuf);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &rsp->impstate->nocode, "nocode");
	UML_HANDLE(block, rsp->impstate->nocode);										// handle  nocode
	UML_GETEXP(block, IREG(0));														// getexp  i0
	UML_MOV(block, MEM(&rsp->pc), IREG(0));											// mov     [pc],i0
	save_fast_iregs(rsp, block);
	UML_EXIT(block, IMM(EXECUTE_MISSING_CODE));										// exit    EXECUTE_MISSING_CODE

	drcuml_block_end(block);
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

static void static_generate_out_of_cycles(rsp_state *rsp)
{
	drcuml_state *drcuml = rsp->impstate->drcuml;
	drcuml_block *block;
	jmp_buf errorbuf;

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
	{
		fatalerror("Unrecoverable error in static_generate_out_of_cycles");
	}

	/* begin generating */
	block = drcuml_block_begin(drcuml, 10, &errorbuf);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &rsp->impstate->out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, rsp->impstate->out_of_cycles);								// handle  out_of_cycles
	UML_GETEXP(block, IREG(0));														// getexp  i0
	UML_MOV(block, MEM(&rsp->pc), IREG(0));											// mov     <pc>,i0
	save_fast_iregs(rsp, block);
	UML_EXIT(block, IMM(EXECUTE_OUT_OF_CYCLES));									// exit    EXECUTE_OUT_OF_CYCLES

	drcuml_block_end(block);
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

static void static_generate_memory_accessor(rsp_state *rsp, int size, int iswrite, const char *name, drcuml_codehandle **handleptr)
{
	/* on entry, address is in I0; data for writes is in I1; mask for accesses is in I2 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I3 */
	drcuml_state *drcuml = rsp->impstate->drcuml;
	drcuml_block *block;
	jmp_buf errorbuf;
#ifdef LSB_FIRST
	int unaligned_case = 1;
#endif

	/* if we get an error back, we're screwed */
	if (setjmp(errorbuf) != 0)
	{
		fatalerror("Unrecoverable error in static_generate_exception");
	}

	/* begin generating */
	block = drcuml_block_begin(drcuml, 1024, &errorbuf);

	/* add a global entry for this */
	alloc_handle(drcuml, handleptr, name);
	UML_HANDLE(block, *handleptr);													// handle  *handleptr

	// write:
	if (iswrite)
	{
		if (size == 1)
		{
#ifdef LSB_FIRST
			UML_XOR(block, IREG(0), IREG(0), IMM(3));									// xor     i0,i0,3
#endif
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));							// and     i0,i0,0xfff
			UML_STORE(block, rsp->impstate->dmem, IREG(0), IREG(1), BYTE);				// store   dmem,i0,i1,byte
		}
		else if (size == 2)
		{
#ifdef LSB_FIRST
			UML_TEST(block, IREG(0), IMM(1));											// test    i0,1
			UML_JMPc(block, IF_NZ, unaligned_case);										// jnz     <unaligned_case>
			UML_XOR(block, IREG(0), IREG(0), IMM(2));									// xor     i0,i0,2
#endif
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));							// and     i0,i0,0xfff
			UML_STORE(block, rsp->impstate->dmem, IREG(0), IREG(1), WORD_x1);			// store   dmem,i0,i1,word_x1
			UML_RET(block);
#ifdef LSB_FIRST
			UML_LABEL(block, unaligned_case);										// unaligned_case:
			UML_AND(block, IREG(2), IREG(0), IMM(3));									// and     i2,i0,3
			UML_AND(block, IREG(0), IREG(0), IMM(0xffc));								// and     i0,i0,0xffc
			UML_SHL(block, IREG(2), IREG(2), IMM(3));									// shl     i2,i2,3
			UML_DLOAD(block, IREG(3), rsp->impstate->dmem, IREG(0), QWORD_x1);			// dload   i3,dmem,i0,qword_x1
			UML_ADD(block, IREG(2), IREG(2), IMM(48));									// add     i2,i2,48
			UML_DAND(block, IREG(1), IREG(1), IMM(0xffff));								// dand    i1,i1,0xffff
			UML_DROLAND(block, IREG(3), IREG(3), IREG(2), IMM(U64(0xffffffffffff0000)));// droland i3,i3,i2,~0xffff
			UML_DOR(block, IREG(1), IREG(1), IREG(3));									// dor     i1,i1,i3
			UML_DROR(block, IREG(1), IREG(1), IREG(2));									// dror    i1,i1,i2
			UML_DSTORE(block, rsp->impstate->dmem, IREG(0), IREG(1), QWORD_x1); 		// dstore  dmem,i0,i1,qword_x1
#endif
		}
		else if (size == 4)
		{
#ifdef LSB_FIRST
			UML_TEST(block, IREG(0), IMM(3));											// test    i0,3
			UML_JMPc(block, IF_NZ, unaligned_case);										// jnz     <unaligned_case>
#endif
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));							// and     i0,i0,0xfff
			UML_STORE(block, rsp->impstate->dmem, IREG(0), IREG(1), DWORD_x1);			// store   dmem,i0,i1,dword_x1
			UML_RET(block);
#ifdef LSB_FIRST
			UML_LABEL(block, unaligned_case);										// unaligned_case:
			UML_AND(block, IREG(2), IREG(0), IMM(3));									// and     i2,i0,3
			UML_AND(block, IREG(0), IREG(0), IMM(0xffc));								// and     i0,i0,0xffc
			UML_SHL(block, IREG(2), IREG(2), IMM(3));									// shl     i2,i2,3
			UML_DLOAD(block, IREG(3), rsp->impstate->dmem, IREG(0), QWORD_x1);			// dload   i3,dmem,i0,qword_x1
			UML_DAND(block, IREG(1), IREG(1), IMM(0xffffffff));							// dand    i1,i1,0xffffffff
			UML_DROLAND(block, IREG(3), IREG(3), IREG(2), IMM(U64(0xffffffff00000000)));// droland i3,i3,i2,~0xffffffff
			UML_DOR(block, IREG(1), IREG(1), IREG(3));									// dor     i1,i1,i3
			UML_DROR(block, IREG(1), IREG(1), IREG(2));									// dror    i1,i1,i2
			UML_DSTORE(block, rsp->impstate->dmem, IREG(0), IREG(1), QWORD_x1); 		// dstore  dmem,i0,i1,qword_x1
#endif
		}
	}
	else
	{
		if (size == 1)
		{
#ifdef LSB_FIRST
			UML_XOR(block, IREG(0), IREG(0), IMM(3));									// xor     i0,i0,3
#endif
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));							// and     i0,i0,0xfff
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), BYTE);				// load    i0,dmem,i0,byte
		}
		else if (size == 2)
		{
#ifdef LSB_FIRST
			UML_TEST(block, IREG(0), IMM(1));											// test    i0,1
			UML_JMPc(block, IF_NZ, unaligned_case);										// jnz     <unaligned_case>
			UML_XOR(block, IREG(0), IREG(0), IMM(2));									// xor     i0,i0,2
#endif
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));							// and     i0,i0,0xfff
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), WORD_x1);			// load    i0,dmem,i0,word_x1
			UML_RET(block);
#ifdef LSB_FIRST
			UML_LABEL(block, unaligned_case);										// unaligned_case:
			UML_AND(block, IREG(1), IREG(0), IMM(3));									// and     i1,i0,3
			UML_AND(block, IREG(0), IREG(0), IMM(0xffc));								// and     i0,i0,0xffc
			UML_SHL(block, IREG(1), IREG(1), IMM(3));									// shl     i1,i1,3
			UML_DLOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), QWORD_x1);			// dload   i0,dmem,i0,qword_x1
			UML_ADD(block, IREG(1), IREG(1), IMM(48));									// add     i1,i1,48
			UML_DROLAND(block, IREG(0), IREG(0), IREG(1), IMM(0xffff));					// droland i0,i0,i1,0xffff
#endif
		}
		else if (size == 4)
		{
#ifdef LSB_FIRST
			UML_TEST(block, IREG(0), IMM(3));											// test    i0,3
			UML_JMPc(block, IF_NZ, unaligned_case);										// jnz     <unaligned_case>
#endif
			UML_AND(block, IREG(0), IREG(0), IMM(0x00000fff));							// and     i0,i0,0xfff
			UML_LOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), DWORD_x1);			// load    i0,dmem,i0,dword_x1
			UML_RET(block);
#ifdef LSB_FIRST
			UML_LABEL(block, unaligned_case);										// unaligned_case:
			UML_AND(block, IREG(1), IREG(0), IMM(3));									// and     i1,i0,3
			UML_AND(block, IREG(0), IREG(0), IMM(0xffc));								// and     i0,i0,0xffc
			UML_SHL(block, IREG(1), IREG(1), IMM(3));									// shl     i1,i1,3
			UML_DLOAD(block, IREG(0), rsp->impstate->dmem, IREG(0), QWORD_x1);			// dload   i0,dmem,i0,qword_x1
			UML_DROL(block, IREG(0), IREG(0), IREG(1));									// drol    i0,i0,i1
#endif
		}
	}
	UML_RET(block);

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
static void generate_update_cycles(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, drcuml_ptype ptype, UINT64 pvalue, int allow_exception)
{
	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, MEM(&rsp->icount), MEM(&rsp->icount), MAPVAR_CYCLES);		// sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);										// mapvar  cycles,0
		UML_EXHc(block, IF_S, rsp->impstate->out_of_cycles, PARAM(ptype, pvalue));
	}
	compiler->cycles = 0;
}

/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

static void generate_checksum_block(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (LOG_UML)
	{
		UML_COMMENT(block, "[Validation for %08X]", seqhead->pc | 0x1000);					// comment
	}
	/* loose verify or single instruction: just compare and fail */
	if (!(rsp->impstate->drcoptions & RSPDRC_STRICT_VERIFY) || seqhead->next == NULL)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			void *base = memory_decrypted_read_ptr(rsp->program, seqhead->physpc | 0x1000);
			UML_LOAD(block, IREG(0), base, IMM(0), DWORD);							// load    i0,base,0,dword
			UML_CMP(block, IREG(0), IMM(seqhead->opptr.l[0]));						// cmp     i0,opptr[0]
			UML_EXHc(block, IF_NE, rsp->impstate->nocode, IMM(epc(seqhead)));		// exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
		UINT32 sum = 0;
		void *base = memory_decrypted_read_ptr(rsp->program, seqhead->physpc | 0x1000);
		UML_LOAD(block, IREG(0), base, IMM(0), DWORD);								// load    i0,base,0,dword
		sum += seqhead->opptr.l[0];
		for (curdesc = seqhead->next; curdesc != seqlast->next; curdesc = curdesc->next)
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = memory_decrypted_read_ptr(rsp->program, curdesc->physpc | 0x1000);
				UML_LOAD(block, IREG(1), base, IMM(0), DWORD);						// load    i1,base,dword
				UML_ADD(block, IREG(0), IREG(0), IREG(1));							// add     i0,i0,i1
				sum += curdesc->opptr.l[0];
			}
		UML_CMP(block, IREG(0), IMM(sum));											// cmp     i0,sum
		UML_EXHc(block, IF_NE, rsp->impstate->nocode, IMM(epc(seqhead)));			// exne    nocode,seqhead->pc
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

static void generate_sequence_instruction(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	offs_t expc;

	/* add an entry for the log */
	if (LOG_UML && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(rsp, block, desc->pc, desc->opptr.l[0]);

	/* set the PC map variable */
	expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 3 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, expc);												// mapvar  PC,expc

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);								// mapvar  CYCLES,compiler->cycles

	/* if we are debugging, call the debugger */
	if ((rsp->device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, MEM(&rsp->pc), IMM(desc->pc));								// mov     [pc],desc->pc
		save_fast_iregs(rsp, block);
		UML_DEBUG(block, IMM(desc->pc));											// debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
	//if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	//{
	//  UML_MOV(block, MEM(&rsp->pc), IMM(desc->pc));                               // mov     [pc],desc->pc
	//  save_fast_iregs(rsp, block);
	//  UML_EXIT(block, IMM(EXECUTE_UNMAPPED_CODE));                                // exit    EXECUTE_UNMAPPED_CODE
	//}

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	/*else*/ if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(rsp, block, compiler, desc))
		{
			UML_MOV(block, MEM(&rsp->pc), IMM(desc->pc));							// mov     [pc],desc->pc
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_unimplemented, rsp);								// callc   cfunc_unimplemented
		}
	}
}

/*------------------------------------------------------------------
    generate_delay_slot_and_branch
------------------------------------------------------------------*/

static void generate_delay_slot_and_branch(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg)
{
	compiler_state compiler_temp = *compiler;
	UINT32 op = desc->opptr.l[0];

	/* fetch the target register if dynamic, in case it is modified by the delay slot */
	if (desc->targetpc == BRANCH_TARGET_DYNAMIC)
	{
		UML_AND(block, MEM(&rsp->impstate->jmpdest), R32(RSREG), IMM(0x00000fff));
		UML_OR(block, MEM(&rsp->impstate->jmpdest), MEM(&rsp->impstate->jmpdest), IMM(0x1000));
	}

	/* set the link if needed -- before the delay slot */
	if (linkreg != 0)
	{
		UML_MOV(block, R32(linkreg), IMM((INT32)(desc->pc + 8)));					// mov    <linkreg>,desc->pc + 8
	}

	/* compile the delay slot using temporary compiler state */
	assert(desc->delay != NULL);
	generate_sequence_instruction(rsp, block, &compiler_temp, desc->delay);		// <next instruction>

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(rsp, block, &compiler_temp, IMM(desc->targetpc), TRUE);	// <subtract cycles>
		if (desc->flags & OPFLAG_INTRABLOCK_BRANCH)
		{
			UML_JMP(block, desc->targetpc | 0x80000000);							// jmp     desc->targetpc
		}
		else
		{
			UML_HASHJMP(block, IMM(0), IMM(desc->targetpc), rsp->impstate->nocode);
																					// hashjmp <mode>,desc->targetpc,nocode
		}
	}
	else
	{
		generate_update_cycles(rsp, block, &compiler_temp, MEM(&rsp->impstate->jmpdest), TRUE);
																					// <subtract cycles>
		UML_HASHJMP(block, IMM(0), MEM(&rsp->impstate->jmpdest), rsp->impstate->nocode);
																					// hashjmp <mode>,<rsreg>,nocode
	}

	/* update the label */
	compiler->labelnum = compiler_temp.labelnum;

	/* reset the mapvar to the current cycles and account for skipped slots */
	compiler->cycles += desc->skipslots;
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);								// mapvar  CYCLES,compiler->cycles
}


/*-------------------------------------------------
    generate_opcode - generate code for a specific
    opcode
-------------------------------------------------*/

static int generate_vector_opcode(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	// Opcode legend:
	//    E = VS2 element type
	//    S = VS1, Source vector 1
	//    T = VS2, Source vector 2
	//    D = Destination vector

	switch (op & 0x3f)
	{
		case 0x00:		/* VMULF */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmulf, rsp);
			return TRUE;

		case 0x01:		/* VMULU */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmulu, rsp);
			return TRUE;

		case 0x04:		/* VMUDL */
#if (DRC_VMUDL)
			return generate_vmudl(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudl, rsp);
			return TRUE;
#endif

		case 0x05:		/* VMUDM */
#if (DRC_VMUDM)
			return generate_vmudm(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudm, rsp);
			return TRUE;
#endif

		case 0x06:		/* VMUDN */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudn, rsp);
			return TRUE;

		case 0x07:		/* VMUDH */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudh, rsp);
			return TRUE;

		case 0x08:		/* VMACF */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmacf, rsp);
			return TRUE;

		case 0x09:		/* VMACU */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmacu, rsp);
			return TRUE;

		case 0x0c:		/* VMADL */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadl, rsp);
			return TRUE;

		case 0x0d:		/* VMADM */
#if (DRC_VMADM)
			return generate_vmadm(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadm, rsp);
			return TRUE;
#endif

		case 0x0e:		/* VMADN */
#if (DRC_VMADN)
			return generate_vmadn(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadn, rsp);
			return TRUE;
#endif

		case 0x0f:		/* VMADH */
#if (DRC_VMADH)
			return generate_vmadh(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadh, rsp);
			return TRUE;
#endif

		case 0x10:		/* VADD */
#if (DRC_VADD)
			return generate_vadd(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vadd, rsp);
			return TRUE;
#endif

		case 0x11:		/* VSUB */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsub, rsp);
			return TRUE;

		case 0x13:		/* VABS */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vabs, rsp);
			return TRUE;

		case 0x14:		/* VADDC */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vaddc, rsp);
			return TRUE;

		case 0x15:		/* VSUBC */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsubc, rsp);
			return TRUE;

		case 0x1d:		/* VSAW */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsaw, rsp);
			return TRUE;

		case 0x20:		/* VLT */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vlt, rsp);
			return TRUE;

		case 0x21:		/* VEQ */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_veq, rsp);
			return TRUE;

		case 0x22:		/* VNE */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vne, rsp);
			return TRUE;

		case 0x23:		/* VGE */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vge, rsp);
			return TRUE;

		case 0x24:		/* VCL */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vcl, rsp);
			return TRUE;

		case 0x25:		/* VCH */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vch, rsp);
			return TRUE;

		case 0x26:		/* VCR */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vcr, rsp);
			return TRUE;

		case 0x27:		/* VMRG */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmrg, rsp);
			return TRUE;

		case 0x28:		/* VAND */
#if (DRC_VAND)
			return generate_vand(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vand, rsp);
			return TRUE;
#endif

		case 0x29:		/* VNAND */
#if (DRC_VNAND)
			return generate_vnand(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnand, rsp);
			return TRUE;
#endif

		case 0x2a:		/* VOR */
#if (DRC_VOR)
			return generate_vor(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vor, rsp);
			return TRUE;
#endif

		case 0x2b:		/* VNOR */
#if (DRC_VNOR)
			return generate_vnor(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnor, rsp);
			return TRUE;
#endif

		case 0x2c:		/* VXOR */
#if (DRC_VXOR)
			return generate_vxor(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vxor, rsp);
			return TRUE;
#endif

		case 0x2d:		/* VNXOR */
#if (DRC_VNXOR)
			return generate_vnxor(rsp, block, compiler, desc);
#else
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnxor, rsp);
			return TRUE;
#endif

		case 0x30:		/* VRCP */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcp, rsp);
			return TRUE;

		case 0x31:		/* VRCPL */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcpl, rsp);
			return TRUE;

		case 0x32:		/* VRCPH */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcph, rsp);
			return TRUE;

		case 0x33:		/* VMOV */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmov, rsp);
			return TRUE;

		case 0x35:		/* VRSQL */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrsql, rsp);
			return TRUE;

		case 0x36:		/* VRSQH */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrsqh, rsp);
			return TRUE;

		default:
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_unimplemented_opcode, rsp);
			return FALSE;
	}
}

static int generate_opcode(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = op >> 26;
	drcuml_codelabel skip;

	switch (opswitch)
	{
		/* ----- sub-groups ----- */

		case 0x00:	/* SPECIAL - MIPS I */
			return generate_special(rsp, block, compiler, desc);

		case 0x01:	/* REGIMM - MIPS I */
			return generate_regimm(rsp, block, compiler, desc);

		/* ----- jumps and branches ----- */

		case 0x02:	/* J - MIPS I */
			generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);		// <next instruction + hashjmp>
			return TRUE;

		case 0x03:	/* JAL - MIPS I */
			generate_delay_slot_and_branch(rsp, block, compiler, desc, 31);		// <next instruction + hashjmp>
			return TRUE;

		case 0x04:	/* BEQ - MIPS I */
			UML_CMP(block, R32(RSREG), R32(RTREG));								// cmp    <rsreg>,<rtreg>
			UML_JMPc(block, IF_NE, skip = compiler->labelnum++);				// jmp    skip,NE
			generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);		// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;

		case 0x05:	/* BNE - MIPS I */
			UML_CMP(block, R32(RSREG), R32(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_JMPc(block, IF_E, skip = compiler->labelnum++);						// jmp     skip,E
			generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);		// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;

		case 0x06:	/* BLEZ - MIPS I */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), IMM(0));								// dcmp    <rsreg>,0
				UML_JMPc(block, IF_G, skip = compiler->labelnum++);					// jmp     skip,G
				generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);	// <next instruction + hashjmp>
				UML_LABEL(block, skip);											// skip:
			}
			else
				generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);	// <next instruction + hashjmp>
			return TRUE;

		case 0x07:	/* BGTZ - MIPS I */
			UML_CMP(block, R32(RSREG), IMM(0));									// dcmp    <rsreg>,0
			UML_JMPc(block, IF_LE, skip = compiler->labelnum++);					// jmp     skip,LE
			generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);		// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;


		/* ----- immediate arithmetic ----- */

		case 0x0f:	/* LUI - MIPS I */
			if (RTREG != 0)
				UML_MOV(block, R32(RTREG), IMM(SIMMVAL << 16));					// dmov    <rtreg>,SIMMVAL << 16
			return TRUE;

		case 0x08:	/* ADDI - MIPS I */
		case 0x09:	/* ADDIU - MIPS I */
			if (RTREG != 0)
			{
				UML_ADD(block, R32(RTREG), R32(RSREG), IMM(SIMMVAL));				// add     i0,<rsreg>,SIMMVAL,V
			}
			return TRUE;

		case 0x0a:	/* SLTI - MIPS I */
			if (RTREG != 0)
			{
				UML_CMP(block, R32(RSREG), IMM(SIMMVAL));							// dcmp    <rsreg>,SIMMVAL
				UML_SETc(block, IF_L, R32(RTREG));									// dset    <rtreg>,l
			}
			return TRUE;

		case 0x0b:	/* SLTIU - MIPS I */
			if (RTREG != 0)
			{
				UML_CMP(block, R32(RSREG), IMM(SIMMVAL));							// dcmp    <rsreg>,SIMMVAL
				UML_SETc(block, IF_B, R32(RTREG));									// dset    <rtreg>,b
			}
			return TRUE;


		case 0x0c:	/* ANDI - MIPS I */
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), R32(RSREG), IMM(UIMMVAL));				// dand    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0d:	/* ORI - MIPS I */
			if (RTREG != 0)
				UML_OR(block, R32(RTREG), R32(RSREG), IMM(UIMMVAL));				// dor     <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0e:	/* XORI - MIPS I */
			if (RTREG != 0)
				UML_XOR(block, R32(RTREG), R32(RSREG), IMM(UIMMVAL));				// dxor    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		/* ----- memory load operations ----- */

		case 0x20:	/* LB - MIPS I */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, rsp->impstate->read8);									// callh   read8
			if (RTREG != 0)
				UML_SEXT(block, R32(RTREG), IREG(0), BYTE);						// dsext   <rtreg>,i0,byte
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, IMM(desc->pc + 4), TRUE);
			return TRUE;

		case 0x21:	/* LH - MIPS I */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, rsp->impstate->read16);								// callh   read16
			if (RTREG != 0)
				UML_SEXT(block, R32(RTREG), IREG(0), WORD);						// dsext   <rtreg>,i0,word
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, IMM(desc->pc + 4), TRUE);
			return TRUE;

		case 0x23:	/* LW - MIPS I */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, rsp->impstate->read32);								// callh   read32
			if (RTREG != 0)
				UML_MOV(block, R32(RTREG), IREG(0));
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, IMM(desc->pc + 4), TRUE);
			return TRUE;

		case 0x24:	/* LBU - MIPS I */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, rsp->impstate->read8);									// callh   read8
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), IREG(0), IMM(0xff));					// dand    <rtreg>,i0,0xff
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, IMM(desc->pc + 4), TRUE);
			return TRUE;

		case 0x25:	/* LHU - MIPS I */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, rsp->impstate->read16);								// callh   read16
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), IREG(0), IMM(0xffff));					// dand    <rtreg>,i0,0xffff
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, IMM(desc->pc + 4), TRUE);
			return TRUE;

		case 0x32:	/* LWC2 - MIPS I */
			return generate_lwc2(rsp, block, compiler, desc);


		/* ----- memory store operations ----- */

		case 0x28:	/* SB - MIPS I */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, IREG(1), R32(RTREG));									// mov     i1,<rtreg>
			UML_CALLH(block, rsp->impstate->write8);								// callh   write8
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, IMM(desc->pc + 4), TRUE);
			return TRUE;

		case 0x29:	/* SH - MIPS I */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, IREG(1), R32(RTREG));									// mov     i1,<rtreg>
			UML_CALLH(block, rsp->impstate->write16);								// callh   write16
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, IMM(desc->pc + 4), TRUE);
			return TRUE;

		case 0x2b:	/* SW - MIPS I */
			UML_ADD(block, IREG(0), R32(RSREG), IMM(SIMMVAL));						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, IREG(1), R32(RTREG));									// mov     i1,<rtreg>
			UML_CALLH(block, rsp->impstate->write32);								// callh   write32
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, IMM(desc->pc + 4), TRUE);
			return TRUE;

		case 0x3a:	/* SWC2 - MIPS I */
			return generate_swc2(rsp, block, compiler, desc);
			//UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));     // mov     [arg0],desc->opptr.l
			//UML_CALLC(block, cfunc_swc2, rsp);                                        // callc   cfunc_mfc2
			//return TRUE;

		/* ----- coprocessor instructions ----- */

		case 0x10:	/* COP0 - MIPS I */
			return generate_cop0(rsp, block, compiler, desc);

		case 0x12:	/* COP2 - MIPS I */
			return generate_cop2(rsp, block, compiler, desc);
			//UML_EXH(block, rsp->impstate->exception[EXCEPTION_INVALIDOP], IMM(0));// exh     invalidop,0
			//return TRUE;


		/* ----- unimplemented/illegal instructions ----- */

//      default:    /* ??? */       invalid_instruction(op);                                                break;
	}

	return FALSE;
}


/*-------------------------------------------------
    generate_special - compile opcodes in the
    'SPECIAL' group
-------------------------------------------------*/

static int generate_special(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = op & 63;
	//drcuml_codelabel skip;

	switch (opswitch)
	{
		/* ----- shift instructions ----- */

		case 0x00:	/* SLL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, R32(RDREG), R32(RTREG), IMM(SHIFT));
			}
			return TRUE;

		case 0x02:	/* SRL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, R32(RDREG), R32(RTREG), IMM(SHIFT));
			}
			return TRUE;

		case 0x03:	/* SRA - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, R32(RDREG), R32(RTREG), IMM(SHIFT));
			}
			return TRUE;

		case 0x04:	/* SLLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, R32(RDREG), R32(RTREG), R32(RSREG));
			}
			return TRUE;

		case 0x06:	/* SRLV - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, R32(RDREG), R32(RTREG), R32(RSREG));
			}
			return TRUE;

		case 0x07:	/* SRAV - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, R32(RDREG), R32(RTREG), R32(RSREG));
			}
			return TRUE;

		/* ----- basic arithmetic ----- */

		case 0x20:	/* ADD - MIPS I */
		case 0x21:	/* ADDU - MIPS I */
			if (RDREG != 0)
			{
				UML_ADD(block, R32(RDREG), R32(RSREG), R32(RTREG));
			}
			return TRUE;

		case 0x22:	/* SUB - MIPS I */
		case 0x23:	/* SUBU - MIPS I */
			if (RDREG != 0)
			{
				UML_SUB(block, R32(RDREG), R32(RSREG), R32(RTREG));
			}
			return TRUE;

		/* ----- basic logical ops ----- */

		case 0x24:	/* AND - MIPS I */
			if (RDREG != 0)
			{
				UML_AND(block, R32(RDREG), R32(RSREG), R32(RTREG));				// dand     <rdreg>,<rsreg>,<rtreg>
			}
			return TRUE;

		case 0x25:	/* OR - MIPS I */
			if (RDREG != 0)
			{
				UML_OR(block, R32(RDREG), R32(RSREG), R32(RTREG));					// dor      <rdreg>,<rsreg>,<rtreg>
			}
			return TRUE;

		case 0x26:	/* XOR - MIPS I */
			if (RDREG != 0)
			{
				UML_XOR(block, R32(RDREG), R32(RSREG), R32(RTREG));				// dxor     <rdreg>,<rsreg>,<rtreg>
			}
			return TRUE;

		case 0x27:	/* NOR - MIPS I */
			if (RDREG != 0)
			{
				UML_OR(block, IREG(0), R32(RSREG), R32(RTREG));					// dor      i0,<rsreg>,<rtreg>
				UML_XOR(block, R32(RDREG), IREG(0), IMM((UINT64)~0));				// dxor     <rdreg>,i0,~0
			}
			return TRUE;


		/* ----- basic comparisons ----- */

		case 0x2a:	/* SLT - MIPS I */
			if (RDREG != 0)
			{
				UML_CMP(block, R32(RSREG), R32(RTREG));							// dcmp    <rsreg>,<rtreg>
				UML_SETc(block, IF_L, R32(RDREG));									// dset    <rdreg>,l
			}
			return TRUE;

		case 0x2b:	/* SLTU - MIPS I */
			if (RDREG != 0)
			{
				UML_CMP(block, R32(RSREG), R32(RTREG));							// dcmp    <rsreg>,<rtreg>
				UML_SETc(block, IF_B, R32(RDREG));									// dset    <rdreg>,b
			}
			return TRUE;


		/* ----- jumps and branches ----- */

		case 0x08:	/* JR - MIPS I */
			generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);		// <next instruction + hashjmp>
			return TRUE;

		case 0x09:	/* JALR - MIPS I */
			generate_delay_slot_and_branch(rsp, block, compiler, desc, RDREG);	// <next instruction + hashjmp>
			return TRUE;


		/* ----- system calls ----- */

		case 0x0d:	/* BREAK - MIPS I */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(3));					// mov     [arg0],3
			UML_CALLC(block, cfunc_sp_set_status_cb, rsp);						// callc   cfunc_sp_set_status_cb
			UML_MOV(block, MEM(&rsp->icount), IMM(0));						// mov icount, #0

			UML_EXIT(block, IMM(EXECUTE_OUT_OF_CYCLES));
			return TRUE;
	}
	return FALSE;
}



/*-------------------------------------------------
    generate_regimm - compile opcodes in the
    'REGIMM' group
-------------------------------------------------*/

static int generate_regimm(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = RTREG;
	drcuml_codelabel skip;

	switch (opswitch)
	{
		case 0x00:	/* BLTZ */
		case 0x10:	/* BLTZAL */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), IMM(0));								// dcmp    <rsreg>,0
				UML_JMPc(block, IF_GE, skip = compiler->labelnum++);				// jmp     skip,GE
				generate_delay_slot_and_branch(rsp, block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
				UML_LABEL(block, skip);											// skip:
			}
			return TRUE;

		case 0x01:	/* BGEZ */
		case 0x11:	/* BGEZAL */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), IMM(0));								// dcmp    <rsreg>,0
				UML_JMPc(block, IF_L, skip = compiler->labelnum++);					// jmp     skip,L
				generate_delay_slot_and_branch(rsp, block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
				UML_LABEL(block, skip);											// skip:
			}
			else
				generate_delay_slot_and_branch(rsp, block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
			return TRUE;
	}
	return FALSE;
}


/*-------------------------------------------------
    generate_cop2 - compile COP2 opcodes
-------------------------------------------------*/

static int generate_cop2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = RSREG;

	switch (opswitch)
	{
		case 0x00:	/* MFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));	// mov     [arg0],desc->opptr.l
				UML_CALLC(block, cfunc_mfc2, rsp);									// callc   cfunc_mfc2
				//UML_SEXT(block, R32(RTREG), IREG(0), DWORD);                      // dsext   <rtreg>,i0,dword
			}
			return TRUE;

		case 0x02:	/* CFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));	// mov     [arg0],desc->opptr.l
				UML_CALLC(block, cfunc_cfc2, rsp);									// callc   cfunc_cfc2
				//UML_SEXT(block, R32(RTREG), IREG(0), DWORD);                      // dsext   <rtreg>,i0,dword
			}
			return TRUE;

		case 0x04:	/* MTCz */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_mtc2, rsp);										// callc   cfunc_mtc2
			return TRUE;

		case 0x06:	/* CTCz */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(desc->opptr.l[0]));		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_ctc2, rsp);										// callc   cfunc_ctc2
			return TRUE;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			return generate_vector_opcode(rsp, block, compiler, desc);
	}
	return FALSE;
}

/*-------------------------------------------------
    generate_cop0 - compile COP0 opcodes
-------------------------------------------------*/

static int generate_cop0(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = RSREG;

	switch (opswitch)
	{
		case 0x00:	/* MFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(RDREG));				// mov     [arg0],<rdreg>
				UML_MOV(block, MEM(&rsp->impstate->arg1), IMM(RTREG));				// mov     [arg1],<rtreg>
				UML_CALLC(block, cfunc_get_cop0_reg, rsp);							// callc   cfunc_get_cop0_reg
			}
			return TRUE;

		case 0x04:	/* MTCz */
			UML_MOV(block, MEM(&rsp->impstate->arg0), IMM(RDREG));					// mov     [arg0],<rdreg>
			UML_MOV(block, MEM(&rsp->impstate->arg1), R32(RTREG));					// mov     [arg1],rtreg
			UML_CALLC(block, cfunc_set_cop0_reg, rsp);								// callc   cfunc_set_cop0_reg
			return TRUE;
	}

	return FALSE;
}

static void cfunc_mfc2(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int el = (op >> 7) & 0xf;
	UINT16 b1 = R_VREG_B(VS1REG, (el+0) & 0xf);
	UINT16 b2 = R_VREG_B(VS1REG, (el+1) & 0xf);
	if (RTREG) RTVAL = (INT32)(INT16)((b1 << 8) | (b2));
}

static void cfunc_cfc2(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	if (RTREG)
	{
		if (RDREG == 2)
		{
			// Anciliary clipping flags
			RTVAL = rsp->flag[RDREG] & 0x00ff;
		}
		else
		{
			// All other flags are 16 bits but sign-extended at retrieval
			RTVAL = (UINT32)rsp->flag[RDREG] | ( ( rsp->flag[RDREG] & 0x8000 ) ? 0xffff0000 : 0 );
		}
	}
}

static void cfunc_mtc2(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	int el = (op >> 7) & 0xf;
	W_VREG_B(VS1REG, (el+0) & 0xf, (RTVAL >> 8) & 0xff);
	W_VREG_B(VS1REG, (el+1) & 0xf, (RTVAL >> 0) & 0xff);
}

static void cfunc_ctc2(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
	rsp->flag[RDREG] = RTVAL & 0xffff;
}

/***************************************************************************
    CODE LOGGING HELPERS
***************************************************************************/

/*-------------------------------------------------
    log_add_disasm_comment - add a comment
    including disassembly of a RSP instruction
-------------------------------------------------*/

static void log_add_disasm_comment(rsp_state *rsp, drcuml_block *block, UINT32 pc, UINT32 op)
{
#if (LOG_UML)
	char buffer[100];
	rsp_dasm_one(buffer, pc, op);
	UML_COMMENT(block, "%08X: %s", pc, buffer);										// comment
#endif
}


static CPU_SET_INFO( rsp )
{
	rsp_state *rsp = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
        case CPUINFO_INT_REGISTER + RSP_PC:             rsp->pc = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R0:             rsp->r[0] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R1:             rsp->r[1] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R2:             rsp->r[2] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R3:             rsp->r[3] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R4:             rsp->r[4] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R5:             rsp->r[5] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R6:             rsp->r[6] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R7:             rsp->r[7] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R8:             rsp->r[8] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R9:             rsp->r[9] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R10:            rsp->r[10] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R11:            rsp->r[11] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R12:            rsp->r[12] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R13:            rsp->r[13] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R14:            rsp->r[14] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R15:            rsp->r[15] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R16:            rsp->r[16] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R17:            rsp->r[17] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R18:            rsp->r[18] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R19:            rsp->r[19] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R20:            rsp->r[20] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R21:            rsp->r[21] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R22:            rsp->r[22] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R23:            rsp->r[23] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R24:            rsp->r[24] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R25:            rsp->r[25] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R26:            rsp->r[26] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R27:            rsp->r[27] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R28:            rsp->r[28] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R29:            rsp->r[29] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_R30:            rsp->r[30] = info->i;        break;
        case CPUINFO_INT_SP:
        case CPUINFO_INT_REGISTER + RSP_R31:            rsp->r[31] = info->i;        break;
        case CPUINFO_INT_REGISTER + RSP_SR:             rsp->sr = info->i;           break;
        case CPUINFO_INT_REGISTER + RSP_NEXTPC:         rsp->nextpc = info->i;       break;
        case CPUINFO_INT_REGISTER + RSP_STEPCNT:        rsp->step_count = info->i;   break;
	}
}

CPU_GET_INFO( rsp )
{
	rsp_state *rsp = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(rsp_state);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case DEVINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case DEVINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE:					info->i = CLEAR_LINE;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = rsp->ppc;						break;

		case CPUINFO_INT_PC:	/* intentional fallthrough */
		case CPUINFO_INT_REGISTER + RSP_PC:				info->i = rsp->pc;						break;

		case CPUINFO_INT_REGISTER + RSP_R0:				info->i = rsp->r[0];						break;
		case CPUINFO_INT_REGISTER + RSP_R1:				info->i = rsp->r[1];						break;
		case CPUINFO_INT_REGISTER + RSP_R2:				info->i = rsp->r[2];						break;
		case CPUINFO_INT_REGISTER + RSP_R3:				info->i = rsp->r[3];						break;
		case CPUINFO_INT_REGISTER + RSP_R4:				info->i = rsp->r[4];						break;
		case CPUINFO_INT_REGISTER + RSP_R5:				info->i = rsp->r[5];						break;
		case CPUINFO_INT_REGISTER + RSP_R6:				info->i = rsp->r[6];						break;
		case CPUINFO_INT_REGISTER + RSP_R7:				info->i = rsp->r[7];						break;
		case CPUINFO_INT_REGISTER + RSP_R8:				info->i = rsp->r[8];						break;
		case CPUINFO_INT_REGISTER + RSP_R9:				info->i = rsp->r[9];						break;
		case CPUINFO_INT_REGISTER + RSP_R10:			info->i = rsp->r[10];					break;
		case CPUINFO_INT_REGISTER + RSP_R11:			info->i = rsp->r[11];					break;
		case CPUINFO_INT_REGISTER + RSP_R12:			info->i = rsp->r[12];					break;
		case CPUINFO_INT_REGISTER + RSP_R13:			info->i = rsp->r[13];					break;
		case CPUINFO_INT_REGISTER + RSP_R14:			info->i = rsp->r[14];					break;
		case CPUINFO_INT_REGISTER + RSP_R15:			info->i = rsp->r[15];					break;
		case CPUINFO_INT_REGISTER + RSP_R16:			info->i = rsp->r[16];					break;
		case CPUINFO_INT_REGISTER + RSP_R17:			info->i = rsp->r[17];					break;
		case CPUINFO_INT_REGISTER + RSP_R18:			info->i = rsp->r[18];					break;
		case CPUINFO_INT_REGISTER + RSP_R19:			info->i = rsp->r[19];					break;
		case CPUINFO_INT_REGISTER + RSP_R20:			info->i = rsp->r[20];					break;
		case CPUINFO_INT_REGISTER + RSP_R21:			info->i = rsp->r[21];					break;
		case CPUINFO_INT_REGISTER + RSP_R22:			info->i = rsp->r[22];					break;
		case CPUINFO_INT_REGISTER + RSP_R23:			info->i = rsp->r[23];					break;
		case CPUINFO_INT_REGISTER + RSP_R24:			info->i = rsp->r[24];					break;
		case CPUINFO_INT_REGISTER + RSP_R25:			info->i = rsp->r[25];					break;
		case CPUINFO_INT_REGISTER + RSP_R26:			info->i = rsp->r[26];					break;
		case CPUINFO_INT_REGISTER + RSP_R27:			info->i = rsp->r[27];					break;
		case CPUINFO_INT_REGISTER + RSP_R28:			info->i = rsp->r[28];					break;
		case CPUINFO_INT_REGISTER + RSP_R29:			info->i = rsp->r[29];					break;
		case CPUINFO_INT_REGISTER + RSP_R30:			info->i = rsp->r[30];					break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + RSP_R31:			info->i = rsp->r[31];					break;
		case CPUINFO_INT_REGISTER + RSP_SR:             info->i = rsp->sr;                       break;
		case CPUINFO_INT_REGISTER + RSP_NEXTPC:         info->i = rsp->nextpc;                   break;
        case CPUINFO_INT_REGISTER + RSP_STEPCNT:        info->i = rsp->step_count;               break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(rsp);			break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(rsp);					break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(rsp);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(rsp);					break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(rsp);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(rsp);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &rsp->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "RSP");					break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "RSP");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team");	break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + RSP_PC:				sprintf(info->s, "PC: %08X", rsp->pc);	break;

		case CPUINFO_STR_REGISTER + RSP_R0:				sprintf(info->s, "R0: %08X", rsp->r[0]); break;
		case CPUINFO_STR_REGISTER + RSP_R1:				sprintf(info->s, "R1: %08X", rsp->r[1]); break;
		case CPUINFO_STR_REGISTER + RSP_R2:				sprintf(info->s, "R2: %08X", rsp->r[2]); break;
		case CPUINFO_STR_REGISTER + RSP_R3:				sprintf(info->s, "R3: %08X", rsp->r[3]); break;
		case CPUINFO_STR_REGISTER + RSP_R4:				sprintf(info->s, "R4: %08X", rsp->r[4]); break;
		case CPUINFO_STR_REGISTER + RSP_R5:				sprintf(info->s, "R5: %08X", rsp->r[5]); break;
		case CPUINFO_STR_REGISTER + RSP_R6:				sprintf(info->s, "R6: %08X", rsp->r[6]); break;
		case CPUINFO_STR_REGISTER + RSP_R7:				sprintf(info->s, "R7: %08X", rsp->r[7]); break;
		case CPUINFO_STR_REGISTER + RSP_R8:				sprintf(info->s, "R8: %08X", rsp->r[8]); break;
		case CPUINFO_STR_REGISTER + RSP_R9:				sprintf(info->s, "R9: %08X", rsp->r[9]); break;
		case CPUINFO_STR_REGISTER + RSP_R10:			sprintf(info->s, "R10: %08X", rsp->r[10]); break;
		case CPUINFO_STR_REGISTER + RSP_R11:			sprintf(info->s, "R11: %08X", rsp->r[11]); break;
		case CPUINFO_STR_REGISTER + RSP_R12:			sprintf(info->s, "R12: %08X", rsp->r[12]); break;
		case CPUINFO_STR_REGISTER + RSP_R13:			sprintf(info->s, "R13: %08X", rsp->r[13]); break;
		case CPUINFO_STR_REGISTER + RSP_R14:			sprintf(info->s, "R14: %08X", rsp->r[14]); break;
		case CPUINFO_STR_REGISTER + RSP_R15:			sprintf(info->s, "R15: %08X", rsp->r[15]); break;
		case CPUINFO_STR_REGISTER + RSP_R16:			sprintf(info->s, "R16: %08X", rsp->r[16]); break;
		case CPUINFO_STR_REGISTER + RSP_R17:			sprintf(info->s, "R17: %08X", rsp->r[17]); break;
		case CPUINFO_STR_REGISTER + RSP_R18:			sprintf(info->s, "R18: %08X", rsp->r[18]); break;
		case CPUINFO_STR_REGISTER + RSP_R19:			sprintf(info->s, "R19: %08X", rsp->r[19]); break;
		case CPUINFO_STR_REGISTER + RSP_R20:			sprintf(info->s, "R20: %08X", rsp->r[20]); break;
		case CPUINFO_STR_REGISTER + RSP_R21:			sprintf(info->s, "R21: %08X", rsp->r[21]); break;
		case CPUINFO_STR_REGISTER + RSP_R22:			sprintf(info->s, "R22: %08X", rsp->r[22]); break;
		case CPUINFO_STR_REGISTER + RSP_R23:			sprintf(info->s, "R23: %08X", rsp->r[23]); break;
		case CPUINFO_STR_REGISTER + RSP_R24:			sprintf(info->s, "R24: %08X", rsp->r[24]); break;
		case CPUINFO_STR_REGISTER + RSP_R25:			sprintf(info->s, "R25: %08X", rsp->r[25]); break;
		case CPUINFO_STR_REGISTER + RSP_R26:			sprintf(info->s, "R26: %08X", rsp->r[26]); break;
		case CPUINFO_STR_REGISTER + RSP_R27:			sprintf(info->s, "R27: %08X", rsp->r[27]); break;
		case CPUINFO_STR_REGISTER + RSP_R28:			sprintf(info->s, "R28: %08X", rsp->r[28]); break;
		case CPUINFO_STR_REGISTER + RSP_R29:			sprintf(info->s, "R29: %08X", rsp->r[29]); break;
		case CPUINFO_STR_REGISTER + RSP_R30:			sprintf(info->s, "R30: %08X", rsp->r[30]); break;
		case CPUINFO_STR_REGISTER + RSP_R31:			sprintf(info->s, "R31: %08X", rsp->r[31]); break;
		case CPUINFO_STR_REGISTER + RSP_SR:             sprintf(info->s, "SR: %08X",  rsp->sr);    break;
		case CPUINFO_STR_REGISTER + RSP_NEXTPC:         sprintf(info->s, "NPC: %08X", rsp->nextpc);break;
        case CPUINFO_STR_REGISTER + RSP_STEPCNT:        sprintf(info->s, "STEP: %d",  rsp->step_count);  break;
	}
}
