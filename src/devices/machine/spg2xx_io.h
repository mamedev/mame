// license:BSD-3-Clause
// copyright-holders:Ryan Holtz

#ifndef MAME_MACHINE_SPG2XX_IO_H
#define MAME_MACHINE_SPG2XX_IO_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "screen.h"

class spg2xx_io_device : public device_t
{
public:
	spg2xx_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }
	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }

	template <size_t Line> auto adc_in() { return m_adc_in[Line].bind(); }

	auto eeprom_w() { return m_eeprom_w.bind(); }
	auto eeprom_r() { return m_eeprom_r.bind(); }

	auto uart_tx() { return m_uart_tx.bind(); }

	auto chip_select() { return m_chip_sel.bind(); }

	void uart_rx(uint8_t data);

	void extint_w(int channel, bool state);

	DECLARE_READ16_MEMBER(io_r);
	DECLARE_WRITE16_MEMBER(io_w);
	
	virtual DECLARE_READ16_MEMBER(io_extended_r);
	virtual DECLARE_WRITE16_MEMBER(io_extended_w);


	auto pal_read_callback() { return m_pal_read_cb.bind(); };

	auto write_timer_irq_callback() { return m_timer_irq_cb.bind(); };
	auto write_uart_adc_irq_callback() { return m_uart_adc_irq_cb.bind(); };
	auto write_external_irq_callback() { return m_external_irq_cb.bind(); };
	auto write_ffrq_tmr1_irq_callback() { return m_ffreq_tmr1_irq_cb.bind(); };
	auto write_ffrq_tmr2_irq_callback() { return m_ffreq_tmr2_irq_cb.bind(); };

protected:
	spg2xx_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint32_t sprite_limit)
		: spg2xx_io_device(mconfig, type, tag, owner, clock)
	{
	}

	void check_extint_irq(int channel);
	void check_irqs(const uint16_t changed);

	static const device_timer_id TIMER_TMB1 = 0;
	static const device_timer_id TIMER_TMB2 = 1;

	static const device_timer_id TIMER_UART_TX = 4;
	static const device_timer_id TIMER_UART_RX = 5;
	static const device_timer_id TIMER_4KHZ = 6;
	static const device_timer_id TIMER_SRC_AB = 7;
	static const device_timer_id TIMER_SRC_C = 8;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void update_porta_special_modes();
	void update_portb_special_modes();
	void do_gpio(uint32_t offset, bool write);
	uint16_t do_special_gpio(uint32_t index, uint16_t mask);

	void update_timer_b_rate();
	void update_timer_ab_src();
	void update_timer_c_src();
	void increment_timer_a();

	void uart_transmit_tick();
	void uart_receive_tick();

	void system_timer_tick();

	void do_i2c();

	uint16_t m_io_regs[0x100];

	uint8_t m_uart_rx_fifo[8];
	uint8_t m_uart_rx_fifo_start;
	uint8_t m_uart_rx_fifo_end;
	uint8_t m_uart_rx_fifo_count;
	bool m_uart_rx_available;
	bool m_uart_rx_irq;
	bool m_uart_tx_irq;

	bool m_extint[2];

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;
	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;

	devcb_read16 m_adc_in[2];

	devcb_write8 m_eeprom_w;
	devcb_read8 m_eeprom_r;

	devcb_write8 m_uart_tx;

	devcb_write8 m_chip_sel;

	uint16_t m_timer_a_preload;
	uint16_t m_timer_b_preload;
	uint16_t m_timer_b_divisor;
	uint16_t m_timer_b_tick_rate;

	emu_timer *m_tmb1;
	emu_timer *m_tmb2;
	emu_timer *m_timer_src_ab;
	emu_timer *m_timer_src_c;

	emu_timer *m_4khz_timer;
	uint32_t m_2khz_divider;
	uint32_t m_1khz_divider;
	uint32_t m_4hz_divider;

	uint32_t m_uart_baud_rate;
	emu_timer *m_uart_tx_timer;
	emu_timer *m_uart_rx_timer;

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;

	devcb_read16 m_pal_read_cb;

	devcb_write_line m_timer_irq_cb;
	devcb_write_line m_uart_adc_irq_cb;
	devcb_write_line m_external_irq_cb;
	devcb_write_line m_ffreq_tmr1_irq_cb;
	devcb_write_line m_ffreq_tmr2_irq_cb;
};

class spg24x_io_device : public spg2xx_io_device
{
public:
	template <typename T, typename U>
	spg24x_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg24x_io_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	spg24x_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class spg28x_io_device : public spg2xx_io_device
{
public:
	template <typename T, typename U>
	spg28x_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg28x_io_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	spg28x_io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE16_MEMBER(io_extended_w) override;
};

DECLARE_DEVICE_TYPE(SPG24X_IO, spg24x_io_device)
DECLARE_DEVICE_TYPE(SPG28X_IO, spg28x_io_device)

#endif // MAME_MACHINE_SPG2XX_IO_H
