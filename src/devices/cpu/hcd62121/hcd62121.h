// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __HCD62121_H__
#define __HCD62121_H__


enum
{
	HCD62121_IP=1, HCD62121_SP, HCD62121_F, HCD62121_LAR,
	HCD62121_CS, HCD62121_DS, HCD62121_SS, HCD62121_DSIZE,
	/* 128 byte register file */
	HCD62121_R00, HCD62121_R04, HCD62121_R08, HCD62121_R0C,
	HCD62121_R10, HCD62121_R14, HCD62121_R18, HCD62121_R1C,
	HCD62121_R20, HCD62121_R24, HCD62121_R28, HCD62121_R2C,
	HCD62121_R30, HCD62121_R34, HCD62121_R38, HCD62121_R3C,
	HCD62121_R40, HCD62121_R44, HCD62121_R48, HCD62121_R4C,
	HCD62121_R50, HCD62121_R54, HCD62121_R58, HCD62121_R5C,
	HCD62121_R60, HCD62121_R64, HCD62121_R68, HCD62121_R6C,
	HCD62121_R70, HCD62121_R74, HCD62121_R78, HCD62121_R7C
};


/* I/O ports */
enum
{
	/* Output ports */
	HCD62121_KOL=0x00,
	HCD62121_KOH,
	/* Input ports */
	HCD62121_KI,
	/* Other I/O ports */
	HCD62121_IN0
};


class hcd62121_cpu_device :  public cpu_device
{
public:
	// construction/destruction
	hcd62121_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 4; }
	virtual uint32_t execute_max_cycles() const override { return 48; }
	virtual uint32_t execute_input_lines() const override { return 2; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 1; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 18; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	uint8_t read_op();
	uint8_t datasize( uint8_t op );
	void read_reg( int size, uint8_t op1 );
	void write_reg( int size, uint8_t op1 );
	void read_regreg( int size, uint8_t op1, uint8_t op2, bool op_is_logical );
	void write_regreg( int size, uint8_t op1, uint8_t op2 );
	void read_iregreg( int size, uint8_t op1, uint8_t op2 );
	void write_iregreg( int size, uint8_t op1, uint8_t op2 );
	void write_iregreg2( int size, uint8_t op1, uint8_t op2 );
	int check_cond( uint8_t op );

	address_space_config m_program_config;
	address_space_config m_io_config;

	uint32_t m_prev_pc;
	uint16_t m_sp;
	uint16_t m_ip;
	uint8_t m_dsize;
	uint8_t m_cseg;
	uint8_t m_dseg;
	uint8_t m_sseg;
	uint8_t m_f;
	uint16_t m_lar;
	uint8_t m_reg[0x80];
	uint8_t m_temp1[0x10];
	uint8_t m_temp2[0x10];

	address_space *m_program;
	address_space *m_io;
	int m_icount;
};


extern const device_type HCD62121;


#endif /* __HCD62121_H__ */
