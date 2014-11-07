#ifndef _TMS0980_H_
#define _TMS0980_H_


/* Registers */
enum {
	TMS0980_PC=1, TMS0980_SR, TMS0980_PA, TMS0980_PB,
	TMS0980_A, TMS0980_X, TMS0980_Y, TMS0980_STATUS
};


#define MCFG_TMS1XXX_OUTPUT_PLA(_pla) \
	tms1xxx_cpu_device::set_output_pla(*device, _pla);

#define MCFG_TMS1XXX_READ_K(_devcb) \
	tms1xxx_cpu_device::set_read_k(*device, DEVCB_##_devcb);

#define MCFG_TMS1XXX_WRITE_O(_devcb) \
	tms1xxx_cpu_device::set_write_o(*device, DEVCB_##_devcb);

#define MCFG_TMS1XXX_WRITE_R(_devcb) \
	tms1xxx_cpu_device::set_write_r(*device, DEVCB_##_devcb);


class tms1xxx_cpu_device : public cpu_device
{
public:
	// construction/destruction
	tms1xxx_cpu_device( const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock
						, const UINT32* decode_table, UINT16 o_mask, UINT16 r_mask, UINT8 pc_size, UINT8 byte_size, UINT8 x_bits
						, int program_addrbus_width, address_map_constructor program, int data_addrbus_width, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device( mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_BIG, byte_size > 8 ? 16 : 8, program_addrbus_width, 0, program )
		, m_data_config("data", ENDIANNESS_BIG, 8, data_addrbus_width, 0, data )
		, m_pc(0)
		, m_pa(0)
		, m_sr(0)
		, m_pb(0)
		, m_a(0)
		, m_x(0)
		, m_y(0)
		, m_status(0)
		, m_o_mask( o_mask )
		, m_r_mask( r_mask )
		, m_pc_size( pc_size )
		, m_byte_size( byte_size )
		, m_x_bits( x_bits )
		, m_decode_table( decode_table )
		, c_output_pla( NULL )
		, m_read_k( *this )
		, m_write_o( *this )
		, m_write_r( *this )
	{ }

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_k(device_t &device, _Object object) { return downcast<tms1xxx_cpu_device &>(device).m_read_k.set_callback(object); }
	template<class _Object> static devcb_base &set_write_o(device_t &device, _Object object) { return downcast<tms1xxx_cpu_device &>(device).m_write_o.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r(device_t &device, _Object object) { return downcast<tms1xxx_cpu_device &>(device).m_write_r.set_callback(object); }
	static void set_output_pla(device_t &device, const UINT16 *output_pla) { downcast<tms1xxx_cpu_device &>(device).c_output_pla = output_pla; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 6; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA ) ? &m_data_config : NULL ); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 1; }

	void next_pc();
	void set_cki_bus();

	address_space_config m_program_config;
	address_space_config m_data_config;

	UINT8   m_prev_pc;      /* previous program counter */
	UINT8   m_prev_pa;      /* previous page address register */
	UINT8   m_pc;           /* program counter is a 7 bit register on tms0980, 6 bit register on tms1000/1070/1200/1270/1100/1300 */
	UINT8   m_pa;           /* page address register is a 4 bit register */
	UINT8   m_sr;           /* subroutine return register is a 7 bit register */
	UINT8   m_pb;           /* page buffer register is a 4 bit register */
	UINT8   m_a;            /* Accumulator is a 4 bit register (?) */
	UINT8   m_x;            /* X-register is a 2, 3, or 4 bit register */
	UINT8   m_y;            /* Y-register is a 4 bit register */
	UINT8   m_dam;          /* DAM register is a 4 bit register */
	UINT8   m_ca;           /* Chapter address bit */
	UINT8   m_cb;           /* Chapter buffer bit */
	UINT8   m_cs;           /* Chapter subroutine bit */
	UINT16  m_r;
	UINT8   m_o;
	UINT8   m_cki_bus;      /* CKI bus */
	UINT8   m_p;            /* adder p-input */
	UINT8   m_n;            /* adder n-input */
	UINT8   m_adder_result; /* adder result */
	UINT8   m_carry_in;     /* carry in */
	UINT8   m_status;
	UINT8   m_status_latch;
	UINT8   m_special_status;
	UINT8   m_call_latch;
	UINT8   m_add_latch;
	UINT8   m_branch_latch;
	int     m_subcycle;
	UINT8   m_ram_address;
	UINT16  m_ram_data;
	UINT16  m_rom_address;
	UINT16  m_opcode;
	UINT32  m_decode;
	int     m_icount;
	UINT16  m_o_mask;       /* mask to determine the number of O outputs */
	UINT16  m_r_mask;       /* mask to determine the number of R outputs */
	UINT8   m_pc_size;      /* how bits in the PC register */
	UINT8   m_byte_size;    /* 8 or 9 bit bytes */
	UINT8   m_x_bits;       /* determine the number of bits in the X register */
	const UINT32 *m_decode_table;
	address_space *m_program;
	address_space *m_data;

	const UINT16 *c_output_pla;
	devcb_read8 m_read_k;
	devcb_write16 m_write_o;
	devcb_write16 m_write_r;

};


class tms0980_cpu_device : public tms1xxx_cpu_device
{
public:
	tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


class tms1000_cpu_device : public tms1xxx_cpu_device
{
public:
	tms1000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms1000_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT16 o_mask, UINT16 r_mask, const char *shortname, const char *source);

protected:
	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


class tms0970_cpu_device : public tms1000_cpu_device
{
public:
	tms0970_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms1070_cpu_device : public tms1000_cpu_device
{
public:
	tms1070_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms1200_cpu_device : public tms1000_cpu_device
{
public:
	tms1200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms1270_cpu_device : public tms1000_cpu_device
{
public:
	tms1270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms1100_cpu_device : public tms1xxx_cpu_device
{
public:
	tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT16 o_mask, UINT16 r_mask, const char *shortname, const char *source);

protected:
	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, astring &string);

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};


class tms1300_cpu_device : public tms1100_cpu_device
{
public:
	tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


/* 9-bit family */
extern const device_type TMS0980;

/* 8-bit family */
extern const device_type TMS1000;
extern const device_type TMS0970;
extern const device_type TMS1070;
extern const device_type TMS1200;
extern const device_type TMS1270;
extern const device_type TMS1100;
extern const device_type TMS1300;


#endif /* _TMS0980_H_ */
