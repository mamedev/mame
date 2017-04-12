// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#pragma once

#ifndef __CLIPPER_H__
#define __CLIPPER_H__

#include <limits.h>

// convenience macros for frequently used instruction fields
#define R1 (m_info.r1)
#define R2 (m_info.r2)

// convenience macros for dealing with the psw
#define PSW(mask) (m_psw & PSW_##mask)
#define SSW(mask) (m_ssw & SSW_##mask)

// macros for setting psw condition codes
#define FLAGS(C,V,Z,N) \
	m_psw = (m_psw & ~(PSW_C | PSW_V | PSW_Z | PSW_N)) | (((C) << 3) | ((V) << 2) | ((Z) << 1) | ((N) << 0));
#define FLAGS_CV(C,V) \
	m_psw = (m_psw & ~(PSW_C | PSW_V)) | (((C) << 3) | ((V) << 2));
#define FLAGS_ZN(Z,N) \
	m_psw = (m_psw & ~(PSW_Z | PSW_N)) | (((Z) << 1) | ((N) << 0));

// over/underflow for addition/subtraction from here: http://stackoverflow.com/questions/199333/how-to-detect-integer-overflow-in-c-c
#define OF_ADD(a, b) ((b > 0) && (a > INT_MAX - b))
#define UF_ADD(a, b) ((b < 0) && (a < INT_MIN - b))
#define OF_SUB(a, b) ((b < 0) && (a > INT_MAX + b))
#define UF_SUB(a, b) ((b > 0) && (a < INT_MIN + b))

// CLIPPER logic for carry and overflow flags
#define C_ADD(a, b) ((u32)a + (u32)b < (u32)a)
#define V_ADD(a, b) (OF_ADD((s32)a, (s32)b) || UF_ADD((s32)a, (s32)b))
#define C_SUB(a, b) ((u32)a < (u32)b)
#define V_SUB(a, b) (OF_SUB((s32)a, (s32)b) || UF_SUB((s32)a, (s32)b))

class clipper_device : public cpu_device
{
	enum registers
	{
		CLIPPER_R0, CLIPPER_R1, CLIPPER_R2, CLIPPER_R3, CLIPPER_R4, CLIPPER_R5, CLIPPER_R6, CLIPPER_R7,
		CLIPPER_R8, CLIPPER_R9, CLIPPER_R10, CLIPPER_R11, CLIPPER_R12, CLIPPER_R13, CLIPPER_R14, CLIPPER_R15,

		CLIPPER_F0, CLIPPER_F1, CLIPPER_F2, CLIPPER_F3, CLIPPER_F4, CLIPPER_F5, CLIPPER_F6, CLIPPER_F7,
		CLIPPER_F8, CLIPPER_F9, CLIPPER_F10, CLIPPER_F11, CLIPPER_F12, CLIPPER_F13, CLIPPER_F14, CLIPPER_F15,

		CLIPPER_PSW,
		CLIPPER_SSW,
		CLIPPER_PC,
	};

	enum addressing_modes
	{
		ADDR_MODE_PC32  = 0x10,
		ADDR_MODE_ABS32 = 0x30,
		ADDR_MODE_REL32 = 0x60,
		ADDR_MODE_PC16  = 0x90,
		ADDR_MODE_REL12 = 0xa0,
		ADDR_MODE_ABS16 = 0xb0,
		ADDR_MODE_PCX   = 0xd0,
		ADDR_MODE_RELX  = 0xe0,
	};

	// branch conditions
	enum branch_conditions
	{
		BRANCH_T   = 0x0,
		BRANCH_LT  = 0x1,
		BRANCH_LE  = 0x2,
		BRANCH_EQ  = 0x3,
		BRANCH_GT  = 0x4,
		BRANCH_GE  = 0x5,
		BRANCH_NE  = 0x6,
		BRANCH_LTU = 0x7,
		BRANCH_LEU = 0x8,
		BRANCH_GTU = 0x9,
		BRANCH_GEU = 0xa,
		BRANCH_V   = 0xb,
		BRANCH_NV  = 0xc,
		BRANCH_N   = 0xd,
		BRANCH_NN  = 0xe,
		BRANCH_FN  = 0xf,
	};

	enum psw
	{
		PSW_N   = 0x00000001, // negative
		PSW_Z   = 0x00000002, // zero
		PSW_V   = 0x00000004, // overflow
		PSW_C   = 0x00000008, // carry out or borrow in
		PSW_FX  = 0x00000010, // floating inexact
		PSW_FU  = 0x00000020, // floating underflow
		PSW_FD  = 0x00000040, // floating divide by zero
		PSW_FV  = 0x00000080, // floating overflow
		PSW_FI  = 0x00000100, // floating invalid operation
		PSW_EFX = 0x00000200, // enable floating inexact trap
		PSW_EFU = 0x00000400, // enable floating underflow trap
		PSW_EFD = 0x00000800, // enable floating divide by zero trap
		PSW_EFV = 0x00001000, // enable floating overflow trap
		PSW_EFI = 0x00002000, // enable floating invalid operation trap
		PSW_EFT = 0x00004000, // enable floating trap
		PSW_FR  = 0x00018000, // floating rounding mode (2 bits)
							  // unused (3 bits)
		PSW_DSP = 0x00300000, // c400 - delay slot pointer (2 bits)
		PSW_BIG = 0x00400000, // c400 - big endian (hardware)
		PSW_T   = 0x00800000, // trace trap
		PSW_CTS = 0x0f000000, // cpu trap status (4 bits)
		PSW_MTS = 0xf0000000, // memory trap status (4 bits)
	};

	enum clipper_ssw
	{
		SSW_IN  = 0x0000000f, // interrupt number (4 bits)
		SSW_IL  = 0x000000f0, // interrupt level (4 bits)
		SSW_EI  = 0x00000100, // enable interrupts
		SSW_ID  = 0x0001fe00, // cpu rev # and type (8 bits)
						  // unused (5 bits)
		SSW_FRD = 0x00400000, // floating registers dirty
		SSW_TP  = 0x00800000, // trace trap pending
		SSW_ECM = 0x01000000, // enabled corrected memory error
		SSW_DF  = 0x02000000, // fpu disabled
		SSW_M   = 0x04000000, // mapped mode
		SSW_KU  = 0x08000000, // user protect key
		SSW_UU  = 0x10000000, // user data mode
		SSW_K   = 0x20000000, // protect key
		SSW_U   = 0x40000000, // user mode
		SSW_P   = 0x80000000, // previous mode
	};

	enum exception_vectors
	{
		// data memory trap group
		EXCEPTION_D_CORRECTED_MEMORY_ERROR     = 0x108,
		EXCEPTION_D_UNCORRECTABLE_MEMORY_ERROR = 0x110,
		EXCEPTION_D_ALIGNMENT_FAULT            = 0x120,
		EXCEPTION_D_PAGE_FAULT                 = 0x128,
		EXCEPTION_D_READ_PROTECT_FAULT         = 0x130,
		EXCEPTION_D_WRITE_PROTECT_FAULT        = 0x138,

		// floating-point arithmetic trap group
		EXCEPTION_FLOATING_INEXACT             = 0x180,
		EXCEPTION_FLOATING_UNDERFLOW           = 0x188,
		EXCEPTION_FLOATING_DIVIDE_BY_ZERO      = 0x190,
		EXCEPTION_FLOATING_OVERFLOW            = 0x1a0,
		EXCEPTION_FLOATING_INVALID_OPERATION   = 0x1c0,

		// integer arithmetic trap group
		EXCEPTION_INTEGER_DIVIDE_BY_ZERO       = 0x208,

		// instruction memory trap group
		EXCEPTION_I_CORRECTED_MEMORY_ERROR     = 0x288,
		EXCEPTION_I_UNCORRECTABLE_MEMORY_ERROR = 0x290,
		EXCEPTION_I_ALIGNMENT_FAULT            = 0x2a0,
		EXCEPTION_I_PAGE_FAULT                 = 0x2a8,
		EXCEPTION_I_EXECUTE_PROTECT_FAULT      = 0x2b0,

		// illegal operation trap group
		EXCEPTION_ILLEGAL_OPERATION            = 0x300,
		EXCEPTION_PRIVILEGED_INSTRUCTION       = 0x308,

		// diagnostic trap group
		EXCEPTION_TRACE                        = 0x380,

		// supervisor calls (0x400-0x7f8)
		EXCEPTION_SUPERVISOR_CALL_BASE         = 0x400,

		// prioritized interrupts (0x800-0xff8)
		EXCEPTION_INTERRUPT_BASE               = 0x800,
	};

	// trap source values are shifted into the correct field in the psw
	enum cpu_trap_sources
	{
		CTS_NO_CPU_TRAP            = 0 << 24,
		CTS_DIVIDE_BY_ZERO         = 2 << 24,
		CTS_ILLEGAL_OPERATION      = 4 << 24,
		CTS_PRIVILEGED_INSTRUCTION = 5 << 24,
		CTS_TRACE_TRAP             = 7 << 24,
	};

	enum memory_trap_sources
	{
		MTS_NO_MEMORY_TRAP                = 0 << 28,
		MTS_CORRECTED_MEMORY_ERROR        = 1 << 28,
		MTS_UNCORRECTABLE_MEMORY_ERROR    = 2 << 28,
		MTS_ALIGNMENT_FAULT               = 4 << 28,
		MTS_PAGE_FAULT                    = 5 << 28,
		MTS_READ_OR_EXECUTE_PROTECT_FAULT = 6 << 28,
		MTS_WRITE_PROTECT_FAULT           = 7 << 28,
	};

public:
	clipper_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source);

	DECLARE_READ_LINE_MEMBER(ssw) { return m_ssw; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const override { return 1; };
	virtual u32 execute_max_cycles() const override { return 1; }; // FIXME: don't know, especially macro instructions
	virtual u32 execute_input_lines() const override { return 2; }; // number of input/interrupt lines (irq/nmi)
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
#if 0
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
#endif
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 2; } // smallest instruction
	virtual uint32_t disasm_max_opcode_bytes() const override { return 8; } // largest instruction
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const u8 *oprom, const u8 *opram, u32 options) override;

	// core registers
	u32 m_pc;
	u32 m_psw;
	u32 m_ssw;

	// integer registers
	s32 *m_r;     // active registers
	s32 m_ru[16]; // user registers
	s32 m_rs[16]; // supervisor registers

	// floating registers
	double m_f[16];

private:
	address_space_config m_insn_config;
	address_space_config m_data_config;

	address_space *m_insn;
	address_space *m_data;

	int m_icount;

	int m_irq;
	int m_nmi;

	// decoded instruction information
	struct
	{
		u8 opcode, subopcode;
		u8 r1, r2;

		s32 imm;
		u16 macro;

		// total size of instruction in bytes
		u32 size;

		// computed effective address
		u32 address;
	} m_info;

	void decode_instruction(u16 insn);
	int execute_instruction();
	bool evaluate_branch();

	uint32_t intrap(u32 vector, u32 pc, u32 cts = CTS_NO_CPU_TRAP, u32 mts = MTS_NO_MEMORY_TRAP);
};

class clipper_c100_device : public clipper_device
{
public:
	clipper_c100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class clipper_c300_device : public clipper_device
{
public:
	clipper_c300_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class clipper_c400_device : public clipper_device
{
public:
	clipper_c400_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

extern const device_type CLIPPER_C100;
extern const device_type CLIPPER_C300;
extern const device_type CLIPPER_C400;

extern CPU_DISASSEMBLE(clipper);
#endif /* __CLIPPER_H__ */
