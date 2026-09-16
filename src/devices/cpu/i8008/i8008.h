// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_CPU_I8008_I8008_H
#define MAME_CPU_I8008_I8008_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> asap_device
class i8008_device : public cpu_device
{
public:
	// construction/destruction
	i8008_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	enum
	{
		I8008_PC,
		I8008_A,I8008_B,I8008_C,I8008_D,I8008_E,I8008_H,I8008_L,
		I8008_ADDR1,I8008_ADDR2,I8008_ADDR3,I8008_ADDR4,I8008_ADDR5,I8008_ADDR6,I8008_ADDR7,I8008_ADDR8
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
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

	virtual void execute_one(int opcode);

	void push_stack();
	void pop_stack();
	uint8_t rop();
	uint8_t get_reg(uint8_t reg);
	void set_reg(uint8_t reg, uint8_t val);
	uint8_t arg();
	void update_flags(uint8_t val);
	uint8_t do_condition(uint8_t val);
	uint16_t get_addr();
	void illegal(uint8_t opcode);
	void take_interrupt();
	void init_tables(void);

	int m_pc_pos; // PC position in ADDR
	int m_icount;

	// configuration
	const address_space_config      m_program_config;
	const address_space_config      m_io_config;

	uint8_t   m_A,m_B,m_C,m_D,m_E,m_H,m_L;
	PAIR    m_PC; // It is in fact one of ADDR regs
	PAIR    m_ADDR[8]; // Address registers
	uint8_t   m_CF; // Carry flag
	uint8_t   m_ZF; // Zero flag
	uint8_t   m_SF; // Sign flag
	uint8_t   m_PF; // Parity flag
	uint8_t   m_HALT;
	uint8_t   m_flags; // temporary I/O only

	uint8_t   m_irq_state;

	uint8_t m_PARITY[256];

	memory_access<14, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<14, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_io;
};

// device type definition
DECLARE_DEVICE_TYPE(I8008, i8008_device)

#endif // MAME_CPU_I8008_I8008_H
