/***************************************************************************

    drcuml.h

    Universal machine language for dynamic recompiling CPU cores.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DRCUML_H__
#define __DRCUML_H__

#include "cpuintrf.h"
#include "drccache.h"
#include <setjmp.h>


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* these options are passed into drcuml_alloc() and control global behaviors */
#define DRCUML_OPTION_USE_C					0x0001		/* always use the C back-end */
#define DRCUML_OPTION_LOG_UML				0x0002		/* generate a UML disassembly of each block */
#define DRCUML_OPTION_LOG_NATIVE			0x0004		/* tell the back-end to generate a native disassembly of each block */


/* opcode parameter types */
enum _drcuml_ptype
{
	DRCUML_PTYPE_NONE = 0,
	DRCUML_PTYPE_IMMEDIATE,
	DRCUML_PTYPE_INT_REGISTER,
	DRCUML_PTYPE_FLOAT_REGISTER,
	DRCUML_PTYPE_MAPVAR,
	DRCUML_PTYPE_MEMORY,
	DRCUML_PTYPE_MAX
};
typedef enum _drcuml_ptype drcuml_ptype;
DECLARE_ENUM_OPERATORS(drcuml_ptype)


/* these define the registers for the UML */
enum
{
	DRCUML_REG_INVALID = 0,	/* 0 is invalid */

	/* integer registers */
	DRCUML_REG_I0 = 0x400,
	DRCUML_REG_I1,
	DRCUML_REG_I2,
	DRCUML_REG_I3,
	DRCUML_REG_I4,
	DRCUML_REG_I5,
	DRCUML_REG_I6,
	DRCUML_REG_I7,
	DRCUML_REG_I8,
	DRCUML_REG_I9,
	DRCUML_REG_I_END,

	/* floating point registers */
	DRCUML_REG_F0 = 0x800,
	DRCUML_REG_F1,
	DRCUML_REG_F2,
	DRCUML_REG_F3,
	DRCUML_REG_F4,
	DRCUML_REG_F5,
	DRCUML_REG_F6,
	DRCUML_REG_F7,
	DRCUML_REG_F8,
	DRCUML_REG_F9,
	DRCUML_REG_F_END,

	/* map variables */
	DRCUML_MAPVAR_M0 = 0xc00,
	DRCUML_MAPVAR_M1,
	DRCUML_MAPVAR_M2,
	DRCUML_MAPVAR_M3,
	DRCUML_MAPVAR_M4,
	DRCUML_MAPVAR_M5,
	DRCUML_MAPVAR_M6,
	DRCUML_MAPVAR_M7,
	DRCUML_MAPVAR_M8,
	DRCUML_MAPVAR_M9,
	DRCUML_MAPVAR_END
};


/* UML flag definitions */
enum
{
	DRCUML_FLAG_C = 1,					/* carry flag */
	DRCUML_FLAG_V = 2,					/* overflow flag (defined for integer only) */
	DRCUML_FLAG_Z = 4,					/* zero flag */
	DRCUML_FLAG_S = 8,					/* sign flag (defined for integer only) */
	DRCUML_FLAG_U = 16					/* unordered flag (defined for FP only) */
};


/* these define the conditions for the UML */
/* note that these are defined such that (condition ^ 1) is always the opposite */
/* they are also defined so as not to conflict with the flag bits above */
enum
{
	DRCUML_COND_ALWAYS = 0,

	DRCUML_COND_Z = 0x80,	/* requires Z */
	DRCUML_COND_NZ,			/* requires Z */
	DRCUML_COND_S,			/* requires S */
	DRCUML_COND_NS,			/* requires S */
	DRCUML_COND_C,			/* requires C */
	DRCUML_COND_NC,			/* requires C */
	DRCUML_COND_V,			/* requires V */
	DRCUML_COND_NV,			/* requires V */
	DRCUML_COND_U,			/* requires U */
	DRCUML_COND_NU,			/* requires U */
	DRCUML_COND_A,			/* requires CZ */
	DRCUML_COND_BE,			/* requires CZ */
	DRCUML_COND_G,			/* requires SVZ */
	DRCUML_COND_LE,			/* requires SVZ */
	DRCUML_COND_L,			/* requires SV */
	DRCUML_COND_GE,			/* requires SV */

	DRCUML_COND_MAX
};


/* basic condition code aliases */
enum
{
	DRCUML_COND_E = DRCUML_COND_Z,
	DRCUML_COND_NE = DRCUML_COND_NZ,
	DRCUML_COND_B = DRCUML_COND_C,
	DRCUML_COND_AE = DRCUML_COND_NC
};


/* floating point modes */
enum
{
	DRCUML_FMOD_TRUNC = 0,	/* truncate */
	DRCUML_FMOD_ROUND,		/* round */
	DRCUML_FMOD_CEIL,		/* round up */
	DRCUML_FMOD_FLOOR,		/* round down */
	DRCUML_FMOD_DEFAULT
};


/* sizes */
enum
{
	DRCUML_SIZE_BYTE = 0,	/* 1-byte */
	DRCUML_SIZE_WORD,		/* 2-byte */
	DRCUML_SIZE_DWORD,		/* 4-byte */
	DRCUML_SIZE_QWORD,		/* 8-byte */
	DRCUML_SIZE_SHORT = DRCUML_SIZE_DWORD,
	DRCUML_SIZE_DOUBLE = DRCUML_SIZE_QWORD
};


/* scale/space combinations */
enum
{
	DRCUML_SCSIZE_BYTE     =  (0 * 16) + DRCUML_SIZE_BYTE,
	DRCUML_SCSIZE_BYTE_x2  =  (1 * 16) + DRCUML_SIZE_BYTE,
	DRCUML_SCSIZE_BYTE_x4  =  (2 * 16) + DRCUML_SIZE_BYTE,
	DRCUML_SCSIZE_BYTE_x8  =  (3 * 16) + DRCUML_SIZE_BYTE,
	DRCUML_SCSIZE_WORD_x1  =  (0 * 16) + DRCUML_SIZE_WORD,
	DRCUML_SCSIZE_WORD     =  (1 * 16) + DRCUML_SIZE_WORD,
	DRCUML_SCSIZE_WORD_x4  =  (2 * 16) + DRCUML_SIZE_WORD,
	DRCUML_SCSIZE_WORD_x8  =  (3 * 16) + DRCUML_SIZE_WORD,
	DRCUML_SCSIZE_DWORD_x1 =  (0 * 16) + DRCUML_SIZE_DWORD,
	DRCUML_SCSIZE_DWORD_x2 =  (1 * 16) + DRCUML_SIZE_DWORD,
	DRCUML_SCSIZE_DWORD    =  (2 * 16) + DRCUML_SIZE_DWORD,
	DRCUML_SCSIZE_DWORD_x8 =  (3 * 16) + DRCUML_SIZE_DWORD,
	DRCUML_SCSIZE_QWORD_x1 =  (0 * 16) + DRCUML_SIZE_QWORD,
	DRCUML_SCSIZE_QWORD_x2 =  (1 * 16) + DRCUML_SIZE_QWORD,
	DRCUML_SCSIZE_QWORD_x4 =  (2 * 16) + DRCUML_SIZE_QWORD,
	DRCUML_SCSIZE_QWORD    =  (3 * 16) + DRCUML_SIZE_QWORD
};


/* size/space combinations */
enum
{
	DRCUML_SPSIZE_PROGRAM_BYTE = (ADDRESS_SPACE_PROGRAM * 16) + DRCUML_SIZE_BYTE,
	DRCUML_SPSIZE_PROGRAM_WORD = (ADDRESS_SPACE_PROGRAM * 16) + DRCUML_SIZE_WORD,
	DRCUML_SPSIZE_PROGRAM_DWORD = (ADDRESS_SPACE_PROGRAM * 16) + DRCUML_SIZE_DWORD,
	DRCUML_SPSIZE_PROGRAM_QWORD = (ADDRESS_SPACE_PROGRAM * 16) + DRCUML_SIZE_QWORD,
	DRCUML_SPSIZE_DATA_BYTE = (ADDRESS_SPACE_DATA * 16) + DRCUML_SIZE_BYTE,
	DRCUML_SPSIZE_DATA_WORD = (ADDRESS_SPACE_DATA * 16) + DRCUML_SIZE_WORD,
	DRCUML_SPSIZE_DATA_DWORD = (ADDRESS_SPACE_DATA * 16) + DRCUML_SIZE_DWORD,
	DRCUML_SPSIZE_DATA_QWORD = (ADDRESS_SPACE_DATA * 16) + DRCUML_SIZE_QWORD,
	DRCUML_SPSIZE_IO_BYTE = (ADDRESS_SPACE_IO * 16) + DRCUML_SIZE_BYTE,
	DRCUML_SPSIZE_IO_WORD = (ADDRESS_SPACE_IO * 16) + DRCUML_SIZE_WORD,
	DRCUML_SPSIZE_IO_DWORD = (ADDRESS_SPACE_IO * 16) + DRCUML_SIZE_DWORD,
	DRCUML_SPSIZE_IO_QWORD = (ADDRESS_SPACE_IO * 16) + DRCUML_SIZE_QWORD
};


/* these define the opcodes for the UML */
enum _drcuml_opcode
{
	DRCUML_OP_INVALID,

	/* Compile-time opcodes */
	DRCUML_OP_HANDLE,		/* HANDLE  handle                 */
	DRCUML_OP_HASH,			/* HASH    mode,pc                */
	DRCUML_OP_LABEL,		/* LABEL   imm                    */
	DRCUML_OP_COMMENT,		/* COMMENT string                 */
	DRCUML_OP_MAPVAR,		/* MAPVAR  mapvar,value           */

	/* Control Flow Operations */
	DRCUML_OP_NOP,			/* NOP                            */
	DRCUML_OP_DEBUG,		/* DEBUG   pc                     */
	DRCUML_OP_EXIT,			/* EXIT    src1[,c]               */
	DRCUML_OP_HASHJMP,		/* HASHJMP mode,pc,handle         */
	DRCUML_OP_JMP,			/* JMP     imm[,c]                */
	DRCUML_OP_EXH,			/* EXH     handle,param[,c]       */
	DRCUML_OP_CALLH,		/* CALLH   handle[,c]             */
	DRCUML_OP_RET,			/* RET     [c]                    */
	DRCUML_OP_CALLC,		/* CALLC   func,ptr[,c]           */
	DRCUML_OP_RECOVER,		/* RECOVER dst,mapvar             */

	/* Internal Register Operations */
	DRCUML_OP_SETFMOD,		/* SETFMOD src                    */
	DRCUML_OP_GETFMOD,		/* GETFMOD dst                    */
	DRCUML_OP_GETEXP,		/* GETEXP  dst                    */
	DRCUML_OP_GETFLGS,		/* GETFLGS dst[,f]                */
	DRCUML_OP_SAVE,			/* SAVE    mem                    */
	DRCUML_OP_RESTORE,		/* RESTORE mem                    */

	/* Integer Operations */
	DRCUML_OP_LOAD,			/* LOAD    dst,base,index,size    */
	DRCUML_OP_LOADS,		/* LOADS   dst,base,index,size    */
	DRCUML_OP_STORE,		/* STORE   base,index,src,size    */
	DRCUML_OP_READ,			/* READ    dst,src1,space/size    */
	DRCUML_OP_READM,		/* READM   dst,src1,mask,space/size */
	DRCUML_OP_WRITE,		/* WRITE   dst,src1,space/size    */
	DRCUML_OP_WRITEM,		/* WRITEM  dst,mask,src1,space/size */
	DRCUML_OP_CARRY,		/* CARRY   src,bitnum             */
	DRCUML_OP_SET,			/* SET     dst,c                  */
	DRCUML_OP_MOV,			/* MOV     dst,src[,c]            */
	DRCUML_OP_SEXT,			/* SEXT    dst,src,size           */
	DRCUML_OP_ROLAND,		/* ROLAND  dst,src,shift,mask     */
	DRCUML_OP_ROLINS,		/* ROLINS  dst,src,shift,mask     */
	DRCUML_OP_ADD,			/* ADD     dst,src1,src2[,f]      */
	DRCUML_OP_ADDC,			/* ADDC    dst,src1,src2[,f]      */
	DRCUML_OP_SUB,			/* SUB     dst,src1,src2[,f]      */
	DRCUML_OP_SUBB,			/* SUBB    dst,src1,src2[,f]      */
	DRCUML_OP_CMP,			/* CMP     src1,src2[,f]          */
	DRCUML_OP_MULU,			/* MULU    dst,edst,src1,src2[,f] */
	DRCUML_OP_MULS,			/* MULS    dst,edst,src1,src2[,f] */
	DRCUML_OP_DIVU,			/* DIVU    dst,edst,src1,src2[,f] */
	DRCUML_OP_DIVS,			/* DIVS    dst,edst,src1,src2[,f] */
	DRCUML_OP_AND,			/* AND     dst,src1,src2[,f]      */
	DRCUML_OP_TEST,			/* TEST    src1,src2[,f]          */
	DRCUML_OP_OR,			/* OR      dst,src1,src2[,f]      */
	DRCUML_OP_XOR,			/* XOR     dst,src1,src2[,f]      */
	DRCUML_OP_LZCNT,		/* LZCNT   dst,src                */
	DRCUML_OP_BSWAP,		/* BSWAP   dst,src                */
	DRCUML_OP_SHL,			/* SHL     dst,src,count[,f]      */
	DRCUML_OP_SHR,			/* SHR     dst,src,count[,f]      */
	DRCUML_OP_SAR,			/* SAR     dst,src,count[,f]      */
	DRCUML_OP_ROL,			/* ROL     dst,src,count[,f]      */
	DRCUML_OP_ROLC,			/* ROLC    dst,src,count[,f]      */
	DRCUML_OP_ROR,			/* ROL     dst,src,count[,f]      */
	DRCUML_OP_RORC,			/* ROLC    dst,src,count[,f]      */

	/* Floating Point Operations */
	DRCUML_OP_FLOAD,		/* FLOAD   dst,base,index         */
	DRCUML_OP_FSTORE,		/* FSTORE  base,index,src         */
	DRCUML_OP_FREAD,		/* FREAD   dst,space,src1         */
	DRCUML_OP_FWRITE,		/* FWRITE  space,dst,src1         */
	DRCUML_OP_FMOV,			/* FMOV    dst,src1[,c]           */
	DRCUML_OP_FTOINT,		/* FTOINT  dst,src1,size,round    */
	DRCUML_OP_FFRINT,		/* FFRINT  dst,src1,size          */
	DRCUML_OP_FFRFLT,		/* FFRFLT  dst,src1,size          */
	DRCUML_OP_FRNDS,		/* FRNDS   dst,src1               */
	DRCUML_OP_FADD,			/* FADD    dst,src1,src2          */
	DRCUML_OP_FSUB,			/* FSUB    dst,src1,src2          */
	DRCUML_OP_FCMP,			/* FCMP    src1,src2              */
	DRCUML_OP_FMUL,			/* FMUL    dst,src1,src2          */
	DRCUML_OP_FDIV,			/* FDIV    dst,src1,src2          */
	DRCUML_OP_FNEG,			/* FNEG    dst,src1               */
	DRCUML_OP_FABS,			/* FABS    dst,src1               */
	DRCUML_OP_FSQRT,		/* FSQRT   dst,src1               */
	DRCUML_OP_FRECIP,		/* FRECIP  dst,src1               */
	DRCUML_OP_FRSQRT,		/* FRSQRT  dst,src1               */

	DRCUML_OP_MAX
};
typedef enum _drcuml_opcode drcuml_opcode;



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* represents a reference to a local label in the code */
typedef UINT32 drcuml_codelabel;

/* represents the value of an opcode parameter */
typedef UINT64 drcuml_pvalue;


/* opaque structure describing UML generation state */
typedef struct _drcuml_state drcuml_state;

/* opaque structure describing UML codegen block */
typedef struct _drcuml_block drcuml_block;

/* opaque structure describing back-end state */
typedef struct _drcbe_state drcbe_state;

/* opaque structure describing a code handle */
typedef struct _drcuml_codehandle drcuml_codehandle;


/* a parameter for a UML instructon is encoded like this */
typedef struct _drcuml_parameter drcuml_parameter;
struct _drcuml_parameter
{
	drcuml_ptype		type;				/* parameter type */
	drcuml_pvalue		value;				/* parameter value */
};


/* a single UML instructon is encoded like this */
typedef struct _drcuml_instruction drcuml_instruction;
struct _drcuml_instruction
{
	drcuml_opcode		opcode;				/* opcode */
	UINT8				condition;			/* condition */
	UINT8				flags;				/* flags */
	UINT8				size;				/* operation size */
	UINT8				numparams;			/* number of parameters */
	drcuml_parameter	param[4];			/* up to 4 parameters */
};


/* structure describing rules for parameter encoding */
typedef struct _drcuml_parameter_info drcuml_parameter_info;
struct _drcuml_parameter_info
{
	UINT8				output;				/* input or output? */
	UINT8				size;				/* size of the parameter */
	UINT16				typemask;			/* types allowed */
};


/* structure describing rules for opcode encoding */
typedef struct _drcuml_opcode_info drcuml_opcode_info;
struct _drcuml_opcode_info
{
	drcuml_opcode		opcode;				/* the opcode itself */
	const char *		mnemonic;			/* mnemonic string */
	UINT8				sizes;				/* allowed sizes */
	UINT8				condition;			/* conditions allowed? */
	UINT8				inflags;			/* input flags */
	UINT8				outflags;			/* output flags */
	UINT8				modflags;			/* modified flags */
	drcuml_parameter_info param[4];			/* information about parameters */
};


/* an integer register, with low/high parts */
typedef union _drcuml_ireg drcuml_ireg;
union _drcuml_ireg
{
#ifdef LSB_FIRST
	struct { UINT32	l,h; } w;				/* 32-bit low, high parts of the register */
#else
	struct { UINT32 h,l; } w;				/* 32-bit low, high parts of the register */
#endif
	UINT64				d;					/* 64-bit full register */
};


/* a floating-point register, with low/high parts */
typedef union _drcuml_freg drcuml_freg;
union _drcuml_freg
{
#ifdef LSB_FIRST
	struct { float l,h; } s;				/* 32-bit low, high parts of the register */
#else
	struct { float h,l;	} s;				/* 32-bit low, high parts of the register */
#endif
	double				d;					/* 64-bit full register */
};


/* the collected machine state of a system */
typedef struct _drcuml_machine_state drcuml_machine_state;
struct _drcuml_machine_state
{
	drcuml_ireg			r[DRCUML_REG_I_END - DRCUML_REG_I0];	/* integer registers */
	drcuml_freg			f[DRCUML_REG_F_END - DRCUML_REG_F0];	/* floating-point registers */
	UINT32				exp;				/* exception parameter register */
	UINT8				fmod;				/* fmod (floating-point mode) register */
	UINT8				flags;				/* flags state */
};


/* hints and information about the back-end */
typedef struct _drcbe_info drcbe_info;
struct _drcbe_info
{
	UINT8				direct_iregs;		/* number of direct-mapped integer registers */
	UINT8				direct_fregs;		/* number of direct-mapped floating point registers */
};


/* typedefs for back-end callback functions */
typedef drcbe_state *(*drcbe_alloc_func)(drcuml_state *drcuml, drccache *cache, const device_config *device, UINT32 flags, int modes, int addrbits, int ignorebits);
typedef void (*drcbe_free_func)(drcbe_state *state);
typedef void (*drcbe_reset_func)(drcbe_state *state);
typedef int	(*drcbe_execute_func)(drcbe_state *state, drcuml_codehandle *entry);
typedef void (*drcbe_generate_func)(drcbe_state *state, drcuml_block *block, const drcuml_instruction *instlist, UINT32 numinst);
typedef int (*drcbe_hash_exists)(drcbe_state *state, UINT32 mode, UINT32 pc);
typedef void (*drcbe_get_info)(drcbe_state *state, drcbe_info *info);


/* interface structure for a back-end */
typedef struct _drcbe_interface drcbe_interface;
struct _drcbe_interface
{
	drcbe_alloc_func	be_alloc;
	drcbe_free_func		be_free;
	drcbe_reset_func	be_reset;
	drcbe_execute_func	be_execute;
	drcbe_generate_func	be_generate;
	drcbe_hash_exists	be_hash_exists;
	drcbe_get_info		be_get_info;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* ----- initialization/teardown ----- */

/* allocate state for the code generator and initialize the back-end */
drcuml_state *drcuml_alloc(const device_config *device, drccache *cache, UINT32 flags, int modes, int addrbits, int ignorebits);

/* return information about the back-end */
void drcuml_get_backend_info(drcuml_state *drcuml, drcbe_info *info);

/* reset the state completely, flushing the cache and all information */
void drcuml_reset(drcuml_state *drcuml);

/* free state for the code generator and the back-end */
void drcuml_free(drcuml_state *drcuml);



/* ----- code block generation ----- */

/* begin a new code block */
drcuml_block *drcuml_block_begin(drcuml_state *drcuml, UINT32 maxinst, jmp_buf *errorbuf);

/* append an opcode to the block, with 0-4 parameters */
void drcuml_block_append_0(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition);
void drcuml_block_append_1(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition, drcuml_ptype p0type, drcuml_pvalue p0value);
void drcuml_block_append_2(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition, drcuml_ptype p0type, drcuml_pvalue p0value, drcuml_ptype p1type, drcuml_pvalue p1value);
void drcuml_block_append_3(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition, drcuml_ptype p0type, drcuml_pvalue p0value, drcuml_ptype p1type, drcuml_pvalue p1value, drcuml_ptype p2type, drcuml_pvalue p2value);
void drcuml_block_append_4(drcuml_block *block, drcuml_opcode op, UINT8 size, UINT8 condition, drcuml_ptype p0type, drcuml_pvalue p0value, drcuml_ptype p1type, drcuml_pvalue p1value, drcuml_ptype p2type, drcuml_pvalue p2value, drcuml_ptype p3type, drcuml_pvalue p3value);

/* complete a code block and commit it to the cache via the back-end */
void drcuml_block_end(drcuml_block *block);

/* abort a code block in progress due to cache allocation failure */
void drcuml_block_abort(drcuml_block *block);

/* return true if a hash entry exists for the given mode/pc */
int drcuml_hash_exists(drcuml_state *drcuml, UINT32 mode, UINT32 pc);



/* ----- code execution ----- */

/* execute at the given PC/mode for the specified cycles */
int drcuml_execute(drcuml_state *drcuml, drcuml_codehandle *entry);



/* ----- code handles ----- */

/* allocate a new handle */
drcuml_codehandle *drcuml_handle_alloc(drcuml_state *drcuml, const char *name);

/* set the handle pointer */
void drcuml_handle_set_codeptr(drcuml_codehandle *handle, drccodeptr code);

/* get the pointer from a handle */
drccodeptr drcuml_handle_codeptr(const drcuml_codehandle *handle);

/* get the address of the pointer from a handle */
drccodeptr *drcuml_handle_codeptr_addr(drcuml_codehandle *handle);

/* get the name of a handle */
const char *drcuml_handle_name(const drcuml_codehandle *handle);



/* ----- code logging ----- */

/* directly printf to the UML log if generated */
void drcuml_log_printf(drcuml_state *drcuml, const char *format, ...) ATTR_PRINTF(2,3);

/* add a symbol to the internal symbol table */
void drcuml_symbol_add(drcuml_state *drcuml, void *base, UINT32 length, const char *name);

/* look up a symbol from the internal symbol table or return NULL if not found */
const char *drcuml_symbol_find(drcuml_state *drcuml, void *base, UINT32 *offset);

/* attach a comment to the current output location in the specified block */
void drcuml_add_comment(drcuml_block *block, const char *format, ...) ATTR_PRINTF(2,3);

/* disassemble a UML instruction to the given buffer */
void drcuml_disasm(const drcuml_instruction *inst, char *buffer, drcuml_state *state);


#endif /* __DRCUML_H__ */
