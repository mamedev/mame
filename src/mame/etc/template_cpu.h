// license:BSD-3-Clause
// copyright-holders:<author_name>
/*****************************************************************************
 *
 * template for CPU cores
 *
 *****************************************************************************/

#ifndef MAME_CPU_XXX_H
#define MAME_CPU_XXX_H

#pragma once

enum
{
	#if UNUSED
	XXX_R0=1, XXX_R1, XXX_R2, XXX_R3,
	XXX_R4, XXX_R5, XXX_R6, XXX_R7
	#endif
};


class xxx_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	xxx_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 7; }
	virtual uint32_t execute_input_lines() const noexcept override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(int spacenum) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : (spacenum == AS_DATA) ? &m_data_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t opcode_alignment() const override { return 4; }
	virtual offs_t disassemble(char *buffer, offs_t pc, const data_buffer &opcodes, const data_buffer &params, uint32_t options) override;

private:
	address_space_config m_program_config;

	uint8_t   m_pc;   /* registers */
	uint8_t   m_flags;  /* flags */
	address_space *m_program;
	address_space *m_data;
	int m_icount;

	void xxx_illegal();

};


DECLARE_DEVICE_TYPE(XXX, xxx_cpu_device)


CPU_DISASSEMBLE( xxx );

#endif // MAME_CPU_XXX_H
