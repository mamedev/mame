// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**************************\
*
*   SunPlus u'nSP emulator
*
*    by Ryan Holtz
*
\**************************/

#pragma once

#ifndef __UNSP_H__
#define __UNSP_H__

enum
{
	UNSP_SP = 1,
	UNSP_R1,
	UNSP_R2,
	UNSP_R3,
	UNSP_R4,
	UNSP_BP,
	UNSP_SR,
	UNSP_PC,

	UNSP_GPR_COUNT = UNSP_PC,

	UNSP_IRQ,
	UNSP_FIQ,
	UNSP_SB

};

enum
{
	UNSP_IRQ0_LINE = 0,
	UNSP_IRQ1_LINE,
	UNSP_IRQ2_LINE,
	UNSP_IRQ3_LINE,
	UNSP_IRQ4_LINE,
	UNSP_IRQ5_LINE,
	UNSP_IRQ6_LINE,
	UNSP_IRQ7_LINE,
	UNSP_FIQ_LINE,
	UNSP_BRK_LINE,

	UNSP_NUM_LINES
};


class unsp_device : public cpu_device
{
public:
	// construction/destruction
	unsp_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 5; }
	virtual UINT32 execute_max_cycles() const override { return 5; }
	virtual UINT32 execute_input_lines() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	UINT16 m_r[16];
	UINT8 m_irq;
	UINT8 m_fiq;
	UINT16 m_curirq;
	UINT16 m_sirq;
	UINT8 m_sb;
	UINT8 m_saved_sb;

	address_space *m_program;
	int m_icount;

	UINT32 m_debugger_temp;

	void unimplemented_opcode(UINT16 op);
	inline UINT16 READ16(UINT32 address);
	inline void WRITE16(UINT32 address, UINT16 data);
	inline void unsp_update_nz(UINT32 value);
	inline void unsp_update_nzsc(UINT32 value, UINT16 r0, UINT16 r1);
	inline void unsp_push(UINT16 value, UINT16 *reg);
	inline UINT16 unsp_pop(UINT16 *reg);


};


extern const device_type UNSP;


#endif /* __UNSP_H__ */
