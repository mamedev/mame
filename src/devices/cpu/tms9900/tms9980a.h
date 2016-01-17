// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    TMS9980A.
    See tms9980a.c and tms9900.c for documentation
*/

#ifndef __TMS9980A_H__
#define __TMS9980A_H__

#include "emu.h"
#include "debugger.h"
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
	tms9980a_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	void        mem_read(void) override;
	void        mem_write(void) override;
	void        acquire_instruction(void) override;

	void        resolve_lines() override;

	UINT16      read_workspace_register_debug(int reg) override;
	void        write_workspace_register_debug(int reg, UINT16 data) override;

	UINT32      execute_min_cycles() const override;
	UINT32      execute_max_cycles() const override;
	UINT32      execute_input_lines() const override;
	void        execute_set_input(int irqline, int state) override;

	UINT32      disasm_min_opcode_bytes() const override;
	UINT32      disasm_max_opcode_bytes() const override;
	offs_t      disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	address_space_config    m_program_config80;
	address_space_config    m_io_config80;
};

// device type definition
extern const device_type TMS9980A;

#endif /* __TMS9980A_H__ */
