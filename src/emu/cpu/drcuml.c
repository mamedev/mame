/***************************************************************************

    drcuml.c

    Universal machine language for dynamic recompiling CPU cores.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

	Future improvements/changes:
	
	* Write a back-end validator:
		- checks all combinations of memory/register/immediate on all params
		- checks behavior of all opcodes
	
	* Extend registers to 16? Depends on if PPC can use them

	* New instructions?
		- EXTRACT dst,src,shift,mask
			dst = (src >> shift) & mask

		- INSERT dst,src,shift,mask
			dst = (dst & ~mask) | ((src << shift) & mask)

		- VALID opcode_desc,handle,param
			checksum/compare code referenced by opcode_desc; if not
			matching, generate exception with handle,param
		
		- RECALL handle
			change code at caller to call handle in the future
			
	* Add interface for getting hints from the backend:
		- number of direct-mapped integer registers
		- number of direct-mapped floating point registers

***************************************************************************/

#include "drcuml.h"
#include "drcumlsh.h"
#include "drcumld.h"
#include "mame.h"
#include "deprecat.h"
#include <stdarg.h>
#include <setjmp.h>


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* opcode validation condition/flag valid bitmasks */
#define OV_CONDFLAG_NONE		0x00
#define OV_CONDFLAG_COND		0x80
#define OV_CONDFLAG_FLAGS		0x1f

/* opcode validation parameter valid bitmasks */
#define OV_PARAM_ALLOWED_NONE	(1 << DRCUML_PTYPE_NONE)
#define OV_PARAM_ALLOWED_IMM	(1 << DRCUML_PTYPE_IMMEDIATE)
#define OV_PARAM_ALLOWED_IREG	(1 << DRCUML_PTYPE_INT_REGISTER)
#define OV_PARAM_ALLOWED_FREG	(1 << DRCUML_PTYPE_FLOAT_REGISTER)
#define OV_PARAM_ALLOWED_MVAR	(1 << DRCUML_PTYPE_MAPVAR)
#define OV_PARAM_ALLOWED_MEM	(1 << DRCUML_PTYPE_MEMORY)
#define OV_PARAM_ALLOWED_NCMEM	(OV_PARAM_ALLOWED_MEM | 0x80)
#define OV_PARAM_ALLOWED_IMV	(OV_PARAM_ALLOWED_IMM | OV_PARAM_ALLOWED_MVAR)
#define OV_PARAM_ALLOWED_IRM	(OV_PARAM_ALLOWED_IREG | OV_PARAM_ALLOWED_MEM)
#define OV_PARAM_ALLOWED_FRM	(OV_PARAM_ALLOWED_FREG | OV_PARAM_ALLOWED_MEM)
#define OV_PARAM_ALLOWED_IANY	(OV_PARAM_ALLOWED_IRM | OV_PARAM_ALLOWED_IMV)
#define OV_PARAM_ALLOWED_FANY	(OV_PARAM_ALLOWED_FRM)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* structure describing UML generation state */
struct _drcuml_state
{
	drccache *				cache;				/* pointer to the codegen cache */
	drcuml_block *			blocklist;			/* list of active blocks */

	const drcbe_interface *	beintf;				/* backend interface pointer */
	drcbe_state *			bestate;			/* pointer to the back-end state */

	drcuml_codehandle *		handlelist;			/* head of linked list of handles */

	FILE *					umllog;				/* handle to the UML logfile */
};


/* structure describing UML codegen block */
struct _drcuml_block
{
	drcuml_state *			drcuml;				/* pointer back to the owning UML */
	drcuml_block *			next;				/* pointer to next block */
	drcuml_instruction *	inst;				/* pointer to the instruction list */
	UINT8					inuse;				/* this block is in use */
	UINT32					maxinst;			/* maximum number of instructions */
	UINT32					nextinst;			/* next instruction to fill in the cache */
	jmp_buf	*				errorbuf;			/* setjmp buffer for deep error handling */
};


/* structure describing a global handle */
struct _drcuml_codehandle
{
	drccodeptr				code;				/* pointer to the associated code */
	char *					string;				/* pointer to string attached to handle */
	drcuml_codehandle *		next;				/* link to next handle in the list */
	drcuml_state *			drcuml;				/* pointer to owning object */
};


/* structure describing an opcode for validation */
typedef struct _drcuml_opcode_valid drcuml_opcode_valid;
struct _drcuml_opcode_valid
{
	drcuml_opcode			opcode;				/* the opcode itself */
	UINT8					sizes;				/* allowed sizes */
	UINT8					condflags;			/* allowed conditions/flags */
	UINT8					p0types;			/* allowed types for parameter 0 */
	UINT8					p1types;			/* allowed types for parameter 1 */
	UINT8					p2types;			/* allowed types for parameter 2 */
	UINT8					p3types;			/* allowed types for parameter 3 */
};



/***************************************************************************
    TABLES
***************************************************************************/

/* macro to simplify the table */
#define OPVALID_ENTRY_0(op,sizes,condflag)				{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE },
#define OPVALID_ENTRY_1(op,sizes,condflag,p0)			{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, OV_PARAM_ALLOWED_##p0, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE },
#define OPVALID_ENTRY_2(op,sizes,condflag,p0,p1)		{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, OV_PARAM_ALLOWED_##p0, OV_PARAM_ALLOWED_##p1, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE },
#define OPVALID_ENTRY_3(op,sizes,condflag,p0,p1,p2)		{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, OV_PARAM_ALLOWED_##p0, OV_PARAM_ALLOWED_##p1, OV_PARAM_ALLOWED_##p2, OV_PARAM_ALLOWED_NONE },
#define OPVALID_ENTRY_4(op,sizes,condflag,p0,p1,p2,p3)	{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, OV_PARAM_ALLOWED_##p0, OV_PARAM_ALLOWED_##p1, OV_PARAM_ALLOWED_##p2, OV_PARAM_ALLOWED_##p3 },

/* opcode validation table */
static const drcuml_opcode_valid opcode_valid_list[] =
{
	/* Compile-time opcodes */
	OPVALID_ENTRY_1(HANDLE,  4,   NONE,  MEM)
	OPVALID_ENTRY_2(HASH,    4,   NONE,  IMV,  IMV)
	OPVALID_ENTRY_1(LABEL,   4,   NONE,  IMV)
	OPVALID_ENTRY_1(COMMENT, 4,   NONE,  NCMEM)
	OPVALID_ENTRY_2(MAPVAR,  4,   NONE,  MVAR, IMV)

	/* Control Flow Operations */
	OPVALID_ENTRY_1(DEBUG,   4,   NONE,  IANY)
	OPVALID_ENTRY_1(EXIT,    4,   COND,  IANY)
	OPVALID_ENTRY_3(HASHJMP, 4,   NONE,  IANY, IANY, MEM)
	OPVALID_ENTRY_1(LABEL,   4,   NONE,  IMV)
	OPVALID_ENTRY_1(JMP,     4,   COND,  IMV)
	OPVALID_ENTRY_2(EXH,     4,   COND,  MEM,  IANY)
	OPVALID_ENTRY_1(CALLH,   4,   COND,  MEM)
	OPVALID_ENTRY_0(RET,     4,   COND)
	OPVALID_ENTRY_2(CALLC,   4,   COND,  NCMEM,NCMEM)
	OPVALID_ENTRY_2(RECOVER, 4,   NONE,  IRM,  MVAR)

	/* Internal Register Operations */
	OPVALID_ENTRY_1(SETFMOD, 4,   NONE,  IANY)
	OPVALID_ENTRY_1(GETFMOD, 4,   NONE,  IRM)
	OPVALID_ENTRY_1(GETEXP,  4,   NONE,  IRM)

	/* Integer Operations */
	OPVALID_ENTRY_3(LOAD1U,  4|8, NONE,  IRM,  NCMEM,IANY)
	OPVALID_ENTRY_3(LOAD1S,  4|8, NONE,  IRM,  NCMEM,IANY)
	OPVALID_ENTRY_3(LOAD2U,  4|8, NONE,  IRM,  NCMEM,IANY)
	OPVALID_ENTRY_3(LOAD2S,  4|8, NONE,  IRM,  NCMEM,IANY)
	OPVALID_ENTRY_3(LOAD4U,  4|8, NONE,  IRM,  NCMEM,IANY)
	OPVALID_ENTRY_3(LOAD4S,    8, NONE,  IRM,  NCMEM,IANY)
	OPVALID_ENTRY_3(LOAD8U,    8, NONE,  IRM,  NCMEM,IANY)
	OPVALID_ENTRY_3(STORE1,  4|8, NONE,  NCMEM,IANY, IANY)
	OPVALID_ENTRY_3(STORE2,  4|8, NONE,  NCMEM,IANY, IANY)
	OPVALID_ENTRY_3(STORE4,  4|8, NONE,  NCMEM,IANY, IANY)
	OPVALID_ENTRY_3(STORE8,    8, NONE,  NCMEM,IANY, IANY)
	OPVALID_ENTRY_3(READ1U,  4|8, NONE,  IRM,  IMV,  IANY)
	OPVALID_ENTRY_3(READ1S,  4|8, NONE,  IRM,  IMV,  IANY)
	OPVALID_ENTRY_3(READ2U,  4|8, NONE,  IRM,  IMV,  IANY)
	OPVALID_ENTRY_3(READ2S,  4|8, NONE,  IRM,  IMV,  IANY)
	OPVALID_ENTRY_4(READ2M,  4|8, NONE,  IRM,  IMV,  IANY, IANY)
	OPVALID_ENTRY_3(READ4U,  4|8, NONE,  IRM,  IMV,  IANY)
	OPVALID_ENTRY_3(READ4S,    8, NONE,  IRM,  IMV,  IANY)
	OPVALID_ENTRY_4(READ4M,  4|8, NONE,  IRM,  IMV,  IANY, IANY)
	OPVALID_ENTRY_3(READ8U,    8, NONE,  IRM,  IMV,  IANY)
	OPVALID_ENTRY_4(READ8M,    8, NONE,  IRM,  IMV,  IANY, IANY)
	OPVALID_ENTRY_3(WRITE1,  4|8, NONE,  IMV,  IANY, IANY)
	OPVALID_ENTRY_3(WRITE2,  4|8, NONE,  IMV,  IANY, IANY)
	OPVALID_ENTRY_4(WRIT2M,  4|8, NONE,  IMV,  IANY, IANY, IANY)
	OPVALID_ENTRY_3(WRITE4,  4|8, NONE,  IMV,  IANY, IANY)
	OPVALID_ENTRY_4(WRIT4M,  4|8, NONE,  IMV,  IANY, IANY, IANY)
	OPVALID_ENTRY_3(WRITE8,    8, NONE,  IMV,  IANY, IANY)
	OPVALID_ENTRY_4(WRIT8M,    8, NONE,  IMV,  IANY, IANY, IANY)
	OPVALID_ENTRY_3(FLAGS,   4|8, NONE,  IRM,  IMV,  MEM)
	OPVALID_ENTRY_2(MOV,     4|8, COND,  IRM,  IANY)
	OPVALID_ENTRY_2(ZEXT1,   4|8, NONE,  IRM,  IANY)
	OPVALID_ENTRY_2(ZEXT2,   4|8, NONE,  IRM,  IANY)
	OPVALID_ENTRY_2(ZEXT4,     8, NONE,  IRM,  IANY)
	OPVALID_ENTRY_2(SEXT1,   4|8, NONE,  IRM,  IANY)
	OPVALID_ENTRY_2(SEXT2,   4|8, NONE,  IRM,  IANY)
	OPVALID_ENTRY_2(SEXT4,     8, NONE,  IRM,  IANY)
	OPVALID_ENTRY_3(ADD,     4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(ADDC,    4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(SUB,     4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(SUBB,    4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_2(CMP,     4|8, FLAGS, IANY, IANY)
	OPVALID_ENTRY_4(MULU,    4|8, FLAGS, IRM,  IRM,  IANY, IANY)
	OPVALID_ENTRY_4(MULS,    4|8, FLAGS, IRM,  IRM,  IANY, IANY)
	OPVALID_ENTRY_4(DIVU,    4|8, FLAGS, IRM,  IRM,  IANY, IANY)
	OPVALID_ENTRY_4(DIVS,    4|8, FLAGS, IRM,  IRM,  IANY, IANY)
	OPVALID_ENTRY_3(AND,     4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_2(TEST,    4|8, FLAGS, IANY, IANY)
	OPVALID_ENTRY_3(OR,      4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(XOR,     4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(SHL,     4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(SHR,     4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(SAR,     4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(ROL,     4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(ROLC,    4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(ROR,     4|8, FLAGS, IRM,  IANY, IANY)
	OPVALID_ENTRY_3(RORC,    4|8, FLAGS, IRM,  IANY, IANY)

	/* Floating Point Operations */
	OPVALID_ENTRY_3(FLOAD,   4|8, NONE,  FRM,  NCMEM,IANY)
	OPVALID_ENTRY_3(FSTORE,  4|8, NONE,  NCMEM,IANY, FANY)
	OPVALID_ENTRY_3(FREAD,   4|8, NONE,  FRM,  IMV,  IANY)
	OPVALID_ENTRY_3(FWRITE,  4|8, NONE,  IMV,  IANY, FANY)
	OPVALID_ENTRY_2(FMOV,    4|8, COND,  FRM,  FANY)
	OPVALID_ENTRY_2(FTOI4,   4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FTOI4T,  4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FTOI4R,  4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FTOI4F,  4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FTOI4C,  4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FTOI8,   4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FTOI8T,  4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FTOI8R,  4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FTOI8F,  4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FTOI8C,  4|8, NONE,  IRM,  FANY)
	OPVALID_ENTRY_2(FFRFS,     8, NONE,  FRM,  FANY)
	OPVALID_ENTRY_2(FFRFD,   4  , NONE,  FRM,  FANY)
	OPVALID_ENTRY_2(FFRI4,   4|8, NONE,  FRM,  IANY)
	OPVALID_ENTRY_2(FFRI8,   4|8, NONE,  FRM,  IANY)
	OPVALID_ENTRY_3(FADD,    4|8, FLAGS, FRM,  FANY, FANY)
	OPVALID_ENTRY_3(FSUB,    4|8, FLAGS, FRM,  FANY, FANY)
	OPVALID_ENTRY_2(FCMP,    4|8, FLAGS, FANY, FANY)
	OPVALID_ENTRY_3(FMUL,    4|8, FLAGS, FRM,  FANY, FANY)
	OPVALID_ENTRY_3(FDIV,    4|8, FLAGS, FRM,  FANY, FANY)
	OPVALID_ENTRY_2(FNEG,    4|8, FLAGS, FRM,  FANY)
	OPVALID_ENTRY_2(FABS,    4|8, FLAGS, FRM,  FANY)
	OPVALID_ENTRY_2(FSQRT,   4|8, FLAGS, FRM,  FANY)
	OPVALID_ENTRY_2(FRECIP,  4|8, NONE,  FRM,  FANY)
	OPVALID_ENTRY_2(FRSQRT,  4|8, NONE,  FRM,  FANY)
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* we presume the existence of a C back-end interface */
extern const drcbe_interface drcbe_c_be_interface;

/* extern a reference to the native DRC interface if it exists */
#ifndef NATIVE_DRC
#define NATIVE_DRC drcbe_c_be_interface
#else
extern const drcbe_interface NATIVE_DRC;
#endif

/* table that is built up on first alloc which looks up via opcode */
static drcuml_opcode_valid opcode_valid_table[DRCUML_OP_MAX];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

#ifdef MAME_DEBUG
static void validate_instruction(drcuml_block *block, const drcuml_instruction *inst);
#else
#define validate_instruction(block, inst) do { } while (0)
#endif



/***************************************************************************
    INITIALIZATION/TEARDOWN
***************************************************************************/

/*-------------------------------------------------
    drcuml_alloc - allocate state for the code
    generator and initialize the back-end
-------------------------------------------------*/

drcuml_state *drcuml_alloc(drccache *cache, UINT32 flags, int modes, int addrbits, int ignorebits)
{
	drcuml_state *drcuml;
	int opnum;

	/* allocate state */
	drcuml = drccache_memory_alloc(cache, sizeof(*drcuml));
	if (drcuml == NULL)
		return NULL;
	memset(drcuml, 0, sizeof(*drcuml));

	/* initialize the state */
	drcuml->cache = cache;
	drcuml->beintf = (flags & DRCUML_OPTION_USE_C) ? &drcbe_c_be_interface : &NATIVE_DRC;

	/* if we're to log, create the logfile */
	if (flags & DRCUML_OPTION_LOG_UML)
		drcuml->umllog = fopen("drcuml.asm", "w");

	/* allocate the back-end */
	drcuml->bestate = (*drcuml->beintf->be_alloc)(drcuml, cache, flags, modes, addrbits, ignorebits);
	if (drcuml->bestate == NULL)
	{
		drcuml_free(drcuml);
		return NULL;
	}

	/* update the valid opcode table */
	for (opnum = 0; opnum < ARRAY_LENGTH(opcode_valid_list); opnum++)
		opcode_valid_table[opcode_valid_list[opnum].opcode] = opcode_valid_list[opnum];

	return drcuml;
}


/*-------------------------------------------------
    drcuml_reset - reset the state completely,
    flushing the cache and all information
-------------------------------------------------*/

void drcuml_reset(drcuml_state *drcuml)
{
	drcuml_codehandle *handle;
	jmp_buf errorbuf;

	/* flush the cache */
	drccache_flush(drcuml->cache);

	/* if we error here, we are screwed */
	if (setjmp(errorbuf) != 0)
		fatalerror("Out of cache space in drcuml_reset");
	
	/* reset all handle code pointers */
	for (handle = drcuml->handlelist; handle != NULL; handle = handle->next)
		handle->code = NULL;

	/* call the backend to reset */
	(*drcuml->beintf->be_reset)(drcuml->bestate);
}


/*-------------------------------------------------
    drcuml_free - free state for the code generator
    and the back-end
-------------------------------------------------*/

void drcuml_free(drcuml_state *drcuml)
{
	/* free the back-end */
	if (drcuml->bestate != NULL)
		(*drcuml->beintf->be_free)(drcuml->bestate);

	/* free all the blocks */
	while (drcuml->blocklist != NULL)
	{
		drcuml_block *block = drcuml->blocklist;

		/* remove the item from the list */
		drcuml->blocklist = block->next;

		/* free memory */
		if (block->inst != NULL)
			free(block->inst);
		free(block);
	}

	/* close any files */
	if (drcuml->umllog != NULL)
		fclose(drcuml->umllog);
}



/***************************************************************************
    CODE BLOCK GENERATION
***************************************************************************/

/*-------------------------------------------------
    drcuml_block_begin - begin a new code block
-------------------------------------------------*/

drcuml_block *drcuml_block_begin(drcuml_state *drcuml, UINT32 maxinst, jmp_buf *errorbuf)
{
	drcuml_block *bestblock = NULL;
	drcuml_block *block;

	/* find an inactive block that matches our qualifications */
	for (block = drcuml->blocklist; block != NULL; block = block->next)
		if (!block->inuse && block->maxinst >= maxinst && (bestblock == NULL || block->maxinst < bestblock->maxinst))
			bestblock = block;

	/* if we failed to find one, allocate a new one */
	if (bestblock == NULL)
	{
		/* allocate the block structure itself */
		bestblock = malloc(sizeof(*bestblock));
		if (bestblock == NULL)
			fatalerror("Out of memory allocating block in drcuml_block_begin");
		memset(bestblock, 0, sizeof(*bestblock));

		/* fill in the structure */
		bestblock->drcuml = drcuml;
		bestblock->next = drcuml->blocklist;
		bestblock->maxinst = maxinst * 3 / 2;
		bestblock->inst = malloc(sizeof(drcuml_instruction) * bestblock->maxinst);
		if (bestblock->inst == NULL)
			fatalerror("Out of memory allocating instruction array in drcuml_block_begin");

		/* hook us into the list */
		drcuml->blocklist = bestblock;
	}

	/* set up the block information and return it */
	bestblock->inuse = TRUE;
	bestblock->nextinst = 0;
	bestblock->errorbuf = errorbuf;

	return bestblock;
}


/*-------------------------------------------------
    drcuml_block_append_0 - append an opcode with
    0 parameters to the block
-------------------------------------------------*/

void drcuml_block_append_0(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condflags)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condflags = condflags;
	inst->numparams = 0;

	/* validation */
	validate_instruction(block, inst);
}


/*-------------------------------------------------
    drcuml_block_append_1 - append an opcode with
    1 parameter to the block
-------------------------------------------------*/

void drcuml_block_append_1(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condflags, drcuml_ptype p0type, drcuml_pvalue p0value)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condflags = condflags;
	inst->numparams = 1;
	inst->param[0].type = p0type;
	inst->param[0].value = p0value;

	/* validation */
	validate_instruction(block, inst);
}


/*-------------------------------------------------
    drcuml_block_append_2 - append an opcode with
    2 parameters to the block
-------------------------------------------------*/

void drcuml_block_append_2(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condflags, drcuml_ptype p0type, drcuml_pvalue p0value, drcuml_ptype p1type, drcuml_pvalue p1value)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condflags = condflags;
	inst->numparams = 2;
	inst->param[0].type = p0type;
	inst->param[0].value = p0value;
	inst->param[1].type = p1type;
	inst->param[1].value = p1value;

	/* validation */
	validate_instruction(block, inst);
}


/*-------------------------------------------------
    drcuml_block_append_3 - append an opcode with
    3 parameters to the block
-------------------------------------------------*/

void drcuml_block_append_3(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condflags, drcuml_ptype p0type, drcuml_pvalue p0value, drcuml_ptype p1type, drcuml_pvalue p1value, drcuml_ptype p2type, drcuml_pvalue p2value)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condflags = condflags;
	inst->numparams = 3;
	inst->param[0].type = p0type;
	inst->param[0].value = p0value;
	inst->param[1].type = p1type;
	inst->param[1].value = p1value;
	inst->param[2].type = p2type;
	inst->param[2].value = p2value;

	/* validation */
	validate_instruction(block, inst);
}


/*-------------------------------------------------
    drcuml_block_append_4 - append an opcode with
    4 parameters to the block
-------------------------------------------------*/

void drcuml_block_append_4(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condflags, drcuml_ptype p0type, drcuml_pvalue p0value, drcuml_ptype p1type, drcuml_pvalue p1value, drcuml_ptype p2type, drcuml_pvalue p2value, drcuml_ptype p3type, drcuml_pvalue p3value)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condflags = condflags;
	inst->numparams = 4;
	inst->param[0].type = p0type;
	inst->param[0].value = p0value;
	inst->param[1].type = p1type;
	inst->param[1].value = p1value;
	inst->param[2].type = p2type;
	inst->param[2].value = p2value;
	inst->param[3].type = p3type;
	inst->param[3].value = p3value;

	/* validation */
	validate_instruction(block, inst);
}


/*-------------------------------------------------
    drcuml_block_end - complete a code block and
    commit it to the cache via the back-end
-------------------------------------------------*/

void drcuml_block_end(drcuml_block *block)
{
	drcuml_state *drcuml = block->drcuml;

	assert(block->inuse);

	/* if we have a logfile, generate a disassembly of the block */
	if (drcuml->umllog != NULL)
	{
		int instnum;

		/* iterate over instructions and output */
		for (instnum = 0; instnum < block->nextinst; instnum++)
		{
			char dasm[512];
			drcuml_disasm(&block->inst[instnum], dasm);
			drcuml_log_printf(drcuml, "%4d: %s\n", instnum, dasm);
		}
		drcuml_log_printf(drcuml, "\n\n");
		fflush(drcuml->umllog);
	}

	/* generate the code via the back-end */
	(*drcuml->beintf->be_generate)(drcuml->bestate, block, block->inst, block->nextinst);

	/* block is no longer in use */
	block->inuse = FALSE;
}


/*-------------------------------------------------
    drcuml_block_abort - abort a code block in
    progress
-------------------------------------------------*/

void drcuml_block_abort(drcuml_block *block)
{
	assert(block->inuse);

	/* block is no longer in use */
	block->inuse = FALSE;

	/* unwind */
	longjmp(*block->errorbuf, 1);
}


/*-------------------------------------------------
    drcuml_hash_exists - return true if a hash
    entry exists for the given mode/pc
-------------------------------------------------*/

int drcuml_hash_exists(drcuml_state *drcuml, UINT32 mode, UINT32 pc)
{
	return (*drcuml->beintf->be_hash_exists)(drcuml->bestate, mode, pc);
}



/***************************************************************************
    CODE EXECUTION
***************************************************************************/

/*-------------------------------------------------
    drcuml_execute - execute at the given PC/mode
    for the specified cycles
-------------------------------------------------*/

int drcuml_execute(drcuml_state *drcuml, drcuml_codehandle *entry)
{
	return (*drcuml->beintf->be_execute)(drcuml->bestate, entry);
}



/***************************************************************************
    CODE HANDLES
***************************************************************************/

/*-------------------------------------------------
    drcuml_handle_alloc - allocate a new handle
-------------------------------------------------*/

drcuml_codehandle *drcuml_handle_alloc(drcuml_state *drcuml, const char *name)
{
	drcuml_codehandle *handle;
	char *string;

	/* allocate space for a copy of the string */
	string = drccache_memory_alloc(drcuml->cache, strlen(name) + 1);
	if (string == NULL)
		return NULL;
	strcpy(string, name);
	
	/* allocate a new handle info */
	handle = drccache_memory_alloc_near(drcuml->cache, sizeof(*handle));
	if (handle == NULL)
	{
		drccache_memory_free(drcuml->cache, string, strlen(name) + 1);
		return NULL;
	}
	memset(handle, 0, sizeof(*handle));
		
	/* fill in the rest of the info and add to the list of handles */
	handle->drcuml = drcuml;
	handle->string = string;
	handle->next = drcuml->handlelist;
	drcuml->handlelist = handle;
	
	return handle;
}


/*-------------------------------------------------
    drcuml_handle_set_codeptr - set a codeptr
    associated with a handle
-------------------------------------------------*/

void drcuml_handle_set_codeptr(drcuml_codehandle *handle, drccodeptr code)
{
	assert(handle->code == NULL);
	assert_in_cache(handle->drcuml->cache, code);
	handle->code = code;
}


/*-------------------------------------------------
    drcuml_handle_codeptr - get the pointer from 
    a handle
-------------------------------------------------*/

drccodeptr drcuml_handle_codeptr(const drcuml_codehandle *handle)
{
	return handle->code;
}


/*-------------------------------------------------
    drcuml_handle_codeptr_addr - get the address
    of the pointer from a handle
-------------------------------------------------*/

drccodeptr *drcuml_handle_codeptr_addr(drcuml_codehandle *handle)
{
	return &handle->code;
}


/*-------------------------------------------------
    drcuml_handle_name - get the name of a handle
-------------------------------------------------*/

const char *drcuml_handle_name(const drcuml_codehandle *handle)
{
	return handle->string;
}



/***************************************************************************
    CODE LOGGING
***************************************************************************/

/*-------------------------------------------------
    drcuml_log_printf - directly printf to the UML
    log if generated
-------------------------------------------------*/

void drcuml_log_printf(drcuml_state *drcuml, const char *format, ...)
{
	/* if we have a file, print to it */
	if (drcuml->umllog != NULL)
	{
		va_list va;

		/* do the printf */
		va_start(va, format);
		vfprintf(drcuml->umllog, format, va);
		va_end(va);
	}
}


/*-------------------------------------------------
    drcuml_add_comment - attach a comment to the
    current output location in the specified block
-------------------------------------------------*/

void drcuml_add_comment(drcuml_block *block, const char *format, ...)
{
	char buffer[512];
	char *comment;
	va_list va;

	assert(block->inuse);

	/* do the printf */
	va_start(va, format);
	vsprintf(buffer, format, va);
	va_end(va);

	/* allocate space in the cache to hold the comment */
	comment = drccache_memory_alloc_temporary(block->drcuml->cache, strlen(buffer) + 1);
	if (comment == NULL)
		return;
	strcpy(comment, buffer);

	/* add an instruction with a pointer */
	drcuml_block_append_1(block, DRCUML_OP_COMMENT, 4, DRCUML_COND_ALWAYS, MEM(comment));
}



/***************************************************************************
    OPCODE VALIDATION
***************************************************************************/

/*-------------------------------------------------
    validate_instruction - verify that the
    instruction created meets all requirements
-------------------------------------------------*/

#ifdef MAME_DEBUG
static void validate_instruction(drcuml_block *block, const drcuml_instruction *inst)
{
	const drcuml_opcode_valid *opvalid = &opcode_valid_table[inst->opcode];

	/* validate information */
	assert(inst->opcode != DRCUML_OP_INVALID && inst->opcode < DRCUML_OP_MAX);
	assert(inst->size == 1 || inst->size == 2 || inst->size == 4 || inst->size == 8);
	assert((opvalid->sizes & inst->size) != 0);
	if (inst->numparams > 0)
	{
		assert(inst->param[0].type > DRCUML_PTYPE_NONE && inst->param[0].type < DRCUML_PTYPE_MAX);
		assert(((opvalid->p0types >> inst->param[0].type) & 1) != 0);
		if (inst->param[0].type == DRCUML_PTYPE_MEMORY && !(opvalid->p0types & 0x80))
			assert_in_near_cache(block->drcuml->cache, (void *)(FPTR)inst->param[0].value);
	}
	if (inst->numparams > 1)
	{
		assert(inst->param[1].type > DRCUML_PTYPE_NONE && inst->param[1].type < DRCUML_PTYPE_MAX);
		assert(((opvalid->p1types >> inst->param[1].type) & 1) != 0);
		if (inst->param[1].type == DRCUML_PTYPE_MEMORY && !(opvalid->p1types & 0x80))
			assert_in_near_cache(block->drcuml->cache, (void *)(FPTR)inst->param[1].value);
	}
	if (inst->numparams > 2)
	{
		assert(inst->param[2].type > DRCUML_PTYPE_NONE && inst->param[2].type < DRCUML_PTYPE_MAX);
		assert(((opvalid->p2types >> inst->param[2].type) & 1) != 0);
		if (inst->param[2].type == DRCUML_PTYPE_MEMORY && !(opvalid->p2types & 0x80))
			assert_in_near_cache(block->drcuml->cache, (void *)(FPTR)inst->param[2].value);
	}
	if (inst->numparams > 3)
	{
		assert(inst->param[3].type > DRCUML_PTYPE_NONE && inst->param[3].type < DRCUML_PTYPE_MAX);
		assert(((opvalid->p3types >> inst->param[3].type) & 1) != 0);
		if (inst->param[3].type == DRCUML_PTYPE_MEMORY && !(opvalid->p3types & 0x80))
			assert_in_near_cache(block->drcuml->cache, (void *)(FPTR)inst->param[3].value);
	}
}
#endif
