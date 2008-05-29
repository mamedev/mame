/***************************************************************************

    drcbex86.c

    32-bit x86 back-end for the universal machine language.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Future improvements/changes:

    * Optimize to avoid unnecessary reloads
        - especially EDX for 64-bit operations
        - also FCMP/FLAGS has unnecessary PUSHF/POP EAX

    * Identify common pairs and optimize output

****************************************************************************

    ---------------
    ABI/conventions
    ---------------

    Registers:
        EAX        - volatile, function return value
        EBX        - non-volatile
        ECX        - volatile
        EDX        - volatile, function return value (upper 32 bits)
        ESI        - non-volatile
        EDI        - non-volatile
        EBP        - non-volatile

        FP stack   - volatile


    ---------------
    Execution model
    ---------------

    Registers:
        EAX        - scratch register
        EBX        - maps to I0 (low 32 bits)
        ECX        - scratch register
        EDX        - scratch register
        ESI        - maps to I1 (low 32 bits)
        EDI        - maps to I2 (low 32 bits)
        EBP        - maps to I3 (low 32 bits)

        FP stack   - scratch registers

    Entry point:
        Assumes 1 parameter passed, which is the codeptr of the code
        to execute once the environment is set up.

    Exit point:
        Assumes exit value is in EAX.

    Entry stack:
        [esp]      - return
        [esp+4]    - input parameter (entry handle)

    Runtime stack:
        [esp]      - temporary space for hash jump
        [esp+4]    - saved ebp
        [esp+8]    - saved edi
        [esp+12]   - saved esi
        [esp+16]   - saved ebx
        [esp+20]   - ret
        [esp+24]   - input parameter (entry handle)

***************************************************************************/

#include "drcuml.h"
#include "drcumld.h"
#include "drcbeut.h"
#include "debugger.h"
#include "x86emit.h"
#include "eminline.h"
#undef REG_SP
#include "x86log.h"
#include <math.h>
#include <stddef.h>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_HASHJMPS		(0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define PTYPE_M				(1 << DRCUML_PTYPE_MEMORY)
#define PTYPE_I				(1 << DRCUML_PTYPE_IMMEDIATE)
#define PTYPE_R				(1 << DRCUML_PTYPE_INT_REGISTER)
#define PTYPE_F				(1 << DRCUML_PTYPE_FLOAT_REGISTER)
#define PTYPE_MI			(PTYPE_M | PTYPE_I)
#define PTYPE_RI			(PTYPE_R | PTYPE_I)
#define PTYPE_MR			(PTYPE_M | PTYPE_R)
#define PTYPE_MRI			(PTYPE_M | PTYPE_R | PTYPE_I)
#define PTYPE_MF			(PTYPE_M | PTYPE_F)



/***************************************************************************
    MACROS
***************************************************************************/

#define X86_CONDITION(condflags)		(condition_map[condflags - DRCUML_COND_Z])
#define X86_NOT_CONDITION(condflags)	(condition_map[condflags - DRCUML_COND_Z] ^ 1)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* entry point */
typedef UINT32 (*x86_entry_point_func)(x86code *entry);

/* opcode handler */
typedef x86code *(*opcode_generate_func)(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);


/* opcode table entry */
typedef struct _opcode_table_entry opcode_table_entry;
struct _opcode_table_entry
{
	drcuml_opcode			opcode;					/* opcode in question */
	opcode_generate_func	func;					/* function pointer to the work */
};


/* internal backend-specific state */
struct _drcbe_state
{
	drcuml_state *			drcuml;					/* pointer back to our owner */
	drccache *				cache;					/* pointer to the cache */
	drcuml_machine_state 	state;					/* state of the machine */
	drchash_state *			hash;					/* hash table state */
	drcmap_state *			map;					/* code map */
	drclabel_list *			labels;                 /* label list */

	x86_entry_point_func	entry;					/* entry point */
	x86code *				exit;					/* exit point */
	x86code *				nocode;					/* nocode handler */

	UINT32 *				reglo[REG_MAX];			/* pointer to low part of data for each register */
	UINT32 *				reghi[REG_MAX];			/* pointer to high part of data for each register */
	double					fptemp;					/* temporary storage for floating point */

	UINT8					sse3;					/* do we have SSE3 support? */
	UINT16					fpumode;				/* saved FPU mode */
	UINT16					fmodesave;				/* temporary location for saving */

	void *					stacksave;				/* saved stack pointer */
	void *					hashstacksave;			/* saved stack pointer for hashjmp */
	UINT64					reslo;					/* extended low result */
	UINT64					reshi;					/* extended high result */

	x86log_context *		log;					/* logging */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* primary back-end callbacks */
static drcbe_state *drcbex86_alloc(drcuml_state *drcuml, drccache *cache, UINT32 flags, int modes, int addrbits, int ignorebits);
static void drcbex86_free(drcbe_state *drcbe);
static void drcbex86_reset(drcbe_state *drcbe);
static int drcbex86_execute(drcbe_state *drcbe, drcuml_codehandle *entry);
static void drcbex86_generate(drcbe_state *drcbe, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst);
static int drcbex86_hash_exists(drcbe_state *drcbe, UINT32 mode, UINT32 pc);
static void drcbex86_get_info(drcbe_state *state, drcbe_info *info);

/* private helper functions */
static void fixup_label(void *parameter, drccodeptr labelcodeptr);

/* miscellaneous functions */
static void dmulu(UINT64 *dstlo, UINT64 *dsthi, UINT64 src1, UINT64 src2);
static void dmuls(UINT64 *dstlo, UINT64 *dsthi, INT64 src1, INT64 src2);
static void ddivu(UINT64 *dstlo, UINT64 *dsthi, UINT64 src1, UINT64 src2);
static void ddivs(UINT64 *dstlo, UINT64 *dsthi, INT64 src1, INT64 src2);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* globally-accessible interface to the backend */
const drcbe_interface drcbe_x86_be_interface =
{
	drcbex86_alloc,
	drcbex86_free,
	drcbex86_reset,
	drcbex86_execute,
	drcbex86_generate,
	drcbex86_hash_exists,
	drcbex86_get_info
};

/* opcode table */
static opcode_generate_func opcode_table[DRCUML_OP_MAX];

/* size-to-mask table */
static const UINT64 size_to_mask[] = { 0, 0xff, 0xffff, 0, 0xffffffff, 0, 0, 0, U64(0xffffffffffffffff) };

/* register mapping tables */
static UINT8 int_register_map[DRCUML_REG_I_END - DRCUML_REG_I0] =
{
	REG_EBX, REG_ESI, REG_EDI, REG_EBP
};

/* flags mapping tables */
static UINT8 flags_map[0x1000];
static UINT32 flags_unmap[0x20];

/* condition mapping table */
static UINT8 condition_map[DRCUML_COND_MAX - DRCUML_COND_Z] =
{
	COND_Z,		/* DRCUML_COND_Z = 0x80,    requires Z */
	COND_NZ,	/* DRCUML_COND_NZ,          requires Z */
	COND_S,		/* DRCUML_COND_S,           requires S */
	COND_NS,	/* DRCUML_COND_NS,          requires S */
	COND_C,		/* DRCUML_COND_C,           requires C */
	COND_NC,	/* DRCUML_COND_NC,          requires C */
	COND_O,		/* DRCUML_COND_V,           requires V */
	COND_NO,	/* DRCUML_COND_NV,          requires V */
	COND_P,		/* DRCUML_COND_U,           requires U */
	COND_NP,	/* DRCUML_COND_NU,          requires U */
	COND_A,		/* DRCUML_COND_A,           requires CZ */
	COND_BE,	/* DRCUML_COND_BE,          requires CZ */
	COND_G,		/* DRCUML_COND_G,           requires SVZ */
	COND_LE,	/* DRCUML_COND_LE,          requires SVZ */
	COND_L,		/* DRCUML_COND_L,           requires SV */
	COND_GE,	/* DRCUML_COND_GE,          requires SV */
};

/* FPU control register mapping */
static const UINT16 fp_control[4] =
{
	0x0e3f, 	/* DRCUML_FMOD_TRUNC */
	0x023f, 	/* DRCUML_FMOD_ROUND */
	0x0a3f, 	/* DRCUML_FMOD_CEIL */
	0x063f	 	/* DRCUML_FMOD_FLOOR */
};



/***************************************************************************
    TABLES
***************************************************************************/

static x86code *op_handle(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_hash(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_label(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_comment(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_mapvar(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);

static x86code *op_debug(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_exit(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_hashjmp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_jmp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_exh(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_callh(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ret(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_callc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_recover(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);

static x86code *op_setfmod(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_getfmod(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_getexp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_save(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_restore(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);

static x86code *op_load1u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_load1s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_load2u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_load2s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_load4u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_load4s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_load8u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_store1(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_store2(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_store4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_store8(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read1u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read1s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read2u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read2s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read2m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read4u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read4s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read4m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read8u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read8m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_write1(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_write2(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_writ2m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_write4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_writ4m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_write8(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_writ8m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_flags(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_setc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_mov(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_zext1(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_zext2(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_zext4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_sext1(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_sext2(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_sext4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_xtract(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_insert(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_add(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_addc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_sub(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_subc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_cmp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_mulu(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_muls(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_divu(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_divs(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_and(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_test(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_or(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_xor(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_lzcnt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_bswap(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_shl(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_shr(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_sar(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ror(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_rol(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_rorc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_rolc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);

static x86code *op_fload(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fstore(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fread(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fwrite(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fmov(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi4t(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi4r(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi4f(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi4c(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi8(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi8t(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi8r(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi8f(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ftoi8c(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ffrfs(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ffrfd(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ffri4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ffri8(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fadd(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fsub(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fcmp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fmul(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fdiv(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fneg(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fabs(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_fsqrt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_frecip(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_frsqrt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);

static const opcode_table_entry opcode_table_source[] =
{
	/* Compile-time opcodes */
	{ DRCUML_OP_HANDLE,	 op_handle },	/* HANDLE  handle                 */
	{ DRCUML_OP_HASH,    op_hash },		/* HASH    mode,pc                */
	{ DRCUML_OP_LABEL,   op_label },	/* LABEL   imm                    */
	{ DRCUML_OP_COMMENT, op_comment },	/* COMMENT string                 */
	{ DRCUML_OP_MAPVAR,  op_mapvar },	/* MAPVAR  mapvar,value           */

	/* Control Flow Operations */
	{ DRCUML_OP_DEBUG,   op_debug },	/* DEBUG   pc                     */
	{ DRCUML_OP_EXIT,    op_exit },		/* EXIT    src1[,c]               */
	{ DRCUML_OP_HASHJMP, op_hashjmp },	/* HASHJMP mode,pc,handle         */
	{ DRCUML_OP_JMP,     op_jmp },		/* JMP     imm[,c]                */
	{ DRCUML_OP_EXH,     op_exh },		/* EXH     handle,param[,c]       */
	{ DRCUML_OP_CALLH,   op_callh },	/* CALLH   handle[,c]             */
	{ DRCUML_OP_RET,     op_ret },		/* RET     [c]                    */
	{ DRCUML_OP_CALLC,   op_callc },	/* CALLC   func,ptr[,c]           */
	{ DRCUML_OP_RECOVER, op_recover },	/* RECOVER dst,mapvar             */

	/* Internal Register Operations */
	{ DRCUML_OP_SETFMOD, op_setfmod },	/* SETFMOD src                    */
	{ DRCUML_OP_GETFMOD, op_getfmod },	/* GETFMOD dst                    */
	{ DRCUML_OP_GETEXP,  op_getexp },	/* GETEXP  dst,index              */
	{ DRCUML_OP_SAVE,    op_save },		/* SAVE    dst,index              */
	{ DRCUML_OP_RESTORE, op_restore },	/* RESTORE dst,index              */

	/* Integer Operations */
	{ DRCUML_OP_LOAD1U,  op_load1u },	/* LOAD1U  dst,base,index         */
	{ DRCUML_OP_LOAD1S,  op_load1s },	/* LOAD1S  dst,base,index         */
	{ DRCUML_OP_LOAD2U,  op_load2u },	/* LOAD2U  dst,base,index         */
	{ DRCUML_OP_LOAD2S,  op_load2s },	/* LOAD2S  dst,base,index         */
	{ DRCUML_OP_LOAD4U,  op_load4u },	/* LOAD4U  dst,base,index         */
	{ DRCUML_OP_LOAD4S,  op_load4s },	/* LOAD4S  dst,base,index         */
	{ DRCUML_OP_LOAD8U,  op_load8u },	/* LOAD8U  dst,base,index         */
	{ DRCUML_OP_STORE1,  op_store1 },	/* STORE1  base,index,src         */
	{ DRCUML_OP_STORE2,  op_store2 },	/* STORE2  base,index,src         */
	{ DRCUML_OP_STORE4,  op_store4 },	/* STORE4  base,index,src         */
	{ DRCUML_OP_STORE8,  op_store8 },	/* STORE8  base,index,src         */
	{ DRCUML_OP_READ1U,  op_read1u },	/* READ1U  dst,space,src1         */
	{ DRCUML_OP_READ1S,  op_read1s },	/* READ1S  dst,space,src1         */
	{ DRCUML_OP_READ2U,  op_read2u },	/* READ2U  dst,space,src1         */
	{ DRCUML_OP_READ2S,  op_read2s },	/* READ2S  dst,space,src1         */
	{ DRCUML_OP_READ2M,  op_read2m },	/* READ2M  dst,space,src1,mask    */
	{ DRCUML_OP_READ4U,  op_read4u },	/* READ4U  dst,space,src1         */
	{ DRCUML_OP_READ4S,  op_read4s },	/* READ4S  dst,space,src1         */
	{ DRCUML_OP_READ4M,  op_read4m },	/* READ4M  dst,space,src1,mask    */
	{ DRCUML_OP_READ8U,  op_read8u },	/* READ8U  dst,space,src1         */
	{ DRCUML_OP_READ8M,  op_read8m },	/* READ8M  dst,space,src1,mask    */
	{ DRCUML_OP_WRITE1,  op_write1 },	/* WRITE1  space,dst,src1         */
	{ DRCUML_OP_WRITE2,  op_write2 },	/* WRITE2  space,dst,src1         */
	{ DRCUML_OP_WRIT2M,  op_writ2m },	/* WRIT2M  space,dst,src1         */
	{ DRCUML_OP_WRITE4,  op_write4 },	/* WRITE4  space,dst,src1         */
	{ DRCUML_OP_WRIT4M,  op_writ4m },	/* WRIT4M  space,dst,mask,src1    */
	{ DRCUML_OP_WRITE8,  op_write8 },	/* WRITE8  space,dst,src1         */
	{ DRCUML_OP_WRIT8M,  op_writ8m },	/* WRIT8M  space,dst,mask,src1    */
	{ DRCUML_OP_FLAGS,   op_flags },	/* FLAGS   dst,mask,table         */
	{ DRCUML_OP_SETC,    op_setc },		/* FLAGS   src,bitnum             */
	{ DRCUML_OP_MOV,     op_mov },		/* MOV     dst,src[,c]            */
	{ DRCUML_OP_ZEXT1,   op_zext1 },	/* ZEXT1   dst,src                */
	{ DRCUML_OP_ZEXT2,   op_zext2 },	/* ZEXT2   dst,src                */
	{ DRCUML_OP_ZEXT4,   op_zext4 },	/* ZEXT4   dst,src                */
	{ DRCUML_OP_SEXT1,   op_sext1 },	/* SEXT1   dst,src                */
	{ DRCUML_OP_SEXT2,   op_sext2 },	/* SEXT2   dst,src                */
	{ DRCUML_OP_SEXT4,   op_sext4 },	/* SEXT4   dst,src                */
	{ DRCUML_OP_XTRACT,  op_xtract },	/* XTRACT  dst,src1,src2,src3     */
	{ DRCUML_OP_INSERT,  op_insert },	/* INSERT  dst,src1,src2,src3     */
	{ DRCUML_OP_ADD,     op_add },		/* ADD     dst,src1,src2[,f]      */
	{ DRCUML_OP_ADDC,    op_addc },		/* ADDC    dst,src1,src2[,f]      */
	{ DRCUML_OP_SUB,     op_sub },		/* SUB     dst,src1,src2[,f]      */
	{ DRCUML_OP_SUBB,    op_subc },		/* SUBB    dst,src1,src2[,f]      */
	{ DRCUML_OP_CMP,     op_cmp },		/* CMP     src1,src2[,f]          */
	{ DRCUML_OP_MULU,    op_mulu },		/* MULU    dst,edst,src1,src2[,f] */
	{ DRCUML_OP_MULS,    op_muls },		/* MULS    dst,edst,src1,src2[,f] */
	{ DRCUML_OP_DIVU,    op_divu },		/* DIVU    dst,edst,src1,src2[,f] */
	{ DRCUML_OP_DIVS,    op_divs },		/* DIVS    dst,edst,src1,src2[,f] */
	{ DRCUML_OP_AND,     op_and },		/* AND     dst,src1,src2[,f]      */
	{ DRCUML_OP_TEST,    op_test },		/* TEST    src1,src2[,f]          */
	{ DRCUML_OP_OR,      op_or },		/* OR      dst,src1,src2[,f]      */
	{ DRCUML_OP_XOR,     op_xor },		/* XOR     dst,src1,src2[,f]      */
	{ DRCUML_OP_LZCNT,   op_lzcnt },	/* LZCNT   dst,src                */
	{ DRCUML_OP_BSWAP,   op_bswap },	/* BSWAP   dst,src                */
	{ DRCUML_OP_SHL,     op_shl },		/* SHL     dst,src,count[,f]      */
	{ DRCUML_OP_SHR,     op_shr },		/* SHR     dst,src,count[,f]      */
	{ DRCUML_OP_SAR,     op_sar },		/* SAR     dst,src,count[,f]      */
	{ DRCUML_OP_ROL,     op_rol },		/* ROL     dst,src,count[,f]      */
	{ DRCUML_OP_ROLC,    op_rolc },		/* ROLC    dst,src,count[,f]      */
	{ DRCUML_OP_ROR,     op_ror },		/* ROR     dst,src,count[,f]      */
	{ DRCUML_OP_RORC,    op_rorc },		/* RORC    dst,src,count[,f]      */

	/* Floating Point Operations */
	{ DRCUML_OP_FLOAD,   op_fload },	/* FLOAD   dst,base,index         */
	{ DRCUML_OP_FSTORE,  op_fstore },	/* FSTORE  base,index,src         */
	{ DRCUML_OP_FREAD,   op_fread },	/* FREAD   dst,space,src1         */
	{ DRCUML_OP_FWRITE,  op_fwrite },	/* FWRITE  space,dst,src1         */
	{ DRCUML_OP_FMOV,    op_fmov },		/* FMOV    dst,src1[,c]           */
	{ DRCUML_OP_FTOI4,   op_ftoi4 },	/* FTOI4   dst,src1               */
	{ DRCUML_OP_FTOI4T,  op_ftoi4t },	/* FTOI4T  dst,src1               */
	{ DRCUML_OP_FTOI4R,  op_ftoi4r },	/* FTOI4R  dst,src1               */
	{ DRCUML_OP_FTOI4F,  op_ftoi4f },	/* FTOI4F  dst,src1               */
	{ DRCUML_OP_FTOI4C,  op_ftoi4c },	/* FTOI4C  dst,src1               */
	{ DRCUML_OP_FTOI8,   op_ftoi8 },	/* FTOI8   dst,src1               */
	{ DRCUML_OP_FTOI8T,  op_ftoi8t },	/* FTOI8T  dst,src1               */
	{ DRCUML_OP_FTOI8R,  op_ftoi8r },	/* FTOI8R  dst,src1               */
	{ DRCUML_OP_FTOI8F,  op_ftoi8f },	/* FTOI8F  dst,src1               */
	{ DRCUML_OP_FTOI8C,  op_ftoi8c },	/* FTOI8C  dst,src1               */
	{ DRCUML_OP_FFRFS,   op_ffrfs },	/* FFRFS   dst,src1               */
	{ DRCUML_OP_FFRFD,   op_ffrfd },	/* FFRFD   dst,src1               */
	{ DRCUML_OP_FFRI4,   op_ffri4 },	/* FFRI4   dst,src1               */
	{ DRCUML_OP_FFRI8,   op_ffri8 },	/* FFRI8   dst,src1               */
	{ DRCUML_OP_FADD,    op_fadd },		/* FADD    dst,src1,src2          */
	{ DRCUML_OP_FSUB,    op_fsub },		/* FSUB    dst,src1,src2          */
	{ DRCUML_OP_FCMP,    op_fcmp },		/* FCMP    src1,src2              */
	{ DRCUML_OP_FMUL,    op_fmul },		/* FMUL    dst,src1,src2          */
	{ DRCUML_OP_FDIV,    op_fdiv },		/* FDIV    dst,src1,src2          */
	{ DRCUML_OP_FNEG,    op_fneg },		/* FNEG    dst,src1               */
	{ DRCUML_OP_FABS,    op_fabs },		/* FABS    dst,src1               */
	{ DRCUML_OP_FSQRT,   op_fsqrt },	/* FSQRT   dst,src1               */
	{ DRCUML_OP_FRECIP,  op_frecip },	/* FRECIP  dst,src1               */
	{ DRCUML_OP_FRSQRT,  op_frsqrt }	/* FRSQRT  dst,src1               */
};



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    param_select_register - select a register
    to use, avoiding conflicts with the optional
    checkparam
-------------------------------------------------*/

INLINE int param_select_register(int defreg, const drcuml_parameter *param, const drcuml_parameter *checkparam)
{
	if (param->type == DRCUML_PTYPE_INT_REGISTER && (checkparam == NULL || param->type != checkparam->type || param->value != checkparam->value))
		return param->value;
	return defreg;
}


/*-------------------------------------------------
    param_select_register2 - select a register
    to use, avoiding conflicts with the optional
    checkparam
-------------------------------------------------*/

INLINE int param_select_register2(int defreg, const drcuml_parameter *param, const drcuml_parameter *checkparam1, const drcuml_parameter *checkparam2)
{
	if (param->type == DRCUML_PTYPE_INT_REGISTER && (param->type != checkparam1->type || param->value != checkparam1->value) && (param->type != checkparam2->type || param->value != checkparam2->value))
		return param->value;
	return defreg;
}


/*-------------------------------------------------
    emit_combine_z_flags - combine the Z flag from
    two 32-bit operations
-------------------------------------------------*/

INLINE void emit_combine_z_flags(x86code **dst)
{
	/* this assumes that the flags from the low 32-bit op are on the stack */
	/* and the flags from the high 32-bit op are live */
	emit_pushf(dst);																	// pushf
	emit_mov_r32_m32(dst, REG_ECX, MBD(REG_ESP, 4));									// mov   ecx,[esp+4]
	emit_or_r32_imm(dst, REG_ECX, ~0x40);												// or    ecx,~0x40
	emit_and_m32_r32(dst, MBD(REG_ESP, 0), REG_ECX);									// and   [esp],ecx
	emit_popf(dst);																		// popf
	emit_lea_r32_m32(dst, REG_ESP, MBD(REG_ESP, 4));									// lea   esp,[esp+4]
}


/*-------------------------------------------------
    emit_combine_z_shl_flags - combine the Z
    flags from two 32-bit shift left operations
-------------------------------------------------*/

INLINE void emit_combine_z_shl_flags(x86code **dst)
{
	/* this assumes that the flags from the high 32-bit op are on the stack */
	/* and the flags from the low 32-bit op are live */
	emit_pushf(dst);																	// pushf
	emit_pop_r32(dst, REG_ECX);															// pop   ecx
	emit_or_r32_imm(dst, REG_ECX, ~0x40);												// or    ecx,~0x40
	emit_and_m32_r32(dst, MBD(REG_ESP, 0), REG_ECX);									// and   [esp],ecx
	emit_popf(dst);																		// popf
}



/***************************************************************************
    BACKEND CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    drcbex86_alloc - allocate back-end-specific
    state
-------------------------------------------------*/

static drcbe_state *drcbex86_alloc(drcuml_state *drcuml, drccache *cache, UINT32 flags, int modes, int addrbits, int ignorebits)
{
	int opnum, regnum, entry;
	drcbe_state *drcbe;

	/* allocate space in the cache for our state */
	drcbe = drccache_memory_alloc(cache, sizeof(*drcbe));
	if (drcbe == NULL)
		return NULL;
	memset(drcbe, 0, sizeof(*drcbe));

	/* remember our pointers */
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

	/* build the opcode table (static but it doesn't hurt to regenerate it) */
	for (opnum = 0; opnum < ARRAY_LENGTH(opcode_table_source); opnum++)
		opcode_table[opcode_table_source[opnum].opcode] = opcode_table_source[opnum].func;

	/* build the flags map (static but it doesn't hurt to regenerate it) */
	for (entry = 0; entry < ARRAY_LENGTH(flags_map); entry++)
	{
		UINT8 flags = 0;
		if (entry & 0x001) flags |= DRCUML_FLAG_C;
		if (entry & 0x004) flags |= DRCUML_FLAG_U;
		if (entry & 0x040) flags |= DRCUML_FLAG_Z;
		if (entry & 0x080) flags |= DRCUML_FLAG_S;
		if (entry & 0x800) flags |= DRCUML_FLAG_V;
		flags_map[entry] = flags;
	}
	for (entry = 0; entry < ARRAY_LENGTH(flags_unmap); entry++)
	{
		UINT32 flags = 0;
		if (entry & DRCUML_FLAG_C) flags |= 0x001;
		if (entry & DRCUML_FLAG_U) flags |= 0x004;
		if (entry & DRCUML_FLAG_Z) flags |= 0x040;
		if (entry & DRCUML_FLAG_S) flags |= 0x080;
		if (entry & DRCUML_FLAG_V) flags |= 0x800;
		flags_unmap[entry] = flags;
	}

	/* compute hi pointers for each register */
	for (regnum = 0; regnum < ARRAY_LENGTH(int_register_map); regnum++)
		if (int_register_map[regnum] != 0)
		{
			drcbe->reglo[int_register_map[regnum]] = &drcbe->state.r[regnum].w.l;
			drcbe->reghi[int_register_map[regnum]] = &drcbe->state.r[regnum].w.h;
		}

	/* create the log */
	if (flags & DRCUML_OPTION_LOG_NATIVE)
		drcbe->log = x86log_create_context("drcbex86.asm");

	return drcbe;
}


/*-------------------------------------------------
    drcbex86_free - free back-end specific state
-------------------------------------------------*/

static void drcbex86_free(drcbe_state *drcbe)
{
	/* free the log context */
	if (drcbe->log != NULL)
		x86log_free_context(drcbe->log);
}


/*-------------------------------------------------
    drcbex86_reset - reset back-end specific state
-------------------------------------------------*/

static void drcbex86_reset(drcbe_state *drcbe)
{
	UINT32 (*cpuid_ecx_stub)(void);
	x86code **dst;

	/* output a note to the log */
	if (drcbe->log != NULL)
		x86log_printf(drcbe->log, "\n\n===========\nCACHE RESET\n===========\n\n");

	/* generate a little bit of glue code to set up the environment */
	dst = (x86code **)drccache_begin_codegen(drcbe->cache, 500);
	if (dst == NULL)
		fatalerror("Out of cache space after a reset!");

	/* generate a simple CPUID stub */
	cpuid_ecx_stub = (UINT32 (*)(void))*dst;
	emit_push_r32(dst, REG_EBX);														// push  ebx
	emit_mov_r32_imm(dst, REG_EAX, 1);													// mov   eax,1
	emit_cpuid(dst);																	// cpuid
	emit_mov_r32_r32(dst, REG_EAX, REG_ECX);											// mov   eax,ecx
	emit_pop_r32(dst, REG_EBX);															// pop   ebx
	emit_ret(dst);																		// ret

	/* call it to determine if we have SSE3 support */
	drcbe->sse3 = (((*cpuid_ecx_stub)() & 1) != 0);

	/* generate an entry point */
	drcbe->entry = (x86_entry_point_func)*dst;
	emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ESP, 4));									// mov   eax,[esp+4]
	emit_push_r32(dst, REG_EBX);														// push  ebx
	emit_push_r32(dst, REG_ESI);														// push  esi
	emit_push_r32(dst, REG_EDI);														// push  edi
	emit_push_r32(dst, REG_EBP);														// push  ebp
	emit_mov_m32_r32(dst, MABS(&drcbe->hashstacksave), REG_ESP);						// mov   [hashstacksave],esp
	emit_sub_r32_imm(dst, REG_ESP, 4);													// sub   esp,4
	emit_mov_m32_r32(dst, MABS(&drcbe->stacksave), REG_ESP);							// mov   [stacksave],esp
	emit_fstcw_m16(dst, MABS(&drcbe->fpumode));											// fstcw [fpumode]
	emit_jmp_r32(dst, REG_EAX);															// jmp   eax
	if (drcbe->log != NULL)
		x86log_disasm_code_range(drcbe->log, "entry_point", (x86code *)drcbe->entry, *dst);

	/* generate an exit point */
	drcbe->exit = *dst;
	emit_fldcw_m16(dst, MABS(&drcbe->fpumode));											// fldcw [fpumode]
	emit_mov_r32_m32(dst, REG_ESP, MABS(&drcbe->hashstacksave));						// mov   esp,[hashstacksave]
	emit_pop_r32(dst, REG_EBP);															// pop   ebp
	emit_pop_r32(dst, REG_EDI);															// pop   edi
	emit_pop_r32(dst, REG_ESI);															// pop   esi
	emit_pop_r32(dst, REG_EBX);															// pop   ebx
	emit_ret(dst);																		// ret
	if (drcbe->log != NULL)
		x86log_disasm_code_range(drcbe->log, "exit_point", drcbe->exit, *dst);

	/* generate a no code point */
	drcbe->nocode = *dst;
	emit_ret(dst);																		// ret
	if (drcbe->log != NULL)
		x86log_disasm_code_range(drcbe->log, "nocode", drcbe->nocode, *dst);

	/* finish up codegen */
	drccache_end_codegen(drcbe->cache);

	/* reset our hash tables */
	drchash_reset(drcbe->hash);
	drchash_set_default_codeptr(drcbe->hash, drcbe->nocode);
}


/*-------------------------------------------------
    drcbex86_execute - execute a block of code
    referenced by the given handle
-------------------------------------------------*/

static int drcbex86_execute(drcbe_state *drcbe, drcuml_codehandle *entry)
{
	/* call our entry point which will jump to the destination */
	return (*drcbe->entry)((x86code *)drcuml_handle_codeptr(entry));
}


/*-------------------------------------------------
    drcbex86_generate - generate code
-------------------------------------------------*/

static void drcbex86_generate(drcbe_state *drcbe, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst)
{
	const char *blockname = NULL;
	char blockbuffer[100];
	drccodeptr *cachetop;
	x86code *base;
	x86code *dst;
	int inum;

	/* tell all of our utility objects that a block is beginning */
	drchash_block_begin(drcbe->hash, block, instlist, numinst);
	drclabel_block_begin(drcbe->labels, block);
	drcmap_block_begin(drcbe->map, block);

	/* begin codegen; fail if we can't */
	cachetop = drccache_begin_codegen(drcbe->cache, numinst * 8 * 4);
	if (cachetop == NULL)
		drcuml_block_abort(block);

	/* compute the base by aligning the cache top to a cache line (assumed to be 64 bytes) */
	base = (x86code *)(((FPTR)*cachetop + 63) & ~63);
	dst = base;

	/* generate code */
	for (inum = 0; inum < numinst; inum++)
	{
		const drcuml_instruction *inst = &instlist[inum];
		assert(inst->opcode < ARRAY_LENGTH(opcode_table));

		/* add a comment */
		if (drcbe->log != NULL)
		{
			char dasm[100];
			drcuml_disasm(inst, dasm);
			x86log_add_comment(drcbe->log, dst, "%s", dasm);
		}

		/* extract a blockname */
		if (blockname == NULL)
		{
			if (inst->opcode == DRCUML_OP_HANDLE)
				blockname = drcuml_handle_name((drcuml_codehandle *)(FPTR)inst->param[0].value);
			else if (inst->opcode == DRCUML_OP_HASH)
			{
				sprintf(blockbuffer, "Code: mode=%d PC=%08X", (UINT32)inst->param[0].value, (offs_t)inst->param[1].value);
				blockname = blockbuffer;
			}
		}

		/* generate code */
		dst = (*opcode_table[inst->opcode])(drcbe, dst, inst);
	}

	/* complete codegen */
	*cachetop = (drccodeptr)dst;
	drccache_end_codegen(drcbe->cache);

	/* log it */
	if (drcbe->log != NULL)
		x86log_disasm_code_range(drcbe->log, (blockname == NULL) ? "Unknown block" : blockname, base, drccache_top(drcbe->cache));

	/* tell all of our utility objects that the block is finished */
	drchash_block_end(drcbe->hash, block);
	drclabel_block_end(drcbe->labels, block);
	drcmap_block_end(drcbe->map, block);
}


/*-------------------------------------------------
    drcbex86_hash_exists - return true if the
    given mode/pc exists in the hash table
-------------------------------------------------*/

static int drcbex86_hash_exists(drcbe_state *drcbe, UINT32 mode, UINT32 pc)
{
	return drchash_code_exists(drcbe->hash, mode, pc);
}


/*-------------------------------------------------
    drcbex86_get_info - return information about
    the back-end implementation
-------------------------------------------------*/

static void drcbex86_get_info(drcbe_state *state, drcbe_info *info)
{
	for (info->direct_iregs = 0; info->direct_iregs < DRCUML_REG_I_END - DRCUML_REG_I0; info->direct_iregs++)
		if (int_register_map[info->direct_iregs] == 0)
			break;
	info->direct_fregs = 0;
}



/***************************************************************************
    COMPILE HELPERS
***************************************************************************/

/*-------------------------------------------------
    param_normalize - convert a full parameter
    into a reduced set
-------------------------------------------------*/

static void param_normalize(drcbe_state *drcbe, const drcuml_parameter *src, drcuml_parameter *dest, UINT32 allowed)
{
	int regnum;

	switch (src->type)
	{
		/* immediates pass through */
		case DRCUML_PTYPE_IMMEDIATE:
			assert(allowed & PTYPE_I);
			dest->type = DRCUML_PTYPE_IMMEDIATE;
			dest->value = src->value;
			break;

		/* mapvars are converted to immediates with their current value */
		case DRCUML_PTYPE_MAPVAR:
			assert(allowed & PTYPE_I);
			dest->type = DRCUML_PTYPE_IMMEDIATE;
			dest->value = drcmap_get_last_value(drcbe->map, src->value);
			break;

		/* memory passes through */
		case DRCUML_PTYPE_MEMORY:
			assert(allowed & PTYPE_M);
			dest->type = DRCUML_PTYPE_MEMORY;
			dest->value = src->value;
			break;

		/* if a register maps to a register, keep it as a register; otherwise map it to memory */
		case DRCUML_PTYPE_INT_REGISTER:
			assert(allowed & PTYPE_R);
			assert(allowed & PTYPE_M);
			regnum = int_register_map[src->value - DRCUML_REG_I0];
			if (regnum != 0)
			{
				dest->type = DRCUML_PTYPE_INT_REGISTER;
				dest->value = regnum;
			}
			else
			{
				dest->type = DRCUML_PTYPE_MEMORY;
				dest->value = (FPTR)&drcbe->state.r[src->value - DRCUML_REG_I0];
			}
			break;

		/* all floating point registers map to memory */
		case DRCUML_PTYPE_FLOAT_REGISTER:
			assert(allowed & PTYPE_F);
			assert(allowed & PTYPE_M);
			dest->type = DRCUML_PTYPE_MEMORY;
			dest->value = (FPTR)&drcbe->state.f[src->value - DRCUML_REG_F0];
			break;

		/* everything else is unexpected */
		default:
			fatalerror("Unexpected parameter type");
			break;
	}
}


/*-------------------------------------------------
    param_normalize_1 - normalize a single
    parameter instruction
-------------------------------------------------*/

static void param_normalize_1(drcbe_state *drcbe, const drcuml_instruction *inst, drcuml_parameter *dest0, UINT32 allowed0)
{
	assert(inst->numparams == 1);
	param_normalize(drcbe, &inst->param[0], dest0, allowed0);
}


/*-------------------------------------------------
    param_normalize_2 - normalize a 2
    parameter instruction
-------------------------------------------------*/

static void param_normalize_2(drcbe_state *drcbe, const drcuml_instruction *inst, drcuml_parameter *dest0, UINT32 allowed0, drcuml_parameter *dest1, UINT32 allowed1)
{
	assert(inst->numparams == 2);
	param_normalize(drcbe, &inst->param[0], dest0, allowed0);
	param_normalize(drcbe, &inst->param[1], dest1, allowed1);
}


/*-------------------------------------------------
    param_normalize_2_commutative - normalize a 2
    parameter instruction, shuffling the
    parameters on the assumption that the two
    parameters can be swapped
-------------------------------------------------*/

static void param_normalize_2_commutative(drcbe_state *drcbe, const drcuml_instruction *inst, drcuml_parameter *dest0, UINT32 allowed0, drcuml_parameter *dest1, UINT32 allowed1)
{
	param_normalize_2(drcbe, inst, dest0, allowed0, dest1, allowed1);

	/* if the inner parameter is a memory operand, push it to the outer */
	if (dest0->type == DRCUML_PTYPE_MEMORY)
	{
		drcuml_parameter temp = *dest0;
		*dest0 = *dest1;
		*dest1 = temp;
	}

	/* if the inner parameter is an immediate, push it to the outer */
	if (dest0->type == DRCUML_PTYPE_IMMEDIATE)
	{
		drcuml_parameter temp = *dest0;
		*dest0 = *dest1;
		*dest1 = temp;
	}
}


/*-------------------------------------------------
    param_normalize_3 - normalize a 3
    parameter instruction
-------------------------------------------------*/

static void param_normalize_3(drcbe_state *drcbe, const drcuml_instruction *inst, drcuml_parameter *dest0, UINT32 allowed0, drcuml_parameter *dest1, UINT32 allowed1, drcuml_parameter *dest2, UINT32 allowed2)
{
	assert(inst->numparams == 3);
	param_normalize(drcbe, &inst->param[0], dest0, allowed0);
	param_normalize(drcbe, &inst->param[1], dest1, allowed1);
	param_normalize(drcbe, &inst->param[2], dest2, allowed2);
}


/*-------------------------------------------------
    param_normalize_3_commutative - normalize a 3
    parameter instruction, shuffling the
    parameters on the assumption that the last
    2 can be swapped
-------------------------------------------------*/

static void param_normalize_3_commutative(drcbe_state *drcbe, const drcuml_instruction *inst, drcuml_parameter *dest0, UINT32 allowed0, drcuml_parameter *dest1, UINT32 allowed1, drcuml_parameter *dest2, UINT32 allowed2)
{
	param_normalize_3(drcbe, inst, dest0, allowed0, dest1, allowed1, dest2, allowed2);

	/* if the inner parameter is a memory operand, push it to the outer */
	if (dest1->type == DRCUML_PTYPE_MEMORY)
	{
		drcuml_parameter temp = *dest1;
		*dest1 = *dest2;
		*dest2 = temp;
	}

	/* if the inner parameter is an immediate, push it to the outer */
	if (dest1->type == DRCUML_PTYPE_IMMEDIATE)
	{
		drcuml_parameter temp = *dest1;
		*dest1 = *dest2;
		*dest2 = temp;
	}

	/* if the destination and outer parameters are equal, move the outer to the inner */
	if (dest0->type == dest2->type && dest0->value == dest2->value && dest0->type != DRCUML_PTYPE_IMMEDIATE)
	{
		drcuml_parameter temp = *dest1;
		*dest1 = *dest2;
		*dest2 = temp;
	}
}


/*-------------------------------------------------
    param_normalize_4 - normalize a 4
    parameter instruction
-------------------------------------------------*/

static void param_normalize_4(drcbe_state *drcbe, const drcuml_instruction *inst, drcuml_parameter *dest0, UINT32 allowed0, drcuml_parameter *dest1, UINT32 allowed1, drcuml_parameter *dest2, UINT32 allowed2, drcuml_parameter *dest3, UINT32 allowed3)
{
	assert(inst->numparams == 4);
	param_normalize(drcbe, &inst->param[0], dest0, allowed0);
	param_normalize(drcbe, &inst->param[1], dest1, allowed1);
	param_normalize(drcbe, &inst->param[2], dest2, allowed2);
	param_normalize(drcbe, &inst->param[3], dest3, allowed3);
}


/*-------------------------------------------------
    param_normalize_4_commutative - normalize a 4
    parameter instruction, shuffling the
    parameters on the assumption that the last
    2 can be swapped
-------------------------------------------------*/

static void param_normalize_4_commutative(drcbe_state *drcbe, const drcuml_instruction *inst, drcuml_parameter *dest0, UINT32 allowed0, drcuml_parameter *dest1, UINT32 allowed1, drcuml_parameter *dest2, UINT32 allowed2, drcuml_parameter *dest3, UINT32 allowed3)
{
	param_normalize_4(drcbe, inst, dest0, allowed0, dest1, allowed1, dest2, allowed2, dest3, allowed3);

	/* if the inner parameter is a memory operand, push it to the outer */
	if (dest2->type == DRCUML_PTYPE_MEMORY)
	{
		drcuml_parameter temp = *dest2;
		*dest2 = *dest3;
		*dest3 = temp;
	}

	/* if the inner parameter is an immediate, push it to the outer */
	if (dest2->type == DRCUML_PTYPE_IMMEDIATE)
	{
		drcuml_parameter temp = *dest2;
		*dest2 = *dest3;
		*dest3 = temp;
	}

	/* if the destination and outer parameters are equal, move the outer to the inner */
	if (dest0->type == dest3->type && dest0->value == dest3->value && dest0->type != DRCUML_PTYPE_IMMEDIATE)
	{
		drcuml_parameter temp = *dest2;
		*dest2 = *dest3;
		*dest3 = temp;
	}
}



/***************************************************************************
    EMITTERS FOR 32-BIT OPERATIONS WITH PARAMETERS
***************************************************************************/

/*-------------------------------------------------
    emit_mov_r32_p32 - move a 32-bit parameter
    into a register
-------------------------------------------------*/

static void emit_mov_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (param->value == 0)
			emit_xor_r32_r32(dst, reg, reg);											// xor   reg,reg
		else
			emit_mov_r32_imm(dst, reg, param->value);									// mov   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_mov_r32_m32(dst, reg, MABS(param->value));									// mov   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_mov_r32_r32(dst, reg, param->value);									// mov   reg,param
	}
}


/*-------------------------------------------------
    emit_mov_m32_p32 - move a 32-bit parameter
    into a memory location
-------------------------------------------------*/

static void emit_mov_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_mov_m32_imm(dst, MEMPARAMS, param->value);									// mov   [mem],param
	else if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_mov_r32_m32(dst, REG_EAX, MABS(param->value));								// mov   eax,[param]
		emit_mov_m32_r32(dst, MEMPARAMS, REG_EAX);										// mov   [mem],eax
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_mov_m32_r32(dst, MEMPARAMS, param->value);									// mov   [mem],param
}


/*-------------------------------------------------
    emit_mov_p32_r32 - move a register into a
    32-bit parameter
-------------------------------------------------*/

static void emit_mov_p32_r32(drcbe_state *drcbe, x86code **dst, const drcuml_parameter *param, UINT8 reg)
{
	assert(param->type != DRCUML_PTYPE_IMMEDIATE);
	if (param->type == DRCUML_PTYPE_MEMORY)
		emit_mov_m32_r32(dst, MABS(param->value), reg);									// mov   [param],reg
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_mov_r32_r32(dst, param->value, reg);									// mov   param,reg
	}
}


/*-------------------------------------------------
    emit_push_p32 - push a 32-bit parameter onto
    the stack
-------------------------------------------------*/

static void emit_push_p32(drcbe_state *drcbe, x86code **dst, const drcuml_parameter *src)
{
	if (src->type == DRCUML_PTYPE_IMMEDIATE)
		emit_push_imm(dst, src->value);													// push  src
	else if (src->type == DRCUML_PTYPE_MEMORY)
		emit_push_m32(dst, MABS(src->value));											// push  [src]
	else if (src->type == DRCUML_PTYPE_INT_REGISTER)
		emit_push_r32(dst, src->value);													// push  src
}


/*-------------------------------------------------
    emit_add_r32_p32 - add operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_add_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags != 0 || param->value != 0)
			emit_add_r32_imm(dst, reg, param->value);									// add   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_add_r32_m32(dst, reg, MABS(param->value));									// add   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_add_r32_r32(dst, reg, param->value);										// add   reg,param
}


/*-------------------------------------------------
    emit_add_m32_p32 - add operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_add_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags != 0 || param->value != 0)
			emit_add_m32_imm(dst, MEMPARAMS, param->value);								// add   [dest],param
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32(drcbe, dst, reg, param);										// mov   reg,param
		emit_add_m32_r32(dst, MEMPARAMS, reg);											// add   [dest],reg
	}
}


/*-------------------------------------------------
    emit_adc_r32_p32 - adc operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_adc_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags != 0 || param->value != 0)
			emit_adc_r32_imm(dst, reg, param->value);									// adc   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_adc_r32_m32(dst, reg, MABS(param->value));									// adc   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_adc_r32_r32(dst, reg, param->value);										// adc   reg,param
}


/*-------------------------------------------------
    emit_adc_m32_p32 - adc operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_adc_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags != 0 || param->value != 0)
			emit_adc_m32_imm(dst, MEMPARAMS, param->value);								// adc   [dest],param
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32(drcbe, dst, reg, param);										// mov   reg,param
		emit_adc_m32_r32(dst, MEMPARAMS, reg);											// adc   [dest],reg
	}
}


/*-------------------------------------------------
    emit_sub_r32_p32 - sub operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_sub_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags != 0 || param->value != 0)
			emit_sub_r32_imm(dst, reg, param->value);									// sub   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_sub_r32_m32(dst, reg, MABS(param->value));									// sub   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_sub_r32_r32(dst, reg, param->value);										// sub   reg,param
}


/*-------------------------------------------------
    emit_sub_m32_p32 - sub operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_sub_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags != 0 || param->value != 0)
			emit_sub_m32_imm(dst, MEMPARAMS, param->value);								// sub   [dest],param
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32(drcbe, dst, reg, param);										// mov   reg,param
		emit_sub_m32_r32(dst, MEMPARAMS, reg);											// sub   [dest],reg
	}
}


/*-------------------------------------------------
    emit_sbb_r32_p32 - sbb operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_sbb_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags != 0 || param->value != 0)
			emit_sbb_r32_imm(dst, reg, param->value);									// sbb   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_sbb_r32_m32(dst, reg, MABS(param->value));									// sbb   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_sbb_r32_r32(dst, reg, param->value);										// sbb   reg,param
}


/*-------------------------------------------------
    emit_sbb_m32_p32 - sbb operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_sbb_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags != 0 || param->value != 0)
			emit_sbb_m32_imm(dst, MEMPARAMS, param->value);								// sbb   [dest],param
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32(drcbe, dst, reg, param);										// mov   reg,param
		emit_sbb_m32_r32(dst, MEMPARAMS, reg);											// sbb   [dest],reg
	}
}


/*-------------------------------------------------
    emit_cmp_r32_p32 - cmp operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_cmp_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_cmp_r32_imm(dst, reg, param->value);										// cmp   reg,param
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_cmp_r32_m32(dst, reg, MABS(param->value));									// cmp   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_cmp_r32_r32(dst, reg, param->value);										// cmp   reg,param
}


/*-------------------------------------------------
    emit_cmp_r32_p32hi - cmp operation to a 32-bit
    register from the upper 32 bits of a 64-bit
    parameter
-------------------------------------------------*/

static void emit_cmp_r32_p32hi(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_cmp_r32_imm(dst, reg, param->value >> 32);									// cmp   reg,param >> 32
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_cmp_r32_m32(dst, reg, MABS(param->value + 4));								// cmp   reg,[param+4]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_cmp_r32_m32(dst, reg, MABS(drcbe->reghi[param->value]));					// cmp   reg,reghi[param]
}


/*-------------------------------------------------
    emit_cmp_m32_p32 - cmp operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_cmp_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_cmp_m32_imm(dst, MEMPARAMS, param->value);									// cmp   [dest],param
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32(drcbe, dst, reg, param);										// mov   reg,param
		emit_cmp_m32_r32(dst, MEMPARAMS, reg);											// cmp   [dest],reg
	}
}


/*-------------------------------------------------
    emit_cmp_m32_p32hi - cmp operation to a 32-bit
    memory location from the upper half of a
    64-bit parameter
-------------------------------------------------*/

static void emit_cmp_m32_p32hi(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_cmp_m32_imm(dst, MEMPARAMS, param->value >> 32);							// cmp   [dest],param >> 32
	else
	{
		if (param->type == DRCUML_PTYPE_MEMORY)
			emit_mov_r32_m32(dst, REG_EAX, MABS(param->value + 4));						// mov   eax,[param+4]
		else if (param->type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_r32_m32(dst, REG_EAX, MABS(drcbe->reghi[param->value]));			// mov   eax,reghi[param]
		emit_cmp_m32_r32(dst, MEMPARAMS, REG_EAX);										// cmp   [dest],eax
	}
}


/*-------------------------------------------------
    emit_and_r32_p32 - and operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_and_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0xffffffff)
			/* skip */;
		else if (inst->condflags != 0 && (UINT32)param->value == 0)
			emit_xor_r32_r32(dst, reg, reg);											// xor   reg,reg
		else
			emit_and_r32_imm(dst, reg, param->value);									// and   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_and_r32_m32(dst, reg, MABS(param->value));									// and   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_and_r32_r32(dst, reg, param->value);										// and   reg,param
}


/*-------------------------------------------------
    emit_and_m32_p32 - and operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_and_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0xffffffff)
			/* skip */;
		else if (inst->condflags != 0 && (UINT32)param->value == 0)
			emit_mov_m32_imm(dst, MEMPARAMS, 0);										// mov   [dest],0
		else
			emit_and_m32_imm(dst, MEMPARAMS, param->value);								// and   [dest],param
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32(drcbe, dst, reg, param);										// mov   reg,param
		emit_and_m32_r32(dst, MEMPARAMS, reg);											// and   [dest],reg
	}
}


/*-------------------------------------------------
    emit_test_r32_p32 - test operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_test_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_test_r32_imm(dst, reg, param->value);										// test   reg,param
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_test_m32_r32(dst, MABS(param->value), reg);								// test   [param],reg
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_test_r32_r32(dst, reg, param->value);										// test   reg,param
}


/*-------------------------------------------------
    emit_test_m32_p32 - test operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_test_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_test_m32_imm(dst, MEMPARAMS, param->value);								// test  [dest],param
	else if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_mov_r32_p32(drcbe, dst, REG_EAX, param);									// mov   reg,param
		emit_test_m32_r32(dst, MEMPARAMS, REG_EAX);										// test  [dest],reg
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_test_m32_r32(dst, MEMPARAMS, param->value);								// test  [dest],param
}


/*-------------------------------------------------
    emit_or_r32_p32 - or operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_or_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->condflags != 0 && (UINT32)param->value == 0xffffffff)
			emit_mov_r32_imm(dst, reg, -1);												// mov  reg,-1
		else
			emit_or_r32_imm(dst, reg, param->value);									// or   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_or_r32_m32(dst, reg, MABS(param->value));									// or   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_or_r32_r32(dst, reg, param->value);										// or   reg,param
}


/*-------------------------------------------------
    emit_or_m32_p32 - or operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_or_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->condflags != 0 && (UINT32)param->value == 0xffffffff)
			emit_mov_m32_imm(dst, MEMPARAMS, -1);										// mov   [dest],-1
		else
			emit_or_m32_imm(dst, MEMPARAMS, param->value);								// or   [dest],param
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32(drcbe, dst, reg, param);										// mov   reg,param
		emit_or_m32_r32(dst, MEMPARAMS, reg);											// or   [dest],reg
	}
}


/*-------------------------------------------------
    emit_xor_r32_p32 - xor operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_xor_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->condflags != 0 && (UINT32)param->value == 0xffffffff)
			emit_not_r32(dst, reg);														// not   reg
		else
			emit_xor_r32_imm(dst, reg, param->value);									// xor   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_xor_r32_m32(dst, reg, MABS(param->value));									// xor   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_xor_r32_r32(dst, reg, param->value);										// xor   reg,param
}


/*-------------------------------------------------
    emit_xor_m32_p32 - xor operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_xor_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->condflags != 0 && (UINT32)param->value == 0xffffffff)
			emit_not_m32(dst, MEMPARAMS);												// not   [dest]
		else
			emit_xor_m32_imm(dst, MEMPARAMS, param->value);								// xor   [dest],param
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32(drcbe, dst, reg, param);										// mov   reg,param
		emit_xor_m32_r32(dst, MEMPARAMS, reg);											// xor   [dest],reg
	}
}


/*-------------------------------------------------
    emit_shl_r32_p32 - shl operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_shl_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_shl_r32_imm(dst, reg, param->value);									// shl   reg,param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_shl_r32_cl(dst, reg);														// shl   reg,cl
	}
}


/*-------------------------------------------------
    emit_shl_m32_p32 - shl operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_shl_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_shl_m32_imm(dst, MEMPARAMS, param->value);								// shl   [dest],param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_shl_m32_cl(dst, MEMPARAMS);												// shl   [dest],cl
	}
}


/*-------------------------------------------------
    emit_shr_r32_p32 - shr operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_shr_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_shr_r32_imm(dst, reg, param->value);									// shr   reg,param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_shr_r32_cl(dst, reg);														// shr   reg,cl
	}
}


/*-------------------------------------------------
    emit_shr_m32_p32 - shr operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_shr_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_shr_m32_imm(dst, MEMPARAMS, param->value);								// shr   [dest],param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_shr_m32_cl(dst, MEMPARAMS);												// shr   [dest],cl
	}
}


/*-------------------------------------------------
    emit_sar_r32_p32 - sar operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_sar_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_sar_r32_imm(dst, reg, param->value);									// sar   reg,param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_sar_r32_cl(dst, reg);														// sar   reg,cl
	}
}


/*-------------------------------------------------
    emit_sar_m32_p32 - sar operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_sar_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_sar_m32_imm(dst, MEMPARAMS, param->value);								// sar   [dest],param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_sar_m32_cl(dst, MEMPARAMS);												// sar   [dest],cl
	}
}


/*-------------------------------------------------
    emit_rol_r32_p32 - rol operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_rol_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rol_r32_imm(dst, reg, param->value);									// rol   reg,param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_rol_r32_cl(dst, reg);														// rol   reg,cl
	}
}


/*-------------------------------------------------
    emit_rol_m32_p32 - rol operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_rol_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rol_m32_imm(dst, MEMPARAMS, param->value);								// rol   [dest],param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_rol_m32_cl(dst, MEMPARAMS);												// rol   [dest],cl
	}
}


/*-------------------------------------------------
    emit_ror_r32_p32 - ror operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_ror_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_ror_r32_imm(dst, reg, param->value);									// ror   reg,param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_ror_r32_cl(dst, reg);														// ror   reg,cl
	}
}


/*-------------------------------------------------
    emit_ror_m32_p32 - ror operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_ror_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_ror_m32_imm(dst, MEMPARAMS, param->value);								// ror   [dest],param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_ror_m32_cl(dst, MEMPARAMS);												// ror   [dest],cl
	}
}


/*-------------------------------------------------
    emit_rcl_r32_p32 - rcl operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_rcl_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcl_r32_imm(dst, reg, param->value);									// rcl   reg,param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_rcl_r32_cl(dst, reg);														// rcl   reg,cl
	}
}


/*-------------------------------------------------
    emit_rcl_m32_p32 - rcl operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_rcl_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcl_m32_imm(dst, MEMPARAMS, param->value);								// rcl   [dest],param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_rcl_m32_cl(dst, MEMPARAMS);												// rcl   [dest],cl
	}
}


/*-------------------------------------------------
    emit_rcr_r32_p32 - rcr operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_rcr_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcr_r32_imm(dst, reg, param->value);									// rcr   reg,param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_rcr_r32_cl(dst, reg);														// rcr   reg,cl
	}
}


/*-------------------------------------------------
    emit_rcr_m32_p32 - rcr operation to a 32-bit
    memory location from a 32-bit parameter
-------------------------------------------------*/

static void emit_rcr_m32_p32(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcr_m32_imm(dst, MEMPARAMS, param->value);								// rcr   [dest],param
	}
	else
	{
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_rcr_m32_cl(dst, MEMPARAMS);												// rcr   [dest],cl
	}
}



/***************************************************************************
    EMITTERS FOR 64-BIT OPERATIONS WITH PARAMETERS
***************************************************************************/

/*-------------------------------------------------
    emit_mov_r64_p64 - move a 64-bit parameter
    into a pair of registers
-------------------------------------------------*/

static void emit_mov_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if ((UINT32)param->value == 0)
			emit_xor_r32_r32(dst, reglo, reglo);										// xor   reglo,reglo
		else
			emit_mov_r32_imm(dst, reglo, param->value);									// mov   reglo,param
		if ((UINT32)(param->value >> 32) == 0)
			emit_xor_r32_r32(dst, reghi, reghi);										// xor   reghi,reghi
		else
			emit_mov_r32_imm(dst, reghi, param->value >> 32);							// mov   reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_mov_r32_m32(dst, reglo, MABS(param->value));								// mov   reglo,[param]
		emit_mov_r32_m32(dst, reghi, MABS(param->value + 4));							// mov   reghi,[param+4]
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reglo != param->value)
			emit_mov_r32_r32(dst, reglo, param->value);									// mov   reglo,param
		emit_mov_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));					// mov   reghi,reghi[param]
	}
}


/*-------------------------------------------------
    emit_mov_p64_r64 - move a pair of registers
    into a 64-bit parameter
-------------------------------------------------*/

static void emit_mov_p64_r64(drcbe_state *drcbe, x86code **dst, const drcuml_parameter *param, UINT8 reglo, UINT8 reghi)
{
	assert(param->type != DRCUML_PTYPE_IMMEDIATE);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_mov_m32_r32(dst, MABS(param->value), reglo);								// mov   [param],reglo
		emit_mov_m32_r32(dst, MABS(param->value + 4), reghi);							// mov   [param+4],reghi
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reglo != param->value)
			emit_mov_r32_r32(dst, param->value, reglo);									// mov   param,reglo
		emit_mov_m32_r32(dst, MABS(drcbe->reghi[param->value]), reghi);					// mov   reghi[param],reghi
	}
}


/*-------------------------------------------------
    emit_push_p64 - push a 64-bit parameter onto
    the stack
-------------------------------------------------*/

static void emit_push_p64(drcbe_state *drcbe, x86code **dst, const drcuml_parameter *src)
{
	if (src->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_push_imm(dst, src->value >> 32);											// push  src >> 32
		emit_push_imm(dst, src->value);													// push  src
	}
	else if (src->type == DRCUML_PTYPE_MEMORY)
	{
		emit_push_m32(dst, MABS(src->value + 4));										// push  [src+4]
		emit_push_m32(dst, MABS(src->value));											// push  [src]
	}
	else if (src->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_push_m32(dst, MABS(drcbe->reghi[src->value]));								// push  reghi[src]
		emit_push_r32(dst, src->value);													// push  src
	}
}


/*-------------------------------------------------
    emit_add_r64_p64 - add operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_add_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_add_r32_m32(dst, reglo, MABS(param->value));								// add   reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_r32_m32(dst, reghi, MABS(param->value + 4));							// adc   reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_add_r32_imm(dst, reglo, param->value);										// add   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_r32_imm(dst, reghi, param->value >> 32);								// adc   reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_add_r32_r32(dst, reglo, param->value);										// add   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));					// adc   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_add_m64_p64 - add operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_add_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_add_m32_imm(dst, MEMPARAMS, param->value);									// add   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);						// adc   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64(drcbe, dst, reglo, REG_EDX, param);							// mov   edx:reglo,param
		emit_add_m32_r32(dst, MEMPARAMS, reglo);										// add   [dest],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_m32_r32(dst, MEMPARAMS + 4, REG_EDX);									// adc   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_adc_r64_p64 - adc operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_adc_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_adc_r32_m32(dst, reglo, MABS(param->value));								// adc   reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_r32_m32(dst, reghi, MABS(param->value + 4));							// adc   reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_adc_r32_imm(dst, reglo, param->value);										// adc   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_r32_imm(dst, reghi, param->value >> 32);								// adc   reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_adc_r32_r32(dst, reglo, param->value);										// adc   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));					// adc   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_adc_m64_p64 - adc operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_adc_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_adc_m32_imm(dst, MEMPARAMS, param->value);									// adc   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);						// adc   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64(drcbe, dst, reglo, REG_EDX, param);							// mov   edx:reglo,param
		emit_adc_m32_r32(dst, MEMPARAMS, reglo);										// adc   [dest],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_m32_r32(dst, MEMPARAMS + 4, REG_EDX);									// adc   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_sub_r64_p64 - sub operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_sub_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_sub_r32_m32(dst, reglo, MABS(param->value));								// sub   reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_r32_m32(dst, reghi, MABS(param->value + 4));							// sbb   reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_sub_r32_imm(dst, reglo, param->value);										// sub   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_r32_imm(dst, reghi, param->value >> 32);								// sbb   reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_sub_r32_r32(dst, reglo, param->value);										// sub   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));					// sbb   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_sub_m64_p64 - sub operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_sub_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_sub_m32_imm(dst, MEMPARAMS, param->value);									// sub   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);						// sbb   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64(drcbe, dst, reglo, REG_EDX, param);							// mov   edx:reglo,param
		emit_sub_m32_r32(dst, MEMPARAMS, reglo);										// sub   [dest],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_m32_r32(dst, MEMPARAMS + 4, REG_EDX);									// sbb   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_sbb_r64_p64 - sbb operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_sbb_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_sbb_r32_m32(dst, reglo, MABS(param->value));								// sbb   reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_r32_m32(dst, reghi, MABS(param->value + 4));							// sbb   reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_sbb_r32_imm(dst, reglo, param->value);										// sbb   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_r32_imm(dst, reghi, param->value >> 32);								// sbb   reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_sbb_r32_r32(dst, reglo, param->value);										// sbb   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));					// sbb   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_sbb_m64_p64 - sbb operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_sbb_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_sbb_m32_imm(dst, MEMPARAMS, param->value);									// sbb   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);						// sbb   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64(drcbe, dst, reglo, REG_EDX, param);							// mov   edx:reglo,param
		emit_sbb_m32_r32(dst, MEMPARAMS, reglo);										// sbb   [dest],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_m32_r32(dst, MEMPARAMS + 4, REG_EDX);									// sbb   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_and_r64_p64 - and operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_and_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_and_r32_m32(dst, reglo, MABS(param->value));								// and   reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_and_r32_m32(dst, reghi, MABS(param->value + 4));							// and   reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0xffffffff)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)param->value == 0)
			emit_xor_r32_r32(dst, reglo, reglo);										// xor   reglo,reglo
		else
			emit_and_r32_imm(dst, reglo, param->value);									// and   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0)
			emit_xor_r32_r32(dst, reghi, reghi);										// xor   reghi,reghi
		else
			emit_and_r32_imm(dst, reghi, param->value >> 32);							// and   reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_and_r32_r32(dst, reglo, param->value);										// and   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_and_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));					// and   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_and_m64_p64 - and operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_and_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0xffffffff)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)param->value == 0)
			emit_mov_m32_imm(dst, MEMPARAMS, 0);										// mov   [dest],0
		else
			emit_and_m32_imm(dst, MEMPARAMS, param->value);								// and   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0)
			emit_mov_m32_imm(dst, MEMPARAMS + 4, 0);									// mov   [dest+4],0
		else
			emit_and_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);					// and   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64(drcbe, dst, reglo, REG_EDX, param);							// mov   edx:reglo,param
		emit_and_m32_r32(dst, MEMPARAMS, reglo);										// and   [dest],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_and_m32_r32(dst, MEMPARAMS + 4, REG_EDX);									// and   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_test_r64_p64 - test operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_test_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_test_m32_r32(dst, MABS(param->value), reglo);								// test  [param],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_test_m32_r32(dst, MABS(param->value + 4), reghi);							// test  [param],reghi
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_test_r32_imm(dst, reglo, param->value);									// test  reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_test_r32_imm(dst, reghi, param->value >> 32);								// test  reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_test_r32_r32(dst, reglo, param->value);									// test  reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_test_m32_r32(dst, MABS(drcbe->reghi[param->value]), reghi);				// test  reghi[param],reghi
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_test_m64_p64 - test operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_test_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_test_m32_imm(dst, MEMPARAMS, param->value);								// test   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_test_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);						// test   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64(drcbe, dst, reglo, REG_EDX, param);							// mov   edx:reglo,param
		emit_test_m32_r32(dst, MEMPARAMS, reglo);										// test  [dest],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_test_m32_r32(dst, MEMPARAMS + 4, REG_EDX);									// test  [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_or_r64_p64 - or operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_or_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_or_r32_m32(dst, reglo, MABS(param->value));								// or    reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_or_r32_m32(dst, reghi, MABS(param->value + 4));							// or    reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)param->value == 0xffffffff)
			emit_mov_r32_imm(dst, reglo, -1);											// mov   reglo,-1
		else
			emit_or_r32_imm(dst, reglo, param->value);									// or    reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			emit_mov_r32_imm(dst, reghi, -1);											// mov   reghi,-1
		else
			emit_or_r32_imm(dst, reghi, param->value >> 32);							// or    reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_or_r32_r32(dst, reglo, param->value);										// or    reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_or_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));					// or    reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_or_m64_p64 - or operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_or_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)param->value == 0xffffffff)
			emit_mov_m32_imm(dst, MEMPARAMS, -1);										// mov   [dest],-1
		else
			emit_or_m32_imm(dst, MEMPARAMS, param->value);								// or    [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			emit_mov_m32_imm(dst, MEMPARAMS + 4, -1);									// mov   [dest+4],-1
		else
			emit_or_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);					// or    [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64(drcbe, dst, reglo, REG_EDX, param);							// mov   edx:reglo,param
		emit_or_m32_r32(dst, MEMPARAMS, reglo);											// or    [dest],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_or_m32_r32(dst, MEMPARAMS + 4, REG_EDX);									// or    [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_xor_r64_p64 - xor operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_xor_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_xor_r32_m32(dst, reglo, MABS(param->value));								// xor   reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_xor_r32_m32(dst, reghi, MABS(param->value + 4));							// xor   reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)param->value == 0xffffffff)
			emit_not_r32(dst, reglo);													// not   reglo
		else
			emit_xor_r32_imm(dst, reglo, param->value);									// xor   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			emit_not_r32(dst, reghi);													// not   reghi
		else
			emit_xor_r32_imm(dst, reghi, param->value >> 32);							// xor   reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_xor_r32_r32(dst, reglo, param->value);										// xor   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_xor_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));					// xor   reghi,reghi[param]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_xor_m64_p64 - xor operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_xor_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->condflags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)param->value == 0xffffffff)
			emit_not_m32(dst, MEMPARAMS);												// not   [dest]
		else
			emit_xor_m32_imm(dst, MEMPARAMS, param->value);								// xor   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0)
			/* skip */;
		else if (inst->condflags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			emit_not_m32(dst, MEMPARAMS + 4);											// not   [dest+4]
		else
			emit_xor_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);					// xor   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64(drcbe, dst, reglo, REG_EDX, param);							// mov   edx:reglo,param
		emit_xor_m32_r32(dst, MEMPARAMS, reglo);										// xor   [dest],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_xor_m32_r32(dst, MEMPARAMS + 4, REG_EDX);									// xor   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_shl_r64_p64 - shl operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_shl_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = (inst->condflags != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->condflags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->condflags != 0)
				{
					emit_shld_r32_r32_imm(dst, reghi, reglo, 31);						// shld  reghi,reglo,31
					emit_shl_r32_imm(dst, reglo, 31);									// shl   reglo,31
					count -= 31;
				}
				else
				{
					emit_mov_r32_r32(dst, reghi, reglo);								// mov   reghi,reglo
					emit_xor_r32_r32(dst, reglo, reglo);								// xor   reglo,reglo
					count -= 32;
				}
			}
			if (inst->condflags != 0 || count > 0)
			{
				emit_shld_r32_r32_imm(dst, reghi, reglo, count);						// shld  reghi,reglo,count
				if (saveflags) emit_pushf(dst);											// pushf
				emit_shl_r32_imm(dst, reglo, count);									// shl   reglo,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);											// test  ecx,0x20
		emit_jcc_short_link(dst, COND_Z, &skip1);										// jz    skip1
		if (inst->condflags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shld_r32_r32_imm(dst, reghi, reglo, 31);								// shld  reghi,reglo,31
			emit_shl_r32_imm(dst, reglo, 31);											// shl   reglo,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shld_r32_r32_imm(dst, reghi, reglo, 31);								// shld  reghi,reglo,31
			emit_shl_r32_imm(dst, reglo, 31);											// shl   reglo,31
			resolve_link(dst, &skip2);												// skip2:
		}
		else
		{
			emit_mov_r32_r32(dst, reghi, reglo);										// mov   reghi,reglo
			emit_xor_r32_r32(dst, reglo, reglo);										// xor   reglo,reglo
		}
		resolve_link(dst, &skip1);													// skip1:
		emit_shld_r32_r32_cl(dst, reghi, reglo);										// shld  reghi,reglo,cl
		if (saveflags) emit_pushf(dst);													// pushf
		emit_shl_r32_cl(dst, reglo);													// shl   reglo,cl
	}
	if (saveflags)
		emit_combine_z_shl_flags(dst);
}


/*-------------------------------------------------
    emit_shr_r64_p64 - shr operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_shr_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->condflags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->condflags != 0)
				{
					emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);						// shrd  reglo,reghi,31
					emit_shr_r32_imm(dst, reghi, 31);									// shr   reghi,31
					count -= 31;
				}
				else
				{
					emit_mov_r32_r32(dst, reglo, reghi);								// mov   reglo,reghi
					emit_xor_r32_r32(dst, reghi, reghi);								// xor   reghi,reghi
					count -= 32;
				}
			}
			if (inst->condflags != 0 || count > 0)
			{
				emit_shrd_r32_r32_imm(dst, reglo, reghi, count);						// shrd  reglo,reghi,count
				if (saveflags) emit_pushf(dst);											// pushf
				emit_shr_r32_imm(dst, reghi, count);									// shr   reghi,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);											// test  ecx,0x20
		emit_jcc_short_link(dst, COND_Z, &skip1);										// jz    skip1
		if (inst->condflags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_shr_r32_imm(dst, reghi, 31);											// shr   reghi,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_shr_r32_imm(dst, reghi, 31);											// shr   reghi,31
			resolve_link(dst, &skip2);												// skip2:
		}
		else
		{
			emit_mov_r32_r32(dst, reglo, reghi);										// mov   reglo,reghi
			emit_xor_r32_r32(dst, reghi, reghi);										// xor   reghi,reghi
		}
		resolve_link(dst, &skip1);													// skip1:
		emit_shrd_r32_r32_cl(dst, reglo, reghi);										// shrd  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);													// pushf
		emit_shr_r32_cl(dst, reghi);													// shr   reghi,cl
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_sar_r64_p64 - sar operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_sar_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->condflags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->condflags != 0)
				{
					emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);						// shrd  reglo,reghi,31
					emit_sar_r32_imm(dst, reghi, 31);									// sar   reghi,31
					count -= 31;
				}
				else
				{
					emit_mov_r32_r32(dst, reglo, reghi);								// mov   reglo,reghi
					emit_sar_r32_imm(dst, reghi, 31);									// sar   reghi,31
					count -= 32;
				}
			}
			if (inst->condflags != 0 || count > 0)
			{
				emit_shrd_r32_r32_imm(dst, reglo, reghi, count);						// shrd  reglo,reghi,count
				if (saveflags) emit_pushf(dst);											// pushf
				emit_sar_r32_imm(dst, reghi, count);									// sar   reghi,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);											// test  ecx,0x20
		emit_jcc_short_link(dst, COND_Z, &skip1);										// jz    skip1
		if (inst->condflags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_sar_r32_imm(dst, reghi, 31);											// sar   reghi,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_sar_r32_imm(dst, reghi, 31);											// sar   reghi,31
			resolve_link(dst, &skip2);												// skip2:
		}
		else
		{
			emit_mov_r32_r32(dst, reglo, reghi);										// mov   reglo,reghi
			emit_sar_r32_imm(dst, reghi, 31);											// sar   reghi,31
		}
		resolve_link(dst, &skip1);													// skip1:
		emit_shrd_r32_r32_cl(dst, reglo, reghi);										// shrd  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sar_r32_cl(dst, reghi);													// sar   reghi,cl
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_rol_r64_p64 - rol operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_rol_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->condflags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->condflags != 0)
				{
					emit_mov_r32_r32(dst, REG_ECX, reglo);								// mov   ecx,reglo
					emit_shld_r32_r32_imm(dst, reglo, reghi, 31);						// shld  reglo,reghi,31
					emit_shld_r32_r32_imm(dst, reghi, REG_ECX, 31);						// shld  reghi,ecx,31
					count -= 31;
				}
				else
				{
					emit_xchg_r32_r32(dst, reghi, reglo);								// xchg  reghi,reglo
					count -= 32;
				}
			}
			if (inst->condflags != 0 || count > 0)
			{
				emit_mov_r32_r32(dst, REG_ECX, reglo);									// mov   ecx,reglo
				emit_shld_r32_r32_imm(dst, reglo, reghi, count);						// shld  reglo,reghi,count
				if (saveflags) emit_pushf(dst);											// pushf
				emit_shld_r32_r32_imm(dst, reghi, REG_ECX, count);						// shld  reghi,ecx,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		emit_mov_m32_r32(dst, MBD(REG_ESP, -8), REG_EBX);								// mov   [esp-8],ebx
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);											// test  ecx,0x20
		emit_jcc_short_link(dst, COND_Z, &skip1);										// jz    skip1
		if (inst->condflags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_mov_r32_r32(dst, REG_EBX, reglo);										// mov   ebx,reglo
			emit_shld_r32_r32_imm(dst, reglo, reghi, 31);								// shld  reglo,reghi,31
			emit_shld_r32_r32_imm(dst, reghi, REG_EBX, 31);								// shld  reghi,ebx,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_mov_r32_r32(dst, REG_EBX, reglo);										// mov   ebx,reglo
			emit_shld_r32_r32_imm(dst, reglo, reghi, 31);								// shld  reglo,reghi,31
			emit_shld_r32_r32_imm(dst, reghi, REG_EBX, 31);								// shld  reghi,ebx,31
			resolve_link(dst, &skip2);												// skip2:
		}
		else
			emit_xchg_r32_r32(dst, reghi, reglo);										// xchg  reghi,reglo
		resolve_link(dst, &skip1);													// skip1:
		emit_mov_r32_r32(dst, REG_EBX, reglo);											// mov   ebx,reglo
		emit_shld_r32_r32_cl(dst, reglo, reghi);										// shld  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);													// pushf
		emit_shld_r32_r32_cl(dst, reghi, REG_EBX);										// shld  reghi,ebx,cl
		emit_mov_r32_m32(dst, REG_EBX, MBD(REG_ESP, saveflags ? -4 : -8));				// mov   ebx,[esp-8]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_ror_r64_p64 - ror operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_ror_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->condflags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->condflags != 0)
				{
					emit_mov_r32_r32(dst, REG_ECX, reglo);								// mov   ecx,reglo
					emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);						// shrd  reglo,reghi,31
					emit_shrd_r32_r32_imm(dst, reghi, REG_ECX, 31);						// shrd  reghi,ecx,31
					count -= 31;
				}
				else
				{
					emit_xchg_r32_r32(dst, reghi, reglo);								// xchg  reghi,reglo
					count -= 32;
				}
			}
			if (inst->condflags != 0 || count > 0)
			{
				emit_mov_r32_r32(dst, REG_ECX, reglo);									// mov   ecx,reglo
				emit_shrd_r32_r32_imm(dst, reglo, reghi, count);						// shrd  reglo,reghi,count
				if (saveflags) emit_pushf(dst);											// pushf
				emit_shrd_r32_r32_imm(dst, reghi, REG_ECX, count);						// shrd  reghi,ecx,count
			}
		}
	}
	else
	{
		emit_link skip1, skip2;
		emit_mov_m32_r32(dst, MBD(REG_ESP, -8), REG_EBX);								// mov   [esp-8],ebx
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);											// test  ecx,0x20
		emit_jcc_short_link(dst, COND_Z, &skip1);										// jz    skip1
		if (inst->condflags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_mov_r32_r32(dst, REG_EBX, reglo);										// mov   ebx,reglo
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_shrd_r32_r32_imm(dst, reghi, REG_EBX, 31);								// shrd  reghi,ebx,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_mov_r32_r32(dst, REG_EBX, reglo);										// mov   ebx,reglo
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_shrd_r32_r32_imm(dst, reghi, REG_EBX, 31);								// shrd  reghi,ebx,31
			resolve_link(dst, &skip2);												// skip2:
		}
		else
			emit_xchg_r32_r32(dst, reghi, reglo);										// xchg  reghi,reglo
		resolve_link(dst, &skip1);													// skip1:
		emit_mov_r32_r32(dst, REG_EBX, reglo);											// mov   ebx,reglo
		emit_shrd_r32_r32_cl(dst, reglo, reghi);										// shrd  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);													// pushf
		emit_shrd_r32_r32_cl(dst, reghi, REG_EBX);										// shrd  reghi,ebx,cl
		emit_mov_r32_m32(dst, REG_EBX, MBD(REG_ESP, saveflags ? -4 : -8));				// mov   ebx,[esp-8]
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_rcl_r64_p64 - rcl operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_rcl_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->condflags & DRCUML_FLAG_Z) != 0);
	emit_link skipall, skiploop;
	x86code *loop;

	emit_mov_r32_p32(drcbe, dst, REG_ECX, param);										// mov   ecx,param
	if (!saveflags)
	{
		loop = *dst;																// loop:
		emit_jecxz_link(dst, &skipall);													// jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));								// lea   ecx,[ecx-1]
		emit_rcl_r32_imm(dst, reglo, 1);												// rcl   reglo,1
		emit_rcl_r32_imm(dst, reghi, 1);												// rcl   reghi,1
		emit_jmp(dst, loop);															// jmp   loop
		resolve_link(dst, &skipall);												// skipall:
	}
	else
	{
		emit_jecxz_link(dst, &skipall);													// jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));								// lea   ecx,[ecx-1]
		loop = *dst;																// loop:
		emit_jecxz_link(dst, &skiploop);												// jecxz skiploop
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));								// lea   ecx,[ecx-1]
		emit_rcl_r32_imm(dst, reglo, 1);												// rcl   reglo,1
		emit_rcl_r32_imm(dst, reghi, 1);												// rcl   reghi,1
		emit_jmp(dst, loop);															// jmp   loop
		resolve_link(dst, &skiploop);												// skiploop:
		emit_rcl_r32_imm(dst, reglo, 1);												// rcl   reglo,1
		emit_pushf(dst);																// pushf
		emit_rcl_r32_imm(dst, reghi, 1);												// rcl   reghi,1
		resolve_link(dst, &skipall);												// skipall:
		emit_combine_z_flags(dst);
	}
}


/*-------------------------------------------------
    emit_rcr_r64_p64 - rcr operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_rcr_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = (inst->condflags != 0);
	emit_link skipall, skiploop;
	x86code *loop;

	emit_mov_r32_p32(drcbe, dst, REG_ECX, param);										// mov   ecx,param
	if (!saveflags)
	{
		loop = *dst;																// loop:
		emit_jecxz_link(dst, &skipall);													// jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));								// lea   ecx,[ecx-1]
		emit_rcr_r32_imm(dst, reghi, 1);												// rcr   reghi,1
		emit_rcr_r32_imm(dst, reglo, 1);												// rcr   reglo,1
		emit_jmp(dst, loop);															// jmp   loop
		resolve_link(dst, &skipall);												// skipall:
	}
	else
	{
		emit_jecxz_link(dst, &skipall);													// jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));								// lea   ecx,[ecx-1]
		loop = *dst;																// loop:
		emit_jecxz_link(dst, &skiploop);												// jecxz skiploop
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));								// lea   ecx,[ecx-1]
		emit_rcr_r32_imm(dst, reghi, 1);												// rcr   reghi,1
		emit_rcr_r32_imm(dst, reglo, 1);												// rcr   reglo,1
		emit_jmp(dst, loop);															// jmp   loop
		resolve_link(dst, &skiploop);												// skiploop:
		emit_rcr_r32_imm(dst, reghi, 1);												// rcr   reghi,1
		emit_pushf(dst);																// pushf
		emit_rcr_r32_imm(dst, reglo, 1);												// rcr   reglo,1
		resolve_link(dst, &skipall);												// skipall:
		emit_combine_z_shl_flags(dst);
	}
}



/***************************************************************************
    EMITTERS FOR FLOATING POINT
***************************************************************************/

/*-------------------------------------------------
    emit_fld_p - load a floating point parameter
    onto the stack
-------------------------------------------------*/

static void emit_fld_p(x86code **dst, int size, const drcuml_parameter *param)
{
	assert(param->type == DRCUML_PTYPE_MEMORY);
	assert(size == 4 || size == 8);
	if (size == 4)
		emit_fld_m32(dst, MABS(param->value));
	else if (size == 8)
		emit_fld_m64(dst, MABS(param->value));
}


/*-------------------------------------------------
    emit_fstp_p - store a floating point parameter
    from the stack and pop it
-------------------------------------------------*/

static void emit_fstp_p(x86code **dst, int size, const drcuml_parameter *param)
{
	assert(param->type == DRCUML_PTYPE_MEMORY);
	assert(size == 4 || size == 8);
	if (size == 4)
		emit_fstp_m32(dst, MABS(param->value));
	else if (size == 8)
		emit_fstp_m64(dst, MABS(param->value));
}



/***************************************************************************
    HELPERS TO CONVERT OTHER OPERATIONS TO MOVES
***************************************************************************/

/*-------------------------------------------------
    convert_to_mov_imm - convert an instruction
    into a mov immediate
-------------------------------------------------*/

static x86code *convert_to_mov_imm(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst, const drcuml_parameter *dstp, UINT64 imm)
{
	drcuml_instruction temp = *inst;
	temp.numparams = 2;
	temp.param[0] = *dstp;
	temp.param[1].type = DRCUML_PTYPE_IMMEDIATE;
	temp.param[1].value = imm;
	return op_mov(drcbe, dst, &temp);
}


/*-------------------------------------------------
    convert_to_mov_src1 - convert an instruction
    into a mov src1
-------------------------------------------------*/

static x86code *convert_to_mov_src1(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst, const drcuml_parameter *dstp, const drcuml_parameter *srcp)
{
	drcuml_instruction temp = *inst;
	temp.numparams = 2;
	temp.param[0] = *dstp;
	temp.param[1] = *srcp;
	return op_mov(drcbe, dst, &temp);
}



/***************************************************************************
    OUT-OF-BAND CODE FIXUP CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    fixup_label - callback to fixup forward-
    referenced labels
-------------------------------------------------*/

static void fixup_label(void *parameter, drccodeptr labelcodeptr)
{
	drccodeptr src = parameter;

	/* find the end of the instruction */
	if (src[0] == 0xe3)
	{
		src += 1 + 1;
		src[-1] = labelcodeptr - src;
	}
	else if (src[0] == 0xe9)
	{
		src += 1 + 4;
		((UINT32 *)src)[-1] = labelcodeptr - src;
	}
	else if (src[0] == 0x0f && (src[1] & 0xf0) == 0x80)
	{
		src += 2 + 4;
		((UINT32 *)src)[-1] = labelcodeptr - src;
	}
	else
		fatalerror("fixup_label called with invalid jmp source!");
}


/*-------------------------------------------------
    fixup_exception - callback to perform cleanup
    and jump to an exception handler
-------------------------------------------------*/

static void fixup_exception(drccodeptr *codeptr, void *param1, void *param2, void *param3)
{
	drcuml_parameter handp, exp;
	drcbe_state *drcbe = param1;
	drccodeptr src = param2;
	const drcuml_instruction *inst = param3;
	drccodeptr dst = *codeptr;
	drccodeptr *targetptr;

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &handp, PTYPE_M, &exp, PTYPE_MRI);

	/* look up the handle target */
	targetptr = drcuml_handle_codeptr_addr((drcuml_codehandle *)(FPTR)handp.value);

	/* first fixup the jump to get us here */
	((UINT32 *)src)[-1] = dst - src;

	/* then store the exception parameter */
	emit_mov_m32_p32(drcbe, &dst, MABS(&drcbe->state.exp), &exp);						// mov   [exp],exp

	/* push the original return address on the stack */
	emit_push_imm(&dst, (FPTR)src);														// push  <return>
	if (*targetptr != NULL)
		emit_jmp(&dst, *targetptr);														// jmp   *targetptr
	else
		emit_jmp_m32(&dst, MABS(targetptr));											// jmp   [targetptr]

	*codeptr = dst;
}



/***************************************************************************
    DEBUG HELPERS
***************************************************************************/

/*-------------------------------------------------
    debug_log_hashjmp - callback to handle
    logging of hashjmps
-------------------------------------------------*/

#if LOG_HASHJMPS
static void debug_log_hashjmp(int mode, offs_t pc)
{
	printf("mode=%d PC=%08X\n", mode, pc);
}
#endif



/***************************************************************************
    COMPILE-TIME OPCODES
***************************************************************************/

/*-------------------------------------------------
    op_handle - process a HANDLE opcode
-------------------------------------------------*/

static x86code *op_handle(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	assert(inst->numparams == 1);
	assert(inst->param[0].type == DRCUML_PTYPE_MEMORY);

	/* register the current pointer for the handle */
	drcuml_handle_set_codeptr((drcuml_codehandle *)(FPTR)inst->param[0].value, dst);
	return dst;
}


/*-------------------------------------------------
    op_hash - process a HASH opcode
-------------------------------------------------*/

static x86code *op_hash(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	assert(inst->numparams == 2);
	assert(inst->param[0].type == DRCUML_PTYPE_IMMEDIATE);
	assert(inst->param[1].type == DRCUML_PTYPE_IMMEDIATE);

	/* register the current pointer for the mode/PC */
	drchash_set_codeptr(drcbe->hash, inst->param[0].value, inst->param[1].value, dst);
	return dst;
}


/*-------------------------------------------------
    op_label - process a LABEL opcode
-------------------------------------------------*/

static x86code *op_label(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	assert(inst->numparams == 1);
	assert(inst->param[0].type == DRCUML_PTYPE_IMMEDIATE);

	/* register the current pointer for the label */
	drclabel_set_codeptr(drcbe->labels, inst->param[0].value, dst);
	return dst;
}


/*-------------------------------------------------
    op_comment - process a COMMENT opcode
-------------------------------------------------*/

static x86code *op_comment(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	assert(inst->numparams == 1);
	assert(inst->param[0].type == DRCUML_PTYPE_MEMORY);

	/* do nothing */
	return dst;
}


/*-------------------------------------------------
    op_mapvar - process a MAPVAR opcode
-------------------------------------------------*/

static x86code *op_mapvar(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	assert(inst->numparams == 2);
	assert(inst->param[0].type == DRCUML_PTYPE_MAPVAR);
	assert(inst->param[1].type == DRCUML_PTYPE_IMMEDIATE);

	/* set the value of the specified mapvar */
	drcmap_set_value(drcbe->map, dst, inst->param[0].value, inst->param[1].value);
	return dst;
}



/***************************************************************************
    CONTROL FLOW OPCODES
***************************************************************************/

/*-------------------------------------------------
    op_debug - process a DEBUG opcode
-------------------------------------------------*/

static x86code *op_debug(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
#ifdef ENABLE_DEBUGGER
	if (Machine->debug_mode)
	{
		drcuml_parameter pcp;

		/* validate instruction */
		assert(inst->size == 4);
		assert(inst->condflags == DRCUML_COND_ALWAYS);

		/* normalize parameters */
		param_normalize_1(drcbe, inst, &pcp, PTYPE_MRI);

		/* push the parameter */
		emit_push_p32(drcbe, &dst, &pcp);												// push  pcp
		emit_call(&dst, (x86code *)mame_debug_hook);									// call  mame_debug_hook
		emit_add_r32_imm(&dst, REG_ESP, 4);												// add   esp,4
	}
#endif

	return dst;
}


/*-------------------------------------------------
    op_exit - process an EXIT opcode
-------------------------------------------------*/

static x86code *op_exit(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter retp;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS || (inst->condflags >= DRCUML_COND_Z && inst->condflags < DRCUML_COND_MAX));

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &retp, PTYPE_MRI);

	/* load the parameter into EAX */
	emit_mov_r32_p32(drcbe, &dst, REG_EAX, &retp);										// mov   eax,retp
	if (inst->condflags == DRCUML_COND_ALWAYS)
		emit_jmp(&dst, drcbe->exit);													// jmp   exit
	else
		emit_jcc(&dst, X86_CONDITION(inst->condflags), drcbe->exit);					// jcc   exit

	return dst;
}


/*-------------------------------------------------
    op_hashjmp - process a HASHJMP opcode
-------------------------------------------------*/

static x86code *op_hashjmp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter modep, pcp, exp;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &modep, PTYPE_MRI, &pcp, PTYPE_MRI, &exp, PTYPE_M);

#if LOG_HASHJMPS
	emit_push_p32(drcbe, &dst, &pcp);
	emit_push_p32(drcbe, &dst, &modep);
	emit_call(&dst, (x86code *)debug_log_hashjmp);
	emit_add_r32_imm(&dst, REG_ESP, 8);
#endif

	/* load the stack base one word early so we end up at the right spot after our call below */
	emit_mov_r32_m32(&dst, REG_ESP, MABS(&drcbe->hashstacksave));						// mov   esp,[hashstacksave]

	/* fixed mode cases */
	if (modep.type == DRCUML_PTYPE_IMMEDIATE)
	{
		/* a straight immediate jump is direct, though we need the PC in EAX in case of failure */
		if (pcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			UINT32 l1val = (pcp.value >> drcbe->hash->l1shift) & drcbe->hash->l1mask;
			UINT32 l2val = (pcp.value >> drcbe->hash->l2shift) & drcbe->hash->l2mask;
			emit_call_m32(&dst, MABS(&drcbe->hash->base[modep.value][l1val][l2val]));	// call  hash[modep][l1val][l2val]
		}

		/* a fixed mode but variable PC */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, REG_EAX, &pcp);								// mov   eax,pcp
			emit_mov_r32_r32(&dst, REG_EDX, REG_EAX);									// mov   edx,eax
			emit_shr_r32_imm(&dst, REG_EDX, drcbe->hash->l1shift);						// shr   edx,l1shift
			emit_and_r32_imm(&dst, REG_EAX, drcbe->hash->l2mask << drcbe->hash->l2shift);// and  eax,l2mask << l2shift
			emit_mov_r32_m32(&dst, REG_EDX, MISD(REG_EDX, 4, &drcbe->hash->base[modep.value][0]));
																						// mov   edx,hash[modep+edx*4]
			emit_call_m32(&dst, MBISD(REG_EDX, REG_EAX, 4 >> drcbe->hash->l2shift, 0));	// call  [edx+eax*shift]
		}
	}
	else
	{
		/* variable mode */
		int modereg = param_select_register(REG_ECX, &modep, NULL);
		emit_mov_r32_p32(drcbe, &dst, modereg, &modep);									// mov   modereg,modep
		emit_mov_r32_m32(&dst, REG_ECX, MISD(modereg, 4, &drcbe->hash->base[0]));		// mov   ecx,hash[modereg*4]

		/* fixed PC */
		if (pcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			UINT32 l1val = (pcp.value >> drcbe->hash->l1shift) & drcbe->hash->l1mask;
			UINT32 l2val = (pcp.value >> drcbe->hash->l2shift) & drcbe->hash->l2mask;
			emit_mov_r32_m32(&dst, REG_EDX, MBD(REG_ECX, l1val*4));						// mov   edx,[ecx+l1val*4]
			emit_call_m32(&dst, MBD(REG_EDX, l2val*4));									// call  [l2val*4]
		}

		/* variable PC */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, REG_EAX, &pcp);								// mov   eax,pcp
			emit_mov_r32_r32(&dst, REG_EDX, REG_EAX);									// mov   edx,eax
			emit_shr_r32_imm(&dst, REG_EDX, drcbe->hash->l1shift);						// shr   edx,l1shift
			emit_mov_r32_m32(&dst, REG_EDX, MBISD(REG_ECX, REG_EDX, 4, 0));				// mov   edx,[ecx+edx*4]
			emit_and_r32_imm(&dst, REG_EAX, drcbe->hash->l2mask << drcbe->hash->l2shift);// and  eax,l2mask << l2shift
			emit_call_m32(&dst, MBISD(REG_EDX, REG_EAX, 4 >> drcbe->hash->l2shift, 0));	// call  [edx+eax*shift]
		}
	}

	/* in all cases, if there is no code, we return here to generate the exception */
	emit_mov_m32_p32(drcbe, &dst, MABS(&drcbe->state.exp), &pcp);						// mov   [exp],param
	emit_sub_r32_imm(&dst, REG_ESP, 4);													// sub   esp,4
	emit_call_m32(&dst, MABS(exp.value));												// call  [exp]

	return dst;
}


/*-------------------------------------------------
    op_jmp - process a JMP opcode
-------------------------------------------------*/

static x86code *op_jmp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter labelp;
	x86code *jmptarget;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS || (inst->condflags >= DRCUML_COND_Z && inst->condflags < DRCUML_COND_MAX));

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &labelp, PTYPE_I);

	/* look up the jump target and jump there */
	jmptarget = (x86code *)drclabel_get_codeptr(drcbe->labels, labelp.value, fixup_label, dst);
	if (inst->condflags == DRCUML_COND_ALWAYS)
		emit_jmp(&dst, jmptarget);														// jmp   target
	else
		emit_jcc(&dst, X86_CONDITION(inst->condflags), jmptarget);						// jcc   target

	return dst;
}


/*-------------------------------------------------
    op_exh - process an EXH opcode
-------------------------------------------------*/

static x86code *op_exh(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter handp, exp;
	drccodeptr *targetptr;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS || (inst->condflags >= DRCUML_COND_Z && inst->condflags < DRCUML_COND_MAX));

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &handp, PTYPE_M, &exp, PTYPE_MRI);

	/* look up the handle target */
	targetptr = drcuml_handle_codeptr_addr((drcuml_codehandle *)(FPTR)handp.value);

	/* perform the exception processing inline if unconditional */
	if (inst->condflags == DRCUML_COND_ALWAYS)
	{
		emit_mov_m32_p32(drcbe, &dst, MABS(&drcbe->state.exp), &exp);					// mov   [exp],exp
		if (*targetptr != NULL)
			emit_call(&dst, *targetptr);												// call  *targetptr
		else
			emit_call_m32(&dst, MABS(targetptr));										// call  [targetptr]
	}

	/* otherwise, jump to an out-of-band handler */
	else
	{
		emit_jcc(&dst, X86_CONDITION(inst->condflags), 0);								// jcc   exception
		drccache_request_oob_codegen(drcbe->cache, fixup_exception, drcbe, dst, (void *)inst);
	}
	return dst;
}


/*-------------------------------------------------
    op_callh - process a CALLH opcode
-------------------------------------------------*/

static x86code *op_callh(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter handp;
	drccodeptr *targetptr;
	emit_link skip = { 0 };

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS || (inst->condflags >= DRCUML_COND_Z && inst->condflags < DRCUML_COND_MAX));

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &handp, PTYPE_M);

	/* look up the handle target */
	targetptr = drcuml_handle_codeptr_addr((drcuml_codehandle *)(FPTR)handp.value);

	/* skip if conditional */
	if (inst->condflags != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condflags), &skip);			// jcc   skip

	/* jump through the handle; directly if a normal jump */
	if (*targetptr != NULL)
		emit_call(&dst, *targetptr);													// call  *targetptr
	else
		emit_call_m32(&dst, MABS(targetptr));											// call  [targetptr]

	/* resolve the conditional link */
	if (inst->condflags != DRCUML_COND_ALWAYS)
		resolve_link(&dst, &skip);													// skip:
	return dst;
}


/*-------------------------------------------------
    op_ret - process a RET opcode
-------------------------------------------------*/

static x86code *op_ret(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	emit_link skip = { 0 };

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS || (inst->condflags >= DRCUML_COND_Z && inst->condflags < DRCUML_COND_MAX));
	assert(inst->numparams == 0);

	/* skip if conditional */
	if (inst->condflags != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condflags), &skip);			// jcc   skip

	/* return */
	emit_ret(&dst);																		// ret

	/* resolve the conditional link */
	if (inst->condflags != DRCUML_COND_ALWAYS)
		resolve_link(&dst, &skip);													// skip:
	return dst;
}


/*-------------------------------------------------
    op_callc - process a CALLC opcode
-------------------------------------------------*/

static x86code *op_callc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter funcp, paramp;
	emit_link skip = { 0 };

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS || (inst->condflags >= DRCUML_COND_Z && inst->condflags < DRCUML_COND_MAX));

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &funcp, PTYPE_M, &paramp, PTYPE_M);

	/* skip if conditional */
	if (inst->condflags != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condflags), &skip);			// jcc   skip

	/* perform the call */
	emit_push_imm(&dst, paramp.value);													// push  paramp
	emit_call(&dst, (x86code *)(FPTR)funcp.value);										// call  funcp
	emit_add_r32_imm(&dst, REG_ESP, 4);													// add   esp,4

	/* resolve the conditional link */
	if (inst->condflags != DRCUML_COND_ALWAYS)
		resolve_link(&dst, &skip);													// skip:
	return dst;
}


/*-------------------------------------------------
    op_recover - process a RECOVER opcode
-------------------------------------------------*/

static x86code *op_recover(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, mvparam;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &mvparam, PTYPE_I);

	/* call the recovery code */
	emit_mov_r32_m32(&dst, REG_EAX, MABS(&drcbe->stacksave));							// mov   eax,stacksave
	emit_mov_r32_m32(&dst, REG_EAX, MBD(REG_EAX, -4));									// mov   eax,[eax-4]
	emit_sub_r32_imm(&dst, REG_EAX, 1);													// sub   eax,1
	emit_push_imm(&dst, inst->param[1].value);											// push  param1
	emit_push_r32(&dst, REG_EAX);														// push  eax
	emit_push_imm(&dst, (FPTR)drcbe->map);												// push  drcbe->map
	emit_call(&dst, (x86code *)drcmap_get_value);										// call  drcmap_get_value
	emit_add_r32_imm(&dst, REG_ESP, 12);												// add   esp,12
	emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);										// mov   dstp,eax

	return dst;
}



/***************************************************************************
    INTERNAL REGISTER OPCODES
***************************************************************************/

/*-------------------------------------------------
    op_setfmod - process a SETFMOD opcode
-------------------------------------------------*/

static x86code *op_setfmod(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &srcp, PTYPE_MRI);

	/* immediate case */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		srcp.value &= 3;
		emit_mov_m8_imm(&dst, MABS(&drcbe->state.fmod), srcp.value);					// mov   [fmod],srcp
		emit_fldcw_m16(&dst, MABS(&fp_control[srcp.value]));							// fldcw fp_control[srcp]
	}

	/* register/memory case */
	else
	{
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &srcp);									// mov   eax,srcp
		emit_and_r32_imm(&dst, REG_EAX, 3);												// and   eax,3
		emit_mov_m8_r8(&dst, MABS(&drcbe->state.fmod), REG_AL);							// mov   [fmod],al
		emit_fldcw_m16(&dst, MISD(REG_EAX, 2, &fp_control[0]));							// fldcw fp_control[eax]
	}

	return dst;
}


/*-------------------------------------------------
    op_getfmod - process a GETFMOD opcode
-------------------------------------------------*/

static x86code *op_getfmod(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_MR);

	/* fetch the current mode and store to the destination */
	if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		emit_movzx_r32_m8(&dst, dstp.value, MABS(&drcbe->state.fmod));					// movzx reg,[fmod]
	else
	{
		emit_movzx_r32_m8(&dst, REG_EAX, MABS(&drcbe->state.fmod));						// movzx eax,[fmod]
		emit_mov_m32_r32(&dst, MABS(dstp.value), REG_EAX);								// mov   [dstp],eax
	}

	return dst;
}


/*-------------------------------------------------
    op_getexp - process a GETEXP opcode
-------------------------------------------------*/

static x86code *op_getexp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_MR);

	/* fetch the exception parameter and store to the destination */
	if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		emit_mov_r32_m32(&dst, dstp.value, MABS(&drcbe->state.exp));					// mov   reg,[exp]
	else
	{
		emit_mov_r32_m32(&dst, REG_EAX, MABS(&drcbe->state.exp));						// mov   eax,[exp]
		emit_mov_m32_r32(&dst, MABS(dstp.value), REG_EAX);								// mov   [dstp],eax
	}

	return dst;
}


/*-------------------------------------------------
    op_save - process a SAVE opcode
-------------------------------------------------*/

static x86code *op_save(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;
	int regnum;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_M);

	/* copy live state to the destination */
	emit_mov_r32_imm(&dst, REG_ECX, dstp.value);										// mov    ecx,dstp

	/* copy flags */
	emit_pushf(&dst);																	// pushf
	emit_pop_r32(&dst, REG_EAX);														// pop    eax
	emit_and_r32_imm(&dst, REG_EAX, 0x8c5);												// and    eax,0x8c5
	emit_mov_r8_m8(&dst, REG_AL, MBD(REG_EAX, flags_map));								// mov    al,[flags_map]
	emit_mov_m8_r8(&dst, MBD(REG_ECX, offsetof(drcuml_machine_state, flags)), REG_AL);	// mov    state->flags,al

	/* copy fmod and exp */
	emit_mov_r8_m8(&dst, REG_AL, MABS(&drcbe->state.fmod));								// mov    al,[fmod]
	emit_mov_m8_r8(&dst, MBD(REG_ECX, offsetof(drcuml_machine_state, fmod)), REG_AL);	// mov    state->fmod,al
	emit_mov_r32_m32(&dst, REG_EAX, MABS(&drcbe->state.exp));							// mov    eax,[exp]
	emit_mov_m32_r32(&dst, MBD(REG_ECX, offsetof(drcuml_machine_state, exp)), REG_EAX);	// mov    state->exp,eax

	/* copy integer registers */
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
			emit_mov_m32_r32(&dst, MBD(REG_ECX, offsetof(drcuml_machine_state, r[regnum].w.l)), int_register_map[regnum]);
		else
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS(&drcbe->state.r[regnum].w.l));
			emit_mov_m32_r32(&dst, MBD(REG_ECX, offsetof(drcuml_machine_state, r[regnum].w.l)), REG_EAX);
		}
		emit_mov_r32_m32(&dst, REG_EAX, MABS(&drcbe->state.r[regnum].w.h));
		emit_mov_m32_r32(&dst, MBD(REG_ECX, offsetof(drcuml_machine_state, r[regnum].w.h)), REG_EAX);
	}

	/* copy FP registers */
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.f); regnum++)
	{
		emit_mov_r32_m32(&dst, REG_EAX, MABS(&drcbe->state.f[regnum].s.l));
		emit_mov_m32_r32(&dst, MBD(REG_ECX, offsetof(drcuml_machine_state, f[regnum].s.l)), REG_EAX);
		emit_mov_r32_m32(&dst, REG_EAX, MABS(&drcbe->state.f[regnum].s.h));
		emit_mov_m32_r32(&dst, MBD(REG_ECX, offsetof(drcuml_machine_state, f[regnum].s.h)), REG_EAX);
	}

	return dst;
}


/*-------------------------------------------------
    op_restore - process a RESTORE opcode
-------------------------------------------------*/

static x86code *op_restore(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;
	int regnum;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_M);

	/* copy live state from the destination */
	emit_mov_r32_imm(&dst, REG_ECX, dstp.value);										// mov    ecx,dstp

	/* copy integer registers */
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
			emit_mov_r32_m32(&dst, int_register_map[regnum], MBD(REG_ECX, offsetof(drcuml_machine_state, r[regnum].w.l)));
		else
		{
			emit_mov_r32_m32(&dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, r[regnum].w.l)));
			emit_mov_m32_r32(&dst, MABS(&drcbe->state.r[regnum].w.l), REG_EAX);
		}
		emit_mov_r32_m32(&dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, r[regnum].w.h)));
		emit_mov_m32_r32(&dst, MABS(&drcbe->state.r[regnum].w.h), REG_EAX);
	}

	/* copy FP registers */
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.f); regnum++)
	{
		emit_mov_r32_m32(&dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, f[regnum].s.l)));
		emit_mov_m32_r32(&dst, MABS(&drcbe->state.f[regnum].s.l), REG_EAX);
		emit_mov_r32_m32(&dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, f[regnum].s.h)));
		emit_mov_m32_r32(&dst, MABS(&drcbe->state.f[regnum].s.h), REG_EAX);
	}

	/* copy fmod and exp */
	emit_movzx_r32_m8(&dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, fmod)));// movzx eax,state->fmod
	emit_and_r32_imm(&dst, REG_EAX, 3);													// and    eax,3
	emit_mov_m8_r8(&dst, MABS(&drcbe->state.fmod), REG_AL);								// mov    [fmod],al
	emit_fldcw_m16(&dst, MISD(REG_EAX, 2, &fp_control[0]));								// fldcw  fp_control[eax]
	emit_mov_r32_m32(&dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, exp)));	// mov    eax,state->exp
	emit_mov_m32_r32(&dst, MABS(&drcbe->state.exp), REG_EAX);							// mov    [exp],eax

	/* copy flags */
	emit_movzx_r32_m8(&dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, flags)));// movzx eax,state->flags
	emit_push_m32(&dst, MISD(REG_EAX, 4, flags_unmap));									// push   flags_unmap[eax*4]
	emit_popf(&dst);																	// popf

	return dst;
}



/***************************************************************************
    INTEGER OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    op_load1u - process a LOAD1U opcode
-------------------------------------------------*/

static x86code *op_load1u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
		emit_movzx_r32_m8(&dst, dstreg, MABS(basep.value + 1*indp.value));				// movzx dstreg,[basep + 1*indp]

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		emit_movzx_r32_m8(&dst, dstreg, MISD(indreg, 1, basep.value));					// movzx dstreg,[basep + 1*indp]
	}

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov   dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* general case */
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   [reghi],0
	}
	return dst;
}


/*-------------------------------------------------
    op_load1s - process a LOAD1S opcode
-------------------------------------------------*/

static x86code *op_load1s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = (inst->size == 8) ? REG_EAX : param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
		emit_movsx_r32_m8(&dst, dstreg, MABS(basep.value + 1*indp.value));				// movsx eax,[basep + 1*indp]

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		emit_movsx_r32_m8(&dst, dstreg, MISD(indreg, 1, basep.value));					// movsx eax,[basep + 1*indp]
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else
	{
		/* general case */
		emit_cdq(&dst);																	// cdq
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_load2u - process a LOAD2U opcode
-------------------------------------------------*/

static x86code *op_load2u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
		emit_movzx_r32_m16(&dst, dstreg, MABS(basep.value + 2*indp.value));				// movzx dstreg,[basep + 2*indp]

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		emit_movzx_r32_m16(&dst, dstreg, MISD(indreg, 2, basep.value));					// movzx dstreg,[basep + 2*indp]
	}

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov   dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* general case */
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   [reghi],0
	}
	return dst;
}


/*-------------------------------------------------
    op_load2s - process a LOAD2S opcode
-------------------------------------------------*/

static x86code *op_load2s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = (inst->size == 8) ? REG_EAX : param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
		emit_movsx_r32_m16(&dst, dstreg, MABS(basep.value + 2*indp.value));				// movsx eax,[basep + 2*indp]

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		emit_movsx_r32_m16(&dst, dstreg, MISD(indreg, 2, basep.value));					// movsx eax,[basep + 2*indp]
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else
	{
		/* general case */
		emit_cdq(&dst);																	// cdq
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_load4u - process a LOAD4U opcode
-------------------------------------------------*/

static x86code *op_load4u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
		emit_mov_r32_m32(&dst, dstreg, MABS(basep.value + 4*indp.value));				// mov   dstreg,[basep + 4*indp]

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		emit_mov_r32_m32(&dst, dstreg, MISD(indreg, 4, basep.value));					// mov   dstreg,[basep + 4*indp]
	}

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov   dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* general case */
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   [reghi],0
	}
	return dst;
}


/*-------------------------------------------------
    op_load4s - process a LOAD4S opcode
-------------------------------------------------*/

static x86code *op_load4s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
		emit_mov_r32_m32(&dst, REG_EAX, MABS(basep.value + 4*indp.value));				// mov   eax,[basep + 4*indp]

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		emit_mov_r32_m32(&dst, REG_EAX, MISD(indreg, 4, basep.value));					// movsx eax,[basep + 4*indp]
	}

	/* general case */
	emit_cdq(&dst);																		// cdq
	emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);								// mov   dstp,edx:eax

	return dst;
}


/*-------------------------------------------------
    op_load8u - process a LOAD8U opcode
-------------------------------------------------*/

static x86code *op_load8u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_mov_r32_m32(&dst, REG_EDX, MABS(basep.value + 8*indp.value + 4));			// mov   edx,[basep + 8*indp + 4]
		emit_mov_r32_m32(&dst, dstreg, MABS(basep.value + 8*indp.value));				// mov   dstreg,[basep + 8*indp]
	}

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		emit_mov_r32_m32(&dst, REG_EDX, MISD(indreg, 8, basep.value + 4));				// mov   edx,[basep + 8*indp + 4]
		emit_mov_r32_m32(&dst, dstreg, MISD(indreg, 8, basep.value));					// mov   dstreg,[basep + 8*indp]
	}

	/* general case */
	emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);								// mov   dstp,edx:dstreg

	return dst;
}


/*-------------------------------------------------
    op_store1 - process a STORE1 opcode
-------------------------------------------------*/

static x86code *op_store1(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, basep, indp;
	int srcreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &basep, PTYPE_M, &indp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* pick a source register for the general case */
	srcreg = param_select_register(REG_EAX, &srcp, NULL);
	if (srcreg & 4)
		srcreg = REG_EAX;

	/* degenerate case: constant index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_mov_m8_imm(&dst, MABS(basep.value + 1*indp.value), srcp.value);		// mov   [basep + 1*indp],srcp
		else
		{
			emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);								// mov   srcreg,srcp
			emit_mov_m8_r8(&dst, MABS(basep.value + 1*indp.value), srcreg);				// mov   [basep + 1*indp],srcreg
		}
	}

	/* normal case: variable index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);									// mov   indreg,indp
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_mov_m8_imm(&dst, MISD(indreg, 1, basep.value), srcp.value);			// mov   [basep + 1*ecx],srcp
		else
		{
			emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);								// mov   srcreg,srcp
			emit_mov_m8_r8(&dst, MISD(indreg, 1, basep.value), srcreg);					// mov   [basep + 1*ecx],srcreg
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_store2 - process a STORE2 opcode
-------------------------------------------------*/

static x86code *op_store2(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, basep, indp;
	int srcreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &basep, PTYPE_M, &indp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* pick a source register for the general case */
	srcreg = param_select_register(REG_EAX, &srcp, NULL);

	/* degenerate case: constant index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_mov_m16_imm(&dst, MABS(basep.value + 2*indp.value), srcp.value);		// mov   [basep + 2*indp],srcp
		else
		{
			emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);								// mov   srcreg,srcp
			emit_mov_m16_r16(&dst, MABS(basep.value + 2*indp.value), srcreg);			// mov   [basep + 2*indp],srcreg
		}
	}

	/* normal case: variable index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);									// mov   indreg,indp
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_mov_m16_imm(&dst, MISD(indreg, 2, basep.value), srcp.value);			// mov   [basep + 2*ecx],srcp
		else
		{
			emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);								// mov   srcreg,srcp
			emit_mov_m16_r16(&dst, MISD(indreg, 2, basep.value), srcreg);				// mov   [basep + 2*ecx],srcreg
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_store4 - process a STORE4 opcode
-------------------------------------------------*/

static x86code *op_store4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, basep, indp;
	int srcreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &basep, PTYPE_M, &indp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* pick a source register for the general case */
	srcreg = param_select_register(REG_EAX, &srcp, NULL);

	/* degenerate case: constant index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_mov_m32_imm(&dst, MABS(basep.value + 4*indp.value), srcp.value);		// mov   [basep + 4*indp],srcp
		else
		{
			emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);								// mov   srcreg,srcp
			emit_mov_m32_r32(&dst, MABS(basep.value + indp.value), srcreg);				// mov   [basep + 4*indp],srcreg
		}
	}

	/* normal case: variable index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);									// mov   indreg,indp
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_mov_m32_imm(&dst, MISD(indreg, 4, basep.value), srcp.value);			// mov   [basep + 4*ecx],srcp
		else
		{
			emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);								// mov   srcreg,srcp
			emit_mov_m32_r32(&dst, MISD(indreg, 4, basep.value), srcreg);				// mov   [basep + 4*ecx],srcreg
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_store8 - process a STORE8 opcode
-------------------------------------------------*/

static x86code *op_store8(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, basep, indp;
	int srcreg;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &basep, PTYPE_M, &indp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* pick a source register for the general case */
	srcreg = param_select_register(REG_EAX, &srcp, NULL);

	/* degenerate case: constant index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			emit_mov_m32_imm(&dst, MABS(basep.value + 8*indp.value), srcp.value);		// mov   [basep + 8*indp],srcp
			emit_mov_m32_imm(&dst, MABS(basep.value + 4 + 8*indp.value), srcp.value >> 32);
																						// mov   [basep + 4 + 8*indp],srcp >> 32
		}
		else
		{
			emit_mov_r64_p64(drcbe, &dst, srcreg, REG_EDX, &srcp);						// mov   edx:srcreg,srcp
			emit_mov_m32_r32(&dst, MABS(basep.value + 8*indp.value), srcreg);			// mov   [basep + 8*indp],srcreg
			emit_mov_m32_r32(&dst, MABS(basep.value + 4 + 8*indp.value), REG_EDX);		// mov   [basep + 8*indp + 4],edx
		}
	}

	/* normal case: variable index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);									// mov   indreg,indp
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			emit_mov_m32_imm(&dst, MISD(indreg, 8, basep.value), srcp.value);			// mov   [basep + 8*ecx],srcp
			emit_mov_m32_imm(&dst, MISD(indreg, 8, basep.value + 4), srcp.value >> 32);	// mov   [basep + 8*ecx + 4],srcp >> 32
		}
		else
		{
			emit_mov_r64_p64(drcbe, &dst, srcreg, REG_EDX, &srcp);						// mov   edx:srcreg,srcp
			emit_mov_m32_r32(&dst, MISD(indreg, 8, basep.value), srcreg);				// mov   [basep + 8*ecx],srcreg
			emit_mov_m32_r32(&dst, MISD(indreg, 8, basep.value + 4), REG_EDX);			// mov   [basep + 4 + 8*ecx],edx
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_read1u - process a READ1U opcode
-------------------------------------------------*/

static x86code *op_read1u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* set up a call to the read byte handler */
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_byte);// call   read_byte
	emit_add_r32_imm(&dst, REG_ESP, 4);													// add    esp,4
	emit_movzx_r32_r8(&dst, dstreg, REG_AL);											// movzx  dstreg,al

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov   dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* general case */
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   [reghi],0
	}
	return dst;
}


/*-------------------------------------------------
    op_read1s - process a READ1S opcode
-------------------------------------------------*/

static x86code *op_read1s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* set up a call to the read byte handler */
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_byte);// call   read_byte
	emit_add_r32_imm(&dst, REG_ESP, 4);													// add    esp,4
	emit_movsx_r32_r8(&dst, dstreg, REG_AL);											// movsx  dstreg,al

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else
	{
		/* general case */
		emit_cdq(&dst);																	// cdq
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_read2u - process a READ2U opcode
-------------------------------------------------*/

static x86code *op_read2u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* set up a call to the read word handler */
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_word);// call   read_word
	emit_add_r32_imm(&dst, REG_ESP, 4);													// add    esp,4
	emit_movzx_r32_r16(&dst, dstreg, REG_AX);											// movzx  dstreg,ax

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov   dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* general case */
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   [reghi],0
	}
	return dst;
}


/*-------------------------------------------------
    op_read2s - process a READ2S opcode
-------------------------------------------------*/

static x86code *op_read2s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* set up a call to the read word handler */
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_word);// call   read_word
	emit_add_r32_imm(&dst, REG_ESP, 4);													// add    esp,4
	emit_movsx_r32_r16(&dst, dstreg, REG_AX);											// movsx  dstreg,ax

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else
	{
		/* general case */
		emit_cdq(&dst);																	// cdq
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_read2m - process a READ2M opcode
-------------------------------------------------*/

static x86code *op_read2m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp, maskp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &maskp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* set up a call to the read dword masked handler */
	emit_push_p32(drcbe, &dst, &maskp);													// push   maskp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_word_masked);
																						// call   read_word_masked
	emit_add_r32_imm(&dst, REG_ESP, 8);													// add    esp,8
	emit_movzx_r32_r16(&dst, dstreg, REG_AX);											// movzx  dstreg,ax

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov   dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* general case */
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   [reghi],0
	}
	return dst;
}


/*-------------------------------------------------
    op_read4u - process a READ4U opcode
-------------------------------------------------*/

static x86code *op_read4u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI);

	/* set up a call to the read dword handler */
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_dword);// call   read_dword
	emit_add_r32_imm(&dst, REG_ESP, 4);													// add    esp,4

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);										// mov   dstp,eax

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* general case */
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   [reghi],0
	}
	return dst;
}


/*-------------------------------------------------
    op_read4s - process a READ4S opcode
-------------------------------------------------*/

static x86code *op_read4s(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI);

	/* set up a call to the read dword handler */
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_dword);// call   read_dword
	emit_add_r32_imm(&dst, REG_ESP, 4);													// add    esp,4

	/* 64-bit form */
	emit_cdq(&dst);																		// cdq
	emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);								// mov   dst,edx:eax

	return dst;
}


/*-------------------------------------------------
    op_read4m - process a READ4M opcode
-------------------------------------------------*/

static x86code *op_read4m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp, maskp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &maskp, PTYPE_MRI);

	/* set up a call to the read dword masked handler */
	emit_push_p32(drcbe, &dst, &maskp);													// push   maskp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_dword_masked);
																						// call   read_dword_masked
	emit_add_r32_imm(&dst, REG_ESP, 8);													// add    esp,8

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);										// mov   dstp,eax

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* general case */
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   [reghi],0
	}
	return dst;
}


/*-------------------------------------------------
    op_read8u - process a READ8U opcode
-------------------------------------------------*/

static x86code *op_read8u(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI);

	/* set up a call to the read qword handler */
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_qword);// call   read_qword
	emit_add_r32_imm(&dst, REG_ESP, 4);													// add    esp,4

	/* 64-bit form */
	emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);								// mov   dst,edx:eax

	return dst;
}


/*-------------------------------------------------
    op_read8m - process a READ8M opcode
-------------------------------------------------*/

static x86code *op_read8m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp, maskp;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &maskp, PTYPE_MRI);

	/* set up a call to the read qword masked handler */
	emit_push_p64(drcbe, &dst, &maskp);													// push   maskp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_qword_masked);
																						// call   read_qword_masked
	emit_add_r32_imm(&dst, REG_ESP, 12);												// add    esp,12

	/* 64-bit form */
	emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);								// mov   dst,edx:eax

	return dst;
}


/*-------------------------------------------------
    op_write1 - process a WRITE1 opcode
-------------------------------------------------*/

static x86code *op_write1(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter spacep, addrp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* set up a call to the write byte handler */
	emit_push_p32(drcbe, &dst, &srcp);													// push   srcp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->write_byte);// call   write_byte
	emit_add_r32_imm(&dst, REG_ESP, 8);													// add    esp,8

	return dst;
}


/*-------------------------------------------------
    op_write2 - process a WRITE2 opcode
-------------------------------------------------*/

static x86code *op_write2(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter spacep, addrp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* set up a call to the write word handler */
	emit_push_p32(drcbe, &dst, &srcp);													// push   srcp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->write_word);// call   write_word
	emit_add_r32_imm(&dst, REG_ESP, 8);													// add    esp,8

	return dst;
}


/*-------------------------------------------------
    op_writ2m - process a WRIT2M opcode
-------------------------------------------------*/

static x86code *op_writ2m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter spacep, addrp, maskp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &maskp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* set up a call to the write word handler */
	emit_push_p32(drcbe, &dst, &maskp);													// push   maskp
	emit_push_p32(drcbe, &dst, &srcp);													// push   srcp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->write_word_masked);
																						// call   write_word_masked
	emit_add_r32_imm(&dst, REG_ESP, 12);												// add    esp,12
	return dst;
}


/*-------------------------------------------------
    op_write4 - process a WRITE4 opcode
-------------------------------------------------*/

static x86code *op_write4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter spacep, addrp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* set up a call to the write dword handler */
	emit_push_p32(drcbe, &dst, &srcp);													// push   srcp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->write_dword);// call   write_dword
	emit_add_r32_imm(&dst, REG_ESP, 8);													// add    esp,8

	return dst;
}


/*-------------------------------------------------
    op_writ4m - process a WRIT4M opcode
-------------------------------------------------*/

static x86code *op_writ4m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter spacep, addrp, maskp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &maskp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* set up a call to the write word handler */
	emit_push_p32(drcbe, &dst, &maskp);													// push   maskp
	emit_push_p32(drcbe, &dst, &srcp);													// push   srcp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->write_dword_masked);
																						// call   write_dword_masked
	emit_add_r32_imm(&dst, REG_ESP, 12);												// add    esp,12
	return dst;
}


/*-------------------------------------------------
    op_write8 - process a WRITE8 opcode
-------------------------------------------------*/

static x86code *op_write8(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter spacep, addrp, srcp;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* set up a call to the write qword handler */
	emit_push_p64(drcbe, &dst, &srcp);													// push   srcp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->write_qword);// call   write_qword

	emit_add_r32_imm(&dst, REG_ESP, 12);												// add    esp,12
	return dst;
}


/*-------------------------------------------------
    op_writ8m - process a WRIT8M opcode
-------------------------------------------------*/

static x86code *op_writ8m(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter spacep, addrp, maskp, srcp;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &maskp, PTYPE_MRI, &srcp, PTYPE_MRI);

	/* set up a call to the write word handler */
	emit_push_p64(drcbe, &dst, &maskp);													// push   maskp
	emit_push_p64(drcbe, &dst, &srcp);													// push   srcp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->write_qword_masked);
																						// call   write_qword_masked
	emit_add_r32_imm(&dst, REG_ESP, 20);												// add    esp,20
	return dst;
}


/*-------------------------------------------------
    op_flags - process a FLAGS opcode
-------------------------------------------------*/

static x86code *op_flags(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, maskp, tablep;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &maskp, PTYPE_I, &tablep, PTYPE_M);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_ECX, &dstp, NULL);

	/* translate live flags into UML flags */
	emit_pushf(&dst);																	// pushf
	emit_pop_r32(&dst, REG_EAX);														// pop    eax
	emit_and_r32_imm(&dst, REG_EAX, 0x8c5);												// and    eax,0x8c5
	emit_movzx_r32_m8(&dst, REG_EAX, MBD(REG_EAX, flags_map));							// movzx  eax,[flags_map]

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* no masking */
		if (maskp.value == 0xffffffff)
		{
			emit_mov_r32_m32(&dst, dstreg, MISD(REG_EAX, 4, tablep.value));				// mov    dstreg,[eax*4+table]
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov    dstp,dstreg
		}

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &dstp);								// mov    dstreg,dstp
			emit_mov_r32_m32(&dst, REG_EDX, MISD(REG_EAX, 4, tablep.value));			// mov    edx,[eax*4+table]
			emit_and_r32_imm(&dst, dstreg, ~maskp.value);								// and    dstreg,~mask
			emit_and_r32_imm(&dst, REG_EDX, maskp.value);								// and    edx,mask
			emit_or_r32_r32(&dst, dstreg, REG_EDX);										// or     dstreg,edx
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov    dstp,dstreg
		}
	}

	/* 64-bit form */
	else
	{
		/* no masking */
		if (maskp.value == U64(0xffffffffffffffff))
		{
			emit_mov_r32_m32(&dst, dstreg, MISD(REG_EAX, 8, tablep.value));				// mov    dstreg,[eax*4+table]
			emit_mov_r32_m32(&dst, REG_EDX, MISD(REG_EAX, 8, tablep.value + 4));		// mov    edx,[eax*4+table+4]
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);						// mov    dst,edx:dstreg
		}

		/* general case */
		else
		{
			if ((UINT32)maskp.value != 0)
			{
				emit_mov_r32_p32(drcbe, &dst, dstreg, &dstp);							// mov    dstreg,dstp
				emit_mov_r32_m32(&dst, REG_EDX, MISD(REG_EAX, 8, tablep.value));		// mov    edx,[eax*8+table]
				emit_and_r32_imm(&dst, dstreg, ~maskp.value);							// and    dstreg,~mask
				emit_and_r32_imm(&dst, REG_EDX, maskp.value);							// and    edx,mask
				emit_or_r32_r32(&dst, dstreg, REG_EDX);									// or     dstreg,edx
				emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);							// mov    dstp,dstreg.lo
			}
			if ((UINT32)(maskp.value >> 32) != 0)
			{
				if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
					emit_mov_r32_m32(&dst, REG_ECX, MABS(drcbe->reghi[dstp.value]));	// mov    ecx,dstp.hi
				else
					emit_mov_r32_m32(&dst, REG_ECX, MABS(dstp.value + 4));				// mov    ecx,dstp.hi
				emit_mov_r32_m32(&dst, REG_EDX, MISD(REG_EDX, 8, tablep.value+4));		// mov    edx,[eax*8+table+4]
				emit_and_r32_imm(&dst, REG_ECX, ~maskp.value);							// and    ecx,~mask
				emit_and_r32_imm(&dst, REG_EDX, maskp.value);							// and    edx,mask
				emit_or_r32_r32(&dst, REG_ECX, REG_EDX);								// or     ecx,edx
				if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
					emit_mov_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_ECX);	// mov    dstp.hi,ecx
				else
					emit_mov_m32_r32(&dst, MABS(dstp.value + 4), REG_ECX);				// mov    dstp.hi,ecx
			}
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_setc - process a SETC opcode
-------------------------------------------------*/

static x86code *op_setc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, bitp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &srcp, PTYPE_MRI, &bitp, PTYPE_MRI);

	/* degenerate case: source is immediate */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE && bitp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (srcp.value & ((UINT64)1 << bitp.value))
			emit_stc(&dst);
		else
			emit_clc(&dst);
		return dst;
	}

	/* load non-immediate bit numbers into a register */
	if (bitp.type != DRCUML_PTYPE_IMMEDIATE)
	{
		emit_mov_r32_p32(drcbe, &dst, REG_ECX, &bitp);
		emit_and_r32_imm(&dst, REG_ECX, inst->size * 8 - 1);
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		if (bitp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_bt_m32_imm(&dst, MABS(srcp.value), bitp.value);					// bt     [srcp],bitp
			else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_bt_r32_imm(&dst, srcp.value, bitp.value);							// bt     srcp,bitp
		}
		else
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_bt_m32_r32(&dst, MABS(srcp.value), REG_ECX);						// bt     [srcp],ecx
			else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_bt_r32_r32(&dst, srcp.value, REG_ECX);								// bt     [srcp],ecx
		}
	}

	/* 64-bit form */
	else
	{
		if (bitp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_bt_m32_imm(&dst, MABS(srcp.value), bitp.value);					// bt     [srcp],bitp
			else if (srcp.type == DRCUML_PTYPE_INT_REGISTER && bitp.value < 32)
				emit_bt_r32_imm(&dst, srcp.value, bitp.value);							// bt     srcp,bitp
			else if (srcp.type == DRCUML_PTYPE_INT_REGISTER && bitp.value >= 32)
				emit_bt_m32_imm(&dst, MABS(drcbe->reghi[srcp.value]), bitp.value - 32);	// bt     [srcp.hi],bitp
		}
		else
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_bt_m32_r32(&dst, MABS(srcp.value), REG_ECX);						// bt     [srcp],ecx
			else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
			{
				emit_mov_m32_r32(&dst, MABS(drcbe->reglo[srcp.value]), srcp.value);		// mov    [srcp.lo],srcp
				emit_bt_m32_r32(&dst, MABS(drcbe->reglo[srcp.value]), REG_ECX);			// bt     [srcp],ecx
			}
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_mov - process a MOV opcode
-------------------------------------------------*/

static x86code *op_mov(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	x86code *savedst = dst;
	emit_link skip = { 0 };
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS || (inst->condflags >= DRCUML_COND_Z && inst->condflags < DRCUML_COND_MAX));

	/* normalize parameters, but only if we got here directly */
	/* other opcodes call through here with pre-normalized parameters */
	if (inst->opcode == DRCUML_OP_MOV)
		param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);
	else
	{
		dstp = inst->param[0];
		srcp = inst->param[1];
	}

	/* degenerate case: dest and source are equal */
	if (dstp.type == srcp.type && dstp.value == srcp.value)
		return dst;

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* always start with a jmp */
	if (inst->condflags != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condflags), &skip);			// jcc   skip

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* register to memory */
		if (dstp.type == DRCUML_PTYPE_MEMORY && srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_r32(&dst, MABS(dstp.value), srcp.value);						// mov   [dstp],srcp

		/* immediate to memory */
		else if (dstp.type == DRCUML_PTYPE_MEMORY && srcp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_mov_m32_imm(&dst, MABS(dstp.value), srcp.value);						// mov   [dstp],srcp

		/* conditional memory to register */
		else if (inst->condflags != 0 && dstp.type == DRCUML_PTYPE_INT_REGISTER && srcp.type == DRCUML_PTYPE_MEMORY)
		{
			dst = savedst;
			skip.target = NULL;
			emit_cmovcc_r32_m32(&dst, X86_CONDITION(inst->condflags), dstp.value, MABS(srcp.value));
																						// cmovcc dstp,[srcp]
		}

		/* conditional register to register */
		else if (inst->condflags != 0 && dstp.type == DRCUML_PTYPE_INT_REGISTER && srcp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			dst = savedst;
			skip.target = NULL;
			emit_cmovcc_r32_r32(&dst, X86_CONDITION(inst->condflags), dstp.value, srcp.value);
																						// cmovcc dstp,srcp
		}

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &srcp);								// mov   dstreg,srcp
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* register to memory */
		if (dstp.type == DRCUML_PTYPE_MEMORY && srcp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS(drcbe->reghi[srcp.value]));			// mov   eax,reghi[srcp]
			emit_mov_m32_r32(&dst, MABS(dstp.value), srcp.value);						// mov   [dstp],srcp
			emit_mov_m32_r32(&dst, MABS(dstp.value + 4), REG_EAX);						// mov   [dstp+4],eax
		}

		/* immediate to memory */
		else if (dstp.type == DRCUML_PTYPE_MEMORY && srcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			emit_mov_m32_imm(&dst, MABS(dstp.value), srcp.value);						// mov   [dstp],srcp
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), srcp.value >> 32);				// mov   [dstp+4],srcp >> 32
		}

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &srcp);						// mov   edx:dstreg,srcp
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);						// mov   dstp,edx:dstreg
		}
	}

	/* resolve the jump */
	if (skip.target != NULL)
		resolve_link(&dst, &skip);
	return dst;
}


/*-------------------------------------------------
    op_zext1 - process a ZEXT1 opcode
-------------------------------------------------*/

static x86code *op_zext1(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT8)srcp.value);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* convert 8-bit source registers */
	if (srcp.type == DRCUML_PTYPE_INT_REGISTER && (srcp.value & 4))
	{
		emit_mov_r32_r32(&dst, REG_EAX, srcp.value);									// mov   eax,srcp
		srcp.value = REG_EAX;
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_movzx_r32_m8(&dst, dstreg, MABS(srcp.value));							// movzx dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_movzx_r32_r8(&dst, dstreg, srcp.value);								// movzx dstreg,srcp
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_movzx_r32_m8(&dst, dstreg, MABS(srcp.value));							// movzx dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_movzx_r32_r8(&dst, dstreg, srcp.value);								// movzx dstreg,srcp
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   reghi[dstp],0
	}
	return dst;
}


/*-------------------------------------------------
    op_zext2 - process a ZEXT2 opcode
-------------------------------------------------*/

static x86code *op_zext2(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT16)srcp.value);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_movzx_r32_m16(&dst, dstreg, MABS(srcp.value));							// movzx dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_movzx_r32_r16(&dst, dstreg, srcp.value);								// movzx dstreg,srcp
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_movzx_r32_m16(&dst, dstreg, MABS(srcp.value));							// movzx dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_movzx_r32_r16(&dst, dstreg, srcp.value);								// movzx dstreg,srcp
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   reghi[dstp],0
	}
	return dst;
}


/*-------------------------------------------------
    op_zext4 - process a ZEXT4 opcode
-------------------------------------------------*/

static x86code *op_zext4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT32)srcp.value);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* 64-bit form */
	if (inst->size == 8)
	{
		/* general case */
		emit_mov_r32_p32(drcbe, &dst, dstreg, &srcp);									// mov   dstreg,srcp
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);							// mov   [dstp+4],0
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);					// mov   reghi[dstp],0
	}
	return dst;
}


/*-------------------------------------------------
    op_sext1 - process a SEXT1 opcode
-------------------------------------------------*/

static x86code *op_sext1(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (INT8)srcp.value);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* convert 8-bit source registers */
	if (srcp.type == DRCUML_PTYPE_INT_REGISTER && (srcp.value & 4))
	{
		emit_mov_r32_r32(&dst, REG_EAX, srcp.value);									// mov   eax,srcp
		srcp.value = REG_EAX;
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_movsx_r32_m8(&dst, dstreg, MABS(srcp.value));							// movsx dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_movsx_r32_r8(&dst, dstreg, srcp.value);								// movsx dstreg,srcp
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_movsx_r32_m8(&dst, REG_EAX, MABS(srcp.value));							// movsx eax,[srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_movsx_r32_r8(&dst, REG_EAX, srcp.value);								// movsx eax,srcp
		emit_cdq(&dst);																	// cdq
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_sext2 - process a SEXT2 opcode
-------------------------------------------------*/

static x86code *op_sext2(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (INT16)srcp.value);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_movsx_r32_m16(&dst, dstreg, MABS(srcp.value));							// movsx dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_movsx_r32_r16(&dst, dstreg, srcp.value);								// movsx dstreg,srcp
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_movsx_r32_m16(&dst, REG_EAX, MABS(srcp.value));						// movsx eax,[srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_movsx_r32_r16(&dst, REG_EAX, srcp.value);								// movsx eax,srcp
		emit_cdq(&dst);																	// cdq
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_sext4 - process a SEXT4 opcode
-------------------------------------------------*/

static x86code *op_sext4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (INT32)srcp.value);

	/* 64-bit form */
	if (inst->size == 8)
	{
		/* general case */
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &srcp);									// mov   eax,srcp
		emit_cdq(&dst);																	// cdq
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_xtract - process an XTRACT opcode
-------------------------------------------------*/

static x86code *op_xtract(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, shiftp, maskp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI, &shiftp, PTYPE_MRI, &maskp, PTYPE_MRI);

	/* degenerate cases -- convert to a move or simple shift */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE && shiftp.type == DRCUML_PTYPE_IMMEDIATE && maskp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->size == 4)
			return convert_to_mov_imm(drcbe, dst, inst, &dstp, ((srcp.value << shiftp.value) | (srcp.value >> (32 - shiftp.value))) & maskp.value);
		else if (inst->size == 8)
			return convert_to_mov_imm(drcbe, dst, inst, &dstp, ((srcp.value << shiftp.value) | (srcp.value >> (64 - shiftp.value))) & maskp.value);
	}
	if (shiftp.type == DRCUML_PTYPE_IMMEDIATE && maskp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if ((inst->size == 4 && maskp.value == (UINT32)(0xffffffffUL << shiftp.value)) || (inst->size == 8 && maskp.value == U64(0xffffffffffffffff) << shiftp.value))
		{
			drcuml_instruction temp = *inst;
			temp.numparams = 3;
			return op_shl(drcbe, dst, &temp);
		}
		if ((inst->size == 4 && maskp.value == (UINT32)(0xffffffffUL >> (32 - shiftp.value))) || (inst->size == 8 && maskp.value == U64(0xffffffffffffffff) >> (64 - shiftp.value)))
		{
			drcuml_instruction temp = *inst;
			temp.numparams = 3;
			temp.param[2].value = inst->size * 8 - temp.param[2].value;
			return op_shr(drcbe, dst, &temp);
		}
	}

	/* pick a target register for the general case */
	dstreg = param_select_register2(REG_EAX, &dstp, &shiftp, &maskp);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_mov_r32_p32(drcbe, &dst, dstreg, &srcp);									// mov   dstreg,srcp
		emit_rol_r32_p32(drcbe, &dst, dstreg, &shiftp, inst);							// rol   dstreg,shiftp
		emit_and_r32_p32(drcbe, &dst, dstreg, &maskp, inst);							// and   dstreg,maskp
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &srcp);							// mov   edx:dstreg,srcp
		emit_rol_r64_p64(drcbe, &dst, dstreg, REG_EDX, &shiftp, inst);					// rol   edx:dstreg,shiftp
		emit_and_r64_p64(drcbe, &dst, dstreg, REG_EDX, &maskp, inst);					// and   edx:dstreg,maskp
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,edx:dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_insert - process an INSERT opcode
-------------------------------------------------*/

static x86code *op_insert(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, shiftp, maskp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI, &shiftp, PTYPE_MRI, &maskp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register2(REG_ECX, &dstp, &shiftp, &maskp);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &srcp);									// mov   eax,srcp
		emit_rol_r32_p32(drcbe, &dst, REG_EAX, &shiftp, inst);							// rol   eax,shiftp
		emit_mov_r32_p32(drcbe, &dst, dstreg, &dstp);									// mov   dstreg,dstp
		if (maskp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			emit_and_r32_imm(&dst, REG_EAX, maskp.value);								// and   eax,maskp
			emit_and_r32_imm(&dst, dstreg, ~maskp.value);								// and   dstreg,~maskp
		}
		else
		{
			emit_mov_r32_p32(drcbe, &dst, REG_EDX, &maskp);								// mov   edx,maskp
			emit_and_r32_r32(&dst, REG_EAX, REG_EDX);									// and   eax,edx
			emit_not_r32(&dst, REG_EDX);												// not   edx
			emit_and_r32_r32(&dst, dstreg, REG_EDX);									// and   dstreg,edx
		}
		emit_or_r32_r32(&dst, dstreg, REG_EAX);											// or    dstreg,eax
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_mov_r64_p64(drcbe, &dst, REG_EAX, REG_EDX, &srcp);							// mov   edx:eax,srcp
		emit_rol_r64_p64(drcbe, &dst, REG_EAX, REG_EDX, &shiftp, inst);					// rol   edx:eax,shiftp
		if (maskp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			emit_and_r32_imm(&dst, REG_EAX, maskp.value);								// and   eax,maskp
			emit_and_r32_imm(&dst, REG_EDX, maskp.value >> 32);							// and   edx,maskp >> 32
			if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			{
				emit_and_r32_imm(&dst, dstp.value, ~maskp.value);						// and   dstp.lo,~maskp
				emit_and_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), ~maskp.value >> 32);// and   dstp.hi,~maskp >> 32
				emit_or_r32_r32(&dst, dstp.value, REG_EAX);								// or    dstp.lo,eax
				emit_or_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_EDX);			// or    dstp.hi,edx
			}
			else
			{
				emit_and_m32_imm(&dst, MABS(dstp.value), ~maskp.value);					// and   dstp.lo,~maskp
				emit_and_m32_imm(&dst, MABS(dstp.value + 4), ~maskp.value >> 32);		// and   dstp.hi,~maskp >> 32
				emit_or_m32_r32(&dst, MABS(dstp.value), REG_EAX);						// or    dstp.lo,eax
				emit_or_m32_r32(&dst, MABS(dstp.value + 4), REG_EDX);					// or    dstp.hi,edx
			}
		}
		else
		{
			emit_mov_m32_r32(&dst, MBD(REG_ESP, -8), REG_EBX);							// mov   [esp-8],ebx
			emit_mov_r64_p64(drcbe, &dst, REG_EBX, REG_ECX, &maskp);					// mov   ecx:ebx,maskp
			emit_and_r32_r32(&dst, REG_EAX, REG_EBX);									// and   eax,ebx
			emit_and_r32_r32(&dst, REG_EDX, REG_ECX);									// and   edx,ecx
			emit_not_r32(&dst, REG_EBX);												// not   ebx
			emit_not_r32(&dst, REG_ECX);												// not   ecx
			if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			{
				emit_and_r32_r32(&dst, dstp.value, REG_EBX);							// and   dstp.lo,ebx
				emit_and_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_ECX);		// and   dstp.hi,ecx
				emit_or_r32_r32(&dst, dstp.value, REG_EAX);								// or    dstp.lo,eax
				emit_or_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_EDX);			// or    dstp.hi,edx
			}
			else
			{
				emit_and_m32_r32(&dst, MABS(dstp.value), REG_EBX);						// and   dstp.lo,ebx
				emit_and_m32_r32(&dst, MABS(dstp.value + 4), REG_ECX);					// and   dstp.hi,ecx
				emit_or_m32_r32(&dst, MABS(dstp.value), REG_EAX);						// or    dstp.lo,eax
				emit_or_m32_r32(&dst, MABS(dstp.value + 4), REG_EDX);					// or    dstp.hi,edx
			}
			emit_mov_r32_m32(&dst, REG_EBX, MBD(REG_ESP, -8));							// mov   ebx,[esp-8]
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_add - process a ADD opcode
-------------------------------------------------*/

static x86code *op_add(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, src1p.value + src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (dstp.type == DRCUML_PTYPE_MEMORY && src1p.type == DRCUML_PTYPE_MEMORY && src1p.value == dstp.value)
			emit_add_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// add   [dstp],src2p

		/* reg = reg + imm */
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER && src1p.type == DRCUML_PTYPE_INT_REGISTER && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
			emit_lea_r32_m32(&dst, dstp.value, MBD(src1p.value, src2p.value));			// lea   dstp,[src1p+src2p]

		/* reg = reg + reg */
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER && src1p.type == DRCUML_PTYPE_INT_REGISTER && src2p.type == DRCUML_PTYPE_INT_REGISTER && inst->condflags == 0)
			emit_lea_r32_m32(&dst, dstp.value, MBISD(src1p.value, src2p.value, 1, 0));	// lea   dstp,[src1p+src2p]

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_add_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// add   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_add_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// add   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);						// mov   dstreg:dstp,[src1p]
			emit_add_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);				// add   dstreg:dstp,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);						// mov   dstp,dstreg:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_addc - process a ADDC opcode
-------------------------------------------------*/

static x86code *op_addc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_adc_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// adc   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_adc_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// adc   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_adc_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// adc   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);						// mov   dstreg:dstp,[src1p]
			emit_adc_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);				// adc   dstreg:dstp,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);						// mov   dstp,dstreg:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_sub - process a SUB opcode
-------------------------------------------------*/

static x86code *op_sub(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, src1p.value - src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (dstp.type == DRCUML_PTYPE_MEMORY && src1p.type == DRCUML_PTYPE_MEMORY && src1p.value == dstp.value)
			emit_sub_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// sub   [dstp],src2p

		/* reg = reg - imm */
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER && src1p.type == DRCUML_PTYPE_INT_REGISTER && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
			emit_lea_r32_m32(&dst, dstp.value, MBD(src1p.value, -src2p.value));			// lea   dstp,[src1p-src2p]

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_sub_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// sub   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_sub_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// sub   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);						// mov   dstreg:dstp,[src1p]
			emit_sub_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);				// sub   dstreg:dstp,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);						// mov   dstp,dstreg:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_subc - process a SUBC opcode
-------------------------------------------------*/

static x86code *op_subc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_sbb_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// sbb   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_sbb_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// sbb   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_sbb_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// sbb   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);						// mov   dstreg:dstp,[src1p]
			emit_sbb_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);				// sbb   dstreg:dstp,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);						// mov   dstp,dstreg:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_cmp - process a CMP opcode
-------------------------------------------------*/

static x86code *op_cmp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter src1p, src2p;
	emit_link skip;
	int src1reg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && src1p.value == src2p.value)
	{
		emit_cmp_r32_r32(&dst, REG_EAX, REG_EAX);										// cmp   eax,eax
		return dst;
	}

	/* pick a target register for the general case */
	src1reg = param_select_register(REG_EAX, &src1p, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* memory versus anything */
		if (src1p.type == DRCUML_PTYPE_MEMORY)
			emit_cmp_m32_p32(drcbe, &dst, MABS(src1p.value), &src2p, inst);				// cmp   [dstp],src2p

		/* general case */
		else
		{
			if (src1p.type == DRCUML_PTYPE_IMMEDIATE)
				emit_mov_r32_imm(&dst, src1reg, src1p.value);							// mov   src1reg,imm
			emit_cmp_r32_p32(drcbe, &dst, src1reg, &src2p, inst);						// cmp   src1reg,src2p
		}
	}

	/* 64-bit form */
	else
	{
		/* memory versus anything */
		if (src1p.type == DRCUML_PTYPE_MEMORY)
		{
			emit_cmp_m32_p32hi(drcbe, &dst, MABS(src1p.value + 4), &src2p, inst);		// cmp   [dstp],src2p.hi
			emit_jcc_short_link(&dst, COND_NE, &skip);									// jne   skip
			emit_cmp_m32_p32(drcbe, &dst, MABS(src1p.value), &src2p, inst);				// cmp   [dstp],src2p
			resolve_link(&dst, &skip);												// skip:
		}

		/* general case */
		else
		{
			if (src1p.type == DRCUML_PTYPE_IMMEDIATE)
				emit_mov_r32_imm(&dst, REG_EAX, src1p.value >> 32);						// mov   eax,imm >> 32
			else if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_r32_m32(&dst, REG_EAX, MABS(drcbe->reghi[src1p.value]));		// mov   eax,reghi[src1p]
			emit_cmp_r32_p32hi(drcbe, &dst, REG_EAX, &src2p, inst);						// cmp   eax,src2p.hi
			emit_jcc_short_link(&dst, COND_NE, &skip);									// jne   skip
			if (src1p.type == DRCUML_PTYPE_IMMEDIATE)
				emit_mov_r32_imm(&dst, src1reg, src1p.value);							// mov   src1reg,imm
			emit_cmp_r32_p32(drcbe, &dst, src1reg, &src2p, inst);						// cmp   src1reg,src2p
			resolve_link(&dst, &skip);												// skip:
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_mulu - process a MULU opcode
-------------------------------------------------*/

static x86code *op_mulu(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, edstp, src1p, src2p;
	int compute_hi;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_4_commutative(drcbe, inst, &dstp, PTYPE_MR, &edstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);
	compute_hi = (dstp.type != edstp.type || dstp.value != edstp.value);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
	{
		if (inst->size == 4)
		{
			if (!compute_hi)
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT32)src1p.value * (UINT32)src2p.value);
			else
			{
				UINT64 result = (UINT64)(UINT32)src1p.value * (UINT64)(UINT32)src2p.value;
				dst = convert_to_mov_imm(drcbe, dst, inst, &dstp, result & 0xffffffff);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, result >> 32);
			}
		}
		else if (inst->size == 8)
		{
			UINT64 reslo, reshi;
			if (!compute_hi)
			{
				dmulu(&reslo, &reslo, src1p.value, src2p.value);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, reslo);
			}
			else
			{
				dmulu(&reslo, &reshi, src1p.value, src2p.value);
				dst = convert_to_mov_imm(drcbe, dst, inst, &dstp, reslo);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, reshi);
			}
		}
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);									// mov   eax,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_mul_m32(&dst, MABS(src2p.value));										// mul   [src2p]
		else if (src2p.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mul_r32(&dst, src2p.value);											// mul   src2p
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE)
		{
			emit_mov_r32_imm(&dst, REG_EDX, src2p.value);								// mov   edx,src2p
			emit_mul_r32(&dst, REG_EDX);												// mul   edx
		}
		emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);									// mov   dstp,eax
		if (compute_hi)
			emit_mov_p32_r32(drcbe, &dst, &edstp, REG_EDX);								// mov   edstp,edx
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_push_p64(drcbe, &dst, &src2p);												// push  src2p
		emit_push_p64(drcbe, &dst, &src1p);												// push  src1p
		if (!compute_hi)
			emit_push_imm(&dst, (FPTR)&drcbe->reslo);									// push  &reslo
		else
			emit_push_imm(&dst, (FPTR)&drcbe->reshi);									// push  &reshi
		emit_push_imm(&dst, (FPTR)&drcbe->reslo);										// push  &reslo
		emit_call(&dst, (x86code *)dmulu);												// call  dmulu
		emit_add_r32_imm(&dst, REG_ESP, 24);											// add   esp,24
		emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reslo + 0));				// mov   eax,reslo.lo
		emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reslo + 1));				// mov   edx,reslo.hi
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
		if (inst->condflags != 0)
			emit_or_r32_r32(&dst, REG_EAX, REG_EDX);									// or    eax,edx
		if (compute_hi)
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reshi + 0));			// mov   eax,reshi.lo
			emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reshi + 1));			// mov   edx,reshi.hi
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_EAX, REG_EDX);					// mov   edstp,edx:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_muls - process a MULS opcode
-------------------------------------------------*/

static x86code *op_muls(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, edstp, src1p, src2p;
	int compute_hi;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_4_commutative(drcbe, inst, &dstp, PTYPE_MR, &edstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);
	compute_hi = (dstp.type != edstp.type || dstp.value != edstp.value);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
	{
		if (inst->size == 4)
		{
			if (!compute_hi)
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, (INT32)src1p.value * (INT32)src2p.value);
			else
			{
				UINT64 result = (INT64)(INT32)src1p.value * (INT64)(INT32)src2p.value;
				dst = convert_to_mov_imm(drcbe, dst, inst, &dstp, result & 0xffffffff);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, result >> 32);
			}
		}
		else if (inst->size == 8)
		{
			UINT64 reslo, reshi;
			if (!compute_hi)
			{
				dmuls(&reslo, &reslo, src1p.value, src2p.value);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, reslo);
			}
			else
			{
				dmuls(&reslo, &reshi, src1p.value, src2p.value);
				dst = convert_to_mov_imm(drcbe, dst, inst, &dstp, reslo);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, reshi);
			}
		}
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* 32-bit destination with memory/immediate or register/immediate */
		if (!compute_hi && src1p.type != DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_imul_r32_m32_imm(&dst, REG_EAX, MABS(src1p.value), src2p.value);	// imul  eax,[src1p],src2p
			else if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_imul_r32_r32_imm(&dst, REG_EAX, src1p.value, src2p.value);			// imul  eax,src1p,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);								// mov   dstp,eax
		}

		/* 32-bit destination, general case */
		else if (!compute_hi)
		{
			emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);								// mov   eax,src1p
			if (src2p.type == DRCUML_PTYPE_MEMORY)
				emit_imul_r32_m32(&dst, REG_EAX, MABS(src2p.value));					// imul  eax,[src2p]
			else if (src2p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_imul_r32_r32(&dst, REG_EAX, src2p.value);							// imul  eax,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);								// mov   dstp,eax
		}

		/* 64-bit destination, general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);								// mov   eax,src1p
			if (src2p.type == DRCUML_PTYPE_MEMORY)
				emit_imul_m32(&dst, MABS(src2p.value));									// imul  [src2p]
			else if (src2p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_imul_r32(&dst, src2p.value);										// imul  src2p
			else if (src2p.type == DRCUML_PTYPE_IMMEDIATE)
			{
				emit_mov_r32_imm(&dst, REG_EDX, src2p.value);							// mov   edx,src2p
				emit_imul_r32(&dst, REG_EDX);											// imul  edx
			}
			emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);								// mov   dstp,eax
			emit_mov_p32_r32(drcbe, &dst, &edstp, REG_EDX);								// mov   edstp,edx
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_push_p64(drcbe, &dst, &src2p);												// push  src2p
		emit_push_p64(drcbe, &dst, &src1p);												// push  src1p
		if (!compute_hi)
			emit_push_imm(&dst, (FPTR)&drcbe->reslo);									// push  &reslo
		else
			emit_push_imm(&dst, (FPTR)&drcbe->reshi);									// push  &reshi
		emit_push_imm(&dst, (FPTR)&drcbe->reslo);										// push  &reslo
		emit_call(&dst, (x86code *)dmuls);												// call  dmuls
		emit_add_r32_imm(&dst, REG_ESP, 24);											// add   esp,24
		emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reslo + 0));				// mov   eax,reslo.lo
		emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reslo + 1));				// mov   edx,reslo.hi
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
		if (inst->condflags != 0)
			emit_or_r32_r32(&dst, REG_EAX, REG_EDX);									// or    eax,edx
		if (compute_hi)
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reshi + 0));			// mov   eax,reshi.lo
			emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reshi + 1));			// mov   edx,reshi.hi
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_EAX, REG_EDX);					// mov   edstp,edx:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_divu - process a DIVU opcode
-------------------------------------------------*/

static x86code *op_divu(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, edstp, src1p, src2p;
	int compute_rem;
	emit_link skip;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &edstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);
	compute_rem = (dstp.type != edstp.type || dstp.value != edstp.value);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
	{
		if (inst->size == 4)
		{
			if ((UINT32)src2p.value == 0)
				return dst;
			if (!compute_rem)
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT32)src1p.value / (UINT32)src2p.value);
			else
			{
				dst = convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT32)src1p.value / (UINT32)src2p.value);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT32)src1p.value % (UINT32)src2p.value);
			}
		}
		else if (inst->size == 8)
		{
			UINT64 reslo, reshi;
			if (src2p.value == 0)
				return dst;
			if (!compute_rem)
			{
				ddivu(&reslo, &reslo, src1p.value, src2p.value);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, reslo);
			}
			else
			{
				ddivu(&reslo, &reshi, src1p.value, src2p.value);
				dst = convert_to_mov_imm(drcbe, dst, inst, &dstp, reslo);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, reshi);
			}
		}
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_r32_p32(drcbe, &dst, REG_ECX, &src2p);									// mov   ecx,src2p
		emit_jecxz_link(&dst, &skip);													// jecxz skip
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);									// mov   eax,src1p
		emit_xor_r32_r32(&dst, REG_EDX, REG_EDX);										// xor   edx,edx
		emit_div_r32(&dst, REG_ECX);													// div   ecx
		emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);									// mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(drcbe, &dst, &edstp, REG_EDX);								// mov   edstp,edx
		resolve_link(&dst, &skip);													// skip:
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_push_p64(drcbe, &dst, &src2p);												// push  src2p
		emit_push_p64(drcbe, &dst, &src1p);												// push  src1p
		if (!compute_rem)
			emit_push_imm(&dst, (FPTR)&drcbe->reslo);									// push  &reslo
		else
			emit_push_imm(&dst, (FPTR)&drcbe->reshi);									// push  &reshi
		emit_push_imm(&dst, (FPTR)&drcbe->reslo);										// push  &reslo
		emit_call(&dst, (x86code *)ddivu);												// call  ddivu
		emit_add_r32_imm(&dst, REG_ESP, 24);											// add   esp,24
		emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reslo + 0));				// mov   eax,reslo.lo
		emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reslo + 1));				// mov   edx,reslo.hi
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
		if (inst->condflags != 0)
			emit_or_r32_r32(&dst, REG_EAX, REG_EDX);									// or    eax,edx
		if (compute_rem)
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reshi + 0));			// mov   eax,reshi.lo
			emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reshi + 1));			// mov   edx,reshi.hi
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_EAX, REG_EDX);					// mov   edstp,edx:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_divs - process a DIVS opcode
-------------------------------------------------*/

static x86code *op_divs(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, edstp, src1p, src2p;
	int compute_rem;
	emit_link skip;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &edstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);
	compute_rem = (dstp.type != edstp.type || dstp.value != edstp.value);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
	{
		if (inst->size == 4)
		{
			if ((INT32)src2p.value == 0)
				return dst;
			if (!compute_rem)
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, (INT32)src1p.value / (INT32)src2p.value);
			else
			{
				dst = convert_to_mov_imm(drcbe, dst, inst, &dstp, (INT32)src1p.value / (INT32)src2p.value);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, (INT32)src1p.value % (INT32)src2p.value);
			}
		}
		else if (inst->size == 8)
		{
			UINT64 reslo, reshi;
			if (src2p.value == 0)
				return dst;
			if (!compute_rem)
			{
				ddivs(&reslo, &reslo, src1p.value, src2p.value);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, reslo);
			}
			else
			{
				ddivs(&reslo, &reshi, src1p.value, src2p.value);
				dst = convert_to_mov_imm(drcbe, dst, inst, &dstp, reslo);
				return convert_to_mov_imm(drcbe, dst, inst, &dstp, reshi);
			}
		}
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_r32_p32(drcbe, &dst, REG_ECX, &src2p);									// mov   ecx,src2p
		emit_jecxz_link(&dst, &skip);													// jecxz skip
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);									// mov   eax,src1p
		emit_cdq(&dst);																	// cdq
		emit_idiv_r32(&dst, REG_ECX);													// idiv  ecx
		emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);									// mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(drcbe, &dst, &edstp, REG_EDX);								// mov   edstp,edx
		resolve_link(&dst, &skip);													// skip:
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_push_p64(drcbe, &dst, &src2p);												// push  src2p
		emit_push_p64(drcbe, &dst, &src1p);												// push  src1p
		if (!compute_rem)
			emit_push_imm(&dst, (FPTR)&drcbe->reslo);									// push  &reslo
		else
			emit_push_imm(&dst, (FPTR)&drcbe->reshi);									// push  &reshi
		emit_push_imm(&dst, (FPTR)&drcbe->reslo);										// push  &reslo
		emit_call(&dst, (x86code *)ddivs);												// call  ddivs
		emit_add_r32_imm(&dst, REG_ESP, 24);											// add   esp,24
		emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reslo + 0));				// mov   eax,reslo.lo
		emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reslo + 1));				// mov   edx,reslo.hi
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
		if (inst->condflags != 0)
			emit_or_r32_r32(&dst, REG_EAX, REG_EDX);									// or    eax,edx
		if (compute_rem)
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reshi + 0));			// mov   eax,reshi.lo
			emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reshi + 1));			// mov   edx,reshi.hi
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_EAX, REG_EDX);					// mov   edstp,edx:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_and - process a AND opcode
-------------------------------------------------*/

static x86code *op_and(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, src1p.value & src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && (src2p.value & size_to_mask[inst->size]) == size_to_mask[inst->size] && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_and_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// and   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_and_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// and   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_and_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// and   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);						// mov   dstreg:dstp,[src1p]
			emit_and_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);				// and   dstreg:dstp,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);						// mov   dstp,dstreg:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_test - process a TEST opcode
-------------------------------------------------*/

static x86code *op_test(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter src1p, src2p;
	int src1reg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_2_commutative(drcbe, inst, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	src1reg = param_select_register(REG_EAX, &src1p, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* src1p in memory */
		if (src1p.type == DRCUML_PTYPE_MEMORY)
			emit_test_m32_p32(drcbe, &dst, MABS(src1p.value), &src2p, inst);			// test  [src1p],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, src1reg, &src1p);								// mov   src1reg,src1p
			emit_test_r32_p32(drcbe, &dst, src1reg, &src2p, inst);						// test  src1reg,src2p
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* src1p in memory */
		if (src1p.type == DRCUML_PTYPE_MEMORY)
			emit_test_m64_p64(drcbe, &dst, MABS(src1p.value), &src2p, inst);			// test  [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, src1reg, REG_EDX, &src1p);					// mov   src1reg:dstp,[src1p]
			emit_test_r64_p64(drcbe, &dst, src1reg, REG_EDX, &src2p, inst);				// test  src1reg:dstp,src2p
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_or - process a OR opcode
-------------------------------------------------*/

static x86code *op_or(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, src1p.value | src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_or_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// or    [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_or_r32_p32(drcbe, &dst, dstreg, &src2p, inst);							// or    dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_or_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// or    [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);						// mov   dstreg:dstp,[src1p]
			emit_or_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);				// or    dstreg:dstp,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);						// mov   dstp,dstreg:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_xor - process a XOR opcode
-------------------------------------------------*/

static x86code *op_xor(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, src1p.value & src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_xor_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// xor   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_xor_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// xor   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_xor_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// xor   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);						// mov   dstreg:dstp,[src1p]
			emit_xor_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);				// xor   dstreg:dstp,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);						// mov   dstp,dstreg:eax
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_lzcnt - process a LZCNT opcode
-------------------------------------------------*/

static x86code *op_lzcnt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == 0);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0 && inst->size == 4)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, count_leading_zeros(srcp.value));

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &srcp);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_mov_r32_p32(drcbe, &dst, dstreg, &srcp);									// mov   dstreg,src1p
		emit_mov_r32_imm(&dst, REG_ECX, 32);											// mov   ecx,32
		emit_bsr_r32_r32(&dst, dstreg, dstreg);											// bsr   dstreg,dstreg
		emit_cmovcc_r32_r32(&dst, COND_Z, dstreg, REG_ECX);								// cmovz dstreg,ecx
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_link skip;

		emit_mov_r64_p64(drcbe, &dst, REG_EDX, dstreg, &srcp);							// mov   dstreg:edx,srcp
		emit_bsr_r32_r32(&dst, dstreg, dstreg);											// bsr   dstreg,dstreg
		emit_jcc_short_link(&dst, COND_NZ, &skip);										// jnz   skip
		emit_mov_r32_imm(&dst, REG_ECX, 32);											// mov   ecx,32
		emit_bsr_r32_r32(&dst, dstreg, REG_EDX);										// bsr   dstreg,edx
		emit_cmovcc_r32_r32(&dst, COND_Z, dstreg, REG_ECX);								// cmovz dstreg,ecx
		emit_add_r32_imm(&dst, REG_ECX, 32);											// add   ecx,32
		resolve_link(&dst, &skip);													// skip:
		emit_xor_r32_r32(&dst, REG_EDX, REG_EDX);								// xor   edx,edx
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,edx:dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_bswap - process a BSWAP opcode
-------------------------------------------------*/

static x86code *op_bswap(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == 0);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
	{
		if (inst->size == 4)
			return convert_to_mov_imm(drcbe, dst, inst, &dstp, FLIPENDIAN_INT32(srcp.value));
		else if (inst->size == 8)
			return convert_to_mov_imm(drcbe, dst, inst, &dstp, FLIPENDIAN_INT64(srcp.value));
	}

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &srcp);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_mov_r32_p32(drcbe, &dst, dstreg, &srcp);									// mov   dstreg,src1p
		emit_bswap_r32(&dst, dstreg);													// bswap dstreg
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_mov_r64_p64(drcbe, &dst, REG_EDX, dstreg, &srcp);							// mov   dstreg:edx,srcp
		emit_bswap_r32(&dst, dstreg);													// bswap dstreg
		emit_bswap_r32(&dst, REG_EDX);													// bswap edx
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EDX, dstreg);							// mov   dstp,dstreg:edx
	}
	return dst;
}


/*-------------------------------------------------
    op_shl - process a SHL opcode
-------------------------------------------------*/

static x86code *op_shl(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, src1p.value << src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_shl_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// shl   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_shl_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// shl   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);							// mov   dstreg:dstp,[src1p]
		emit_shl_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);					// shl   dstreg:dstp,src2p
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,dstreg:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_shr - process a SHR opcode
-------------------------------------------------*/

static x86code *op_shr(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT64)src1p.value >> src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_shr_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// shr   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_shr_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// shr   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);							// mov   dstreg:dstp,[src1p]
		emit_shr_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);					// shr   dstreg:dstp,src2p
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,dstreg:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_sar - process a SAR opcode
-------------------------------------------------*/

static x86code *op_sar(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
	{
		if (inst->size == 4)
			return convert_to_mov_imm(drcbe, dst, inst, &dstp, (INT32)src1p.value >> src2p.value);
		else if (inst->size == 8)
			return convert_to_mov_imm(drcbe, dst, inst, &dstp, (INT64)src1p.value >> src2p.value);
	}
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_sar_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// sar   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_sar_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// sar   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);							// mov   dstreg:dstp,[src1p]
		emit_sar_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);					// sar   dstreg:dstp,src2p
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,dstreg:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_rol - process a rol opcode
-------------------------------------------------*/

static x86code *op_rol(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT64)src1p.value >> src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_rol_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// rol   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_rol_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// rol   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);							// mov   dstreg:dstp,[src1p]
		emit_rol_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);					// rol   dstreg:dstp,src2p
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,dstreg:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_ror - process a ROR opcode
-------------------------------------------------*/

static x86code *op_ror(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT64)src1p.value >> src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_ror_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// ror   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_ror_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// ror   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);							// mov   dstreg:dstp,[src1p]
		emit_ror_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);					// ror   dstreg:dstp,src2p
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,dstreg:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_rolc - process a ROLC opcode
-------------------------------------------------*/

static x86code *op_rolc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT64)src1p.value >> src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_rcl_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// rcl   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_rcl_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// rcl   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);							// mov   dstreg:dstp,[src1p]
		emit_rcl_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);					// rcl   dstreg:dstp,src2p
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,dstreg:eax
	}
	return dst;
}


/*-------------------------------------------------
    op_rorc - process a RORC opcode
-------------------------------------------------*/

static x86code *op_rorc(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_Z | DRCUML_FLAG_S)) == 0);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* degenerate cases -- convert to a move */
	if (src1p.type == DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->condflags == 0)
		return convert_to_mov_imm(drcbe, dst, inst, &dstp, (UINT64)src1p.value >> src2p.value);
	if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0 && inst->condflags == 0)
		return convert_to_mov_src1(drcbe, dst, inst, &dstp, &src1p);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_rcr_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// rcr   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_rcr_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// rcr   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src1p);							// mov   dstreg:dstp,[src1p]
		emit_rcr_r64_p64(drcbe, &dst, dstreg, REG_EDX, &src2p, inst);					// rcr   dstreg:dstp,src2p
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,dstreg:eax
	}
	return dst;
}



/***************************************************************************
    FLOATING POINT OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    op_fload - process a FLOAD opcode
-------------------------------------------------*/

static x86code *op_fload(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->size == 4)
			emit_fld_m32(&dst, MABS(basep.value + 4*indp.value));						// fld   [basep + 4*indp]
		else if (inst->size == 8)
			emit_fld_m64(&dst, MABS(basep.value + 8*indp.value));						// fld   [basep + 8*indp]
	}

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		if (inst->size == 4)
			emit_fld_m32(&dst, MISD(indreg, 4, basep.value));							// fld   [basep + 4*indp]
		else if (inst->size == 8)
			emit_fld_m64(&dst, MISD(indreg, 8, basep.value));							// fld   [basep + 8*indp]
	}

	/* general case */
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_fstore - process a FSTORE opcode
-------------------------------------------------*/

static x86code *op_fstore(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, basep, indp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &basep, PTYPE_M, &indp, PTYPE_MRI, &srcp, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->size == 4)
			emit_fstp_m32(&dst, MABS(basep.value + 4*indp.value));						// fstp  [basep + 4*indp]
		else if (inst->size == 8)
			emit_fstp_m64(&dst, MABS(basep.value + 8*indp.value));						// fstp  [basep + 8*indp]
	}

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		if (inst->size == 4)
			emit_fstp_m32(&dst, MISD(indreg, 4, basep.value));							// fstp  [basep + 4*indp]
		else if (inst->size == 8)
			emit_fstp_m64(&dst, MISD(indreg, 8, basep.value));							// fstp  [basep + 8*indp]
	}
	return dst;
}


/*-------------------------------------------------
    op_fread - process a FREAD opcode
-------------------------------------------------*/

static x86code *op_fread(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, spacep, addrp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &spacep, PTYPE_I, &addrp, PTYPE_MRI);

	/* set up a call to the read dword/qword handler */
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	if (inst->size == 4)
		emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_dword);// call   read_dword
	else if (inst->size == 8)
		emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->read_qword);// call   read_qword
	emit_add_r32_imm(&dst, REG_ESP, 4);													// add    esp,4

	/* store result */
	if (inst->size == 4)
		emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);									// mov   dstp,eax
	else if (inst->size == 8)
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax

	return dst;
}


/*-------------------------------------------------
    op_fwrite - process a FWRITE opcode
-------------------------------------------------*/

static x86code *op_fwrite(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter spacep, addrp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &spacep, PTYPE_I, &addrp, PTYPE_MRI, &srcp, PTYPE_MF);

	/* set up a call to the write dword/qword handler */
	if (inst->size == 4)
		emit_push_p32(drcbe, &dst, &srcp);												// push   srcp
	else if (inst->size == 8)
		emit_push_p64(drcbe, &dst, &srcp);												// push   srcp
	emit_push_p32(drcbe, &dst, &addrp);													// push   addrp
	if (inst->size == 4)
		emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->write_dword);// call   write_dword
	else if (inst->size == 8)
		emit_call(&dst, (x86code *)active_address_space[spacep.value].accessors->write_qword);// call   write_qword
	emit_add_r32_imm(&dst, REG_ESP, 4 + inst->size);									// add    esp,8

	return dst;
}


/*-------------------------------------------------
    op_fmov - process a FMOV opcode
-------------------------------------------------*/

static x86code *op_fmov(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	emit_link skip = { 0 };

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS || (inst->condflags >= DRCUML_COND_Z && inst->condflags < DRCUML_COND_MAX));

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* always start with a jmp */
	if (inst->condflags != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condflags), &skip);			// jcc   skip

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	/* resolve the jump */
	if (skip.target != NULL)
		resolve_link(&dst, &skip);													// skip:
	return dst;
}


/*-------------------------------------------------
    op_ftoi4 - process a FTOI4 opcode
-------------------------------------------------*/

static x86code *op_ftoi4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	if (dstp.type == DRCUML_PTYPE_MEMORY)
		emit_fistp_m32(&dst, MABS(dstp.value));											// fistp [dstp]
	else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_fistp_m32(&dst, MABS(drcbe->reglo[dstp.value]));							// fistp reglo[dstp]
		emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));				// mov   dstp,reglo[dstp]
	}
	return dst;
}


/*-------------------------------------------------
    op_ftoi4t - process a FTOI4T opcode
-------------------------------------------------*/

static x86code *op_ftoi4t(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* non-SSE3 case */
	if (!drcbe->sse3)
	{
		/* save and set the control word */
		emit_fstcw_m16(&dst, MABS(&drcbe->fmodesave));									// fstcw [fmodesave]
		emit_fldcw_m16(&dst, MABS(&fp_control[DRCUML_FMOD_TRUNC]));						// fldcw fpcontrol[DRCUML_FMOD_TRUNC]

		/* general case */
		emit_fld_p(&dst, inst->size, &srcp);											// fld   srcp
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_fistp_m32(&dst, MABS(dstp.value));										// fistp [dstp]
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			emit_fistp_m32(&dst, MABS(drcbe->reglo[dstp.value]));						// fistp reglo[dstp]
			emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));			// mov   dstp,reglo[dstp]
		}

		/* restore control word and proceed */
		emit_fldcw_m16(&dst, MABS(&drcbe->fmodesave));									// fldcw [fmodesave]
	}

	/* SSE3 case */
	else
	{
		/* general case */
		emit_fld_p(&dst, inst->size, &srcp);											// fld   srcp
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_fisttp_m32(&dst, MABS(dstp.value));									// fisttp [dstp]
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			emit_fisttp_m32(&dst, MABS(drcbe->reglo[dstp.value]));						// fisttp reglo[dstp]
			emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));			// mov   dstp,reglo[dstp]
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_ftoi4r - process a FTOI4R opcode
-------------------------------------------------*/

static x86code *op_ftoi4r(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* save and set the control word */
	emit_fstcw_m16(&dst, MABS(&drcbe->fmodesave));										// fstcw [fmodesave]
	emit_fldcw_m16(&dst, MABS(&fp_control[DRCUML_FMOD_ROUND]));							// fldcw fpcontrol[DRCUML_FMOD_ROUND]

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	if (dstp.type == DRCUML_PTYPE_MEMORY)
		emit_fistp_m32(&dst, MABS(dstp.value));											// fistp [dstp]
	else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_fistp_m32(&dst, MABS(drcbe->reglo[dstp.value]));							// fistp reglo[dstp]
		emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));				// mov   dstp,reglo[dstp]
	}

	/* restore control word and proceed */
	emit_fldcw_m16(&dst, MABS(&drcbe->fmodesave));										// fldcw [fmodesave]
	return dst;
}


/*-------------------------------------------------
    op_ftoi4f - process a FTOI4F opcode
-------------------------------------------------*/

static x86code *op_ftoi4f(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* save and set the control word */
	emit_fstcw_m16(&dst, MABS(&drcbe->fmodesave));										// fstcw [fmodesave]
	emit_fldcw_m16(&dst, MABS(&fp_control[DRCUML_FMOD_FLOOR]));							// fldcw fpcontrol[DRCUML_FMOD_FLOOR]

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	if (dstp.type == DRCUML_PTYPE_MEMORY)
		emit_fistp_m32(&dst, MABS(dstp.value));											// fistp [dstp]
	else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_fistp_m32(&dst, MABS(drcbe->reglo[dstp.value]));							// fistp reglo[dstp]
		emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));				// mov   dstp,reglo[dstp]
	}

	/* restore control word and proceed */
	emit_fldcw_m16(&dst, MABS(&drcbe->fmodesave));										// fldcw [fmodesave]
	return dst;
}


/*-------------------------------------------------
    op_ftoi4c - process a FTOI4C opcode
-------------------------------------------------*/

static x86code *op_ftoi4c(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* save and set the control word */
	emit_fstcw_m16(&dst, MABS(&drcbe->fmodesave));										// fstcw [fmodesave]
	emit_fldcw_m16(&dst, MABS(&fp_control[DRCUML_FMOD_CEIL]));							// fldcw fpcontrol[DRCUML_FMOD_CEIL]

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	if (dstp.type == DRCUML_PTYPE_MEMORY)
		emit_fistp_m32(&dst, MABS(dstp.value));											// fistp [dstp]
	else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_fistp_m32(&dst, MABS(drcbe->reglo[dstp.value]));							// fistp reglo[dstp]
		emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));				// mov   dstp,reglo[dstp]
	}

	/* restore control word and proceed */
	emit_fldcw_m16(&dst, MABS(&drcbe->fmodesave));										// fldcw [fmodesave]
	return dst;
}


/*-------------------------------------------------
    op_ftoi8 - process a FTOI8 opcode
-------------------------------------------------*/

static x86code *op_ftoi8(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	if (dstp.type == DRCUML_PTYPE_MEMORY)
		emit_fistp_m64(&dst, MABS(dstp.value));											// fistp [dstp]
	else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_fistp_m64(&dst, MABS(drcbe->reglo[dstp.value]));							// fistp reglo[dstp]
		emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));				// mov   dstp,reglo[dstp]
	}
	return dst;
}


/*-------------------------------------------------
    op_ftoi8t - process a FTOI8T opcode
-------------------------------------------------*/

static x86code *op_ftoi8t(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* non-SSE3 case */
	if (!drcbe->sse3)
	{
		/* save and set the control word */
		emit_fstcw_m16(&dst, MABS(&drcbe->fmodesave));									// fstcw [fmodesave]
		emit_fldcw_m16(&dst, MABS(&fp_control[DRCUML_FMOD_TRUNC]));						// fldcw fpcontrol[DRCUML_FMOD_TRUNC]

		/* general case */
		emit_fld_p(&dst, inst->size, &srcp);											// fld   srcp
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_fistp_m64(&dst, MABS(dstp.value));										// fistp [dstp]
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			emit_fistp_m64(&dst, MABS(drcbe->reglo[dstp.value]));						// fistp reglo[dstp]
			emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));			// mov   dstp,reglo[dstp]
		}

		/* restore control word and proceed */
		emit_fldcw_m16(&dst, MABS(&drcbe->fmodesave));									// fldcw [fmodesave]
	}

	/* SSE3 case */
	else
	{
		/* general case */
		emit_fld_p(&dst, inst->size, &srcp);											// fld   srcp
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_fisttp_m64(&dst, MABS(dstp.value));									// fisttp [dstp]
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			emit_fisttp_m64(&dst, MABS(drcbe->reglo[dstp.value]));						// fisttp reglo[dstp]
			emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));			// mov   dstp,reglo[dstp]
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_ftoi8r - process a FTOI8R opcode
-------------------------------------------------*/

static x86code *op_ftoi8r(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* save and set the control word */
	emit_fstcw_m16(&dst, MABS(&drcbe->fmodesave));										// fstcw [fmodesave]
	emit_fldcw_m16(&dst, MABS(&fp_control[DRCUML_FMOD_ROUND]));							// fldcw fpcontrol[DRCUML_FMOD_ROUND]

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	if (dstp.type == DRCUML_PTYPE_MEMORY)
		emit_fistp_m64(&dst, MABS(dstp.value));											// fistp [dstp]
	else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_fistp_m64(&dst, MABS(drcbe->reglo[dstp.value]));							// fistp reglo[dstp]
		emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));				// mov   dstp,reglo[dstp]
	}

	/* restore control word and proceed */
	emit_fldcw_m16(&dst, MABS(&drcbe->fmodesave));										// fldcw [fmodesave]
	return dst;
}


/*-------------------------------------------------
    op_ftoi8f - process a FTOI8F opcode
-------------------------------------------------*/

static x86code *op_ftoi8f(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* save and set the control word */
	emit_fstcw_m16(&dst, MABS(&drcbe->fmodesave));										// fstcw [fmodesave]
	emit_fldcw_m16(&dst, MABS(&fp_control[DRCUML_FMOD_FLOOR]));							// fldcw fpcontrol[DRCUML_FMOD_FLOOR]

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	if (dstp.type == DRCUML_PTYPE_MEMORY)
		emit_fistp_m64(&dst, MABS(dstp.value));											// fistp [dstp]
	else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_fistp_m64(&dst, MABS(drcbe->reglo[dstp.value]));							// fistp reglo[dstp]
		emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));				// mov   dstp,reglo[dstp]
	}

	/* restore control word and proceed */
	emit_fldcw_m16(&dst, MABS(&drcbe->fmodesave));										// fldcw [fmodesave]
	return dst;
}


/*-------------------------------------------------
    op_ftoi8c - process a FTOI8C opcode
-------------------------------------------------*/

static x86code *op_ftoi8c(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF);

	/* save and set the control word */
	emit_fstcw_m16(&dst, MABS(&drcbe->fmodesave));										// fstcw [fmodesave]
	emit_fldcw_m16(&dst, MABS(&fp_control[DRCUML_FMOD_CEIL]));							// fldcw fpcontrol[DRCUML_FMOD_CEIL]

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	if (dstp.type == DRCUML_PTYPE_MEMORY)
		emit_fistp_m64(&dst, MABS(dstp.value));											// fistp [dstp]
	else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_fistp_m64(&dst, MABS(drcbe->reglo[dstp.value]));							// fistp reglo[dstp]
		emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));				// mov   dstp,reglo[dstp]
	}

	/* restore control word and proceed */
	emit_fldcw_m16(&dst, MABS(&drcbe->fmodesave));										// fldcw [fmodesave]
	return dst;
}


/*-------------------------------------------------
    op_ffrfs - process a FFRFS opcode
-------------------------------------------------*/

static x86code *op_ffrfs(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* general case */
	emit_fld_m32(&dst, MABS(srcp.value));												// fld   [srcp]
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_ffrfd - process a FFRFD opcode
-------------------------------------------------*/

static x86code *op_ffrfd(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* general case */
	emit_fld_m64(&dst, MABS(srcp.value));												// fld   [srcp]
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_ffri4 - process a FFRI4 opcode
-------------------------------------------------*/

static x86code *op_ffri4(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MRI);

	/* general case */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_mov_m32_imm(&dst, MABS(&drcbe->fptemp), srcp.value);						// mov   [fptemp],srcp
		emit_fild_m32(&dst, MABS(&drcbe->fptemp));										// fild  [fptemp]
	}
	else if (srcp.type == DRCUML_PTYPE_MEMORY)
		emit_fild_m32(&dst, MABS(srcp.value));											// fild  [srcp]
	else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_mov_m32_r32(&dst, MABS(drcbe->reglo[srcp.value]), srcp.value);				// mov   reglo[srcp],srcp
		emit_fild_m32(&dst, MABS(drcbe->reglo[srcp.value]));							// fild  reglo[srcp]
	}
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  [dstp]
	return dst;
}


/*-------------------------------------------------
    op_ffri8 - process a FFRI8 opcode
-------------------------------------------------*/

static x86code *op_ffri8(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MRI);

	/* general case */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_mov_m32_imm(&dst, MABS(&drcbe->fptemp), srcp.value);						// mov   [fptemp],srcp
		emit_mov_m32_imm(&dst, MABS((UINT8 *)&drcbe->fptemp + 4), srcp.value);			// mov   [fptemp+4],srcp
		emit_fild_m64(&dst, MABS(&drcbe->fptemp));										// fild  [fptemp]
	}
	else if (srcp.type == DRCUML_PTYPE_MEMORY)
		emit_fild_m64(&dst, MABS(srcp.value));											// fild  [srcp]
	else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_mov_m32_r32(&dst, MABS(drcbe->reglo[srcp.value]), srcp.value);				// mov   reglo[srcp],srcp
		emit_fild_m64(&dst, MABS(drcbe->reglo[srcp.value]));							// fild  reglo[srcp]
	}
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  [dstp]
	return dst;
}


/*-------------------------------------------------
    op_fadd - process a FADD opcode
-------------------------------------------------*/

static x86code *op_fadd(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &src1p);												// fld   src1p
	emit_fld_p(&dst, inst->size, &src2p);												// fld   src2p
	emit_faddp(&dst);																	// faddp
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_fsub - process a FSUB opcode
-------------------------------------------------*/

static x86code *op_fsub(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &src1p);												// fld   src1p
	emit_fld_p(&dst, inst->size, &src2p);												// fld   src2p
	emit_fsubp(&dst);																	// fsubp
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_fcmp - process a FCMP opcode
-------------------------------------------------*/

static x86code *op_fcmp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter src1p, src2p;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert((inst->condflags & ~(DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_U)) == 0);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &src2p);												// fld   src2p
	emit_fld_p(&dst, inst->size, &src1p);												// fld   src1p
	emit_fcompp(&dst);																	// fcompp
	emit_fstsw_ax(&dst);																// fnstsw ax
	emit_sahf(&dst);																	// sahf

	return dst;
}


/*-------------------------------------------------
    op_fmul - process a FMUL opcode
-------------------------------------------------*/

static x86code *op_fmul(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &src1p);												// fld   src1p
	emit_fld_p(&dst, inst->size, &src2p);												// fld   src2p
	emit_fmulp(&dst);																	// fmulp
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_fdiv - process a FDIV opcode
-------------------------------------------------*/

static x86code *op_fdiv(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &src1p);												// fld   src1p
	emit_fld_p(&dst, inst->size, &src2p);												// fld   src2p
	emit_fdivp(&dst);																	// fdivp
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_fneg - process a FNEG opcode
-------------------------------------------------*/

static x86code *op_fneg(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	emit_fchs(&dst);																	// fchs
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_fabs - process a FABS opcode
-------------------------------------------------*/

static x86code *op_fabs(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	emit_fabs(&dst);																	// fabs
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_fsqrt - process a FSQRT opcode
-------------------------------------------------*/

static x86code *op_fsqrt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	emit_fsqrt(&dst);																	// fsqrt
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_frecip - process a FRECIP opcode
-------------------------------------------------*/

static x86code *op_frecip(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* general case */
	emit_fld1(&dst);																	// fld1
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	emit_fdivp(&dst);																	// fdivp
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_frsqrt - process a FRSQRT opcode
-------------------------------------------------*/

static x86code *op_frsqrt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert(inst->condflags == DRCUML_COND_ALWAYS);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* general case */
	emit_fld1(&dst);																	// fld1
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	emit_fsqrt(&dst);																	// fsqrt
	emit_fdivp(&dst);																	// fdivp
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}



/***************************************************************************
    MISCELLAENOUS FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    dmulu - perform a double-wide unsigned multiply
-------------------------------------------------*/

static void dmulu(UINT64 *dstlo, UINT64 *dsthi, UINT64 src1, UINT64 src2)
{
	UINT64 lo, hi, prevlo;
	UINT64 a, b, temp;

	/* shortcut if we don't care about the high bits */
	if (dstlo == dsthi)
	{
		*dstlo = src1 * src2;
		return;
	}

	/* fetch source values */
	a = src1;
	b = src2;
	if (a == 0 || b == 0)
	{
		*dsthi = *dstlo = 0;
		return;
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
}


/*-------------------------------------------------
    dmuls - perform a double-wide signed multiply
-------------------------------------------------*/

static void dmuls(UINT64 *dstlo, UINT64 *dsthi, INT64 src1, INT64 src2)
{
	UINT64 lo, hi, prevlo;
	UINT64 a, b, temp;

	/* shortcut if we don't care about the high bits */
	if (dstlo == dsthi)
	{
		*dstlo = src1 * src2;
		return;
	}

	/* fetch absolute source values */
	a = src1; if ((INT64)a < 0) a = -a;
	b = src2; if ((INT64)b < 0) b = -b;
	if (a == 0 || b == 0)
	{
		*dsthi = *dstlo = 0;
		return;
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
}


/*-------------------------------------------------
    ddivu - perform a double-wide unsigned divide
-------------------------------------------------*/

static void ddivu(UINT64 *dstlo, UINT64 *dsthi, UINT64 src1, UINT64 src2)
{
	/* do nothing if src2 == 0 */
	if (src2 == 0)
		return;

	/* shortcut if no remainder */
	*dstlo = src1 / src2;
	if (dstlo != dsthi)
		*dsthi = src1 % src2;
}


/*-------------------------------------------------
    ddivs - perform a double-wide signed divide
-------------------------------------------------*/

static void ddivs(UINT64 *dstlo, UINT64 *dsthi, INT64 src1, INT64 src2)
{
	/* do nothing if src2 == 0 */
	if (src2 == 0)
		return;

	/* shortcut if no remainder */
	*dstlo = src1 / src2;
	if (dstlo != dsthi)
		*dsthi = src1 % src2;
}
