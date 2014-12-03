/*********************************\

 ARCompact Core

\*********************************/

#pragma once

#ifndef __ARCOMPACT_H__
#define __ARCOMPACT_H__

class arcompact_device : public cpu_device
{
public:
	// construction/destruction
	arcompact_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 5; }
	virtual UINT32 execute_max_cycles() const { return 5; }
	virtual UINT32 execute_input_lines() const { return 0; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : NULL; }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;

	UINT32 m_pc;

	address_space *m_program;
	int m_icount;

	UINT32 m_debugger_temp;

	void unimplemented_opcode(UINT16 op);
	inline UINT32 READ32(UINT32 address);
	inline void WRITE32(UINT32 address, UINT32 data);
	inline UINT16 READ16(UINT32 address);
	inline void WRITE16(UINT32 address, UINT16 data);


};


extern const device_type ARCA5;


#endif /* __ARCOMPACT_H__ */
