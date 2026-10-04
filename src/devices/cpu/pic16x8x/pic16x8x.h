// license: BSD-3-Clause
// copyright-holders: Tony La Porta, Grull Osgo, Dirk Best
/************************************************************************

    Microchip PIC16 Mid-Range Devices

************************************************************************/

#ifndef MAME_CPU_PIC16X8X_PIC16X8X_H
#define MAME_CPU_PIC16X8X_PIC16X8X_H

#pragma once

enum
{
	PIC16X8x_PC = 1,
	PIC16X8x_W,
	PIC16X8x_ALU,
	PIC16X8x_PSCL,
	PIC16X8x_CONFIG
};

// input lines
enum
{
	PIC16x8x_T0CKI = 0,
	PIC16x8x_RB0INT
};

DECLARE_DEVICE_TYPE(PIC16CR83,  pic16cr83_device)
DECLARE_DEVICE_TYPE(PIC16CR84,  pic16cr84_device)
DECLARE_DEVICE_TYPE(PIC16F83,   pic16f83_device)
DECLARE_DEVICE_TYPE(PIC16F84,   pic16f84_device)
DECLARE_DEVICE_TYPE(PIC16F84A,  pic16f84a_device)
DECLARE_DEVICE_TYPE(PIC16F628A, pic16f628a_device)

class pic16x8x_device : public cpu_device, public device_nvram_interface
{
	enum
	{
		PORTA = 0,
		PORTB
	};

public:
	// port a, 5 or 8 bits, 2-way
	auto read_a() { return m_read_port[PORTA].bind(); }
	auto write_a() { return m_write_port[PORTA].bind(); }

	// port b, 8 bits, 2-way
	auto read_b() { return m_read_port[PORTB].bind(); }
	auto write_b() { return m_write_port[PORTB].bind(); }

	void rom_9(address_map &map) ATTR_COLD;
	void rom_10(address_map &map) ATTR_COLD;
	void rom_11(address_map &map) ATTR_COLD;
	void rom_12(address_map &map) ATTR_COLD;

	void core_regs(address_map &map, u8 mirror = 0) ATTR_COLD;
	void ram_6(address_map &map) ATTR_COLD;
	void ram_7(address_map &map) ATTR_COLD;

protected:
	pic16x8x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int program_width, address_map_constructor program_map, address_map_constructor data_map, u16 eeprom_size, u8 status_mask, u8 porta_mask);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 2; }
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

	// special function register
	u8 m_INTCON;

	// special function register access functions
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

	virtual void set_eeif();
	virtual bool irq_active() const;
	virtual void wdt_reset();

private:
	enum : u8
	{
		EEPROM_LOCKED,
		EEPROM_55_WRITTEN,
		EEPROM_AA_WRITTEN,
	};

	int m_program_width;

	address_space_config m_program_config;
	address_space_config m_data_config;

	memory_access<13, 1, -1, ENDIANNESS_LITTLE>::cache m_program;
	memory_access< 8, 0,  0, ENDIANNESS_LITTLE>::specific m_data;

	optional_memory_region m_region;

	/******************** CPU Internal Registers *******************/
	u16     m_PC;
	u16     m_PREVPC;
	u8      m_W;
	u8      m_OPTION;
	u16     m_CONFIG;
	u8      m_ALU;
	u8      m_TMR0;
	u8      m_STATUS;
	u8      m_FSR;
	u8      m_EEDATA;
	u8      m_EEADR;
	u8      m_PCLATH;
	u8      m_EECON1;
	u8      m_port_data[2];
	u8      m_port_tris[2];
	u8      m_porta_mask;
	u16     m_STACK[8];
	u16     m_prescaler;  // Note: this is really an 8-bit register
	PAIR16  m_opcode;
	int     m_icount;
	int     m_delay_timer;
	int     m_rtcc;
	u8      m_count_cycles;
	u16     m_program_mask;
	const u8 m_status_mask;
	u8      m_status_write_protect;
	u8      m_inst_cycles;
	u8      m_stack_pointer;
	u8      m_rb0; // rb0 edge detection
	u8      m_portb_mismatch_detect;

	bool m_sleeping;

	emu_timer *m_wdt_timer;

	std::unique_ptr<u8[]> m_eeprom_data;
	const u16 m_internal_eeprom_size;
	u8 m_eeprom_unlock_state;

	// i/o handlers
	devcb_read8::array<2> m_read_port;
	devcb_write8::array<2> m_write_port;

	// For debugger
	int m_debugger_temp;

	// opcode table entry
	typedef void (pic16x8x_device::*pic16x8x_ophandler)();
	struct pic16x8x_opcode
	{
		pic16x8x_ophandler function;
		u8 cycles;
		bool affects_alu_flags;
	};

	static const pic16x8x_opcode s_opcode_main[128];
	static const pic16x8x_opcode s_opcode_00x[128];

	void update_timer(int counts);
	void check_irqs();

	// watchdog
	TIMER_CALLBACK_MEMBER(wdt_timeout);
	void restart_wdt();

	// EEPROM
	u8 eeprom_read(offs_t offs);
	void eeprom_write(offs_t offs, u8 data);

	// helper functions
	offs_t addr() const;
	u8 bit_pos() const { return (m_opcode.w >> 7) & 0x07; }

	void calc_zero_flag();
	void calc_add_flags(u8 augend);
	void calc_sub_flags(u8 minuend);

	u16 pop_stack();
	void push_stack(u16 data);
	void set_pc(u16 addr);

	u8 get_regfile(offs_t offset);
	void store_regfile(offs_t offset, u8 data);
	void store_result(offs_t offset, u8 data);

	// instructions
	void op_illegal();
	void op_addlw();
	void op_addwf();
	void op_andwf();
	void op_andlw();
	void op_bcf();
	void op_bsf();
	void op_btfss();
	void op_btfsc();
	void op_call();
	void op_clrw();
	void op_clrf();
	void op_clrwdt();
	void op_comf();
	void op_decf();
	void op_decfsz();
	void op_goto();
	void op_incf();
	void op_incfsz();
	void op_iorlw();
	void op_iorwf();
	void op_movf();
	void op_movlw();
	void op_movwf();
	void op_nop();
	void op_option(); // deprecated, but still supported
	void op_retfie();
	void op_retlw();
	void op_return();
	void op_rlf();
	void op_rrf();
	void op_sleep();
	void op_sublw();
	void op_subwf();
	void op_swapf();
	void op_tris(); // deprecated, but still supported
	void op_xorlw();
	void op_xorwf();
};

class pic16x83_device : public pic16x8x_device
{
public:
	pic16x83_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock); //, int program_width);
};

class pic16x84_device : public pic16x8x_device
{
public:
	pic16x84_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock); //, int program_width);
};

class pic16cr83_device : public pic16x83_device
{
public:
	pic16cr83_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class pic16cr84_device : public pic16x84_device
{
public:
	pic16cr84_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class  pic16f83_device : public pic16x83_device
{
public:
	 pic16f83_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class pic16f84_device : public pic16x84_device
{
public:
	pic16f84_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class pic16f84a_device : public pic16x84_device
{
public:
	pic16f84a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class pic16f628a_device : public pic16x8x_device
{
public:
	pic16f628a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void set_eeif() override;
	virtual bool irq_active() const override;
	virtual void wdt_reset() override;

private:
	void data_map(address_map &map);

	// register file functions
	u8 pir1_r();
	void pir1_w(u8 data);
	u8 tmr1l_r();
	void tmr1l_w(u8 data);
	u8 tmr1h_r();
	void tmr1h_w(u8 data);
	u8 t1con_r();
	void t1con_w(u8 data);
	u8 tmr2_r();
	void tmr2_w(u8 data);
	u8 t2con_r();
	void t2con_w(u8 data);
	u8 ccpr1l_r();
	void ccpr1l_w(u8 data);
	u8 ccpr1h_r();
	void ccpr1h_w(u8 data);
	u8 ccp1con_r();
	void ccp1con_w(u8 data);
	u8 rcsta_r();
	void rcsta_w(u8 data);
	u8 txreg_r();
	void txreg_w(u8 data);
	u8 rcreg_r();
	void rcreg_w(u8 data);
	u8 cmcon_r();
	void cmcon_w(u8 data);
	u8 pie1_r();
	void pie1_w(u8 data);
	u8 pcon_r();
	void pcon_w(u8 data);
	u8 pr2_r();
	void pr2_w(u8 data);
	u8 txsta_r();
	void txsta_w(u8 data);
	u8 spbrg_r();
	void spbrg_w(u8 data);
	u8 vrcon_r();
	void vrcon_w(u8 data);

	u8 m_PIR1;
	u8 m_T1CON;
	u8 m_T2CON;
	u8 m_CCP1CON;
	u8 m_RCSTA;
	u8 m_CMCON;
	u8 m_PIE1;
	u8 m_PCON;
	u8 m_TXSTA;
	u8 m_VRCON;
};

#endif  // MAME_CPU_PIC16X8X_PIC16X8X_H
