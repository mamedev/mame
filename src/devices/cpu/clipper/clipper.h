// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#pragma once

#ifndef __CLIPPER_H__
#define __CLIPPER_H__

// enumerate registers
enum
{
	CLIPPER_R0, CLIPPER_R1, CLIPPER_R2, CLIPPER_R3, CLIPPER_R4, CLIPPER_R5, CLIPPER_R6, CLIPPER_R7,
	CLIPPER_R8, CLIPPER_R9, CLIPPER_R10, CLIPPER_R11, CLIPPER_R12, CLIPPER_R13, CLIPPER_R14, CLIPPER_R15,

	CLIPPER_F0, CLIPPER_F1, CLIPPER_F2, CLIPPER_F3, CLIPPER_F4, CLIPPER_F5, CLIPPER_F6, CLIPPER_F7,
	CLIPPER_F8, CLIPPER_F9, CLIPPER_F10, CLIPPER_F11, CLIPPER_F12, CLIPPER_F13, CLIPPER_F14, CLIPPER_F15,

	CLIPPER_PSW, CLIPPER_SSW,
	CLIPPER_PC
};

enum
{
	ADDR_MODE_PC32 = 0x10,
	ADDR_MODE_ABS32 = 0x30,
	ADDR_MODE_REL32 = 0x60,
	ADDR_MODE_PC16 = 0x90,
	ADDR_MODE_REL12 = 0xa0,
	ADDR_MODE_ABS16 = 0xb0,
	ADDR_MODE_PCX = 0xd0,
	ADDR_MODE_RELX = 0xe0
};

enum
{
	FLAG_N = 0x1,
	FLAG_Z = 0x2,
	FLAG_V = 0x4,
	FLAG_C = 0x8
};

#define FLAGS_NONE 0x0
#define FLAGS_ZN   FLAG_Z | FLAG_N
#define FLAGS_CZN  FLAG_C | FLAG_Z | FLAG_N
#define FLAGS_CVZN FLAG_C | FLAG_V | FLAG_Z | FLAG_N

// branch conditions
enum
{
	BRANCH_T = 0x0,
	BRANCH_LT = 0x1,
	BRANCH_LE = 0x2,
	BRANCH_EQ = 0x3,
	BRANCH_GT = 0x4,
	BRANCH_GE = 0x5,
	BRANCH_NE = 0x6,
	BRANCH_LTU = 0x7,
	BRANCH_LEU = 0x8,
	BRANCH_GTU = 0x9,
	BRANCH_GEU = 0xa,
	BRANCH_V = 0xb,
	BRANCH_NV = 0xc,
	BRANCH_N = 0xd,
	BRANCH_NN = 0xe,
	BRANCH_FN = 0xf
};

class clipper_device : public cpu_device
{
public:
	clipper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; };
	virtual uint32_t execute_max_cycles() const override { return 1; }; // FIXME: don't know, especially macro instructions
	virtual uint32_t execute_input_lines() const override { return 16; }; // FIXME: number of input/interrupt lines
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
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	// core registers
	uint32_t m_pc;
	union {
		struct {
			uint32_t n : 1; // negative
			uint32_t z : 1; // zero
			uint32_t v : 1; // overflow
			uint32_t c : 1; // carry out or borrow in
			uint32_t fx : 1; // floating inexact
			uint32_t fu : 1; // floating underflow
			uint32_t fd : 1; // floating divide by zero
			uint32_t fv : 1; // floating overflow
			uint32_t fi : 1; // floating invalid operation
			uint32_t efx : 1; // enable floating inexact trap
			uint32_t efu : 1; // enable floating underflow trap
			uint32_t efd : 1; // enable floating divide by zero trap
			uint32_t efv : 1; // enable floating overflow trap
			uint32_t efi : 1; // enable floating invalid operation trap
			uint32_t eft : 1; // enable floating trap
			uint32_t fr : 2; // floating rounding mode
			uint32_t : 3;
			uint32_t dsp : 2; // c400 - delay slot pointer
			uint32_t big : 1; // c400 - big endian (hardware)
			uint32_t t : 1; // trace trap
			uint32_t cts : 4; // cpu trap status
			uint32_t mts : 4; // memory trap status
		} bits;
		uint32_t d;
	} m_psw;
	union {
		struct {
			uint32_t in : 4; // interrupt number
			uint32_t il : 4; // interrupt level
			uint32_t ei : 1; // enable interrupts
			uint32_t id : 8; // cpu rev # and type
			uint32_t : 5;
			uint32_t frd : 1; // floating registers dirty
			uint32_t tp : 1; // trace trap pending
			uint32_t ecm : 1; // enable corrected memory error
			uint32_t df : 1; // fpu disabled
			uint32_t m : 1; // mapped mode
			uint32_t ku : 1; // user protect key
			uint32_t uu : 1; // user data mode
			uint32_t k : 1; // protect key
			uint32_t u : 1; // user mode
			uint32_t p : 1; // previous mode
		} bits;
		uint32_t d;
	} m_ssw;
	int32_t m_r[2][16];
	double m_f[16];

private:
	address_space_config m_program_config;

	address_space *m_program;
	direct_read_data *m_direct;

	int m_icount;
	int m_interrupt_cycles;

	int m_immediate_irq;
	int m_immediate_vector;

	struct
	{
		// decoded operand information
		union {
			int32_t imm;
			uint32_t r2 : 4;
			uint16_t macro;
		} op;

		// total size of instruction in bytes
		uint32_t size;

		// computed effective address
		uint32_t address;
	} m_info;

	void clipper_device::decode_instruction(uint16_t insn);
	int clipper_device::execute_instruction(uint16_t insn);

	// condition code evaluation
	void clipper_device::evaluate_cc2(int32_t v0, int32_t v1, uint32_t flags);
	void clipper_device::evaluate_cc2f(double v0, double v1);

	bool clipper_device::evaluate_branch(uint32_t condition);

	uint32_t clipper_device::intrap(uint32_t vector, uint32_t pc);
};

extern const device_type CLIPPER;
#endif /* __CLIPPER_H__ */
