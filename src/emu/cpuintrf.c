/***************************************************************************

    cpuintrf.c

    Core CPU interface functions and definitions.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"

/* mingw-gcc defines this */
#undef i386


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define TEMP_STRING_POOL_ENTRIES		16
#define MAX_STRING_LENGTH				256



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static char temp_string_pool[TEMP_STRING_POOL_ENTRIES][MAX_STRING_LENGTH];
static int temp_string_pool_index;



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_temp_string_buffer - return a pointer to
    a temporary string buffer
-------------------------------------------------*/

INLINE char *get_temp_string_buffer(void)
{
	char *string = &temp_string_pool[temp_string_pool_index++ % TEMP_STRING_POOL_ENTRIES][0];
	string[0] = 0;
	return string;
}



/***************************************************************************
    GLOBAL MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    cpuintrf_init - initialize global structures
-------------------------------------------------*/

void cpuintrf_init(running_machine *machine)
{
}



/***************************************************************************
    LIVE CPU ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    cpu_get_info_* - return information about a
    live CPU
-------------------------------------------------*/

INT64 cpu_get_info_int(const device_config *device, UINT32 state)
{
	const cpu_config *config = device->inline_config;
	cpuinfo info;

	info.i = 0;
	(*config->type)(device, state, &info);
	return info.i;
}

void *cpu_get_info_ptr(const device_config *device, UINT32 state)
{
	const cpu_config *config = device->inline_config;
	cpuinfo info;

	info.p = NULL;
	(*config->type)(device, state, &info);
	return info.p;
}

genf *cpu_get_info_fct(const device_config *device, UINT32 state)
{
	const cpu_config *config = device->inline_config;
	cpuinfo info;

	info.f = NULL;
	(*config->type)(device, state, &info);
	return info.f;
}

const char *cpu_get_info_string(const device_config *device, UINT32 state)
{
	const cpu_config *config = device->inline_config;
	cpuinfo info;

	info.s = get_temp_string_buffer();
	(*config->type)(device, state, &info);
	return info.s;
}


/*-------------------------------------------------
    cpu_set_info_* - set information about a
    live CPU
-------------------------------------------------*/

void cpu_set_info_int(const device_config *device, UINT32 state, INT64 data)
{
	cpu_class_header *classheader = cpu_get_class_header(device);
	cpuinfo info;

	info.i = data;
	(*classheader->set_info)(device, state, &info);
}

void cpu_set_info_ptr(const device_config *device, UINT32 state, void *data)
{
	cpu_class_header *classheader = cpu_get_class_header(device);
	cpuinfo info;

	info.p = data;
	(*classheader->set_info)(device, state, &info);
}

void cpu_set_info_fct(const device_config *device, UINT32 state, genf *data)
{
	cpu_class_header *classheader = cpu_get_class_header(device);
	cpuinfo info;

	info.f = data;
	(*classheader->set_info)(device, state, &info);
}



/*-------------------------------------------------
    cpu_get_physical_pc_byte - return the PC,
    corrected to a byte offset and translated to
    physical space, on a given CPU
-------------------------------------------------*/

offs_t cpu_get_physical_pc_byte(const device_config *device)
{
	const address_space *space = cpu_get_address_space(device, ADDRESS_SPACE_PROGRAM);
	offs_t pc;

	pc = memory_address_to_byte(space, cpu_get_info_int(device, CPUINFO_INT_PC));
	memory_address_physical(space, TRANSLATE_FETCH, &pc);
	return pc;
}


/*-------------------------------------------------
    cpu_dasm - disassemble a line at a given PC
    on a given CPU
-------------------------------------------------*/

offs_t cpu_dasm(const device_config *device, char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	cpu_class_header *classheader = cpu_get_class_header(device);
	offs_t result = 0;

	/* check for disassembler override */
	if (classheader->dasm_override != NULL)
		result = (*classheader->dasm_override)(device, buffer, pc, oprom, opram);

	/* if we have a disassembler, run it */
	if (result == 0 && classheader->disassemble != NULL)
		result = (*classheader->disassemble)(device, buffer, pc, oprom, opram);

	/* if we still have nothing, output vanilla bytes */
	if (result == 0)
	{
		result = cpu_get_min_opcode_bytes(device);
		switch (result)
		{
			case 1:
			default:
				sprintf(buffer, "$%02X", *(UINT8 *)oprom);
				break;

			case 2:
				sprintf(buffer, "$%04X", *(UINT16 *)oprom);
				break;

			case 4:
				sprintf(buffer, "$%08X", *(UINT32 *)oprom);
				break;

			case 8:
				sprintf(buffer, "$%08X%08X", (UINT32)(*(UINT64 *)oprom >> 32), (UINT32)(*(UINT64 *)oprom >> 0));
				break;
		}
	}

	/* make sure we get good results */
	assert((result & DASMFLAG_LENGTHMASK) != 0);
#ifdef MAME_DEBUG
{
	const address_space *space = classheader->space[ADDRESS_SPACE_PROGRAM];
	int bytes = memory_address_to_byte(space, result & DASMFLAG_LENGTHMASK);
	assert(bytes >= cpu_get_min_opcode_bytes(device));
	assert(bytes <= cpu_get_max_opcode_bytes(device));
	(void) bytes; /* appease compiler */
}
#endif

	return result;
}


/*-------------------------------------------------
    cpu_set_dasm_override - set a dasm override
    handler
-------------------------------------------------*/

void cpu_set_dasm_override(const device_config *device, cpu_disassemble_func dasm_override)
{
	cpu_class_header *classheader = cpu_get_class_header(device);
	classheader->dasm_override = dasm_override;
}



/***************************************************************************
    CPU TYPE ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    cputype_get_info_* - return information about a
    given CPU type
-------------------------------------------------*/

INT64 cputype_get_info_int(cpu_type cputype, UINT32 state)
{
	cpuinfo info;

	assert(cputype != NULL);
	info.i = 0;
	(*cputype)(NULL, state, &info);
	return info.i;
}

void *cputype_get_info_ptr(cpu_type cputype, UINT32 state)
{
	cpuinfo info;

	assert(cputype != NULL);
	info.p = NULL;
	(*cputype)(NULL, state, &info);
	return info.p;
}

genf *cputype_get_info_fct(cpu_type cputype, UINT32 state)
{
	cpuinfo info;

	assert(cputype != NULL);
	info.f = NULL;
	(*cputype)(NULL, state, &info);
	return info.f;
}

const char *cputype_get_info_string(cpu_type cputype, UINT32 state)
{
	cpuinfo info;

	assert(cputype != NULL);
	info.s = get_temp_string_buffer();
	(*cputype)(NULL, state, &info);
	return info.s;
}



/***************************************************************************
    DUMMY CPU DEFINITION
***************************************************************************/

typedef struct _dummy_context dummy_context;
struct _dummy_context
{
	UINT32		icount;
};

static CPU_INIT( dummy ) { }
static CPU_RESET( dummy ) { }
static CPU_EXIT( dummy ) { }
static CPU_EXECUTE( dummy ) { return cycles; }

static CPU_DISASSEMBLE( dummy )
{
	strcpy(buffer, "???");
	return 1;
}

static CPU_SET_INFO( dummy )
{
}

CPU_GET_INFO( dummy )
{
	dummy_context *cpustate = (device != NULL) ? device->token : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(dummy_context); 		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 1;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = 0;							break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = 0;							break;
		case CPUINFO_INT_PC:							info->i = 0;							break;
		case CPUINFO_INT_SP:							info->i = 0;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(dummy); break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(dummy);		break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(dummy);	break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(dummy);		break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(dummy);break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(dummy); break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "");					break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "no CPU");				break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.0");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "The MAME team");		break;

		case CPUINFO_STR_FLAGS:							strcpy(info->s, "--");					break;
	}
}
