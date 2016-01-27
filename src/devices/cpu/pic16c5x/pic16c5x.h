// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                      Microchip PIC16C5x Emulator                         *
	*                                                                          *
	*                    Copyright Tony La Porta                               *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	*      Addressing architecture is based on the Harvard addressing scheme.  *
	*                                                                          *
	\**************************************************************************/

#pragma once

#ifndef __PIC16C5X_H__
#define __PIC16C5X_H__


// i/o ports
enum
{
	PIC16C5x_PORTA = 0,
	PIC16C5x_PORTB,
	PIC16C5x_PORTC
};

// port a, 4 bits, 2-way
#define MCFG_PIC16C5x_READ_A_CB(_devcb) \
	pic16c5x_device::set_read_a_callback(*device, DEVCB_##_devcb);
#define MCFG_PIC16C5x_WRITE_A_CB(_devcb) \
	pic16c5x_device::set_write_a_callback(*device, DEVCB_##_devcb);

// port b, 8 bits, 2-way
#define MCFG_PIC16C5x_READ_B_CB(_devcb) \
	pic16c5x_device::set_read_b_callback(*device, DEVCB_##_devcb);
#define MCFG_PIC16C5x_WRITE_B_CB(_devcb) \
	pic16c5x_device::set_write_b_callback(*device, DEVCB_##_devcb);

// port c, 8 bits, 2-way
#define MCFG_PIC16C5x_READ_C_CB(_devcb) \
	pic16c5x_device::set_read_c_callback(*device, DEVCB_##_devcb);
#define MCFG_PIC16C5x_WRITE_C_CB(_devcb) \
	pic16c5x_device::set_write_c_callback(*device, DEVCB_##_devcb);

// T0 pin (readline)
#define MCFG_PIC16C5x_T0_CB(_devcb) \
	pic16c5x_device::set_t0_callback(*device, DEVCB_##_devcb);

// CONFIG register
#define MCFG_PIC16C5x_SET_CONFIG(_data) \
	pic16c5x_device::set_config_static(*device, _data);



extern const device_type PIC16C54;
extern const device_type PIC16C55;
extern const device_type PIC16C56;
extern const device_type PIC16C57;
extern const device_type PIC16C58;


class pic16c5x_device : public cpu_device
{
public:
	// construction/destruction
	pic16c5x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int program_width, int data_width, int picmodel);

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_a_callback(device_t &device, _Object object) { return downcast<pic16c5x_device &>(device).m_read_a.set_callback(object); }
	template<class _Object> static devcb_base &set_read_b_callback(device_t &device, _Object object) { return downcast<pic16c5x_device &>(device).m_read_b.set_callback(object); }
	template<class _Object> static devcb_base &set_read_c_callback(device_t &device, _Object object) { return downcast<pic16c5x_device &>(device).m_read_c.set_callback(object); }

	template<class _Object> static devcb_base &set_write_a_callback(device_t &device, _Object object) { return downcast<pic16c5x_device &>(device).m_write_a.set_callback(object); }
	template<class _Object> static devcb_base &set_write_b_callback(device_t &device, _Object object) { return downcast<pic16c5x_device &>(device).m_write_b.set_callback(object); }
	template<class _Object> static devcb_base &set_write_c_callback(device_t &device, _Object object) { return downcast<pic16c5x_device &>(device).m_write_c.set_callback(object); }

	template<class _Object> static devcb_base &set_t0_callback(device_t &device, _Object object) { return downcast<pic16c5x_device &>(device).m_read_t0.set_callback(object); }

	/****************************************************************************
	 *  Function to configure the CONFIG register. This is actually hard-wired
	 *  during ROM programming, so should be called in the driver INIT, with
	 *  the value if known (available in HEX dumps of the ROM).
	 */
	void pic16c5x_set_config(UINT16 data);

	// or with a macro
	static void set_config_static(device_t &device, UINT16 data) { downcast<pic16c5x_device &>(device).m_temp_config = data; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	/**************************************************************************
	 *  Internal Clock divisor
	 *
	 *  External Clock is divided internally by 4 for the instruction cycle
	 *  times. (Each instruction cycle passes through 4 machine states). This
	 *  is handled by the cpu execution engine.
	 */
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 4 - 1) / 4; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 4); }
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 2; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual UINT32 execute_default_irq_vector() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr );
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	/******************** CPU Internal Registers *******************/
	UINT16  m_PC;
	UINT16  m_PREVPC;     /* previous program counter */
	UINT8   m_W;
	UINT8   m_OPTION;
	UINT16  m_CONFIG;
	UINT8   m_ALU;
	UINT16  m_WDT;
	UINT8   m_TRISA;
	UINT8   m_TRISB;
	UINT8   m_TRISC;
	UINT16  m_STACK[2];
	UINT16  m_prescaler;  /* Note: this is really an 8-bit register */
	PAIR    m_opcode;
	UINT8   *m_internalram;

	int     m_icount;
	int     m_reset_vector;
	int     m_picmodel;
	int     m_delay_timer;
	UINT16  m_temp_config;
	UINT8   m_old_T0;
	INT8    m_old_data;
	UINT8   m_picRAMmask;
	int     m_inst_cycles;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;

	// i/o handlers
	devcb_read8 m_read_a;
	devcb_read8 m_read_b;
	devcb_read8 m_read_c;
	devcb_write8 m_write_a;
	devcb_write8 m_write_b;
	devcb_write8 m_write_c;
	devcb_read_line m_read_t0;

	// For debugger
	int m_debugger_temp;

	/* opcode table entry */
	typedef void (pic16c5x_device::*pic16c5x_ophandler)();
	struct pic16c5x_opcode
	{
		UINT8   cycles;
		pic16c5x_ophandler function;
	};
	static const pic16c5x_opcode s_opcode_main[256];
	static const pic16c5x_opcode s_opcode_00x[16];

	void update_internalram_ptr();
	void CALCULATE_Z_FLAG();
	void CALCULATE_ADD_CARRY();
	void CALCULATE_ADD_DIGITCARRY();
	void CALCULATE_SUB_CARRY();
	void CALCULATE_SUB_DIGITCARRY();
	UINT16 POP_STACK();
	void PUSH_STACK(UINT16 data);
	UINT8 GET_REGFILE(offs_t addr);
	void STORE_REGFILE(offs_t addr, UINT8 data);
	void STORE_RESULT(offs_t addr, UINT8 data);
	void illegal();
	void addwf();
	void andwf();
	void andlw();
	void bcf();
	void bsf();
	void btfss();
	void btfsc();
	void call();
	void clrw();
	void clrf();
	void clrwdt();
	void comf();
	void decf();
	void decfsz();
	void goto_op();
	void incf();
	void incfsz();
	void iorlw();
	void iorwf();
	void movf();
	void movlw();
	void movwf();
	void nop();
	void option();
	void retlw();
	void rlf();
	void rrf();
	void sleepic();
	void subwf();
	void swapf();
	void tris();
	void xorlw();
	void xorwf();
	void pic16c5x_reset_regs();
	void pic16c5x_soft_reset();
	void pic16c5x_update_watchdog(int counts);
	void pic16c5x_update_timer(int counts);

};


class pic16c54_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c54_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class pic16c55_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c55_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class pic16c56_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c56_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class pic16c57_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c57_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class pic16c58_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c58_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

#endif  /* __PIC16C5X_H__ */
