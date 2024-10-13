// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    STmicro ST6-series microcontroller emulation

    TODO: Implement ST62T25 handling

    Notable ST62T25 differences:
    - No GPIO port D
    - No UART, SPI, or AR Timer
    - No bankable RAM space

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

	void porta0_w(int state);
	void porta1_w(int state);
	void porta2_w(int state);
	void porta3_w(int state);
	void porta4_w(int state);
	void porta5_w(int state);

	void portb4_w(int state);
	void portb5_w(int state);
	void portb6_w(int state);

	void portc4_w(int state);
	void portc5_w(int state);
	void portc6_w(int state);
	void portc7_w(int state);

	void portd1_w(int state);
	void portd2_w(int state);
	void portd3_w(int state);
	void portd4_w(int state);
	void portd5_w(int state);
	void portd6_w(int state);
	void portd7_w(int state);

	void timer_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// address spaces
	void st6228_program_map(address_map &map) ATTR_COLD;
	void st6228_data_map(address_map &map) ATTR_COLD;

	void unimplemented_opcode(uint8_t op);

	// Banking
	void data_rom_window_w(offs_t offset, uint8_t data);
	void rom_bank_select_w(offs_t offset, uint8_t data);
	void ram_bank_select_w(offs_t offset, uint8_t data);

	// GPIO
	void gpio_update_mode(uint8_t index, uint8_t changed);
	void gpio_set_output_bit(uint8_t index, uint8_t bit, uint8_t state);
	void gpio_data_w(offs_t offset, uint8_t data);
	void gpio_dir_w(offs_t offset, uint8_t data);
	void gpio_option_w(offs_t offset, uint8_t data);
	uint8_t gpio_data_r(offs_t offset);
	uint8_t gpio_dir_r(offs_t offset);
	uint8_t gpio_option_r(offs_t offset);

	// Timer
	void timer_counter_tick();
	void timer_prescaler_tick();
	void timer_update_count();
	void timer_prescale_w(offs_t offset, uint8_t data);
	void timer_counter_w(offs_t offset, uint8_t data);
	void timer_control_w(offs_t offset, uint8_t data);
	uint8_t timer_prescale_r(offs_t offset);
	uint8_t timer_counter_r(offs_t offset);
	uint8_t timer_status_r(offs_t offset);

	// Watchdog
	void watchdog_w(offs_t offset, uint8_t data);

	// Catch-all
	void unimplemented_reg_w(offs_t offset, uint8_t data);
	uint8_t unimplemented_reg_r(offs_t offset);

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

	enum : uint8_t
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

	enum : uint8_t
	{
		PSC_MASK                = 0x7f,

		TSCR_PS_BIT             = 0,
		TSCR_PS_MASK            = 0x07,
		TSCR_PSI_BIT            = 3,
		TSCR_DOUT_BIT           = 4,
		TSCR_TOUT_BIT           = 5,
		TSCR_ETI_BIT            = 6,
		TSCR_TMZ_BIT            = 7,

		TSCR_MODE_MASK          = (1 << TSCR_DOUT_BIT) | (1 << TSCR_TOUT_BIT),
		TSCR_MODE_EVENT         = 0x00,
		TSCR_MODE_GATED         = (1 << TSCR_DOUT_BIT),
		TSCR_MODE_OUTPUT_0      = (1 << TSCR_TOUT_BIT),
		TSCR_MODE_OUTPUT_1      = (1 << TSCR_DOUT_BIT) | (1 << TSCR_TOUT_BIT)
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

	devcb_write_line::array<6> m_porta_out;
	devcb_write_line::array<3> m_portb_out;
	devcb_write_line::array<4> m_portc_out;
	devcb_write_line::array<7> m_portd_out;

	uint8_t m_port_dir[4];
	uint8_t m_port_option[4];
	uint8_t m_port_data[4];
	uint8_t m_port_pullup[4];
	uint8_t m_port_analog[4];
	uint8_t m_port_input[4];
	uint8_t m_port_irq_enable[4];

	int m_timer_divider;
	int m_timer_pin;
	bool m_timer_active;
	devcb_write_line m_timer_out;

	address_space *m_program;
	address_space *m_data;

	required_memory_bank m_ram_bank;
	required_memory_bank m_data_bank;

	required_memory_bank m_program_rom_bank;
	required_memory_bank m_data_rom_bank;

	required_memory_region m_rom;
};

DECLARE_DEVICE_TYPE(ST6228, st6228_device)

#endif // MAME_CPU_ST62XX_H
