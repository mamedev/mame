/***************************************************************************

    cpuintrf.h

    Core CPU interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CPUINTRF_H__
#define __CPUINTRF_H__

#include "cpuint.h"
#include "cpuexec.h"
#include "watchdog.h"
#include "mame.h"
#include "state.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_CPU 8


/* Interrupt line constants */
enum
{
	/* line states */
	CLEAR_LINE = 0,				/* clear (a fired, held or pulsed) line */
	ASSERT_LINE,				/* assert an interrupt immediately */
	HOLD_LINE,					/* hold interrupt line until acknowledged */
	PULSE_LINE,					/* pulse interrupt line for one instruction */

	/* internal flags (not for use by drivers!) */
	INTERNAL_CLEAR_LINE = 100 + CLEAR_LINE,
	INTERNAL_ASSERT_LINE = 100 + ASSERT_LINE,

	/* input lines */
	MAX_INPUT_LINES = 32+3,
	INPUT_LINE_IRQ0 = 0,
	INPUT_LINE_IRQ1 = 1,
	INPUT_LINE_IRQ2 = 2,
	INPUT_LINE_IRQ3 = 3,
	INPUT_LINE_IRQ4 = 4,
	INPUT_LINE_IRQ5 = 5,
	INPUT_LINE_IRQ6 = 6,
	INPUT_LINE_IRQ7 = 7,
	INPUT_LINE_IRQ8 = 8,
	INPUT_LINE_IRQ9 = 9,
	INPUT_LINE_NMI = MAX_INPUT_LINES - 3,

	/* special input lines that are implemented in the core */
	INPUT_LINE_RESET = MAX_INPUT_LINES - 2,
	INPUT_LINE_HALT = MAX_INPUT_LINES - 1,

	/* output lines */
	MAX_OUTPUT_LINES = 32
};


/* Maximum number of registers of any CPU */
enum
{
	MAX_REGS = 256
};


/* CPU information constants */
enum
{
	/* --- the following bits of info are returned as 64-bit signed integers --- */
	CPUINFO_INT_FIRST = 0x00000,

	CPUINFO_INT_CONTEXT_SIZE = CPUINFO_INT_FIRST,		/* R/O: size of CPU context in bytes */
	CPUINFO_INT_INPUT_LINES,							/* R/O: number of input lines */
	CPUINFO_INT_OUTPUT_LINES,							/* R/O: number of output lines */
	CPUINFO_INT_DEFAULT_IRQ_VECTOR,						/* R/O: default IRQ vector */
	CPUINFO_INT_ENDIANNESS,								/* R/O: either CPU_IS_BE or CPU_IS_LE */
	CPUINFO_INT_CLOCK_MULTIPLIER,						/* R/O: internal clock multiplier */
	CPUINFO_INT_CLOCK_DIVIDER,							/* R/O: internal clock divider */
	CPUINFO_INT_MIN_INSTRUCTION_BYTES,					/* R/O: minimum bytes per instruction */
	CPUINFO_INT_MAX_INSTRUCTION_BYTES,					/* R/O: maximum bytes per instruction */
	CPUINFO_INT_MIN_CYCLES,								/* R/O: minimum cycles for a single instruction */
	CPUINFO_INT_MAX_CYCLES,								/* R/O: maximum cycles for a single instruction */

	CPUINFO_INT_DATABUS_WIDTH,							/* R/O: data bus size for each address space (8,16,32,64) */
	CPUINFO_INT_DATABUS_WIDTH_LAST = CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACES - 1,
	CPUINFO_INT_ADDRBUS_WIDTH,							/* R/O: address bus size for each address space (12-32) */
	CPUINFO_INT_ADDRBUS_WIDTH_LAST = CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACES - 1,
	CPUINFO_INT_ADDRBUS_SHIFT,							/* R/O: shift applied to addresses each address space (+3 means >>3, -1 means <<1) */
	CPUINFO_INT_ADDRBUS_SHIFT_LAST = CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACES - 1,
	CPUINFO_INT_LOGADDR_WIDTH,							/* R/O: address bus size for logical accesses in each space (0=same as physical) */
	CPUINFO_INT_LOGADDR_WIDTH_LAST = CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACES - 1,
	CPUINFO_INT_PAGE_SHIFT,								/* R/O: size of a page log 2 (i.e., 12=4096), or 0 if paging not supported */
	CPUINFO_INT_PAGE_SHIFT_LAST = CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACES - 1,

	CPUINFO_INT_SP,										/* R/W: the current stack pointer value */
	CPUINFO_INT_PC,										/* R/W: the current PC value */
	CPUINFO_INT_PREVIOUSPC,								/* R/W: the previous PC value */
	CPUINFO_INT_INPUT_STATE,							/* R/W: states for each input line */
	CPUINFO_INT_INPUT_STATE_LAST = CPUINFO_INT_INPUT_STATE + MAX_INPUT_LINES - 1,
	CPUINFO_INT_OUTPUT_STATE,							/* R/W: states for each output line */
	CPUINFO_INT_OUTPUT_STATE_LAST = CPUINFO_INT_OUTPUT_STATE + MAX_OUTPUT_LINES - 1,
	CPUINFO_INT_REGISTER,								/* R/W: values of up to MAX_REGs registers */
	CPUINFO_INT_REGISTER_LAST = CPUINFO_INT_REGISTER + MAX_REGS - 1,

	CPUINFO_INT_CPU_SPECIFIC = 0x08000,					/* R/W: CPU-specific values start here */

	/* --- the following bits of info are returned as pointers to data or functions --- */
	CPUINFO_PTR_FIRST = 0x10000,

	CPUINFO_PTR_SET_INFO = CPUINFO_PTR_FIRST,			/* R/O: void (*set_info)(UINT32 state, INT64 data, void *ptr) */
	CPUINFO_PTR_GET_CONTEXT,							/* R/O: void (*get_context)(void *buffer) */
	CPUINFO_PTR_SET_CONTEXT,							/* R/O: void (*set_context)(void *buffer) */
	CPUINFO_PTR_INIT,									/* R/O: void (*init)(int index, int clock, const void *config, int (*irqcallback)(int)) */
	CPUINFO_PTR_RESET,									/* R/O: void (*reset)(void) */
	CPUINFO_PTR_EXIT,									/* R/O: void (*exit)(void) */
	CPUINFO_PTR_EXECUTE,								/* R/O: int (*execute)(int cycles) */
	CPUINFO_PTR_BURN,									/* R/O: void (*burn)(int cycles) */
	CPUINFO_PTR_DISASSEMBLE,							/* R/O: offs_t (*disassemble)(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram) */
	CPUINFO_PTR_TRANSLATE,								/* R/O: int (*translate)(int space, offs_t *address) */
	CPUINFO_PTR_READ,									/* R/O: int (*read)(int space, UINT32 offset, int size, UINT64 *value) */
	CPUINFO_PTR_WRITE,									/* R/O: int (*write)(int space, UINT32 offset, int size, UINT64 value) */
	CPUINFO_PTR_READOP,									/* R/O: int (*readop)(UINT32 offset, int size, UINT64 *value) */
	CPUINFO_PTR_DEBUG_SETUP_COMMANDS,					/* R/O: void (*setup_commands)(void) */
	CPUINFO_PTR_INSTRUCTION_COUNTER,					/* R/O: int *icount */
	CPUINFO_PTR_INTERNAL_MEMORY_MAP,					/* R/O: const addrmap_token *map */
	CPUINFO_PTR_INTERNAL_MEMORY_MAP_LAST = CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACES - 1,
	CPUINFO_PTR_DEBUG_REGISTER_LIST,					/* R/O: int *list: list of registers for the debugger */

	CPUINFO_PTR_CPU_SPECIFIC = 0x18000,					/* R/W: CPU-specific values start here */

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	CPUINFO_STR_FIRST = 0x20000,

	CPUINFO_STR_NAME = CPUINFO_STR_FIRST,				/* R/O: name of the CPU */
	CPUINFO_STR_CORE_FAMILY,							/* R/O: family of the CPU */
	CPUINFO_STR_CORE_VERSION,							/* R/O: version of the CPU core */
	CPUINFO_STR_CORE_FILE,								/* R/O: file containing the CPU core */
	CPUINFO_STR_CORE_CREDITS,							/* R/O: credits for the CPU core */
	CPUINFO_STR_FLAGS,									/* R/O: string representation of the main flags value */
	CPUINFO_STR_REGISTER,								/* R/O: string representation of up to MAX_REGs registers */
	CPUINFO_STR_REGISTER_LAST = CPUINFO_STR_REGISTER + MAX_REGS - 1,

	CPUINFO_STR_CPU_SPECIFIC = 0x28000					/* R/W: CPU-specific values start here */
};


/* get_reg/set_reg constants */
enum
{
	/* This value is passed to activecpu_get_reg to retrieve the previous
     * program counter value, ie. before a CPU emulation started
     * to fetch opcodes and arguments for the current instrution. */
	REG_PREVIOUSPC = CPUINFO_INT_PREVIOUSPC - CPUINFO_INT_REGISTER,

	/* This value is passed to activecpu_get_reg to retrieve the current
     * program counter value. */
	REG_PC = CPUINFO_INT_PC - CPUINFO_INT_REGISTER,

	/* This value is passed to activecpu_get_reg to retrieve the current
     * stack pointer value. */
	REG_SP = CPUINFO_INT_SP - CPUINFO_INT_REGISTER
};


/* Endianness constants */
enum
{
	CPU_IS_LE = 0,				/* emulated CPU is little endian */
	CPU_IS_BE					/* emulated CPU is big endian */
};


/* Disassembler constants */
#define DASMFLAG_SUPPORTED		0x80000000	/* are disassembly flags supported? */
#define DASMFLAG_STEP_OUT		0x40000000	/* this instruction should be the end of a step out sequence */
#define DASMFLAG_STEP_OVER		0x20000000	/* this instruction should be stepped over by setting a breakpoint afterwards */
#define DASMFLAG_OVERINSTMASK	0x18000000	/* number of extra instructions to skip when stepping over */
#define DASMFLAG_OVERINSTSHIFT	27			/* bits to shift after masking to get the value */
#define DASMFLAG_LENGTHMASK 	0x0000ffff	/* the low 16-bits contain the actual length */
#define DASMFLAG_STEP_OVER_EXTRA(x)			((x) << DASMFLAG_OVERINSTSHIFT)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward declaration of this union */
typedef union _cpuinfo cpuinfo;


/* define the various callback functions */
typedef void (*cpu_get_info_func)(UINT32 state, cpuinfo *info);
typedef void (*cpu_set_info_func)(UINT32 state, cpuinfo *info);
typedef void (*cpu_get_context_func)(void *buffer);
typedef void (*cpu_set_context_func)(void *buffer);
typedef void (*cpu_init_func)(int index, int clock, const void *config, int (*irqcallback)(int));
typedef void (*cpu_reset_func)(void);
typedef void (*cpu_exit_func)(void);
typedef int	(*cpu_execute_func)(int cycles);
typedef void (*cpu_burn_func)(int cycles);
typedef offs_t (*cpu_disassemble_func)(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
typedef int	(*cpu_translate_func)(int space, offs_t *address);
typedef int	(*cpu_read_func)(int space, UINT32 offset, int size, UINT64 *value);
typedef int	(*cpu_write_func)(int space, UINT32 offset, int size, UINT64 value);
typedef int	(*cpu_readop_func)(UINT32 offset, int size, UINT64 *value);
typedef void (*cpu_setup_commands_func)(void);


/* cpuinfo union used to pass data to/from the get_info/set_info functions */
union _cpuinfo
{
	INT64					i;							/* generic integers */
	void *					p;							/* generic pointers */
	genf *  				f;							/* generic function pointers */
	char *					s;							/* generic strings */

	cpu_set_info_func		setinfo;					/* CPUINFO_PTR_SET_INFO */
	cpu_get_context_func	getcontext;					/* CPUINFO_PTR_GET_CONTEXT */
	cpu_set_context_func	setcontext;					/* CPUINFO_PTR_SET_CONTEXT */
	cpu_init_func			init;						/* CPUINFO_PTR_INIT */
	cpu_reset_func			reset;						/* CPUINFO_PTR_RESET */
	cpu_exit_func			exit;						/* CPUINFO_PTR_EXIT */
	cpu_execute_func		execute;					/* CPUINFO_PTR_EXECUTE */
	cpu_burn_func			burn;						/* CPUINFO_PTR_BURN */
	cpu_disassemble_func	disassemble;				/* CPUINFO_PTR_DISASSEMBLE */
	cpu_translate_func		translate;					/* CPUINFO_PTR_TRANSLATE */
	cpu_read_func			read;						/* CPUINFO_PTR_READ */
	cpu_write_func			write;						/* CPUINFO_PTR_WRITE */
	cpu_readop_func			readop;						/* CPUINFO_PTR_READOP */
	cpu_setup_commands_func	setup_commands;				/* CPUINFO_PTR_DEBUG_SETUP_COMMANDS */
	int *					icount;						/* CPUINFO_PTR_INSTRUCTION_COUNTER */
	const addrmap8_token *	internal_map8;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap16_token *	internal_map16;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap32_token *	internal_map32;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap64_token *	internal_map64;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
};


typedef struct _cpu_interface cpu_interface;
struct _cpu_interface
{
	/* table of core functions */
	cpu_get_info_func		get_info;
	cpu_set_info_func		set_info;
	cpu_get_context_func	get_context;
	cpu_set_context_func	set_context;
	cpu_init_func			init;
	cpu_reset_func			reset;
	cpu_exit_func			exit;
	cpu_execute_func		execute;
	cpu_burn_func			burn;
	cpu_disassemble_func	disassemble;
	cpu_translate_func		translate;

	/* other info */
	size_t					context_size;
	INT8					address_shift;
	int *					icount;
};



/***************************************************************************
    CORE CPU INTERFACE FUNCTIONS
***************************************************************************/

/* reset the internal CPU tracking */
void cpuintrf_init(running_machine *machine);

/* set up the interface for one CPU of a given type */
int	cpuintrf_init_cpu(int cpunum, cpu_type cputype, int clock, const void *config, int (*irqcallback)(int));

/* clean up the interface for one CPU */
void cpuintrf_exit_cpu(int cpunum);

/* remember the previous context and set a new one */
void cpuintrf_push_context(int cpunum);

/* restore the previous context */
void cpuintrf_pop_context(void);

/* circular string buffer */
char *cpuintrf_temp_str(void);

/* set the dasm override handler */
void cpuintrf_set_dasm_override(int cpunum, offs_t (*dasm_override)(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram));



/***************************************************************************
    ACTIVE CPU ACCCESSORS
***************************************************************************/

/* get info accessors */
INT64 activecpu_get_info_int(UINT32 state);
void *activecpu_get_info_ptr(UINT32 state);
genf *activecpu_get_info_fct(UINT32 state);
const char *activecpu_get_info_string(UINT32 state);

/* set info accessors */
void activecpu_set_info_int(UINT32 state, INT64 data);
void activecpu_set_info_ptr(UINT32 state, void *data);
void activecpu_set_info_fct(UINT32 state, genf *data);

/* apply a +/- to the current icount */
void activecpu_adjust_icount(int delta);

/* return the current icount */
int activecpu_get_icount(void);

/* ensure banking is reset properly */
void activecpu_reset_banking(void);

/* set the input line on a CPU -- drivers use cpu_set_input_line() */
void activecpu_set_input_line(int irqline, int state);

/* return the PC, corrected to a byte offset, on the active CPU */
offs_t activecpu_get_physical_pc_byte(void);

/* update the banking on the active CPU */
void activecpu_set_opbase(offs_t val);

/* disassemble a line at a given PC on the active CPU */
offs_t activecpu_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#define activecpu_context_size()				activecpu_get_info_int(CPUINFO_INT_CONTEXT_SIZE)
#define activecpu_input_lines()					activecpu_get_info_int(CPUINFO_INT_INPUT_LINES)
#define activecpu_output_lines()				activecpu_get_info_int(CPUINFO_INT_OUTPUT_LINES)
#define activecpu_default_irq_vector()			activecpu_get_info_int(CPUINFO_INT_DEFAULT_IRQ_VECTOR)
#define activecpu_endianness()					activecpu_get_info_int(CPUINFO_INT_ENDIANNESS)
#define activecpu_clock_multiplier()			activecpu_get_info_int(CPUINFO_INT_CLOCK_MULTIPLIER)
#define activecpu_clock_divider()				activecpu_get_info_int(CPUINFO_INT_CLOCK_DIVIDER)
#define activecpu_min_instruction_bytes()		activecpu_get_info_int(CPUINFO_INT_MIN_INSTRUCTION_BYTES)
#define activecpu_max_instruction_bytes()		activecpu_get_info_int(CPUINFO_INT_MAX_INSTRUCTION_BYTES)
#define activecpu_min_cycles()					activecpu_get_info_int(CPUINFO_INT_MIN_CYCLES)
#define activecpu_max_cycles()					activecpu_get_info_int(CPUINFO_INT_MAX_CYCLES)
#define activecpu_databus_width(space)			activecpu_get_info_int(CPUINFO_INT_DATABUS_WIDTH + (space))
#define activecpu_addrbus_width(space)			activecpu_get_info_int(CPUINFO_INT_ADDRBUS_WIDTH + (space))
#define activecpu_addrbus_shift(space)			activecpu_get_info_int(CPUINFO_INT_ADDRBUS_SHIFT + (space))
#define activecpu_logaddr_width(space)			activecpu_get_info_int(CPUINFO_INT_LOGADDR_WIDTH + (space))
#define activecpu_page_shift(space)				activecpu_get_info_int(CPUINFO_INT_PAGE_SHIFT + (space))
#define activecpu_get_reg(reg)					activecpu_get_info_int(CPUINFO_INT_REGISTER + (reg))
#define activecpu_debug_register_list()			activecpu_get_info_ptr(CPUINFO_PTR_DEBUG_REGISTER_LIST)
#define activecpu_name()						activecpu_get_info_string(CPUINFO_STR_NAME)
#define activecpu_core_family()					activecpu_get_info_string(CPUINFO_STR_CORE_FAMILY)
#define activecpu_core_version()				activecpu_get_info_string(CPUINFO_STR_CORE_VERSION)
#define activecpu_core_file()					activecpu_get_info_string(CPUINFO_STR_CORE_FILE)
#define activecpu_core_credits()				activecpu_get_info_string(CPUINFO_STR_CORE_CREDITS)
#define activecpu_flags()						activecpu_get_info_string(CPUINFO_STR_FLAGS)
#define activecpu_irq_string(irq)				activecpu_get_info_string(CPUINFO_STR_IRQ_STATE + (irq))
#define activecpu_reg_string(reg)				activecpu_get_info_string(CPUINFO_STR_REGISTER + (reg))

#define activecpu_set_reg(reg, val)				activecpu_set_info_int(CPUINFO_INT_REGISTER + (reg), (val))



/***************************************************************************
    SPECIFIC CPU ACCCESSORS
***************************************************************************/

/* get info accessors */
INT64 cpunum_get_info_int(int cpunum, UINT32 state);
void *cpunum_get_info_ptr(int cpunum, UINT32 state);
genf *cpunum_get_info_fct(int cpunum, UINT32 state);
const char *cpunum_get_info_string(int cpunum, UINT32 state);

/* set info accessors */
void cpunum_set_info_int(int cpunum, UINT32 state, INT64 data);
void cpunum_set_info_ptr(int cpunum, UINT32 state, void *data);
void cpunum_set_info_fct(int cpunum, UINT32 state, genf *data);

/* execute the requested cycles on a given CPU */
int cpunum_execute(int cpunum, int cycles);

/* signal a reset for a given CPU */
void cpunum_reset(int cpunum);

/* read a byte from another CPU's memory space */
UINT8 cpunum_read_byte(int cpunum, offs_t address);

/* write a byte from another CPU's memory space */
void cpunum_write_byte(int cpunum, offs_t address, UINT8 data);

/* return a pointer to the saved context of a given CPU, or NULL if the
   context is active (and contained within the CPU core */
void *cpunum_get_context_ptr(int cpunum);

/* return the PC, corrected to a byte offset, on a given CPU */
offs_t cpunum_get_physical_pc_byte(int cpunum);

/* update the banking on a given CPU */
void cpunum_set_opbase(int cpunum, offs_t val);

/* disassemble a line at a given PC on a given CPU */
offs_t cpunum_dasm(int cpunum, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#define cpunum_context_size(cpunum)				cpunum_get_info_int(cpunum, CPUINFO_INT_CONTEXT_SIZE)
#define cpunum_input_lines(cpunum)				cpunum_get_info_int(cpunum, CPUINFO_INT_INPUT_LINES)
#define cpunum_output_lines(cpunum)				cpunum_get_info_int(cpunum, CPUINFO_INT_OUTPUT_LINES)
#define cpunum_default_irq_vector(cpunum)		cpunum_get_info_int(cpunum, CPUINFO_INT_DEFAULT_IRQ_VECTOR)
#define cpunum_endianness(cpunum)				cpunum_get_info_int(cpunum, CPUINFO_INT_ENDIANNESS)
#define cpunum_clock_multiplier(cpunum)			cpunum_get_info_int(cpunum, CPUINFO_INT_CLOCK_MULTIPLIER)
#define cpunum_clock_divider(cpunum)			cpunum_get_info_int(cpunum, CPUINFO_INT_CLOCK_DIVIDER)
#define cpunum_min_instruction_bytes(cpunum)	cpunum_get_info_int(cpunum, CPUINFO_INT_MIN_INSTRUCTION_BYTES)
#define cpunum_max_instruction_bytes(cpunum)	cpunum_get_info_int(cpunum, CPUINFO_INT_MAX_INSTRUCTION_BYTES)
#define cpunum_min_cycles(cpunum)				cpunum_get_info_int(cpunum, CPUINFO_INT_MIN_CYCLES)
#define cpunum_max_cycles(cpunum)				cpunum_get_info_int(cpunum, CPUINFO_INT_MAX_CYCLES)
#define cpunum_databus_width(cpunum, space)		cpunum_get_info_int(cpunum, CPUINFO_INT_DATABUS_WIDTH + (space))
#define cpunum_addrbus_width(cpunum, space)		cpunum_get_info_int(cpunum, CPUINFO_INT_ADDRBUS_WIDTH + (space))
#define cpunum_addrbus_shift(cpunum, space)		cpunum_get_info_int(cpunum, CPUINFO_INT_ADDRBUS_SHIFT + (space))
#define cpunum_logaddr_width(cpunum, space)		cpunum_get_info_int(cpunum, CPUINFO_INT_LOGADDR_WIDTH + (space))
#define cpunum_page_shift(cpunum, space)		cpunum_get_info_int(cpunum, CPUINFO_INT_PAGE_SHIFT + (space))
#define cpunum_get_reg(cpunum, reg)				cpunum_get_info_int(cpunum, CPUINFO_INT_REGISTER + (reg))
#define cpunum_debug_register_list(cpunum)		cpunum_get_info_ptr(cpunum, CPUINFO_PTR_DEBUG_REGISTER_LIST)
#define cpunum_name(cpunum)						cpunum_get_info_string(cpunum, CPUINFO_STR_NAME)
#define cpunum_core_family(cpunum)				cpunum_get_info_string(cpunum, CPUINFO_STR_CORE_FAMILY)
#define cpunum_core_version(cpunum)				cpunum_get_info_string(cpunum, CPUINFO_STR_CORE_VERSION)
#define cpunum_core_file(cpunum)				cpunum_get_info_string(cpunum, CPUINFO_STR_CORE_FILE)
#define cpunum_core_credits(cpunum)				cpunum_get_info_string(cpunum, CPUINFO_STR_CORE_CREDITS)
#define cpunum_flags(cpunum)					cpunum_get_info_string(cpunum, CPUINFO_STR_FLAGS)
#define cpunum_irq_string(cpunum, irq)			cpunum_get_info_string(cpunum, CPUINFO_STR_IRQ_STATE + (irq))
#define cpunum_reg_string(cpunum, reg)			cpunum_get_info_string(cpunum, CPUINFO_STR_REGISTER + (reg))

#define cpunum_set_reg(cpunum, reg, val)		cpunum_set_info_int(cpunum, CPUINFO_INT_REGISTER + (reg), (val))



/***************************************************************************
    CPU TYPE ACCCESSORS
***************************************************************************/

/* get info accessors */
INT64 cputype_get_info_int(cpu_type cputype, UINT32 state);
void *cputype_get_info_ptr(cpu_type cputype, UINT32 state);
genf *cputype_get_info_fct(cpu_type cputype, UINT32 state);
const char *cputype_get_info_string(cpu_type cputype, UINT32 state);

#define cputype_context_size(cputype)			cputype_get_info_int(cputype, CPUINFO_INT_CONTEXT_SIZE)
#define cputype_input_lines(cputype)			cputype_get_info_int(cputype, CPUINFO_INT_INPUT_LINES)
#define cputype_output_lines(cputype)			cputype_get_info_int(cputype, CPUINFO_INT_OUTPUT_LINES)
#define cputype_default_irq_vector(cputype)		cputype_get_info_int(cputype, CPUINFO_INT_DEFAULT_IRQ_VECTOR)
#define cputype_endianness(cputype)				cputype_get_info_int(cputype, CPUINFO_INT_ENDIANNESS)
#define cputype_clock_multiplier(cputype)		cputype_get_info_int(cputype, CPUINFO_INT_CLOCK_MULTIPLIER)
#define cputype_clock_divider(cputype)			cputype_get_info_int(cputype, CPUINFO_INT_CLOCK_DIVIDER)
#define cputype_min_instruction_bytes(cputype)	cputype_get_info_int(cputype, CPUINFO_INT_MIN_INSTRUCTION_BYTES)
#define cputype_max_instruction_bytes(cputype)	cputype_get_info_int(cputype, CPUINFO_INT_MAX_INSTRUCTION_BYTES)
#define cputype_min_cycles(cputype)				cputype_get_info_int(cputype, CPUINFO_INT_MIN_CYCLES)
#define cputype_max_cycles(cputype)				cputype_get_info_int(cputype, CPUINFO_INT_MAX_CYCLES)
#define cputype_databus_width(cputype, space)	cputype_get_info_int(cputype, CPUINFO_INT_DATABUS_WIDTH + (space))
#define cputype_addrbus_width(cputype, space)	cputype_get_info_int(cputype, CPUINFO_INT_ADDRBUS_WIDTH + (space))
#define cputype_addrbus_shift(cputype, space)	cputype_get_info_int(cputype, CPUINFO_INT_ADDRBUS_SHIFT + (space))
#define cputype_page_shift(cputype, space)		cputype_get_info_int(cputype, CPUINFO_INT_PAGE_SHIFT + (space))
#define cputype_debug_register_list(cputype)	cputype_get_info_ptr(cputype, CPUINFO_PTR_DEBUG_REGISTER_LIST)
#define cputype_name(cputype)					cputype_get_info_string(cputype, CPUINFO_STR_NAME)
#define cputype_core_family(cputype)			cputype_get_info_string(cputype, CPUINFO_STR_CORE_FAMILY)
#define cputype_core_version(cputype)			cputype_get_info_string(cputype, CPUINFO_STR_CORE_VERSION)
#define cputype_core_file(cputype)				cputype_get_info_string(cputype, CPUINFO_STR_CORE_FILE)
#define cputype_core_credits(cputype)			cputype_get_info_string(cputype, CPUINFO_STR_CORE_CREDITS)



/***************************************************************************
    MACROS
***************************************************************************/

#define		activecpu_get_previouspc()			((offs_t)activecpu_get_reg(REG_PREVIOUSPC))
#define		activecpu_get_pc()					((offs_t)activecpu_get_reg(REG_PC))
#define		activecpu_get_sp()					activecpu_get_reg(REG_SP)



/***************************************************************************
    CPU INTERFACE ACCESSORS
***************************************************************************/

/* return a pointer to the interface struct for a given CPU type */
INLINE const cpu_interface *cputype_get_interface(cpu_type cputype)
{
	extern cpu_interface cpuintrf[];
	return &cpuintrf[cputype];
}


/* return a the index of the active CPU */
INLINE int cpu_getactivecpu(void)
{
	extern int activecpu;
	return activecpu;
}


/* return a the index of the executing CPU */
INLINE int cpu_getexecutingcpu(void)
{
	extern int executingcpu;
	return executingcpu;
}


/* return a the total number of registered CPUs */
INLINE int cpu_gettotalcpu(void)
{
	extern int totalcpu;
	return totalcpu;
}


/* return the current PC or ~0 if no CPU is active */
INLINE offs_t safe_activecpu_get_pc(void)
{
	return (cpu_getactivecpu() >= 0) ? activecpu_get_pc() : ~0;
}


#endif	/* __CPUINTRF_H__ */
