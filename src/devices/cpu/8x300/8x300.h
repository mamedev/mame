// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * 8x300.h
 *
 *  Implementation of the Scientific Micro Systems SMS300 / Signetics 8X300 Microcontroller
 *  Created on: 18/12/2013
 */

#ifndef _8X300_H_
#define _8X300_H_

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
	n8x300_cpu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 1; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override
	{
		switch (spacenum)
		{
			case AS_PROGRAM: return &m_program_config;
			case AS_IO:      return &m_io_config;
			default:         return nullptr;
		}
	}

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	address_space_config m_program_config;
	address_space_config m_io_config;

	int m_icount;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;

	UINT16 m_PC;  // Program Counter
	UINT16 m_AR;  // Address Register
	UINT16 m_IR;  // Instruction Register
	UINT8 m_AUX;  // Auxiliary Register (second operand for AND, ADD, XOR)
	UINT8 m_R1;
	UINT8 m_R2;
	UINT8 m_R3;
	UINT8 m_R4;
	UINT8 m_R5;
	UINT8 m_R6;
	UINT8 m_R11;
	UINT8 m_IVL;  // Interface vector (I/O) left bank  (write-only)
	UINT8 m_IVR;  // Interface vector (I/O) right bank (write-only)
	UINT8 m_OVF;  // Overflow register (read-only)
	UINT16 m_genPC;

	UINT8 m_left_IV;  // IV bank contents, these are latched when IVL or IVR are set
	UINT8 m_right_IV;

private:
	inline bool is_rot(UINT16 opcode)
	{
		if((opcode & 0x1000) || (opcode & 0x0010))
			return false;
		else
			return true;
	}
	inline bool is_src_reg(UINT16 opcode)
	{
		if((opcode & 0x1000))
			return false;
		else
			return true;
	}
	inline bool is_dst_reg(UINT16 opcode)
	{
		if((opcode & 0x0010))
			return false;
		else
			return true;
	}
	inline UINT8 rotate(UINT8 s, UINT8 n)  // right rotate
	{
		return ((s & ((UINT8)0xff << n)) >> n) | ((s & ((UINT8)0xff >> (8-n))) << (8-n));
	}
	void set_reg(UINT8 reg,UINT8 val);
	UINT8 get_reg(UINT8 reg);
};

extern const device_type N8X300;

#endif /* 8X300_H_ */
