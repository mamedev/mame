// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#pragma once

#ifndef __S2650_H__
#define __S2650_H__


enum
{
	S2650_PC=1, S2650_PS, S2650_R0, S2650_R1, S2650_R2, S2650_R3,
	S2650_R1A, S2650_R2A, S2650_R3A,
	S2650_HALT, S2650_SI, S2650_FO
};

/* fake I/O space ports */
enum
{
	S2650_EXT_PORT      = 0x00ff,   /* M/~IO=0 D/~C=x E/~NE=1 */
	S2650_CTRL_PORT     = 0x0100,   /* M/~IO=0 D/~C=0 E/~NE=0 */
	S2650_DATA_PORT     = 0x0101,   /* M/~IO=0 D/~C=1 E/~NE=0 */
	S2650_SENSE_PORT    = 0x0102    /* Fake Sense Line */
};


extern const device_type S2650;


#define MCFG_S2650_FLAG_HANDLER(_devcb) \
	devcb = &s2650_device::set_flag_handler(*device, DEVCB_##_devcb);

class s2650_device : public cpu_device
{
public:
	// construction/destruction
	s2650_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER(write_sense);

	// static configuration helpers
	template<class _Object> static devcb_base &set_flag_handler(device_t &device, _Object object) { return downcast<s2650_device &>(device).m_flag_handler.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 5; }
	virtual UINT32 execute_max_cycles() const { return 13; }
	virtual UINT32 execute_input_lines() const { return 2; }
	virtual UINT32 execute_default_irq_vector() const { return 0; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : NULL );
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry);
	virtual void state_export(const device_state_entry &entry);
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 3; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;
	address_space_config m_io_config;

	devcb_write_line m_flag_handler;

	UINT16  m_ppc;    /* previous program counter (page + iar) */
	UINT16  m_page;   /* 8K page select register (A14..A13) */
	UINT16  m_iar;    /* instruction address register (A12..A0) */
	UINT16  m_ea;     /* effective address (A14..A0) */
	UINT8   m_psl;    /* processor status lower */
	UINT8   m_psu;    /* processor status upper */
	UINT8   m_r;      /* absolute addressing dst/src register */
	UINT8   m_reg[7]; /* 7 general purpose registers */
	UINT8   m_halt;   /* 1 if cpu is halted */
	UINT8   m_ir;     /* instruction register */
	UINT16  m_ras[8]; /* 8 return address stack entries */
	UINT8   m_irq_state;

	int     m_icount;
	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;

	// For debugger
	UINT16  m_debugger_temp;

	inline void set_psu(UINT8 new_val);
	inline UINT8 get_sp();
	inline void set_sp(UINT8 new_sp);
	inline int check_irq_line();
	inline UINT8 ROP();
	inline UINT8 ARG();
	void s2650_set_flag(int state);
	int s2650_get_flag();
	void s2650_set_sense(int state);
};


#endif /* __S2650_H__ */
