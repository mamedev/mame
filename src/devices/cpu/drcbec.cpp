// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbec.c

    Interpreted C core back-end for the universal machine language.

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "drcbec.h"

#include <cmath>

using namespace uml;


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// define a bit to match each possible condition, starting at bit 12
#define ZBIT            (0x1000 << (COND_Z & 15))
#define NZBIT           (0x1000 << (COND_NZ & 15))
#define SBIT            (0x1000 << (COND_S & 15))
#define NSBIT           (0x1000 << (COND_NS & 15))
#define CBIT            (0x1000 << (COND_C & 15))
#define NCBIT           (0x1000 << (COND_NC & 15))
#define VBIT            (0x1000 << (COND_V & 15))
#define NVBIT           (0x1000 << (COND_NV & 15))
#define UBIT            (0x1000 << (COND_U & 15))
#define NUBIT           (0x1000 << (COND_NU & 15))
#define ABIT            (0x1000 << (COND_A & 15))
#define BEBIT           (0x1000 << (COND_BE & 15))
#define GBIT            (0x1000 << (COND_G & 15))
#define GEBIT           (0x1000 << (COND_GE & 15))
#define LBIT            (0x1000 << (COND_L & 15))
#define LEBIT           (0x1000 << (COND_LE & 15))


// internal opcodes
enum
{
	OP_LOAD1 = OP_MAX,
	OP_LOAD1x2,
	OP_LOAD1x4,
	OP_LOAD1x8,
	OP_LOAD2x1,
	OP_LOAD2,
	OP_LOAD2x4,
	OP_LOAD2x8,
	OP_LOAD4x1,
	OP_LOAD4x2,
	OP_LOAD4,
	OP_LOAD4x8,
	OP_LOAD8x1,
	OP_LOAD8x2,
	OP_LOAD8x4,
	OP_LOAD8,
	OP_LOADS1,
	OP_LOADS1x2,
	OP_LOADS1x4,
	OP_LOADS1x8,
	OP_LOADS2x1,
	OP_LOADS2,
	OP_LOADS2x4,
	OP_LOADS2x8,
	OP_LOADS4x1,
	OP_LOADS4x2,
	OP_LOADS4,
	OP_LOADS4x8,
	OP_LOADS8x1,
	OP_LOADS8x2,
	OP_LOADS8x4,
	OP_LOADS8,
	OP_STORE1,
	OP_STORE1x2,
	OP_STORE1x4,
	OP_STORE1x8,
	OP_STORE2x1,
	OP_STORE2,
	OP_STORE2x4,
	OP_STORE2x8,
	OP_STORE4x1,
	OP_STORE4x2,
	OP_STORE4,
	OP_STORE4x8,
	OP_STORE8x1,
	OP_STORE8x2,
	OP_STORE8x4,
	OP_STORE8,
	OP_READ1,
	OP_READ2,
	OP_READ4,
	OP_READ8,
	OP_READM1,
	OP_READM2,
	OP_READM4,
	OP_READM8,
	OP_WRITE1,
	OP_WRITE2,
	OP_WRITE4,
	OP_WRITE8,
	OP_WRITEM1,
	OP_WRITEM2,
	OP_WRITEM4,
	OP_WRITEM8,
	OP_SEXT1,
	OP_SEXT2,
	OP_SEXT4,
	OP_SEXT8,
	OP_FTOI4T,
	OP_FTOI4R,
	OP_FTOI4C,
	OP_FTOI4F,
	OP_FTOI4,
	OP_FTOI8T,
	OP_FTOI8R,
	OP_FTOI8C,
	OP_FTOI8F,
	OP_FTOI8,
	OP_FFRI4,
	OP_FFRI8,
	OP_FFRFS,
	OP_FFRFD
};



//**************************************************************************
//  MACROS
//**************************************************************************

//
// opcode format:
//
//  bits 31..28 == number of words following the opcode itself (0-15)
//  bits 27..12 == bitmask specify which condition code we care about
//  bits 11.. 2 == opcode
//  bit       1 == flags/condition summary (0 if no condition/flags, 1 otherwise)
//  bit       0 == operation size (0=32-bit, 1=64-bit)
//

// build a short opcode from the raw opcode and size
#define MAKE_OPCODE_SHORT(op, size, conditionorflags) \
	((((size) == 8) << 0) | (((conditionorflags) != 0) << 1) | ((op) << 2))

// build a full opcode from the raw opcode, size, condition/flags, and immediate count
#define MAKE_OPCODE_FULL(op, size, condition, flags, pwords) \
	(MAKE_OPCODE_SHORT(op, size, (condition | flags)) | ((condition != COND_ALWAYS) ? (0x1000 << ((condition) & 15)) : 0) | ((pwords) << 28))

// extract various parts of the opcode
#define OPCODE_GET_SHORT(op)        ((op) & 0xfff)
#define OPCODE_PASS_CONDITION(op,f) (((op) & s_condition_map[f]) != 0)
#define OPCODE_FAIL_CONDITION(op,f) (((op) & s_condition_map[f]) == 0)
#define OPCODE_GET_PWORDS(op)       ((op) >> 28)

// shorthand for accessing parameters in the instruction stream
#define PARAM0                      (*inst[0].puint32)
#define PARAM1                      (*inst[1].puint32)
#define PARAM2                      (*inst[2].puint32)
#define PARAM3                      (*inst[3].puint32)

#define DPARAM0                     (*inst[0].puint64)
#define DPARAM1                     (*inst[1].puint64)
#define DPARAM2                     (*inst[2].puint64)
#define DPARAM3                     (*inst[3].puint64)

#define FSPARAM0                    (*inst[0].pfloat)
#define FSPARAM1                    (*inst[1].pfloat)
#define FSPARAM2                    (*inst[2].pfloat)
#define FSPARAM3                    (*inst[3].pfloat)

#define FDPARAM0                    (*inst[0].pdouble)
#define FDPARAM1                    (*inst[1].pdouble)
#define FDPARAM2                    (*inst[2].pdouble)
#define FDPARAM3                    (*inst[3].pdouble)

// compute C and V flags for 32-bit add/subtract
#define FLAGS32_C_ADD(a,b)          ((UINT32)~(a) < (UINT32)(b))
#define FLAGS32_C_SUB(a,b)          ((UINT32)(b) > (UINT32)(a))
#define FLAGS32_V_SUB(r,a,b)        (((((a) ^ (b)) & ((a) ^ (r))) >> 30) & FLAG_V)
#define FLAGS32_V_ADD(r,a,b)        (((~((a) ^ (b)) & ((a) ^ (r))) >> 30) & FLAG_V)

// compute N and Z flags for 32-bit operations
#define FLAGS32_NZ(v)               ((((v) >> 28) & FLAG_S) | (((UINT32)(v) == 0) << 2))
#define FLAGS32_NZCV_ADD(r,a,b)     (FLAGS32_NZ(r) | FLAGS32_C_ADD(a,b) | FLAGS32_V_ADD(r,a,b))
#define FLAGS32_NZCV_SUB(r,a,b)     (FLAGS32_NZ(r) | FLAGS32_C_SUB(a,b) | FLAGS32_V_SUB(r,a,b))

// compute C and V flags for 64-bit add/subtract
#define FLAGS64_C_ADD(a,b)          ((UINT64)~(a) < (UINT64)(b))
#define FLAGS64_C_SUB(a,b)          ((UINT64)(b) > (UINT64)(a))
#define FLAGS64_V_SUB(r,a,b)        (((((a) ^ (b)) & ((a) ^ (r))) >> 62) & FLAG_V)
#define FLAGS64_V_ADD(r,a,b)        (((~((a) ^ (b)) & ((a) ^ (r))) >> 62) & FLAG_V)

// compute N and Z flags for 64-bit operations
#define FLAGS64_NZ(v)               ((((v) >> 60) & FLAG_S) | (((UINT64)(v) == 0) << 2))
#define FLAGS64_NZCV_ADD(r,a,b)     (FLAGS64_NZ(r) | FLAGS64_C_ADD(a,b) | FLAGS64_V_ADD(r,a,b))
#define FLAGS64_NZCV_SUB(r,a,b)     (FLAGS64_NZ(r) | FLAGS64_C_SUB(a,b) | FLAGS64_V_SUB(r,a,b))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// union to simplify accessing data via the instruction stream
union drcbec_instruction
{
	UINT32              i;
	void *              v;
	char *              c;
	UINT8 *             puint8;
	INT8 *              pint8;
	UINT16 *            puint16;
	INT16 *             pint16;
	UINT32 *            puint32;
	INT32 *             pint32;
	UINT64 *            puint64;
	INT64 *             pint64;
	float *             pfloat;
	double *            pdouble;
	void                (*cfunc)(void *);
	drcuml_machine_state *state;
	const code_handle * handle;
	const drcbec_instruction *inst;
	const drcbec_instruction **pinst;
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

UINT64 drcbe_c::s_immediate_zero = 0;

const UINT32 drcbe_c::s_condition_map[] =
{
	/* ..... */     NCBIT | NVBIT | NZBIT | NSBIT | NUBIT | ABIT  | GBIT  | GEBIT,
	/* ....C */     CBIT  | NVBIT | NZBIT | NSBIT | NUBIT | BEBIT | GBIT  | GEBIT,
	/* ...V. */     NCBIT | VBIT  | NZBIT | NSBIT | NUBIT | ABIT  | LEBIT | LBIT,
	/* ...VC */     CBIT  | VBIT  | NZBIT | NSBIT | NUBIT | BEBIT | LEBIT | LBIT,
	/* ..Z.. */     NCBIT | NVBIT | ZBIT  | NSBIT | NUBIT | BEBIT | LEBIT | GEBIT,
	/* ..Z.C */     CBIT  | NVBIT | ZBIT  | NSBIT | NUBIT | BEBIT | LEBIT | GEBIT,
	/* ..ZV. */     NCBIT | VBIT  | ZBIT  | NSBIT | NUBIT | BEBIT | LEBIT | LBIT,
	/* ..ZVC */     CBIT  | VBIT  | ZBIT  | NSBIT | NUBIT | BEBIT | LEBIT | LBIT,
	/* .S... */     NCBIT | NVBIT | NZBIT | SBIT  | NUBIT | ABIT  | LEBIT | LBIT,
	/* .S..C */     CBIT  | NVBIT | NZBIT | SBIT  | NUBIT | BEBIT | LEBIT | LBIT,
	/* .S.V. */     NCBIT | VBIT  | NZBIT | SBIT  | NUBIT | ABIT  | GBIT  | GEBIT,
	/* .S.VC */     CBIT  | VBIT  | NZBIT | SBIT  | NUBIT | BEBIT | GBIT  | GEBIT,
	/* .SZ.. */     NCBIT | NVBIT | ZBIT  | SBIT  | NUBIT | BEBIT | LEBIT | LBIT,
	/* .SZ.C */     CBIT  | NVBIT | ZBIT  | SBIT  | NUBIT | BEBIT | LEBIT | LBIT,
	/* .SZV. */     NCBIT | VBIT  | ZBIT  | SBIT  | NUBIT | BEBIT | LEBIT | GEBIT,
	/* .SZVC */     CBIT  | VBIT  | ZBIT  | SBIT  | NUBIT | BEBIT | LEBIT | GEBIT,
	/* U.... */     NCBIT | NVBIT | NZBIT | NSBIT | UBIT  | ABIT  | GBIT  | GEBIT,
	/* U...C */     CBIT  | NVBIT | NZBIT | NSBIT | UBIT  | BEBIT | GBIT  | GEBIT,
	/* U..V. */     NCBIT | VBIT  | NZBIT | NSBIT | UBIT  | ABIT  | LEBIT | LBIT,
	/* U..VC */     CBIT  | VBIT  | NZBIT | NSBIT | UBIT  | BEBIT | LEBIT | LBIT,
	/* U.Z.. */     NCBIT | NVBIT | ZBIT  | NSBIT | UBIT  | BEBIT | LEBIT | GEBIT,
	/* U.Z.C */     CBIT  | NVBIT | ZBIT  | NSBIT | UBIT  | BEBIT | LEBIT | GEBIT,
	/* U.ZV. */     NCBIT | VBIT  | ZBIT  | NSBIT | UBIT  | BEBIT | LEBIT | LBIT,
	/* U.ZVC */     CBIT  | VBIT  | ZBIT  | NSBIT | UBIT  | BEBIT | LEBIT | LBIT,
	/* US... */     NCBIT | NVBIT | NZBIT | SBIT  | UBIT  | ABIT  | LEBIT | LBIT,
	/* US..C */     CBIT  | NVBIT | NZBIT | SBIT  | UBIT  | BEBIT | LEBIT | LBIT,
	/* US.V. */     NCBIT | VBIT  | NZBIT | SBIT  | UBIT  | ABIT  | GBIT  | GEBIT,
	/* US.VC */     CBIT  | VBIT  | NZBIT | SBIT  | UBIT  | BEBIT | GBIT  | GEBIT,
	/* USZ.. */     NCBIT | NVBIT | ZBIT  | SBIT  | UBIT  | BEBIT | LEBIT | LBIT,
	/* USZ.C */     CBIT  | NVBIT | ZBIT  | SBIT  | UBIT  | BEBIT | LEBIT | LBIT,
	/* USZV. */     NCBIT | VBIT  | ZBIT  | SBIT  | UBIT  | BEBIT | LEBIT | GEBIT,
	/* USZVC */     CBIT  | VBIT  | ZBIT  | SBIT  | UBIT  | BEBIT | LEBIT | GEBIT
};



//**************************************************************************
//  C BACKEND
//**************************************************************************

//-------------------------------------------------
//  drcbe_c - constructor
//-------------------------------------------------

drcbe_c::drcbe_c(drcuml_state &drcuml, device_t &device, drc_cache &cache, UINT32 flags, int modes, int addrbits, int ignorebits)
	: drcbe_interface(drcuml, cache, device),
		m_hash(cache, modes, addrbits, ignorebits),
		m_map(cache, 0),
		m_labels(cache),
		m_fixup_delegate(FUNC(drcbe_c::fixup_label), this)
{
}


//-------------------------------------------------
//  ~drcbe_c - destructor
//-------------------------------------------------

drcbe_c::~drcbe_c()
{
}


//-------------------------------------------------
//  reset - reset back-end specific state
//-------------------------------------------------

void drcbe_c::reset()
{
	// reset our hash tables
	m_hash.reset();
	m_hash.set_default_codeptr(nullptr);
}


//-------------------------------------------------
//  drcbec_generate - generate code
//-------------------------------------------------

void drcbe_c::generate(drcuml_block &block, const instruction *instlist, UINT32 numinst)
{
	// tell all of our utility objects that a block is beginning
	m_hash.block_begin(block, instlist, numinst);
	m_labels.block_begin(block);
	m_map.block_begin(block);

	// begin codegen; fail if we can't
	drccodeptr *cachetop = m_cache.begin_codegen(numinst * sizeof(drcbec_instruction) * 4);
	if (cachetop == nullptr)
		block.abort();

	// compute the base by aligning the cache top to an even multiple of drcbec_instruction
	drcbec_instruction *base = (drcbec_instruction *)(((FPTR)*cachetop + sizeof(drcbec_instruction) - 1) & ~(sizeof(drcbec_instruction) - 1));
	drcbec_instruction *dst = base;

	// generate code by copying the instructions and extracting immediates
	for (int inum = 0; inum < numinst; inum++)
	{
		const instruction &inst = instlist[inum];
		UINT8 psize[instruction::MAX_PARAMS];

		// handle most instructions generally, but a few special cases
		opcode_t opcode = inst.opcode();
		switch (opcode)
		{
			// when we hit a HANDLE opcode, register the current pointer for the handle
			case OP_HANDLE:
				inst.param(0).handle().set_codeptr((drccodeptr)dst);
				break;

			// when we hit a HASH opcode, register the current pointer for the mode/PC
			case OP_HASH:
				m_hash.set_codeptr(inst.param(0).immediate(), inst.param(1).immediate(), (drccodeptr)dst);
				break;

			// when we hit a LABEL opcode, register the current pointer for the label
			case OP_LABEL:
				m_labels.set_codeptr(inst.param(0).label(), (drccodeptr)dst);
				break;

			// ignore COMMENT and NOP opcodes
			case OP_COMMENT:
			case OP_NOP:
				break;

			// when we hit a MAPVAR opcode, log the change for the current PC
			case OP_MAPVAR:
				m_map.set_value((drccodeptr)dst, inst.param(0).mapvar(), inst.param(1).immediate());
				break;

			// JMP instructions need to resolve their labels
			case OP_JMP:
				(dst++)->i = MAKE_OPCODE_FULL(opcode, inst.size(), inst.condition(), inst.flags(), 1);
				dst->inst = (drcbec_instruction *)m_labels.get_codeptr(inst.param(0).label(), m_fixup_delegate, dst);
				dst++;
				break;

			// generically handle everything else
			default:

				// determine the operand size for each operand; mostly this is just the instruction size
				for (int pnum = 0; pnum < inst.numparams(); pnum++)
					psize[pnum] = inst.size();
				if (opcode == OP_LOAD || opcode == OP_FLOAD)
					psize[2] = 4;
				if (opcode == OP_STORE || opcode == OP_FSTORE)
					psize[1] = 4;
				if (opcode == OP_READ || opcode == OP_READM || opcode == OP_FREAD)
					psize[1] = psize[2] = 4;
				if (opcode == OP_WRITE || opcode == OP_WRITEM || opcode == OP_FWRITE)
					psize[0] = psize[2] = 4;
				if (opcode == OP_SEXT && inst.param(2).size() != SIZE_QWORD)
					psize[1] = 4;
				if (opcode == OP_FTOINT)
					psize[0] = 1 << inst.param(2).size();
				if (opcode == OP_FFRINT || opcode == OP_FFRFLT)
					psize[1] = 1 << inst.param(2).size();

				// pre-expand opcodes that encode size/scale in them
				if (opcode == OP_LOAD)
					opcode = (opcode_t)(OP_LOAD1 + inst.param(3).size() * 4 + inst.param(3).scale());
				if (opcode == OP_LOADS)
					opcode = (opcode_t)(OP_LOADS1 + inst.param(3).size() * 4 + inst.param(3).scale());
				if (opcode == OP_STORE)
					opcode = (opcode_t)(OP_STORE1 + inst.param(3).size() * 4 + inst.param(3).scale());
				if (opcode == OP_READ)
					opcode = (opcode_t)(OP_READ1 + inst.param(2).size());
				if (opcode == OP_READM)
					opcode = (opcode_t)(OP_READM1 + inst.param(3).size());
				if (opcode == OP_WRITE)
					opcode = (opcode_t)(OP_WRITE1 + inst.param(2).size());
				if (opcode == OP_WRITEM)
					opcode = (opcode_t)(OP_WRITEM1 + inst.param(3).size());
				if (opcode == OP_SEXT)
					opcode = (opcode_t)(OP_SEXT1 + inst.param(2).size());
				if (opcode == OP_FTOINT)
					opcode = (opcode_t)(OP_FTOI4T + 5 * (inst.param(2).size() - 2) + inst.param(3).rounding());
				if (opcode == OP_FFRINT)
					opcode = (opcode_t)(OP_FFRI4 + (inst.param(2).size() - 2));
				if (opcode == OP_FFRFLT)
					opcode = (opcode_t)(OP_FFRFS + (inst.param(2).size() - 2));

				// count how many bytes of immediates we need
				int immedbytes = 0;
				for (int pnum = 0; pnum < inst.numparams(); pnum++)
					if (inst.param(pnum).is_mapvar() ||
						(inst.param(pnum).is_immediate() && inst.param(pnum).immediate() != 0) ||
						(inst.param(pnum).is_size_space() && inst.param(pnum).space() != 0))
						immedbytes += psize[pnum];

				// compute how many instruction words we need for that
				int immedwords = (immedbytes + sizeof(drcbec_instruction) - 1) / sizeof(drcbec_instruction);

				// first item is the opcode, size, condition flags and length
				(dst++)->i = MAKE_OPCODE_FULL(opcode, inst.size(), inst.condition(), inst.flags(), inst.numparams() + immedwords);

				// immediates start after parameters
				void *immed = dst + inst.numparams();

				// output each of the parameters
				for (int pnum = 0; pnum < inst.numparams(); pnum++)
					output_parameter(&dst, &immed, psize[pnum], inst.param(pnum));

				// point past the end of the immediates
				dst += immedwords;
				break;
		}
	}

	// complete codegen
	*cachetop = (drccodeptr)dst;
	m_cache.end_codegen();

	// tell all of our utility objects that the block is finished
	m_hash.block_end(block);
	m_labels.block_end(block);
	m_map.block_end(block);
}


//-------------------------------------------------
//  hash_exists - return true if the given mode/pc
//  exists in the hash table
//-------------------------------------------------

bool drcbe_c::hash_exists(UINT32 mode, UINT32 pc)
{
	return m_hash.code_exists(mode, pc);
}


//-------------------------------------------------
//  get_info - return information about the
//  back-end implementation
//-------------------------------------------------

void drcbe_c::get_info(drcbe_info &info)
{
	info.direct_iregs = 0;
	info.direct_fregs = 0;
}


//-------------------------------------------------
//  execute - execute a block of code registered
//  at the given mode/pc
//-------------------------------------------------

int drcbe_c::execute(code_handle &entry)
{
	// get the entry point
	const drcbec_instruction *inst = (const drcbec_instruction *)entry.codeptr();
	assert_in_cache(m_cache, inst);

	// loop while we have cycles
	const drcbec_instruction *callstack[32];
	const drcbec_instruction *newinst;
	UINT32 temp32;
	UINT64 temp64;
	int shift;
	UINT8 flags = 0;
	UINT8 sp = 0;
	while (true)
	{
		UINT32 opcode = (inst++)->i;

		switch (OPCODE_GET_SHORT(opcode))
		{
			// ----------------------- Control Flow Operations -----------------------

			case MAKE_OPCODE_SHORT(OP_HANDLE, 4, 0):    // HANDLE  handle
			case MAKE_OPCODE_SHORT(OP_HASH, 4, 0):      // HASH    mode,pc
			case MAKE_OPCODE_SHORT(OP_LABEL, 4, 0):     // LABEL   imm
			case MAKE_OPCODE_SHORT(OP_COMMENT, 4, 0):   // COMMENT string
			case MAKE_OPCODE_SHORT(OP_MAPVAR, 4, 0):    // MAPVAR  mapvar,value

				// these opcodes should be processed at compile-time only
				fatalerror("Unexpected opcode\n");

			case MAKE_OPCODE_SHORT(OP_DEBUG, 4, 0):     // DEBUG   pc
				debugger_instruction_hook(&m_device, PARAM0);
				break;

			case MAKE_OPCODE_SHORT(OP_HASHJMP, 4, 0):   // HASHJMP mode,pc,handle
				sp = 0;
				newinst = (const drcbec_instruction *)m_hash.get_codeptr(PARAM0, PARAM1);
				if (newinst == nullptr)
				{
					assert(sp < ARRAY_LENGTH(callstack));
					m_state.exp = PARAM1;
					newinst = (const drcbec_instruction *)inst[2].handle->codeptr();
					callstack[sp++] = inst;
				}
				assert_in_cache(m_cache, newinst);
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(OP_EXIT, 4, 1):      // EXIT    src1[,c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_EXIT, 4, 0):
				return PARAM0;

			case MAKE_OPCODE_SHORT(OP_JMP, 4, 1):       // JMP     imm[,c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_JMP, 4, 0):
				newinst = inst[0].inst;
				assert_in_cache(m_cache, newinst);
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(OP_CALLH, 4, 1):     // CALLH   handle[,c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_CALLH, 4, 0):
				assert(sp < ARRAY_LENGTH(callstack));
				newinst = (const drcbec_instruction *)inst[0].handle->codeptr();
				assert_in_cache(m_cache, newinst);
				callstack[sp++] = inst + OPCODE_GET_PWORDS(opcode);
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(OP_RET, 4, 1):       // RET     [c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_RET, 4, 0):
				assert(sp > 0);
				newinst = callstack[--sp];
				assert_in_cache(m_cache, newinst);
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(OP_EXH, 4, 1):       // EXH     handle,param[,c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_EXH, 4, 0):
				assert(sp < ARRAY_LENGTH(callstack));
				newinst = (const drcbec_instruction *)inst[0].handle->codeptr();
				assert_in_cache(m_cache, newinst);
				m_state.exp = PARAM1;
				callstack[sp++] = inst;
				inst = newinst;
				continue;

			case MAKE_OPCODE_SHORT(OP_CALLC, 4, 1):     // CALLC   func,ptr[,c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_CALLC, 4, 0):
				(*inst[0].cfunc)(inst[1].v);
				break;

			case MAKE_OPCODE_SHORT(OP_RECOVER, 4, 0):   // RECOVER dst,mapvar
				assert(sp > 0);
				PARAM0 = m_map.get_value((drccodeptr)callstack[0], MAPVAR_M0 + PARAM1);
				break;


			// ----------------------- Internal Register Operations -----------------------

			case MAKE_OPCODE_SHORT(OP_SETFMOD, 4, 0):   // SETFMOD src
				m_state.fmod = PARAM0;
				break;

			case MAKE_OPCODE_SHORT(OP_GETFMOD, 4, 0):   // GETFMOD dst
				PARAM0 = m_state.fmod;
				break;

			case MAKE_OPCODE_SHORT(OP_GETEXP, 4, 0):    // GETEXP  dst
				PARAM0 = m_state.exp;
				break;

			case MAKE_OPCODE_SHORT(OP_GETFLGS, 4, 0):   // GETFLGS dst[,f]
				PARAM0 = flags & PARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_SAVE, 4, 0):      // SAVE    dst
				*inst[0].state = m_state;
				inst[0].state->flags = flags;
				break;

			case MAKE_OPCODE_SHORT(OP_RESTORE, 4, 0):   // RESTORE dst
			case MAKE_OPCODE_SHORT(OP_RESTORE, 4, 1):   // RESTORE dst
				m_state = *inst[0].state;
				flags = inst[0].state->flags;
				break;


			// ----------------------- 32-Bit Integer Operations -----------------------

			case MAKE_OPCODE_SHORT(OP_LOAD1, 4, 0):     // LOAD    dst,base,index,BYTE
				PARAM0 = inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD1x2, 4, 0):   // LOAD    dst,base,index,BYTE_x2
				PARAM0 = *(UINT8 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD1x4, 4, 0):   // LOAD    dst,base,index,BYTE_x4
				PARAM0 = *(UINT8 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD1x8, 4, 0):   // LOAD    dst,base,index,BYTE_x8
				PARAM0 = *(UINT8 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD2x1, 4, 0):   // LOAD    dst,base,index,WORD_x1
				PARAM0 = *(UINT16 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD2, 4, 0):     // LOAD    dst,base,index,WORD
				PARAM0 = inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD2x4, 4, 0):   // LOAD    dst,base,index,WORD_x4
				PARAM0 = *(UINT16 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD2x8, 4, 0):   // LOAD    dst,base,index,WORD_x8
				PARAM0 = *(UINT16 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD4x1, 4, 0):   // LOAD    dst,base,index,DWORD_x1
				PARAM0 = *(UINT32 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD4x2, 4, 0):   // LOAD    dst,base,index,DWORD_x2
				PARAM0 = *(UINT32 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD4, 4, 0):     // LOAD    dst,base,index,DWORD
				PARAM0 = inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD4x8, 4, 0):   // LOAD    dst,base,index,DWORD_x8
				PARAM0 = *(UINT32 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS1, 4, 0):    // LOADS   dst,base,index,BYTE
				PARAM0 = inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS1x2, 4, 0):  // LOADS   dst,base,index,BYTE_x2
				PARAM0 = *(INT8 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS1x4, 4, 0):  // LOADS   dst,base,index,BYTE_x4
				PARAM0 = *(INT8 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS1x8, 4, 0):  // LOADS   dst,base,index,BYTE_x8
				PARAM0 = *(INT8 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS2x1, 4, 0):  // LOADS   dst,base,index,WORD_x1
				PARAM0 = *(INT16 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS2, 4, 0):    // LOADS   dst,base,index,WORD
				PARAM0 = inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS2x4, 4, 0):  // LOADS   dst,base,index,WORD_x4
				PARAM0 = *(INT16 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS2x8, 4, 0):  // LOADS   dst,base,index,WORD_x8
				PARAM0 = *(INT16 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS4x1, 4, 0):  // LOADS   dst,base,index,DWORD_x1
				PARAM0 = *(INT32 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS4x2, 4, 0):  // LOADS   dst,base,index,DWORD_x2
				PARAM0 = *(INT32 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS4, 4, 0):    // LOADS   dst,base,index,DWORD
				PARAM0 = inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS4x8, 4, 0):  // LOADS   dst,base,index,DWORD_x8
				PARAM0 = *(INT32 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_STORE1, 4, 0):    // STORE   dst,base,index,BYTE
				inst[0].puint8[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE1x2, 4, 0):  // STORE   dst,base,index,BYTE_x2
				*(UINT8 *)&inst[0].puint16[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE1x4, 4, 0):  // STORE   dst,base,index,BYTE_x4
				*(UINT8 *)&inst[0].puint32[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE1x8, 4, 0):  // STORE   dst,base,index,BYTE_x8
				*(UINT8 *)&inst[0].puint64[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE2x1, 4, 0):  // STORE   dst,base,index,WORD_x1
				*(UINT16 *)&inst[0].puint8[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE2, 4, 0):    // STORE   dst,base,index,WORD
				inst[0].puint16[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE2x4, 4, 0):  // STORE   dst,base,index,WORD_x4
				*(UINT16 *)&inst[0].puint32[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE2x8, 4, 0):  // STORE   dst,base,index,WORD_x8
				*(UINT16 *)&inst[0].puint64[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE4x1, 4, 0):  // STORE   dst,base,index,DWORD_x1
				*(UINT32 *)&inst[0].puint8[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE4x2, 4, 0):  // STORE   dst,base,index,DWORD_x2
				*(UINT32 *)&inst[0].puint16[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE4, 4, 0):    // STORE   dst,base,index,DWORD
				inst[0].puint32[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE4x8, 4, 0):  // STORE   dst,base,index,DWORD_x8
				*(UINT32 *)&inst[0].puint64[PARAM1] = PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_READ1, 4, 0):     // READ    dst,src1,space_BYTE
				PARAM0 = m_space[PARAM2]->read_byte(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_READ2, 4, 0):     // READ    dst,src1,space_WORD
				PARAM0 = m_space[PARAM2]->read_word(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_READ4, 4, 0):     // READ    dst,src1,space_DWORD
				PARAM0 = m_space[PARAM2]->read_dword(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_READM2, 4, 0):    // READM   dst,src1,mask,space_WORD
				PARAM0 = m_space[PARAM3]->read_word(PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_READM4, 4, 0):    // READM   dst,src1,mask,space_DWORD
				PARAM0 = m_space[PARAM3]->read_dword(PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITE1, 4, 0):    // WRITE   dst,src1,space_BYTE
				m_space[PARAM2]->write_byte(PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITE2, 4, 0):    // WRITE   dst,src1,space_WORD
				m_space[PARAM2]->write_word(PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITE4, 4, 0):    // WRITE   dst,src1,space_DWORD
				m_space[PARAM2]->write_dword(PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITEM2, 4, 0):   // WRITEM  dst,src1,mask,space_WORD
				m_space[PARAM3]->write_word(PARAM0, PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITEM4, 4, 0):   // WRITEM  dst,src1,mask,space_DWORD
				m_space[PARAM3]->write_dword(PARAM0, PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_CARRY, 4, 1):     // CARRY   src,bitnum
				flags = (flags & ~FLAG_C) | ((PARAM0 >> (PARAM1 & 31)) & FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(OP_MOV, 4, 1):       // MOV     dst,src[,c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_MOV, 4, 0):
				PARAM0 = PARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_SET, 4, 1):       // SET     dst,c
				PARAM0 = OPCODE_FAIL_CONDITION(opcode, flags) ? 0 : 1;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT1, 4, 0):     // SEXT1   dst,src
				PARAM0 = (INT8)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT1, 4, 1):
				temp32 = (INT8)PARAM1;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT2, 4, 0):     // SEXT2   dst,src
				PARAM0 = (INT16)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT2, 4, 1):
				temp32 = (INT16)PARAM1;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_ROLAND, 4, 0):    // ROLAND  dst,src,count,mask[,f]
				shift = PARAM2 & 31;
				PARAM0 = ((PARAM1 << shift) | (PARAM1 >> (32 - shift))) & PARAM3;
				break;

			case MAKE_OPCODE_SHORT(OP_ROLAND, 4, 1):
				shift = PARAM2 & 31;
				temp32 = ((PARAM1 << shift) | (PARAM1 >> (32 - shift))) & PARAM3;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_ROLINS, 4, 0):    // ROLINS  dst,src,count,mask[,f]
				shift = PARAM2 & 31;
				PARAM0 = (PARAM0 & ~PARAM3) | (((PARAM1 << shift) | (PARAM1 >> (32 - shift))) & PARAM3);
				break;

			case MAKE_OPCODE_SHORT(OP_ROLINS, 4, 1):
				shift = PARAM2 & 31;
				temp32 = (PARAM0 & ~PARAM3) | (((PARAM1 << shift) | (PARAM1 >> (32 - shift))) & PARAM3);
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_ADD, 4, 0):       // ADD     dst,src1,src2[,f]
				PARAM0 = PARAM1 + PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_ADD, 4, 1):
				temp32 = PARAM1 + PARAM2;
				flags = FLAGS32_NZCV_ADD(temp32, PARAM1, PARAM2);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_ADDC, 4, 0):      // ADDC    dst,src1,src2[,f]
				PARAM0 = PARAM1 + PARAM2 + (flags & FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(OP_ADDC, 4, 1):
				temp32 = PARAM1 + PARAM2 + (flags & FLAG_C);
				if (PARAM2 + 1 != 0)
					flags = FLAGS32_NZCV_ADD(temp32, PARAM1, PARAM2 + (flags & FLAG_C));
				else
				{
					if ((PARAM2 == 0xffffffff) && (flags & FLAG_C))
					{
						flags = FLAGS32_NZCV_ADD(temp32, PARAM1 + (flags & FLAG_C), PARAM2);
						flags |= FLAG_C;
					}
					else
						flags = FLAGS32_NZCV_ADD(temp32, PARAM1 + (flags & FLAG_C), PARAM2);
				}
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_SUB, 4, 0):       // SUB     dst,src1,src2[,f]
				PARAM0 = PARAM1 - PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_SUB, 4, 1):
				temp32 = PARAM1 - PARAM2;
				flags = FLAGS32_NZCV_SUB(temp32, PARAM1, PARAM2);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_SUBB, 4, 0):      // SUBB    dst,src1,src2[,f]
				PARAM0 = PARAM1 - PARAM2 - (flags & FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(OP_SUBB, 4, 1):
				temp32 = PARAM1 - PARAM2 - (flags & FLAG_C);
				temp64 = (UINT64)PARAM1 - (UINT64)PARAM2 - (UINT64)(flags & FLAG_C);
				if (PARAM2 + 1 != 0)
					flags = FLAGS32_NZCV_SUB(temp32, PARAM1, PARAM2 + (flags & FLAG_C));
				else
				{
					flags = FLAGS32_NZCV_SUB(temp32, PARAM1 - (flags & FLAG_C), PARAM2);
					flags &= ~(FLAG_C | FLAG_V);
					flags |= ((temp64>>32) & 1) ? FLAG_C : 0;
					flags |= (((PARAM1) ^ (PARAM2)) & ((PARAM1) ^ (temp64)) & 0x80000000) ? FLAG_V : 0;
				}
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_CMP, 4, 1):       // CMP     src1,src2[,f]
				temp32 = PARAM0 - PARAM1;
				flags = FLAGS32_NZCV_SUB(temp32, PARAM0, PARAM1);
//                printf("CMP: %08x - %08x = flags %x\n", PARAM0, PARAM1, flags);
				break;

			case MAKE_OPCODE_SHORT(OP_MULU, 4, 0):      // MULU    dst,edst,src1,src2[,f]
				temp64 = (UINT64)(UINT32)PARAM2 * (UINT64)(UINT32)PARAM3;
				PARAM1 = temp64 >> 32;
				PARAM0 = (UINT32)temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_MULU, 4, 1):
				temp64 = (UINT64)(UINT32)PARAM2 * (UINT64)(UINT32)PARAM3;
				flags = FLAGS64_NZ(temp64);
				PARAM1 = temp64 >> 32;
				PARAM0 = (UINT32)temp64;
				if (temp64 != (UINT32)temp64)
					flags |= FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(OP_MULS, 4, 0):      // MULS    dst,edst,src1,src2[,f]
				temp64 = (INT64)(INT32)PARAM2 * (INT64)(INT32)PARAM3;
				PARAM1 = temp64 >> 32;
				PARAM0 = (UINT32)temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_MULS, 4, 1):
				temp64 = (INT64)(INT32)PARAM2 * (INT64)(INT32)PARAM3;
				temp32 = (INT32)temp64;
				flags = FLAGS32_NZ(temp32);
				PARAM1 = temp64 >> 32;
				PARAM0 = (UINT32)temp64;
				if (temp64 != (INT32)temp64)
					flags |= FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(OP_DIVU, 4, 0):      // DIVU    dst,edst,src1,src2[,f]
				if (PARAM3 != 0)
				{
					temp32 = (UINT32)PARAM2 / (UINT32)PARAM3;
					PARAM1 = (UINT32)PARAM2 % (UINT32)PARAM3;
					PARAM0 = temp32;
				}
				break;

			case MAKE_OPCODE_SHORT(OP_DIVU, 4, 1):
				if (PARAM3 != 0)
				{
					temp32 = (UINT32)PARAM2 / (UINT32)PARAM3;
					PARAM1 = (UINT32)PARAM2 % (UINT32)PARAM3;
					flags = FLAGS32_NZ(temp32);
					PARAM0 = temp32;
				}
				else
					flags = FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(OP_DIVS, 4, 0):      // DIVS    dst,edst,src1,src2[,f]
				if (PARAM3 != 0)
				{
					temp32 = (INT32)PARAM2 / (INT32)PARAM3;
					PARAM1 = (INT32)PARAM2 % (INT32)PARAM3;
					PARAM0 = temp32;
				}
				break;

			case MAKE_OPCODE_SHORT(OP_DIVS, 4, 1):
				if (PARAM3 != 0)
				{
					temp32 = (INT32)PARAM2 / (INT32)PARAM3;
					PARAM1 = (INT32)PARAM2 % (INT32)PARAM3;
					flags = FLAGS32_NZ(temp32);
					PARAM0 = temp32;
				}
				else
					flags = FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(OP_AND, 4, 0):       // AND     dst,src1,src2[,f]
				PARAM0 = PARAM1 & PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_AND, 4, 1):
				temp32 = PARAM1 & PARAM2;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_TEST, 4, 1):      // TEST    src1,src2[,f]
				temp32 = PARAM0 & PARAM1;
				flags = FLAGS32_NZ(temp32);
				break;

			case MAKE_OPCODE_SHORT(OP_OR, 4, 0):        // OR      dst,src1,src2[,f]
				PARAM0 = PARAM1 | PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_OR, 4, 1):
				temp32 = PARAM1 | PARAM2;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_XOR, 4, 0):       // XOR     dst,src1,src2[,f]
				PARAM0 = PARAM1 ^ PARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_XOR, 4, 1):
				temp32 = PARAM1 ^ PARAM2;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_LZCNT, 4, 0):     // LZCNT   dst,src
				PARAM0 = count_leading_zeros(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_LZCNT, 4, 1):
				temp32 = count_leading_zeros(PARAM1);
				flags = FLAGS32_NZ(temp32);
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_BSWAP, 4, 0):     // BSWAP   dst,src
				temp32 = PARAM1;
				PARAM0 = FLIPENDIAN_INT32(temp32);
				break;

			case MAKE_OPCODE_SHORT(OP_BSWAP, 4, 1):
				temp32 = PARAM1;
				flags = FLAGS32_NZ(temp32);
				PARAM0 = FLIPENDIAN_INT32(temp32);
				break;

			case MAKE_OPCODE_SHORT(OP_SHL, 4, 0):       // SHL     dst,src,count[,f]
				PARAM0 = PARAM1 << (PARAM2 & 31);
				break;

			case MAKE_OPCODE_SHORT(OP_SHL, 4, 1):
				shift = PARAM2 & 31;
				temp32 = PARAM1 << shift;
				if (shift != 0)
				{
					flags = FLAGS32_NZ(temp32);
					flags |= ((PARAM1 << (shift - 1)) >> 31) & FLAG_C;
				}
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_SHR, 4, 0):       // SHR     dst,src,count[,f]
				PARAM0 = PARAM1 >> (PARAM2 & 31);
				break;

			case MAKE_OPCODE_SHORT(OP_SHR, 4, 1):
				shift = PARAM2 & 31;
				temp32 = PARAM1 >> shift;
				if (shift != 0)
				{
					flags = FLAGS32_NZ(temp32);
					flags |= (PARAM1 >> (shift - 1)) & FLAG_C;
				}
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_SAR, 4, 0):       // SAR     dst,src,count[,f]
				PARAM0 = (INT32)PARAM1 >> (PARAM2 & 31);
				break;

			case MAKE_OPCODE_SHORT(OP_SAR, 4, 1):
				shift = PARAM2 & 31;
				temp32 = (INT32)PARAM1 >> shift;
				if (shift != 0)
				{
					flags = FLAGS32_NZ(temp32);
					flags |= (PARAM1 >> (shift - 1)) & FLAG_C;
				}
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_ROL, 4, 0):       // ROL     dst,src,count[,f]
				shift = PARAM2 & 31;
				PARAM0 = (PARAM1 << shift) | (PARAM1 >> ((32 - shift) & 31));
				break;

			case MAKE_OPCODE_SHORT(OP_ROL, 4, 1):
				shift = PARAM2 & 31;
				temp32 = (PARAM1 << shift) | (PARAM1 >> ((32 - shift) & 31));
				if (shift != 0)
				{
					flags = FLAGS32_NZ(temp32);
					flags |= ((PARAM1 << (shift - 1)) >> 31) & FLAG_C;
				}
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_ROLC, 4, 0):      // ROLC    dst,src,count[,f]
				shift = PARAM2 & 31;
				if (shift > 1)
					PARAM0 = (PARAM1 << shift) | ((flags & FLAG_C) << (shift - 1)) | (PARAM1 >> (33 - shift));
				else if (shift == 1)
					PARAM0 = (PARAM1 << shift) | (flags & FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(OP_ROLC, 4, 1):
				shift = PARAM2 & 31;
				if (shift > 1)
					temp32 = (PARAM1 << shift) | ((flags & FLAG_C) << (shift - 1)) | (PARAM1 >> (33 - shift));
				else if (shift == 1)
					temp32 = (PARAM1 << shift) | (flags & FLAG_C);
				else
					temp32 = PARAM1;
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= ((PARAM1 << (shift - 1)) >> 31) & FLAG_C;
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_ROR, 4, 0):       // ROR     dst,src,count[,f]
				shift = PARAM2 & 31;
				PARAM0 = (PARAM1 >> shift) | (PARAM1 << ((32 - shift) & 31));
				break;

			case MAKE_OPCODE_SHORT(OP_ROR, 4, 1):
				shift = PARAM2 & 31;
				temp32 = (PARAM1 >> shift) | (PARAM1 << ((32 - shift) & 31));
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= (PARAM1 >> (shift - 1)) & FLAG_C;
				PARAM0 = temp32;
				break;

			case MAKE_OPCODE_SHORT(OP_RORC, 4, 0):      // RORC    dst,src,count[,f]
				shift = PARAM2 & 31;
				if (shift > 1)
					PARAM0 = (PARAM1 >> shift) | (((flags & FLAG_C) << 31) >> (shift - 1)) | (PARAM1 << (33 - shift));
				else if (shift == 1)
					PARAM0 = (PARAM1 >> shift) | ((flags & FLAG_C) << 31);
				break;

			case MAKE_OPCODE_SHORT(OP_RORC, 4, 1):
				shift = PARAM2 & 31;
				if (shift > 1)
					temp32 = (PARAM1 >> shift) | (((flags & FLAG_C) << 31) >> (shift - 1)) | (PARAM1 << (33 - shift));
				else if (shift == 1)
					temp32 = (PARAM1 >> shift) | ((flags & FLAG_C) << 31);
				else
					temp32 = PARAM1;
				flags = FLAGS32_NZ(temp32);
				if (shift != 0) flags |= (PARAM1 >> (shift - 1)) & FLAG_C;
				PARAM0 = temp32;
				break;


			// ----------------------- 64-Bit Integer Operations -----------------------

			case MAKE_OPCODE_SHORT(OP_LOAD1, 8, 0):     // DLOAD   dst,base,index,BYTE
				DPARAM0 = inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD1x2, 8, 0):   // DLOAD   dst,base,index,BYTE_x2
				DPARAM0 = *(UINT8 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD1x4, 8, 0):   // DLOAD   dst,base,index,BYTE_x4
				DPARAM0 = *(UINT8 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD1x8, 8, 0):   // DLOAD   dst,base,index,BYTE_x8
				DPARAM0 = *(UINT8 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD2x1, 8, 0):   // DLOAD   dst,base,index,WORD_x1
				DPARAM0 = *(UINT16 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD2, 8, 0):     // DLOAD   dst,base,index,WORD
				DPARAM0 = inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD2x4, 8, 0):   // DLOAD   dst,base,index,WORD_x4
				DPARAM0 = *(UINT16 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD2x8, 8, 0):   // DLOAD   dst,base,index,WORD_x8
				DPARAM0 = *(UINT16 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD4x1, 8, 0):   // DLOAD   dst,base,index,DWORD_x1
				DPARAM0 = *(UINT32 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD4x2, 8, 0):   // DLOAD   dst,base,index,DWORD_x2
				DPARAM0 = *(UINT32 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD4, 8, 0):     // DLOAD   dst,base,index,DWORD
				DPARAM0 = inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD4x8, 8, 0):   // DLOAD   dst,base,index,DWORD_x8
				DPARAM0 = *(UINT32 *)&inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD8x1, 8, 0):   // DLOAD   dst,base,index,QWORD_x1
				DPARAM0 = *(UINT64 *)&inst[1].puint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD8x2, 8, 0):   // DLOAD   dst,base,index,QWORD_x2
				DPARAM0 = *(UINT64 *)&inst[1].puint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD8x4, 8, 0):   // DLOAD   dst,base,index,QWORD_x4
				DPARAM0 = *(UINT64 *)&inst[1].puint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOAD8, 8, 0):     // DLOAD   dst,base,index,QWORD
				DPARAM0 = inst[1].puint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS1, 8, 0):    // DLOADS  dst,base,index,BYTE
				DPARAM0 = inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS1x2, 8, 0):  // DLOADS  dst,base,index,BYTE_x2
				DPARAM0 = *(INT8 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS1x4, 8, 0):  // DLOADS  dst,base,index,BYTE_x4
				DPARAM0 = *(INT8 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS1x8, 8, 0):  // DLOADS  dst,base,index,BYTE_x8
				DPARAM0 = *(INT8 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS2x1, 8, 0):  // DLOADS  dst,base,index,WORD_x1
				DPARAM0 = *(INT16 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS2, 8, 0):    // DLOADS  dst,base,index,WORD
				DPARAM0 = inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS2x4, 8, 0):  // DLOADS  dst,base,index,WORD_x4
				DPARAM0 = *(INT16 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS2x8, 8, 0):  // DLOADS  dst,base,index,WORD_x8
				DPARAM0 = *(INT16 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS4x1, 8, 0):  // DLOADS  dst,base,index,DWORD_x1
				DPARAM0 = *(INT32 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS4x2, 8, 0):  // DLOADS  dst,base,index,DWORD_x2
				DPARAM0 = *(INT32 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS4, 8, 0):    // DLOADS  dst,base,index,DWORD
				DPARAM0 = inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS4x8, 8, 0):  // DLOADS  dst,base,index,DWORD_x8
				DPARAM0 = *(INT32 *)&inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS8x1, 8, 0):  // DLOADS  dst,base,index,QWORD_x1
				DPARAM0 = *(INT64 *)&inst[1].pint8[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS8x2, 8, 0):  // DLOADS  dst,base,index,QWORD_x2
				DPARAM0 = *(INT64 *)&inst[1].pint16[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS8x4, 8, 0):  // DLOADS  dst,base,index,QWORD_x4
				DPARAM0 = *(INT64 *)&inst[1].pint32[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_LOADS8, 8, 0):    // DLOADS  dst,base,index,QWORD
				DPARAM0 = inst[1].pint64[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_STORE1, 8, 0):    // DSTORE  dst,base,index,BYTE
				inst[0].puint8[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE1x2, 8, 0):  // DSTORE  dst,base,index,BYTE_x2
				*(UINT8 *)&inst[0].puint16[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE1x4, 8, 0):  // DSTORE  dst,base,index,BYTE_x4
				*(UINT8 *)&inst[0].puint32[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE1x8, 8, 0):  // DSTORE  dst,base,index,BYTE_x8
				*(UINT8 *)&inst[0].puint64[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE2x1, 8, 0):  // DSTORE  dst,base,index,WORD_x1
				*(UINT16 *)&inst[0].puint8[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE2, 8, 0):    // DSTORE  dst,base,index,WORD
				inst[0].puint16[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE2x4, 8, 0):  // DSTORE  dst,base,index,WORD_x4
				*(UINT16 *)&inst[0].puint32[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE2x8, 8, 0):  // DSTORE  dst,base,index,WORD_x8
				*(UINT16 *)&inst[0].puint64[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE4x1, 8, 0):  // DSTORE  dst,base,index,DWORD_x1
				*(UINT32 *)&inst[0].puint8[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE4x2, 8, 0):  // DSTORE  dst,base,index,DWORD_x2
				*(UINT32 *)&inst[0].puint16[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE4, 8, 0):    // DSTORE  dst,base,index,DWORD
				inst[0].puint32[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE4x8, 8, 0):  // DSTORE  dst,base,index,DWORD_x8
				*(UINT32 *)&inst[0].puint64[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE8x1, 8, 0):  // DSTORE  dst,base,index,QWORD_x1
				*(UINT64 *)&inst[0].puint8[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE8x2, 8, 0):  // DSTORE  dst,base,index,QWORD_x2
				*(UINT64 *)&inst[0].puint16[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE8x4, 8, 0):  // DSTORE  dst,base,index,QWORD_x4
				*(UINT64 *)&inst[0].puint32[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_STORE8, 8, 0):    // DSTORE  dst,base,index,QWORD
				inst[0].puint64[PARAM1] = DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_READ1, 8, 0):     // DREAD   dst,src1,space_BYTE
				DPARAM0 = m_space[PARAM2]->read_byte(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_READ2, 8, 0):     // DREAD   dst,src1,space_WORD
				DPARAM0 = m_space[PARAM2]->read_word(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_READ4, 8, 0):     // DREAD   dst,src1,space_DWORD
				DPARAM0 = m_space[PARAM2]->read_dword(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_READ8, 8, 0):     // DREAD   dst,src1,space_QOWRD
				DPARAM0 = m_space[PARAM2]->read_qword(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_READM2, 8, 0):    // DREADM  dst,src1,mask,space_WORD
				DPARAM0 = m_space[PARAM3]->read_word(PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_READM4, 8, 0):    // DREADM  dst,src1,mask,space_DWORD
				DPARAM0 = m_space[PARAM3]->read_dword(PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_READM8, 8, 0):    // DREADM  dst,src1,mask,space_QWORD
				DPARAM0 = m_space[PARAM3]->read_qword(PARAM1, PARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITE1, 8, 0):    // DWRITE  dst,src1,space_BYTE
				m_space[PARAM2]->write_byte(PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITE2, 8, 0):    // DWRITE  dst,src1,space_WORD
				m_space[PARAM2]->write_word(PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITE4, 8, 0):    // DWRITE  dst,src1,space_DWORD
				m_space[PARAM2]->write_dword(PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITE8, 8, 0):    // DWRITE  dst,src1,space_QWORD
				m_space[PARAM2]->write_qword(PARAM0, DPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITEM2, 8, 0):   // DWRITEM dst,src1,mask,space_WORD
				m_space[PARAM3]->write_word(PARAM0, DPARAM1, DPARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITEM4, 8, 0):   // DWRITEM dst,src1,mask,space_DWORD
				m_space[PARAM3]->write_dword(PARAM0, DPARAM1, DPARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_WRITEM8, 8, 0):   // DWRITEM dst,src1,mask,space_QWORD
				m_space[PARAM3]->write_qword(PARAM0, DPARAM1, DPARAM2);
				break;

			case MAKE_OPCODE_SHORT(OP_CARRY, 8, 0):     // DCARRY  src,bitnum
				flags = (flags & ~FLAG_C) | ((DPARAM0 >> (DPARAM1 & 63)) & FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(OP_MOV, 8, 1):       // DMOV    dst,src[,c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_MOV, 8, 0):
				DPARAM0 = DPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_SET, 8, 1):       // DSET    dst,c
				DPARAM0 = OPCODE_FAIL_CONDITION(opcode, flags) ? 0 : 1;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT1, 8, 0):     // DSEXT   dst,src,BYTE
				DPARAM0 = (INT8)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT1, 8, 1):
				temp64 = (INT8)PARAM1;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT2, 8, 0):     // DSEXT   dst,src,WORD
				DPARAM0 = (INT16)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT2, 8, 1):
				temp64 = (INT16)PARAM1;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT4, 8, 0):     // DSEXT   dst,src,DWORD
				DPARAM0 = (INT32)PARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_SEXT4, 8, 1):
				temp64 = (INT32)PARAM1;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_ROLAND, 8, 0):    // DROLAND dst,src,count,mask[,f]
				shift = DPARAM2 & 63;
				DPARAM0 = ((DPARAM1 << shift) | (DPARAM1 >> (64 - shift))) & DPARAM3;
				break;

			case MAKE_OPCODE_SHORT(OP_ROLAND, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = ((DPARAM1 << shift) | (DPARAM1 >> (64 - shift))) & DPARAM3;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_ROLINS, 8, 0):    // DROLINS dst,src,count,mask[,f]
				shift = DPARAM2 & 63;
				DPARAM0 = (DPARAM0 & ~DPARAM3) | (((DPARAM1 << shift) | (DPARAM1 >> (64 - shift))) & DPARAM3);
				break;

			case MAKE_OPCODE_SHORT(OP_ROLINS, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = (DPARAM0 & ~DPARAM3) | (((DPARAM1 << shift) | (DPARAM1 >> (64 - shift))) & DPARAM3);
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_ADD, 8, 0):       // DADD    dst,src1,src2[,f]
				DPARAM0 = DPARAM1 + DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_ADD, 8, 1):
				temp64 = DPARAM1 + DPARAM2;
				flags = FLAGS64_NZCV_ADD(temp64, DPARAM1, DPARAM2);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_ADDC, 8, 0):      // DADDC   dst,src1,src2[,f]
				DPARAM0 = DPARAM1 + DPARAM2 + (flags & FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(OP_ADDC, 8, 1):
				temp64 = DPARAM1 + DPARAM2 + (flags & FLAG_C);
				if (DPARAM2 + 1 != 0)
					flags = FLAGS64_NZCV_ADD(temp64, DPARAM1, DPARAM2 + (flags & FLAG_C));
				else
					flags = FLAGS64_NZCV_ADD(temp64, DPARAM1 + (flags & FLAG_C), DPARAM2);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_SUB, 8, 0):       // DSUB    dst,src1,src2[,f]
				DPARAM0 = DPARAM1 - DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_SUB, 8, 1):
				temp64 = DPARAM1 - DPARAM2;
				flags = FLAGS64_NZCV_SUB(temp64, DPARAM1, DPARAM2);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_SUBB, 8, 0):      // DSUBB   dst,src1,src2[,f]
				DPARAM0 = DPARAM1 - DPARAM2 - (flags & FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(OP_SUBB, 8, 1):
				temp64 = DPARAM1 - DPARAM2 - (flags & FLAG_C);
				if (DPARAM2 + 1 != 0)
					flags = FLAGS64_NZCV_SUB(temp64, DPARAM1, DPARAM2 + (flags & FLAG_C));
				else
					flags = FLAGS64_NZCV_SUB(temp64, DPARAM1 - (flags & FLAG_C), DPARAM2);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_CMP, 8, 1):       // DCMP    src1,src2[,f]
				temp64 = DPARAM0 - DPARAM1;
				flags = FLAGS64_NZCV_SUB(temp64, DPARAM0, DPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_MULU, 8, 0):      // DMULU   dst,edst,src1,src2[,f]
				dmulu(*inst[0].puint64, *inst[1].puint64, DPARAM2, DPARAM3, FALSE);
				break;

			case MAKE_OPCODE_SHORT(OP_MULU, 8, 1):
				flags = dmulu(*inst[0].puint64, *inst[1].puint64, DPARAM2, DPARAM3, TRUE);
				break;

			case MAKE_OPCODE_SHORT(OP_MULS, 8, 0):      // DMULS   dst,edst,src1,src2[,f]
				dmuls(*inst[0].puint64, *inst[1].puint64, DPARAM2, DPARAM3, FALSE);
				break;

			case MAKE_OPCODE_SHORT(OP_MULS, 8, 1):
				flags = dmuls(*inst[0].puint64, *inst[1].puint64, DPARAM2, DPARAM3, TRUE);
				break;

			case MAKE_OPCODE_SHORT(OP_DIVU, 8, 0):      // DDIVU   dst,edst,src1,src2[,f]
				if (DPARAM3 != 0)
				{
					temp64 = (UINT64)DPARAM2 / (UINT64)DPARAM3;
					DPARAM1 = (UINT64)DPARAM2 % (UINT64)DPARAM3;
					DPARAM0 = temp64;
				}
				break;

			case MAKE_OPCODE_SHORT(OP_DIVU, 8, 1):
				if (DPARAM3 != 0)
				{
					temp64 = (UINT64)DPARAM2 / (UINT64)DPARAM3;
					DPARAM1 = (UINT64)DPARAM2 % (UINT64)DPARAM3;
					flags = FLAGS64_NZ(temp64);
					DPARAM0 = temp64;
				}
				else
					flags = FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(OP_DIVS, 8, 0):      // DDIVS   dst,edst,src1,src2[,f]
				if (DPARAM3 != 0)
				{
					temp64 = (INT64)DPARAM2 / (INT64)DPARAM3;
					DPARAM1 = (INT64)DPARAM2 % (INT64)DPARAM3;
					DPARAM0 = temp64;
				}
				break;

			case MAKE_OPCODE_SHORT(OP_DIVS, 8, 1):
				if (DPARAM3 != 0)
				{
					temp64 = (INT64)DPARAM2 / (INT64)DPARAM3;
					DPARAM1 = (INT64)DPARAM2 % (INT64)DPARAM3;
					flags = FLAGS64_NZ(temp64);
					DPARAM0 = temp64;
				}
				else
					flags = FLAG_V;
				break;

			case MAKE_OPCODE_SHORT(OP_AND, 8, 0):       // DAND    dst,src1,src2[,f]
				DPARAM0 = DPARAM1 & DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_AND, 8, 1):
				temp64 = DPARAM1 & DPARAM2;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_TEST, 8, 1):      // DTEST   src1,src2[,f]
				temp64 = DPARAM1 & DPARAM2;
				flags = FLAGS64_NZ(temp64);
				break;

			case MAKE_OPCODE_SHORT(OP_OR, 8, 0):        // DOR     dst,src1,src2[,f]
				DPARAM0 = DPARAM1 | DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_OR, 8, 1):
				temp64 = DPARAM1 | DPARAM2;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_XOR, 8, 0):       // DXOR    dst,src1,src2[,f]
				DPARAM0 = DPARAM1 ^ DPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_XOR, 8, 1):
				temp64 = DPARAM1 ^ DPARAM2;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_LZCNT, 8, 0):     // DLZCNT  dst,src
				if ((UINT32)(DPARAM1 >> 32) != 0)
					DPARAM0 = count_leading_zeros(DPARAM1 >> 32);
				else
					DPARAM0 = 32 + count_leading_zeros(DPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_LZCNT, 8, 1):
				if ((UINT32)(DPARAM1 >> 32) != 0)
					temp64 = count_leading_zeros(DPARAM1 >> 32);
				else
					temp64 = 32 + count_leading_zeros(DPARAM1);
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_BSWAP, 8, 0):     // DBSWAP  dst,src
				temp64 = DPARAM1;
				DPARAM0 = FLIPENDIAN_INT64(temp64);
				break;

			case MAKE_OPCODE_SHORT(OP_BSWAP, 8, 1):
				temp64 = DPARAM1;
				flags = FLAGS64_NZ(temp64);
				DPARAM0 = FLIPENDIAN_INT64(temp64);
				break;

			case MAKE_OPCODE_SHORT(OP_SHL, 8, 0):       // DSHL    dst,src,count[,f]
				DPARAM0 = DPARAM1 << (DPARAM2 & 63);
				break;

			case MAKE_OPCODE_SHORT(OP_SHL, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = DPARAM1 << shift;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= ((DPARAM1 << (shift - 1)) >> 63) & FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_SHR, 8, 0):       // DSHR    dst,src,count[,f]
				DPARAM0 = DPARAM1 >> (DPARAM2 & 63);
				break;

			case MAKE_OPCODE_SHORT(OP_SHR, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = DPARAM1 >> shift;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= (DPARAM1 >> (shift - 1)) & FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_SAR, 8, 0):       // DSAR    dst,src,count[,f]
				DPARAM0 = (INT64)DPARAM1 >> (DPARAM2 & 63);
				break;

			case MAKE_OPCODE_SHORT(OP_SAR, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = (INT32)DPARAM1 >> shift;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= (DPARAM1 >> (shift - 1)) & FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_ROL, 8, 0):       // DROL    dst,src,count[,f]
				shift = DPARAM2 & 31;
				DPARAM0 = (DPARAM1 << shift) | (DPARAM1 >> ((64 - shift) & 63));
				break;

			case MAKE_OPCODE_SHORT(OP_ROL, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = (DPARAM1 << shift) | (DPARAM1 >> ((64 - shift) & 63));
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= ((DPARAM1 << (shift - 1)) >> 63) & FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_ROLC, 8, 0):      // DROLC   dst,src,count[,f]
				shift = DPARAM2 & 63;
				if (shift > 1)
					DPARAM0 = (DPARAM1 << shift) | ((flags & FLAG_C) << (shift - 1)) | (DPARAM1 >> (65 - shift));
				else if (shift == 1)
					DPARAM0 = (DPARAM1 << shift) | (flags & FLAG_C);
				break;

			case MAKE_OPCODE_SHORT(OP_ROLC, 8, 1):
				shift = DPARAM2 & 63;
				if (shift > 1)
					temp64 = (DPARAM1 << shift) | ((flags & FLAG_C) << (shift - 1)) | (DPARAM1 >> (65 - shift));
				else if (shift == 1)
					temp64 = (DPARAM1 << shift) | (flags & FLAG_C);
				else
					temp64 = DPARAM1;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= ((DPARAM1 << (shift - 1)) >> 63) & FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_ROR, 8, 0):       // DROR    dst,src,count[,f]
				shift = DPARAM2 & 63;
				DPARAM0 = (DPARAM1 >> shift) | (DPARAM1 << ((64 - shift) & 63));
				break;

			case MAKE_OPCODE_SHORT(OP_ROR, 8, 1):
				shift = DPARAM2 & 63;
				temp64 = (DPARAM1 >> shift) | (DPARAM1 << ((64 - shift) & 63));
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= (DPARAM1 >> (shift - 1)) & FLAG_C;
				DPARAM0 = temp64;
				break;

			case MAKE_OPCODE_SHORT(OP_RORC, 8, 0):      // DRORC   dst,src,count[,f]
				shift = DPARAM2 & 63;
				if (shift > 1)
					DPARAM0 = (DPARAM1 >> shift) | ((((UINT64)flags & FLAG_C) << 63) >> (shift - 1)) | (DPARAM1 << (65 - shift));
				else if (shift == 1)
					DPARAM0 = (DPARAM1 >> shift) | (((UINT64)flags & FLAG_C) << 63);
				break;

			case MAKE_OPCODE_SHORT(OP_RORC, 8, 1):
				shift = DPARAM2 & 63;
				if (shift > 1)
					temp64 = (DPARAM1 >> shift) | ((((UINT64)flags & FLAG_C) << 63) >> (shift - 1)) | (DPARAM1 << (65 - shift));
				else if (shift == 1)
					temp64 = (DPARAM1 >> shift) | (((UINT64)flags & FLAG_C) << 63);
				else
					temp64 = DPARAM1;
				flags = FLAGS64_NZ(temp64);
				if (shift != 0) flags |= (DPARAM1 >> (shift - 1)) & FLAG_C;
				DPARAM0 = temp64;
				break;


			// ----------------------- 32-Bit Floating Point Operations -----------------------

			case MAKE_OPCODE_SHORT(OP_FLOAD, 4, 0):     // FSLOAD  dst,base,index
				FSPARAM0 = inst[1].pfloat[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_FSTORE, 4, 0):    // FSSTORE dst,base,index
				inst[0].pfloat[PARAM1] = FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FREAD, 4, 0):     // FSREAD  dst,src1,space
				PARAM0 = m_space[PARAM2]->read_dword(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FWRITE, 4, 0):    // FSWRITE dst,src1,space
				m_space[PARAM2]->write_dword(PARAM0, PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FMOV, 4, 1):      // FSMOV   dst,src[,c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_FMOV, 4, 0):
				FSPARAM0 = FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4T, 4, 0):    // FSTOI4T dst,src1
				if (FSPARAM1 >= 0)
					*inst[0].pint32 = floor(FSPARAM1);
				else
					*inst[0].pint32 = ceil(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4R, 4, 0):    // FSTOI4R dst,src1
				if (FSPARAM1 >= 0)
					*inst[0].pint32 = floor(FSPARAM1 + 0.5f);
				else
					*inst[0].pint32 = ceil(FSPARAM1 - 0.5f);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4F, 4, 0):    // FSTOI4F dst,src1
				*inst[0].pint32 = floor(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4C, 4, 0):    // FSTOI4C dst,src1
				*inst[0].pint32 = ceil(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4, 4, 0):     // FSTOI4  dst,src1
				*inst[0].pint32 = FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8T, 4, 0):    // FSTOI8T dst,src1
				if (FSPARAM1 >= 0)
					*inst[0].pint64 = floor(FSPARAM1);
				else
					*inst[0].pint64 = ceil(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8R, 4, 0):    // FSTOI8R dst,src1
				if (FSPARAM1 >= 0)
					*inst[0].pint64 = floor(FSPARAM1 + 0.5f);
				else
					*inst[0].pint64 = ceil(FSPARAM1 - 0.5f);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8F, 4, 0):    // FSTOI8F dst,src1
				*inst[0].pint64 = floor(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8C, 4, 0):    // FSTOI8C dst,src1
				*inst[0].pint64 = ceil(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8, 4, 0):     // FSTOI8  dst,src1
				*inst[0].pint64 = FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FFRI4, 4, 0):     // FSFRI4  dst,src1
				FSPARAM0 = *inst[1].pint32;
				break;

			case MAKE_OPCODE_SHORT(OP_FFRI8, 4, 0):     // FSFRI8  dst,src1
				FSPARAM0 = *inst[1].pint64;
				break;

			case MAKE_OPCODE_SHORT(OP_FFRFD, 4, 0):     // FSFRFD  dst,src1
				FSPARAM0 = FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FADD, 4, 0):      // FSADD   dst,src1,src2
				FSPARAM0 = FSPARAM1 + FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FSUB, 4, 0):      // FSSUB   dst,src1,src2
				FSPARAM0 = FSPARAM1 - FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FCMP, 4, 1):      // FSCMP   src1,src2
				if (std::isnan(FSPARAM0) || std::isnan(FSPARAM1))
					flags = FLAG_U;
				else
					flags = (FSPARAM0 < FSPARAM1) | ((FSPARAM0 == FSPARAM1) << 2);
				break;

			case MAKE_OPCODE_SHORT(OP_FMUL, 4, 0):      // FSMUL   dst,src1,src2
				FSPARAM0 = FSPARAM1 * FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FDIV, 4, 0):      // FSDIV   dst,src1,src2
				FSPARAM0 = FSPARAM1 / FSPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FNEG, 4, 0):      // FSNEG   dst,src1
				FSPARAM0 = -FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FABS, 4, 0):      // FSABS   dst,src1
				FSPARAM0 = fabs(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FSQRT, 4, 0):     // FSSQRT  dst,src1
				FSPARAM0 = sqrt(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FRECIP, 4, 0):    // FSRECIP dst,src1
				FSPARAM0 = 1.0f / FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FRSQRT, 4, 0):    // FSRSQRT dst,src1
				FSPARAM0 = 1.0f / sqrtf(FSPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FCOPYI, 4, 0):    // FSCOPYI dst,src
				FSPARAM0 = u2f(*inst[1].pint32);
				break;

			case MAKE_OPCODE_SHORT(OP_ICOPYF, 4, 0):    // ICOPYFS dst,src
				*inst[0].pint32 = f2u(FSPARAM1);
				break;


			// ----------------------- 64-Bit Floating Point Operations -----------------------

			case MAKE_OPCODE_SHORT(OP_FLOAD, 8, 0):     // FDLOAD  dst,base,index
				FDPARAM0 = inst[1].pdouble[PARAM2];
				break;

			case MAKE_OPCODE_SHORT(OP_FSTORE, 8, 0):    // FDSTORE dst,base,index
				inst[0].pdouble[PARAM1] = FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FREAD, 8, 0):     // FDREAD  dst,src1,space
				DPARAM0 = m_space[PARAM2]->read_qword(PARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FWRITE, 8, 0):    // FDWRITE dst,src1,space
				m_space[PARAM2]->write_qword(PARAM0, DPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FMOV, 8, 1):      // FDMOV   dst,src[,c]
				if (OPCODE_FAIL_CONDITION(opcode, flags))
					break;
				// fall through...

			case MAKE_OPCODE_SHORT(OP_FMOV, 8, 0):
				FDPARAM0 = FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4T, 8, 0):    // FDTOI4T dst,src1
				if (FDPARAM1 >= 0)
					*inst[0].pint32 = floor(FDPARAM1);
				else
					*inst[0].pint32 = ceil(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4R, 8, 0):    // FDTOI4R dst,src1
				if (FDPARAM1 >= 0)
					*inst[0].pint32 = floor(FDPARAM1 + 0.5);
				else
					*inst[0].pint32 = ceil(FDPARAM1 - 0.5);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4F, 8, 0):    // FDTOI4F dst,src1
				*inst[0].pint32 = floor(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4C, 8, 0):    // FDTOI4C dst,src1
				*inst[0].pint32 = ceil(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI4, 8, 0):     // FDTOI4  dst,src1
				*inst[0].pint32 = FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8T, 8, 0):    // FDTOI8T dst,src1
				if (FDPARAM1 >= 0)
					*inst[0].pint64 = floor(FDPARAM1);
				else
					*inst[0].pint64 = ceil(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8R, 8, 0):    // FDTOI8R  dst,src1
				if (FDPARAM1 >= 0)
					*inst[0].pint64 = floor(FDPARAM1 + 0.5);
				else
					*inst[0].pint64 = ceil(FDPARAM1 - 0.5);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8F, 8, 0):    // FDTOI8F dst,src1
				*inst[0].pint64 = floor(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8C, 8, 0):    // FDTOI8C dst,src1
				*inst[0].pint64 = ceil(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FTOI8, 8, 0):     // FDTOI8  dst,src1
				*inst[0].pint64 = FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FFRI4, 8, 0):     // FDFRI4  dst,src1
				FDPARAM0 = *inst[1].pint32;
				break;

			case MAKE_OPCODE_SHORT(OP_FFRI8, 8, 0):     // FDFRI8  dst,src1
				FDPARAM0 = *inst[1].pint64;
				break;

			case MAKE_OPCODE_SHORT(OP_FFRFS, 8, 0):     // FDFRFS  dst,src1
				FDPARAM0 = FSPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FRNDS, 8, 0):     // FDRNDS  dst,src1
				FDPARAM0 = (float)FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FADD, 8, 0):      // FDADD   dst,src1,src2
				FDPARAM0 = FDPARAM1 + FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FSUB, 8, 0):      // FDSUB   dst,src1,src2
				FDPARAM0 = FDPARAM1 - FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FCMP, 8, 1):      // FDCMP   src1,src2
				if (std::isnan(FDPARAM0) || std::isnan(FDPARAM1))
					flags = FLAG_U;
				else
					flags = (FDPARAM0 < FDPARAM1) | ((FDPARAM0 == FDPARAM1) << 2);
				break;

			case MAKE_OPCODE_SHORT(OP_FMUL, 8, 0):      // FDMUL   dst,src1,src2
				FDPARAM0 = FDPARAM1 * FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FDIV, 8, 0):      // FDDIV   dst,src1,src2
				FDPARAM0 = FDPARAM1 / FDPARAM2;
				break;

			case MAKE_OPCODE_SHORT(OP_FNEG, 8, 0):      // FDNEG   dst,src1
				FDPARAM0 = -FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FABS, 8, 0):      // FDABS   dst,src1
				FDPARAM0 = fabs(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FSQRT, 8, 0):     // FDSQRT  dst,src1
				FDPARAM0 = sqrt(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FRECIP, 8, 0):    // FDRECIP dst,src1
				FDPARAM0 = 1.0 / FDPARAM1;
				break;

			case MAKE_OPCODE_SHORT(OP_FRSQRT, 8, 0):    // FDRSQRT dst,src1
				FDPARAM0 = 1.0 / sqrt(FDPARAM1);
				break;

			case MAKE_OPCODE_SHORT(OP_FCOPYI, 8, 0):    // FDCOPYI dst,src
				FDPARAM0 = u2d(*inst[1].pint64);
				break;

			case MAKE_OPCODE_SHORT(OP_ICOPYF, 8, 0):    // ICOPYFD dst,src
				*inst[0].pint64 = d2u(FDPARAM1);
				break;

			default:
				fatalerror("Unexpected opcode!\n");
		}

		// advance past the parameters and immediates
		inst += OPCODE_GET_PWORDS(opcode);
	}

	// never executed
	//return 0;
}


//-------------------------------------------------
//  output_parameter - output a parameter
//-------------------------------------------------

void drcbe_c::output_parameter(drcbec_instruction **dstptr, void **immedptr, int size, const parameter &param)
{
	drcbec_instruction *dst = *dstptr;
	void *immed = *immedptr;
	parameter temp_param;

	switch (param.type())
	{
		// immediates store a pointer to the immediate data, which is stored at the end of the instruction
		case parameter::PTYPE_IMMEDIATE:
			if (param.immediate() == 0)
				(dst++)->v = &s_immediate_zero;
			else
			{
				(dst++)->v = immed;
				if (size == 4)
					*(UINT32 *)immed = (UINT32)param.immediate();
				else
					*(UINT64 *)immed = (UINT64)param.immediate();
				immed = (UINT8 *)immed + size;
			}
			break;

		// int registers point to the appropriate part of the integer register state
		case parameter::PTYPE_INT_REGISTER:
			if (size == 4)
				(dst++)->puint32 = &m_state.r[param.ireg() - REG_I0].w.l;
			else
				(dst++)->puint64 = &m_state.r[param.ireg() - REG_I0].d;
			break;

		// float registers point to the appropriate part of the floating point register state
		case parameter::PTYPE_FLOAT_REGISTER:
			if (size == 4)
				(dst++)->pfloat = &m_state.f[param.freg() - REG_F0].s.l;
			else
				(dst++)->pdouble = &m_state.f[param.freg() - REG_F0].d;
			break;

		// convert mapvars to immediates
		case parameter::PTYPE_MAPVAR:
			temp_param = m_map.get_last_value(param.mapvar());
			return output_parameter(dstptr, immedptr, size, temp_param);

		// memory just points to the memory
		case parameter::PTYPE_MEMORY:
			(dst++)->v = param.memory();
			break;

		// ignore these parameters: they are directly encoded in the opcode
		case parameter::PTYPE_SIZE:
		case parameter::PTYPE_SIZE_SCALE:
		case parameter::PTYPE_ROUNDING:
		case parameter::PTYPE_STRING:
			return output_parameter(dstptr, immedptr, size, 0);

		// space/size parameters; sizes are built into our opcodes, but space needs to be encoded
		case parameter::PTYPE_SIZE_SPACE:
			return output_parameter(dstptr, immedptr, size, param.space());

		// code handle just points to the handle
		case parameter::PTYPE_CODE_HANDLE:
			(dst++)->handle = &param.handle();
			break;

		// code label just contains the label value
		case parameter::PTYPE_CODE_LABEL:
			return output_parameter(dstptr, immedptr, size, UINT32(param.label()));

		// c_function just points to the C function
		case parameter::PTYPE_C_FUNCTION:
			(dst++)->cfunc = param.cfunc();
			break;

		default:
			fatalerror("Unexpected param->type\n");
	}

	*dstptr = dst;
	*immedptr = immed;
}


//-------------------------------------------------
//  fixup_label - callback to fixup forward-
//  referenced labels
//-------------------------------------------------

void drcbe_c::fixup_label(void *parameter, drccodeptr labelcodeptr)
{
	drcbec_instruction *dst = (drcbec_instruction *)parameter;
	dst->inst = (drcbec_instruction *)labelcodeptr;
}


//-------------------------------------------------
//  dmulu - perform a double-wide unsigned multiply
//-------------------------------------------------

int drcbe_c::dmulu(UINT64 &dstlo, UINT64 &dsthi, UINT64 src1, UINT64 src2, int flags)
{
	// shortcut if we don't care about the high bits or the flags
	if (&dstlo == &dsthi && flags == 0)
	{
		dstlo = src1 * src2;
		return 0;
	}

	// fetch source values
	UINT64 a = src1;
	UINT64 b = src2;
	if (a == 0 || b == 0)
	{
		dsthi = dstlo = 0;
		return FLAG_Z;
	}

	// compute high and low parts first
	UINT64 lo = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 0);
	UINT64 hi = (UINT64)(UINT32)(a >> 32) * (UINT64)(UINT32)(b >> 32);

	// compute middle parts
	UINT64 prevlo = lo;
	UINT64 temp = (UINT64)(UINT32)(a >> 32)  * (UINT64)(UINT32)(b >> 0);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	prevlo = lo;
	temp = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 32);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	// store the results
	dsthi = hi;
	dstlo = lo;
	return ((hi >> 60) & FLAG_S) | ((dsthi != 0) << 1);
}


//-------------------------------------------------
//  dmuls - perform a double-wide signed multiply
//-------------------------------------------------

int drcbe_c::dmuls(UINT64 &dstlo, UINT64 &dsthi, INT64 src1, INT64 src2, int flags)
{
	// shortcut if we don't care about the high bits or the flags
	if (&dstlo == &dsthi && flags == 0)
	{
		dstlo = src1 * src2;
		return 0;
	}

	// fetch absolute source values
	UINT64 a = src1; if ((INT64)a < 0) a = -a;
	UINT64 b = src2; if ((INT64)b < 0) b = -b;
	if (a == 0 || b == 0)
	{
		dsthi = dstlo = 0;
		return FLAG_Z;
	}

	// compute high and low parts first
	UINT64 lo = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 0);
	UINT64 hi = (UINT64)(UINT32)(a >> 32) * (UINT64)(UINT32)(b >> 32);

	// compute middle parts
	UINT64 prevlo = lo;
	UINT64 temp = (UINT64)(UINT32)(a >> 32)  * (UINT64)(UINT32)(b >> 0);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	prevlo = lo;
	temp = (UINT64)(UINT32)(a >> 0)  * (UINT64)(UINT32)(b >> 32);
	lo += temp << 32;
	hi += (temp >> 32) + (lo < prevlo);

	// adjust for signage
	if ((INT64)(src1 ^ src2) < 0)
	{
		hi = ~hi + (lo == 0);
		lo = ~lo + 1;
	}

	// store the results
	dsthi = hi;
	dstlo = lo;
	return ((hi >> 60) & FLAG_S) | ((dsthi != ((INT64)lo >> 63)) << 1);
}
