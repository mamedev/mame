// license:BSD-3-Clause
// copyright-holders:Christian Brunschen
/**********************************************************************************************
 *
 *   es5510.h - Ensoniq ES5510 (ESP) driver
 *   by Christian Brunschen
 *
 **********************************************************************************************/

#pragma once

#ifndef __ES5510_H__
#define __ES5510_H__

#include "emu.h"

class es5510_device : public cpu_device {
public:
	es5510_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(host_r);
	DECLARE_WRITE8_MEMBER(host_w);

	INT16 ser_r(int offset);
	void ser_w(int offset, INT16 data);

	enum line_t {
		ES5510_HALT = 0
	};

	enum state_t {
		STATE_RUNNING = 0,
		STATE_HALTED = 1
	};

	struct alu_op_t {
		int operands;
		const char * const opcode;
	};

	enum op_src_dst_t {
		SRC_DST_REG =   1 << 0,
		SRC_DST_DELAY = 1 << 1,
		SRC_DST_BOTH =  (1 << 0) | (1 << 1)
	};

	struct op_select_t {
		const op_src_dst_t alu_src;
		const op_src_dst_t alu_dst;
		const op_src_dst_t mac_src;
		const op_src_dst_t mac_dst;
	};

	enum ram_control_access_t {
		RAM_CONTROL_DELAY = 0,
		RAM_CONTROL_TABLE_A,
		RAM_CONTROL_TABLE_B,
		RAM_CONTROL_IO
	};

	enum ram_cycle_t {
		RAM_CYCLE_READ = 0,
		RAM_CYCLE_WRITE = 1,
		RAM_CYCLE_DUMP_FIFO = 2
	};

	struct ram_control_t {
		ram_cycle_t cycle;
		ram_control_access_t access;
		const char * const description;
	};

	static const alu_op_t ALU_OPS[16];
	static const op_select_t OPERAND_SELECT[16];
	static const ram_control_t RAM_CONTROL[8];

	struct alu_t {
		UINT8 aReg;
		UINT8 bReg;
		op_src_dst_t src;
		op_src_dst_t dst;
		UINT8 op;
		INT32 aValue;
		INT32 bValue;
		INT32 result;
		bool update_ccr;
		bool write_result;
	};

	struct mulacc_t {
		UINT8 cReg;
		UINT8 dReg;
		op_src_dst_t src;
		op_src_dst_t dst;
		bool accumulate;
		INT32 cValue;
		INT32 dValue;
		INT64 product;
		INT64 result;
		bool write_result;
	};

	struct ram_t {
		INT32 address;     // up to 20 bits, left-justified within the right 24 bits of the 32-bit word
		bool io;           // I/O space, rather than delay line memory
		ram_cycle_t cycle; // cycle type
	};

	// direct access to the 'HALT' pin - not just through the
	void set_HALT(bool halt) { halt_asserted = halt; }
	bool get_HALT() { return halt_asserted; }

	void run_once();
	void list_program(void(p)(const char *, ...));

	// for testing purposes
	UINT64 &_instr(int pc) { return instr[pc % 160]; }
	INT16 &_dram(int addr) { return dram[addr & 0xfffff]; }

	// publicly visible for testing purposes
	INT32 read_reg(UINT8 reg);
	void write_reg(UINT8 reg, INT32 value);
	void write_to_dol(INT32 value);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override;
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override;
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual UINT32 execute_input_lines() const override;
	virtual void execute_run() override;
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void execute_set_input(int linenum, int state) override;

	INT32 alu_operation(UINT8 op, INT32 aValue, INT32 bValue, UINT8 &flags);
	void alu_operation_end();

private:
	int icount;
	bool halt_asserted;
	UINT8 pc;
	state_t state;
	INT32 gpr[0xc0];     // 24 bits, right justified
	INT16 ser0r;
	INT16 ser0l;
	INT16 ser1r;
	INT16 ser1l;
	INT16 ser2r;
	INT16 ser2l;
	INT16 ser3r;
	INT16 ser3l;
	INT64 machl;        // 48 bits, right justified and sign extended
	bool mac_overflow;  // whether reading the MAC register should return a saturated replacement value
	INT32 dil;
	INT32 memsiz;
	INT32 memmask;
	INT32 memincrement;
	INT8 memshift;
	INT32 dlength;
	INT32 abase;
	INT32 bbase;
	INT32 dbase;
	INT32 sigreg;
	int mulshift;
	INT8 ccr;           // really, 5 bits, left justified
	INT8 cmr;           // really, 6 bits, left justified
	INT32 dol[2];
	int dol_count;

	UINT64 instr[160];    // 48 bits, right justified
	INT16 dram[1<<20];   // there are up to 20 address bits (at least 16 expected), left justified within the 24 bits of a gpr or dadr; we preallocate all of it.

	// latch registers for host interaction
	INT32  dol_latch;     // 24 bits
	INT32  dil_latch;     // 24 bits
	UINT32 dadr_latch;    // 24 bits
	INT32  gpr_latch;     // 24 bits, holding up to 20 address bits, left justified
	UINT64 instr_latch;   // 48 bits, right justified
	UINT8  ram_sel;       // effectively a boolean
	UINT8  host_control;  //

	// currently executing instruction(s)
	alu_t alu;
	mulacc_t mulacc;
	ram_t ram, ram_p, ram_pp; // ram operations for cycles N, N-1 and N-2
};

extern const device_type ES5510;

#endif // __ES5510_H__
