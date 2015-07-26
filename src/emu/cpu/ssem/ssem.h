// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    Manchester Small-Scale Experimental Machine (SSEM) emulator

    Written by MooglyGuy
*/

#pragma once

#ifndef __SSEM_H__
#define __SSEM_H__

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ssem_device

// Used by core CPU interface
class ssem_device : public cpu_device
{
public:
	// construction/destruction
	ssem_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_stop();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str);

	// address spaces
	const address_space_config m_program_config;

	// memory access
	inline UINT32 program_read32(UINT32 addr);
	inline void program_write32(UINT32 addr, UINT32 data);

	// CPU registers
	UINT32 m_pc;
	UINT32 m_shifted_pc;
	UINT32 m_a;
	UINT32 m_halt;

	// other internal states
	int m_icount;

	// address spaces
	address_space *m_program;
};

// device type definition
extern const device_type SSEMCPU;

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	SSEM_PC = 1,
	SSEM_A,
	SSEM_HALT
};

CPU_DISASSEMBLE( ssem );

#endif /* __SSEM_H__ */
