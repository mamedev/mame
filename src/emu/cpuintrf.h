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
#include <stddef.h>


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
	MAX_REGS = 256,

	REG_GENPCBASE = MAX_REGS - 1,
	REG_GENPC = MAX_REGS - 2,
	REG_GENSP = MAX_REGS - 3
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

		CPUINFO_INT_INPUT_STATE,							/* R/W: states for each input line */
		CPUINFO_INT_INPUT_STATE_LAST = CPUINFO_INT_INPUT_STATE + MAX_INPUT_LINES - 1,
		CPUINFO_INT_OUTPUT_STATE,							/* R/W: states for each output line */
		CPUINFO_INT_OUTPUT_STATE_LAST = CPUINFO_INT_OUTPUT_STATE + MAX_OUTPUT_LINES - 1,
		CPUINFO_INT_REGISTER,								/* R/W: values of up to MAX_REGs registers */
		CPUINFO_INT_SP = CPUINFO_INT_REGISTER + REG_GENSP,		/* R/W: the current stack pointer value */
		CPUINFO_INT_PC = CPUINFO_INT_REGISTER + REG_GENPC,		/* R/W: the current PC value */
		CPUINFO_INT_PREVIOUSPC = CPUINFO_INT_REGISTER + REG_GENPCBASE,	/* R/W: the previous PC value */
		CPUINFO_INT_REGISTER_LAST = CPUINFO_INT_REGISTER + MAX_REGS - 1,

	CPUINFO_INT_CPU_SPECIFIC = 0x08000,						/* R/W: CPU-specific values start here */

	/* --- the following bits of info are returned as pointers to data or functions --- */
	CPUINFO_PTR_FIRST = DEVINFO_PTR_FIRST,

		/* CPU-specific additions */
		CPUINFO_PTR_INSTRUCTION_COUNTER = DEVINFO_PTR_CLASS_SPECIFIC,
															/* R/O: int *icount */
		CPUINFO_PTR_INTERNAL_MEMORY_MAP,					/* R/O: const addrmap_token *map */
		CPUINFO_PTR_INTERNAL_MEMORY_MAP_LAST = CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACES - 1,
		CPUINFO_PTR_STATE_TABLE,							/* R/O: cpu_state_table *state */

	CPUINFO_PTR_CPU_SPECIFIC = DEVINFO_PTR_DEVICE_SPECIFIC,	/* R/W: CPU-specific values start here */

	/* --- the following bits of info are returned as pointers to functions --- */
	CPUINFO_FCT_FIRST = DEVINFO_FCT_FIRST,

		/* CPU-specific additions */
		CPUINFO_PTR_SET_INFO = DEVINFO_FCT_CLASS_SPECIFIC,	/* R/O: void (*set_info)(const device_config *device, UINT32 state, INT64 data, void *ptr) */
		CPUINFO_PTR_INIT,									/* R/O: void (*init)(const device_config *device, int index, int clock, int (*irqcallback)(const device_config *device, int)) */
		CPUINFO_PTR_RESET,									/* R/O: void (*reset)(const device_config *device) */
		CPUINFO_PTR_EXIT,									/* R/O: void (*exit)(const device_config *device) */
		CPUINFO_PTR_EXECUTE,								/* R/O: int (*execute)(const device_config *device, int cycles) */
		CPUINFO_PTR_BURN,									/* R/O: void (*burn)(const device_config *device, int cycles) */
		CPUINFO_PTR_DISASSEMBLE,							/* R/O: offs_t (*disassemble)(const device_config *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram) */
		CPUINFO_PTR_TRANSLATE,								/* R/O: int (*translate)(const device_config *device, int space, int intention, offs_t *address) */
		CPUINFO_PTR_READ,									/* R/O: int (*read)(const device_config *device, int space, UINT32 offset, int size, UINT64 *value) */
		CPUINFO_PTR_WRITE,									/* R/O: int (*write)(const device_config *device, int space, UINT32 offset, int size, UINT64 value) */
		CPUINFO_PTR_READOP,									/* R/O: int (*readop)(const device_config *device, UINT32 offset, int size, UINT64 *value) */
		CPUINFO_PTR_DEBUG_INIT,								/* R/O: void (*debug_init)(const device_config *device) */
		CPUINFO_PTR_VALIDITY_CHECK,							/* R/O: int (*validity_check)(const game_driver *driver, const void *config) */
		CPUINFO_FCT_IMPORT_STATE,							/* R/O: void (*import_state)(const device_config *device, void *baseptr, const cpu_state_entry *entry) */
		CPUINFO_FCT_EXPORT_STATE,							/* R/O: void (*export_state)(const device_config *device, void *baseptr, const cpu_state_entry *entry) */
		CPUINFO_FCT_IMPORT_STRING,							/* R/O: void (*import_string)(const device_config *device, void *baseptr, const cpu_state_entry *entry, const char *format, char *string) */
		CPUINFO_FCT_EXPORT_STRING,							/* R/O: void (*export_string)(const device_config *device, void *baseptr, const cpu_state_entry *entry, const char *format, char *string) */

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


/* state table flags */
#define CPUSTATE_NOSHOW			0x01		/* don't display this entry in the registers view */
#define CPUSTATE_IMPORT			0x02		/* call the import function after writing new data */
#define CPUSTATE_IMPORT_SEXT	0x04		/* sign-extend the data when writing new data */
#define CPUSTATE_EXPORT			0x08		/* call the export function prior to fetching the data */



/***************************************************************************
    MACROS
***************************************************************************/

#define IRQ_CALLBACK(func)			int func(const device_config *device, int irqline)

#define CPU_GET_INFO_NAME(name)			cpu_get_info_##name
#define CPU_GET_INFO(name)				void CPU_GET_INFO_NAME(name)(const device_config *device, UINT32 state, cpuinfo *info)
#define CPU_GET_INFO_CALL(name)			CPU_GET_INFO_NAME(name)(device, state, info)

#define CPU_SET_INFO_NAME(name)			cpu_set_info_##name
#define CPU_SET_INFO(name)				void CPU_SET_INFO_NAME(name)(const device_config *device, UINT32 state, cpuinfo *info)
#define CPU_SET_INFO_CALL(name)			CPU_SET_INFO_NAME(name)(device, state, info)

#define CPU_INIT_NAME(name)				cpu_init_##name
#define CPU_INIT(name)					void CPU_INIT_NAME(name)(const device_config *device, cpu_irq_callback irqcallback)
#define CPU_INIT_CALL(name)				CPU_INIT_NAME(name)(device, irqcallback)

#define CPU_RESET_NAME(name)			cpu_reset_##name
#define CPU_RESET(name)					void CPU_RESET_NAME(name)(const device_config *device)
#define CPU_RESET_CALL(name)			CPU_RESET_NAME(name)(device)

#define CPU_EXIT_NAME(name)				cpu_exit_##name
#define CPU_EXIT(name)					void CPU_EXIT_NAME(name)(const device_config *device)
#define CPU_EXIT_CALL(name)				CPU_EXIT_NAME(name)(device)

#define CPU_EXECUTE_NAME(name)			cpu_execute_##name
#define CPU_EXECUTE(name)				int CPU_EXECUTE_NAME(name)(const device_config *device, int cycles)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(device, cycles)

#define CPU_BURN_NAME(name)				cpu_burn_##name
#define CPU_BURN(name)					void CPU_BURN_NAME(name)(const device_config *device, int cycles)
#define CPU_BURN_CALL(name)				CPU_BURN_NAME(name)(device, cycles)

#define CPU_TRANSLATE_NAME(name)		cpu_translate_##name
#define CPU_TRANSLATE(name)				int CPU_TRANSLATE_NAME(name)(const device_config *device, int space, int intention, offs_t *address)
#define CPU_TRANSLATE_CALL(name)		CPU_TRANSLATE_NAME(name)(device, space, intention, address)

#define CPU_READ_NAME(name)				cpu_read_##name
#define CPU_READ(name)					int CPU_READ_NAME(name)(const device_config *device, int space, UINT32 offset, int size, UINT64 *value)
#define CPU_READ_CALL(name)				CPU_READ_NAME(name)(device, space, offset, size, value)

#define CPU_WRITE_NAME(name)			cpu_write_##name
#define CPU_WRITE(name)					int CPU_WRITE_NAME(name)(const device_config *device, int space, UINT32 offset, int size, UINT64 value)
#define CPU_WRITE_CALL(name)			CPU_WRITE_NAME(name)(device, space, offset, size, value)

#define CPU_READOP_NAME(name)			cpu_readop_##name
#define CPU_READOP(name)				int CPU_READOP_NAME(name)(const device_config *device, UINT32 offset, int size, UINT64 *value)
#define CPU_READOP_CALL(name)			CPU_READOP_NAME(name)(device, offset, size, value)

#define CPU_DEBUG_INIT_NAME(name) 		cpu_debug_init_##name
#define CPU_DEBUG_INIT(name)			void CPU_DEBUG_INIT_NAME(name)(const device_config *device)
#define CPU_DEBUG_INIT_CALL(name)		CPU_DEBUG_INIT_NAME(name)(device)

#define CPU_DISASSEMBLE_NAME(name)		cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)			offs_t CPU_DISASSEMBLE_NAME(name)(const device_config *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(device, buffer, pc, oprom, opram)

#define CPU_VALIDITY_CHECK_NAME(name)	cpu_validity_check_##name
#define CPU_VALIDITY_CHECK(name)		int CPU_VALIDITY_CHECK_NAME(name)(const game_driver *driver, const void *config)
#define CPU_VALIDITY_CHECK_CALL(name)	CPU_VALIDITY_CHECK_NAME(name)(driver, config)

#define CPU_IMPORT_STATE_NAME(name)		cpu_state_import_##name
#define CPU_IMPORT_STATE(name)			void CPU_IMPORT_STATE_NAME(name)(const device_config *device, void *baseptr, const cpu_state_entry *entry)
#define CPU_IMPORT_STATE_CALL(name)		CPU_IMPORT_STATE_NAME(name)(device, baseptr, entry)

#define CPU_EXPORT_STATE_NAME(name)		cpu_state_export_##name
#define CPU_EXPORT_STATE(name)			void CPU_EXPORT_STATE_NAME(name)(const device_config *device, void *baseptr, const cpu_state_entry *entry)
#define CPU_EXPORT_STATE_CALL(name)		CPU_EXPORT_STATE_NAME(name)(device, baseptr, entry)

#define CPU_IMPORT_STRING_NAME(name)	cpu_string_import_##name
#define CPU_IMPORT_STRING(name)			void CPU_IMPORT_STRING_NAME(name)(const device_config *device, void *baseptr, const cpu_state_entry *entry, char *string)
#define CPU_IMPORT_STRING_CALL(name)	CPU_IMPORT_STRING_NAME(name)(device, baseptr, entry, string)

#define CPU_EXPORT_STRING_NAME(name)	cpu_string_export_##name
#define CPU_EXPORT_STRING(name)			void CPU_EXPORT_STRING_NAME(name)(const device_config *device, void *baseptr, const cpu_state_entry *entry, char *string)
#define CPU_EXPORT_STRING_CALL(name)	CPU_EXPORT_STRING_NAME(name)(device, baseptr, entry, string)



#define cpu_get_index(cpu)					device_list_index((cpu)->machine->config->devicelist, CPU, (cpu)->tag)


/* helpers for accessing common CPU state */
#define cpu_get_context_size(cpu)			device_get_info_int(cpu, CPUINFO_INT_CONTEXT_SIZE)
#define cpu_get_input_lines(cpu)			device_get_info_int(cpu, CPUINFO_INT_INPUT_LINES)
#define cpu_get_output_lines(cpu)			device_get_info_int(cpu, CPUINFO_INT_OUTPUT_LINES)
#define cpu_get_default_irq_vector(cpu)		device_get_info_int(cpu, CPUINFO_INT_DEFAULT_IRQ_VECTOR)
#define cpu_get_endianness(cpu)				device_get_info_int(cpu, CPUINFO_INT_ENDIANNESS)
#define cpu_get_clock_multiplier(cpu)		device_get_info_int(cpu, CPUINFO_INT_CLOCK_MULTIPLIER)
#define cpu_get_clock_divider(cpu)			device_get_info_int(cpu, CPUINFO_INT_CLOCK_DIVIDER)
#define cpu_get_min_opcode_bytes(cpu)		device_get_info_int(cpu, CPUINFO_INT_MIN_INSTRUCTION_BYTES)
#define cpu_get_max_opcode_bytes(cpu)		device_get_info_int(cpu, CPUINFO_INT_MAX_INSTRUCTION_BYTES)
#define cpu_get_min_cycles(cpu)				device_get_info_int(cpu, CPUINFO_INT_MIN_CYCLES)
#define cpu_get_max_cycles(cpu)				device_get_info_int(cpu, CPUINFO_INT_MAX_CYCLES)
#define cpu_get_databus_width(cpu, space)	device_get_info_int(cpu, CPUINFO_INT_DATABUS_WIDTH + (space))
#define cpu_get_addrbus_width(cpu, space)	device_get_info_int(cpu, CPUINFO_INT_ADDRBUS_WIDTH + (space))
#define cpu_get_addrbus_shift(cpu, space)	device_get_info_int(cpu, CPUINFO_INT_ADDRBUS_SHIFT + (space))
#define cpu_get_logaddr_width(cpu, space)	device_get_info_int(cpu, CPUINFO_INT_LOGADDR_WIDTH + (space))
#define cpu_get_page_shift(cpu, space)		device_get_info_int(cpu, CPUINFO_INT_PAGE_SHIFT + (space))
#define cpu_get_reg(cpu, reg)				device_get_info_int(cpu, CPUINFO_INT_REGISTER + (reg))
#define	cpu_get_previouspc(cpu)				((offs_t)cpu_get_reg(cpu, REG_GENPCBASE))
#define	cpu_get_pc(cpu)						((offs_t)cpu_get_reg(cpu, REG_GENPC))
#define	cpu_get_sp(cpu)						cpu_get_reg(cpu, REG_GENSP)
#define cpu_get_icount_ptr(cpu)				(int *)device_get_info_ptr(cpu, CPUINFO_PTR_INSTRUCTION_COUNTER)
#define cpu_get_state_table(cpu)			(const cpu_state_table *)device_get_info_ptr(cpu, CPUINFO_PTR_STATE_TABLE)
#define cpu_get_name(cpu)					device_get_info_string(cpu, CPUINFO_STR_NAME)
#define cpu_get_core_family(cpu)			device_get_info_string(cpu, CPUINFO_STR_CORE_FAMILY)
#define cpu_get_core_version(cpu)			device_get_info_string(cpu, CPUINFO_STR_CORE_VERSION)
#define cpu_get_core_file(cpu)				device_get_info_string(cpu, CPUINFO_STR_CORE_FILE)
#define cpu_get_core_credits(cpu)			device_get_info_string(cpu, CPUINFO_STR_CORE_CREDITS)
#define cpu_get_flags_string(cpu)			device_get_info_string(cpu, CPUINFO_STR_FLAGS)
#define cpu_get_irq_string(cpu, irq)		device_get_info_string(cpu, CPUINFO_STR_IRQ_STATE + (irq))
#define cpu_get_reg_string(cpu, reg)		device_get_info_string(cpu, CPUINFO_STR_REGISTER + (reg))
#define cpu_set_reg(cpu, reg, val)			device_set_info_int(cpu, CPUINFO_INT_REGISTER + (reg), (val))


/* helpers for using machine/cputag instead of cpu objects */
#define cputag_reset(mach, tag)						device_reset(cputag_get_cpu(mach, tag))
#define cputag_get_index(mach, tag)					cpu_get_index(cputag_get_cpu(mach, tag))
#define cputag_get_address_space(mach, tag, space)	cpu_get_address_space(cputag_get_cpu(mach, tag), space)


#define CPU_STATE_ENTRY(_index, _symbol, _format, _struct, _member, _datamask, _validmask, _flags) \
	{ _index, _validmask, offsetof(_struct, _member), _datamask, sizeof(((_struct *)0)->_member), _flags, _symbol, _format },



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* opaque definition of CPU internal and debugging info */
typedef struct _cpu_debug_data cpu_debug_data;


/* forward declaration of types */
typedef union _cpuinfo cpuinfo;
typedef struct _cpu_state_entry cpu_state_entry;


/* define the various callback functions */
typedef int (*cpu_irq_callback)(const device_config *device, int irqnum);

typedef void (*cpu_get_info_func)(const device_config *device, UINT32 state, cpuinfo *info);
typedef void (*cpu_set_info_func)(const device_config *device, UINT32 state, cpuinfo *info);
typedef void (*cpu_init_func)(const device_config *device, cpu_irq_callback irqcallback);
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
typedef void (*cpu_state_io_func)(const device_config *device, void *baseptr, const cpu_state_entry *entry);
typedef void (*cpu_string_io_func)(const device_config *device, void *baseptr, const cpu_state_entry *entry, char *string);


/* structure describing a single item of exposed CPU state */
struct _cpu_state_entry
{
	UINT32					index;						/* state index this entry applies to */
	UINT32					validmask;					/* mask for which CPU subtypes this entry is valid */
	FPTR					dataoffs;					/* offset to the data, relative to the baseptr */
	UINT64					mask;						/* mask applied to the data */
	UINT8					datasize;					/* size of the data item in memory */
	UINT8					flags;						/* flags */
	const char *			symbol;						/* symbol for display; all lower-case version for expressions */
	const char *			format;						/* supported formats */
};


/* structure describing a table of exposed CPU state */
typedef struct _cpu_state_table cpu_state_table;
struct _cpu_state_table
{
	void *					baseptr;					/* pointer to the base of state (offsets are relative to this) */
	UINT32					subtypemask;				/* mask of subtypes that apply to this CPU */
	UINT32					entrycount;					/* number of entries */
	const cpu_state_entry *	entrylist;					/* array of entries */
};


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
	cpu_state_io_func		import_state;				/* CPUINFO_FCT_IMPORT_STATE */
	cpu_state_io_func		export_state;				/* CPUINFO_FCT_EXPORT_STATE */
	cpu_string_io_func 		import_string;				/* CPUINFO_FCT_IMPORT_STRING */
	cpu_string_io_func 		export_string;				/* CPUINFO_FCT_EXPORT_STRING */
	int *					icount;						/* CPUINFO_PTR_INSTRUCTION_COUNTER */
	const cpu_state_table *	state_table;				/* CPUINFO_PTR_STATE_TABLE */
	const addrmap8_token *	internal_map8;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap16_token *	internal_map16;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap32_token *	internal_map32;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap64_token *	internal_map64;				/* CPUINFO_PTR_INTERNAL_MEMORY_MAP */
};


/* partial data hanging off of the classtoken */
typedef struct _cpu_class_header cpu_class_header;
struct _cpu_class_header
{
	cpu_debug_data *		debug;						/* debugging data */
	const address_space *	space[ADDRESS_SPACES];		/* address spaces */

	/* table of core functions */
	cpu_set_info_func		set_info;
	cpu_execute_func		execute;
	cpu_burn_func			burn;
	cpu_translate_func		translate;
	cpu_disassemble_func	disassemble;
	cpu_disassemble_func 	dasm_override;
};



/***************************************************************************
    CPU DEFINITIONS
***************************************************************************/

#define cpu_count(config)		device_list_items((config)->devicelist, CPU)
#define cpu_first(config)		device_list_first((config)->devicelist, CPU)
#define cpu_next(previous)		device_list_next((previous), CPU)


CPU_GET_INFO( dummy );
#define CPU_DUMMY CPU_GET_INFO_NAME( dummy )


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- live CPU accessors ----- */

/* return the PC, corrected to a byte offset and translated to physical space, on a given CPU */
offs_t cpu_get_physical_pc_byte(const device_config *cpu);

/* disassemble a line at a given PC on a given CPU */
offs_t cpu_dasm(const device_config *cpu, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

/* set a dasm override handler */
void cpu_set_dasm_override(const device_config *cpu, cpu_disassemble_func dasm_override);



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
