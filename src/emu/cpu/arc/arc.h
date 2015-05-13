// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCtangent (A4) core
 ARC == Argonaut RISC Core

\*********************************/

#pragma once

#ifndef __ARC_H__
#define __ARC_H__

class arc_device : public cpu_device
{
public:
	// construction/destruction
	arc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	virtual UINT32 disasm_min_opcode_bytes() const { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;

	// 0 - 28 = r00 - r28 (General Purpose Registers)
	//     29 = r29 (ILINK1)
	//     30 = r30 (ILINE2)
	//     31 = r31 (BLINK)
	// 32- 59 = r32 - r59 (Reserved Registers)
	//     60 = LPCOUNT
	//     61 = Short Immediate Data Indicator Settings Flag
	//     62 = Long Immediate Data Indicator
	//     63 = Short Immediate Data Indicator NOT Settings Flag
	UINT32 m_pc;
	//UINT32 m_r[64];


	address_space *m_program;
	int m_icount;

	UINT32 m_debugger_temp;

	void unimplemented_opcode(UINT16 op);
	inline UINT32 READ32(UINT32 address);
	inline void WRITE32(UINT32 address, UINT32 data);
};


extern const device_type ARC;


#endif /* __ARC_H__ */
