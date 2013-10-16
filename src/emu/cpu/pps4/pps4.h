// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef __PPS4_H__
#define __PPS4_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/
enum
{
	PPS4_PC,
	PPS4_A,PPS4_X,PPS4_SA,PPS4_SB,PPS4_B,
	PPS4_GENPC = STATE_GENPC,
	PPS4_GENSP = STATE_GENSP,
	PPS4_GENPCBASE = STATE_GENPCBASE
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

extern const device_type PPS4;


class pps4_device : public cpu_device
{
public:
	// construction/destruction
	pps4_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 2; }
	virtual UINT32 execute_input_lines() const { return 0; }
	virtual UINT32 execute_default_irq_vector() const { return 0; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : ( (spacenum == AS_DATA) ? &m_data_config : NULL ) );
	}

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	UINT8   m_A; // Accumulator
	UINT8   m_X;

	PAIR    m_P;
	PAIR    m_SA;
	PAIR    m_SB;
	PAIR    m_B; // BU + BM + BL

	UINT8   m_C; // Carry flag
	UINT8   m_FF1; // Flip-flop 1
	UINT8   m_FF2; // Flip-flop 2

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;

	int                 m_icount;

	inline UINT8 ROP();
	inline UINT8 ARG();
	inline void DO_SKIP();
	void execute_one(int opcode);

};


#endif
