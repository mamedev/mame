#include <stdio.h>
#include <stdlib.h>
#include "m68k.h"
#include "m68000.h"

/* global access */

struct m68k_memory_interface m68k_memory_intf;
offs_t m68k_encrypted_opcode_start[MAX_CPU];
offs_t m68k_encrypted_opcode_end[MAX_CPU];


void m68k_set_encrypted_opcode_range(int cpunum, offs_t start, offs_t end)
{
	m68k_encrypted_opcode_start[cpunum] = start;
	m68k_encrypted_opcode_end[cpunum] = end;
}

/****************************************************************************
 * 8-bit data memory interface
 ****************************************************************************/

static UINT16 readword_d8(offs_t address)
{
	UINT16 result = program_read_byte_8(address) << 8;
	return result | program_read_byte_8(address + 1);
}

static void writeword_d8(offs_t address, UINT16 data)
{
	program_write_byte_8(address, data >> 8);
	program_write_byte_8(address + 1, data);
}

static UINT32 readlong_d8(offs_t address)
{
	UINT32 result = program_read_byte_8(address) << 24;
	result |= program_read_byte_8(address + 1) << 16;
	result |= program_read_byte_8(address + 2) << 8;
	return result | program_read_byte_8(address + 3);
}

static void writelong_d8(offs_t address, UINT32 data)
{
	program_write_byte_8(address, data >> 24);
	program_write_byte_8(address + 1, data >> 16);
	program_write_byte_8(address + 2, data >> 8);
	program_write_byte_8(address + 3, data);
}

/* interface for 20/22-bit address bus, 8-bit data bus (68008) */
static const struct m68k_memory_interface interface_d8 =
{
	0,
	program_read_byte_8,
	readword_d8,
	readlong_d8,
	program_write_byte_8,
	writeword_d8,
	writelong_d8
};

/****************************************************************************
 * 16-bit data memory interface
 ****************************************************************************/

static UINT32 readlong_d16(offs_t address)
{
	UINT32 result = program_read_word_16be(address) << 16;
	return result | program_read_word_16be(address + 2);
}

static void writelong_d16(offs_t address, UINT32 data)
{
	program_write_word_16be(address, data >> 16);
	program_write_word_16be(address + 2, data);
}

/* interface for 24-bit address bus, 16-bit data bus (68000, 68010) */
static const struct m68k_memory_interface interface_d16 =
{
	0,
	program_read_byte_16be,
	program_read_word_16be,
	readlong_d16,
	program_write_byte_16be,
	program_write_word_16be,
	writelong_d16
};

/****************************************************************************
 * 32-bit data memory interface
 ****************************************************************************/

/* potentially misaligned 16-bit reads with a 32-bit data bus (and 24-bit address bus) */
static UINT16 readword_d32(offs_t address)
{
	UINT16 result;

	if (!(address & 1))
		return program_read_word_32be(address);
	result = program_read_byte_32be(address) << 8;
	return result | program_read_byte_32be(address + 1);
}

/* potentially misaligned 16-bit writes with a 32-bit data bus (and 24-bit address bus) */
static void writeword_d32(offs_t address, UINT16 data)
{
	if (!(address & 1))
	{
		program_write_word_32be(address, data);
		return;
	}
	program_write_byte_32be(address, data >> 8);
	program_write_byte_32be(address + 1, data);
}

/* potentially misaligned 32-bit reads with a 32-bit data bus (and 24-bit address bus) */
static UINT32 readlong_d32(offs_t address)
{
	UINT32 result;

	if (!(address & 3))
		return program_read_dword_32be(address);
	else if (!(address & 1))
	{
		result = program_read_word_32be(address) << 16;
		return result | program_read_word_32be(address + 2);
	}
	result = program_read_byte_32be(address) << 24;
	result |= program_read_word_32be(address + 1) << 8;
	return result | program_read_byte_32be(address + 3);
}

/* potentially misaligned 32-bit writes with a 32-bit data bus (and 24-bit address bus) */
static void writelong_d32(offs_t address, UINT32 data)
{
	if (!(address & 3))
	{
		program_write_dword_32be(address, data);
		return;
	}
	else if (!(address & 1))
	{
		program_write_word_32be(address, data >> 16);
		program_write_word_32be(address + 2, data);
		return;
	}
	program_write_byte_32be(address, data >> 24);
	program_write_word_32be(address + 1, data >> 8);
	program_write_byte_32be(address + 3, data);
}

/* interface for 32-bit data bus (68EC020, 68020) */
static const struct m68k_memory_interface interface_d32 =
{
	WORD_XOR_BE(0),
	program_read_byte_32be,
	readword_d32,
	readlong_d32,
	program_write_byte_32be,
	writeword_d32,
	writelong_d32
};


/* global access */
struct m68k_memory_interface m68k_memory_intf;


static void set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
		irqline = 7;
	switch(state)
	{
		case CLEAR_LINE:
			m68k_set_irq(0);
			break;
		case ASSERT_LINE:
			m68k_set_irq(irqline);
			break;
		default:
			m68k_set_irq(irqline);
			break;
	}
}


/****************************************************************************
 * 68000 section
 ****************************************************************************/

static void m68000_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68000);
	m68k_memory_intf = interface_d16;
	m68k_state_register("m68000", index);
	m68k_set_int_ack_callback(irqcallback);
}

static void m68000_reset(void)
{
	m68k_pulse_reset();
}

static void m68000_exit(void)
{
	/* nothing to do */
}

static int m68000_execute(int cycles)
{
	return m68k_execute(cycles);
}

static void m68000_get_context(void *dst)
{
	m68k_get_context(dst);
}

static void m68000_set_context(void *src)
{
	if (m68k_memory_intf.read8 != program_read_byte_16be)
		m68k_memory_intf = interface_d16;
	m68k_set_context(src);
}

#ifdef MAME_DEBUG
static offs_t m68000_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	M68K_SET_PC_CALLBACK(pc);
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68000);
}
#endif /* MAME_DEBUG */

/****************************************************************************
 * M68008 section
 ****************************************************************************/
#if HAS_M68008

static void m68008_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68008);
	m68k_memory_intf = interface_d8;
	m68k_state_register("m68008", index);
	m68k_set_int_ack_callback(irqcallback);
}

static void m68008_reset(void)
{
	m68k_pulse_reset();
}

static void m68008_exit(void)
{
	/* nothing to do */
}

static int m68008_execute(int cycles)
{
	return m68k_execute(cycles);
}

static void m68008_get_context(void *dst)
{
	m68k_get_context(dst);
}

static void m68008_set_context(void *src)
{
	if (m68k_memory_intf.read8 != program_read_byte_8)
		m68k_memory_intf = interface_d8;
	m68k_set_context(src);
}

#ifdef MAME_DEBUG
static offs_t m68008_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	M68K_SET_PC_CALLBACK(pc);
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68008);
}
#endif /* MAME_DEBUG */

#endif

/****************************************************************************
 * M68010 section
 ****************************************************************************/
#if HAS_M68010

void m68010_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68010);
	m68k_memory_intf = interface_d16;
	m68k_state_register("m68010", index);
	m68k_set_int_ack_callback(irqcallback);
}

#ifdef MAME_DEBUG
static offs_t m68010_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	M68K_SET_PC_CALLBACK(pc);
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68010);
}
#endif /* MAME_DEBUG */

#endif /* HAS_M68010 */

/****************************************************************************
 * M68020 section
 ****************************************************************************/

static void m68020_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68020);
	m68k_memory_intf = interface_d32;
	m68k_state_register("m68020", index);
	m68k_set_int_ack_callback(irqcallback);
}

static void m68020_reset(void)
{
	m68k_pulse_reset();
}

static void m68020_exit(void)
{
	/* nothing to do */
}

static int m68020_execute(int cycles)
{
	return m68k_execute(cycles);
}

static void m68020_get_context(void *dst)
{
	m68k_get_context(dst);
}

static void m68020_set_context(void *src)
{
	if (m68k_memory_intf.read8 != program_read_byte_32be)
		m68k_memory_intf = interface_d32;
	m68k_set_context(src);
}

#ifdef MAME_DEBUG
static offs_t m68020_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	M68K_SET_PC_CALLBACK(pc);
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68020);
}
#endif /* MAME_DEBUG */


/****************************************************************************
 * M680EC20 section
 ****************************************************************************/
#if HAS_M68EC020

static void m68ec020_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68EC020);
	m68k_memory_intf = interface_d32;
	m68k_state_register("m68ec020", index);
	m68k_set_int_ack_callback(irqcallback);
}

#ifdef MAME_DEBUG
static offs_t m68ec020_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	M68K_SET_PC_CALLBACK(pc);
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68EC020);
}
#endif /* MAME_DEBUG */

#endif /* HAS_M68EC020 */

/****************************************************************************
 * M68040 section
 ****************************************************************************/
#if HAS_M68040

static void m68040_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68040);
	m68k_memory_intf = interface_d32;
	m68k_state_register("m68040", index);
	m68k_set_int_ack_callback(irqcallback);
}

static void m68040_reset(void)
{
	m68k_pulse_reset();
}

static void m68040_exit(void)
{
	/* nothing to do */
}

static int m68040_execute(int cycles)
{
	return m68k_execute(cycles);
}

static void m68040_get_context(void *dst)
{
	m68k_get_context(dst);
}

static void m68040_set_context(void *src)
{
	if (m68k_memory_intf.read8 != program_read_byte_32be)
		m68k_memory_intf = interface_d32;
	m68k_set_context(src);
}

#ifdef MAME_DEBUG
static offs_t m68040_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	M68K_SET_PC_CALLBACK(pc);
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68040);
}
#endif /* MAME_DEBUG */

#endif /* HAS_M68040 */

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void m68000_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			set_irq_line(0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 1:			set_irq_line(1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 2:			set_irq_line(2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 3:			set_irq_line(3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 4:			set_irq_line(4, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 5:			set_irq_line(5, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 6:			set_irq_line(6, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 7:			set_irq_line(7, info->i);					break;

		case CPUINFO_INT_PC:  						m68k_set_reg(M68K_REG_PC, info->i&0x00ffffff); break;
		case CPUINFO_INT_REGISTER + M68K_PC:  		m68k_set_reg(M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:  		m68k_set_reg(M68K_REG_SP, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_ISP: 		m68k_set_reg(M68K_REG_ISP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_USP: 		m68k_set_reg(M68K_REG_USP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_SR:  		m68k_set_reg(M68K_REG_SR, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D0:  		m68k_set_reg(M68K_REG_D0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D1:  		m68k_set_reg(M68K_REG_D1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D2:  		m68k_set_reg(M68K_REG_D2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D3:  		m68k_set_reg(M68K_REG_D3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D4:  		m68k_set_reg(M68K_REG_D4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D5:  		m68k_set_reg(M68K_REG_D5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D6:  		m68k_set_reg(M68K_REG_D6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D7:  		m68k_set_reg(M68K_REG_D7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A0:  		m68k_set_reg(M68K_REG_A0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A1:  		m68k_set_reg(M68K_REG_A1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A2:  		m68k_set_reg(M68K_REG_A2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A3:  		m68k_set_reg(M68K_REG_A3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A4:  		m68k_set_reg(M68K_REG_A4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A5:  		m68k_set_reg(M68K_REG_A5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A6:  		m68k_set_reg(M68K_REG_A6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A7:  		m68k_set_reg(M68K_REG_A7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:	m68k_set_reg(M68K_REG_PREF_ADDR, info->i);	break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		m68k_set_reset_instr_callback(info->f);		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		m68k_set_cmpild_instr_callback((void (*)(unsigned int,int))(info->f)); break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:			m68k_set_rte_instr_callback(info->f);		break;
		case CPUINFO_PTR_M68K_TAS_CALLBACK:			m68k_set_tas_instr_callback((int (*)(void))(info->f)); break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

void m68000_get_info(UINT32 state, cpuinfo *info)
{
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = m68k_get_context(NULL);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 10;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = 0;	/* fix me */			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m68k_get_reg(NULL, M68K_REG_PPC); break;

		case CPUINFO_INT_PC:							info->i = m68k_get_reg(NULL, M68K_REG_PC)&0x00ffffff; break;
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = m68k_get_reg(NULL, M68K_REG_PC); break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:			info->i = m68k_get_reg(NULL, M68K_REG_SP); break;
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = m68k_get_reg(NULL, M68K_REG_ISP); break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = m68k_get_reg(NULL, M68K_REG_USP); break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = m68k_get_reg(NULL, M68K_REG_SR); break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = m68k_get_reg(NULL, M68K_REG_D0); break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = m68k_get_reg(NULL, M68K_REG_D1); break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = m68k_get_reg(NULL, M68K_REG_D2); break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = m68k_get_reg(NULL, M68K_REG_D3); break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = m68k_get_reg(NULL, M68K_REG_D4); break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = m68k_get_reg(NULL, M68K_REG_D5); break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = m68k_get_reg(NULL, M68K_REG_D6); break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = m68k_get_reg(NULL, M68K_REG_D7); break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = m68k_get_reg(NULL, M68K_REG_A0); break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = m68k_get_reg(NULL, M68K_REG_A1); break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = m68k_get_reg(NULL, M68K_REG_A2); break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = m68k_get_reg(NULL, M68K_REG_A3); break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = m68k_get_reg(NULL, M68K_REG_A4); break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = m68k_get_reg(NULL, M68K_REG_A5); break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = m68k_get_reg(NULL, M68K_REG_A6); break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = m68k_get_reg(NULL, M68K_REG_A7); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:		info->i = m68k_get_reg(NULL, M68K_REG_PREF_ADDR); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_DATA:		info->i = m68k_get_reg(NULL, M68K_REG_PREF_DATA); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m68000_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m68000_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m68000_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m68000_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m68000_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m68000_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m68000_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68000_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68000");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "3.31");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright 1998-2007 Karl Stenerud. All rights reserved. (2.1 fixes HJB)"); break;

		case CPUINFO_STR_FLAGS:
			sr = m68k_get_reg(NULL, M68K_REG_SR);
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				sr & 0x8000 ? 'T':'.',
				sr & 0x4000 ? '?':'.',
				sr & 0x2000 ? 'S':'.',
				sr & 0x1000 ? '?':'.',
				sr & 0x0800 ? '?':'.',
				sr & 0x0400 ? 'I':'.',
				sr & 0x0200 ? 'I':'.',
				sr & 0x0100 ? 'I':'.',
				sr & 0x0080 ? '?':'.',
				sr & 0x0040 ? '?':'.',
				sr & 0x0020 ? '?':'.',
				sr & 0x0010 ? 'X':'.',
				sr & 0x0008 ? 'N':'.',
				sr & 0x0004 ? 'Z':'.',
				sr & 0x0002 ? 'V':'.',
				sr & 0x0001 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%08X", m68k_get_reg(NULL, M68K_REG_PC)); break;
		case CPUINFO_STR_REGISTER + M68K_SR:  			sprintf(info->s, "SR :%04X", m68k_get_reg(NULL, M68K_REG_SR)); break;
		case CPUINFO_STR_REGISTER + M68K_SP:  			sprintf(info->s, "SP :%08X", m68k_get_reg(NULL, M68K_REG_SP)); break;
		case CPUINFO_STR_REGISTER + M68K_ISP: 			sprintf(info->s, "ISP:%08X", m68k_get_reg(NULL, M68K_REG_ISP)); break;
		case CPUINFO_STR_REGISTER + M68K_USP: 			sprintf(info->s, "USP:%08X", m68k_get_reg(NULL, M68K_REG_USP)); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", m68k_get_reg(NULL, M68K_REG_D0)); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", m68k_get_reg(NULL, M68K_REG_D1)); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", m68k_get_reg(NULL, M68K_REG_D2)); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", m68k_get_reg(NULL, M68K_REG_D3)); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", m68k_get_reg(NULL, M68K_REG_D4)); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", m68k_get_reg(NULL, M68K_REG_D5)); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", m68k_get_reg(NULL, M68K_REG_D6)); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", m68k_get_reg(NULL, M68K_REG_D7)); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", m68k_get_reg(NULL, M68K_REG_A0)); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", m68k_get_reg(NULL, M68K_REG_A1)); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", m68k_get_reg(NULL, M68K_REG_A2)); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", m68k_get_reg(NULL, M68K_REG_A3)); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", m68k_get_reg(NULL, M68K_REG_A4)); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", m68k_get_reg(NULL, M68K_REG_A5)); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", m68k_get_reg(NULL, M68K_REG_A6)); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", m68k_get_reg(NULL, M68K_REG_A7)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_ADDR:		sprintf(info->s, "PAR:%08X", m68k_get_reg(NULL, M68K_REG_PREF_ADDR)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_DATA:		sprintf(info->s, "PDA:%08X", m68k_get_reg(NULL, M68K_REG_PREF_DATA)); break;
	}
}

#if HAS_M68008

static void m68008_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			set_irq_line(0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 1:			set_irq_line(1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 2:			set_irq_line(2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 3:			set_irq_line(3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 4:			set_irq_line(4, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 5:			set_irq_line(5, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 6:			set_irq_line(6, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 7:			set_irq_line(7, info->i);					break;

		case CPUINFO_INT_PC:  						m68k_set_reg(M68K_REG_PC, info->i&0x00ffffff); break;
		case CPUINFO_INT_REGISTER + M68K_PC:  		m68k_set_reg(M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:  		m68k_set_reg(M68K_REG_SP, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_ISP: 		m68k_set_reg(M68K_REG_ISP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_USP: 		m68k_set_reg(M68K_REG_USP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_SR:  		m68k_set_reg(M68K_REG_SR, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D0:  		m68k_set_reg(M68K_REG_D0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D1:  		m68k_set_reg(M68K_REG_D1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D2:  		m68k_set_reg(M68K_REG_D2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D3:  		m68k_set_reg(M68K_REG_D3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D4:  		m68k_set_reg(M68K_REG_D4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D5:  		m68k_set_reg(M68K_REG_D5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D6:  		m68k_set_reg(M68K_REG_D6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D7:  		m68k_set_reg(M68K_REG_D7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A0:  		m68k_set_reg(M68K_REG_A0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A1:  		m68k_set_reg(M68K_REG_A1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A2:  		m68k_set_reg(M68K_REG_A2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A3:  		m68k_set_reg(M68K_REG_A3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A4:  		m68k_set_reg(M68K_REG_A4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A5:  		m68k_set_reg(M68K_REG_A5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A6:  		m68k_set_reg(M68K_REG_A6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A7:  		m68k_set_reg(M68K_REG_A7, info->i);			break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		m68k_set_reset_instr_callback(info->f);		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		m68k_set_cmpild_instr_callback((void (*)(unsigned int,int))(info->f)); break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:			m68k_set_rte_instr_callback(info->f);		break;
	}
}

void m68008_get_info(UINT32 state, cpuinfo *info)
{
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = m68k_get_context(NULL);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 10;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 22;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = 0;	/* fix me */			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m68k_get_reg(NULL, M68K_REG_PPC); break;

		case CPUINFO_INT_PC:							info->i = m68k_get_reg(NULL, M68K_REG_PC)&0x00ffffff; break;
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = m68k_get_reg(NULL, M68K_REG_PC); break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:			info->i = m68k_get_reg(NULL, M68K_REG_SP); break;
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = m68k_get_reg(NULL, M68K_REG_ISP); break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = m68k_get_reg(NULL, M68K_REG_USP); break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = m68k_get_reg(NULL, M68K_REG_SR); break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = m68k_get_reg(NULL, M68K_REG_D0); break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = m68k_get_reg(NULL, M68K_REG_D1); break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = m68k_get_reg(NULL, M68K_REG_D2); break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = m68k_get_reg(NULL, M68K_REG_D3); break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = m68k_get_reg(NULL, M68K_REG_D4); break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = m68k_get_reg(NULL, M68K_REG_D5); break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = m68k_get_reg(NULL, M68K_REG_D6); break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = m68k_get_reg(NULL, M68K_REG_D7); break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = m68k_get_reg(NULL, M68K_REG_A0); break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = m68k_get_reg(NULL, M68K_REG_A1); break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = m68k_get_reg(NULL, M68K_REG_A2); break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = m68k_get_reg(NULL, M68K_REG_A3); break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = m68k_get_reg(NULL, M68K_REG_A4); break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = m68k_get_reg(NULL, M68K_REG_A5); break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = m68k_get_reg(NULL, M68K_REG_A6); break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = m68k_get_reg(NULL, M68K_REG_A7); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:		info->i = m68k_get_reg(NULL, M68K_REG_PREF_ADDR); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_DATA:		info->i = m68k_get_reg(NULL, M68K_REG_PREF_DATA); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m68008_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m68008_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m68008_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m68008_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m68008_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m68008_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m68008_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68008_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68008");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "3.31");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright 1998-2007 Karl Stenerud. All rights reserved. (2.1 fixes HJB)"); break;

		case CPUINFO_STR_FLAGS:
			sr = m68k_get_reg(NULL, M68K_REG_SR);
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				sr & 0x8000 ? 'T':'.',
				sr & 0x4000 ? '?':'.',
				sr & 0x2000 ? 'S':'.',
				sr & 0x1000 ? '?':'.',
				sr & 0x0800 ? '?':'.',
				sr & 0x0400 ? 'I':'.',
				sr & 0x0200 ? 'I':'.',
				sr & 0x0100 ? 'I':'.',
				sr & 0x0080 ? '?':'.',
				sr & 0x0040 ? '?':'.',
				sr & 0x0020 ? '?':'.',
				sr & 0x0010 ? 'X':'.',
				sr & 0x0008 ? 'N':'.',
				sr & 0x0004 ? 'Z':'.',
				sr & 0x0002 ? 'V':'.',
				sr & 0x0001 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%08X", m68k_get_reg(NULL, M68K_REG_PC)); break;
		case CPUINFO_STR_REGISTER + M68K_SR:  			sprintf(info->s, "SR :%04X", m68k_get_reg(NULL, M68K_REG_SR)); break;
		case CPUINFO_STR_REGISTER + M68K_SP:  			sprintf(info->s, "SP :%08X", m68k_get_reg(NULL, M68K_REG_SP)); break;
		case CPUINFO_STR_REGISTER + M68K_ISP: 			sprintf(info->s, "ISP:%08X", m68k_get_reg(NULL, M68K_REG_ISP)); break;
		case CPUINFO_STR_REGISTER + M68K_USP: 			sprintf(info->s, "USP:%08X", m68k_get_reg(NULL, M68K_REG_USP)); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", m68k_get_reg(NULL, M68K_REG_D0)); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", m68k_get_reg(NULL, M68K_REG_D1)); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", m68k_get_reg(NULL, M68K_REG_D2)); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", m68k_get_reg(NULL, M68K_REG_D3)); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", m68k_get_reg(NULL, M68K_REG_D4)); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", m68k_get_reg(NULL, M68K_REG_D5)); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", m68k_get_reg(NULL, M68K_REG_D6)); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", m68k_get_reg(NULL, M68K_REG_D7)); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", m68k_get_reg(NULL, M68K_REG_A0)); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", m68k_get_reg(NULL, M68K_REG_A1)); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", m68k_get_reg(NULL, M68K_REG_A2)); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", m68k_get_reg(NULL, M68K_REG_A3)); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", m68k_get_reg(NULL, M68K_REG_A4)); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", m68k_get_reg(NULL, M68K_REG_A5)); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", m68k_get_reg(NULL, M68K_REG_A6)); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", m68k_get_reg(NULL, M68K_REG_A7)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_ADDR:		sprintf(info->s, "PAR:%08X", m68k_get_reg(NULL, M68K_REG_PREF_ADDR)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_DATA:		sprintf(info->s, "PDA:%08X", m68k_get_reg(NULL, M68K_REG_PREF_DATA)); break;
	}
}

#endif

/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/
#if HAS_M68010

static void m68010_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_REGISTER + M68K_VBR:  			m68k_set_reg(M68K_REG_VBR, info->i);	break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  			m68k_set_reg(M68K_REG_SFC, info->i);	break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  			m68k_set_reg(M68K_REG_DFC, info->i);	break;

		default:										m68000_set_info(state, info);			break;
	}
}

void m68010_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_REGISTER + M68K_VBR:  			info->i = m68k_get_reg(NULL, M68K_REG_VBR); break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  			info->i = m68k_get_reg(NULL, M68K_REG_SFC); break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  			info->i = m68k_get_reg(NULL, M68K_REG_DFC); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m68010_set_info;		break;
		case CPUINFO_PTR_INIT:							info->init = m68010_init;				break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68010_dasm;		break;
#endif /* MAME_DEBUG */
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68010");				break;
		case CPUINFO_STR_REGISTER + M68K_SFC:			sprintf(info->s, "SFC:%X",   m68k_get_reg(NULL, M68K_REG_SFC)); break;
		case CPUINFO_STR_REGISTER + M68K_DFC:			sprintf(info->s, "DFC:%X",   m68k_get_reg(NULL, M68K_REG_DFC)); break;
		case CPUINFO_STR_REGISTER + M68K_VBR:			sprintf(info->s, "VBR:%08X", m68k_get_reg(NULL, M68K_REG_VBR)); break;

		default:										m68000_get_info(state, info);			break;
	}
}

#endif

/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static void m68020_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			set_irq_line(0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 1:			set_irq_line(1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 2:			set_irq_line(2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 3:			set_irq_line(3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 4:			set_irq_line(4, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 5:			set_irq_line(5, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 6:			set_irq_line(6, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 7:			set_irq_line(7, info->i);					break;

		case CPUINFO_INT_PC:  						m68k_set_reg(M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_PC:  		m68k_set_reg(M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:  		m68k_set_reg(M68K_REG_SP, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_ISP: 		m68k_set_reg(M68K_REG_ISP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_USP: 		m68k_set_reg(M68K_REG_USP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_SR:  		m68k_set_reg(M68K_REG_SR, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D0:  		m68k_set_reg(M68K_REG_D0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D1:  		m68k_set_reg(M68K_REG_D1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D2:  		m68k_set_reg(M68K_REG_D2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D3:  		m68k_set_reg(M68K_REG_D3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D4:  		m68k_set_reg(M68K_REG_D4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D5:  		m68k_set_reg(M68K_REG_D5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D6:  		m68k_set_reg(M68K_REG_D6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D7:  		m68k_set_reg(M68K_REG_D7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A0:  		m68k_set_reg(M68K_REG_A0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A1:  		m68k_set_reg(M68K_REG_A1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A2:  		m68k_set_reg(M68K_REG_A2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A3:  		m68k_set_reg(M68K_REG_A3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A4:  		m68k_set_reg(M68K_REG_A4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A5:  		m68k_set_reg(M68K_REG_A5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A6:  		m68k_set_reg(M68K_REG_A6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A7:  		m68k_set_reg(M68K_REG_A7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_MSP:		m68k_set_reg(M68K_REG_MSP, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_CACR:		m68k_set_reg(M68K_REG_CACR, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_CAAR:		m68k_set_reg(M68K_REG_CAAR, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_VBR:		m68k_set_reg(M68K_REG_VBR, info->i);		break; /* 68010+ */
		case CPUINFO_INT_REGISTER + M68K_SFC:		m68k_set_reg(M68K_REG_SFC, info->i);		break; /* 68010+ */
		case CPUINFO_INT_REGISTER + M68K_DFC:		m68k_set_reg(M68K_REG_DFC, info->i);		break; /* 68010+ */

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		m68k_set_reset_instr_callback(info->f);		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		m68k_set_cmpild_instr_callback((void (*)(unsigned int,int))(info->f)); break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:			m68k_set_rte_instr_callback(info->f);		break;
	}
}

void m68020_get_info(UINT32 state, cpuinfo *info)
{
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = m68k_get_context(NULL);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 16;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = 1;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = 2;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = 3;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = 4;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = 5;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = 6;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = 7;	/* fix me */			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m68k_get_reg(NULL, M68K_REG_PPC); break;

		case CPUINFO_INT_PC:							info->i = m68k_get_reg(NULL, M68K_REG_PC); break;
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = m68k_get_reg(NULL, M68K_REG_PC); break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:			info->i = m68k_get_reg(NULL, M68K_REG_SP); break;
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = m68k_get_reg(NULL, M68K_REG_ISP); break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = m68k_get_reg(NULL, M68K_REG_USP); break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = m68k_get_reg(NULL, M68K_REG_SR); break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = m68k_get_reg(NULL, M68K_REG_D0); break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = m68k_get_reg(NULL, M68K_REG_D1); break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = m68k_get_reg(NULL, M68K_REG_D2); break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = m68k_get_reg(NULL, M68K_REG_D3); break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = m68k_get_reg(NULL, M68K_REG_D4); break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = m68k_get_reg(NULL, M68K_REG_D5); break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = m68k_get_reg(NULL, M68K_REG_D6); break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = m68k_get_reg(NULL, M68K_REG_D7); break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = m68k_get_reg(NULL, M68K_REG_A0); break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = m68k_get_reg(NULL, M68K_REG_A1); break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = m68k_get_reg(NULL, M68K_REG_A2); break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = m68k_get_reg(NULL, M68K_REG_A3); break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = m68k_get_reg(NULL, M68K_REG_A4); break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = m68k_get_reg(NULL, M68K_REG_A5); break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = m68k_get_reg(NULL, M68K_REG_A6); break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = m68k_get_reg(NULL, M68K_REG_A7); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:		info->i = m68k_get_reg(NULL, M68K_REG_PREF_ADDR); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_DATA:		info->i = m68k_get_reg(NULL, M68K_REG_PREF_DATA); break;
		case CPUINFO_INT_REGISTER + M68K_MSP:			info->i = m68k_get_reg(NULL, M68K_REG_MSP); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_CACR: 			info->i = m68k_get_reg(NULL, M68K_REG_CACR); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_CAAR: 			info->i = m68k_get_reg(NULL, M68K_REG_CAAR); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_VBR:			info->i = m68k_get_reg(NULL, M68K_REG_VBR); /* 68010+ */ break;
		case CPUINFO_INT_REGISTER + M68K_SFC:			info->i = m68k_get_reg(NULL, M68K_REG_SFC); /* 68010" */ break;
		case CPUINFO_INT_REGISTER + M68K_DFC:			info->i = m68k_get_reg(NULL, M68K_REG_DFC); /* 68010+ */ break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m68020_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m68020_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m68020_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m68020_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m68020_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m68020_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m68020_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68020_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68020");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "3.31");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright 1998-2007 Karl Stenerud. All rights reserved. (2.1 fixes HJB)"); break;

		case CPUINFO_STR_FLAGS:
			sr = m68k_get_reg(NULL, M68K_REG_SR);
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				sr & 0x8000 ? 'T':'.',
				sr & 0x4000 ? 't':'.',
				sr & 0x2000 ? 'S':'.',
				sr & 0x1000 ? 'M':'.',
				sr & 0x0800 ? '?':'.',
				sr & 0x0400 ? 'I':'.',
				sr & 0x0200 ? 'I':'.',
				sr & 0x0100 ? 'I':'.',
				sr & 0x0080 ? '?':'.',
				sr & 0x0040 ? '?':'.',
				sr & 0x0020 ? '?':'.',
				sr & 0x0010 ? 'X':'.',
				sr & 0x0008 ? 'N':'.',
				sr & 0x0004 ? 'Z':'.',
				sr & 0x0002 ? 'V':'.',
				sr & 0x0001 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%08X", m68k_get_reg(NULL, M68K_REG_PC)); break;
		case CPUINFO_STR_REGISTER + M68K_SR:  			sprintf(info->s, "SR :%04X", m68k_get_reg(NULL, M68K_REG_SR)); break;
		case CPUINFO_STR_REGISTER + M68K_SP:  			sprintf(info->s, "SP :%08X", m68k_get_reg(NULL, M68K_REG_SP)); break;
		case CPUINFO_STR_REGISTER + M68K_ISP: 			sprintf(info->s, "ISP:%08X", m68k_get_reg(NULL, M68K_REG_ISP)); break;
		case CPUINFO_STR_REGISTER + M68K_USP: 			sprintf(info->s, "USP:%08X", m68k_get_reg(NULL, M68K_REG_USP)); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", m68k_get_reg(NULL, M68K_REG_D0)); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", m68k_get_reg(NULL, M68K_REG_D1)); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", m68k_get_reg(NULL, M68K_REG_D2)); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", m68k_get_reg(NULL, M68K_REG_D3)); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", m68k_get_reg(NULL, M68K_REG_D4)); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", m68k_get_reg(NULL, M68K_REG_D5)); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", m68k_get_reg(NULL, M68K_REG_D6)); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", m68k_get_reg(NULL, M68K_REG_D7)); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", m68k_get_reg(NULL, M68K_REG_A0)); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", m68k_get_reg(NULL, M68K_REG_A1)); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", m68k_get_reg(NULL, M68K_REG_A2)); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", m68k_get_reg(NULL, M68K_REG_A3)); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", m68k_get_reg(NULL, M68K_REG_A4)); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", m68k_get_reg(NULL, M68K_REG_A5)); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", m68k_get_reg(NULL, M68K_REG_A6)); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", m68k_get_reg(NULL, M68K_REG_A7)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_ADDR:		sprintf(info->s, "PAR:%08X", m68k_get_reg(NULL, M68K_REG_PREF_ADDR)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_DATA:		sprintf(info->s, "PDA:%08X", m68k_get_reg(NULL, M68K_REG_PREF_DATA)); break;
		case CPUINFO_STR_REGISTER + M68K_MSP:			sprintf(info->s, "MSP:%08X", m68k_get_reg(NULL, M68K_REG_MSP)); break;
		case CPUINFO_STR_REGISTER + M68K_CACR:			sprintf(info->s, "CCR:%08X", m68k_get_reg(NULL, M68K_REG_CACR)); break;
		case CPUINFO_STR_REGISTER + M68K_CAAR:			sprintf(info->s, "CAR:%08X", m68k_get_reg(NULL, M68K_REG_CAAR)); break;
		case CPUINFO_STR_REGISTER + M68K_SFC:			sprintf(info->s, "SFC:%X",   m68k_get_reg(NULL, M68K_REG_SFC)); break;
		case CPUINFO_STR_REGISTER + M68K_DFC:			sprintf(info->s, "DFC:%X",   m68k_get_reg(NULL, M68K_REG_DFC)); break;
		case CPUINFO_STR_REGISTER + M68K_VBR:			sprintf(info->s, "VBR:%08X", m68k_get_reg(NULL, M68K_REG_VBR)); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/
#if HAS_M68EC020

static void m68ec020_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:  							m68k_set_reg(M68K_REG_PC, info->i&0x00ffffff); break;

		default:										m68020_set_info(state, info);			break;
	}
}

void m68ec020_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_REGISTER + REG_PC:				info->i = m68k_get_reg(NULL, M68K_REG_PC)&0x00ffffff; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m68ec020_set_info;		break;
		case CPUINFO_PTR_INIT:							info->init = m68ec020_init;				break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68ec020_dasm;		break;
#endif /* MAME_DEBUG */

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68EC020");				break;

		default:										m68020_get_info(state, info);			break;
	}
}

#endif

/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

#if HAS_M68040
static void m68040_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			set_irq_line(0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 1:			set_irq_line(1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 2:			set_irq_line(2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 3:			set_irq_line(3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 4:			set_irq_line(4, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 5:			set_irq_line(5, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 6:			set_irq_line(6, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 7:			set_irq_line(7, info->i);					break;

		case CPUINFO_INT_PC:  						m68k_set_reg(M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_PC:  		m68k_set_reg(M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:  		m68k_set_reg(M68K_REG_SP, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_ISP: 		m68k_set_reg(M68K_REG_ISP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_USP: 		m68k_set_reg(M68K_REG_USP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_SR:  		m68k_set_reg(M68K_REG_SR, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D0:  		m68k_set_reg(M68K_REG_D0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D1:  		m68k_set_reg(M68K_REG_D1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D2:  		m68k_set_reg(M68K_REG_D2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D3:  		m68k_set_reg(M68K_REG_D3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D4:  		m68k_set_reg(M68K_REG_D4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D5:  		m68k_set_reg(M68K_REG_D5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D6:  		m68k_set_reg(M68K_REG_D6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D7:  		m68k_set_reg(M68K_REG_D7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A0:  		m68k_set_reg(M68K_REG_A0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A1:  		m68k_set_reg(M68K_REG_A1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A2:  		m68k_set_reg(M68K_REG_A2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A3:  		m68k_set_reg(M68K_REG_A3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A4:  		m68k_set_reg(M68K_REG_A4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A5:  		m68k_set_reg(M68K_REG_A5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A6:  		m68k_set_reg(M68K_REG_A6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A7:  		m68k_set_reg(M68K_REG_A7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_MSP:		m68k_set_reg(M68K_REG_MSP, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_CACR:		m68k_set_reg(M68K_REG_CACR, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_CAAR:		m68k_set_reg(M68K_REG_CAAR, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_VBR:		m68k_set_reg(M68K_REG_VBR, info->i);		break; /* 68010+ */
		case CPUINFO_INT_REGISTER + M68K_SFC:		m68k_set_reg(M68K_REG_SFC, info->i);		break; /* 68010+ */
		case CPUINFO_INT_REGISTER + M68K_DFC:		m68k_set_reg(M68K_REG_DFC, info->i);		break; /* 68010+ */

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		m68k_set_reset_instr_callback(info->f);		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		m68k_set_cmpild_instr_callback((void (*)(unsigned int,int))(info->f)); break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:			m68k_set_rte_instr_callback(info->f);		break;
	}
}

void m68040_get_info(UINT32 state, cpuinfo *info)
{
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = m68k_get_context(NULL);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 16;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 4;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 158;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = 1;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = 2;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = 3;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = 4;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = 5;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = 6;	/* fix me */			break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = 7;	/* fix me */			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m68k_get_reg(NULL, M68K_REG_PPC); break;

		case CPUINFO_INT_PC:							info->i = m68k_get_reg(NULL, M68K_REG_PC); break;
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = m68k_get_reg(NULL, M68K_REG_PC); break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:			info->i = m68k_get_reg(NULL, M68K_REG_SP); break;
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = m68k_get_reg(NULL, M68K_REG_ISP); break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = m68k_get_reg(NULL, M68K_REG_USP); break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = m68k_get_reg(NULL, M68K_REG_SR); break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = m68k_get_reg(NULL, M68K_REG_D0); break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = m68k_get_reg(NULL, M68K_REG_D1); break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = m68k_get_reg(NULL, M68K_REG_D2); break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = m68k_get_reg(NULL, M68K_REG_D3); break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = m68k_get_reg(NULL, M68K_REG_D4); break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = m68k_get_reg(NULL, M68K_REG_D5); break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = m68k_get_reg(NULL, M68K_REG_D6); break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = m68k_get_reg(NULL, M68K_REG_D7); break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = m68k_get_reg(NULL, M68K_REG_A0); break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = m68k_get_reg(NULL, M68K_REG_A1); break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = m68k_get_reg(NULL, M68K_REG_A2); break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = m68k_get_reg(NULL, M68K_REG_A3); break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = m68k_get_reg(NULL, M68K_REG_A4); break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = m68k_get_reg(NULL, M68K_REG_A5); break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = m68k_get_reg(NULL, M68K_REG_A6); break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = m68k_get_reg(NULL, M68K_REG_A7); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:		info->i = m68k_get_reg(NULL, M68K_REG_PREF_ADDR); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_DATA:		info->i = m68k_get_reg(NULL, M68K_REG_PREF_DATA); break;
		case CPUINFO_INT_REGISTER + M68K_MSP:			info->i = m68k_get_reg(NULL, M68K_REG_MSP); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_CACR: 			info->i = m68k_get_reg(NULL, M68K_REG_CACR); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_CAAR: 			info->i = m68k_get_reg(NULL, M68K_REG_CAAR); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_VBR:			info->i = m68k_get_reg(NULL, M68K_REG_VBR); /* 68010+ */ break;
		case CPUINFO_INT_REGISTER + M68K_SFC:			info->i = m68k_get_reg(NULL, M68K_REG_SFC); /* 68010" */ break;
		case CPUINFO_INT_REGISTER + M68K_DFC:			info->i = m68k_get_reg(NULL, M68K_REG_DFC); /* 68010+ */ break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = m68040_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = m68040_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = m68040_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = m68040_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = m68040_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = m68040_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = m68040_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = m68040_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68040");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "3.31");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright 1998-2007 Karl Stenerud. All rights reserved. (2.1 fixes HJB)"); break;

		case CPUINFO_STR_FLAGS:
			sr = m68k_get_reg(NULL, M68K_REG_SR);
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				sr & 0x8000 ? 'T':'.',
				sr & 0x4000 ? 't':'.',
				sr & 0x2000 ? 'S':'.',
				sr & 0x1000 ? 'M':'.',
				sr & 0x0800 ? '?':'.',
				sr & 0x0400 ? 'I':'.',
				sr & 0x0200 ? 'I':'.',
				sr & 0x0100 ? 'I':'.',
				sr & 0x0080 ? '?':'.',
				sr & 0x0040 ? '?':'.',
				sr & 0x0020 ? '?':'.',
				sr & 0x0010 ? 'X':'.',
				sr & 0x0008 ? 'N':'.',
				sr & 0x0004 ? 'Z':'.',
				sr & 0x0002 ? 'V':'.',
				sr & 0x0001 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%08X", m68k_get_reg(NULL, M68K_REG_PC)); break;
		case CPUINFO_STR_REGISTER + M68K_SR:  			sprintf(info->s, "SR :%04X", m68k_get_reg(NULL, M68K_REG_SR)); break;
		case CPUINFO_STR_REGISTER + M68K_SP:  			sprintf(info->s, "SP :%08X", m68k_get_reg(NULL, M68K_REG_SP)); break;
		case CPUINFO_STR_REGISTER + M68K_ISP: 			sprintf(info->s, "ISP:%08X", m68k_get_reg(NULL, M68K_REG_ISP)); break;
		case CPUINFO_STR_REGISTER + M68K_USP: 			sprintf(info->s, "USP:%08X", m68k_get_reg(NULL, M68K_REG_USP)); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", m68k_get_reg(NULL, M68K_REG_D0)); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", m68k_get_reg(NULL, M68K_REG_D1)); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", m68k_get_reg(NULL, M68K_REG_D2)); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", m68k_get_reg(NULL, M68K_REG_D3)); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", m68k_get_reg(NULL, M68K_REG_D4)); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", m68k_get_reg(NULL, M68K_REG_D5)); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", m68k_get_reg(NULL, M68K_REG_D6)); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", m68k_get_reg(NULL, M68K_REG_D7)); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", m68k_get_reg(NULL, M68K_REG_A0)); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", m68k_get_reg(NULL, M68K_REG_A1)); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", m68k_get_reg(NULL, M68K_REG_A2)); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", m68k_get_reg(NULL, M68K_REG_A3)); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", m68k_get_reg(NULL, M68K_REG_A4)); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", m68k_get_reg(NULL, M68K_REG_A5)); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", m68k_get_reg(NULL, M68K_REG_A6)); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", m68k_get_reg(NULL, M68K_REG_A7)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_ADDR:		sprintf(info->s, "PAR:%08X", m68k_get_reg(NULL, M68K_REG_PREF_ADDR)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_DATA:		sprintf(info->s, "PDA:%08X", m68k_get_reg(NULL, M68K_REG_PREF_DATA)); break;
		case CPUINFO_STR_REGISTER + M68K_MSP:			sprintf(info->s, "MSP:%08X", m68k_get_reg(NULL, M68K_REG_MSP)); break;
		case CPUINFO_STR_REGISTER + M68K_CACR:			sprintf(info->s, "CCR:%08X", m68k_get_reg(NULL, M68K_REG_CACR)); break;
		case CPUINFO_STR_REGISTER + M68K_CAAR:			sprintf(info->s, "CAR:%08X", m68k_get_reg(NULL, M68K_REG_CAAR)); break;
		case CPUINFO_STR_REGISTER + M68K_SFC:			sprintf(info->s, "SFC:%X",   m68k_get_reg(NULL, M68K_REG_SFC)); break;
		case CPUINFO_STR_REGISTER + M68K_DFC:			sprintf(info->s, "DFC:%X",   m68k_get_reg(NULL, M68K_REG_DFC)); break;
		case CPUINFO_STR_REGISTER + M68K_VBR:			sprintf(info->s, "VBR:%08X", m68k_get_reg(NULL, M68K_REG_VBR)); break;
	}
}

#endif

