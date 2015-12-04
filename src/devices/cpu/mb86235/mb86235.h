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
	mb86235_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 7; }
	virtual UINT32 execute_input_lines() const { return 0; }
	virtual void execute_run();
	//virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 8; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;

	UINT8   m_pc;   /* registers */
	UINT8   m_flags;  /* flags */
	address_space *m_program;
	int m_icount;

	void mb86235_illegal();

};


extern const device_type MB86235;


CPU_DISASSEMBLE( mb86235 );

#endif /* __MB86235_H__ */
