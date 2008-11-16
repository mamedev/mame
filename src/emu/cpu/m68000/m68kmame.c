#include <stdio.h>
#include <stdlib.h>
#include "m68kcpu.h"
#include "m68000.h"

/* global access */

void m68k_set_encrypted_opcode_range(const device_config *device, offs_t start, offs_t end)
{
	m68ki_cpu_core *m68k = device->token;
	m68k->encrypted_start = start;
	m68k->encrypted_end = end;
}

/****************************************************************************
 * 8-bit data memory interface
 ****************************************************************************/

static UINT16 m68008_read_immediate_16(const address_space *space, offs_t address)
{
	offs_t addr = address;
	return (cpu_readop(addr) << 8) | (cpu_readop(addr + 1));
}

/* interface for 20/22-bit address bus, 8-bit data bus (68008) */
static const m68k_memory_interface interface_d8 =
{
	0,
	m68008_read_immediate_16,
	memory_read_byte_8be,
	memory_read_word_8be,
	memory_read_dword_8be,
	memory_write_byte_8be,
	memory_write_word_8be,
	memory_write_dword_8be
};

/****************************************************************************
 * 16-bit data memory interface
 ****************************************************************************/

static UINT16 read_immediate_16(const address_space *space, offs_t address)
{
	m68ki_cpu_core *m68k = space->cpu->token;
	return cpu_readop16((address) ^ m68k->memory.opcode_xor);
}

static UINT16 simple_read_immediate_16(const address_space *space, offs_t address)
{
	return cpu_readop16(address);
}

/* interface for 24-bit address bus, 16-bit data bus (68000, 68010) */
static const m68k_memory_interface interface_d16 =
{
	0,
	simple_read_immediate_16,
	memory_read_byte_16be,
	memory_read_word_16be,
	memory_read_dword_16be,
	memory_write_byte_16be,
	memory_write_word_16be,
	memory_write_dword_16be
};

/****************************************************************************
 * 32-bit data memory interface
 ****************************************************************************/

/* potentially misaligned 16-bit reads with a 32-bit data bus (and 24-bit address bus) */
static UINT16 readword_d32(const address_space *space, offs_t address)
{
	UINT16 result;

	if (!(address & 1))
		return memory_read_word_32be(space, address);
	result = memory_read_byte_32be(space, address) << 8;
	return result | memory_read_byte_32be(space, address + 1);
}

/* potentially misaligned 16-bit writes with a 32-bit data bus (and 24-bit address bus) */
static void writeword_d32(const address_space *space, offs_t address, UINT16 data)
{
	if (!(address & 1))
	{
		memory_write_word_32be(space, address, data);
		return;
	}
	memory_write_byte_32be(space, address, data >> 8);
	memory_write_byte_32be(space, address + 1, data);
}

/* potentially misaligned 32-bit reads with a 32-bit data bus (and 24-bit address bus) */
static UINT32 readlong_d32(const address_space *space, offs_t address)
{
	UINT32 result;

	if (!(address & 3))
		return memory_read_dword_32be(space, address);
	else if (!(address & 1))
	{
		result = memory_read_word_32be(space, address) << 16;
		return result | memory_read_word_32be(space, address + 2);
	}
	result = memory_read_byte_32be(space, address) << 24;
	result |= memory_read_word_32be(space, address + 1) << 8;
	return result | memory_read_byte_32be(space, address + 3);
}

/* potentially misaligned 32-bit writes with a 32-bit data bus (and 24-bit address bus) */
static void writelong_d32(const address_space *space, offs_t address, UINT32 data)
{
	if (!(address & 3))
	{
		memory_write_dword_32be(space, address, data);
		return;
	}
	else if (!(address & 1))
	{
		memory_write_word_32be(space, address, data >> 16);
		memory_write_word_32be(space, address + 2, data);
		return;
	}
	memory_write_byte_32be(space, address, data >> 24);
	memory_write_word_32be(space, address + 1, data >> 8);
	memory_write_byte_32be(space, address + 3, data);
}

/* interface for 32-bit data bus (68EC020, 68020) */
static const m68k_memory_interface interface_d32 =
{
	WORD_XOR_BE(0),
	read_immediate_16,
	memory_read_byte_32be,
	readword_d32,
	readlong_d32,
	memory_write_byte_32be,
	writeword_d32,
	writelong_d32
};


static void set_irq_line(m68ki_cpu_core *m68k, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
		irqline = 7;

	m68k_set_virq(m68k, irqline, state == ASSERT_LINE ? 1 : 0);
}


/****************************************************************************
 * 68000 section
 ****************************************************************************/

static CPU_INIT( m68000 )
{
	m68ki_cpu_core *m68k = device->token;
	m68k->device = device;
	m68k->program = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	m68k->memory = interface_d16;
	m68k_init(m68k);
	m68k_set_cpu_type(m68k, M68K_CPU_TYPE_68000);
	m68k_state_register(m68k, "m68000", index);
	m68k_set_int_ack_callback(m68k, (void *)device, (int (*)(void *param, int int_level)) irqcallback);
}

static CPU_RESET( m68000 )
{
	m68k_pulse_reset(device->token);
}

static CPU_EXIT( m68000 )
{
	/* nothing to do */
}

static CPU_EXECUTE( m68000 )
{
	return m68k_execute(device->token, cycles);
}

static CPU_GET_CONTEXT( m68000 )
{
}

static CPU_SET_CONTEXT( m68000 )
{
}

static CPU_DISASSEMBLE( m68000 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68000);
}

/****************************************************************************
 * M68008 section
 ****************************************************************************/
#if HAS_M68008

static CPU_INIT( m68008 )
{
	m68ki_cpu_core *m68k = device->token;
	m68k->device = device;
	m68k->program = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	m68k->memory = interface_d8;
	m68k_init(m68k);
	m68k_set_cpu_type(m68k, M68K_CPU_TYPE_68008);
	m68k_state_register(m68k, "m68008", index);
	m68k_set_int_ack_callback(m68k, (void *)device, irqcallback);
}

static CPU_RESET( m68008 )
{
	m68k_pulse_reset(device->token);
}

static CPU_EXIT( m68008 )
{
	/* nothing to do */
}

static CPU_EXECUTE( m68008 )
{
	return m68k_execute(device->token, cycles);
}

static CPU_GET_CONTEXT( m68008 )
{
}

static CPU_SET_CONTEXT( m68008 )
{
}

static CPU_DISASSEMBLE( m68008 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68008);
}

#endif /* HAS_M68008 */

/****************************************************************************
 * M68010 section
 ****************************************************************************/
#if HAS_M68010

static CPU_INIT( m68010 )
{
	m68ki_cpu_core *m68k = device->token;
	m68k->device = device;
	m68k->program = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	m68k->memory = interface_d16;
	m68k_init(m68k);
	m68k_set_cpu_type(m68k, M68K_CPU_TYPE_68010);
	m68k_state_register(m68k, "m68010", index);
	m68k_set_int_ack_callback(m68k, (void *)device, (int (*)(void *param, int int_level)) irqcallback);
}

static CPU_DISASSEMBLE( m68010 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68010);
}

#endif /* HAS_M68010 */

/****************************************************************************
 * M68020 section
 ****************************************************************************/

static CPU_INIT( m68020 )
{
	m68ki_cpu_core *m68k = device->token;
	m68k->device = device;
	m68k->program = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	m68k->memory = interface_d32;
	m68k_init(m68k);
	m68k_set_cpu_type(m68k, M68K_CPU_TYPE_68020);
	m68k_state_register(m68k, "m68020", index);
	m68k_set_int_ack_callback(m68k, (void *)device, (int (*)(void *param, int int_level)) irqcallback);
}

static CPU_RESET( m68020 )
{
	m68k_pulse_reset(device->token);
}

static CPU_EXIT( m68020 )
{
	/* nothing to do */
}

static CPU_EXECUTE( m68020 )
{
	return m68k_execute(device->token, cycles);
}

static CPU_GET_CONTEXT( m68020 )
{
}

static CPU_SET_CONTEXT( m68020 )
{
}

static CPU_DISASSEMBLE( m68020 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68020);
}


/****************************************************************************
 * M680EC20 section
 ****************************************************************************/
#if HAS_M68EC020

static CPU_INIT( m68ec020 )
{
	m68ki_cpu_core *m68k = device->token;
	m68k->device = device;
	m68k->program = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	m68k->memory = interface_d32;
	m68k_init(m68k);
	m68k_set_cpu_type(m68k, M68K_CPU_TYPE_68EC020);
	m68k_state_register(m68k, "m68ec020", index);
	m68k_set_int_ack_callback(m68k, (void *)device, (int (*)(void *param, int int_level)) irqcallback);
}

static CPU_DISASSEMBLE( m68ec020 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68EC020);
}

#endif /* HAS_M68EC020 */

/****************************************************************************
 * M68040 section
 ****************************************************************************/
#if HAS_M68040

static CPU_INIT( m68040 )
{
	m68ki_cpu_core *m68k = device->token;
	m68k->device = device;
	m68k->program = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	m68k->memory = interface_d32;
	m68k_init(m68k);
	m68k_set_cpu_type(m68k, M68K_CPU_TYPE_68040);
	m68k_state_register(m68k, "m68040", index);
	m68k_set_int_ack_callback(m68k, (void *)device, (int (*)(void *param, int int_level)) irqcallback);
}

static CPU_RESET( m68040 )
{
	m68k_pulse_reset(device->token);
}

static CPU_EXIT( m68040 )
{
	/* nothing to do */
}

static CPU_EXECUTE( m68040 )
{
	return m68k_execute(device->token, cycles);
}

static CPU_GET_CONTEXT( m68040 )
{
}

static CPU_SET_CONTEXT( m68040 )
{
}

static CPU_DISASSEMBLE( m68040 )
{
	return m68k_disassemble_raw(buffer, pc, oprom, opram, M68K_CPU_TYPE_68040);
}

#endif /* HAS_M68040 */

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( m68000 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			set_irq_line(m68k, 0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 1:			set_irq_line(m68k, 1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 2:			set_irq_line(m68k, 2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 3:			set_irq_line(m68k, 3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 4:			set_irq_line(m68k, 4, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 5:			set_irq_line(m68k, 5, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 6:			set_irq_line(m68k, 6, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 7:			set_irq_line(m68k, 7, info->i);					break;

		case CPUINFO_INT_PC:  						m68k_set_reg(m68k, M68K_REG_PC, info->i&0x00ffffff); break;
		case CPUINFO_INT_REGISTER + M68K_PC:  		m68k_set_reg(m68k, M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:  		m68k_set_reg(m68k, M68K_REG_SP, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_ISP: 		m68k_set_reg(m68k, M68K_REG_ISP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_USP: 		m68k_set_reg(m68k, M68K_REG_USP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_SR:  		m68k_set_reg(m68k, M68K_REG_SR, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D0:  		m68k_set_reg(m68k, M68K_REG_D0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D1:  		m68k_set_reg(m68k, M68K_REG_D1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D2:  		m68k_set_reg(m68k, M68K_REG_D2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D3:  		m68k_set_reg(m68k, M68K_REG_D3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D4:  		m68k_set_reg(m68k, M68K_REG_D4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D5:  		m68k_set_reg(m68k, M68K_REG_D5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D6:  		m68k_set_reg(m68k, M68K_REG_D6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D7:  		m68k_set_reg(m68k, M68K_REG_D7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A0:  		m68k_set_reg(m68k, M68K_REG_A0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A1:  		m68k_set_reg(m68k, M68K_REG_A1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A2:  		m68k_set_reg(m68k, M68K_REG_A2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A3:  		m68k_set_reg(m68k, M68K_REG_A3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A4:  		m68k_set_reg(m68k, M68K_REG_A4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A5:  		m68k_set_reg(m68k, M68K_REG_A5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A6:  		m68k_set_reg(m68k, M68K_REG_A6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A7:  		m68k_set_reg(m68k, M68K_REG_A7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:	m68k_set_reg(m68k, M68K_REG_PREF_ADDR, info->i);	break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		m68k_set_reset_instr_callback(m68k, info->f);		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		m68k_set_cmpild_instr_callback(m68k, (void (*)(unsigned int,int))(info->f)); break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:			m68k_set_rte_instr_callback(m68k, info->f);		break;
		case CPUINFO_PTR_M68K_TAS_CALLBACK:			m68k_set_tas_instr_callback(m68k, (int (*)(void))(info->f)); break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( m68000 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m68ki_cpu_core);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
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

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;  /* there is no level 0 */	break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = m68k_get_virq(m68k, 1);				break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = m68k_get_virq(m68k, 2);				break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = m68k_get_virq(m68k, 3);				break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = m68k_get_virq(m68k, 4);				break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = m68k_get_virq(m68k, 5);				break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = m68k_get_virq(m68k, 6);				break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = m68k_get_virq(m68k, 7);				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m68k_get_reg(m68k, M68K_REG_PPC); break;

		case CPUINFO_INT_PC:							info->i = m68k_get_reg(m68k, M68K_REG_PC)&0x00ffffff; break;
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = m68k_get_reg(m68k, M68K_REG_PC); break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:			info->i = m68k_get_reg(m68k, M68K_REG_SP); break;
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = m68k_get_reg(m68k, M68K_REG_ISP); break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = m68k_get_reg(m68k, M68K_REG_USP); break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = m68k_get_reg(m68k, M68K_REG_SR); break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = m68k_get_reg(m68k, M68K_REG_D0); break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = m68k_get_reg(m68k, M68K_REG_D1); break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = m68k_get_reg(m68k, M68K_REG_D2); break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = m68k_get_reg(m68k, M68K_REG_D3); break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = m68k_get_reg(m68k, M68K_REG_D4); break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = m68k_get_reg(m68k, M68K_REG_D5); break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = m68k_get_reg(m68k, M68K_REG_D6); break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = m68k_get_reg(m68k, M68K_REG_D7); break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = m68k_get_reg(m68k, M68K_REG_A0); break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = m68k_get_reg(m68k, M68K_REG_A1); break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = m68k_get_reg(m68k, M68K_REG_A2); break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = m68k_get_reg(m68k, M68K_REG_A3); break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = m68k_get_reg(m68k, M68K_REG_A4); break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = m68k_get_reg(m68k, M68K_REG_A5); break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = m68k_get_reg(m68k, M68K_REG_A6); break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = m68k_get_reg(m68k, M68K_REG_A7); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:		info->i = m68k_get_reg(m68k, M68K_REG_PREF_ADDR); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_DATA:		info->i = m68k_get_reg(m68k, M68K_REG_PREF_DATA); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m68000);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(m68000);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(m68000);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m68000);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(m68000);				break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(m68000);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m68000);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m68000);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k->remaining_cycles;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68000");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "3.32");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Karl Stenerud. All rights reserved. (2.1 fixes HJB)"); break;

		case CPUINFO_STR_FLAGS:
			sr = m68k_get_reg(m68k, M68K_REG_SR);
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

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%08X", m68k_get_reg(m68k, M68K_REG_PC)); break;
		case CPUINFO_STR_REGISTER + M68K_SR:  			sprintf(info->s, "SR :%04X", m68k_get_reg(m68k, M68K_REG_SR)); break;
		case CPUINFO_STR_REGISTER + M68K_SP:  			sprintf(info->s, "SP :%08X", m68k_get_reg(m68k, M68K_REG_SP)); break;
		case CPUINFO_STR_REGISTER + M68K_ISP: 			sprintf(info->s, "ISP:%08X", m68k_get_reg(m68k, M68K_REG_ISP)); break;
		case CPUINFO_STR_REGISTER + M68K_USP: 			sprintf(info->s, "USP:%08X", m68k_get_reg(m68k, M68K_REG_USP)); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", m68k_get_reg(m68k, M68K_REG_D0)); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", m68k_get_reg(m68k, M68K_REG_D1)); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", m68k_get_reg(m68k, M68K_REG_D2)); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", m68k_get_reg(m68k, M68K_REG_D3)); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", m68k_get_reg(m68k, M68K_REG_D4)); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", m68k_get_reg(m68k, M68K_REG_D5)); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", m68k_get_reg(m68k, M68K_REG_D6)); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", m68k_get_reg(m68k, M68K_REG_D7)); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", m68k_get_reg(m68k, M68K_REG_A0)); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", m68k_get_reg(m68k, M68K_REG_A1)); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", m68k_get_reg(m68k, M68K_REG_A2)); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", m68k_get_reg(m68k, M68K_REG_A3)); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", m68k_get_reg(m68k, M68K_REG_A4)); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", m68k_get_reg(m68k, M68K_REG_A5)); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", m68k_get_reg(m68k, M68K_REG_A6)); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", m68k_get_reg(m68k, M68K_REG_A7)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_ADDR:		sprintf(info->s, "PAR:%08X", m68k_get_reg(m68k, M68K_REG_PREF_ADDR)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_DATA:		sprintf(info->s, "PDA:%08X", m68k_get_reg(m68k, M68K_REG_PREF_DATA)); break;
	}
}

#if HAS_M68008

static CPU_SET_INFO( m68008 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			set_irq_line(m68k, 0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 1:			set_irq_line(m68k, 1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 2:			set_irq_line(m68k, 2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 3:			set_irq_line(m68k, 3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 4:			set_irq_line(m68k, 4, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 5:			set_irq_line(m68k, 5, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 6:			set_irq_line(m68k, 6, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 7:			set_irq_line(m68k, 7, info->i);					break;

		case CPUINFO_INT_PC:  						m68k_set_reg(m68k, M68K_REG_PC, info->i&0x00ffffff); break;
		case CPUINFO_INT_REGISTER + M68K_PC:  		m68k_set_reg(m68k, M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:  		m68k_set_reg(m68k, M68K_REG_SP, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_ISP: 		m68k_set_reg(m68k, M68K_REG_ISP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_USP: 		m68k_set_reg(m68k, M68K_REG_USP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_SR:  		m68k_set_reg(m68k, M68K_REG_SR, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D0:  		m68k_set_reg(m68k, M68K_REG_D0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D1:  		m68k_set_reg(m68k, M68K_REG_D1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D2:  		m68k_set_reg(m68k, M68K_REG_D2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D3:  		m68k_set_reg(m68k, M68K_REG_D3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D4:  		m68k_set_reg(m68k, M68K_REG_D4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D5:  		m68k_set_reg(m68k, M68K_REG_D5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D6:  		m68k_set_reg(m68k, M68K_REG_D6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D7:  		m68k_set_reg(m68k, M68K_REG_D7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A0:  		m68k_set_reg(m68k, M68K_REG_A0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A1:  		m68k_set_reg(m68k, M68K_REG_A1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A2:  		m68k_set_reg(m68k, M68K_REG_A2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A3:  		m68k_set_reg(m68k, M68K_REG_A3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A4:  		m68k_set_reg(m68k, M68K_REG_A4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A5:  		m68k_set_reg(m68k, M68K_REG_A5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A6:  		m68k_set_reg(m68k, M68K_REG_A6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A7:  		m68k_set_reg(m68k, M68K_REG_A7, info->i);			break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		m68k_set_reset_instr_callback(m68k, info->f);		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		m68k_set_cmpild_instr_callback(m68k, (void (*)(unsigned int,int))(info->f)); break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:			m68k_set_rte_instr_callback(m68k, info->f);		break;
	}
}

CPU_GET_INFO( m68008 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m68ki_cpu_core);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
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

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;  /* there is no level 0 */	break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = m68k_get_virq(m68k, 1);				break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = m68k_get_virq(m68k, 2);				break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = m68k_get_virq(m68k, 3);				break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = m68k_get_virq(m68k, 4);				break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = m68k_get_virq(m68k, 5);				break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = m68k_get_virq(m68k, 6);				break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = m68k_get_virq(m68k, 7);				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m68k_get_reg(m68k, M68K_REG_PPC); break;

		case CPUINFO_INT_PC:							info->i = m68k_get_reg(m68k, M68K_REG_PC)&0x00ffffff; break;
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = m68k_get_reg(m68k, M68K_REG_PC); break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:			info->i = m68k_get_reg(m68k, M68K_REG_SP); break;
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = m68k_get_reg(m68k, M68K_REG_ISP); break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = m68k_get_reg(m68k, M68K_REG_USP); break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = m68k_get_reg(m68k, M68K_REG_SR); break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = m68k_get_reg(m68k, M68K_REG_D0); break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = m68k_get_reg(m68k, M68K_REG_D1); break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = m68k_get_reg(m68k, M68K_REG_D2); break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = m68k_get_reg(m68k, M68K_REG_D3); break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = m68k_get_reg(m68k, M68K_REG_D4); break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = m68k_get_reg(m68k, M68K_REG_D5); break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = m68k_get_reg(m68k, M68K_REG_D6); break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = m68k_get_reg(m68k, M68K_REG_D7); break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = m68k_get_reg(m68k, M68K_REG_A0); break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = m68k_get_reg(m68k, M68K_REG_A1); break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = m68k_get_reg(m68k, M68K_REG_A2); break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = m68k_get_reg(m68k, M68K_REG_A3); break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = m68k_get_reg(m68k, M68K_REG_A4); break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = m68k_get_reg(m68k, M68K_REG_A5); break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = m68k_get_reg(m68k, M68K_REG_A6); break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = m68k_get_reg(m68k, M68K_REG_A7); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:		info->i = m68k_get_reg(m68k, M68K_REG_PREF_ADDR); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_DATA:		info->i = m68k_get_reg(m68k, M68K_REG_PREF_DATA); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m68008);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(m68008);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(m68008);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m68008);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(m68008);				break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(m68008);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m68008);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m68008);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k->remaining_cycles;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68008");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "3.32");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Karl Stenerud. All rights reserved. (2.1 fixes HJB)"); break;

		case CPUINFO_STR_FLAGS:
			sr = m68k_get_reg(m68k, M68K_REG_SR);
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

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%08X", m68k_get_reg(m68k, M68K_REG_PC)); break;
		case CPUINFO_STR_REGISTER + M68K_SR:  			sprintf(info->s, "SR :%04X", m68k_get_reg(m68k, M68K_REG_SR)); break;
		case CPUINFO_STR_REGISTER + M68K_SP:  			sprintf(info->s, "SP :%08X", m68k_get_reg(m68k, M68K_REG_SP)); break;
		case CPUINFO_STR_REGISTER + M68K_ISP: 			sprintf(info->s, "ISP:%08X", m68k_get_reg(m68k, M68K_REG_ISP)); break;
		case CPUINFO_STR_REGISTER + M68K_USP: 			sprintf(info->s, "USP:%08X", m68k_get_reg(m68k, M68K_REG_USP)); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", m68k_get_reg(m68k, M68K_REG_D0)); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", m68k_get_reg(m68k, M68K_REG_D1)); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", m68k_get_reg(m68k, M68K_REG_D2)); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", m68k_get_reg(m68k, M68K_REG_D3)); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", m68k_get_reg(m68k, M68K_REG_D4)); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", m68k_get_reg(m68k, M68K_REG_D5)); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", m68k_get_reg(m68k, M68K_REG_D6)); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", m68k_get_reg(m68k, M68K_REG_D7)); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", m68k_get_reg(m68k, M68K_REG_A0)); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", m68k_get_reg(m68k, M68K_REG_A1)); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", m68k_get_reg(m68k, M68K_REG_A2)); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", m68k_get_reg(m68k, M68K_REG_A3)); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", m68k_get_reg(m68k, M68K_REG_A4)); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", m68k_get_reg(m68k, M68K_REG_A5)); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", m68k_get_reg(m68k, M68K_REG_A6)); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", m68k_get_reg(m68k, M68K_REG_A7)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_ADDR:		sprintf(info->s, "PAR:%08X", m68k_get_reg(m68k, M68K_REG_PREF_ADDR)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_DATA:		sprintf(info->s, "PDA:%08X", m68k_get_reg(m68k, M68K_REG_PREF_DATA)); break;
	}
}

#endif /* HAS_M68008 */

/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/
#if HAS_M68010

static CPU_SET_INFO( m68010 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_REGISTER + M68K_VBR:  			m68k_set_reg(m68k, M68K_REG_VBR, info->i);	break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  			m68k_set_reg(m68k, M68K_REG_SFC, info->i);	break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  			m68k_set_reg(m68k, M68K_REG_DFC, info->i);	break;

		default:										CPU_SET_INFO_CALL(m68000);					break;
	}
}

CPU_GET_INFO( m68010 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_REGISTER + M68K_VBR:  			info->i = m68k_get_reg(m68k, M68K_REG_VBR); break;
		case CPUINFO_INT_REGISTER + M68K_SFC:  			info->i = m68k_get_reg(m68k, M68K_REG_SFC); break;
		case CPUINFO_INT_REGISTER + M68K_DFC:  			info->i = m68k_get_reg(m68k, M68K_REG_DFC); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m68010);		break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m68010);				break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m68010);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68010");				break;
		case CPUINFO_STR_REGISTER + M68K_SFC:			sprintf(info->s, "SFC:%X",   m68k_get_reg(m68k, M68K_REG_SFC)); break;
		case CPUINFO_STR_REGISTER + M68K_DFC:			sprintf(info->s, "DFC:%X",   m68k_get_reg(m68k, M68K_REG_DFC)); break;
		case CPUINFO_STR_REGISTER + M68K_VBR:			sprintf(info->s, "VBR:%08X", m68k_get_reg(m68k, M68K_REG_VBR)); break;

		default:										CPU_GET_INFO_CALL(m68000);					break;
	}
}

#endif /* HAS_M68010 */

/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

static CPU_SET_INFO( m68020 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			set_irq_line(m68k, 0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 1:			set_irq_line(m68k, 1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 2:			set_irq_line(m68k, 2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 3:			set_irq_line(m68k, 3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 4:			set_irq_line(m68k, 4, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 5:			set_irq_line(m68k, 5, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 6:			set_irq_line(m68k, 6, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 7:			set_irq_line(m68k, 7, info->i);					break;

		case CPUINFO_INT_PC:  						m68k_set_reg(m68k, M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_PC:  		m68k_set_reg(m68k, M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:  		m68k_set_reg(m68k, M68K_REG_SP, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_ISP: 		m68k_set_reg(m68k, M68K_REG_ISP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_USP: 		m68k_set_reg(m68k, M68K_REG_USP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_SR:  		m68k_set_reg(m68k, M68K_REG_SR, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D0:  		m68k_set_reg(m68k, M68K_REG_D0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D1:  		m68k_set_reg(m68k, M68K_REG_D1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D2:  		m68k_set_reg(m68k, M68K_REG_D2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D3:  		m68k_set_reg(m68k, M68K_REG_D3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D4:  		m68k_set_reg(m68k, M68K_REG_D4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D5:  		m68k_set_reg(m68k, M68K_REG_D5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D6:  		m68k_set_reg(m68k, M68K_REG_D6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D7:  		m68k_set_reg(m68k, M68K_REG_D7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A0:  		m68k_set_reg(m68k, M68K_REG_A0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A1:  		m68k_set_reg(m68k, M68K_REG_A1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A2:  		m68k_set_reg(m68k, M68K_REG_A2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A3:  		m68k_set_reg(m68k, M68K_REG_A3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A4:  		m68k_set_reg(m68k, M68K_REG_A4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A5:  		m68k_set_reg(m68k, M68K_REG_A5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A6:  		m68k_set_reg(m68k, M68K_REG_A6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A7:  		m68k_set_reg(m68k, M68K_REG_A7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_MSP:		m68k_set_reg(m68k, M68K_REG_MSP, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_CACR:		m68k_set_reg(m68k, M68K_REG_CACR, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_CAAR:		m68k_set_reg(m68k, M68K_REG_CAAR, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_VBR:		m68k_set_reg(m68k, M68K_REG_VBR, info->i);		break; /* 68010+ */
		case CPUINFO_INT_REGISTER + M68K_SFC:		m68k_set_reg(m68k, M68K_REG_SFC, info->i);		break; /* 68010+ */
		case CPUINFO_INT_REGISTER + M68K_DFC:		m68k_set_reg(m68k, M68K_REG_DFC, info->i);		break; /* 68010+ */

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		m68k_set_reset_instr_callback(m68k, info->f);		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		m68k_set_cmpild_instr_callback(m68k, (void (*)(unsigned int,int))(info->f)); break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:			m68k_set_rte_instr_callback(m68k, info->f);		break;
	}
}

CPU_GET_INFO( m68020 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m68ki_cpu_core);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
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

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;  /* there is no level 0 */	break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = m68k_get_virq(m68k, 1);				break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = m68k_get_virq(m68k, 2);				break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = m68k_get_virq(m68k, 3);				break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = m68k_get_virq(m68k, 4);				break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = m68k_get_virq(m68k, 5);				break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = m68k_get_virq(m68k, 6);				break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = m68k_get_virq(m68k, 7);				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m68k_get_reg(m68k, M68K_REG_PPC); break;

		case CPUINFO_INT_PC:							info->i = m68k_get_reg(m68k, M68K_REG_PC); break;
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = m68k_get_reg(m68k, M68K_REG_PC); break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:			info->i = m68k_get_reg(m68k, M68K_REG_SP); break;
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = m68k_get_reg(m68k, M68K_REG_ISP); break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = m68k_get_reg(m68k, M68K_REG_USP); break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = m68k_get_reg(m68k, M68K_REG_SR); break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = m68k_get_reg(m68k, M68K_REG_D0); break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = m68k_get_reg(m68k, M68K_REG_D1); break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = m68k_get_reg(m68k, M68K_REG_D2); break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = m68k_get_reg(m68k, M68K_REG_D3); break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = m68k_get_reg(m68k, M68K_REG_D4); break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = m68k_get_reg(m68k, M68K_REG_D5); break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = m68k_get_reg(m68k, M68K_REG_D6); break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = m68k_get_reg(m68k, M68K_REG_D7); break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = m68k_get_reg(m68k, M68K_REG_A0); break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = m68k_get_reg(m68k, M68K_REG_A1); break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = m68k_get_reg(m68k, M68K_REG_A2); break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = m68k_get_reg(m68k, M68K_REG_A3); break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = m68k_get_reg(m68k, M68K_REG_A4); break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = m68k_get_reg(m68k, M68K_REG_A5); break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = m68k_get_reg(m68k, M68K_REG_A6); break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = m68k_get_reg(m68k, M68K_REG_A7); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:		info->i = m68k_get_reg(m68k, M68K_REG_PREF_ADDR); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_DATA:		info->i = m68k_get_reg(m68k, M68K_REG_PREF_DATA); break;
		case CPUINFO_INT_REGISTER + M68K_MSP:			info->i = m68k_get_reg(m68k, M68K_REG_MSP); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_CACR: 			info->i = m68k_get_reg(m68k, M68K_REG_CACR); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_CAAR: 			info->i = m68k_get_reg(m68k, M68K_REG_CAAR); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_VBR:			info->i = m68k_get_reg(m68k, M68K_REG_VBR); /* 68010+ */ break;
		case CPUINFO_INT_REGISTER + M68K_SFC:			info->i = m68k_get_reg(m68k, M68K_REG_SFC); /* 68010" */ break;
		case CPUINFO_INT_REGISTER + M68K_DFC:			info->i = m68k_get_reg(m68k, M68K_REG_DFC); /* 68010+ */ break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m68020);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(m68020);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(m68020);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m68020);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(m68020);				break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(m68020);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m68020);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m68020);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k->remaining_cycles;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68020");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "3.32");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Karl Stenerud. All rights reserved. (2.1 fixes HJB)"); break;

		case CPUINFO_STR_FLAGS:
			sr = m68k_get_reg(m68k, M68K_REG_SR);
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

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%08X", m68k_get_reg(m68k, M68K_REG_PC)); break;
		case CPUINFO_STR_REGISTER + M68K_SR:  			sprintf(info->s, "SR :%04X", m68k_get_reg(m68k, M68K_REG_SR)); break;
		case CPUINFO_STR_REGISTER + M68K_SP:  			sprintf(info->s, "SP :%08X", m68k_get_reg(m68k, M68K_REG_SP)); break;
		case CPUINFO_STR_REGISTER + M68K_ISP: 			sprintf(info->s, "ISP:%08X", m68k_get_reg(m68k, M68K_REG_ISP)); break;
		case CPUINFO_STR_REGISTER + M68K_USP: 			sprintf(info->s, "USP:%08X", m68k_get_reg(m68k, M68K_REG_USP)); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", m68k_get_reg(m68k, M68K_REG_D0)); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", m68k_get_reg(m68k, M68K_REG_D1)); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", m68k_get_reg(m68k, M68K_REG_D2)); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", m68k_get_reg(m68k, M68K_REG_D3)); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", m68k_get_reg(m68k, M68K_REG_D4)); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", m68k_get_reg(m68k, M68K_REG_D5)); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", m68k_get_reg(m68k, M68K_REG_D6)); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", m68k_get_reg(m68k, M68K_REG_D7)); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", m68k_get_reg(m68k, M68K_REG_A0)); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", m68k_get_reg(m68k, M68K_REG_A1)); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", m68k_get_reg(m68k, M68K_REG_A2)); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", m68k_get_reg(m68k, M68K_REG_A3)); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", m68k_get_reg(m68k, M68K_REG_A4)); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", m68k_get_reg(m68k, M68K_REG_A5)); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", m68k_get_reg(m68k, M68K_REG_A6)); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", m68k_get_reg(m68k, M68K_REG_A7)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_ADDR:		sprintf(info->s, "PAR:%08X", m68k_get_reg(m68k, M68K_REG_PREF_ADDR)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_DATA:		sprintf(info->s, "PDA:%08X", m68k_get_reg(m68k, M68K_REG_PREF_DATA)); break;
		case CPUINFO_STR_REGISTER + M68K_MSP:			sprintf(info->s, "MSP:%08X", m68k_get_reg(m68k, M68K_REG_MSP)); break;
		case CPUINFO_STR_REGISTER + M68K_CACR:			sprintf(info->s, "CCR:%08X", m68k_get_reg(m68k, M68K_REG_CACR)); break;
		case CPUINFO_STR_REGISTER + M68K_CAAR:			sprintf(info->s, "CAR:%08X", m68k_get_reg(m68k, M68K_REG_CAAR)); break;
		case CPUINFO_STR_REGISTER + M68K_SFC:			sprintf(info->s, "SFC:%X",   m68k_get_reg(m68k, M68K_REG_SFC)); break;
		case CPUINFO_STR_REGISTER + M68K_DFC:			sprintf(info->s, "DFC:%X",   m68k_get_reg(m68k, M68K_REG_DFC)); break;
		case CPUINFO_STR_REGISTER + M68K_VBR:			sprintf(info->s, "VBR:%08X", m68k_get_reg(m68k, M68K_REG_VBR)); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/
#if HAS_M68EC020

static CPU_SET_INFO( m68ec020 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:  							m68k_set_reg(m68k, M68K_REG_PC, info->i&0x00ffffff); break;

		default:										CPU_SET_INFO_CALL(m68020);					break;
	}
}

CPU_GET_INFO( m68ec020 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 24;					break;
		case CPUINFO_INT_PC:							info->i = m68k_get_reg(m68k, M68K_REG_PC)&0x00ffffff; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m68ec020);		break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m68ec020);				break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m68ec020);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68EC020");				break;

		default:										CPU_GET_INFO_CALL(m68020);					break;
	}
}

#endif /* HAS_M68EC020 */

/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

#if HAS_M68040
static CPU_SET_INFO( m68040 )
{
	m68ki_cpu_core *m68k = device->token;
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:			set_irq_line(m68k, 0, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 1:			set_irq_line(m68k, 1, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 2:			set_irq_line(m68k, 2, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 3:			set_irq_line(m68k, 3, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 4:			set_irq_line(m68k, 4, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 5:			set_irq_line(m68k, 5, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 6:			set_irq_line(m68k, 6, info->i);					break;
		case CPUINFO_INT_INPUT_STATE + 7:			set_irq_line(m68k, 7, info->i);					break;

		case CPUINFO_INT_PC:  						m68k_set_reg(m68k, M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_PC:  		m68k_set_reg(m68k, M68K_REG_PC, info->i);			break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:  		m68k_set_reg(m68k, M68K_REG_SP, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_ISP: 		m68k_set_reg(m68k, M68K_REG_ISP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_USP: 		m68k_set_reg(m68k, M68K_REG_USP, info->i);		break;
		case CPUINFO_INT_REGISTER + M68K_SR:  		m68k_set_reg(m68k, M68K_REG_SR, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D0:  		m68k_set_reg(m68k, M68K_REG_D0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D1:  		m68k_set_reg(m68k, M68K_REG_D1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D2:  		m68k_set_reg(m68k, M68K_REG_D2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D3:  		m68k_set_reg(m68k, M68K_REG_D3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D4:  		m68k_set_reg(m68k, M68K_REG_D4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D5:  		m68k_set_reg(m68k, M68K_REG_D5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D6:  		m68k_set_reg(m68k, M68K_REG_D6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_D7:  		m68k_set_reg(m68k, M68K_REG_D7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A0:  		m68k_set_reg(m68k, M68K_REG_A0, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A1:  		m68k_set_reg(m68k, M68K_REG_A1, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A2:  		m68k_set_reg(m68k, M68K_REG_A2, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A3:  		m68k_set_reg(m68k, M68K_REG_A3, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A4:  		m68k_set_reg(m68k, M68K_REG_A4, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A5:  		m68k_set_reg(m68k, M68K_REG_A5, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A6:  		m68k_set_reg(m68k, M68K_REG_A6, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_A7:  		m68k_set_reg(m68k, M68K_REG_A7, info->i);			break;
		case CPUINFO_INT_REGISTER + M68K_MSP:		m68k_set_reg(m68k, M68K_REG_MSP, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_CACR:		m68k_set_reg(m68k, M68K_REG_CACR, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_CAAR:		m68k_set_reg(m68k, M68K_REG_CAAR, info->i);		break; /* 68020+ */
		case CPUINFO_INT_REGISTER + M68K_VBR:		m68k_set_reg(m68k, M68K_REG_VBR, info->i);		break; /* 68010+ */
		case CPUINFO_INT_REGISTER + M68K_SFC:		m68k_set_reg(m68k, M68K_REG_SFC, info->i);		break; /* 68010+ */
		case CPUINFO_INT_REGISTER + M68K_DFC:		m68k_set_reg(m68k, M68K_REG_DFC, info->i);		break; /* 68010+ */

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_M68K_RESET_CALLBACK:		m68k_set_reset_instr_callback(m68k, info->f);		break;
		case CPUINFO_PTR_M68K_CMPILD_CALLBACK:		m68k_set_cmpild_instr_callback(m68k, (void (*)(unsigned int,int))(info->f)); break;
		case CPUINFO_PTR_M68K_RTE_CALLBACK:			m68k_set_rte_instr_callback(m68k, info->f);		break;
	}
}

CPU_GET_INFO( m68040 )
{
	m68ki_cpu_core *m68k = (device != NULL) ? device->token : NULL;
	int sr;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(m68ki_cpu_core);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = -1;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 20;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 2;							break;
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

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;  /* there is no level 0 */	break;
		case CPUINFO_INT_INPUT_STATE + 1:				info->i = m68k_get_virq(m68k, 1);				break;
		case CPUINFO_INT_INPUT_STATE + 2:				info->i = m68k_get_virq(m68k, 2);				break;
		case CPUINFO_INT_INPUT_STATE + 3:				info->i = m68k_get_virq(m68k, 3);				break;
		case CPUINFO_INT_INPUT_STATE + 4:				info->i = m68k_get_virq(m68k, 4);				break;
		case CPUINFO_INT_INPUT_STATE + 5:				info->i = m68k_get_virq(m68k, 5);				break;
		case CPUINFO_INT_INPUT_STATE + 6:				info->i = m68k_get_virq(m68k, 6);				break;
		case CPUINFO_INT_INPUT_STATE + 7:				info->i = m68k_get_virq(m68k, 7);				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = m68k_get_reg(m68k, M68K_REG_PPC); break;

		case CPUINFO_INT_PC:							info->i = m68k_get_reg(m68k, M68K_REG_PC); break;
		case CPUINFO_INT_REGISTER + M68K_PC:			info->i = m68k_get_reg(m68k, M68K_REG_PC); break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + M68K_SP:			info->i = m68k_get_reg(m68k, M68K_REG_SP); break;
		case CPUINFO_INT_REGISTER + M68K_ISP:			info->i = m68k_get_reg(m68k, M68K_REG_ISP); break;
		case CPUINFO_INT_REGISTER + M68K_USP:			info->i = m68k_get_reg(m68k, M68K_REG_USP); break;
		case CPUINFO_INT_REGISTER + M68K_SR:			info->i = m68k_get_reg(m68k, M68K_REG_SR); break;
		case CPUINFO_INT_REGISTER + M68K_D0:			info->i = m68k_get_reg(m68k, M68K_REG_D0); break;
		case CPUINFO_INT_REGISTER + M68K_D1:			info->i = m68k_get_reg(m68k, M68K_REG_D1); break;
		case CPUINFO_INT_REGISTER + M68K_D2:			info->i = m68k_get_reg(m68k, M68K_REG_D2); break;
		case CPUINFO_INT_REGISTER + M68K_D3:			info->i = m68k_get_reg(m68k, M68K_REG_D3); break;
		case CPUINFO_INT_REGISTER + M68K_D4:			info->i = m68k_get_reg(m68k, M68K_REG_D4); break;
		case CPUINFO_INT_REGISTER + M68K_D5:			info->i = m68k_get_reg(m68k, M68K_REG_D5); break;
		case CPUINFO_INT_REGISTER + M68K_D6:			info->i = m68k_get_reg(m68k, M68K_REG_D6); break;
		case CPUINFO_INT_REGISTER + M68K_D7:			info->i = m68k_get_reg(m68k, M68K_REG_D7); break;
		case CPUINFO_INT_REGISTER + M68K_A0:			info->i = m68k_get_reg(m68k, M68K_REG_A0); break;
		case CPUINFO_INT_REGISTER + M68K_A1:			info->i = m68k_get_reg(m68k, M68K_REG_A1); break;
		case CPUINFO_INT_REGISTER + M68K_A2:			info->i = m68k_get_reg(m68k, M68K_REG_A2); break;
		case CPUINFO_INT_REGISTER + M68K_A3:			info->i = m68k_get_reg(m68k, M68K_REG_A3); break;
		case CPUINFO_INT_REGISTER + M68K_A4:			info->i = m68k_get_reg(m68k, M68K_REG_A4); break;
		case CPUINFO_INT_REGISTER + M68K_A5:			info->i = m68k_get_reg(m68k, M68K_REG_A5); break;
		case CPUINFO_INT_REGISTER + M68K_A6:			info->i = m68k_get_reg(m68k, M68K_REG_A6); break;
		case CPUINFO_INT_REGISTER + M68K_A7:			info->i = m68k_get_reg(m68k, M68K_REG_A7); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_ADDR:		info->i = m68k_get_reg(m68k, M68K_REG_PREF_ADDR); break;
		case CPUINFO_INT_REGISTER + M68K_PREF_DATA:		info->i = m68k_get_reg(m68k, M68K_REG_PREF_DATA); break;
		case CPUINFO_INT_REGISTER + M68K_MSP:			info->i = m68k_get_reg(m68k, M68K_REG_MSP); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_CACR: 			info->i = m68k_get_reg(m68k, M68K_REG_CACR); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_CAAR: 			info->i = m68k_get_reg(m68k, M68K_REG_CAAR); /* 68020+ */ break;
		case CPUINFO_INT_REGISTER + M68K_VBR:			info->i = m68k_get_reg(m68k, M68K_REG_VBR); /* 68010+ */ break;
		case CPUINFO_INT_REGISTER + M68K_SFC:			info->i = m68k_get_reg(m68k, M68K_REG_SFC); /* 68010" */ break;
		case CPUINFO_INT_REGISTER + M68K_DFC:			info->i = m68k_get_reg(m68k, M68K_REG_DFC); /* 68010+ */ break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(m68040);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(m68040);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(m68040);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(m68040);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(m68040);				break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(m68040);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(m68040);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(m68040);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &m68k->remaining_cycles;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "68040");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Motorola 68K");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "3.32");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Karl Stenerud. All rights reserved. (2.1 fixes HJB)"); break;

		case CPUINFO_STR_FLAGS:
			sr = m68k_get_reg(m68k, M68K_REG_SR);
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

		case CPUINFO_STR_REGISTER + M68K_PC:			sprintf(info->s, "PC :%08X", m68k_get_reg(m68k, M68K_REG_PC)); break;
		case CPUINFO_STR_REGISTER + M68K_SR:  			sprintf(info->s, "SR :%04X", m68k_get_reg(m68k, M68K_REG_SR)); break;
		case CPUINFO_STR_REGISTER + M68K_SP:  			sprintf(info->s, "SP :%08X", m68k_get_reg(m68k, M68K_REG_SP)); break;
		case CPUINFO_STR_REGISTER + M68K_ISP: 			sprintf(info->s, "ISP:%08X", m68k_get_reg(m68k, M68K_REG_ISP)); break;
		case CPUINFO_STR_REGISTER + M68K_USP: 			sprintf(info->s, "USP:%08X", m68k_get_reg(m68k, M68K_REG_USP)); break;
		case CPUINFO_STR_REGISTER + M68K_D0:			sprintf(info->s, "D0 :%08X", m68k_get_reg(m68k, M68K_REG_D0)); break;
		case CPUINFO_STR_REGISTER + M68K_D1:			sprintf(info->s, "D1 :%08X", m68k_get_reg(m68k, M68K_REG_D1)); break;
		case CPUINFO_STR_REGISTER + M68K_D2:			sprintf(info->s, "D2 :%08X", m68k_get_reg(m68k, M68K_REG_D2)); break;
		case CPUINFO_STR_REGISTER + M68K_D3:			sprintf(info->s, "D3 :%08X", m68k_get_reg(m68k, M68K_REG_D3)); break;
		case CPUINFO_STR_REGISTER + M68K_D4:			sprintf(info->s, "D4 :%08X", m68k_get_reg(m68k, M68K_REG_D4)); break;
		case CPUINFO_STR_REGISTER + M68K_D5:			sprintf(info->s, "D5 :%08X", m68k_get_reg(m68k, M68K_REG_D5)); break;
		case CPUINFO_STR_REGISTER + M68K_D6:			sprintf(info->s, "D6 :%08X", m68k_get_reg(m68k, M68K_REG_D6)); break;
		case CPUINFO_STR_REGISTER + M68K_D7:			sprintf(info->s, "D7 :%08X", m68k_get_reg(m68k, M68K_REG_D7)); break;
		case CPUINFO_STR_REGISTER + M68K_A0:			sprintf(info->s, "A0 :%08X", m68k_get_reg(m68k, M68K_REG_A0)); break;
		case CPUINFO_STR_REGISTER + M68K_A1:			sprintf(info->s, "A1 :%08X", m68k_get_reg(m68k, M68K_REG_A1)); break;
		case CPUINFO_STR_REGISTER + M68K_A2:			sprintf(info->s, "A2 :%08X", m68k_get_reg(m68k, M68K_REG_A2)); break;
		case CPUINFO_STR_REGISTER + M68K_A3:			sprintf(info->s, "A3 :%08X", m68k_get_reg(m68k, M68K_REG_A3)); break;
		case CPUINFO_STR_REGISTER + M68K_A4:			sprintf(info->s, "A4 :%08X", m68k_get_reg(m68k, M68K_REG_A4)); break;
		case CPUINFO_STR_REGISTER + M68K_A5:			sprintf(info->s, "A5 :%08X", m68k_get_reg(m68k, M68K_REG_A5)); break;
		case CPUINFO_STR_REGISTER + M68K_A6:			sprintf(info->s, "A6 :%08X", m68k_get_reg(m68k, M68K_REG_A6)); break;
		case CPUINFO_STR_REGISTER + M68K_A7:			sprintf(info->s, "A7 :%08X", m68k_get_reg(m68k, M68K_REG_A7)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_ADDR:		sprintf(info->s, "PAR:%08X", m68k_get_reg(m68k, M68K_REG_PREF_ADDR)); break;
		case CPUINFO_STR_REGISTER + M68K_PREF_DATA:		sprintf(info->s, "PDA:%08X", m68k_get_reg(m68k, M68K_REG_PREF_DATA)); break;
		case CPUINFO_STR_REGISTER + M68K_MSP:			sprintf(info->s, "MSP:%08X", m68k_get_reg(m68k, M68K_REG_MSP)); break;
		case CPUINFO_STR_REGISTER + M68K_CACR:			sprintf(info->s, "CCR:%08X", m68k_get_reg(m68k, M68K_REG_CACR)); break;
		case CPUINFO_STR_REGISTER + M68K_CAAR:			sprintf(info->s, "CAR:%08X", m68k_get_reg(m68k, M68K_REG_CAAR)); break;
		case CPUINFO_STR_REGISTER + M68K_SFC:			sprintf(info->s, "SFC:%X",   m68k_get_reg(m68k, M68K_REG_SFC)); break;
		case CPUINFO_STR_REGISTER + M68K_DFC:			sprintf(info->s, "DFC:%X",   m68k_get_reg(m68k, M68K_REG_DFC)); break;
		case CPUINFO_STR_REGISTER + M68K_VBR:			sprintf(info->s, "VBR:%08X", m68k_get_reg(m68k, M68K_REG_VBR)); break;
	}
}

#endif /* HAS_M68040 */

