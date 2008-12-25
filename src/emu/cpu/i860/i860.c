#include "debugger.h"
#include "i860.h"

/**************************************************************************
 * The core struct
 **************************************************************************/
typedef struct _i860_state_t i860_state_t;
struct _i860_state_t
{
	const device_config *device;
	const address_space *program;

	UINT32 pc;
	UINT32 ppc;

	int icount;
};


/**************************************************************************
 * Functions specified by GET_INFO
 **************************************************************************/

static CPU_INIT( i860 )
{
	i860_state_t *i860 = device->token;
	i860->device = device;
	i860->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
}

static CPU_RESET( i860 )
{
	logerror("i860 reset\n");
}

static CPU_EXECUTE( i860 )
{
	/* Just a disassembler ATM */
	return cycles;
}

extern CPU_DISASSEMBLE( i860 );


/**************************************************************************
 * Generic set_info/get_info
 **************************************************************************/
static CPU_SET_INFO( i860 )
{
	i860_state_t *i860 = device->token;

	switch(state)
	{
		/* Interfacing */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I860_PC:	i860->pc = info->i & 0xffffffff;	break;
	}
}

CPU_GET_INFO( i860 )
{
	i860_state_t *i860 = (device != NULL) ? device->token : NULL;

	switch (state)
	{
		// --- the following bits of info are returned as 64-bit signed integers ---
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i860_state_t);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0x00000000;					break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 8;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		// --- the following bits of info are returned as pointers to data or functions ---
		case CPUINFO_FCT_SET_INFO:						info->setinfo 	  = CPU_SET_INFO_NAME(i860);	break;
		case CPUINFO_FCT_INIT:							info->init 		  = CPU_INIT_NAME(i860);		break;
		case CPUINFO_FCT_RESET:							info->reset 	  = CPU_RESET_NAME(i860);		break;
		case CPUINFO_FCT_EXIT:							info->exit 		  = NULL;						break;
		case CPUINFO_FCT_EXECUTE:						info->execute 	  = CPU_EXECUTE_NAME(i860);		break;
		case CPUINFO_FCT_BURN:							info->burn 		  = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(i860);	break;
		case CPUINFO_FCT_DEBUG_INIT:					info->debug_init  = NULL;						break;
		case CPUINFO_FCT_TRANSLATE:						info->translate	  = NULL;						break;
		case CPUINFO_FCT_READ:							info->read 		  = NULL;						break;
		case CPUINFO_FCT_WRITE:							info->write 	  = NULL;						break;
		case CPUINFO_FCT_READOP:						info->readop 	  = NULL;						break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount 	  = &i860->icount;				break;

		// --- the following bits of info are returned as NULL-terminated strings ---
		case CPUINFO_STR_NAME:							strcpy(info->s, "i860");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Intel");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.1");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);			break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Jason Eckhardt and Andrew Gardner");	break;
	
		case CPUINFO_INT_PC:							info->i = i860->pc;					break;
		case CPUINFO_INT_PREVIOUSPC:					info->i = i860->ppc;				break;
	}
}
