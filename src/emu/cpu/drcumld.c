/***************************************************************************

    drcumld.c

    Universal machine language disassembler.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "drcumld.h"
#include "drcumlsh.h"
#include "memory.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef char *(*output_function)(char *dest, int size, const drcuml_parameter *param);


typedef struct _drcuml_opdesc drcuml_opdesc;
struct _drcuml_opdesc
{
	UINT16			opcode;
	const char *	opstring;
	output_function	param[5];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static char *output_cond(char *dest, int size, const drcuml_parameter *param);
static char *output_flags(char *dest, int size, const drcuml_parameter *param);
static char *output_flags_cmptest(char *dest, int size, const drcuml_parameter *param);
static char *output_param(char *dest, int size, const drcuml_parameter *param);
static char *output_param_string(char *dest, int size, const drcuml_parameter *param);
static char *output_param_handle(char *dest, int size, const drcuml_parameter *param);
static char *output_param_size(char *dest, int size, const drcuml_parameter *param);
static char *output_param_space(char *dest, int size, const drcuml_parameter *param);
static char *output_param_space_size(char *dest, int size, const drcuml_parameter *param);
static char *output_param_round(char *dest, int size, const drcuml_parameter *param);



/***************************************************************************
    TABLES
***************************************************************************/

/* these define the conditions for the UML */
static const char *condname[] =
{
	"z",	"nz",
	"s",	"ns",
	"c",	"nc",
	"v",	"nv",
	"u",	"nu",
	"a",	"be",
	"g",	"le",
	"l",	"ge"
};

/* the source opcode table */
static const drcuml_opdesc opcode_source_table[] =
{
	/* Compile-time opcodes */
	{ DRCUML_OP_HANDLE,		"handle",	{ output_param_handle } },
	{ DRCUML_OP_HASH,		"hash",		{ output_param, output_param } },
	{ DRCUML_OP_LABEL,		"label",	{ output_param } },
	{ DRCUML_OP_COMMENT,	"comment",	{ output_param_string } },
	{ DRCUML_OP_MAPVAR,		"mapvar",	{ output_param, output_param } },

	/* Control Flow Operations */
	{ DRCUML_OP_DEBUG,		"debug",	{ output_param } },
	{ DRCUML_OP_EXIT,		"exit",		{ output_param, output_cond } },
	{ DRCUML_OP_HASHJMP,	"hashjmp",	{ output_param, output_param, output_param_handle } },
	{ DRCUML_OP_JMP,		"jmp",		{ output_param, output_cond } },
	{ DRCUML_OP_EXH,		"exh",		{ output_param_handle, output_param, output_cond } } ,
	{ DRCUML_OP_CALLH,		"callh",	{ output_param_handle, output_cond } },
	{ DRCUML_OP_RET,		"ret",		{ output_cond } },
	{ DRCUML_OP_CALLC,		"callc",	{ output_param, output_param, output_cond } },
	{ DRCUML_OP_RECOVER,	"recover",	{ output_param, output_param } },

	/* Internal Register Operations */
	{ DRCUML_OP_SETFMOD,	"setfmod",	{ output_param } },
	{ DRCUML_OP_GETFMOD,	"getfmod",	{ output_param } },
	{ DRCUML_OP_GETEXP,		"getexp",	{ output_param } },
	{ DRCUML_OP_GETFLGS,	"getflgs",	{ output_param, output_flags } },
	{ DRCUML_OP_SAVE,		"save",		{ output_param } },
	{ DRCUML_OP_RESTORE,	"restore",	{ output_param } },

	/* Integer Operations */
	{ DRCUML_OP_LOAD,		"!load",	{ output_param, output_param, output_param, output_param_size } },
	{ DRCUML_OP_LOADS,		"!loads",	{ output_param, output_param, output_param, output_param_size } },
	{ DRCUML_OP_STORE,		"!store",	{ output_param, output_param, output_param, output_param_size } },
	{ DRCUML_OP_READ,		"!read",	{ output_param, output_param, output_param_space_size } },
	{ DRCUML_OP_READM,		"!readm",	{ output_param, output_param, output_param, output_param_space_size } },
	{ DRCUML_OP_WRITE,		"!write",	{ output_param, output_param, output_param_space_size } },
	{ DRCUML_OP_WRITEM,		"!writem",	{ output_param, output_param, output_param, output_param_space_size } },
	{ DRCUML_OP_CARRY,		"!carry",	{ output_param, output_param } },
	{ DRCUML_OP_SET,		"!set",		{ output_param, output_cond } },
	{ DRCUML_OP_MOV,		"!mov",		{ output_param, output_param, output_cond } },
	{ DRCUML_OP_SEXT,		"!sext",	{ output_param, output_param, output_param_size } },
	{ DRCUML_OP_ROLAND,		"!roland",	{ output_param, output_param, output_param, output_param } },
	{ DRCUML_OP_ROLINS,		"!rolins",	{ output_param, output_param, output_param, output_param } },
	{ DRCUML_OP_ADD,		"!add",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_ADDC,		"!addc",	{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_SUB,		"!sub",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_SUBB,		"!subb",	{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_CMP,		"!cmp",		{ output_param, output_param, output_flags_cmptest } },
	{ DRCUML_OP_MULU,		"!mulu",	{ output_param, output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_MULS,		"!muls",	{ output_param, output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_DIVU,		"!divu",	{ output_param, output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_DIVS,		"!divs",	{ output_param, output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_AND,		"!and",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_TEST,		"!test",	{ output_param, output_param, output_flags_cmptest } },
	{ DRCUML_OP_OR,			"!or",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_XOR,		"!xor",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_LZCNT,		"!lzcnt",	{ output_param, output_param } },
	{ DRCUML_OP_BSWAP,		"!bswap",	{ output_param, output_param } },
	{ DRCUML_OP_SHL,		"!shl",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_SHR,		"!shr",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_SAR,		"!sar",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_ROL,		"!rol",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_ROLC,		"!rolc",	{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_ROR,		"!ror",		{ output_param, output_param, output_param, output_flags } },
	{ DRCUML_OP_RORC,		"!rorc",	{ output_param, output_param, output_param, output_flags } },

	/* Floating-Point Operations */
	{ DRCUML_OP_FLOAD,		"f!load",	{ output_param, output_param, output_param } },
	{ DRCUML_OP_FSTORE,		"f!store",	{ output_param, output_param, output_param } },
	{ DRCUML_OP_FREAD,		"f!read",	{ output_param, output_param, output_param_space } },
	{ DRCUML_OP_FWRITE,		"f!write",	{ output_param, output_param, output_param_space } },
	{ DRCUML_OP_FMOV,		"f!mov",	{ output_param, output_param, output_cond } },
	{ DRCUML_OP_FTOINT,		"f!toint",	{ output_param, output_param, output_param_size, output_param_round } },
	{ DRCUML_OP_FFRINT,		"f!frint",	{ output_param, output_param, output_param_size } },
	{ DRCUML_OP_FFRFLT,		"f!frflt",	{ output_param, output_param, output_param_size } },
	{ DRCUML_OP_FRNDS,		"f!rnds",	{ output_param, output_param } },
	{ DRCUML_OP_FADD,		"f!add",	{ output_param, output_param, output_param } },
	{ DRCUML_OP_FSUB,		"f!sub",	{ output_param, output_param, output_param } },
	{ DRCUML_OP_FCMP,		"f!cmp",	{ output_param, output_param, output_flags_cmptest } },
	{ DRCUML_OP_FMUL,		"f!mul",	{ output_param, output_param, output_param } },
	{ DRCUML_OP_FDIV,		"f!div",	{ output_param, output_param, output_param } },
	{ DRCUML_OP_FNEG,		"f!neg",	{ output_param, output_param } },
	{ DRCUML_OP_FABS,		"f!abs",	{ output_param, output_param } },
	{ DRCUML_OP_FSQRT,		"f!sqrt",	{ output_param, output_param } },
	{ DRCUML_OP_FRECIP,		"f!recip",	{ output_param, output_param } },
	{ DRCUML_OP_FRSQRT,		"f!rsqrt",	{ output_param, output_param } },
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static const drcuml_opdesc *opcode_table[DRCUML_OP_MAX];



/***************************************************************************
    CORE DISASSEMBLER
***************************************************************************/

/*-------------------------------------------------
    drcuml_disasm - disassemble one instruction
-------------------------------------------------*/

void drcuml_disasm(const drcuml_instruction *inst, char *buffer)
{
	const drcuml_opdesc *opdesc;
	const char *stringptr;
	char *dest = buffer;
	int pnum, iparam = 0;

	/* if this is the first time through, populate the opcode table */
	if (opcode_table[DRCUML_OP_HASH] == NULL)
	{
		int tabledex;

		for (tabledex = 0; tabledex < ARRAY_LENGTH(opcode_source_table); tabledex++)
			opcode_table[opcode_source_table[tabledex].opcode] = &opcode_source_table[tabledex];
	}

	/* find the opcode in the list */
	opdesc = opcode_table[inst->opcode];

	/* NULL means invalid opcode */
	if (opdesc == NULL)
	{
		strcpy(buffer, "???");
		return;
	}

	/* start with the opcode itself */
	stringptr = opdesc->opstring;

	/* insert a prefix for integer operations */
	if (stringptr[0] == '!')
	{
		stringptr++;
		switch (inst->size)
		{
			case 1:	*dest++ = 'b'; break;
			case 2:	*dest++ = 'w'; break;
			case 4:	break;
			case 8:	*dest++ = 'd'; break;
			default: *dest++ = '?'; break;
		}
	}

	/* insert a prefix for floating point operations */
	if (stringptr[0] == 'f' && stringptr[1] == '!')
	{
		*dest++ = *stringptr++;
		stringptr++;
		switch (inst->size)
		{
			case 4:	*dest++ = 's'; break;
			case 8:	*dest++ = 'd'; break;
			default: *dest++ = '?'; break;
		}
	}

	/* copy the rest of the string */
	dest += sprintf(dest, "%s", stringptr);

	/* iterate over parameters */
	for (pnum = 0; pnum < ARRAY_LENGTH(opdesc->param); pnum++)
	{
		output_function func = opdesc->param[pnum];
		drcuml_parameter param;

		/* stop when we hit NULL */
		if (func == NULL)
			break;

		/* if we are outputting a condition but it's "always", skip it */
		if (func == output_cond)
		{
			if (inst->condflags == DRCUML_COND_ALWAYS)
				continue;
			param.value = inst->condflags;
		}

		/* if we are outputting flags but they are "none", skip it */
		else if (func == output_flags)
		{
			if (inst->condflags == FLAGS_NONE)
				continue;
			param.value = inst->condflags;
		}

		/* if we are outputting test/cmp flags but they are "all", skip it */
		else if (func == output_flags_cmptest)
		{
			if (inst->opcode == DRCUML_OP_CMP && inst->condflags == FLAGS_ALLI)
				continue;
			if (inst->opcode == DRCUML_OP_FCMP && inst->condflags == FLAGS_ALLF)
				continue;
			if (inst->opcode == DRCUML_OP_TEST && inst->condflags == (FLAGS_S|FLAGS_Z))
				continue;
			param.value = inst->condflags;
		}

		/* otherwise, we are fetching an actual parameter */
		else
			param = inst->param[iparam++];

		/* if we are less than 8 characters, pad to 8 */
		if (dest - buffer < 8)
			dest += sprintf(dest, "%*s", (int)(8 - (dest - buffer)), "");

		/* otherwise, add a comma */
		else
			dest += sprintf(dest, ",");

		/* then append the parameter */
		dest = (*func)(dest, inst->size, &param);
	}
}



/***************************************************************************
    OUTPUT HELPERS
***************************************************************************/

/*-------------------------------------------------
    output_cond - output condition
-------------------------------------------------*/

static char *output_cond(char *dest, int size, const drcuml_parameter *param)
{
	if (param->value >= DRCUML_COND_Z && param->value < DRCUML_COND_MAX)
		return dest + sprintf(dest, "%s", condname[param->value - DRCUML_COND_Z]);
	else
		return dest + sprintf(dest, "??");
}


/*-------------------------------------------------
    output_flags - output flags
-------------------------------------------------*/

static char *output_flags(char *dest, int size, const drcuml_parameter *param)
{
	if ((param->value & (DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0)
		return dest + sprintf(dest, "noflags");
	if (param->value & DRCUML_FLAG_C)
		*dest++ = 'C';
	if (param->value & DRCUML_FLAG_V)
		*dest++ = 'V';
	if (param->value & DRCUML_FLAG_Z)
		*dest++ = 'Z';
	if (param->value & DRCUML_FLAG_S)
		*dest++ = 'S';
	*dest = 0;
	return dest;
}


/*-------------------------------------------------
    output_flags_cmptest - output flags
-------------------------------------------------*/

static char *output_flags_cmptest(char *dest, int size, const drcuml_parameter *param)
{
	if ((param->value & (DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S)) == (DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S))
		return dest + sprintf(dest, "allflags");
	return output_flags(dest, size, param);
}


/*-------------------------------------------------
    output_param - generic parameter output
-------------------------------------------------*/

static char *output_param(char *dest, int size, const drcuml_parameter *param)
{
	UINT64 value;
	switch (param->type)
	{
		case DRCUML_PTYPE_NONE:
			return dest += sprintf(dest, "<none?>");

		case DRCUML_PTYPE_IMMEDIATE:
			value = param->value;
			if (size == 1) value = (UINT8)value;
			if (size == 2) value = (UINT16)value;
			if (size == 4) value = (UINT32)value;
			if ((UINT32)value == value)
				return dest += sprintf(dest, "$%X", (UINT32)value);
			else
				return dest += sprintf(dest, "$%X%08X", (UINT32)(value >> 32), (UINT32)value);

		case DRCUML_PTYPE_MAPVAR:
			if (param->value >= DRCUML_MAPVAR_M0 && param->value < DRCUML_MAPVAR_END)
				return dest += sprintf(dest, "m%d", (UINT32)(param->value - DRCUML_MAPVAR_M0));
			else
				return dest += sprintf(dest, "m(%X?)", (UINT32)param->value);

		case DRCUML_PTYPE_INT_REGISTER:
			if (param->value >= DRCUML_REG_I0 && param->value < DRCUML_REG_I_END)
				return dest += sprintf(dest, "i%d", (UINT32)(param->value - DRCUML_REG_I0));
			else
				return dest += sprintf(dest, "i(%X?)", (UINT32)param->value);

		case DRCUML_PTYPE_FLOAT_REGISTER:
			if (param->value >= DRCUML_REG_F0 && param->value < DRCUML_REG_F_END)
				return dest += sprintf(dest, "f%d", (UINT32)(param->value - DRCUML_REG_F0));
			else
				return dest += sprintf(dest, "f(%X?)", (UINT32)param->value);

		case DRCUML_PTYPE_MEMORY:
			return dest += sprintf(dest, "[$%p]", (void *)(FPTR)param->value);

		default:
			return dest += sprintf(dest, "???");
	}
	return dest;
}


/*-------------------------------------------------
    output_param_string - output parameter 0,
    assuming it is a string
-------------------------------------------------*/

static char *output_param_string(char *dest, int size, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_MEMORY)
		return dest += sprintf(dest, "%s", (char *)(FPTR)param->value);
	return output_param(dest, size, param);
}


/*-------------------------------------------------
    output_param_handle - output parameter 0,
    assuming it is a handle
-------------------------------------------------*/

static char *output_param_handle(char *dest, int size, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_MEMORY)
		return dest += sprintf(dest, "%s", drcuml_handle_name((drcuml_codehandle *)(FPTR)param->value));
	return output_param(dest, size, param);
}


/*-------------------------------------------------
    output_param_size - output parameter 0,
    assuming it is size
-------------------------------------------------*/

static char *output_param_size(char *dest, int size, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		static const char *strings[] = { "byte", "word", "dword", "qword" };
		return dest + sprintf(dest, "%s", strings[param->value & 3]);
	}
	return output_param(dest, size, param);
}


/*-------------------------------------------------
    output_param_space - output parameter 0,
    assuming it is an address space
-------------------------------------------------*/

static char *output_param_space(char *dest, int size, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE && param->value >= ADDRESS_SPACE_PROGRAM && param->value <= ADDRESS_SPACE_IO)
		return dest + sprintf(dest, "%s", address_space_names[param->value]);
	return output_param(dest, size, param);
}


/*-------------------------------------------------
    output_param_space_size - output parameter 0,
    assuming it is an address space/size combo
-------------------------------------------------*/

static char *output_param_space_size(char *dest, int size, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE && param->value / 16 >= ADDRESS_SPACE_PROGRAM && param->value / 16 <= ADDRESS_SPACE_IO)
	{
		dest += sprintf(dest, "%s_", address_space_names[param->value / 16]);
		return output_param_size(dest, size, param);
	}
		
	return output_param(dest, size, param);
}


/*-------------------------------------------------
    output_param_round - output parameter 0,
    assuming it is a rounding mode
-------------------------------------------------*/

static char *output_param_round(char *dest, int size, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE && param->value >= DRCUML_FMOD_TRUNC && param->value <= DRCUML_FMOD_DEFAULT)
	{
		static const char *strings[] = { "trunc", "round", "ceil", "floor", "default" };
		return dest + sprintf(dest, "%s", strings[param->value]);
	}
	return output_param(dest, size, param);
}
