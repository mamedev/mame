// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*****************************************************************************
 *
 * template for CPU cores
 *
 *****************************************************************************/

#pragma once

#ifndef __XXX_H__
#define __XXX_H__

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
	xxx_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 7; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA) ? &m_data_config : NULL ); }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	UINT8   m_pc;   /* registers */
	UINT8   m_flags;  /* flags */
	address_space *m_program;
	address_space *m_data;
	int m_icount;

	void xxx_illegal();

};


extern const device_type XXX;


CPU_DISASSEMBLE( xxx );

#endif /* __XXX_H__ */
