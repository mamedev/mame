/***************************************************************************

    cpuintrf.h

    Core CPU interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __CPUINTRF_H__
#define __CPUINTRF_H__

#include "devintrf.h"
#include "memory.h"
#include "watchdog.h"
#include "state.h"


// mingw has this defined for 32-bit compiles
#undef i386


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_CPU 				8
#define MAX_INPUT_EVENTS		32


/* Interrupt line constants */
enum
{
	/* line states */
	CLEAR_LINE = 0,				/* clear (a fired, held or pulsed) line */
	ASSERT_LINE,				/* assert an interrupt immediately */
	HOLD_LINE,					/* hold interrupt line until acknowledged */
	PULSE_LINE,					/* pulse interrupt line for one instruction */

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
	CPUINFO_INT_FIRST = DEVINFO_INT_FIRST,

		/* CPU-specific additions */
		CPUINFO_INT_CONTEXT_SIZE = DEVINFO_INT_CLASS_SPECIFIC,	/* R/O: size of CPU context in bytes */
		CPUINFO_INT_INPUT_LINES,							/* R/O: number of input lines */
		CPUINFO_INT_OUTPUT_LINES,							/* R/O: number of output lines */
		CPUINFO_INT_DEFAULT_IRQ_VECTOR,						/* R/O: default IRQ vector */
		CPUINFO_INT_ENDIANNESS,								/* R/O: either ENDIANNESS_BIG or ENDIANNESS_LITTLE */
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

	CPUINFO_INT_CPU_SPECIFIC = 0x08000,						/* R/W: CPU-specific values start here */

	/* --- the following bits of info are returned as pointers to data or functions --- */
	CPUINFO_PTR_FIRST = DEVINFO_PTR_FIRST,

		/* CPU-specific additions */
		CPUINFO_PTR_INSTRUCTION_COUNTER = DEVINFO_PTR_CLASS_SPECIFIC,
															/* R/O: int *icount */
		CPUINFO_PTR_INTERNAL_MEMORY_MAP,					/* R/O: const addrmap_token *map */
		CPUINFO_PTR_INTERNAL_MEMORY_MAP_LAST = CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACES - 1,
		CPUINFO_PTR_DEBUG_REGISTER_LIST,					/* R/O: int *list: list of registers for the debugger */

	CPUINFO_PTR_CPU_SPECIFIC = DEVINFO_PTR_DEVICE_SPECIFIC,	/* R/W: CPU-specific values start here */

	/* --- the following bits of info are returned as pointers to functions --- */
	CPUINFO_FCT_FIRST = DEVINFO_FCT_FIRST,

		/* direct map to device data */
		CPUINFO_PTR_RESET = DEVINFO_FCT_RESET,				/* R/O: void (*reset)(const device_config *device) */
		CPUINFO_PTR_EXIT = DEVINFO_FCT_STOP,				/* R/O: void (*exit)(const device_config *device) */

		/* CPU-specific additions */
		CPUINFO_PTR_SET_INFO = DEVINFO_FCT_CLASS_SPECIFIC,	/* R/O: void (*set_info)(const device_config *device, UINT32 state, INT64 data, void *ptr) */
		CPUINFO_PTR_INIT,									/* R/O: void (*init)(const device_config *device, int index, int clock, int (*irqcallback)(const device_config *device, int)) */
		CPUINFO_PTR_EXECUTE,								/* R/O: int (*execute)(const device_config *device, int cycles) */
		CPUINFO_PTR_BURN,									/* R/O: void (*burn)(const device_config *device, int cycles) */
		CPUINFO_PTR_DISASSEMBLE,							/* R/O: offs_t (*disassemble)(const device_config *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram) */
		CPUINFO_PTR_TRANSLATE,								/* R/O: int (*translate)(const device_config *device, int space, int intention, offs_t *address) */
		CPUINFO_PTR_READ,									/* R/O: int (*read)(const device_config *device, int space, UINT32 offset, int size, UINT64 *value) */
		CPUINFO_PTR_WRITE,									/* R/O: int (*write)(const device_config *device, int space, UINT32 offset, int size, UINT64 value) */
		CPUINFO_PTR_READOP,									/* R/O: int (*readop)(const device_config *device, UINT32 offset, int size, UINT64 *value) */
		CPUINFO_PTR_DEBUG_INIT,								/* R/O: void (*debug_init)(const device_config *device) */
		CPUINFO_PTR_VALIDITY_CHECK,							/* R/O: int (*validity_check)(const game_driver *driver, const void *config) */

	CPUINFO_FCT_CPU_SPECIFIC = DEVINFO_FCT_DEVICE_SPECIFIC,	/* R/W: CPU-specific values start here */

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	CPUINFO_STR_FIRST = DEVINFO_STR_FIRST,

		/* direct map to device data */
		CPUINFO_STR_NAME = DEVINFO_STR_NAME,				/* R/O: name of the CPU */
		CPUINFO_STR_CORE_FAMILY = DEVINFO_STR_FAMILY,		/* R/O: family of the CPU */
		CPUINFO_STR_CORE_VERSION = DEVINFO_STR_VERSION,		/* R/O: version of the CPU core */
		CPUINFO_STR_CORE_FILE = DEVINFO_STR_SOURCE_FILE,	/* R/O: file containing the CPU core */
		CPUINFO_STR_CORE_CREDITS = DEVINFO_STR_CREDITS,		/* R/O: credits for the CPU core */

		/* CPU-specific additions */
		CPUINFO_STR_FLAGS = DEVINFO_STR_CLASS_SPECIFIC,		/* R/O: string representation of the main flags value */
		CPUINFO_STR_REGISTER,								/* R/O: string representation of up to MAX_REGs registers */
		CPUINFO_STR_REGISTER_LAST = CPUINFO_STR_REGISTER + MAX_REGS - 1,

	CPUINFO_STR_CPU_SPECIFIC = DEVINFO_STR_DEVICE_SPECIFIC	/* R/W: CPU-specific values start here */
};


/* get_reg/set_reg constants */
enum
{
	/* This value is passed to cpu_get_reg to retrieve the previous
     * program counter value, ie. before a CPU emulation started
     * to fetch opcodes and arguments for the current instrution. */
	REG_PREVIOUSPC = CPUINFO_INT_PREVIOUSPC - CPUINFO_INT_REGISTER,

	/* This value is passed to cpu_get_reg to retrieve the current
     * program counter value. */
	REG_PC = CPUINFO_INT_PC - CPUINFO_INT_REGISTER,

	/* This value is passed to cpu_get_reg to retrieve the current
     * stack pointer value. */
	REG_SP = CPUINFO_INT_SP - CPUINFO_INT_REGISTER
};


/* Translation intentions */
#define TRANSLATE_TYPE_MASK		0x03		/* read write or fetch */
#define TRANSLATE_USER_MASK		0x04		/* user mode or fully privileged */
#define TRANSLATE_DEBUG_MASK	0x08		/* debug mode (no side effects) */

#define TRANSLATE_READ			0			/* translate for read */
#define TRANSLATE_WRITE			1			/* translate for write */
#define TRANSLATE_FETCH			2			/* translate for instruction fetch */
#define TRANSLATE_READ_USER		(TRANSLATE_READ | TRANSLATE_USER_MASK)
#define TRANSLATE_WRITE_USER	(TRANSLATE_WRITE | TRANSLATE_USER_MASK)
#define TRANSLATE_FETCH_USER	(TRANSLATE_FETCH | TRANSLATE_USER_MASK)
#define TRANSLATE_READ_DEBUG	(TRANSLATE_READ | TRANSLATE_DEBUG_MASK)
#define TRANSLATE_WRITE_DEBUG	(TRANSLATE_WRITE | TRANSLATE_DEBUG_MASK)
#define TRANSLATE_FETCH_DEBUG	(TRANSLATE_FETCH | TRANSLATE_DEBUG_MASK)


/* Disassembler constants */
#define DASMFLAG_SUPPORTED		0x80000000	/* are disassembly flags supported? */
#define DASMFLAG_STEP_OUT		0x40000000	/* this instruction should be the end of a step out sequence */
#define DASMFLAG_STEP_OVER		0x20000000	/* this instruction should be stepped over by setting a breakpoint afterwards */
#define DASMFLAG_OVERINSTMASK	0x18000000	/* number of extra instructions to skip when stepping over */
#define DASMFLAG_OVERINSTSHIFT	27			/* bits to shift after masking to get the value */
#define DASMFLAG_LENGTHMASK 	0x0000ffff	/* the low 16-bits contain the actual length */
#define DASMFLAG_STEP_OVER_EXTRA(x)			((x) << DASMFLAG_OVERINSTSHIFT)



/***************************************************************************
    MACROS
***************************************************************************/

#define IRQ_CALLBACK(func)			int func(const device_config *device, int irqline)

#define CPU_GET_INFO_NAME(name)		cpu_get_info_##name
#define CPU_GET_INFO(name)			void CPU_GET_INFO_NAME(name)(const device_config *device, UINT32 state, cpuinfo *info)
#define CPU_GET_INFO_CALL(name)		CPU_GET_INFO_NAME(name)(device, state, info)

#define CPU_SET_INFO_NAME(name)		cpu_set_info_##name
#define CPU_SET_INFO(name)			void CPU_SET_INFO_NAME(name)(const device_config *device, UINT32 state, cpuinfo *info)
#define CPU_SET_INFO_CALL(name)		CPU_SET_INFO_NAME(name)(device, state, info)

#define CPU_INIT_NAME(name)			cpu_init_##name
#define CPU_INIT(name)				void CPU_INIT_NAME(name)(const device_config *device, int index, int clock, cpu_irq_callback irqcallback)
#define CPU_INIT_CALL(name)			CPU_INIT_NAME(name)(device, index, clock, irqcallback)

#define CPU_RESET_NAME(name)		cpu_reset_##name
#define CPU_RESET(name)				void CPU_RESET_NAME(name)(const device_config *device)
#define CPU_RESET_CALL(name)		CPU_RESET_NAME(name)(device)

#define CPU_EXIT_NAME(name)			cpu_exit_##name
#define CPU_EXIT(name)				void CPU_EXIT_NAME(name)(const device_config *device)
#define CPU_EXIT_CALL(name)			CPU_EXIT_NAME(name)(device)

#define CPU_EXECUTE_NAME(name)		cpu_execute_##name
#define CPU_EXECUTE(name)			int CPU_EXECUTE_NAME(name)(const device_config *device, int cycles)
#define CPU_EXECUTE_CALL(name)		CPU_EXECUTE_NAME(name)(device, cycles)

#define CPU_BURN_NAME(name)			cpu_burn_##name
#define CPU_BURN(name)				void CPU_BURN_NAME(name)(const device_config *device, int cycles)
#define CPU_BURN_CALL(name)			CPU_BURN_NAME(name)(device, cycles)

#define CPU_TRANSLATE_NAME(name)	cpu_translate_##name
#define CPU_TRANSLATE(name)			int CPU_TRANSLATE_NAME(name)(const device_config *device, int space, int intention, offs_t *address)
#define CPU_TRANSLATE_CALL(name)	CPU_TRANSLATE_NAME(name)(device, space, intention, address)

#define CPU_READ_NAME(name)			cpu_read_##name
#define CPU_READ(name)				int CPU_READ_NAME(name)(const device_config *device, int space, UINT32 offset, int size, UINT64 *value)
#define CPU_READ_CALL(name)			CPU_READ_NAME(name)(device, space, offset, size, value)

#define CPU_WRITE_NAME(name)		cpu_write_##name
#define CPU_WRITE(name)				int CPU_WRITE_NAME(name)(const device_config *device, int space, UINT32 offset, int size, UINT64 value)
#define CPU_WRITE_CALL(name)		CPU_WRITE_NAME(name)(device, space, offset, size, value)

#define CPU_READOP_NAME(name)		cpu_readop_##name
#define CPU_READOP(name)			int CPU_READOP_NAME(name)(const device_config *device, UINT32 offset, int size, UINT64 *value)
#define CPU_READOP_CALL(name)		CPU_READOP_NAME(name)(device, offset, size, value)

#define CPU_DEBUG_INIT_NAME(name) 	cpu_debug_init_##name
#define CPU_DEBUG_INIT(name)		void CPU_DEBUG_INIT_NAME(name)(const device_config *device)
#define CPU_DEBUG_INIT_CALL(name)	CPU_DEBUG_INIT_NAME(name)(device)

#define CPU_DISASSEMBLE_NAME(name)	cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)		offs_t CPU_DISASSEMBLE_NAME(name)(const device_config *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
#define CPU_DISASSEMBLE_CALL(name)	CPU_DISASSEMBLE_NAME(name)(device, buffer, pc, oprom, opram)

#define CPU_VALIDITY_CHECK_NAME(name)	cpu_validity_check_##name
#define CPU_VALIDITY_CHECK(name)		int CPU_VALIDITY_CHECK_NAME(name)(const game_driver *driver, const void *config)
#define CPU_VALIDITY_CHECK_CALL(name)	CPU_VALIDITY_CHECK_NAME(name)(driver, config)



#define cpu_get_index(cpu)					device_list_index((cpu)->machine->config->devicelist, CPU, (cpu)->tag)


/* helpers for accessing common CPU state */
#define cpu_get_context_size(cpu)			cpu_get_info_int(cpu, CPUINFO_INT_CONTEXT_SIZE)
#define cpu_get_input_lines(cpu)			cpu_get_info_int(cpu, CPUINFO_INT_INPUT_LINES)
#define cpu_get_output_lines(cpu)			cpu_get_info_int(cpu, CPUINFO_INT_OUTPUT_LINES)
#define cpu_get_default_irq_vector(cpu)		cpu_get_info_int(cpu, CPUINFO_INT_DEFAULT_IRQ_VECTOR)
#define cpu_get_endianness(cpu)				cpu_get_info_int(cpu, CPUINFO_INT_ENDIANNESS)
#define cpu_get_clock_multiplier(cpu)		cpu_get_info_int(cpu, CPUINFO_INT_CLOCK_MULTIPLIER)
#define cpu_get_clock_divider(cpu)			cpu_get_info_int(cpu, CPUINFO_INT_CLOCK_DIVIDER)
#define cpu_get_min_opcode_bytes(cpu)		cpu_get_info_int(cpu, CPUINFO_INT_MIN_INSTRUCTION_BYTES)
#define cpu_get_max_opcode_bytes(cpu)		cpu_get_info_int(cpu, CPUINFO_INT_MAX_INSTRUCTION_BYTES)
#define cpu_get_min_cycles(cpu)				cpu_get_info_int(cpu, CPUINFO_INT_MIN_CYCLES)
#define cpu_get_max_cycles(cpu)				cpu_get_info_int(cpu, CPUINFO_INT_MAX_CYCLES)
#define cpu_get_databus_width(cpu, space)	cpu_get_info_int(cpu, CPUINFO_INT_DATABUS_WIDTH + (space))
#define cpu_get_addrbus_width(cpu, space)	cpu_get_info_int(cpu, CPUINFO_INT_ADDRBUS_WIDTH + (space))
#define cpu_get_addrbus_shift(cpu, space)	cpu_get_info_int(cpu, CPUINFO_INT_ADDRBUS_SHIFT + (space))
#define cpu_get_logaddr_width(cpu, space)	cpu_get_info_int(cpu, CPUINFO_INT_LOGADDR_WIDTH + (space))
#define cpu_get_page_shift(cpu, space)		cpu_get_info_int(cpu, CPUINFO_INT_PAGE_SHIFT + (space))
#define cpu_get_reg(cpu, reg)				cpu_get_info_int(cpu, CPUINFO_INT_REGISTER + (reg))
#define	cpu_get_previouspc(cpu)				((offs_t)cpu_get_reg(cpu, REG_PREVIOUSPC))
#define	cpu_get_pc(cpu)						((offs_t)cpu_get_reg(cpu, REG_PC))
#define	cpu_get_sp(cpu)						cpu_get_reg(cpu, REG_SP)
#define cpu_get_icount_ptr(cpu)				(int *)cpu_get_info_ptr(cpu, CPUINFO_PTR_INSTRUCTION_COUNTER)
#define cpu_get_debug_register_list(cpu)	cpu_get_info_ptr(cpu, CPUINFO_PTR_DEBUG_REGISTER_LIST)
#define cpu_get_name(cpu)					cpu_get_info_string(cpu, CPUINFO_STR_NAME)
#define cpu_get_core_family(cpu)			cpu_get_info_string(cpu, CPUINFO_STR_CORE_FAMILY)
#define cpu_get_core_version(cpu)			cpu_get_info_string(cpu, CPUINFO_STR_CORE_VERSION)
#define cpu_get_core_file(cpu)				cpu_get_info_string(cpu, CPUINFO_STR_CORE_FILE)
#define cpu_get_core_credits(cpu)			cpu_get_info_string(cpu, CPUINFO_STR_CORE_CREDITS)
#define cpu_get_flags_string(cpu)			cpu_get_info_string(cpu, CPUINFO_STR_FLAGS)
#define cpu_get_irq_string(cpu, irq)		cpu_get_info_string(cpu, CPUINFO_STR_IRQ_STATE + (irq))
#define cpu_get_reg_string(cpu, reg)		cpu_get_info_string(cpu, CPUINFO_STR_REGISTER + (reg))
#define cpu_set_reg(cpu, reg, val)			cpu_set_info_int(cpu, CPUINFO_INT_REGISTER + (reg), (val))


/* helpers for accessing common CPU type state */
#define cputype_get_context_size(cputype)			cputype_get_info_int(cputype, CPUINFO_INT_CONTEXT_SIZE)
#define cputype_get_input_lines(cputype)			cputype_get_info_int(cputype, CPUINFO_INT_INPUT_LINES)
#define cputype_get_output_lines(cputype)			cputype_get_info_int(cputype, CPUINFO_INT_OUTPUT_LINES)
#define cputype_get_default_irq_vector(cputype)		cputype_get_info_int(cputype, CPUINFO_INT_DEFAULT_IRQ_VECTOR)
#define cputype_get_endianness(cputype)				cputype_get_info_int(cputype, CPUINFO_INT_ENDIANNESS)
#define cputype_get_clock_multiplier(cputype)		cputype_get_info_int(cputype, CPUINFO_INT_CLOCK_MULTIPLIER)
#define cputype_get_clock_divider(cputype)			cputype_get_info_int(cputype, CPUINFO_INT_CLOCK_DIVIDER)
#define cputype_get_min_instruction_bytes(cputype)	cputype_get_info_int(cputype, CPUINFO_INT_MIN_INSTRUCTION_BYTES)
#define cputype_get_max_instruction_bytes(cputype)	cputype_get_info_int(cputype, CPUINFO_INT_MAX_INSTRUCTION_BYTES)
#define cputype_get_min_cycles(cputype)				cputype_get_info_int(cputype, CPUINFO_INT_MIN_CYCLES)
#define cputype_get_max_cycles(cputype)				cputype_get_info_int(cputype, CPUINFO_INT_MAX_CYCLES)
#define cputype_get_databus_width(cputype, space)	cputype_get_info_int(cputype, CPUINFO_INT_DATABUS_WIDTH + (space))
#define cputype_get_addrbus_width(cputype, space)	cputype_get_info_int(cputype, CPUINFO_INT_ADDRBUS_WIDTH + (space))
#define cputype_get_addrbus_shift(cputype, space)	cputype_get_info_int(cputype, CPUINFO_INT_ADDRBUS_SHIFT + (space))
#define cputype_get_logaddr_width(cputype, space)	cputype_get_info_int(cputype, CPUINFO_INT_LOGADDR_WIDTH + (space))
#define cputype_get_page_shift(cputype, space)		cputype_get_info_int(cputype, CPUINFO_INT_PAGE_SHIFT + (space))
#define cputype_get_debug_register_list(cputype)	cputype_get_info_ptr(cputype, CPUINFO_PTR_DEBUG_REGISTER_LIST)
#define cputype_get_name(cputype)					cputype_get_info_string(cputype, CPUINFO_STR_NAME)
#define cputype_get_core_family(cputype)			cputype_get_info_string(cputype, CPUINFO_STR_CORE_FAMILY)
#define cputype_get_core_version(cputype)			cputype_get_info_string(cputype, CPUINFO_STR_CORE_VERSION)
#define cputype_get_core_file(cputype)				cputype_get_info_string(cputype, CPUINFO_STR_CORE_FILE)
#define cputype_get_core_credits(cputype)			cputype_get_info_string(cputype, CPUINFO_STR_CORE_CREDITS)


/* helpers for using machine/cputag instead of cpu objects */
#define cputag_reset(mach, tag)						cpu_reset(cputag_get_cpu(mach, tag))
#define cputag_get_index(mach, tag)					cpu_get_index(cputag_get_cpu(mach, tag))
#define cputag_get_address_space(mach, tag, space)	cpu_get_address_space(cputag_get_cpu(mach, tag), space)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* opaque definition of CPU internal and debugging info */
typedef struct _cpu_debug_data cpu_debug_data;


/* forward declaration of this union */
typedef union _cpuinfo cpuinfo;


/* define the various callback functions */
typedef int (*cpu_irq_callback)(const device_config *device, int irqnum);

typedef void (*cpu_get_info_func)(const device_config *device, UINT32 state, cpuinfo *info);
typedef void (*cpu_set_info_func)(const device_config *device, UINT32 state, cpuinfo *info);
typedef void (*cpu_init_func)(const device_config *device, int index, int clock, cpu_irq_callback irqcallback);
typedef void (*cpu_reset_func)(const device_config *device);
typedef void (*cpu_exit_func)(const device_config *device);
typedef int	(*cpu_execute_func)(const device_config *device, int cycles);
typedef void (*cpu_burn_func)(const device_config *device, int cycles);
typedef int	(*cpu_translate_func)(const device_config *device, int space, int intention, offs_t *address);
typedef int	(*cpu_read_func)(const device_config *device, int space, UINT32 offset, int size, UINT64 *value);
typedef int	(*cpu_write_func)(const device_config *device, int space, UINT32 offset, int size, UINT64 value);
typedef int	(*cpu_readop_func)(const device_config *device, UINT32 offset, int size, UINT64 *value);
typedef void (*cpu_debug_init_func)(const device_config *device);
typedef offs_t (*cpu_disassemble_func)(const device_config *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
typedef int (*cpu_validity_check_func)(const game_driver *driver, const void *config);


/* a cpu_type is just a pointer to the CPU's get_info function */
typedef cpu_get_info_func cpu_type;


/* cpuinfo union used to pass data to/from the get_info/set_info functions */
union _cpuinfo
{
	INT64					i;							/* generic integers */
	void *					p;							/* generic pointers */
	genf *  				f;							/* generic function pointers */
	char *					s;							/* generic strings */

	cpu_set_info_func		setinfo;					/* CPUINFO_PTR_SET_INFO */
	cpu_init_func			init;						/* CPUINFO_PTR_INIT */
	cpu_reset_func			reset;						/* CPUINFO_PTR_RESET */
	cpu_exit_func			exit;						/* CPUINFO_PTR_EXIT */
	cpu_execute_func		execute;					/* CPUINFO_PTR_EXECUTE */
	cpu_burn_func			burn;						/* CPUINFO_PTR_BURN */
	cpu_translate_func		translate;					/* CPUINFO_PTR_TRANSLATE */
	cpu_read_func			read;						/* CPUINFO_PTR_READ */
	cpu_write_func			write;						/* CPUINFO_PTR_WRITE */
	cpu_readop_func			readop;						/* CPUINFO_PTR_READOP */
	cpu_debug_init_func		debug_init;					/* CPUINFO_PTR_DEBUG_INIT */
	cpu_disassemble_func	disassemble;				/* CPUINFO_PTR_DISASSEMBLE */
	cpu_validity_check_func	validity_check;				/* CPUINFO_PTR_VALIDITY_CHECK */
	int *					icount;						/* CPUINFO_PTR_INSTRUCTION_COUNTER */
	const addrmap8_token *	internal_map8;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap16_token *	internal_map16;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap32_token *	internal_map32;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap64_token *	internal_map64;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
};


/* partial data hanging off of the classtoken */
typedef struct _cpu_class_header cpu_class_header;
struct _cpu_class_header
{
	int						index;					/* index of this CPU */
	cpu_debug_data *		debug;					/* debugging data */
	const address_space *	space[ADDRESS_SPACES];	/* address spaces */

	/* table of core functions */
	cpu_set_info_func		set_info;
	cpu_execute_func		execute;
	cpu_burn_func			burn;
	cpu_translate_func		translate;
	cpu_disassemble_func	disassemble;
	cpu_disassemble_func 	dasm_override;

	/* other frequently-needed information */
	UINT32					clock_divider;
	UINT32					clock_multiplier;
};



/***************************************************************************
    CPU DEFINITIONS
***************************************************************************/

#define cpu_count(config)		device_list_items((config)->devicelist, CPU)
#define cpu_first(config)		device_list_first((config)->devicelist, CPU)
#define cpu_next(previous)		device_list_next((previous), CPU)


/* signal a reset for a given CPU */
#define cpu_reset device_reset



/* eventually all drivers should include the CPU core's header, which should define these */
CPU_GET_INFO( dummy );
#define CPU_DUMMY CPU_GET_INFO_NAME( dummy )
CPU_GET_INFO( z80 );
#define CPU_Z80 CPU_GET_INFO_NAME( z80 )
CPU_GET_INFO( z180 );
#define CPU_Z180 CPU_GET_INFO_NAME( z180 )
CPU_GET_INFO( i8080 );
#define CPU_8080 CPU_GET_INFO_NAME( i8080 )
CPU_GET_INFO( i8085 );
#define CPU_8085A CPU_GET_INFO_NAME( i8085 )
CPU_GET_INFO( m6502 );
#define CPU_M6502 CPU_GET_INFO_NAME( m6502 )
CPU_GET_INFO( m65c02 );
#define CPU_M65C02 CPU_GET_INFO_NAME( m65c02 )
CPU_GET_INFO( m65sc02 );
#define CPU_M65SC02 CPU_GET_INFO_NAME( m65sc02 )
CPU_GET_INFO( m65ce02 );
#define CPU_M65CE02 CPU_GET_INFO_NAME( m65ce02 )
CPU_GET_INFO( m6509 );
#define CPU_M6509 CPU_GET_INFO_NAME( m6509 )
CPU_GET_INFO( m6510 );
#define CPU_M6510 CPU_GET_INFO_NAME( m6510 )
CPU_GET_INFO( m6510t );
#define CPU_M6510T CPU_GET_INFO_NAME( m6510t )
CPU_GET_INFO( m7501 );
#define CPU_M7501 CPU_GET_INFO_NAME( m7501 )
CPU_GET_INFO( m8502 );
#define CPU_M8502 CPU_GET_INFO_NAME( m8502 )
CPU_GET_INFO( n2a03 );
#define CPU_N2A03 CPU_GET_INFO_NAME( n2a03 )
CPU_GET_INFO( deco16 );
#define CPU_DECO16 CPU_GET_INFO_NAME( deco16 )
CPU_GET_INFO( m4510 );
#define CPU_M4510 CPU_GET_INFO_NAME( m4510 )
CPU_GET_INFO( h6280 );
#define CPU_H6280 CPU_GET_INFO_NAME( h6280 )
CPU_GET_INFO( i8086 );
#define CPU_I8086 CPU_GET_INFO_NAME( i8086 )
CPU_GET_INFO( i8088 );
#define CPU_I8088 CPU_GET_INFO_NAME( i8088 )
CPU_GET_INFO( i80186 );
#define CPU_I80186 CPU_GET_INFO_NAME( i80186 )
CPU_GET_INFO( i80188 );
#define CPU_I80188 CPU_GET_INFO_NAME( i80188 )
CPU_GET_INFO( i80286 );
#define CPU_I80286 CPU_GET_INFO_NAME( i80286 )
CPU_GET_INFO( v20 );
#define CPU_V20 CPU_GET_INFO_NAME( v20 )
CPU_GET_INFO( v25 );
#define CPU_V25 CPU_GET_INFO_NAME( v25 )
CPU_GET_INFO( v30 );
#define CPU_V30 CPU_GET_INFO_NAME( v30 )
CPU_GET_INFO( v33 );
#define CPU_V33 CPU_GET_INFO_NAME( v33 )
CPU_GET_INFO( v35 );
#define CPU_V35 CPU_GET_INFO_NAME( v35 )
CPU_GET_INFO( v60 );
#define CPU_V60 CPU_GET_INFO_NAME( v60 )
CPU_GET_INFO( v70 );
#define CPU_V70 CPU_GET_INFO_NAME( v70 )
CPU_GET_INFO( i8035 );
#define CPU_I8035 CPU_GET_INFO_NAME( i8035 )
CPU_GET_INFO( i8048 );
#define CPU_I8048 CPU_GET_INFO_NAME( i8048 )
CPU_GET_INFO( i8648 );
#define CPU_I8648 CPU_GET_INFO_NAME( i8648 )
CPU_GET_INFO( i8748 );
#define CPU_I8748 CPU_GET_INFO_NAME( i8748 )
CPU_GET_INFO( mb8884 );
#define CPU_MB8884 CPU_GET_INFO_NAME( mb8884 )
CPU_GET_INFO( n7751 );
#define CPU_N7751 CPU_GET_INFO_NAME( n7751 )
CPU_GET_INFO( i8039 );
#define CPU_I8039 CPU_GET_INFO_NAME( i8039 )
CPU_GET_INFO( i8049 );
#define CPU_I8049 CPU_GET_INFO_NAME( i8049 )
CPU_GET_INFO( i8749 );
#define CPU_I8749 CPU_GET_INFO_NAME( i8749 )
CPU_GET_INFO( m58715 );
#define CPU_M58715 CPU_GET_INFO_NAME( m58715 )
CPU_GET_INFO( i8041 );
#define CPU_I8041 CPU_GET_INFO_NAME( i8041 )
CPU_GET_INFO( i8741 );
#define CPU_I8741 CPU_GET_INFO_NAME( i8741 )
CPU_GET_INFO( i8042 );
#define CPU_I8042 CPU_GET_INFO_NAME( i8042 )
CPU_GET_INFO( i8242 );
#define CPU_I8242 CPU_GET_INFO_NAME( i8242 )
CPU_GET_INFO( i8742 );
#define CPU_I8742 CPU_GET_INFO_NAME( i8742 )
CPU_GET_INFO( i8031 );
#define CPU_I8031 CPU_GET_INFO_NAME( i8031 )
CPU_GET_INFO( i8032 );
#define CPU_I8032 CPU_GET_INFO_NAME( i8032 )
CPU_GET_INFO( i8051 );
#define CPU_I8051 CPU_GET_INFO_NAME( i8051 )
CPU_GET_INFO( i8052 );
#define CPU_I8052 CPU_GET_INFO_NAME( i8052 )
CPU_GET_INFO( i8751 );
#define CPU_I8751 CPU_GET_INFO_NAME( i8751 )
CPU_GET_INFO( i8752 );
#define CPU_I8752 CPU_GET_INFO_NAME( i8752 )
CPU_GET_INFO( i80c31 );
#define CPU_I80C31 CPU_GET_INFO_NAME( i80c31 )
CPU_GET_INFO( i80c32 );
#define CPU_I80C32 CPU_GET_INFO_NAME( i80c32 )
CPU_GET_INFO( i80c51 );
#define CPU_I80C51 CPU_GET_INFO_NAME( i80c51 )
CPU_GET_INFO( i80c52 );
#define CPU_I80C52 CPU_GET_INFO_NAME( i80c52 )
CPU_GET_INFO( i87c51 );
#define CPU_I87C51 CPU_GET_INFO_NAME( i87c51 )
CPU_GET_INFO( i87c52 );
#define CPU_I87C52 CPU_GET_INFO_NAME( i87c52 )
CPU_GET_INFO( at89c4051 );
#define CPU_AT89C4051 CPU_GET_INFO_NAME( at89c4051 )
CPU_GET_INFO( ds5002fp );
#define CPU_DS5002FP CPU_GET_INFO_NAME( ds5002fp )
CPU_GET_INFO( m6800 );
#define CPU_M6800 CPU_GET_INFO_NAME( m6800 )
CPU_GET_INFO( m6801 );
#define CPU_M6801 CPU_GET_INFO_NAME( m6801 )
CPU_GET_INFO( m6802 );
#define CPU_M6802 CPU_GET_INFO_NAME( m6802 )
CPU_GET_INFO( m6803 );
#define CPU_M6803 CPU_GET_INFO_NAME( m6803 )
CPU_GET_INFO( m6808 );
#define CPU_M6808 CPU_GET_INFO_NAME( m6808 )
CPU_GET_INFO( hd63701 );
#define CPU_HD63701 CPU_GET_INFO_NAME( hd63701 )
CPU_GET_INFO( nsc8105 );
#define CPU_NSC8105 CPU_GET_INFO_NAME( nsc8105 )
CPU_GET_INFO( m6805 );
#define CPU_M6805 CPU_GET_INFO_NAME( m6805 )
CPU_GET_INFO( m68705 );
#define CPU_M68705 CPU_GET_INFO_NAME( m68705 )
CPU_GET_INFO( hd63705 );
#define CPU_HD63705 CPU_GET_INFO_NAME( hd63705 )
CPU_GET_INFO( hd6309 );
#define CPU_HD6309 CPU_GET_INFO_NAME( hd6309 )
CPU_GET_INFO( m6809 );
#define CPU_M6809 CPU_GET_INFO_NAME( m6809 )
CPU_GET_INFO( m6809e );
#define CPU_M6809E CPU_GET_INFO_NAME( m6809e )
CPU_GET_INFO( konami );
#define CPU_KONAMI CPU_GET_INFO_NAME( konami )
CPU_GET_INFO( m68000 );
#define CPU_M68000 CPU_GET_INFO_NAME( m68000 )
CPU_GET_INFO( m68008 );
#define CPU_M68008 CPU_GET_INFO_NAME( m68008 )
CPU_GET_INFO( m68010 );
#define CPU_M68010 CPU_GET_INFO_NAME( m68010 )
CPU_GET_INFO( m68ec020 );
#define CPU_M68EC020 CPU_GET_INFO_NAME( m68ec020 )
CPU_GET_INFO( m68020 );
#define CPU_M68020 CPU_GET_INFO_NAME( m68020 )
CPU_GET_INFO( m68040 );
#define CPU_M68040 CPU_GET_INFO_NAME( m68040 )
CPU_GET_INFO( t11 );
#define CPU_T11 CPU_GET_INFO_NAME( t11 )
CPU_GET_INFO( s2650 );
#define CPU_S2650 CPU_GET_INFO_NAME( s2650 )
CPU_GET_INFO( tms34010 );
#define CPU_TMS34010 CPU_GET_INFO_NAME( tms34010 )
CPU_GET_INFO( tms34020 );
#define CPU_TMS34020 CPU_GET_INFO_NAME( tms34020 )
CPU_GET_INFO( ti990_10 );
#define CPU_TI990_10 CPU_GET_INFO_NAME( ti990_10 )
CPU_GET_INFO( tms9900 );
#define CPU_TMS9900 CPU_GET_INFO_NAME( tms9900 )
CPU_GET_INFO( tms9980a );
#define CPU_TMS9980 CPU_GET_INFO_NAME( tms9980a )
CPU_GET_INFO( tms9995 );
#define CPU_TMS9995 CPU_GET_INFO_NAME( tms9995 )
CPU_GET_INFO( z8000 );
#define CPU_Z8000 CPU_GET_INFO_NAME( z8000 )
CPU_GET_INFO( tms32010 );
#define CPU_TMS32010 CPU_GET_INFO_NAME( tms32010 )
CPU_GET_INFO( tms32025 );
#define CPU_TMS32025 CPU_GET_INFO_NAME( tms32025 )
CPU_GET_INFO( tms32026 );
#define CPU_TMS32026 CPU_GET_INFO_NAME( tms32026 )
CPU_GET_INFO( tms32031 );
#define CPU_TMS32031 CPU_GET_INFO_NAME( tms32031 )
CPU_GET_INFO( tms32032 );
#define CPU_TMS32032 CPU_GET_INFO_NAME( tms32032 )
CPU_GET_INFO( tms32051 );
#define CPU_TMS32051 CPU_GET_INFO_NAME( tms32051 )
CPU_GET_INFO( ccpu );
#define CPU_CCPU CPU_GET_INFO_NAME( ccpu )
CPU_GET_INFO( adsp2100 );
#define CPU_ADSP2100 CPU_GET_INFO_NAME( adsp2100 )
 CPU_GET_INFO( adsp2101 );
#define CPU_ADSP2101 CPU_GET_INFO_NAME( adsp2101 )
CPU_GET_INFO( adsp2104 );
#define CPU_ADSP2104 CPU_GET_INFO_NAME( adsp2104 )
CPU_GET_INFO( adsp2105 );
#define CPU_ADSP2105 CPU_GET_INFO_NAME( adsp2105 )
CPU_GET_INFO( adsp2115 );
#define CPU_ADSP2115 CPU_GET_INFO_NAME( adsp2115 )
CPU_GET_INFO( adsp2181 );
#define CPU_ADSP2181 CPU_GET_INFO_NAME( adsp2181 )
CPU_GET_INFO( psxcpu );
#define CPU_PSXCPU CPU_GET_INFO_NAME( psxcpu )
CPU_GET_INFO( asap );
#define CPU_ASAP CPU_GET_INFO_NAME( asap )
CPU_GET_INFO( upd7810 );
#define CPU_UPD7810 CPU_GET_INFO_NAME( upd7810 )
CPU_GET_INFO( upd7807 );
#define CPU_UPD7807 CPU_GET_INFO_NAME( upd7807 )
CPU_GET_INFO( upd7801 );
#define CPU_UPD7801 CPU_GET_INFO_NAME( upd7801 )
CPU_GET_INFO( upd78C05 );
#define CPU_UPD78C05 CPU_GET_INFO_NAME( upd78C05 )
CPU_GET_INFO( upd78C06 );
#define CPU_UPD78C06 CPU_GET_INFO_NAME( upd78C06 )
CPU_GET_INFO( jaguargpu );
#define CPU_JAGUARGPU CPU_GET_INFO_NAME( jaguargpu )
CPU_GET_INFO( jaguardsp );
#define CPU_JAGUARDSP CPU_GET_INFO_NAME( jaguardsp )
CPU_GET_INFO( cquestsnd );
#define CPU_CQUESTSND CPU_GET_INFO_NAME( cquestsnd )
CPU_GET_INFO( cquestrot );
#define CPU_CQUESTROT CPU_GET_INFO_NAME( cquestrot )
CPU_GET_INFO( cquestlin );
#define CPU_CQUESTLIN CPU_GET_INFO_NAME( cquestlin )
CPU_GET_INFO( r3000be );
#define CPU_R3000BE CPU_GET_INFO_NAME( r3000be )
CPU_GET_INFO( r3000le );
#define CPU_R3000LE CPU_GET_INFO_NAME( r3000le )
CPU_GET_INFO( r3041be );
#define CPU_R3041BE CPU_GET_INFO_NAME( r3041be )
CPU_GET_INFO( r3041le );
#define CPU_R3041LE CPU_GET_INFO_NAME( r3041le )
CPU_GET_INFO( r4600be );
#define CPU_R4600BE CPU_GET_INFO_NAME( r4600be )
CPU_GET_INFO( r4600le );
#define CPU_R4600LE CPU_GET_INFO_NAME( r4600le )
CPU_GET_INFO( r4650be );
#define CPU_R4650BE CPU_GET_INFO_NAME( r4650be )
CPU_GET_INFO( r4650le );
#define CPU_R4650LE CPU_GET_INFO_NAME( r4650le )
CPU_GET_INFO( r4700be );
#define CPU_R4700BE CPU_GET_INFO_NAME( r4700be )
CPU_GET_INFO( r4700le );
#define CPU_R4700LE CPU_GET_INFO_NAME( r4700le )
CPU_GET_INFO( r5000be );
#define CPU_R5000BE CPU_GET_INFO_NAME( r5000be )
CPU_GET_INFO( r5000le );
#define CPU_R5000LE CPU_GET_INFO_NAME( r5000le )
CPU_GET_INFO( qed5271be );
#define CPU_QED5271BE CPU_GET_INFO_NAME( qed5271be )
CPU_GET_INFO( qed5271le );
#define CPU_QED5271LE CPU_GET_INFO_NAME( qed5271le )
CPU_GET_INFO( rm7000be );
#define CPU_RM7000BE CPU_GET_INFO_NAME( rm7000be )
CPU_GET_INFO( rm7000le );
#define CPU_RM7000LE CPU_GET_INFO_NAME( rm7000le )
CPU_GET_INFO( arm );
#define CPU_ARM CPU_GET_INFO_NAME( arm )
CPU_GET_INFO( arm7 );
#define CPU_ARM7 CPU_GET_INFO_NAME( arm7 )
CPU_GET_INFO( sh1 );
#define CPU_SH1 CPU_GET_INFO_NAME( sh1 )
CPU_GET_INFO( sh2 );
#define CPU_SH2 CPU_GET_INFO_NAME( sh2 )
CPU_GET_INFO( sh4 );
#define CPU_SH4 CPU_GET_INFO_NAME( sh4 )
CPU_GET_INFO( dsp32c );
#define CPU_DSP32C CPU_GET_INFO_NAME( dsp32c )
CPU_GET_INFO( pic16c54 );
#define CPU_PIC16C54 CPU_GET_INFO_NAME( pic16c54 )
CPU_GET_INFO( pic16c55 );
#define CPU_PIC16C55 CPU_GET_INFO_NAME( pic16c55 )
CPU_GET_INFO( pic16c56 );
#define CPU_PIC16C56 CPU_GET_INFO_NAME( pic16c56 )
CPU_GET_INFO( pic16c57 );
#define CPU_PIC16C57 CPU_GET_INFO_NAME( pic16c57 )
CPU_GET_INFO( pic16c58 );
#define CPU_PIC16C58 CPU_GET_INFO_NAME( pic16c58 )
CPU_GET_INFO( g65816 );
#define CPU_G65816 CPU_GET_INFO_NAME( g65816 )
CPU_GET_INFO( spc700 );
#define CPU_SPC700 CPU_GET_INFO_NAME( spc700 )
CPU_GET_INFO( e116t );
#define CPU_E116T CPU_GET_INFO_NAME( e116t )
CPU_GET_INFO( e116xt );
#define CPU_E116XT CPU_GET_INFO_NAME( e116xt )
CPU_GET_INFO( e116xs );
#define CPU_E116XS CPU_GET_INFO_NAME( e116xs )
CPU_GET_INFO( e116xsr );
#define CPU_E116XSR CPU_GET_INFO_NAME( e116xsr )
CPU_GET_INFO( e132n );
#define CPU_E132N CPU_GET_INFO_NAME( e132n )
CPU_GET_INFO( e132t );
#define CPU_E132T CPU_GET_INFO_NAME( e132t )
CPU_GET_INFO( e132xn );
#define CPU_E132XN CPU_GET_INFO_NAME( e132xn )
CPU_GET_INFO( e132xt );
#define CPU_E132XT CPU_GET_INFO_NAME( e132xt )
CPU_GET_INFO( e132xs );
#define CPU_E132XS CPU_GET_INFO_NAME( e132xs )
CPU_GET_INFO( e132xsr );
#define CPU_E132XSR CPU_GET_INFO_NAME( e132xsr )
CPU_GET_INFO( gms30c2116 );
#define CPU_GMS30C2116 CPU_GET_INFO_NAME( gms30c2116 )
CPU_GET_INFO( gms30c2132 );
#define CPU_GMS30C2132 CPU_GET_INFO_NAME( gms30c2132 )
CPU_GET_INFO( gms30c2216 );
#define CPU_GMS30C2216 CPU_GET_INFO_NAME( gms30c2216 )
CPU_GET_INFO( gms30c2232 );
#define CPU_GMS30C2232 CPU_GET_INFO_NAME( gms30c2232 )
CPU_GET_INFO( i386 );
#define CPU_I386 CPU_GET_INFO_NAME( i386 )
CPU_GET_INFO( i486 );
#define CPU_I486 CPU_GET_INFO_NAME( i486 )
CPU_GET_INFO( pentium );
#define CPU_PENTIUM CPU_GET_INFO_NAME( pentium )
CPU_GET_INFO( mediagx );
#define CPU_MEDIAGX CPU_GET_INFO_NAME( mediagx )
CPU_GET_INFO( i960 );
#define CPU_I960 CPU_GET_INFO_NAME( i960 )
CPU_GET_INFO( h8_3002 );
#define CPU_H83002 CPU_GET_INFO_NAME( h8_3002 )
CPU_GET_INFO( h8_3007 );
#define CPU_H83007 CPU_GET_INFO_NAME( h8_3007 )
CPU_GET_INFO( h8_3044 );
#define CPU_H83044 CPU_GET_INFO_NAME( h8_3044 )
CPU_GET_INFO( h8_3334 );
#define CPU_H83334 CPU_GET_INFO_NAME( h8_3334 )
CPU_GET_INFO( v810 );
#define CPU_V810 CPU_GET_INFO_NAME( v810 )
CPU_GET_INFO( m37702 );
#define CPU_M37702 CPU_GET_INFO_NAME( m37702 )
CPU_GET_INFO( m37710 );
#define CPU_M37710 CPU_GET_INFO_NAME( m37710 )
CPU_GET_INFO( ppc403ga );
#define CPU_PPC403GA CPU_GET_INFO_NAME( ppc403ga )
CPU_GET_INFO( ppc403gcx );
#define CPU_PPC403GCX CPU_GET_INFO_NAME( ppc403gcx )
CPU_GET_INFO( ppc601 );
#define CPU_PPC601 CPU_GET_INFO_NAME( ppc601 )
CPU_GET_INFO( ppc602 );
#define CPU_PPC602 CPU_GET_INFO_NAME( ppc602 )
CPU_GET_INFO( ppc603 );
#define CPU_PPC603 CPU_GET_INFO_NAME( ppc603 )
CPU_GET_INFO( ppc603e );
#define CPU_PPC603E CPU_GET_INFO_NAME( ppc603e )
CPU_GET_INFO( ppc603r );
#define CPU_PPC603R CPU_GET_INFO_NAME( ppc603r )
CPU_GET_INFO( ppc604 );
#define CPU_PPC604 CPU_GET_INFO_NAME( ppc604 )
CPU_GET_INFO( mpc8240 );
#define CPU_MPC8240 CPU_GET_INFO_NAME( mpc8240 )
CPU_GET_INFO( se3208 );
#define CPU_SE3208 CPU_GET_INFO_NAME( se3208 )
CPU_GET_INFO( mc68hc11 );
#define CPU_MC68HC11 CPU_GET_INFO_NAME( mc68hc11 )
CPU_GET_INFO( adsp21062 );
#define CPU_ADSP21062 CPU_GET_INFO_NAME( adsp21062 )
CPU_GET_INFO( dsp56k );
#define CPU_DSP56156 CPU_GET_INFO_NAME( dsp56k )
CPU_GET_INFO( rsp );
#define CPU_RSP CPU_GET_INFO_NAME( rsp )
CPU_GET_INFO( alpha8201 );
#define CPU_ALPHA8201 CPU_GET_INFO_NAME( alpha8201 )
CPU_GET_INFO( alpha8301 );
#define CPU_ALPHA8301 CPU_GET_INFO_NAME( alpha8301 )
CPU_GET_INFO( cdp1802 );
#define CPU_CDP1802 CPU_GET_INFO_NAME( cdp1802 )
CPU_GET_INFO( cop401 );
#define CPU_COP401 CPU_GET_INFO_NAME( cop401 )
CPU_GET_INFO( cop410 );
#define CPU_COP410 CPU_GET_INFO_NAME( cop410 )
CPU_GET_INFO( cop411 );
#define CPU_COP411 CPU_GET_INFO_NAME( cop411 )
CPU_GET_INFO( cop402 );
#define CPU_COP402 CPU_GET_INFO_NAME( cop402 )
CPU_GET_INFO( cop420 );
#define CPU_COP420 CPU_GET_INFO_NAME( cop420 )
CPU_GET_INFO( cop421 );
#define CPU_COP421 CPU_GET_INFO_NAME( cop421 )
CPU_GET_INFO( cop422 );
#define CPU_COP422 CPU_GET_INFO_NAME( cop422 )
CPU_GET_INFO( cop404 );
#define CPU_COP404 CPU_GET_INFO_NAME( cop404 )
CPU_GET_INFO( cop424 );
#define CPU_COP424 CPU_GET_INFO_NAME( cop424 )
CPU_GET_INFO( cop425 );
#define CPU_COP425 CPU_GET_INFO_NAME( cop425 )
CPU_GET_INFO( cop426 );
#define CPU_COP426 CPU_GET_INFO_NAME( cop426 )
CPU_GET_INFO( cop444 );
#define CPU_COP444 CPU_GET_INFO_NAME( cop444 )
CPU_GET_INFO( cop445 );
#define CPU_COP445 CPU_GET_INFO_NAME( cop445 )
CPU_GET_INFO( tmp90840 );
#define CPU_TMP90840 CPU_GET_INFO_NAME( tmp90840 )
CPU_GET_INFO( tmp90841 );
#define CPU_TMP90841 CPU_GET_INFO_NAME( tmp90841 )
CPU_GET_INFO( tmp91640 );
#define CPU_TMP91640 CPU_GET_INFO_NAME( tmp91640 )
CPU_GET_INFO( tmp91641 );
#define CPU_TMP91641 CPU_GET_INFO_NAME( tmp91641 )
CPU_GET_INFO( apexc );
#define CPU_APEXC CPU_GET_INFO_NAME( apexc )
CPU_GET_INFO( cp1610 );
#define CPU_CP1610 CPU_GET_INFO_NAME( cp1610 )
CPU_GET_INFO( f8 );
#define CPU_F8 CPU_GET_INFO_NAME( f8 )
CPU_GET_INFO( lh5801 );
#define CPU_LH5801 CPU_GET_INFO_NAME( lh5801 )
CPU_GET_INFO( pdp1 );
#define CPU_PDP1 CPU_GET_INFO_NAME( pdp1 )
CPU_GET_INFO( saturn );
#define CPU_SATURN CPU_GET_INFO_NAME( saturn )
CPU_GET_INFO( sc61860 );
#define CPU_SC61860 CPU_GET_INFO_NAME( sc61860 )
CPU_GET_INFO( tx0_64kw );
#define CPU_TX0_64KW CPU_GET_INFO_NAME( tx0_64kw )
CPU_GET_INFO( tx0_8kw );
#define CPU_TX0_8KW CPU_GET_INFO_NAME( tx0_8kw )
CPU_GET_INFO( lrR35902 );
#define CPU_LR35902 CPU_GET_INFO_NAME( lr35902 )
CPU_GET_INFO( tms7000 );
#define CPU_TMS7000 CPU_GET_INFO_NAME( tms7000 )
CPU_GET_INFO( tms7000_EXL );
#define CPU_TMS7000_EXL CPU_GET_INFO_NAME( tms7000_EXL )
CPU_GET_INFO( SM8500 );
#define CPU_sm8500 CPU_GET_INFO_NAME( sm8500 )
CPU_GET_INFO( v30mz );
#define CPU_V30MZ CPU_GET_INFO_NAME( v30mz )
CPU_GET_INFO( mb8841 );
#define CPU_MB8841 CPU_GET_INFO_NAME( mb8841 )
CPU_GET_INFO( mb8842 );
#define CPU_MB8842 CPU_GET_INFO_NAME( mb8842 )
CPU_GET_INFO( mb8843 );
#define CPU_MB8843 CPU_GET_INFO_NAME( mb8843 )
CPU_GET_INFO( mb8844 );
#define CPU_MB8844 CPU_GET_INFO_NAME( mb8844 )
CPU_GET_INFO( mb86233 );
#define CPU_MB86233 CPU_GET_INFO_NAME( mb86233 )
CPU_GET_INFO( ssp1601 );
#define CPU_SSP1601 CPU_GET_INFO_NAME( ssp1601 )
CPU_GET_INFO( minx );
#define CPU_MINX CPU_GET_INFO_NAME( minx )
CPU_GET_INFO( cxd8661r );
#define CPU_CXD8661R CPU_GET_INFO_NAME( cxd8661r )



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- global management ----- */

/* reset the internal CPU tracking */
void cpuintrf_init(running_machine *machine);

/* circular string buffer */
char *cpuintrf_temp_str(void);



/* ----- live CPU accessors ----- */

/* return information about a live CPU */
INT64 cpu_get_info_int(const device_config *cpu, UINT32 state);
void *cpu_get_info_ptr(const device_config *cpu, UINT32 state);
genf *cpu_get_info_fct(const device_config *cpu, UINT32 state);
const char *cpu_get_info_string(const device_config *cpu, UINT32 state);

/* set information about a live CPU */
void cpu_set_info_int(const device_config *cpu, UINT32 state, INT64 data);
void cpu_set_info_ptr(const device_config *cpu, UINT32 state, void *data);
void cpu_set_info_fct(const device_config *cpu, UINT32 state, genf *data);

/* return the PC, corrected to a byte offset and translated to physical space, on a given CPU */
offs_t cpu_get_physical_pc_byte(const device_config *cpu);

/* disassemble a line at a given PC on a given CPU */
offs_t cpu_dasm(const device_config *cpu, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

/* set a dasm override handler */
void cpu_set_dasm_override(const device_config *cpu, cpu_disassemble_func dasm_override);



/* ----- CPU type accessors ----- */

/* return information about a given CPU type */
INT64 cputype_get_info_int(cpu_type cputype, UINT32 state);
void *cputype_get_info_ptr(cpu_type cputype, UINT32 state);
genf *cputype_get_info_fct(cpu_type cputype, UINT32 state);
const char *cputype_get_info_string(cpu_type cputype, UINT32 state);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    cpu_get_class_header - return a pointer to
    the class header
-------------------------------------------------*/

INLINE cpu_class_header *cpu_get_class_header(const device_config *device)
{
	if (device->token != NULL)
		return (cpu_class_header *)((UINT8 *)device->token + device->tokenbytes) - 1;
	return NULL;
}


/*-------------------------------------------------
    cpu_get_debug_data - return a pointer to
    the given CPU's debugger data
-------------------------------------------------*/

INLINE cpu_debug_data *cpu_get_debug_data(const device_config *device)
{
	cpu_class_header *classheader = cpu_get_class_header(device);
	return classheader->debug;
}


/*-------------------------------------------------
    cpu_get_address_space - return a pointer to
    the given CPU's address space
-------------------------------------------------*/

INLINE const address_space *cpu_get_address_space(const device_config *device, int spacenum)
{
	/* it is faster to pull this from the class header, but only after we've started */
	if (device->token != NULL)
	{
		cpu_class_header *classheader = cpu_get_class_header(device);
		return classheader->space[spacenum];
	}
	return memory_find_address_space(device, spacenum);
}


/*-------------------------------------------------
    cpu_execute - execute the requested cycles on
    a given CPU
-------------------------------------------------*/

INLINE int cpu_execute(const device_config *device, int cycles)
{
	cpu_class_header *classheader = cpu_get_class_header(device);
	return (*classheader->execute)(device, cycles);
}


#endif	/* __CPUINTRF_H__ */
