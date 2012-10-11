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
#include "rsp.h"
#include "rspdiv.h"
#include "rspfe.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

using namespace uml;

CPU_DISASSEMBLE( rsp );

extern offs_t rsp_dasm_one(char *buffer, offs_t pc, UINT32 op);

#ifdef USE_RSPDRC

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define FORCE_C_BACKEND					(0)
#define LOG_UML							(0)
#define LOG_NATIVE						(0)

#define SINGLE_INSTRUCTION_MODE			(0)


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* map variables */
#define MAPVAR_PC						M0
#define MAPVAR_CYCLES					M1

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

#define R32(reg)				rsp->impstate->regmap[reg]

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* fast RAM info */
struct fast_ram_info
{
	offs_t				start;						/* start of the RAM block */
	offs_t				end;						/* end of the RAM block */
	UINT8				readonly;					/* TRUE if read-only */
	void *				base;						/* base in memory where the RAM lives */
};


/* internal compiler state */
struct compiler_state
{
	UINT32				cycles;						/* accumulated cycles */
	UINT8				checkints;					/* need to check interrupts before next instruction */
	UINT8				checksoftints;				/* need to check software interrupts before next instruction */
	code_label	labelnum;					/* index for local labels */
};

struct rspimp_state
{
	/* core state */
	drc_cache *			cache;						/* pointer to the DRC code cache */
	drcuml_state *		drcuml;						/* DRC UML generator state */
	rsp_frontend *		drcfe;						/* pointer to the DRC front-end state */
	UINT32				drcoptions;					/* configurable DRC options */

	/* internal stuff */
	UINT8				cache_dirty;				/* true if we need to flush the cache */
	UINT32				jmpdest;					/* destination jump target */

	/* parameters for subroutines */
	UINT64				numcycles;					/* return value from gettotalcycles */
	const char *		format;						/* format string for print_debug */
	UINT32				arg0;						/* print_debug argument 1 */
	UINT32				arg1;						/* print_debug argument 2 */
	UINT32				arg2;						/* print_debug argument 3 */
	UINT32				arg3;						/* print_debug argument 4 */
	UINT32				vres[8];					/* used for temporary vector results */

	/* register mappings */
	parameter	regmap[34];					/* parameter to register mappings for all 32 integer registers */

	/* subroutines */
	code_handle *	entry;						/* entry point */
	code_handle *	nocode;						/* nocode exception handler */
	code_handle *	out_of_cycles;				/* out of cycles exception handler */
	code_handle *	read8;						/* read byte */
	code_handle *	write8;						/* write byte */
	code_handle *	read16;						/* read half */
	code_handle *	write16;					/* write half */
	code_handle *	read32;						/* read word */
	code_handle *	write32;					/* write word */
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
static void cfunc_rsp_lsv(void *param);
static void cfunc_rsp_llv(void *param);
static void cfunc_rsp_ldv(void *param);
static void cfunc_rsp_lqv(void *param);
static void cfunc_rsp_lrv(void *param);
static void cfunc_rsp_lpv(void *param);
static void cfunc_rsp_luv(void *param);
static void cfunc_rsp_lhv(void *param);
static void cfunc_rsp_lfv(void *param);
static void cfunc_rsp_lwv(void *param);
static void cfunc_rsp_ltv(void *param);

static void cfunc_rsp_sbv(void *param);
static void cfunc_rsp_ssv(void *param);
static void cfunc_rsp_slv(void *param);
static void cfunc_rsp_sdv(void *param);
static void cfunc_rsp_sqv(void *param);
static void cfunc_rsp_srv(void *param);
static void cfunc_rsp_spv(void *param);
static void cfunc_rsp_suv(void *param);
static void cfunc_rsp_shv(void *param);
static void cfunc_rsp_sfv(void *param);
static void cfunc_rsp_swv(void *param);
static void cfunc_rsp_stv(void *param);

static void static_generate_entry_point(rsp_state *rsp);
static void static_generate_nocode_handler(rsp_state *rsp);
static void static_generate_out_of_cycles(rsp_state *rsp);
static void static_generate_memory_accessor(rsp_state *rsp, int size, int iswrite, const char *name, code_handle *&handleptr);

static int generate_lwc2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static int generate_swc2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static void generate_update_cycles(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, parameter param, int allow_exception);
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

#define VREG_B(reg, offset)		rsp->v[(reg)].b[(offset)^1]
#define W_VREG_S(reg, offset)		rsp->v[(reg)].s[(offset)]
#define VREG_S(reg, offset)		(INT16)rsp->v[(reg)].s[(offset)]

#define VEC_EL_2(x,z)				(vector_elements_2[(x)][(z)])

#define ACCUM(x)		rsp->accum[x].q
#define ACCUM_H(x)		rsp->accum[((x))].w[3]
#define ACCUM_M(x)		rsp->accum[((x))].w[2]
#define ACCUM_L(x)		rsp->accum[((x))].w[1]

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

INLINE rsp_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == RSP);
	return *(rsp_state **)downcast<legacy_cpu_device *>(device)->token();
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

INLINE void alloc_handle(drcuml_state *drcuml, code_handle **handleptr, const char *name)
{
	if (*handleptr == NULL)
		*handleptr = drcuml->handle_alloc(name);
}


/*-------------------------------------------------
    load_fast_iregs - load any fast integer
    registers
-------------------------------------------------*/

INLINE void load_fast_iregs(rsp_state *rsp, drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(rsp->impstate->regmap); regnum++)
		if (rsp->impstate->regmap[regnum].is_int_register())
			UML_MOV(block, ireg(rsp->impstate->regmap[regnum].ireg() - REG_I0), mem(&rsp->r[regnum]));
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

INLINE void save_fast_iregs(rsp_state *rsp, drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(rsp->impstate->regmap); regnum++)
		if (rsp->impstate->regmap[regnum].is_int_register())
			UML_MOV(block, mem(&rsp->r[regnum]), ireg(rsp->impstate->regmap[regnum].ireg() - REG_I0));
}

/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

void rspdrc_add_imem(device_t *device, UINT32 *base)
{
	rsp_state *rsp = get_safe_token(device);
	rsp->imem32 = base;
	rsp->imem16 = (UINT16*)base;
	rsp->imem8 = (UINT8*)base;
}

void rspdrc_add_dmem(device_t *device, UINT32 *base)
{
	rsp_state *rsp = get_safe_token(device);
	rsp->dmem32 = base;
	rsp->dmem16 = (UINT16*)base;
	rsp->dmem8 = (UINT8*)base;
}

INLINE UINT8 READ8(rsp_state *rsp, UINT32 address)
{
	UINT8 ret = rsp->dmem8[BYTE4_XOR_BE(address & 0xfff)];
	//printf("%04xr%02x\n",address, ret);
	return ret;
}

static void cfunc_read8(void *param)
{
	rsp_state *rsp = (rsp_state *)param;
	rsp->impstate->arg0 = READ8(rsp, rsp->impstate->arg0);
}

INLINE UINT16 READ16(rsp_state *rsp, UINT32 address)
{
	UINT16 ret;
	address &= 0xfff;
	ret = rsp->dmem8[BYTE4_XOR_BE(address)] << 8;
	ret |= rsp->dmem8[BYTE4_XOR_BE(address + 1)];
	//printf("%04xr%04x\n",address, ret);
	return ret;
}

static void cfunc_read16(void *param)
{
	rsp_state *rsp = (rsp_state *)param;
	rsp->impstate->arg0 = READ16(rsp, rsp->impstate->arg0);
}

INLINE UINT32 READ32(rsp_state *rsp, UINT32 address)
{
	UINT32 ret;
	address &= 0xfff;
	ret = rsp->dmem8[BYTE4_XOR_BE(address)] << 24;
	ret |= rsp->dmem8[BYTE4_XOR_BE(address + 1)] << 16;
	ret |= rsp->dmem8[BYTE4_XOR_BE(address + 2)] << 8;
	ret |= rsp->dmem8[BYTE4_XOR_BE(address + 3)];
	//printf("%04xr%08x\n",address, ret);
	return ret;
}

static void cfunc_read32(void *param)
{
	rsp_state *rsp = (rsp_state *)param;
	rsp->impstate->arg0 = READ32(rsp, rsp->impstate->arg0);
}

INLINE void WRITE8(rsp_state *rsp, UINT32 address, UINT8 data)
{
	address &= 0xfff;
	rsp->dmem8[BYTE4_XOR_BE(address)] = data;
	//printf("%04x:%02x\n",address, data);
}

static void cfunc_write8(void *param)
{
	rsp_state *rsp = (rsp_state *)param;
	WRITE8(rsp, rsp->impstate->arg0, (UINT8)rsp->impstate->arg1);
}

INLINE void WRITE16(rsp_state *rsp, UINT32 address, UINT16 data)
{
	address &= 0xfff;
	rsp->dmem8[BYTE4_XOR_BE(address)] = data >> 8;
	rsp->dmem8[BYTE4_XOR_BE(address + 1)] = data & 0xff;
	//printf("%04x:%04x\n",address, data);
}

static void cfunc_write16(void *param)
{
	rsp_state *rsp = (rsp_state *)param;
	WRITE16(rsp, rsp->impstate->arg0, (UINT16)rsp->impstate->arg1);
}

INLINE void WRITE32(rsp_state *rsp, UINT32 address, UINT32 data)
{
	address &= 0xfff;
	rsp->dmem8[BYTE4_XOR_BE(address)] = data >> 24;
	rsp->dmem8[BYTE4_XOR_BE(address + 1)] = (data >> 16) & 0xff;
	rsp->dmem8[BYTE4_XOR_BE(address + 2)] = (data >> 8) & 0xff;
	rsp->dmem8[BYTE4_XOR_BE(address + 3)] = data & 0xff;
	//printf("%04x:%08x\n",address, data);
}

static void cfunc_write32(void *param)
{
	rsp_state *rsp = (rsp_state *)param;
	WRITE32(rsp, rsp->impstate->arg0, rsp->impstate->arg1);
}

/*****************************************************************************/

/*-------------------------------------------------
    rspdrc_set_options - configure DRC options
-------------------------------------------------*/

void rspdrc_set_options(device_t *device, UINT32 options)
{
	rsp_state *rsp = get_safe_token(device);
	rsp->impstate->drcoptions = options;
}


/*-------------------------------------------------
    cfunc_printf_debug - generic printf for
    debugging
-------------------------------------------------*/

#ifdef UNUSED_CODE
static void cfunc_printf_debug(void *param)
{
	rsp_state *rsp = (rsp_state *)param;
	switch(rsp->impstate->arg2)
	{
		case 0: // WRITE8
			printf("%04x:%02x\n", rsp->impstate->arg0 & 0xffff, (UINT8)rsp->impstate->arg1);
			break;
		case 1: // WRITE16
			printf("%04x:%04x\n", rsp->impstate->arg0 & 0xffff, (UINT16)rsp->impstate->arg1);
			break;
		case 2: // WRITE32
			printf("%04x:%08x\n", rsp->impstate->arg0 & 0xffff, rsp->impstate->arg1);
			break;
		case 3: // READ8
			printf("%04xr%02x\n", rsp->impstate->arg0 & 0xffff, (UINT8)rsp->impstate->arg1);
			break;
		case 4: // READ16
			printf("%04xr%04x\n", rsp->impstate->arg0 & 0xffff, (UINT16)rsp->impstate->arg1);
			break;
		case 5: // READ32
			printf("%04xr%08x\n", rsp->impstate->arg0 & 0xffff, rsp->impstate->arg1);
			break;
		case 6: // Checksum
			printf("Sum: %08x\n", rsp->impstate->arg0);
			break;
		case 7: // Checksum
			printf("Correct Sum: %08x\n", rsp->impstate->arg0);
			break;
		default: // ???
			printf("%08x %08x\n", rsp->impstate->arg0 & 0xffff, rsp->impstate->arg1);
			break;
	}
}
#endif

static void cfunc_get_cop0_reg(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int reg = rsp->impstate->arg0;
	int dest = rsp->impstate->arg1;

	if (reg >= 0 && reg < 8)
	{
		if(dest)
		{
			rsp->r[dest] = (rsp->sp_reg_r_func)(reg, 0x00000000);
		}
	}
	else if (reg >= 8 && reg < 16)
	{
		if(dest)
		{
			rsp->r[dest] = (rsp->dp_reg_r_func)(reg - 8, 0x00000000);
		}
	}
	else
	{
		fatalerror("RSP: cfunc_get_cop0_reg: %d\n", reg);
	}
}

static void cfunc_set_cop0_reg(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int reg = rsp->impstate->arg0;
	UINT32 data = rsp->impstate->arg1;

	if (reg >= 0 && reg < 8)
	{
		(rsp->sp_reg_w_func)(reg, data, 0x00000000);
	}
	else if (reg >= 8 && reg < 16)
	{
		(rsp->dp_reg_w_func)(reg - 8, data, 0x00000000);
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
	if ((rsp->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		char string[200];
		rsp_dasm_one(string, rsp->ppc, op);
		mame_printf_debug("%08X: %s\n", rsp->ppc, string);
	}

	fatalerror("RSP: unknown opcode %02X (%08X) at %08X\n", op >> 26, op, rsp->ppc);
}

static void unimplemented_opcode(rsp_state *rsp, UINT32 op)
{
	if ((rsp->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
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

static void rspcom_init(rsp_state *rsp, legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback)
{
	int regIdx = 0;
    int accumIdx;

	memset(rsp, 0, sizeof(*rsp));

	const rsp_config *config = (const rsp_config *)device->static_config();
	// resolve callbacks
	rsp->dp_reg_r_func.resolve(config->dp_reg_r_cb, *device);
	rsp->dp_reg_w_func.resolve(config->dp_reg_w_cb, *device);
	rsp->sp_reg_r_func.resolve(config->sp_reg_r_cb, *device);
	rsp->sp_reg_w_func.resolve(config->sp_reg_w_cb, *device);
	rsp->sp_set_status_func.resolve(config->sp_set_status_cb, *device);
	
	rsp->irq_callback = irqcallback;
	rsp->device = device;
	rsp->program = &device->space(AS_PROGRAM);
	rsp->direct = &rsp->program->direct();

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
		rsp->accum[accumIdx].q = 0;
	}

	rsp->sr = RSP_STATUS_HALT;
	rsp->step_count = 0;
}

static CPU_INIT( rsp )
{
	rsp_state *rsp;
	drc_cache *cache;
	UINT32 flags = 0;
	int regnum;
	//int elnum;

	/* allocate enough space for the cache and the core */
	cache = auto_alloc(device->machine(), drc_cache(CACHE_SIZE + sizeof(*rsp)));

	/* allocate the core memory */
	*(rsp_state **)device->token() = rsp = (rsp_state *)cache->alloc_near(sizeof(*rsp));
	memset(rsp, 0, sizeof(*rsp));

	rspcom_init(rsp, device, irqcallback);

	/* allocate the implementation-specific state from the full cache */
	rsp->impstate = (rspimp_state *)cache->alloc_near(sizeof(*rsp->impstate));
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
	rsp->impstate->drcuml = auto_alloc(device->machine(), drcuml_state(*device, *cache, flags, 8, 32, 2));

	/* add symbols for our stuff */
	rsp->impstate->drcuml->symbol_add(&rsp->pc, sizeof(rsp->pc), "pc");
	rsp->impstate->drcuml->symbol_add(&rsp->icount, sizeof(rsp->icount), "icount");
	for (regnum = 0; regnum < 32; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		rsp->impstate->drcuml->symbol_add(&rsp->r[regnum], sizeof(rsp->r[regnum]), buf);
	}
	rsp->impstate->drcuml->symbol_add(&rsp->impstate->arg0, sizeof(rsp->impstate->arg0), "arg0");
	rsp->impstate->drcuml->symbol_add(&rsp->impstate->arg1, sizeof(rsp->impstate->arg1), "arg1");
	rsp->impstate->drcuml->symbol_add(&rsp->impstate->arg2, sizeof(rsp->impstate->arg2), "arg2");
	rsp->impstate->drcuml->symbol_add(&rsp->impstate->arg3, sizeof(rsp->impstate->arg3), "arg3");
	rsp->impstate->drcuml->symbol_add(&rsp->impstate->numcycles, sizeof(rsp->impstate->numcycles), "numcycles");

	/* initialize the front-end helper */
	rsp->impstate->drcfe = auto_alloc(device->machine(), rsp_frontend(*rsp, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE));

	/* compute the register parameters */
	for (regnum = 0; regnum < 32; regnum++)
		rsp->impstate->regmap[regnum] = (regnum == 0) ? parameter(0) : parameter::make_memory(&rsp->r[regnum]);

	/*
    drcbe_info beinfo;
    rsp->impstate->drcuml->get_backend_info(beinfo);
    if (beinfo.direct_iregs > 2)
    {
        rsp->impstate->regmap[30] = I2;
    }
    if (beinfo.direct_iregs > 3)
    {
        rsp->impstate->regmap[31] = I3;
    }
    if (beinfo.direct_iregs > 4)
    {
        rsp->impstate->regmap[2] = I4;
    }
    if (beinfo.direct_iregs > 5)
    {
        rsp->impstate->regmap[3] = I5;
    }
    if (beinfo.direct_iregs > 6)
    {
        rsp->impstate->regmap[4] = I6;
    }
    */

	/* mark the cache dirty so it is updated on next execute */
	rsp->impstate->cache_dirty = TRUE;
}

static CPU_EXIT( rsp )
{
	rsp_state *rsp = get_safe_token(device);

	/* clean up the DRC */
	auto_free(device->machine(), rsp->impstate->drcfe);
	auto_free(device->machine(), rsp->impstate->drcuml);
	auto_free(device->machine(), rsp->impstate->cache);
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
	VREG_B(dest, index) = READ8(rsp, ea);
}

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

	int end = index + 2;

	for (int i = index; i < end; i++)
	{
		VREG_B(dest, i) = READ8(rsp, ea);
		ea++;
	}
}

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

	int end = index + 4;

	for (int i = index; i < end; i++)
	{
		VREG_B(dest, i) = READ8(rsp, ea);
		ea++;
	}
}

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

	int end = index + 8;

	for (int i = index; i < end; i++)
	{
		VREG_B(dest, i) = READ8(rsp, ea);
		ea++;
	}
}

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
		VREG_B(dest, i) = READ8(rsp, ea);
		ea++;
	}
}

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
		VREG_B(dest, i) = READ8(rsp, ea);
		ea++;
	}
}

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
		W_VREG_S(dest, i) = READ8(rsp, ea + (((16-index) + i) & 0xf)) << 8;
	}
}

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
		W_VREG_S(dest, i) = READ8(rsp, ea + (((16-index) + i) & 0xf)) << 7;
	}
}

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
		W_VREG_S(dest, i) = READ8(rsp, ea + (((16-index) + (i<<1)) & 0xf)) << 7;
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


	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	// not sure what happens if 16-byte boundary is crossed...

	end = (index >> 1) + 4;

	for (i=index >> 1; i < end; i++)
	{
		W_VREG_S(dest, i) = READ8(rsp, ea) << 7;
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

	end = (16 - index) + 16;

	for (i=(16 - index); i < end; i++)
	{
		VREG_B(dest, i & 0xf) = READ8(rsp, ea);
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
	{
		ve = 32;
	}

	element = 7 - (index >> 1);

	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	ea = ((ea + 8) & ~0xf) + (index & 1);
	for (i = vs; i < ve; i++)
	{
		element = ((8 - (index >> 1) + (i - vs)) << 1);
		VREG_B(i, (element & 0xf)) = READ8(rsp, ea);
		VREG_B(i, ((element + 1) & 0xf)) = READ8(rsp, ea + 1);

		ea += 2;
	}
}

static int generate_lwc2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	//int loopdest;
	UINT32 op = desc->opptr.l[0];
	//int dest = (op >> 16) & 0x1f;
	//int base = (op >> 21) & 0x1f;
	//int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	//int skip;
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:		/* LBV */
			//UML_ADD(block, I0, R32(RSREG), offset);
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lbv, rsp);
			return TRUE;
		case 0x01:		/* LSV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lsv, rsp);
			return TRUE;
		case 0x02:		/* LLV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_llv, rsp);
			return TRUE;
		case 0x03:		/* LDV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ldv, rsp);
			return TRUE;
		case 0x04:		/* LQV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lqv, rsp);
			return TRUE;
		case 0x05:		/* LRV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lrv, rsp);
			return TRUE;
		case 0x06:		/* LPV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lpv, rsp);
			return TRUE;
		case 0x07:		/* LUV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_luv, rsp);
			return TRUE;
		case 0x08:		/* LHV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lhv, rsp);
			return TRUE;
		case 0x09:		/* LFV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lfv, rsp);
			return TRUE;
		case 0x0a:		/* LWV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_lwv, rsp);
			return TRUE;
		case 0x0b:		/* LTV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
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
	WRITE8(rsp, ea, VREG_B(dest, index));
}

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

	int end = index + 2;

	for (int i = index; i < end; i++)
	{
		WRITE8(rsp, ea, VREG_B(dest, i));
		ea++;
	}
}

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

	int end = index + 4;

	for (int i = index; i < end; i++)
	{
		WRITE8(rsp, ea, VREG_B(dest, i));
		ea++;
	}
}

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

	for (int i = index; i < end; i++)
	{
		WRITE8(rsp, ea, VREG_B(dest, i));
		ea++;
	}
}

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
		WRITE8(rsp, ea, VREG_B(dest, i & 0xf));
		ea++;
	}
}

static void cfunc_rsp_srv(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	UINT32 op = rsp->impstate->arg0;
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

	UINT32 ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	int end = index + (ea & 0xf);
	int o = (16 - (ea & 0xf)) & 0xf;
	ea &= ~0xf;

	for (int i = index; i < end; i++)
	{
		WRITE8(rsp, ea, VREG_B(dest, ((i + o) & 0xf)));
		ea++;
	}
}

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
			WRITE8(rsp, ea, VREG_B(dest, ((i & 0xf) << 1)));
		}
		else
		{
			WRITE8(rsp, ea, VREG_S(dest, (i & 0x7)) >> 7);
		}
		ea++;
	}
}

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
			WRITE8(rsp, ea, VREG_S(dest, (i & 0x7)) >> 7);
		}
		else
		{
			WRITE8(rsp, ea, VREG_B(dest, ((i & 0x7) << 1)));
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
		UINT8 d = ((VREG_B(dest, ((index + (i << 1) + 0) & 0xf))) << 1) |
				  ((VREG_B(dest, ((index + (i << 1) + 1) & 0xf))) >> 7);

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

	if (index & 0x7)	printf("RSP: SFV: index = %d at %08X\n", index, rsp->ppc);

	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	eaoffset = ea & 0xf;
	ea &= ~0xf;

	end = (index >> 1) + 4;

	for (i=index >> 1; i < end; i++)
	{
		WRITE8(rsp, ea + (eaoffset & 0xf), VREG_S(dest, i) >> 7);
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
		WRITE8(rsp, ea + (eaoffset & 0xf), VREG_B(dest, i & 0xf));
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

	int vs = dest;
	int ve = dest + 8;
	if (ve > 32)
	{
		ve = 32;
	}

	int element = 8 - (index >> 1);

	ea = (base) ? rsp->r[base] + (offset * 16) : (offset * 16);

	int eaoffset = (ea & 0xf) + (element * 2);
	ea &= ~0xf;

	for (i=vs; i < ve; i++)
	{
		WRITE16(rsp, ea + (eaoffset & 0xf), VREG_S(i, element & 0x7));
		eaoffset += 2;
		element++;
	}
}

static int generate_swc2(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
//  int loopdest;
	UINT32 op = desc->opptr.l[0];
	//int dest = (op >> 16) & 0x1f;
	//int base = (op >> 21) & 0x1f;
	//int index = (op >> 7) & 0xf;
	int offset = (op & 0x7f);
	//int skip;
	if (offset & 0x40)
	{
		offset |= 0xffffffc0;
	}

	switch ((op >> 11) & 0x1f)
	{
		case 0x00:		/* SBV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sbv, rsp);
			return TRUE;
		case 0x01:		/* SSV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_ssv, rsp);
			return TRUE;
		case 0x02:		/* SLV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_slv, rsp);
			return TRUE;
		case 0x03:		/* SDV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sdv, rsp);
			return TRUE;
		case 0x04:		/* SQV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sqv, rsp);
			return TRUE;
		case 0x05:		/* SRV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_srv, rsp);
			return TRUE;
		case 0x06:		/* SPV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_spv, rsp);
			return TRUE;
		case 0x07:		/* SUV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_suv, rsp);
			return TRUE;
		case 0x08:		/* SHV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_shv, rsp);
			return TRUE;
		case 0x09:		/* SFV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_sfv, rsp);
			return TRUE;
		case 0x0a:		/* SWV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_swv, rsp);
			return TRUE;
		case 0x0b:		/* STV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_stv, rsp);
			return TRUE;

		default:
			unimplemented_opcode(rsp, op);
			return FALSE;
	}

	return TRUE;
}

INLINE UINT16 SATURATE_ACCUM(rsp_state *rsp, int accum, int slice, UINT16 negative, UINT16 positive)
{
	if ((INT16)ACCUM_H(accum) < 0)
	{
		if ((UINT16)(ACCUM_H(accum)) != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) >= 0)
			{
				return negative;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else if (slice == 1)
				{
					return ACCUM_M(accum);
				}
			}
		}
	}
	else
	{
		if ((UINT16)(ACCUM_H(accum)) != 0)
		{
			return positive;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) < 0)
			{
				return positive;
			}
			else
			{
				if (slice == 0)
				{
					return ACCUM_L(accum);
				}
				else
				{
					return ACCUM_M(accum);
				}
			}
		}
	}

	return 0;
}

INLINE UINT16 SATURATE_ACCUM1(rsp_state *rsp, int accum, UINT16 negative, UINT16 positive)
{
	if ((INT16)ACCUM_H(accum) < 0)
	{
		if ((UINT16)(ACCUM_H(accum)) != 0xffff)
		{
			return negative;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) >= 0)
			{
				return negative;
			}
			else
			{
        		return ACCUM_M(accum);
			}
		}
	}
	else
	{
		if ((UINT16)(ACCUM_H(accum)) != 0)
		{
			return positive;
		}
		else
		{
			if ((INT16)ACCUM_M(accum) < 0)
			{
				return positive;
			}
			else
			{
        		return ACCUM_M(accum);
			}
		}
	}

	return 0;
}

#define WRITEBACK_RESULT() { \
		W_VREG_S(VDREG, 0) = vres[0];	\
		W_VREG_S(VDREG, 1) = vres[1];	\
		W_VREG_S(VDREG, 2) = vres[2];	\
		W_VREG_S(VDREG, 3) = vres[3];	\
		W_VREG_S(VDREG, 4) = vres[4];	\
		W_VREG_S(VDREG, 5) = vres[5];	\
		W_VREG_S(VDREG, 6) = vres[6];	\
		W_VREG_S(VDREG, 7) = vres[7];	\
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

	int sel;
	INT32 s1, s2;
	INT64 r;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
		if (s1 == -32768 && s2 == -32768)
		{
			// overflow
			ACCUM_H(i) = 0;
			ACCUM_M(i) = -32768;
			ACCUM_L(i) = -32768;
			vres[i] = 0x7fff;
		}
		else
		{
			r =  s1 * s2 * 2;
			r += 0x8000;	// rounding ?
			ACCUM_H(i) = (r < 0) ? 0xffff : 0;		// sign-extend to 48-bit
			ACCUM_M(i) = (INT16)(r >> 16);
			ACCUM_L(i) = (UINT16)(r);
			vres[i] = ACCUM_M(i);
		}
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

	int sel;
	INT32 s1, s2;
	INT64 r;
	for (i=0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
		r = s1 * s2 * 2;
		r += 0x8000;	// rounding ?

		ACCUM_H(i) = (UINT16)(r >> 32);
		ACCUM_M(i) = (UINT16)(r >> 16);
		ACCUM_L(i) = (UINT16)(r);

		if (r < 0)
		{
			vres[i] = 0;
		}
		else if (((INT16)(ACCUM_H(i)) ^ (INT16)(ACCUM_M(i))) < 0)
		{
			vres[i] = -1;
		}
		else
		{
			vres[i] = ACCUM_M(i);
		}
	}
	WRITEBACK_RESULT();
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

	int sel;
	UINT32 s1, s2;
	UINT32 r;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
		s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
		r = s1 * s2;

		ACCUM_H(i) = 0;
		ACCUM_M(i) = 0;
		ACCUM_L(i) = (UINT16)(r >> 16);

		vres[i] = ACCUM_L(i);
	}
	WRITEBACK_RESULT();
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

	int sel;
	INT32 s1, s2;
	INT32 r;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (UINT16)VREG_S(VS2REG, sel);	// not sign-extended
		r =  s1 * s2;

		ACCUM_H(i) = (r < 0) ? 0xffff : 0;		// sign-extend to 48-bit
		ACCUM_M(i) = (INT16)(r >> 16);
		ACCUM_L(i) = (UINT16)(r);

		vres[i] = ACCUM_M(i);
	}
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vmudn(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8] = { 0 };

	// 31       25  24     20      15      10      5        0
	// ------------------------------------------------------
	// | 010010 | 1 | EEEE | SSSSS | TTTTT | DDDDD | 000110 |
	// ------------------------------------------------------
	//
	// Multiplies unsigned fraction by signed integer
	// The result is stored into accumulator
	// The low slice of accumulator is stored into destination element

	int sel;
	INT32 s1, s2;
	INT32 r;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (UINT16)VREG_S(VS1REG, i);		// not sign-extended
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
		r = s1 * s2;

		ACCUM_H(i) = (r < 0) ? 0xffff : 0;		// sign-extend to 48-bit
		ACCUM_M(i) = (INT16)(r >> 16);
		ACCUM_L(i) = (UINT16)(r);

		vres[i] = ACCUM_L(i);
	}
	WRITEBACK_RESULT();
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

	int sel;
	INT32 s1, s2;
	INT32 r;
	for (i=0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
		r = s1 * s2;

		ACCUM_H(i) = (INT16)(r >> 16);
		ACCUM_M(i) = (UINT16)(r);
		ACCUM_L(i) = 0;

		if (r < -32768) r = -32768;
		if (r >  32767)	r = 32767;
		vres[i] = (INT16)(r);
	}
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vmacf(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];

	int sel;
	INT32 s1, s2;
	INT32 r;
	UINT16 res;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
		r = s1 * s2;

		ACCUM(i) += (INT64)(r) << 17;
		res = SATURATE_ACCUM(rsp, i, 1, 0x8000, 0x7fff);

		vres[i] = res;
	}
	WRITEBACK_RESULT();
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

	UINT16 res;
	int sel;
	INT32 s1, s2, r1;
	UINT32 r2, r3;
	for (i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
		r1 = s1 * s2;
		r2 = (UINT16)ACCUM_L(i) + ((UINT16)(r1) * 2);
		r3 = (UINT16)ACCUM_M(i) + (UINT16)((r1 >> 16) * 2) + (UINT16)(r2 >> 16);

		ACCUM_L(i) = (UINT16)(r2);
		ACCUM_M(i) = (UINT16)(r3);
		ACCUM_H(i) += (UINT16)(r3 >> 16) + (UINT16)(r1 >> 31);

		//res = SATURATE_ACCUM(i, 1, 0x0000, 0xffff);
		if ((INT16)ACCUM_H(i) < 0)
		{
			res = 0;
		}
		else
		{
			if (ACCUM_H(i) != 0)
			{
				res = 0xffff;
			}
			else
			{
				if ((INT16)ACCUM_M(i) < 0)
				{
					res = 0xffff;
				}
				else
				{
					res = ACCUM_M(i);
				}
			}
		}

		vres[i] = res;
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

	UINT16 res;
	int sel;
	UINT32 s1, s2, r1;
	UINT32 r2, r3;
	for (i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
		s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
		r1 = s1 * s2;
		r2 = (UINT16)ACCUM_L(i) + (r1 >> 16);
		r3 = (UINT16)ACCUM_M(i) + (r2 >> 16);

		ACCUM_L(i) = (UINT16)(r2);
		ACCUM_M(i) = (UINT16)(r3);
		ACCUM_H(i) += (INT16)(r3 >> 16);

		res = SATURATE_ACCUM(rsp, i, 0, 0x0000, 0xffff);

		vres[i] = res;
	}
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vmadm(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];

	UINT16 res;
	int sel;
	UINT32 s1, s2, r1;
	UINT32 r2, r3;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (UINT16)VREG_S(VS2REG, sel);	// not sign-extended
		r1 = s1 * s2;
		r2 = (UINT16)ACCUM_L(i) + (UINT16)(r1);
		r3 = (UINT16)ACCUM_M(i) + (r1 >> 16) + (r2 >> 16);

		ACCUM_L(i) = (UINT16)(r2);
		ACCUM_M(i) = (UINT16)(r3);
		ACCUM_H(i) += (UINT16)(r3 >> 16);
		if ((INT32)(r1) < 0)
			ACCUM_H(i) -= 1;

		res = SATURATE_ACCUM(rsp, i, 1, 0x8000, 0x7fff);

		vres[i] = res;
	}
	WRITEBACK_RESULT();
}

INLINE void cfunc_rsp_vmadn(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	int op = rsp->impstate->arg0;
	INT16 vres[8];

	INT32 s1, s2;
	UINT16 res;
	int sel;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (UINT16)VREG_S(VS1REG, i);		// not sign-extended
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);

		ACCUM(i) += (INT64)(s1*s2)<<16;

		res = SATURATE_ACCUM(rsp, i, 0, 0x0000, 0xffff);
		vres[i] = res;
	}
	WRITEBACK_RESULT();
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

	UINT16 res;
	int sel;
	INT32 s1, s2;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);

		rsp->accum[i].l[1] += s1*s2;

		res = SATURATE_ACCUM1(rsp, i, 0x8000, 0x7fff);

		vres[i] = res;
	}
	WRITEBACK_RESULT();
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

	int sel;
	INT32 s1, s2, r;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
		r = s1 + s2 + CARRY_FLAG(i);

		ACCUM_L(i) = (INT16)(r);

		if (r > 32767) r = 32767;
		if (r < -32768) r = -32768;
		vres[i] = (INT16)(r);
	}
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
	WRITEBACK_RESULT();
}

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

	int sel;
	INT32 s1, s2, r;
	for (i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT32)(INT16)VREG_S(VS1REG, i);
		s2 = (INT32)(INT16)VREG_S(VS2REG, sel);
		r = s1 - s2 - CARRY_FLAG(i);

		ACCUM_L(i) = (INT16)(r);

		if (r > 32767) r = 32767;
		if (r < -32768) r = -32768;

		vres[i] = (INT16)(r);
	}
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();
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

	int sel;
	INT16 s1, s2;
	for (i=0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (INT16)VREG_S(VS1REG, i);
		s2 = (INT16)VREG_S(VS2REG, sel);

		if (s1 < 0)
		{
			if (s2 == -32768)
			{
				vres[i] = 32767;
			}
			else
			{
				vres[i] = -s2;
			}
		}
		else if (s1 > 0)
		{
			vres[i] = s2;
		}
		else
		{
			vres[i] = 0;
		}

		ACCUM_L(i) = vres[i];
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

	int sel;
	INT32 s1, s2, r;
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();

	for (i=0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
		s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
		r = s1 + s2;

		vres[i] = (INT16)(r);
		ACCUM_L(i) = (INT16)(r);

		if (r & 0xffff0000)
		{
			SET_CARRY_FLAG(i);
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

	int sel;
	INT32 s1, s2, r;
	CLEAR_ZERO_FLAGS();
	CLEAR_CARRY_FLAGS();

	for (i=0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = (UINT32)(UINT16)VREG_S(VS1REG, i);
		s2 = (UINT32)(UINT16)VREG_S(VS2REG, sel);
		r = s1 - s2;

		vres[i] = (INT16)(r);
		ACCUM_L(i) = (UINT16)(r);

		if ((UINT16)(r) != 0)
		{
			SET_ZERO_FLAG(i);
		}
		if (r & 0xffff0000)
		{
			SET_CARRY_FLAG(i);
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
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(VDREG, i) = ACCUM_H(i);
			}
			break;
		}
		case 0x09:		// VSAWM
		{
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(VDREG, i) = ACCUM_M(i);
			}
			break;
		}
		case 0x0a:		// VSAWL
		{
			for (int i = 0; i < 8; i++)
			{
				W_VREG_S(VDREG, i) = ACCUM_L(i);
			}
			break;
		}
		default:	fatalerror("RSP: VSAW: el = %d\n", EL);
	}
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

	int sel;
	rsp->flag[1] = 0;

	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);

		if (VREG_S(VS1REG, i) < VREG_S(VS2REG, sel))
		{
			SET_COMPARE_FLAG(i);
		}
		else if (VREG_S(VS1REG, i) == VREG_S(VS2REG, sel))
		{
			if (ZERO_FLAG(i) == 1 && CARRY_FLAG(i) != 0)
			{
				SET_COMPARE_FLAG(i);
			}
		}

		if (COMPARE_FLAG(i))
		{
			vres[i] = VREG_S(VS1REG, i);
		}
		else
		{
			vres[i] = VREG_S(VS2REG, sel);
		}

		ACCUM_L(i) = vres[i];
	}

	rsp->flag[0] = 0;
	WRITEBACK_RESULT();
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

	int sel;
	rsp->flag[1] = 0;

	for (i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);

		if ((VREG_S(VS1REG, i) == VREG_S(VS2REG, sel)) && ZERO_FLAG(i) == 0)
		{
			SET_COMPARE_FLAG(i);
			vres[i] = VREG_S(VS1REG, i);
		}
		else
		{
			vres[i] = VREG_S(VS2REG, sel);
		}
		ACCUM_L(i) = vres[i];
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

	int sel;
	rsp->flag[1] = 0;

	for (i=0; i < 8; i++)//?????????? ????
	{
		sel = VEC_EL_2(EL, i);

		if (VREG_S(VS1REG, i) != VREG_S(VS2REG, sel))
		{
			SET_COMPARE_FLAG(i);
		}
		else
		{
			if (ZERO_FLAG(i) == 1)
			{
				SET_COMPARE_FLAG(i);
			}
		}
		if (COMPARE_FLAG(i))
		{
			vres[i] = VREG_S(VS1REG, i);
		}
		else
		{
			vres[i] = VREG_S(VS2REG, sel);
		}
		ACCUM_L(i) = vres[i];
	}

	rsp->flag[0] = 0;
	WRITEBACK_RESULT();
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

	int sel;
	rsp->flag[1] = 0;

	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);

		if (VREG_S(VS1REG, i) == VREG_S(VS2REG, sel))
		{
			if (ZERO_FLAG(i) == 0 || CARRY_FLAG(i) == 0)
			{
				SET_COMPARE_FLAG(i);
			}
		}
		else if (VREG_S(VS1REG, i) > VREG_S(VS2REG, sel))
		{
			SET_COMPARE_FLAG(i);
		}

		if (COMPARE_FLAG(i) != 0)
		{
			vres[i] = VREG_S(VS1REG, i);
		}
		else
		{
			vres[i] = VREG_S(VS2REG, sel);
		}

		ACCUM_L(i) = vres[i];
	}

	rsp->flag[0] = 0;
	WRITEBACK_RESULT();
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

	int sel;
	INT16 s1, s2;
	for (i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = VREG_S(VS1REG, i);
		s2 = VREG_S(VS2REG, sel);

		if (CARRY_FLAG(i) != 0)
		{

			if (ZERO_FLAG(i) != 0)
			{

				if (COMPARE_FLAG(i) != 0)
				{
					ACCUM_L(i) = -(UINT16)s2;
				}
				else
				{
					ACCUM_L(i) = s1;
				}
			}
			else//ZERO_FLAG(i)==0
			{

				if (rsp->flag[2] & (1 << (i)))
				{

					if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) > 0x10000)
					{//proper fix for Harvest Moon 64, r4

						ACCUM_L(i) = s1;
						CLEAR_COMPARE_FLAG(i);
					}
					else
					{

						ACCUM_L(i) = -((UINT16)s2);
						SET_COMPARE_FLAG(i);
					}
				}
				else
				{
					if (((UINT32)(UINT16)(s1) + (UINT32)(UINT16)(s2)) != 0)
					{
						ACCUM_L(i) = s1;
						CLEAR_COMPARE_FLAG(i);
					}
					else
					{
						ACCUM_L(i) = -((UINT16)s2);
						SET_COMPARE_FLAG(i);
					}
				}
			}
		}//
		else//CARRY_FLAG(i)==0
		{

			if (ZERO_FLAG(i) != 0)
			{

				if (rsp->flag[1] & (1 << (8+i)))
				{
					ACCUM_L(i) = s2;
				}
				else
				{
					ACCUM_L(i) = s1;
				}
			}
			else
			{
				if (((INT32)(UINT16)s1 - (INT32)(UINT16)s2) >= 0)
				{
					ACCUM_L(i) = s2;
					rsp->flag[1] |= (1 << (8+i));
				}
				else
				{
					ACCUM_L(i) = s1;
					rsp->flag[1] &= ~(1 << (8+i));
				}
			}
		}

		vres[i] = ACCUM_L(i);
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

	int sel;
	INT16 s1, s2;
	rsp->flag[0] = 0;
	rsp->flag[1] = 0;
	rsp->flag[2] = 0;
	UINT32 vce = 0;

	for (i=0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = VREG_S(VS1REG, i);
		s2 = VREG_S(VS2REG, sel);

		if ((s1 ^ s2) < 0)
		{
			vce = (s1 + s2 == -1);
			SET_CARRY_FLAG(i);
			if (s2 < 0)
			{
				rsp->flag[1] |= (1 << (8+i));
			}

			if (s1 + s2 <= 0)
			{
				SET_COMPARE_FLAG(i);
				vres[i] = -((UINT16)s2);
			}
			else
			{
				vres[i] = s1;
			}

			if (s1 + s2 != 0)
			{
				if (s1 != ~s2)
				{
					SET_ZERO_FLAG(i);
				}
			}
		}//sign
		else
		{
			vce = 0;
			if (s2 < 0)
			{
				SET_COMPARE_FLAG(i);
			}
			if (s1 - s2 >= 0)
			{
				rsp->flag[1] |= (1 << (8+i));
				vres[i] = s2;
			}
			else
			{
				vres[i] = s1;
			}

			if ((s1 - s2) != 0)
			{
				if (s1 != ~s2)
				{
					SET_ZERO_FLAG(i);
				}
			}
		}
		rsp->flag[2] |= (vce << (i));
		ACCUM_L(i) = vres[i];
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

	int sel;
	INT16 s1, s2;
	rsp->flag[0] = 0;
	rsp->flag[1] = 0;
	rsp->flag[2] = 0;

	for (i=0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		s1 = VREG_S(VS1REG, i);
		s2 = VREG_S(VS2REG, sel);

		if ((INT16)(s1 ^ s2) < 0)
		{
			if (s2 < 0)
			{
				rsp->flag[1] |= (1 << (8+i));
			}
			if ((s1 + s2) <= 0)
			{
				ACCUM_L(i) = ~((UINT16)s2);
				SET_COMPARE_FLAG(i);
			}
			else
			{
				ACCUM_L(i) = s1;
			}
		}
		else
		{
			if (s2 < 0)
			{
				SET_COMPARE_FLAG(i);
			}
			if ((s1 - s2) >= 0)
			{
				ACCUM_L(i) = s2;
				rsp->flag[1] |= (1 << (8+i));
			}
			else
			{
				ACCUM_L(i) = s1;
			}
		}

		vres[i] = ACCUM_L(i);
	}
	WRITEBACK_RESULT();
}

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

	int sel;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		if (COMPARE_FLAG(i) != 0)
		{
			vres[i] = VREG_S(VS1REG, i);
		}
		else
		{
			vres[i] = VREG_S(VS2REG, sel);//??? ???????????
		}

		ACCUM_L(i) = vres[i];
	}
	WRITEBACK_RESULT();
}

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

	int sel;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		vres[i] = VREG_S(VS1REG, i) & VREG_S(VS2REG, sel);
		ACCUM_L(i) = vres[i];
	}
	WRITEBACK_RESULT();
}

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

	int sel;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		vres[i] = ~((VREG_S(VS1REG, i) & VREG_S(VS2REG, sel)));
		ACCUM_L(i) = vres[i];
	}
	WRITEBACK_RESULT();
}

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

	int sel;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		vres[i] = VREG_S(VS1REG, i) | VREG_S(VS2REG, sel);
		ACCUM_L(i) = vres[i];
	}
	WRITEBACK_RESULT();
}

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

	int sel;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		vres[i] = ~((VREG_S(VS1REG, i) | VREG_S(VS2REG, sel)));
		ACCUM_L(i) = vres[i];
	}
	WRITEBACK_RESULT();
}

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

	int sel;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		vres[i] = VREG_S(VS1REG, i) ^ VREG_S(VS2REG, sel);
		ACCUM_L(i) = vres[i];
	}
	WRITEBACK_RESULT();
}

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

	int sel;
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		vres[i] = ~((VREG_S(VS1REG, i) ^ VREG_S(VS2REG, sel)));
		ACCUM_L(i) = vres[i];
	}
	WRITEBACK_RESULT();
}

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

	int del = VS1REG & 7;
	int sel = EL & 7;
	INT32 shifter = 0;

	INT32 rec = (INT16)(VREG_S(VS2REG, sel));
	INT32 datainput = (rec < 0) ? (-rec) : rec;
	if (datainput)
	{
		for (i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
			{
				shifter = i;
				break;
			}
		}
	}
	else
	{
		shifter = 0x10;
	}

	INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
	INT32 fetchval = rsp_divtable[address];
	INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
	if (rec < 0)
	{
		temp = ~temp;
	}
	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	rsp->reciprocal_res = rec;
	rsp->dp_allowed = 0;

	W_VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

	for (i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		ACCUM_L(i) = VREG_S(VS2REG, sel);
	}
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

	int del = VS1REG & 7;
	int sel = EL & 7;
	INT32 shifter = 0;

	INT32 rec = ((UINT16)(VREG_S(VS2REG, sel)) | ((UINT32)(rsp->reciprocal_high) & 0xffff0000));

	INT32 datainput = rec;

	if (rec < 0)
	{
		if (rsp->dp_allowed)
		{
			if (rec < -32768)
			{
				datainput = ~datainput;
			}
			else
			{
				datainput = -datainput;
			}
		}
		else
		{
			datainput = -datainput;
		}
	}


	if (datainput)
	{
		for (i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))//?.?.??? 31 - i
			{
				shifter = i;
				break;
			}
		}
	}
	else
	{
		if (rsp->dp_allowed)
		{
			shifter = 0;
		}
		else
		{
			shifter = 0x10;
		}
	}

	INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
	INT32 fetchval = rsp_divtable[address];
	INT32 temp = (0x40000000 | (fetchval << 14)) >> ((~shifter) & 0x1f);
	if (rec < 0)
	{
		temp = ~temp;
	}
	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	rsp->reciprocal_res = rec;
	rsp->dp_allowed = 0;

	W_VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

	for (i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		ACCUM_L(i) = VREG_S(VS2REG, sel);
	}
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

	int del = VS1REG & 7;
	int sel = EL & 7;

	rsp->reciprocal_high = (VREG_S(VS2REG, sel)) << 16;
	rsp->dp_allowed = 1;

	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		ACCUM_L(i) = VREG_S(VS2REG, sel);
	}

	W_VREG_S(VDREG, del) = (INT16)(rsp->reciprocal_res >> 16);
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

	int del = VS1REG & 7;
	int sel = EL & 7;

	W_VREG_S(VDREG, del) = VREG_S(VS2REG, sel);
	for (int i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		ACCUM_L(i) = VREG_S(VS2REG, sel);
	}
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

	int del = VS1REG & 7;
	int sel = EL & 7;
	INT32 shifter = 0;

	INT32 rec = ((UINT16)(VREG_S(VS2REG, sel)) | ((UINT32)(rsp->reciprocal_high) & 0xffff0000));

	INT32 datainput = rec;

	if (rec < 0)
	{
		if (rsp->dp_allowed)
		{
			if (rec < -32768)//VDIV.C,208
			{
				datainput = ~datainput;
			}
			else
			{
				datainput = -datainput;
			}
		}
		else
		{
			datainput = -datainput;
		}
	}

	if (datainput)
	{
		for (i = 0; i < 32; i++)
		{
			if (datainput & (1 << ((~i) & 0x1f)))
			{
				shifter = i;
				break;
			}
		}
	}
	else
	{
		if (rsp->dp_allowed)
		{
			shifter = 0;
		}
		else
		{
			shifter = 0x10;
		}
	}

	INT32 address = ((datainput << shifter) & 0x7fc00000) >> 22;
	address = ((address | 0x200) & 0x3fe) | (shifter & 1);

	INT32 fetchval = rsp_divtable[address];
	INT32 temp = (0x40000000 | (fetchval << 14)) >> (((~shifter) & 0x1f) >> 1);
	if (rec < 0)
	{
		temp = ~temp;
	}
	if (!rec)
	{
		temp = 0x7fffffff;
	}
	else if (rec == 0xffff8000)
	{
		temp = 0xffff0000;
	}
	rec = temp;

	rsp->reciprocal_res = rec;
	rsp->dp_allowed = 0;

	W_VREG_S(VDREG, del) = (UINT16)(rec & 0xffff);

	for (i = 0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		ACCUM_L(i) = VREG_S(VS2REG, sel);
	}
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

	int del = VS1REG & 7;
	int sel = EL & 7;

	rsp->reciprocal_high = (VREG_S(VS2REG, sel)) << 16;
	rsp->dp_allowed = 1;

	for (i=0; i < 8; i++)
	{
		sel = VEC_EL_2(EL, i);
		ACCUM_L(i) = VREG_S(VS2REG, sel);
	}

	W_VREG_S(VDREG, del) = (INT16)(rsp->reciprocal_res >> 16);	// store high part
}

static void cfunc_sp_set_status_cb(void *param)
{
	rsp_state *rsp = (rsp_state*)param;
	(rsp->sp_set_status_func)(0, rsp->impstate->arg0);
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
	do
	{
		if( rsp->sr & ( RSP_STATUS_HALT | RSP_STATUS_BROKE ) )
		{
			rsp->icount = MIN(rsp->icount, 0);
			break;
		}

		/* run as much as we can */
		execute_result = drcuml->execute(*rsp->impstate->entry);

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
}

/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    rspdrc_flush_drc_cache - outward-facing
    accessor to code_flush_cache
-------------------------------------------------*/

void rspdrc_flush_drc_cache(device_t *device)
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
	rsp->impstate->drcuml->reset();

	try
	{
		/* generate the entry point and out-of-cycles handlers */
		static_generate_entry_point(rsp);
		static_generate_nocode_handler(rsp);
		static_generate_out_of_cycles(rsp);

		/* add subroutines for memory accesses */
		static_generate_memory_accessor(rsp, 1, FALSE, "read8",       rsp->impstate->read8);
		static_generate_memory_accessor(rsp, 1, TRUE,  "write8",      rsp->impstate->write8);
		static_generate_memory_accessor(rsp, 2, FALSE, "read16",      rsp->impstate->read16);
		static_generate_memory_accessor(rsp, 2, TRUE,  "write16",     rsp->impstate->write16);
		static_generate_memory_accessor(rsp, 4, FALSE, "read32",      rsp->impstate->read32);
		static_generate_memory_accessor(rsp, 4, TRUE,  "write32",     rsp->impstate->write32);
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Unable to generate static RSP code\n");
	}
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

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	desclist = rsp->impstate->drcfe->describe_code(pc);

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			block = drcuml->begin_block(4096);

			/* loop until we get through all instruction sequences */
			for (seqhead = desclist; seqhead != NULL; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				UINT32 nextpc;

				/* add a code log entry */
				if (LOG_UML)
					block->append_comment("-------------------------");					// comment

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != NULL; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != NULL);

				/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
				if (override || !drcuml->hash_exists(0, seqhead->pc))
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
					UML_HASHJMP(block, 0, seqhead->pc, *rsp->impstate->nocode);
																							// hashjmp <0>,seqhead->pc,nocode
					continue;
				}

				/* validate this code block if we're not pointing into ROM */
				if (rsp->program->get_write_ptr(seqhead->physpc) != NULL)
					generate_checksum_block(rsp, block, &compiler, seqhead, seqlast);

				/* label this instruction, if it may be jumped to locally */
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
					UML_LABEL(block, seqhead->pc | 0x80000000);								// label   seqhead->pc

				/* iterate over instructions in the sequence and compile them */
				for (curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
					generate_sequence_instruction(rsp, block, &compiler, curdesc);

				/* if we need to return to the start, do it */
				if (seqlast->flags & OPFLAG_RETURN_TO_START)
					nextpc = pc;

				/* otherwise we just go to the next instruction */
				else
					nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;

				/* count off cycles and go there */
				generate_update_cycles(rsp, block, &compiler, nextpc, TRUE);			// <subtract cycles>

				/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
				if (seqlast->next() == NULL || seqlast->next()->pc != nextpc)
					UML_HASHJMP(block, 0, nextpc, *rsp->impstate->nocode);			// hashjmp <mode>,nextpc,nocode
			}

			/* end the sequence */
			block->end();
			g_profiler.stop();
			succeeded = true;
		}
		catch (drcuml_block::abort_compilation &)
		{
			code_flush_cache(rsp);
		}
	}
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
	fatalerror("PC=%08X: Unimplemented op %08X (%02X,%02X)\n", rsp->pc, opcode, opcode >> 26, opcode & 0x3f);
}


/*-------------------------------------------------
    cfunc_fatalerror - a generic fatalerror call
-------------------------------------------------*/

#ifdef UNUSED_CODE
static void cfunc_fatalerror(void *param)
{
	fatalerror("fatalerror\n");
}
#endif


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

	/* begin generating */
	block = drcuml->begin_block(20);

	/* forward references */
	alloc_handle(drcuml, &rsp->impstate->nocode, "nocode");

	alloc_handle(drcuml, &rsp->impstate->entry, "entry");
	UML_HANDLE(block, *rsp->impstate->entry);										// handle  entry

	/* load fast integer registers */
	load_fast_iregs(rsp, block);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&rsp->pc), *rsp->impstate->nocode);					// hashjmp <mode>,<pc>,nocode
	block->end();
}


/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

static void static_generate_nocode_handler(rsp_state *rsp)
{
	drcuml_state *drcuml = rsp->impstate->drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &rsp->impstate->nocode, "nocode");
	UML_HANDLE(block, *rsp->impstate->nocode);										// handle  nocode
	UML_GETEXP(block, I0);														// getexp  i0
	UML_MOV(block, mem(&rsp->pc), I0);											// mov     [pc],i0
	save_fast_iregs(rsp, block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);										// exit    EXECUTE_MISSING_CODE

	block->end();
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

static void static_generate_out_of_cycles(rsp_state *rsp)
{
	drcuml_state *drcuml = rsp->impstate->drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &rsp->impstate->out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *rsp->impstate->out_of_cycles);								// handle  out_of_cycles
	UML_GETEXP(block, I0);														// getexp  i0
	UML_MOV(block, mem(&rsp->pc), I0);											// mov     <pc>,i0
	save_fast_iregs(rsp, block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);									// exit    EXECUTE_OUT_OF_CYCLES

	block->end();
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

static void static_generate_memory_accessor(rsp_state *rsp, int size, int iswrite, const char *name, code_handle *&handleptr)
{
	/* on entry, address is in I0; data for writes is in I1 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I1 */
	drcuml_state *drcuml = rsp->impstate->drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(1024);

	/* add a global entry for this */
	alloc_handle(drcuml, &handleptr, name);
	UML_HANDLE(block, *handleptr);													// handle  *handleptr

	// write:
	if (iswrite)
	{
		if (size == 1)
		{
			UML_MOV(block, mem(&rsp->impstate->arg0), I0);				// mov     [arg0],i0 ; address
			UML_MOV(block, mem(&rsp->impstate->arg1), I1);				// mov     [arg1],i1 ; data
			UML_CALLC(block, cfunc_write8, rsp);							// callc   cfunc_write8
		}
		else if (size == 2)
		{
			UML_MOV(block, mem(&rsp->impstate->arg0), I0);				// mov     [arg0],i0 ; address
			UML_MOV(block, mem(&rsp->impstate->arg1), I1);				// mov     [arg1],i1 ; data
			UML_CALLC(block, cfunc_write16, rsp);							// callc   cfunc_write16
		}
		else if (size == 4)
		{
			UML_MOV(block, mem(&rsp->impstate->arg0), I0);				// mov     [arg0],i0 ; address
			UML_MOV(block, mem(&rsp->impstate->arg1), I1);				// mov     [arg1],i1 ; data
			UML_CALLC(block, cfunc_write32, rsp);							// callc   cfunc_write32
		}
	}
	else
	{
		if (size == 1)
		{
			UML_MOV(block, mem(&rsp->impstate->arg0), I0);			// mov     [arg0],i0 ; address
			UML_CALLC(block, cfunc_read8, rsp);							// callc   cfunc_printf_debug
			UML_MOV(block, I0, mem(&rsp->impstate->arg0));			// mov     i0,[arg0],i0 ; result
		}
		else if (size == 2)
		{
			UML_MOV(block, mem(&rsp->impstate->arg0), I0);			// mov     [arg0],i0 ; address
			UML_CALLC(block, cfunc_read16, rsp);						// callc   cfunc_read16
			UML_MOV(block, I0, mem(&rsp->impstate->arg0));			// mov     i0,[arg0],i0 ; result
		}
		else if (size == 4)
		{
			UML_MOV(block, mem(&rsp->impstate->arg0), I0);			// mov     [arg0],i0 ; address
			UML_CALLC(block, cfunc_read32, rsp);						// callc   cfunc_read32
			UML_MOV(block, I0, mem(&rsp->impstate->arg0));			// mov     i0,[arg0],i0 ; result
		}
	}
	UML_RET(block);

	block->end();
}



/***************************************************************************
    CODE GENERATION
***************************************************************************/

/*-------------------------------------------------
    generate_update_cycles - generate code to
    subtract cycles from the icount and generate
    an exception if out
-------------------------------------------------*/
static void generate_update_cycles(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, parameter param, int allow_exception)
{
	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, mem(&rsp->icount), mem(&rsp->icount), MAPVAR_CYCLES);		// sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);										// mapvar  cycles,0
		UML_EXHc(block, COND_S, *rsp->impstate->out_of_cycles, param);
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
		block->append_comment("[Validation for %08X]", seqhead->pc | 0x1000);		// comment
	}
	/* loose verify or single instruction: just compare and fail */
	if (!(rsp->impstate->drcoptions & RSPDRC_STRICT_VERIFY) || seqhead->next() == NULL)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			UINT32 sum = seqhead->opptr.l[0];
			void *base = rsp->direct->read_decrypted_ptr(seqhead->physpc | 0x1000);
			UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);							// load    i0,base,0,dword

			if (seqhead->delay.first() != NULL && seqhead->physpc != seqhead->delay.first()->physpc)
			{
				base = rsp->direct->read_decrypted_ptr(seqhead->delay.first()->physpc | 0x1000);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);					// load    i1,base,dword
				UML_ADD(block, I0, I0, I1);						// add     i0,i0,i1

				sum += seqhead->delay.first()->opptr.l[0];
			}

			UML_CMP(block, I0, sum);									// cmp     i0,opptr[0]
			UML_EXHc(block, COND_NE, *rsp->impstate->nocode, epc(seqhead));		// exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
		UINT32 sum = 0;
		void *base = rsp->direct->read_decrypted_ptr(seqhead->physpc | 0x1000);
		UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);								// load    i0,base,0,dword
		sum += seqhead->opptr.l[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = rsp->direct->read_decrypted_ptr(curdesc->physpc | 0x1000);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);						// load    i1,base,dword
				UML_ADD(block, I0, I0, I1);							// add     i0,i0,i1
				sum += curdesc->opptr.l[0];

				if (curdesc->delay.first() != NULL && (curdesc == seqlast || (curdesc->next() != NULL && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
				{
					base = rsp->direct->read_decrypted_ptr(curdesc->delay.first()->physpc | 0x1000);
					UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);					// load    i1,base,dword
					UML_ADD(block, I0, I0, I1);						// add     i0,i0,i1

					sum += curdesc->delay.first()->opptr.l[0];
				}
			}
		UML_CMP(block, I0, sum);											// cmp     i0,sum
		UML_EXHc(block, COND_NE, *rsp->impstate->nocode, epc(seqhead));			// exne    nocode,seqhead->pc
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
	if ((rsp->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&rsp->pc), desc->pc);								// mov     [pc],desc->pc
		save_fast_iregs(rsp, block);
		UML_DEBUG(block, desc->pc);											// debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
#if 0
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, mem(&rsp->pc), desc->pc);							   // mov     [pc],desc->pc
		save_fast_iregs(rsp, block);
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);								// exit EXECUTE_UNMAPPED_CODE
	}
#endif

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	/*else*/ if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(rsp, block, compiler, desc))
		{
			UML_MOV(block, mem(&rsp->pc), desc->pc);							// mov     [pc],desc->pc
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
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
		UML_AND(block, mem(&rsp->impstate->jmpdest), R32(RSREG), 0x00000fff);
		UML_OR(block, mem(&rsp->impstate->jmpdest), mem(&rsp->impstate->jmpdest), 0x1000);
	}

	/* set the link if needed -- before the delay slot */
	if (linkreg != 0)
	{
		UML_MOV(block, R32(linkreg), (INT32)(desc->pc + 8));					// mov    <linkreg>,desc->pc + 8
	}

	/* compile the delay slot using temporary compiler state */
	assert(desc->delay.first() != NULL);
	generate_sequence_instruction(rsp, block, &compiler_temp, desc->delay.first());		// <next instruction>

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(rsp, block, &compiler_temp, desc->targetpc, TRUE);	// <subtract cycles>
		if (desc->flags & OPFLAG_INTRABLOCK_BRANCH)
			UML_JMP(block, desc->targetpc | 0x80000000);							// jmp     desc->targetpc
		else
			UML_HASHJMP(block, 0, desc->targetpc, *rsp->impstate->nocode);
																					// hashjmp <mode>,desc->targetpc,nocode
	}
	else
	{
		generate_update_cycles(rsp, block, &compiler_temp, mem(&rsp->impstate->jmpdest), TRUE);
																					// <subtract cycles>
		UML_HASHJMP(block, 0, mem(&rsp->impstate->jmpdest), *rsp->impstate->nocode);
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
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmulf, rsp);
			return TRUE;

		case 0x01:		/* VMULU */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmulu, rsp);
			return TRUE;

		case 0x04:		/* VMUDL */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudl, rsp);
			return TRUE;

		case 0x05:		/* VMUDM */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudm, rsp);
			return TRUE;

		case 0x06:		/* VMUDN */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudn, rsp);
			return TRUE;

		case 0x07:		/* VMUDH */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmudh, rsp);
			return TRUE;

		case 0x08:		/* VMACF */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmacf, rsp);
			return TRUE;

		case 0x09:		/* VMACU */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmacu, rsp);
			return TRUE;

		case 0x0c:		/* VMADL */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadl, rsp);
			return TRUE;

		case 0x0d:		/* VMADM */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadm, rsp);
			return TRUE;

		case 0x0e:		/* VMADN */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadn, rsp);
			return TRUE;

		case 0x0f:		/* VMADH */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmadh, rsp);
			return TRUE;

		case 0x10:		/* VADD */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vadd, rsp);
			return TRUE;

		case 0x11:		/* VSUB */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsub, rsp);
			return TRUE;

		case 0x13:		/* VABS */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vabs, rsp);
			return TRUE;

		case 0x14:		/* VADDC */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vaddc, rsp);
			return TRUE;

		case 0x15:		/* VSUBC */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsubc, rsp);
			return TRUE;

		case 0x1d:		/* VSAW */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vsaw, rsp);
			return TRUE;

		case 0x20:		/* VLT */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vlt, rsp);
			return TRUE;

		case 0x21:		/* VEQ */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_veq, rsp);
			return TRUE;

		case 0x22:		/* VNE */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vne, rsp);
			return TRUE;

		case 0x23:		/* VGE */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vge, rsp);
			return TRUE;

		case 0x24:		/* VCL */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vcl, rsp);
			return TRUE;

		case 0x25:		/* VCH */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vch, rsp);
			return TRUE;

		case 0x26:		/* VCR */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vcr, rsp);
			return TRUE;

		case 0x27:		/* VMRG */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmrg, rsp);
			return TRUE;

		case 0x28:		/* VAND */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vand, rsp);
			return TRUE;

		case 0x29:		/* VNAND */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnand, rsp);
			return TRUE;

		case 0x2a:		/* VOR */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vor, rsp);
			return TRUE;

		case 0x2b:		/* VNOR */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnor, rsp);
			return TRUE;

		case 0x2c:		/* VXOR */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vxor, rsp);
			return TRUE;

		case 0x2d:		/* VNXOR */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vnxor, rsp);
			return TRUE;

		case 0x30:		/* VRCP */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcp, rsp);
			return TRUE;

		case 0x31:		/* VRCPL */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcpl, rsp);
			return TRUE;

		case 0x32:		/* VRCPH */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrcph, rsp);
			return TRUE;

		case 0x33:		/* VMOV */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vmov, rsp);
			return TRUE;

		case 0x35:		/* VRSQL */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrsql, rsp);
			return TRUE;

		case 0x36:		/* VRSQH */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_rsp_vrsqh, rsp);
			return TRUE;

		default:
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_unimplemented_opcode, rsp);
			return FALSE;
	}
}

static int generate_opcode(rsp_state *rsp, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = op >> 26;
	code_label skip;

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
			UML_JMPc(block, COND_NE, skip = compiler->labelnum++);				// jmp    skip,NE
			generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);		// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;

		case 0x05:	/* BNE - MIPS I */
			UML_CMP(block, R32(RSREG), R32(RTREG));								// dcmp    <rsreg>,<rtreg>
			UML_JMPc(block, COND_E, skip = compiler->labelnum++);						// jmp     skip,E
			generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);		// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;

		case 0x06:	/* BLEZ - MIPS I */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), 0);								// dcmp    <rsreg>,0
				UML_JMPc(block, COND_G, skip = compiler->labelnum++);					// jmp     skip,G
				generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);	// <next instruction + hashjmp>
				UML_LABEL(block, skip);											// skip:
			}
			else
				generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);	// <next instruction + hashjmp>
			return TRUE;

		case 0x07:	/* BGTZ - MIPS I */
			UML_CMP(block, R32(RSREG), 0);									// dcmp    <rsreg>,0
			UML_JMPc(block, COND_LE, skip = compiler->labelnum++);					// jmp     skip,LE
			generate_delay_slot_and_branch(rsp, block, compiler, desc, 0);		// <next instruction + hashjmp>
			UML_LABEL(block, skip);												// skip:
			return TRUE;


		/* ----- immediate arithmetic ----- */

		case 0x0f:	/* LUI - MIPS I */
			if (RTREG != 0)
				UML_MOV(block, R32(RTREG), SIMMVAL << 16);					// dmov    <rtreg>,SIMMVAL << 16
			return TRUE;

		case 0x08:	/* ADDI - MIPS I */
		case 0x09:	/* ADDIU - MIPS I */
			if (RTREG != 0)
			{
				UML_ADD(block, R32(RTREG), R32(RSREG), SIMMVAL);				// add     i0,<rsreg>,SIMMVAL,V
			}
			return TRUE;

		case 0x0a:	/* SLTI - MIPS I */
			if (RTREG != 0)
			{
				UML_CMP(block, R32(RSREG), SIMMVAL);							// dcmp    <rsreg>,SIMMVAL
				UML_SETc(block, COND_L, R32(RTREG));									// dset    <rtreg>,l
			}
			return TRUE;

		case 0x0b:	/* SLTIU - MIPS I */
			if (RTREG != 0)
			{
				UML_CMP(block, R32(RSREG), SIMMVAL);							// dcmp    <rsreg>,SIMMVAL
				UML_SETc(block, COND_B, R32(RTREG));									// dset    <rtreg>,b
			}
			return TRUE;


		case 0x0c:	/* ANDI - MIPS I */
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), R32(RSREG), UIMMVAL);				// dand    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0d:	/* ORI - MIPS I */
			if (RTREG != 0)
				UML_OR(block, R32(RTREG), R32(RSREG), UIMMVAL);				// dor     <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		case 0x0e:	/* XORI - MIPS I */
			if (RTREG != 0)
				UML_XOR(block, R32(RTREG), R32(RSREG), UIMMVAL);				// dxor    <rtreg>,<rsreg>,UIMMVAL
			return TRUE;

		/* ----- memory load operations ----- */

		case 0x20:	/* LB - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *rsp->impstate->read8);									// callh   read8
			if (RTREG != 0)
				UML_SEXT(block, R32(RTREG), I0, SIZE_BYTE);						// dsext   <rtreg>,i0,byte
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x21:	/* LH - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *rsp->impstate->read16);								// callh   read16
			if (RTREG != 0)
				UML_SEXT(block, R32(RTREG), I0, SIZE_WORD);						// dsext   <rtreg>,i0,word
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x23:	/* LW - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *rsp->impstate->read32);								// callh   read32
			if (RTREG != 0)
				UML_MOV(block, R32(RTREG), I0);
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x24:	/* LBU - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *rsp->impstate->read8);									// callh   read8
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), I0, 0xff);					// dand    <rtreg>,i0,0xff
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x25:	/* LHU - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);						// add     i0,<rsreg>,SIMMVAL
			UML_CALLH(block, *rsp->impstate->read16);								// callh   read16
			if (RTREG != 0)
				UML_AND(block, R32(RTREG), I0, 0xffff);					// dand    <rtreg>,i0,0xffff
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x32:	/* LWC2 - MIPS I */
			return generate_lwc2(rsp, block, compiler, desc);


		/* ----- memory store operations ----- */

		case 0x28:	/* SB - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));									// mov     i1,<rtreg>
			UML_CALLH(block, *rsp->impstate->write8);								// callh   write8
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x29:	/* SH - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));									// mov     i1,<rtreg>
			UML_CALLH(block, *rsp->impstate->write16);								// callh   write16
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x2b:	/* SW - MIPS I */
			UML_ADD(block, I0, R32(RSREG), SIMMVAL);						// add     i0,<rsreg>,SIMMVAL
			UML_MOV(block, I1, R32(RTREG));									// mov     i1,<rtreg>
			UML_CALLH(block, *rsp->impstate->write32);								// callh   write32
			if (!in_delay_slot)
				generate_update_cycles(rsp, block, compiler, desc->pc + 4, TRUE);
			return TRUE;

		case 0x3a:	/* SWC2 - MIPS I */
			return generate_swc2(rsp, block, compiler, desc);
			//UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);     // mov     [arg0],desc->opptr.l
			//UML_CALLC(block, cfunc_swc2, rsp);                                        // callc   cfunc_mfc2
			//return TRUE;

		/* ----- coprocessor instructions ----- */

		case 0x10:	/* COP0 - MIPS I */
			return generate_cop0(rsp, block, compiler, desc);

		case 0x12:	/* COP2 - MIPS I */
			return generate_cop2(rsp, block, compiler, desc);
			//UML_EXH(block, rsp->impstate->exception[EXCEPTION_INVALIDOP], 0);// exh     invalidop,0
			//return TRUE;


		/* ----- unimplemented/illegal instructions ----- */

		//default:    /* ??? */       invalid_instruction(op);                                                break;
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
	//code_label skip;

	switch (opswitch)
	{
		/* ----- shift instructions ----- */

		case 0x00:	/* SLL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHL(block, R32(RDREG), R32(RTREG), SHIFT);
			}
			return TRUE;

		case 0x02:	/* SRL - MIPS I */
			if (RDREG != 0)
			{
				UML_SHR(block, R32(RDREG), R32(RTREG), SHIFT);
			}
			return TRUE;

		case 0x03:	/* SRA - MIPS I */
			if (RDREG != 0)
			{
				UML_SAR(block, R32(RDREG), R32(RTREG), SHIFT);
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
				UML_OR(block, I0, R32(RSREG), R32(RTREG));					// dor      i0,<rsreg>,<rtreg>
				UML_XOR(block, R32(RDREG), I0, (UINT64)~0);				// dxor     <rdreg>,i0,~0
			}
			return TRUE;


		/* ----- basic comparisons ----- */

		case 0x2a:	/* SLT - MIPS I */
			if (RDREG != 0)
			{
				UML_CMP(block, R32(RSREG), R32(RTREG));							// dcmp    <rsreg>,<rtreg>
				UML_SETc(block, COND_L, R32(RDREG));									// dset    <rdreg>,l
			}
			return TRUE;

		case 0x2b:	/* SLTU - MIPS I */
			if (RDREG != 0)
			{
				UML_CMP(block, R32(RSREG), R32(RTREG));							// dcmp    <rsreg>,<rtreg>
				UML_SETc(block, COND_B, R32(RDREG));									// dset    <rdreg>,b
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
			UML_MOV(block, mem(&rsp->impstate->arg0), 3);					// mov     [arg0],3
			UML_CALLC(block, cfunc_sp_set_status_cb, rsp);						// callc   cfunc_sp_set_status_cb
			UML_MOV(block, mem(&rsp->icount), 0);						// mov icount, #0

			UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);
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
	code_label skip;

	switch (opswitch)
	{
		case 0x00:	/* BLTZ */
		case 0x10:	/* BLTZAL */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), 0);								// dcmp    <rsreg>,0
				UML_JMPc(block, COND_GE, skip = compiler->labelnum++);				// jmp     skip,GE
				generate_delay_slot_and_branch(rsp, block, compiler, desc, (opswitch & 0x10) ? 31 : 0);
																					// <next instruction + hashjmp>
				UML_LABEL(block, skip);											// skip:
			}
			return TRUE;

		case 0x01:	/* BGEZ */
		case 0x11:	/* BGEZAL */
			if (RSREG != 0)
			{
				UML_CMP(block, R32(RSREG), 0);								// dcmp    <rsreg>,0
				UML_JMPc(block, COND_L, skip = compiler->labelnum++);					// jmp     skip,L
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
				UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);	// mov     [arg0],desc->opptr.l
				UML_CALLC(block, cfunc_mfc2, rsp);									// callc   cfunc_mfc2
				//UML_SEXT(block, R32(RTREG), I0, DWORD);                      // dsext   <rtreg>,i0,dword
			}
			return TRUE;

		case 0x02:	/* CFCz */
			if (RTREG != 0)
			{
				UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);	// mov     [arg0],desc->opptr.l
				UML_CALLC(block, cfunc_cfc2, rsp);									// callc   cfunc_cfc2
				//UML_SEXT(block, R32(RTREG), I0, DWORD);                      // dsext   <rtreg>,i0,dword
			}
			return TRUE;

		case 0x04:	/* MTCz */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_mtc2, rsp);										// callc   cfunc_mtc2
			return TRUE;

		case 0x06:	/* CTCz */
			UML_MOV(block, mem(&rsp->impstate->arg0), desc->opptr.l[0]);		// mov     [arg0],desc->opptr.l
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
				UML_MOV(block, mem(&rsp->impstate->arg0), RDREG);				// mov     [arg0],<rdreg>
				UML_MOV(block, mem(&rsp->impstate->arg1), RTREG);				// mov     [arg1],<rtreg>
				UML_CALLC(block, cfunc_get_cop0_reg, rsp);							// callc   cfunc_get_cop0_reg
				if(RDREG == 2)
				{
					generate_update_cycles(rsp, block, compiler, mem(&rsp->pc), TRUE);
					UML_HASHJMP(block, 0, mem(&rsp->pc), *rsp->impstate->nocode);
				}
			}
			return TRUE;

		case 0x04:	/* MTCz */
			UML_MOV(block, mem(&rsp->impstate->arg0), RDREG);					// mov     [arg0],<rdreg>
			UML_MOV(block, mem(&rsp->impstate->arg1), R32(RTREG));					// mov     [arg1],rtreg
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
	UINT16 b1 = VREG_B(VS1REG, (el+0) & 0xf);
	UINT16 b2 = VREG_B(VS1REG, (el+1) & 0xf);
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
	VREG_B(VS1REG, (el+0) & 0xf) = (RTVAL >> 8) & 0xff;
	VREG_B(VS1REG, (el+1) & 0xf) = (RTVAL >> 0) & 0xff;
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
	block->append_comment("%08X: %s", pc, buffer);									// comment
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
	rsp_state *rsp = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(rsp_state);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE:					info->i = CLEAR_LINE;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = rsp->ppc | 0x04000000;						break;

		case CPUINFO_INT_PC:	/* intentional fallthrough */
		case CPUINFO_INT_REGISTER + RSP_PC:				info->i = rsp->pc | 0x04000000;						break;

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
		case CPUINFO_INT_REGISTER + RSP_R31:			info->i = rsp->r[31];					 break;
		case CPUINFO_INT_REGISTER + RSP_SR:             info->i = rsp->sr;                       break;
		case CPUINFO_INT_REGISTER + RSP_NEXTPC:         info->i = rsp->nextpc | 0x04000000;      break;
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
		case CPUINFO_STR_NAME:							strcpy(info->s, "RSP");					break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "RSP");					break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.0");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team");	break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, " ");					break;

		case CPUINFO_STR_REGISTER + RSP_PC:				sprintf(info->s, "PC: %08X", rsp->pc | 0x04000000);	break;

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

DEFINE_LEGACY_CPU_DEVICE(RSP, rsp);

#endif // USE_RSPDRC
