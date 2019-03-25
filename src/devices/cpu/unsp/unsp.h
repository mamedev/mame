// license:GPL-2.0+
// copyright-holders:Segher Boessenkool,Ryan Holtz
/*****************************************************************************

    SunPlus µ'nSP emulator

    Copyright 2008-2017  Segher Boessenkool  <segher@kernel.crashing.org>
    Licensed under the terms of the GNU GPL, version 2
    http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

    Ported to MAME framework by Ryan Holtz

*****************************************************************************/

#ifndef MAME_CPU_UNSP_UNSP_H
#define MAME_CPU_UNSP_UNSP_H

#pragma once

#define UNSP_LOG_OPCODES        (0)

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

	UNSP_IRQ_EN,
	UNSP_FIQ_EN,
	UNSP_IRQ,
	UNSP_FIQ,
#if UNSP_LOG_OPCODES
	UNSP_SB,
	UNSP_LOG_OPS
#else
	UNSP_SB
#endif
};

enum
{
	UNSP_FIQ_LINE = 0,
	UNSP_IRQ0_LINE,
	UNSP_IRQ1_LINE,
	UNSP_IRQ2_LINE,
	UNSP_IRQ3_LINE,
	UNSP_IRQ4_LINE,
	UNSP_IRQ5_LINE,
	UNSP_IRQ6_LINE,
	UNSP_IRQ7_LINE,
	UNSP_BRK_LINE,

	UNSP_NUM_LINES
};

class unsp_device : public cpu_device
{
public:
	// construction/destruction
	unsp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_timer_interval(int timer, uint32_t interval);

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
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	void add_lpc(const int32_t offset);

	inline void execute_one(const uint16_t op);

	address_space_config m_program_config;

	uint16_t m_r[16];
	bool m_enable_irq;
	bool m_enable_fiq;
	bool m_irq;
	bool m_fiq;
	uint16_t m_curirq;
	uint16_t m_sirq;
	uint8_t m_sb;
	uint8_t m_saved_sb[3];

	address_space *m_program;
	int m_icount;

	uint32_t m_debugger_temp;
#if UNSP_LOG_OPCODES
	uint32_t m_log_ops;
#endif

	void unimplemented_opcode(uint16_t op);
	inline uint16_t read16(uint32_t address);
	inline void write16(uint32_t address, uint16_t data);
	inline void update_nz(uint32_t value);
	inline void update_sc(uint32_t value, uint16_t r0, uint16_t r1);
	inline void push(uint16_t value, uint16_t *reg);
	inline uint16_t pop(uint16_t *reg);
	inline void trigger_fiq();
	inline void trigger_irq(int line);
	inline void check_irqs();
};


DECLARE_DEVICE_TYPE(UNSP, unsp_device)

#endif // MAME_CPU_UNSP_UNSP_H
