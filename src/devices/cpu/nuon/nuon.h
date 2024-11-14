// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*
    VM Labs Aries 3 "NUON Multi-Media Architecture" simulator

    - Changelist -
      10 Mar. 2018
      - Initial skeleton version.
*/

#ifndef MAME_CPU_NUON_NUON_H
#define MAME_CPU_NUON_NUON_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nuon_device : public cpu_device
{
public:
	nuon_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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

	// utility functions
	void unimplemented_opcode(uint32_t op);

	struct mpe_t
	{
		// address spaces
		const address_space_config m_program_config;
		address_space *m_program;

		// registers
		uint32_t m_pc;
	};

	const address_space_config m_program_configs[4];
	address_space *m_program[4];

	// registers
	uint32_t m_pc[4];

	// other internal states
	int m_icount;
};

DECLARE_DEVICE_TYPE(NUON,   nuon_device)

#endif /* MAME_CPU_NUON_NUON_H */
