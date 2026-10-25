// license:BSD-3-Clause
// copyright-holders:baco, Jeff Parsons
/*
  TMC1500 Family - Serial Register Processors

  The TMC1500 family (TMC1501, TMC1502, TMC1503) are 4-bit serial processors
  designed by Texas Instruments for calculators.

  Architecture Overview:
  - 13-bit Instruction Words
  - 2048 words of Program ROM (Internal)
  - SAM (Serial-Access Memory): 20 registers of 16 4-bit digits each.
    - Registers O (4): A, B, C, D
    - Registers X (8): Storage
    - Registers Y (8): Storage
  - PC: 11-bit Program Counter
  - Stack: 3-level Subroutine Stack
  - RAB: 3-bit Register Address Buffer (points to X or Y registers)
  - COND: Condition flag (latch)
  - R5: 8-bit status/temporary register

  Family Members:
  - TMC1501: Used in TI-57 Programmable
  - TMC1502: Used in MBA / TI-42 Financial
  - TMC1503: Used in TI-55 / TI-51 III Scientific
*/

#ifndef MAME_CPU_TMC1500_TMC1500_H
#define MAME_CPU_TMC1500_TMC1500_H

#pragma once

#include "emu.h"

class tmc1500_base_device : public cpu_device
{
public:
	tmc1500_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// Configuration callbacks
	auto read_k()  { return m_read_k.bind(); }
	auto write_o() { return m_write_o.bind(); }
	auto write_r() { return m_write_r.bind(); }

	uint8_t scan_idx() const { return m_scan_idx; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// cpu_device-level overrides
	virtual u32  execute_min_cycles() const noexcept override { return 1; }
	virtual u32  execute_max_cycles() const noexcept override { return 32; }
	virtual void execute_run() override;
	virtual space_config_vector memory_space_config() const override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// Common CPU State
	address_space_config m_program_config;
	address_space *m_program;
	int m_icount;

	devcb_read8    m_read_k;
	devcb_write16  m_write_o;
	devcb_write32  m_write_r;

	// Architectural State
	uint16_t m_pc;
	uint16_t m_pc_last;
	uint16_t m_opcode;
	uint16_t m_stack[3];
	uint8_t  m_rab;         // Register Address Buffer (0-7)
	uint8_t  m_base;        // 10 or 16
	bool     m_cond;        // Condition flag
	uint8_t  m_regR5;
	
	// Serial Access Memory (SAM)
	// 20 registers total (A,B,C,D + X0-X7 + Y0-Y7)
	uint8_t m_regsO[4][16];
	uint8_t m_regsX[8][16];
	uint8_t m_regsY[8][16];
	
	uint8_t m_scan_idx; // Current multiplexed digit index (0-15)
	uint32_t m_disp_activity;

	// Execution Helpers
	void execute_instruction();
	void opDISP();
	void opCALL(uint16_t addr);
	void opRET();
	void push(uint16_t addr);
	uint16_t pop();

	// Serial Arithmetic/Logic Helpers
	void update_r5(int lo, int hi, const uint8_t *data);
	void reg_add(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int lo, int hi, int base);
	void reg_sub(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int lo, int hi, int base);
	void reg_move(uint8_t *dst, const uint8_t *src, int lo, int hi, int base);
	void reg_shl(uint8_t *dst, const uint8_t *src, int lo, int hi);
	void reg_shr(uint8_t *dst, const uint8_t *src, int lo, int hi);
	void reg_xchg(uint8_t *r1, uint8_t *r2, int lo, int hi);
	void reg_store(uint8_t *dst, const uint8_t *src);
};

// Derived CPU Types
class tmc1501_cpu_device : public tmc1500_base_device
{
public:
	tmc1501_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class tmc1502_cpu_device : public tmc1500_base_device
{
public:
	tmc1502_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class tmc1503_cpu_device : public tmc1500_base_device
{
public:
	tmc1503_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(TMC1501, tmc1501_cpu_device)
DECLARE_DEVICE_TYPE(TMC1502, tmc1502_cpu_device)
DECLARE_DEVICE_TYPE(TMC1503, tmc1503_cpu_device)

#endif // MAME_CPU_TMC1500_TMC1500_H