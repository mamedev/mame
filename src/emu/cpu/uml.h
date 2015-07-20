// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    uml.h

    Universal machine language definitions and classes.

***************************************************************************/

#pragma once

#ifndef __UML_H__
#define __UML_H__

#include "drccache.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// opaque structure describing UML generation state
class drcuml_state;

struct drcuml_machine_state;


// use a namespace to wrap all the UML instruction concepts so that
// we can keep names short
namespace uml
{
	// integer registers
	const int REG_I0 = 0x400;
	const int REG_I_COUNT = 10;
	const int REG_I_END = REG_I0 + REG_I_COUNT;

	// floating point registers
	const int REG_F0 = 0x800;
	const int REG_F_COUNT = 10;
	const int REG_F_END = REG_F0 + REG_F_COUNT;

	// vector registers
	const int REG_V0 = 0xc00;
	const int REG_V_COUNT = 10;
	const int REG_V_END = REG_V0 + REG_V_COUNT;

	// map variables
	const int MAPVAR_M0 = 0x1000;
	const int MAPVAR_COUNT = 10;
	const int MAPVAR_END = MAPVAR_M0 + MAPVAR_COUNT;

	// flag definitions
	const UINT8 FLAG_C = 0x01;      // carry flag
	const UINT8 FLAG_V = 0x02;      // overflow flag (defined for integer only)
	const UINT8 FLAG_Z = 0x04;      // zero flag
	const UINT8 FLAG_S = 0x08;      // sign flag (defined for integer only)
	const UINT8 FLAG_U = 0x10;      // unordered flag (defined for FP only)

	// testable conditions; note that these are defined such that (condition ^ 1) is
	// always the opposite
	enum condition_t
	{
		COND_ALWAYS = 0,

		COND_Z = 0x80,              // requires Z
		COND_NZ,                    // requires Z
		COND_S,                     // requires S
		COND_NS,                    // requires S
		COND_C,                     // requires C
		COND_NC,                    // requires C
		COND_V,                     // requires V
		COND_NV,                    // requires V
		COND_U,                     // requires U
		COND_NU,                    // requires U
		COND_A,                     // requires CZ
		COND_BE,                    // requires CZ
		COND_G,                     // requires SVZ
		COND_LE,                    // requires SVZ
		COND_L,                     // requires SV
		COND_GE,                    // requires SV

		COND_MAX,

		// basic condition code aliases
		COND_E = COND_Z,
		COND_NE = COND_NZ,
		COND_B = COND_C,
		COND_AE = COND_NC
	};

	// floating point rounding modes
	enum float_rounding_mode
	{
		ROUND_TRUNC = 0,            // truncate
		ROUND_ROUND,                // round
		ROUND_CEIL,                 // round up
		ROUND_FLOOR,                // round down
		ROUND_DEFAULT
	};

	// operand sizes
	enum operand_size
	{
		SIZE_BYTE = 0,              // 1-byte
		SIZE_WORD,                  // 2-byte
		SIZE_DWORD,                 // 4-byte
		SIZE_QWORD,                 // 8-byte
		SIZE_DQWORD,                // 16-byte (vector)
		SIZE_SHORT = SIZE_DWORD,    // 4-byte (float)
		SIZE_DOUBLE = SIZE_QWORD    // 8-byte (float)
	};

	// memory scale factors
	enum memory_scale
	{
		SCALE_x1 = 0,               // index * 1
		SCALE_x2,                   // index * 2
		SCALE_x4,                   // index * 4
		SCALE_x8,                   // index * 8
		SCALE_DEFAULT
	};

	// spaces
	enum memory_space
	{
		SPACE_PROGRAM = AS_PROGRAM,
		SPACE_DATA = AS_DATA,
		SPACE_IO = AS_IO
	};

	// opcodes
	enum opcode_t
	{
		OP_INVALID,

		// compile-time opcodes
		OP_HANDLE,                  // HANDLE  handle
		OP_HASH,                    // HASH    mode,pc
		OP_LABEL,                   // LABEL   imm
		OP_COMMENT,                 // COMMENT string
		OP_MAPVAR,                  // MAPVAR  mapvar,value

		// control flow operations
		OP_NOP,                     // NOP
		OP_DEBUG,                   // DEBUG   pc
		OP_EXIT,                    // EXIT    src1[,c]
		OP_HASHJMP,                 // HASHJMP mode,pc,handle
		OP_JMP,                     // JMP     imm[,c]
		OP_EXH,                     // EXH     handle,param[,c]
		OP_CALLH,                   // CALLH   handle[,c]
		OP_RET,                     // RET     [c]
		OP_CALLC,                   // CALLC   func,ptr[,c]
		OP_RECOVER,                 // RECOVER dst,mapvar

		// internal register operations
		OP_SETFMOD,                 // SETFMOD src
		OP_GETFMOD,                 // GETFMOD dst
		OP_GETEXP,                  // GETEXP  dst
		OP_GETFLGS,                 // GETFLGS dst[,f]
		OP_SAVE,                    // SAVE    mem
		OP_RESTORE,                 // RESTORE mem

		// integer operations
		OP_LOAD,                    // LOAD    dst,base,index,size
		OP_LOADS,                   // LOADS   dst,base,index,size
		OP_STORE,                   // STORE   base,index,src,size
		OP_READ,                    // READ    dst,src1,space/size
		OP_READM,                   // READM   dst,src1,mask,space/size
		OP_WRITE,                   // WRITE   dst,src1,space/size
		OP_WRITEM,                  // WRITEM  dst,mask,src1,space/size
		OP_CARRY,                   // CARRY   src,bitnum
		OP_SET,                     // SET     dst,c
		OP_MOV,                     // MOV     dst,src[,c]
		OP_SEXT,                    // SEXT    dst,src,size
		OP_ROLAND,                  // ROLAND  dst,src,shift,mask
		OP_ROLINS,                  // ROLINS  dst,src,shift,mask
		OP_ADD,                     // ADD     dst,src1,src2[,f]
		OP_ADDC,                    // ADDC    dst,src1,src2[,f]
		OP_SUB,                     // SUB     dst,src1,src2[,f]
		OP_SUBB,                    // SUBB    dst,src1,src2[,f]
		OP_CMP,                     // CMP     src1,src2[,f]
		OP_MULU,                    // MULU    dst,edst,src1,src2[,f]
		OP_MULS,                    // MULS    dst,edst,src1,src2[,f]
		OP_DIVU,                    // DIVU    dst,edst,src1,src2[,f]
		OP_DIVS,                    // DIVS    dst,edst,src1,src2[,f]
		OP_AND,                     // AND     dst,src1,src2[,f]
		OP_TEST,                    // TEST    src1,src2[,f]
		OP_OR,                      // OR      dst,src1,src2[,f]
		OP_XOR,                     // XOR     dst,src1,src2[,f]
		OP_LZCNT,                   // LZCNT   dst,src
		OP_BSWAP,                   // BSWAP   dst,src
		OP_SHL,                     // SHL     dst,src,count[,f]
		OP_SHR,                     // SHR     dst,src,count[,f]
		OP_SAR,                     // SAR     dst,src,count[,f]
		OP_ROL,                     // ROL     dst,src,count[,f]
		OP_ROLC,                    // ROLC    dst,src,count[,f]
		OP_ROR,                     // ROL     dst,src,count[,f]
		OP_RORC,                    // ROLC    dst,src,count[,f]

		// floating point operations
		OP_FLOAD,                   // FLOAD   dst,base,index
		OP_FSTORE,                  // FSTORE  base,index,src
		OP_FREAD,                   // FREAD   dst,space,src1
		OP_FWRITE,                  // FWRITE  space,dst,src1
		OP_FMOV,                    // FMOV    dst,src1[,c]
		OP_FTOINT,                  // FTOINT  dst,src1,size,round
		OP_FFRINT,                  // FFRINT  dst,src1,size
		OP_FFRFLT,                  // FFRFLT  dst,src1,size
		OP_FRNDS,                   // FRNDS   dst,src1
		OP_FADD,                    // FADD    dst,src1,src2
		OP_FSUB,                    // FSUB    dst,src1,src2
		OP_FCMP,                    // FCMP    src1,src2
		OP_FMUL,                    // FMUL    dst,src1,src2
		OP_FDIV,                    // FDIV    dst,src1,src2
		OP_FNEG,                    // FNEG    dst,src1
		OP_FABS,                    // FABS    dst,src1
		OP_FSQRT,                   // FSQRT   dst,src1
		OP_FRECIP,                  // FRECIP  dst,src1
		OP_FRSQRT,                  // FRSQRT  dst,src1

		OP_MAX
	};

	// C function callback deinition
	typedef void (*c_function)(void *ptr);

	// class describing a global code handle
	class code_handle
	{
		friend class ::drcuml_state;
		friend class ::simple_list<code_handle>;

		// construction/destruction
		code_handle(drcuml_state &drcuml, const char *name);

	public:
		// getters
		code_handle *next() const { return m_next; }
		drccodeptr codeptr() const { return *m_code; }
		drccodeptr *codeptr_addr() { return m_code; }
		const char *string() const { return m_string.c_str(); }

		// setters
		void set_codeptr(drccodeptr code);

	private:
		// internal state
		drccodeptr *            m_code;             // pointer in the cache to the associated code
		std::string             m_string;           // pointer to string attached to handle
		code_handle *           m_next;             // link to next handle in the list
		drcuml_state &          m_drcuml;           // pointer to owning object
	};

	// class describing a local code label
	class code_label
	{
	public:
		// construction
		code_label(UINT32 label = 0) : m_label(label) { }

		// operators
		operator UINT32 &() { return m_label; }
		bool operator==(const code_label &rhs) const { return (m_label == rhs.m_label); }
		bool operator!=(const code_label &rhs) const { return (m_label != rhs.m_label); }

		// getters
		UINT32 label() const { return m_label; }

	private:
		UINT32 m_label;
	};

	// a parameter for a UML instructon is encoded like this
	class parameter
	{
	public:
		// opcode parameter types
		enum parameter_type
		{
			PTYPE_NONE = 0,                     // invalid
			PTYPE_IMMEDIATE,                    // immediate; value = sign-extended to 64 bits
			PTYPE_INT_REGISTER,                 // integer register; value = REG_I0 - REG_I_END
			PTYPE_FLOAT_REGISTER,               // floating point register; value = REG_F0 - REG_F_END
			PTYPE_VECTOR_REGISTER,              // vector register; value = REG_V0 - REG_V_END
			PTYPE_MAPVAR,                       // map variable; value = MAPVAR_M0 - MAPVAR_END
			PTYPE_MEMORY,                       // memory; value = pointer to memory
			PTYPE_SIZE,                         // size; value = operand_size
			PTYPE_SIZE_SCALE,                   // scale + size; value = memory_scale * 16 + operand_size
			PTYPE_SIZE_SPACE,                   // space + size; value = memory_space * 16 + operand_size
			PTYPE_CODE_HANDLE,                  // code handle; value = pointer to handle
			PTYPE_CODE_LABEL,                   // code label; value = label index
			PTYPE_C_FUNCTION,                   // C function; value = pointer to C code
			PTYPE_ROUNDING,                     // floating point rounding mode; value = float_rounding_mode
			PTYPE_STRING,                       // string parameter; value = pointer to string
			PTYPE_MAX
		};

		// represents the value of an opcode parameter
		typedef UINT64 parameter_value;

		// construction
		parameter() : m_type(PTYPE_NONE), m_value(0) { }
		parameter(const parameter &param) : m_type(param.m_type), m_value(param.m_value) { }
		parameter(UINT64 val) : m_type(PTYPE_IMMEDIATE), m_value(val) { }
		parameter(operand_size size, memory_scale scale) : m_type(PTYPE_SIZE_SCALE), m_value((scale << 4) | size) { assert(size >= SIZE_BYTE && size <= SIZE_DQWORD); assert(scale >= SCALE_x1 && scale <= SCALE_x8); }
		parameter(operand_size size, memory_space space) : m_type(PTYPE_SIZE_SPACE), m_value((space << 4) | size) { assert(size >= SIZE_BYTE && size <= SIZE_DQWORD); assert(space >= SPACE_PROGRAM && space <= SPACE_IO); }
		parameter(code_handle &handle) : m_type(PTYPE_CODE_HANDLE), m_value(reinterpret_cast<parameter_value>(&handle)) { }
		parameter(code_label &label) : m_type(PTYPE_CODE_LABEL), m_value(label) { }

		// creators for types that don't safely default
		static inline parameter make_ireg(int regnum) { assert(regnum >= REG_I0 && regnum < REG_I_END); return parameter(PTYPE_INT_REGISTER, regnum); }
		static inline parameter make_freg(int regnum) { assert(regnum >= REG_F0 && regnum < REG_F_END); return parameter(PTYPE_FLOAT_REGISTER, regnum); }
		static inline parameter make_vreg(int regnum) { assert(regnum >= REG_V0 && regnum < REG_V_END); return parameter(PTYPE_VECTOR_REGISTER, regnum); }
		static inline parameter make_mapvar(int mvnum) { assert(mvnum >= MAPVAR_M0 && mvnum < MAPVAR_END); return parameter(PTYPE_MAPVAR, mvnum); }
		static inline parameter make_memory(void *base) { return parameter(PTYPE_MEMORY, reinterpret_cast<parameter_value>(base)); }
		static inline parameter make_memory(const void *base) { return parameter(PTYPE_MEMORY, reinterpret_cast<parameter_value>(const_cast<void *>(base))); }
		static inline parameter make_size(operand_size size) { assert(size >= SIZE_BYTE && size <= SIZE_DQWORD); return parameter(PTYPE_SIZE, size); }
		static inline parameter make_string(const char *string) { return parameter(PTYPE_STRING, reinterpret_cast<parameter_value>(const_cast<char *>(string))); }
		static inline parameter make_cfunc(c_function func) { return parameter(PTYPE_C_FUNCTION, reinterpret_cast<parameter_value>(func)); }
		static inline parameter make_rounding(float_rounding_mode mode) { assert(mode >= ROUND_TRUNC && mode <= ROUND_DEFAULT); return parameter(PTYPE_ROUNDING, mode); }

		// operators
		bool operator==(const parameter &rhs) const { return (m_type == rhs.m_type && m_value == rhs.m_value); }
		bool operator!=(const parameter &rhs) const { return (m_type != rhs.m_type || m_value != rhs.m_value); }

		// getters
		parameter_type type() const { return m_type; }
		UINT64 immediate() const { assert(m_type == PTYPE_IMMEDIATE); return m_value; }
		int ireg() const { assert(m_type == PTYPE_INT_REGISTER); assert(m_value >= REG_I0 && m_value < REG_I_END); return m_value; }
		int freg() const { assert(m_type == PTYPE_FLOAT_REGISTER); assert(m_value >= REG_F0 && m_value < REG_F_END); return m_value; }
		int vreg() const { assert(m_type == PTYPE_VECTOR_REGISTER); assert(m_value >= REG_V0 && m_value < REG_V_END); return m_value; }
		int mapvar() const { assert(m_type == PTYPE_MAPVAR); assert(m_value >= MAPVAR_M0 && m_value < MAPVAR_END); return m_value; }
		void *memory() const { assert(m_type == PTYPE_MEMORY); return reinterpret_cast<void *>(m_value); }
		operand_size size() const { assert(m_type == PTYPE_SIZE || m_type == PTYPE_SIZE_SCALE || m_type == PTYPE_SIZE_SPACE); return operand_size(m_value & 15); }
		memory_scale scale() const { assert(m_type == PTYPE_SIZE_SCALE); return memory_scale(m_value >> 4); }
		memory_space space() const { assert(m_type == PTYPE_SIZE_SPACE); return memory_space(m_value >> 4); }
		code_handle &handle() const { assert(m_type == PTYPE_CODE_HANDLE); return *reinterpret_cast<code_handle *>(m_value); }
		code_label label() const { assert(m_type == PTYPE_CODE_LABEL); return code_label(m_value); }
		c_function cfunc() const { assert(m_type == PTYPE_C_FUNCTION); return reinterpret_cast<c_function>(m_value); }
		float_rounding_mode rounding() const { assert(m_type == PTYPE_ROUNDING); return float_rounding_mode(m_value); }
		const char *string() const { assert(m_type == PTYPE_STRING); return reinterpret_cast<const char *>(m_value); }

		// type queries
		bool is_immediate() const { return (m_type == PTYPE_IMMEDIATE); }
		bool is_int_register() const { return (m_type == PTYPE_INT_REGISTER); }
		bool is_float_register() const { return (m_type == PTYPE_FLOAT_REGISTER); }
		bool is_vector_register() const { return (m_type == PTYPE_VECTOR_REGISTER); }
		bool is_mapvar() const { return (m_type == PTYPE_MAPVAR); }
		bool is_memory() const { return (m_type == PTYPE_MEMORY); }
		bool is_size() const { return (m_type == PTYPE_SIZE); }
		bool is_size_scale() const { return (m_type == PTYPE_SIZE_SCALE); }
		bool is_size_space() const { return (m_type == PTYPE_SIZE_SPACE); }
		bool is_code_handle() const { return (m_type == PTYPE_CODE_HANDLE); }
		bool is_code_label() const { return (m_type == PTYPE_CODE_LABEL); }
		bool is_c_function() const { return (m_type == PTYPE_C_FUNCTION); }
		bool is_rounding() const { return (m_type == PTYPE_ROUNDING); }
		bool is_string() const { return (m_type == PTYPE_STRING); }

		// other queries
		bool is_immediate_value(UINT64 value) const { return (m_type == PTYPE_IMMEDIATE && m_value == value); }

	private:
		// private constructor
		parameter(parameter_type type, parameter_value value) : m_type(type), m_value(value) { }

		// internals
		parameter_type      m_type;             // parameter type
		parameter_value     m_value;            // parameter value
	};

	// structure describing rules for opcode encoding
	struct opcode_info
	{
		struct parameter_info
		{
			UINT8               output;         // input or output?
			UINT8               size;           // size of the parameter
			UINT16              typemask;       // types allowed
		};

		opcode_t            opcode;             // the opcode itself
		const char *        mnemonic;           // mnemonic string
		UINT8               sizes;              // allowed sizes
		bool                condition;          // conditions allowed?
		UINT8               inflags;            // input flags
		UINT8               outflags;           // output flags
		UINT8               modflags;           // modified flags
		parameter_info      param[4];           // information about parameters
	};

	// a single UML instructon is encoded like this
	class instruction
	{
	public:
		// construction/destruction
		instruction();

		// getters
		opcode_t opcode() const { return m_opcode; }
		condition_t condition() const { return m_condition; }
		UINT8 flags() const { return m_flags; }
		UINT8 size() const { return m_size; }
		UINT8 numparams() const { return m_numparams; }
		const parameter &param(int index) const { assert(index < m_numparams); return m_param[index]; }

		// setters
		void set_flags(UINT8 flags) { m_flags = flags; }
		void set_mapvar(int paramnum, UINT32 value) { assert(paramnum < m_numparams); assert(m_param[paramnum].is_mapvar()); m_param[paramnum] = value; }

		// misc
		const char *disasm(std::string &str, drcuml_state *drcuml = NULL) const;
		UINT8 input_flags() const;
		UINT8 output_flags() const;
		UINT8 modified_flags() const;
		void simplify();

		// compile-time opcodes
		void handle(code_handle &hand) { configure(OP_HANDLE, 4, hand); }
		void hash(UINT32 mode, UINT32 pc) { configure(OP_HASH, 4, mode, pc); }
		void label(code_label lab) { configure(OP_LABEL, 4, lab); }
		void comment(const char *string) { configure(OP_COMMENT, 4, parameter::make_string(string)); }
		void mapvar(parameter mapvar, UINT32 value) { assert(mapvar.is_mapvar()); configure(OP_MAPVAR, 4, mapvar, value); }

		// control flow operations
		void nop() { configure(OP_NOP, 4); }
		void debug(UINT32 pc) { configure(OP_DEBUG, 4, pc); }
		void exit(parameter param) { configure(OP_EXIT, 4, param); }
		void exit(condition_t cond, parameter param) { configure(OP_EXIT, 4, param, cond); }
		void hashjmp(parameter mode, parameter pc, code_handle &handle) { configure(OP_HASHJMP, 4, mode, pc, handle); }
		void jmp(code_label label) { configure(OP_JMP, 4, label); }
		void jmp(condition_t cond, code_label label) { configure(OP_JMP, 4, label, cond); }
		void exh(code_handle &handle, parameter param) { configure(OP_EXH, 4, handle, param); }
		void exh(condition_t cond, code_handle &handle, parameter param) { configure(OP_EXH, 4, handle, param, cond); }
		void callh(code_handle &handle) { configure(OP_CALLH, 4, handle); }
		void callh(condition_t cond, code_handle &handle) { configure(OP_CALLH, 4, handle, cond); }
		void ret() { configure(OP_RET, 4); }
		void ret(condition_t cond) { configure(OP_RET, 4, cond); }
		void callc(c_function func, void *ptr) { configure(OP_CALLC, 4, parameter::make_cfunc(func), parameter::make_memory(ptr)); }
		void callc(condition_t cond, c_function func, void *ptr) { configure(OP_CALLC, 4, parameter::make_cfunc(func), parameter::make_memory(ptr), cond); }
		void recover(parameter dst, parameter mapvar) { assert(mapvar.is_mapvar()); configure(OP_RECOVER, 4, dst, mapvar); }

		// internal register operations
		void setfmod(parameter mode) { configure(OP_SETFMOD, 4, mode); }
		void getfmod(parameter dst) { configure(OP_GETFMOD, 4, dst); }
		void getexp(parameter dst) { configure(OP_GETEXP, 4, dst); }
		void getflgs(parameter dst, UINT32 flags) { configure(OP_GETFLGS, 4, dst, flags); }
		void save(drcuml_machine_state *dst) { configure(OP_SAVE, 4, parameter::make_memory(dst)); }
		void restore(drcuml_machine_state *src) { configure(OP_RESTORE, 4, parameter::make_memory(src)); }

		// 32-bit integer operations
		void load(parameter dst, const void *base, parameter index, operand_size size, memory_scale scale = SCALE_DEFAULT) { configure(OP_LOAD, 4, dst, parameter::make_memory(base), index, parameter(size, scale)); }
		void loads(parameter dst, const void *base, parameter index, operand_size size, memory_scale scale = SCALE_DEFAULT) { configure(OP_LOADS, 4, dst, parameter::make_memory(base), index, parameter(size, scale)); }
		void store(void *base, parameter index, parameter src1, operand_size size, memory_scale scale = SCALE_DEFAULT) { configure(OP_STORE, 4, parameter::make_memory(base), index, src1, parameter(size, scale)); }
		void read(parameter dst, parameter src1, operand_size size, memory_space space = SPACE_PROGRAM) { configure(OP_READ, 4, dst, src1, parameter(size, space)); }
		void readm(parameter dst, parameter src1, parameter mask, operand_size size, memory_space space = SPACE_PROGRAM) { configure(OP_READM, 4, dst, src1, mask, parameter(size, space)); }
		void write(parameter dst, parameter src1, operand_size size, memory_space space = SPACE_PROGRAM) { configure(OP_WRITE, 4, dst, src1, parameter(size, space)); }
		void writem(parameter dst, parameter src1, parameter mask, operand_size size, memory_space space = SPACE_PROGRAM) { configure(OP_WRITEM, 4, dst, src1, mask, parameter(size, space)); }
		void carry(parameter src, parameter bitnum) { configure(OP_CARRY, 4, src, bitnum); }
		void set(condition_t cond, parameter dst) { configure(OP_SET, 4, dst, cond); }
		void mov(parameter dst, parameter src1) { configure(OP_MOV, 4, dst, src1); }
		void mov(condition_t cond, parameter dst, parameter src1) { configure(OP_MOV, 4, dst, src1, cond); }
		void sext(parameter dst, parameter src1, operand_size size) { configure(OP_SEXT, 4, dst, src1, parameter::make_size(size)); }
		void roland(parameter dst, parameter src, parameter shift, parameter mask) { configure(OP_ROLAND, 4, dst, src, shift, mask); }
		void rolins(parameter dst, parameter src, parameter shift, parameter mask) { configure(OP_ROLINS, 4, dst, src, shift, mask); }
		void add(parameter dst, parameter src1, parameter src2) { configure(OP_ADD, 4, dst, src1, src2); }
		void addc(parameter dst, parameter src1, parameter src2) { configure(OP_ADDC, 4, dst, src1, src2); }
		void sub(parameter dst, parameter src1, parameter src2) { configure(OP_SUB, 4, dst, src1, src2); }
		void subb(parameter dst, parameter src1, parameter src2) { configure(OP_SUBB, 4, dst, src1, src2); }
		void cmp(parameter src1, parameter src2) { configure(OP_CMP, 4, src1, src2); }
		void mulu(parameter dst, parameter edst, parameter src1, parameter src2) { configure(OP_MULU, 4, dst, edst, src1, src2); }
		void muls(parameter dst, parameter edst, parameter src1, parameter src2) { configure(OP_MULS, 4, dst, edst, src1, src2); }
		void divu(parameter dst, parameter edst, parameter src1, parameter src2) { configure(OP_DIVU, 4, dst, edst, src1, src2); }
		void divs(parameter dst, parameter edst, parameter src1, parameter src2) { configure(OP_DIVS, 4, dst, edst, src1, src2); }
		void _and(parameter dst, parameter src1, parameter src2) { configure(OP_AND, 4, dst, src1, src2); }
		void test(parameter src1, parameter src2) { configure(OP_TEST, 4, src1, src2); }
		void _or(parameter dst, parameter src1, parameter src2) { configure(OP_OR, 4, dst, src1, src2); }
		void _xor(parameter dst, parameter src1, parameter src2) { configure(OP_XOR, 4, dst, src1, src2); }
		void lzcnt(parameter dst, parameter src) { configure(OP_LZCNT, 4, dst, src); }
		void bswap(parameter dst, parameter src) { configure(OP_BSWAP, 4, dst, src); }
		void shl(parameter dst, parameter src, parameter count) { configure(OP_SHL, 4, dst, src, count); }
		void shr(parameter dst, parameter src, parameter count) { configure(OP_SHR, 4, dst, src, count); }
		void sar(parameter dst, parameter src, parameter count) { configure(OP_SAR, 4, dst, src, count); }
		void rol(parameter dst, parameter src, parameter count) { configure(OP_ROL, 4, dst, src, count); }
		void rolc(parameter dst, parameter src, parameter count) { configure(OP_ROLC, 4, dst, src, count); }
		void ror(parameter dst, parameter src, parameter count) { configure(OP_ROR, 4, dst, src, count); }
		void rorc(parameter dst, parameter src, parameter count) { configure(OP_RORC, 4, dst, src, count); }

		// 64-bit integer operations
		void dload(parameter dst, const void *base, parameter index, operand_size size, memory_scale scale = SCALE_DEFAULT) { configure(OP_LOAD, 8, dst, parameter::make_memory(base), index, parameter(size, scale)); }
		void dloads(parameter dst, const void *base, parameter index, operand_size size, memory_scale scale = SCALE_DEFAULT) { configure(OP_LOADS, 8, dst, parameter::make_memory(base), index, parameter(size, scale)); }
		void dstore(void *base, parameter index, parameter src1, operand_size size, memory_scale scale = SCALE_DEFAULT) { configure(OP_STORE, 8, parameter::make_memory(base), index, src1, parameter(size, scale)); }
		void dread(parameter dst, parameter src1, operand_size size, memory_space space = SPACE_PROGRAM) { configure(OP_READ, 8, dst, src1, parameter(size, space)); }
		void dreadm(parameter dst, parameter src1, parameter mask, operand_size size, memory_space space = SPACE_PROGRAM) { configure(OP_READM, 8, dst, src1, mask, parameter(size, space)); }
		void dwrite(parameter dst, parameter src1, operand_size size, memory_space space = SPACE_PROGRAM) { configure(OP_WRITE, 8, dst, src1, parameter(size, space)); }
		void dwritem(parameter dst, parameter src1, parameter mask, operand_size size, memory_space space = SPACE_PROGRAM) { configure(OP_WRITEM, 8, dst, src1, mask, parameter(size, space)); }
		void dcarry(parameter src, parameter bitnum) { configure(OP_CARRY, 8, src, bitnum); }
		void dset(condition_t cond, parameter dst) { configure(OP_SET, 8, dst, cond); }
		void dmov(parameter dst, parameter src1) { configure(OP_MOV, 8, dst, src1); }
		void dmov(condition_t cond, parameter dst, parameter src1) { configure(OP_MOV, 8, dst, src1, cond); }
		void dsext(parameter dst, parameter src1, operand_size size) { configure(OP_SEXT, 8, dst, src1, parameter::make_size(size)); }
		void droland(parameter dst, parameter src, parameter shift, parameter mask) { configure(OP_ROLAND, 8, dst, src, shift, mask); }
		void drolins(parameter dst, parameter src, parameter shift, parameter mask) { configure(OP_ROLINS, 8, dst, src, shift, mask); }
		void dadd(parameter dst, parameter src1, parameter src2) { configure(OP_ADD, 8, dst, src1, src2); }
		void daddc(parameter dst, parameter src1, parameter src2) { configure(OP_ADDC, 8, dst, src1, src2); }
		void dsub(parameter dst, parameter src1, parameter src2) { configure(OP_SUB, 8, dst, src1, src2); }
		void dsubb(parameter dst, parameter src1, parameter src2) { configure(OP_SUBB, 8, dst, src1, src2); }
		void dcmp(parameter src1, parameter src2) { configure(OP_CMP, 8, src1, src2); }
		void dmulu(parameter dst, parameter edst, parameter src1, parameter src2) { configure(OP_MULU, 8, dst, edst, src1, src2); }
		void dmuls(parameter dst, parameter edst, parameter src1, parameter src2) { configure(OP_MULS, 8, dst, edst, src1, src2); }
		void ddivu(parameter dst, parameter edst, parameter src1, parameter src2) { configure(OP_DIVU, 8, dst, edst, src1, src2); }
		void ddivs(parameter dst, parameter edst, parameter src1, parameter src2) { configure(OP_DIVS, 8, dst, edst, src1, src2); }
		void dand(parameter dst, parameter src1, parameter src2) { configure(OP_AND, 8, dst, src1, src2); }
		void dtest(parameter src1, parameter src2) { configure(OP_TEST, 8, src1, src2); }
		void dor(parameter dst, parameter src1, parameter src2) { configure(OP_OR, 8, dst, src1, src2); }
		void dxor(parameter dst, parameter src1, parameter src2) { configure(OP_XOR, 8, dst, src1, src2); }
		void dlzcnt(parameter dst, parameter src) { configure(OP_LZCNT, 8, dst, src); }
		void dbswap(parameter dst, parameter src) { configure(OP_BSWAP, 8, dst, src); }
		void dshl(parameter dst, parameter src, parameter count) { configure(OP_SHL, 8, dst, src, count); }
		void dshr(parameter dst, parameter src, parameter count) { configure(OP_SHR, 8, dst, src, count); }
		void dsar(parameter dst, parameter src, parameter count) { configure(OP_SAR, 8, dst, src, count); }
		void drol(parameter dst, parameter src, parameter count) { configure(OP_ROL, 8, dst, src, count); }
		void drolc(parameter dst, parameter src, parameter count) { configure(OP_ROLC, 8, dst, src, count); }
		void dror(parameter dst, parameter src, parameter count) { configure(OP_ROR, 8, dst, src, count); }
		void drorc(parameter dst, parameter src, parameter count) { configure(OP_RORC, 8, dst, src, count); }

		// 32-bit floating point operations
		void fsload(parameter dst, const void *base, parameter index) { configure(OP_FLOAD, 4, dst, parameter::make_memory(base), index); }
		void fsstore(void *base, parameter index, parameter src1) { configure(OP_FSTORE, 4, parameter::make_memory(base), index, src1); }
		void fsread(parameter dst, parameter src1, memory_space space) { configure(OP_FREAD, 4, dst, src1, parameter(SIZE_SHORT, space)); }
		void fswrite(parameter dst, parameter src1, memory_space space) { configure(OP_FWRITE, 4, dst, src1, parameter(SIZE_SHORT, space)); }
		void fsmov(parameter dst, parameter src1) { configure(OP_FMOV, 4, dst, src1); }
		void fsmov(condition_t cond, parameter dst, parameter src1) { configure(OP_FMOV, 4, dst, src1, cond); }
		void fstoint(parameter dst, parameter src1, operand_size size, float_rounding_mode round) { configure(OP_FTOINT, 4, dst, src1, parameter::make_size(size), parameter::make_rounding(round)); }
		void fsfrint(parameter dst, parameter src1, operand_size size) { configure(OP_FFRINT, 4, dst, src1, parameter::make_size(size)); }
		void fsfrflt(parameter dst, parameter src1, operand_size size) { configure(OP_FFRFLT, 4, dst, src1, parameter::make_size(size)); }
		void fsadd(parameter dst, parameter src1, parameter src2) { configure(OP_FADD, 4, dst, src1, src2); }
		void fssub(parameter dst, parameter src1, parameter src2) { configure(OP_FSUB, 4, dst, src1, src2); }
		void fscmp(parameter src1, parameter src2) { configure(OP_FCMP, 4, src1, src2); }
		void fsmul(parameter dst, parameter src1, parameter src2) { configure(OP_FMUL, 4, dst, src1, src2); }
		void fsdiv(parameter dst, parameter src1, parameter src2) { configure(OP_FDIV, 4, dst, src1, src2); }
		void fsneg(parameter dst, parameter src1) { configure(OP_FNEG, 4, dst, src1); }
		void fsabs(parameter dst, parameter src1) { configure(OP_FABS, 4, dst, src1); }
		void fssqrt(parameter dst, parameter src1) { configure(OP_FSQRT, 4, dst, src1); }
		void fsrecip(parameter dst, parameter src1) { configure(OP_FRECIP, 4, dst, src1); }
		void fsrsqrt(parameter dst, parameter src1) { configure(OP_FRSQRT, 4, dst, src1); }

		// 64-bit floating point operations
		void fdload(parameter dst, const void *base, parameter index) { configure(OP_FLOAD, 8, dst, parameter::make_memory(base), index); }
		void fdstore(void *base, parameter index, parameter src1) { configure(OP_FSTORE, 8, parameter::make_memory(base), index, src1); }
		void fdread(parameter dst, parameter src1, memory_space space) { configure(OP_FREAD, 8, dst, src1, parameter(SIZE_DOUBLE, space)); }
		void fdwrite(parameter dst, parameter src1, memory_space space) { configure(OP_FWRITE, 8, dst, src1, parameter(SIZE_DOUBLE, space)); }
		void fdmov(parameter dst, parameter src1) { configure(OP_FMOV, 8, dst, src1); }
		void fdmov(condition_t cond, parameter dst, parameter src1) { configure(OP_FMOV, 8, dst, src1, cond); }
		void fdtoint(parameter dst, parameter src1, operand_size size, float_rounding_mode round) { configure(OP_FTOINT, 8, dst, src1, parameter::make_size(size), parameter::make_rounding(round)); }
		void fdfrint(parameter dst, parameter src1, operand_size size) { configure(OP_FFRINT, 8, dst, src1, parameter::make_size(size)); }
		void fdfrflt(parameter dst, parameter src1, operand_size size) { configure(OP_FFRFLT, 8, dst, src1, parameter::make_size(size)); }
		void fdrnds(parameter dst, parameter src1) { configure(OP_FRNDS, 8, dst, src1); }
		void fdadd(parameter dst, parameter src1, parameter src2) { configure(OP_FADD, 8, dst, src1, src2); }
		void fdsub(parameter dst, parameter src1, parameter src2) { configure(OP_FSUB, 8, dst, src1, src2); }
		void fdcmp(parameter src1, parameter src2) { configure(OP_FCMP, 8, src1, src2); }
		void fdmul(parameter dst, parameter src1, parameter src2) { configure(OP_FMUL, 8, dst, src1, src2); }
		void fddiv(parameter dst, parameter src1, parameter src2) { configure(OP_FDIV, 8, dst, src1, src2); }
		void fdneg(parameter dst, parameter src1) { configure(OP_FNEG, 8, dst, src1); }
		void fdabs(parameter dst, parameter src1) { configure(OP_FABS, 8, dst, src1); }
		void fdsqrt(parameter dst, parameter src1) { configure(OP_FSQRT, 8, dst, src1); }
		void fdrecip(parameter dst, parameter src1) { configure(OP_FRECIP, 8, dst, src1); }
		void fdrsqrt(parameter dst, parameter src1) { configure(OP_FRSQRT, 8, dst, src1); }

		// constants
		static const int MAX_PARAMS = 4;

	private:
		// internal configuration
		void configure(opcode_t op, UINT8 size, condition_t cond = COND_ALWAYS);
		void configure(opcode_t op, UINT8 size, parameter p0, condition_t cond = COND_ALWAYS);
		void configure(opcode_t op, UINT8 size, parameter p0, parameter p1, condition_t cond = COND_ALWAYS);
		void configure(opcode_t op, UINT8 size, parameter p0, parameter p1, parameter p2, condition_t cond = COND_ALWAYS);
		void configure(opcode_t op, UINT8 size, parameter p0, parameter p1, parameter p2, parameter p3, condition_t cond = COND_ALWAYS);

		// opcode validation and simplification
		void validate();
		void convert_to_mov_immediate(UINT64 immediate) { m_opcode = OP_MOV; m_numparams = 2; m_param[1] = immediate; }
		void convert_to_mov_param(int pnum) { m_opcode = OP_MOV; m_numparams = 2; m_param[1] = m_param[pnum]; }

		// internal state
		opcode_t            m_opcode;           // opcode
		condition_t         m_condition;        // condition
		UINT8               m_flags;            // flags
		UINT8               m_size;             // operation size
		UINT8               m_numparams;        // number of parameters
		parameter           m_param[MAX_PARAMS];// up to 4 parameters

		static const opcode_info s_opcode_info_table[OP_MAX];
	};

	// structure describing rules for parameter encoding
	struct parameter_info
	{
		UINT8               output;             // input or output?
		UINT8               size;               // size of the parameter
		UINT16              typemask;           // types allowed
	};

	// global inline functions to specify a register parameter by index
	inline parameter ireg(int n) { return parameter::make_ireg(REG_I0 + n); }
	inline parameter freg(int n) { return parameter::make_freg(REG_F0 + n); }
	inline parameter vreg(int n) { return parameter::make_vreg(REG_V0 + n); }
	inline parameter mapvar(int n) { return parameter::make_mapvar(MAPVAR_M0 + n); }

	// global inline functions to define memory parameters
	inline parameter mem(const void *ptr) { return parameter::make_memory(ptr); }

	// global register objects for direct access
	const parameter I0(parameter::make_ireg(REG_I0 + 0));
	const parameter I1(parameter::make_ireg(REG_I0 + 1));
	const parameter I2(parameter::make_ireg(REG_I0 + 2));
	const parameter I3(parameter::make_ireg(REG_I0 + 3));
	const parameter I4(parameter::make_ireg(REG_I0 + 4));
	const parameter I5(parameter::make_ireg(REG_I0 + 5));
	const parameter I6(parameter::make_ireg(REG_I0 + 6));
	const parameter I7(parameter::make_ireg(REG_I0 + 7));
	const parameter I8(parameter::make_ireg(REG_I0 + 8));
	const parameter I9(parameter::make_ireg(REG_I0 + 9));

	const parameter F0(parameter::make_freg(REG_F0 + 0));
	const parameter F1(parameter::make_freg(REG_F0 + 1));
	const parameter F2(parameter::make_freg(REG_F0 + 2));
	const parameter F3(parameter::make_freg(REG_F0 + 3));
	const parameter F4(parameter::make_freg(REG_F0 + 4));
	const parameter F5(parameter::make_freg(REG_F0 + 5));
	const parameter F6(parameter::make_freg(REG_F0 + 6));
	const parameter F7(parameter::make_freg(REG_F0 + 7));
	const parameter F8(parameter::make_freg(REG_F0 + 8));
	const parameter F9(parameter::make_freg(REG_F0 + 9));

	const parameter V0(parameter::make_vreg(REG_V0 + 0));
	const parameter V1(parameter::make_vreg(REG_V0 + 1));
	const parameter V2(parameter::make_vreg(REG_V0 + 2));
	const parameter V3(parameter::make_vreg(REG_V0 + 3));
	const parameter V4(parameter::make_vreg(REG_V0 + 4));
	const parameter V5(parameter::make_vreg(REG_V0 + 5));
	const parameter V6(parameter::make_vreg(REG_V0 + 6));
	const parameter V7(parameter::make_vreg(REG_V0 + 7));
	const parameter V8(parameter::make_vreg(REG_V0 + 8));
	const parameter V9(parameter::make_vreg(REG_V0 + 9));

	const parameter M0(parameter::make_mapvar(MAPVAR_M0 + 0));
	const parameter M1(parameter::make_mapvar(MAPVAR_M0 + 1));
	const parameter M2(parameter::make_mapvar(MAPVAR_M0 + 2));
	const parameter M3(parameter::make_mapvar(MAPVAR_M0 + 3));
	const parameter M4(parameter::make_mapvar(MAPVAR_M0 + 4));
	const parameter M5(parameter::make_mapvar(MAPVAR_M0 + 5));
	const parameter M6(parameter::make_mapvar(MAPVAR_M0 + 6));
	const parameter M7(parameter::make_mapvar(MAPVAR_M0 + 7));
	const parameter M8(parameter::make_mapvar(MAPVAR_M0 + 8));
	const parameter M9(parameter::make_mapvar(MAPVAR_M0 + 9));
}


#endif /* __UML_H__ */
