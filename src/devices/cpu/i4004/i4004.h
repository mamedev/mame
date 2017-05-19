// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef MAME_CPU_I4004_I4004_H
#define MAME_CPU_I4004_I4004_H

#pragma once


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	I4004_PC,
	I4004_A,
	I4004_R01, I4004_R23, I4004_R45, I4004_R67, I4004_R89, I4004_RAB, I4004_RCD, I4004_REF,
	I4004_ADDR1,I4004_ADDR2,I4004_ADDR3,I4004_ADDR4,I4004_RAM
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class i4004_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	i4004_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t _clock);

	void set_test(uint8_t val);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 8; }
	virtual uint32_t execute_max_cycles() const override { return 16; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override
	{
		switch (spacenum)
		{
			case AS_PROGRAM: return &m_program_config;
			case AS_IO:      return &m_io_config;
			case AS_DATA:    return &m_data_config;
			default:         return nullptr;
		}
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 1; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	uint8_t ROP();
	uint8_t READ_ROM();
	void WPM();
	uint8_t ARG();
	uint8_t RM();
	uint8_t RMS(uint32_t a);
	void WM(uint8_t v);
	void WMP(uint8_t v);
	void WMS(uint32_t a, uint8_t v);
	uint8_t RIO();
	void WIO(uint8_t v);
	uint8_t GET_REG(uint8_t num);
	void SET_REG(uint8_t num, uint8_t val);
	void PUSH_STACK();
	void POP_STACK();
	void execute_one(int opcode);

	address_space_config m_program_config;
	address_space_config m_io_config;
	address_space_config m_data_config;

	uint8_t   m_A; // Accumulator
	uint8_t   m_R[8];
	PAIR    m_ADDR[4]; // Address registers
	PAIR    m_RAM;
	uint8_t   m_C; // Carry flag
	uint8_t   m_TEST; // Test PIN status
	PAIR    m_PC; // It is in fact one of ADDR regs
	uint8_t   m_flags; // used for I/O only

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;
	int                 m_icount;
	int                 m_pc_pos; // PC position in ADDR
	int                 m_addr_mask;
};


DECLARE_DEVICE_TYPE(I4004, i4004_cpu_device)

#endif // MAME_CPU_I4004_I4004_H
