/***************************************************************************

    drcuml.c

    Universal machine language for dynamic recompiling CPU cores.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Future improvements/changes:

    * UML optimizer:
        - detects constant operations, converts to MOV
        - eliminates no-ops (add i0,i0,0)
        - determines flags automatically
        - constant folding

    * Write a back-end validator:
        - checks all combinations of memory/register/immediate on all params
        - checks behavior of all opcodes

    * Extend registers to 16? Depends on if PPC can use them

    * Support for FPU exceptions

    * New instructions?
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
#define OPFLAGS_NONE	0x00
#define OPFLAGS_C		DRCUML_FLAG_C
#define OPFLAGS_SZ		(DRCUML_FLAG_S | DRCUML_FLAG_Z)
#define OPFLAGS_SZC		(DRCUML_FLAG_S | DRCUML_FLAG_Z | DRCUML_FLAG_C)
#define OPFLAGS_SZV		(DRCUML_FLAG_S | DRCUML_FLAG_Z | DRCUML_FLAG_V)
#define OPFLAGS_SZVC	(DRCUML_FLAG_S | DRCUML_FLAG_Z | DRCUML_FLAG_V | DRCUML_FLAG_C)
#define OPFLAGS_UZC		(DRCUML_FLAG_U | DRCUML_FLAG_Z | DRCUML_FLAG_C)
#define OPFLAGS_ALL		0x1f
#define OPFLAGS_P1		0x81
#define OPFLAGS_P2		0x82
#define OPFLAGS_P3		0x83
#define OPFLAGS_P4		0x84

/* parameter input/output states */
#define PIO_IN			0x01
#define PIO_OUT			0x02
#define PIO_INOUT		(PIO_IN | PIO_OUT)

/* parameter sizes */
#define PSIZE_4			DRCUML_SIZE_DWORD
#define PSIZE_8			DRCUML_SIZE_QWORD
#define PSIZE_OP		0x80
#define PSIZE_P1		0x81
#define PSIZE_P2		0x82
#define PSIZE_P3		0x83
#define PSIZE_P4		0x84

/* basic parameter types */
#define PTYPES_NONE		0
#define PTYPES_IMM		(1 << DRCUML_PTYPE_IMMEDIATE)
#define PTYPES_IREG		(1 << DRCUML_PTYPE_INT_REGISTER)
#define PTYPES_FREG		(1 << DRCUML_PTYPE_FLOAT_REGISTER)
#define PTYPES_MVAR		(1 << DRCUML_PTYPE_MAPVAR)
#define PTYPES_MEM		(1 << DRCUML_PTYPE_MEMORY)

/* special parameter types */
#define PTYPES_PTR		(PTYPES_MEM | 0x1000)
#define PTYPES_STATE	(PTYPES_MEM | 0x2000)
#define PTYPES_STR		(PTYPES_MEM | 0x3000)
#define PTYPES_CFUNC	(PTYPES_MEM | 0x4000)
#define PTYPES_HAND		(PTYPES_MEM | 0x5000)
#define PTYPES_SIZE		(PTYPES_IMM | 0x8000)
#define PTYPES_SPACE	(PTYPES_IMM | 0x9000)
#define PTYPES_SPSZ		(PTYPES_IMM | 0xa000)
#define PTYPES_FMOD		(PTYPES_IMM | 0xb000)
#define PTYPES_LAB		(PTYPES_IMM | 0xc000)

/* combinations of types */
#define PTYPES_IRM		(PTYPES_IREG | PTYPES_MEM)
#define PTYPES_FRM		(PTYPES_FREG | PTYPES_MEM)
#define PTYPES_IMV		(PTYPES_IMM | PTYPES_MVAR)
#define PTYPES_IANY		(PTYPES_IRM | PTYPES_IMV)
#define PTYPES_FANY		(PTYPES_FRM)



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
#define PINFO(inout, size, types)									{ PIO_##inout, PSIZE_##size, PTYPES_##types }
#define OPINFO0(op,str,sizes,cond,iflag,oflag,mflag)				{ DRCUML_OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { { 0 } } },
#define OPINFO1(op,str,sizes,cond,iflag,oflag,mflag,p0)				{ DRCUML_OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { p0 } },
#define OPINFO2(op,str,sizes,cond,iflag,oflag,mflag,p0,p1)			{ DRCUML_OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { p0, p1 } },
#define OPINFO3(op,str,sizes,cond,iflag,oflag,mflag,p0,p1,p2)		{ DRCUML_OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { p0, p1, p2 } },
#define OPINFO4(op,str,sizes,cond,iflag,oflag,mflag,p0,p1,p2,p3)	{ DRCUML_OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { p0, p1, p2, p3 } },

/* opcode validation table */
static const drcuml_opcode_info opcode_info_source[] =
{
	/* Compile-time opcodes */
	OPINFO1(HANDLE,  "handle",   4,   FALSE, NONE, NONE, NONE, PINFO(IN, OP, HAND))
	OPINFO2(HASH,    "hash",     4,   FALSE, NONE, NONE, NONE, PINFO(IN, OP, IMV), PINFO(IN, OP, IMV))
	OPINFO1(LABEL,   "label",    4,   FALSE, NONE, NONE, NONE, PINFO(IN, OP, LAB))
	OPINFO1(COMMENT, "comment",  4,   FALSE, NONE, NONE, NONE, PINFO(IN, OP, STR))
	OPINFO2(MAPVAR,  "mapvar",   4,   FALSE, NONE, NONE, NONE, PINFO(OUT, OP, MVAR), PINFO(IN, OP, IMV))

	/* Control Flow Operations */
	OPINFO1(DEBUG,   "debug",    4,   FALSE, NONE, NONE, ALL,  PINFO(IN, OP, IANY))
	OPINFO1(EXIT,    "exit",     4,   TRUE,  NONE, NONE, ALL,  PINFO(IN, OP, IANY))
	OPINFO3(HASHJMP, "hashjmp",  4,   FALSE, NONE, NONE, ALL,  PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, HAND))
	OPINFO1(JMP,     "jmp",      4,   TRUE,  NONE, NONE, NONE, PINFO(IN, OP, LAB))
	OPINFO2(EXH,     "exh",      4,   TRUE,  NONE, NONE, ALL,  PINFO(IN, OP, HAND), PINFO(IN, OP, IANY))
	OPINFO1(CALLH,   "callh",    4,   TRUE,  NONE, NONE, ALL,  PINFO(IN, OP, HAND))
	OPINFO0(RET,     "ret",      4,   TRUE,  NONE, NONE, ALL)
	OPINFO2(CALLC,   "callc",    4,   TRUE,  NONE, NONE, ALL,  PINFO(IN, OP, CFUNC), PINFO(IN, OP, PTR))
	OPINFO2(RECOVER, "recover",  4,   FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, MVAR))

	/* Internal Register Operations */
	OPINFO1(SETFMOD, "setfmod",  4,   FALSE, NONE, NONE, ALL,  PINFO(IN, OP, IANY))
	OPINFO1(GETFMOD, "getfmod",  4,   FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, IRM))
	OPINFO1(GETEXP,  "getexp",   4,   FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, IRM))
	OPINFO2(GETFLGS, "getflgs",  4,   FALSE, P2,   NONE, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IMV))
	OPINFO1(SAVE,    "save",     4,   FALSE, ALL,  NONE, ALL,  PINFO(OUT, OP, STATE))
	OPINFO1(RESTORE, "restore",  4,   FALSE, NONE, ALL,  ALL,  PINFO(IN, OP, STATE))

	/* Integer Operations */
	OPINFO4(LOAD,    "!load",    4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, PTR), PINFO(IN, 4, IANY), PINFO(IN, OP, SIZE))
	OPINFO4(LOADS,   "!loads",   4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, PTR), PINFO(IN, 4, IANY), PINFO(IN, OP, SIZE))
	OPINFO4(STORE,   "!store",   4|8, FALSE, NONE, NONE, ALL,  PINFO(IN, OP, PTR), PINFO(IN, 4, IANY), PINFO(IN, OP, IRM), PINFO(IN, OP, SIZE))
	OPINFO3(READ,    "!read",    4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, 4, IANY), PINFO(IN, OP, SPSZ))
	OPINFO4(READM,   "!readm",   4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, 4, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, SPSZ))
	OPINFO3(WRITE,   "!write",   4|8, FALSE, NONE, NONE, ALL,  PINFO(IN, 4, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, SPSZ))
	OPINFO4(WRITEM,  "!writem",  4|8, FALSE, NONE, NONE, ALL,  PINFO(IN, 4, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, SPSZ))
	OPINFO2(CARRY,   "!carry",   4|8, FALSE, C,    C,    ALL,  PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO2(MOV,     "!mov",     4|8, TRUE,  NONE, NONE, NONE, PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY))
	OPINFO1(SET,     "!set",     4|8, TRUE,  NONE, NONE, ALL,  PINFO(OUT, OP, IRM))
	OPINFO3(SEXT,    "!sext",    4|8, FALSE, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, P3, IANY), PINFO(IN, OP, SIZE))
	OPINFO4(ROLAND,  "!roland",  4|8, FALSE, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO4(ROLINS,  "!rolins",  4|8, FALSE, NONE, SZ,   ALL,  PINFO(INOUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(ADD,     "!add",     4|8, FALSE, NONE, SZVC, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(ADDC,    "!addc",    4|8, FALSE, C,    SZVC, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(SUB,     "!sub",     4|8, FALSE, NONE, SZVC, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(SUBB,    "!subb",    4|8, FALSE, C,    SZVC, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO2(CMP,     "!cmp",     4|8, FALSE, NONE, SZVC, ALL,  PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO4(MULU,    "!mulu",    4|8, FALSE, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO4(MULS,    "!muls",    4|8, FALSE, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO4(DIVU,    "!divu",    4|8, FALSE, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO4(DIVS,    "!divs",    4|8, FALSE, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(AND,     "!and",     4|8, FALSE, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO2(TEST,    "!test",    4|8, FALSE, NONE, SZ,   ALL,  PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(OR,      "!or",      4|8, FALSE, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(XOR,     "!xor",     4|8, FALSE, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO2(LZCNT,   "!lzcnt",   4|8, FALSE, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY))
	OPINFO2(BSWAP,   "!bswap",   4|8, FALSE, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY))
	OPINFO3(SHL,     "!shl",     4|8, FALSE, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(SHR,     "!shr",     4|8, FALSE, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(SAR,     "!sar",     4|8, FALSE, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(ROL,     "!rol",     4|8, FALSE, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(ROLC,    "!rolc",    4|8, FALSE, C,    SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(ROR,     "!ror",     4|8, FALSE, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(RORC,    "!rorc",    4|8, FALSE, C,    SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))

	/* Floating Point Operations */
	OPINFO3(FLOAD,   "f#load",   4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, PTR), PINFO(IN, 4, IANY))
	OPINFO3(FSTORE,  "f#store",  4|8, FALSE, NONE, NONE, ALL,  PINFO(IN, OP, PTR), PINFO(IN, 4, IANY), PINFO(IN, OP, FRM))
	OPINFO3(FREAD,   "f#read",   4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, 4, IANY), PINFO(IN, OP, SPACE))
	OPINFO3(FWRITE,  "f#write",  4|8, FALSE, NONE, NONE, ALL,  PINFO(IN, 4, IANY), PINFO(IN, OP, FANY), PINFO(IN, OP, SPACE))
	OPINFO2(FMOV,    "f#mov",    4|8, TRUE,  NONE, NONE, NONE, PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO4(FTOINT,  "f#toint",  4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, P3, IRM), PINFO(IN, OP, FANY), PINFO(IN, OP, SIZE), PINFO(IN, OP, FMOD))
	OPINFO3(FFRINT,  "f#frint",  4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, P3, IANY), PINFO(IN, OP, SIZE))
	OPINFO3(FFRFLT,  "f#frflt",  4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, P3, FANY), PINFO(IN, OP, SIZE))
	OPINFO2(FRNDS,   "f#rnds",     8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, P3, FANY))
	OPINFO3(FADD,    "f#add",    4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO3(FSUB,    "f#sub",    4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO2(FCMP,    "f#cmp",    4|8, FALSE, NONE, UZC,  ALL,  PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO3(FMUL,    "f#mul",    4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO3(FDIV,    "f#div",    4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO2(FNEG,    "f#neg",    4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO2(FABS,    "f#abs",    4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO2(FSQRT,   "f#sqrt",   4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO2(FRECIP,  "f#recip",  4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO2(FRSQRT,  "f#rsqrt",  4|8, FALSE, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
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
static const drcuml_opcode_info *opcode_info_table[DRCUML_OP_MAX];



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void validate_instruction(drcuml_block *block, const drcuml_instruction *inst);

#if 0
static void validate_backend(drcuml_state *drcuml);
static void bevalidate_iterate_over_params(drcuml_state *drcuml, drcuml_codehandle **handles, const bevalidate_test *test, drcuml_parameter *paramlist, int pnum);
static void bevalidate_iterate_over_flags(drcuml_state *drcuml, drcuml_codehandle **handles, const bevalidate_test *test, drcuml_parameter *paramlist);
static void bevalidate_execute(drcuml_state *drcuml, drcuml_codehandle **handles, const bevalidate_test *test, const drcuml_parameter *paramlist, UINT8 flagmask);
static void bevalidate_initialize_random_state(drcuml_block *block, drcuml_machine_state *state);
static int bevalidate_populate_state(drcuml_block *block, drcuml_machine_state *state, const bevalidate_test *test, const drcuml_parameter *paramlist, drcuml_parameter *params, UINT64 *parammem);
static int bevalidate_verify_state(drcuml_state *drcuml, const drcuml_machine_state *istate, drcuml_machine_state *state, const bevalidate_test *test, const drcuml_parameter *params, const drcuml_instruction *testinst, drccodeptr codestart, drccodeptr codeend, UINT8 flagmask);
#endif



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    effective_inflags - return the effective
    input flags based on any conditions encoded
    in an instruction
-------------------------------------------------*/

INLINE UINT8 effective_inflags(const drcuml_instruction *inst, const drcuml_opcode_info *opinfo)
{
	static const UINT8 flags_for_condition[] =
	{
		/* DRCUML_COND_Z */		DRCUML_FLAG_Z,
		/* DRCUML_COND_NZ */	DRCUML_FLAG_Z,
		/* DRCUML_COND_S */		DRCUML_FLAG_S,
		/* DRCUML_COND_NS */	DRCUML_FLAG_S,
		/* DRCUML_COND_C */		DRCUML_FLAG_C,
		/* DRCUML_COND_NC */	DRCUML_FLAG_C,
		/* DRCUML_COND_V */		DRCUML_FLAG_V,
		/* DRCUML_COND_NV */	DRCUML_FLAG_V,
		/* DRCUML_COND_U */		DRCUML_FLAG_U,
		/* DRCUML_COND_NU */	DRCUML_FLAG_U,
		/* DRCUML_COND_A */		DRCUML_FLAG_C | DRCUML_FLAG_Z,
		/* DRCUML_COND_BE */	DRCUML_FLAG_C | DRCUML_FLAG_Z,
		/* DRCUML_COND_G */		DRCUML_FLAG_S | DRCUML_FLAG_V | DRCUML_FLAG_Z,
		/* DRCUML_COND_LE */	DRCUML_FLAG_S | DRCUML_FLAG_V | DRCUML_FLAG_Z,
		/* DRCUML_COND_L */		DRCUML_FLAG_S | DRCUML_FLAG_V,
		/* DRCUML_COND_GE */	DRCUML_FLAG_S | DRCUML_FLAG_V
	};
	UINT8 flags = opinfo->inflags;
	if (flags & 0x80)
		flags = inst->param[flags - OPFLAGS_P1].value & OPFLAGS_ALL;
	if (inst->condition != DRCUML_COND_ALWAYS)
		flags |= flags_for_condition[inst->condition & 0x0f];
	return flags;
}


/*-------------------------------------------------
    effective_outflags - return the effective
    output flags based on any conditions encoded
    in an instruction
-------------------------------------------------*/

INLINE UINT8 effective_outflags(const drcuml_instruction *inst, const drcuml_opcode_info *opinfo)
{
	UINT8 flags = opinfo->outflags;
	if (flags & 0x80)
		flags = inst->param[flags - OPFLAGS_P1].value & OPFLAGS_ALL;
	return flags;
}


/*-------------------------------------------------
    effective_psize - return the effective
    parameter size based on the instruction and
    parameters passed
-------------------------------------------------*/

INLINE UINT8 effective_psize(const drcuml_instruction *inst, const drcuml_opcode_info *opinfo, int pnum)
{
	switch (opinfo->param[pnum].size)
	{
		case PSIZE_4:	return 4;
		case PSIZE_8:	return 8;
		case PSIZE_OP:	return inst->size;
		case PSIZE_P1:	assert(inst->param[0].type == DRCUML_PTYPE_IMMEDIATE); return 1 << (inst->param[0].value & 3);
		case PSIZE_P2:	assert(inst->param[1].type == DRCUML_PTYPE_IMMEDIATE); return 1 << (inst->param[1].value & 3);
		case PSIZE_P3:	assert(inst->param[2].type == DRCUML_PTYPE_IMMEDIATE); return 1 << (inst->param[2].value & 3);
		case PSIZE_P4:	assert(inst->param[3].type == DRCUML_PTYPE_IMMEDIATE); return 1 << (inst->param[3].value & 3);
	}
	return inst->size;
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
	for (opnum = 0; opnum < ARRAY_LENGTH(opcode_info_source); opnum++)
		opcode_info_table[opcode_info_source[opnum].opcode] = &opcode_info_source[opnum];

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
/*	if (VALIDATE_BACKEND)
	{
		static int validated = FALSE;
		if (!validated)
		{
			validated = TRUE;
			validate_backend(drcuml);
		}
	}*/
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

void drcuml_block_append_0(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition, UINT8 flags)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condition = condition;
	inst->flags = flags;
	inst->numparams = 0;

	/* validation */
	validate_instruction(block, inst);
}


/*-------------------------------------------------
    drcuml_block_append_1 - append an opcode with
    1 parameter to the block
-------------------------------------------------*/

void drcuml_block_append_1(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition, UINT8 flags, drcuml_ptype p0type, drcuml_pvalue p0value)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condition = condition;
	inst->flags = flags;
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

void drcuml_block_append_2(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition, UINT8 flags, drcuml_ptype p0type, drcuml_pvalue p0value, drcuml_ptype p1type, drcuml_pvalue p1value)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condition = condition;
	inst->flags = flags;
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

void drcuml_block_append_3(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition, UINT8 flags, drcuml_ptype p0type, drcuml_pvalue p0value, drcuml_ptype p1type, drcuml_pvalue p1value, drcuml_ptype p2type, drcuml_pvalue p2value)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condition = condition;
	inst->flags = flags;
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

void drcuml_block_append_4(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition, UINT8 flags, drcuml_ptype p0type, drcuml_pvalue p0value, drcuml_ptype p1type, drcuml_pvalue p1value, drcuml_ptype p2type, drcuml_pvalue p2value, drcuml_ptype p3type, drcuml_pvalue p3value)
{
	drcuml_instruction *inst = &block->inst[block->nextinst++];

	assert(block->inuse);

	/* get a pointer to the next instruction */
	if (block->nextinst > block->maxinst)
		fatalerror("Overran maxinst in drcuml_block_append");

	/* fill in the instruction */
	inst->opcode = (UINT8)op;
	inst->size = size;
	inst->condition = condition;
	inst->flags = flags;
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
			char dasm[256];
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
	drcuml_block_append_1(block, DRCUML_OP_COMMENT, 4, DRCUML_COND_ALWAYS, 0, MEM(comment));
}


/*-------------------------------------------------
    drcuml_disasm - disassemble an
    instruction to the given buffer
-------------------------------------------------*/

void drcuml_disasm(const drcuml_instruction *inst, char *buffer)
{
	static const char *conditions[] = { "z", "nz", "s", "ns", "c", "nc", "v", "nv", "u", "nu", "a", "be", "g", "le", "l", "ge" };
	static const char *pound_size[] = { "?", "?", "?", "?", "s", "?", "?", "?", "d" };
	static const char *bang_size[] = { "?", "b", "h", "?", "", "?", "?", "?", "d" };
	static const char *fmods[] = { "trunc", "round", "ceil", "floor", "default" };
	static const char *sizes[] = { "byte", "word", "dword", "qword" };
	const drcuml_opcode_info *opinfo = opcode_info_table[inst->opcode];
	char *dest = buffer;
	const char *opsrc;
	int pnum;

	assert(inst->opcode != DRCUML_OP_INVALID && inst->opcode < DRCUML_OP_MAX);
	
	/* start with the raw mnemonic and substitute sizes */
	for (opsrc = opinfo->mnemonic; *opsrc != 0; opsrc++)
		if (*opsrc == '!')
			dest += sprintf(dest, "%s", bang_size[inst->size]);
		else if (*opsrc == '#')
			dest += sprintf(dest, "%s", pound_size[inst->size]);
		else
			*dest++ = *opsrc;
	
	/* pad to 8 spaces */
	while (dest < &buffer[8])
		*dest++ = ' ';
	
	/* iterate through parameters */
	for (pnum = 0; pnum < inst->numparams; pnum++)
	{
		const drcuml_parameter *param = &inst->param[pnum];
		UINT16 typemask = opinfo->param[pnum].typemask;

		/* start with a comma for all except the first parameter */
		if (pnum != 0)
			*dest++ = ',';
		
		/* ouput based on type */
		switch (param->type)
		{
			/* immediates have several special cases */
			case DRCUML_PTYPE_IMMEDIATE:
			
				/* size immediate */
				if (typemask == PTYPES_SIZE)
					dest += sprintf(dest, "%s", sizes[param->value]);
				
				/* address space immediate */
				else if (typemask == PTYPES_SPACE)
					dest += sprintf(dest, "%s", address_space_names[param->value]);
				
				/* size + address space immediate */
				else if (typemask == PTYPES_SPSZ)
					dest += sprintf(dest, "%s_%s", address_space_names[param->value / 16], sizes[param->value % 16]);
				
				/* fmod immediate */
				else if (typemask == PTYPES_FMOD)
					dest += sprintf(dest, "%s", fmods[param->value]);
				
				/* general immediate */
				else
				{
					int size = effective_psize(inst, opinfo, pnum);
					UINT64 value = param->value;
					
					/* truncate to size */
					if (size == 1) value = (UINT8)value;
					if (size == 2) value = (UINT16)value;
					if (size == 4) value = (UINT32)value;
					if ((UINT32)value == value)
						dest += sprintf(dest, "$%X", (UINT32)value);
					else
						dest += sprintf(dest, "$%X%08X", (UINT32)(value >> 32), (UINT32)value);
				}
				break;
			
			/* integer registers */
			case DRCUML_PTYPE_INT_REGISTER:
				if (param->value >= DRCUML_REG_I0 && param->value < DRCUML_REG_I_END)
					dest += sprintf(dest, "i%d", (UINT32)(param->value - DRCUML_REG_I0));
				else
					dest += sprintf(dest, "i(%X?)", (UINT32)param->value);
				break;

			/* floating point registers */
			case DRCUML_PTYPE_FLOAT_REGISTER:
				if (param->value >= DRCUML_REG_F0 && param->value < DRCUML_REG_F_END)
					dest += sprintf(dest, "f%d", (UINT32)(param->value - DRCUML_REG_F0));
				else
					dest += sprintf(dest, "f(%X?)", (UINT32)param->value);
				break;

			/* map variables */
			case DRCUML_PTYPE_MAPVAR:
				if (param->value >= DRCUML_MAPVAR_M0 && param->value < DRCUML_MAPVAR_END)
					dest += sprintf(dest, "m%d", (UINT32)(param->value - DRCUML_MAPVAR_M0));
				else
					dest += sprintf(dest, "m(%X?)", (UINT32)param->value);
				break;

			/* memory */
			case DRCUML_PTYPE_MEMORY:
			
				/* handle pointer */
				if (typemask == PTYPES_HAND)
					dest += sprintf(dest, "%s", drcuml_handle_name((const drcuml_codehandle *)(FPTR)param->value));
				
				/* string pointer */
				else if (typemask == PTYPES_STR)
					dest += sprintf(dest, "%s", (const char *)(FPTR)param->value);
				
				/* general memory */
				else
					dest += sprintf(dest, "[$%p]", (void *)(FPTR)param->value);
				break;
			
			default:
				dest += sprintf(dest, "???");
				break;
		}
	}
	
	/* if there's a condition, append it */
	if (inst->condition != DRCUML_COND_ALWAYS)
		dest += sprintf(dest, ",%s", conditions[inst->condition & 0x0f]);
	
	/* if there are flags, append them */
	if (inst->flags != 0)
	{
		*dest++ = ',';
		if (inst->flags & DRCUML_FLAG_U)
			*dest++ = 'U';
		if (inst->flags & DRCUML_FLAG_S)
			*dest++ = 'S';
		if (inst->flags & DRCUML_FLAG_Z)
			*dest++ = 'Z';
		if (inst->flags & DRCUML_FLAG_V)
			*dest++ = 'V';
		if (inst->flags & DRCUML_FLAG_C)
			*dest++ = 'C';
	}
	
	/* ensure we are NULL-terminated */
	*dest = 0;
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
	const drcuml_opcode_info *opinfo = opcode_info_table[inst->opcode];
	int pnum;

	/* validate raw information */
	assert(inst->opcode != DRCUML_OP_INVALID && inst->opcode < DRCUML_OP_MAX);
	assert(inst->size == 1 || inst->size == 2 || inst->size == 4 || inst->size == 8);
	
	/* validate against opcode limits */
	assert((opinfo->sizes & inst->size) != 0);
	assert(inst->condition == DRCUML_COND_ALWAYS || opinfo->condition);
	
	/* validate each parameter */
	for (pnum = 0; pnum < inst->numparams; pnum++)
	{
		const drcuml_parameter *param = &inst->param[pnum];
		UINT16 typemask = opinfo->param[pnum].typemask;
		
		/* ensure the type is correct */
		assert(param->type > DRCUML_PTYPE_NONE && param->type < DRCUML_PTYPE_MAX);
		assert((typemask >> param->type) & 1);
		
		/* most memory parameters must be in the near cache */
		if (param->type == DRCUML_PTYPE_MEMORY && typemask != PTYPES_PTR && typemask != PTYPES_STATE && typemask != PTYPES_STR && typemask != PTYPES_CFUNC)
			assert_in_near_cache(block->drcuml->cache, (void *)(FPTR)param->value);
		
		/* validate immediate parameters */
		if (param->type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (typemask == PTYPES_SIZE)
				assert(param->value >= DRCUML_SIZE_BYTE && param->value <= DRCUML_SIZE_QWORD);
			else if (typemask == PTYPES_SPACE)
				assert(param->value >= ADDRESS_SPACE_PROGRAM && param->value <= ADDRESS_SPACE_IO);
			else if (typemask == PTYPES_SPSZ)
			{
				assert(param->value % 16 >= DRCUML_SIZE_BYTE && param->value % 16 <= DRCUML_SIZE_QWORD);
				assert(param->value / 16 >= ADDRESS_SPACE_PROGRAM && param->value / 16 <= ADDRESS_SPACE_IO);
			}
			else if (typemask == PTYPES_FMOD)
				assert(param->value >= DRCUML_FMOD_TRUNC && param->value <= DRCUML_FMOD_DEFAULT);
		}
	}

	/* make sure we aren't missing any parameters */
	if (inst->numparams < ARRAY_LENGTH(opinfo->param))
		assert(opinfo->param[inst->numparams].typemask == 0);
}



#if 0

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
	const drcuml_opcode_info *opinfo = opcode_info_table[test->opcode];

	/* if no parameters, execute now */
	if (pnum >= ARRAY_LENGTH(opinfo->ptypes) || opinfo->ptypes[pnum] == OV_PARAM_ALLOWED_NONE)
	{
		bevalidate_iterate_over_flags(drcuml, handles, test, paramlist);
		return;
	}

	/* iterate over valid parameter types */
	for (paramlist[pnum].type = DRCUML_PTYPE_IMMEDIATE; paramlist[pnum].type < DRCUML_PTYPE_MAX; paramlist[pnum].type++)
		if (opinfo->ptypes[pnum] & (1 << paramlist[pnum].type))
		{
			int pindex, pcount;

			/* mapvars can only do 32-bit tests */
			if (paramlist[pnum].type == DRCUML_PTYPE_MAPVAR && size_for_param(test->size, opinfo->ptypes[pnum]) == 8)
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
	const drcuml_opcode_info *opinfo = opcode_info_table[test->opcode];
	UINT8 flagmask = opinfo->flags & 0x1f;
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
			drcuml_block_append_0(block, test->opcode, test->size, DRCUML_COND_ALWAYS, flagmask);
			break;

		case 1:
			drcuml_block_append_1(block, test->opcode, test->size, DRCUML_COND_ALWAYS, flagmask, params[0].type, params[0].value);
			break;

		case 2:
			drcuml_block_append_2(block, test->opcode, test->size, DRCUML_COND_ALWAYS, flagmask, params[0].type, params[0].value, params[1].type, params[1].value);
			break;

		case 3:
			drcuml_block_append_3(block, test->opcode, test->size, DRCUML_COND_ALWAYS, flagmask, params[0].type, params[0].value, params[1].type, params[1].value, params[2].type, params[2].value);
			break;

		case 4:
			drcuml_block_append_4(block, test->opcode, test->size, DRCUML_COND_ALWAYS, flagmask, params[0].type, params[0].value, params[1].type, params[1].value, params[2].type, params[2].value, params[3].type, params[3].value);
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
	const drcuml_opcode_info *opinfo = opcode_info_table[test->opcode];
	int numparams = ARRAY_LENGTH(test->param);
	int pnum;

	/* copy flags as-is */
	state->flags = test->iflags;

	/* iterate over parameters */
	for (pnum = 0; pnum < ARRAY_LENGTH(test->param); pnum++)
	{
		int psize = size_for_param(test->size, opinfo->ptypes[pnum]);
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
	const drcuml_opcode_info *opinfo = opcode_info_table[test->opcode];
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
			UINT64 mask = mask_for_param(test->size, opinfo->ptypes[pnum]);
			int psize = size_for_param(test->size, opinfo->ptypes[pnum]);
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
		char disasm[256];

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

#endif
