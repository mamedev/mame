// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    STmicro ST6-series microcontroller emulation

**********************************************************************
                            _____   _____
                   Vdd   1 |*    \_/     | 28  Vss
                 TIMER   2 |             | 27  PA0
                 OSCin   3 |             | 26  PA1
                OSCout   4 |             | 25  PA2/ARTIMout
                   NMI   5 |             | 24  PA3/ARTIMin
                   PC7   6 |             | 23  PA4
                   PC6   7 |  ST62T28C   | 22  PA5
               Ain/PC5   8 |  ST62E28C   | 21  PD1/Ain/Scl
               Ain/PC4   9 |             | 20  PD2/Ain/Sin
              TEST/Vpp  10 |             | 19  PD3/Ain/Sout
                /RESET  11 |             | 18  PD4/Ain/RXD1
               Ain/PB6  12 |             | 17  PD5/Ain/TXD1
               Ain/PB5  13 |             | 16  PD6/Ain
               Ain/PB4  14 |_____________| 15  PD7/Ain

**********************************************************************/

#ifndef MAME_CPU_ST62XX_H
#define MAME_CPU_ST62XX_H

#pragma once

class st6228_device : public cpu_device
{
public:
	st6228_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned Bit> auto port_a()
	{
		static_assert(Bit < 6, "ST6228 port A bit must be in the range 0..5\n");
		return m_porta_out[Bit].bind();
	}
	template <unsigned Bit> auto port_b()
	{
		static_assert(Bit >= 4 && Bit <= 6, "ST6228 port B bit must be in the range 4..6\n");
		return m_portb_out[Bit - 4].bind();
	}
	template <unsigned Bit> auto port_c()
	{
		static_assert(Bit >= 4 && Bit <= 7, "ST6228 port C bit must be in the range 4..7\n");
		return m_portc_out[Bit - 4].bind();
	}
	template <unsigned Bit> auto port_d()
	{
		static_assert(Bit >= 1 && Bit <= 7, "ST6228 port D bit must be in the range 1..7\n");
		return m_portd_out[Bit - 1].bind();
	}

	DECLARE_WRITE_LINE_MEMBER(porta0_w);
	DECLARE_WRITE_LINE_MEMBER(porta1_w);
	DECLARE_WRITE_LINE_MEMBER(porta2_w);
	DECLARE_WRITE_LINE_MEMBER(porta3_w);
	DECLARE_WRITE_LINE_MEMBER(porta4_w);
	DECLARE_WRITE_LINE_MEMBER(porta5_w);

	DECLARE_WRITE_LINE_MEMBER(portb4_w);
	DECLARE_WRITE_LINE_MEMBER(portb5_w);
	DECLARE_WRITE_LINE_MEMBER(portb6_w);

	DECLARE_WRITE_LINE_MEMBER(portc4_w);
	DECLARE_WRITE_LINE_MEMBER(portc5_w);
	DECLARE_WRITE_LINE_MEMBER(portc6_w);
	DECLARE_WRITE_LINE_MEMBER(portc7_w);

	DECLARE_WRITE_LINE_MEMBER(portd1_w);
	DECLARE_WRITE_LINE_MEMBER(portd2_w);
	DECLARE_WRITE_LINE_MEMBER(portd3_w);
	DECLARE_WRITE_LINE_MEMBER(portd4_w);
	DECLARE_WRITE_LINE_MEMBER(portd5_w);
	DECLARE_WRITE_LINE_MEMBER(portd6_w);
	DECLARE_WRITE_LINE_MEMBER(portd7_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override;
	virtual uint32_t execute_max_cycles() const override;
	virtual uint32_t execute_input_lines() const override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	void st6228_program_map(address_map &map);
	void st6228_data_map(address_map &map);

	void unimplemented_opcode(uint8_t op);
	void tick_timers(int cycles);
	void update_port_mode(uint8_t index, uint8_t changed);
	void set_port_output_bit(uint8_t index, uint8_t bit, uint8_t state);

	DECLARE_WRITE8_MEMBER(regs_w);
	DECLARE_READ8_MEMBER(regs_r);

	enum
	{
		STATE_FLAGS = 1,
		STATE_PC,
		STATE_SP,
		STATE_STACK0,
		STATE_STACK1,
		STATE_STACK2,
		STATE_STACK3,
		STATE_STACK4,
		STATE_STACK5,
		STATE_A,
		STATE_X,
		STATE_Y,
		STATE_V,
		STATE_W
	};

	enum
	{
		PROGRAM_ROM_START       = 0x40,
		REGS_START              = 0x80,
		REG_X                   = 0x80,
		REG_Y                   = 0x81,
		REG_V                   = 0x82,
		REG_W                   = 0x83,
		DATA_RAM_START          = 0x84,
		REG_PORTA_DATA          = 0xc0,
		REG_PORTB_DATA          = 0xc1,
		REG_PORTC_DATA          = 0xc2,
		REG_PORTD_DATA          = 0xc3,
		REG_PORTA_DIR           = 0xc4,
		REG_PORTB_DIR           = 0xc5,
		REG_PORTC_DIR           = 0xc6,
		REG_PORTD_DIR           = 0xc7,
		REG_INT_OPTION          = 0xc8,
		REG_DATA_ROM_WINDOW     = 0xc9,
		REG_ROM_BANK_SELECT     = 0xca,
		REG_RAM_BANK_SELECT     = 0xcb,
		REG_PORTA_OPTION        = 0xcc,
		REG_PORTB_OPTION        = 0xcd,
		REG_PORTC_OPTION        = 0xce,
		REG_PORTD_OPTION        = 0xcf,
		REG_AD_DATA             = 0xd0,
		REG_AD_CONTROL          = 0xd1,
		REG_TIMER_PRESCALE      = 0xd2,
		REG_TIMER_COUNT         = 0xd3,
		REG_TIMER_CONTROL       = 0xd4,
		REG_UART_DATA           = 0xd6,
		REG_UART_CONTROL        = 0xd7,
		REG_WATCHDOG            = 0xd8,
		REG_INT_POLARITY        = 0xda,
		REG_SPI_INT_DISABLE     = 0xdc,
		REG_SPI_DATA            = 0xdd,
		REG_ARTIMER_MODE        = 0xe5,
		REG_ARTIMER_ARCS0       = 0xe6,
		REG_ARTIMER_ARCS1       = 0xe7,
		REG_ARTIMER_RELOAD      = 0xe9,
		REG_ARTIMER_COMPARE     = 0xea,
		REG_ARTIMER_LOAD        = 0xeb,
		REG_A                   = 0xff
	};

	enum
	{
		MODE_NORMAL,
		MODE_IRQ,
		MODE_NMI
	};

	enum
	{
		VEC_IRQ0  = 0xff0,
		VEC_IRQ1  = 0xff2,
		VEC_IRQ2  = 0xff4,
		VEC_IRQ3  = 0xff6,
		VEC_NMI   = 0xffc,
		VEC_RESET = 0xffe
	};

	enum
	{
		FLAG_C  = 0x01,
		FLAG_Z  = 0x02
	};

	enum
	{
		PORT_A = 0,
		PORT_B,
		PORT_C,
		PORT_D
	};

	// CPU registers
	uint8_t m_regs[0x100];
	uint8_t m_ram[64*2];

	uint16_t m_pc;
	uint8_t m_mode;
	uint8_t m_prev_mode;
	uint8_t m_flags[3];

	uint16_t m_stack[6];
	uint8_t m_stack_index;

	int m_icount;

	const address_space_config m_program_config;
	const address_space_config m_data_config;

	devcb_write_line m_porta_out[6];
	devcb_write_line m_portb_out[3];
	devcb_write_line m_portc_out[4];
	devcb_write_line m_portd_out[7];

	uint8_t m_port_dir[4];
	uint8_t m_port_option[4];
	uint8_t m_port_data[4];
	uint8_t m_port_pullup[4];
	uint8_t m_port_analog[4];
	uint8_t m_port_input[4];
	uint8_t m_port_irq_enable[4];

	address_space *m_program;
	address_space *m_data;

	// FIXME: memory banks do not currently work with internal maps.
	required_memory_bank m_rambank;
	required_memory_bank m_program_rombank;
	required_memory_bank m_data_rombank;
	required_memory_region m_rom;
};

DECLARE_DEVICE_TYPE(ST6228, st6228_device)

#endif // MAME_CPU_ST62XX_H
