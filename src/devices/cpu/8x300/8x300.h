// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * 8x300.h
 *
 *  Implementation of the Scientific Micro Systems SMS300 / Signetics 8X300 Microcontroller
 *  Created on: 18/12/2013
 */

#ifndef MAME_CPU_8X300_8X300_H
#define MAME_CPU_8X300_8X300_H

#pragma once

// Register enumeration
enum
{
	_8X300_PC = 1,
	_8X300_AR,
	_8X300_IR,
	_8X300_AUX,
	_8X300_R1,
	_8X300_R2,
	_8X300_R3,
	_8X300_R4,
	_8X300_R5,
	_8X300_R6,
	_8X300_IVL,
	_8X300_OVF,
	_8X300_R11,
	_8X300_UNUSED12,
	_8X300_UNUSED13,
	_8X300_UNUSED14,
	_8X300_UNUSED15,
	_8X300_UNUSED16,
	_8X300_IVR,
	_8X300_LIV,
	_8X300_RIV,
	_8X300_GENPC
};

class n8x300_cpu_device : public cpu_device
{
public:
	// construction/destruction
	n8x300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 1; }
	virtual uint32_t execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config m_program_config;
	address_space_config m_io_config;

	int m_icount;
	bool m_increment_pc;

	address_space *m_program;
	memory_access_cache<1, 0, ENDIANNESS_BIG> *m_cache;
	address_space *m_io;

	uint16_t m_PC;  // Program Counter
	uint16_t m_AR;  // Address Register
	uint16_t m_IR;  // Instruction Register
	uint8_t m_AUX;  // Auxiliary Register (second operand for AND, ADD, XOR)
	uint8_t m_R1;
	uint8_t m_R2;
	uint8_t m_R3;
	uint8_t m_R4;
	uint8_t m_R5;
	uint8_t m_R6;
	uint8_t m_R11;
	uint8_t m_IVL;  // Interface vector (I/O) left bank  (write-only)
	uint8_t m_IVR;  // Interface vector (I/O) right bank (write-only)
	uint8_t m_OVF;  // Overflow register (read-only)
	uint16_t m_genPC;

	uint8_t m_left_IV;  // IV bank contents, these are latched when IVL or IVR are set
	uint8_t m_right_IV;

private:
	inline bool is_rot(uint16_t opcode)
	{
		if((opcode & 0x1000) || (opcode & 0x0010))
			return false;
		else
			return true;
	}
	inline bool is_src_reg(uint16_t opcode)
	{
		if((opcode & 0x1000))
			return false;
		else
			return true;
	}
	inline bool is_dst_reg(uint16_t opcode)
	{
		if((opcode & 0x0010))
			return false;
		else
			return true;
	}
	inline uint8_t rotate(uint8_t s, uint8_t n)  // right rotate
	{
		return ((s & ((uint8_t)0xff << n)) >> n) | ((s & ((uint8_t)0xff >> (8-n))) << (8-n));
	}
	void set_reg(uint8_t reg,uint8_t val);
	uint8_t get_reg(uint8_t reg);
};

DECLARE_DEVICE_TYPE(N8X300, n8x300_cpu_device)

#endif // MAME_CPU_8X300_8X300_H
