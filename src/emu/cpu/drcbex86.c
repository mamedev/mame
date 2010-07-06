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

    * Convert SUB a,0,b to NEG

    * Optimize, e.g., and [r5],i0,$FF to use ebx as temporary register
        (avoid initial move) if i0 is not needed going forward

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
        [esp]      - param 0
        [esp+4]    - param 1
        [esp+8]    - param 2
        [esp+12]   - param 3
        [esp+16]   - param 4
        [esp+20]   - alignment
        [esp+24]   - alignment
        [esp+28]   - saved ebp
        [esp+32]   - saved edi
        [esp+36]   - saved esi
        [esp+40]   - saved ebx
        [esp+44]   - ret
        [esp+48]   - input parameter (entry handle)

***************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "drcuml.h"
#include "drcbeut.h"
#include "x86emit.h"
#include "x86log.h"



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

#define X86_CONDITION(condition)		(condition_map[condition - DRCUML_COND_Z])
#define X86_NOT_CONDITION(condition)	(condition_map[condition - DRCUML_COND_Z] ^ 1)

#define assert_no_condition(inst)		assert((inst)->condition == DRCUML_COND_ALWAYS)
#define assert_any_condition(inst)		assert((inst)->condition == DRCUML_COND_ALWAYS || ((inst)->condition >= DRCUML_COND_Z && (inst)->condition < DRCUML_COND_MAX))
#define assert_no_flags(inst)			assert((inst)->flags == 0)
#define assert_flags(inst, valid)		assert(((inst)->flags & ~(valid)) == 0)



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
	running_device *	device;					/* CPU device we are associated with */
	drcuml_state *			drcuml;					/* pointer back to our owner */
	drccache *				cache;					/* pointer to the cache */
	drcuml_machine_state	state;					/* state of the machine */
	drchash_state *			hash;					/* hash table state */
	drcmap_state *			map;					/* code map */
	drclabel_list *			labels;                 /* label list */

	x86_entry_point_func	entry;					/* entry point */
	x86code *				exit;					/* exit point */
	x86code *				nocode;					/* nocode handler */
	x86code *				save;					/* save handler */
	x86code *				restore;				/* restore handler */

	UINT32 *				reglo[REG_MAX];			/* pointer to low part of data for each register */
	UINT32 *				reghi[REG_MAX];			/* pointer to high part of data for each register */
	UINT8					last_lower_reg;			/* last register we stored a lower from */
	x86code *				last_lower_pc;			/* PC after instruction where we last stored a lower register */
	UINT32 *				last_lower_addr;		/* address where we last stored an lower register */
	UINT8					last_upper_reg;			/* last register we stored an upper from */
	x86code *				last_upper_pc;			/* PC after instruction where we last stored an upper register */
	UINT32 *				last_upper_addr;		/* address where we last stored an upper register */
	double					fptemp;					/* temporary storage for floating point */

	const address_space *	space[ADDRESS_SPACES];	/* address spaces */

	UINT8					sse3;					/* do we have SSE3 support? */
	UINT16					fpumode;				/* saved FPU mode */
	UINT16					fmodesave;				/* temporary location for saving */

	void *					stacksave;				/* saved stack pointer */
	void *					hashstacksave;			/* saved stack pointer for hashjmp */
	UINT64					reslo;					/* extended low result */
	UINT64					reshi;					/* extended high result */

	x86log_context *		log;					/* logging */
	UINT8					logged_common;			/* logged common code already? */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* primary back-end callbacks */
static drcbe_state *drcbex86_alloc(drcuml_state *drcuml, drccache *cache, running_device *device, UINT32 flags, int modes, int addrbits, int ignorebits);
static void drcbex86_free(drcbe_state *drcbe);
static void drcbex86_reset(drcbe_state *drcbe);
static int drcbex86_execute(drcbe_state *drcbe, drcuml_codehandle *entry);
static void drcbex86_generate(drcbe_state *drcbe, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst);
static int drcbex86_hash_exists(drcbe_state *drcbe, UINT32 mode, UINT32 pc);
static void drcbex86_get_info(drcbe_state *state, drcbe_info *info);

/* private helper functions */
static void fixup_label(void *parameter, drccodeptr labelcodeptr);

/* miscellaneous functions */
static int dmulu(UINT64 *dstlo, UINT64 *dsthi, UINT64 src1, UINT64 src2, int flags);
static int dmuls(UINT64 *dstlo, UINT64 *dsthi, INT64 src1, INT64 src2, int flags);
static int ddivu(UINT64 *dstlo, UINT64 *dsthi, UINT64 src1, UINT64 src2);
static int ddivs(UINT64 *dstlo, UINT64 *dsthi, INT64 src1, INT64 src2);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* globally-accessible interface to the backend */
extern const drcbe_interface drcbe_x86_be_interface =
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
static const UINT8 int_register_map[DRCUML_REG_I_END - DRCUML_REG_I0] =
{
	REG_EBX, REG_ESI, REG_EDI, REG_EBP
};

/* flags mapping tables */
static UINT8 flags_map[0x1000];
static UINT32 flags_unmap[0x20];

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

/* FPU control register mapping */
static const UINT16 fp_control[4] =
{
	0x0e3f, 	/* DRCUML_FMOD_TRUNC */
	0x023f, 	/* DRCUML_FMOD_ROUND */
	0x0a3f, 	/* DRCUML_FMOD_CEIL */
	0x063f		/* DRCUML_FMOD_FLOOR */
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
	{ DRCUML_OP_GETFLGS, op_getflgs },	/* GETFLGS dst,mask               */
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
	{ DRCUML_OP_FREAD,   op_fread },	/* FREAD   dst,src1,space         */
	{ DRCUML_OP_FWRITE,  op_fwrite },	/* FWRITE  dst,src1,space         */
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


/*-------------------------------------------------
    reset_last_upper_lower_reg - reset the last
    upper/lower register state
-------------------------------------------------*/

INLINE void reset_last_upper_lower_reg(drcbe_state *drcbe)
{
	drcbe->last_lower_reg = REG_NONE;
	drcbe->last_upper_reg = REG_NONE;
}


/*-------------------------------------------------
    set_last_lower_reg - note that we have just
    loaded a lower register
-------------------------------------------------*/

INLINE void set_last_lower_reg(drcbe_state *drcbe, x86code *dst, const drcuml_parameter *param, UINT8 reglo)
{
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		drcbe->last_lower_reg = reglo;
		drcbe->last_lower_addr = (UINT32 *)((FPTR)param->value);
		drcbe->last_lower_pc = dst;
	}
}


/*-------------------------------------------------
    set_last_upper_reg - note that we have just
    loaded an upper register
-------------------------------------------------*/

INLINE void set_last_upper_reg(drcbe_state *drcbe, x86code *dst, const drcuml_parameter *param, UINT8 reghi)
{
	drcbe->last_upper_reg = reghi;
	drcbe->last_upper_addr = (param->type == DRCUML_PTYPE_INT_REGISTER) ? drcbe->reghi[param->value] : (UINT32 *)((FPTR)param->value + 4);
	drcbe->last_upper_pc = dst;
}


/*-------------------------------------------------
    can_skip_lower_load - return TRUE if we can
    skip re-loading a lower half of a register
-------------------------------------------------*/

INLINE int can_skip_lower_load(drcbe_state *drcbe, x86code *dst, UINT32 *memptr, UINT8 reglo)
{
//  return FALSE;
	return (dst == drcbe->last_lower_pc && memptr == drcbe->last_lower_addr && reglo == drcbe->last_lower_reg);
}


/*-------------------------------------------------
    can_skip_upper_load - return TRUE if we can
    skip re-loading an upper half of a register
-------------------------------------------------*/

INLINE int can_skip_upper_load(drcbe_state *drcbe, x86code *dst, UINT32 *memptr, UINT8 reghi)
{
//  return FALSE;
	return (dst == drcbe->last_upper_pc && memptr == drcbe->last_upper_addr && reghi == drcbe->last_upper_reg);
}


/*-------------------------------------------------
    track_resolve_link - wrapper for resolve_link
    that resets all register tracking info
-------------------------------------------------*/

INLINE void track_resolve_link(drcbe_state *drcbe, x86code **destptr, const emit_link *linkinfo)
{
	reset_last_upper_lower_reg(drcbe);
	resolve_link(destptr, linkinfo);
}

#define resolve_link INVALID



/***************************************************************************
    BACKEND CALLBACKS
***************************************************************************/

/*-------------------------------------------------
    drcbex86_alloc - allocate back-end-specific
    state
-------------------------------------------------*/

static drcbe_state *drcbex86_alloc(drcuml_state *drcuml, drccache *cache, running_device *device, UINT32 flags, int modes, int addrbits, int ignorebits)
{
	int opnum, regnum, entry, spacenum;
	drcbe_state *drcbe;

	/* allocate space in the cache for our state */
	drcbe = (drcbe_state *)drccache_memory_alloc(cache, sizeof(*drcbe));
	if (drcbe == NULL)
		return NULL;
	memset(drcbe, 0, sizeof(*drcbe));

	/* remember our pointers */
	drcbe->device = device;
	drcbe->drcuml = drcuml;
	drcbe->cache = cache;

	/* get address spaces */
	for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		drcbe->space[spacenum] = downcast<cpu_device *>(device)->space(spacenum);

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
	int regnum;

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
	emit_sub_r32_imm(dst, REG_ESP, 24);													// sub   esp,24
	emit_mov_m32_r32(dst, MABS(&drcbe->hashstacksave), REG_ESP);						// mov   [hashstacksave],esp
	emit_sub_r32_imm(dst, REG_ESP, 4);													// sub   esp,4
	emit_mov_m32_r32(dst, MABS(&drcbe->stacksave), REG_ESP);							// mov   [stacksave],esp
	emit_fstcw_m16(dst, MABS(&drcbe->fpumode));											// fstcw [fpumode]
	emit_jmp_r32(dst, REG_EAX);															// jmp   eax
	if (drcbe->log != NULL && !drcbe->logged_common)
		x86log_disasm_code_range(drcbe->log, "entry_point", (x86code *)drcbe->entry, *dst);

	/* generate an exit point */
	drcbe->exit = *dst;
	emit_fldcw_m16(dst, MABS(&drcbe->fpumode));											// fldcw [fpumode]
	emit_mov_r32_m32(dst, REG_ESP, MABS(&drcbe->hashstacksave));						// mov   esp,[hashstacksave]
	emit_add_r32_imm(dst, REG_ESP, 24);													// add   esp,24
	emit_pop_r32(dst, REG_EBP);															// pop   ebp
	emit_pop_r32(dst, REG_EDI);															// pop   edi
	emit_pop_r32(dst, REG_ESI);															// pop   esi
	emit_pop_r32(dst, REG_EBX);															// pop   ebx
	emit_ret(dst);																		// ret
	if (drcbe->log != NULL && !drcbe->logged_common)
		x86log_disasm_code_range(drcbe->log, "exit_point", drcbe->exit, *dst);

	/* generate a no code point */
	drcbe->nocode = *dst;
	emit_ret(dst);																		// ret
	if (drcbe->log != NULL && !drcbe->logged_common)
		x86log_disasm_code_range(drcbe->log, "nocode", drcbe->nocode, *dst);

	/* generate a save subroutine */
	drcbe->save = *dst;
	emit_pushf(dst);																	// pushf
	emit_pop_r32(dst, REG_EAX);															// pop    eax
	emit_and_r32_imm(dst, REG_EAX, 0x8c5);												// and    eax,0x8c5
	emit_mov_r8_m8(dst, REG_AL, MBD(REG_EAX, flags_map));								// mov    al,[flags_map]
	emit_mov_m8_r8(dst, MBD(REG_ECX, offsetof(drcuml_machine_state, flags)), REG_AL);	// mov    state->flags,al
	emit_mov_r8_m8(dst, REG_AL, MABS(&drcbe->state.fmod));								// mov    al,[fmod]
	emit_mov_m8_r8(dst, MBD(REG_ECX, offsetof(drcuml_machine_state, fmod)), REG_AL);	// mov    state->fmod,al
	emit_mov_r32_m32(dst, REG_EAX, MABS(&drcbe->state.exp));							// mov    eax,[exp]
	emit_mov_m32_r32(dst, MBD(REG_ECX, offsetof(drcuml_machine_state, exp)), REG_EAX);	// mov    state->exp,eax
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.r); regnum++)
	{
		int regoffsl = (int)&((drcuml_machine_state *)NULL)->r[regnum].w.l;
		int regoffsh = (int)&((drcuml_machine_state *)NULL)->r[regnum].w.h;
		if (int_register_map[regnum] != 0)
			emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsl), int_register_map[regnum]);
		else
		{
			emit_mov_r32_m32(dst, REG_EAX, MABS(&drcbe->state.r[regnum].w.l));
			emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsl), REG_EAX);
		}
		emit_mov_r32_m32(dst, REG_EAX, MABS(&drcbe->state.r[regnum].w.h));
		emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsh), REG_EAX);
	}
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.f); regnum++)
	{
		int regoffsl = (int)&((drcuml_machine_state *)NULL)->f[regnum].s.l;
		int regoffsh = (int)&((drcuml_machine_state *)NULL)->f[regnum].s.h;
		emit_mov_r32_m32(dst, REG_EAX, MABS(&drcbe->state.f[regnum].s.l));
		emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsl), REG_EAX);
		emit_mov_r32_m32(dst, REG_EAX, MABS(&drcbe->state.f[regnum].s.h));
		emit_mov_m32_r32(dst, MBD(REG_ECX, regoffsh), REG_EAX);
	}
	emit_ret(dst);																		// ret
	if (drcbe->log != NULL && !drcbe->logged_common)
		x86log_disasm_code_range(drcbe->log, "save", drcbe->save, *dst);

	/* generate a restore subroutine */
	drcbe->restore = *dst;
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.r); regnum++)
	{
		int regoffsl = (int)&((drcuml_machine_state *)NULL)->r[regnum].w.l;
		int regoffsh = (int)&((drcuml_machine_state *)NULL)->r[regnum].w.h;
		if (int_register_map[regnum] != 0)
			emit_mov_r32_m32(dst, int_register_map[regnum], MBD(REG_ECX, regoffsl));
		else
		{
			emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, regoffsl));
			emit_mov_m32_r32(dst, MABS(&drcbe->state.r[regnum].w.l), REG_EAX);
		}
		emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, regoffsh));
		emit_mov_m32_r32(dst, MABS(&drcbe->state.r[regnum].w.h), REG_EAX);
	}
	for (regnum = 0; regnum < ARRAY_LENGTH(drcbe->state.f); regnum++)
	{
		int regoffsl = (int)&((drcuml_machine_state *)NULL)->f[regnum].s.l;
		int regoffsh = (int)&((drcuml_machine_state *)NULL)->f[regnum].s.h;
		emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, regoffsl));
		emit_mov_m32_r32(dst, MABS(&drcbe->state.f[regnum].s.l), REG_EAX);
		emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, regoffsh));
		emit_mov_m32_r32(dst, MABS(&drcbe->state.f[regnum].s.h), REG_EAX);
	}
	emit_movzx_r32_m8(dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, fmod)));// movzx eax,state->fmod
	emit_and_r32_imm(dst, REG_EAX, 3);													// and    eax,3
	emit_mov_m8_r8(dst, MABS(&drcbe->state.fmod), REG_AL);								// mov    [fmod],al
	emit_fldcw_m16(dst, MISD(REG_EAX, 2, &fp_control[0]));								// fldcw  fp_control[eax]
	emit_mov_r32_m32(dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, exp)));	// mov    eax,state->exp
	emit_mov_m32_r32(dst, MABS(&drcbe->state.exp), REG_EAX);							// mov    [exp],eax
	emit_movzx_r32_m8(dst, REG_EAX, MBD(REG_ECX, offsetof(drcuml_machine_state, flags)));// movzx eax,state->flags
	emit_push_m32(dst, MISD(REG_EAX, 4, flags_unmap));									// push   flags_unmap[eax*4]
	emit_popf(dst);																		// popf
	emit_ret(dst);																		// ret
	if (drcbe->log != NULL && !drcbe->logged_common)
		x86log_disasm_code_range(drcbe->log, "restore", drcbe->restore, *dst);

	/* finish up codegen */
	drccache_end_codegen(drcbe->cache);
	drcbe->logged_common = TRUE;

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
    emit_mov_r32_p32_keepflags - move a 32-bit
    parameter into a register without affecting
    any flags
-------------------------------------------------*/

static void emit_mov_r32_p32_keepflags(drcbe_state *drcbe, x86code **dst, UINT8 reg, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
		emit_mov_r32_imm(dst, reg, param->value);										// mov   reg,param
	else if (param->type == DRCUML_PTYPE_MEMORY)
	{
		if (!can_skip_lower_load(drcbe, *dst, (UINT32 *)((FPTR)param->value), reg))
			emit_mov_r32_m32(dst, reg, MABS(param->value));								// mov   reg,[param]
	}
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
		if (!can_skip_lower_load(drcbe, *dst, (UINT32 *)((FPTR)param->value), REG_EAX))
			emit_mov_r32_m32(dst, REG_EAX, MABS(param->value));							// mov   eax,[param]
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
	{
		emit_mov_m32_r32(dst, MABS(param->value), reg);									// mov   [param],reg
		set_last_lower_reg(drcbe, *dst, param, reg);
	}
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
		emit_cmp_r32_m32(dst, reg, MABS(param->value));									// cmp   reg,[param]
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			emit_mov_r32_imm(dst, reg, ~0);												// mov  reg,-1
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
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			emit_mov_m32_imm(dst, MEMPARAMS, ~0);										// mov   [dest],-1
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
    into a pair of registers
-------------------------------------------------*/

static void emit_mov_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (reglo == REG_NONE)
			;
		else if ((UINT32)param->value == 0)
			emit_xor_r32_r32(dst, reglo, reglo);										// xor   reglo,reglo
		else
			emit_mov_r32_imm(dst, reglo, param->value);									// mov   reglo,param
		if (reghi == REG_NONE)
			;
		else if ((UINT32)(param->value >> 32) == 0)
			emit_xor_r32_r32(dst, reghi, reghi);										// xor   reghi,reghi
		else
			emit_mov_r32_imm(dst, reghi, param->value >> 32);							// mov   reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
	{
		int skip_lower = can_skip_lower_load(drcbe, *dst, (UINT32 *)((FPTR)param->value), reglo);
		int skip_upper = can_skip_upper_load(drcbe, *dst, (UINT32 *)((FPTR)param->value + 4), reghi);
		if (reglo != REG_NONE && !skip_lower)
			emit_mov_r32_m32(dst, reglo, MABS(param->value));							// mov   reglo,[param]
		if (reghi != REG_NONE && !skip_upper)
			emit_mov_r32_m32(dst, reghi, MABS(param->value + 4));						// mov   reghi,[param+4]
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		int skip_upper = can_skip_upper_load(drcbe, *dst, drcbe->reghi[param->value], reghi);
		if (reglo != REG_NONE && reglo != param->value)
			emit_mov_r32_r32(dst, reglo, param->value);									// mov   reglo,param
		if (reghi != REG_NONE && !skip_upper)
			emit_mov_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));				// mov   reghi,reghi[param]
	}
}


/*-------------------------------------------------
    emit_mov_r64_p64_keepflags - move a 64-bit
    parameter into a pair of registers without
    affecting any flags
-------------------------------------------------*/

static void emit_mov_r64_p64_keepflags(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (reglo != REG_NONE)
			emit_mov_r32_imm(dst, reglo, param->value);									// mov   reglo,param
		if (reghi != REG_NONE)
			emit_mov_r32_imm(dst, reghi, param->value >> 32);							// mov   reghi,param >> 32
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
	{
		int skip_lower = can_skip_lower_load(drcbe, *dst, (UINT32 *)((FPTR)param->value), reglo);
		int skip_upper = can_skip_upper_load(drcbe, *dst, (UINT32 *)((FPTR)param->value + 4), reghi);
		if (reglo != REG_NONE && !skip_lower)
			emit_mov_r32_m32(dst, reglo, MABS(param->value));							// mov   reglo,[param]
		if (reghi != REG_NONE && !skip_upper)
			emit_mov_r32_m32(dst, reghi, MABS(param->value + 4));						// mov   reghi,[param+4]
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		int skip_upper = can_skip_upper_load(drcbe, *dst, drcbe->reghi[param->value], reghi);
		if (reglo != REG_NONE && reglo != param->value)
			emit_mov_r32_r32(dst, reglo, param->value);									// mov   reglo,param
		if (reghi != REG_NONE && !skip_upper)
			emit_mov_r32_m32(dst, reghi, MABS(drcbe->reghi[param->value]));				// mov   reghi,reghi[param]
	}
}


/*-------------------------------------------------
    emit_mov_m64_p64 - move a 64-bit parameter
    into a memory location
-------------------------------------------------*/

static void emit_mov_m64_p64(drcbe_state *drcbe, x86code **dst, DECLARE_MEMPARAMS, const drcuml_parameter *param)
{
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_mov_m32_imm(dst, MEMPARAMS + 0, param->value);								// mov   [mem],param
		emit_mov_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);						// mov   [mem],param >> 32
	}
	else if (param->type == DRCUML_PTYPE_MEMORY)
	{
		int skip_lower = can_skip_lower_load(drcbe, *dst, (UINT32 *)((FPTR)param->value), REG_EAX);
		if (!skip_lower)
			emit_mov_r32_m32(dst, REG_EAX, MABS(param->value));							// mov   eax,[param]
		emit_mov_m32_r32(dst, MEMPARAMS + 0, REG_EAX);									// mov   [mem],eax
		emit_mov_r32_m32(dst, REG_EAX, MABS(param->value + 4));							// mov   eax,[param+4]
		emit_mov_m32_r32(dst, MEMPARAMS + 4, REG_EAX);									// mov   [mem+4],eax
	}
	else if (param->type == DRCUML_PTYPE_INT_REGISTER)
	{
		emit_mov_m32_r32(dst, MEMPARAMS + 0, param->value);								// mov   [mem],param
		emit_mov_r32_m32(dst, REG_EAX, MABS(drcbe->reghi[param->value]));				// mov   eax,[param.hi]
		emit_mov_m32_r32(dst, MEMPARAMS + 4, REG_EAX);									// mov   [mem+4],eax
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
	set_last_lower_reg(drcbe, *dst, param, reglo);
	set_last_upper_reg(drcbe, *dst, param, reghi);
}


/*-------------------------------------------------
    emit_add_r64_p64 - add operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_add_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_adc_m32_imm(dst, MEMPARAMS, param->value);									// adc   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_adc_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);						// adc   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64_keepflags(drcbe, dst, reglo, REG_EDX, param);					// mov   edx:reglo,param
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_sbb_m32_imm(dst, MEMPARAMS, param->value);									// sbb   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_m32_imm(dst, MEMPARAMS + 4, param->value >> 32);						// sbb   [dest+4],param >> 32
	}
	else
	{
		int reglo = (param->type == DRCUML_PTYPE_INT_REGISTER) ? param->value : REG_EAX;
		emit_mov_r64_p64_keepflags(drcbe, dst, reglo, REG_EDX, param);					// mov   edx:reglo,param
		emit_sbb_m32_r32(dst, MEMPARAMS, reglo);										// sbb   [dest],reglo
		if (saveflags) emit_pushf(dst);													// pushf
		emit_sbb_m32_r32(dst, MEMPARAMS + 4, REG_EDX);									// sbb   [dest+4],edx
	}
	if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_cmp_r64_p64 - sub operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_cmp_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = (inst->flags != DRCUML_FLAG_Z && (inst->flags & DRCUML_FLAG_Z) != 0);
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
	if (inst->flags == DRCUML_FLAG_Z)
		emit_or_r32_r32(dst, reghi, reglo);												// or    reghi,reglo
	else if (saveflags)
		emit_combine_z_flags(dst);
}


/*-------------------------------------------------
    emit_and_r64_p64 - and operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_and_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_and_r32_m32(dst, reglo, MABS(param->value));								// and   reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_and_r32_m32(dst, reghi, MABS(param->value + 4));							// and   reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0)
			emit_xor_r32_r32(dst, reglo, reglo);										// xor   reglo,reglo
		else
			emit_and_r32_imm(dst, reglo, param->value);									// and   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0)
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0)
			emit_mov_m32_imm(dst, MEMPARAMS, 0);										// mov   [dest],0
		else
			emit_and_m32_imm(dst, MEMPARAMS, param->value);								// and   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0)
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_or_r32_m32(dst, reglo, MABS(param->value));								// or    reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_or_r32_m32(dst, reghi, MABS(param->value + 4));							// or    reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			emit_mov_r32_imm(dst, reglo, ~0);											// mov   reglo,-1
		else
			emit_or_r32_imm(dst, reglo, param->value);									// or    reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			emit_mov_r32_imm(dst, reghi, ~0);											// mov   reghi,-1
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			emit_mov_m32_imm(dst, MEMPARAMS, ~0);										// mov   [dest],-1
		else
			emit_or_m32_imm(dst, MEMPARAMS, param->value);								// or    [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
			emit_mov_m32_imm(dst, MEMPARAMS + 4, ~0);									// mov   [dest+4],-1
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_MEMORY)
	{
		emit_xor_r32_m32(dst, reglo, MABS(param->value));								// xor   reglo,[param]
		if (saveflags) emit_pushf(dst);													// pushf
		emit_xor_r32_m32(dst, reghi, MABS(param->value + 4));							// xor   reghi,[param]
	}
	else if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			emit_not_r32(dst, reglo);													// not   reglo
		else
			emit_xor_r32_imm(dst, reglo, param->value);									// xor   reglo,param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (inst->flags == 0 && (UINT32)param->value == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)param->value == 0xffffffff)
			emit_not_m32(dst, MEMPARAMS);												// not   [dest]
		else
			emit_xor_m32_imm(dst, MEMPARAMS, param->value);								// xor   [dest],param
		if (saveflags) emit_pushf(dst);													// pushf
		if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0)
			/* skip */;
		else if (inst->flags == 0 && (UINT32)(param->value >> 32) == 0xffffffff)
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
	int saveflags = (inst->flags != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->flags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->flags != 0)
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
			if (inst->flags != 0 || count > 0)
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
		if (inst->flags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shld_r32_r32_imm(dst, reghi, reglo, 31);								// shld  reghi,reglo,31
			emit_shl_r32_imm(dst, reglo, 31);											// shl   reglo,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shld_r32_r32_imm(dst, reghi, reglo, 31);								// shld  reghi,reglo,31
			emit_shl_r32_imm(dst, reglo, 31);											// shl   reglo,31
			track_resolve_link(drcbe, dst, &skip2);									// skip2:
		}
		else
		{
			emit_mov_r32_r32(dst, reghi, reglo);										// mov   reghi,reglo
			emit_xor_r32_r32(dst, reglo, reglo);										// xor   reglo,reglo
		}
		track_resolve_link(drcbe, dst, &skip1);										// skip1:
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->flags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->flags != 0)
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
			if (inst->flags != 0 || count > 0)
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
		if (inst->flags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_shr_r32_imm(dst, reghi, 31);											// shr   reghi,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_shr_r32_imm(dst, reghi, 31);											// shr   reghi,31
			track_resolve_link(drcbe, dst, &skip2);									// skip2:
		}
		else
		{
			emit_mov_r32_r32(dst, reglo, reghi);										// mov   reglo,reghi
			emit_xor_r32_r32(dst, reghi, reghi);										// xor   reghi,reghi
		}
		track_resolve_link(drcbe, dst, &skip1);										// skip1:
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->flags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->flags != 0)
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
			if (inst->flags != 0 || count > 0)
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
		if (inst->flags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_sar_r32_imm(dst, reghi, 31);											// sar   reghi,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_sar_r32_imm(dst, reghi, 31);											// sar   reghi,31
			track_resolve_link(drcbe, dst, &skip2);									// skip2:
		}
		else
		{
			emit_mov_r32_r32(dst, reglo, reghi);										// mov   reglo,reghi
			emit_sar_r32_imm(dst, reghi, 31);											// sar   reghi,31
		}
		track_resolve_link(drcbe, dst, &skip1);										// skip1:
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->flags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->flags != 0)
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
			if (inst->flags != 0 || count > 0)
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
		int tempreg = REG_EBX;
		emit_mov_m32_r32(dst, MBD(REG_ESP, -8), tempreg);                               // mov   [esp-8],ebx
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);											// test  ecx,0x20
		emit_jcc_short_link(dst, COND_Z, &skip1);										// jz    skip1
		if (inst->flags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_mov_r32_r32(dst, tempreg, reglo);										// mov   ebx,reglo
			emit_shld_r32_r32_imm(dst, reglo, reghi, 31);								// shld  reglo,reghi,31
			emit_shld_r32_r32_imm(dst, reghi, tempreg, 31);								// shld  reghi,ebx,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_mov_r32_r32(dst, tempreg, reglo);										// mov   ebx,reglo
			emit_shld_r32_r32_imm(dst, reglo, reghi, 31);								// shld  reglo,reghi,31
			emit_shld_r32_r32_imm(dst, reghi, tempreg, 31);								// shld  reghi,ebx,31
			track_resolve_link(drcbe, dst, &skip2);									// skip2:
		}
		else
			emit_xchg_r32_r32(dst, reghi, reglo);										// xchg  reghi,reglo
		track_resolve_link(drcbe, dst, &skip1);										// skip1:
		emit_mov_r32_r32(dst, tempreg, reglo);											// mov   ebx,reglo
		emit_shld_r32_r32_cl(dst, reglo, reghi);										// shld  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);													// pushf
		emit_shld_r32_r32_cl(dst, reghi, tempreg);										// shld  reghi,ebx,cl
		emit_mov_r32_m32(dst, tempreg, MBD(REG_ESP, saveflags ? -4 : -8));              // mov   ebx,[esp-8]
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	if (param->type == DRCUML_PTYPE_IMMEDIATE)
	{
		int count = param->value & 63;
		if (inst->flags == 0 && count == 0)
			/* skip */;
		else
		{
			while (count >= 32)
			{
				if (inst->flags != 0)
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
			if (inst->flags != 0 || count > 0)
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
		int tempreg = REG_EBX;
		emit_mov_m32_r32(dst, MBD(REG_ESP, -8), tempreg);                               // mov   [esp-8],ebx
		emit_mov_r32_p32(drcbe, dst, REG_ECX, param);									// mov   ecx,param
		emit_test_r32_imm(dst, REG_ECX, 0x20);											// test  ecx,0x20
		emit_jcc_short_link(dst, COND_Z, &skip1);										// jz    skip1
		if (inst->flags != 0)
		{
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_mov_r32_r32(dst, tempreg, reglo);										// mov   ebx,reglo
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_shrd_r32_r32_imm(dst, reghi, tempreg, 31);								// shrd  reghi,ebx,31
			emit_test_r32_imm(dst, REG_ECX, 0x20);										// test  ecx,0x20
			emit_jcc_short_link(dst, COND_Z, &skip2);									// jz    skip2
			emit_sub_r32_imm(dst, REG_ECX, 31);											// sub   ecx,31
			emit_mov_r32_r32(dst, tempreg, reglo);										// mov   ebx,reglo
			emit_shrd_r32_r32_imm(dst, reglo, reghi, 31);								// shrd  reglo,reghi,31
			emit_shrd_r32_r32_imm(dst, reghi, tempreg, 31);								// shrd  reghi,ebx,31
			track_resolve_link(drcbe, dst, &skip2);									// skip2:
		}
		else
			emit_xchg_r32_r32(dst, reghi, reglo);										// xchg  reghi,reglo
		track_resolve_link(drcbe, dst, &skip1);										// skip1:
		emit_mov_r32_r32(dst, tempreg, reglo);											// mov   ebx,reglo
		emit_shrd_r32_r32_cl(dst, reglo, reghi);										// shrd  reglo,reghi,cl
		if (saveflags) emit_pushf(dst);													// pushf
		emit_shrd_r32_r32_cl(dst, reghi, tempreg);										// shrd  reghi,ebx,cl
		emit_mov_r32_m32(dst, tempreg, MBD(REG_ESP, saveflags ? -4 : -8));              // mov   ebx,[esp-8]
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
	int saveflags = ((inst->flags & DRCUML_FLAG_Z) != 0);
	emit_link skipall, skiploop;
	x86code *loop;

	emit_mov_r32_p32_keepflags(drcbe, dst, REG_ECX, param);								// mov   ecx,param
	if (!saveflags)
	{
		loop = *dst;																// loop:
		emit_jecxz_link(dst, &skipall);													// jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));								// lea   ecx,[ecx-1]
		emit_rcl_r32_imm(dst, reglo, 1);												// rcl   reglo,1
		emit_rcl_r32_imm(dst, reghi, 1);												// rcl   reghi,1
		emit_jmp(dst, loop);															// jmp   loop
		track_resolve_link(drcbe, dst, &skipall);									// skipall:
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
		track_resolve_link(drcbe, dst, &skiploop);									// skiploop:
		emit_rcl_r32_imm(dst, reglo, 1);												// rcl   reglo,1
		emit_pushf(dst);																// pushf
		emit_rcl_r32_imm(dst, reghi, 1);												// rcl   reghi,1
		track_resolve_link(drcbe, dst, &skipall);									// skipall:
		emit_combine_z_flags(dst);
	}
}


/*-------------------------------------------------
    emit_rcr_r64_p64 - rcr operation to a 64-bit
    pair of registers from a 64-bit parameter
-------------------------------------------------*/

static void emit_rcr_r64_p64(drcbe_state *drcbe, x86code **dst, UINT8 reglo, UINT8 reghi, const drcuml_parameter *param, const drcuml_instruction *inst)
{
	int saveflags = (inst->flags != 0);
	emit_link skipall, skiploop;
	x86code *loop;

	emit_mov_r32_p32_keepflags(drcbe, dst, REG_ECX, param);								// mov   ecx,param
	if (!saveflags)
	{
		loop = *dst;																// loop:
		emit_jecxz_link(dst, &skipall);													// jecxz skipall
		emit_lea_r32_m32(dst, REG_ECX, MBD(REG_ECX, -1));								// lea   ecx,[ecx-1]
		emit_rcr_r32_imm(dst, reghi, 1);												// rcr   reghi,1
		emit_rcr_r32_imm(dst, reglo, 1);												// rcr   reglo,1
		emit_jmp(dst, loop);															// jmp   loop
		track_resolve_link(drcbe, dst, &skipall);									// skipall:
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
		track_resolve_link(drcbe, dst, &skiploop);									// skiploop:
		emit_rcr_r32_imm(dst, reghi, 1);												// rcr   reghi,1
		emit_pushf(dst);																// pushf
		emit_rcr_r32_imm(dst, reglo, 1);												// rcr   reglo,1
		track_resolve_link(drcbe, dst, &skipall);									// skipall:
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

	reset_last_upper_lower_reg(drcbe);

	/* emit a jump around the stack adjust in case code falls through here */
	emit_jmp_short_link(&dst, &skip);													// jmp   skip

	/* register the current pointer for the handle */
	drcuml_handle_set_codeptr((drcuml_codehandle *)(FPTR)inst->param[0].value, dst);

	/* by default, the handle points to prolog code that moves the stack pointer */
	emit_lea_r32_m32(&dst, REG_ESP, MBD(REG_ESP, -28));									// lea   rsp,[rsp-28]
	track_resolve_link(drcbe, &dst, &skip);											// skip:
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
	reset_last_upper_lower_reg(drcbe);
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
	reset_last_upper_lower_reg(drcbe);
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
		emit_test_m32_imm(&dst, MABS(&drcbe->device->machine->debug_flags), DEBUG_FLAG_CALL_HOOK);		// test  [debug_flags],DEBUG_FLAG_CALL_HOOK
		emit_jcc_short_link(&dst, COND_Z, &skip);										// jz    skip

		/* push the parameter */
		emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 4), &pcp);							// mov   [esp+4],pcp
		emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (FPTR)drcbe->device);					// mov   [esp],device
		emit_call(&dst, (x86code *)debugger_instruction_hook);							// call  debug_cpu_instruction_hook

		track_resolve_link(drcbe, &dst, &skip);										// skip:
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
		emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 4), &pcp);
		emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 0), &modep);
		emit_call(&dst, (x86code *)debug_log_hashjmp);
	}

	/* load the stack base one word early so we end up at the right spot after our call below */
	emit_mov_r32_m32(&dst, REG_ESP, MABS(&drcbe->hashstacksave));						// mov   esp,[hashstacksave]

	/* fixed mode cases */
	if (modep.type == DRCUML_PTYPE_IMMEDIATE && drcbe->hash->base[modep.value] != drcbe->hash->emptyl1)
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
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &labelp, PTYPE_I);

	/* look up the jump target and jump there */
	jmptarget = (x86code *)drclabel_get_codeptr(drcbe->labels, labelp.value, fixup_label, dst);
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
	targetptr = drcuml_handle_codeptr_addr((drcuml_codehandle *)(FPTR)handp.value);

	/* perform the exception processing inline if unconditional */
	if (inst->condition == DRCUML_COND_ALWAYS)
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
		emit_jcc(&dst, X86_CONDITION(inst->condition), 0);								// jcc   exception
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
	targetptr = drcuml_handle_codeptr_addr((drcuml_codehandle *)(FPTR)handp.value);

	/* skip if conditional */
	if (inst->condition != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condition), &skip);			// jcc   skip

	/* jump through the handle; directly if a normal jump */
	if (*targetptr != NULL)
		emit_call(&dst, *targetptr);													// call  *targetptr
	else
		emit_call_m32(&dst, MABS(targetptr));											// call  [targetptr]

	/* resolve the conditional link */
	if (inst->condition != DRCUML_COND_ALWAYS)
		track_resolve_link(drcbe, &dst, &skip);										// skip:
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
	emit_lea_r32_m32(&dst, REG_ESP, MBD(REG_ESP, 28));									// lea   rsp,[rsp+28]
	emit_ret(&dst);																		// ret

	/* resolve the conditional link */
	if (inst->condition != DRCUML_COND_ALWAYS)
		track_resolve_link(drcbe, &dst, &skip);										// skip:
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
	emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), paramp.value);								// mov   [esp],paramp
	emit_call(&dst, (x86code *)(FPTR)funcp.value);										// call  funcp

	/* resolve the conditional link */
	if (inst->condition != DRCUML_COND_ALWAYS)
		track_resolve_link(drcbe, &dst, &skip);										// skip:
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
	emit_mov_r32_m32(&dst, REG_EAX, MABS(&drcbe->stacksave));							// mov   eax,stacksave
	emit_mov_r32_m32(&dst, REG_EAX, MBD(REG_EAX, -4));									// mov   eax,[eax-4]
	emit_sub_r32_imm(&dst, REG_EAX, 1);													// sub   eax,1
	emit_mov_m32_imm(&dst, MBD(REG_ESP, 8), inst->param[1].value);						// mov   [esp+8],param1
	emit_mov_m32_r32(&dst, MBD(REG_ESP, 4), REG_EAX);									// mov   [esp+4],eax
	emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (FPTR)drcbe->map);							// mov   [esp],drcbe->map
	emit_call(&dst, (x86code *)drcmap_get_value);										// call  drcmap_get_value
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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
			emit_pop_r32(&dst, REG_EAX);												// pop    eax
			emit_and_r32_imm(&dst, REG_EAX, flagmask);									// and    eax,flagmask
			emit_movzx_r32_m8(&dst, dstreg, MBD(REG_EAX, flags_map));					// movzx  dstreg,[flags_map]
			break;
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
    op_save - process a SAVE opcode
-------------------------------------------------*/

static x86code *op_save(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;

	/* validate instruction */
	assert(inst->size == 4);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_M);

	/* copy live state to the destination */
	emit_mov_r32_imm(&dst, REG_ECX, dstp.value);										// mov    ecx,dstp
	emit_call(&dst, drcbe->save);														// call   save
	return dst;
}


/*-------------------------------------------------
    op_restore - process a RESTORE opcode
-------------------------------------------------*/

static x86code *op_restore(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp;

	/* validate instruction */
	assert(inst->size == 4);
	assert_no_condition(inst);

	/* normalize parameters */
	param_normalize_1(drcbe, inst, &dstp, PTYPE_M);

	/* copy live state from the destination */
	emit_mov_r32_imm(&dst, REG_ECX, dstp.value);										// mov    ecx,dstp
	emit_call(&dst, drcbe->restore);													// call   restore
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
	int dstreg, scale, size;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI, &scalesizep, PTYPE_I);
	scale = 1 << (scalesizep.value / 16);
	size = scalesizep.value % 16;

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (size == DRCUML_SIZE_BYTE)
			emit_movzx_r32_m8(&dst, dstreg, MABS(basep.value + scale*indp.value));		// movzx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_WORD)
			emit_movzx_r32_m16(&dst, dstreg, MABS(basep.value + scale*indp.value));		// movzx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_DWORD)
			emit_mov_r32_m32(&dst, dstreg, MABS(basep.value + scale*indp.value));		// mov   dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_QWORD)
		{
			emit_mov_r32_m32(&dst, REG_EDX, MABS(basep.value + scale*indp.value + 4));	// mov   edx,[basep + scale*indp + 4]
			emit_mov_r32_m32(&dst, dstreg, MABS(basep.value + scale*indp.value));		// mov   dstreg,[basep + scale*indp]
		}
	}

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		if (size == DRCUML_SIZE_BYTE)
			emit_movzx_r32_m8(&dst, dstreg, MISD(indreg, scale, basep.value));			// movzx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_WORD)
			emit_movzx_r32_m16(&dst, dstreg, MISD(indreg, scale, basep.value));			// movzx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_DWORD)
			emit_mov_r32_m32(&dst, dstreg, MISD(indreg, scale, basep.value));			// mov   dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_QWORD)
		{
			emit_mov_r32_m32(&dst, REG_EDX, MISD(indreg, scale, basep.value + 4));		// mov   edx,[basep + scale*indp + 4]
			emit_mov_r32_m32(&dst, dstreg, MISD(indreg, scale, basep.value));			// mov   dstreg,[basep + scale*indp]
		}
	}

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov   dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* 1, 2, or 4-byte case */
		if (size != DRCUML_SIZE_QWORD)
		{
			if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);						// mov   [dstp+4],0
			else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);				// mov   [reghi],0
		}

		/* 8-byte case */
		else
		{
			if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_r32(&dst, MABS(dstp.value + 4), REG_EDX);					// mov   [dstp+4],edx
			else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_EDX);		// mov   [reghi],edx
			set_last_upper_reg(drcbe, dst, &dstp, REG_EDX);
		}
	}
	set_last_lower_reg(drcbe, dst, &dstp, dstreg);
	return dst;
}


/*-------------------------------------------------
    op_loads - process a LOADS opcode
-------------------------------------------------*/

static x86code *op_loads(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, basep, indp, scalesizep;
	int dstreg, scale, size;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &basep, PTYPE_M, &indp, PTYPE_MRI, &scalesizep, PTYPE_I);
	scale = 1 << (scalesizep.value / 16);
	size = scalesizep.value % 16;

	/* pick a target register for the general case */
	dstreg = param_select_register(REG_EAX, &dstp, NULL);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		if (size == DRCUML_SIZE_BYTE)
			emit_movsx_r32_m8(&dst, dstreg, MABS(basep.value + scale*indp.value));		// movsx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_WORD)
			emit_movsx_r32_m16(&dst, dstreg, MABS(basep.value + scale*indp.value));		// movsx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_DWORD)
			emit_mov_r32_m32(&dst, dstreg, MABS(basep.value + scale*indp.value));		// mov   dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_QWORD)
		{
			emit_mov_r32_m32(&dst, REG_EDX, MABS(basep.value + scale*indp.value + 4));	// mov   edx,[basep + scale*indp + 4]
			emit_mov_r32_m32(&dst, dstreg, MABS(basep.value + scale*indp.value));		// mov   dstreg,[basep + scale*indp]
		}
	}

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		if (size == DRCUML_SIZE_BYTE)
			emit_movsx_r32_m8(&dst, dstreg, MISD(indreg, scale, basep.value));			// movsx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_WORD)
			emit_movsx_r32_m16(&dst, dstreg, MISD(indreg, scale, basep.value));			// movsx dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_DWORD)
			emit_mov_r32_m32(&dst, dstreg, MISD(indreg, scale, basep.value));			// mov   dstreg,[basep + scale*indp]
		else if (size == DRCUML_SIZE_QWORD)
		{
			emit_mov_r32_m32(&dst, REG_EDX, MISD(indreg, scale, basep.value + 4));		// mov   edx,[basep + scale*indp + 4]
			emit_mov_r32_m32(&dst, dstreg, MISD(indreg, scale, basep.value));			// mov   dstreg,[basep + scale*indp]
		}
	}

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov   dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		emit_cdq(&dst);																	// cdq
		if (dstp.type == DRCUML_PTYPE_MEMORY)
			emit_mov_m32_r32(&dst, MABS(dstp.value + 4), REG_EDX);						// mov   [dstp+4],edx
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			emit_mov_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_EDX);			// mov   [reghi],edx
		set_last_upper_reg(drcbe, dst, &dstp, REG_EDX);
	}
	set_last_lower_reg(drcbe, dst, &dstp, dstreg);
	return dst;
}


/*-------------------------------------------------
    op_store - process a STORE opcode
-------------------------------------------------*/

static x86code *op_store(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter srcp, basep, indp, scalesizep;
	int srcreg, scale, size;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &basep, PTYPE_M, &indp, PTYPE_MRI, &srcp, PTYPE_MRI, &scalesizep, PTYPE_I);
	scale = 1 << (scalesizep.value / 16);
	size = scalesizep.value % 16;

	/* pick a source register for the general case */
	srcreg = param_select_register(REG_EAX, &srcp, NULL);
	if (size == DRCUML_SIZE_BYTE && (srcreg & 4))
		srcreg = REG_EAX;

	/* degenerate case: constant index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		/* immediate source */
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (size == DRCUML_SIZE_BYTE)
				emit_mov_m8_imm(&dst, MABS(basep.value + scale*indp.value), srcp.value);	// mov   [basep + scale*indp],srcp
			else if (size == DRCUML_SIZE_WORD)
				emit_mov_m16_imm(&dst, MABS(basep.value + scale*indp.value), srcp.value);	// mov   [basep + scale*indp],srcp
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_m32_imm(&dst, MABS(basep.value + scale*indp.value), srcp.value);	// mov   [basep + scale*indp],srcp
			else if (size == DRCUML_SIZE_QWORD)
			{
				emit_mov_m32_imm(&dst, MABS(basep.value + scale*indp.value), srcp.value);	// mov   [basep + scale*indp],srcp
				emit_mov_m32_imm(&dst, MABS(basep.value + scale*indp.value + 4), srcp.value >> 32);
																						// mov   [basep + scale*indp + 4],srcp >> 32
			}
		}

		/* variable source */
		else
		{
			if (size != DRCUML_SIZE_QWORD)
				emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);							// mov   srcreg,srcp
			else
				emit_mov_r64_p64(drcbe, &dst, srcreg, REG_EDX, &srcp);					// mov   edx:srcreg,srcp
			if (size == DRCUML_SIZE_BYTE)
				emit_mov_m8_r8(&dst, MABS(basep.value + scale*indp.value), srcreg);		// mov   [basep + scale*indp],srcreg
			else if (size == DRCUML_SIZE_WORD)
				emit_mov_m16_r16(&dst, MABS(basep.value + scale*indp.value), srcreg);	// mov   [basep + scale*indp],srcreg
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_m32_r32(&dst, MABS(basep.value + scale*indp.value), srcreg);	// mov   [basep + scale*indp],srcreg
			else if (size == DRCUML_SIZE_QWORD)
			{
				emit_mov_m32_r32(&dst, MABS(basep.value + scale*indp.value), srcreg);	// mov   [basep + scale*indp],srcreg
				emit_mov_m32_r32(&dst, MABS(basep.value + scale*indp.value + 4), REG_EDX);	// mov   [basep + scale*indp + 4],edx
			}
		}
	}

	/* normal case: variable index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);									// mov   indreg,indp

		/* immediate source */
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			if (size == DRCUML_SIZE_BYTE)
				emit_mov_m8_imm(&dst, MISD(indreg, scale, basep.value), srcp.value);	// mov   [basep + 1*ecx],srcp
			else if (size == DRCUML_SIZE_WORD)
				emit_mov_m16_imm(&dst, MISD(indreg, scale, basep.value), srcp.value);	// mov   [basep + 2*ecx],srcp
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_m32_imm(&dst, MISD(indreg, scale, basep.value), srcp.value);	// mov   [basep + 4*ecx],srcp
			else if (size == DRCUML_SIZE_QWORD)
			{
				emit_mov_m32_imm(&dst, MISD(indreg, scale, basep.value), srcp.value);	// mov   [basep + 8*ecx],srcp
				emit_mov_m32_imm(&dst, MISD(indreg, scale, basep.value + 4), srcp.value >> 32);
																						// mov   [basep + 8*ecx + 4],srcp >> 32
			}
		}

		/* variable source */
		else
		{
			if (size != DRCUML_SIZE_QWORD)
				emit_mov_r32_p32(drcbe, &dst, srcreg, &srcp);							// mov   srcreg,srcp
			else
				emit_mov_r64_p64(drcbe, &dst, srcreg, REG_EDX, &srcp);					// mov   edx:srcreg,srcp
			if (size == DRCUML_SIZE_BYTE)
				emit_mov_m8_r8(&dst, MISD(indreg, scale, basep.value), srcreg);			// mov   [basep + 1*ecx],srcreg
			else if (size == DRCUML_SIZE_WORD)
				emit_mov_m16_r16(&dst, MISD(indreg, scale, basep.value), srcreg);		// mov   [basep + 2*ecx],srcreg
			else if (size == DRCUML_SIZE_DWORD)
				emit_mov_m32_r32(&dst, MISD(indreg, scale, basep.value), srcreg);		// mov   [basep + 4*ecx],srcreg
			else if (size == DRCUML_SIZE_QWORD)
			{
				emit_mov_m32_r32(&dst, MISD(indreg, scale, basep.value), srcreg);		// mov   [basep + 8*ecx],srcreg
				emit_mov_m32_r32(&dst, MISD(indreg, scale, basep.value + 4), REG_EDX);	// mov   [basep + 8*ecx],edx
			}
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
	emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 4), &addrp);								// mov    [esp+4],addrp
	emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (UINT32)drcbe->space[spacesizep.value / 16]);// mov    [esp],space
	if ((spacesizep.value & 3) == DRCUML_SIZE_BYTE)
	{
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.read_byte);
																						// call   read_byte
		emit_movzx_r32_r8(&dst, dstreg, REG_AL);										// movzx  dstreg,al
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_WORD)
	{
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.read_word);
																						// call   read_word
		emit_movzx_r32_r16(&dst, dstreg, REG_AX);										// movzx  dstreg,ax
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_DWORD)
	{
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.read_dword);
																						// call   read_dword
		emit_mov_r32_r32(&dst, dstreg, REG_EAX);										// mov    dstreg,eax
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_QWORD)
	{
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.read_qword);
																						// call   read_qword
		emit_mov_r32_r32(&dst, dstreg, REG_EAX);										// mov    dstreg,eax
	}

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov    dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* 1, 2, or 4-byte case */
		if ((spacesizep.value & 3) != DRCUML_SIZE_QWORD)
		{
			if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);						// mov   [dstp+4],0
			else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);				// mov   [reghi],0
		}

		/* 8-byte case */
		else
		{
			if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_r32(&dst, MABS(dstp.value + 4), REG_EDX);					// mov   [dstp+4],edx
			else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_EDX);		// mov   [reghi],edx
		}
	}
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
	if ((spacesizep.value & 3) != DRCUML_SIZE_QWORD)
		emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 8), &maskp);							// mov    [esp+8],maskp
	else
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 8), &maskp);							// mov    [esp+8],maskp
	emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 4), &addrp);								// mov    [esp+4],addrp
	emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (UINT32)drcbe->space[spacesizep.value / 16]);// mov    [esp],space
	if ((spacesizep.value & 3) == DRCUML_SIZE_WORD)
	{
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.read_word_masked);
																						// call   read_word_masked
		emit_movzx_r32_r16(&dst, dstreg, REG_AX);										// movzx  dstreg,ax
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_DWORD)
	{
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.read_dword_masked);
																						// call   read_dword_masked
		emit_mov_r32_r32(&dst, dstreg, REG_EAX);										// mov    dstreg,eax
	}
	else if ((spacesizep.value & 3) == DRCUML_SIZE_QWORD)
	{
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.read_qword_masked);
																						// call   read_qword_masked
		emit_mov_r32_r32(&dst, dstreg, REG_EAX);										// mov    dstreg,eax
	}

	/* store low 32 bits */
	emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);										// mov    dstp,dstreg

	/* 64-bit form stores upper 32 bits */
	if (inst->size == 8)
	{
		/* 1, 2, or 4-byte case */
		if ((spacesizep.value & 3) != DRCUML_SIZE_QWORD)
		{
			if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);						// mov   [dstp+4],0
			else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);				// mov   [reghi],0
		}

		/* 8-byte case */
		else
		{
			if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_r32(&dst, MABS(dstp.value + 4), REG_EDX);					// mov   [dstp+4],edx
			else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_EDX);		// mov   [reghi],edx
		}
	}
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
	if ((spacesizep.value & 3) != DRCUML_SIZE_QWORD)
		emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 8), &srcp);							// mov    [esp+8],srcp
	else
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 8), &srcp);							// mov    [esp+8],srcp
	emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 4), &addrp);								// mov    [esp+4],addrp
	emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (UINT32)drcbe->space[spacesizep.value / 16]);// mov    [esp],space
	if ((spacesizep.value & 3) == DRCUML_SIZE_BYTE)
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.write_byte);
																						// call   write_byte
	else if ((spacesizep.value & 3) == DRCUML_SIZE_WORD)
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.write_word);
																						// call   write_word
	else if ((spacesizep.value & 3) == DRCUML_SIZE_DWORD)
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.write_dword);
																						// call   write_dword
	else if ((spacesizep.value & 3) == DRCUML_SIZE_QWORD)
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.write_qword);
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
	if ((spacesizep.value & 3) != DRCUML_SIZE_QWORD)
	{
		emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 12), &maskp);						// mov    [esp+12],maskp
		emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 8), &srcp);							// mov    [esp+8],srcp
	}
	else
	{
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 16), &maskp);						// mov    [esp+16],maskp
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 8), &srcp);							// mov    [esp+8],srcp
	}
	emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 4), &addrp);								// mov    [esp+4],addrp
	emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (UINT32)drcbe->space[spacesizep.value / 16]);// mov    [esp],space
	if ((spacesizep.value & 3) == DRCUML_SIZE_WORD)
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.write_word_masked);
																						// call   write_word_masked
	else if ((spacesizep.value & 3) == DRCUML_SIZE_DWORD)
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.write_dword_masked);
																						// call   write_dword_masked
	else if ((spacesizep.value & 3) == DRCUML_SIZE_QWORD)
		emit_call(&dst, (x86code *)drcbe->space[spacesizep.value / 16]->accessors.write_qword_masked);
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
			emit_mov_m32_r32(&dst, MABS(dstp.value), srcp.value);						// mov   [dstp],srcp

		/* immediate to memory */
		else if (dstp.type == DRCUML_PTYPE_MEMORY && srcp.type == DRCUML_PTYPE_IMMEDIATE)
			emit_mov_m32_imm(&dst, MABS(dstp.value), srcp.value);						// mov   [dstp],srcp

		/* conditional memory to register */
		else if (inst->condition != DRCUML_COND_ALWAYS && dstp.type == DRCUML_PTYPE_INT_REGISTER && srcp.type == DRCUML_PTYPE_MEMORY)
		{
			dst = savedst;
			skip.target = NULL;
			emit_cmovcc_r32_m32(&dst, X86_CONDITION(inst->condition), dstp.value, MABS(srcp.value));
																						// cmovcc dstp,[srcp]
		}

		/* conditional register to register */
		else if (inst->condition != DRCUML_COND_ALWAYS && dstp.type == DRCUML_PTYPE_INT_REGISTER && srcp.type == DRCUML_PTYPE_INT_REGISTER)
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
		track_resolve_link(drcbe, &dst, &skip);
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
	dstreg = (inst->size == 8) ? REG_EAX : param_select_register(REG_EAX, &dstp, NULL);

	/* convert 8-bit source registers to EAX */
	if (sizep.value == DRCUML_SIZE_BYTE && srcp.type == DRCUML_PTYPE_INT_REGISTER && (srcp.value & 4))
	{
		emit_mov_r32_r32(&dst, REG_EAX, srcp.value);									// mov   eax,srcp
		srcp.value = REG_EAX;
	}

	/* general case */
	if (srcp.type == DRCUML_PTYPE_MEMORY)
	{
		if (sizep.value == DRCUML_SIZE_BYTE)
			emit_movsx_r32_m8(&dst, dstreg, MABS(srcp.value));							// movsx dstreg,[srcp]
		else if (sizep.value == DRCUML_SIZE_WORD)
			emit_movsx_r32_m16(&dst, dstreg, MABS(srcp.value));							// movsx dstreg,[srcp]
		else if (sizep.value == DRCUML_SIZE_DWORD)
			emit_mov_r32_m32(&dst, dstreg, MABS(srcp.value));							// mov   dstreg,[srcp]
	}
	else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
	{
		if (sizep.value == DRCUML_SIZE_BYTE)
			emit_movsx_r32_r8(&dst, dstreg, srcp.value);								// movsx dstreg,srcp
		else if (sizep.value == DRCUML_SIZE_WORD)
			emit_movsx_r32_r16(&dst, dstreg, srcp.value);								// movsx dstreg,srcp
		else if (sizep.value == DRCUML_SIZE_DWORD && dstreg != srcp.value)
			emit_mov_r32_r32(&dst, dstreg, srcp.value);									// mov   dstreg,srcp
	}
	if (inst->flags != 0)
		emit_test_r32_r32(&dst, dstreg, dstreg);										// test  dstreg,dstreg

	/* 32-bit form: store the low 32 bits */
	if (inst->size == 4)
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg

	/* 64-bit form: sign extend to 64 bits and store edx:eax */
	else if (inst->size == 8)
	{
		emit_cdq(&dst);																	// cdq
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
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
		emit_mov_r64_p64(drcbe, &dst, dstreg, REG_EDX, &srcp);							// mov   edx:dstreg,srcp
		emit_rol_r64_p64(drcbe, &dst, dstreg, REG_EDX, &shiftp, inst);					// rol   edx:dstreg,shiftp
		emit_and_r64_p64(drcbe, &dst, dstreg, REG_EDX, &maskp, inst);					// and   edx:dstreg,maskp
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,edx:dstreg
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
			int tempreg = REG_EBX;
			emit_mov_m32_r32(&dst, MBD(REG_ESP, -8), tempreg);							// mov   [esp-8],ebx
			emit_mov_r64_p64(drcbe, &dst, tempreg, REG_ECX, &maskp);					// mov   ecx:ebx,maskp
			emit_and_r32_r32(&dst, REG_EAX, tempreg);									// and   eax,ebx
			emit_and_r32_r32(&dst, REG_EDX, REG_ECX);									// and   edx,ecx
			emit_not_r32(&dst, tempreg);												// not   ebx
			emit_not_r32(&dst, REG_ECX);												// not   ecx
			if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
			{
				emit_and_r32_r32(&dst, dstp.value, tempreg);							// and   dstp.lo,ebx
				emit_and_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_ECX);		// and   dstp.hi,ecx
				emit_or_r32_r32(&dst, dstp.value, REG_EAX);								// or    dstp.lo,eax
				emit_or_m32_r32(&dst, MABS(drcbe->reghi[dstp.value]), REG_EDX);			// or    dstp.hi,edx
			}
			else
			{
				emit_and_m32_r32(&dst, MABS(dstp.value), tempreg);						// and   dstp.lo,ebx
				emit_and_m32_r32(&dst, MABS(dstp.value + 4), REG_ECX);					// and   dstp.hi,ecx
				emit_or_m32_r32(&dst, MABS(dstp.value), REG_EAX);						// or    dstp.lo,eax
				emit_or_m32_r32(&dst, MABS(dstp.value + 4), REG_EDX);					// or    dstp.hi,edx
			}
			emit_mov_r32_m32(&dst, tempreg, MBD(REG_ESP, -8));							// mov   ebx,[esp-8]
		}
		if (inst->flags == DRCUML_FLAG_Z)
			emit_or_r32_r32(&dst, REG_EAX, REG_EDX);									// or    eax,edx
		else if (inst->flags == DRCUML_FLAG_S)
			;// do nothing -- final OR will have the right result
		else if (inst->flags == (DRCUML_FLAG_Z | DRCUML_FLAG_S))
		{
			emit_movzx_r32_r16(&dst, REG_ECX, REG_AX);									// movzx ecx,ax
			emit_shr_r32_imm(&dst, REG_EAX, 16);										// shr   eax,16
			emit_or_r32_r32(&dst, REG_EDX, REG_ECX);									// or    edx,ecx
			emit_or_r32_r32(&dst, REG_EDX, REG_EAX);									// or    edx,eax
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
			emit_add_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// add   [dstp],src2p

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
			emit_adc_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// adc   [dstp],src2p

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
			emit_adc_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// adc   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64_keepflags(drcbe, &dst, dstreg, REG_EDX, &src1p);			// mov   dstreg:dstp,[src1p]
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
			emit_sub_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// sub   [dstp],src2p

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
			emit_sbb_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// sbb   [dstp],src2p

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
			emit_sbb_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// sbb   [dstp],src2p

		/* general case */
		else
		{
			emit_mov_r64_p64_keepflags(drcbe, &dst, dstreg, REG_EDX, &src1p);			// mov   dstreg:dstp,[src1p]
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
		/* general case */
		emit_mov_r64_p64(drcbe, &dst, REG_EAX, REG_EDX, &src1p);						// mov   eax:dstp,[src1p]
		emit_cmp_r64_p64(drcbe, &dst, REG_EAX, REG_EDX, &src2p, inst);					// cmp   eax:dstp,src2p
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
					emit_pop_r32(&dst, REG_EAX);										// pop   eax
					emit_and_m32_imm(&dst, MBD(REG_ESP, 0), ~0x84);						// and   [esp],~0x84
					emit_or_m32_r32(&dst, MBD(REG_ESP, 0), REG_EAX);					// or    [esp],eax
					emit_popf(&dst);													// popf
				}
			}
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_m32_imm(&dst, MBD(REG_ESP, 24), inst->flags);							// mov   [esp+24],flags
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 16), &src2p);						// mov   [esp+16],src2p
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 8), &src1p);							// mov   [esp+8],src1p
		if (!compute_hi)
			emit_mov_m32_imm(&dst, MBD(REG_ESP, 4), (FPTR)&drcbe->reslo);				// mov   [esp+4],&reslo
		else
			emit_mov_m32_imm(&dst, MBD(REG_ESP, 4), (FPTR)&drcbe->reshi);				// mov   [esp+4],&reshi
		emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (FPTR)&drcbe->reslo);					// mov   [esp],&reslo
		emit_call(&dst, (x86code *)dmulu);												// call  dmulu
		if (inst->flags != 0)
			emit_push_m32(&dst, MISD(REG_EAX, 4, flags_unmap));							// push   flags_unmap[eax*4]
		emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reslo + 0));				// mov   eax,reslo.lo
		emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reslo + 1));				// mov   edx,reslo.hi
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
		if (compute_hi)
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reshi + 0));			// mov   eax,reshi.lo
			emit_mov_r32_m32(&dst, REG_ECX, MABS((UINT32 *)&drcbe->reshi + 1));			// mov   ecx,reshi.hi
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_EAX, REG_ECX);					// mov   edstp,ecx:eax
		}
		if (inst->flags != 0)
			emit_popf(&dst);															// popf
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

		/* compute flags */
		if (inst->flags != 0)
		{
			if (zsflags != 0)
			{
				if (vflag)
					emit_pushf(&dst);													// pushf
				if (compute_hi)
				{
					if (inst->flags == DRCUML_FLAG_Z)
						emit_or_r32_r32(&dst, REG_EDX, REG_EAX);						// or    edx,eax
					else if (inst->flags == DRCUML_FLAG_S)
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
					emit_pop_r32(&dst, REG_EAX);										// pop   eax
					emit_and_m32_imm(&dst, MBD(REG_ESP, 0), ~0x84);						// and   [esp],~0x84
					emit_or_m32_r32(&dst, MBD(REG_ESP, 0), REG_EAX);					// or    [esp],eax
					emit_popf(&dst);													// popf
				}
			}
		}
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_m32_imm(&dst, MBD(REG_ESP, 24), inst->flags);							// mov   [esp+24],flags
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 16), &src2p);						// mov   [esp+16],src2p
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 8), &src1p);							// mov   [esp+8],src1p
		if (!compute_hi)
			emit_mov_m32_imm(&dst, MBD(REG_ESP, 4), (FPTR)&drcbe->reslo);				// mov   [esp+4],&reslo
		else
			emit_mov_m32_imm(&dst, MBD(REG_ESP, 4), (FPTR)&drcbe->reshi);				// push  [esp+4],&reshi
		emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (FPTR)&drcbe->reslo);					// mov   [esp],&reslo
		emit_call(&dst, (x86code *)dmuls);												// call  dmuls
		if (inst->flags != 0)
			emit_push_m32(&dst, MISD(REG_EAX, 4, flags_unmap));							// push   flags_unmap[eax*4]
		emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reslo + 0));				// mov   eax,reslo.lo
		emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reslo + 1));				// mov   edx,reslo.hi
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
		if (compute_hi)
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reshi + 0));			// mov   eax,reshi.lo
			emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reshi + 1));			// mov   edx,reshi.hi
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_EAX, REG_EDX);					// mov   edstp,edx:eax
		}
		if (inst->flags != 0)
			emit_popf(&dst);															// popf
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
		track_resolve_link(drcbe, &dst, &skip);										// skip:
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 16), &src2p);						// mov   [esp+16],src2p
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 8), &src1p);							// mov   [esp+8],src1p
		if (!compute_rem)
			emit_mov_m32_imm(&dst, MBD(REG_ESP, 4), (FPTR)&drcbe->reslo);				// mov   [esp+4],&reslo
		else
			emit_mov_m32_imm(&dst, MBD(REG_ESP, 4), (FPTR)&drcbe->reshi);				// push  [esp+4],&reshi
		emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (FPTR)&drcbe->reslo);					// mov   [esp],&reslo
		emit_call(&dst, (x86code *)ddivu);												// call  ddivu
		if (inst->flags != 0)
			emit_push_m32(&dst, MISD(REG_EAX, 4, flags_unmap));							// push   flags_unmap[eax*4]
		emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reslo + 0));				// mov   eax,reslo.lo
		emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reslo + 1));				// mov   edx,reslo.hi
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
		if (compute_rem)
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reshi + 0));			// mov   eax,reshi.lo
			emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reshi + 1));			// mov   edx,reshi.hi
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_EAX, REG_EDX);					// mov   edstp,edx:eax
		}
		if (inst->flags != 0)
			emit_popf(&dst);															// popf
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
		track_resolve_link(drcbe, &dst, &skip);										// skip:
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		/* general case */
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 16), &src2p);						// mov   [esp+16],src2p
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 8), &src1p);							// mov   [esp+8],src1p
		if (!compute_rem)
			emit_mov_m32_imm(&dst, MBD(REG_ESP, 4), (FPTR)&drcbe->reslo);				// mov   [esp+4],&reslo
		else
			emit_mov_m32_imm(&dst, MBD(REG_ESP, 4), (FPTR)&drcbe->reshi);				// push  [esp+4],&reshi
		emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (FPTR)&drcbe->reslo);					// mov   [esp],&reslo
		emit_call(&dst, (x86code *)ddivs);												// call  ddivs
		if (inst->flags != 0)
			emit_push_m32(&dst, MISD(REG_EAX, 4, flags_unmap));							// push   flags_unmap[eax*4]
		emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reslo + 0));				// mov   eax,reslo.lo
		emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reslo + 1));				// mov   edx,reslo.hi
		emit_mov_p64_r64(drcbe, &dst, &dstp, REG_EAX, REG_EDX);							// mov   dstp,edx:eax
		if (compute_rem)
		{
			emit_mov_r32_m32(&dst, REG_EAX, MABS((UINT32 *)&drcbe->reshi + 0));			// mov   eax,reshi.lo
			emit_mov_r32_m32(&dst, REG_EDX, MABS((UINT32 *)&drcbe->reshi + 1));			// mov   edx,reshi.hi
			emit_mov_p64_r64(drcbe, &dst, &edstp, REG_EAX, REG_EDX);					// mov   edstp,edx:eax
		}
		if (inst->flags != 0)
			emit_popf(&dst);															// popf
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
			emit_and_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// and   [dstp],src2p

		/* AND with immediate 0xff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xff && inst->flags == 0)
		{
			if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_movzx_r32_r8(&dst, dstreg, src1p.value);							// movzx dstreg,src1p
			else if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_movzx_r32_m8(&dst, dstreg, MABS(src1p.value));						// movzx dstreg,[src1p]
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
		}

		/* AND with immediate 0xffff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xffff && inst->flags == 0)
		{
			if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_movzx_r32_r16(&dst, dstreg, src1p.value);							// movzx dstreg,src1p
			else if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_movzx_r32_m16(&dst, dstreg, MABS(src1p.value));					// movzx dstreg,[src1p]
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
			emit_and_m64_p64(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// and   [dstp],src2p

		/* AND with immediate 0xff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xff && inst->flags == 0)
		{
			if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_movzx_r32_r8(&dst, dstreg, src1p.value);							// movzx dstreg,src1p
			else if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_movzx_r32_m8(&dst, dstreg, MABS(src1p.value));						// movzx dstreg,[src1p]
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
			if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);				// mov   dsthi,0
			else if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);						// mov   dsthi,0
		}

		/* AND with immediate 0xffff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xffff && inst->flags == 0)
		{
			if (src1p.type == DRCUML_PTYPE_INT_REGISTER)
				emit_movzx_r32_r16(&dst, dstreg, src1p.value);							// movzx dstreg,src1p
			else if (src1p.type == DRCUML_PTYPE_MEMORY)
				emit_movzx_r32_m16(&dst, dstreg, MABS(src1p.value));					// movzx dstreg,[src1p]
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
			if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);				// mov   dsthi,0
			else if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);						// mov   dsthi,0
		}

		/* AND with immediate 0xffffffff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == 0xffffffff && inst->flags == 0)
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
			if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);				// mov   dsthi,0
			else if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);						// mov   dsthi,0
		}

		/* AND with immediate 0xffffffff00000000 */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value == U64(0xffffffff00000000) && inst->flags == 0)
		{
			if (src1p.type != dstp.type || src1p.value != dstp.value)
			{
				emit_mov_r64_p64(drcbe, &dst, REG_NONE, REG_EDX, &src1p);				// mov   dstreg,src1p
				emit_mov_p64_r64(drcbe, &dst, &dstp, REG_NONE, REG_EDX);				// mov   dstp,dstreg
			}
			if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_xor_r32_r32(&dst, dstp.value, dstp.value);							// xor   dstlo,dstlo
			else if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_imm(&dst, MABS(dstp.value), 0);							// mov   dstlo,0
		}

		/* AND with immediate <= 0xffffffff */
		else if (src2p.type == DRCUML_PTYPE_IMMEDIATE && src2p.value <= 0xffffffff && inst->flags == 0)
		{
			emit_mov_r32_p32(drcbe, &dst, dstreg, &src1p);								// mov   dstreg,src1p
			emit_and_r32_p32(drcbe, &dst, dstreg, &src2p, inst);						// and   dstreg,src2p
			emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);								// mov   dstp,dstreg
			if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
				emit_mov_m32_imm(&dst, MABS(drcbe->reghi[dstp.value]), 0);				// mov   dsthi,0
			else if (dstp.type == DRCUML_PTYPE_MEMORY)
				emit_mov_m32_imm(&dst, MABS(dstp.value + 4), 0);						// mov   dsthi,0
		}

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
		emit_link skip;

		emit_mov_r64_p64(drcbe, &dst, REG_EDX, dstreg, &srcp);							// mov   dstreg:edx,srcp
		emit_bsr_r32_r32(&dst, dstreg, dstreg);											// bsr   dstreg,dstreg
		emit_jcc_short_link(&dst, COND_NZ, &skip);										// jnz   skip
		emit_mov_r32_imm(&dst, REG_ECX, 32 ^ 31);										// mov   ecx,32 ^ 31
		emit_bsr_r32_r32(&dst, dstreg, REG_EDX);										// bsr   dstreg,edx
		emit_cmovcc_r32_r32(&dst, COND_Z, dstreg, REG_ECX);								// cmovz dstreg,ecx
		emit_add_r32_imm(&dst, REG_ECX, 32);											// add   ecx,32
		track_resolve_link(drcbe, &dst, &skip);										// skip:
		emit_xor_r32_r32(&dst, REG_EDX, REG_EDX);										// xor   edx,edx
		emit_xor_r32_imm(&dst, dstreg, 31);												// xor   dstreg,31
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
		emit_bswap_r32(&dst, dstreg);													// bswap dstreg
		if (inst->flags != 0)
			emit_test_r32_r32(&dst, dstreg, dstreg);									// test  dstreg,dstreg
		emit_mov_p32_r32(drcbe, &dst, &dstp, dstreg);									// mov   dstp,dstreg
	}

	/* 64-bit form */
	else if (inst->size == 8)
	{
		emit_mov_r64_p64(drcbe, &dst, REG_EDX, dstreg, &srcp);							// mov   dstreg:edx,srcp
		emit_bswap_r32(&dst, dstreg);													// bswap dstreg
		emit_bswap_r32(&dst, REG_EDX);													// bswap edx
		emit_mov_p64_r64(drcbe, &dst, &dstp, dstreg, REG_EDX);							// mov   dstp,edx:dstreg
		if (inst->flags == DRCUML_FLAG_Z)
			emit_or_r32_r32(&dst, REG_EDX, dstreg);										// or    edx,eax
		else if (inst->flags == DRCUML_FLAG_S)
			emit_test_r32_r32(&dst, REG_EDX, REG_EDX);									// test  edx,edx
		else
		{
			emit_movzx_r32_r16(&dst, REG_ECX, dstreg);									// movzx ecx,dstreg
			emit_or_r32_r32(&dst, REG_EDX, REG_ECX);									// or    edx,ecx
			emit_mov_r32_r32(&dst, REG_ECX, dstreg);									// mov   ecx,dstreg
			emit_shr_r32_imm(&dst, REG_ECX, 16);										// shr   ecx,16
			emit_or_r32_r32(&dst, REG_EDX, REG_ECX);									// or    edx,ecx
		}
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
			emit_rcl_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// rcl   [dstp],src2p

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
		/* general case */
		emit_mov_r64_p64_keepflags(drcbe, &dst, dstreg, REG_EDX, &src1p);				// mov   dstreg:dstp,[src1p]
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
			emit_rcr_m32_p32(drcbe, &dst, MABS(dstp.value), &src2p, inst);				// rcr   [dstp],src2p

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
		/* general case */
		emit_mov_r64_p64_keepflags(drcbe, &dst, dstreg, REG_EDX, &src1p);				// mov   dstreg:dstp,[src1p]
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
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &basep, PTYPE_M, &indp, PTYPE_MRI);

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_mov_r32_m32(&dst, REG_EAX, MABS(basep.value + 4*indp.value));				// mov   eax,[basep + 4*indp]
		if (inst->size == 8)
			emit_mov_r32_m32(&dst, REG_EDX, MABS(basep.value + 4 + 4*indp.value));		// mov   edx,[basep + 4*indp + 4]
	}

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		emit_mov_r32_m32(&dst, REG_EAX, MISD(indreg, 4, basep.value));					// mov   eax,[basep + 4*indp]
		if (inst->size == 8)
			emit_mov_r32_m32(&dst, REG_EDX, MISD(indreg, 4, basep.value + 4));			// mov   edx,[basep + 4*indp + 4]
	}

	/* general case */
	emit_mov_m32_r32(&dst, MABS(dstp.value), REG_EAX);									// mov   [dstp],eax
	if (inst->size == 8)
		emit_mov_m32_r32(&dst, MABS(dstp.value + 4), REG_EDX);							// mov   [dstp + 4],edx
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
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &basep, PTYPE_M, &indp, PTYPE_MRI, &srcp, PTYPE_MF);

	/* general case */
	emit_mov_r32_m32(&dst, REG_EAX, MABS(srcp.value));									// mov   eax,[srcp]
	if (inst->size == 8)
		emit_mov_r32_m32(&dst, REG_EDX, MABS(srcp.value + 4));							// mov   edx,[srcp + 4]

	/* immediate index */
	if (indp.type == DRCUML_PTYPE_IMMEDIATE)
	{
		emit_mov_m32_r32(&dst, MABS(basep.value + 4*indp.value), REG_EAX);				// mov   [basep + 4*indp],eax
		if (inst->size == 8)
			emit_mov_m32_r32(&dst, MABS(basep.value + 4 + 4*indp.value), REG_EDX);		// mov   [basep + 4*indp + 4],edx
	}

	/* other index */
	else
	{
		int indreg = param_select_register(REG_ECX, &indp, NULL);
		emit_mov_r32_p32(drcbe, &dst, indreg, &indp);
		emit_mov_m32_r32(&dst, MISD(indreg, 4, basep.value), REG_EAX);					// mov   [basep + 4*indp],eax
		if (inst->size == 8)
			emit_mov_m32_r32(&dst, MISD(indreg, 4, basep.value + 4), REG_EDX);			// mov   [basep + 4*indp + 4],edx
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
	emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 4), &addrp);								// mov    [esp+4],addrp
	emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (UINT32)drcbe->space[spacep.value]);		// mov    [esp],space
	if (inst->size == 4)
		emit_call(&dst, (x86code *)drcbe->space[spacep.value]->accessors.read_dword);	// call   read_dword
	else if (inst->size == 8)
		emit_call(&dst, (x86code *)drcbe->space[spacep.value]->accessors.read_qword);	// call   read_qword

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
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &addrp, PTYPE_MRI, &srcp, PTYPE_MF, &spacep, PTYPE_I);

	/* set up a call to the write dword/qword handler */
	if (inst->size == 4)
		emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 8), &srcp);							// mov    [esp+8],srcp
	else if (inst->size == 8)
		emit_mov_m64_p64(drcbe, &dst, MBD(REG_ESP, 8), &srcp);							// mov    [esp+8],srcp
	emit_mov_m32_p32(drcbe, &dst, MBD(REG_ESP, 4), &addrp);								// mov    [esp+4],addrp
	emit_mov_m32_imm(&dst, MBD(REG_ESP, 0), (UINT32)drcbe->space[spacep.value]);		// mov    [esp],space
	if (inst->size == 4)
		emit_call(&dst, (x86code *)drcbe->space[spacep.value]->accessors.write_dword);	// call   write_dword
	else if (inst->size == 8)
		emit_call(&dst, (x86code *)drcbe->space[spacep.value]->accessors.write_qword);	// call   write_qword

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
	assert_any_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* always start with a jmp */
	if (inst->condition != DRCUML_COND_ALWAYS)
		emit_jcc_short_link(&dst, X86_NOT_CONDITION(inst->condition), &skip);			// jcc   skip

	/* general case */
	emit_mov_r32_m32(&dst, REG_EAX, MABS(srcp.value));									// mov   eax,[srcp]
	if (inst->size == 8)
		emit_mov_r32_m32(&dst, REG_EDX, MABS(srcp.value + 4));							// mov   edx,[srcp + 4]
	emit_mov_m32_r32(&dst, MABS(dstp.value), REG_EAX);									// mov   [dstp],eax
	if (inst->size == 8)
		emit_mov_m32_r32(&dst, MABS(dstp.value + 4), REG_EDX);							// mov   [dstp + 4],edx

	/* resolve the jump */
	if (skip.target != NULL)
		track_resolve_link(drcbe, &dst, &skip);										// skip:
	return dst;
}


/*-------------------------------------------------
    op_ftoint - process a FTOINT opcode
-------------------------------------------------*/

static x86code *op_ftoint(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, sizep, roundp;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_4(drcbe, inst, &dstp, PTYPE_MR, &srcp, PTYPE_MF, &sizep, PTYPE_I, &roundp, PTYPE_I);

	/* set rounding mode if necessary */
	if (roundp.value != DRCUML_FMOD_DEFAULT && (!drcbe->sse3 || roundp.value != DRCUML_FMOD_TRUNC))
	{
		emit_fstcw_m16(&dst, MABS(&drcbe->fmodesave));									// fstcw [fmodesave]
		emit_fldcw_m16(&dst, MABS(&fp_control[roundp.value]));							// fldcw fpcontrol[roundp]
	}

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp

	/* 4-byte integer case */
	if (sizep.value == DRCUML_SIZE_DWORD)
	{
		if (dstp.type == DRCUML_PTYPE_MEMORY)
		{
			if (!drcbe->sse3 || roundp.value != DRCUML_FMOD_TRUNC)
				emit_fistp_m32(&dst, MABS(dstp.value));									// fistp [dstp]
			else
				emit_fisttp_m32(&dst, MABS(dstp.value));								// fisttp [dstp]
		}
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			if (!drcbe->sse3 || roundp.value != DRCUML_FMOD_TRUNC)
				emit_fistp_m32(&dst, MABS(drcbe->reglo[dstp.value]));					// fistp reglo[dstp]
			else
				emit_fisttp_m32(&dst, MABS(drcbe->reglo[dstp.value]));					// fisttp reglo[dstp]
			emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));			// mov   dstp,reglo[dstp]
		}
	}

	/* 8-byte integer case */
	else if (sizep.value == DRCUML_SIZE_QWORD)
	{
		if (dstp.type == DRCUML_PTYPE_MEMORY)
		{
			if (!drcbe->sse3 || roundp.value != DRCUML_FMOD_TRUNC)
				emit_fistp_m64(&dst, MABS(dstp.value));									// fistp [dstp]
			else
				emit_fisttp_m64(&dst, MABS(dstp.value));								// fisttp [dstp]
		}
		else if (dstp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			if (!drcbe->sse3 || roundp.value != DRCUML_FMOD_TRUNC)
				emit_fistp_m64(&dst, MABS(drcbe->reglo[dstp.value]));					// fistp reglo[dstp]
			else
				emit_fisttp_m64(&dst, MABS(drcbe->reglo[dstp.value]));					// fisttp reglo[dstp]
			emit_mov_r32_m32(&dst, dstp.value, MABS(drcbe->reglo[dstp.value]));			// mov   dstp,reglo[dstp]
		}
	}

	/* restore control word and proceed */
	if (roundp.value != DRCUML_FMOD_DEFAULT && (!drcbe->sse3 || roundp.value != DRCUML_FMOD_TRUNC))
		emit_fldcw_m16(&dst, MABS(&drcbe->fmodesave));									// fldcw [fmodesave]

	return dst;
}


/*-------------------------------------------------
    op_ffrint - process a FFRINT opcode
-------------------------------------------------*/

static x86code *op_ffrint(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, sizep;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MRI, &sizep, PTYPE_I);

	/* 4-byte integer case */
	if (sizep.value == DRCUML_SIZE_DWORD)
	{
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			emit_mov_m32_imm(&dst, MABS(&drcbe->fptemp), srcp.value);					// mov   [fptemp],srcp
			emit_fild_m32(&dst, MABS(&drcbe->fptemp));									// fild  [fptemp]
		}
		else if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_fild_m32(&dst, MABS(srcp.value));										// fild  [srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			emit_mov_m32_r32(&dst, MABS(drcbe->reglo[srcp.value]), srcp.value);			// mov   reglo[srcp],srcp
			emit_fild_m32(&dst, MABS(drcbe->reglo[srcp.value]));						// fild  reglo[srcp]
		}
	}

	/* 8-bit integer case */
	else if (sizep.value == DRCUML_SIZE_QWORD)
	{
		if (srcp.type == DRCUML_PTYPE_IMMEDIATE)
		{
			emit_mov_m32_imm(&dst, MABS(&drcbe->fptemp), srcp.value);					// mov   [fptemp],srcp
			emit_mov_m32_imm(&dst, MABS((UINT8 *)&drcbe->fptemp + 4), srcp.value);		// mov   [fptemp+4],srcp
			emit_fild_m64(&dst, MABS(&drcbe->fptemp));									// fild  [fptemp]
		}
		else if (srcp.type == DRCUML_PTYPE_MEMORY)
			emit_fild_m64(&dst, MABS(srcp.value));										// fild  [srcp]
		else if (srcp.type == DRCUML_PTYPE_INT_REGISTER)
		{
			emit_mov_m32_r32(&dst, MABS(drcbe->reglo[srcp.value]), srcp.value);			// mov   reglo[srcp],srcp
			emit_fild_m64(&dst, MABS(drcbe->reglo[srcp.value]));						// fild  reglo[srcp]
		}
	}

	/* store the result and be done */
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  [dstp]
	return dst;
}


/*-------------------------------------------------
    op_ffrflt - process a FFRFLT opcode
-------------------------------------------------*/

static x86code *op_ffrflt(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp, sizep;

	/* validate instruction */
	assert(inst->size == 4 || inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_3(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF, &sizep, PTYPE_I);

	/* general case */
	if (sizep.value == DRCUML_SIZE_DWORD)
		emit_fld_m32(&dst, MABS(srcp.value));											// fld   [srcp]
	else if (sizep.value == DRCUML_SIZE_QWORD)
		emit_fld_m64(&dst, MABS(srcp.value));											// fld   [srcp]
	emit_fstp_p(&dst, inst->size, &dstp);												// fstp  dstp

	return dst;
}


/*-------------------------------------------------
    op_frnds - process a FRNDS opcode
-------------------------------------------------*/

static x86code *op_frnds(drcbe_state *drcbe, x86code *dst, const drcuml_instruction *inst)
{
	drcuml_parameter dstp, srcp;

	/* validate instruction */
	assert(inst->size == 8);
	assert_no_condition(inst);
	assert_no_flags(inst);

	/* normalize parameters */
	param_normalize_2(drcbe, inst, &dstp, PTYPE_MF, &srcp, PTYPE_MF);

	/* general case */
	emit_fld_p(&dst, inst->size, &srcp);												// fld   srcp
	emit_fstp_m32(&dst, MABS(&drcbe->fptemp));											// fstp  [fptemp]
	emit_fld_m32(&dst, MABS(&drcbe->fptemp));											// fld   [fptemp]
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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
	assert_no_condition(inst);
	assert_flags(inst, DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_U);

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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
	assert_no_condition(inst);
	assert_no_flags(inst);

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
	assert_no_condition(inst);
	assert_no_flags(inst);

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


/*-------------------------------------------------
    ddivu - perform a double-wide unsigned divide
-------------------------------------------------*/

static int ddivu(UINT64 *dstlo, UINT64 *dsthi, UINT64 src1, UINT64 src2)
{
	/* do nothing if src2 == 0 */
	if (src2 == 0)
		return DRCUML_FLAG_V;

	/* shortcut if no remainder */
	*dstlo = src1 / src2;
	if (dstlo != dsthi)
		*dsthi = src1 % src2;
	return ((*dstlo == 0) << 2) | ((*dstlo >> 60) & DRCUML_FLAG_S);
}


/*-------------------------------------------------
    ddivs - perform a double-wide signed divide
-------------------------------------------------*/

static int ddivs(UINT64 *dstlo, UINT64 *dsthi, INT64 src1, INT64 src2)
{
	/* do nothing if src2 == 0 */
	if (src2 == 0)
		return DRCUML_FLAG_V;

	/* shortcut if no remainder */
	*dstlo = src1 / src2;
	if (dstlo != dsthi)
		*dsthi = src1 % src2;
	return ((*dstlo == 0) << 2) | ((*dstlo >> 60) & DRCUML_FLAG_S);
}
