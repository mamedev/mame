/***************************************************************************

    drcbec.c

    Interpreted C core back-end for the universal machine language.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "drcuml.h"
#include "drcbeut.h"

#ifdef _MSC_VER
#include <float.h>
#define isnan _isnan
#endif


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* define a bit to match each possible condition, starting at bit 12 */
#define ZBIT			(0x1000 << (DRCUML_COND_Z & 15))
#define NZBIT			(0x1000 << (DRCUML_COND_NZ & 15))
#define SBIT			(0x1000 << (DRCUML_COND_S & 15))
#define NSBIT			(0x1000 << (DRCUML_COND_NS & 15))
#define CBIT			(0x1000 << (DRCUML_COND_C & 15))
#define NCBIT			(0x1000 << (DRCUML_COND_NC & 15))
#define VBIT			(0x1000 << (DRCUML_COND_V & 15))
#define NVBIT			(0x1000 << (DRCUML_COND_NV & 15))
#define UBIT			(0x1000 << (DRCUML_COND_U & 15))
#define NUBIT			(0x1000 << (DRCUML_COND_NU & 15))
#define ABIT			(0x1000 << (DRCUML_COND_A & 15))
#define BEBIT			(0x1000 << (DRCUML_COND_BE & 15))
#define GBIT			(0x1000 << (DRCUML_COND_G & 15))
#define GEBIT			(0x1000 << (DRCUML_COND_GE & 15))
#define LBIT			(0x1000 << (DRCUML_COND_L & 15))
#define LEBIT			(0x1000 << (DRCUML_COND_LE & 15))


/* internal opcodes */
enum
{
	DRCUML_OP_LOAD1 = DRCUML_OP_MAX,
	DRCUML_OP_LOAD1x2,
	DRCUML_OP_LOAD1x4,
	DRCUML_OP_LOAD1x8,
	DRCUML_OP_LOAD2x1,
	DRCUML_OP_LOAD2,
	DRCUML_OP_LOAD2x4,
	DRCUML_OP_LOAD2x8,
	DRCUML_OP_LOAD4x1,
	DRCUML_OP_LOAD4x2,
	DRCUML_OP_LOAD4,
	DRCUML_OP_LOAD4x8,
	DRCUML_OP_LOAD8x1,
	DRCUML_OP_LOAD8x2,
	DRCUML_OP_LOAD8x4,
	DRCUML_OP_LOAD8,
	DRCUML_OP_LOADS1,
	DRCUML_OP_LOADS1x2,
	DRCUML_OP_LOADS1x4,
	DRCUML_OP_LOADS1x8,
	DRCUML_OP_LOADS2x1,
	DRCUML_OP_LOADS2,
	DRCUML_OP_LOADS2x4,
	DRCUML_OP_LOADS2x8,
	DRCUML_OP_LOADS4x1,
	DRCUML_OP_LOADS4x2,
	DRCUML_OP_LOADS4,
	DRCUML_OP_LOADS4x8,
	DRCUML_OP_LOADS8x1,
	DRCUML_OP_LOADS8x2,
	DRCUML_OP_LOADS8x4,
	DRCUML_OP_LOADS8,
	DRCUML_OP_STORE1,
	DRCUML_OP_STORE1x2,
	DRCUML_OP_STORE1x4,
	DRCUML_OP_STORE1x8,
	DRCUML_OP_STORE2x1,
	DRCUML_OP_STORE2,
	DRCUML_OP_STORE2x4,
	DRCUML_OP_STORE2x8,
	DRCUML_OP_STORE4x1,
	DRCUML_OP_STORE4x2,
	DRCUML_OP_STORE4,
	DRCUML_OP_STORE4x8,
	DRCUML_OP_STORE8x1,
	DRCUML_OP_STORE8x2,
	DRCUML_OP_STORE8x4,
	DRCUML_OP_STORE8,
	DRCUML_OP_READ1,
	DRCUML_OP_READ2,
	DRCUML_OP_READ4,
	DRCUML_OP_READ8,
	DRCUML_OP_READM1,
	DRCUML_OP_READM2,
	DRCUML_OP_READM4,
	DRCUML_OP_READM8,
	DRCUML_OP_WRITE1,
	DRCUML_OP_WRITE2,
	DRCUML_OP_WRITE4,
	DRCUML_OP_WRITE8,
	DRCUML_OP_WRITEM1,
	DRCUML_OP_WRITEM2,
	DRCUML_OP_WRITEM4,
	DRCUML_OP_WRITEM8,
	DRCUML_OP_SEXT1,
	DRCUML_OP_SEXT2,
	DRCUML_OP_SEXT4,
	DRCUML_OP_SEXT8,
	DRCUML_OP_FTOI4T,
	DRCUML_OP_FTOI4R,
	DRCUML_OP_FTOI4C,
	DRCUML_OP_FTOI4F,
	DRCUML_OP_FTOI4,
	DRCUML_OP_FTOI8T,
	DRCUML_OP_FTOI8R,
	DRCUML_OP_FTOI8C,
	DRCUML_OP_FTOI8F,
	DRCUML_OP_FTOI8,
	DRCUML_OP_FFRI4,
	DRCUML_OP_FFRI8,
	DRCUML_OP_FFRFS,
	DRCUML_OP_FFRFD,
};



/***************************************************************************
    MACROS
***************************************************************************/

/* opcode format:

    bits 31..28 == number of words following the opcode itself (0-15)
    bits 27..12 == bitmask specify which condition code we care about
    bits 11.. 2 == opcode
    bit       1 == flags/condition summary (0 if no condition/flags, 1 otherwise)
    bit       0 == operation size (0=32-bit, 1=64-bit)
*/

/* build a short opcode from the raw opcode and size */
#define MAKE_OPCODE_SHORT(op, size, conditionorflags) \
	((((size) == 8) << 0) | (((conditionorflags) != 0) << 1) | ((op) << 2))

/* build a full opcode from the raw opcode, size, condition/flags, and immediate count */
#define MAKE_OPCODE_FULL(op, size, condition, flags, pwords) \
	(MAKE_OPCODE_SHORT(op, size, (condition | flags)) | ((condition != DRCUML_COND_ALWAYS) ? (0x1000 << ((condition) & 15)) : 0) | ((pwords) << 28))

/* extract various parts of the opcode */
#define OPCODE_GET_SHORT(op)		((op) & 0xfff)
#define OPCODE_PASS_CONDITION(op,f)	(((op) & condition_map[f]) != 0)
#define OPCODE_FAIL_CONDITION(op,f)	(((op) & condition_map[f]) == 0)
#define OPCODE_GET_PWORDS(op)		((op) >> 28)

/* shorthand for accessing parameters in the instruction stream */
#define PARAM0						(*inst[0].puint32)
#define PARAM1						(*inst[1].puint32)
#define PARAM2						(*inst[2].puint32)
#define PARAM3						(*inst[3].puint32)

#define DPARAM0						(*inst[0].puint64)
#define DPARAM1						(*inst[1].puint64)
#define DPARAM2						(*inst[2].puint64)
#define DPARAM3						(*inst[3].puint64)

#define FSPARAM0					(*inst[0].pfloat)
#define FSPARAM1					(*inst[1].pfloat)
#define FSPARAM2					(*inst[2].pfloat)
#define FSPARAM3					(*inst[3].pfloat)

#define FDPARAM0					(*inst[0].pdouble)
#define FDPARAM1					(*inst[1].pdouble)
#define FDPARAM2					(*inst[2].pdouble)
#define FDPARAM3					(*inst[3].pdouble)

/* compute C and V flags for 32-bit add/subtract */
#define FLAGS32_C_ADD(a,b)			((UINT32)~(a) < (UINT32)(b))
#define FLAGS32_C_SUB(a,b)			((UINT32)(b) > (UINT32)(a))
#define FLAGS32_V_SUB(r,a,b)		(((((a) ^ (b)) & ((a) ^ (r))) >> 30) & DRCUML_FLAG_V)
#define FLAGS32_V_ADD(r,a,b)		(((~((a) ^ (b)) & ((a) ^ (r))) >> 30) & DRCUML_FLAG_V)

/* compute N and Z flags for 32-bit operations */
#define FLAGS32_NZ(v)				((((v) >> 28) & DRCUML_FLAG_S) | (((UINT32)(v) == 0) << 2))
#define FLAGS32_NZCV_ADD(r,a,b)		(FLAGS32_NZ(r) | FLAGS32_C_ADD(a,b) | FLAGS32_V_ADD(r,a,b))
#define FLAGS32_NZCV_SUB(r,a,b)		(FLAGS32_NZ(r) | FLAGS32_C_SUB(a,b) | FLAGS32_V_SUB(r,a,b))

/* compute C and V flags for 64-bit add/subtract */
#define FLAGS64_C_ADD(a,b)			((UINT64)~(a) < (UINT64)(b))
#define FLAGS64_C_SUB(a,b)			((UINT64)(b) > (UINT64)(a))
#define FLAGS64_V_SUB(r,a,b)		(((((a) ^ (b)) & ((a) ^ (r))) >> 62) & DRCUML_FLAG_V)
#define FLAGS64_V_ADD(r,a,b)		(((~((a) ^ (b)) & ((a) ^ (r))) >> 62) & DRCUML_FLAG_V)

/* compute N and Z flags for 64-bit operations */
#define FLAGS64_NZ(v)				((((v) >> 60) & DRCUML_FLAG_S) | (((UINT64)(v) == 0) << 2))
#define FLAGS64_NZCV_ADD(r,a,b)		(FLAGS64_NZ(r) | FLAGS64_C_ADD(a,b) | FLAGS64_V_ADD(r,a,b))
#define FLAGS64_NZCV_SUB(r,a,b)		(FLAGS64_NZ(r) | FLAGS64_C_SUB(a,b) | FLAGS64_V_SUB(r,a,b))



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* internal backend-specific state */
struct _drcbe_state
{
	running_device *	device;					/* CPU device we are associated with */
	const address_space *	space[ADDRESS_SPACES];	/* pointers to CPU's address space */
	drcuml_state *			drcuml;					/* pointer back to our owner */
	drccache *				cache;					/* pointer to the cache */
	drcuml_machine_state	state;					/* state of the machine */
	drchash_state *			hash;					/* hash table state */
	drcmap_state *			map;					/* code map */
	drclabel_list *			labels;                 /* label list */
};


/* union to simplify accessing data via the instruction stream */
typedef union _drcbec_instruction drcbec_instruction;
union _drcbec_instruction
{
	UINT32				i;
	void *				v;
	char *				c;
	UINT8 *				puint8;
	INT8 *				pint8;
	UINT16 *			puint16;
	INT16 *				pint16;
	UINT32 *			puint32;
	INT32 *				pint32;
	UINT64 *			puint64;
	INT64 *				pint64;
	float *				pfloat;
	double *			pdouble;
	void				(*cfunc)(void *);
	drcuml_machine_state *state;
	const drcuml_codehandle *handle;
	const drcbec_instruction *inst;
	const drcbec_instruction **pinst;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* primary back-end callbacks */
static drcbe_state *drcbec_alloc(drcuml_state *drcuml, drccache *cache, running_device *device, UINT32 flags, int modes, int addrbits, int ignorebits);
static void drcbec_free(drcbe_state *drcbe);
static void drcbec_reset(drcbe_state *drcbe);
static int drcbec_execute(drcbe_state *state, drcuml_codehandle *entry);
static void drcbec_generate(drcbe_state *drcbe, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst);
static int drcbec_hash_exists(drcbe_state *state, UINT32 mode, UINT32 pc);
static void drcbec_get_info(drcbe_state *state, drcbe_info *info);

/* private helper functions */
static void output_parameter(drcbe_state *drcbe, drcbec_instruction **dstptr, void **immedptr, int size, const drcuml_parameter *param);
static void fixup_label(void *parameter, drccodeptr labelcodeptr);
static int dmulu(UINT64 *dstlo, UINT64 *dsthi, UINT64 src1, UINT64 src2, int flags);
static int dmuls(UINT64 *dstlo, UINT64 *dsthi, INT64 src1, INT64 src2, int flags);



/***************************************************************************
    TABLES
***************************************************************************/

static const UINT32 condition_map[] =
{
	/* ..... */		NCBIT | NVBIT | NZBIT | NSBIT | NUBIT | ABIT  | GBIT  | GEBIT,
	/* ....C */		CBIT  | NVBIT | NZBIT | NSBIT | NUBIT | BEBIT | GBIT  | GEBIT,
	/* ...V. */		NCBIT | VBIT  | NZBIT | NSBIT | NUBIT | ABIT  | LEBIT | LBIT,
	/* ...VC */		CBIT  | VBIT  | NZBIT | NSBIT | NUBIT | BEBIT | LEBIT | LBIT,
	/* ..Z.. */		NCBIT | NVBIT | ZBIT  | NSBIT | NUBIT | BEBIT | LEBIT | GEBIT,
	/* ..Z.C */		CBIT  | NVBIT | ZBIT  | NSBIT | NUBIT | BEBIT | LEBIT | GEBIT,
	/* ..ZV. */		NCBIT | VBIT  | ZBIT  | NSBIT | NUBIT | BEBIT | LEBIT | LBIT,
	/* ..ZVC */		CBIT  | VBIT  | ZBIT  | NSBIT | NUBIT | BEBIT | LEBIT | LBIT,
	/* .S... */		NCBIT | NVBIT | NZBIT | SBIT  | NUBIT | ABIT  | LEBIT | LBIT,
	/* .S..C */		CBIT  | NVBIT | NZBIT | SBIT  | NUBIT | BEBIT | LEBIT | LBIT,
	/* .S.V. */		NCBIT | VBIT  | NZBIT | SBIT  | NUBIT | ABIT  | GBIT  | GEBIT,
	/* .S.VC */		CBIT  | VBIT  | NZBIT | SBIT  | NUBIT | BEBIT | GBIT  | GEBIT,
	/* .SZ.. */		NCBIT | NVBIT | ZBIT  | SBIT  | NUBIT | BEBIT | LEBIT | LBIT,
	/* .SZ.C */		CBIT  | NVBIT | ZBIT  | SBIT  | NUBIT | BEBIT | LEBIT | LBIT,
	/* .SZV. */		NCBIT | VBIT  | ZBIT  | SBIT  | NUBIT | BEBIT | LEBIT | GEBIT,
	/* .SZVC */		CBIT  | VBIT  | ZBIT  | SBIT  | NUBIT | BEBIT | LEBIT | GEBIT,
	/* U.... */		NCBIT | NVBIT | NZBIT | NSBIT | UBIT  | ABIT  | GBIT  | GEBIT,
	/* U...C */		CBIT  | NVBIT | NZBIT | NSBIT | UBIT  | BEBIT | GBIT  | GEBIT,
	/* U..V. */		NCBIT | VBIT  | NZBIT | NSBIT | UBIT  | ABIT  | LEBIT | LBIT,
	/* U..VC */		CBIT  | VBIT  | NZBIT | NSBIT | UBIT  | BEBIT | LEBIT | LBIT,
	/* U.Z.. */		NCBIT | NVBIT | ZBIT  | NSBIT | UBIT  | BEBIT | LEBIT | GEBIT,
	/* U.Z.C */		CBIT  | NVBIT | ZBIT  | NSBIT | UBIT  | BEBIT | LEBIT | GEBIT,
	/* U.ZV. */		NCBIT | VBIT  | ZBIT  | NSBIT | UBIT  | BEBIT | LEBIT | LBIT,
	/* U.ZVC */		CBIT  | VBIT  | ZBIT  | NSBIT | UBIT  | BEBIT | LEBIT | LBIT,
	/* US... */		NCBIT | NVBIT | NZBIT | SBIT  | UBIT  | ABIT  | LEBIT | LBIT,
	/* US..C */		CBIT  | NVBIT | NZBIT | SBIT  | UBIT  | BEBIT | LEBIT | LBIT,
	/* US.V. */		NCBIT | VBIT  | NZBIT | SBIT  | UBIT  | ABIT  | GBIT  | GEBIT,
	/* US.VC */		CBIT  | VBIT  | NZBIT | SBIT  | UBIT  | BEBIT | GBIT  | GEBIT,
	/* USZ.. */		NCBIT | NVBIT | ZBIT  | SBIT  | UBIT  | BEBIT | LEBIT | LBIT,
	/* USZ.C */		CBIT  | NVBIT | ZBIT  | SBIT  | UBIT  | BEBIT | LEBIT | LBIT,
	/* USZV. */		NCBIT | VBIT  | ZBIT  | SBIT  | UBIT  | BEBIT | LEBIT | GEBIT,
	/* USZVC */		CBIT  | VBIT  | ZBIT  | SBIT  | UBIT  | BEBIT | LEBIT | GEBIT
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT64 immediate_zero = 0;

extern const drcbe_interface drcbe_c_be_interface =
{
	drcbec_alloc,
	drcbec_free,
	drcbec_reset,
	drcbec_execute,
	drcbec_generate,
	drcbec_hash_exists,
	drcbec_get_info
};



/***************************************************************************
    BACKEND CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    drcbec_alloc - allocate back-end-specific
    state
-------------------------------------------------*/

static drcbe_state *drcbec_alloc(drcuml_state *drcuml, drccache *cache, running_device *device, UINT32 flags, int modes, int addrbits, int ignorebits)
{
	int spacenum;

	/* allocate space in the cache for our state */
	drcbe_state *drcbe = (drcbe_state *)drccache_memory_alloc(cache, sizeof(*drcbe));
	if (drcbe == NULL)
		return NULL;
	memset(drcbe, 0, sizeof(*drcbe));

	/* remember our pointers */
	drcbe->device = device;
	for (spacenum = 0; spacenum < ARRAY_LENGTH(drcbe->space); spacenum++)
		drcbe->space[spacenum] = device->space(spacenum);
	drcbe->drcuml = drcuml;
	drcbe->cache = cache;

	/* allocate hash tables */
	drcbe->hash = drchash_alloc(cache, modes, addrbits, ignorebits);
	if (drcbe->hash == NULL)
		return NULL;

	/* allocate code map */
	drcbe->map = drcmap_alloc(cache, 0);
	if (drcbe->map == NULL)
		return NULL;

	/* allocate a label tracker */
	drcbe->labels = drclabel_list_alloc(cache);
	if (drcbe->labels == NULL)
		return NULL;

	return drcbe;
}


/*-------------------------------------------------
    drcbec_free - free back-end specific state
-------------------------------------------------*/

static void drcbec_free(drcbe_state *drcbe)
{
}


/*-------------------------------------------------
    drcbec_reset - reset back-end specific state
-------------------------------------------------*/

static void drcbec_reset(drcbe_state *drcbe)
{
	/* reset our hash tables */
	drchash_reset(drcbe->hash);
	drchash_set_default_codeptr(drcbe->hash, NULL);
}


/*-------------------------------------------------
    drcbec_generate - generate code
-------------------------------------------------*/

static void drcbec_generate(drcbe_state *drcbe, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst)
{
	drccodeptr *cachetop;
	drcbec_instruction *base;
	drcbec_instruction *dst;
	int inum;

	/* tell all of our utility objects that a block is beginning */
	drchash_block_begin(drcbe->hash, block, instlist, numinst);
	drclabel_block_begin(drcbe->labels, block);
	drcmap_block_begin(drcbe->map, block);

	/* begin codegen; fail if we can't */
	cachetop = drccache_begin_codegen(drcbe->cache, numinst * sizeof(drcbec_instruction) * 4);
	if (cachetop == NULL)
		drcuml_block_abort(block);

	/* compute the base by aligning the cache top to an even multiple of drcbec_instruction */
	base = (drcbec_instruction *)(((FPTR)*cachetop + sizeof(drcbec_instruction) - 1) & ~(sizeof(drcbec_instruction) - 1));
	dst = base;

	/* generate code by copying the instructions and extracting immediates */
	for (inum = 0; inum < numinst; inum++)
	{
		const drcuml_instruction *inst = &instlist[inum];
		UINT8 psize[ARRAY_LENGTH(instlist->param)];
		drcuml_instruction modified_inst;
		int immedbytes, immedwords, pnum;
		drcuml_opcode opcode = inst->opcode;
		void *immed;

		/* handle most instructions generally, but a few special cases */
		switch (opcode)
		{
			/* when we hit a HANDLE opcode, register the current pointer for the handle */
			case DRCUML_OP_HANDLE:
				assert(inst->numparams == 1);
				assert(inst->param[0].type == DRCUML_PTYPE_MEMORY);
				drcuml_handle_set_codeptr((drcuml_codehandle *)(FPTR)inst->param[0].value, (drccodeptr)dst);
				break;

			/* when we hit a HASH opcode, register the current pointer for the mode/PC */
			case DRCUML_OP_HASH:
				/* we already verified the parameter count and types above */
				drchash_set_codeptr(drcbe->hash, inst->param[0].value, inst->param[1].value, (drccodeptr)dst);
				break;

			/* when we hit a LABEL opcode, register the current pointer for the label */
			case DRCUML_OP_LABEL:
				assert(inst->numparams == 1);
				assert(inst->param[0].type == DRCUML_PTYPE_IMMEDIATE);
				drclabel_set_codeptr(drcbe->labels, inst->param[0].value, (drccodeptr)dst);
				break;

			/* ignore COMMENT and NOP opcodes */
			case DRCUML_OP_COMMENT:
			case DRCUML_OP_NOP:
				break;

			/* when we hit a MAPVAR opcode, log the change for the current PC */
			case DRCUML_OP_MAPVAR:
				assert(inst->numparams == 2);
				assert(inst->param[0].type == DRCUML_PTYPE_MAPVAR);
				assert(inst->param[1].type == DRCUML_PTYPE_IMMEDIATE);
				drcmap_set_value(drcbe->map, (drccodeptr)dst, inst->param[0].value, inst->param[1].value);
				break;

			/* JMP instructions need to resolve their labels */
			case DRCUML_OP_JMP:
				assert(inst->numparams == 1);
				assert(inst->param[0].type == DRCUML_PTYPE_IMMEDIATE);
				(dst++)->i = MAKE_OPCODE_FULL(opcode, inst->size, inst->condition, inst->flags, 1);
				dst->inst = (drcbec_instruction *)drclabel_get_codeptr(drcbe->labels, inst->param[0].value, fixup_label, dst);
				dst++;
				break;

			/* generically handle everything else */
			default:

				/* for RECOVER opcodes, we need to fixup the second parameter into an immediate */
				if (opcode == DRCUML_OP_RECOVER)
				{
					assert(inst->param[1].type == DRCUML_PTYPE_MAPVAR);
					modified_inst = *inst;
					modified_inst.param[1].type = DRCUML_PTYPE_IMMEDIATE;
					inst = &modified_inst;
				}

				/* determine the operand size for each operand; mostly this is just the instruction size */
				for (pnum = 0; pnum < inst->numparams; pnum++)
					psize[pnum] = inst->size;
				if (opcode == DRCUML_OP_LOAD || opcode == DRCUML_OP_FLOAD)
					psize[2] = 4;
				if (opcode == DRCUML_OP_STORE || opcode == DRCUML_OP_FSTORE)
					psize[1] = 4;
				if (opcode == DRCUML_OP_READ || opcode == DRCUML_OP_READM || opcode == DRCUML_OP_FREAD)
					psize[0] = psize[1] = 4;
				if (opcode == DRCUML_OP_WRITE || opcode == DRCUML_OP_WRITEM || opcode == DRCUML_OP_FWRITE)
					psize[0] = psize[1] = 4;
				if (opcode == DRCUML_OP_SEXT && inst->param[2].value != DRCUML_SIZE_QWORD)
					psize[1] = 4;
				if (opcode == DRCUML_OP_FTOINT)
					psize[0] = 1 << inst->param[2].value;
				if (opcode == DRCUML_OP_FFRINT || opcode == DRCUML_OP_FFRFLT)
					psize[1] = 1 << inst->param[2].value;

				/* pre-expand opcodes that encode size/scale in them */
				if (opcode == DRCUML_OP_LOAD)
					opcode = (drcuml_opcode)(DRCUML_OP_LOAD1 + (inst->param[3].value & 3) * 4 + ((inst->param[3].value >> 4) & 3));
				if (opcode == DRCUML_OP_LOADS)
					opcode = (drcuml_opcode)(DRCUML_OP_LOADS1 + (inst->param[3].value & 3) * 4 + ((inst->param[3].value >> 4) & 3));
				if (opcode == DRCUML_OP_STORE)
					opcode = (drcuml_opcode)(DRCUML_OP_STORE1 + (inst->param[3].value & 3) * 4 + ((inst->param[3].value >> 4) & 3));
				if (opcode == DRCUML_OP_READ)
					opcode = (drcuml_opcode)(DRCUML_OP_READ1 + (inst->param[2].value & 3));
				if (opcode == DRCUML_OP_READM)
					opcode = (drcuml_opcode)(DRCUML_OP_READM1 + (inst->param[3].value & 3));
				if (opcode == DRCUML_OP_WRITE)
					opcode = (drcuml_opcode)(DRCUML_OP_WRITE1 + (inst->param[2].value & 3));
				if (opcode == DRCUML_OP_WRITEM)
					opcode = (drcuml_opcode)(DRCUML_OP_WRITEM1 + (inst->param[3].value & 3));
				if (opcode == DRCUML_OP_SEXT)
					opcode = (drcuml_opcode)(DRCUML_OP_SEXT1 + (inst->param[2].value & 3));
				if (opcode == DRCUML_OP_FTOINT)
					opcode = (drcuml_opcode)(DRCUML_OP_FTOI4T + 5 * ((inst->param[2].value & 3) - 2) + inst->param[3].value);
				if (opcode == DRCUML_OP_FFRINT)
					opcode = (drcuml_opcode)(DRCUML_OP_FFRI4 + ((inst->param[2].value & 3) - 2));
				if (opcode == DRCUML_OP_FFRFLT)
					opcode = (drcuml_opcode)(DRCUML_OP_FFRFS + ((inst->param[2].value & 3) - 2));

				/* count how many bytes of immediates we need */
				immedbytes = 0;
				for (pnum = 0; pnum < inst->numparams; pnum++)
					if (inst->param[pnum].type == DRCUML_PTYPE_MAPVAR ||
						(inst->param[pnum].type == DRCUML_PTYPE_IMMEDIATE && inst->param[pnum].value != 0))
						immedbytes += psize[pnum];

				/* compute how many instruction words we need for that */
				immedwords = (immedbytes + sizeof(drcbec_instruction) - 1) / sizeof(drcbec_instruction);

				/* first item is the opcode, size, condition flags and length */
				(dst++)->i = MAKE_OPCODE_FULL(opcode, inst->size, inst->condition, inst->flags, inst->numparams + immedwords);

				/* immediates start after parameters */
				immed = dst + inst->numparams;

				/* output each of the parameters */
				for (pnum = 0; pnum < inst->numparams; pnum++)
					output_parameter(drcbe, &dst, &immed, psize[pnum], &inst->param[pnum]);

				/* point past the end of the immediates */
				dst += immedwords;
				break;
		}
	}

	/* complete codegen */
	*cachetop = (drccodeptr)dst;
	drccache_end_codegen(drcbe->cache);

	/* tell all of our utility objects that the block is finished */
	drchash_block_end(drcbe->hash, block);
	drclabel_block_end(drcbe->labels, block);
	drcmap_block_end(drcbe->map, block);
}


/*-------------------------------------------------
    drcbec_hash_exists - return true if the
    given mode/pc exists in the hash table
-------------------------------------------------*/

static int drcbec_hash_exists(drcbe_state *state, UINT32 mode, UINT32 pc)
{
	return drchash_code_exists(state->hash, mode, pc);
}


/*-------------------------------------------------
    drcbec_get_info - return information about
    the back-end implementation
-------------------------------------------------*/

static void drcbec_get_info(drcbe_state *state, drcbe_info *info)
{
	info->direct_iregs = 0;
	info->direct_fregs = 0;
}



/***************************************************************************
    BACK-END EXECUTION
***************************************************************************/

/*-------------------------------------------------
    drcbec_execute - execute a block of code
    registered at the given mode/pc
-------------------------------------------------*/

static int drcbec_execute(drcbe_state *drcbe, drcuml_codehandle *entry)
{
	const drcbec_instruction *callstack[32];
	const drcbec_instruction *newinst;
	const drcbec_instruction *inst;
	UINT32 temp32;
	UINT64 temp64;
	int shift;
	UINT8 flags = 0;
	UINT8 sp = 0;

	/* get the entry point */
	inst = (const drcbec_instruction *)drcuml_handle_codeptr(entry);
	assert_in_cache(drcbe->cache, inst);

	/* loop while we have cycles */
	while (TRUE)
	{
		UINT32 opcode = (inst++)->i;

		switch (OPCODE_GET_SHORT(opcode))
		{
			/* ----------------------- Control Flow Operations ----------------------- */

			case MAKE_OPCODE_SHORT(DRCUML_OP_HANDLE, 4, 0):		/* HANDLE  handle                 */
			case MAKE_OPCODE_SHORT(DRCUML_OP_HASH, 4, 0):		/* HASH    mode,pc                */
			case MAKE_OPCODE_SHORT(DRCUML_OP_LABEL, 4, 0):		/* LABEL   imm                    */
			case MAKE_OPCODE_SHORT(DRCUML_OP_COMMENT, 4, 0):	/* COMMENT string                 */
			case MAKE_OPCODE_SHORT(DRCUML_OP_MAPVAR, 4, 0):		/* MAPVAR  mapvar,value           */

				/* these opcodes should be processed at compile-time only */
				fatalerror("Unexpected opcode");
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_DEBUG, 4, 0):		/* DEBUG   pc                     */
				debugger_instruction_hook(drcbe->device, PARAM0);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_HASHJMP, 4, 0):	/* HASHJMP mode,pc,handle         */
				sp = 0;
				newinst = (const drcbec_instruction *)drchash_get_codeptr(drcbe->hash, PARAM0, PARAM1);
				if (newinst == NULL)
				{
					assert(sp < ARRAY_LENGTH(callstack));
					drcbe->state.exp = PARAM1;
					newinst = (const drcbec_instruction *)drcuml_handle_codeptr(inst[2].handle);
					callstack[sp++] = inst;
				}
				assert_in_cache(drcbe->cache, newinst);
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(DRCUML_OP_EXIT, 4, 1):		/* EXIT    src1[,c]               */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_EXIT, 4, 0):
				return PARAM0;

			case MAKE_OPCODE_SHORT(DRCUML_OP_JMP, 4, 1):		/* JMP     imm[,c]                */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_JMP, 4, 0):
				newinst = inst[0].inst;
				assert_in_cache(drcbe->cache, newinst);
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(DRCUML_OP_CALLH, 4, 1):		/* CALLH   handle[,c]             */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_CALLH, 4, 0):
				assert(sp < ARRAY_LENGTH(callstack));
				newinst = (const drcbec_instruction *)drcuml_handle_codeptr(inst[0].handle);
				assert_in_cache(drcbe->cache, newinst);
				callstack[sp++] = inst + OPCODE_GET_PWORDS(opcode);
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(DRCUML_OP_RET, 4, 1):		/* RET     [c]                    */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_RET, 4, 0):
				assert(sp > 0);
				newinst = callstack[--sp];
				assert_in_cache(drcbe->cache, newinst);
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(DRCUML_OP_EXH, 4, 1):		/* EXH     handle,param[,c]       */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_EXH, 4, 0):
				assert(sp < ARRAY_LENGTH(callstack));
				newinst = (const drcbec_instruction *)drcuml_handle_codeptr(inst[0].handle);
				assert_in_cache(drcbe->cache, newinst);
				drcbe->state.exp = PARAM1;
				callstack[sp++] = inst;
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(DRCUML_OP_CALLC, 4, 1):		/* CALLC   func,ptr[,c]           */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_CALLC, 4, 0):
				(*inst[0].cfunc)(inst[1].v);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_RECOVER, 4, 0):	/* RECOVER dst,mapvar             */
				assert(sp > 0);
				PARAM0 = drcmap_get_value(drcbe->map, (drccodeptr)callstack[0], PARAM1);
				break;


			/* ----------------------- Internal Register Operations ----------------------- */

			case MAKE_OPCODE_SHORT(DRCUML_OP_SETFMOD, 4, 0):	/* SETFMOD src                    */
				drcbe->state.fmod = PARAM0;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_GETFMOD, 4, 0):	/* GETFMOD dst                    */
				PARAM0 = drcbe->state.fmod;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_GETEXP, 4, 0):		/* GETEXP  dst                    */
				PARAM0 = drcbe->state.exp;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_GETFLGS, 4, 0):	/* GETFLGS dst[,f]                */
				PARAM0 = flags & PARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SAVE, 4, 0):		/* SAVE    dst                    */
				*inst[0].state = drcbe->state;
				inst[0].state->flags = flags;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_RESTORE, 4, 0):	/* RESTORE dst                    */
			case MAKE_OPCODE_SHORT(DRCUML_OP_RESTORE, 4, 1):	/* RESTORE dst                    */
				drcbe->state = *inst[0].state;
				flags = inst[0].state->flags;
				break;


			/* ----------------------- 32-Bit Integer Operations ----------------------- */

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD1, 4, 0):		/* LOAD    dst,base,index,BYTE    */
				PARAM0 = inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD1x2, 4, 0):	/* LOAD    dst,base,index,BYTE_x2 */
				PARAM0 = *(UINT8 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD1x4, 4, 0):	/* LOAD    dst,base,index,BYTE_x4 */
				PARAM0 = *(UINT8 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD1x8, 4, 0):	/* LOAD    dst,base,index,BYTE_x8 */
				PARAM0 = *(UINT8 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD2x1, 4, 0):	/* LOAD    dst,base,index,WORD_x1 */
				PARAM0 = *(UINT16 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD2, 4, 0):		/* LOAD    dst,base,index,WORD    */
				PARAM0 = inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD2x4, 4, 0):	/* LOAD    dst,base,index,WORD_x4 */
				PARAM0 = *(UINT16 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD2x8, 4, 0):	/* LOAD    dst,base,index,WORD_x8 */
				PARAM0 = *(UINT16 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD4x1, 4, 0):	/* LOAD    dst,base,index,DWORD_x1 */
				PARAM0 = *(UINT32 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD4x2, 4, 0):	/* LOAD    dst,base,index,DWORD_x2 */
				PARAM0 = *(UINT32 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD4, 4, 0):		/* LOAD    dst,base,index,DWORD   */
				PARAM0 = inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD4x8, 4, 0):	/* LOAD    dst,base,index,DWORD_x8 */
				PARAM0 = *(UINT32 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS1, 4, 0):		/* LOADS   dst,base,index,BYTE    */
				PARAM0 = inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS1x2, 4, 0):	/* LOADS   dst,base,index,BYTE_x2 */
				PARAM0 = *(INT8 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS1x4, 4, 0):	/* LOADS   dst,base,index,BYTE_x4 */
				PARAM0 = *(INT8 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS1x8, 4, 0):	/* LOADS   dst,base,index,BYTE_x8 */
				PARAM0 = *(INT8 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS2x1, 4, 0):	/* LOADS   dst,base,index,WORD_x1 */
				PARAM0 = *(INT16 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS2, 4, 0):		/* LOADS   dst,base,index,WORD    */
				PARAM0 = inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS2x4, 4, 0):	/* LOADS   dst,base,index,WORD_x4 */
				PARAM0 = *(INT16 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS2x8, 4, 0):	/* LOADS   dst,base,index,WORD_x8 */
				PARAM0 = *(INT16 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS4x1, 4, 0):	/* LOADS   dst,base,index,DWORD_x1 */
				PARAM0 = *(INT32 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS4x2, 4, 0):	/* LOADS   dst,base,index,DWORD_x2 */
				PARAM0 = *(INT32 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS4, 4, 0):		/* LOADS   dst,base,index,DWORD   */
				PARAM0 = inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS4x8, 4, 0):	/* LOADS   dst,base,index,DWORD_x8 */
				PARAM0 = *(INT32 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE1, 4, 0):		/* STORE   dst,base,index,BYTE    */
				inst[0].puint8[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE1x2, 4, 0):	/* STORE   dst,base,index,BYTE_x2 */
				*(UINT8 *)&inst[0].puint16[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE1x4, 4, 0):	/* STORE   dst,base,index,BYTE_x4 */
				*(UINT8 *)&inst[0].puint32[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE1x8, 4, 0):	/* STORE   dst,base,index,BYTE_x8 */
				*(UINT8 *)&inst[0].puint64[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE2x1, 4, 0):	/* STORE   dst,base,index,WORD_x1 */
				*(UINT16 *)&inst[0].puint8[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE2, 4, 0):		/* STORE   dst,base,index,WORD    */
				inst[0].puint16[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE2x4, 4, 0):	/* STORE   dst,base,index,WORD_x4 */
				*(UINT16 *)&inst[0].puint32[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE2x8, 4, 0):	/* STORE   dst,base,index,WORD_x8 */
				*(UINT16 *)&inst[0].puint64[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE4x1, 4, 0):	/* STORE   dst,base,index,DWORD_x1 */
				*(UINT32 *)&inst[0].puint8[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE4x2, 4, 0):	/* STORE   dst,base,index,DWORD_x2 */
				*(UINT32 *)&inst[0].puint16[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE4, 4, 0):		/* STORE   dst,base,index,DWORD   */
				inst[0].puint32[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE4x8, 4, 0):	/* STORE   dst,base,index,DWORD_x8 */
				*(UINT32 *)&inst[0].puint64[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READ1, 4, 0):		/* READ    dst,src1,space_BYTE    */
				PARAM0 = memory_read_byte(drcbe->space[PARAM2 / 16], PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READ2, 4, 0):		/* READ    dst,src1,space_WORD    */
				PARAM0 = memory_read_word(drcbe->space[PARAM2 / 16], PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READ4, 4, 0):		/* READ    dst,src1,space_DWORD   */
				PARAM0 = memory_read_dword(drcbe->space[PARAM2 / 16], PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READM2, 4, 0):		/* READM   dst,src1,mask,space_WORD */
				PARAM0 = memory_read_word_masked(drcbe->space[PARAM3 / 16], PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READM4, 4, 0):		/* READM   dst,src1,mask,space_DWORD */
				PARAM0 = memory_read_dword_masked(drcbe->space[PARAM3 / 16], PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITE1, 4, 0):		/* WRITE   dst,src1,space_BYTE     */
				memory_write_byte(drcbe->space[PARAM2 / 16], PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITE2, 4, 0):		/* WRITE   dst,src1,space_WORD     */
				memory_write_word(drcbe->space[PARAM2 / 16], PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITE4, 4, 0):		/* WRITE   dst,src1,space_DWORD    */
				memory_write_dword(drcbe->space[PARAM2 / 16], PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITEM2, 4, 0):	/* WRITEM  dst,src1,mask,space_WORD */
				memory_write_word_masked(drcbe->space[PARAM3 / 16], PARAM0, PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITEM4, 4, 0):	/* WRITEM  dst,src1,mask,space_DWORD */
				memory_write_dword_masked(drcbe->space[PARAM3 / 16], PARAM0, PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_CARRY, 4, 1):		/* CARRY   src,bitnum             */
				flags = (flags & ~DRCUML_FLAG_C) | ((PARAM0 >> (PARAM1 & 31)) & DRCUML_FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MOV, 4, 1):		/* MOV     dst,src[,c]            */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_MOV, 4, 0):
				PARAM0 = PARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SET, 4, 1):		/* SET     dst,c                  */
				PARAM0 = OPCODE_FAIL_CONDITION(opcode, flags) ? 0 : 1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT1, 4, 0):		/* SEXT1   dst,src                */
				PARAM0 = (INT8)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT1, 4, 1):
				temp32 = (INT8)PARAM1;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT2, 4, 0):		/* SEXT2   dst,src                */
				PARAM0 = (INT16)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT2, 4, 1):
				temp32 = (INT16)PARAM1;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLAND, 4, 0):		/* ROLAND  dst,src,count,mask[,f] */
				shift = PARAM2 & 31;
				PARAM0 = ((PARAM1 << shift) | (PARAM1 >> (32 - shift))) & PARAM3;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLAND, 4, 1):
				shift = PARAM2 & 31;
				temp32 = ((PARAM1 << shift) | (PARAM1 >> (32 - shift))) & PARAM3;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLINS, 4, 0):		/* ROLINS  dst,src,count,mask[,f] */
				shift = PARAM2 & 31;
				PARAM0 = (PARAM0 & ~PARAM3) | (((PARAM1 << shift) | (PARAM1 >> (32 - shift))) & PARAM3);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLINS, 4, 1):
				shift = PARAM2 & 31;
				temp32 = (PARAM0 & ~PARAM3) | (((PARAM1 << shift) | (PARAM1 >> (32 - shift))) & PARAM3);
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ADD, 4, 0):		/* ADD     dst,src1,src2[,f]      */
				PARAM0 = PARAM1 + PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ADD, 4, 1):
				temp32 = PARAM1 + PARAM2;
				flags = FLAGS32_NZCV_ADD(temp32, PARAM1, PARAM2);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ADDC, 4, 0):		/* ADDC    dst,src1,src2[,f]      */
				PARAM0 = PARAM1 + PARAM2 + (flags & DRCUML_FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ADDC, 4, 1):
				temp32 = PARAM1 + PARAM2 + (flags & DRCUML_FLAG_C);
				if (PARAM2 + 1 != 0)
					flags = FLAGS32_NZCV_ADD(temp32, PARAM1, PARAM2 + (flags & DRCUML_FLAG_C));
				else
					flags = FLAGS32_NZCV_ADD(temp32, PARAM1 + (flags & DRCUML_FLAG_C), PARAM2);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SUB, 4, 0):		/* SUB     dst,src1,src2[,f]      */
				PARAM0 = PARAM1 - PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SUB, 4, 1):
				temp32 = PARAM1 - PARAM2;
				flags = FLAGS32_NZCV_SUB(temp32, PARAM1, PARAM2);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SUBB, 4, 0):		/* SUBB    dst,src1,src2[,f]      */
				PARAM0 = PARAM1 - PARAM2 - (flags & DRCUML_FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SUBB, 4, 1):
				temp32 = PARAM1 - PARAM2 - (flags & DRCUML_FLAG_C);
				if (PARAM2 + 1 != 0)
					flags = FLAGS32_NZCV_SUB(temp32, PARAM1, PARAM2 + (flags & DRCUML_FLAG_C));
				else
					flags = FLAGS32_NZCV_SUB(temp32, PARAM1 - (flags & DRCUML_FLAG_C), PARAM2);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_CMP, 4, 1):		/* CMP     src1,src2[,f]          */
				temp32 = PARAM0 - PARAM1;
				flags = FLAGS32_NZCV_SUB(temp32, PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MULU, 4, 0):		/* MULU    dst,edst,src1,src2[,f] */
				temp64 = (UINT64)(UINT32)PARAM2 * (UINT64)(UINT32)PARAM3;
				PARAM1 = temp64 >> 32;
				PARAM0 = (UINT32)temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MULU, 4, 1):
				temp64 = (UINT64)(UINT32)PARAM2 * (UINT64)(UINT32)PARAM3;
				flags = FLAGS64_NZ(temp64);
				PARAM1 = temp64 >> 32;
				PARAM0 = (UINT32)temp64;
				if (temp64 != (UINT32)temp64)
					flags |= DRCUML_FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MULS, 4, 0):		/* MULS    dst,edst,src1,src2[,f] */
				temp64 = (INT64)(INT32)PARAM2 * (INT64)(INT32)PARAM3;
				PARAM1 = temp64 >> 32;
				PARAM0 = (UINT32)temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MULS, 4, 1):
				temp64 = (INT64)(INT32)PARAM2 * (INT64)(INT32)PARAM3;
				flags = FLAGS64_NZ(temp64);
				PARAM1 = temp64 >> 32;
				PARAM0 = (UINT32)temp64;
				if (temp64 != (INT32)temp64)
					flags |= DRCUML_FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_DIVU, 4, 0):		/* DIVU    dst,edst,src1,src2[,f] */
				if (PARAM3 != 0)
				{
					temp32 = (UINT32)PARAM2 / (UINT32)PARAM3;
					PARAM1 = (UINT32)PARAM2 % (UINT32)PARAM3;
					PARAM0 = temp32;
				}
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_DIVU, 4, 1):
				if (PARAM3 != 0)
				{
					temp32 = (UINT32)PARAM2 / (UINT32)PARAM3;
					PARAM1 = (UINT32)PARAM2 % (UINT32)PARAM3;
					flags = FLAGS32_NZ(temp32);
					PARAM0 = temp32;
				}
				else
					flags = DRCUML_FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_DIVS, 4, 0):		/* DIVS    dst,edst,src1,src2[,f] */
				if (PARAM3 != 0)
				{
					temp32 = (INT32)PARAM2 / (INT32)PARAM3;
					PARAM1 = (INT32)PARAM2 % (INT32)PARAM3;
					PARAM0 = temp32;
				}
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_DIVS, 4, 1):
				if (PARAM3 != 0)
				{
					temp32 = (INT32)PARAM2 / (INT32)PARAM3;
					PARAM1 = (INT32)PARAM2 % (INT32)PARAM3;
					flags = FLAGS32_NZ(temp32);
					PARAM0 = temp32;
				}
				else
					flags = DRCUML_FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_AND, 4, 0):		/* AND     dst,src1,src2[,f]      */
				PARAM0 = PARAM1 & PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_AND, 4, 1):
				temp32 = PARAM1 & PARAM2;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_TEST, 4, 1):		/* TEST    src1,src2[,f]          */
				temp32 = PARAM0 & PARAM1;
				flags = FLAGS32_NZ(temp32);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_OR, 4, 0):			/* OR      dst,src1,src2[,f]      */
				PARAM0 = PARAM1 | PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_OR, 4, 1):
				temp32 = PARAM1 | PARAM2;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_XOR, 4, 0):		/* XOR     dst,src1,src2[,f]      */
				PARAM0 = PARAM1 ^ PARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_XOR, 4, 1):
				temp32 = PARAM1 ^ PARAM2;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LZCNT, 4, 0):		/* LZCNT   dst,src                */
				PARAM0 = count_leading_zeros(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LZCNT, 4, 1):
				temp32 = count_leading_zeros(PARAM1);
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_BSWAP, 4, 0):		/* BSWAP   dst,src                */
				temp32 = PARAM1;
				PARAM0 = FLIPENDIAN_INT32(temp32);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_BSWAP, 4, 1):
				temp32 = PARAM1;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = FLIPENDIAN_INT32(temp32);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SHL, 4, 0):		/* SHL     dst,src,count[,f]      */
				PARAM0 = PARAM1 << (PARAM2 & 31);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SHL, 4, 1):
				shift = PARAM2 & 31;
				temp32 = PARAM1 << shift;
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= ((PARAM1 << (shift - 1)) >> 31) & DRCUML_FLAG_C;
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SHR, 4, 0):		/* SHR     dst,src,count[,f]      */
				PARAM0 = PARAM1 >> (PARAM2 & 31);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SHR, 4, 1):
				shift = PARAM2 & 31;
				temp32 = PARAM1 >> shift;
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= (PARAM1 >> (shift - 1)) & DRCUML_FLAG_C;
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SAR, 4, 0):		/* SAR     dst,src,count[,f]      */
				PARAM0 = (INT32)PARAM1 >> (PARAM2 & 31);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SAR, 4, 1):
				shift = PARAM2 & 31;
				temp32 = (INT32)PARAM1 >> shift;
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= (PARAM1 >> (shift - 1)) & DRCUML_FLAG_C;
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROL, 4, 0):		/* ROL     dst,src,count[,f]      */
				shift = PARAM2 & 31;
				PARAM0 = (PARAM1 << shift) | (PARAM1 >> ((32 - shift) & 31));
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROL, 4, 1):
				shift = PARAM2 & 31;
				temp32 = (PARAM1 << shift) | (PARAM1 >> ((32 - shift) & 31));
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= ((PARAM1 << (shift - 1)) >> 31) & DRCUML_FLAG_C;
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLC, 4, 0):		/* ROLC    dst,src,count[,f]      */
				shift = PARAM2 & 31;
				if (shift > 1)
					PARAM0 = (PARAM1 << shift) | ((flags & DRCUML_FLAG_C) << (shift - 1)) | (PARAM1 >> (33 - shift));
				else if (shift == 1)
					PARAM0 = (PARAM1 << shift) | (flags & DRCUML_FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLC, 4, 1):
				shift = PARAM2 & 31;
				if (shift > 1)
					temp32 = (PARAM1 << shift) | ((flags & DRCUML_FLAG_C) << (shift - 1)) | (PARAM1 >> (33 - shift));
				else if (shift == 1)
					temp32 = (PARAM1 << shift) | (flags & DRCUML_FLAG_C);
				else
					temp32 = PARAM1;
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= ((PARAM1 << (shift - 1)) >> 31) & DRCUML_FLAG_C;
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROR, 4, 0):		/* ROR     dst,src,count[,f]      */
				shift = PARAM2 & 31;
				PARAM0 = (PARAM1 >> shift) | (PARAM1 << ((32 - shift) & 31));
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROR, 4, 1):
				shift = PARAM2 & 31;
				temp32 = (PARAM1 >> shift) | (PARAM1 << ((32 - shift) & 31));
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= (PARAM1 >> (shift - 1)) & DRCUML_FLAG_C;
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_RORC, 4, 0):		/* RORC    dst,src,count[,f]      */
				shift = PARAM2 & 31;
				if (shift > 1)
					PARAM0 = (PARAM1 >> shift) | (((flags & DRCUML_FLAG_C) << 31) >> (shift - 1)) | (PARAM1 << (33 - shift));
				else if (shift == 1)
					PARAM0 = (PARAM1 >> shift) | ((flags & DRCUML_FLAG_C) << 31);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_RORC, 4, 1):
				shift = PARAM2 & 31;
				if (shift > 1)
					temp32 = (PARAM1 >> shift) | (((flags & DRCUML_FLAG_C) << 31) >> (shift - 1)) | (PARAM1 << (33 - shift));
				else if (shift == 1)
					temp32 = (PARAM1 >> shift) | ((flags & DRCUML_FLAG_C) << 31);
				else
					temp32 = PARAM1;
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= (PARAM1 >> (shift - 1)) & DRCUML_FLAG_C;
				PARAM0 = temp32;
				break;


			/* ----------------------- 64-Bit Integer Operations ----------------------- */

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD1, 8, 0):		/* DLOAD   dst,base,index,BYTE    */
				DPARAM0 = inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD1x2, 8, 0):	/* DLOAD   dst,base,index,BYTE_x2 */
				DPARAM0 = *(UINT8 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD1x4, 8, 0):	/* DLOAD   dst,base,index,BYTE_x4 */
				DPARAM0 = *(UINT8 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD1x8, 8, 0):	/* DLOAD   dst,base,index,BYTE_x8 */
				DPARAM0 = *(UINT8 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD2x1, 8, 0):	/* DLOAD   dst,base,index,WORD_x1 */
				DPARAM0 = *(UINT16 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD2, 8, 0):		/* DLOAD   dst,base,index,WORD    */
				DPARAM0 = inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD2x4, 8, 0):	/* DLOAD   dst,base,index,WORD_x4 */
				DPARAM0 = *(UINT16 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD2x8, 8, 0):	/* DLOAD   dst,base,index,WORD_x8 */
				DPARAM0 = *(UINT16 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD4x1, 8, 0):	/* DLOAD   dst,base,index,DWORD_x1 */
				DPARAM0 = *(UINT32 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD4x2, 8, 0):	/* DLOAD   dst,base,index,DWORD_x2 */
				DPARAM0 = *(UINT32 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD4, 8, 0):		/* DLOAD   dst,base,index,DWORD   */
				DPARAM0 = inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD4x8, 8, 0):	/* DLOAD   dst,base,index,DWORD_x8 */
				DPARAM0 = *(UINT32 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD8x1, 8, 0):	/* DLOAD   dst,base,index,QWORD_x1 */
				DPARAM0 = *(UINT64 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD8x2, 8, 0):	/* DLOAD   dst,base,index,QWORD_x2 */
				DPARAM0 = *(UINT64 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD8x4, 8, 0):	/* DLOAD   dst,base,index,QWORD_x4 */
				DPARAM0 = *(UINT64 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOAD8, 8, 0):		/* DLOAD   dst,base,index,QWORD */
				DPARAM0 = inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS1, 8, 0):		/* DLOADS  dst,base,index,BYTE    */
				DPARAM0 = inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS1x2, 8, 0):	/* DLOADS  dst,base,index,BYTE_x2 */
				DPARAM0 = *(INT8 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS1x4, 8, 0):	/* DLOADS  dst,base,index,BYTE_x4 */
				DPARAM0 = *(INT8 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS1x8, 8, 0):	/* DLOADS  dst,base,index,BYTE_x8 */
				DPARAM0 = *(INT8 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS2x1, 8, 0):	/* DLOADS  dst,base,index,WORD_x1 */
				DPARAM0 = *(INT16 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS2, 8, 0):		/* DLOADS  dst,base,index,WORD    */
				DPARAM0 = inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS2x4, 8, 0):	/* DLOADS  dst,base,index,WORD_x4 */
				DPARAM0 = *(INT16 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS2x8, 8, 0):	/* DLOADS  dst,base,index,WORD_x8 */
				DPARAM0 = *(INT16 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS4x1, 8, 0):	/* DLOADS  dst,base,index,DWORD_x1 */
				DPARAM0 = *(INT32 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS4x2, 8, 0):	/* DLOADS  dst,base,index,DWORD_x2 */
				DPARAM0 = *(INT32 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS4, 8, 0):		/* DLOADS  dst,base,index,DWORD   */
				DPARAM0 = inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS4x8, 8, 0):	/* DLOADS  dst,base,index,DWORD_x8 */
				DPARAM0 = *(INT32 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS8x1, 8, 0):	/* DLOADS  dst,base,index,QWORD_x1 */
				DPARAM0 = *(INT64 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS8x2, 8, 0):	/* DLOADS  dst,base,index,QWORD_x2 */
				DPARAM0 = *(INT64 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS8x4, 8, 0):	/* DLOADS  dst,base,index,QWORD_x4 */
				DPARAM0 = *(INT64 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LOADS8, 8, 0):		/* DLOADS  dst,base,index,QWORD   */
				DPARAM0 = inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE1, 8, 0):		/* DSTORE  dst,base,index,BYTE    */
				inst[0].puint8[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE1x2, 8, 0):	/* DSTORE  dst,base,index,BYTE_x2 */
				*(UINT8 *)&inst[0].puint16[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE1x4, 8, 0):	/* DSTORE  dst,base,index,BYTE_x4 */
				*(UINT8 *)&inst[0].puint32[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE1x8, 8, 0):	/* DSTORE  dst,base,index,BYTE_x8 */
				*(UINT8 *)&inst[0].puint64[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE2x1, 8, 0):	/* DSTORE  dst,base,index,WORD_x1 */
				*(UINT16 *)&inst[0].puint8[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE2, 8, 0):		/* DSTORE  dst,base,index,WORD    */
				inst[0].puint16[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE2x4, 8, 0):	/* DSTORE  dst,base,index,WORD_x4 */
				*(UINT16 *)&inst[0].puint32[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE2x8, 8, 0):	/* DSTORE  dst,base,index,WORD_x8 */
				*(UINT16 *)&inst[0].puint64[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE4x1, 8, 0):	/* DSTORE  dst,base,index,DWORD_x1 */
				*(UINT32 *)&inst[0].puint8[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE4x2, 8, 0):	/* DSTORE  dst,base,index,DWORD_x2 */
				*(UINT32 *)&inst[0].puint16[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE4, 8, 0):		/* DSTORE  dst,base,index,DWORD   */
				inst[0].puint32[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE4x8, 8, 0):	/* DSTORE  dst,base,index,DWORD_x8 */
				*(UINT32 *)&inst[0].puint64[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE8x1, 8, 0):	/* DSTORE  dst,base,index,QWORD_x1 */
				*(UINT64 *)&inst[0].puint8[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE8x2, 8, 0):	/* DSTORE  dst,base,index,QWORD_x2 */
				*(UINT64 *)&inst[0].puint16[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE8x4, 8, 0):	/* DSTORE  dst,base,index,QWORD_x4 */
				*(UINT64 *)&inst[0].puint32[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_STORE8, 8, 0):		/* DSTORE  dst,base,index,QWORD    */
				inst[0].puint64[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READ1, 8, 0):		/* DREAD   dst,src1,space_BYTE    */
				DPARAM0 = memory_read_byte(drcbe->space[PARAM2 / 16], PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READ2, 8, 0):		/* DREAD   dst,src1,space_WORD    */
				DPARAM0 = memory_read_word(drcbe->space[PARAM2 / 16], PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READ4, 8, 0):		/* DREAD   dst,src1,space_DWORD   */
				DPARAM0 = memory_read_dword(drcbe->space[PARAM2 / 16], PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READ8, 8, 0):		/* DREAD   dst,src1,space_QOWRD   */
				DPARAM0 = memory_read_qword(drcbe->space[PARAM2 / 16], PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READM2, 8, 0):		/* DREADM  dst,src1,mask,space_WORD */
				DPARAM0 = memory_read_word_masked(drcbe->space[PARAM3 / 16], PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READM4, 8, 0):		/* DREADM  dst,src1,mask,space_DWORD */
				DPARAM0 = memory_read_dword_masked(drcbe->space[PARAM3 / 16], PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_READM8, 8, 0):		/* DREADM  dst,src1,mask,space_QWORD */
				DPARAM0 = memory_read_qword_masked(drcbe->space[PARAM3 / 16], PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITE1, 8, 0):		/* DWRITE  dst,src1,space_BYTE    */
				memory_write_byte(drcbe->space[PARAM2 / 16], PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITE2, 8, 0):		/* DWRITE  dst,src1,space_WORD    */
				memory_write_word(drcbe->space[PARAM2 / 16], PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITE4, 8, 0):		/* DWRITE  dst,src1,space_DWORD   */
				memory_write_dword(drcbe->space[PARAM2 / 16], PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITE8, 8, 0):		/* DWRITE  dst,src1,space_QWORD   */
				memory_write_qword(drcbe->space[PARAM2 / 16], PARAM0, DPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITEM2, 8, 0):	/* DWRITEM dst,src1,mask,space_WORD */
				memory_write_word_masked(drcbe->space[PARAM3 / 16], PARAM0, DPARAM1, DPARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITEM4, 8, 0):	/* DWRITEM dst,src1,mask,space_DWORD */
				memory_write_dword_masked(drcbe->space[PARAM3 / 16], PARAM0, DPARAM1, DPARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_WRITEM8, 8, 0):	/* DWRITEM dst,src1,mask,space_QWORD */
				memory_write_qword_masked(drcbe->space[PARAM3 / 16], PARAM0, DPARAM1, DPARAM2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_CARRY, 8, 0):		/* DCARRY  src,bitnum             */
				flags = (flags & ~DRCUML_FLAG_C) | ((DPARAM0 >> (DPARAM1 & 63)) & DRCUML_FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MOV, 8, 1):		/* DMOV    dst,src[,c]            */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_MOV, 8, 0):
				DPARAM0 = DPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SET, 8, 1):		/* DSET    dst,c                  */
				DPARAM0 = OPCODE_FAIL_CONDITION(opcode, flags) ? 0 : 1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT1, 8, 0):		/* DSEXT   dst,src,BYTE           */
				DPARAM0 = (INT8)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT1, 8, 1):
				temp64 = (INT8)PARAM1;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT2, 8, 0):		/* DSEXT   dst,src,WORD           */
				DPARAM0 = (INT16)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT2, 8, 1):
				temp64 = (INT16)PARAM1;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT4, 8, 0):		/* DSEXT   dst,src,DWORD          */
				DPARAM0 = (INT32)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SEXT4, 8, 1):
				temp64 = (INT32)PARAM1;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLAND, 8, 0):		/* DROLAND dst,src,count,mask[,f] */
				shift = DPARAM2 & 63;
				DPARAM0 = ((DPARAM1 << shift) | (DPARAM1 >> (64 - shift))) & DPARAM3;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLAND, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = ((DPARAM1 << shift) | (DPARAM1 >> (64 - shift))) & DPARAM3;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLINS, 8, 0):		/* DROLINS dst,src,count,mask[,f] */
				shift = DPARAM2 & 63;
				DPARAM0 = (DPARAM0 & ~DPARAM3) | (((DPARAM1 << shift) | (DPARAM1 >> (64 - shift))) & DPARAM3);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLINS, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = (DPARAM0 & ~DPARAM3) | (((DPARAM1 << shift) | (DPARAM1 >> (64 - shift))) & DPARAM3);
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ADD, 8, 0):		/* DADD    dst,src1,src2[,f]      */
				DPARAM0 = DPARAM1 + DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ADD, 8, 1):
				temp64 = DPARAM1 + DPARAM2;
				flags = FLAGS64_NZCV_ADD(temp64, DPARAM1, DPARAM2);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ADDC, 8, 0):		/* DADDC   dst,src1,src2[,f]      */
				DPARAM0 = DPARAM1 + DPARAM2 + (flags & DRCUML_FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ADDC, 8, 1):
				temp64 = DPARAM1 + DPARAM2 + (flags & DRCUML_FLAG_C);
				if (DPARAM2 + 1 != 0)
					flags = FLAGS64_NZCV_ADD(temp64, DPARAM1, DPARAM2 + (flags & DRCUML_FLAG_C));
				else
					flags = FLAGS64_NZCV_ADD(temp64, DPARAM1 + (flags & DRCUML_FLAG_C), DPARAM2);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SUB, 8, 0):		/* DSUB    dst,src1,src2[,f]      */
				DPARAM0 = DPARAM1 - DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SUB, 8, 1):
				temp64 = DPARAM1 - DPARAM2;
				flags = FLAGS64_NZCV_SUB(temp64, DPARAM1, DPARAM2);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SUBB, 8, 0):		/* DSUBB   dst,src1,src2[,f]      */
				DPARAM0 = DPARAM1 - DPARAM2 - (flags & DRCUML_FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SUBB, 8, 1):
				temp64 = DPARAM1 - DPARAM2 - (flags & DRCUML_FLAG_C);
				if (DPARAM2 + 1 != 0)
					flags = FLAGS64_NZCV_SUB(temp64, DPARAM1, DPARAM2 + (flags & DRCUML_FLAG_C));
				else
					flags = FLAGS64_NZCV_SUB(temp64, DPARAM1 - (flags & DRCUML_FLAG_C), DPARAM2);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_CMP, 8, 1):		/* DCMP    src1,src2[,f]          */
				temp64 = DPARAM0 - DPARAM1;
				flags = FLAGS64_NZCV_SUB(temp64, DPARAM0, DPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MULU, 8, 0):		/* DMULU   dst,edst,src1,src2[,f] */
				dmulu(inst[0].puint64, inst[1].puint64, DPARAM2, DPARAM3, FALSE);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MULU, 8, 1):
				flags = dmulu(inst[0].puint64, inst[1].puint64, DPARAM2, DPARAM3, TRUE);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MULS, 8, 0):		/* DMULS   dst,edst,src1,src2[,f] */
				dmuls(inst[0].puint64, inst[1].puint64, DPARAM2, DPARAM3, FALSE);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_MULS, 8, 1):
				flags = dmuls(inst[0].puint64, inst[1].puint64, DPARAM2, DPARAM3, TRUE);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_DIVU, 8, 0):		/* DDIVU   dst,edst,src1,src2[,f] */
				if (DPARAM3 != 0)
				{
					temp64 = (UINT64)DPARAM2 / (UINT64)DPARAM3;
					DPARAM1 = (UINT64)DPARAM2 % (UINT64)DPARAM3;
					DPARAM0 = temp64;
				}
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_DIVU, 8, 1):
				if (DPARAM3 != 0)
				{
					temp64 = (UINT64)DPARAM2 / (UINT64)DPARAM3;
					DPARAM1 = (UINT64)DPARAM2 % (UINT64)DPARAM3;
					flags = FLAGS64_NZ(temp64);
					DPARAM0 = temp64;
				}
				else
					flags = DRCUML_FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_DIVS, 8, 0):		/* DDIVS   dst,edst,src1,src2[,f] */
				if (DPARAM3 != 0)
				{
					temp64 = (INT64)DPARAM2 / (INT64)DPARAM3;
					DPARAM1 = (INT64)DPARAM2 % (INT64)DPARAM3;
					DPARAM0 = temp64;
				}
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_DIVS, 8, 1):
				if (DPARAM3 != 0)
				{
					temp64 = (INT64)DPARAM2 / (INT64)DPARAM3;
					DPARAM1 = (INT64)DPARAM2 % (INT64)DPARAM3;
					flags = FLAGS64_NZ(temp64);
					DPARAM0 = temp64;
				}
				else
					flags = DRCUML_FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_AND, 8, 0):		/* DAND    dst,src1,src2[,f]      */
				DPARAM0 = DPARAM1 & DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_AND, 8, 1):
				temp64 = DPARAM1 & DPARAM2;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_TEST, 8, 1):		/* DTEST   src1,src2[,f]          */
				temp64 = DPARAM1 & DPARAM2;
				flags = FLAGS64_NZ(temp64);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_OR, 8, 0):			/* DOR     dst,src1,src2[,f]      */
				DPARAM0 = DPARAM1 | DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_OR, 8, 1):
				temp64 = DPARAM1 | DPARAM2;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_XOR, 8, 0):		/* DXOR    dst,src1,src2[,f]      */
				DPARAM0 = DPARAM1 ^ DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_XOR, 8, 1):
				temp64 = DPARAM1 ^ DPARAM2;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LZCNT, 8, 0):		/* DLZCNT  dst,src                */
				if ((UINT32)(DPARAM1 >> 32) != 0)
					DPARAM0 = count_leading_zeros(DPARAM1 >> 32);
				else
					DPARAM0 = 32 + count_leading_zeros(DPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_LZCNT, 8, 1):
				if ((UINT32)(DPARAM1 >> 32) != 0)
					temp64 = count_leading_zeros(DPARAM1 >> 32);
				else
					temp64 = 32 + count_leading_zeros(DPARAM1);
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_BSWAP, 8, 0):		/* DBSWAP  dst,src                */
				temp64 = DPARAM1;
				DPARAM0 = FLIPENDIAN_INT64(temp64);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_BSWAP, 8, 1):
				temp64 = DPARAM1;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = FLIPENDIAN_INT64(temp64);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SHL, 8, 0):		/* DSHL    dst,src,count[,f]      */
				DPARAM0 = DPARAM1 << (DPARAM2 & 63);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SHL, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = DPARAM1 << shift;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= ((DPARAM1 << (shift - 1)) >> 63) & DRCUML_FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SHR, 8, 0):		/* DSHR    dst,src,count[,f]      */
				DPARAM0 = DPARAM1 >> (DPARAM2 & 63);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SHR, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = DPARAM1 >> shift;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= (DPARAM1 >> (shift - 1)) & DRCUML_FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SAR, 8, 0):		/* DSAR    dst,src,count[,f]      */
				DPARAM0 = (INT64)DPARAM1 >> (DPARAM2 & 63);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_SAR, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = (INT32)DPARAM1 >> shift;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= (DPARAM1 >> (shift - 1)) & DRCUML_FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROL, 8, 0):		/* DROL    dst,src,count[,f]      */
				shift = DPARAM2 & 31;
				DPARAM0 = (DPARAM1 << shift) | (DPARAM1 >> ((64 - shift) & 63));
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROL, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = (DPARAM1 << shift) | (DPARAM1 >> ((64 - shift) & 63));
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= ((DPARAM1 << (shift - 1)) >> 63) & DRCUML_FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLC, 8, 0):		/* DROLC   dst,src,count[,f]      */
				shift = DPARAM2 & 63;
				if (shift > 1)
					DPARAM0 = (DPARAM1 << shift) | ((flags & DRCUML_FLAG_C) << (shift - 1)) | (DPARAM1 >> (65 - shift));
				else if (shift == 1)
					DPARAM0 = (DPARAM1 << shift) | (flags & DRCUML_FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROLC, 8, 1):
				shift = DPARAM2 & 63;
				if (shift > 1)
					temp64 = (DPARAM1 << shift) | ((flags & DRCUML_FLAG_C) << (shift - 1)) | (DPARAM1 >> (65 - shift));
				else if (shift == 1)
					temp64 = (DPARAM1 << shift) | (flags & DRCUML_FLAG_C);
				else
					temp64 = DPARAM1;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= ((DPARAM1 << (shift - 1)) >> 63) & DRCUML_FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROR, 8, 0):		/* DROR    dst,src,count[,f]      */
				shift = DPARAM2 & 63;
				DPARAM0 = (DPARAM1 >> shift) | (DPARAM1 << ((64 - shift) & 63));
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_ROR, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = (DPARAM1 >> shift) | (DPARAM1 << ((64 - shift) & 63));
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= (DPARAM1 >> (shift - 1)) & DRCUML_FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_RORC, 8, 0):		/* DRORC   dst,src,count[,f]      */
				shift = DPARAM2 & 63;
				if (shift > 1)
					DPARAM0 = (DPARAM1 >> shift) | ((((UINT64)flags & DRCUML_FLAG_C) << 63) >> (shift - 1)) | (DPARAM1 << (65 - shift));
				else if (shift == 1)
					DPARAM0 = (DPARAM1 >> shift) | (((UINT64)flags & DRCUML_FLAG_C) << 63);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_RORC, 8, 1):
				shift = DPARAM2 & 63;
				if (shift > 1)
					temp64 = (DPARAM1 >> shift) | ((((UINT64)flags & DRCUML_FLAG_C) << 63) >> (shift - 1)) | (DPARAM1 << (65 - shift));
				else if (shift == 1)
					temp64 = (DPARAM1 >> shift) | (((UINT64)flags & DRCUML_FLAG_C) << 63);
				else
					temp64 = DPARAM1;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= (DPARAM1 >> (shift - 1)) & DRCUML_FLAG_C;
				DPARAM0 = temp64;
				break;


			/* ----------------------- 32-Bit Floating Point Operations ----------------------- */

			case MAKE_OPCODE_SHORT(DRCUML_OP_FLOAD, 4, 0):		/* FSLOAD  dst,base,index         */
				FSPARAM0 = inst[1].pfloat[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FSTORE, 4, 0):		/* FSSTORE dst,base,index         */
				inst[0].pfloat[PARAM1] = FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FREAD, 4, 0):		/* FSREAD  dst,src1,space         */
				PARAM0 = memory_read_dword(drcbe->space[PARAM2], PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FWRITE, 4, 0):		/* FSWRITE dst,src1,space         */
				memory_write_dword(drcbe->space[PARAM2], PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FMOV, 4, 1):		/* FSMOV   dst,src[,c]            */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_FMOV, 4, 0):
				FSPARAM0 = FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4T, 4, 0):		/* FSTOI4T dst,src1               */
				if (FSPARAM1 >= 0)
					*inst[0].pint32 = floor(FSPARAM1);
				else
					*inst[0].pint32 = ceil(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4R, 4, 0):		/* FSTOI4R dst,src1               */
				if (FSPARAM1 >= 0)
					*inst[0].pint32 = floor(FSPARAM1 + 0.5f);
				else
					*inst[0].pint32 = ceil(FSPARAM1 - 0.5f);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4F, 4, 0):		/* FSTOI4F dst,src1               */
				*inst[0].pint32 = floor(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4C, 4, 0):		/* FSTOI4C dst,src1               */
				*inst[0].pint32 = ceil(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4, 4, 0):		/* FSTOI4  dst,src1               */
				*inst[0].pint32 = FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8T, 4, 0):		/* FSTOI8T dst,src1               */
				if (FSPARAM1 >= 0)
					*inst[0].pint64 = floor(FSPARAM1);
				else
					*inst[0].pint64 = ceil(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8R, 4, 0):		/* FSTOI8R dst,src1               */
				if (FSPARAM1 >= 0)
					*inst[0].pint64 = floor(FSPARAM1 + 0.5f);
				else
					*inst[0].pint64 = ceil(FSPARAM1 - 0.5f);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8F, 4, 0):		/* FSTOI8F dst,src1               */
				*inst[0].pint64 = floor(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8C, 4, 0):		/* FSTOI8C dst,src1               */
				*inst[0].pint64 = ceil(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8, 4, 0):		/* FSTOI8  dst,src1               */
				*inst[0].pint64 = FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FFRI4, 4, 0):		/* FSFRI4  dst,src1               */
				FSPARAM0 = *inst[1].pint32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FFRI8, 4, 0):		/* FSFRI8  dst,src1               */
				FSPARAM0 = *inst[1].pint64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FFRFD, 4, 0):		/* FSFRFD  dst,src1               */
				FSPARAM0 = FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FADD, 4, 0):		/* FSADD   dst,src1,src2          */
				FSPARAM0 = FSPARAM1 + FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FSUB, 4, 0):		/* FSSUB   dst,src1,src2          */
				FSPARAM0 = FSPARAM1 - FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FCMP, 4, 1):		/* FSCMP   src1,src2              */
				if (isnan(FSPARAM0) || isnan(FSPARAM1))
					flags = DRCUML_FLAG_U;
				else
					flags = (FSPARAM0 < FSPARAM1) | ((FSPARAM0 == FSPARAM1) << 2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FMUL, 4, 0):		/* FSMUL   dst,src1,src2          */
				FSPARAM0 = FSPARAM1 * FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FDIV, 4, 0):		/* FSDIV   dst,src1,src2          */
				FSPARAM0 = FSPARAM1 / FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FNEG, 4, 0):		/* FSNEG   dst,src1               */
				FSPARAM0 = -FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FABS, 4, 0):		/* FSABS   dst,src1               */
				FSPARAM0 = fabs(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FSQRT, 4, 0):		/* FSSQRT  dst,src1               */
				FSPARAM0 = sqrt(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FRECIP, 4, 0):		/* FSRECIP dst,src1               */
				FSPARAM0 = 1.0f / FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FRSQRT, 4, 0):		/* FSRSQRT dst,src1               */
				FSPARAM0 = 1.0f / sqrt(FSPARAM1);
				break;


			/* ----------------------- 64-Bit Floating Point Operations ----------------------- */

			case MAKE_OPCODE_SHORT(DRCUML_OP_FLOAD, 8, 0):		/* FDLOAD  dst,base,index         */
				FDPARAM0 = inst[1].pdouble[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FSTORE, 8, 0):		/* FDSTORE dst,base,index         */
				inst[0].pdouble[PARAM1] = FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FREAD, 8, 0):		/* FDREAD  dst,src1,space         */
				DPARAM0 = memory_read_qword(drcbe->space[PARAM2], PARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FWRITE, 8, 0):		/* FDWRITE dst,src1,space         */
				memory_write_qword(drcbe->space[PARAM2], PARAM0, DPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FMOV, 8, 1):		/* FDMOV   dst,src[,c]            */
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				/* fall through... */

			case MAKE_OPCODE_SHORT(DRCUML_OP_FMOV, 8, 0):
				FDPARAM0 = FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4T, 8, 0):		/* FDTOI4T dst,src1               */
				if (FDPARAM1 >= 0)
					*inst[0].pint32 = floor(FDPARAM1);
				else
					*inst[0].pint32 = ceil(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4R, 8, 0):		/* FDTOI4R dst,src1               */
				if (FDPARAM1 >= 0)
					*inst[0].pint32 = floor(FDPARAM1 + 0.5);
				else
					*inst[0].pint32 = ceil(FDPARAM1 - 0.5);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4F, 8, 0):		/* FDTOI4F dst,src1               */
				*inst[0].pint32 = floor(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4C, 8, 0):		/* FDTOI4C dst,src1               */
				*inst[0].pint32 = ceil(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI4, 8, 0):		/* FDTOI4  dst,src1               */
				*inst[0].pint32 = FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8T, 8, 0):		/* FDTOI8T dst,src1               */
				if (FDPARAM1 >= 0)
					*inst[0].pint64 = floor(FDPARAM1);
				else
					*inst[0].pint64 = ceil(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8R, 8, 0):		/* FDTOI8R  dst,src1               */
				if (FDPARAM1 >= 0)
					*inst[0].pint64 = floor(FDPARAM1 + 0.5);
				else
					*inst[0].pint64 = ceil(FDPARAM1 - 0.5);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8F, 8, 0):		/* FDTOI8F dst,src1               */
				*inst[0].pint64 = floor(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8C, 8, 0):		/* FDTOI8C dst,src1               */
				*inst[0].pint64 = ceil(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FTOI8, 8, 0):		/* FDTOI8  dst,src1               */
				*inst[0].pint64 = FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FFRI4, 8, 0):		/* FDFRI4  dst,src1               */
				FDPARAM0 = *inst[1].pint32;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FFRI8, 8, 0):		/* FDFRI8  dst,src1               */
				FDPARAM0 = *inst[1].pint64;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FFRFS, 8, 0):		/* FDFRFS  dst,src1               */
				FDPARAM0 = FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FRNDS, 8, 0):		/* FDRNDS  dst,src1               */
				FDPARAM0 = (float)FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FADD, 8, 0):		/* FDADD   dst,src1,src2          */
				FDPARAM0 = FDPARAM1 + FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FSUB, 8, 0):		/* FDSUB   dst,src1,src2          */
				FDPARAM0 = FDPARAM1 - FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FCMP, 8, 1):		/* FDCMP   src1,src2              */
				if (isnan(FDPARAM0) || isnan(FDPARAM1))
					flags = DRCUML_FLAG_U;
				else
					flags = (FDPARAM0 < FDPARAM1) | ((FDPARAM0 == FDPARAM1) << 2);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FMUL, 8, 0):		/* FDMUL   dst,src1,src2          */
				FDPARAM0 = FDPARAM1 * FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FDIV, 8, 0):		/* FDDIV   dst,src1,src2          */
				FDPARAM0 = FDPARAM1 / FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FNEG, 8, 0):		/* FDNEG   dst,src1               */
				FDPARAM0 = -FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FABS, 8, 0):		/* FDABS   dst,src1               */
				FDPARAM0 = fabs(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FSQRT, 8, 0):		/* FDSQRT  dst,src1               */
				FDPARAM0 = sqrt(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FRECIP, 8, 0):		/* FDRECIP dst,src1               */
				FDPARAM0 = 1.0 / FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(DRCUML_OP_FRSQRT, 8, 0):		/* FDRSQRT dst,src1               */
				FDPARAM0 = 1.0 / sqrt(FDPARAM1);
				break;

			default:
				fatalerror("Unexpected opcode!");
				break;
		}

		/* advance past the parameters and immediates */
		inst += OPCODE_GET_PWORDS(opcode);
	}

	return 0;
}



/***************************************************************************
    PRIVATE HELPER FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    output_parameter - output a parameter
-------------------------------------------------*/

static void output_parameter(drcbe_state *drcbe, drcbec_instruction **dstptr, void **immedptr, int size, const drcuml_parameter *param)
{
	drcbec_instruction *dst = *dstptr;
	void *immed = *immedptr;
	drcuml_parameter temp_param;

	switch (param->type)
	{
		/* convert mapvars to immediates */
		case DRCUML_PTYPE_MAPVAR:
			temp_param.value = drcmap_get_last_value(drcbe->map, param->value);
			param = &temp_param;
			/* fall through to immediate case */

		/* immediates store a pointer to the immediate data, which is stored at the end of the instruction */
		case DRCUML_PTYPE_IMMEDIATE:
			if (param->value == 0)
				(dst++)->v = &immediate_zero;
			else
			{
				(dst++)->v = immed;
				if (size == 4)
					*(UINT32 *)immed = (UINT32)param->value;
				else
					*(UINT64 *)immed = (UINT64)param->value;
				immed = (UINT8 *)immed + size;
			}
			break;

		/* int registers point to the appropriate part of the integer register state */
		case DRCUML_PTYPE_INT_REGISTER:
			if (size == 4)
				(dst++)->puint32 = &drcbe->state.r[param->value - DRCUML_REG_I0].w.l;
			else
				(dst++)->puint64 = &drcbe->state.r[param->value - DRCUML_REG_I0].d;
			break;

		/* float registers point to the appropriate part of the floating point register state */
		case DRCUML_PTYPE_FLOAT_REGISTER:
			if (size == 4)
				(dst++)->pfloat = &drcbe->state.f[param->value - DRCUML_REG_F0].s.l;
			else
				(dst++)->pdouble = &drcbe->state.f[param->value - DRCUML_REG_F0].d;
			break;

		/* memory just points to the memory */
		case DRCUML_PTYPE_MEMORY:
			(dst++)->v = (void *)(FPTR)param->value;
			break;

		default:
			fatalerror("Unexpected param->type");
			break;
	}

	*dstptr = dst;
	*immedptr = immed;
}


/*-------------------------------------------------
    fixup_label - callback to fixup forward-
    referenced labels
-------------------------------------------------*/

static void fixup_label(void *parameter, drccodeptr labelcodeptr)
{
	drcbec_instruction *dst = (drcbec_instruction *)parameter;
	dst->inst = (drcbec_instruction *)labelcodeptr;
}


/*-------------------------------------------------
    dmulu - perform a double-wide unsigned multiply
-------------------------------------------------*/

static int dmulu(UINT64 *dstlo, UINT64 *dsthi, UINT64 src1, UINT64 src2, int flags)
{
	UINT64 lo, hi, prevlo;
	UINT64 a, b, temp;

	/* shortcut if we don't care about the high bits or the flags */
	if (dstlo == dsthi && flags == 0)
	{
		*dstlo = src1 * src2;
		return 0;
	}

	/* fetch source values */
	a = src1;
	b = src2;
	if (a == 0 || b == 0)
	{
		*dsthi = *dstlo = 0;
		return DRCUML_FLAG_Z;
	}

	/* compute high and low parts first */
	lo = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 0);
	hi = (UINT64)(UINT32)(a >> 32) * (UINT64)(UINT32)(b >> 32);

	/* compute middle parts */
	prevlo = lo;
	temp = (UINT64)(UINT32)(a >> 32)  * (UINT64)(UINT32)(b >> 0);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	prevlo = lo;
	temp = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 32);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	/* store the results */
	*dsthi = hi;
	*dstlo = lo;
	return ((hi >> 60) & DRCUML_FLAG_S) | ((*dsthi != 0) << 1);
}


/*-------------------------------------------------
    dmuls - perform a double-wide signed multiply
-------------------------------------------------*/

static int dmuls(UINT64 *dstlo, UINT64 *dsthi, INT64 src1, INT64 src2, int flags)
{
	UINT64 lo, hi, prevlo;
	UINT64 a, b, temp;

	/* shortcut if we don't care about the high bits or the flags */
	if (dstlo == dsthi && flags == 0)
	{
		*dstlo = src1 * src2;
		return 0;
	}

	/* fetch absolute source values */
	a = src1; if ((INT64)a < 0) a = -a;
	b = src2; if ((INT64)b < 0) b = -b;
	if (a == 0 || b == 0)
	{
		*dsthi = *dstlo = 0;
		return DRCUML_FLAG_Z;
	}

	/* compute high and low parts first */
	lo = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 0);
	hi = (UINT64)(UINT32)(a >> 32) * (UINT64)(UINT32)(b >> 32);

	/* compute middle parts */
	prevlo = lo;
	temp = (UINT64)(UINT32)(a >> 32)  * (UINT64)(UINT32)(b >> 0);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	prevlo = lo;
	temp = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 32);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	/* adjust for signage */
	if ((INT64)(src1 ^ src2) < 0)
	{
		hi = ~hi + (lo == 0);
		lo = ~lo + 1;
	}

	/* store the results */
	*dsthi = hi;
	*dstlo = lo;
	return ((hi >> 60) & DRCUML_FLAG_S) | ((*dsthi != ((INT64)lo >> 63)) << 1);
}
