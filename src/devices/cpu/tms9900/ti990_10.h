// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    TI 990 CPU board
    See ti990_10.c for documentation
*/

#ifndef MAME_CPU_TMS9900_TI990_10_H
#define MAME_CPU_TMS9900_TI990_10_H

#pragma once


#include "tms99com.h"

class ti990_10_device : public cpu_device
{
public:
	ti990_10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~ti990_10_device();

protected:
	// device-level overrides
	void        device_start() override;
	void        device_stop() override;
	void        device_reset() override;

	// device_execute_interface overrides
	uint32_t    execute_min_cycles() const noexcept override;
	uint32_t    execute_max_cycles() const noexcept override;
	void        execute_set_input(int irqline, int state) override;
	void        execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual space_config_vector memory_space_config() const override;

	const address_space_config      m_program_config;
	const address_space_config      m_io_config;
	address_space*                  m_prgspace;
	address_space*                  m_cru;

	// Cycle counter
	int     m_icount;

	// Hardware registers
	uint16_t  WP;     // Workspace pointer
	uint16_t  PC;     // Program counter
	uint16_t  ST;     // Status register

private:
	uint16_t  m_state_any;
};

// device type definition
DECLARE_DEVICE_TYPE(TI990_10, ti990_10_device)

#endif // MAME_CPU_TMS9900_TI990_10_H
