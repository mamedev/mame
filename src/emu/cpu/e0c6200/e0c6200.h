// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6200 CPU core and E0C62 MCU family

*/

#ifndef _E06200_H_
#define _E06200_H_

#include "emu.h"


class e0c6200_cpu_device : public cpu_device
{
public:
	// construction/destruction
	e0c6200_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_LITTLE, 16, prgwidth, 0, program)
		, m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data)
		, m_prgwidth(prgwidth)
		, m_datawidth(datawidth)
	{ }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 5; }
	virtual UINT32 execute_max_cycles() const { return 12; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return(spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_DATA) ? &m_data_config : NULL); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	void state_string_export(const device_state_entry &entry, std::string &str);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_prgwidth;
	int m_datawidth;
	int m_prgmask;
	int m_datamask;
	
	int m_icount;
};


class e0c6s46_device : public e0c6200_cpu_device
{
public:
	e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



extern const device_type EPSON_E0C6S46;


#endif /* _E06200_H_ */
