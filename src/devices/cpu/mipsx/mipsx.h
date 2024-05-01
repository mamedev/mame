// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 MIPSXtangent (A4) core
 MIPSX == Argonaut RISC Core

\*********************************/

#ifndef MAME_CPU_MIPSX_MIPSX_H
#define MAME_CPU_MIPSX_MIPSX_H

#pragma once

enum
{
	MIPSX_PC = STATE_GENPC
};

class mipsx_cpu_device : public cpu_device
{
public:
	// construction/destruction
	mipsx_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 5; }
	virtual uint32_t execute_input_lines() const noexcept override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	uint32_t m_pc;
	uint32_t m_debugger_temp;

	address_space *m_program;
	int m_icount;
};

DECLARE_DEVICE_TYPE(MIPSX, mipsx_cpu_device)

#endif // MAME_CPU_MIPSX_MIPSX_H
