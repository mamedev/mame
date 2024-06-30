// license:BSD-3-Clause
// copyright-holders:Tony La Porta, Grull Osgo
/************************************************************************

  Microchip PIC16x8x Emulator

  Based on MAME's PIC16C5x/62x cpu devices developed by Tony La Porta
  and improvements to SFR's accesss made by ajrhacker.


************************************************************************/

#ifndef MAME_CPU_PIC16X8X_PIC16X8X_H
#define MAME_CPU_PIC16X8X_PIC16X8X_H

#pragma once

// input lines
enum
{
	PIC16x8x_T0CKI = 0,
	PIC16x8x_RB0INT
};

DECLARE_DEVICE_TYPE(PIC16CR83, pic16cr83_device)
DECLARE_DEVICE_TYPE(PIC16CR84, pic16cr84_device)
DECLARE_DEVICE_TYPE(PIC16F83,  pic16f83_device)
DECLARE_DEVICE_TYPE(PIC16F84,  pic16f84_device)
DECLARE_DEVICE_TYPE(PIC16F84A, pic16f84a_device)

class pic16x8x_device : public cpu_device, public device_nvram_interface
{

// i/o ports
	enum
	{
		PORTA = 0,
		PORTB
	};

public:
	// port a, 5 bits, 2-way
	auto read_a() { return m_read_port[PORTA].bind(); }
	auto write_a() { return m_write_port[PORTA].bind(); }

	// port b, 8 bits, 2-way
	auto read_b() { return m_read_port[PORTB].bind(); }
	auto write_b() { return m_write_port[PORTB].bind(); }

	/****************************************************************************
	 *  Function to configure the CONFIG register. This is actually hard-wired
	 *  during ROM programming, so should be called in the driver INIT, with
	 *  the value if known (available in HEX dumps of the ROM).
	 ****************************************************************************/
	void set_config(u16 data);

	void core_regs(address_map &map, u8 mirror = 0);

	void ram_6(address_map &map);
	void rom_9(address_map &map);
	void ram_7(address_map &map);
	void rom_10(address_map &map);

protected:
	// construction/destruction
	pic16x8x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, address_map_constructor program_map, address_map_constructor data_map);

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
	 **************************************************************************/
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }
	virtual u32 execute_input_lines() const noexcept override { return 1; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == PIC16x8x_T0CKI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int line, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_nvram_interface implementation
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual void nvram_default() override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	u8 m_eeprom_data[0x40];
	u16 m_buff[0x40];

	optional_memory_region m_region;

	// address spaces
	address_space_config m_program_config;
	address_space_config m_data_config;

	int m_program_width;

private:

	memory_access<13, 1, -1, ENDIANNESS_LITTLE>::cache m_program;
	memory_access< 8, 0,  0, ENDIANNESS_LITTLE>::specific m_data;

	/******************** CPU Internal Registers *******************/
	u16     m_PCL;
	u16     m_PREVPC;
	u8      m_W;
	u8      m_OPTION;
	u16     m_CONFIG;
	u8      m_ALU;
	u16     m_WDT;
	u8      m_TMR0;
	u8      m_STATUS;
	u8      m_FSR;
	u8      m_EEDATA;
	u8      m_EEADR;
	u8      m_PCLATH;
	u8      m_INTCON;
	u8      m_EECON1;
	u8      m_EECON2;
	u8      m_port_data[2];
	u8      m_port_tris[2];
	u16     m_STACK[8];
	u16     m_prescaler;  // Note: this is really an 8-bit register
	PAIR16  m_opcode;
	int     m_icount;
	int     m_delay_timer;
	int     m_rtcc;
	u8      m_count_cycles;
	u8      m_data_mask;
	u16     m_program_mask;
	u8      m_status_mask;
	u8      m_inst_cycles;
	u8      m_stack_pointer;
	u8      m_old_RB0;
	u8      m_portb_chdetect_temp;
	bool    m_irq_in_progress;

	const u8 m_internal_eeprom_size = 0x40;

	// i/o handlers
	devcb_read8::array<2> m_read_port;
	devcb_write8::array<2> m_write_port;

	// For debugger
	int m_debugger_temp;

	// opcode table entry
	typedef void (pic16x8x_device::*pic16x8x_ophandler)();
	struct pic16x8x_opcode
	{
		u8 cycles;
		pic16x8x_ophandler function;
	};

	static const pic16x8x_opcode s_opcode_main[128];
	static const pic16x8x_opcode s_opcode_00x[128];

	void check_irqs();

	// EEPROM data access
	u8 m_eeread(offs_t offs);
	void m_eewrite(offs_t offs, u8 data);

	//void update_internalram_ptr();
	void calc_zero_flag();
	void calc_add_flags(u8 augend);
	void calc_sub_flags(u8 minuend);
	u16 pop_stack();
	void push_stack(u16 data);
	void set_pc(u16 addr);
	u8 get_regfile(u8 addr);
	void store_regfile(u8 addr, u8 data);
	void store_result(u8 addr, u8 data);
	u8 tmr0_r();
	void tmr0_w(u8 data);
	u8 pcl_r();
	void pcl_w(u8 data);
	u8 status_r();
	void status_w(u8 data);
	u8 fsr_r();
	void fsr_w(u8 data);
	u8 porta_r();
	void porta_w(u8 data);
	u8 portb_r();
	void portb_w(u8 data);
	u8 eedata_r();
	void eedata_w(u8 data);
	u8 eeadr_r();
	void eeadr_w(u8 data);
	u8 pclath_r();
	void pclath_w(u8 data);
	u8 intcon_r();
	void intcon_w(u8 data);
	u8 trisa_r();
	void trisa_w(u8 data);
	u8 trisb_r();
	void trisb_w(u8 data);
	u8 eecon1_r();
	void eecon1_w(u8 data);
	u8 eecon2_r();
	void eecon2_w(u8 data);
	u8 option_r();
	void option_w(u8 data);
	void reset_regs();
	void watchdog_reset();
	void update_watchdog(int counts);
	void update_timer(int counts);

	void illegal();
	void addlw();   // new for 16x8x
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
	void retfie();  // new for 16x8x
	void retlw();
	void retrn();   // new for 16x8x - can´t use return as mnemonic - Others PIC's uses "returns", should I use it?
	void rlf();
	void rrf();
	void sleepic();
	void sublw();   // new for 16x8x
	void subwf();
	void swapf();
	void xorlw();
	void xorwf();
};


class pic16x83_device : public pic16x8x_device
{
public:
	// construction/destruction
	pic16x83_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock); //, int program_width);
};

class pic16x84_device : public pic16x8x_device
{
public:
	// construction/destruction
	pic16x84_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock); //, int program_width);
};

class pic16cr83_device : public pic16x83_device
{
public:
	// construction/destruction
	pic16cr83_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class pic16cr84_device : public pic16x84_device
{
public:
	// construction/destruction
	pic16cr84_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class  pic16f83_device : public pic16x83_device
{
public:
	// construction/destruction
	 pic16f83_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class pic16f84_device : public pic16x84_device
{
public:
	// construction/destruction
	pic16f84_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class pic16f84a_device : public pic16x84_device
{
public:
	// construction/destruction
	pic16f84a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

#endif  // MAME_CPU_PIC16X8X_PIC16X8X_H
