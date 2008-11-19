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


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_CPU 8
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
	CPUINFO_PTR_INIT,									/* R/O: void (*init)(int index, int clock, int (*irqcallback)(const device_config *device, int)) */
	CPUINFO_PTR_RESET,									/* R/O: void (*reset)(void) */
	CPUINFO_PTR_EXIT,									/* R/O: void (*exit)(void) */
	CPUINFO_PTR_EXECUTE,								/* R/O: int (*execute)(int cycles) */
	CPUINFO_PTR_BURN,									/* R/O: void (*burn)(int cycles) */
	CPUINFO_PTR_DISASSEMBLE,							/* R/O: offs_t (*disassemble)(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram) */
	CPUINFO_PTR_TRANSLATE,								/* R/O: int (*translate)(int space, int intention, offs_t *address) */
	CPUINFO_PTR_READ,									/* R/O: int (*read)(int space, UINT32 offset, int size, UINT64 *value) */
	CPUINFO_PTR_WRITE,									/* R/O: int (*write)(int space, UINT32 offset, int size, UINT64 value) */
	CPUINFO_PTR_READOP,									/* R/O: int (*readop)(UINT32 offset, int size, UINT64 *value) */
	CPUINFO_PTR_DEBUG_INIT,								/* R/O: void (*debug_init)(void) */
	CPUINFO_PTR_INSTRUCTION_COUNTER,					/* R/O: int *icount */
	CPUINFO_PTR_INTERNAL_MEMORY_MAP,					/* R/O: const addrmap_token *map */
	CPUINFO_PTR_INTERNAL_MEMORY_MAP_LAST = CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACES - 1,
	CPUINFO_PTR_DEBUG_REGISTER_LIST,					/* R/O: int *list: list of registers for the debugger */
	CPUINFO_PTR_VALIDITY_CHECK,							/* R/O: int (*validity_check)(const void *config) */

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


/* list of all possible CPUs we might be compiled with */
enum _cpu_type
{
	CPU_DUMMY,
	CPU_Z80,
	CPU_Z180,
	CPU_8080,
	CPU_8085A,
	CPU_M6502,
	CPU_M65C02,
	CPU_M65SC02,
	CPU_M65CE02,
	CPU_M6509,
	CPU_M6510,
	CPU_M6510T,
	CPU_M7501,
	CPU_M8502,
	CPU_N2A03,
	CPU_DECO16,
	CPU_M4510,
	CPU_H6280,
	CPU_I8086,
	CPU_I8088,
	CPU_I80186,
	CPU_I80188,
	CPU_I80286,
	CPU_V20,
	CPU_V25,
	CPU_V30,
	CPU_V33,
	CPU_V35,
	CPU_V60,
	CPU_V70,
	CPU_I8035,
	CPU_I8048,
	CPU_I8648,
	CPU_I8748,
	CPU_MB8884,
	CPU_N7751,
	CPU_I8039,
	CPU_I8049,
	CPU_I8749,
	CPU_M58715,
	CPU_I8041,
	CPU_I8741,
	CPU_I8042,
	CPU_I8242,
	CPU_I8742,
	CPU_I8031,
	CPU_I8032,
	CPU_I8051,
	CPU_I8052,
	CPU_I8751,
	CPU_I8752,
	CPU_I80C31,
	CPU_I80C32,
	CPU_I80C51,
	CPU_I80C52,
	CPU_I87C51,
	CPU_I87C52,
	CPU_AT89C4051,
	CPU_DS5002FP,
	CPU_M6800,
	CPU_M6801,
	CPU_M6802,
	CPU_M6803,
	CPU_M6808,
	CPU_HD63701,
	CPU_NSC8105,
	CPU_M6805,
	CPU_M68705,
	CPU_HD63705,
	CPU_HD6309,
	CPU_M6809,
	CPU_M6809E,
	CPU_KONAMI,
	CPU_M68000,
	CPU_M68008,
	CPU_M68010,
	CPU_M68EC020,
	CPU_M68020,
	CPU_M68040,
	CPU_T11,
	CPU_S2650,
	CPU_TMS34010,
	CPU_TMS34020,
	CPU_TI990_10,
	CPU_TMS9900,
	CPU_TMS9940,
	CPU_TMS9980,
	CPU_TMS9985,
	CPU_TMS9989,
	CPU_TMS9995,
	CPU_TMS99100,
	CPU_TMS99105A,
	CPU_TMS99110A,
	CPU_TMS99000,
	CPU_Z8000,
	CPU_TMS32010,
	CPU_TMS32025,
	CPU_TMS32026,
	CPU_TMS32031,
	CPU_TMS32032,
	CPU_TMS32051,
	CPU_CCPU,
	CPU_ADSP2100,
 	CPU_ADSP2101,
	CPU_ADSP2104,
	CPU_ADSP2105,
	CPU_ADSP2115,
	CPU_ADSP2181,
	CPU_PSXCPU,
	CPU_ASAP,
	CPU_UPD7810,
	CPU_UPD7807,
	CPU_UPD7801,
	CPU_UPD78C05,
	CPU_UPD78C06,
	CPU_JAGUARGPU,
	CPU_JAGUARDSP,
	CPU_CQUESTSND,
	CPU_CQUESTROT,
	CPU_CQUESTLIN,
	CPU_R3000BE,
	CPU_R3000LE,
	CPU_R3041BE,
	CPU_R3041LE,
	CPU_R4600BE,
	CPU_R4600LE,
	CPU_R4650BE,
	CPU_R4650LE,
	CPU_R4700BE,
	CPU_R4700LE,
	CPU_R5000BE,
	CPU_R5000LE,
	CPU_QED5271BE,
	CPU_QED5271LE,
	CPU_RM7000BE,
	CPU_RM7000LE,
	CPU_ARM,
	CPU_ARM7,
	CPU_SH1,
	CPU_SH2,
	CPU_SH4,
	CPU_DSP32C,
	CPU_PIC16C54,
	CPU_PIC16C55,
	CPU_PIC16C56,
	CPU_PIC16C57,
	CPU_PIC16C58,
	CPU_G65816,
	CPU_SPC700,
	CPU_E116T,
	CPU_E116XT,
	CPU_E116XS,
	CPU_E116XSR,
	CPU_E132N,
	CPU_E132T,
	CPU_E132XN,
	CPU_E132XT,
	CPU_E132XS,
	CPU_E132XSR,
	CPU_GMS30C2116,
	CPU_GMS30C2132,
	CPU_GMS30C2216,
	CPU_GMS30C2232,
	CPU_I386,
	CPU_I486,
	CPU_PENTIUM,
	CPU_MEDIAGX,
	CPU_I960,
	CPU_H83002,
	CPU_H83007,
	CPU_H83044,
	CPU_H83344,
	CPU_V810,
	CPU_M37702,
	CPU_M37710,
	CPU_PPC403GA,
	CPU_PPC403GCX,
	CPU_PPC601,
	CPU_PPC602,
	CPU_PPC603,
	CPU_PPC603E,
	CPU_PPC603R,
	CPU_PPC604,
	CPU_MPC8240,
	CPU_SE3208,
	CPU_MC68HC11,
	CPU_ADSP21062,
	CPU_DSP56156,
	CPU_RSP,
	CPU_ALPHA8201,
	CPU_ALPHA8301,
	CPU_CDP1802,
	CPU_COP401,
	CPU_COP410,
	CPU_COP411,
	CPU_COP402,
	CPU_COP420,
	CPU_COP421,
	CPU_COP422,
	CPU_COP404,
	CPU_COP424,
	CPU_COP425,
	CPU_COP426,
	CPU_COP444,
	CPU_COP445,
	CPU_TMP90840,
	CPU_TMP90841,
	CPU_TMP91640,
	CPU_TMP91641,
	CPU_APEXC,
	CPU_CP1610,
	CPU_F8,
	CPU_LH5801,
	CPU_PDP1,
	CPU_SATURN,
	CPU_SC61860,
	CPU_TX0_64KW,
	CPU_TX0_8KW,
	CPU_LR35902,
	CPU_TMS7000,
	CPU_TMS7000_EXL,
	CPU_SM8500,
	CPU_V30MZ,
	CPU_MB8841,
	CPU_MB8842,
	CPU_MB8843,
	CPU_MB8844,
	CPU_MB86233,
	CPU_SSP1601,
	CPU_MINX,
	CPU_CXD8661R,
    CPU_COUNT
};
typedef enum _cpu_type cpu_type;



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

#define CPU_GET_CONTEXT_NAME(name)	cpu_get_context_##name
#define CPU_GET_CONTEXT(name)		void CPU_GET_CONTEXT_NAME(name)(void *dst)
#define CPU_GET_CONTEXT_CALL(name)	CPU_GET_CONTEXT_NAME(name)(buffer)

#define CPU_SET_CONTEXT_NAME(name)	cpu_set_context_##name
#define CPU_SET_CONTEXT(name)		void CPU_SET_CONTEXT_NAME(name)(void *src)
#define CPU_SET_CONTEXT_CALL(name)	CPU_SET_CONTEXT_NAME(name)(buffer)


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
#define cputype_get_page_shift(cputype, space)		cputype_get_info_int(cputype, CPUINFO_INT_PAGE_SHIFT + (space))
#define cputype_get_debug_register_list(cputype)	cputype_get_info_ptr(cputype, CPUINFO_PTR_DEBUG_REGISTER_LIST)
#define cputype_get_name(cputype)					cputype_get_info_string(cputype, CPUINFO_STR_NAME)
#define cputype_get_core_family(cputype)			cputype_get_info_string(cputype, CPUINFO_STR_CORE_FAMILY)
#define cputype_get_core_version(cputype)			cputype_get_info_string(cputype, CPUINFO_STR_CORE_VERSION)
#define cputype_get_core_file(cputype)				cputype_get_info_string(cputype, CPUINFO_STR_CORE_FILE)
#define cputype_get_core_credits(cputype)			cputype_get_info_string(cputype, CPUINFO_STR_CORE_CREDITS)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

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
typedef void (*cpu_get_context_func)(void *buffer);
typedef void (*cpu_set_context_func)(void *buffer);


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
	cpu_type				cputype; 				/* type index of this CPU */
	address_space *			space[ADDRESS_SPACES];	/* address spaces */

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
	cpu_translate_func		translate;
	cpu_disassemble_func	disassemble;
	cpu_disassemble_func 	dasm_override;

	/* other frequently-needed information */
	INT8					address_shift[ADDRESS_SPACES];
	UINT32					clock_divider;
	UINT32					clock_multiplier;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/


/* ----- global management ----- */

/* reset the internal CPU tracking */
void cpuintrf_init(running_machine *machine);

/* circular string buffer */
char *cpuintrf_temp_str(void);



/* ----- live context control ----- */

/* remember the current context and push a new one on the stack */
void cpu_push_context(const device_config *cpu);

/* restore the previously saved context */
void cpu_pop_context(void);

/* return the index of the active CPU (deprecated soon) */
int cpunum_get_active(void);

/* find a CPU in the machine by searching */
int cpu_get_index_slow(const device_config *cpu);



/* ----- live CPU accessors ----- */

/* initialize a live CPU */
void cpu_init(const device_config *cpu, int index, int clock, cpu_irq_callback irqcallback);

/* free a live CPU */
void cpu_exit(const device_config *cpu);

/* return information about a live CPU */
INT64 cpu_get_info_int(const device_config *cpu, UINT32 state);
void *cpu_get_info_ptr(const device_config *cpu, UINT32 state);
genf *cpu_get_info_fct(const device_config *cpu, UINT32 state);
const char *cpu_get_info_string(const device_config *cpu, UINT32 state);

/* set information about a live CPU */
void cpu_set_info_int(const device_config *cpu, UINT32 state, INT64 data);
void cpu_set_info_ptr(const device_config *cpu, UINT32 state, void *data);
void cpu_set_info_fct(const device_config *cpu, UINT32 state, genf *data);

/* execute the requested cycles on a given CPU */
int cpu_execute(const device_config *cpu, int cycles);

/* signal a reset for a given CPU */
void cpu_reset(const device_config *cpu);

/* read a byte from another CPU's memory space */
UINT8 cpu_read_byte(const device_config *cpu, offs_t address);

/* write a byte from another CPU's memory space */
void cpu_write_byte(const device_config *cpu, offs_t address, UINT8 data);

/* return the PC, corrected to a byte offset and translated to physical space, on a given CPU */
offs_t cpu_get_physical_pc_byte(const device_config *cpu);

/* disassemble a line at a given PC on a given CPU */
offs_t cpu_dasm(const device_config *cpu, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

/* set a dasm override handler */
void cpu_set_dasm_override(const device_config *cpu, cpu_disassemble_func dasm_override);



/* ----- CPU type accessors ----- */

/* return a header template for a given CPU type */
const cpu_class_header *cputype_get_header_template(cpu_type cputype);

/* return information about a given CPU type */
INT64 cputype_get_info_int(cpu_type cputype, UINT32 state);
void *cputype_get_info_ptr(cpu_type cputype, UINT32 state);
genf *cputype_get_info_fct(cpu_type cputype, UINT32 state);
const char *cputype_get_info_string(cpu_type cputype, UINT32 state);




/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    safe_cpu_get_pc - return the current PC or ~0
    if the CPU is invalid
-------------------------------------------------*/

INLINE offs_t safe_cpu_get_pc(const device_config *cpu)
{
	return (cpu != NULL) ? cpu_get_pc(cpu) : ~0;
}


/*-------------------------------------------------
    cpu_get_index - return the index of the
    specified CPU (deprecated soon)
-------------------------------------------------*/

INLINE int cpu_get_index(const device_config *cpu)
{
	cpu_class_header *classheader = cpu->classtoken;
	return (classheader != NULL) ? classheader->index : cpu_get_index_slow(cpu);
}


/*-------------------------------------------------
    cpu_get_address_space - return a pointer to
    the given CPU's address space
-------------------------------------------------*/

INLINE const address_space *cpu_get_address_space(const device_config *cpu, int spacenum)
{
	cpu_class_header *classheader = cpu->classtoken;
	return classheader->space[spacenum];
}


/*-------------------------------------------------
    cpu_address_to_byte - convert an address in
    the specified address space to a byte offset
-------------------------------------------------*/

INLINE offs_t cpu_address_to_byte(const device_config *cpu, int space, offs_t address)
{
	cpu_class_header *classheader = cpu->classtoken;
	int shift = classheader->address_shift[space];
	return (shift < 0) ? (address << -shift) : (address >> shift);
}


/*-------------------------------------------------
    cpu_address_to_byte_end - convert an address
    in the specified address space to a byte
    offset specifying the last byte covered by
    the address
-------------------------------------------------*/

INLINE offs_t cpu_address_to_byte_end(const device_config *cpu, int space, offs_t address)
{
	cpu_class_header *classheader = cpu->classtoken;
	int shift = classheader->address_shift[space];
	return (shift < 0) ? ((address << -shift) | ((1 << -shift) - 1)) : (address >> shift);
}


/*-------------------------------------------------
    cpu_byte_to_address - convert a byte offset
    to an address in the specified address space
-------------------------------------------------*/

INLINE offs_t cpu_byte_to_address(const device_config *cpu, int space, offs_t address)
{
	cpu_class_header *classheader = cpu->classtoken;
	int shift = classheader->address_shift[space];
	return (shift < 0) ? (address >> -shift) : (address << shift);
}


/*-------------------------------------------------
    cpu_byte_to_address_end - convert a byte offset
    to an address in the specified address space
    specifying the last address covered by the
    byte
-------------------------------------------------*/

INLINE offs_t cpu_byte_to_address_end(const device_config *cpu, int space, offs_t address)
{
	cpu_class_header *classheader = cpu->classtoken;
	int shift = classheader->address_shift[space];
	return (shift < 0) ? (address >> -shift) : ((address << shift) | ((1 << shift) - 1));
}


/*-------------------------------------------------
    cpu_address_physical - return the physical
    address corresponding to the given logical
    address
-------------------------------------------------*/

INLINE offs_t cpu_address_physical(const device_config *cpu, int space, int intention, offs_t address)
{
	cpu_class_header *classheader = cpu->classtoken;
	if (classheader->translate != NULL)
		(*classheader->translate)(cpu, space, intention, &address);
	return address;
}

#endif	/* __CPUINTRF_H__ */
