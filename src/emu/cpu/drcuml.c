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
    
    * Support for FPU exceptions

    * New instructions?
		- FDRNDS dst,src
			round to single-precision

		- FCOPYI, ICOPYF
			copy raw between float and integer registers
			
        - VALID opcode_desc,handle,param
            checksum/compare code referenced by opcode_desc; if not
            matching, generate exception with handle,param

        - RECALL handle
            change code at caller to call handle in the future

***************************************************************************/

#include "drcuml.h"
#include "drcumlsh.h"
#include "drcumld.h"
#include "deprecat.h"
#include "mame.h"
#include <stdarg.h>
#include <setjmp.h>


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VALIDATE_BACKEND		(0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* opcode validation condition/flag valid bitmasks */
#define OV_CONDFLAG_NONE		0x00
#define OV_CONDFLAG_COND		0x80
#define OV_CONDFLAG_SZ			(DRCUML_FLAG_S | DRCUML_FLAG_Z)
#define OV_CONDFLAG_SZC			(DRCUML_FLAG_S | DRCUML_FLAG_Z | DRCUML_FLAG_C)
#define OV_CONDFLAG_SZVC		(DRCUML_FLAG_S | DRCUML_FLAG_Z | DRCUML_FLAG_V | DRCUML_FLAG_C)
#define OV_CONDFLAG_UZC			(DRCUML_FLAG_U | DRCUML_FLAG_Z | DRCUML_FLAG_C)
#define OV_CONDFLAG_FLAGS		0x1f

/* bitmasks of valid parameter types and flags */
#define OV_PARAM_ALLOWED_NONE	(1 << DRCUML_PTYPE_NONE)
#define OV_PARAM_ALLOWED_IMM	(1 << DRCUML_PTYPE_IMMEDIATE)
#define OV_PARAM_ALLOWED_IREG	(1 << DRCUML_PTYPE_INT_REGISTER)
#define OV_PARAM_ALLOWED_FREG	(1 << DRCUML_PTYPE_FLOAT_REGISTER)
#define OV_PARAM_ALLOWED_MVAR	(1 << DRCUML_PTYPE_MAPVAR)
#define OV_PARAM_ALLOWED_MEM	(1 << DRCUML_PTYPE_MEMORY)
#define OV_PARAM_FLAG_ANYMEM	0x100
#define OV_PARAM_FLAG_FIXED4	0x200
#define OV_PARAM_FLAG_FIXED8	0x400

/* opcode validation parameter valid bitmasks */
#define OV_PARAM_ALLOWED_ANYMEM	(OV_PARAM_ALLOWED_MEM | OV_PARAM_FLAG_ANYMEM)
#define OV_PARAM_ALLOWED_IMV	(OV_PARAM_ALLOWED_IMM | OV_PARAM_ALLOWED_MVAR)
#define OV_PARAM_ALLOWED_IMV4	(OV_PARAM_ALLOWED_IMM | OV_PARAM_ALLOWED_MVAR | OV_PARAM_FLAG_FIXED4)
#define OV_PARAM_ALLOWED_IMV8	(OV_PARAM_ALLOWED_IMM | OV_PARAM_ALLOWED_MVAR | OV_PARAM_FLAG_FIXED8)
#define OV_PARAM_ALLOWED_IRM	(OV_PARAM_ALLOWED_IREG | OV_PARAM_ALLOWED_MEM)
#define OV_PARAM_ALLOWED_IRM4	(OV_PARAM_ALLOWED_IREG | OV_PARAM_ALLOWED_MEM | OV_PARAM_FLAG_FIXED4)
#define OV_PARAM_ALLOWED_IRM8	(OV_PARAM_ALLOWED_IREG | OV_PARAM_ALLOWED_MEM | OV_PARAM_FLAG_FIXED8)
#define OV_PARAM_ALLOWED_FRM	(OV_PARAM_ALLOWED_FREG | OV_PARAM_ALLOWED_MEM)
#define OV_PARAM_ALLOWED_FRM4	(OV_PARAM_ALLOWED_FREG | OV_PARAM_ALLOWED_MEM | OV_PARAM_FLAG_FIXED4)
#define OV_PARAM_ALLOWED_FRM8	(OV_PARAM_ALLOWED_FREG | OV_PARAM_ALLOWED_MEM | OV_PARAM_FLAG_FIXED8)
#define OV_PARAM_ALLOWED_IANY	(OV_PARAM_ALLOWED_IRM | OV_PARAM_ALLOWED_IMV)
#define OV_PARAM_ALLOWED_IANY4	(OV_PARAM_ALLOWED_IRM | OV_PARAM_ALLOWED_IMV | OV_PARAM_FLAG_FIXED4)
#define OV_PARAM_ALLOWED_IANY8	(OV_PARAM_ALLOWED_IRM | OV_PARAM_ALLOWED_IMV | OV_PARAM_FLAG_FIXED8)
#define OV_PARAM_ALLOWED_FANY	(OV_PARAM_ALLOWED_FRM)
#define OV_PARAM_ALLOWED_FANY4	(OV_PARAM_ALLOWED_FRM | OV_PARAM_FLAG_FIXED4)
#define OV_PARAM_ALLOWED_FANY8	(OV_PARAM_ALLOWED_FRM | OV_PARAM_FLAG_FIXED8)



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
	UINT16					ptypes[4];			/* allowed types for parameters */
};


/* structure describing back-end validation test */
typedef struct _bevalidate_test bevalidate_test;
struct _bevalidate_test
{
	drcuml_opcode		opcode;
	UINT8				size;
	UINT8				destmask;
	UINT8				iflags;
	UINT8				flags;
	UINT64				param[4];
};



/***************************************************************************
    TABLES
***************************************************************************/

/* macro to simplify the table */
#define OPVALID_ENTRY_0(op,sizes,condflag)				{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, { OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE } },
#define OPVALID_ENTRY_1(op,sizes,condflag,p0)			{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, { OV_PARAM_ALLOWED_##p0, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE } },
#define OPVALID_ENTRY_2(op,sizes,condflag,p0,p1)		{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, { OV_PARAM_ALLOWED_##p0, OV_PARAM_ALLOWED_##p1, OV_PARAM_ALLOWED_NONE, OV_PARAM_ALLOWED_NONE } },
#define OPVALID_ENTRY_3(op,sizes,condflag,p0,p1,p2)		{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, { OV_PARAM_ALLOWED_##p0, OV_PARAM_ALLOWED_##p1, OV_PARAM_ALLOWED_##p2, OV_PARAM_ALLOWED_NONE } },
#define OPVALID_ENTRY_4(op,sizes,condflag,p0,p1,p2,p3)	{ DRCUML_OP_##op, sizes, OV_CONDFLAG_##condflag, { OV_PARAM_ALLOWED_##p0, OV_PARAM_ALLOWED_##p1, OV_PARAM_ALLOWED_##p2, OV_PARAM_ALLOWED_##p3 } },

/* opcode validation table */
static const drcuml_opcode_valid opcode_valid_list[] =
{
	/* Compile-time opcodes */
	OPVALID_ENTRY_1(HANDLE,  4,   NONE,  MEM)
	OPVALID_ENTRY_2(HASH,    4,   NONE,  IMV,  IMV)
	OPVALID_ENTRY_1(LABEL,   4,   NONE,  IMV)
	OPVALID_ENTRY_1(COMMENT, 4,   NONE,  ANYMEM)
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
	OPVALID_ENTRY_2(CALLC,   4,   COND,  ANYMEM,ANYMEM)
	OPVALID_ENTRY_2(RECOVER, 4,   NONE,  IRM,  MVAR)

	/* Internal Register Operations */
	OPVALID_ENTRY_1(SETFMOD, 4,   NONE,  IANY)
	OPVALID_ENTRY_1(GETFMOD, 4,   NONE,  IRM)
	OPVALID_ENTRY_1(GETEXP,  4,   NONE,  IRM)
	OPVALID_ENTRY_1(SAVE,    4,   NONE,  ANYMEM)
	OPVALID_ENTRY_1(RESTORE, 4,   NONE,  ANYMEM)

	/* Integer Operations */
	OPVALID_ENTRY_3(LOAD1U,  4|8, NONE,  IRM,   ANYMEM,IANY4)
	OPVALID_ENTRY_3(LOAD1S,  4|8, NONE,  IRM,   ANYMEM,IANY4)
	OPVALID_ENTRY_3(LOAD2U,  4|8, NONE,  IRM,   ANYMEM,IANY4)
	OPVALID_ENTRY_3(LOAD2S,  4|8, NONE,  IRM,   ANYMEM,IANY4)
	OPVALID_ENTRY_3(LOAD4U,  4|8, NONE,  IRM,   ANYMEM,IANY4)
	OPVALID_ENTRY_3(LOAD4S,    8, NONE,  IRM,   ANYMEM,IANY4)
	OPVALID_ENTRY_3(LOAD8U,    8, NONE,  IRM,   ANYMEM,IANY4)
	OPVALID_ENTRY_3(STORE1,  4|8, NONE,  ANYMEM,IANY4, IANY)
	OPVALID_ENTRY_3(STORE2,  4|8, NONE,  ANYMEM,IANY4, IANY)
	OPVALID_ENTRY_3(STORE4,  4|8, NONE,  ANYMEM,IANY4, IANY)
	OPVALID_ENTRY_3(STORE8,    8, NONE,  ANYMEM,IANY4, IANY)
	OPVALID_ENTRY_3(READ1U,  4|8, NONE,  IRM,   IMV4,  IANY4)
	OPVALID_ENTRY_3(READ1S,  4|8, NONE,  IRM,   IMV4,  IANY4)
	OPVALID_ENTRY_3(READ2U,  4|8, NONE,  IRM,   IMV4,  IANY4)
	OPVALID_ENTRY_3(READ2S,  4|8, NONE,  IRM,   IMV4,  IANY4)
	OPVALID_ENTRY_4(READ2M,  4|8, NONE,  IRM,   IMV4,  IANY4, IANY)
	OPVALID_ENTRY_3(READ4U,  4|8, NONE,  IRM,   IMV4,  IANY4)
	OPVALID_ENTRY_3(READ4S,    8, NONE,  IRM,   IMV4,  IANY4)
	OPVALID_ENTRY_4(READ4M,  4|8, NONE,  IRM,   IMV4,  IANY4, IANY)
	OPVALID_ENTRY_3(READ8U,    8, NONE,  IRM,   IMV4,  IANY4)
	OPVALID_ENTRY_4(READ8M,    8, NONE,  IRM,   IMV4,  IANY4, IANY)
	OPVALID_ENTRY_3(WRITE1,  4|8, NONE,  IMV4,  IANY4, IANY)
	OPVALID_ENTRY_3(WRITE2,  4|8, NONE,  IMV4,  IANY4, IANY)
	OPVALID_ENTRY_4(WRIT2M,  4|8, NONE,  IMV4,  IANY4, IANY,  IANY)
	OPVALID_ENTRY_3(WRITE4,  4|8, NONE,  IMV4,  IANY4, IANY)
	OPVALID_ENTRY_4(WRIT4M,  4|8, NONE,  IMV4,  IANY4, IANY,  IANY)
	OPVALID_ENTRY_3(WRITE8,    8, NONE,  IMV4,  IANY4, IANY)
	OPVALID_ENTRY_4(WRIT8M,    8, NONE,  IMV4,  IANY4, IANY,  IANY)
	OPVALID_ENTRY_3(FLAGS,   4|8, NONE,  IRM,   IMV,   MEM)
	OPVALID_ENTRY_2(SETC,    4|8, NONE,  IANY,  IANY)
	OPVALID_ENTRY_2(MOV,     4|8, COND,  IRM,   IANY)
	OPVALID_ENTRY_2(ZEXT1,   4|8, NONE,  IRM,   IANY4)
	OPVALID_ENTRY_2(ZEXT2,   4|8, NONE,  IRM,   IANY4)
	OPVALID_ENTRY_2(ZEXT4,     8, NONE,  IRM,   IANY4)
	OPVALID_ENTRY_2(SEXT1,   4|8, NONE,  IRM,   IANY4)
	OPVALID_ENTRY_2(SEXT2,   4|8, NONE,  IRM,   IANY4)
	OPVALID_ENTRY_2(SEXT4,     8, NONE,  IRM,   IANY4)
	OPVALID_ENTRY_4(XTRACT,  4|8, NONE,  IRM,   IANY,  IANY,  IANY)
	OPVALID_ENTRY_4(INSERT,  4|8, NONE,  IRM,   IANY,  IANY,  IANY)
	OPVALID_ENTRY_3(ADD,     4|8, SZVC,  IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(ADDC,    4|8, SZVC,  IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(SUB,     4|8, SZVC,  IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(SUBB,    4|8, SZVC,  IRM,   IANY,  IANY)
	OPVALID_ENTRY_2(CMP,     4|8, SZVC,  IANY,  IANY)
	OPVALID_ENTRY_4(MULU,    4|8, SZ,    IRM,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_4(MULS,    4|8, SZ,    IRM,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_4(DIVU,    4|8, SZ,    IRM,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_4(DIVS,    4|8, SZ,    IRM,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(AND,     4|8, SZ,    IRM,   IANY,  IANY)
	OPVALID_ENTRY_2(TEST,    4|8, SZ,    IANY,  IANY)
	OPVALID_ENTRY_3(OR,      4|8, SZ,    IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(XOR,     4|8, SZ,    IRM,   IANY,  IANY)
	OPVALID_ENTRY_2(LZCNT,   4|8, NONE,  IRM,   IANY)
	OPVALID_ENTRY_2(BSWAP,   4|8, NONE,  IRM,   IANY)
	OPVALID_ENTRY_3(SHL,     4|8, SZC,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(SHR,     4|8, SZC,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(SAR,     4|8, SZC,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(ROL,     4|8, SZC,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(ROLC,    4|8, SZC,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(ROR,     4|8, SZC,   IRM,   IANY,  IANY)
	OPVALID_ENTRY_3(RORC,    4|8, SZC,   IRM,   IANY,  IANY)

	/* Floating Point Operations */
	OPVALID_ENTRY_3(FLOAD,   4|8, NONE,  FRM,   ANYMEM,IANY4)
	OPVALID_ENTRY_3(FSTORE,  4|8, NONE,  ANYMEM,IANY4, FANY)
	OPVALID_ENTRY_3(FREAD,   4|8, NONE,  FRM,   IMV4,  IANY4)
	OPVALID_ENTRY_3(FWRITE,  4|8, NONE,  IMV4,  IANY4, FANY)
	OPVALID_ENTRY_2(FMOV,    4|8, COND,  FRM,   FANY)
	OPVALID_ENTRY_2(FTOI4,   4|8, NONE,  IRM4,  FANY)
	OPVALID_ENTRY_2(FTOI4T,  4|8, NONE,  IRM4,  FANY)
	OPVALID_ENTRY_2(FTOI4R,  4|8, NONE,  IRM4,  FANY)
	OPVALID_ENTRY_2(FTOI4F,  4|8, NONE,  IRM4,  FANY)
	OPVALID_ENTRY_2(FTOI4C,  4|8, NONE,  IRM4,  FANY)
	OPVALID_ENTRY_2(FTOI8,   4|8, NONE,  IRM8,  FANY)
	OPVALID_ENTRY_2(FTOI8T,  4|8, NONE,  IRM8,  FANY)
	OPVALID_ENTRY_2(FTOI8R,  4|8, NONE,  IRM8,  FANY)
	OPVALID_ENTRY_2(FTOI8F,  4|8, NONE,  IRM8,  FANY)
	OPVALID_ENTRY_2(FTOI8C,  4|8, NONE,  IRM8,  FANY)
	OPVALID_ENTRY_2(FFRFS,     8, NONE,  FRM,   FANY4)
	OPVALID_ENTRY_2(FFRFD,   4  , NONE,  FRM,   FANY8)
	OPVALID_ENTRY_2(FFRI4,   4|8, NONE,  FRM,   IANY4)
	OPVALID_ENTRY_2(FFRI8,   4|8, NONE,  FRM,   IANY8)
	OPVALID_ENTRY_3(FADD,    4|8, NONE,  FRM,   FANY,  FANY)
	OPVALID_ENTRY_3(FSUB,    4|8, NONE,  FRM,   FANY,  FANY)
	OPVALID_ENTRY_2(FCMP,    4|8, UZC,   FANY,  FANY)
	OPVALID_ENTRY_3(FMUL,    4|8, NONE,  FRM,   FANY,  FANY)
	OPVALID_ENTRY_3(FDIV,    4|8, NONE,  FRM,   FANY,  FANY)
	OPVALID_ENTRY_2(FNEG,    4|8, NONE,  FRM,   FANY)
	OPVALID_ENTRY_2(FABS,    4|8, NONE,  FRM,   FANY)
	OPVALID_ENTRY_2(FSQRT,   4|8, NONE,  FRM,   FANY)
	OPVALID_ENTRY_2(FRECIP,  4|8, NONE,  FRM,   FANY)
	OPVALID_ENTRY_2(FRSQRT,  4|8, NONE,  FRM,   FANY)
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

static void validate_instruction(drcuml_block *block, const drcuml_instruction *inst);
static void validate_backend(drcuml_state *drcuml);
static void bevalidate_iterate_over_params(drcuml_state *drcuml, drcuml_codehandle **handles, const bevalidate_test *test, drcuml_parameter *paramlist, int pnum);
static void bevalidate_iterate_over_flags(drcuml_state *drcuml, drcuml_codehandle **handles, const bevalidate_test *test, drcuml_parameter *paramlist);
static void bevalidate_execute(drcuml_state *drcuml, drcuml_codehandle **handles, const bevalidate_test *test, const drcuml_parameter *paramlist, UINT8 flagmask);
static void bevalidate_initialize_random_state(drcuml_block *block, drcuml_machine_state *state);
static int bevalidate_populate_state(drcuml_block *block, drcuml_machine_state *state, const bevalidate_test *test, const drcuml_parameter *paramlist, drcuml_parameter *params, UINT64 *parammem);
static int bevalidate_verify_state(drcuml_state *drcuml, const drcuml_machine_state *istate, drcuml_machine_state *state, const bevalidate_test *test, const drcuml_parameter *params, const drcuml_instruction *testinst, drccodeptr codestart, drccodeptr codeend, UINT8 flagmask);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    size_for_param - given a set of parameter
    flags and the instruction size, return the
    size of the parameter
-------------------------------------------------*/

INLINE int size_for_param(int instsize, UINT16 flags)
{
	if (flags & OV_PARAM_FLAG_FIXED4)
		return 4;
	else if (flags & OV_PARAM_FLAG_FIXED8)
		return 8;
	else
		return instsize;
}


/*-------------------------------------------------
    mask_for_param - given a set of parameter
    flags and the instruction size, return a
    mask of valid bits for that parameter
-------------------------------------------------*/

INLINE UINT64 mask_for_param(int instsize, UINT16 flags)
{
	switch (size_for_param(instsize, flags))
	{
		case 4:		return U64(0x00000000ffffffff);
		case 8:		return U64(0xffffffffffffffff);
	}
	return 0;
}



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
    drcuml_get_backend_info - return information
    about the back-end
-------------------------------------------------*/

void drcuml_get_backend_info(drcuml_state *drcuml, drcbe_info *info)
{
	(*drcuml->beintf->be_get_info)(drcuml->bestate, info);
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

	/* do a one-time validation if requested */
	if (VALIDATE_BACKEND)
	{
		static int validated = FALSE;
		if (!validated)
		{
			validated = TRUE;
			validate_backend(drcuml);
		}
	}
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
		assert(((opvalid->ptypes[0] >> inst->param[0].type) & 1) != 0);
		if (inst->param[0].type == DRCUML_PTYPE_MEMORY && !(opvalid->ptypes[0] & OV_PARAM_FLAG_ANYMEM))
			assert_in_near_cache(block->drcuml->cache, (void *)(FPTR)inst->param[0].value);
	}
	if (inst->numparams > 1)
	{
		assert(inst->param[1].type > DRCUML_PTYPE_NONE && inst->param[1].type < DRCUML_PTYPE_MAX);
		assert(((opvalid->ptypes[1] >> inst->param[1].type) & 1) != 0);
		if (inst->param[1].type == DRCUML_PTYPE_MEMORY && !(opvalid->ptypes[1] & OV_PARAM_FLAG_ANYMEM))
			assert_in_near_cache(block->drcuml->cache, (void *)(FPTR)inst->param[1].value);
	}
	if (inst->numparams > 2)
	{
		assert(inst->param[2].type > DRCUML_PTYPE_NONE && inst->param[2].type < DRCUML_PTYPE_MAX);
		assert(((opvalid->ptypes[2] >> inst->param[2].type) & 1) != 0);
		if (inst->param[2].type == DRCUML_PTYPE_MEMORY && !(opvalid->ptypes[2] & OV_PARAM_FLAG_ANYMEM))
			assert_in_near_cache(block->drcuml->cache, (void *)(FPTR)inst->param[2].value);
	}
	if (inst->numparams > 3)
	{
		assert(inst->param[3].type > DRCUML_PTYPE_NONE && inst->param[3].type < DRCUML_PTYPE_MAX);
		assert(((opvalid->ptypes[3] >> inst->param[3].type) & 1) != 0);
		if (inst->param[3].type == DRCUML_PTYPE_MEMORY && !(opvalid->ptypes[3] & OV_PARAM_FLAG_ANYMEM))
			assert_in_near_cache(block->drcuml->cache, (void *)(FPTR)inst->param[3].value);
	}
}



/***************************************************************************
    BACK-END VALIDATION
***************************************************************************/

#define TEST_ENTRY_DSS(op, size, p1, p2, p3, flags) { DRCUML_OP_##op, size, 0x01, 0, flags, { U64(p1), U64(p2), U64(p3) } },
#define TEST_ENTRY_DSSI(op, size, p1, p2, p3, iflags, flags) { DRCUML_OP_##op, size, 0x01, iflags, flags, { U64(p1), U64(p2), U64(p3) } },

static const bevalidate_test bevalidate_test_list[] =
{
	TEST_ENTRY_DSS(ADD, 4, 0x7fffffff, 0x12345678, 0x6dcba987, 0)
	TEST_ENTRY_DSS(ADD, 4, 0x80000000, 0x12345678, 0x6dcba988, FLAGS_V | FLAGS_S)
	TEST_ENTRY_DSS(ADD, 4, 0xffffffff, 0x92345678, 0x6dcba987, FLAGS_S)
	TEST_ENTRY_DSS(ADD, 4, 0x00000000, 0x92345678, 0x6dcba988, FLAGS_C | FLAGS_Z)

	TEST_ENTRY_DSS(ADD, 8, 0x7fffffffffffffff, 0x0123456789abcdef, 0x7edcba9876543210, 0)
	TEST_ENTRY_DSS(ADD, 8, 0x8000000000000000, 0x0123456789abcdef, 0x7edcba9876543211, FLAGS_V | FLAGS_S)
	TEST_ENTRY_DSS(ADD, 8, 0xffffffffffffffff, 0x8123456789abcdef, 0x7edcba9876543210, FLAGS_S)
	TEST_ENTRY_DSS(ADD, 8, 0x0000000000000000, 0x8123456789abcdef, 0x7edcba9876543211, FLAGS_C | FLAGS_Z)

	TEST_ENTRY_DSSI(ADDC, 4, 0x7fffffff, 0x12345678, 0x6dcba987, 0,       0)
	TEST_ENTRY_DSSI(ADDC, 4, 0x7fffffff, 0x12345678, 0x6dcba986, FLAGS_C, 0)
	TEST_ENTRY_DSSI(ADDC, 4, 0x80000000, 0x12345678, 0x6dcba988, 0,       FLAGS_V | FLAGS_S)
	TEST_ENTRY_DSSI(ADDC, 4, 0x80000000, 0x12345678, 0x6dcba987, FLAGS_C, FLAGS_V | FLAGS_S)
	TEST_ENTRY_DSSI(ADDC, 4, 0xffffffff, 0x92345678, 0x6dcba987, 0,       FLAGS_S)
	TEST_ENTRY_DSSI(ADDC, 4, 0xffffffff, 0x92345678, 0x6dcba986, FLAGS_C, FLAGS_S)
	TEST_ENTRY_DSSI(ADDC, 4, 0x00000000, 0x92345678, 0x6dcba988, 0,       FLAGS_C | FLAGS_Z)
	TEST_ENTRY_DSSI(ADDC, 4, 0x00000000, 0x92345678, 0x6dcba987, FLAGS_C, FLAGS_C | FLAGS_Z)
	TEST_ENTRY_DSSI(ADDC, 4, 0x12345678, 0x12345678, 0xffffffff, FLAGS_C, FLAGS_C)

	TEST_ENTRY_DSSI(ADDC, 8, 0x7fffffffffffffff, 0x0123456789abcdef, 0x7edcba9876543210, 0,       0)
	TEST_ENTRY_DSSI(ADDC, 8, 0x7fffffffffffffff, 0x0123456789abcdef, 0x7edcba987654320f, FLAGS_C, 0)
	TEST_ENTRY_DSSI(ADDC, 8, 0x8000000000000000, 0x0123456789abcdef, 0x7edcba9876543211, 0,       FLAGS_V | FLAGS_S)
	TEST_ENTRY_DSSI(ADDC, 8, 0x8000000000000000, 0x0123456789abcdef, 0x7edcba9876543210, FLAGS_C, FLAGS_V | FLAGS_S)
	TEST_ENTRY_DSSI(ADDC, 8, 0xffffffffffffffff, 0x8123456789abcdef, 0x7edcba9876543210, 0,       FLAGS_S)
	TEST_ENTRY_DSSI(ADDC, 8, 0xffffffffffffffff, 0x8123456789abcdef, 0x7edcba987654320f, FLAGS_C, FLAGS_S)
	TEST_ENTRY_DSSI(ADDC, 8, 0x0000000000000000, 0x8123456789abcdef, 0x7edcba9876543211, 0,       FLAGS_C | FLAGS_Z)
	TEST_ENTRY_DSSI(ADDC, 8, 0x0000000000000000, 0x8123456789abcdef, 0x7edcba9876543210, FLAGS_C, FLAGS_C | FLAGS_Z)
	TEST_ENTRY_DSSI(ADDC, 8, 0x123456789abcdef0, 0x123456789abcdef0, 0xffffffffffffffff, FLAGS_C, FLAGS_C)
};


/*-------------------------------------------------
    validate_backend - execute a number of
    generic tests on the backend code generator
-------------------------------------------------*/

static void validate_backend(drcuml_state *drcuml)
{
	drcuml_codehandle *handles[3];
	int tnum;

	/* allocate handles for the code */
	handles[0] = drcuml_handle_alloc(drcuml, "test_entry");
	handles[1] = drcuml_handle_alloc(drcuml, "code_start");
	handles[2] = drcuml_handle_alloc(drcuml, "code_end");

	/* iterate over test entries */
	printf("Backend validation....\n");
	for (tnum = 0; tnum < ARRAY_LENGTH(bevalidate_test_list); tnum++)
	{
		const bevalidate_test *test = &bevalidate_test_list[tnum];
		drcuml_parameter param[ARRAY_LENGTH(test->param)];

		/* reset parameter list and iterate */
		memset(param, 0, sizeof(param));
		printf("Executing test %d/%d", tnum + 1, (int)ARRAY_LENGTH(bevalidate_test_list));
		bevalidate_iterate_over_params(drcuml, handles, test, param, 0);
		printf("\n");
	}
	fatalerror("All tests passed!");
}


/*-------------------------------------------------
    bevalidate_iterate_over_params - iterate over
    all supported types and values of a parameter
    and recursively hand off to the next parameter,
    or else move on to iterate over the flags
-------------------------------------------------*/

static void bevalidate_iterate_over_params(drcuml_state *drcuml, drcuml_codehandle **handles, const bevalidate_test *test, drcuml_parameter *paramlist, int pnum)
{
	const drcuml_opcode_valid *opvalid = &opcode_valid_table[test->opcode];

	/* if no parameters, execute now */
	if (pnum >= ARRAY_LENGTH(opvalid->ptypes) || opvalid->ptypes[pnum] == OV_PARAM_ALLOWED_NONE)
	{
		bevalidate_iterate_over_flags(drcuml, handles, test, paramlist);
		return;
	}

	/* iterate over valid parameter types */
	for (paramlist[pnum].type = DRCUML_PTYPE_IMMEDIATE; paramlist[pnum].type < DRCUML_PTYPE_MAX; paramlist[pnum].type++)
		if (opvalid->ptypes[pnum] & (1 << paramlist[pnum].type))
		{
			int pindex, pcount;

			/* mapvars can only do 32-bit tests */
			if (paramlist[pnum].type == DRCUML_PTYPE_MAPVAR && size_for_param(test->size, opvalid->ptypes[pnum]) == 8)
				continue;

			/* for some parameter types, we wish to iterate over all possibilities */
			switch (paramlist[pnum].type)
			{
				case DRCUML_PTYPE_INT_REGISTER:		pcount = DRCUML_REG_I_END - DRCUML_REG_I0;		break;
				case DRCUML_PTYPE_FLOAT_REGISTER:	pcount = DRCUML_REG_F_END - DRCUML_REG_F0;		break;
				case DRCUML_PTYPE_MAPVAR:			pcount = DRCUML_MAPVAR_END - DRCUML_MAPVAR_M0;	break;
				default:							pcount = 1;										break;
			}

			/* iterate over possibilities */
			for (pindex = 0; pindex < pcount; pindex++)
			{
				int skip = FALSE;
				int pscannum;

				/* for param 0, print a dot */
				if (pnum == 0)
					printf(".");

				/* can't duplicate multiple source parameters unless they are immediates */
				if (paramlist[pnum].type != DRCUML_PTYPE_IMMEDIATE && ((test->destmask >> pnum) & 1) == 0)

					/* loop over all parameters we've done before; if the parameter is a source and matches us, skip this case */
					for (pscannum = 0; pscannum < pnum; pscannum++)
						if (((test->destmask >> pscannum) & 1) == 0 && paramlist[pnum].type == paramlist[pscannum].type && pindex == paramlist[pscannum].value)
							skip = TRUE;

				/* iterate over the next parameter in line */
				if (!skip)
				{
					paramlist[pnum].value = pindex;
					bevalidate_iterate_over_params(drcuml, handles, test, paramlist, pnum + 1);
				}
			}
		}
}


/*-------------------------------------------------
    bevalidate_iterate_over_flags - iterate over
    all supported flag masks
-------------------------------------------------*/

static void bevalidate_iterate_over_flags(drcuml_state *drcuml, drcuml_codehandle **handles, const bevalidate_test *test, drcuml_parameter *paramlist)
{
	const drcuml_opcode_valid *opvalid = &opcode_valid_table[test->opcode];
	UINT8 flagmask = opvalid->condflags & 0x1f;
	UINT8 curmask;

	/* iterate over all possible flag combinations */
	for (curmask = 0; curmask <= flagmask; curmask++)
		if ((curmask & flagmask) == curmask)
			bevalidate_execute(drcuml, handles, test, paramlist, curmask);
}


/*-------------------------------------------------
    bevalidate_execute - execute a single instance
    of a test, generating code and verifying the
    results
-------------------------------------------------*/

static void bevalidate_execute(drcuml_state *drcuml, drcuml_codehandle **handles, const bevalidate_test *test, const drcuml_parameter *paramlist, UINT8 flagmask)
{
	drcuml_parameter params[ARRAY_LENGTH(test->param)];
	drcuml_machine_state istate, fstate;
	drcuml_instruction testinst;
	drcuml_block *block;
	UINT64 *parammem;
	int numparams;

	/* allocate memory for parameters */
	parammem = drccache_memory_alloc_near(drcuml->cache, sizeof(UINT64) * ARRAY_LENGTH(test->param));

	/* flush the cache */
	drcuml_reset(drcuml);

	/* start a new block */
	block = drcuml_block_begin(drcuml, 30, NULL);
	UML_HANDLE(block, handles[0]);

	/* set up a random initial state */
	bevalidate_initialize_random_state(block, &istate);

	/* then populate the state with the parameters */
	numparams = bevalidate_populate_state(block, &istate, test, paramlist, params, parammem);

	/* generate the code */
	UML_RESTORE(block, &istate);
	UML_HANDLE(block, handles[1]);
	switch (numparams)
	{
		case 0:
			drcuml_block_append_0(block, test->opcode, test->size, flagmask);
			break;

		case 1:
			drcuml_block_append_1(block, test->opcode, test->size, flagmask, params[0].type, params[0].value);
			break;

		case 2:
			drcuml_block_append_2(block, test->opcode, test->size, flagmask, params[0].type, params[0].value, params[1].type, params[1].value);
			break;

		case 3:
			drcuml_block_append_3(block, test->opcode, test->size, flagmask, params[0].type, params[0].value, params[1].type, params[1].value, params[2].type, params[2].value);
			break;

		case 4:
			drcuml_block_append_4(block, test->opcode, test->size, flagmask, params[0].type, params[0].value, params[1].type, params[1].value, params[2].type, params[2].value, params[3].type, params[3].value);
			break;
	}
	testinst = block->inst[block->nextinst - 1];
	UML_HANDLE(block, handles[2]);
	UML_SAVE(block, &fstate);
	UML_EXIT(block, IMM(0));

	/* end the block */
	drcuml_block_end(block);

	/* execute */
	drcuml_execute(drcuml, handles[0]);

	/* verify the results */
	bevalidate_verify_state(drcuml, &istate, &fstate, test, params, &testinst, handles[1]->code, handles[2]->code, flagmask);

	/* free memory */
	drccache_memory_free(drcuml->cache, parammem, sizeof(UINT64) * ARRAY_LENGTH(test->param));
}


/*-------------------------------------------------
    bevalidate_initialize_random_state -
    initialize the machine state to randomness
-------------------------------------------------*/

static void bevalidate_initialize_random_state(drcuml_block *block, drcuml_machine_state *state)
{
	int regnum;

	/* initialize core state to random values */
	state->fmod = mame_rand(Machine) & 0x03;
	state->flags = mame_rand(Machine) & 0x1f;
	state->exp = mame_rand(Machine);

	/* initialize integer registers to random values */
	for (regnum = 0; regnum < ARRAY_LENGTH(state->r); regnum++)
	{
		state->r[regnum].w.h = mame_rand(Machine);
		state->r[regnum].w.l = mame_rand(Machine);
	}

	/* initialize float registers to random values */
	for (regnum = 0; regnum < ARRAY_LENGTH(state->f); regnum++)
	{
		*(UINT32 *)&state->f[regnum].s.h = mame_rand(Machine);
		*(UINT32 *)&state->f[regnum].s.l = mame_rand(Machine);
	}

	/* initialize map variables to random values */
	for (regnum = 0; regnum < DRCUML_MAPVAR_END - DRCUML_MAPVAR_M0; regnum++)
		UML_MAPVAR(block, MVAR(regnum), mame_rand(Machine));
}


/*-------------------------------------------------
    bevalidate_populate_state - populate the
    machine state with the proper values prior
    to executing a test
-------------------------------------------------*/

static int bevalidate_populate_state(drcuml_block *block, drcuml_machine_state *state, const bevalidate_test *test, const drcuml_parameter *paramlist, drcuml_parameter *params, UINT64 *parammem)
{
	const drcuml_opcode_valid *opvalid = &opcode_valid_table[test->opcode];
	int numparams = ARRAY_LENGTH(test->param);
	int pnum;

	/* copy flags as-is */
	state->flags = test->iflags;

	/* iterate over parameters */
	for (pnum = 0; pnum < ARRAY_LENGTH(test->param); pnum++)
	{
		int psize = size_for_param(test->size, opvalid->ptypes[pnum]);
		drcuml_parameter *curparam = &params[pnum];

		/* start with a copy of the parameter from the list */
		*curparam = paramlist[pnum];

		/* switch off the type */
		switch (curparam->type)
		{
			/* immediate parameters: take the value from the test entry */
			case DRCUML_PTYPE_IMMEDIATE:
				curparam->value = test->param[pnum];
				break;

			/* register parameters: set the register value in the state and set the parameter value to the register index */
			case DRCUML_PTYPE_INT_REGISTER:
				state->r[curparam->value].d = test->param[pnum];
				curparam->value += DRCUML_REG_I0;
				break;

			/* register parameters: set the register value in the state and set the parameter value to the register index */
			case DRCUML_PTYPE_FLOAT_REGISTER:
				state->f[curparam->value].d = test->param[pnum];
				curparam->value += DRCUML_REG_F0;
				break;

			/* memory parameters: set the memory value in the parameter space and set the parameter value to point to it */
			case DRCUML_PTYPE_MEMORY:
				curparam->value = (FPTR)&parammem[pnum];
				if (psize == 4)
					*(UINT32 *)(FPTR)curparam->value = test->param[pnum];
				else
					*(UINT64 *)(FPTR)curparam->value = test->param[pnum];
				break;

			/* map variables: issue a MAPVAR instruction to set the value and set the parameter value to the mapvar index */
			case DRCUML_PTYPE_MAPVAR:
				UML_MAPVAR(block, MVAR(curparam->value), test->param[pnum]);
				curparam->value += DRCUML_MAPVAR_M0;
				break;

			/* use anything else to count the number of parameters */
			default:
				numparams = MIN(numparams, pnum);
				break;
		}
	}

	/* return the total number of parameters */
	return numparams;
}


/*-------------------------------------------------
    bevalidate_verify_state - verify the final
    state after executing a test, and report any
    discrepancies
-------------------------------------------------*/

static int bevalidate_verify_state(drcuml_state *drcuml, const drcuml_machine_state *istate, drcuml_machine_state *state, const bevalidate_test *test, const drcuml_parameter *params, const drcuml_instruction *testinst, drccodeptr codestart, drccodeptr codeend, UINT8 flagmask)
{
	const drcuml_opcode_valid *opvalid = &opcode_valid_table[test->opcode];
	UINT8 ireg[DRCUML_REG_I_END - DRCUML_REG_I0] = { 0 };
	UINT8 freg[DRCUML_REG_F_END - DRCUML_REG_F0] = { 0 };
	char errorbuf[1024];
	char *errend = errorbuf;
	int pnum, regnum;

	*errend = 0;

	/* check flags */
	if ((state->flags & flagmask) != (test->flags & flagmask))
	{
		errend += sprintf(errend, "  Flags ... result:%c%c%c%c%c  expected:%c%c%c%c%c\n",
			(flagmask & DRCUML_FLAG_U) ? ((state->flags & DRCUML_FLAG_U) ? 'U' : '.') : '-',
			(flagmask & DRCUML_FLAG_S) ? ((state->flags & DRCUML_FLAG_S) ? 'S' : '.') : '-',
			(flagmask & DRCUML_FLAG_Z) ? ((state->flags & DRCUML_FLAG_Z) ? 'Z' : '.') : '-',
			(flagmask & DRCUML_FLAG_V) ? ((state->flags & DRCUML_FLAG_V) ? 'V' : '.') : '-',
			(flagmask & DRCUML_FLAG_C) ? ((state->flags & DRCUML_FLAG_C) ? 'C' : '.') : '-',
			(flagmask & DRCUML_FLAG_U) ? ((test->flags & DRCUML_FLAG_U) ? 'U' : '.') : '-',
			(flagmask & DRCUML_FLAG_S) ? ((test->flags & DRCUML_FLAG_S) ? 'S' : '.') : '-',
			(flagmask & DRCUML_FLAG_Z) ? ((test->flags & DRCUML_FLAG_Z) ? 'Z' : '.') : '-',
			(flagmask & DRCUML_FLAG_V) ? ((test->flags & DRCUML_FLAG_V) ? 'V' : '.') : '-',
			(flagmask & DRCUML_FLAG_C) ? ((test->flags & DRCUML_FLAG_C) ? 'C' : '.') : '-');
	}

	/* check destination parameters */
	for (pnum = 0; pnum < ARRAY_LENGTH(test->param); pnum++)
		if (test->destmask & (1 << pnum))
		{
			UINT64 mask = mask_for_param(test->size, opvalid->ptypes[pnum]);
			int psize = size_for_param(test->size, opvalid->ptypes[pnum]);
			UINT64 result = 0;

			/* fetch the result from the parameters */
			switch (params[pnum].type)
			{
				/* integer registers fetch from the state */
				case DRCUML_PTYPE_INT_REGISTER:
					ireg[params[pnum].value - DRCUML_REG_I0] = 1;
					result = state->r[params[pnum].value - DRCUML_REG_I0].d;
					break;

				/* float registers fetch from the state */
				case DRCUML_PTYPE_FLOAT_REGISTER:
					freg[params[pnum].value - DRCUML_REG_I0] = 1;
					result = state->f[params[pnum].value - DRCUML_REG_F0].d;
					break;

				/* memory registers fetch from the memory address */
				case DRCUML_PTYPE_MEMORY:
					if (psize == 4)
						result = *(UINT32 *)(FPTR)params[pnum].value;
					else
						result = *(UINT64 *)(FPTR)params[pnum].value;
					break;

				default:
					break;
			}

			/* check against the mask */
			if ((result & mask) != (test->param[pnum] & mask))
			{
				if ((UINT32)mask == mask)
					errend += sprintf(errend, "  Parameter %d ... result:%08X  expected:%08X\n", pnum,
										(UINT32)(result & mask), (UINT32)(test->param[pnum] & mask));
				else
					errend += sprintf(errend, "  Parameter %d ... result:%08X%08X  expected:%08X%08X\n", pnum,
										(UINT32)((result & mask) >> 32), (UINT32)(result & mask),
										(UINT32)((test->param[pnum] & mask) >> 32), (UINT32)(test->param[pnum] & mask));
			}
		}

	/* check source integer parameters for unexpected alterations */
	for (regnum = 0; regnum < ARRAY_LENGTH(state->r); regnum++)
		if (ireg[regnum] == 0 && istate->r[regnum].d != state->r[regnum].d)
			errend += sprintf(errend, "  Register i%d ... result:%08X%08X  originally:%08X%08X\n", regnum,
								(UINT32)(state->r[regnum].d >> 32), (UINT32)state->r[regnum].d,
								(UINT32)(istate->r[regnum].d >> 32), (UINT32)istate->r[regnum].d);

	/* check source float parameters for unexpected alterations */
	for (regnum = 0; regnum < ARRAY_LENGTH(state->f); regnum++)
		if (freg[regnum] == 0 && *(UINT64 *)&istate->f[regnum].d != *(UINT64 *)&state->f[regnum].d)
			errend += sprintf(errend, "  Register f%d ... result:%08X%08X  originally:%08X%08X\n", regnum,
								(UINT32)(*(UINT64 *)&state->f[regnum].d >> 32), (UINT32)*(UINT64 *)&state->f[regnum].d,
								(UINT32)(*(UINT64 *)&istate->f[regnum].d >> 32), (UINT32)*(UINT64 *)&istate->f[regnum].d);

	/* output the error if we have one */
	if (errend != errorbuf)
	{
		char disasm[100];

		/* disassemble the test instruction */
		drcuml_disasm(testinst, disasm);

		/* output a description of what went wrong */
		printf("\n");
		printf("----------------------------------------------\n");
		printf("Backend validation error:\n");
		printf("   %s\n", disasm);
		printf("\n");
		printf("Errors:\n");
		printf("%s\n", errorbuf);
		fatalerror("Error during validation");
	}
	return errend != errorbuf;
}
