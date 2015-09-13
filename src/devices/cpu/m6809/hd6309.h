// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    hd6309.h

    Portable Hitachi 6309 emulator

**********************************************************************/

#pragma once

#ifndef __HD6309_H__
#define __HD6309_H__

#include "m6809.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
extern const device_type HD6309;

// ======================> hd6309_device

class hd6309_device : public m6809_base_device
{
public:
	// construction/destruction
	hd6309_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_pre_save();
	virtual void device_post_load();

	// device_execute_interface overrides
	virtual void execute_run();

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

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
	PAIR16  m_w;
	PAIR16  m_v;
	UINT8   m_md;

	// other state
	UINT8   m_temp_im;

	// operand reading/writing
	UINT8 read_operand();
	UINT8 read_operand(int ordinal);
	void write_operand(UINT8 data);
	void write_operand(int ordinal, UINT8 data);

	// interrupt registers
	bool firq_saves_entire_state()      { return m_md & 0x02; }
	UINT16 entire_state_registers()     { return hd6309_native_mode() ? 0x3FF : 0xFF; }

	// bit tests
	UINT8 &bittest_register();
	bool bittest_source();
	bool bittest_dest();
	void bittest_set(bool result);

	// complex instructions
	void muld();
	bool divq();
	bool divd();

	// the Q register
	UINT32 get_q();
	void put_q(UINT32 value);

	// miscellaneous
	void set_e()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_E; }
	void set_f()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_F; }
	void set_w()                                    { m_addressing_mode = ADDRESSING_MODE_REGISTER_W; }
	exgtfr_register read_exgtfr_register(UINT8 reg);
	void write_exgtfr_register(UINT8 reg, exgtfr_register value);
	bool tfr_read(UINT8 opcode, UINT8 arg, UINT8 &data);
	bool tfr_write(UINT8 opcode, UINT8 arg, UINT8 data);
	bool add8_sets_h()                              { return (m_opcode & 0xFE) != 0x30; }
	void register_register_op();
	bool hd6309_native_mode()           { return m_md & 0x01; }

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
	HD6309_V,
	HD6309_MD,
	HD6309_ZERO_BYTE,
	HD6309_ZERO_WORD
};

#define HD6309_IRQ_LINE  M6809_IRQ_LINE   /* 0 - IRQ line number */
#define HD6309_FIRQ_LINE M6809_FIRQ_LINE  /* 1 - FIRQ line number */

#endif // __HD6309_H__
