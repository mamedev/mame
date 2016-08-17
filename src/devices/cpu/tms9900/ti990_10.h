// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/*
    TI 990 CPU board
    See ti990_10.c for documentation
*/

#ifndef __TI990_10_H__
#define __TI990_10_H__

#include "emu.h"
#include "debugger.h"
#include "tms99com.h"

class ti990_10_device : public cpu_device
{
public:
	ti990_10_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~ti990_10_device();

protected:
	// device-level overrides
	void        device_start() override;
	void        device_stop() override;
	void        device_reset() override;

	// device_execute_interface overrides
	UINT32      execute_min_cycles() const override;
	UINT32      execute_max_cycles() const override;
	UINT32      execute_input_lines() const override;
	void        execute_set_input(int irqline, int state) override;
	void        execute_run() override;

	// device_disasm_interface overrides
	UINT32      disasm_min_opcode_bytes() const override;
	UINT32      disasm_max_opcode_bytes() const override;
	offs_t      disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	const address_space_config* memory_space_config(address_spacenum spacenum) const override;

	const address_space_config      m_program_config;
	const address_space_config      m_io_config;
	address_space*                  m_prgspace;
	address_space*                  m_cru;

	// Cycle counter
	int     m_icount;

	// Hardware registers
	UINT16  WP;     // Workspace pointer
	UINT16  PC;     // Program counter
	UINT16  ST;     // Status register

private:
	UINT16  m_state_any;
};

// device type definition
extern const device_type TI990_10;

#endif /* __TI990_10_H__ */
