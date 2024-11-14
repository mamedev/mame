// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    Portable Hitachi 6309 emulator

**********************************************************************/

#ifndef MAME_CPU_M6809_HD6309_H
#define MAME_CPU_M6809_HD6309_H

#pragma once

#include "m6809.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definitions
DECLARE_DEVICE_TYPE(HD6309, hd6309_device)
DECLARE_DEVICE_TYPE(HD6309E, hd6309e_device)

// ======================> hd6309_device

class hd6309_device : public m6809_base_device
{
public:
	// construction/destruction
	hd6309_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// delegating constructor
	hd6309_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, int divider);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual bool is_6809() override { return false; }
	virtual bool hd6309_native_mode() override { return m_md & 0x01; }

private:
	typedef m6809_base_device super;

	// addressing modes
	enum
	{
		ADDRESSING_MODE_REGISTER_E = 5,
		ADDRESSING_MODE_REGISTER_F = 6,
		ADDRESSING_MODE_REGISTER_W = 7,
		ADDRESSING_MODE_REGISTER_X = 8,
		ADDRESSING_MODE_REGISTER_Y = 9,
		ADDRESSING_MODE_REGISTER_U = 10,
		ADDRESSING_MODE_REGISTER_S = 11,
		ADDRESSING_MODE_REGISTER_CC = 12,
		ADDRESSING_MODE_REGISTER_DP = 13,
		ADDRESSING_MODE_REGISTER_PC = 14,
		ADDRESSING_MODE_REGISTER_V = 15,
		ADDRESSING_MODE_ZERO = 16
	};

	// interrupt vectors
	enum
	{
		VECTOR_ILLEGAL = 0xFFF0
	};

	// CPU registers
	PAIR16 m_v;
	uint8_t m_md;

	// other state
	uint8_t m_temp_im;

	// operand reading/writing
	uint8_t read_operand();
	uint8_t read_operand(int ordinal);
	void write_operand(uint8_t data);
	void write_operand(int ordinal, uint8_t data);

	// interrupt registers
	bool firq_saves_entire_state() { return m_md & 0x02; }
	uint16_t entire_state_registers() { return hd6309_native_mode() ? 0x3FF : 0xFF; }

	// bit tests
	uint8_t &bittest_register();
	bool bittest_source();
	bool bittest_dest();
	void bittest_set(bool result);

	// complex instructions
	void muld();
	bool divq();
	bool divd();

	// miscellaneous
	void set_e() { m_addressing_mode = ADDRESSING_MODE_REGISTER_E; }
	void set_f() { m_addressing_mode = ADDRESSING_MODE_REGISTER_F; }
	void set_w() { m_addressing_mode = ADDRESSING_MODE_REGISTER_W; }
	uint16_t read_exgtfr_register(uint8_t reg);
	void write_exgtfr_register(uint8_t reg, uint16_t value);
	bool tfr_read(uint8_t opcode, uint8_t arg, uint8_t &data);
	bool tfr_write(uint8_t opcode, uint8_t arg, uint8_t data);
	bool add8_sets_h() { return (m_opcode & 0xFE) != 0x30; }
	void register_register_op();

	void execute_one();
};

enum
{
	HD6309_PC = M6809_PC,
	HD6309_S = M6809_S,
	HD6309_CC = M6809_CC,
	HD6309_A = M6809_A,
	HD6309_B = M6809_B,
	HD6309_D = M6809_D,
	HD6309_U = M6809_U,
	HD6309_X = M6809_X,
	HD6309_Y = M6809_Y,
	HD6309_DP = M6809_DP,

	HD6309_E = 1000,
	HD6309_F,
	HD6309_W,
	HD6309_Q,
	HD6309_V,
	HD6309_MD,
	HD6309_ZERO_BYTE,
	HD6309_ZERO_WORD
};

// ======================> hd6309e_device

class hd6309e_device : public hd6309_device
{
public:
	// construction/destruction
	hd6309e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#define HD6309_IRQ_LINE  M6809_IRQ_LINE   /* 0 - IRQ line number */
#define HD6309_FIRQ_LINE M6809_FIRQ_LINE  /* 1 - FIRQ line number */

#endif // MAME_CPU_M6809_HD6309_H
