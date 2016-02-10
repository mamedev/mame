// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef __IE15_H__
#define __IE15_H__

//**************************************************************************
//  ENUMERATIONS
//**************************************************************************

enum
{
	IE15_PC,
	IE15_A,
	IE15_R0, IE15_R1, IE15_R2, IE15_R3, IE15_R4, IE15_R5, IE15_R6, IE15_R7,
	IE15_R8, IE15_R9, IE15_R10, IE15_R11, IE15_R12, IE15_R13, IE15_R14, IE15_R15,
	IE15_R16, IE15_R17, IE15_R18, IE15_R19, IE15_R20, IE15_R21, IE15_R22, IE15_R23,
	IE15_R24, IE15_R25, IE15_R26, IE15_R27, IE15_R28, IE15_R29, IE15_R30, IE15_R31,

	IE15_GENPC = STATE_GENPC,
	IE15_GENPCBASE = STATE_GENPCBASE
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ie15_device;

class ie15_device : public cpu_device
{
public:
	// construction/destruction
	ie15_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override;
	virtual UINT32 execute_max_cycles() const override;
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override;
	virtual UINT32 disasm_max_opcode_bytes() const override;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	virtual void execute_one(int opcode);

	UINT8 rop();
	UINT8 get_reg_lo(UINT8 reg);
	UINT16 get_reg(UINT8 reg);
	void set_reg(UINT8 reg, UINT16 val);
	UINT8 arg();
	void update_flags(UINT8 val);
	UINT8 do_condition(UINT8 val);
	UINT16 get_addr(UINT8 val);
	void illegal(UINT8 opcode);

	int m_icount;

	// configuration
	const address_space_config      m_program_config;
	const address_space_config      m_io_config;

	UINT8   m_A;
	PAIR    m_PC;
	UINT16  m_REGS[32]; // General registers (2 pages of 16)
	UINT8   m_CF; // Carry flag
	UINT8   m_ZF; // Zero flag
	UINT8   m_RF; // Current register page
	UINT8   m_flags; // temporary I/O only

	address_space *m_program;
	address_space *m_io;
	direct_read_data *m_direct;
};

// device type definition
extern const device_type IE15;

#endif
