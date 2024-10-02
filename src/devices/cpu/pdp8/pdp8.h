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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	address_space_config m_program_config;

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

#endif /* __PDP8_H__ */
