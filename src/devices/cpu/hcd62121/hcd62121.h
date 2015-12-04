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
	hcd62121_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 4; }
	virtual UINT32 execute_max_cycles() const { return 48; }
	virtual UINT32 execute_input_lines() const { return 2; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 18; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	UINT8 read_op();
	UINT8 datasize( UINT8 op );
	void read_reg( int size, UINT8 op1 );
	void write_reg( int size, UINT8 op1 );
	void read_regreg( int size, UINT8 op1, UINT8 op2, bool op_is_logical );
	void write_regreg( int size, UINT8 op1, UINT8 op2 );
	void read_iregreg( int size, UINT8 op1, UINT8 op2 );
	void write_iregreg( int size, UINT8 op1, UINT8 op2 );
	void write_iregreg2( int size, UINT8 op1, UINT8 op2 );
	int check_cond( UINT8 op );

	address_space_config m_program_config;
	address_space_config m_io_config;

	UINT32 m_prev_pc;
	UINT16 m_sp;
	UINT16 m_ip;
	UINT8 m_dsize;
	UINT8 m_cseg;
	UINT8 m_dseg;
	UINT8 m_sseg;
	UINT8 m_f;
	UINT16 m_lar;
	UINT8 m_reg[0x80];
	UINT8 m_temp1[0x10];
	UINT8 m_temp2[0x10];

	address_space *m_program;
	address_space *m_io;
	int m_icount;
};


extern const device_type HCD62121;


#endif /* __HCD62121_H__ */
