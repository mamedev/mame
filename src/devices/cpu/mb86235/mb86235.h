// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi
/*****************************************************************************
 *
 * template for CPU cores
 *
 *****************************************************************************/

#pragma once

#ifndef __MB86235_H__
#define __MB86235_H__

#if 0
enum
{
	MB86235_R0=1, MB86235_R1, MB86235_R2, MB86235_R3,
	MB86235_R4, MB86235_R5, MB86235_R6, MB86235_R7
};
#endif


class mb86235_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	mb86235_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 7; }
	virtual uint32_t execute_input_lines() const override { return 0; }
	virtual void execute_run() override;
	//virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 8; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 8; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

private:
	address_space_config m_program_config;

	uint8_t   m_pc;   /* registers */
	uint8_t   m_flags;  /* flags */
	address_space *m_program;
	int m_icount;

	void mb86235_illegal();

};


extern const device_type MB86235;


CPU_DISASSEMBLE( mb86235 );

#endif /* __MB86235_H__ */
