// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    First-gen DEC PDP-8 CPU emulator

    Written by Ryan Holtz
*/

#pragma once

#ifndef __PDP8_H__
#define __PDP8_H__

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pdp8_device

// Used by core CPU interface
class pdp8_device : public cpu_device
{
public:
	// construction/destruction
	pdp8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override;
	virtual uint32_t execute_max_cycles() const override;
	virtual uint32_t execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override;
	virtual uint32_t disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	const address_space_config m_program_config;

	enum state
	{
		FETCH,
		DEFER,
		EXECUTE,
		WORD_COUNT,
		CURRENT_ADDR,
		BREAK
	};

	enum opcode
	{
		AND = 0,
		TAD,
		ISZ,
		DCA,
		JMS,
		JMP,
		IOT,
		OPR
	};
private:
	// CPU registers
	uint16_t m_pc;
	uint16_t m_ac;
	uint16_t m_mb;
	uint16_t m_ma;
	uint16_t m_sr;
	uint8_t m_l;
	uint8_t m_ir;
	bool m_halt;

	// other internal states
	int m_icount;

	// address spaces
	address_space *m_program;
};

// device type definition
DECLARE_DEVICE_TYPE(PDP8, pdp8_device)

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	PDP8_PC = 1,
	PDP8_AC,
	PDP8_MB,
	PDP8_MA,
	PDP8_SR,
	PDP8_L,
	PDP8_IR,
	PDP8_HALT
};

CPU_DISASSEMBLE( pdp8 );

#endif /* __PDP8_H__ */
