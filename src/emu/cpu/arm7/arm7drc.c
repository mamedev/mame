/*****************************************************************************
 *
 *   arm7.c
 *   Portable CPU Emulator for 32-bit ARM v3/4/5/6
 *
 *   Copyright Steve Ellenoff, all rights reserved.
 *   Thumb, DSP, and MMU support and many bugfixes by R. Belmont and Ryan Holtz.
 *   Dyanmic Recompiler (DRC) / Just In Time Compiler (JIT) by Ryan Holtz.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     sellenoff@hotmail.com
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on:
 *  #1) 'Atmel Corporation ARM7TDMI (Thumb) Datasheet - January 1999'
 *  #2) Arm 2/3/6 emulator By Bryan McPhail (bmcphail@tendril.co.uk) and Phil Stroffolino (MAME CORE 0.76)
 *
 *****************************************************************************/

/******************************************************************************
 *  Notes:

    ** This is a plain vanilla implementation of an ARM7 cpu which incorporates my ARM7 core.
       It can be used as is, or used to demonstrate how to utilize the arm7 core to create a cpu
       that uses the core, since there are numerous different mcu packages that incorporate an arm7 core.

       See the notes in the arm7core.c file itself regarding issues/limitations of the arm7 core.
    **
*****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "arm7fe.h"
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

#ifdef ARM7_USE_DRC

using namespace uml;

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define FORCE_C_BACKEND                 (0)
#define LOG_UML                         (0)
#define LOG_NATIVE                      (0)

#define SINGLE_INSTRUCTION_MODE         (0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

typedef const void (*arm7thumb_drcophandler)(arm_state*, drcuml_block*, compiler_state*, opcode_desc*);

#include "arm7tdrc.c"

/* map variables */
#define MAPVAR_PC                       M0
#define MAPVAR_CYCLES                   M1

/* size of the execution code cache */
#define CACHE_SIZE                      (32 * 1024 * 1024)

/* compilation boundaries -- how far back/forward does the analysis extend? */
#define COMPILE_BACKWARDS_BYTES         128
#define COMPILE_FORWARDS_BYTES          512
#define COMPILE_MAX_INSTRUCTIONS        ((COMPILE_BACKWARDS_BYTES/4) + (COMPILE_FORWARDS_BYTES/4))
#define COMPILE_MAX_SEQUENCE            64

/* exit codes */
#define EXECUTE_OUT_OF_CYCLES           0
#define EXECUTE_MISSING_CODE            1
#define EXECUTE_UNMAPPED_CODE           2
#define EXECUTE_RESET_CACHE             3



/***************************************************************************
    MACROS
***************************************************************************/

/***************************************************************************
    STRUCTURES & TYPEDEFS
***************************************************************************/

/* fast RAM info */
struct fast_ram_info
{
	offs_t              start;                      /* start of the RAM block */
	offs_t              end;                        /* end of the RAM block */
	UINT8               readonly;                   /* TRUE if read-only */
	void *              base;                       /* base in memory where the RAM lives */
};


/* internal compiler state */
struct compiler_state
{
	UINT32              cycles;                     /* accumulated cycles */
	UINT8               checkints;                  /* need to check interrupts before next instruction */
	UINT8               checksoftints;              /* need to check software interrupts before next instruction */
	code_label  labelnum;                   /* index for local labels */
};


/* ARM7 registers */
struct arm7imp_state
{
	/* core state */
	drc_cache *         cache;                      /* pointer to the DRC code cache */
	drcuml_state *      drcuml;                     /* DRC UML generator state */
	arm7_frontend *    	drcfe;                      /* pointer to the DRC front-end state */
	UINT32              drcoptions;                 /* configurable DRC options */

	/* internal stuff */
	UINT8               cache_dirty;                /* true if we need to flush the cache */
	UINT32              jmpdest;                    /* destination jump target */

	/* parameters for subroutines */
	UINT64              numcycles;                  /* return value from gettotalcycles */
	UINT32              mode;                       /* current global mode */
	const char *        format;                     /* format string for print_debug */
	UINT32              arg0;                       /* print_debug argument 1 */
	UINT32              arg1;                       /* print_debug argument 2 */

	/* register mappings */
	parameter   regmap[NUM_REGS];               /* parameter to register mappings for all 16 integer registers */

	/* subroutines */
	code_handle *   entry;                      /* entry point */
	code_handle *   nocode;                     /* nocode exception handler */
	code_handle *   out_of_cycles;              /* out of cycles exception handler */
	code_handle *   tlb_translate;              /* tlb translation handler */
	code_handle *   detect_fault;               /* tlb fault detection handler */
	code_handle *   check_irq;               	/* irq check handler */
	code_handle *   read8;                   	/* read byte */
	code_handle *   write8;                  	/* write byte */
	code_handle *   read16;                  	/* read half */
	code_handle *   write16;                 	/* write half */
	code_handle *   read32;                  	/* read word */
	code_handle *   write32;                 	/* write word */

	/* fast RAM */
	UINT32              fastram_select;
	fast_ram_info       fastram[ARM7_MAX_FASTRAM];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void code_flush_cache(arm_state *arm);
static void code_compile_block(arm_state *arm, UINT8 mode, offs_t pc);

static void cfunc_printf_exception(void *param);
static void cfunc_get_cycles(void *param);

static void static_generate_entry_point(arm_state *arm);
static void static_generate_nocode_handler(arm_state *arm);
static void static_generate_out_of_cycles(arm_state *arm);
static void static_generate_tlb_translate(arm_state *arm);
static void static_generate_detect_fault(arm_state *arm);
static void static_generate_check_irq(arm_state *arm);

static void generate_update_cycles(arm_state *arm, drcuml_block *block, compiler_state *compiler, parameter param, int allow_exception);
static void generate_checksum_block(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
static void generate_sequence_instruction(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
static void generate_delay_slot_and_branch(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg);
static int generate_opcode(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);

static void log_add_disasm_comment(arm_state *arm, drcuml_block *block, UINT32 pc, UINT32 op);
static const char *log_desc_flags_to_string(UINT32 flags);
static void log_register_list(drcuml_state *drcuml, const char *string, const UINT32 *reglist, const UINT32 *regnostarlist);
static void log_opcode_desc(drcuml_state *drcuml, const opcode_desc *desclist, int indent);

/***************************************************************************
    PRIVATE GLOBAL VARIABLES
***************************************************************************/

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE arm_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ARM7 ||
			device->type() == ARM7_BE ||
			device->type() == ARM7500 ||
			device->type() == ARM9 ||
			device->type() == ARM920T ||
			device->type() == PXA255 ||
			device->type() == SA1110);
	return *(arm_state **)downcast<legacy_cpu_device *>(device)->token();
}

/*-------------------------------------------------
    epc - compute the exception PC from a
    descriptor
-------------------------------------------------*/

INLINE UINT32 epc(const opcode_desc *desc)
{
	return desc->pc;
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

INLINE void load_fast_iregs(arm_state *arm, drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(arm->impstate->regmap); regnum++)
		if (arm->impstate->regmap[regnum].is_int_register())
			UML_DMOV(block, ireg(arm->impstate->regmap[regnum].ireg() - REG_I0), mem(&arm->r[regnum]));
}


/*-------------------------------------------------
    save_fast_iregs - save any fast integer
    registers
-------------------------------------------------*/

INLINE void save_fast_iregs(arm_state *arm, drcuml_block *block)
{
	int regnum;

	for (regnum = 0; regnum < ARRAY_LENGTH(arm->impstate->regmap); regnum++)
		if (arm->impstate->regmap[regnum].is_int_register())
			UML_DMOV(block, mem(&arm->r[regnum]), ireg(arm->impstate->regmap[regnum].ireg() - REG_I0));
}



/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    arm7_init - initialize the processor
-------------------------------------------------*/

static void arm7_init(arm7_flavor flavor, int bigendian, legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback)
{
	arm_state *arm;
	drc_cache *cache;
	drcbe_info beinfo;
	UINT32 flags = 0;
	int regnum;

	arm7_core_init(device, "arm7");
	/* allocate enough space for the cache and the core */
	cache = auto_alloc(device->machine(), drc_cache(CACHE_SIZE + sizeof(*arm)));
	if (cache == NULL)
		fatalerror("Unable to allocate cache of size %d\n", (UINT32)(CACHE_SIZE + sizeof(*arm)));

	/* allocate the core memory */
	*(arm_state **)device->token() = arm = (arm_state *)cache->alloc_near(sizeof(*arm));
	memset(arm, 0, sizeof(*arm));

	/* initialize the core */
	arm7_core_init(device, "arm7");

	/* allocate the implementation-specific state from the full cache */
	arm->impstate = (arm7imp_state *)cache->alloc_near(sizeof(*arm->impstate));
	memset(arm->impstate, 0, sizeof(*arm->impstate));
	arm->impstate->cache = cache;

	/* initialize the UML generator */
	if (FORCE_C_BACKEND)
		flags |= DRCUML_OPTION_USE_C;
	if (LOG_UML)
		flags |= DRCUML_OPTION_LOG_UML;
	if (LOG_NATIVE)
		flags |= DRCUML_OPTION_LOG_NATIVE;
	arm->impstate->drcuml = auto_alloc(device->machine(), drcuml_state(*device, *cache, flags, 1, 32, 1));

	/* add symbols for our stuff */
	arm->impstate->drcuml->symbol_add(&arm->icount, sizeof(arm->icount), "icount");
	for (int regnum = 0; regnum < 37; regnum++)
	{
		char buf[10];
		sprintf(buf, "r%d", regnum);
		arm->impstate->drcuml->symbol_add(&arm->r[regnum], sizeof(arm->r[regnum]), buf);
	}
	arm->impstate->drcuml->symbol_add(&arm->impstate->mode, sizeof(arm->impstate->mode), "mode");
	arm->impstate->drcuml->symbol_add(&arm->impstate->arg0, sizeof(arm->impstate->arg0), "arg0");
	arm->impstate->drcuml->symbol_add(&arm->impstate->arg1, sizeof(arm->impstate->arg1), "arg1");
	arm->impstate->drcuml->symbol_add(&arm->impstate->numcycles, sizeof(arm->impstate->numcycles), "numcycles");
	arm->impstate->drcuml->symbol_add(&arm->impstate->fpmode, sizeof(arm->impstate->fpmode), "fpmode");

	/* initialize the front-end helper */
	arm->impstate->drcfe = auto_alloc(device->machine(), arm7_frontend(*arm, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE));

	/* allocate memory for cache-local state and initialize it */
	memcpy(arm->impstate->fpmode, fpmode_source, sizeof(fpmode_source));

	/* compute the register parameters */
	for (int regnum = 0; regnum < 37; regnum++)
	{
		arm->impstate->regmap[regnum] = (regnum == 0) ? parameter(0) : parameter::make_memory(&arm->r[regnum]);
	}

	/* if we have registers to spare, assign r2, r3, r4 to leftovers */
	if (!DISABLE_FAST_REGISTERS)
	{
		arm->impstate->drcuml->get_backend_info(beinfo);
		if (beinfo.direct_iregs > 4)
		{	// PC
			arm->impstate->regmap[eR15] = I4;
		}
		if (beinfo.direct_iregs > 5)
		{	// Status
			arm->impstate->regmap[eCPSR] = I5;
		}
		if (beinfo.direct_iregs > 6)
		{	// SP
			arm->impstate->regmap[eR13] = I6;
		}
	}

	/* mark the cache dirty so it is updated on next execute */
	arm->impstate->cache_dirty = TRUE;
}


/*-------------------------------------------------
    arm7_reset - reset the processor
-------------------------------------------------*/

static CPU_RESET( arm7 )
{
	arm_state *arm = get_safe_token(device);

	/* reset the common code and mark the cache dirty */
	arm7_core_reset(arm);

	arm->impstate->cache_dirty = TRUE;

	arm->archRev = 4;  // ARMv4
	arm->archFlags = eARM_ARCHFLAGS_T; // has Thumb
}

static CPU_RESET( arm7_be )
{
	arm_state *arm = get_safe_token(device);

	/* reset the common code and mark the cache dirty */
	arm7_core_reset(arm);

	arm->impstate->cache_dirty = TRUE;

	arm->endian = ENDIANNESS_BIG;

	arm->archRev = 4;  // ARMv4
	arm->archFlags = eARM_ARCHFLAGS_T; // has Thumb
}

static CPU_RESET( arm7500 )
{
	arm_state *arm = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	arm->impstate->cache_dirty = TRUE;

	arm->archRev = 3;  // ARMv3
	arm->archFlags = eARM_ARCHFLAGS_MODE26;
}

static CPU_RESET( arm9 )
{
	arm_state *arm = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	arm->impstate->cache_dirty = TRUE;

	arm->archRev = 5;  // ARMv5
	arm->archFlags = eARM_ARCHFLAGS_T | eARM_ARCHFLAGS_E;  // has TE extensions
}

static CPU_RESET( arm920t )
{
	arm_state *arm = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	arm->impstate->cache_dirty = TRUE;

	arm->archRev = 4;  // ARMv4
	arm->archFlags = eARM_ARCHFLAGS_T; // has T extension
}

static CPU_RESET( pxa255 )
{
	arm_state *arm = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	arm->impstate->cache_dirty = TRUE;

	arm->archRev = 5;  // ARMv5
	arm->archFlags = eARM_ARCHFLAGS_T | eARM_ARCHFLAGS_E | eARM_ARCHFLAGS_XSCALE;  // has TE and XScale extensions
}

static CPU_RESET( sa1110 )
{
	arm_state *arm = get_safe_token(device);

	// must call core reset
	arm7_core_reset(device);

	arm->impstate->cache_dirty = TRUE;

	arm->archRev = 4;  // ARMv4
	arm->archFlags = eARM_ARCHFLAGS_SA;    // has StrongARM, no Thumb, no Enhanced DSP
}

/*-------------------------------------------------
    arm7_execute - execute the CPU for the
    specified number of cycles
-------------------------------------------------*/

static CPU_EXECUTE( arm7 )
{
	arm_state *arm = get_safe_token(device);
	drcuml_state *drcuml = arm->impstate->drcuml;
	int execute_result;

	/* reset the cache if dirty */
	if (arm->impstate->cache_dirty)
		code_flush_cache(arm);
	arm->impstate->cache_dirty = FALSE;

	/* execute */
	do
	{
		/* run as much as we can */
		execute_result = drcuml->execute(*arm->impstate->entry);

		/* if we need to recompile, do it */
		if (execute_result == EXECUTE_MISSING_CODE)
			code_compile_block(arm, arm->impstate->mode, arm->r[eR15]);
		else if (execute_result == EXECUTE_UNMAPPED_CODE)
			fatalerror("Attempted to execute unmapped code at PC=%08X\n", arm->r[eR15]);
		else if (execute_result == EXECUTE_RESET_CACHE)
			code_flush_cache(arm);

	} while (execute_result != EXECUTE_OUT_OF_CYCLES);
}

/*-------------------------------------------------
    arm7_exit - cleanup from execution
-------------------------------------------------*/

static CPU_EXIT( arm7 )
{
	arm_state *arm = get_safe_token(device);

	/* clean up the DRC */
	auto_free(device->machine(), arm->impstate->drcfe);
	auto_free(device->machine(), arm->impstate->drcuml);
	auto_free(device->machine(), arm->impstate->cache);
}


/*-------------------------------------------------
    arm7_translate - perform virtual-to-physical
    address translation
-------------------------------------------------*/

static CPU_TRANSLATE( arm7 )
{
	arm_state *arm = get_safe_token(device);

	/* only applies to the program address space and only does something if the MMU's enabled */
	if( space == AS_PROGRAM && ( COPRO_CTRL & COPRO_CTRL_MMU_EN ) )
	{
		return arm7_tlb_translate(arm, address, 0);
	}
	return TRUE;
}


static CPU_DISASSEMBLE( arm7 )
{
	CPU_DISASSEMBLE( arm7arm );
	CPU_DISASSEMBLE( arm7thumb );

	arm_state *arm = get_safe_token(device);

	if (T_IS_SET(GET_CPSR))
		return CPU_DISASSEMBLE_CALL(arm7thumb);
	else
		return CPU_DISASSEMBLE_CALL(arm7arm);
}


/*-------------------------------------------------
    arm7_set_info - set information about a given
    CPU instance
-------------------------------------------------*/

static CPU_SET_INFO( arm7 )
{
	arm_state *arm = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */

		/* interrupt lines/exceptions */
		case CPUINFO_INT_INPUT_STATE + ARM7_IRQ_LINE:                   set_irq_line(arm, ARM7_IRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + ARM7_FIRQ_LINE:                  set_irq_line(arm, ARM7_FIRQ_LINE, info->i); break;
		case CPUINFO_INT_INPUT_STATE + ARM7_ABORT_EXCEPTION:            set_irq_line(arm, ARM7_ABORT_EXCEPTION, info->i); break;
		case CPUINFO_INT_INPUT_STATE + ARM7_ABORT_PREFETCH_EXCEPTION:   set_irq_line(arm, ARM7_ABORT_PREFETCH_EXCEPTION, info->i); break;
		case CPUINFO_INT_INPUT_STATE + ARM7_UNDEFINE_EXCEPTION:         set_irq_line(arm, ARM7_UNDEFINE_EXCEPTION, info->i); break;

		/* registers shared by all operating modes */
		case CPUINFO_INT_REGISTER + ARM7_R0:            ARM7REG( 0) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R1:            ARM7REG( 1) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R2:            ARM7REG( 2) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R3:            ARM7REG( 3) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R4:            ARM7REG( 4) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R5:            ARM7REG( 5) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R6:            ARM7REG( 6) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R7:            ARM7REG( 7) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R8:            ARM7REG( 8) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R9:            ARM7REG( 9) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R10:           ARM7REG(10) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R11:           ARM7REG(11) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R12:           ARM7REG(12) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R13:           ARM7REG(13) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R14:           ARM7REG(14) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_R15:           ARM7REG(15) = info->i;                  break;
		case CPUINFO_INT_REGISTER + ARM7_CPSR:          SET_CPSR(info->i);                      break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ARM7_PC:            R15 = info->i;                          break;
		case CPUINFO_INT_SP:                            SetRegister(arm, 13,info->i);                break;

		/* FIRQ Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_FR8:           ARM7REG(eR8_FIQ)  = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_FR9:           ARM7REG(eR9_FIQ)  = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_FR10:          ARM7REG(eR10_FIQ) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_FR11:          ARM7REG(eR11_FIQ) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_FR12:          ARM7REG(eR12_FIQ) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_FR13:          ARM7REG(eR13_FIQ) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_FR14:          ARM7REG(eR14_FIQ) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_FSPSR:         ARM7REG(eSPSR_FIQ) = info->i;           break;

		/* IRQ Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_IR13:          ARM7REG(eR13_IRQ) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_IR14:          ARM7REG(eR14_IRQ) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_ISPSR:         ARM7REG(eSPSR_IRQ) = info->i;           break;

		/* Supervisor Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_SR13:          ARM7REG(eR13_SVC) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_SR14:          ARM7REG(eR14_SVC) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_SSPSR:         ARM7REG(eSPSR_SVC) = info->i;           break;

		/* Abort Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_AR13:          ARM7REG(eR13_ABT) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_AR14:          ARM7REG(eR14_ABT) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_ASPSR:         ARM7REG(eSPSR_ABT) = info->i;           break;

		/* Undefined Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_UR13:          ARM7REG(eR13_UND) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_UR14:          ARM7REG(eR14_UND) = info->i;            break;
		case CPUINFO_INT_REGISTER + ARM7_USPSR:         ARM7REG(eSPSR_UND) = info->i;           break;
	}
}


/*-------------------------------------------------
    arm7_get_info - return information about a
    given CPU instance
-------------------------------------------------*/

static CPU_GET_INFO( arm7 )
{
	arm_state *arm = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */

		/* cpu implementation data */
		case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(arm_state);                 break;
		case CPUINFO_INT_INPUT_LINES:                   info->i = ARM7_NUM_LINES;               break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:            info->i = 0;                            break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_LITTLE;                    break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                            break;
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 1;                            break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 2;                            break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 4;                            break;
		case CPUINFO_INT_MIN_CYCLES:                    info->i = 3;                            break;
		case CPUINFO_INT_MAX_CYCLES:                    info->i = 4;                            break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM: info->i = 32;                   break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;                   break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;                    break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:    info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:    info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:    info->i = 0;                    break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:      info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:      info->i = 0;                    break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:      info->i = 0;                    break;

		/* interrupt lines/exceptions */
		case CPUINFO_INT_INPUT_STATE + ARM7_IRQ_LINE:                   info->i = arm->pendingIrq; break;
		case CPUINFO_INT_INPUT_STATE + ARM7_FIRQ_LINE:                  info->i = arm->pendingFiq; break;
		case CPUINFO_INT_INPUT_STATE + ARM7_ABORT_EXCEPTION:            info->i = arm->pendingAbtD; break;
		case CPUINFO_INT_INPUT_STATE + ARM7_ABORT_PREFETCH_EXCEPTION:   info->i = arm->pendingAbtP; break;
		case CPUINFO_INT_INPUT_STATE + ARM7_UNDEFINE_EXCEPTION:         info->i = arm->pendingUnd; break;

		/* registers shared by all operating modes */
		case CPUINFO_INT_REGISTER + ARM7_R0:    info->i = ARM7REG( 0);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R1:    info->i = ARM7REG( 1);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R2:    info->i = ARM7REG( 2);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R3:    info->i = ARM7REG( 3);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R4:    info->i = ARM7REG( 4);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R5:    info->i = ARM7REG( 5);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R6:    info->i = ARM7REG( 6);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R7:    info->i = ARM7REG( 7);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R8:    info->i = ARM7REG( 8);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R9:    info->i = ARM7REG( 9);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R10:   info->i = ARM7REG(10);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R11:   info->i = ARM7REG(11);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R12:   info->i = ARM7REG(12);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R13:   info->i = ARM7REG(13);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R14:   info->i = ARM7REG(14);                          break;
		case CPUINFO_INT_REGISTER + ARM7_R15:   info->i = ARM7REG(15);                          break;

		case CPUINFO_INT_PREVIOUSPC:            info->i = 0;    /* not implemented */           break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ARM7_PC:    info->i = GET_PC;                                  break;
		case CPUINFO_INT_SP:                    info->i = GetRegister(arm, 13);            break;

		/* FIRQ Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_FR8:   info->i = ARM7REG(eR8_FIQ);                     break;
		case CPUINFO_INT_REGISTER + ARM7_FR9:   info->i = ARM7REG(eR9_FIQ);                     break;
		case CPUINFO_INT_REGISTER + ARM7_FR10:  info->i = ARM7REG(eR10_FIQ);                    break;
		case CPUINFO_INT_REGISTER + ARM7_FR11:  info->i = ARM7REG(eR11_FIQ);                    break;
		case CPUINFO_INT_REGISTER + ARM7_FR12:  info->i = ARM7REG(eR12_FIQ);                    break;
		case CPUINFO_INT_REGISTER + ARM7_FR13:  info->i = ARM7REG(eR13_FIQ);                    break;
		case CPUINFO_INT_REGISTER + ARM7_FR14:  info->i = ARM7REG(eR14_FIQ);                    break;
		case CPUINFO_INT_REGISTER + ARM7_FSPSR: info->i = ARM7REG(eSPSR_FIQ);                   break;

		/* IRQ Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_IR13:  info->i = ARM7REG(eR13_IRQ);                    break;
		case CPUINFO_INT_REGISTER + ARM7_IR14:  info->i = ARM7REG(eR14_IRQ);                    break;
		case CPUINFO_INT_REGISTER + ARM7_ISPSR: info->i = ARM7REG(eSPSR_IRQ);                   break;

		/* Supervisor Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_SR13:  info->i = ARM7REG(eR13_SVC);                    break;
		case CPUINFO_INT_REGISTER + ARM7_SR14:  info->i = ARM7REG(eR14_SVC);                    break;
		case CPUINFO_INT_REGISTER + ARM7_SSPSR: info->i = ARM7REG(eSPSR_SVC);                   break;

		/* Abort Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_AR13:  info->i = ARM7REG(eR13_ABT);                    break;
		case CPUINFO_INT_REGISTER + ARM7_AR14:  info->i = ARM7REG(eR14_ABT);                    break;
		case CPUINFO_INT_REGISTER + ARM7_ASPSR: info->i = ARM7REG(eSPSR_ABT);                   break;

		/* Undefined Mode Shadowed Registers */
		case CPUINFO_INT_REGISTER + ARM7_UR13:  info->i = ARM7REG(eR13_UND);                    break;
		case CPUINFO_INT_REGISTER + ARM7_UR14:  info->i = ARM7REG(eR14_UND);                    break;
		case CPUINFO_INT_REGISTER + ARM7_USPSR: info->i = ARM7REG(eSPSR_UND);                   break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:              info->setinfo = CPU_SET_INFO_NAME(arm7);                  break;
		case CPUINFO_FCT_INIT:                  info->init = CPU_INIT_NAME(arm7);                         break;
		case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(arm7);                       break;
		case CPUINFO_FCT_EXIT:                  info->exit = CPU_EXIT_NAME(arm7);                         break;
		case CPUINFO_FCT_EXECUTE:               info->execute = CPU_EXECUTE_NAME(arm7);                   break;
		case CPUINFO_FCT_BURN:                  info->burn = NULL;                              break;
		case CPUINFO_FCT_DISASSEMBLE:           info->disassemble = CPU_DISASSEMBLE_NAME(arm7);                  break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:   info->icount = &ARM7_ICOUNT;                    break;
	case CPUINFO_FCT_TRANSLATE:         info->translate = CPU_TRANSLATE_NAME(arm7);     break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                  strcpy(info->s, "ARM7");                        break;
		case CPUINFO_STR_FAMILY:           strcpy(info->s, "Acorn Risc Machine");          break;
		case CPUINFO_STR_VERSION:          strcpy(info->s, "2.0");                         break;
		case CPUINFO_STR_SOURCE_FILE:             strcpy(info->s, __FILE__);                      break;
		case CPUINFO_STR_CREDITS:          strcpy(info->s, "Copyright Steve Ellenoff, sellenoff@hotmail.com"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c %s",
					(ARM7REG(eCPSR) & N_MASK) ? 'N' : '-',
					(ARM7REG(eCPSR) & Z_MASK) ? 'Z' : '-',
					(ARM7REG(eCPSR) & C_MASK) ? 'C' : '-',
					(ARM7REG(eCPSR) & V_MASK) ? 'V' : '-',
					(ARM7REG(eCPSR) & Q_MASK) ? 'Q' : '-',
					(ARM7REG(eCPSR) & I_MASK) ? 'I' : '-',
					(ARM7REG(eCPSR) & F_MASK) ? 'F' : '-',
					(ARM7REG(eCPSR) & T_MASK) ? 'T' : '-',
					GetModeText(ARM7REG(eCPSR)));
		break;

		/* registers shared by all operating modes */
		case CPUINFO_STR_REGISTER + ARM7_PC:    sprintf(info->s, "PC  :%08x", GET_PC);            break;
		case CPUINFO_STR_REGISTER + ARM7_R0:    sprintf(info->s, "R0  :%08x", ARM7REG( 0));    break;
		case CPUINFO_STR_REGISTER + ARM7_R1:    sprintf(info->s, "R1  :%08x", ARM7REG( 1));    break;
		case CPUINFO_STR_REGISTER + ARM7_R2:    sprintf(info->s, "R2  :%08x", ARM7REG( 2));    break;
		case CPUINFO_STR_REGISTER + ARM7_R3:    sprintf(info->s, "R3  :%08x", ARM7REG( 3));    break;
		case CPUINFO_STR_REGISTER + ARM7_R4:    sprintf(info->s, "R4  :%08x", ARM7REG( 4));    break;
		case CPUINFO_STR_REGISTER + ARM7_R5:    sprintf(info->s, "R5  :%08x", ARM7REG( 5));    break;
		case CPUINFO_STR_REGISTER + ARM7_R6:    sprintf(info->s, "R6  :%08x", ARM7REG( 6));    break;
		case CPUINFO_STR_REGISTER + ARM7_R7:    sprintf(info->s, "R7  :%08x", ARM7REG( 7));    break;
		case CPUINFO_STR_REGISTER + ARM7_R8:    sprintf(info->s, "R8  :%08x", ARM7REG( 8));    break;
		case CPUINFO_STR_REGISTER + ARM7_R9:    sprintf(info->s, "R9  :%08x", ARM7REG( 9));    break;
		case CPUINFO_STR_REGISTER + ARM7_R10:   sprintf(info->s, "R10 :%08x", ARM7REG(10));    break;
		case CPUINFO_STR_REGISTER + ARM7_R11:   sprintf(info->s, "R11 :%08x", ARM7REG(11));    break;
		case CPUINFO_STR_REGISTER + ARM7_R12:   sprintf(info->s, "R12 :%08x", ARM7REG(12));    break;
		case CPUINFO_STR_REGISTER + ARM7_R13:   sprintf(info->s, "R13 :%08x", ARM7REG(13));    break;
		case CPUINFO_STR_REGISTER + ARM7_R14:   sprintf(info->s, "R14 :%08x", ARM7REG(14));    break;
		case CPUINFO_STR_REGISTER + ARM7_R15:   sprintf(info->s, "R15 :%08x", ARM7REG(15));    break;

		/* FIRQ Mode Shadowed Registers */
		case CPUINFO_STR_REGISTER + ARM7_FR8:   sprintf(info->s, "FR8 :%08x", ARM7REG(eR8_FIQ)  ); break;
		case CPUINFO_STR_REGISTER + ARM7_FR9:   sprintf(info->s, "FR9 :%08x", ARM7REG(eR9_FIQ)  ); break;
		case CPUINFO_STR_REGISTER + ARM7_FR10:  sprintf(info->s, "FR10:%08x", ARM7REG(eR10_FIQ) ); break;
		case CPUINFO_STR_REGISTER + ARM7_FR11:  sprintf(info->s, "FR11:%08x", ARM7REG(eR11_FIQ) ); break;
		case CPUINFO_STR_REGISTER + ARM7_FR12:  sprintf(info->s, "FR12:%08x", ARM7REG(eR12_FIQ) ); break;
		case CPUINFO_STR_REGISTER + ARM7_FR13:  sprintf(info->s, "FR13:%08x", ARM7REG(eR13_FIQ) ); break;
		case CPUINFO_STR_REGISTER + ARM7_FR14:  sprintf(info->s, "FR14:%08x", ARM7REG(eR14_FIQ) ); break;
		case CPUINFO_STR_REGISTER + ARM7_FSPSR: sprintf(info->s, "FR16:%08x", ARM7REG(eSPSR_FIQ)); break;

		/* IRQ Mode Shadowed Registers */
		case CPUINFO_STR_REGISTER + ARM7_IR13:  sprintf(info->s, "IR13:%08x", ARM7REG(eR13_IRQ) ); break;
		case CPUINFO_STR_REGISTER + ARM7_IR14:  sprintf(info->s, "IR14:%08x", ARM7REG(eR14_IRQ) ); break;
		case CPUINFO_STR_REGISTER + ARM7_ISPSR: sprintf(info->s, "IR16:%08x", ARM7REG(eSPSR_IRQ)); break;

		/* Supervisor Mode Shadowed Registers */
		case CPUINFO_STR_REGISTER + ARM7_SR13:  sprintf(info->s, "SR13:%08x", ARM7REG(eR13_SVC) ); break;
		case CPUINFO_STR_REGISTER + ARM7_SR14:  sprintf(info->s, "SR14:%08x", ARM7REG(eR14_SVC) ); break;
		case CPUINFO_STR_REGISTER + ARM7_SSPSR: sprintf(info->s, "SR16:%08x", ARM7REG(eSPSR_SVC)); break;

		/* Abort Mode Shadowed Registers */
		case CPUINFO_STR_REGISTER + ARM7_AR13:  sprintf(info->s, "AR13:%08x", ARM7REG(eR13_ABT) ); break;
		case CPUINFO_STR_REGISTER + ARM7_AR14:  sprintf(info->s, "AR14:%08x", ARM7REG(eR14_ABT) ); break;
		case CPUINFO_STR_REGISTER + ARM7_ASPSR: sprintf(info->s, "AR16:%08x", ARM7REG(eSPSR_ABT)); break;

		/* Undefined Mode Shadowed Registers */
		case CPUINFO_STR_REGISTER + ARM7_UR13:  sprintf(info->s, "UR13:%08x", ARM7REG(eR13_UND) ); break;
		case CPUINFO_STR_REGISTER + ARM7_UR14:  sprintf(info->s, "UR14:%08x", ARM7REG(eR14_UND) ); break;
		case CPUINFO_STR_REGISTER + ARM7_USPSR: sprintf(info->s, "UR16:%08x", ARM7REG(eSPSR_UND)); break;
	}
}

CPU_GET_INFO( arm7_be )
{
	switch (state)
	{
		case CPUINFO_INT_ENDIANNESS:        info->i = ENDIANNESS_BIG;                               break;
		case CPUINFO_FCT_RESET:             info->reset = CPU_RESET_NAME(arm7_be);                  break;
		case CPUINFO_FCT_DISASSEMBLE:       info->disassemble = CPU_DISASSEMBLE_NAME(arm7_be);      break;
		case CPUINFO_STR_NAME:              strcpy(info->s, "ARM7 (big endian)");                   break;
		default:                            CPU_GET_INFO_CALL(arm7);
	}
}

CPU_GET_INFO( arm7500 )
{
	switch (state)
	{
		case CPUINFO_FCT_RESET:     info->reset = CPU_RESET_NAME(arm7500);      break;
		case CPUINFO_STR_NAME:      strcpy(info->s, "ARM7500");             break;
		default:                    CPU_GET_INFO_CALL(arm7);
		break;
	}
}

CPU_GET_INFO( arm9 )
{
	switch (state)
	{
		case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(arm9);                       break;
		case CPUINFO_STR_NAME:             strcpy(info->s, "ARM9");                        break;
	default:    CPU_GET_INFO_CALL(arm7);
		break;
	}
}

CPU_GET_INFO( arm920t )
{
	switch (state)
	{
		case CPUINFO_FCT_RESET:                 info->reset = CPU_RESET_NAME(arm920t);                       break;
		case CPUINFO_STR_NAME:             strcpy(info->s, "ARM920T");                        break;
	default:    CPU_GET_INFO_CALL(arm7);
		break;
	}
}

CPU_GET_INFO( pxa255 )
{
	switch (state)
	{
		case CPUINFO_FCT_RESET:            info->reset = CPU_RESET_NAME(pxa255);                       break;
		case CPUINFO_STR_NAME:             strcpy(info->s, "PXA255");                        break;
	default:    CPU_GET_INFO_CALL(arm7);
		break;
	}
}

CPU_GET_INFO( sa1110 )
{
	switch (state)
	{
		case CPUINFO_FCT_RESET:            info->reset = CPU_RESET_NAME(sa1110);                       break;
		case CPUINFO_STR_NAME:             strcpy(info->s, "SA1110");                        break;
	default:    CPU_GET_INFO_CALL(arm7);
		break;
	}
}


/* ARM system coprocessor support */
static WRITE32_DEVICE_HANDLER( arm7_do_callback )
{
	arm_state *arm = get_safe_token(device);
	arm->pendingUnd = 1;
}

static READ32_DEVICE_HANDLER( arm7_rt_r_callback )
{
	arm_state *arm = get_safe_token(device);
	UINT32 opcode = offset;
	UINT8 cReg = ( opcode & INSN_COPRO_CREG ) >> INSN_COPRO_CREG_SHIFT;
	UINT8 op2 =  ( opcode & INSN_COPRO_OP2 )  >> INSN_COPRO_OP2_SHIFT;
	UINT8 op3 =    opcode & INSN_COPRO_OP3;
	UINT8 cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;
	UINT32 data = 0;

//    printf("cpnum %d cReg %d op2 %d op3 %d (%x)\n", cpnum, cReg, op2, op3, GET_REGISTER(arm, 15));

	// we only handle system copro here
	if (cpnum != 15)
	{
		if (arm->archFlags & eARM_ARCHFLAGS_XSCALE)
	{
		// handle XScale specific CP14
		if (cpnum == 14)
		{
			switch( cReg )
			{
				case 1: // clock counter
					data = (UINT32)arm->device->total_cycles();
					break;

				default:
					break;
			}
		}
		else
		{
			fatalerror("XScale: Unhandled coprocessor %d (archFlags %x)\n", cpnum, arm->archFlags);
		}

		return data;
	}
	else
	{
		LOG( ("ARM7: Unhandled coprocessor %d (archFlags %x)\n", cpnum, arm->archFlags) );
		arm->pendingUnd = 1;
		return 0;
	}
	}

	switch( cReg )
	{
		case 4:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			// RESERVED
			LOG( ( "arm7_rt_r_callback CR%d, RESERVED\n", cReg ) );
			break;
		case 0:             // ID
		switch(op2)
		{
			case 0:
			switch (arm->archRev)
			{
				case 3: // ARM6 32-bit
				data = 0x41;
				break;

			case 4: // ARM7/SA11xx
				if (arm->archFlags & eARM_ARCHFLAGS_SA)
				{
					// ARM Architecture Version 4
					// Part Number 0xB11 (SA1110)
					// Stepping B5
						data = 0x69 | ( 0x01 << 16 ) | ( 0xB11 << 4 ) | 0x9;
				}
				else
				{
					if (device->type() == ARM920T)
					{
						data = (0x41 << 24) | (1 << 20) | (2 << 16) | (0x920 << 4) | (0 << 0); // ARM920T (S3C24xx)
					}
					else if (device->type() == ARM7500)
					{
						data = (0x41 << 24) | (0 << 20) | (1 << 16) | (0x710 << 4) | (0 << 0); // ARM7500
					}
					else
					{
						data = 0x41 | (1 << 23) | (7 << 12); // <-- where did this come from?
					}
				}
				break;

			case 5: // ARM9/10/XScale
				data = 0x41 | (9 << 12);
				if (arm->archFlags & eARM_ARCHFLAGS_T)
				{
					if (arm->archFlags & eARM_ARCHFLAGS_E)
					{
						if (arm->archFlags & eARM_ARCHFLAGS_J)
						{
							data |= (6<<16);    // v5TEJ
						}
						else
						{
							data |= (5<<16);    // v5TE
						}
					}
					else
					{
						data |= (4<<16);    // v5T
					}
				}
				break;

			case 6: // ARM11
				data = 0x41 | (10<< 12) | (7<<16);  // v6
				break;
			}
			break;
			case 1: // cache type
			data = 0x0f0d2112;  // HACK: value expected by ARMWrestler (probably Nintendo DS ARM9's value)
			//data = (6 << 25) | (1 << 24) | (0x172 << 12) | (0x172 << 0); // ARM920T (S3C24xx)
			break;
		case 2: // TCM type
			data = 0;
			break;
		case 3: // TLB type
			data = 0;
			break;
		case 4: // MPU type
			data = 0;
			break;
		}
			LOG( ( "arm7_rt_r_callback, ID\n" ) );
			break;
		case 1:             // Control
			data = COPRO_CTRL | 0x70;   // bits 4-6 always read back as "1" (bit 3 too in XScale)
			break;
		case 2:             // Translation Table Base
			data = COPRO_TLB_BASE;
			break;
		case 3:             // Domain Access Control
			LOG( ( "arm7_rt_r_callback, Domain Access Control\n" ) );
			data = COPRO_DOMAIN_ACCESS_CONTROL;
			break;
		case 5:             // Fault Status
			LOG( ( "arm7_rt_r_callback, Fault Status\n" ) );
			switch (op3)
			{
				case 0: data = COPRO_FAULT_STATUS_D; break;
				case 1: data = COPRO_FAULT_STATUS_P; break;
			}
			break;
		case 6:             // Fault Address
			LOG( ( "arm7_rt_r_callback, Fault Address\n" ) );
			data = COPRO_FAULT_ADDRESS;
			break;
		case 13:            // Read Process ID (PID)
			LOG( ( "arm7_rt_r_callback, Read PID\n" ) );
			data = COPRO_FCSE_PID;
			break;
		case 14:            // Read Breakpoint
			LOG( ( "arm7_rt_r_callback, Read Breakpoint\n" ) );
			break;
		case 15:            // Test, Clock, Idle
			LOG( ( "arm7_rt_r_callback, Test / Clock / Idle \n" ) );
			break;
	}

	return data;
}

static WRITE32_DEVICE_HANDLER( arm7_rt_w_callback )
{
	arm_state *arm = get_safe_token(device);
	UINT32 opcode = offset;
	UINT8 cReg = ( opcode & INSN_COPRO_CREG ) >> INSN_COPRO_CREG_SHIFT;
	UINT8 op2 =  ( opcode & INSN_COPRO_OP2 )  >> INSN_COPRO_OP2_SHIFT;
	UINT8 op3 =    opcode & INSN_COPRO_OP3;
	UINT8 cpnum = (opcode & INSN_COPRO_CPNUM) >> INSN_COPRO_CPNUM_SHIFT;

	// handle XScale specific CP14 - just eat writes for now
	if (cpnum != 15)
	{
		if (cpnum == 14)
		{
			LOG( ("arm7_rt_w_callback: write %x to XScale CP14 reg %d\n", data, cReg) );
			return;
		}
		else
		{
			LOG( ("ARM7: Unhandled coprocessor %d\n", cpnum) );
			arm->pendingUnd = 1;
			return;
		}
	}

	switch( cReg )
	{
		case 0:
		case 4:
		case 10:
		case 11:
		case 12:
			// RESERVED
			LOG( ( "arm7_rt_w_callback CR%d, RESERVED = %08x\n", cReg, data) );
			break;
		case 1:             // Control
			LOG( ( "arm7_rt_w_callback Control = %08x (%d) (%d)\n", data, op2, op3 ) );
			LOG( ( "    MMU:%d, Address Fault:%d, Data Cache:%d, Write Buffer:%d\n",
					data & COPRO_CTRL_MMU_EN, ( data & COPRO_CTRL_ADDRFAULT_EN ) >> COPRO_CTRL_ADDRFAULT_EN_SHIFT,
					( data & COPRO_CTRL_DCACHE_EN ) >> COPRO_CTRL_DCACHE_EN_SHIFT,
					( data & COPRO_CTRL_WRITEBUF_EN ) >> COPRO_CTRL_WRITEBUF_EN_SHIFT ) );
			LOG( ( "    Endianness:%d, System:%d, ROM:%d, Instruction Cache:%d\n",
					( data & COPRO_CTRL_ENDIAN ) >> COPRO_CTRL_ENDIAN_SHIFT,
					( data & COPRO_CTRL_SYSTEM ) >> COPRO_CTRL_SYSTEM_SHIFT,
					( data & COPRO_CTRL_ROM ) >> COPRO_CTRL_ROM_SHIFT,
					( data & COPRO_CTRL_ICACHE_EN ) >> COPRO_CTRL_ICACHE_EN_SHIFT ) );
			LOG( ( "    Int Vector Adjust:%d\n", ( data & COPRO_CTRL_INTVEC_ADJUST ) >> COPRO_CTRL_INTVEC_ADJUST_SHIFT ) );
#if ARM7_MMU_ENABLE_HACK
			if (((data & COPRO_CTRL_MMU_EN) != 0) && ((COPRO_CTRL & COPRO_CTRL_MMU_EN) == 0))
			{
				arm->mmu_enable_addr = R15;
			}
			if (((data & COPRO_CTRL_MMU_EN) == 0) && ((COPRO_CTRL & COPRO_CTRL_MMU_EN) != 0))
			{
				if (!arm7_tlb_translate( arm, &R15, 0))
				{
					fatalerror("ARM7_MMU_ENABLE_HACK translate failed\n");
				}
			}
#endif
			COPRO_CTRL = data & COPRO_CTRL_MASK;
			break;
		case 2:             // Translation Table Base
			LOG( ( "arm7_rt_w_callback TLB Base = %08x (%d) (%d)\n", data, op2, op3 ) );
			COPRO_TLB_BASE = data;
			break;
		case 3:             // Domain Access Control
			LOG( ( "arm7_rt_w_callback Domain Access Control = %08x (%d) (%d)\n", data, op2, op3 ) );
			COPRO_DOMAIN_ACCESS_CONTROL = data;
			break;
		case 5:             // Fault Status
			LOG( ( "arm7_rt_w_callback Fault Status = %08x (%d) (%d)\n", data, op2, op3 ) );
			switch (op3)
			{
				case 0: COPRO_FAULT_STATUS_D = data; break;
				case 1: COPRO_FAULT_STATUS_P = data; break;
			}
			break;
		case 6:             // Fault Address
			LOG( ( "arm7_rt_w_callback Fault Address = %08x (%d) (%d)\n", data, op2, op3 ) );
			COPRO_FAULT_ADDRESS = data;
			break;
		case 7:             // Cache Operations
//            LOG( ( "arm7_rt_w_callback Cache Ops = %08x (%d) (%d)\n", data, op2, op3 ) );
			break;
		case 8:             // TLB Operations
			LOG( ( "arm7_rt_w_callback TLB Ops = %08x (%d) (%d)\n", data, op2, op3 ) );
			break;
		case 9:             // Read Buffer Operations
			LOG( ( "arm7_rt_w_callback Read Buffer Ops = %08x (%d) (%d)\n", data, op2, op3 ) );
			break;
		case 13:            // Write Process ID (PID)
			LOG( ( "arm7_rt_w_callback Write PID = %08x (%d) (%d)\n", data, op2, op3 ) );
			COPRO_FCSE_PID = data;
			break;
		case 14:            // Write Breakpoint
			LOG( ( "arm7_rt_w_callback Write Breakpoint = %08x (%d) (%d)\n", data, op2, op3 ) );
			break;
		case 15:            // Test, Clock, Idle
			LOG( ( "arm7_rt_w_callback Test / Clock / Idle = %08x (%d) (%d)\n", data, op2, op3 ) );
			break;
	}
}

void arm7_dt_r_callback(arm_state *arm, UINT32 insn, UINT32 *prn, UINT32 (*read32)(arm_state *arm, UINT32 addr))
{
	UINT8 cpn = (insn >> 8) & 0xF;
	if ((arm->archFlags & eARM_ARCHFLAGS_XSCALE) && (cpn == 0))
	{
		LOG( ( "arm7_dt_r_callback: DSP Coprocessor 0 (CP0) not yet emulated (PC %08x)\n", GET_PC ) );
	}
	else
	{
		arm->pendingUnd = 1;
	}
}

void arm7_dt_w_callback(arm_state *arm, UINT32 insn, UINT32 *prn, void (*write32)(arm_state *arm, UINT32 addr, UINT32 data))
{
	UINT8 cpn = (insn >> 8) & 0xF;
	if ((arm->archFlags & eARM_ARCHFLAGS_XSCALE) && (cpn == 0))
	{
		LOG( ( "arm7_dt_w_callback: DSP Coprocessor 0 (CP0) not yet emulated (PC %08x)\n", GET_PC ) );
	}
	else
	{
		arm->pendingUnd = 1;
	}
}


/*-------------------------------------------------
    arm7drc_set_options - configure DRC options
-------------------------------------------------*/

void arm7drc_set_options(device_t *device, UINT32 options)
{
	arm_state *arm = get_safe_token(device);
	arm->impstate->drcoptions = options;
}


/*-------------------------------------------------
    arm7drc_add_fastram - add a new fastram
    region
-------------------------------------------------*/

void arm7drc_add_fastram(device_t *device, offs_t start, offs_t end, UINT8 readonly, void *base)
{
	arm_state *arm = get_safe_token(device);
	if (arm->impstate->fastram_select < ARRAY_LENGTH(arm->impstate->fastram))
	{
		arm->impstate->fastram[arm->impstate->fastram_select].start = start;
		arm->impstate->fastram[arm->impstate->fastram_select].end = end;
		arm->impstate->fastram[arm->impstate->fastram_select].readonly = readonly;
		arm->impstate->fastram[arm->impstate->fastram_select].base = base;
		arm->impstate->fastram_select++;
	}
}


/*-------------------------------------------------
    arm7drc_add_hotspot - add a new hotspot
-------------------------------------------------*/

void arm7drc_add_hotspot(device_t *device, offs_t pc, UINT32 opcode, UINT32 cycles)
{
	arm_state *arm = get_safe_token(device);
	if (arm->impstate->hotspot_select < ARRAY_LENGTH(arm->impstate->hotspot))
	{
		arm->impstate->hotspot[arm->impstate->hotspot_select].pc = pc;
		arm->impstate->hotspot[arm->impstate->hotspot_select].opcode = opcode;
		arm->impstate->hotspot[arm->impstate->hotspot_select].cycles = cycles;
		arm->impstate->hotspot_select++;
	}
}



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    code_flush_cache - flush the cache and
    regenerate static code
-------------------------------------------------*/

static void code_flush_cache(arm_state *arm)
{
	int mode;

	/* empty the transient cache contents */
	arm->impstate->drcuml->reset();

	try
	{
		/* generate the entry point and out-of-cycles handlers */
		static_generate_entry_point(arm);
		static_generate_nocode_handler(arm);
		static_generate_out_of_cycles(arm);
		static_generate_tlb_translate(arm);
		static_generate_detect_fault(arm);
		//static_generate_tlb_mismatch(arm);

		/* add subroutines for memory accesses */
		static_generate_memory_accessor(arm, mode, 1, FALSE, FALSE, "read8",       &arm->impstate->read8);
		static_generate_memory_accessor(arm, mode, 1, TRUE,  FALSE, "write8",      &arm->impstate->write8);
		static_generate_memory_accessor(arm, mode, 2, FALSE, FALSE, "read16",      &arm->impstate->read16);
		static_generate_memory_accessor(arm, mode, 2, TRUE,  FALSE, "write16",     &arm->impstate->write16);
		static_generate_memory_accessor(arm, mode, 4, FALSE, FALSE, "read32",      &arm->impstate->read32);
		static_generate_memory_accessor(arm, mode, 4, TRUE,  FALSE, "write32",     &arm->impstate->write32);
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("Unrecoverable error generating static code\n");
	}
}


/*-------------------------------------------------
    code_compile_block - compile a block of the
    given mode at the specified pc
-------------------------------------------------*/

static void code_compile_block(arm_state *arm, UINT8 mode, offs_t pc)
{
	drcuml_state *drcuml = arm->impstate->drcuml;
	compiler_state compiler = { 0 };
	const opcode_desc *seqlast;
	int override = FALSE;

	g_profiler.start(PROFILER_DRC_COMPILE);

	/* get a description of this sequence */
	const opcode_desc *desclist = arm->impstate->drcfe->describe_code(pc);
	if (LOG_UML || LOG_NATIVE)
		log_opcode_desc(drcuml, desclist, 0);

	/* if we get an error back, flush the cache and try again */
	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			/* start the block */
			drcuml_block *block = drcuml->begin_block(4096);

			/* loop until we get through all instruction sequences */
			for (const opcode_desc *seqhead = desclist; seqhead != NULL; seqhead = seqlast->next())
			{
				const opcode_desc *curdesc;
				UINT32 nextpc;

				/* add a code log entry */
				if (LOG_UML)
					block->append_comment("-------------------------");                     // comment

				/* determine the last instruction in this sequence */
				for (seqlast = seqhead; seqlast != NULL; seqlast = seqlast->next())
					if (seqlast->flags & OPFLAG_END_SEQUENCE)
						break;
				assert(seqlast != NULL);

				/* if we don't have a hash for this mode/pc, or if we are overriding all, add one */
				if (override || !drcuml->hash_exists(mode, seqhead->pc))
					UML_HASH(block, mode, seqhead->pc);                                     // hash    mode,pc

				/* if we already have a hash, and this is the first sequence, assume that we */
				/* are recompiling due to being out of sync and allow future overrides */
				else if (seqhead == desclist)
				{
					override = TRUE;
					UML_HASH(block, mode, seqhead->pc);                                     // hash    mode,pc
				}

				/* otherwise, redispatch to that fixed PC and skip the rest of the processing */
				else
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc | 0x80000000
					UML_HASHJMP(block, 0, seqhead->pc, *arm->impstate->nocode);
																							// hashjmp <mode>,seqhead->pc,nocode
					continue;
				}

				/* validate this code block if we're not pointing into ROM */
				if (arm->program->get_write_ptr(seqhead->physpc) != NULL)
					generate_checksum_block(arm, block, &compiler, seqhead, seqlast);

				/* label this instruction, if it may be jumped to locally */
				if (seqhead->flags & OPFLAG_IS_BRANCH_TARGET)
					UML_LABEL(block, seqhead->pc | 0x80000000);                             // label   seqhead->pc | 0x80000000

				/* iterate over instructions in the sequence and compile them */
				for (curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
					generate_sequence_instruction(arm, block, &compiler, curdesc);

				/* if we need to return to the start, do it */
				if (seqlast->flags & OPFLAG_RETURN_TO_START)
					nextpc = pc;

				/* otherwise we just go to the next instruction */
				else
					nextpc = seqlast->pc + (seqlast->skipslots + 1) * 4;

				/* count off cycles and go there */
				generate_update_cycles(arm, block, &compiler, nextpc, TRUE);          // <subtract cycles>

				/* if the last instruction can change modes, use a variable mode; otherwise, assume the same mode */
				/*if (seqlast->flags & OPFLAG_CAN_CHANGE_MODES)
					UML_HASHJMP(block, mem(&arm->impstate->mode), nextpc, *arm->impstate->nocode);
																							// hashjmp <mode>,nextpc,nocode
				else*/ if (seqlast->next() == NULL || seqlast->next()->pc != nextpc)
					UML_HASHJMP(block, arm->impstate->mode, nextpc, *arm->impstate->nocode);
																							// hashjmp <mode>,nextpc,nocode
			}

			/* end the sequence */
			block->end();
			g_profiler.stop();
			succeeded = true;
		}
		catch (drcuml_block::abort_compilation &)
		{
			code_flush_cache(arm);
		}
	}
}


/***************************************************************************
    C FUNCTION CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    cfunc_get_cycles - compute the total number
    of cycles executed so far
-------------------------------------------------*/

static void cfunc_get_cycles(void *param)
{
	arm_state *arm = (arm_state *)param;
	arm->impstate->numcycles = arm->device->total_cycles();
}


/*-------------------------------------------------
    cfunc_unimplemented - handler for
    unimplemented opcdes
-------------------------------------------------*/

static void cfunc_unimplemented(void *param)
{
	arm_state *arm = (arm_state *)param;
	UINT32 opcode = arm->impstate->arg0;
	fatalerror("PC=%08X: Unimplemented op %08X\n", arm->r[eR15], opcode);
}


/***************************************************************************
    STATIC CODEGEN
***************************************************************************/

/*-------------------------------------------------
    static_generate_entry_point - generate a
    static entry point
-------------------------------------------------*/

static void static_generate_entry_point(arm_state *arm)
{
	drcuml_state *drcuml = arm->impstate->drcuml;
	code_label nodabt;
	code_label nofiq;
	code_label noirq;
	code_label irq32;
	code_label nopabd;
	code_label nound;
	code_label swi32;
	code_label irqadjust;
	code_label done;
	int label = 1;
	drcuml_block *block;

	block = drcuml->begin_block(110);

	/* forward references */
	alloc_handle(drcuml, &arm->impstate->exception_norecover[EXCEPTION_INTERRUPT], "interrupt_norecover");
	alloc_handle(drcuml, &arm->impstate->nocode, "nocode");
	alloc_handle(drcuml, &arm->impstate->detect_fault, "detect_fault");
	alloc_handle(drcuml, &arm->impstate->tlb_translate, "tlb_translate");

	alloc_handle(drcuml, &arm->impstate->entry, "entry");
	UML_HANDLE(block, *arm->impstate->entry);                        	// handle  entry

	/* load fast integer registers */
	load_fast_iregs(arm, block);

	UML_CALLH(block, *arm->impstate->check_irq);

	/* generate a hash jump via the current mode and PC */
	UML_HASHJMP(block, 0, mem(&arm->pc), *arm->impstate->nocode);		// hashjmp 0,<pc>,nocode
	block->end();
}


/*-------------------------------------------------
    static_generate_check_irq - generate a handler
    to check IRQs
-------------------------------------------------*/

static void static_generate_check_irq(arm_state *arm)
{
	drcuml_state *drcuml = arm->impstate->drcuml;
	drcuml_block *block;
	int nodabt = 0;
	int nopabt = 0;
	int irqadjust = 0;
	int nofiq = 0;
	int irq32 = 0;
	int swi32 = 0;
	int done = 0;
	int label = 1;

	/* begin generating */
	block = drcuml->begin_block(120);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &arm->impstate->check_irq, "check_irq");
	UML_HANDLE(block, *arm->impstate->check_irq);						// handle  check_irq
	/* Exception priorities:

	    Reset
	    Data abort
	    FIRQ
	    IRQ
	    Prefetch abort
	    Undefined instruction
	    Software Interrupt
	*/

	UML_ADD(block, I0, mem(&R15), 4);									// add		i0, PC, 4  ;insn pc

	// Data Abort
	UML_TEST(block, mem(&arm->pendingAbtD, 1);							// test		pendingAbtD, 1
	UML_JMPc(block, COND_Z, nodabt = label++);							// jmpz		nodabt

	UML_ROLINS(block, mem(&GET_CPSR), eARM7_MODE_ABT, 0, MODE_FLAG)		// rolins	CPSR, eARM7_MODE_ABT, 0, MODE_FLAG
	UML_MOV(block, mem(&GET_REGISTER(arm, 14)), I0);					// mov		LR, i0
	UML_MOV(block, mem(&GET_REGISTER(arm, SPSR)), mem(&GET_CPSR));		// mov		SPSR, CPSR
	UML_OR(block, mem(&GET_CPSR), mem(&GET_CPSR), I_MASK);				// or		CPSR, CPSR, I_MASK
	UML_ROLAND(block, mem(&GET_CPSR), mem(&GET_CPSR), 0, ~T_MASK);		// roland	CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, mem(&R15), 0x00000010);								// mov		PC, 0x10 (Data Abort vector address)
	UML_MOV(block, mem(&arm->pendingAbtD, 0);							// mov		pendingAbtD, 0
	UML_JMP(block, irqadjust = label++);								// jmp		irqadjust

	UML_LABEL(block, nodabt);                                           // nodabt:

	// FIQ
	UML_TEST(block, mem(&arm->pendingFiq, 1);							// test		pendingFiq, 1
	UML_JMPc(block, COND_Z, nofiq = label++);							// jmpz		nofiq
	UML_TEST(block, mem(&GET_CPSR), F_MASK);							// test		CPSR, F_MASK
	UML_JMPc(block, COND_Z, nofiq);										// jmpz		nofiq

	UML_MOV(block, mem(&GET_REGISTER(arm, 14)), I0);					// mov		LR, i0
	UML_MOV(block, mem(&GET_REGISTER(arm, SPSR)), mem(&GET_CPSR));		// mov		SPSR, CPSR
	UML_OR(block, mem(&GET_CPSR), mem(&GET_CPSR), I_MASK | F_MASK);		// or		CPSR, CPSR, I_MASK | F_MASK
	UML_ROLAND(block, mem(&GET_CPSR), mem(&CPSR), 0, ~T_MASK);			// roland	CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, mem(&R15), 0x0000001c);								// mov		PC, 0x1c (FIQ vector address)
	UML_MOV(block, mem(&arm->pendingFiq, 0);							// mov		pendingFiq, 0
	UML_JMP(block, irqadjust);											// jmp		irqadjust

	UML_LABEL(block, nofiq);											// nofiq:

	// IRQ
	UML_TEST(block, mem(&arm->pendingIrq, 1);							// test		pendingIrq, 1
	UML_JMPc(block, COND_Z, noirq = label++);							// jmpz		noirq
	UML_TEST(block, mem(&GET_CPSR), I_MASK);							// test		CPSR, I_MASK
	UML_JMPc(block, COND_Z, noirq);										// jmpz		noirq

	UML_MOV(block, mem(&GET_REGISTER(arm, 14)), I0);					// mov		LR, i0
	UML_TEST(block, mem(&GET_CPSR), SR_MODE32);							// test		CPSR, MODE32
	UML_JMPc(block, COND_NZ, irq32 = label++);							// jmpnz	irq32
	UML_AND(block, I1, I0, 0xf4000000);									// and		i1, i0, 0xf4000000
	UML_OR(block, mem(&R15), I1, 0x0800001a);							// or		PC, i1, 0x0800001a
	UML_AND(block, I1, mem(&GET_CPSR), 0x0fffff3f);						// and		i1, CPSR, 0x0fffff3f
	UML_ROLAND(block, I0, mem(&R15), 32-20, 0x0000000c);				// roland	i0, R15, 32-20, 0x0000000c
	UML_ROLINS(block, I0, mem(&R15), 0, 0xf0000000);					// rolins	i0, R15, 0, 0xf0000000
	UML_OR(block, mem(&GET_CPSR), I0, I1);								// or		CPSR, i0, i1
	UML_JMP(block, irqadjust);											// jmp		irqadjust

	UML_LABEL(block, irq32);											// irq32:
	UML_MOV(block, mem(&GET_REGISTER(arm, SPSR)), mem(&GET_CPSR));		// mov		SPSR, CPSR
	UML_OR(block, mem(&GET_CPSR), mem(&GET_CPSR), I_MASK);				// or		CPSR, CPSR, I_MASK
	UML_ROLAND(block, mem(&GET_CPSR), mem(&CPSR), 0, ~T_MASK);			// roland	CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, mem(&R15), 0x00000018);								// mov		PC, 0x18 (IRQ vector address)

	UML_JMP(block, irqadjust);											// jmp		irqadjust

	UML_LABEL(block, noirq);											// noirq:

	// Prefetch Abort
	UML_TEST(block, mem(&arm->pendingAbtP, 1);							// test		pendingAbtP, 1
	UML_JMPc(block, COND_Z, nopabt = label++);							// jmpz		nopabt

	UML_ROLINS(block, mem(&GET_CPSR), eARM7_MODE_ABT, 0, MODE_FLAG)		// rolins	CPSR, eARM7_MODE_ABT, 0, MODE_FLAG
	UML_MOV(block, mem(&GET_REGISTER(arm, 14)), I0);					// mov		LR, i0
	UML_MOV(block, mem(&GET_REGISTER(arm, SPSR)), mem(&GET_CPSR));		// mov		SPSR, CPSR
	UML_OR(block, mem(&GET_CPSR), mem(&GET_CPSR), I_MASK);				// or		CPSR, CPSR, I_MASK
	UML_ROLAND(block, mem(&GET_CPSR), mem(&CPSR), 0, ~T_MASK);			// roland	CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, mem(&R15), 0x0000000c);								// mov		PC, 0x0c (Prefetch Abort vector address)
	UML_MOV(block, mem(&arm->pendingAbtP, 0);							// mov		pendingAbtP, 0
	UML_JMP(block, irqadjust);											// jmp		irqadjust

	UML_LABEL(block, nopabt);                                           // nopabt:

	// Undefined instruction
	UML_TEST(block, mem(&arm->pendingUnd, 1);							// test		pendingUnd, 1
	UML_JMPc(block, COND_Z, nopabt = label++);							// jmpz		nound

	UML_ROLINS(block, mem(&GET_CPSR), eARM7_MODE_UND, 0, MODE_FLAG)		// rolins	CPSR, eARM7_MODE_UND, 0, MODE_FLAG
	UML_MOV(block, I1, -4);												// mov		i1, -4
	UML_TEST(block, mem(&GET_CPSR), T_MASK);							// test		CPSR, T_MASK
	UML_MOVc(block, COND_NZ, I1, -2);									// movnz	i1, -2
	UML_ADD(block, mem(&GET_REGISTER(arm, 14)), I0, I1);				// add		LR, i0, i1
	UML_MOV(block, mem(&GET_REGISTER(arm, SPSR)), mem(&GET_CPSR));		// mov		SPSR, CPSR
	UML_OR(block, mem(&GET_CPSR), mem(&GET_CPSR), I_MASK);				// or		CPSR, CPSR, I_MASK
	UML_ROLAND(block, mem(&GET_CPSR), mem(&CPSR), 0, ~T_MASK);			// roland	CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, mem(&R15), 0x00000004);								// mov		PC, 0x0c (Undefined Insn vector address)
	UML_MOV(block, mem(&arm->pendingUnd, 0);							// mov		pendingUnd, 0
	UML_JMP(block, irqadjust);											// jmp		irqadjust

	UML_LABEL(block, nopabt);                                           // nopabt:

	// Software Interrupt
	UML_TEST(block, mem(&arm->pendingSwi, 1);							// test		pendingSwi, 1
	UML_JMPc(block, COND_Z, done = label++);							// jmpz		done

	UML_ROLINS(block, mem(&GET_CPSR), eARM7_MODE_SVC, 0, MODE_FLAG)		// rolins	CPSR, eARM7_MODE_SVC, 0, MODE_FLAG
	UML_MOV(block, I1, -4);												// mov		i1, -4
	UML_TEST(block, mem(&GET_CPSR), T_MASK);							// test		CPSR, T_MASK
	UML_MOVc(block, COND_NZ, I1, -2);									// movnz	i1, -2
	UML_ADD(block, mem(&GET_REGISTER(arm, 14)), I0, I1);				// add		LR, i0, i1

	UML_TEST(block, mem(&GET_CPSR), SR_MODE32);							// test		CPSR, MODE32
	UML_JMPc(block, COND_NZ, swi32 = label++);							// jmpnz	swi32
	UML_AND(block, I1, I0, 0xf4000000);									// and		i1, i0, 0xf4000000
	UML_OR(block, mem(&R15), I1, 0x0800001b);							// or		PC, i1, 0x0800001b
	UML_AND(block, I1, mem(&GET_CPSR), 0x0fffff3f);						// and		i1, CPSR, 0x0fffff3f
	UML_ROLAND(block, I0, mem(&R15), 32-20, 0x0000000c);				// roland	i0, R15, 32-20, 0x0000000c
	UML_ROLINS(block, I0, mem(&R15), 0, 0xf0000000);					// rolins	i0, R15, 0, 0xf0000000
	UML_OR(block, mem(&GET_CPSR), I0, I1);								// or		CPSR, i0, i1
	UML_MOV(block, mem(&arm->pendingSwi, 0);							// mov		pendingSwi, 0
	UML_JMP(block, irqadjust);											// jmp		irqadjust

	UML_LABEL(block, swi32);											// irq32:
	UML_MOV(block, mem(&GET_REGISTER(arm, SPSR)), mem(&GET_CPSR));		// mov		SPSR, CPSR
	UML_OR(block, mem(&GET_CPSR), mem(&GET_CPSR), I_MASK);				// or		CPSR, CPSR, I_MASK
	UML_ROLAND(block, mem(&GET_CPSR), mem(&CPSR), 0, ~T_MASK);			// roland	CPSR, CPSR, 0, ~T_MASK
	UML_MOV(block, mem(&R15), 0x00000008);								// mov		PC, 0x08 (SWI vector address)
	UML_MOV(block, mem(&arm->pendingSwi, 0);							// mov		pendingSwi, 0
	UML_JMP(block, irqadjust);											// jmp		irqadjust

	UML_LABEL(block, irqadjust);										// irqadjust:
	UML_MOV(block, I1, 0);												// mov		i1, 0
	UML_TEST(block, mem(&COPRO_CTRL), COPRO_CTRL_MMU_EN | COPRO_CTRL_INTVEC_ADJUST);	// test	COPRO_CTRL, MMU_EN | INTVEC_ADJUST
	UML_MOVc(block, COND_NZ, I1, 0xffff0000);							// movnz	i1, 0xffff0000
	UML_OR(block, mem(&R15), mem(R15), I1);								// or		PC, i1

	UML_LABEL(block, done);												// done:

	block->end();
};

/*-------------------------------------------------
    static_generate_nocode_handler - generate an
    exception handler for "out of code"
-------------------------------------------------*/

static void static_generate_nocode_handler(arm_state *arm)
{
	drcuml_state *drcuml = arm->impstate->drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &arm->impstate->nocode, "nocode");
	UML_HANDLE(block, *arm->impstate->nocode);                                  // handle  nocode
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&R15), I0);                                        		// mov     [pc],i0
	save_fast_iregs(arm, block);
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE

	block->end();
}


/*-------------------------------------------------
    static_generate_out_of_cycles - generate an
    out of cycles exception handler
-------------------------------------------------*/

static void static_generate_out_of_cycles(arm_state *arm)
{
	drcuml_state *drcuml = arm->impstate->drcuml;
	drcuml_block *block;

	/* begin generating */
	block = drcuml->begin_block(10);

	/* generate a hash jump via the current mode and PC */
	alloc_handle(drcuml, &arm->impstate->out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *arm->impstate->out_of_cycles);                      	// handle  out_of_cycles
	UML_GETEXP(block, I0);                                                  // getexp  i0
	UML_MOV(block, mem(&R15), I0);                                    		// mov     <pc>,i0
	save_fast_iregs(arm, block);
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);                                 // exit    EXECUTE_OUT_OF_CYCLES

	block->end();
}


/*------------------------------------------------------------------
    static_generate_tlb_translate
------------------------------------------------------------------*/

static void static_generate_detect_fault(arm_state *arm, code_handle **handleptr)
{
	/* on entry, flags are in I2, vaddr is in I3, desc_lvl1 is in I4, ap is in R5 */
	/* on exit, fault result is in I6 */
	drcuml_state *drcuml = arm->impstate->drcuml;
	drcuml_block *block;
	int donefault = 0;
	int checkuser = 0;
	int label = 1;

	/* begin generating */
	block = drcuml->begin_block(1024);

	/* add a global entry for this */
	alloc_handle(drcuml, &arm->impstate->detect_fault, "detect_fault");
	UML_HANDLE(block, *arm->impstate->detect_fault);               	// handle  	detect_fault

	UML_ROLAND(block, I6, I4, 32-4, 0x0f<<1);						// roland	i6, i4, 32-4, 0xf<<1
	UML_ROLAND(block, I6, mem(&COPRO_DOMAIN_ACCESS_CONTROL), I6, 3);// roland	i6, COPRO_DOMAIN_ACCESS_CONTROL, i6, 3
	// if permission == 3, FAULT_NONE
	UML_CMP(block, I6, 3);											// cmp		i6, 3
	UML_MOVc(block, COND_E, I6, FAULT_NONE);						// move		i6, FAULT_NONE
	UML_JMPc(block, COND_E, donefault = label++);					// jmpe		donefault
	// if permission == 0 || permission == 2, FAULT_DOMAIN
	UML_CMP(block, I6, 1);											// cmp		i6, 1
	UML_MOVc(block, COND_NE, I6, FAULT_DOMAIN);						// movne	i6, FAULT_DOMAIN
	UML_JMPc(block, COND_NE, donefault);							// jmpne	donefault

	// if permission == 1
	UML_CMP(block, I5, 3);											// cmp		i5, 3
	UML_MOVc(block, COND_E, I6, FAULT_NONE);						// move		i6, FAULT_NONE
	UML_JMPc(block, COND_E, donefault);								// jmpe		donefault
	UML_CMP(block, I5, 0);											// cmp		i5, 1
	UML_JMPc(block, COND_NE, checkuser = label++);					// jmpne	checkuser
	UML_ROLAND(block, I6, mem(&COPRO_CTRL),							// roland	i6, COPRO_CTRL, 32 - COPRO_CTRL_SYSTEM_SHIFT,
				32 - COPRO_CTRL_SYSTEM_SHIFT,						// 			COPRO_CTRL_SYSTEM | COPRO_CTRL_ROM
				COPRO_CTRL_SYSTEM | COPRO_CTRL_ROM);
	// if s == 0 && r == 0, FAULT_PERMISSION
	UML_CMP(block, I6, 0);											// cmp		i6, 0
	UML_MOVc(block, COND_E, I6, FAULT_PERMISSION);					// move		i6, FAULT_PERMISSION
	UML_JMPc(block, COND_E, donefault);								// jmpe		donefault
	// if s == 1 && r == 1, FAULT_PERMISSION
	UML_CMP(block, I6, 3);											// cmp		i6, 3
	UML_MOVc(block, COND_E, I6, FAULT_PERMISSION);					// move		i6, FAULT_PERMISSION
	UML_JMPc(block, COND_E, donefault);								// jmpe		donefault
	// if flags & TLB_WRITE, FAULT_PERMISSION
	UML_TEST(block, I2, ARM7_TLB_WRITE);							// test		i2, ARM7_TLB_WRITE
	UML_MOVc(block, COND_NZ, I6, FAULT_PERMISSION);					// move		i6, FAULT_PERMISSION
	UML_JMPc(block, COND_NZ, donefault);							// jmpe		donefault
	// if r == 1 && s == 0, FAULT_NONE
	UML_CMP(block, I6, 2);											// cmp		i6, 2
	UML_MOVc(block, COND_E, I6, FAULT_NONE);						// move		i6, FAULT_NONE
	UML_JMPc(block, COND_E, donefault);								// jmpe		donefault
	UML_AND(block, I6, mem(&GET_CPSR), MODE_FLAG);					// and		i6, GET_CPSR, MODE_FLAG
	UML_CMP(block, I6, eARM7_MODE_USER);							// cmp		i6, eARM7_MODE_USER
	// if r == 0 && s == 1 && usermode, FAULT_PERMISSION
	UML_MOVc(block, COND_E, I6, FAULT_PERMISSION);					// move		i6, FAULT_PERMISSION
	UML_MOVc(block, COND_NE, I6, FAULT_NONE);						// movne	i6, FAULT_NONE
	UML_JMP(block, donefault);										// jmp		donefault

	UML_LABEL(block, checkuser);									// checkuser:
	// if !write, FAULT_NONE
	UML_TEST(block, I2, ARM7_TLB_WRITE);							// test		i2, ARM7_TLB_WRITE
	UML_MOVc(block, COND_Z, I6, FAULT_NONE);						// movz		i6, FAULT_NONE
	UML_JMPc(block, COND_Z, donefault);								// jmp		donefault
	UML_AND(block, I6, mem(&GET_CPSR), MODE_FLAG);					// and		i6, GET_CPSR, MODE_FLAG
	UML_CMP(block, I6, eARM7_MODE_USER);							// cmp		i6, eARM7_MODE_USER
	UML_MOVc(block, COND_E, I6, FAULT_PERMISSION);					// move		i6, FAULT_PERMISSION
	UML_MOVc(block, COND_NE, I6, FAULT_NONE);						// move		i6, FAULT_NONE

	UML_LABEL(block, donefault);									// donefault:
	UML_RET(block);													// ret
}

/*------------------------------------------------------------------
    static_generate_tlb_translate
------------------------------------------------------------------*/

static void static_generate_tlb_translate(arm_state *arm, code_handle **handleptr)
{
	/* on entry, address is in I0 and flags are in I2 */
	/* on exit, translated address is in I0 and success/failure is in I2 */
	/* routine trashes I4-I7 */
	drcuml_state *drcuml = arm->impstate->drcuml;
	drcuml_block *block;
	int nopid = 0;
	int nounmapped = 0;
	int nounmapped2 = 0;
	int nocoarse = 0;
	int nofine = 0;
	int nosection = 0;
	int nolargepage = 0;
	int nosmallpage = 0;
	int notinypage = 0;
	int handlefault = 0;
	int level2 = 0;
	int prefetch = 0;
	int prefetch2 = 0;
	int label = 1;

	/* begin generating */
	block = drcuml->begin_block(170);

	alloc_handle(drcuml, &arm->impstate->tlb_translate, "tlb_translate");
	UML_HANDLE(block, *arm->impstate->tlb_translate);               // handle  	tlb_translate

	// I3: vaddr
	UML_CMP(block, I0, 32 * 1024 * 1024);							// cmp		i0, 32*1024*1024
	UML_JMPc(block, COND_GE, nopid = label++);						// jmpge	nopid
	UML_AND(block, I3, mem(&COPRO_FCSE_PID), 0xfe000000);			// and		i3, COPRO_FCSE_PID, 0xfe000000
	UML_ADD(block, I3, I3, I0);										// add		i3, i3, i0

	// I4: desc_lvl1
	UML_AND(block, I4, mem(&COPRO_TLB_BASE), COPRO_TLB_BASE_MASK);	// and		i4, COPRO_TLB_BASE, COPRO_TLB_BASE_MASK
	UML_ROLINS(block, I4, I3, 32 - COPRO_TLB_VADDR_FLTI_MASK_SHIFT, // rolins	i4, i3, 32-COPRO_TLB_VADDR_FLTI_MASK_SHIFT,
				COPRO_TLB_VADDR_FLTI_MASK);							//			COPRO_TLB_VADDR_FLTI_MASK
	UML_READ(block, I4, I4, SIZE_DWORD, SPACE_PROGRAM);				// read32	i4, i4, PROGRAM

	// I7: desc_lvl1 & 3
	UML_AND(block, I7, I4, 3);										// and		i7, i4, 3

	UML_CMP(block, I7, COPRO_TLB_UNMAPPED);							// cmp		i7, COPRO_TLB_UNMAPPED
	UML_JMPc(block, COND_NE, nounmapped = label++);					// jmpne	nounmapped

	// TLB Unmapped
	UML_TEST(block, I2, ARM7_TLB_ABORT_D);							// test		i2, ARM7_TLB_ABORT_D
	UML_MOVc(block, COND_E, mem(&COPRO_FAULT_STATUS_D), (5 << 0));	// move		COPRO_FAULT_STATUS_D, (5 << 0)
	UML_MOVc(block, COND_E, mem(&COPRO_FAULT_ADDRESS), I3);			// move		COPRO_FAULT_ADDRESS, i3
	UML_MOVc(block, COND_E, mem(&arm->pendingAbtD), 1);				// move		pendingAbtD, 1
	UML_MOVc(block, COND_E, I2, 0);									// move		i2, 0
	UML_RETc(block, COND_E);                                        // rete

	UML_TEST(block, I2, ARM7_TLB_ABORT_P);							// test		i2, ARM7_TLB_ABORT_P
	UML_MOVc(block, COND_E, mem(&arm->pendingAbtP), 1);				// move		pendingAbtP, 1
	UML_MOV(block, I2, 0);											// mov		i2, 0
	UML_RET(block);                                        			// ret

	UML_LABEL(block, nounmapped);									// nounmapped:
	UML_CMP(block, I7, COPRO_TLB_COARSE_TABLE);						// cmp		i7, COPRO_TLB_COARSE_TABLE
	UML_JMPc(block, COND_NE, nocoarse = label++);					// jmpne	nocoarse

	UML_ROLAND(block, I5, I4, 32-4, 0x0f<<1);						// roland	i5, i4, 32-4, 0xf<<1
	UML_ROLAND(block, I5, mem(&COPRO_DOMAIN_ACCESS_CONTROL), I5, 3);// roland	i5, COPRO_DOMAIN_ACCESS_CONTROL, i5, 3
	UML_CMP(block, I5, 1);											// cmp		i5, 1
	UML_JMPc(block, COND_E, level2 = label++);						// jmpe		level2
	UML_CMP(block, I5, 3);											// cmp		i5, 3
	UML_JMPc(block, COND_NE, nofine = label++);						// jmpne	nofine
	UML_LABEL(block, level2);										// level2:

	// I7: desc_level2
	UML_AND(block, I7, I4, COPRO_TLB_CFLD_ADDR_MASK);				// and		i7, i4, COPRO_TLB_CFLD_ADDR_MASK
	UML_ROLINS(block, I7, I3, 32 - COPRO_TLB_VADDR_CSLTI_MASK_SHIFT,// rolins	i7, i3, 32 - COPRO_TLB_VADDR_CSLTI_MASK_SHIFT
				COPRO_TLB_VADDR_CSLTI_MASK);						// 			COPRO_TLB_VADDR_CSLTI_MASK
	UML_READ(block, I7, I7, SIZE_DWORD, SPACE_PROGRAM);				// read32	i7, i7, PROGRAM
	UML_JMP(block, nofine);											// jmp		nofine

	UML_LABEL(block, nocoarse);										// nocoarse:
	UML_CMP(block, I7, COPRO_TLB_SECTION_TABLE);					// cmp		i7, COPRO_TLB_SECTION_TABLE
	UML_JMPc(block, COND_NE, nosection = label++);					// jmpne	nosection

	UML_ROLAND(block, I5, I4, 32-10, 3);							// roland	i7, i4, 32-10, 3
	// result in I6
	UML_CALLH(block, *arm->impstate->detect_fault);					// callh	detect_fault
	UML_CMP(block, I6, FAULT_NONE);									// cmp 		i6, FAULT_NONE
	UML_JMPc(block, COND_NE, handlefault = label++);				// jmpne	handlefault

	// no fault, return translated address
	UML_AND(block, I0, I3, ~COPRO_TLB_SECTION_PAGE_MASK);			// and		i0, i3, ~COPRO_TLB_SECTION_PAGE_MASK
	UML_ROLINS(block, I0, I4, 0, COPRO_TLB_SECTION_PAGE_MASK);		// rolins	i0, i4, COPRO_TLB_SECTION_PAGE_MASK
	UML_MOV(block, I2, 1);											// mov		i2, 1
	UML_RET(block);													// ret

	UML_LABEL(block, handlefault);									// handlefault:
	UML_TEST(block, I2, ARM7_TLB_ABORT_D);							// test		i2, ARM7_TLB_ABORT_D
	UML_JMPc(block, COND_Z, prefetch = label++);					// jmpz		prefetch
	UML_MOV(block, mem(&COPRO_FAULT_ADDRESS), I3);					// mov		COPRO_FAULT_ADDRESS, i3
	UML_MOV(block, mem(&arm->pendingAbtD), 1);						// mov		arm->pendingAbtD, 1
	UML_ROLAND(block, I5, I4, 31, 0xf0);							// roland	i5, i4, 31, 0xf0
	UML_CMP(block, I6, FAULT_DOMAIN);								// cmp		i6, FAULT_DOMAIN
	UML_MOVc(block, COND_E, I6, 9 << 0);							// move		i6, 9 << 0
	UML_MOVc(block, COND_NE, I6, 13 << 0);							// movne	i6, 13 << 0
	UML_OR(block, mem(&COPRO_FAULT_STATUS_D), I5, I6);				// or		COPRO_FAULT_STATUS_D, i5, i6
	UML_MOV(block, I2, 0);											// mov		i2, 0
	UML_RET(block);													// ret

	UML_LABEL(block, prefetch);										// prefetch:
	UML_MOV(block, mem(&arm->pendingAbtP), 1);						// mov		arm->pendingAbtP, 1
	UML_MOV(block, I2, 0);											// mov		i2, 0
	UML_RET(block);													// ret

	UML_LABEL(block, nosection);									// nosection:
	UML_CMP(block, I7, COPRO_TLB_FINE_TABLE);						// cmp		i7, COPRO_TLB_FINE_TABLE
	UML_JMPc(block, COND_NE, nofine);								// jmpne	nofine

	// Not yet implemented
	UML_MOV(block, I2, 1);											// mov		i2, 1
	UML_RET(block);													// ret

	UML_LABEL(block, nofine);										// nofine:

	// I7: desc_lvl2
	UML_AND(block, I6, I7, 3);										// and		i6, i7, 3
	UML_CMP(block, I6, COPRO_TLB_UNMAPPED);							// cmp		i6, COPRO_TLB_UNMAPPED
	UML_JMPc(block, COND_NE, nounmapped2 = label++);				// jmpne	nounmapped2

	UML_TEST(block, I2, ARM7_TLB_ABORT_D);							// test		i2, ARM7_TLB_ABORT_D
	UML_JMPc(block, COND_Z, prefetch2 = label++);					// jmpz		prefetch2
	UML_MOV(block, mem(&COPRO_FAULT_ADDRESS), I3);					// mov		COPRO_FAULT_ADDRESS, i3
	UML_MOV(block, mem(&arm->pendingAbtD), 1);						// mov		arm->pendingAbtD, 1
	UML_ROLAND(block, I5, I4, 31, 0xf0);							// roland	i5, i4, 31, 0xf0
	UML_OR(block, I5, I5, 7 << 0);									// or		i5, i5, 7 << 0
	UML_OR(block, mem(&COPRO_FAULT_STATUS_D), I5, I6);				// or		COPRO_FAULT_STATUS_D, i5, i6
	UML_MOV(block, I2, 0);											// mov		i2, 0
	UML_RET(block);													// ret

	UML_LABEL(block, prefetch2);									// prefetch2:
	UML_MOV(block, mem(&arm->pendingAbtP), 1);						// mov		arm->pendingAbtP, 1
	UML_MOV(block, I2, 0);											// mov		i2, 0
	UML_RET(block);													// ret

	UML_LABEL(block, nounmapped2);									// nounmapped2:
	UML_CMP(block, I6, COPRO_TLB_LARGE_PAGE);						// cmp		i6, COPRO_TLB_LARGE_PAGE
	UML_JMPc(block, COND_NE, nolargepage = label++);				// jmpne	nolargepage

	UML_AND(block, I0, I3, ~COPRO_TLB_LARGE_PAGE_MASK);				// and		i0, i3, ~COPRO_TLB_LARGE_PAGE_MASK
	UML_ROLINS(block, I0, I7, 0, COPRO_TLB_LARGE_PAGE_MASK);		// rolins	i0, i7, 0, COPRO_TLB_LARGE_PAGE_MASK
	UML_MOV(block, I2, 1);											// mov		i2, 1
	UML_RET(block);													// ret

	UML_LABEL(block, nolargepage);									// nolargepage:
	UML_CMP(block, I6, COPRO_TLB_SMALL_PAGE);						// cmp		i6, COPRO_TLB_SMALL_PAGE
	UML_JMPc(block, COND_NE, nosmallpage = label++);				// jmpne	nosmallpage

	UML_ROLAND(block, I5, I3, 32-9, 3<<1);							// roland	i5, i3, 32-9, 3<<1
	UML_ROLAND(block, I6, I7, 32-4, 0xff);							// roland	i6, i7, 32-4, 0xff
	UML_SHR(block, I5, I7, I5);										// shr		i5, i7, i5
	UML_AND(block, I5, I5, 3);										// and		i5, i5, 3
	// result in I6
	UML_CALLH(block, *arm->impstate->detect_fault);					// callh	detect_fault

	UML_CMP(block, I6, FAULT_NONE);									// cmp		i6, FAULT_NONE
	UML_JMPc(block, COND_NE, smallfault = label++);					// jmpne	smallfault
	UML_AND(block, I0, I7, COPRO_TLB_SMALL_PAGE_MASK);				// and		i0, i7, COPRO_TLB_SMALL_PAGE_MASK
	UML_ROLINS(block, I0, I3, 0, ~COPRO_TLB_SMALL_PAGE_MASK);		// rolins	i0, i3, 0, ~COPRO_TLB_SMALL_PAGE_MASK
	UML_MOV(block, I2, 1);											// mov		i2, 1
	UML_RET(block);													// ret

	UML_LABEL(block, smallfault);									// smallfault:
	UML_TEST(block, I2, ARM7_TLB_ABORT_D);							// test		i2, ARM7_TLB_ABORT_D
	UML_JMPc(block, COND_NZ, smallprefetch = label++);				// jmpnz	smallprefetch
	UML_MOV(block, mem(&COPRO_FAULT_ADDRESS), I3);					// mov		COPRO_FAULT_ADDRESS, i3
	UML_MOV(block, mem(&arm->pendingAbtD), 1);						// mov		pendingAbtD, 1
	UML_CMP(block, I6, FAULT_DOMAIN);								// cmp		i6, FAULT_DOMAIN
	UML_MOVc(block, COND_E, I5, 11 << 0);							// move		i5, 11 << 0
	UML_MOVc(block, COND_NE, I5, 15 << 0);							// movne	i5, 15 << 0
	UML_ROLINS(block, I5, I4, 31, 0xf0);							// rolins	i5, i4, 31, 0xf0
	UML_MOV(block, mem(&COPRO_FAULT_STATUS_D), I5);					// mov		COPRO_FAULT_STATUS_D, i5
	UML_MOV(block, I2, 0);											// mov		i2, 0
	UML_RET(block);													// ret

	UML_LABEL(block, smallprefetch);								// smallprefetch:
	UML_MOV(block, mem(&arm->pendingAbtP), 1);						// mov		pendingAbtP, 1
	UML_MOV(block, I2, 0);											// mov		i2, 0
	UML_RET(block);													// ret

	UML_LABEL(block, nosmallpage);									// nosmallpage:
	UML_CMP(block, I6, COPRO_TLB_TINY_PAGE);						// cmp		i6, COPRO_TLB_TINY_PAGE
	UML_JMPc(block, COND_NE, notinypage = label++);					// jmpne	notinypage

	UML_AND(block, I0, I3, ~COPRO_TLB_TINY_PAGE_MASK);				// and		i0, i3, ~COPRO_TLB_TINY_PAGE_MASK
	UML_ROLINS(block, I0, I7, 0, COPRO_TLB_TINY_PAGE_MASK);			// rolins	i0, i7, 0, COPRO_TLB_TINY_PAGE_MASK
	UML_MOV(block, I2, 1);											// mov		i2, 1
	UML_RET(block);													// ret

	UML_LABEL(block, notinypage);									// notinypage:
	UML_MOV(block, I0, I3);											// mov		i0, i3
	UML_RET(block);													// ret

	block->end();
}

/*------------------------------------------------------------------
    static_generate_memory_accessor
------------------------------------------------------------------*/

static void static_generate_memory_accessor(arm_state *arm, int size, bool istlb, bool iswrite, const char *name, code_handle **handleptr)
{
	/* on entry, address is in I0; data for writes is in I1, fetch type in I2 */
	/* on exit, read result is in I0 */
	/* routine trashes I0-I3 */
	drcuml_state *drcuml = arm->impstate->drcuml;
	drcuml_block *block;
	int tlbmiss = 0;
	int label = 1;

	/* begin generating */
	block = drcuml->begin_block(1024);

	/* add a global entry for this */
	alloc_handle(drcuml, handleptr, name);
	UML_HANDLE(block, **handleptr);											// handle  *handleptr

	if (istlb)
	{
		UML_TEST(block, mem(&COPRO_CTRL), COPRO_CTRL_MMU_EN);				// test		COPRO_CTRL, COPRO_CTRL_MMU_EN
		if (iswrite)
		{
			UML_MOVc(block, COND_NZ, I3, ARM7_TLB_WRITE);					// movnz	i3, ARM7_TLB_WRITE
		}
		else
		{
			UML_MOVc(block, COND_NZ, I3, ARM7_TLB_READ);					// movnz	i3, ARM7_TLB_READ
		}
		UML_OR(block, I2, I2, I3);											// or		i2, i2, i3
		UML_CALLHc(block, COND_NZ, *arm->impstate->tlb_translate);			// callhnz	tlb_translate
	}

	/* general case: assume paging and perform a translation */
	if ((arm->device->machine().debug_flags & DEBUG_FLAG_ENABLED) == 0)
	{
		for (int ramnum = 0; ramnum < ARM7_MAX_FASTRAM; ramnum++)
		{
			if (arm->impstate->fastram[ramnum].base != NULL && (!iswrite || !arm->impstate->fastram[ramnum].readonly))
			{
				void *fastbase = (UINT8 *)arm->impstate->fastram[ramnum].base - arm->impstate->fastram[ramnum].start;
				UINT32 skip = label++;
				if (arm->impstate->fastram[ramnum].end != 0xffffffff)
				{
					UML_CMP(block, I0, arm->impstate->fastram[ramnum].end);		// cmp     i0, end
					UML_JMPc(block, COND_A, skip);								// ja      skip
				}
				if (arm->impstate->fastram[ramnum].start != 0x00000000)
				{
					UML_CMP(block, I0, arm->impstate->fastram[ramnum].start);	// cmp     i0, fastram_start
					UML_JMPc(block, COND_B, skip);								// jb      skip
				}

				if (!iswrite)
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, (arm->endianess == ENDIANNESS_BIG) ? BYTE4_XOR_BE(0) : BYTE4_XOR_LE(0));
																						// xor     i0, i0, bytexor
						UML_LOAD(block, I0, fastbase, I0, SIZE_BYTE, SCALE_x1);			// load    i0, fastbase, i0, byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, (arm->endianess == ENDIANNESS_BIG) ? WORD_XOR_BE(0) : WORD_XOR_LE(0));
																						// xor     i0, i0, wordxor
						UML_LOAD(block, I0, fastbase, I0, SIZE_WORD, SCALE_x1);			// load    i0, fastbase, i0, word_x1
					}
					else if (size == 4)
					{
						UML_LOAD(block, I0, fastbase, I0, SIZE_DWORD, SCALE_x1);		// load    i0, fastbase, i0, dword_x1
					}
					UML_RET(block);														// ret
				}
				else
				{
					if (size == 1)
					{
						UML_XOR(block, I0, I0, (arm->endianess == ENDIANNESS_BIG) ? BYTE4_XOR_BE(0) : BYTE4_XOR_LE(0));
																						// xor     i0, i0, bytexor
						UML_STORE(block, fastbase, I0, I1, SIZE_BYTE, SCALE_x1);		// store   fastbase, i0, i1, byte
					}
					else if (size == 2)
					{
						UML_XOR(block, I0, I0, arm->bigendian ? WORD_XOR_BE(0) : WORD_XOR_LE(0));
																						// xor     i0, i0, wordxor
						UML_STORE(block, fastbase, I0, I1, SIZE_WORD, SCALE_x1);		// store   fastbase, i0, i1, word_x1
					}
					else if (size == 4)
					{
						UML_STORE(block, fastbase, I0, I1, SIZE_DWORD, SCALE_x1);		// store   fastbase,i0,i1,dword_x1
					}
					UML_RET(block);                                                     // ret
				}

				UML_LABEL(block, skip);													// skip:
			}
		}
	}

	switch (size)
	{
		case 1:
			if (iswrite)
			{
				UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM);					// write   i0, i1, program_byte
			}
			else
			{
				UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);					// read    i0, i0, program_byte
			}
			break;

		case 2:
			if (iswrite)
			{
				UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM);					// write   i0,i1,program_word
			}
			else
			{
				UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);					// read    i0,i0,program_word
			}
			break;

		case 4:
			if (iswrite)
			{
				UML_WRITE(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);				// write   i0,i1,program_dword
			}
			else
			{
				UML_READ(block, I0, I0, SIZE_DWORD, SPACE_PROGRAM);					// read    i0,i0,program_dword
			}
			break;
	}
	UML_RET(block);                                                                 // ret

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

static void generate_update_cycles(arm_state *arm, drcuml_block *block, compiler_state *compiler, parameter param)
{
	/* check full interrupts if pending */
	if (compiler->checkints)
	{
		code_label skip;

		compiler->checkints = FALSE;
		UML_CALLH(block, *arm->impstate->check_irq);
	}

	/* account for cycles */
	if (compiler->cycles > 0)
	{
		UML_SUB(block, mem(&arm->icount), mem(&arm->icount), MAPVAR_CYCLES);    // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                    // mapvar  cycles,0
		UML_EXHc(block, COND_S, *arm->impstate->out_of_cycles, param);			// exh     out_of_cycles,nextpc
	}
	compiler->cycles = 0;
}


/*-------------------------------------------------
    generate_checksum_block - generate code to
    validate a sequence of opcodes
-------------------------------------------------*/

static void generate_checksum_block(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	const opcode_desc *curdesc;
	if (LOG_UML)
	{
		block->append_comment("[Validation for %08X]", seqhead->pc);                // comment
	}

	/* loose verify or single instruction: just compare and fail */
	if (!(arm->impstate->drcoptions & ARM7DRC_STRICT_VERIFY) || seqhead->next() == NULL)
	{
		if (!(seqhead->flags & OPFLAG_VIRTUAL_NOOP))
		{
			UINT32 sum = seqhead->opptr.l[0];
			void *base = arm->direct->read_decrypted_ptr(seqhead->physpc);
			UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);				// load    i0,base,0,dword

			if (seqhead->delay.first() != NULL && seqhead->physpc != seqhead->delay.first()->physpc)
			{
				base = arm->direct->read_decrypted_ptr(seqhead->delay.first()->physpc);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);			// load    i1,base,dword
				UML_ADD(block, I0, I0, I1);									// add     i0,i0,i1

				sum += seqhead->delay.first()->opptr.l[0];
			}

			UML_CMP(block, I0, sum);                                    	// cmp     i0,opptr[0]
			UML_EXHc(block, COND_NE, *arm->impstate->nocode, epc(seqhead));	// exne    nocode,seqhead->pc
		}
	}

	/* full verification; sum up everything */
	else
	{
		UINT32 sum = 0;
		void *base = arm->direct->read_decrypted_ptr(seqhead->physpc);
		UML_LOAD(block, I0, base, 0, SIZE_DWORD, SCALE_x4);					// load    i0,base,0,dword
		sum += seqhead->opptr.l[0];
		for (curdesc = seqhead->next(); curdesc != seqlast->next(); curdesc = curdesc->next())
			if (!(curdesc->flags & OPFLAG_VIRTUAL_NOOP))
			{
				base = arm->direct->read_decrypted_ptr(curdesc->physpc);
				UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);			// load    i1,base,dword
				UML_ADD(block, I0, I0, I1);									// add     i0,i0,i1
				sum += curdesc->opptr.l[0];

				if (curdesc->delay.first() != NULL && (curdesc == seqlast || (curdesc->next() != NULL && curdesc->next()->physpc != curdesc->delay.first()->physpc)))
				{
					base = arm->direct->read_decrypted_ptr(curdesc->delay.first()->physpc);
					UML_LOAD(block, I1, base, 0, SIZE_DWORD, SCALE_x4);		// load    i1,base,dword
					UML_ADD(block, I0, I0, I1);								// add     i0,i0,i1
					sum += curdesc->delay.first()->opptr.l[0];
				}
			}
		UML_CMP(block, I0, sum);											// cmp     i0,sum
		UML_EXHc(block, COND_NE, *arm->impstate->nocode, epc(seqhead));		// exne    nocode,seqhead->pc
	}
}


/*-------------------------------------------------
    generate_sequence_instruction - generate code
    for a single instruction in a sequence
-------------------------------------------------*/

static void generate_sequence_instruction(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	offs_t expc;
	int hotnum;

	/* add an entry for the log */
	if (LOG_UML && !(desc->flags & OPFLAG_VIRTUAL_NOOP))
		log_add_disasm_comment(arm, block, desc->pc, desc->opptr.l[0]);

	/* set the PC map variable */
	//expc = (desc->flags & OPFLAG_IN_DELAY_SLOT) ? desc->pc - 3 : desc->pc;
	UML_MAPVAR(block, MAPVAR_PC, desc->pc);                                 // mapvar  PC,pc

	/* accumulate total cycles */
	compiler->cycles += desc->cycles;

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                   	// mapvar  CYCLES,compiler->cycles

	/* is this a hotspot? */
	for (hotnum = 0; hotnum < MIPS3_MAX_HOTSPOTS; hotnum++)
	{
		if (arm->impstate->hotspot[hotnum].pc != 0 && desc->pc == arm->impstate->hotspot[hotnum].pc && desc->opptr.l[0] == arm->impstate->hotspot[hotnum].opcode)
		{
			compiler->cycles += arm->impstate->hotspot[hotnum].cycles;
			break;
		}
	}

	/* update the icount map variable */
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);						// mapvar  CYCLES,compiler->cycles

	/* if we are debugging, call the debugger */
	if ((arm->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		UML_MOV(block, mem(&R15), desc->pc);								// mov     [pc],desc->pc
		save_fast_iregs(arm, block);
		UML_DEBUG(block, desc->pc);											// debug   desc->pc
	}

	/* if we hit an unmapped address, fatal error */
	if (desc->flags & OPFLAG_COMPILER_UNMAPPED)
	{
		UML_MOV(block, mem(&R15), desc->pc);								// mov     R15,desc->pc
		save_fast_iregs(arm, block);
		UML_EXIT(block, EXECUTE_UNMAPPED_CODE);								// exit    EXECUTE_UNMAPPED_CODE
	}

	/* otherwise, unless this is a virtual no-op, it's a regular instruction */
	else if (!(desc->flags & OPFLAG_VIRTUAL_NOOP))
	{
		/* compile the instruction */
		if (!generate_opcode(arm, block, compiler, desc))
		{
			UML_MOV(block, mem(&R15), desc->pc);							// mov     R15,desc->pc
			UML_MOV(block, mem(&arm->impstate->arg0), desc->opptr.l[0]);	// mov     [arg0],desc->opptr.l
			UML_CALLC(block, cfunc_unimplemented, arm);                 	// callc   cfunc_unimplemented
		}
	}
}


/*------------------------------------------------------------------
    generate_delay_slot_and_branch
------------------------------------------------------------------*/

static void generate_delay_slot_and_branch(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT8 linkreg)
{
	compiler_state compiler_temp = *compiler;
	UINT32 op = desc->opptr.l[0];

	/* update the cycles and jump through the hash table to the target */
	if (desc->targetpc != BRANCH_TARGET_DYNAMIC)
	{
		generate_update_cycles(arm, block, &compiler_temp, desc->targetpc, TRUE);	// <subtract cycles>
		UML_HASHJMP(block, 0, desc->targetpc, *arm->impstate->nocode);
																					// hashjmp 0,desc->targetpc,nocode
	}
	else
	{
		generate_update_cycles(arm, block, &compiler_temp, mem(&arm->impstate->jmpdest), TRUE);
																					// <subtract cycles>
		UML_HASHJMP(block, 0, mem(&arm->impstate->jmpdest), *arm->impstate->nocode);// hashjmp 0,<rsreg>,nocode
	}

	/* update the label */
	compiler->labelnum = compiler_temp.labelnum;

	/* reset the mapvar to the current cycles and account for skipped slots */
	compiler->cycles += desc->skipslots;
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler->cycles);                             // mapvar  CYCLES,compiler->cycles
}

typedef const bool (*drcarm7ops_ophandler)(arm_state*, drcuml_block*, compiler_state*, const opcode_desc*, UINT32);

static drcarm7ops_ophandler drcops_handler[0x10] =
{
	drcarm7ops_0123, drcarm7ops_0123, drcarm7ops_0123, drcarm7ops_0123,
	drcarm7ops_4567, drcarm7ops_4567, drcarm7ops_4567, drcarm7ops_4567,
	drcarm7ops_89,   drcarm7ops_89,   drcarm7ops_ab,   drcarm7ops_ab,
	drcarm7ops_cd,   drcarm7ops_cd,   drcarm7ops_e,    drcarm7ops_f,
};

INLINE void saturate_qbit_overflow(arm_state *arm, drcuml_block *block)
{
	UML_MOV(block, I1, 0);
	UML_DCMP(block, I0, 0x000000007fffffffL);
	UML_MOVc(block, COND_G, I1, Q_MASK);
	UML_MOVc(block, COND_G, I0, 0x7fffffff);
	UML_DCMP(block, I0, 0xffffffff80000000L);
	UML_MOVc(block, COND_L, I1, Q_MASK);
	UML_MOVc(block, COND_L, I0, 0x80000000);
	UML_OR(block, DRC_CPSR, DRC_CPSR, I1);
}

const bool drcarm7ops_0123(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
	code_label done;
	/* Branch and Exchange (BX) */
	if ((insn & 0x0ffffff0) == 0x012fff10)     // bits 27-4 == 000100101111111111110001
	{
		UML_MOV(block, DRC_PC, DRC_REG(insn & 0x0f));
		UML_TEST(block, DRC_PC, 1);
		UML_JMPc(block, COND_Z, done = compiler->labelnum++);
		UML_OR(block, DRC_CPSR, DRC_CPSR, T_MASK);
		UML_AND(block, DRC_PC, DRC_PC, ~1);
	}
	else if ((insn & 0x0ff000f0) == 0x01600010) // CLZ - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rd = (insn>>12)&0xf;

		UML_LZCNT(block, DRC_REG(rd), DRC_REG(rm));
		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff000f0) == 0x01000050) // QADD - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>16)&0xf;
		UINT32 rd = (insn>>12)&0xf;
		UML_DSEXT(block, I0, DRC_REG(rm), SIZE_DWORD);
		UML_DSEXT(block, I1, DRC_REG(rn), SIZE_DWORD);
		UML_DADD(block, I0, I0, I1);
		saturate_qbit_overflow(arm, block);
		UML_MOV(block, DRC_REG(rd), I0);
		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff000f0) == 0x01400050) // QDADD - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>16)&0xf;
		UINT32 rd = (insn>>12)&0xf;

		UML_DSEXT(block, I1, DRC_REG(rn), SIZE_DWORD);
		UML_DADD(block, I0, I1, I1);
		saturate_qbit_overflow(arm, block);

		UML_DSEXT(block, I0, DRC_REG(rm), SIZE_DWORD);
		UML_DSEXT(block, I1, DRC_REG(rn), SIZE_DWORD);
		UML_DADD(block, I1, I1, I1);
		UML_DADD(block, I0, I0, I1);
		saturate_qbit_overflow(arm, block);
		UML_MOV(block, DRC_REG(rd), I0);

		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff000f0) == 0x01200050) // QSUB - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>16)&0xf;
		UINT32 rd = (insn>>12)&0xf;

		UML_DSEXT(block, I0, DRC_REG(rm), SIZE_DWORD);
		UML_DSEXT(block, I1, DRC_REG(rn), SIZE_DWORD);
		UML_DSUB(block, I0, I0, I1);
		saturate_qbit_overflow(arm, block);
		UML_MOV(block, DRC_REG(rd), I0);
		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff000f0) == 0x01600050) // QDSUB - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>16)&0xf;
		UINT32 rd = (insn>>12)&0xf;

		UML_DSEXT(block, I1, DRC_REG(rn), SIZE_DWORD);
		UML_DADD(block, I0, I1, I1);
		saturate_qbit_overflow(arm, block);

		UML_DSEXT(block, I0, DRC_REG(rm), SIZE_DWORD);
		UML_DSEXT(block, I1, DRC_REG(rn), SIZE_DWORD);
		UML_DADD(block, I1, I1, I1);
		UML_DSUB(block, I0, I0, I1);
		saturate_qbit_overflow(arm, block);
		UML_MOV(block, DRC_REG(rd), I0);

		UML_ADD(block, DRC_PC, DRC_PC, 4);
	}
	else if ((insn & 0x0ff00090) == 0x01000080) // SMLAxy - v5
	{
		UINT32 rm = insn&0xf;
		UINT32 rn = (insn>>16)&0xf;
		UINT32 rd = (insn>>12)&0xf;
		UINT32 rr = (insn>>8)&0xf;

		INT32 src1 = GET_REGISTER(arm, insn&0xf);
		INT32 src2 = GET_REGISTER(arm, (insn>>8)&0xf);
		INT32 res1;

		UML_MOV(block, I0, DRC_REG(rm));
		UML_MOV(block, I1, DRC_REG(rr));

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
		{
			UML_SHR(block, I0, I0, 16);
		}
		else
		{
			UML_SEXT(block, I1, I1, SIZE_WORD);
			src1 &= 0xffff;
			if (src1 & 0x8000)
			{
				src1 |= 0xffff;
			}
		}

		if (insn & 0x40)
		{
			src2 >>= 16;
		}
		else
		{
			src2 &= 0xffff;
			if (src2 & 0x8000)
			{
				src2 |= 0xffff;
			}
		}

		// do the signed multiply
		res1 = src1 * src2;
		// and the accumulate.  NOTE: only the accumulate can cause an overflow, which is why we do it this way.
		saturate_qbit_overflow(arm, (INT64)res1 + (INT64)GET_REGISTER(arm, (insn>>12)&0xf));

		SET_REGISTER(arm, (insn>>16)&0xf, res1 + GET_REGISTER(arm, (insn>>12)&0xf));
		R15 += 4;
	}
	else if ((insn & 0x0ff00090) == 0x01400080) // SMLALxy - v5
	{
		INT32 src1 = GET_REGISTER(arm, insn&0xf);
		INT32 src2 = GET_REGISTER(arm, (insn>>8)&0xf);
		INT64 dst;

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
		{
			src1 >>= 16;
		}
		else
		{
			src1 &= 0xffff;
			if (src1 & 0x8000)
			{
				src1 |= 0xffff;
			}
		}

		if (insn & 0x40)
		{
			src2 >>= 16;
		}
		else
		{
			src2 &= 0xffff;
			if (src2 & 0x8000)
			{
				src2 |= 0xffff;
			}
		}

		dst = (INT64)GET_REGISTER(arm, (insn>>12)&0xf);
		dst |= (INT64)GET_REGISTER(arm, (insn>>16)&0xf)<<32;

		// do the multiply and accumulate
		dst += (INT64)src1 * (INT64)src2;

		// write back the result
		SET_REGISTER(cpustart, (insn>>12)&0xf, (UINT32)(dst&0xffffffff));
		SET_REGISTER(cpustart, (insn>>16)&0xf, (UINT32)(dst>>32));
	}
	else if ((insn & 0x0ff00090) == 0x01600080) // SMULxy - v5
	{
		INT32 src1 = GET_REGISTER(arm, insn&0xf);
		INT32 src2 = GET_REGISTER(arm, (insn>>8)&0xf);
		INT32 res;

		// select top and bottom halves of src1/src2 and sign extend if necessary
		if (insn & 0x20)
		{
			src1 >>= 16;
		}
		else
		{
			src1 &= 0xffff;
			if (src1 & 0x8000)
			{
				src1 |= 0xffff;
			}
		}

		if (insn & 0x40)
		{
			src2 >>= 16;
		}
		else
		{
			src2 &= 0xffff;
			if (src2 & 0x8000)
			{
				src2 |= 0xffff;
			}
		}

		res = src1 * src2;
		SET_REGISTER(cpustart, (insn>>16)&0xf, res);
	}
	else if ((insn & 0x0ff000b0) == 0x012000a0) // SMULWy - v5
	{
		INT32 src1 = GET_REGISTER(arm, insn&0xf);
		INT32 src2 = GET_REGISTER(arm, (insn>>8)&0xf);
		INT64 res;

		if (insn & 0x40)
		{
			src2 >>= 16;
		}
		else
		{
			src2 &= 0xffff;
			if (src2 & 0x8000)
			{
				src2 |= 0xffff;
			}
		}

		res = (INT64)src1 * (INT64)src2;
		res >>= 16;
		SET_REGISTER(cpustart, (insn>>16)&0xf, (UINT32)res);
	}
	else if ((insn & 0x0ff000b0) == 0x01200080) // SMLAWy - v5
	{
		INT32 src1 = GET_REGISTER(arm, insn&0xf);
		INT32 src2 = GET_REGISTER(arm, (insn>>8)&0xf);
		INT32 src3 = GET_REGISTER(arm, (insn>>12)&0xf);
		INT64 res;

		if (insn & 0x40)
		{
			src2 >>= 16;
		}
		else
		{
			src2 &= 0xffff;
			if (src2 & 0x8000)
			{
				src2 |= 0xffff;
			}
		}

		res = (INT64)src1 * (INT64)src2;
		res >>= 16;

		// check for overflow and set the Q bit
		saturate_qbit_overflow(arm, (INT64)src3 + res);

		// do the real accumulate
		src3 += (INT32)res;

		// write the result back
		SET_REGISTER(cpustart, (insn>>16)&0xf, (UINT32)res);
	}
	else
	/* Multiply OR Swap OR Half Word Data Transfer */
	if ((insn & 0x0e000000) == 0 && (insn & 0x80) && (insn & 0x10))  // bits 27-25=000 bit 7=1 bit 4=1
	{
		/* Half Word Data Transfer */
		if (insn & 0x60)         // bits = 6-5 != 00
		{
			HandleHalfWordDT(arm, insn);
		}
		else
		/* Swap */
		if (insn & 0x01000000)   // bit 24 = 1
		{
			HandleSwap(arm, insn);
		}
		/* Multiply Or Multiply Long */
		else
		{
			/* multiply long */
			if (insn & 0x800000) // Bit 23 = 1 for Multiply Long
			{
				/* Signed? */
				if (insn & 0x00400000)
					HandleSMulLong(arm, insn);
				else
					HandleUMulLong(arm, insn);
			}
			/* multiply */
			else
			{
				HandleMul(arm, insn);
			}
			R15 += 4;
		}
	}
	/* Data Processing OR PSR Transfer */
	else if ((insn & 0x0c000000) == 0)   // bits 27-26 == 00 - This check can only exist properly after Multiplication check above
	{
		/* PSR Transfer (MRS & MSR) */
		if (((insn & 0x00100000) == 0) && ((insn & 0x01800000) == 0x01000000)) // S bit must be clear, and bit 24,23 = 10
		{
			HandlePSRTransfer(arm, insn);
			ARM7_ICOUNT += 2;       // PSR only takes 1 - S Cycle, so we add + 2, since at end, we -3..
			R15 += 4;
		}
		/* Data Processing */
		else
		{
			HandleALU(arm, insn);
		}
	}

	UML_LABEL(block, done);
	return true;
}

const bool drcarm7ops_4567(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
}

const bool drcarm7ops_89(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
}

const bool drcarm7ops_ab(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
}

const bool drcarm7ops_cd(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
}

const bool drcarm7ops_e(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
}

const bool drcarm7ops_f(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, UINT32 op)
{
}

/*-------------------------------------------------
    generate_opcode - generate code for a specific
    opcode
-------------------------------------------------*/

static int generate_opcode(arm_state *arm, drcuml_block *block, compiler_state *compiler, const opcode_desc *desc)
{
	//int in_delay_slot = ((desc->flags & OPFLAG_IN_DELAY_SLOT) != 0);
	UINT32 op = desc->opptr.l[0];
	UINT8 opswitch = op >> 26;
	code_label skip;
	code_label contdecode;
	code_label unexecuted;

	if (T_IS_SET(GET_CPSR))
	{
		// "In Thumb state, bit [0] is undefined and must be ignored. Bits [31:1] contain the PC."
		UML_AND(block, I0, DRC_PC, ~1);
	}
	else
	{
		UML_AND(block, I0, DRC_PC, ~3);
	}

	UML_TEST(block, mem(&COPRO_CTRL), COPRO_CTRL_MMU_EN);						// test		COPRO_CTRL, COPRO_CTRL_MMU_EN
	UML_MOVc(block, COND_NZ, I2, ARM7_TLB_ABORT_P | ARM7_TLB_READ);				// movnz	i0, ARM7_TLB_ABORT_P | ARM7_TLB_READ
	UML_CALLHc(block, COND_NZ, *arm->impstate->tlb_translate);					// callhnz	tlb_translate);

	if (T_IS_SET(GET_CPSR))
	{
		UML_CALLH(block, *arm->impstate->drcthumb[(op & 0xffc0) >> 6);			// callh	drcthumb[op]
		return TRUE;
	}

	switch (op >> INSN_COND_SHIFT)
	{
		case COND_EQ:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_NE:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, COND_NZ, unexecuted = compiler->labelnum++);
			break;
		case COND_CS:
			UML_TEST(block, DRC_CPSR, C_MASK);
			UML_JMPc(block, COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_CC:
			UML_TEST(block, DRC_CPSR, C_MASK);
			UML_JMPc(block, COND_NZ, unexecuted = compiler->labelnum++);
			break;
		case COND_MI:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_JMPc(block, COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_PL:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_JMPc(block, COND_NZ, unexecuted = compiler->labelnum++);
			break;
		case COND_VS:
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_JMPc(block, COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_VC:
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_JMPc(block, COND_NZ, unexecuted = compiler->labelnum++);
			break;
		case COND_HI:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, COND_NZ, unexecuted = compiler->labelnum++);
			UML_TEST(block, DRC_CPSR, C_MASK);
			UML_JMPc(block, COND_Z, unexecuted = compiler->labelnum++);
			break;
		case COND_LS:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, COND_NZ, contdecode = compiler->labelnum++);
			UML_TEST(block, DRC_CPSR, C_MASK);
			UML_JMPc(block, COND_Z, contdecode);
			UML_JMP(block, unexecuted);
			break;
		case COND_GE:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_MOVc(block, COND_Z, I0, 0);
			UML_MOVc(block, COND_NZ, I0, 1);
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_MOVc(block, COND_Z, I1, 0);
			UML_MOVc(block, COND_NZ, I1, 1);
			UML_CMP(block, I0, I1);
			UML_JMPc(block, COND_NE, unexecuted);
			break;
		case COND_LT:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_MOVc(block, COND_Z, I0, 0);
			UML_MOVc(block, COND_NZ, I0, 1);
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_MOVc(block, COND_Z, I1, 0);
			UML_MOVc(block, COND_NZ, I1, 1);
			UML_CMP(block, I0, I1);
			UML_JMPc(block, COND_E, unexecuted);
			break;
		case COND_GT:
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, COND_NZ, unexecuted);
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_MOVc(block, COND_Z, I0, 0);
			UML_MOVc(block, COND_NZ, I0, 1);
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_MOVc(block, COND_Z, I1, 0);
			UML_MOVc(block, COND_NZ, I1, 1);
			UML_CMP(block, I0, I1);
			UML_JMPc(block, COND_NE, unexecuted);
			break;
		case COND_LE:
			UML_TEST(block, DRC_CPSR, N_MASK);
			UML_MOVc(block, COND_Z, I0, 0);
			UML_MOVc(block, COND_NZ, I0, 1);
			UML_TEST(block, DRC_CPSR, V_MASK);
			UML_MOVc(block, COND_Z, I1, 0);
			UML_MOVc(block, COND_NZ, I1, 1);
			UML_CMP(block, I0, I1);
			UML_JMPc(block, COND_NE, contdecode);
			UML_TEST(block, DRC_CPSR, Z_MASK);
			UML_JMPc(block, COND_Z, unexecuted);
			break;
		case COND_NV:
			UML_JMP(block, unexecuted);
			break;
	}

	UML_LABEL(block, contdecode);

	drcops_handler[(op & 0xF000000) >> 24](arm, block, compiler, desc);

	UML_LABEL(block, unexecuted);
	UML_ADD(block, DRC_PC, DRC_PC, 4);
	UML_ADD(block, MAPVAR_CYCLES, MAPVAR_CYCLES, 2);								// add		cycles, cycles, 2

	UML_LABEL(block, skip);

	switch (opswitch)
	{
		/* ----- sub-groups ----- */

		case 0x00:  /* SPECIAL - MIPS I */
			UML_DCMP(block, R64(RSREG), R64(RTREG));                                // dcmp    <rsreg>,<rtreg>
			UML_JMPc(block, COND_NE, skip = compiler->labelnum++);                  // jmp     skip,NE
			generate_delay_slot_and_branch(mips3, block, compiler, desc, 0);        // <next instruction + hashjmp>
			UML_LABEL(block, skip);                                             // skip:
			return TRUE;

DEFINE_LEGACY_CPU_DEVICE(ARM7, arm7);
DEFINE_LEGACY_CPU_DEVICE(ARM7_BE, arm7_be);
DEFINE_LEGACY_CPU_DEVICE(ARM7500, arm7500);
DEFINE_LEGACY_CPU_DEVICE(ARM9, arm9);
DEFINE_LEGACY_CPU_DEVICE(ARM920T, arm920t);
DEFINE_LEGACY_CPU_DEVICE(PXA255, pxa255);
DEFINE_LEGACY_CPU_DEVICE(SA1110, sa1110);

#endif  // ARM7_USE_DRC
