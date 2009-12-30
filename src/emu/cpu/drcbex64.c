/***************************************************************************

    drcbex64.c

    64-bit x64 back-end for the universal machine language.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Future improvements/changes:

    * Add support for FP registers

    * Optimize to avoid unnecessary reloads

    * Identify common pairs and optimize output

    * Convert SUB a,0,b to NEG

    * Optimize, e.g., and [r5],i0,$FF to use rbx as temporary register
        (avoid initial move) if i0 is not needed going forward

****************************************************************************

    -------------------------
    ABI/conventions (Windows)
    -------------------------

    Registers:
        RAX        - volatile, function return value
        RBX        - non-volatile
        RCX        - volatile, integer function parameter 1
        RDX        - volatile, integer function parameter 2
        RSI        - non-volatile
        RDI        - non-volatile
        RBP        - non-volatile
        R8         - volatile, integer function parameter 3
        R9         - volatile, integer function parameter 4
        R10        - volatile
        R11        - volatile, scratch immediate storage
        R12        - non-volatile
        R13        - non-volatile
        R14        - non-volatile
        R15        - non-volatile

        XMM0       - volatile, FP function parameter 1
        XMM1       - volatile, FP function parameter 2
        XMM2       - volatile, FP function parameter 3
        XMM3       - volatile, FP function parameter 4
        XMM4       - volatile
        XMM5       - volatile
        XMM6       - non-volatile
        XMM7       - non-volatile
        XMM8       - non-volatile
        XMM9       - non-volatile
        XMM10      - non-volatile
        XMM11      - non-volatile
        XMM12      - non-volatile
        XMM13      - non-volatile
        XMM14      - non-volatile
        XMM15      - non-volatile


    -----------------------------
    ABI/conventions (Linux/MacOS)
    -----------------------------

    Registers:
        RAX        - volatile, function return value
        RBX        - non-volatile
        RCX        - volatile, integer function parameter 4
        RDX        - volatile, integer function parameter 3
        RSI        - volatile, integer function parameter 2
        RDI        - volatile, integer function parameter 1
        RBP        - non-volatile
        R8         - volatile, integer function parameter 5
        R9         - volatile, integer function parameter 6
        R10        - volatile
        R11        - volatile, scratch immediate storage
        R12        - non-volatile
        R13        - non-volatile
        R14        - non-volatile
        R15        - non-volatile

        XMM0       - volatile, FP function parameter 1
        XMM1       - volatile, FP function parameter 2
        XMM2       - volatile, FP function parameter 3
        XMM3       - volatile, FP function parameter 4
        XMM4       - volatile
        XMM5       - volatile
        XMM6       - volatile
        XMM7       - volatile
        XMM8       - volatile
        XMM9       - volatile
        XMM10      - volatile
        XMM11      - volatile
        XMM12      - volatile
        XMM13      - volatile
        XMM14      - volatile
        XMM15      - volatile


    ---------------
    Execution model
    ---------------

    Registers (Windows):
        RAX        - scratch register
        RBX        - maps to I0
        RCX        - scratch register
        RDX        - scratch register
        RSI        - maps to I1
        RDI        - maps to I2
        RBP        - pointer to code cache
        R8         - scratch register
        R9         - scratch register
        R10        - scratch register
        R11        - scratch register
        R12        - maps to I3
        R13        - maps to I4
        R14        - maps to I5
        R15        - maps to I6

    Registers (Linux/MacOS):
        RAX        - scratch register
        RBX        - maps to I0
        RCX        - scratch register
        RDX        - scratch register
        RSI        - unused
        RDI        - unused
        RBP        - pointer to code cache
        R8         - scratch register
        R9         - scratch register
        R10        - scratch register
        R11        - scratch register
        R12        - maps to I1
        R13        - maps to I2
        R14        - maps to I3
        R15        - maps to I4

    Entry point:
        Assumes 1 parameter passed, which is the codeptr of the code
        to execute once the environment is set up.

    Exit point:
        Assumes exit value is in RAX.

    Entry stack:
        [rsp]      - return

    Runtime stack:
        [rsp]      - r9 home
        [rsp+8]    - r8 home
        [rsp+16]   - rdx home
        [rsp+24]   - rcx home
        [rsp+40]   - saved r15
        [rsp+48]   - saved r14
        [rsp+56]   - saved r13
        [rsp+64]   - saved r12
        [rsp+72]   - saved ebp
        [rsp+80]   - saved edi
        [rsp+88]   - saved esi
        [rsp+96]   - saved ebx
        [rsp+104]  - ret

***************************************************************************/

#include "drcuml.h"
#include "drcbeut.h"
#include "debugger.h"
#include "x86emit.h"
#include "eminline.h"
#include "x86log.h"
#include <math.h>
#include <stddef.h>



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_HASHJMPS			(0)

#define USE_RCPSS_FOR_SINGLES	(0)
#define USE_RSQRTSS_FOR_SINGLES	(0)
#define USE_RCPSS_FOR_DOUBLES	(0)
#define USE_RSQRTSS_FOR_DOUBLES	(0)



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

#ifdef X64_WINDOWS_ABI

#define REG_PARAM1			REG_RCX
#define REG_PARAM2			REG_RDX
#define REG_PARAM3			REG_R8
#define REG_PARAM4			REG_R9

#else

#define REG_PARAM1			REG_RDI
#define REG_PARAM2			REG_RSI
#define REG_PARAM3			REG_RDX
#define REG_PARAM4			REG_RCX

#endif



/***************************************************************************
    MACROS
***************************************************************************/

#define X86_CONDITION(condition)		(condition_map[condition - DRCUML_COND_Z])
#define X86_NOT_CONDITION(condition)	(condition_map[condition - DRCUML_COND_Z] ^ 1)

#undef MABS
#define MABS(drcbe, ptr)	MBD(REG_RBP, offset_from_rbp(drcbe, (FPTR)(ptr)))

#define assert_no_condition(inst)		assert((inst)->condition == DRCUML_COND_ALWAYS)
#define assert_any_condition(inst)		assert((inst)->condition == DRCUML_COND_ALWAYS || ((inst)->condition >= DRCUML_COND_Z && (inst)->condition < DRCUML_COND_MAX))
#define assert_no_flags(inst)			assert((inst)->flags == 0)
#define assert_flags(inst, valid)		assert(((inst)->flags & ~(valid)) == 0)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* entry point */
typedef UINT32 (*x86_entry_point_func)(UINT8 *rbpvalue, x86code *entry);

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
	const device_config *	device;					/* CPU device we are associated with */
	drcuml_state *			drcuml;					/* pointer back to our owner */
	drccache *				cache;					/* pointer to the cache */
	drcuml_machine_state	state;					/* state of the machine */
	drchash_state *			hash;					/* hash table state */
	drcmap_state *			map;					/* code map */
	drclabel_list *			labels;                 /* label list */

	x86_entry_point_func	entry;					/* entry point */
	x86code *				exit;					/* exit point */
	x86code *				nocode;					/* nocode handler */

	x86code *				debug_cpu_instruction_hook;/* debugger callback */
	x86code *				debug_log_hashjmp;		/* hashjmp debugging */
	x86code *				drcmap_get_value;		/* map lookup helper */
	data_accessors			accessors[ADDRESS_SPACES];/* memory accessors */
	const address_space *	space[ADDRESS_SPACES];	/* address spaces */

	UINT8					sse41;					/* do we have SSE4.1 support? */
	UINT32					ssemode;				/* saved SSE mode */
	UINT32					ssemodesave;			/* temporary location for saving */
	UINT32					ssecontrol[4];			/* copy of the sse_control array */
	UINT32 *				absmask32;				/* absolute value mask (32-bit) */
	UINT64 *				absmask64;				/* absolute value mask (32-bit) */
	float					single1;				/* 1.0 is single-precision */
	double					double1;				/* 1.0 in double-precision */

	void *					stacksave;				/* saved stack pointer */
	void *					hashstacksave;			/* saved stack pointer for hashjmp */

	UINT8 *					rbpvalue;				/* value of RBP */
	UINT8					flagsmap[0x1000];		/* flags map */
	UINT64					flagsunmap[0x20];		/* flags unmapper */

	x86log_context *		log;					/* logging */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* primary back-end callbacks */
static drcbe_state *drcbex64_alloc(drcuml_state *drcuml, drccache *cache, const device_config *device, UINT32 flags, int modes, int addrbits, int ignorebits);
static void drcbex64_free(drcbe_state *drcbe);
static void drcbex64_reset(drcbe_state *drcbe);
static int drcbex64_execute(drcbe_state *drcbe, drcuml_codehandle *entry);
static void drcbex64_generate(drcbe_state *drcbe, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst);
static int drcbex64_hash_exists(drcbe_state *drcbe, UINT32 mode, UINT32 pc);
static void drcbex64_get_info(drcbe_state *state, drcbe_info *info);

/* private helper functions */
static void fixup_label(void *parameter, drccodeptr labelcodeptr);
static void debug_log_hashjmp(int mode, offs_t pc);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* globally-accessible interface to the backend */
const drcbe_interface drcbe_x64_be_interface =
{
	drcbex64_alloc,
	drcbex64_free,
	drcbex64_reset,
	drcbex64_execute,
	drcbex64_generate,
	drcbex64_hash_exists,
	drcbex64_get_info
};

/* opcode table */
static opcode_generate_func opcode_table[DRCUML_OP_MAX];

/* size-to-mask table */
static const UINT64 size_to_mask[] = { 0, 0xff, 0xffff, 0, 0xffffffff, 0, 0, 0, U64(0xffffffffffffffff) };

/* register mapping tables */
static const UINT8 int_register_map[DRCUML_REG_I_END - DRCUML_REG_I0] =
{
#ifdef X64_WINDOWS_ABI
	REG_RBX, REG_RSI, REG_RDI, REG_R12, REG_R13, REG_R14, REG_R15
#else
	REG_RBX, REG_R12, REG_R13, REG_R14, REG_R15
#endif
};

static UINT8 float_register_map[DRCUML_REG_F_END - DRCUML_REG_F0] =
{
	0
};

/* condition mapping table */
static const UINT8 condition_map[DRCUML_COND_MAX - DRCUML_COND_Z] =
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

/* rounding mode mapping table */
static const UINT8 fprnd_map[4] =
{
	FPRND_CHOP,		/* DRCUML_FMOD_TRUNC,       truncate */
	FPRND_NEAR,		/* DRCUML_FMOD_ROUND,       round */
	FPRND_UP,		/* DRCUML_FMOD_CEIL,        round up */
	FPRND_DOWN		/* DRCUML_FMOD_FLOOR        round down */
};



/***************************************************************************
    TABLES
***************************************************************************/

static x86code *op_handle(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_hash(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_label(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_comment(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_mapvar(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);

static x86code *op_nop(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
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
static x86code *op_getflgs(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_save(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_restore(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);

static x86code *op_load(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_loads(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_store(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_read(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_readm(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_write(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_writem(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_carry(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_set(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_mov(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_sext(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_roland(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_rolins(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
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
static x86code *op_ftoint(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ffrint(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_ffrflt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
static x86code *op_frnds(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst);
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
	{ DRCUML_OP_NOP,     op_nop },		/* NOP                            */
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
	{ DRCUML_OP_GETEXP,  op_getexp },	/* GETEXP  dst                    */
	{ DRCUML_OP_GETFLGS, op_getflgs },	/* GETFLGS dst[,f]                */
	{ DRCUML_OP_SAVE,    op_save },		/* SAVE    dst                    */
	{ DRCUML_OP_RESTORE, op_restore },	/* RESTORE dst                    */

	/* Integer Operations */
	{ DRCUML_OP_LOAD,    op_load },		/* LOAD    dst,base,index,size    */
	{ DRCUML_OP_LOADS,   op_loads },	/* LOADS   dst,base,index,size    */
	{ DRCUML_OP_STORE,   op_store },	/* STORE   base,index,src,size    */
	{ DRCUML_OP_READ,    op_read },		/* READ    dst,src1,spacesize     */
	{ DRCUML_OP_READM,   op_readm },	/* READM   dst,src1,mask,spacesize */
	{ DRCUML_OP_WRITE,   op_write },	/* WRITE   dst,src1,spacesize     */
	{ DRCUML_OP_WRITEM,  op_writem },	/* WRITEM  dst,src1,spacesize     */
	{ DRCUML_OP_CARRY,   op_carry },	/* CARRY   src,bitnum             */
	{ DRCUML_OP_SET,     op_set },		/* SET     dst,c                  */
	{ DRCUML_OP_MOV,     op_mov },		/* MOV     dst,src[,c]            */
	{ DRCUML_OP_SEXT,    op_sext },		/* SEXT    dst,src                */
	{ DRCUML_OP_ROLAND,  op_roland },	/* ROLAND  dst,src1,src2,src3     */
	{ DRCUML_OP_ROLINS,  op_rolins },	/* ROLINS  dst,src1,src2,src3     */
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
	{ DRCUML_OP_LZCNT,   op_lzcnt },	/* LZCNT   dst,src[,f]            */
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
	{ DRCUML_OP_FTOINT,  op_ftoint },	/* FTOINT  dst,src1,size,round    */
	{ DRCUML_OP_FFRINT,  op_ffrint },	/* FFRINT  dst,src1,size          */
	{ DRCUML_OP_FFRFLT,  op_ffrflt },	/* FFRFLT  dst,src1,size          */
	{ DRCUML_OP_FRNDS,   op_frnds },	/* FRNDS   dst,src1               */
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
	if ((param->type == DRCUML_PTYPE_INT_REGISTER || param->type == DRCUML_PTYPE_FLOAT_REGISTER) && (checkparam == NULL || param->type != checkparam->type || param->value != checkparam->value))
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
    offset_from_rbp - return the verified offset
    from rbp
-------------------------------------------------*/

INLINE INT32 offset_from_rbp(drcbe_state *drcbe, FPTR ptr)
{
	INT64 delta = (UINT8 *)ptr - drcbe->rbpvalue;
	assert_always((INT32)delta == delta, "offset_from_rbp: delta out of range");
	return (INT32)delta;
}


/*-------------------------------------------------
    short_immediate - true if the given immediate
    fits as a signed 32-bit value
-------------------------------------------------*/

INLINE int short_immediate(INT64 immediate)
{
	return (INT32)immediate == immediate;
}


/*-------------------------------------------------
    get_base_register_and_offset - determine right
    base register and offset to access the given
    target address
-------------------------------------------------*/

INLINE int get_base_register_and_offset(drcbe_state *drcbe, x86code **dst, FPTR target, UINT8 reg, INT32 *offset)
{
	INT64 delta = (UINT8 *)target - drcbe->rbpvalue;
	if (short_immediate(delta))
	{
		*offset = delta;
		return REG_RBP;
	}
	else
	{
		*offset = 0;
		emit_mov_r64_imm(dst, reg, target);												// mov   reg,target
		return reg;
	}
}


/*-------------------------------------------------
    emit_smart_call_r64 - generate a call either
    directly or via a call through pointer
-------------------------------------------------*/

INLINE void emit_smart_call_r64(drcbe_state *drcbe, x86code **dst, x86code *target, UINT8 reg)
{
	INT64 delta = target - (*dst + 5);
	if (short_immediate(delta))
		emit_call(dst, target);															// call  target
	else
	{
		emit_mov_r64_imm(dst, reg, (FPTR)target);										// mov   reg,target
		emit_call_r64(dst, reg);														// call  reg
	}
}


/*-------------------------------------------------
    emit_smart_call_m64 - generate a call either
    directly or via a call through pointer
-------------------------------------------------*/

INLINE void emit_smart_call_m64(drcbe_state *drcbe, x86code **dst, x86code **target)
{
	INT64 delta = *target - (*dst + 5);
	if (short_immediate(delta))
		emit_call(dst, *target);														// call  *target
	else
		emit_call_m64(dst, MABS(drcbe, target));										// call  [target]
}



/***************************************************************************
    BACKEND CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    drcbex64_alloc - allocate back-end-specific
    state
-------------------------------------------------*/

static drcbe_state *drcbex64_alloc(drcuml_state *drcuml, drccache *cache, const device_config *device, UINT32 flags, int modes, int addrbits, int ignorebits)
{
	/* SSE control register mapping */
	static const UINT32 sse_control[4] =
	{
		0xffc0, 	/* DRCUML_FMOD_TRUNC */
		0x9fc0, 	/* DRCUML_FMOD_ROUND */
		0xdfc0, 	/* DRCUML_FMOD_CEIL */
		0xbfc0		/* DRCUML_FMOD_FLOOR */
	};
	drcbe_state *drcbe;
	int opnum, entry;
	int spacenum;

	/* allocate space in the cache for our state */
	drcbe = (drcbe_state *)drccache_memory_alloc_near(cache, sizeof(*drcbe));
	if (drcbe == NULL)
		return NULL;
	memset(drcbe, 0, sizeof(*drcbe));

	/* remember our pointers */
	drcbe->device = device;
	drcbe->drcuml = drcuml;
	drcbe->cache = cache;
	drcbe->rbpvalue = drccache_near(cache) + 0x80;

	/* get address spaces and accessors */
	for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
	{
		drcbe->space[spacenum] = memory_find_address_space(device, spacenum);
		if (drcbe->space[spacenum] != NULL)
			drcbe->accessors[spacenum] = drcbe->space[spacenum]->accessors;
	}

	/* build up necessary arrays */
	memcpy(drcbe->ssecontrol, sse_control, sizeof(drcbe->ssecontrol));
	drcbe->absmask32 = (UINT32 *)drccache_memory_alloc_near(cache, 16*2 + 15);
	drcbe->absmask32 = (UINT32 *)(((FPTR)drcbe->absmask32 + 15) & ~15);
	drcbe->absmask32[0] = drcbe->absmask32[1] = drcbe->absmask32[2] = drcbe->absmask32[3] = 0x7fffffff;
	drcbe->absmask64 = (UINT64 *)&drcbe->absmask32[4];
	drcbe->absmask64[0] = drcbe->absmask64[1] = U64(0x7fffffffffffffff);
	drcbe->single1 = 1.0f;
	drcbe->double1 = 1.0;

	/* get pointers to C functions we need to call */
	drcbe->debug_cpu_instruction_hook = (x86code *)debug_cpu_instruction_hook;
	if (LOG_HASHJMPS)
		drcbe->debug_log_hashjmp = (x86code *)debug_log_hashjmp;
	drcbe->drcmap_get_value = (x86code *)drcmap_get_value;

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

	/* build the flags map */
	for (entry = 0; entry < ARRAY_LENGTH(drcbe->flagsmap); entry++)
	{
		UINT8 flags = 0;
		if (entry & 0x001) flags |= DRCUML_FLAG_C;
		if (entry & 0x004) flags |= DRCUML_FLAG_U;
		if (entry & 0x040) flags |= DRCUML_FLAG_Z;
		if (entry & 0x080) flags |= DRCUML_FLAG_S;
		if (entry & 0x800) flags |= DRCUML_FLAG_V;
		drcbe->flagsmap[entry] = flags;
	}
	for (entry = 0; entry < ARRAY_LENGTH(drcbe->flagsunmap); entry++)
	{
		UINT64 flags = 0;
		if (entry & DRCUML_FLAG_C) flags |= 0x001;
		if (entry & DRCUML_FLAG_U) flags |= 0x004;
		if (entry & DRCUML_FLAG_Z) flags |= 0x040;
		if (entry & DRCUML_FLAG_S) flags |= 0x080;
		if (entry & DRCUML_FLAG_V) flags |= 0x800;
		drcbe->flagsunmap[entry] = flags;
	}

	/* create the log */
	if (flags & DRCUML_OPTION_LOG_NATIVE)
		drcbe->log = x86log_create_context("drcbex64.asm");

	return drcbe;
}


/*-------------------------------------------------
    drcbex64_free - free back-end specific state
-------------------------------------------------*/

static void drcbex64_free(drcbe_state *drcbe)
{
	/* free the log context */
	if (drcbe->log != NULL)
		x86log_free_context(drcbe->log);
}


/*-------------------------------------------------
    drcbex64_reset - reset back-end specific state
-------------------------------------------------*/

static void drcbex64_reset(drcbe_state *drcbe)
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
	emit_push_r64(dst, REG_RBX);														// push  rbx
	emit_mov_r32_imm(dst, REG_EAX, 1);													// mov   eax,1
	emit_cpuid(dst);																	// cpuid
	emit_mov_r32_r32(dst, REG_EAX, REG_ECX);											// mov   eax,ecx
	emit_pop_r64(dst, REG_RBX);															// pop   rbx
	emit_ret(dst);																		// ret

	/* call it to determine if we have SSE4.1 support */
	drcbe->sse41 = (((*cpuid_ecx_stub)() & 0x80000) != 0);

	/* generate an entry point */
	drcbe->entry = (x86_entry_point_func)*dst;
	emit_push_r64(dst, REG_RBX);														// push  rbx
	emit_push_r64(dst, REG_RSI);														// push  rsi
	emit_push_r64(dst, REG_RDI);														// push  rdi
	emit_push_r64(dst, REG_RBP);														// push  rbp
	emit_push_r64(dst, REG_R12);														// push  r12
	emit_push_r64(dst, REG_R13);														// push  r13
	emit_push_r64(dst, REG_R14);														// push  r14
	emit_push_r64(dst, REG_R15);														// push  r15
	emit_mov_r64_r64(dst, REG_RBP, REG_PARAM1);											// mov   rbp,param1
	emit_sub_r64_imm(dst, REG_RSP, 32);													// sub   rsp,32
	emit_mov_m64_r64(dst, MABS(drcbe, &drcbe->hashstacksave), REG_RSP);					// mov   [hashstacksave],rsp
	emit_sub_r64_imm(dst, REG_RSP, 8);													// sub   rsp,8
	emit_mov_m64_r64(dst, MABS(drcbe, &drcbe->stacksave), REG_RSP);						// mov   [stacksave],rsp
	emit_stmxcsr_m32(dst, MABS(drcbe, &drcbe->ssemode));								// stmxcsr [ssemode]
	emit_jmp_r64(dst, REG_PARAM2);														// jmp   param2
	if (drcbe->log != NULL)
		x86log_disasm_code_range(drcbe->log, "entry_point", (x86code *)drcbe->entry, *dst);

	/* generate an exit point */
	drcbe->exit = *dst;
	emit_ldmxcsr_m32(dst, MABS(drcbe, &drcbe->ssemode));								// ldmxcsr [ssemode]
	emit_mov_r64_m64(dst, REG_RSP, MABS(drcbe, &drcbe->hashstacksave));					// mov   rsp,[hashstacksave]
	emit_add_r64_imm(dst, REG_RSP, 32);													// add   rsp,32
	emit_pop_r64(dst, REG_R15);															// pop   r15
	emit_pop_r64(dst, REG_R14);															// pop   r14
	emit_pop_r64(dst, REG_R13);															// pop   r13
	emit_pop_r64(dst, REG_R12);															// pop   r12
	emit_pop_r64(dst, REG_RBP);															// pop   rbp
	emit_pop_r64(dst, REG_RDI);															// pop   rdi
	emit_pop_r64(dst, REG_RSI);															// pop   rsi
	emit_pop_r64(dst, REG_RBX);															// pop   rbx
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
    drcbex64_execute - execute a block of code
    referenced by the given handle
-------------------------------------------------*/

static int drcbex64_execute(drcbe_state *drcbe, drcuml_codehandle *entry)
{
	/* call our entry point which will jump to the destination */
	return (*drcbe->entry)(drcbe->rbpvalue, (x86code *)drcuml_handle_codeptr(entry));
}


/*-------------------------------------------------
    drcbex64_generate - generate code
-------------------------------------------------*/

static void drcbex64_generate(drcbe_state *drcbe, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst)
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
			char dasm[256];
			drcuml_disasm(inst, dasm, drcbe->drcuml);
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
    drcbex64_hash_exists - return true if the
    given mode/pc exists in the hash table
-------------------------------------------------*/

static int drcbex64_hash_exists(drcbe_state *drcbe, UINT32 mode, UINT32 pc)
{
	return drchash_code_exists(drcbe->hash, mode, pc);
}


/*-------------------------------------------------
    drcbex64_get_info - return information about
    the back-end implementation
-------------------------------------------------*/

static void drcbex64_get_info(drcbe_state *state, drcbe_info *info)
{
	for (info->direct_iregs = 0; info->direct_iregs < DRCUML_REG_I_END - DRCUML_REG_I0; info->direct_iregs++)
		if (int_register_map[info->direct_iregs] == 0)
			break;
	for (info->direct_fregs = 0; info->direct_fregs < DRCUML_REG_F_END - DRCUML_REG_F0; info->direct_fregs++)
		if (float_register_map[info->direct_fregs] == 0)
			break;
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

		/* memory passes through */
		case DRCUML_PTYPE_MEMORY:
			assert(allowed & PTYPE_M);
			dest->type = DRCUML_PTYPE_MEMORY;
			dest->value = (FPTR)src->value;
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

		/* if a register maps to a register, keep it as a register; otherwise map it to memory */
		case DRCUML_PTYPE_FLOAT_REGISTER:
			assert(allowed & PTYPE_F);
			assert(allowed & PTYPE_M);
			regnum = float_register_map[src->value - DRCUML_REG_F0];
			if (regnum != 0)
			{
				dest->type = DRCUML_PTYPE_FLOAT_REGISTER;
				dest->value = regnum;
			}
			else
			{
				dest->type = DRCUML_PTYPE_MEMORY;
				dest->value = (FPTR)&drcbe->state.f[src->value - DRCUML_REG_F0];
			}
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
		emit_mov_r32_m32(dst, reg, MABS(drcbe, param->value));							// mov   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_mov_r32_r32(dst, reg, param->value);									// mov   reg,param
	}
}


/*-------------------------------------------------
    emit_movsx_r64_p32 - move a 32-bit parameter
    sign-extended into a register
-------------------------------------------------*/

static void emit_movsx_r64_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (param->value == 0)
			emit_xor_r32_r32(dst, reg, reg);											// xor   reg,reg
		else if ((INT32)param->value >= 0)
			emit_mov_r32_imm(dst, reg, param->value);									// mov   reg,param
		else
			emit_mov_r64_imm(dst, reg, (INT32)param->value);							// mov   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_movsxd_r64_m32(dst, reg, MABS(drcbe, param->value));						// movsxd reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_movsxd_r64_r32(dst, reg, param->value);									// movsdx reg,param
}


/*-------------------------------------------------
    emit_mov_r32_p32_keepflags - move a 32-bit
    parameter into a register without affecting
    any flags
-------------------------------------------------*/

static void emit_mov_r32_p32_keepflags(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_mov_r32_imm(dst, reg, param->value);										// mov   reg,param
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_mov_r32_m32(dst, reg, MABS(drcbe, param->value));							// mov   reg,[param]
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
		emit_mov_r32_m32(dst, REG_EAX, MABS(drcbe, param->value));						// mov   eax,[param]
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
		emit_mov_m32_r32(dst, MABS(drcbe, param->value), reg);							// mov   [param],reg
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_mov_r32_r32(dst, param->value, reg);									// mov   param,reg
	}
}


/*-------------------------------------------------
    emit_add_r32_p32 - add operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_add_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != 0)
			emit_add_r32_imm(dst, reg, param->value);									// add   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_add_r32_m32(dst, reg, MABS(drcbe, param->value));							// add   reg,[param]
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
		if (inst->flags != 0 || param->value != 0)
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
		emit_adc_r32_imm(dst, reg, param->value);										// adc   reg,param
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_adc_r32_m32(dst, reg, MABS(drcbe, param->value));							// adc   reg,[param]
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
		emit_adc_m32_imm(dst, MEMPARAMS, param->value);									// adc   [dest],param
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32_keepflags(drcbe, dst, reg, param);								// mov   reg,param
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
		if (inst->flags != 0 || param->value != 0)
			emit_sub_r32_imm(dst, reg, param->value);									// sub   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_sub_r32_m32(dst, reg, MABS(drcbe, param->value));							// sub   reg,[param]
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
		if (inst->flags != 0 || param->value != 0)
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
		emit_sbb_r32_imm(dst, reg, param->value);										// sbb   reg,param
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_sbb_r32_m32(dst, reg, MABS(drcbe, param->value));							// sbb   reg,[param]
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
		emit_sbb_m32_imm(dst, MEMPARAMS, param->value);									// sbb   [dest],param
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r32_p32_keepflags(drcbe, dst, reg, param);								// mov   reg,param
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
		emit_cmp_r32_m32(dst, reg, MABS(drcbe, param->value));							// cmp   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_cmp_r32_r32(dst, reg, param->value);										// cmp   reg,param
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
    emit_and_r32_p32 - and operation to a 32-bit
    register from a 32-bit parameter
-------------------------------------------------*/

static void emit_and_r32_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0)
			emit_xor_r32_r32(dst, reg, reg);											// xor   reg,reg
		else
			emit_and_r32_imm(dst, reg, param->value);									// and   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_and_r32_m32(dst, reg, MABS(drcbe, param->value));							// and   reg,[param]
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
		if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0)
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
		emit_test_m32_r32(dst, MABS(drcbe, param->value), reg);							// test   [param],reg
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			emit_mov_r32_imm(dst, reg, -1);												// mov  reg,-1
		else
			emit_or_r32_imm(dst, reg, param->value);									// or   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_or_r32_m32(dst, reg, MABS(drcbe, param->value));							// or   reg,[param]
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			emit_not_r32(dst, reg);														// not   reg
		else
			emit_xor_r32_imm(dst, reg, param->value);									// xor   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_xor_r32_m32(dst, reg, MABS(drcbe, param->value));							// xor   reg,[param]
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcl_r32_imm(dst, reg, param->value);									// rcl   reg,param
	}
	else
	{
		emit_mov_r32_p32_keepflags(drcbe, dst, REG_ECX, param);							// mov   ecx,param
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcl_m32_imm(dst, MEMPARAMS, param->value);								// rcl   [dest],param
	}
	else
	{
		emit_mov_r32_p32_keepflags(drcbe, dst, REG_ECX, param);							// mov   ecx,param
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcr_r32_imm(dst, reg, param->value);									// rcr   reg,param
	}
	else
	{
		emit_mov_r32_p32_keepflags(drcbe, dst, REG_ECX, param);							// mov   ecx,param
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcr_m32_imm(dst, MEMPARAMS, param->value);								// rcr   [dest],param
	}
	else
	{
		emit_mov_r32_p32_keepflags(drcbe, dst, REG_ECX, param);							// mov   ecx,param
		emit_rcr_m32_cl(dst, MEMPARAMS);												// rcr   [dest],cl
	}
}



/***************************************************************************
    EMITTERS FOR 64-BIT OPERATIONS WITH PARAMETERS
***************************************************************************/

/*-------------------------------------------------
    emit_mov_r64_p64 - move a 64-bit parameter
    into a register
-------------------------------------------------*/

static void emit_mov_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (param->value == 0)
			emit_xor_r32_r32(dst, reg, reg);											// xor   reg,reg
		else
			emit_mov_r64_imm(dst, reg, param->value);									// mov   reg,param
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_mov_r64_m64(dst, reg, MABS(drcbe, param->value));							// mov   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_mov_r64_r64(dst, reg, param->value);									// mov   reg,param
	}
}


/*-------------------------------------------------
    emit_mov_r64_p64_keepflags - move a 64-bit
    parameter into a register without affecting
    any flags
-------------------------------------------------*/

static void emit_mov_r64_p64_keepflags(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_mov_r64_imm(dst, reg, param->value);										// mov   reg,param
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_mov_r64_m64(dst, reg, MABS(drcbe, param->value));							// mov   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_mov_r64_r64(dst, reg, param->value);									// mov   reg,param
	}
}


/*-------------------------------------------------
    emit_mov_p64_r64 - move a registers into a
    64-bit parameter
-------------------------------------------------*/

static void emit_mov_p64_r64(drcbe_state *drcbe, x86code **dst, const drcuml_parameter *param, UINT8 reg)
{
	assert(param->type != DRCUML_PTYPE_IMMEDIATE);
	if (param->type == DRCUML_PTYPE_MEMORY)
		emit_mov_m64_r64(dst, MABS(drcbe, param->value), reg);							// mov   [param],reg
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_mov_r64_r64(dst, param->value, reg);									// mov   param,reg
	}
}


/*-------------------------------------------------
    emit_add_r64_p64 - add operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_add_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != 0)
		{
			if (short_immediate(param->value))
				emit_add_r64_imm(dst, reg, param->value);								// add   reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_add_r64_r64(dst, reg, REG_R11);									// add   reg,r11
			}
		}
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_add_r64_m64(dst, reg, MABS(drcbe, param->value));							// add   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_add_r64_r64(dst, reg, param->value);										// add   reg,param
}


/*-------------------------------------------------
    emit_add_m64_p64 - add operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_add_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != 0)
		{
			if (short_immediate(param->value))
				emit_add_m64_imm(dst, MEMPARAMS, param->value);							// add   [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_add_m64_r64(dst, MEMPARAMS, REG_R11);								// add   [mem],r11
			}
		}
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r64_p64(drcbe, dst, reg, param);										// mov   reg,param
		emit_add_m64_r64(dst, MEMPARAMS, reg);											// add   [dest],reg
	}
}


/*-------------------------------------------------
    emit_adc_r64_p64 - adc operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_adc_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (short_immediate(param->value))
			emit_adc_r64_imm(dst, reg, param->value);									// adc   reg,param
		else
		{
			emit_mov_r64_imm(dst, REG_R11, param->value);								// mov   r11,param
			emit_adc_r64_r64(dst, reg, REG_R11);										// adc   reg,r11
		}
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_adc_r64_m64(dst, reg, MABS(drcbe, param->value));							// adc   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_adc_r64_r64(dst, reg, param->value);										// adc   reg,param
}


/*-------------------------------------------------
    emit_adc_m64_p64 - adc operation to a 64-bit
    memory locaiton from a 64-bit parameter
-------------------------------------------------*/

static void emit_adc_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE && short_immediate(param->value))
		emit_adc_m64_imm(dst, MEMPARAMS, param->value);									// adc   [mem],param
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r64_p64_keepflags(drcbe, dst, reg, param);								// mov   reg,param
		emit_adc_m64_r64(dst, MEMPARAMS, reg);											// adc   [dest],reg
	}
}


/*-------------------------------------------------
    emit_sub_r64_p64 - sub operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_sub_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != 0)
		{
			if (short_immediate(param->value))
				emit_sub_r64_imm(dst, reg, param->value);								// sub   reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_sub_r64_r64(dst, reg, REG_R11);									// sub   reg,r11
			}
		}
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_sub_r64_m64(dst, reg, MABS(drcbe, param->value));							// sub   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_sub_r64_r64(dst, reg, param->value);										// sub   reg,param
}


/*-------------------------------------------------
    emit_sub_m64_p64 - sub operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_sub_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != 0)
		{
			if (short_immediate(param->value))
				emit_sub_m64_imm(dst, MEMPARAMS, param->value);							// sub   [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_sub_m64_r64(dst, MEMPARAMS, REG_R11);								// sub   [mem],r11
			}
		}
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r64_p64(drcbe, dst, reg, param);										// mov   reg,param
		emit_sub_m64_r64(dst, MEMPARAMS, reg);											// sub   [dest],reg
	}
}


/*-------------------------------------------------
    emit_sbb_r64_p64 - sbb operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_sbb_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (short_immediate(param->value))
			emit_sbb_r64_imm(dst, reg, param->value);									// sbb   reg,param
		else
		{
			emit_mov_r64_imm(dst, REG_R11, param->value);								// mov   r11,param
			emit_sbb_r64_r64(dst, reg, REG_R11);										// sbb   reg,r11
		}
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_sbb_r64_m64(dst, reg, MABS(drcbe, param->value));							// sbb   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_sbb_r64_r64(dst, reg, param->value);										// sbb   reg,param
}


/*-------------------------------------------------
    emit_sbb_m64_p64 - sbb operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_sbb_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE && short_immediate(param->value))
		emit_sbb_m64_imm(dst, MEMPARAMS, param->value);									// sbb   [mem],param
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r64_p64_keepflags(drcbe, dst, reg, param);								// mov   reg,param
		emit_sbb_m64_r64(dst, MEMPARAMS, reg);											// sbb   [dest],reg
	}
}


/*-------------------------------------------------
    emit_cmp_r64_p64 - cmp operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_cmp_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (short_immediate(param->value))
			emit_cmp_r64_imm(dst, reg, param->value);									// cmp   reg,param
		else
		{
			emit_mov_r64_imm(dst, REG_R11, param->value);								// mov   r11,param
			emit_cmp_r64_r64(dst, reg, REG_R11);										// cmp   reg,r11
		}
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_cmp_r64_m64(dst, reg, MABS(drcbe, param->value));							// cmp   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_cmp_r64_r64(dst, reg, param->value);										// cmp   reg,param
}


/*-------------------------------------------------
    emit_cmp_m64_p64 - cmp operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_cmp_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE && short_immediate(param->value))
		emit_cmp_m64_imm(dst, MEMPARAMS, param->value);									// cmp   [dest],param
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r64_p64(drcbe, dst, reg, param);										// mov   reg,param
		emit_cmp_m64_r64(dst, MEMPARAMS, reg);											// cmp   [dest],reg
	}
}


/*-------------------------------------------------
    emit_and_r64_p64 - and operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_and_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != U64(0xffffffffffffffff))
		{
			if (short_immediate(param->value))
				emit_and_r64_imm(dst, reg, param->value);								// and   reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_and_r64_r64(dst, reg, REG_R11);									// and   reg,r11
			}
		}
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_and_r64_m64(dst, reg, MABS(drcbe, param->value));							// and   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_and_r64_r64(dst, reg, param->value);										// and   reg,param
}


/*-------------------------------------------------
    emit_and_m64_p64 - and operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_and_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != U64(0xffffffffffffffff))
		{
			if (short_immediate(param->value))
				emit_and_m64_imm(dst, MEMPARAMS, param->value);							// and   [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_and_m64_r64(dst, MEMPARAMS, REG_R11);								// and   [mem],r11
			}
		}
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r64_p64(drcbe, dst, reg, param);										// mov   reg,param
		emit_and_m64_r64(dst, MEMPARAMS, reg);											// and   [dest],reg
	}
}


/*-------------------------------------------------
    emit_test_r64_p64 - test operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_test_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (short_immediate(param->value))
			emit_test_r64_imm(dst, reg, param->value);									// test  reg,param
		else
		{
			emit_mov_r64_imm(dst, REG_R11, param->value);								// mov   r11,param
			emit_test_r64_r64(dst, reg, REG_R11);										// test  reg,r11
		}
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_test_m64_r64(dst, MABS(drcbe, param->value), reg);							// test  [param],reg
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_test_r64_r64(dst, reg, param->value);										// test  reg,param
}


/*-------------------------------------------------
    emit_test_m64_p64 - test operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_test_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE && short_immediate(param->value))
		emit_test_m64_imm(dst, MEMPARAMS, param->value);								// test  [dest],param
	else if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_mov_r64_p64(drcbe, dst, REG_EAX, param);									// mov   reg,param
		emit_test_m64_r64(dst, MEMPARAMS, REG_EAX);										// test  [dest],reg
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_test_m64_r64(dst, MEMPARAMS, param->value);								// test  [dest],param
}


/*-------------------------------------------------
    emit_or_r64_p64 - or operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_or_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != 0)
		{
			if (short_immediate(param->value))
				emit_or_r64_imm(dst, reg, param->value);								// or    reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_or_r64_r64(dst, reg, REG_R11);										// or    reg,r11
			}
		}
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_or_r64_m64(dst, reg, MABS(drcbe, param->value));							// or    reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_or_r64_r64(dst, reg, param->value);										// or    reg,param
}


/*-------------------------------------------------
    emit_or_m64_p64 - or operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_or_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != 0)
		{
			if (short_immediate(param->value))
				emit_or_m64_imm(dst, MEMPARAMS, param->value);							// or    [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_or_m64_r64(dst, MEMPARAMS, REG_R11);								// or    [mem],r11
			}
		}
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r64_p64(drcbe, dst, reg, param);										// mov   reg,param
		emit_or_m64_r64(dst, MEMPARAMS, reg);											// or    [dest],reg
	}
}


/*-------------------------------------------------
    emit_xor_r64_p64 - xor operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_xor_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != 0)
		{
			if (param->value == U64(0xffffffffffffffff))
				emit_not_r64(dst, reg);													// not   reg
			else if (short_immediate(param->value))
				emit_xor_r64_imm(dst, reg, param->value);								// xor   reg,param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_xor_r64_r64(dst, reg, REG_R11);									// xor   reg,r11
			}
		}
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
		emit_xor_r64_m64(dst, reg, MABS(drcbe, param->value));							// xor   reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
		emit_xor_r64_r64(dst, reg, param->value);										// xor   reg,param
}


/*-------------------------------------------------
    emit_xor_m64_p64 - xor operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_xor_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags != 0 || param->value != 0)
		{
			if (param->value == U64(0xffffffffffffffff))
				emit_not_m64(dst, MEMPARAMS);											// not   [mem]
			else if (short_immediate(param->value))
				emit_xor_m64_imm(dst, MEMPARAMS, param->value);							// xor   [mem],param
			else
			{
				emit_mov_r64_imm(dst, REG_R11, param->value);							// mov   r11,param
				emit_xor_m64_r64(dst, MEMPARAMS, REG_R11);								// xor   [mem],r11
			}
		}
	}
	else
	{
		int reg = param_select_register(REG_EAX, param, NULL);
		emit_mov_r64_p64(drcbe, dst, reg, param);										// mov   reg,param
		emit_xor_m64_r64(dst, MEMPARAMS, reg);											// xor   [dest],reg
	}
}


/*-------------------------------------------------
    emit_shl_r64_p64 - shl operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_shl_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_shl_r64_imm(dst, reg, param->value);									// shl   reg,param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_shl_r64_cl(dst, reg);														// shl   reg,cl
	}
}


/*-------------------------------------------------
    emit_shl_m64_p64 - shl operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_shl_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT64)param->value == 0)
			/* skip */;
		else
			emit_shl_m64_imm(dst, MEMPARAMS, param->value);								// shl   [dest],param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_shl_m64_cl(dst, MEMPARAMS);												// shl   [dest],cl
	}
}


/*-------------------------------------------------
    emit_shr_r64_p64 - shr operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_shr_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_shr_r64_imm(dst, reg, param->value);									// shr   reg,param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_shr_r64_cl(dst, reg);														// shr   reg,cl
	}
}


/*-------------------------------------------------
    emit_shr_m64_p64 - shr operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_shr_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT64)param->value == 0)
			/* skip */;
		else
			emit_shr_m64_imm(dst, MEMPARAMS, param->value);								// shr   [dest],param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_shr_m64_cl(dst, MEMPARAMS);												// shr   [dest],cl
	}
}


/*-------------------------------------------------
    emit_sar_r64_p64 - sar operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_sar_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_sar_r64_imm(dst, reg, param->value);									// sar   reg,param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_sar_r64_cl(dst, reg);														// sar   reg,cl
	}
}


/*-------------------------------------------------
    emit_sar_m64_p64 - sar operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_sar_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT64)param->value == 0)
			/* skip */;
		else
			emit_sar_m64_imm(dst, MEMPARAMS, param->value);								// sar   [dest],param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_sar_m64_cl(dst, MEMPARAMS);												// sar   [dest],cl
	}
}


/*-------------------------------------------------
    emit_rol_r64_p64 - rol operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_rol_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rol_r64_imm(dst, reg, param->value);									// rol   reg,param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_rol_r64_cl(dst, reg);														// rol   reg,cl
	}
}


/*-------------------------------------------------
    emit_rol_m64_p64 - rol operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_rol_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT64)param->value == 0)
			/* skip */;
		else
			emit_rol_m64_imm(dst, MEMPARAMS, param->value);								// rol   [dest],param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_rol_m64_cl(dst, MEMPARAMS);												// rol   [dest],cl
	}
}


/*-------------------------------------------------
    emit_ror_r64_p64 - ror operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_ror_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_ror_r64_imm(dst, reg, param->value);									// ror   reg,param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_ror_r64_cl(dst, reg);														// ror   reg,cl
	}
}


/*-------------------------------------------------
    emit_ror_m64_p64 - ror operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_ror_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT64)param->value == 0)
			/* skip */;
		else
			emit_ror_m64_imm(dst, MEMPARAMS, param->value);								// ror   [dest],param
	}
	else
	{
		emit_mov_r64_p64(drcbe, dst, REG_RCX, param);									// mov   rcx,param
		emit_ror_m64_cl(dst, MEMPARAMS);												// ror   [dest],cl
	}
}


/*-------------------------------------------------
    emit_rcl_r64_p64 - rcl operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_rcl_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcl_r64_imm(dst, reg, param->value);									// rcl   reg,param
	}
	else
	{
		emit_mov_r64_p64_keepflags(drcbe, dst, REG_RCX, param);							// mov   rcx,param
		emit_rcl_r64_cl(dst, reg);														// rcl   reg,cl
	}
}


/*-------------------------------------------------
    emit_rcl_m64_p64 - rcl operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_rcl_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT64)param->value == 0)
			/* skip */;
		else
			emit_rcl_m64_imm(dst, MEMPARAMS, param->value);								// rcl   [dest],param
	}
	else
	{
		emit_mov_r64_p64_keepflags(drcbe, dst, REG_RCX, param);							// mov   rcx,param
		emit_rcl_m64_cl(dst, MEMPARAMS);												// rcl   [dest],cl
	}
}


/*-------------------------------------------------
    emit_rcr_r64_p64 - rcr operation to a 64-bit
    register from a 64-bit parameter
-------------------------------------------------*/

static void emit_rcr_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else
			emit_rcr_r64_imm(dst, reg, param->value);									// rcr   reg,param
	}
	else
	{
		emit_mov_r64_p64_keepflags(drcbe, dst, REG_RCX, param);							// mov   rcx,param
		emit_rcr_r64_cl(dst, reg);														// rcr   reg,cl
	}
}


/*-------------------------------------------------
    emit_rcr_m64_p64 - rcr operation to a 64-bit
    memory location from a 64-bit parameter
-------------------------------------------------*/

static void emit_rcr_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT64)param->value == 0)
			/* skip */;
		else
			emit_rcr_m64_imm(dst, MEMPARAMS, param->value);								// rcr   [dest],param
	}
	else
	{
		emit_mov_r64_p64_keepflags(drcbe, dst, REG_RCX, param);							// mov   rcx,param
		emit_rcr_m64_cl(dst, MEMPARAMS);												// rcr   [dest],cl
	}
}



/***************************************************************************
    EMITTERS FOR FLOATING POINT OPERATIONS WITH PARAMETERS
***************************************************************************/

/*-------------------------------------------------
    emit_movss_r128_p32 - move a 32-bit parameter
    into a register
-------------------------------------------------*/

static void emit_movss_r128_p32(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param)
{
	assert(param->type != DRCUML_PTYPE_IMMEDIATE);
	if (param->type == DRCUML_PTYPE_MEMORY)
		emit_movss_r128_m32(dst, reg, MABS(drcbe, param->value));						// movss reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_movss_r128_r128(dst, reg, param->value);								// movss reg,param
	}
}


/*-------------------------------------------------
    emit_movss_p32_r128 - move a register into a
    32-bit parameter
-------------------------------------------------*/

static void emit_movss_p32_r128(drcbe_state *drcbe, x86code **dst, const drcuml_parameter *param, UINT8 reg)
{
	assert(param->type != DRCUML_PTYPE_IMMEDIATE);
	if (param->type == DRCUML_PTYPE_MEMORY)
		emit_movss_m32_r128(dst, MABS(drcbe, param->value), reg);						// movss [param],reg
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_movss_r128_r128(dst, param->value, reg);								// movss param,reg
	}
}


/*-------------------------------------------------
    emit_movsd_r128_p64 - move a 64-bit parameter
    into a register
-------------------------------------------------*/

static void emit_movsd_r128_p64(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param)
{
	assert(param->type != DRCUML_PTYPE_IMMEDIATE);
	if (param->type == DRCUML_PTYPE_MEMORY)
		emit_movsd_r128_m64(dst, reg, MABS(drcbe, param->value));						// movsd reg,[param]
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_movsd_r128_r128(dst, reg, param->value);								// movsd reg,param
	}
}


/*-------------------------------------------------
    emit_movsd_p64_r128 - move a register into a
    64-bit parameter
-------------------------------------------------*/

static void emit_movsd_p64_r128(drcbe_state *drcbe, x86code **dst, const drcuml_parameter *param, UINT8 reg)
{
	assert(param->type != DRCUML_PTYPE_IMMEDIATE);
	if (param->type == DRCUML_PTYPE_MEMORY)
		emit_movsd_m64_r128(dst, MABS(drcbe, param->value), reg);						// movsd [param],reg
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (reg != param->value)
			emit_movsd_r128_r128(dst, param->value, reg);								// movsd param,reg
	}
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
	drccodeptr src = (drccodeptr)parameter;

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
	drcbe_state *drcbe = (drcbe_state *)param1;
	drccodeptr src = (drccodeptr)param2;
	const drcuml_instruction *inst = (const drcuml_instruction *)param3;
	drccodeptr dst = *codeptr;
	drccodeptr *targetptr;

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &handp, PTYPE_M, &exp, PTYPE_MRI);

	/* look up the handle target */
	targetptr = drcuml_handle_codeptr_addr((drcuml_codehandle *)handp.value);

	/* first fixup the jump to get us here */
	((UINT32 *)src)[-1] = dst - src;

	/* then store the exception parameter */
	emit_mov_m32_p32(drcbe, &dst, MABS(drcbe, &drcbe->state.exp), &exp);				// mov   [exp],exp

	/* push the original return address on the stack */
	emit_lea_r64_m64(&dst, REG_RAX, MABS(drcbe, src));									// lea   rax,[return]
	emit_push_r64(&dst, REG_RAX);														// push  rax
	if (*targetptr != NULL)
		emit_jmp(&dst, *targetptr);														// jmp   *targetptr
	else
		emit_jmp_m64(&dst, MABS(drcbe, targetptr));										// jmp   [targetptr]

	*codeptr = dst;
}



/***************************************************************************
    DEBUG HELPERS
***************************************************************************/

/*-------------------------------------------------
    debug_log_hashjmp - callback to handle
    logging of hashjmps
-------------------------------------------------*/

static void debug_log_hashjmp(int mode, offs_t pc)
{
	printf("mode=%d PC=%08X\n", mode, pc);
}



/***************************************************************************
    COMPILE-TIME OPCODES
***************************************************************************/

/*-------------------------------------------------
    op_handle - process a HANDLE opcode
-------------------------------------------------*/

static x86code *op_handle(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	emit_link skip;

	assert_no_condition(inst);
	assert_no_flags(inst);
	assert(inst->numparams == 1);
	assert(inst->param[0].type == DRCUML_PTYPE_MEMORY);

	/* emit a jump around the stack adjust in case code falls through here */
	emit_jmp_short_link(&dst, &skip);													// jmp   skip

	/* register the current pointer for the handle */
	drcuml_handle_set_codeptr((drcuml_codehandle *)(FPTR)inst->param[0].value, dst);

	/* by default, the handle points to prolog code that moves the stack pointer */
	emit_lea_r64_m64(&dst, REG_RSP, MBD(REG_RSP, -40));									// lea   rsp,[rsp-40]
	resolve_link(&dst, &skip);														// skip:
	return dst;
}


/*-------------------------------------------------
    op_hash - process a HASH opcode
-------------------------------------------------*/

static x86code *op_hash(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	assert_no_condition(inst);
	assert_no_flags(inst);
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
	assert_no_condition(inst);
	assert_no_flags(inst);
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
	assert_no_condition(inst);
	assert_no_flags(inst);
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
	assert_no_condition(inst);
	assert_no_flags(inst);
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
    op_nop - process a NOP opcode
-------------------------------------------------*/

static x86code *op_nop(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	/* nothing */
	return dst;
}


/*-------------------------------------------------
    op_debug - process a DEBUG opcode
-------------------------------------------------*/

static x86code *op_debug(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	emit_link skip = { 0 };

	/* validate instruction */
	assert(inst->size == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	if ((drcbe->device->machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
	{
		drcuml_parameter pcp;

		/* normalize parameters */
		param_normalize_1(drcbe, inst, &pcp, PTYPE_MRI);

		/* test and branch */
		emit_mov_r64_imm(&dst, REG_RAX, (FPTR)&drcbe->device->machine->debug_flags);	// mov   rax,&debug_flags
		emit_test_m32_imm(&dst, MBD(REG_RAX, 0), DEBUG_FLAG_CALL_HOOK);					// test  [debug_flags],DEBUG_FLAG_CALL_HOOK
		emit_jcc_short_link(&dst, COND_Z, &skip);										// jz    skip

		/* push the parameter */
		emit_mov_r64_imm(&dst, REG_PARAM1, (FPTR)drcbe->device);						// mov   param1,device
		emit_mov_r32_p32(drcbe, &dst, REG_PARAM2, &pcp);								// mov   param2,pcp
		emit_smart_call_m64(drcbe, &dst, &drcbe->debug_cpu_instruction_hook);			// call  debug_cpu_instruction_hook

		resolve_link(&dst, &skip);													// skip:
	}

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
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &retp, PTYPE_MRI);

	/* load the parameter into EAX */
	emit_mov_r32_p32(drcbe, &dst, REG_EAX, &retp);										// mov   eax,retp
	if (inst->condition == DRCUML_COND_ALWAYS)
		emit_jmp(&dst, drcbe->exit);													// jmp   exit
	else
		emit_jcc(&dst, X86_CONDITION(inst->condition), drcbe->exit);					// jcc   exit

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
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &modep, PTYPE_MRI, &pcp, PTYPE_MRI, &exp, PTYPE_M);

	if (LOG_HASHJMPS)
	{
		emit_mov_r32_p32(drcbe, &dst, REG_PARAM1, &pcp);
		emit_mov_r32_p32(drcbe, &dst, REG_PARAM2, &modep);
		emit_smart_call_m64(drcbe, &dst, &drcbe->debug_log_hashjmp);
	}

	/* load the stack base one word early so we end up at the right spot after our call below */
	emit_mov_r64_m64(&dst, REG_RSP, MABS(drcbe, &drcbe->hashstacksave));				// mov   rsp,[hashstacksave]

	/* fixed mode cases */
	if (modep.type == DRCUML_PTYPE_IMMEDIATE && drcbe->hash->base[modep.value] != drcbe->hash->emptyl1)
	{
		/* a straight immediate jump is direct, though we need the PC in EAX in case of failure */
		if (pcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			UINT32 l1val = (pcp.value >> drcbe->hash->l1shift) & drcbe->hash->l1mask;
			UINT32 l2val = (pcp.value >> drcbe->hash->l2shift) & drcbe->hash->l2mask;
			emit_call_m64(&dst, MABS(drcbe, &drcbe->hash->base[modep.value][l1val][l2val]));
																						// call  hash[modep][l1val][l2val]
		}

		/* a fixed mode but variable PC */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, REG_EAX, &pcp);								// mov   eax,pcp
			emit_mov_r32_r32(&dst, REG_EDX, REG_EAX);									// mov   edx,eax
			emit_shr_r32_imm(&dst, REG_EDX, drcbe->hash->l1shift);						// shr   edx,l1shift
			emit_and_r32_imm(&dst, REG_EAX, drcbe->hash->l2mask << drcbe->hash->l2shift);// and  eax,l2mask << l2shift
			emit_mov_r64_m64(&dst, REG_RDX, MBISD(REG_RBP, REG_RDX, 8, offset_from_rbp(drcbe, (FPTR)&drcbe->hash->base[modep.value][0])));
																						// mov   rdx,hash[modep+edx*8]
			emit_call_m64(&dst, MBISD(REG_RDX, REG_RAX, 8 >> drcbe->hash->l2shift, 0));	// call  [rdx+rax*shift]
		}
	}
	else
	{
		/* variable mode */
		int modereg = param_select_register(REG_ECX, &modep, NULL);
		emit_mov_r32_p32(drcbe, &dst, modereg, &modep);									// mov   modereg,modep
		emit_mov_r64_m64(&dst, REG_RCX, MBISD(REG_RBP, modereg, 8, offset_from_rbp(drcbe, (FPTR)&drcbe->hash->base[0])));
																						// mov   rcx,hash[modereg*8]

		/* fixed PC */
		if (pcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			UINT32 l1val = (pcp.value >> drcbe->hash->l1shift) & drcbe->hash->l1mask;
			UINT32 l2val = (pcp.value >> drcbe->hash->l2shift) & drcbe->hash->l2mask;
			emit_mov_r64_m64(&dst, REG_RDX, MBD(REG_RCX, l1val*8));						// mov   rdx,[rcx+l1val*8]
			emit_call_m64(&dst, MBD(REG_RDX, l2val*8));									// call  [l2val*8]
		}

		/* variable PC */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, REG_EAX, &pcp);								// mov   eax,pcp
			emit_mov_r32_r32(&dst, REG_EDX, REG_EAX);									// mov   edx,eax
			emit_shr_r32_imm(&dst, REG_EDX, drcbe->hash->l1shift);						// shr   edx,l1shift
			emit_mov_r64_m64(&dst, REG_RDX, MBISD(REG_RCX, REG_RDX, 8, 0));				// mov   rdx,[rcx+rdx*8]
			emit_and_r32_imm(&dst, REG_EAX, drcbe->hash->l2mask << drcbe->hash->l2shift);// and  eax,l2mask << l2shift
			emit_call_m64(&dst, MBISD(REG_RDX, REG_RAX, 8 >> drcbe->hash->l2shift, 0));	// call  [rdx+rax*shift]
		}
	}

	/* in all cases, if there is no code, we return here to generate the exception */
	emit_mov_m32_p32(drcbe, &dst, MABS(drcbe, &drcbe->state.exp), &pcp);				// mov   [exp],param
	emit_sub_r64_imm(&dst, REG_RSP, 8);													// sub   rsp,8
	emit_call_m64(&dst, MABS(drcbe, exp.value));										// call  [exp]

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
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &labelp, PTYPE_I);

	/* look up the jump target and jump there */
	jmptarget = (x86code *)drclabel_get_codeptr(drcbe->labels, labelp.value, fixup_label, dst);
	if (jmptarget == NULL)
		jmptarget = dst + 0x7ffffff0;
	if (inst->condition == DRCUML_COND_ALWAYS)
		emit_jmp(&dst, jmptarget);														// jmp   target
	else
		emit_jcc(&dst, X86_CONDITION(inst->condition), jmptarget);						// jcc   target

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
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &handp, PTYPE_M, &exp, PTYPE_MRI);

	/* look up the handle target */
	targetptr = drcuml_handle_codeptr_addr((drcuml_codehandle *)handp.value);

	/* perform the exception processing inline if unconditional */
	if (inst->condition == DRCUML_COND_ALWAYS)
	{
		emit_mov_m32_p32(drcbe, &dst, MABS(drcbe, &drcbe->state.exp), &exp);			// mov   [exp],exp
		if (*targetptr != NULL)
			emit_call(&dst, *targetptr);												// call  *targetptr
		else
			emit_call_m64(&dst, MABS(drcbe, targetptr));								// call  [targetptr]
	}

	/* otherwise, jump to an out-of-band handler */
	else
	{
		emit_jcc(&dst, X86_CONDITION(inst->condition), dst + 0x7ffffff0);				// jcc   exception
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
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &handp, PTYPE_M);

	/* look up the handle target */
	targetptr = drcuml_handle_codeptr_addr((drcuml_codehandle *)handp.value);

	/* skip if conditional */
	if (inst->condition != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condition), &skip);			// jcc   skip

	/* jump through the handle; directly if a normal jump */
	if (*targetptr != NULL)
		emit_call(&dst, *targetptr);													// call  *targetptr
	else
		emit_call_m64(&dst, MABS(drcbe, targetptr));									// call  [targetptr]

	/* resolve the conditional link */
	if (inst->condition != DRCUML_COND_ALWAYS)
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
	assert_any_condition(inst);
	assert_no_flags(inst);
	assert(inst->numparams == 0);

	/* skip if conditional */
	if (inst->condition != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condition), &skip);			// jcc   skip

	/* return */
	emit_lea_r64_m64(&dst, REG_RSP, MBD(REG_RSP, 40));									// lea   rsp,[rsp+40]
	emit_ret(&dst);																		// ret

	/* resolve the conditional link */
	if (inst->condition != DRCUML_COND_ALWAYS)
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
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &funcp, PTYPE_M, &paramp, PTYPE_M);

	/* skip if conditional */
	if (inst->condition != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condition), &skip);			// jcc   skip

	/* perform the call */
	emit_mov_r64_imm(&dst, REG_PARAM1, paramp.value);									// mov   param1,paramp
	emit_smart_call_r64(drcbe, &dst, (x86code *)(FPTR)funcp.value, REG_RAX);			// call  funcp

	/* resolve the conditional link */
	if (inst->condition != DRCUML_COND_ALWAYS)
		resolve_link(&dst, &skip);													// skip:
	return dst;
}


/*-------------------------------------------------
    op_recover - process a RECOVER opcode
-------------------------------------------------*/

static x86code *op_recover(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;

	/* validate instruction */
	assert(inst->size == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize(drcbe, &inst->param[0], &dstp, PTYPE_MR);

	/* call the recovery code */
	emit_mov_r64_m64(&dst, REG_RAX, MABS(drcbe, &drcbe->stacksave));					// mov   rax,stacksave
	emit_mov_r64_m64(&dst, REG_RAX, MBD(REG_RAX, -8));									// mov   rax,[rax-4]
	emit_mov_r64_imm(&dst, REG_PARAM1, (FPTR)drcbe->map);								// mov   param1,drcbe->map
	emit_lea_r64_m64(&dst, REG_PARAM2, MBD(REG_RAX, -1));								// lea   param2,[rax-1]
	emit_mov_r64_imm(&dst, REG_PARAM3, inst->param[1].value);							// mov   param3,param[1].value
	emit_smart_call_m64(drcbe, &dst, &drcbe->drcmap_get_value);							// call  drcmap_get_value
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
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &srcp, PTYPE_MRI);

	/* immediate case */
	if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		srcp.value &= 3;
		emit_mov_m8_imm(&dst, MABS(drcbe, &drcbe->state.fmod), srcp.value);				// mov   [fmod],srcp
		emit_ldmxcsr_m32(&dst, MABS(drcbe, &drcbe->ssecontrol[srcp.value]));			// ldmxcsr fp_control[srcp]
	}

	/* register/memory case */
	else
	{
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &srcp);									// mov   eax,srcp
		emit_and_r32_imm(&dst, REG_EAX, 3);												// and   eax,3
		emit_mov_m8_r8(&dst, MABS(drcbe, &drcbe->state.fmod), REG_AL);					// mov   [fmod],al
		emit_ldmxcsr_m32(&dst, MBISD(REG_RBP, REG_RAX, 4, offset_from_rbp(drcbe, (FPTR)&drcbe->ssecontrol[0])));
																						// ldmxcsr fp_control[eax]
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
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_MR);

	/* fetch the current mode and store to the destination */
	if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		emit_movzx_r32_m8(&dst, dstp.value, MABS(drcbe, &drcbe->state.fmod));					// movzx reg,[fmod]
	else
	{
		emit_movzx_r32_m8(&dst, REG_EAX, MABS(drcbe, &drcbe->state.fmod));						// movzx eax,[fmod]
		emit_mov_m32_r32(&dst, MABS(drcbe, dstp.value), REG_EAX);								// mov   [dstp],eax
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
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_MR);

	/* fetch the exception parameter and store to the destination */
	if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe, &drcbe->state.exp));					// mov   reg,[exp]
	else
	{
		emit_mov_r32_m32(&dst, REG_EAX, MABS(drcbe, &drcbe->state.exp));						// mov   eax,[exp]
		emit_mov_m32_r32(&dst, MABS(drcbe, dstp.value), REG_EAX);								// mov   [dstp],eax
	}

	return dst;
}


/*-------------------------------------------------
    op_getflgs - process a GETFLGS opcode
-------------------------------------------------*/

static x86code *op_getflgs(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, maskp;
	UINT32 flagmask = 0;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &maskp, PTYPE_I);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* compute mask for flags */
	if (maskp.value & DRCUML_FLAG_C) flagmask |= 0x001;
	if (maskp.value & DRCUML_FLAG_V) flagmask |= 0x800;
	if (maskp.value & DRCUML_FLAG_Z) flagmask |= 0x040;
	if (maskp.value & DRCUML_FLAG_S) flagmask |= 0x080;
	if (maskp.value & DRCUML_FLAG_U) flagmask |= 0x004;

	switch (maskp.value)
	{
		/* single flags only */
		case DRCUML_FLAG_C:
			emit_setcc_r8(&dst, COND_C, REG_AL);										// setc   al
			emit_movzx_r32_r8(&dst, dstreg, REG_AL);									// movzx  dstreg,al
			break;

		case DRCUML_FLAG_V:
			emit_setcc_r8(&dst, COND_O, REG_AL);										// seto   al
			emit_movzx_r32_r8(&dst, dstreg, REG_AL);									// movzx  dstreg,al
			emit_shl_r32_imm(&dst, dstreg, 1);											// shl    dstreg,1
			break;

		case DRCUML_FLAG_Z:
			emit_setcc_r8(&dst, COND_Z, REG_AL);										// setz   al
			emit_movzx_r32_r8(&dst, dstreg, REG_AL);									// movzx  dstreg,al
			emit_shl_r32_imm(&dst, dstreg, 2);											// shl    dstreg,2
			break;

		case DRCUML_FLAG_S:
			emit_setcc_r8(&dst, COND_S, REG_AL);										// sets   al
			emit_movzx_r32_r8(&dst, dstreg, REG_AL);									// movzx  dstreg,al
			emit_shl_r32_imm(&dst, dstreg, 3);											// shl    dstreg,3
			break;

		case DRCUML_FLAG_U:
			emit_setcc_r8(&dst, COND_P, REG_AL);										// setp   al
			emit_movzx_r32_r8(&dst, dstreg, REG_AL);									// movzx  dstreg,al
			emit_shl_r32_imm(&dst, dstreg, 4);											// shl    dstreg,4
			break;

		/* carry plus another flag */
		case DRCUML_FLAG_C | DRCUML_FLAG_V:
			emit_setcc_r8(&dst, COND_C, REG_AL);										// setc   al
			emit_setcc_r8(&dst, COND_O, REG_CL);										// seto   cl
			emit_movzx_r32_r8(&dst, REG_EAX, REG_AL);									// movzx  eax,al
			emit_movzx_r32_r8(&dst, REG_ECX, REG_CL);									// movzx  ecx,al
			emit_lea_r32_m32(&dst, dstreg, MBISD(REG_EAX, REG_ECX, 2, 0));				// lea    dstreg,[eax+ecx*2]
			break;

		case DRCUML_FLAG_C | DRCUML_FLAG_Z:
			emit_setcc_r8(&dst, COND_C, REG_AL);										// setc   al
			emit_setcc_r8(&dst, COND_Z, REG_CL);										// setz   cl
			emit_movzx_r32_r8(&dst, REG_EAX, REG_AL);									// movzx  eax,al
			emit_movzx_r32_r8(&dst, REG_ECX, REG_CL);									// movzx  ecx,al
			emit_lea_r32_m32(&dst, dstreg, MBISD(REG_EAX, REG_ECX, 4, 0));				// lea    dstreg,[eax+ecx*4]
			break;

		case DRCUML_FLAG_C | DRCUML_FLAG_S:
			emit_setcc_r8(&dst, COND_C, REG_AL);										// setc   al
			emit_setcc_r8(&dst, COND_S, REG_CL);										// sets   cl
			emit_movzx_r32_r8(&dst, REG_EAX, REG_AL);									// movzx  eax,al
			emit_movzx_r32_r8(&dst, REG_ECX, REG_CL);									// movzx  ecx,al
			emit_lea_r32_m32(&dst, dstreg, MBISD(REG_EAX, REG_ECX, 8, 0));				// lea    dstreg,[eax+ecx*8]
			break;

		/* overflow plus another flag */
		case DRCUML_FLAG_V | DRCUML_FLAG_Z:
			emit_setcc_r8(&dst, COND_O, REG_AL);										// seto   al
			emit_setcc_r8(&dst, COND_Z, REG_CL);										// setz   cl
			emit_movzx_r32_r8(&dst, REG_EAX, REG_AL);									// movzx  eax,al
			emit_movzx_r32_r8(&dst, REG_ECX, REG_CL);									// movzx  ecx,al
			emit_lea_r32_m32(&dst, dstreg, MBISD(REG_EAX, REG_ECX, 2, 0));				// lea    dstreg,[eax+ecx*2]
			emit_shl_r32_imm(&dst, dstreg, 1);											// shl    dstreg,1
			break;

		case DRCUML_FLAG_V | DRCUML_FLAG_S:
			emit_setcc_r8(&dst, COND_O, REG_AL);										// seto   al
			emit_setcc_r8(&dst, COND_S, REG_CL);										// sets   cl
			emit_movzx_r32_r8(&dst, REG_EAX, REG_AL);									// movzx  eax,al
			emit_movzx_r32_r8(&dst, REG_ECX, REG_CL);									// movzx  ecx,al
			emit_lea_r32_m32(&dst, dstreg, MBISD(REG_EAX, REG_ECX, 4, 0));				// lea    dstreg,[eax+ecx*4]
			emit_shl_r32_imm(&dst, dstreg, 1);											// shl    dstreg,1
			break;

		/* zero plus another flag */
		case DRCUML_FLAG_Z | DRCUML_FLAG_S:
			emit_setcc_r8(&dst, COND_Z, REG_AL);										// setz   al
			emit_setcc_r8(&dst, COND_S, REG_CL);										// sets   cl
			emit_movzx_r32_r8(&dst, REG_EAX, REG_AL);									// movzx  eax,al
			emit_movzx_r32_r8(&dst, REG_ECX, REG_CL);									// movzx  ecx,al
			emit_lea_r32_m32(&dst, dstreg, MBISD(REG_EAX, REG_ECX, 2, 0));				// lea    dstreg,[eax+ecx*2]
			emit_shl_r32_imm(&dst, dstreg, 2);											// shl    dstreg,2
			break;

		/* default cases */
		default:
			emit_pushf(&dst);															// pushf
			emit_pop_r64(&dst, REG_EAX);												// pop    eax
			emit_and_r32_imm(&dst, REG_EAX, flagmask);									// and    eax,flagmask
			emit_movzx_r32_m8(&dst, dstreg, MBISD(REG_RBP, REG_RAX, 1, offset_from_rbp(drcbe, (FPTR)&drcbe->flagsmap[0])));
																						// movzx  dstreg,[flags_map]
			break;
	}

	/* 32-bit form */
	if (inst->size == 4)
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

	/* 64-bit form */
	else if (inst->size == 8)
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

	return dst;
}


/*-------------------------------------------------
    op_save - process a SAVE opcode
-------------------------------------------------*/

static x86code *op_save(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;
	int regoffs;
	int regnum;

	/* validate instruction */
	assert(inst->size == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_M);

	/* copy live state to the destination */
	emit_mov_r64_imm(&dst, REG_RCX, dstp.value);										// mov    rcx,dstp

	/* copy flags */
	emit_pushf(&dst);																	// pushf
	emit_pop_r64(&dst, REG_RAX);														// pop    rax
	emit_and_r32_imm(&dst, REG_EAX, 0x8c5);												// and    eax,0x8c5
	emit_mov_r8_m8(&dst, REG_AL, MBISD(REG_RBP, REG_RAX, 1, offset_from_rbp(drcbe, (FPTR)&drcbe->flagsmap[0])));
																						// mov    al,[flags_map]
	emit_mov_m8_r8(&dst, MBD(REG_RCX, offsetof(drcuml_machine_state, flags)), REG_AL);	// mov    state->flags,al

	/* copy fmod and exp */
	emit_mov_r8_m8(&dst, REG_AL, MABS(drcbe, &drcbe->state.fmod));						// mov    al,[fmod]
	emit_mov_m8_r8(&dst, MBD(REG_RCX, offsetof(drcuml_machine_state, fmod)), REG_AL);	// mov    state->fmod,al
	emit_mov_r32_m32(&dst, REG_EAX, MABS(drcbe, &drcbe->state.exp));					// mov    eax,[exp]
	emit_mov_m32_r32(&dst, MBD(REG_RCX, offsetof(drcuml_machine_state, exp)), REG_EAX);	// mov    state->exp,eax

	/* copy integer registers */
	regoffs = offsetof(drcuml_machine_state, r);
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
			emit_mov_m64_r64(&dst, MBD(REG_RCX, regoffs + 8 * regnum), int_register_map[regnum]);
		else
		{
			emit_mov_r64_m64(&dst, REG_RAX, MABS(drcbe, &drcbe->state.r[regnum].d));
			emit_mov_m64_r64(&dst, MBD(REG_RCX, regoffs + 8 * regnum), REG_RAX);
		}
	}

	/* copy FP registers */
	regoffs = offsetof(drcuml_machine_state, f);
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
			emit_movsd_m64_r128(&dst, MBD(REG_RCX, regoffs + 8 * regnum), float_register_map[regnum]);
		else
		{
			emit_mov_r64_m64(&dst, REG_RAX, MABS(drcbe, &drcbe->state.f[regnum].d));
			emit_mov_m64_r64(&dst, MBD(REG_RCX, regoffs + 8 * regnum), REG_RAX);
		}
	}

	return dst;
}


/*-------------------------------------------------
    op_restore - process a RESTORE opcode
-------------------------------------------------*/

static x86code *op_restore(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;
	int regoffs;
	int regnum;

	/* validate instruction */
	assert(inst->size == 4);
	assert_no_condition(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_M);

	/* copy live state from the destination */
	emit_mov_r64_imm(&dst, REG_ECX, dstp.value);										// mov    rcx,dstp

	/* copy integer registers */
	regoffs = offsetof(drcuml_machine_state, r);
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.r); regnum++)
	{
		if (int_register_map[regnum] != 0)
			emit_mov_r64_m64(&dst, int_register_map[regnum], MBD(REG_RCX, regoffs + 8 * regnum));
		else
		{
			emit_mov_r64_m64(&dst, REG_RAX, MBD(REG_RCX, regoffs + 8 * regnum));
			emit_mov_m64_r64(&dst, MABS(drcbe, &drcbe->state.r[regnum].d), REG_RAX);
		}
	}

	/* copy FP registers */
	regoffs = offsetof(drcuml_machine_state, f);
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.f); regnum++)
	{
		if (float_register_map[regnum] != 0)
			emit_movsd_r128_m64(&dst, float_register_map[regnum], MBD(REG_RCX, regoffs + 8 * regnum));
		else
		{
			emit_mov_r64_m64(&dst, REG_RAX, MBD(REG_RCX, regoffs + 8 * regnum));
			emit_mov_m64_r64(&dst, MABS(drcbe, &drcbe->state.f[regnum].d), REG_RAX);
		}
	}

	/* copy fmod and exp */
	emit_movzx_r32_m8(&dst, REG_EAX, MBD(REG_RCX, offsetof(drcuml_machine_state, fmod)));// movzx eax,state->fmod
	emit_and_r32_imm(&dst, REG_EAX, 3);													// and   eax,3
	emit_mov_m8_r8(&dst, MABS(drcbe, &drcbe->state.fmod), REG_AL);						// mov   [fmod],al
	emit_ldmxcsr_m32(&dst, MBISD(REG_RBP, REG_RAX, 4, offset_from_rbp(drcbe, (FPTR)&drcbe->ssecontrol[0])));
	emit_mov_r32_m32(&dst, REG_EAX, MBD(REG_RCX, offsetof(drcuml_machine_state, exp)));	// mov    eax,state->exp
	emit_mov_m32_r32(&dst, MABS(drcbe, &drcbe->state.exp), REG_EAX);					// mov    [exp],eax

	/* copy flags */
	emit_movzx_r32_m8(&dst, REG_EAX, MBD(REG_RCX, offsetof(drcuml_machine_state, flags)));// movzx eax,state->flags
	emit_push_m64(&dst, MBISD(REG_RBP, REG_RAX, 8, offset_from_rbp(drcbe, (FPTR)&drcbe->flagsunmap[0])));
																						// push   flags_unmap[eax*8]
	emit_popf(&dst);																	// popf

	return dst;
}



/***************************************************************************
    INTEGER OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    op_load - process a LOAD opcode
-------------------------------------------------*/

static x86code *op_load(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp, scalesizep;
	int basereg, dstreg, scale, size;
	INT32 baseoffs;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI, &scalesizep, PTYPE_I);
	scale = 1 << (scalesizep.value / 16);
	size = scalesizep.value % 16;

	/* determine the pointer base */
	basereg = get_base_register_and_offset(drcbe, &dst, basep.value, REG_RDX, &baseoffs);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (size == DRCUML_SIZE_BYTE)
			emit_movzx_r32_m8(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));	// movzx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_WORD)
			emit_movzx_r32_m16(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));// movzx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_DWORD)
			emit_mov_r32_m32(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));	// mov   dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_QWORD)
			emit_mov_r64_m64(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));	// mov   dstreg,[basep + scale*indp]
	}

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_movsx_r64_p32(drcbe, &dst, indreg, &indp);
		if (size == DRCUML_SIZE_BYTE)
			emit_movzx_r32_m8(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// movzx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_WORD)
			emit_movzx_r32_m16(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// movzx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_DWORD)
			emit_mov_r32_m32(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// mov   dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_QWORD)
			emit_mov_r64_m64(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// mov   dstreg,[basep + scale*indp]
	}

	/* store result */
	if (inst->size == 4)
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	else
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

	return dst;
}


/*-------------------------------------------------
    op_loads - process a LOADS opcode
-------------------------------------------------*/

static x86code *op_loads(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp, scalesizep;
	int basereg, dstreg, scale, size;
	INT32 baseoffs;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI, &scalesizep, PTYPE_I);
	scale = 1 << (scalesizep.value / 16);
	size = scalesizep.value % 16;

	/* determine the pointer base */
	basereg = get_base_register_and_offset(drcbe, &dst, basep.value, REG_RDX, &baseoffs);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->size == 4)
		{
			if (size == DRCUML_SIZE_BYTE)
				emit_movsx_r32_m8(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));	// movsx dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_WORD)
				emit_movsx_r32_m16(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));// movsx dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_r32_m32(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));	// mov   dstreg,[basep + scale*indp]
		}
		else if (inst->size == 8)
		{
			if (size == DRCUML_SIZE_BYTE)
				emit_movsx_r64_m8(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));	// movzx dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_WORD)
				emit_movsx_r64_m16(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));// movzx dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_DWORD)
				emit_movsxd_r64_m32(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));// movsxd dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_QWORD)
				emit_mov_r64_m64(&dst, dstreg, MBD(basereg, baseoffs + scale*indp.value));	// mov   dstreg,[basep + scale*indp]
		}
	}

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_movsx_r64_p32(drcbe, &dst, indreg, &indp);
		if (inst->size == 4)
		{
			if (size == DRCUML_SIZE_BYTE)
				emit_movsx_r32_m8(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// movsx dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_WORD)
				emit_movsx_r32_m16(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// movsx dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_r32_m32(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// mov   dstreg,[basep + scale*indp]
		}
		else if (inst->size == 8)
		{
			if (size == DRCUML_SIZE_BYTE)
				emit_movsx_r64_m8(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// movsx dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_WORD)
				emit_movsx_r64_m16(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// movsx dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_DWORD)
				emit_movsxd_r64_m32(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// movsxd dstreg,[basep + scale*indp]
			else if (size == DRCUML_SIZE_QWORD)
				emit_mov_r64_m64(&dst, dstreg, MBISD(basereg, indreg, scale, baseoffs));	// mov   dstreg,[basep + scale*indp]
		}
	}

	/* store result */
	if (inst->size == 4)
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	else
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

	return dst;
}


/*-------------------------------------------------
    op_store - process a STORE opcode
-------------------------------------------------*/

static x86code *op_store(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, basep, indp, scalesizep;
	int srcreg, basereg, scale, size;
	INT32 baseoffs;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &basep, PTYPE_M, &indp, PTYPE_MRI, &srcp, PTYPE_MRI, &scalesizep, PTYPE_I);
	scale = 1 << (scalesizep.value / 16);
	size = scalesizep.value % 16;

	/* determine the pointer base */
	basereg = get_base_register_and_offset(drcbe, &dst, basep.value, REG_RDX, &baseoffs);

	/* pick a source register for the general case */
	srcreg = param_select_register(REG_EAX, &srcp, NULL);

	/* degenerate case: constant index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		/* immediate source */
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (size == DRCUML_SIZE_BYTE)
				emit_mov_m8_imm(&dst, MBD(basereg, baseoffs + scale*indp.value), srcp.value);	// mov   [basep + scale*indp],srcp
			else if (size == DRCUML_SIZE_WORD)
				emit_mov_m16_imm(&dst, MBD(basereg, baseoffs + scale*indp.value), srcp.value);	// mov   [basep + scale*indp],srcp
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_m32_imm(&dst, MBD(basereg, baseoffs + scale*indp.value), srcp.value);	// mov   [basep + scale*indp],srcp
			else if (size == DRCUML_SIZE_QWORD)
			{
				if (short_immediate(srcp.value))
					emit_mov_m64_imm(&dst, MBD(basereg, baseoffs + scale*indp.value), srcp.value);// mov   [basep + scale*indp],srcp
				else
				{
					emit_mov_m32_imm(&dst, MBD(basereg, baseoffs + scale*indp.value), srcp.value);// mov   [basep + scale*indp],srcp
					emit_mov_m32_imm(&dst, MBD(basereg, baseoffs + scale*indp.value + 4), srcp.value >> 32);
																						// mov   [basep + scale*indp + 4],srcp >> 32
				}
			}
		}

		/* variable source */
		else
		{
			if (size != DRCUML_SIZE_QWORD)
				emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);							// mov   srcreg,srcp
			else
				emit_mov_r64_p64(drcbe, &dst, srcreg, &srcp);							// mov   srcreg,srcp
			if (size == DRCUML_SIZE_BYTE)
				emit_mov_m8_r8(&dst, MBD(basereg, baseoffs + scale*indp.value), srcreg);	// mov   [basep + scale*indp],srcreg
			else if (size == DRCUML_SIZE_WORD)
				emit_mov_m16_r16(&dst, MBD(basereg, baseoffs + scale*indp.value), srcreg);	// mov   [basep + scale*indp],srcreg
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_m32_r32(&dst, MBD(basereg, baseoffs + scale*indp.value), srcreg);	// mov   [basep + scale*indp],srcreg
			else if (size == DRCUML_SIZE_QWORD)
				emit_mov_m64_r64(&dst, MBD(basereg, baseoffs + scale*indp.value), srcreg);	// mov   [basep + scale*indp],srcreg
		}
	}

	/* normal case: variable index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_movsx_r64_p32(drcbe, &dst, indreg, &indp);									// mov   indreg,indp

		/* immediate source */
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (size == DRCUML_SIZE_BYTE)
				emit_mov_m8_imm(&dst, MBISD(basereg, indreg, scale, baseoffs), srcp.value);	// mov   [basep + scale*ecx],srcp
			else if (size == DRCUML_SIZE_WORD)
				emit_mov_m16_imm(&dst, MBISD(basereg, indreg, scale, baseoffs), srcp.value);// mov   [basep + scale*ecx],srcp
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_m32_imm(&dst, MBISD(basereg, indreg, scale, baseoffs), srcp.value);// mov   [basep + scale*ecx],srcp
			else if (size == DRCUML_SIZE_QWORD)
			{
				if (short_immediate(srcp.value))
					emit_mov_m64_imm(&dst, MBISD(basereg, indreg, scale, baseoffs), srcp.value);// mov   [basep + scale*indp],srcp
				else
				{
					emit_mov_m32_imm(&dst, MBISD(basereg, indreg, scale, baseoffs), srcp.value);// mov   [basep + scale*ecx],srcp
					emit_mov_m32_imm(&dst, MBISD(basereg, indreg, scale, baseoffs + 4), srcp.value >> 32);
																						// mov   [basep + scale*ecx + 4],srcp >> 32
				}
			}
		}

		/* variable source */
		else
		{
			if (size != DRCUML_SIZE_QWORD)
				emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);							// mov   srcreg,srcp
			else
				emit_mov_r64_p64(drcbe, &dst, srcreg, &srcp);							// mov   edx:srcreg,srcp
			if (size == DRCUML_SIZE_BYTE)
				emit_mov_m8_r8(&dst, MBISD(basereg, indreg, scale, baseoffs), srcreg);	// mov   [basep + scale*ecx],srcreg
			else if (size == DRCUML_SIZE_WORD)
				emit_mov_m16_r16(&dst, MBISD(basereg, indreg, scale, baseoffs), srcreg);// mov   [basep + scale*ecx],srcreg
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_m32_r32(&dst, MBISD(basereg, indreg, scale, baseoffs), srcreg);// mov   [basep + scale*ecx],srcreg
			else if (size == DRCUML_SIZE_QWORD)
				emit_mov_m64_r64(&dst, MBISD(basereg, indreg, scale, baseoffs), srcreg);// mov   [basep + scale*ecx],srcreg
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_read - process a READ opcode
-------------------------------------------------*/

static x86code *op_read(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, addrp, spacesizep;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &addrp, PTYPE_MRI, &spacesizep, PTYPE_I);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* set up a call to the read byte handler */
	emit_mov_r64_imm(&dst, REG_PARAM1, (FPTR)(drcbe->space[spacesizep.value / 16]));		// mov    param1,space
	emit_mov_r32_p32(drcbe, &dst, REG_PARAM2, &addrp);									// mov    param2,addrp
	if ((spacesizep.value & 3) == DRCUML_SIZE_BYTE)
	{
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].read_byte);
																						// call   read_byte
		emit_movzx_r32_r8(&dst, dstreg, REG_AL);										// movzx  dstreg,al
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_WORD)
	{
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].read_word);
																						// call   read_word
		emit_movzx_r32_r16(&dst, dstreg, REG_AX);										// movzx  dstreg,ax
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_DWORD)
	{
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].read_dword);
																						// call   read_dword
		if (dstreg != REG_EAX || inst->size == 8)
			emit_mov_r32_r32(&dst, dstreg, REG_EAX);									// mov    dstreg,eax
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_QWORD)
	{
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].read_qword);
																						// call   read_qword
		if (dstreg != REG_RAX)
			emit_mov_r64_r64(&dst, dstreg, REG_RAX);									// mov    dstreg,rax
	}

	/* store result */
	if (inst->size == 4)
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	else
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

	return dst;
}


/*-------------------------------------------------
    op_readm - process a READM opcode
-------------------------------------------------*/

static x86code *op_readm(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, addrp, maskp, spacesizep;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &addrp, PTYPE_MRI, &maskp, PTYPE_MRI, &spacesizep, PTYPE_I);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* set up a call to the read byte handler */
	emit_mov_r64_imm(&dst, REG_PARAM1, (FPTR)(drcbe->space[spacesizep.value / 16]));		// mov    param1,space
	emit_mov_r32_p32(drcbe, &dst, REG_PARAM2, &addrp);									// mov    param2,addrp
	if ((spacesizep.value & 3) != DRCUML_SIZE_QWORD)
		emit_mov_r32_p32(drcbe, &dst, REG_PARAM3, &maskp);								// mov    param3,maskp
	else
		emit_mov_r64_p64(drcbe, &dst, REG_PARAM3, &maskp);								// mov    param3,maskp
	if ((spacesizep.value & 3) == DRCUML_SIZE_WORD)
	{
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].read_word_masked);
																						// call   read_word_masked
		emit_movzx_r32_r16(&dst, dstreg, REG_AX);										// movzx  dstreg,ax
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_DWORD)
	{
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].read_dword_masked);
																						// call   read_dword_masked
		if (dstreg != REG_EAX || inst->size == 8)
			emit_mov_r32_r32(&dst, dstreg, REG_EAX);									// mov    dstreg,eax
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_QWORD)
	{
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].read_qword_masked);
																						// call   read_qword_masked
		if (dstreg != REG_RAX)
			emit_mov_r64_r64(&dst, dstreg, REG_RAX);									// mov    dstreg,rax
	}

	/* store result */
	if (inst->size == 4)
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	else
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

	return dst;
}


/*-------------------------------------------------
    op_write - process a WRITE opcode
-------------------------------------------------*/

static x86code *op_write(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter addrp, srcp, spacesizep;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &addrp, PTYPE_MRI, &srcp, PTYPE_MRI, &spacesizep, PTYPE_I);

	/* set up a call to the write byte handler */
	emit_mov_r64_imm(&dst, REG_PARAM1, (FPTR)(drcbe->space[spacesizep.value / 16]));		// mov    param1,space
	emit_mov_r32_p32(drcbe, &dst, REG_PARAM2, &addrp);									// mov    param2,addrp
	if ((spacesizep.value & 3) != DRCUML_SIZE_QWORD)
		emit_mov_r32_p32(drcbe, &dst, REG_PARAM3, &srcp);								// mov    param3,srcp
	else
		emit_mov_r64_p64(drcbe, &dst, REG_PARAM3, &srcp);								// mov    param3,srcp
	if ((spacesizep.value & 3) == DRCUML_SIZE_BYTE)
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].write_byte);
																						// call   write_byte
	else if ((spacesizep.value & 3) == DRCUML_SIZE_WORD)
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].write_word);
																						// call   write_word
	else if ((spacesizep.value & 3) == DRCUML_SIZE_DWORD)
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].write_dword);
																						// call   write_dword
	else if ((spacesizep.value & 3) == DRCUML_SIZE_QWORD)
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].write_qword);
																						// call   write_qword
	return dst;
}


/*-------------------------------------------------
    op_writem - process a WRITEM opcode
-------------------------------------------------*/

static x86code *op_writem(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter addrp, srcp, maskp, spacesizep;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &addrp, PTYPE_MRI, &srcp, PTYPE_MRI, &maskp, PTYPE_MRI, &spacesizep, PTYPE_I);

	/* set up a call to the write byte handler */
	emit_mov_r64_imm(&dst, REG_PARAM1, (FPTR)(drcbe->space[spacesizep.value / 16]));		// mov    param1,space
	emit_mov_r32_p32(drcbe, &dst, REG_PARAM2, &addrp);									// mov    param2,addrp
	if ((spacesizep.value & 3) != DRCUML_SIZE_QWORD)
	{
		emit_mov_r32_p32(drcbe, &dst, REG_PARAM3, &srcp);								// mov    param3,srcp
		emit_mov_r32_p32(drcbe, &dst, REG_PARAM4, &maskp);								// mov    param4,maskp
	}
	else
	{
		emit_mov_r64_p64(drcbe, &dst, REG_PARAM3, &srcp);								// mov    param3,srcp
		emit_mov_r64_p64(drcbe, &dst, REG_PARAM4, &maskp);								// mov    param4,maskp
	}
	if ((spacesizep.value & 3) == DRCUML_SIZE_WORD)
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].write_word_masked);
																						// call   write_word_masked
	else if ((spacesizep.value & 3) == DRCUML_SIZE_DWORD)
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].write_dword_masked);
																						// call   write_dword_masked
	else if ((spacesizep.value & 3) == DRCUML_SIZE_QWORD)
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacesizep.value / 16].write_qword_masked);
																						// call   write_qword_masked
	return dst;
}


/*-------------------------------------------------
    op_carry - process a CARRY opcode
-------------------------------------------------*/

static x86code *op_carry(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, bitp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C);

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
				emit_bt_m32_imm(&dst, MABS(drcbe, srcp.value), bitp.value);				// bt     [srcp],bitp
			else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_bt_r32_imm(&dst, srcp.value, bitp.value);							// bt     srcp,bitp
		}
		else
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_bt_m32_r32(&dst, MABS(drcbe, srcp.value), REG_ECX);				// bt     [srcp],ecx
			else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_bt_r32_r32(&dst, srcp.value, REG_ECX);								// bt     srcp,ecx
		}
	}

	/* 64-bit form */
	else
	{
		if (bitp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_bt_m64_imm(&dst, MABS(drcbe, srcp.value), bitp.value);				// bt     [srcp],bitp
			else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_bt_r64_imm(&dst, srcp.value, bitp.value);							// bt     srcp,bitp
		}
		else
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_bt_m64_r64(&dst, MABS(drcbe, srcp.value), REG_ECX);				// bt     [srcp],ecx
			else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_bt_r64_r64(&dst, srcp.value, REG_ECX);								// bt     srcp,ecx
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_set - process a SET opcode
-------------------------------------------------*/

static x86code *op_set(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_MR);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* set to AL */
	emit_setcc_r8(&dst, X86_CONDITION(inst->condition), REG_AL);						// setcc  al
	emit_movzx_r32_r8(&dst, dstreg, REG_AL);											// movzx  dstreg,al

	/* 32-bit form */
	if (inst->size == 4)
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

	/* 64-bit form */
	else if (inst->size == 8)
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

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
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters, but only if we got here directly */
	/* other opcodes call through here with pre-normalized parameters */
	if (inst->opcode == DRCUML_OP_MOV)
		param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);
	else
	{
		dstp = inst->param[0];
		srcp = inst->param[1];
	}

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* always start with a jmp */
	if (inst->condition != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condition), &skip);			// jcc   skip

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* register to memory */
		if (dstp.type == DRCUML_PTYPE_MEMORY && srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_r32(&dst, MABS(drcbe, dstp.value), srcp.value);				// mov   [dstp],srcp

		/* immediate to memory */
		else if (dstp.type == DRCUML_PTYPE_MEMORY && srcp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_mov_m32_imm(&dst, MABS(drcbe, dstp.value), srcp.value);				// mov   [dstp],srcp

		/* conditional memory to register */
		else if (inst->condition != 0 && dstp.type == DRCUML_PTYPE_INT_REGISTER && srcp.type == DRCUML_PTYPE_MEMORY)
		{
			dst = savedst;
			skip.target = NULL;
			emit_cmovcc_r32_m32(&dst, X86_CONDITION(inst->condition), dstp.value, MABS(drcbe, srcp.value));
																						// cmovcc dstp,[srcp]
		}

		/* conditional register to register */
		else if (inst->condition != 0 && dstp.type == DRCUML_PTYPE_INT_REGISTER && srcp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			dst = savedst;
			skip.target = NULL;
			emit_cmovcc_r32_r32(&dst, X86_CONDITION(inst->condition), dstp.value, srcp.value);
																						// cmovcc dstp,srcp
		}

		/* general case */
		else
		{
			emit_mov_r32_p32_keepflags(drcbe, &dst, dstreg, &srcp);						// mov   dstreg,srcp
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* register to memory */
		if (dstp.type == DRCUML_PTYPE_MEMORY && srcp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m64_r64(&dst, MABS(drcbe, dstp.value), srcp.value);				// mov   [dstp],srcp

		/* immediate to memory */
		else if (dstp.type == DRCUML_PTYPE_MEMORY && srcp.type == DRCUML_PTYPE_IMMEDIATE && short_immediate(srcp.value))
			emit_mov_m64_imm(&dst, MABS(drcbe, dstp.value), srcp.value);				// mov   [dstp],srcp

		/* conditional memory to register */
		else if (inst->condition != 0 && dstp.type == DRCUML_PTYPE_INT_REGISTER && srcp.type == DRCUML_PTYPE_MEMORY)
		{
			dst = savedst;
			skip.target = NULL;
			emit_cmovcc_r64_m64(&dst, X86_CONDITION(inst->condition), dstp.value, MABS(drcbe, srcp.value));
																						// cmovcc dstp,[srcp]
		}

		/* conditional register to register */
		else if (inst->condition != 0 && dstp.type == DRCUML_PTYPE_INT_REGISTER && srcp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			dst = savedst;
			skip.target = NULL;
			emit_cmovcc_r64_r64(&dst, X86_CONDITION(inst->condition), dstp.value, srcp.value);
																						// cmovcc dstp,srcp
		}

		/* general case */
		else
		{
			emit_mov_r64_p64_keepflags(drcbe, &dst, dstreg, &srcp);						// mov   dstreg,srcp
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* resolve the jump */
	if (skip.target != NULL)
		resolve_link(&dst, &skip);
	return dst;
}


/*-------------------------------------------------
    op_sext - process a SEXT opcode
-------------------------------------------------*/

static x86code *op_sext(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, sizep;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_S | DRCUML_FLAG_Z);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI, &sizep, PTYPE_I);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
		{
			if (sizep.value == DRCUML_SIZE_BYTE)
				emit_movsx_r32_m8(&dst, dstreg, MABS(drcbe, srcp.value));				// movsx dstreg,[srcp]
			else if (sizep.value == DRCUML_SIZE_WORD)
				emit_movsx_r32_m16(&dst, dstreg, MABS(drcbe, srcp.value));				// movsx dstreg,[srcp]
			else if (sizep.value == DRCUML_SIZE_DWORD)
				emit_mov_r32_m32(&dst, dstreg, MABS(drcbe, srcp.value));				// mov   dstreg,[srcp]
		}
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			if (sizep.value == DRCUML_SIZE_BYTE)
				emit_movsx_r32_r8(&dst, dstreg, srcp.value);							// movsx dstreg,srcp
			else if (sizep.value == DRCUML_SIZE_WORD)
				emit_movsx_r32_r16(&dst, dstreg, srcp.value);							// movsx dstreg,srcp
			else if (sizep.value == DRCUML_SIZE_DWORD)
				emit_mov_r32_r32(&dst, dstreg, srcp.value);								// mov   dstreg,srcp
		}
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
		if (inst->flags != 0)
			emit_test_r32_r32(&dst, dstreg, dstreg);									// test  dstreg,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		if (srcp.type == DRCUML_PTYPE_MEMORY)
		{
			if (sizep.value == DRCUML_SIZE_BYTE)
				emit_movsx_r64_m8(&dst, dstreg, MABS(drcbe, srcp.value));				// movsx dstreg,[srcp]
			else if (sizep.value == DRCUML_SIZE_WORD)
				emit_movsx_r64_m16(&dst, dstreg, MABS(drcbe, srcp.value));				// movsx dstreg,[srcp]
			else if (sizep.value == DRCUML_SIZE_DWORD)
				emit_movsxd_r64_m32(&dst, dstreg, MABS(drcbe, srcp.value));				// movsxd dstreg,[srcp]
			else if (sizep.value == DRCUML_SIZE_QWORD)
				emit_mov_r64_m64(&dst, dstreg, MABS(drcbe, srcp.value));				// mov   dstreg,[srcp]
		}
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			if (sizep.value == DRCUML_SIZE_BYTE)
				emit_movsx_r64_r8(&dst, dstreg, srcp.value);							// movsx dstreg,srcp
			else if (sizep.value == DRCUML_SIZE_WORD)
				emit_movsx_r64_r16(&dst, dstreg, srcp.value);							// movsx dstreg,srcp
			else if (sizep.value == DRCUML_SIZE_DWORD)
				emit_movsxd_r64_r32(&dst, dstreg, srcp.value);							// movsxd dstreg,srcp
			else if (sizep.value == DRCUML_SIZE_QWORD)
				emit_mov_r64_r64(&dst, dstreg, srcp.value);								// mov   dstreg,srcp
		}
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
		if (inst->flags != 0)
			emit_test_r64_r64(&dst, dstreg, dstreg);									// test  dstreg,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_roland - process an ROLAND opcode
-------------------------------------------------*/

static x86code *op_roland(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, shiftp, maskp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_S | DRCUML_FLAG_Z);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI, &shiftp, PTYPE_MRI, &maskp, PTYPE_MRI);

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
		emit_mov_r64_p64(drcbe, &dst, dstreg, &srcp);									// mov   dstreg,srcp
		emit_rol_r64_p64(drcbe, &dst, dstreg, &shiftp, inst);							// rol   dstreg,shiftp
		emit_and_r64_p64(drcbe, &dst, dstreg, &maskp, inst);							// and   dstreg,maskp
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_rolins - process an ROLINS opcode
-------------------------------------------------*/

static x86code *op_rolins(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, shiftp, maskp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_S | DRCUML_FLAG_Z);

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
		emit_mov_r64_p64(drcbe, &dst, REG_RAX, &srcp);									// mov   rax,srcp
		emit_mov_r64_p64(drcbe, &dst, REG_RDX, &maskp);									// mov   rdx,maskp
		emit_rol_r64_p64(drcbe, &dst, REG_RAX, &shiftp, inst);							// rol   rax,shiftp
		emit_mov_r64_p64(drcbe, &dst, dstreg, &dstp);									// mov   dstreg,dstp
		emit_and_r64_r64(&dst, REG_RAX, REG_RDX);										// and   eax,rdx
		emit_not_r64(&dst, REG_RDX);													// not   rdx
		emit_and_r64_r64(&dst, dstreg, REG_RDX);										// and   dstreg,rdx
		emit_or_r64_r64(&dst, dstreg, REG_RAX);											// or    dstreg,rax
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (dstp.type == DRCUML_PTYPE_MEMORY && src1p.type == DRCUML_PTYPE_MEMORY && src1p.value == dstp.value)
			emit_add_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// add   [dstp],src2p

		/* reg = reg + imm */
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER && src1p.type == DRCUML_PTYPE_INT_REGISTER && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->flags == 0)
			emit_lea_r32_m32(&dst, dstp.value, MBD(src1p.value, src2p.value));			// lea   dstp,[src1p+src2p]

		/* reg = reg + reg */
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER && src1p.type == DRCUML_PTYPE_INT_REGISTER && src2p.type == DRCUML_PTYPE_INT_REGISTER && inst->flags == 0)
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
		if (dstp.type == DRCUML_PTYPE_MEMORY && src1p.type == DRCUML_PTYPE_MEMORY && src1p.value == dstp.value)
			emit_add_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// add   [dstp],src2p

		/* reg = reg + imm */
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER && src1p.type == DRCUML_PTYPE_INT_REGISTER && src2p.type == DRCUML_PTYPE_IMMEDIATE && short_immediate(src2p.value) && inst->flags == 0)
			emit_lea_r64_m64(&dst, dstp.value, MBD(src1p.value, src2p.value));			// lea   dstp,[src1p+src2p]

		/* reg = reg + reg */
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER && src1p.type == DRCUML_PTYPE_INT_REGISTER && src2p.type == DRCUML_PTYPE_INT_REGISTER && inst->flags == 0)
			emit_lea_r64_m64(&dst, dstp.value, MBISD(src1p.value, src2p.value, 1, 0));	// lea   dstp,[src1p+src2p]

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_add_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// add   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_adc_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// adc   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32_keepflags(drcbe, &dst, dstreg, &src1p);					// mov   dstreg,src1p
			emit_adc_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// adc   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_adc_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// adc   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64_keepflags(drcbe, &dst, dstreg, &src1p);					// mov   dstreg,src1p
			emit_adc_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// adc   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (dstp.type == DRCUML_PTYPE_MEMORY && src1p.type == DRCUML_PTYPE_MEMORY && src1p.value == dstp.value)
			emit_sub_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// sub   [dstp],src2p

		/* reg = reg - imm */
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER && src1p.type == DRCUML_PTYPE_INT_REGISTER && src2p.type == DRCUML_PTYPE_IMMEDIATE && inst->flags == 0)
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
		if (dstp.type == DRCUML_PTYPE_MEMORY && src1p.type == DRCUML_PTYPE_MEMORY && src1p.value == dstp.value)
			emit_sub_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// sub   [dstp],src2p

		/* reg = reg - imm */
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER && src1p.type == DRCUML_PTYPE_INT_REGISTER && src2p.type == DRCUML_PTYPE_IMMEDIATE && short_immediate(src2p.value) && inst->flags == 0)
			emit_lea_r64_m64(&dst, dstp.value, MBD(src1p.value, -src2p.value));			// lea   dstp,[src1p-src2p]

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_sub_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// sub   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_sbb_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// sbb   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32_keepflags(drcbe, &dst, dstreg, &src1p);					// mov   dstreg,src1p
			emit_sbb_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// sbb   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_sbb_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// sbb   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64_keepflags(drcbe, &dst, dstreg, &src1p);					// mov   dstreg,src1p
			emit_sbb_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// sbb   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
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
	int src1reg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	src1reg = param_select_register(REG_EAX, &src1p, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* memory versus anything */
		if (src1p.type == DRCUML_PTYPE_MEMORY)
			emit_cmp_m32_p32(drcbe, &dst, MABS(drcbe, src1p.value), &src2p, inst);		// cmp   [dstp],src2p

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
			emit_cmp_m64_p64(drcbe, &dst, MABS(drcbe, src1p.value), &src2p, inst);		// cmp   [dstp],src2p

		/* general case */
		else
		{
			if (src1p.type == DRCUML_PTYPE_IMMEDIATE)
				emit_mov_r64_imm(&dst, src1reg, src1p.value);							// mov   src1reg,imm
			emit_cmp_r64_p64(drcbe, &dst, src1reg, &src2p, inst);						// cmp   src1reg,src2p
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_mulu - process a MULU opcode
-------------------------------------------------*/

static x86code *op_mulu(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	UINT8 zsflags = inst->flags & (DRCUML_FLAG_Z | DRCUML_FLAG_S);
	UINT8 vflag =  inst->flags & DRCUML_FLAG_V;
	drcuml_parameter dstp, edstp, src1p, src2p;
	int compute_hi;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_4_commutative(drcbe, inst, &dstp, PTYPE_MR, &edstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);
	compute_hi = (dstp.type != edstp.type || dstp.value != edstp.value);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);									// mov   eax,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_mul_m32(&dst, MABS(drcbe, src2p.value));								// mul   [src2p]
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

		/* compute flags */
		if (inst->flags != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					emit_pushf(&dst);													// pushf
				if (compute_hi)
				{
					if (zsflags == DRCUML_FLAG_Z)
						emit_or_r32_r32(&dst, REG_EDX, REG_EAX);						// or    edx,eax
					else if (zsflags == DRCUML_FLAG_S)
						emit_test_r32_r32(&dst, REG_EDX, REG_EDX);						// test  edx,edx
					else
					{
						emit_movzx_r32_r16(&dst, REG_ECX, REG_AX);						// movzx ecx,ax
						emit_shr_r32_imm(&dst, REG_EAX, 16);							// shr   eax,16
						emit_or_r32_r32(&dst, REG_EDX, REG_ECX);						// or    edx,ecx
						emit_or_r32_r32(&dst, REG_EDX, REG_EAX);						// or    edx,eax
					}
				}
				else
					emit_test_r32_r32(&dst, REG_EAX, REG_EAX);							// test  eax,eax

				/* we rely on the fact that OF is cleared by all logical operations above */
				if (vflag)
				{
					emit_pushf(&dst);													// pushf
					emit_pop_r64(&dst, REG_RAX);										// pop   rax
					emit_and_m64_imm(&dst, MBD(REG_RSP, 0), ~0x84);						// and   [rsp],~0x84
					emit_or_m64_r64(&dst, MBD(REG_RSP, 0), REG_RAX);					// or    [rsp],rax
					emit_popf(&dst);													// popf
				}
			}
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, REG_RAX, &src1p);									// mov   rax,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_mul_m64(&dst, MABS(drcbe, src2p.value));								// mul   [src2p]
		else if (src2p.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mul_r64(&dst, src2p.value);											// mul   src2p
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE)
		{
			emit_mov_r64_imm(&dst, REG_RDX, src2p.value);								// mov   rdx,src2p
			emit_mul_r64(&dst, REG_RDX);												// mul   rdx
		}
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_RAX);									// mov   dstp,rax
		if (compute_hi)
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_RDX);								// mov   edstp,rdx

		/* compute flags */
		if (inst->flags != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					emit_pushf(&dst);													// pushf
				if (compute_hi)
				{
					if (zsflags == DRCUML_FLAG_Z)
						emit_or_r64_r64(&dst, REG_RDX, REG_RAX);						// or    rdx,rax
					else if (zsflags == DRCUML_FLAG_S)
						emit_test_r64_r64(&dst, REG_RDX, REG_RDX);						// test  rdx,rdx
					else
					{
						emit_mov_r32_r32(&dst, REG_ECX, REG_EAX);						// mov   ecx,eax
						emit_shr_r64_imm(&dst, REG_RAX, 32);							// shr   rax,32
						emit_or_r64_r64(&dst, REG_RDX, REG_RCX);						// or    rdx,rcx
						emit_or_r64_r64(&dst, REG_RDX, REG_RAX);						// or    rdx,rax
					}
				}
				else
					emit_test_r64_r64(&dst, REG_RAX, REG_RAX);							// test  rax,rax

				/* we rely on the fact that OF is cleared by all logical operations above */
				if (vflag)
				{
					emit_pushf(&dst);													// pushf
					emit_pop_r64(&dst, REG_RAX);										// pop   rax
					emit_and_m64_imm(&dst, MBD(REG_RSP, 0), ~0x84);						// and   [rsp],~0x84
					emit_or_m64_r64(&dst, MBD(REG_RSP, 0), REG_RAX);					// or    [rsp],rax
					emit_popf(&dst);													// popf
				}
			}
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_muls - process a MULS opcode
-------------------------------------------------*/

static x86code *op_muls(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	UINT8 zsflags = inst->flags & (DRCUML_FLAG_Z | DRCUML_FLAG_S);
	UINT8 vflag =  inst->flags & DRCUML_FLAG_V;
	drcuml_parameter dstp, edstp, src1p, src2p;
	int compute_hi;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_4_commutative(drcbe, inst, &dstp, PTYPE_MR, &edstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);
	compute_hi = (dstp.type != edstp.type || dstp.value != edstp.value);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* 32-bit destination with memory/immediate or register/immediate */
		if (!compute_hi && src1p.type != DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_imul_r32_m32_imm(&dst, REG_EAX, MABS(drcbe, src1p.value), src2p.value);	// imul  eax,[src1p],src2p
			else if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_imul_r32_r32_imm(&dst, REG_EAX, src1p.value, src2p.value);			// imul  eax,src1p,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);								// mov   dstp,eax
		}

		/* 32-bit destination, general case */
		else if (!compute_hi)
		{
			emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);								// mov   eax,src1p
			if (src2p.type == DRCUML_PTYPE_MEMORY)
				emit_imul_r32_m32(&dst, REG_EAX, MABS(drcbe, src2p.value));					// imul  eax,[src2p]
			else if (src2p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_imul_r32_r32(&dst, REG_EAX, src2p.value);							// imul  eax,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);								// mov   dstp,eax
		}

		/* 64-bit destination, general case */
		else
		{
			emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);								// mov   eax,src1p
			if (src2p.type == DRCUML_PTYPE_MEMORY)
				emit_imul_m32(&dst, MABS(drcbe, src2p.value));									// imul  [src2p]
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

		/* compute flags */
		if (inst->flags != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					emit_pushf(&dst);													// pushf
				if (compute_hi)
				{
					if (zsflags == DRCUML_FLAG_Z)
						emit_or_r32_r32(&dst, REG_EDX, REG_EAX);						// or    edx,eax
					else if (zsflags == DRCUML_FLAG_S)
						emit_test_r32_r32(&dst, REG_EDX, REG_EDX);						// test  edx,edx
					else
					{
						emit_movzx_r32_r16(&dst, REG_ECX, REG_AX);						// movzx ecx,ax
						emit_shr_r32_imm(&dst, REG_EAX, 16);							// shr   eax,16
						emit_or_r32_r32(&dst, REG_EDX, REG_ECX);						// or    edx,ecx
						emit_or_r32_r32(&dst, REG_EDX, REG_EAX);						// or    edx,eax
					}
				}
				else
					emit_test_r32_r32(&dst, REG_EAX, REG_EAX);							// test  eax,eax

				/* we rely on the fact that OF is cleared by all logical operations above */
				if (vflag)
				{
					emit_pushf(&dst);													// pushf
					emit_pop_r64(&dst, REG_RAX);										// pop   rax
					emit_and_m64_imm(&dst, MBD(REG_RSP, 0), ~0x84);						// and   [rsp],~0x84
					emit_or_m64_r64(&dst, MBD(REG_RSP, 0), REG_RAX);					// or    [rsp],rax
					emit_popf(&dst);													// popf
				}
			}
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* 64-bit destination with memory/immediate or register/immediate */
		if (!compute_hi && src1p.type != DRCUML_PTYPE_IMMEDIATE && src2p.type == DRCUML_PTYPE_IMMEDIATE && short_immediate(src2p.type))
		{
			if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_imul_r64_m64_imm(&dst, REG_RAX, MABS(drcbe, src1p.value), src2p.value);// imul  rax,[src1p],src2p
			else if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_imul_r64_r64_imm(&dst, REG_RAX, src1p.value, src2p.value);			// imul  rax,src1p,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, REG_RAX);								// mov   dstp,rax
		}

		/* 64-bit destination, general case */
		else if (!compute_hi)
		{
			emit_mov_r64_p64(drcbe, &dst, REG_RAX, &src1p);								// mov   rax,src1p
			if (src2p.type == DRCUML_PTYPE_MEMORY)
				emit_imul_r64_m64(&dst, REG_RAX, MABS(drcbe, src2p.value));				// imul  rax,[src2p]
			else if (src2p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_imul_r64_r64(&dst, REG_RAX, src2p.value);							// imul  rax,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, REG_RAX);								// mov   dstp,rax
		}

		/* 128-bit destination, general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, REG_RAX, &src1p);								// mov   rax,src1p
			if (src2p.type == DRCUML_PTYPE_MEMORY)
				emit_imul_m64(&dst, MABS(drcbe, src2p.value));							// imul  [src2p]
			else if (src2p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_imul_r64(&dst, src2p.value);										// imul  src2p
			else if (src2p.type == DRCUML_PTYPE_IMMEDIATE)
			{
				emit_mov_r64_imm(&dst, REG_RDX, src2p.value);							// mov   rdx,src2p
				emit_imul_r64(&dst, REG_RDX);											// imul  rdx
			}
			emit_mov_p64_r64(drcbe, &dst, &dstp, REG_RAX);								// mov   dstp,rax
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_RDX);								// mov   edstp,rdx
		}

		/* compute flags */
		if (inst->flags != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					emit_pushf(&dst);													// pushf
				if (compute_hi)
				{
					if (zsflags == DRCUML_FLAG_Z)
						emit_or_r64_r64(&dst, REG_RDX, REG_RAX);						// or    rdx,rax
					else if (zsflags == DRCUML_FLAG_S)
						emit_test_r64_r64(&dst, REG_RDX, REG_RDX);						// test  rdx,rdx
					else
					{
						emit_mov_r32_r32(&dst, REG_ECX, REG_EAX);						// mov   ecx,eax
						emit_shr_r64_imm(&dst, REG_RAX, 32);							// shr   rax,32
						emit_or_r64_r64(&dst, REG_RDX, REG_RCX);						// or    rdx,rcx
						emit_or_r64_r64(&dst, REG_RDX, REG_RAX);						// or    rdx,rax
					}
				}
				else
					emit_test_r64_r64(&dst, REG_RAX, REG_RAX);							// test  rax,rax

				/* we rely on the fact that OF is cleared by all logical operations above */
				if (vflag)
				{
					emit_pushf(&dst);													// pushf
					emit_pop_r64(&dst, REG_RAX);										// pop   rax
					emit_and_m64_imm(&dst, MBD(REG_RSP, 0), ~0x84);						// and   [rsp],~0x84
					emit_or_m64_r64(&dst, MBD(REG_RSP, 0), REG_RAX);					// or    [rsp],rax
					emit_popf(&dst);													// popf
				}
			}
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &edstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);
	compute_rem = (dstp.type != edstp.type || dstp.value != edstp.value);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_r32_p32(drcbe, &dst, REG_ECX, &src2p);									// mov   ecx,src2p
		if (inst->flags != 0)
		{
			emit_mov_r32_imm(&dst, REG_EAX, 0xa0000000);								// mov   eax,0xa0000000
			emit_add_r32_r32(&dst, REG_EAX, REG_EAX);									// add   eax,eax
		}
		emit_jecxz_link(&dst, &skip);													// jecxz skip
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);									// mov   eax,src1p
		emit_xor_r32_r32(&dst, REG_EDX, REG_EDX);										// xor   edx,edx
		emit_div_r32(&dst, REG_ECX);													// div   ecx
		emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);									// mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(drcbe, &dst, &edstp, REG_EDX);								// mov   edstp,edx
		if (inst->flags != 0)
			emit_test_r32_r32(&dst, REG_EAX, REG_EAX);									// test  eax,eax
		resolve_link(&dst, &skip);													// skip:
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, REG_RCX, &src2p);									// mov   rcx,src2p
		if (inst->flags != 0)
		{
			emit_mov_r32_imm(&dst, REG_EAX, 0xa0000000);								// mov   eax,0xa0000000
			emit_add_r32_r32(&dst, REG_EAX, REG_EAX);									// add   eax,eax
		}
		emit_jrcxz_link(&dst, &skip);													// jrcxz skip
		emit_mov_r64_p64(drcbe, &dst, REG_RAX, &src1p);									// mov   rax,src1p
		emit_xor_r32_r32(&dst, REG_EDX, REG_EDX);										// xor   edx,edx
		emit_div_r64(&dst, REG_RCX);													// div   rcx
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_RAX);									// mov   dstp,rax
		if (compute_rem)
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_RDX);								// mov   edstp,rdx
		if (inst->flags != 0)
			emit_test_r64_r64(&dst, REG_RAX, REG_RAX);									// test  eax,eax
		resolve_link(&dst, &skip);													// skip:
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &edstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);
	compute_rem = (dstp.type != edstp.type || dstp.value != edstp.value);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* general case */
		emit_mov_r32_p32(drcbe, &dst, REG_ECX, &src2p);									// mov   ecx,src2p
		if (inst->flags != 0)
		{
			emit_mov_r32_imm(&dst, REG_EAX, 0xa0000000);								// mov   eax,0xa0000000
			emit_add_r32_r32(&dst, REG_EAX, REG_EAX);									// add   eax,eax
		}
		emit_jecxz_link(&dst, &skip);													// jecxz skip
		emit_mov_r32_p32(drcbe, &dst, REG_EAX, &src1p);									// mov   eax,src1p
		emit_cdq(&dst);																	// cdq
		emit_idiv_r32(&dst, REG_ECX);													// idiv  ecx
		emit_mov_p32_r32(drcbe, &dst, &dstp, REG_EAX);									// mov   dstp,eax
		if (compute_rem)
			emit_mov_p32_r32(drcbe, &dst, &edstp, REG_EDX);								// mov   edstp,edx
		if (inst->flags != 0)
			emit_test_r32_r32(&dst, REG_EAX, REG_EAX);									// test  eax,eax
		resolve_link(&dst, &skip);													// skip:
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, REG_RCX, &src2p);									// mov   rcx,src2p
		if (inst->flags != 0)
		{
			emit_mov_r32_imm(&dst, REG_EAX, 0xa0000000);								// mov   eax,0xa0000000
			emit_add_r32_r32(&dst, REG_EAX, REG_EAX);									// add   eax,eax
		}
		emit_jrcxz_link(&dst, &skip);													// jrcxz skip
		emit_mov_r64_p64(drcbe, &dst, REG_RAX, &src1p);									// mov   rax,src1p
		emit_cqo(&dst);																	// cqo
		emit_idiv_r64(&dst, REG_RCX);													// idiv  rcx
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_RAX);									// mov   dstp,rax
		if (compute_rem)
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_RDX);								// mov   edstp,rdx
		if (inst->flags != 0)
			emit_test_r64_r64(&dst, REG_RAX, REG_RAX);									// test  eax,eax
		resolve_link(&dst, &skip);													// skip:
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_and_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// and   [dstp],src2p

		/* AND with immediate 0xff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xff && inst->flags == 0)
		{
			if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_movzx_r32_r8(&dst, dstreg, src1p.value);							// movzx dstreg,src1p
			else if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_movzx_r32_m8(&dst, dstreg, MABS(drcbe, src1p.value));				// movzx dstreg,[src1p]
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}

		/* AND with immediate 0xffff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xffff && inst->flags == 0)
		{
			if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_movzx_r32_r16(&dst, dstreg, src1p.value);							// movzx dstreg,src1p
			else if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_movzx_r32_m16(&dst, dstreg, MABS(drcbe, src1p.value));				// movzx dstreg,[src1p]
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}

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
			emit_and_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// and   [dstp],src2p

		/* AND with immediate 0xff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xff && inst->flags == 0)
		{
			if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_movzx_r32_r8(&dst, dstreg, src1p.value);							// movzx dstreg,src1p
			else if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_movzx_r32_m8(&dst, dstreg, MABS(drcbe, src1p.value));				// movzx dstreg,[src1p]
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}

		/* AND with immediate 0xffff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xffff && inst->flags == 0)
		{
			if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_movzx_r32_r16(&dst, dstreg, src1p.value);							// movzx dstreg,src1p
			else if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_movzx_r32_m16(&dst, dstreg, MABS(drcbe, src1p.value));				// movzx dstreg,[src1p]
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}

		/* AND with immediate 0xffffffff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xffffffff && inst->flags == 0)
		{
			if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_r32_r32(&dst, dstreg, dstreg);									// mov   dstreg,dstreg
			else
			{
				emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);							// mov   dstreg,src1p
				emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);							// mov   dstp,dstreg
			}
		}

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_and_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// and   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_2_commutative(drcbe, inst, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	src1reg = param_select_register(REG_EAX, &src1p, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* src1p in memory */
		if (src1p.type == DRCUML_PTYPE_MEMORY)
			emit_test_m32_p32(drcbe, &dst, MABS(drcbe, src1p.value), &src2p, inst);		// test  [src1p],src2p

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
			emit_test_m64_p64(drcbe, &dst, MABS(drcbe, src1p.value), &src2p, inst);		// test  [src1p],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, src1reg, &src1p);								// mov   src1reg,src1p
			emit_test_r64_p64(drcbe, &dst, src1reg, &src2p, inst);						// test  src1reg,src2p
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_or_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// or    [dstp],src2p

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
			emit_or_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// or    [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_or_r64_p64(drcbe, &dst, dstreg, &src2p, inst);							// or    dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_xor_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// xor   [dstp],src2p

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
			emit_xor_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// xor   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_xor_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// xor   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_mov_r32_p32(drcbe, &dst, dstreg, &srcp);									// mov   dstreg,src1p
		emit_mov_r32_imm(&dst, REG_ECX, 32 ^ 31);										// mov   ecx,32 ^ 31
		emit_bsr_r32_r32(&dst, dstreg, dstreg);											// bsr   dstreg,dstreg
		emit_cmovcc_r32_r32(&dst, COND_Z, dstreg, REG_ECX);								// cmovz dstreg,ecx
		emit_xor_r32_imm(&dst, dstreg, 31);												// xor   dstreg,31
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_mov_r64_p64(drcbe, &dst, dstreg, &srcp);									// mov   dstreg,src1p
		emit_mov_r64_imm(&dst, REG_RCX, 64 ^ 63);										// mov   rcx,64 ^ 63
		emit_bsr_r64_r64(&dst, dstreg, dstreg);											// bsr   dstreg,dstreg
		emit_cmovcc_r64_r64(&dst, COND_Z, dstreg, REG_RCX);								// cmovz dstreg,rcx
		emit_xor_r32_imm(&dst, dstreg, 63);												// xor   dstreg,63
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_RAX, &dstp, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_mov_r32_p32(drcbe, &dst, dstreg, &srcp);									// mov   dstreg,src1p
		emit_bswap_r32(&dst, dstreg);													// bswap dstreg
		if (inst->flags != 0)
			emit_test_r32_r32(&dst, dstreg, dstreg);									// test  dstreg,dstreg
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_mov_r64_p64(drcbe, &dst, dstreg, &srcp);									// mov   dstreg,src1p
		emit_bswap_r64(&dst, dstreg);													// bswap dstreg
		if (inst->flags != 0)
			emit_test_r64_r64(&dst, dstreg, dstreg);									// test  dstreg,dstreg
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_shl_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// shl   [dstp],src2p

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
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_shl_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// shl   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_shl_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// shl   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_shr_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// shr   [dstp],src2p

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
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_shr_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// shr   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_shr_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// shr   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_sar_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// sar   [dstp],src2p

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
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_sar_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// sar   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_sar_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// sar   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_rol_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// rol   [dstp],src2p

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
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_rol_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// rol   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_rol_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// rol   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_ror_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// ror   [dstp],src2p

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
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_ror_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// ror   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_ror_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// ror   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_rcl_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// rcl   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32_keepflags(drcbe, &dst, dstreg, &src1p);					// mov   dstreg,src1p
			emit_rcl_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// rcl   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_rcl_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// rcl   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64_keepflags(drcbe, &dst, dstreg, &src1p);					// mov   dstreg,src1p
			emit_rcl_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// rcl   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_S);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MR, &src1p, PTYPE_MRI, &src2p, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_rcr_m32_p32(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// rcr   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r32_p32_keepflags(drcbe, &dst, dstreg, &src1p);					// mov   dstreg,src1p
			emit_rcr_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// rcr   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* dstp == src1p in memory */
		if (src1p.type == dstp.type && src1p.value == dstp.value && dstp.type == DRCUML_PTYPE_MEMORY)
			emit_rcr_m64_p64(drcbe, &dst, MABS(drcbe, dstp.value), &src2p, inst);		// rcr   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64_keepflags(drcbe, &dst, dstreg, &src1p);					// mov   dstreg,src1p
			emit_rcr_r64_p64(drcbe, &dst, dstreg, &src2p, inst);						// rcr   dstreg,src2p
			emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}
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
	int basereg, dstreg;
	INT32 baseoffs;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, NULL);

	/* determine the pointer base */
	basereg = get_base_register_and_offset(drcbe, &dst, basep.value, REG_RDX, &baseoffs);

	/* 32-bit form */
	if (inst->size == 4)
	{
		if (indp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_movss_r128_m32(&dst, dstreg, MBD(basereg, baseoffs + 4*indp.value));	// movss  dstreg,[basep + 4*indp]
		else
		{
			int indreg = param_select_register(REG_ECX, &indp, NULL);
			emit_mov_r32_p32(drcbe, &dst, indreg, &indp);								// mov    indreg,indp
			emit_movss_r128_m32(&dst, dstreg, MBISD(basereg, indreg, 4, baseoffs));		// movss  dstreg,[basep + 4*indp]
		}
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss  dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		if (indp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_movsd_r128_m64(&dst, dstreg, MBD(basereg, baseoffs + 8*indp.value));	// movsd  dstreg,[basep + 8*indp]
		else
		{
			int indreg = param_select_register(REG_ECX, &indp, NULL);
			emit_mov_r32_p32(drcbe, &dst, indreg, &indp);								// mov    indreg,indp
			emit_movsd_r128_m64(&dst, dstreg, MBISD(basereg, indreg, 8, baseoffs));		// movsd  dstreg,[basep + 8*indp]
		}
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd  dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_fstore - process a FSTORE opcode
-------------------------------------------------*/

static x86code *op_fstore(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, basep, indp;
	int basereg, srcreg;
	INT32 baseoffs;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &basep, PTYPE_M, &indp, PTYPE_MRI, &srcp, PTYPE_MF);

	/* pick a target register for the general case */
	srcreg = param_select_register(REG_XMM0, &srcp, NULL);

	/* determine the pointer base */
	basereg = get_base_register_and_offset(drcbe, &dst, basep.value, REG_RDX, &baseoffs);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_movss_r128_p32(drcbe, &dst, srcreg, &srcp);								// movss  srcreg,srcp
		if (indp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_movss_m32_r128(&dst, MBD(basereg, baseoffs + 4*indp.value), srcreg);	// movss  [basep + 4*indp],srcreg
		else
		{
			int indreg = param_select_register(REG_ECX, &indp, NULL);
			emit_mov_r32_p32(drcbe, &dst, indreg, &indp);								// mov    indreg,indp
			emit_movss_m32_r128(&dst, MBISD(basereg, indreg, 4, baseoffs), srcreg);		// movss  [basep + 4*indp],srcreg
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_movsd_r128_p64(drcbe, &dst, srcreg, &srcp);								// movsd  srcreg,srcp
		if (indp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_movsd_m64_r128(&dst, MBD(basereg, baseoffs + 8*indp.value), srcreg);	// movsd  [basep + 8*indp],srcreg
		else
		{
			int indreg = param_select_register(REG_ECX, &indp, NULL);
			emit_mov_r32_p32(drcbe, &dst, indreg, &indp);								// mov    indreg,indp
			emit_movsd_m64_r128(&dst, MBISD(basereg, indreg, 8, baseoffs), srcreg);		// movsd  [basep + 8*indp],srcreg
		}
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
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &addrp, PTYPE_MRI, &spacep, PTYPE_I);

	/* set up a call to the read dword/qword handler */
	emit_mov_r64_imm(&dst, REG_PARAM1, (FPTR)(drcbe->space[spacep.value]));				// mov    param1,space
	emit_mov_r32_p32(drcbe, &dst, REG_PARAM2, &addrp);									// mov    param2,addrp
	if (inst->size == 4)
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacep.value].read_dword);// call   read_dword
	else if (inst->size == 8)
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacep.value].read_qword);// call   read_qword

	/* store result */
	if (inst->size == 4)
	{
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_r32(&dst, MABS(drcbe, dstp.value), REG_EAX);					// mov   [dstp],eax
		else if (dstp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_movd_r128_r32(&dst, dstp.value, REG_EAX);								// movd  dstp,eax
	}
	else if (inst->size == 8)
	{
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m64_r64(&dst, MABS(drcbe, dstp.value), REG_RAX);					// mov   [dstp],rax
		else if (dstp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_movq_r128_r64(&dst, dstp.value, REG_RAX);								// movq  dstp,rax
	}
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
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &addrp, PTYPE_MRI, &srcp, PTYPE_MF, &spacep, PTYPE_I);

	/* general case */
	emit_mov_r64_imm(&dst, REG_PARAM1, (FPTR)(drcbe->space[spacep.value]));				// mov    param1,space
	emit_mov_r32_p32(drcbe, &dst, REG_PARAM2, &addrp);									// mov    param21,addrp

	/* 32-bit form */
	if (inst->size == 4)
	{
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_r32_m32(&dst, REG_PARAM3, MABS(drcbe, srcp.value));				// mov    param3,[srcp]
		else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_movd_r32_r128(&dst, REG_PARAM3, srcp.value);							// movd   param3,srcp
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacep.value].write_dword);// call   write_dword
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_r64_m64(&dst, REG_PARAM3, MABS(drcbe, srcp.value));				// mov    param3,[srcp]
		else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_movq_r64_r128(&dst, REG_PARAM3, srcp.value);							// movq   param3,srcp
		emit_smart_call_m64(drcbe, &dst, (x86code **)&drcbe->accessors[spacep.value].write_qword);// call   write_qword
	}

	return dst;
}


/*-------------------------------------------------
    op_fmov - process a FMOV opcode
-------------------------------------------------*/

static x86code *op_fmov(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	emit_link skip = { 0 };
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, NULL);

	/* always start with a jmp */
	if (inst->condition != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condition), &skip);			// jcc   skip

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_movss_r128_p32(drcbe, &dst, dstreg, &srcp);								// movss dstreg,srcp
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_movsd_r128_p64(drcbe, &dst, dstreg, &srcp);								// movsd dstreg,srcp
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd dstp,dstreg
	}

	/* resolve the jump */
	if (inst->condition != DRCUML_COND_ALWAYS)
		resolve_link(&dst, &skip);													// skip:
	return dst;
}


/*-------------------------------------------------
    op_ftoint - process a FTOINT opcode
-------------------------------------------------*/

static x86code *op_ftoint(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, sizep, roundp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF, &sizep, PTYPE_I, &roundp, PTYPE_I);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* set rounding mode if necessary */
	if (roundp.value != DRCUML_FMOD_DEFAULT && roundp.value != DRCUML_FMOD_TRUNC)
	{
		emit_stmxcsr_m32(&dst, MABS(drcbe, &drcbe->ssemodesave));						// stmxcsr [ssemodesave]
		emit_ldmxcsr_m32(&dst, MABS(drcbe, &drcbe->ssecontrol[roundp.value]));			// ldmxcsr fpcontrol[mode]
	}

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* 32-bit integer source */
		if (sizep.value == DRCUML_SIZE_DWORD)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
			{
				if (roundp.value != DRCUML_FMOD_TRUNC)
					emit_cvtss2si_r32_m32(&dst, dstreg, MABS(drcbe, srcp.value));		// cvtss2si dstreg,[srcp]
				else
					emit_cvttss2si_r32_m32(&dst, dstreg, MABS(drcbe, srcp.value));		// cvttss2si dstreg,[srcp]
			}
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			{
				if (roundp.value != DRCUML_FMOD_TRUNC)
					emit_cvtss2si_r32_r128(&dst, dstreg, srcp.value);					// cvtss2si dstreg,srcp
				else
					emit_cvttss2si_r32_r128(&dst, dstreg, srcp.value);					// cvttss2si dstreg,srcp
			}
		}

		/* 64-bit integer source */
		else if (sizep.value == DRCUML_SIZE_QWORD)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
			{
				if (roundp.value != DRCUML_FMOD_TRUNC)
					emit_cvtss2si_r64_m32(&dst, dstreg, MABS(drcbe, srcp.value));		// cvtss2si dstreg,[srcp]
				else
					emit_cvttss2si_r64_m32(&dst, dstreg, MABS(drcbe, srcp.value));		// cvttss2si dstreg,[srcp]
			}
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			{
				if (roundp.value != DRCUML_FMOD_TRUNC)
					emit_cvtss2si_r64_r128(&dst, dstreg, srcp.value);					// cvtss2si dstreg,srcp
				else
					emit_cvttss2si_r64_r128(&dst, dstreg, srcp.value);					// cvttss2si dstreg,srcp
			}
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* 32-bit integer source */
		if (sizep.value == DRCUML_SIZE_DWORD)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
			{
				if (roundp.value != DRCUML_FMOD_TRUNC)
					emit_cvtsd2si_r32_m64(&dst, dstreg, MABS(drcbe, srcp.value));		// cvtsd2si dstreg,[srcp]
				else
					emit_cvttsd2si_r32_m64(&dst, dstreg, MABS(drcbe, srcp.value));		// cvttsd2si dstreg,[srcp]
			}
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			{
				if (roundp.value != DRCUML_FMOD_TRUNC)
					emit_cvtsd2si_r32_r128(&dst, dstreg, srcp.value);					// cvtsd2si dstreg,srcp
				else
					emit_cvttsd2si_r32_r128(&dst, dstreg, srcp.value);					// cvttsd2si dstreg,srcp
			}
		}

		/* 64-bit integer source */
		else if (sizep.value == DRCUML_SIZE_QWORD)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
			{
				if (roundp.value != DRCUML_FMOD_TRUNC)
					emit_cvtsd2si_r64_m64(&dst, dstreg, MABS(drcbe, srcp.value));		// cvtsd2si dstreg,[srcp]
				else
					emit_cvttsd2si_r64_m64(&dst, dstreg, MABS(drcbe, srcp.value));		// cvttsd2si dstreg,[srcp]
			}
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			{
				if (roundp.value != DRCUML_FMOD_TRUNC)
					emit_cvtsd2si_r64_r128(&dst, dstreg, srcp.value);					// cvtsd2si dstreg,srcp
				else
					emit_cvttsd2si_r64_r128(&dst, dstreg, srcp.value);					// cvttsd2si dstreg,srcp
			}
		}
	}

	/* general case */
	if (sizep.value == DRCUML_SIZE_DWORD)
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	else
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

	/* restore rounding mode */
	if (roundp.value != DRCUML_FMOD_DEFAULT && roundp.value != DRCUML_FMOD_TRUNC)
		emit_ldmxcsr_m32(&dst, MABS(drcbe, &drcbe->ssemodesave));						// ldmxcsr [ssemodesave]

	return dst;
}


/*-------------------------------------------------
    op_ffrint - process a FFRINT opcode
-------------------------------------------------*/

static x86code *op_ffrint(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, sizep;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MRI, &sizep, PTYPE_I);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		/* 32-bit integer source */
		if (sizep.value == DRCUML_SIZE_DWORD)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_cvtsi2ss_r128_m32(&dst, dstreg, MABS(drcbe, srcp.value));			// cvtsi2ss dstreg,[srcp]
			else
			{
				int srcreg = param_select_register(REG_EAX, &srcp, NULL);
				emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);							// mov      srcreg,srcp
				emit_cvtsi2ss_r128_r32(&dst, dstreg, srcreg);							// cvtsi2ss dstreg,srcreg
			}
		}

		/* 64-bit integer source */
		else
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_cvtsi2ss_r128_m64(&dst, dstreg, MABS(drcbe, srcp.value));			// cvtsi2ss dstreg,[srcp]
			else
			{
				int srcreg = param_select_register(REG_RAX, &srcp, NULL);
				emit_mov_r64_p64(drcbe, &dst, srcreg, &srcp);							// mov      srcreg,srcp
				emit_cvtsi2ss_r128_r64(&dst, dstreg, srcreg);							// cvtsi2ss dstreg,srcreg
			}
		}
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss    dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* 32-bit integer source */
		if (sizep.value == DRCUML_SIZE_DWORD)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_cvtsi2sd_r128_m32(&dst, dstreg, MABS(drcbe, srcp.value));			// cvtsi2sd dstreg,[srcp]
			else
			{
				int srcreg = param_select_register(REG_EAX, &srcp, NULL);
				emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);							// mov      srcreg,srcp
				emit_cvtsi2sd_r128_r32(&dst, dstreg, srcreg);							// cvtsi2sd dstreg,srcreg
			}
		}

		/* 64-bit integer source */
		else
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_cvtsi2sd_r128_m64(&dst, dstreg, MABS(drcbe, srcp.value));			// cvtsi2sd dstreg,[srcp]
			else
			{
				int srcreg = param_select_register(REG_EAX, &srcp, NULL);
				emit_mov_r64_p64(drcbe, &dst, srcreg, &srcp);							// mov      srcreg,srcp
				emit_cvtsi2sd_r128_r64(&dst, dstreg, srcreg);							// cvtsi2sd dstreg,srcreg
			}
		}
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd    dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_ffrflt - process a FFRFLT opcode
-------------------------------------------------*/

static x86code *op_ffrflt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, sizep;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF, &sizep, PTYPE_I);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, NULL);

	/* single-to-double */
	if (inst->size == 8 && sizep.value == DRCUML_SIZE_DWORD)
	{
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_cvtss2sd_r128_m32(&dst, dstreg, MABS(drcbe, srcp.value));				// cvtss2sd dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_cvtss2sd_r128_r128(&dst, dstreg, srcp.value);							// cvtss2sd dstreg,srcp
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd    dstp,dstreg
	}

	/* double-to-single */
	else if (inst->size == 4 && sizep.value == DRCUML_SIZE_QWORD)
	{
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_cvtsd2ss_r128_m64(&dst, dstreg, MABS(drcbe, srcp.value));				// cvtsd2ss dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_cvtsd2ss_r128_r128(&dst, dstreg, srcp.value);							// cvtsd2ss dstreg,srcp
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss    dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_frnds - process a FRNDS opcode
-------------------------------------------------*/

static x86code *op_frnds(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, NULL);

	/* 64-bit form */
	if (srcp.type == DRCUML_PTYPE_MEMORY)
		emit_cvtsd2ss_r128_m64(&dst, dstreg, MABS(drcbe, srcp.value));					// cvtsd2ss dstreg,[srcp]
	else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
		emit_cvtsd2ss_r128_r128(&dst, dstreg, srcp.value);								// cvtsd2ss dstreg,srcp
	emit_cvtss2sd_r128_r128(&dst, dstreg, dstreg);										// cvtss2sd dstreg,dstreg
	emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);									// movsd    dstp,dstreg
	return dst;
}


/*-------------------------------------------------
    op_fadd - process a FADD opcode
-------------------------------------------------*/

static x86code *op_fadd(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MF, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_movss_r128_p32(drcbe, &dst, dstreg, &src1p);								// movss dstreg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_addss_r128_m32(&dst, dstreg, MABS(drcbe, src2p.value));				// addss dstreg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_addss_r128_r128(&dst, dstreg, src2p.value);							// addss dstreg,src2p
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_movsd_r128_p64(drcbe, &dst, dstreg, &src1p);								// movsd dstreg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_addsd_r128_m64(&dst, dstreg, MABS(drcbe, src2p.value));				// addsd dstreg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_addsd_r128_r128(&dst, dstreg, src2p.value);							// addsd dstreg,src2p
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_fsub - process a FSUB opcode
-------------------------------------------------*/

static x86code *op_fsub(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_movss_r128_p32(drcbe, &dst, dstreg, &src1p);								// movss dstreg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_subss_r128_m32(&dst, dstreg, MABS(drcbe, src2p.value));				// subss dstreg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_subss_r128_r128(&dst, dstreg, src2p.value);							// subss dstreg,src2p
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_movsd_r128_p64(drcbe, &dst, dstreg, &src1p);								// movsd dstreg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_subsd_r128_m64(&dst, dstreg, MABS(drcbe, src2p.value));				// subsd dstreg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_subsd_r128_r128(&dst, dstreg, src2p.value);							// subsd dstreg,src2p
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_fcmp - process a FCMP opcode
-------------------------------------------------*/

static x86code *op_fcmp(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter src1p, src2p;
	int src1reg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_U);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* pick a target register for the general case */
	src1reg = param_select_register(REG_XMM0, &src1p, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_movss_r128_p32(drcbe, &dst, src1reg, &src1p);								// movss src1reg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_comiss_r128_m32(&dst, src1reg, MABS(drcbe, src2p.value));				// comiss src1reg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_comiss_r128_r128(&dst, src1reg, src2p.value);							// comiss src1reg,src2p
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_movsd_r128_p64(drcbe, &dst, src1reg, &src1p);								// movsd src1reg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_comisd_r128_m64(&dst, src1reg, MABS(drcbe, src2p.value));				// comisd src1reg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_comisd_r128_r128(&dst, src1reg, src2p.value);							// comisd src1reg,src2p
	}
	return dst;
}


/*-------------------------------------------------
    op_fmul - process a FMUL opcode
-------------------------------------------------*/

static x86code *op_fmul(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3_commutative(drcbe, inst, &dstp, PTYPE_MF, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_movss_r128_p32(drcbe, &dst, dstreg, &src1p);								// movss dstreg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_mulss_r128_m32(&dst, dstreg, MABS(drcbe, src2p.value));				// mulss dstreg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_mulss_r128_r128(&dst, dstreg, src2p.value);							// mulss dstreg,src2p
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_movsd_r128_p64(drcbe, &dst, dstreg, &src1p);								// movsd dstreg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_mulsd_r128_m64(&dst, dstreg, MABS(drcbe, src2p.value));				// mulsd dstreg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_mulsd_r128_r128(&dst, dstreg, src2p.value);							// mulsd dstreg,src2p
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_fdiv - process a FDIV opcode
-------------------------------------------------*/

static x86code *op_fdiv(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, src1p, src2p;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &src1p, PTYPE_MF, &src2p, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, &src2p);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_movss_r128_p32(drcbe, &dst, dstreg, &src1p);								// movss dstreg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_divss_r128_m32(&dst, dstreg, MABS(drcbe, src2p.value));				// divss dstreg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_divss_r128_r128(&dst, dstreg, src2p.value);							// divss dstreg,src2p
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_movsd_r128_p64(drcbe, &dst, dstreg, &src1p);								// movsd dstreg,src1p
		if (src2p.type == DRCUML_PTYPE_MEMORY)
			emit_divsd_r128_m64(&dst, dstreg, MABS(drcbe, src2p.value));				// divsd dstreg,[src2p]
		else if (src2p.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_divsd_r128_r128(&dst, dstreg, src2p.value);							// divsd dstreg,src2p
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_fneg - process a FNEG opcode
-------------------------------------------------*/

static x86code *op_fneg(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, &srcp);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_xorps_r128_r128(&dst, dstreg, dstreg);										// xorps dstreg,dstreg
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_subss_r128_m32(&dst, dstreg, MABS(drcbe, srcp.value));					// subss dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_subss_r128_r128(&dst, dstreg, srcp.value);								// subss dstreg,srcp
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_xorpd_r128_r128(&dst, dstreg, dstreg);										// xorpd dstreg,dstreg
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_subsd_r128_m64(&dst, dstreg, MABS(drcbe, srcp.value));					// subsd dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_subsd_r128_r128(&dst, dstreg, srcp.value);								// subsd dstreg,srcp
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_fabs - process a FABS opcode
-------------------------------------------------*/

static x86code *op_fabs(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, &srcp);

	/* 32-bit form */
	if (inst->size == 4)
	{
		emit_movss_r128_p32(drcbe, &dst, dstreg, &srcp);								// movss dstreg,srcp
		emit_andps_r128_m128(&dst, dstreg, MABS(drcbe, drcbe->absmask32));				// andps dstreg,[absmask32]
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_movsd_r128_p64(drcbe, &dst, dstreg, &srcp);								// movsd dstreg,srcp
		emit_andpd_r128_m128(&dst, dstreg, MABS(drcbe, drcbe->absmask64));				// andpd dstreg,[absmask64]
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_fsqrt - process a FSQRT opcode
-------------------------------------------------*/

static x86code *op_fsqrt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_sqrtss_r128_m32(&dst, dstreg, MABS(drcbe, srcp.value));				// sqrtss dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_sqrtss_r128_r128(&dst, dstreg, srcp.value);							// sqrtss dstreg,srcp
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_sqrtsd_r128_m64(&dst, dstreg, MABS(drcbe, srcp.value));				// sqrtsd dstreg,[srcp]
		else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
			emit_sqrtsd_r128_r128(&dst, dstreg, srcp.value);							// sqrtsd dstreg,srcp
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd dstp,dstreg
	}
	return dst;
}


/*-------------------------------------------------
    op_frecip - process a FRECIP opcode
-------------------------------------------------*/

static x86code *op_frecip(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		if (USE_RCPSS_FOR_SINGLES)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_rcpss_r128_m32(&dst, dstreg, MABS(drcbe, srcp.value));				// rcpss dstreg,[srcp]
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
				emit_rcpss_r128_r128(&dst, dstreg, srcp.value);							// rcpss dstreg,srcp
			emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);							// movss dstp,dstreg
		}
		else
		{
			emit_movss_r128_m32(&dst, REG_XMM1, MABS(drcbe, &drcbe->single1));			// movss xmm1,1.0
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_divss_r128_m32(&dst, REG_XMM1, MABS(drcbe, srcp.value));			// divss xmm1,[srcp]
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
				emit_divss_r128_r128(&dst, REG_XMM1, srcp.value);						// divss xmm1,srcp
			emit_movss_p32_r128(drcbe, &dst, &dstp, REG_XMM1);							// movss dstp,xmm1
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		if (USE_RCPSS_FOR_DOUBLES)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_cvtsd2ss_r128_m64(&dst, dstreg, MABS(drcbe, srcp.value));			// cvtsd2ss dstreg,[srcp]
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
				emit_cvtsd2ss_r128_r128(&dst, dstreg, srcp.value);						// cvtsd2ss dstreg,srcp
			emit_rcpss_r128_r128(&dst, dstreg, dstreg);									// rcpss dstreg,dstreg
			emit_cvtss2sd_r128_r128(&dst, dstreg, dstreg);								// cvtss2sd dstreg,dstreg
			emit_movsd_p64_r128(drcbe, &dst, &dstp, REG_XMM1);							// movsd dstp,dstreg
		}
		else
		{
			emit_movsd_r128_m64(&dst, REG_XMM1, MABS(drcbe, &drcbe->double1));			// movsd xmm1,1.0
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_divsd_r128_m64(&dst, REG_XMM1, MABS(drcbe, srcp.value));			// divsd xmm1,[srcp]
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
				emit_divsd_r128_r128(&dst, REG_XMM1, srcp.value);						// divsd xmm1,srcp
			emit_movsd_p64_r128(drcbe, &dst, &dstp, REG_XMM1);							// movsd dstp,xmm1
		}
	}
	return dst;
}


/*-------------------------------------------------
    op_frsqrt - process a FRSQRT opcode
-------------------------------------------------*/

static x86code *op_frsqrt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;
	int dstreg;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_XMM0, &dstp, NULL);

	/* 32-bit form */
	if (inst->size == 4)
	{
		if (USE_RSQRTSS_FOR_SINGLES)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_rsqrtss_r128_m32(&dst, dstreg, MABS(drcbe, srcp.value));			// rsqrtss dstreg,[srcp]
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
				emit_rsqrtss_r128_r128(&dst, dstreg, srcp.value);						// rsqrtss dstreg,srcp
		}
		else
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_sqrtss_r128_m32(&dst, REG_XMM1, MABS(drcbe, srcp.value));			// sqrtss xmm1,[srcp]
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
				emit_sqrtss_r128_r128(&dst, REG_XMM1, srcp.value);						// sqrtss xmm1,srcp
			emit_movss_r128_m32(&dst, dstreg, MABS(drcbe, &drcbe->single1));			// movss dstreg,1.0
			emit_divss_r128_r128(&dst, dstreg, REG_XMM1);								// divss dstreg,xmm1
		}
		emit_movss_p32_r128(drcbe, &dst, &dstp, dstreg);								// movss dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		if (USE_RSQRTSS_FOR_DOUBLES)
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_cvtsd2ss_r128_m64(&dst, dstreg, MABS(drcbe, srcp.value));			// cvtsd2ss dstreg,[srcp]
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
				emit_cvtsd2ss_r128_r128(&dst, dstreg, srcp.value);						// cvtsd2ss dstreg,srcp
			emit_rsqrtss_r128_r128(&dst, dstreg, dstreg);								// rsqrtss dstreg,dstreg
			emit_cvtss2sd_r128_r128(&dst, dstreg, dstreg);								// cvtss2sd dstreg,dstreg
		}
		else
		{
			if (srcp.type == DRCUML_PTYPE_MEMORY)
				emit_sqrtsd_r128_m64(&dst, REG_XMM1, MABS(drcbe, srcp.value));			// sqrtsd xmm1,[srcp]
			else if (srcp.type == DRCUML_PTYPE_FLOAT_REGISTER)
				emit_sqrtsd_r128_r128(&dst, REG_XMM1, srcp.value);						// sqrtsd xmm1,srcp
			emit_movsd_r128_m64(&dst, dstreg, MABS(drcbe, &drcbe->double1));			// movsd dstreg,1.0
			emit_divsd_r128_r128(&dst, dstreg, REG_XMM1);								// divsd dstreg,xmm1
		}
		emit_movsd_p64_r128(drcbe, &dst, &dstp, dstreg);								// movsd dstp,dstreg
	}
	return dst;
}
