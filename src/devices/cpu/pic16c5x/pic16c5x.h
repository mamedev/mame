// license:BSD-3-Clause
// copyright-holders:Tony La Porta
/*

  Microchip PIC16C5x Emulator

  Copyright Tony La Porta
  Originally written for the MAME project.

*/

#ifndef MAME_CPU_PIC16C5X_PIC16C5X_H
#define MAME_CPU_PIC16C5X_PIC16C5X_H

#pragma once

// input lines
enum
{
	PIC16C5x_RTCC = 0
};

// in the mid-90s RTCC was renamed to T0CKI
#define PIC16C5x_T0CKI PIC16C5x_RTCC


// i/o ports
enum
{
	PIC16C5x_PORTA = 0,
	PIC16C5x_PORTB,
	PIC16C5x_PORTC,
	PIC16C5x_PORTD
};

DECLARE_DEVICE_TYPE(PIC16C54, pic16c54_device)
DECLARE_DEVICE_TYPE(PIC16C55, pic16c55_device)
DECLARE_DEVICE_TYPE(PIC16C56, pic16c56_device)
DECLARE_DEVICE_TYPE(PIC16C57, pic16c57_device)
DECLARE_DEVICE_TYPE(PIC16C58, pic16c58_device)

DECLARE_DEVICE_TYPE(PIC1650,  pic1650_device)
DECLARE_DEVICE_TYPE(PIC1654S, pic1654s_device)
DECLARE_DEVICE_TYPE(PIC1655,  pic1655_device)


class pic16c5x_device : public cpu_device
{
public:
	// port a, 4 or 8 bits, 2-way
	auto read_a() { return m_read_a.bind(); }
	auto write_a() { return m_write_a.bind(); }

	// port b, 8 bits, 2-way
	auto read_b() { return m_read_b.bind(); }
	auto write_b() { return m_write_b.bind(); }

	// port c, 8 bits, 2-way
	auto read_c() { return m_read_c.bind(); }
	auto write_c() { return m_write_c.bind(); }

	// port d, 8 bits, 2-way
	auto read_d() { return m_read_d.bind(); }
	auto write_d() { return m_write_d.bind(); }

	/****************************************************************************
	 *  Function to configure the CONFIG register. This is actually hard-wired
	 *  during ROM programming, so should be called in the driver INIT, with
	 *  the value if known (available in HEX dumps of the ROM).
	 */
	void set_config(u16 data);

	void ram_5(address_map &map);
	void ram_7(address_map &map);
	void rom_10(address_map &map);
	void rom_11(address_map &map);
	void rom_9(address_map &map);

protected:
	// construction/destruction
	pic16c5x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, int data_width, int picmodel);

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
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }
	virtual u32 execute_input_lines() const noexcept override { return 1; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == PIC16C5x_RTCC; }
	virtual void execute_run() override;
	virtual void execute_set_input(int line, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;

	/******************** CPU Internal Registers *******************/
	u16       m_PC;
	u16       m_PREVPC;     // previous program counter
	u8        m_W;
	u8        m_OPTION;
	u16       m_CONFIG;
	u8        m_ALU;
	u16       m_WDT;
	u8        m_TRISA;
	u8        m_TRISB;
	u8        m_TRISC;
	u16       m_STACK[2];
	u16       m_prescaler;  // Note: this is really an 8-bit register
	PAIR      m_opcode;
	u8        *m_internalram;

	int       m_icount;
	int       m_picmodel;
	int       m_data_width;
	int       m_program_width;
	int       m_delay_timer;
	u16       m_temp_config;
	int       m_rtcc;
	int       m_count_cycles;
	u8        m_data_mask;
	u16       m_program_mask;
	u8        m_status_mask;
	int       m_inst_cycles;

	memory_access<11, 1, -1, ENDIANNESS_LITTLE>::cache m_program;
	memory_access< 7, 0,  0, ENDIANNESS_LITTLE>::specific m_data;

	// i/o handlers
	devcb_read8 m_read_a;
	devcb_read8 m_read_b;
	devcb_read8 m_read_c;
	devcb_read8 m_read_d;
	devcb_write8 m_write_a;
	devcb_write8 m_write_b;
	devcb_write8 m_write_c;
	devcb_write8 m_write_d;

	// For debugger
	int m_debugger_temp;

	// opcode table entry
	typedef void (pic16c5x_device::*pic16c5x_ophandler)();
	struct pic16c5x_opcode
	{
		u8 cycles;
		pic16c5x_ophandler function;
	};
	static const pic16c5x_opcode s_opcode_main[256];
	static const pic16c5x_opcode s_opcode_00x[16];

	void update_internalram_ptr();
	void calc_zero_flag();
	void calc_add_flags(u8 augend);
	void calc_sub_flags(u8 minuend);
	u16 pop_stack();
	void push_stack(u16 data);
	void set_pc(offs_t addr);
	u8 get_regfile(offs_t addr);
	void store_regfile(offs_t addr, u8 data);
	void store_result(offs_t addr, u8 data);

	void reset_regs();
	void watchdog_reset();
	void update_watchdog(int counts);
	void update_timer(int counts);

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
};


class pic16c54_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c54_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class pic16c55_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c55_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class pic16c56_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c56_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class pic16c57_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c57_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class pic16c58_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic16c58_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class pic1650_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic1650_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class pic1654s_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic1654s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// 1654S has a /8 clock divider instead of the typical /4
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 8 - 1) / 8; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 8); }
};

class pic1655_device : public pic16c5x_device
{
public:
	// construction/destruction
	pic1655_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

#endif  // MAME_CPU_PIC16C5X_PIC16C5X_H
