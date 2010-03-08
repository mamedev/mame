/***************************************************************************

    cpuintrf.h

    Core CPU interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __CPUINTRF_H__
#define __CPUINTRF_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_INPUT_EVENTS		32


/* alternate address space names for common use */
enum
{
	ADDRESS_SPACE_PROGRAM = ADDRESS_SPACE_0,	/* program address space */
	ADDRESS_SPACE_DATA = ADDRESS_SPACE_1,		/* data address space */
	ADDRESS_SPACE_IO = ADDRESS_SPACE_2			/* I/O address space */
};


/* I/O line states */
enum
{
	CLEAR_LINE = 0,				/* clear (a fired or held) line */
	ASSERT_LINE,				/* assert an interrupt immediately */
	HOLD_LINE,					/* hold interrupt line until acknowledged */
	PULSE_LINE					/* pulse interrupt line instantaneously (only for NMI, RESET) */
};


/* I/O line definitions */
enum
{
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


/* register definitions */
enum
{
	MAX_REGS = 256,

	REG_GENPCBASE = MAX_REGS - 1,	/* generic "base" PC, should point to start of current opcode */
	REG_GENPC = MAX_REGS - 2,		/* generic PC, may point within an opcode */
	REG_GENSP = MAX_REGS - 3		/* generic SP, or closest equivalent */
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
		CPUINFO_INT_CLOCK_MULTIPLIER,						/* R/O: internal clock multiplier */
		CPUINFO_INT_CLOCK_DIVIDER,							/* R/O: internal clock divider */
		CPUINFO_INT_MIN_INSTRUCTION_BYTES,					/* R/O: minimum bytes per instruction */
		CPUINFO_INT_MAX_INSTRUCTION_BYTES,					/* R/O: maximum bytes per instruction */
		CPUINFO_INT_MIN_CYCLES,								/* R/O: minimum cycles for a single instruction */
		CPUINFO_INT_MAX_CYCLES,								/* R/O: maximum cycles for a single instruction */

		CPUINFO_INT_LOGADDR_WIDTH,							/* R/O: address bus size for logical accesses in each space (0=same as physical) */
		CPUINFO_INT_LOGADDR_WIDTH_PROGRAM = CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACE_PROGRAM,
		CPUINFO_INT_LOGADDR_WIDTH_DATA = CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACE_DATA,
		CPUINFO_INT_LOGADDR_WIDTH_IO = CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACE_IO,
		CPUINFO_INT_LOGADDR_WIDTH_LAST = CPUINFO_INT_LOGADDR_WIDTH + ADDRESS_SPACES - 1,
		CPUINFO_INT_PAGE_SHIFT,								/* R/O: size of a page log 2 (i.e., 12=4096), or 0 if paging not supported */
		CPUINFO_INT_PAGE_SHIFT_PROGRAM = CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACE_PROGRAM,
		CPUINFO_INT_PAGE_SHIFT_DATA = CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACE_DATA,
		CPUINFO_INT_PAGE_SHIFT_IO = CPUINFO_INT_PAGE_SHIFT + ADDRESS_SPACE_IO,
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
		CPUINFO_PTR_STATE_TABLE,							/* R/O: cpu_state_table *state */

	CPUINFO_PTR_CPU_SPECIFIC = DEVINFO_PTR_DEVICE_SPECIFIC,	/* R/W: CPU-specific values start here */

	/* --- the following bits of info are returned as pointers to functions --- */
	CPUINFO_FCT_FIRST = DEVINFO_FCT_FIRST,

		/* CPU-specific additions */
		CPUINFO_FCT_SET_INFO = DEVINFO_FCT_CLASS_SPECIFIC,	/* R/O: void (*set_info)(running_device *device, UINT32 state, INT64 data, void *ptr) */
		CPUINFO_FCT_INIT,									/* R/O: void (*init)(running_device *device, int index, int clock, int (*irqcallback)(running_device *device, int)) */
		CPUINFO_FCT_RESET,									/* R/O: void (*reset)(running_device *device) */
		CPUINFO_FCT_EXIT,									/* R/O: void (*exit)(running_device *device) */
		CPUINFO_FCT_EXECUTE,								/* R/O: int (*execute)(running_device *device, int cycles) */
		CPUINFO_FCT_BURN,									/* R/O: void (*burn)(running_device *device, int cycles) */
		CPUINFO_FCT_DISASSEMBLE,							/* R/O: offs_t (*disassemble)(running_device *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options) */
		CPUINFO_FCT_TRANSLATE,								/* R/O: int (*translate)(running_device *device, int space, int intention, offs_t *address) */
		CPUINFO_FCT_READ,									/* R/O: int (*read)(running_device *device, int space, UINT32 offset, int size, UINT64 *value) */
		CPUINFO_FCT_WRITE,									/* R/O: int (*write)(running_device *device, int space, UINT32 offset, int size, UINT64 value) */
		CPUINFO_FCT_READOP,									/* R/O: int (*readop)(running_device *device, UINT32 offset, int size, UINT64 *value) */
		CPUINFO_FCT_DEBUG_INIT,								/* R/O: void (*debug_init)(running_device *device) */
		CPUINFO_FCT_VALIDITY_CHECK,							/* R/O: int (*validity_check)(const game_driver *driver, const void *config) */
		CPUINFO_FCT_IMPORT_STATE,							/* R/O: void (*import_state)(running_device *device, void *baseptr, const cpu_state_entry *entry) */
		CPUINFO_FCT_EXPORT_STATE,							/* R/O: void (*export_state)(running_device *device, void *baseptr, const cpu_state_entry *entry) */
		CPUINFO_FCT_IMPORT_STRING,							/* R/O: void (*import_string)(running_device *device, void *baseptr, const cpu_state_entry *entry, const char *format, char *string) */
		CPUINFO_FCT_EXPORT_STRING,							/* R/O: void (*export_string)(running_device *device, void *baseptr, const cpu_state_entry *entry, const char *format, char *string) */

	CPUINFO_FCT_CPU_SPECIFIC = DEVINFO_FCT_DEVICE_SPECIFIC,	/* R/W: CPU-specific values start here */

	/* --- the following bits of info are returned as NULL-terminated strings --- */
	CPUINFO_STR_FIRST = DEVINFO_STR_FIRST,

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

/* device iteration helpers */
#define cpu_count(config)				(config)->devicelist.count(CPU)
#define cpu_first(config)				(config)->devicelist.first(CPU)
#define cpu_next(previous)				(previous)->typenext()
#define cpu_get_index(cpu)				(cpu)->machine->devicelist.index(CPU, (cpu)->tag())


/* IRQ callback to be called by CPU cores when an IRQ is actually taken */
#define IRQ_CALLBACK(func)				int func(running_device *device, int irqline)


/* CPU interface functions */
#define CPU_GET_INFO_NAME(name)			cpu_get_info_##name
#define CPU_GET_INFO(name)				void CPU_GET_INFO_NAME(name)(const device_config *devconfig, running_device *device, UINT32 state, cpuinfo *info)
#define CPU_GET_INFO_CALL(name)			CPU_GET_INFO_NAME(name)(devconfig, device, state, info)

#define CPU_SET_INFO_NAME(name)			cpu_set_info_##name
#define CPU_SET_INFO(name)				void CPU_SET_INFO_NAME(name)(running_device *device, UINT32 state, cpuinfo *info)
#define CPU_SET_INFO_CALL(name)			CPU_SET_INFO_NAME(name)(device, state, info)

#define CPU_INIT_NAME(name)				cpu_init_##name
#define CPU_INIT(name)					void CPU_INIT_NAME(name)(running_device *device, cpu_irq_callback irqcallback)
#define CPU_INIT_CALL(name)				CPU_INIT_NAME(name)(device, irqcallback)

#define CPU_RESET_NAME(name)			cpu_reset_##name
#define CPU_RESET(name)					void CPU_RESET_NAME(name)(running_device *device)
#define CPU_RESET_CALL(name)			CPU_RESET_NAME(name)(device)

#define CPU_EXIT_NAME(name)				cpu_exit_##name
#define CPU_EXIT(name)					void CPU_EXIT_NAME(name)(running_device *device)
#define CPU_EXIT_CALL(name)				CPU_EXIT_NAME(name)(device)

#define CPU_EXECUTE_NAME(name)			cpu_execute_##name
#define CPU_EXECUTE(name)				int CPU_EXECUTE_NAME(name)(running_device *device, int cycles)
#define CPU_EXECUTE_CALL(name)			CPU_EXECUTE_NAME(name)(device, cycles)

#define CPU_BURN_NAME(name)				cpu_burn_##name
#define CPU_BURN(name)					void CPU_BURN_NAME(name)(running_device *device, int cycles)
#define CPU_BURN_CALL(name)				CPU_BURN_NAME(name)(device, cycles)

#define CPU_TRANSLATE_NAME(name)		cpu_translate_##name
#define CPU_TRANSLATE(name)				int CPU_TRANSLATE_NAME(name)(running_device *device, int space, int intention, offs_t *address)
#define CPU_TRANSLATE_CALL(name)		CPU_TRANSLATE_NAME(name)(device, space, intention, address)

#define CPU_READ_NAME(name)				cpu_read_##name
#define CPU_READ(name)					int CPU_READ_NAME(name)(running_device *device, int space, UINT32 offset, int size, UINT64 *value)
#define CPU_READ_CALL(name)				CPU_READ_NAME(name)(device, space, offset, size, value)

#define CPU_WRITE_NAME(name)			cpu_write_##name
#define CPU_WRITE(name)					int CPU_WRITE_NAME(name)(running_device *device, int space, UINT32 offset, int size, UINT64 value)
#define CPU_WRITE_CALL(name)			CPU_WRITE_NAME(name)(device, space, offset, size, value)

#define CPU_READOP_NAME(name)			cpu_readop_##name
#define CPU_READOP(name)				int CPU_READOP_NAME(name)(running_device *device, UINT32 offset, int size, UINT64 *value)
#define CPU_READOP_CALL(name)			CPU_READOP_NAME(name)(device, offset, size, value)

#define CPU_DEBUG_INIT_NAME(name)		cpu_debug_init_##name
#define CPU_DEBUG_INIT(name)			void CPU_DEBUG_INIT_NAME(name)(running_device *device)
#define CPU_DEBUG_INIT_CALL(name)		CPU_DEBUG_INIT_NAME(name)(device)

#define CPU_DISASSEMBLE_NAME(name)		cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)			offs_t CPU_DISASSEMBLE_NAME(name)(running_device *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options)
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(device, buffer, pc, oprom, opram, options)

#define CPU_VALIDITY_CHECK_NAME(name)	cpu_validity_check_##name
#define CPU_VALIDITY_CHECK(name)		int CPU_VALIDITY_CHECK_NAME(name)(const game_driver *driver, const void *config)
#define CPU_VALIDITY_CHECK_CALL(name)	CPU_VALIDITY_CHECK_NAME(name)(driver, config)

#define CPU_IMPORT_STATE_NAME(name)		cpu_state_import_##name
#define CPU_IMPORT_STATE(name)			void CPU_IMPORT_STATE_NAME(name)(running_device *device, void *baseptr, const cpu_state_entry *entry)
#define CPU_IMPORT_STATE_CALL(name)		CPU_IMPORT_STATE_NAME(name)(device, baseptr, entry)

#define CPU_EXPORT_STATE_NAME(name)		cpu_state_export_##name
#define CPU_EXPORT_STATE(name)			void CPU_EXPORT_STATE_NAME(name)(running_device *device, void *baseptr, const cpu_state_entry *entry)
#define CPU_EXPORT_STATE_CALL(name)		CPU_EXPORT_STATE_NAME(name)(device, baseptr, entry)

#define CPU_IMPORT_STRING_NAME(name)	cpu_string_import_##name
#define CPU_IMPORT_STRING(name)			void CPU_IMPORT_STRING_NAME(name)(running_device *device, void *baseptr, const cpu_state_entry *entry, char *string)
#define CPU_IMPORT_STRING_CALL(name)	CPU_IMPORT_STRING_NAME(name)(device, baseptr, entry, string)

#define CPU_EXPORT_STRING_NAME(name)	cpu_string_export_##name
#define CPU_EXPORT_STRING(name)			void CPU_EXPORT_STRING_NAME(name)(running_device *device, void *baseptr, const cpu_state_entry *entry, char *string)
#define CPU_EXPORT_STRING_CALL(name)	CPU_EXPORT_STRING_NAME(name)(device, baseptr, entry, string)


/* base macro for defining CPU state entries */
#define CPU_STATE_ENTRY(_index, _symbol, _format, _struct, _member, _datamask, _validmask, _flags) \
	{ _index, _validmask, offsetof(_struct, _member), _datamask, sizeof(((_struct *)0)->_member), _flags, _symbol, _format },


/* helpers for accessing common CPU state */
#define cpu_get_context_size(cpu)			(cpu)->get_config_int(CPUINFO_INT_CONTEXT_SIZE)
#define cpu_get_input_lines(cpu)			(cpu)->get_config_int(CPUINFO_INT_INPUT_LINES)
#define cpu_get_output_lines(cpu)			(cpu)->get_config_int(CPUINFO_INT_OUTPUT_LINES)
#define cpu_get_default_irq_vector(cpu)		(cpu)->get_config_int(CPUINFO_INT_DEFAULT_IRQ_VECTOR)
#define cpu_get_clock_multiplier(cpu)		(cpu)->get_config_int(CPUINFO_INT_CLOCK_MULTIPLIER)
#define cpu_get_clock_divider(cpu)			(cpu)->get_config_int(CPUINFO_INT_CLOCK_DIVIDER)
#define cpu_get_min_opcode_bytes(cpu)		(cpu)->get_config_int(CPUINFO_INT_MIN_INSTRUCTION_BYTES)
#define cpu_get_max_opcode_bytes(cpu)		(cpu)->get_config_int(CPUINFO_INT_MAX_INSTRUCTION_BYTES)
#define cpu_get_min_cycles(cpu)				(cpu)->get_config_int(CPUINFO_INT_MIN_CYCLES)
#define cpu_get_max_cycles(cpu)				(cpu)->get_config_int(CPUINFO_INT_MAX_CYCLES)
#define cpu_get_logaddr_width(cpu, space)	(cpu)->get_config_int(CPUINFO_INT_LOGADDR_WIDTH + (space))
#define cpu_get_page_shift(cpu, space)		(cpu)->get_config_int(CPUINFO_INT_PAGE_SHIFT + (space))
#define cpu_get_reg(cpu, reg)				(cpu)->get_runtime_int(CPUINFO_INT_REGISTER + (reg))
#define	cpu_get_previouspc(cpu)				((offs_t)cpu_get_reg(cpu, REG_GENPCBASE))
#define	cpu_get_pc(cpu)						((offs_t)cpu_get_reg(cpu, REG_GENPC))
#define	cpu_get_sp(cpu)						cpu_get_reg(cpu, REG_GENSP)
#define cpu_get_icount_ptr(cpu)				(int *)(cpu)->get_runtime_ptr(CPUINFO_PTR_INSTRUCTION_COUNTER)
#define cpu_get_state_table(cpu)			(const cpu_state_table *)(cpu)->get_runtime_ptr(CPUINFO_PTR_STATE_TABLE)
#define cpu_get_flags_string(cpu)			(cpu)->get_runtime_string(CPUINFO_STR_FLAGS)
#define cpu_get_irq_string(cpu, irq)		(cpu)->get_runtime_string(CPUINFO_STR_IRQ_STATE + (irq))
#define cpu_get_reg_string(cpu, reg)		(cpu)->get_runtime_string(CPUINFO_STR_REGISTER + (reg))

#define cpu_set_reg(cpu, reg, val)			cpu_set_info(cpu, CPUINFO_INT_REGISTER + (reg), (val))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* forward declaration of types */
typedef union _cpuinfo cpuinfo;
typedef struct _cpu_state_entry cpu_state_entry;


/* IRQ callback to be called by CPU cores when an IRQ is actually taken */
typedef int (*cpu_irq_callback)(running_device *device, int irqnum);


/* CPU interface functions */
typedef void (*cpu_get_info_func)(const device_config *devconfig, running_device *device, UINT32 state, cpuinfo *info);
typedef void (*cpu_set_info_func)(running_device *device, UINT32 state, cpuinfo *info);
typedef void (*cpu_init_func)(running_device *device, cpu_irq_callback irqcallback);
typedef void (*cpu_reset_func)(running_device *device);
typedef void (*cpu_exit_func)(running_device *device);
typedef int	(*cpu_execute_func)(running_device *device, int cycles);
typedef void (*cpu_burn_func)(running_device *device, int cycles);
typedef int	(*cpu_translate_func)(running_device *device, int space, int intention, offs_t *address);
typedef int	(*cpu_read_func)(running_device *device, int space, UINT32 offset, int size, UINT64 *value);
typedef int	(*cpu_write_func)(running_device *device, int space, UINT32 offset, int size, UINT64 value);
typedef int	(*cpu_readop_func)(running_device *device, UINT32 offset, int size, UINT64 *value);
typedef void (*cpu_debug_init_func)(running_device *device);
typedef offs_t (*cpu_disassemble_func)(running_device *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, int options);
typedef int (*cpu_validity_check_func)(const game_driver *driver, const void *config);
typedef void (*cpu_state_io_func)(running_device *device, void *baseptr, const cpu_state_entry *entry);
typedef void (*cpu_string_io_func)(running_device *device, void *baseptr, const cpu_state_entry *entry, char *string);


/* a cpu_type is just a pointer to the CPU's get_info function */
typedef cpu_get_info_func cpu_type;


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


/* cpuinfo union used to pass data to/from the get_info/set_info functions */
union _cpuinfo
{
	INT64					i;							/* generic integers */
	void *					p;							/* generic pointers */
	genf *  				f;							/* generic function pointers */
	char *					s;							/* generic strings */

	cpu_set_info_func		setinfo;					/* CPUINFO_FCT_SET_INFO */
	cpu_init_func			init;						/* CPUINFO_FCT_INIT */
	cpu_reset_func			reset;						/* CPUINFO_FCT_RESET */
	cpu_exit_func			exit;						/* CPUINFO_FCT_EXIT */
	cpu_execute_func		execute;					/* CPUINFO_FCT_EXECUTE */
	cpu_burn_func			burn;						/* CPUINFO_FCT_BURN */
	cpu_translate_func		translate;					/* CPUINFO_FCT_TRANSLATE */
	cpu_read_func			read;						/* CPUINFO_FCT_READ */
	cpu_write_func			write;						/* CPUINFO_FCT_WRITE */
	cpu_readop_func			readop;						/* CPUINFO_FCT_READOP */
	cpu_debug_init_func		debug_init;					/* CPUINFO_FCT_DEBUG_INIT */
	cpu_disassemble_func	disassemble;				/* CPUINFO_FCT_DISASSEMBLE */
	cpu_validity_check_func	validity_check;				/* CPUINFO_FCT_VALIDITY_CHECK */
	cpu_state_io_func		import_state;				/* CPUINFO_FCT_IMPORT_STATE */
	cpu_state_io_func		export_state;				/* CPUINFO_FCT_EXPORT_STATE */
	cpu_string_io_func		import_string;				/* CPUINFO_FCT_IMPORT_STRING */
	cpu_string_io_func		export_string;				/* CPUINFO_FCT_EXPORT_STRING */
	int *					icount;						/* CPUINFO_PTR_INSTRUCTION_COUNTER */
	const cpu_state_table *	state_table;				/* CPUINFO_PTR_STATE_TABLE */
	const addrmap8_token *	internal_map8;				/* DEVINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap16_token *	internal_map16;				/* DEVINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap32_token *	internal_map32;				/* DEVINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap64_token *	internal_map64;				/* DEVINFO_PTR_INTERNAL_MEMORY_MAP */
	const addrmap8_token *	default_map8;				/* DEVINFO_PTR_DEFAULT_MEMORY_MAP */
	const addrmap16_token *	default_map16;				/* DEVINFO_PTR_DEFAULT_MEMORY_MAP */
	const addrmap32_token *	default_map32;				/* DEVINFO_PTR_DEFAULT_MEMORY_MAP */
	const addrmap64_token *	default_map64;				/* DEVINFO_PTR_DEFAULT_MEMORY_MAP */
};


void cpu_set_info(running_device *device, UINT32 state, UINT64 value);

#endif	/* __CPUINTRF_H__ */
