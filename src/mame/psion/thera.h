// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC14 (THERA) peripheral

******************************************************************************/

#ifndef MAME_PSION_THERA_H
#define MAME_PSION_THERA_H

#pragma once


class thera_device : public device_t
{
public:
	// construction/destruction
	thera_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock);

	// callbacks
	auto int_cb() { return m_int_cb.bind(); }
	auto col_cb() { return m_col_cb.bind(); }
	auto spi_r() { return m_spi_r.bind(); }

	auto porta_r() { return m_port_r[0].bind(); }
	auto porta_w() { return m_port_w[0].bind(); }
	auto portb_r() { return m_port_r[1].bind(); }
	auto portb_w() { return m_port_w[1].bind(); }
	auto portc_r() { return m_port_r[2].bind(); }
	auto portc_w() { return m_port_w[2].bind(); }
	auto portd_r() { return m_port_r[3].bind(); }
	auto portd_w() { return m_port_w[3].bind(); }

	uint16_t read(offs_t offset, uint16_t mem_mask);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask);

	void write_pb0(int state) { set_pb_line(0, state); }
	void write_pb1(int state) { set_pb_line(1, state); }
	void write_pb2(int state) { set_pb_line(2, state); }
	void write_pb3(int state) { set_pb_line(3, state); }
	void write_pb4(int state) { set_pb_line(4, state); }
	void write_pb5(int state) { set_pb_line(5, state); }
	void write_pb6(int state) { set_pb_line(6, state); }
	void write_pb7(int state) { set_pb_line(7, state); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(update_fg);
	TIMER_CALLBACK_MEMBER(update_timer);

	void set_timer_ctrl(int timer, uint8_t value);
	void update_interrupts();

	void set_pb_line(int line, int state);

	devcb_write_line m_int_cb;
	devcb_write8 m_col_cb;
	devcb_read16 m_spi_r;
	devcb_read8::array<4> m_port_r;
	devcb_write8::array<4> m_port_w;

	// interrupt sources
	static constexpr int IRQ_TC1  = 0;  // Timer Counter 1 Interrupt
	static constexpr int IRQ_TC2  = 1;  // Timer Counter 2 Interrupt
	static constexpr int IRQ_RSR  = 2;  // Rx FIFO Service Request
	static constexpr int IRQ_TSR  = 3;  // Tx FIFO Service Request
	static constexpr int IRQ_EESR = 4;  // EEPROM Service Request
	static constexpr int IRQ_ROR  = 5;  // Rx FIFO Overrun
	static constexpr int IRQ_EEF  = 6;  // EEPROM Fail Interrupt
	static constexpr int IRQ_A2DF = 7;  // All AtoD Data has been received
	static constexpr int IRQ_PB0  = 8;  // Port B Inputs Interrupts
	static constexpr int IRQ_PB1  = 9;  //
	static constexpr int IRQ_PB2  = 10; //
	static constexpr int IRQ_PB3  = 11; //
	static constexpr int IRQ_PB4  = 12; //
	static constexpr int IRQ_PB5  = 13; //
	static constexpr int IRQ_PB6  = 14; //
	static constexpr int IRQ_PB7  = 15; //

	enum
	{
		PORTA,
		PORTB,
		PORTC,
		PORTD
	};

	emu_timer *m_fg[2]{};
	emu_timer *m_timer[2]{};

	uint8_t m_port_data[4];
	uint8_t m_port_ddr[4];
	uint8_t m_port_level[4];
	uint8_t m_port_run[4];

	uint16_t m_syscon;
	uint16_t m_ctrl_status;

	uint16_t m_fg_load[2]{};
	uint16_t m_fg_ctrl[2]{};
	uint16_t m_fg_value[2]{};
	int m_fg_state[2]{};

	uint16_t m_timer_load[2]{};
	uint16_t m_timer_ctrl[2]{};
	uint16_t m_timer_value[2]{};

	uint16_t m_irq_status;
	uint16_t m_irq_mask;
	uint16_t m_irq_edge;
	uint16_t m_status3;
	uint16_t m_keyb;
	uint16_t m_spi_status;
	uint16_t m_spi_data;
	uint16_t m_spi_fn;

	std::string m_machine_type = "Series 7";
	uint16_t m_prom_addr;
	uint8_t m_prom[0x80];
};


// device type definition
DECLARE_DEVICE_TYPE(THERA, thera_device)

#endif // MAME_PSION_THERA_H
