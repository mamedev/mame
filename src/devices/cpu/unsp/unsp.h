// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**************************\
*
*   SunPlus u'nSP emulator
*
*    by Ryan Holtz
*
\**************************/

#ifndef MAME_CPU_UNSP_UNSP_H
#define MAME_CPU_UNSP_UNSP_H

#pragma once

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
	unsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 5; }
	virtual uint32_t execute_max_cycles() const override { return 5; }
	virtual uint32_t execute_input_lines() const override { return 0; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	uint16_t m_r[16];
	uint8_t m_irq;
	uint8_t m_fiq;
	uint16_t m_curirq;
	uint16_t m_sirq;
	uint8_t m_sb;
	uint8_t m_saved_sb;

	address_space *m_program;
	int m_icount;

	uint32_t m_debugger_temp;

	void unimplemented_opcode(uint16_t op);
	inline uint16_t READ16(uint32_t address);
	inline void WRITE16(uint32_t address, uint16_t data);
	inline void unsp_update_nz(uint32_t value);
	inline void unsp_update_nzsc(uint32_t value, uint16_t r0, uint16_t r1);
	inline void unsp_push(uint16_t value, uint16_t *reg);
	inline uint16_t unsp_pop(uint16_t *reg);


};


DECLARE_DEVICE_TYPE(UNSP, unsp_device)

#endif // MAME_CPU_UNSP_UNSP_H
