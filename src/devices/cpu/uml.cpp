// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    uml.cpp

    Universal machine language definitions and classes.

****************************************************************************

    Future improvements/changes:

    * UML optimizer:
        - constant folding

    * Write a back-end validator:
        - checks all combinations of memory/register/immediate on all params
        - checks behavior of all opcodes

    * Extend registers to 16? Depends on if PPC can use them

    * Support for FPU exceptions

    * New instructions?
        - VALID opcode_desc,handle,param
            checksum/compare code referenced by opcode_desc; if not
            matching, generate exception with handle,param

        - RECALL handle
            change code at caller to call handle in the future

***************************************************************************/

#include "emu.h"
#include "drcuml.h"

#include "drcumlsh.h"

#include <type_traits>


using namespace uml;



//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_SIMPLIFICATIONS     (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// opcode validation condition/flag valid bitmasks
constexpr u8  OPFLAGS_NONE   = FLAGS_NONE;
constexpr u8  OPFLAGS_C      = FLAG_C;
constexpr u8  OPFLAGS_Z      = FLAG_Z;
constexpr u8  OPFLAGS_SZ     = FLAG_S | FLAG_Z;
constexpr u8  OPFLAGS_SZC    = FLAG_S | FLAG_Z | FLAG_C;
constexpr u8  OPFLAGS_SZV    = FLAG_S | FLAG_Z | FLAG_V;
constexpr u8  OPFLAGS_SZVC   = FLAG_S | FLAG_Z | FLAG_V | FLAG_C;
constexpr u8  OPFLAGS_UZC    = FLAG_U | FLAG_Z | FLAG_C;
constexpr u8  OPFLAGS_ALL    = FLAGS_ALL;
constexpr u8  OPFLAGS_P1     = 0x81;
constexpr u8  OPFLAGS_P2     = 0x82;
constexpr u8  OPFLAGS_P3     = 0x83;
constexpr u8  OPFLAGS_P4     = 0x84;

// parameter input/output states
constexpr u8  PIO_IN         = 0x01;
constexpr u8  PIO_OUT        = 0x02;
constexpr u8  PIO_INOUT      = (PIO_IN | PIO_OUT);

// parameter sizes
constexpr u8  PSIZE_4        = SIZE_DWORD;
constexpr u8  PSIZE_8        = SIZE_QWORD;
constexpr u8  PSIZE_OP       = 0x80;
constexpr u8  PSIZE_P1       = 0x81;
constexpr u8  PSIZE_P2       = 0x82;
constexpr u8  PSIZE_P3       = 0x83;
constexpr u8  PSIZE_P4       = 0x84;

// basic parameter types
constexpr u16 PTYPES_NONE    = 0;
constexpr u16 PTYPES_IMM     = (1 << parameter::PTYPE_IMMEDIATE);
constexpr u16 PTYPES_IREG    = (1 << parameter::PTYPE_INT_REGISTER);
constexpr u16 PTYPES_FREG    = (1 << parameter::PTYPE_FLOAT_REGISTER);
constexpr u16 PTYPES_MVAR    = (1 << parameter::PTYPE_MAPVAR);
constexpr u16 PTYPES_MEM     = (1 << parameter::PTYPE_MEMORY);
constexpr u16 PTYPES_SIZE    = (1 << parameter::PTYPE_SIZE);
constexpr u16 PTYPES_SCSIZE  = (1 << parameter::PTYPE_SIZE_SCALE);
constexpr u16 PTYPES_SPSIZE  = (1 << parameter::PTYPE_SIZE_SPACE);
constexpr u16 PTYPES_HANDLE  = (1 << parameter::PTYPE_CODE_HANDLE);
constexpr u16 PTYPES_LABEL   = (1 << parameter::PTYPE_CODE_LABEL);
constexpr u16 PTYPES_CFUNC   = (1 << parameter::PTYPE_C_FUNCTION);
constexpr u16 PTYPES_ROUND   = (1 << parameter::PTYPE_ROUNDING);
constexpr u16 PTYPES_STR     = (1 << parameter::PTYPE_STRING);

// special parameter types
constexpr u16 PTYPES_PTR     = (PTYPES_MEM | 0x1000);
constexpr u16 PTYPES_STATE   = (PTYPES_MEM | 0x2000);

// combinations of types
constexpr u16 PTYPES_IRM     = (PTYPES_IREG | PTYPES_MEM);
constexpr u16 PTYPES_FRM     = (PTYPES_FREG | PTYPES_MEM);
constexpr u16 PTYPES_IMV     = (PTYPES_IMM | PTYPES_MVAR);
constexpr u16 PTYPES_IANY    = (PTYPES_IRM | PTYPES_IMV);
constexpr u16 PTYPES_FANY    = (PTYPES_FRM);



//**************************************************************************
//  TABLES
//**************************************************************************

// macro to simplify the table
#define PINFO(inout, size, types)                                   { PIO_##inout, PSIZE_##size, PTYPES_##types }
#define OPINFO0(op,str,sizes,cond,iflag,oflag,mflag)                { OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { { 0 } } },
#define OPINFO1(op,str,sizes,cond,iflag,oflag,mflag,p0)             { OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { p0 } },
#define OPINFO2(op,str,sizes,cond,iflag,oflag,mflag,p0,p1)          { OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { p0, p1 } },
#define OPINFO3(op,str,sizes,cond,iflag,oflag,mflag,p0,p1,p2)       { OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { p0, p1, p2 } },
#define OPINFO4(op,str,sizes,cond,iflag,oflag,mflag,p0,p1,p2,p3)    { OP_##op, str, sizes, cond, OPFLAGS_##iflag, OPFLAGS_##oflag, OPFLAGS_##mflag, { p0, p1, p2, p3 } },

// opcode validation table
opcode_info const instruction::s_opcode_info_table[OP_MAX] =
{
	OPINFO0(INVALID, "invalid",  4,   false, NONE, NONE, NONE)

	// Compile-time opcodes
	OPINFO1(HANDLE,  "handle",   4,   false, NONE, NONE, NONE, PINFO(IN, OP, HANDLE))
	OPINFO2(HASH,    "hash",     4,   false, NONE, NONE, NONE, PINFO(IN, OP, IMV), PINFO(IN, OP, IMV))
	OPINFO1(LABEL,   "label",    4,   false, NONE, NONE, NONE, PINFO(IN, OP, LABEL))
	OPINFO1(COMMENT, "comment",  4,   false, NONE, NONE, NONE, PINFO(IN, OP, STR))
	OPINFO2(MAPVAR,  "mapvar",   4,   false, NONE, NONE, NONE, PINFO(OUT, OP, MVAR), PINFO(IN, OP, IMV))

	// Control Flow Operations
	OPINFO0(NOP,     "nop",      4,   false, NONE, NONE, NONE)
	OPINFO1(DEBUG,   "debug",    4,   false, NONE, NONE, ALL,  PINFO(IN, OP, IANY)) // MAME debugger breakpoint
	OPINFO0(BREAK,   "break",    4,   false, NONE, NONE, ALL) // (for debugging) Issues a breakpoint exception to allow for debugging the generated assembly
	OPINFO1(EXIT,    "exit",     4,   true,  NONE, NONE, ALL,  PINFO(IN, OP, IANY))
	OPINFO3(HASHJMP, "hashjmp",  4,   false, NONE, NONE, ALL,  PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, HANDLE))
	OPINFO1(JMP,     "jmp",      4,   true,  NONE, NONE, NONE, PINFO(IN, OP, LABEL))
	OPINFO2(EXH,     "exh",      4,   true,  NONE, NONE, ALL,  PINFO(IN, OP, HANDLE), PINFO(IN, OP, IANY)) // Call exception handler
	OPINFO1(CALLH,   "callh",    4,   true,  NONE, NONE, ALL,  PINFO(IN, OP, HANDLE)) // Call handle
	OPINFO0(RET,     "ret",      4,   true,  NONE, NONE, NONE)
	OPINFO2(CALLC,   "callc",    4,   true,  NONE, NONE, ALL,  PINFO(IN, OP, CFUNC), PINFO(IN, OP, PTR)) // Call C function
	OPINFO2(RECOVER, "recover",  4,   false, NONE, NONE, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, MVAR)) // Get value from mapvar

	// Internal Register Operations
	OPINFO1(SETFMOD, "setfmod",  4,   false, NONE, NONE, ALL,  PINFO(IN, OP, IANY)) // Set floating point control mode
	OPINFO1(GETFMOD, "getfmod",  4,   false, NONE, NONE, ALL,  PINFO(OUT, OP, IRM)) // Get floating point control mode
	OPINFO1(GETEXP,  "getexp",   4,   false, NONE, NONE, ALL,  PINFO(OUT, OP, IRM)) // Get exception parameter value
	OPINFO2(GETFLGS, "getflgs",  4,   false, P2,   NONE, NONE, PINFO(OUT, OP, IRM), PINFO(IN, OP, IMV)) // Get status register flags
	OPINFO1(SETFLGS, "setflgs",  4,   false, NONE, ALL,  ALL,  PINFO(IN, OP, IANY)) // (for debugging) Set status register flags
	OPINFO1(SAVE,    "save",     4,   false, ALL,  NONE, ALL,  PINFO(OUT, OP, STATE)) // Save current state to drcuml_machine_state
	OPINFO1(RESTORE, "restore",  4,   false, NONE, ALL,  ALL,  PINFO(IN, OP, STATE)) // Load saved state from drcuml_machine_state

	// Integer Operations
	OPINFO4(LOAD,    "!load",    4|8, false, NONE, NONE, NONE, PINFO(OUT, OP, IRM), PINFO(IN, OP, PTR), PINFO(IN, 4, IANY), PINFO(IN, OP, SCSIZE)) // Load unsigned value from specified memory location
	OPINFO4(LOADS,   "!loads",   4|8, false, NONE, NONE, NONE, PINFO(OUT, OP, IRM), PINFO(IN, OP, PTR), PINFO(IN, 4, IANY), PINFO(IN, OP, SCSIZE)) // Load signed value from specified memory location
	OPINFO4(STORE,   "!store",   4|8, false, NONE, NONE, NONE, PINFO(IN, OP, PTR), PINFO(IN, 4, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, SCSIZE)) // Store value to specified memory location
	OPINFO3(READ,    "!read",    4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, 4, IANY), PINFO(IN, OP, SPSIZE)) // Read memory from emulated machine using memory space reader
	OPINFO4(READM,   "!readm",   4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, 4, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, SPSIZE)) // Read memory from emulated machine using memory space reader (masked)
	OPINFO3(WRITE,   "!write",   4|8, false, NONE, NONE, ALL,  PINFO(IN, 4, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, SPSIZE)) // Write to emulated machine's memory using memory space writer
	OPINFO4(WRITEM,  "!writem",  4|8, false, NONE, NONE, ALL,  PINFO(IN, 4, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, SPSIZE)) // Write to emulated machine's memory using memory space writer (masked)
	OPINFO2(CARRY,   "!carry",   4|8, false, NONE, C,    ALL,  PINFO(IN, OP, IANY), PINFO(IN, OP, IANY)) // Set carry status flag on CPU
	OPINFO1(SET,     "!set",     4|8, true,  NONE, NONE, NONE, PINFO(OUT, OP, IRM)) // Get the state of the specified condition (e.g. calling UML_SET with COND_NZ will return 0 if the condition is not met and 1 if the condition is met)
	OPINFO2(MOV,     "!mov",     4|8, true,  NONE, NONE, NONE, PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY))
	OPINFO3(SEXT,    "!sext",    4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, P3, IANY), PINFO(IN, OP, SIZE))
	OPINFO4(BFXU,    "!bfxu",    4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO4(BFXS,    "!bfxs",    4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO4(ROLAND,  "!roland",  4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY)) // Rotate left + AND (see drcbec.cpp for implementation)
	OPINFO4(ROLINS,  "!rolins",  4|8, false, NONE, SZ,   ALL,  PINFO(INOUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY)) // Rotate left + OR (see drcbec.cpp for implementation)
	OPINFO3(ADD,     "!add",     4|8, false, NONE, SZVC, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(ADDC,    "!addc",    4|8, false, C,    SZVC, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(SUB,     "!sub",     4|8, false, NONE, SZVC, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(SUBB,    "!subb",    4|8, false, C,    SZVC, ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO2(CMP,     "!cmp",     4|8, false, NONE, SZVC, ALL,  PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO4(MULU,    "!mulu",    4|8, false, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY)) // Unsigned 32x32=64 and 64x64=128 multiplication
	OPINFO3(MULULW,  "!mululw",  4|8, false, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY)) // Unsigned 32x32=32 and 64x64=64 multiplication (overflow set based on 32x32=64 calculation but zero and sign based on 32-bit result)
	OPINFO4(MULS,    "!muls",    4|8, false, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY)) // Signed 32x32=64 and 64x64=128 multiplication
	OPINFO3(MULSLW,  "!mulslw",  4|8, false, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY)) // Signed 32x32=32 and 64x64=64 multiplication (overflow set based on 32x32=64 calculation but zero and sign based on 32-bit result)
	OPINFO4(DIVU,    "!divu",    4|8, false, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO4(DIVS,    "!divs",    4|8, false, NONE, SZV,  ALL,  PINFO(OUT, OP, IRM), PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(AND,     "!and",     4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO2(TEST,    "!test",    4|8, false, NONE, SZ,   ALL,  PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(OR,      "!or",      4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(XOR,     "!xor",     4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO2(LZCNT,   "!lzcnt",   4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY))
	OPINFO2(TZCNT,   "!tzcnt",   4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY))
	OPINFO2(BSWAP,   "!bswap",   4|8, false, NONE, SZ,   ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY))
	OPINFO3(SHL,     "!shl",     4|8, false, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(SHR,     "!shr",     4|8, false, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(SAR,     "!sar",     4|8, false, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(ROL,     "!rol",     4|8, false, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(ROLC,    "!rolc",    4|8, false, C,    SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(ROR,     "!ror",     4|8, false, NONE, SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))
	OPINFO3(RORC,    "!rorc",    4|8, false, C,    SZC,  ALL,  PINFO(OUT, OP, IRM), PINFO(IN, OP, IANY), PINFO(IN, OP, IANY))

	// Floating Point Operations
	OPINFO3(FLOAD,   "f#load",   4|8, false, NONE, NONE, NONE, PINFO(OUT, OP, FRM), PINFO(IN, OP, PTR), PINFO(IN, 4, IANY)) // Load float/double value from specified memory location
	OPINFO3(FSTORE,  "f#store",  4|8, false, NONE, NONE, NONE, PINFO(IN, OP, PTR), PINFO(IN, 4, IANY), PINFO(IN, OP, FRM)) // Save float/double value to specified memory location
	OPINFO3(FREAD,   "f#read",   4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, 4, IANY), PINFO(IN, OP, SPSIZE)) // Read float/double value from emulated machine using memory space reader
	OPINFO3(FWRITE,  "f#write",  4|8, false, NONE, NONE, ALL,  PINFO(IN, 4, IANY), PINFO(IN, OP, FANY), PINFO(IN, OP, SPSIZE)) // Write float/double value to emulated machine using memory space writer
	OPINFO2(FMOV,    "f#mov",    4|8, true,  NONE, NONE, NONE, PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO4(FTOINT,  "f#toint",  4|8, false, NONE, NONE, ALL,  PINFO(OUT, P3, IRM), PINFO(IN, OP, FANY), PINFO(IN, OP, SIZE), PINFO(IN, OP, ROUND)) // Float/double to integer
	OPINFO3(FFRINT,  "f#frint",  4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, P3, IANY), PINFO(IN, OP, SIZE)) // Float/double from integer
	OPINFO3(FFRFLT,  "f#frflt",  4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, P3, FANY), PINFO(IN, OP, SIZE)) // Convert float to double or double to float
	OPINFO2(FRNDS,   "f#rnds",     8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, P3, FANY)) // Convert double to float and then back to double, or float to double and back to float
	OPINFO3(FADD,    "f#add",    4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO3(FSUB,    "f#sub",    4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO2(FCMP,    "f#cmp",    4|8, false, NONE, UZC,  ALL,  PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO3(FMUL,    "f#mul",    4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO3(FDIV,    "f#div",    4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY), PINFO(IN, OP, FANY))
	OPINFO2(FNEG,    "f#neg",    4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO2(FABS,    "f#abs",    4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO2(FSQRT,   "f#sqrt",   4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO2(FRECIP,  "f#recip",  4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO2(FRSQRT,  "f#rsqrt",  4|8, false, NONE, NONE, ALL,  PINFO(OUT, OP, FRM), PINFO(IN, OP, FANY))
	OPINFO2(FCOPYI,  "f#copyi",  4|8, false, NONE, NONE, NONE, PINFO(OUT, OP, FRM), PINFO(IN, OP, IRM)) // Load float/double value from integer representation (e.g. 0x3f800000 -> 1.0f)
	OPINFO2(ICOPYF,  "icopyf#",  4|8, false, NONE, NONE, NONE, PINFO(OUT, OP, IRM), PINFO(IN, OP, FRM)) // Store float/double value as integer representation (e.g. 1.0f -> 0x3f800000)
};



//**************************************************************************
//  UML CODE HANDLE
//**************************************************************************

//-------------------------------------------------
//  code_handle - constructor
//-------------------------------------------------

uml::code_handle::code_handle(drcuml_state &drcuml, const char *name)
	: m_code(reinterpret_cast<drccodeptr *>(drcuml.cache().alloc_near(sizeof(drccodeptr))))
	, m_string(name)
	, m_drcuml(drcuml)
{
	(void)m_drcuml; // without this, non-debug builds fail because the asserts are preprocessed out
	if (!m_code)
		throw std::bad_alloc();
	*m_code = nullptr;
}


//-------------------------------------------------
//  set_codeptr - set a new code pointer
//-------------------------------------------------

void uml::code_handle::set_codeptr(drccodeptr code)
{
	assert(*m_code == nullptr);
	assert_in_cache(m_drcuml.cache(), code);
	*m_code = code;
}



//**************************************************************************
//  UML INSTRUCTION
//**************************************************************************

struct uml::instruction::simplify_op
{
private:
	static inline constexpr u64 paramsizemask[] = { 0xffU, 0xffffU, 0xffffffffU, 0xffffffff'ffffffffU };

	static u64 size_mask(instruction const &inst)
	{
		return (inst.size() == 4) ? 0xffffffffU : 0xffffffff'ffffffffU;
	}

	static void truncate_immediate(instruction &inst, int pnum, u64 mask)
	{
		if (inst.param(pnum).is_immediate())
			inst.m_param[pnum] = inst.param(pnum).immediate() & mask;
	}

	static void convert_to_mov_immediate(instruction &inst, u64 immediate)
	{
		u64 const mask = size_mask(inst);
		immediate &= mask;
		if (!inst.flags())
		{
			inst.m_opcode = OP_MOV;
			inst.m_numparams = 2;
			inst.m_param[1] = immediate;
		}
		else if (immediate == mask)
		{
			inst.m_opcode = OP_OR;
			inst.m_numparams = 3;
			inst.m_param[1] = inst.param(0);
			inst.m_param[2] = mask;
		}
		else
		{
			inst.m_opcode = OP_AND;
			inst.m_numparams = 3;
			if (immediate)
			{
				inst.m_param[1] = immediate;
				inst.m_param[2] = mask;
			}
			else
			{
				inst.m_param[1] = inst.param(0);
				inst.m_param[2] = 0;
			}
		}
	}

public:
	static void truncate_imm(instruction &inst)
	{
		u64 const mask = size_mask(inst);

		for (int i = 0; inst.numparams() > i; ++i)
			truncate_immediate(inst, i, mask);
	}

	static void normalise_commutative(instruction &inst)
	{
		const u64 mask = size_mask(inst);

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, mask);

		// if a source is the destination put it first, and put a single immediate last
		if ((inst.param(0) == inst.param(2)) || (inst.param(1).is_immediate() && !inst.param(2).is_immediate()))
		{
			using std::swap;
			swap(inst.m_param[1], inst.m_param[2]);
		}
	}

	static void read(instruction &inst)
	{
		// truncate immediate address to size
		truncate_immediate(inst, 1, 0xffffffff);
	}

	static void readm(instruction &inst)
	{
		auto const size = inst.param(3).size();
		u64 const mask = paramsizemask[size];

		// truncate immediate address and mask to size
		truncate_immediate(inst, 1, 0xffffffff);
		truncate_immediate(inst, 2, mask);

		// convert to READ if the mask is all ones
		if (inst.param(2).is_immediate_value(mask))
		{
			inst.m_opcode = OP_READ;
			inst.m_param[2] = inst.param(3);
			inst.m_numparams = 3;
		}
	}

	static void write(instruction &inst)
	{
		auto const size = inst.param(2).size();

		// truncate immediate address and data to size
		truncate_immediate(inst, 0, 0xffffffff);
		truncate_immediate(inst, 1, paramsizemask[size]);
	}

	static void writem(instruction &inst)
	{
		auto const size = inst.param(3).size();
		u64 const mask = paramsizemask[size];

		// truncate immediate address, data and mask to size
		truncate_immediate(inst, 0, 0xffffffff);
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, mask);

		// convert to WRITE if the mask is all ones
		if (inst.param(2).is_immediate_value(mask))
		{
			inst.m_opcode = OP_WRITE;
			inst.m_param[2] = inst.param(3);
			inst.m_numparams = 3;
		}
	}

	static void carry(instruction &inst)
	{
		// truncate immediates to instruction size
		truncate_immediate(inst, 0, size_mask(inst));
		truncate_immediate(inst, 1, (inst.size() << 3) - 1);
	}

	static void set(instruction &inst)
	{
		// convert to MOV if the condition is constant
		if (inst.condition() == COND_ALWAYS)
			convert_to_mov_immediate(inst, 1);
	}

	static void mov(instruction &inst)
	{
		// convert move-to-self to NOP if the destination isn't larger than the size
		if (inst.param(0) == inst.param(1))
		{
			if (!inst.param(0).is_int_register() || (inst.size() == 8))
			{
				inst.nop();
				return;
			}
		}

		// truncate immediate source to instruction size
		truncate_immediate(inst, 1, size_mask(inst));
	}

	static void sext(instruction &inst)
	{
		// convert immediate source to MOV or a logic operation if flags are requested
		if (inst.param(1).is_immediate())
		{
			u64 val = inst.param(1).immediate();
			switch (inst.param(2).size())
			{
			case SIZE_BYTE:  val = u64(s64(s8(u8(val))));   break;
			case SIZE_WORD:  val = u64(s64(s16(u16(val)))); break;
			case SIZE_DWORD: val = u64(s64(s32(u32(val)))); break;
			case SIZE_QWORD:                                break;
			}
			convert_to_mov_immediate(inst, val);
		}
		else if ((1 << inst.param(2).size()) >= inst.size())
		{
			if (inst.flags())
			{
				inst.m_opcode = OP_AND;
				inst.m_param[2] = size_mask(inst);
			}
			else
			{
				inst.m_opcode = OP_MOV;
				inst.m_numparams = 2;
			}
		}
	}

	static void bfxu(instruction &inst)
	{
		auto const size = inst.size();
		auto const bits = size << 3;
		u64 const mask = size_mask(inst);
		assert((size == 4) || (size == 8));

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, bits - 1);
		truncate_immediate(inst, 3, bits - 1);

		if (inst.param(2).is_immediate() && inst.param(3).is_immediate())
		{
			auto const field = util::make_bitmask<u64>(inst.param(3).immediate());

			if (inst.param(1).is_immediate())
			{
				// constant result, convert to MOV or a logic operation
				auto const rot = inst.param(2).immediate();

				if (size == 4)
					convert_to_mov_immediate(inst, rotr_32(inst.param(1).immediate(), rot) & field);
				else
					convert_to_mov_immediate(inst, rotr_64(inst.param(1).immediate(), rot) & field);
			}
			else if (inst.param(2).is_immediate_value(0))
			{
				// no shift, convert to AND
				inst.m_opcode = OP_AND;
				inst.m_param[2] = field;
				inst.m_numparams = 3;
			}
			else if ((inst.param(2).immediate() + inst.param(3).immediate()) == bits)
			{
				// equivalent to right shift
				inst.m_opcode = OP_SHR;
				inst.m_numparams = 3;
			}
		}
		else if (inst.param(3).is_immediate_value(0))
		{
			// undefined behaviour - just generate zero
			convert_to_mov_immediate(inst, 0);
		}
	}

	static void bfxs(instruction &inst)
	{
		auto const size = inst.size();
		auto const bits = size << 3;
		u64 const mask = size_mask(inst);
		assert((size == 4) || (size == 8));

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, bits - 1);
		truncate_immediate(inst, 3, bits - 1);

		if (inst.param(2).is_immediate() && inst.param(3).is_immediate())
		{
			if (inst.param(1).is_immediate())
			{
				// constant result, convert to MOV or a logic operation
				auto const rot = inst.param(2).immediate() + inst.param(3).immediate();
				auto const shift = -s64(inst.param(3).immediate()) & (bits - 1);

				if (size == 4)
					convert_to_mov_immediate(inst, u32(s32(rotr_32(inst.param(1).immediate(), rot)) >> shift));
				else
					convert_to_mov_immediate(inst, u64(s64(rotr_64(inst.param(1).immediate(), rot)) >> shift));
			}
			else if (inst.param(2).is_immediate_value(0))
			{
				// no shift, convert to SEXT if possible
				switch (inst.param(3).immediate())
				{
				case 8:
					inst.m_opcode = OP_SEXT;
					inst.m_param[2] = parameter::make_size(SIZE_BYTE);
					inst.m_numparams = 3;
					break;
				case 16:
					inst.m_opcode = OP_SEXT;
					inst.m_param[2] = parameter::make_size(SIZE_WORD);
					inst.m_numparams = 3;
					break;
				case 32:
					inst.m_opcode = OP_SEXT;
					inst.m_param[2] = parameter::make_size(SIZE_DWORD);
					inst.m_numparams = 3;
					break;
				}
			}
			else if ((inst.param(2).immediate() + inst.param(3).immediate()) == bits)
			{
				// equivalent to right shift
				inst.m_opcode = OP_SAR;
				inst.m_numparams = 3;
			}
		}
		else if (inst.param(3).is_immediate_value(0))
		{
			// undefined behaviour - just generate zero
			convert_to_mov_immediate(inst, 0);
		}
	}

	static void roland(instruction &inst)
	{
		auto const size = inst.size();
		auto const bits = size << 3;
		u64 const mask = size_mask(inst);
		assert((size == 4) || (size == 8));

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, bits - 1);
		truncate_immediate(inst, 3, mask);

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate() && inst.param(3).is_immediate())
		{
			// constant result, convert to MOV or a logic operation
			if (size == 4)
				convert_to_mov_immediate(inst, rotl_32(inst.param(1).immediate(), inst.param(2).immediate()) & inst.param(3).immediate());
			else
				convert_to_mov_immediate(inst, rotl_64(inst.param(1).immediate(), inst.param(2).immediate()) & inst.param(3).immediate());
		}
		else if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// only mask is variable, convert to AND
			inst.m_opcode = OP_AND;
			if (size == 4)
				inst.m_param[1] = rotl_32(inst.param(1).immediate(), inst.param(2).immediate());
			else
				inst.m_param[1] = rotl_64(inst.param(1).immediate(), inst.param(2).immediate());
			inst.m_param[2] = inst.param(3);
			inst.m_numparams = 3;
		}
		else if (inst.param(2).is_immediate_value(0) || inst.param(3).is_immediate_value(0))
		{
			// no shift or zero mask, convert to AND (may be subsequently converted to MOV)
			inst.m_opcode = OP_AND;
			inst.m_param[2] = inst.param(3);
			inst.m_numparams = 3;
		}
		else if (inst.param(3).is_immediate_value(mask))
		{
			// all mask bits set, convert to ROL
			inst.m_opcode = OP_ROL;
			inst.m_numparams = 3;
		}
		else if (inst.param(2).is_immediate() && inst.param(3).is_immediate_value((mask << inst.param(2).immediate()) & mask))
		{
			// equivalent to shift left
			inst.m_opcode = OP_SHL;
			inst.m_numparams = 3;
		}
		else if (inst.param(2).is_immediate() && inst.param(3).is_immediate_value(mask >> (bits - inst.param(2).immediate())))
		{
			// equivalent to shift right
			inst.m_opcode = OP_SHR;
			inst.m_numparams = 3;
			inst.m_param[2] = bits - inst.param(2).immediate();
		}
		else if (inst.param(2).is_immediate() && inst.param(3).is_immediate() && !(inst.param(3).immediate() & (inst.param(3).immediate() + 1)))
		{
			// extract right-aligned field, convert to BFXU
			inst.m_opcode = OP_BFXU;
			inst.m_param[2] = bits - inst.param(2).immediate();
			inst.m_param[3] = 64 - count_leading_zeros_64(inst.param(3).immediate());
		}
	}

	static void rolins(instruction &inst)
	{
		auto const size = inst.size();
		auto const bits = size << 3;
		u64 const mask = size_mask(inst);

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, bits - 1);
		truncate_immediate(inst, 3, mask);

		if (inst.param(3).is_immediate_value(mask))
		{
			// all mask bits set, convert to ROL
			inst.m_opcode = OP_ROL;
			inst.m_numparams = 3;
		}
		else if (inst.param(3).is_immediate_value(0))
		{
			// no mask bits set, convert to AND
			inst.m_opcode = OP_AND;
			inst.m_numparams = 3;
			inst.m_param[1] = inst.param(0);
			inst.m_param[2] = mask;
		}
	}

	static void add(instruction &inst)
	{
		// clean up operands
		normalise_commutative(inst);

		// can't optimise carry or overflow flag generation
		if (inst.flags() & (FLAG_C | FLAG_V))
			return;

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// constant value, convert to MOV or a logic operation
			convert_to_mov_immediate(inst, inst.param(1).immediate() + inst.param(2).immediate());
		}
		else if (inst.param(2).is_immediate_value(0))
		{
			// add zero, convert to MOV or AND
			if (inst.flags())
			{
				inst.m_opcode = OP_AND;
				inst.m_param[2] = size_mask(inst);
			}
			else
			{
				inst.m_opcode = OP_MOV;
				inst.m_numparams = 2;
			}
		}
	}

	static void sub(instruction &inst)
	{
		u64 const mask = size_mask(inst);

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, mask);

		// can't optimise carry or overflow flag generation
		if (inst.flags() & (FLAG_C | FLAG_V))
			return;

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// constant value, convert to MOV or a logic operation
			convert_to_mov_immediate(inst, inst.param(1).immediate() - inst.param(2).immediate());
		}
		else if (inst.param(2).is_immediate_value(0))
		{
			// subtract zero, convert to MOV or AND
			if (inst.flags())
			{
				inst.m_opcode = OP_AND;
				inst.m_param[2] = mask;
			}
			else
			{
				inst.m_opcode = OP_MOV;
				inst.m_numparams = 2;
			}
		}
	}

	static void cmp(instruction &inst)
	{
		u64 const mask = size_mask(inst);

		// completely elide if flags are not used
		if (!inst.flags())
		{
			inst.nop();
			return;
		}

		// truncate immediates to instruction size
		truncate_immediate(inst, 0, mask);
		truncate_immediate(inst, 1, mask);
	}

	template <opcode_t LoWordOp, typename Short, typename Long>
	static void mul(instruction &inst)
	{
		bool const low_only = inst.param(0) == inst.param(1);

		// if the two destination operands are identical and the S and Z flags aren't required, convert to MUL.LW
		if (low_only && !(inst.flags() & (FLAG_Z | FLAG_S)))
		{
			inst.m_opcode = LoWordOp;
			inst.m_param[1] = inst.param(2);
			inst.m_param[2] = inst.param(3);
			inst.m_numparams = 3;
			return;
		}

		const u64 mask = size_mask(inst);

		// truncate immediates to instruction size
		truncate_immediate(inst, 2, mask);
		truncate_immediate(inst, 3, mask);

		// put a single immediate last
		if (inst.param(2).is_immediate() && !inst.param(3).is_immediate())
		{
			using std::swap;
			swap(inst.m_param[2], inst.m_param[3]);
		}

		// can only simplify the low-only form, can't optimise overflow flag generation
		if (!low_only || (inst.flags() & FLAG_V))
			return;

		if (inst.param(2).is_immediate_value(0) || inst.param(3).is_immediate_value(0))
		{
			// multiplying anything by zero yields zero
			convert_to_mov_immediate(inst, 0);
		}
		else if (inst.param(2).is_immediate() && inst.param(3).is_immediate())
		{
			// convert constant result to MOV or a logic op
			auto const size = inst.size();
			auto const bits = size << 3;
			assert((size == 4) || (size == 8));

			if (size == 4)
			{
				Short const param2 = Short(u32(inst.param(2).immediate()));
				Short const param3 = Short(u32(inst.param(3).immediate()));
				Short const val = param2 * param3;
				bool const no_overflow = (val / param2) == param3;
				bool const z_ok = !(inst.flags() & FLAG_Z) || no_overflow;
				bool const s_ok = !(inst.flags() & FLAG_S) || (std::is_signed_v<Short> ? !BIT(param2 ^ param3 ^ val, bits - 1) : (no_overflow && !BIT(val, bits - 1)));

				if (z_ok && s_ok)
					convert_to_mov_immediate(inst, u32(val));
			}
			else
			{
				Long const param2 = Long(inst.param(2).immediate());
				Long const param3 = Long(inst.param(3).immediate());
				Long const val = param2 * param3;
				bool const no_overflow = (val / param2) == param3;
				bool const z_ok = !(inst.flags() & FLAG_Z) || no_overflow;
				bool const s_ok = !(inst.flags() & FLAG_S) || (std::is_signed_v<Long> ? !BIT(param2 ^ param3 ^ val, bits - 1) : (no_overflow && !BIT(val, bits - 1)));

				if (z_ok && s_ok)
					convert_to_mov_immediate(inst, u64(val));
			}
		}
		else if (inst.param(3).is_immediate_value(1))
		{
			// multiplying by unity produces the same value
			if (!inst.flags())
			{
				inst.m_opcode = OP_MOV;
				inst.m_param[1] = inst.param(2);
				inst.m_numparams = 2;
			}
			else if (!(inst.flags() & FLAG_S) || (inst.opcode() == OP_MULS))
			{
				inst.m_opcode = OP_AND;
				inst.m_param[1] = inst.param(2);
				inst.m_param[2] = size_mask(inst);
				inst.m_numparams = 3;
			}
		}
	}

	template <typename Short, typename Long>
	static void mullw(instruction &inst)
	{
		// clean up operands
		normalise_commutative(inst);

		// can't optimise overflow flag generation
		if (inst.flags() & FLAG_V)
			return;

		if (inst.param(1).is_immediate_value(0) || inst.param(2).is_immediate_value(0))
		{
			// multiplying anything by zero yields zero
			convert_to_mov_immediate(inst, 0);
		}
		else if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// convert constant result to MOV or a logic op
			auto const size = inst.size();
			assert((size == 4) || (size == 8));

			if (size == 4)
			{
				Short const param1 = Short(u32(inst.param(1).immediate()));
				Short const param2 = Short(u32(inst.param(2).immediate()));
				Short const val = param1 * param2;
				convert_to_mov_immediate(inst, u32(val));
			}
			else
			{
				Long const param1 = Long(inst.param(1).immediate());
				Long const param2 = Long(inst.param(2).immediate());
				Long const val = param1 * param2;
				convert_to_mov_immediate(inst, u64(val));
			}
		}
		else if (inst.param(2).is_immediate_value(1))
		{
			// multiplying by unity produces the same value
			if (inst.flags())
			{
				inst.m_opcode = OP_AND;
				inst.m_param[2] = size_mask(inst);
			}
			else
			{
				inst.m_opcode = OP_MOV;
				inst.m_numparams = 2;
			}
		}
	}

	template <typename Short, typename Long>
	static void div(instruction &inst)
	{
		auto const size = inst.size();
		u64 const mask = size_mask(inst);

		// truncate immediates to instruction size
		truncate_immediate(inst, 2, mask);
		truncate_immediate(inst, 3, mask);

		// can't optimise remainder calculation or overflow flag generation
		if ((inst.param(0) != inst.param(1)) || (inst.flags() & FLAG_V))
			return;

		if (inst.param(2).is_immediate() && inst.param(3).is_immediate() && !inst.param(3).is_immediate_value(0))
		{
			// optimise the quotient-only form with two immediate inputs if not dividing by zero
			if (inst.param(2).is_immediate_value(0))
			{
				// dividing zero by anything yields zero
				convert_to_mov_immediate(inst, 0);
			}
			else
			{
				// convert constant result to MOV or a logic op
				assert((size == 4) || (size == 8));

				if (size == 4)
					convert_to_mov_immediate(inst, u32(Short(u32(inst.param(2).immediate())) / Short(u32(inst.param(3).immediate()))));
				else
					convert_to_mov_immediate(inst, u64(Long(inst.param(2).immediate()) / Long(inst.param(3).immediate())));
			}
		}
		else if (inst.param(3).is_immediate_value(1))
		{
			// dividing by unity produces the dividend
			if (inst.flags())
			{
				inst.m_opcode = OP_AND;
				inst.m_param[1] = inst.param(2);
				inst.m_param[2] = size_mask(inst);
				inst.m_numparams = 3;
			}
			else
			{
				inst.m_opcode = OP_MOV;
				inst.m_param[1] = inst.param(2);
				inst.m_numparams = 2;
			}
		}
	}

	static void _and(instruction &inst)
	{
		// clean up operands
		normalise_commutative(inst);

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// constant value, convert to MOV or a logic operation
			convert_to_mov_immediate(inst, inst.param(1).immediate() & inst.param(2).immediate());
		}
		else if (inst.param(2).is_immediate_value(0))
		{
			// any immediate zero always yields zero
			convert_to_mov_immediate(inst, 0);
		}
		else if (inst.param(2).is_immediate_value(size_mask(inst)) || (inst.param(1) == inst.param(2)))
		{
			if (!inst.flags())
			{
				// convert to MOV if the value will be unaffected and flags aren't updated
				inst.m_opcode = OP_MOV;
				inst.m_numparams = 2;
			}
			else if ((inst.param(0) == inst.param(1)) && (!inst.param(0).is_int_register() || (inst.size() == 8)))
			{
				// convert to TEST if the value will be unaffected and the destination is no larger than the operand size
				inst.m_opcode = OP_TEST;
				inst.m_numparams = 2;
			}
		}
	}

	static void test(instruction &inst)
	{
		u64 const mask = size_mask(inst);

		// completely elide if flags are not used
		if (!inst.flags())
		{
			inst.nop();
			return;
		}

		// truncate immediates to instruction size
		truncate_immediate(inst, 0, mask);
		truncate_immediate(inst, 1, mask);

		// put a single immediate second
		if (inst.param(0).is_immediate() && !inst.param(1).is_immediate())
		{
			using std::swap;
			swap(inst.m_param[0], inst.m_param[1]);
		}

		if (inst.param(0).is_immediate() && inst.param(1).is_immediate())
		{
			// two immediates, combine values and set second operand to all 0 or all 1
			u64 const val = inst.param(0).immediate() & inst.param(1).immediate();
			inst.m_param[0] = val;
			inst.m_param[1] = val ? mask : 0;
		}
		else if (inst.param(1).is_immediate_value(0))
		{
			// testing against zero always produces the same result
			inst.m_param[0] = 0;
		}
		else if (inst.param(0) == inst.param(1))
		{
			// testing a value against itself, turn the second operand into an immediate
			inst.m_param[1] = mask;
		}
	}

	static void _or(instruction &inst)
	{
		u64 const mask = size_mask(inst);

		// clean up operands
		normalise_commutative(inst);

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// constant value, convert to MOV or a logic operation
			convert_to_mov_immediate(inst, inst.param(1).immediate() | inst.param(2).immediate());
		}
		else if (inst.param(2).is_immediate_value(mask))
		{
			// an immediate with all bits set is unaffected by the other value
			convert_to_mov_immediate(inst, mask);
		}
		else if (inst.param(2).is_immediate_value(0) || (inst.param(1) == inst.param(2)))
		{
			if (!inst.flags())
			{
				// convert to MOV if the value will be unaffected and flags aren't updated
				inst.m_opcode = OP_MOV;
				inst.m_numparams = 2;
			}
			else if ((inst.param(0) == inst.param(1)) && (!inst.param(0).is_int_register() || (inst.size() == 8)))
			{
				// convert to TEST if the value will be unaffected and the destination is no larger than the operand size
				inst.m_opcode = OP_TEST;
				inst.m_numparams = 2;
			}
			else
			{
				// convert to AND to simplify code generation
				inst.m_opcode = OP_AND;
				inst.m_param[2] = mask;
			}
		}
	}

	static void _xor(instruction &inst)
	{
		// clean up operands
		normalise_commutative(inst);

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// constant value, convert to MOV or a logic operation
			convert_to_mov_immediate(inst, inst.param(1).immediate() ^ inst.param(2).immediate());
		}
		else if (inst.param(1) == inst.param(2))
		{
			// equivalent values cancel out
			convert_to_mov_immediate(inst, 0);
		}
		else if (inst.param(2).is_immediate_value(0))
		{
			if (!inst.flags())
			{
				// convert to MOV if the value will be unaffected and flags aren't updated
				inst.m_opcode = OP_MOV;
				inst.m_numparams = 2;
			}
			else if ((inst.param(0) == inst.param(1)) && (!inst.param(0).is_int_register() || (inst.size() == 8)))
			{
				// convert to TEST if the value will be unaffected and the destination is no larger than the operand size
				inst.m_opcode = OP_TEST;
				inst.m_numparams = 2;
			}
			else
			{
				// convert to AND to simplify code generation
				inst.m_opcode = OP_AND;
				inst.m_param[2] = size_mask(inst);
			}
		}
	}

	static void lzcnt(instruction &inst)
	{
		// convert immediate source to MOV or a logic operation if flags are requested
		if (inst.param(1).is_immediate())
		{
			auto const size = inst.size();
			assert((size == 4) || (size == 8));

			u64 const val = inst.param(1).immediate();
			convert_to_mov_immediate(inst, (size == 4) ? count_leading_zeros_32(u32(val)) : count_leading_zeros_64(val));
		}
	}

	static void bswap(instruction &inst)
	{
		// convert immediate source to MOV or a logic operation if flags are requested
		if (inst.param(1).is_immediate())
		{
			auto const size = inst.size();
			assert((size == 4) || (size == 8));

			u64 const val = inst.param(1).immediate();
			convert_to_mov_immediate(inst, (size == 4) ? swapendian_int32(u32(val)) : swapendian_int64(val));
		}
	}

	static void shl(instruction &inst)
	{
		auto const size = inst.size();
		auto const bits = size << 3;
		u64 const mask = size_mask(inst);
		assert((size == 4) || (size == 8));

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, bits - 1);

		// can't optimise carry flag generation
		if (inst.flags() & FLAG_C)
			return;

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// constant result, convert to MOV or a logic operation
			if (size == 4)
				convert_to_mov_immediate(inst, u32(inst.param(1).immediate()) << inst.param(2).immediate());
			else
				convert_to_mov_immediate(inst, inst.param(1).immediate() << inst.param(2).immediate());
		}
		else if (inst.param(2).is_immediate_value(0))
		{
			// no shift, convert to AND (may be subsequently converted to MOV)
			inst.m_opcode = OP_AND;
			inst.m_param[2] = mask;
		}
	}

	template <typename Short, typename Long>
	static void shr(instruction &inst)
	{
		auto const size = inst.size();
		auto const bits = size << 3;
		u64 const mask = size_mask(inst);
		assert((size == 4) || (size == 8));

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, bits - 1);

		// can't optimise carry flag generation
		if (inst.flags() & FLAG_C)
			return;

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// constant result, convert to MOV or a logic operation
			if (size == 4)
				convert_to_mov_immediate(inst, u32(Short(u32(inst.param(1).immediate())) >> inst.param(2).immediate()));
			else
				convert_to_mov_immediate(inst, u64(Long(inst.param(1).immediate()) >> inst.param(2).immediate()));
		}
		else if (inst.param(2).is_immediate_value(0))
		{
			// no shift, convert to AND (may be subsequently converted to MOV)
			inst.m_opcode = OP_AND;
			inst.m_param[2] = mask;
		}
	}

	static void rol(instruction &inst)
	{
		auto const size = inst.size();
		auto const bits = size << 3;
		u64 const mask = size_mask(inst);
		assert((size == 4) || (size == 8));

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, bits - 1);

		// can't optimise carry flag generation
		if (inst.flags() & FLAG_C)
			return;

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// constant result, convert to MOV or a logic operation
			if (size == 4)
				convert_to_mov_immediate(inst, rotl_32(inst.param(1).immediate(), inst.param(2).immediate()));
			else
				convert_to_mov_immediate(inst, rotl_64(inst.param(1).immediate(), inst.param(2).immediate()));
		}
		else if (inst.param(2).is_immediate_value(0))
		{
			// no shift, convert to AND (may be subsequently converted to MOV)
			inst.m_opcode = OP_AND;
			inst.m_param[2] = mask;
		}
	}

	static void ror(instruction &inst)
	{
		auto const size = inst.size();
		auto const bits = size << 3;
		u64 const mask = size_mask(inst);
		assert((size == 4) || (size == 8));

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, bits - 1);

		// can't optimise carry flag generation
		if (inst.flags() & FLAG_C)
			return;

		if (inst.param(1).is_immediate() && inst.param(2).is_immediate())
		{
			// constant result, convert to MOV or a logic operation
			if (size == 4)
				convert_to_mov_immediate(inst, rotr_32(inst.param(1).immediate(), inst.param(2).immediate()));
			else
				convert_to_mov_immediate(inst, rotr_64(inst.param(1).immediate(), inst.param(2).immediate()));
		}
		else if (inst.param(2).is_immediate_value(0))
		{
			// no shift, convert to AND (may be subsequently converted to MOV)
			inst.m_opcode = OP_AND;
			inst.m_param[2] = mask;
		}
	}

	static void rolrc(instruction &inst)
	{
		auto const size = inst.size();
		auto const bits = size << 3;
		u64 const mask = size_mask(inst);
		assert((size == 4) || (size == 8));

		// truncate immediates to instruction size
		truncate_immediate(inst, 1, mask);
		truncate_immediate(inst, 2, bits - 1);

		// can't optimise zero or sign flag generation
		if (inst.flags() & (FLAG_Z | FLAG_S))
			return;

		// convert to NOP or MOV if there's no rotation
		if (inst.param(2).is_immediate_value(0))
		{
			inst.m_flags &= ~FLAG_C; // carry flag unchanged after zero-bit rotate, MOV and NOP won't change carry flag
			if ((inst.param(0) == inst.param(1)) && (!inst.param(0).is_int_register() || (inst.size() == 8)))
			{
				inst.nop();
			}
			else
			{
				inst.m_opcode = OP_MOV;
				inst.m_numparams = 2;
			}
		}
	}

	static void fmov(instruction &inst)
	{
		// convert move-to-self to NOP
		if (inst.param(0) == inst.param(1))
			inst.nop();
	}

	static void fread(instruction &inst)
	{
		// truncate immediate address to size
		truncate_immediate(inst, 1, 0xffffffff);
	}

	static void fwrite(instruction &inst)
	{
		// truncate immediate address to size
		truncate_immediate(inst, 0, 0xffffffff);
	}

	static void ffrint(instruction &inst)
	{
		// truncate immediate source to size
		if (inst.param(2).size() == SIZE_DWORD)
			truncate_immediate(inst, 1, 0xffffffff);
	}

	static void ffrflt(instruction &inst)
	{
		// convert to FMOV or NOP if the source and destination formats match
		auto const dst = inst.size();
		auto const src = inst.param(2).size();
		if (((4 == dst) && (SIZE_DWORD == src)) || ((8 == dst) && (SIZE_QWORD == src)))
		{
			if (inst.param(0) == inst.param(1))
			{
				inst.nop();
			}
			else
			{
				inst.m_opcode = OP_FMOV;
				inst.m_numparams = 2;
			}
		}
	}
};


//-------------------------------------------------
//  configure - configure an opcode with no
//  parameters
//-------------------------------------------------

void uml::instruction::configure(opcode_t op, u8 size, condition_t condition)
{
	// fill in the instruction
	m_opcode = opcode_t(u8(op));
	m_size = size;
	m_condition = condition;
	m_flags = 0;
	m_numparams = 0;

	// validate
	validate();
}


//-------------------------------------------------
//  configure - configure an opcode with 1
//  parameter
//-------------------------------------------------

void uml::instruction::configure(opcode_t op, u8 size, parameter p0, condition_t condition)
{
	// fill in the instruction
	m_opcode = opcode_t(u8(op));
	m_size = size;
	m_condition = condition;
	m_flags = 0;
	m_numparams = 1;
	m_param[0] = p0;

	// validate
	validate();
}


//-------------------------------------------------
//  configure - configure an opcode with 2
//  parameters
//-------------------------------------------------

void uml::instruction::configure(opcode_t op, u8 size, parameter p0, parameter p1, condition_t condition)
{
	// fill in the instruction
	m_opcode = opcode_t(u8(op));
	m_size = size;
	m_condition = condition;
	m_flags = 0;
	m_numparams = 2;
	m_param[0] = p0;
	m_param[1] = p1;

	// validate
	validate();
}


//-------------------------------------------------
//  configure - configure an opcode with 3
//  parameters
//-------------------------------------------------

void uml::instruction::configure(opcode_t op, u8 size, parameter p0, parameter p1, parameter p2, condition_t condition)
{
	// fill in the instruction
	m_opcode = opcode_t(u8(op));
	m_size = size;
	m_condition = condition;
	m_flags = 0;
	m_numparams = 3;
	m_param[0] = p0;
	m_param[1] = p1;
	m_param[2] = p2;

	// validate
	validate();
}


//-------------------------------------------------
//  configure - configure an opcode with 4
//  parameters
//-------------------------------------------------

void uml::instruction::configure(opcode_t op, u8 size, parameter p0, parameter p1, parameter p2, parameter p3, condition_t condition)
{
	// fill in the instruction
	m_opcode = opcode_t(u8(op));
	m_size = size;
	m_condition = condition;
	m_flags = 0;
	m_numparams = 4;
	m_param[0] = p0;
	m_param[1] = p1;
	m_param[2] = p2;
	m_param[3] = p3;

	// validate
	validate();
}


//-------------------------------------------------
//  simplify - simplify instructions that have
//  immediate values we can evaluate at compile
//  time
//-------------------------------------------------

void uml::instruction::simplify()
{
	// loop until we've simplified all we can
	opcode_t origop;
	do
	{
#if LOG_SIMPLIFICATIONS
		uml::instruction const orig = *this;
#endif

		// switch off the opcode
		origop = m_opcode;
		switch (m_opcode)
		{
		case OP_DEBUG:  simplify_op::truncate_imm(*this);             break;
		case OP_EXIT:   simplify_op::truncate_imm(*this);             break;
		case OP_EXH:    simplify_op::truncate_imm(*this);             break;
		case OP_READ:   simplify_op::read(*this);                     break;
		case OP_READM:  simplify_op::readm(*this);                    break;
		case OP_WRITE:  simplify_op::write(*this);                    break;
		case OP_WRITEM: simplify_op::writem(*this);                   break;
		case OP_CARRY:  simplify_op::carry(*this);                    break;
		case OP_SET:    simplify_op::set(*this);                      break;
		case OP_MOV:    simplify_op::mov(*this);                      break;
		case OP_SEXT:   simplify_op::sext(*this);                     break;
		case OP_BFXU:   simplify_op::bfxu(*this);                     break;
		case OP_BFXS:   simplify_op::bfxs(*this);                     break;
		case OP_ROLAND: simplify_op::roland(*this);                   break;
		case OP_ROLINS: simplify_op::rolins(*this);                   break;
		case OP_ADD:    simplify_op::add(*this);                      break;
		case OP_ADDC:   simplify_op::normalise_commutative(*this);    break;
		case OP_SUB:    simplify_op::sub(*this);                      break;
		case OP_SUBB:   simplify_op::truncate_imm(*this);             break;
		case OP_CMP:    simplify_op::cmp(*this);                      break;
		case OP_MULU:   simplify_op::mul<OP_MULULW, u32, u64>(*this); break;
		case OP_MULULW: simplify_op::mullw<u32, u64>(*this);          break;
		case OP_MULS:   simplify_op::mul<OP_MULSLW, s32, s64>(*this); break;
		case OP_MULSLW: simplify_op::mullw<s32, s64>(*this);          break;
		case OP_DIVU:   simplify_op::div<u32, u64>(*this);            break;
		case OP_DIVS:   simplify_op::div<s32, s64>(*this);            break;
		case OP_AND:    simplify_op::_and(*this);                     break;
		case OP_TEST:   simplify_op::test(*this);                     break;
		case OP_OR:     simplify_op::_or(*this);                      break;
		case OP_XOR:    simplify_op::_xor(*this);                     break;
		case OP_LZCNT:  simplify_op::lzcnt(*this);                    break;
		case OP_TZCNT:  simplify_op::truncate_imm(*this);             break;
		case OP_BSWAP:  simplify_op::bswap(*this);                    break;
		case OP_SHL:    simplify_op::shl(*this);                      break;
		case OP_SHR:    simplify_op::shr<u32, u64>(*this);            break;
		case OP_SAR:    simplify_op::shr<s32, s64>(*this);            break;
		case OP_ROL:    simplify_op::rol(*this);                      break;
		case OP_ROLC:   simplify_op::rolrc(*this);                    break;
		case OP_ROR:    simplify_op::ror(*this);                      break;
		case OP_RORC:   simplify_op::rolrc(*this);                    break;
		case OP_FREAD:  simplify_op::fread(*this);                    break;
		case OP_FWRITE: simplify_op::fwrite(*this);                   break;
		case OP_FFRINT: simplify_op::ffrint(*this);                   break;
		case OP_FFRFLT: simplify_op::ffrflt(*this);                   break;

		default:                                                      break;
		}

#if LOG_SIMPLIFICATIONS
		if (orig != *this)
			osd_printf_debug("Simplified: %-50.50s -> %s\n", orig.disasm(), disasm());
#endif

		// loop until we stop changing opcodes
	}
	while (m_opcode != origop);
}


//-------------------------------------------------
//  validate - verify that the instruction created
//  meets all requirements
//-------------------------------------------------

void uml::instruction::validate()
{
#ifdef MAME_DEBUG
	const opcode_info &opinfo = s_opcode_info_table[m_opcode];
	assert(opinfo.opcode == m_opcode);

	// validate raw information
	assert(m_opcode != OP_INVALID && m_opcode < OP_MAX);
	assert(m_size == 1 || m_size == 2 || m_size == 4 || m_size == 8);

	// validate against opcode limits
	assert((opinfo.sizes & m_size) != 0);
	assert(m_condition == COND_ALWAYS || opinfo.condition);

	// validate each parameter
	for (int pnum = 0; pnum < m_numparams; pnum++)
	{
		// ensure the type is valid
		const parameter &param = m_param[pnum];
		assert((opinfo.param[pnum].typemask >> param.type()) & 1);
		(void)param;
	}

	// make sure we aren't missing any parameters
	if (m_numparams < std::size(opinfo.param))
		assert(opinfo.param[m_numparams].typemask == 0);
#endif // MAME_DEBUG
}


//-------------------------------------------------
//  input_flags - return the effective input flags
//  based on any conditions encoded in an
//  instruction
//-------------------------------------------------

u8 uml::instruction::input_flags() const
{
	static constexpr u8 flags_for_condition[] =
	{
		FLAG_Z,                     // COND_Z
		FLAG_Z,                     // COND_NZ
		FLAG_S,                     // COND_S
		FLAG_S,                     // COND_NS
		FLAG_C,                     // COND_C
		FLAG_C,                     // COND_NC
		FLAG_V,                     // COND_V
		FLAG_V,                     // COND_NV
		FLAG_U,                     // COND_U
		FLAG_U,                     // COND_NU
		FLAG_C | FLAG_Z,            // COND_A
		FLAG_C | FLAG_Z,            // COND_BE
		FLAG_S | FLAG_V | FLAG_Z,   // COND_G
		FLAG_S | FLAG_V | FLAG_Z,   // COND_LE
		FLAG_S | FLAG_V,            // COND_L
		FLAG_S | FLAG_V             // COND_GE
	};

	u8 flags = s_opcode_info_table[m_opcode].inflags;
	if (flags & 0x80)
		flags = m_param[flags - OPFLAGS_P1].immediate() & OPFLAGS_ALL;
	if (m_condition != COND_ALWAYS)
		flags |= flags_for_condition[m_condition & 0x0f];
	return flags;
}


//-------------------------------------------------
//  output_flags - return the effective output
//  flags based on any conditions encoded in an
//  instruction
//-------------------------------------------------

u8 uml::instruction::output_flags() const
{
	u8 flags = s_opcode_info_table[m_opcode].outflags;
	if (flags & 0x80)
		flags = m_param[flags - OPFLAGS_P1].immediate() & OPFLAGS_ALL;
	return flags;
}


//-------------------------------------------------
//  modified_flags - return the effective output
//  flags based on any conditions encoded in an
//  instruction
//-------------------------------------------------

u8 uml::instruction::modified_flags() const
{
	return s_opcode_info_table[m_opcode].modflags;
}


//-------------------------------------------------
//  disasm - disassemble an instruction to the
//  given buffer
//-------------------------------------------------

std::string uml::instruction::disasm(drcuml_state *drcuml) const
{
	static char const *const conditions[] = { "z", "nz", "s", "ns", "c", "nc", "v", "nv", "u", "nu", "a", "be", "g", "le", "l", "ge" };
	static char const *const pound_size[] = { "?", "?", "?", "?", "s", "?", "?", "?", "d" };
	static char const *const bang_size[] = { "?", "b", "h", "?", "", "?", "?", "?", "d" };
	static char const *const fmods[] = { "trunc", "round", "ceil", "floor", "default" };
	static char const *const spaces[] = { "program", "data", "io", "3", "4", "5", "6", "7" };
	static char const *const sizes[] = { "byte", "word", "dword", "qword" };

	assert(m_opcode != OP_INVALID && m_opcode < OP_MAX);

	opcode_info const &opinfo = s_opcode_info_table[m_opcode];

	// start with the raw mnemonic and substitute sizes
	std::ostringstream buffer;
	for (char const *opsrc = opinfo.mnemonic; *opsrc != 0; opsrc++)
		if (*opsrc == '!')
			util::stream_format(buffer, "%s", bang_size[m_size]);
		else if (*opsrc == '#')
			util::stream_format(buffer, "%s", pound_size[m_size]);
		else
			util::stream_format(buffer, "%c", *opsrc);

	// pad to 8 spaces
	for (int pad = 8 - buffer.tellp(); (pad > 0); --pad)
		buffer.put(' ');

	// iterate through parameters
	for (int pnum = 0; pnum < m_numparams; pnum++)
	{
		const parameter &param = m_param[pnum];

		// start with a comma for all except the first parameter
		if (pnum != 0)
			buffer.put(',');

		// ouput based on type
		switch (param.type())
		{
		// immediates have several special cases
		case parameter::PTYPE_IMMEDIATE:
			{
				// determine the size of the immediate
				int size;
				switch (opinfo.param[pnum].size)
				{
					case PSIZE_4:   size = 4; break;
					case PSIZE_8:   size = 8; break;
					case PSIZE_P1:  size = 1 << m_param[0].size(); break;
					case PSIZE_P2:  size = 1 << m_param[1].size(); break;
					case PSIZE_P3:  size = 1 << m_param[2].size(); break;
					case PSIZE_P4:  size = 1 << m_param[3].size(); break;
					default:
					case PSIZE_OP:  size = m_size; break;
				}

				// truncate to size
				u64 value = param.immediate();
				if (size == 1) value = u8(value);
				if (size == 2) value = u16(value);
				if (size == 4) value = u32(value);
				util::stream_format(buffer, "$%X", value);
			}
			break;

		// immediates have several special cases
		case parameter::PTYPE_SIZE:
			util::stream_format(buffer, "%s", sizes[param.size()]);
			break;

		// size + address space immediate
		case parameter::PTYPE_SIZE_SPACE:
			util::stream_format(buffer, "%s_%s", spaces[param.space()], sizes[param.size()]);
			break;

		// size + scale immediate
		case parameter::PTYPE_SIZE_SCALE:
			{
				int const scale = param.scale();
				int const size  = param.size();
				if (scale == size)
					util::stream_format(buffer, "%s", sizes[size]);
				else
					util::stream_format(buffer, "%s_x%d", sizes[size], 1 << scale);
			}
			break;

		// fmod immediate
		case parameter::PTYPE_ROUNDING:
			util::stream_format(buffer, "%s", fmods[param.rounding()]);
			break;

		// integer registers
		case parameter::PTYPE_INT_REGISTER:
			util::stream_format(buffer, "i%d", param.ireg() - REG_I0);
			break;

		// floating point registers
		case parameter::PTYPE_FLOAT_REGISTER:
			util::stream_format(buffer, "f%d", param.freg() - REG_F0);
			break;

		// map variables
		case parameter::PTYPE_MAPVAR:
			util::stream_format(buffer, "m%d", param.mapvar() - MAPVAR_M0);
			break;

		// memory
		case parameter::PTYPE_MEMORY:
			{
				const char *symbol;
				u32 symoffset;

				if (drcuml && (symbol = drcuml->symbol_find(param.memory(), &symoffset)) != nullptr)
				{
					// symbol
					if (symoffset == 0)
						util::stream_format(buffer, "[%s]", symbol);
					else
						util::stream_format(buffer, "[%s+$%X]", symbol, symoffset);
				}
				else if (drcuml != nullptr && drcuml->cache().contains_pointer(param.memory()))
				{
					// cache memory
					util::stream_format(buffer, "[+$%X]", u32(uintptr_t(drccodeptr(param.memory()) - drcuml->cache().near())));
				}
				else
				{
					// general memory
					util::stream_format(buffer, "[[$%p]]", param.memory());
				}
			}
			break;

		// string pointer
		case parameter::PTYPE_STRING:
			util::stream_format(buffer, "%s", reinterpret_cast<char const *>(uintptr_t(param.string())));
			break;

		// handle pointer
		case parameter::PTYPE_CODE_HANDLE:
			util::stream_format(buffer, "%s", param.handle().string());
			break;

		// label
		case parameter::PTYPE_CODE_LABEL:
			util::stream_format(buffer, "$%8X", param.label().label());
			break;

		default:
			util::stream_format(buffer, "???");
			break;
		}
	}

	// if there's a condition, append it
	if (m_condition != COND_ALWAYS)
		util::stream_format(buffer, ",%s", conditions[m_condition & 0x0f]);

	// if there are flags, append them
	if (m_flags != 0)
	{
		buffer.put(',');
		if (m_flags & FLAG_U)
			buffer.put('U');
		if (m_flags & FLAG_S)
			buffer.put('S');
		if (m_flags & FLAG_Z)
			buffer.put('Z');
		if (m_flags & FLAG_V)
			buffer.put('V');
		if (m_flags & FLAG_C)
			buffer.put('C');
	}
	return std::move(buffer).str();
}
