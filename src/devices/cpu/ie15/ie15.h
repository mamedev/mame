// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_CPU_IE15_IE15_H
#define MAME_CPU_IE15_IE15_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ie15_cpu_device : public cpu_device
{
public:
	// construction/destruction
	ie15_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	enum
	{
		IE15_PC,
		IE15_A,
		IE15_R0, IE15_R1, IE15_R2, IE15_R3, IE15_R4, IE15_R5, IE15_R6, IE15_R7,
		IE15_R8, IE15_R9, IE15_R10, IE15_R11, IE15_R12, IE15_R13, IE15_R14, IE15_R15,
		IE15_R16, IE15_R17, IE15_R18, IE15_R19, IE15_R20, IE15_R21, IE15_R22, IE15_R23,
		IE15_R24, IE15_R25, IE15_R26, IE15_R27, IE15_R28, IE15_R29, IE15_R30, IE15_R31
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual void execute_one(int opcode);

	uint8_t rop();
	uint8_t get_reg_lo(uint8_t reg);
	uint16_t get_reg(uint8_t reg);
	void set_reg(uint8_t reg, uint16_t val);
	uint8_t arg();
	void update_flags(uint8_t val);
	uint8_t do_condition(uint8_t val);
	uint16_t get_addr(uint8_t val);
	void illegal(uint8_t opcode);

	int m_icount;

	// configuration
	const address_space_config      m_program_config;
	const address_space_config      m_io_config;

	uint8_t   m_A;
	PAIR    m_PC;
	uint16_t  m_REGS[32]; // General registers (2 pages of 16)
	uint8_t   m_CF; // Carry flag
	uint8_t   m_ZF; // Zero flag
	uint8_t   m_RF; // Current register page
	uint8_t   m_flags; // temporary I/O only

	memory_access<14, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<14, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access< 8, 0, 0, ENDIANNESS_LITTLE>::specific m_io;
};

// device type definition
DECLARE_DEVICE_TYPE(IE15_CPU, ie15_cpu_device)

#endif // MAME_CPU_IE15_IE15_H
