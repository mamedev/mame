// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    TMS9980A.
    See tms9980a.c and tms9900.c for documentation
*/

#ifndef MAME_CPU_TMS9900_TMS9980A_H
#define MAME_CPU_TMS9900_TMS9980A_H

#pragma once

#include "tms9900.h"

enum
{
	INT_9980A_RESET = 0,
	INT_9980A_LOAD = 2,
	INT_9980A_LEVEL1 = 3,
	INT_9980A_LEVEL2 = 4,
	INT_9980A_LEVEL3 = 5,
	INT_9980A_LEVEL4 = 6,
	INT_9980A_CLEAR= 7
};

class tms9980a_device : public tms99xx_device
{
public:
	tms9980a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	tms9980a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void        mem_read(void) override;
	void        mem_write(void) override;
	void        acquire_instruction(void) override;

	uint16_t    read_workspace_register_debug(int reg) override;
	void        write_workspace_register_debug(int reg, uint16_t data) override;

	uint32_t    execute_min_cycles() const noexcept override;
	uint32_t    execute_max_cycles() const noexcept override;
	void        execute_set_input(int irqline, int state) override;

	// The clock is internally divided by 4
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 4); }

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	address_space_config    m_program_config80;
	address_space_config    m_io_config80;
};

class tms9981_device : public tms9980a_device
{
public:
	tms9981_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// device type definition
DECLARE_DEVICE_TYPE(TMS9980A, tms9980a_device)
DECLARE_DEVICE_TYPE(TMS9981, tms9981_device)

#endif // MAME_CPU_TMS9900_TMS9980A_H
