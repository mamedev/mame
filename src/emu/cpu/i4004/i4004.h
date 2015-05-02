// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef __I4004_H__
#define __I4004_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	I4004_PC,
	I4004_A,
	I4004_R01, I4004_R23, I4004_R45, I4004_R67, I4004_R89, I4004_RAB, I4004_RCD, I4004_REF,
	I4004_ADDR1,I4004_ADDR2,I4004_ADDR3,I4004_ADDR4,I4004_RAM,
	I4004_GENPC = STATE_GENPC,
	I4004_GENSP = STATE_GENSP,
	I4004_GENPCBASE = STATE_GENPCBASE
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class i4004_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	i4004_cpu_device(const machine_config &mconfig, const char *_tag, device_t *_owner, UINT32 _clock);

	void set_test(UINT8 val);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 8; }
	virtual UINT32 execute_max_cycles() const { return 16; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		switch (spacenum)
		{
			case AS_PROGRAM: return &m_program_config;
			case AS_IO:      return &m_io_config;
			case AS_DATA:    return &m_data_config;
			default:         return NULL;
		}
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	virtual void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	UINT8 ROP();
	UINT8 READ_ROM();
	void WPM();
	UINT8 ARG();
	UINT8 RM();
	UINT8 RMS(UINT32 a);
	void WM(UINT8 v);
	void WMP(UINT8 v);
	void WMS(UINT32 a, UINT8 v);
	UINT8 RIO();
	void WIO(UINT8 v);
	UINT8 GET_REG(UINT8 num);
	void SET_REG(UINT8 num, UINT8 val);
	void PUSH_STACK();
	void POP_STACK();
	void execute_one(int opcode);

	address_space_config m_program_config;
	address_space_config m_io_config;
	address_space_config m_data_config;

	UINT8   m_A; // Accumulator
	UINT8   m_R[8];
	PAIR    m_ADDR[4]; // Address registers
	PAIR    m_RAM;
	UINT8   m_C; // Carry flag
	UINT8   m_TEST; // Test PIN status
	PAIR    m_PC; // It is in fact one of ADDR regs
	UINT8   m_flags; // used for I/O only

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;
	int                 m_icount;
	int                 m_pc_pos; // PC possition in ADDR
	int                 m_addr_mask;
};


extern const device_type I4004;


#endif
