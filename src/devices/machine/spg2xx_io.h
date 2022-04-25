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

	auto i2c_w() { return m_i2c_w.bind(); }
	auto i2c_r() { return m_i2c_r.bind(); }

	auto uart_tx() { return m_uart_tx.bind(); }
	auto spi_tx() { return m_spi_tx.bind(); }
	auto chip_select() { return m_chip_sel.bind(); }

	void uart_rx(uint8_t data);
	void spi_rx(int state);

	void extint_w(int channel, bool state);

	uint16_t io_r(offs_t offset);
	void io_w(offs_t offset, uint16_t data);

	virtual uint16_t io_extended_r(offs_t offset);
	virtual void io_extended_w(offs_t offset, uint16_t data);

	auto pal_read_callback() { return m_pal_read_cb.bind(); }

	auto write_timer_irq_callback() { return m_timer_irq_cb.bind(); }
	auto write_uart_adc_irq_callback() { return m_uart_adc_irq_cb.bind(); }
	auto write_external_irq_callback() { return m_external_irq_cb.bind(); }
	auto write_ffrq_tmr1_irq_callback() { return m_ffreq_tmr1_irq_cb.bind(); }
	auto write_ffrq_tmr2_irq_callback() { return m_ffreq_tmr2_irq_cb.bind(); }

	auto write_fiq_vector_callback() { return m_fiq_vector_w.bind(); }

	template <size_t Line> uint16_t adc_r() { return m_adc_in[Line](); }

protected:
	spg2xx_io_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint32_t sprite_limit)
		: spg2xx_io_device(mconfig, type, tag, owner, clock)
	{
	}

	enum
	{
		REG_IO_MODE,
		REG_IOA_DATA,
		REG_IOA_BUFFER,
		REG_IOA_DIR,
		REG_IOA_ATTRIB,
		REG_IOA_MASK,
		REG_IOB_DATA,
		REG_IOB_BUFFER,
		REG_IOB_DIR,
		REG_IOB_ATTRIB,
		REG_IOB_MASK,
		REG_IOC_DATA,
		REG_IOC_BUFFER,
		REG_IOC_DIR,
		REG_IOC_ATTRIB,
		REG_IOC_MASK,

		REG_TIMEBASE_SETUP,
		REG_TIMEBASE_CLEAR,
		REG_TIMERA_DATA,
		REG_TIMERA_CTRL,
		REG_TIMERA_ON,
		REG_TIMERA_IRQCLR,
		REG_TIMERB_DATA,
		REG_TIMERB_CTRL,
		REG_TIMERB_ON,
		REG_TIMERB_IRQCLR,

		REG_VERT_LINE = 0x1c,

		REG_SYSTEM_CTRL = 0x20,
		REG_INT_CTRL,
		REG_INT_CLEAR,
		REG_EXT_MEMORY_CTRL,
		REG_WATCHDOG_CLEAR,
		REG_ADC_CTRL,
		REG_ADC_PAD,
		REG_ADC_DATA,

		REG_SLEEP_MODE,
		REG_WAKEUP_SOURCE,
		REG_WAKEUP_TIME,

		REG_NTSC_PAL,

		REG_PRNG1 = 0x2c,
		REG_PRNG2,

		REG_FIQ_SEL,
		REG_DATA_SEGMENT,

		REG_UART_CTRL,
		REG_UART_STATUS,
		REG_UART_RESET,
		REG_UART_BAUD1,
		REG_UART_BAUD2,
		REG_UART_TXBUF,
		REG_UART_RXBUF,
		REG_UART_RXFIFO,

		REG_SPI_CTRL = 0x40,
		REG_SPI_TXSTATUS,
		REG_SPI_TXDATA,
		REG_SPI_RXSTATUS,
		REG_SPI_RXDATA,
		REG_SPI_MISC,

		REG_SIO_SETUP = 0x50,
		REG_SIO_STATUS,
		REG_SIO_ADDRL,
		REG_SIO_ADDRH,
		REG_SIO_DATA,
		REG_SIO_AUTO_TX_NUM,

		REG_I2C_CMD = 0x58,
		REG_I2C_STATUS,
		REG_I2C_ACCESS,
		REG_I2C_ADDR,
		REG_I2C_SUBADDR,
		REG_I2C_DATA_OUT,
		REG_I2C_DATA_IN,
		REG_I2C_MODE,

		REG_REGULATOR_CTRL = 0x60,
		REG_CLOCK_CTRL,
		REG_IO_DRIVE_CTRL
	};
	void check_extint_irq(int channel);
	void check_timers_irq();
	void check_data_irq();
	void check_external_irq();
	void check_hifreq_periodic_irq();
	void check_tmb_lofreq_key_irq();
	void check_all_irqs(const uint16_t changed);

	virtual void device_start() override;
	virtual void device_reset() override;

	TIMER_CALLBACK_MEMBER(timer_ab_tick);
	TIMER_CALLBACK_MEMBER(timer_c_tick);
	template <int Which> TIMER_CALLBACK_MEMBER(tmb_timer_tick);
	TIMER_CALLBACK_MEMBER(uart_transmit_tick);
	TIMER_CALLBACK_MEMBER(uart_receive_tick);
	TIMER_CALLBACK_MEMBER(system_timer_tick);
	TIMER_CALLBACK_MEMBER(rng_clock_tick);
	TIMER_CALLBACK_MEMBER(watchdog_tick);
	TIMER_CALLBACK_MEMBER(spi_tx_tick);
	template <int Which> TIMER_CALLBACK_MEMBER(adc_convert_tick);

	uint16_t clock_rng(int which);

	void update_porta_special_modes();
	void update_portb_special_modes();
	void do_gpio(uint32_t offset, bool write);
	uint16_t do_special_gpio(uint32_t index, uint16_t mask);

	void update_timer_b_rate();
	void increment_timer_a();

	void set_spi_irq(bool set);
	void update_spi_irqs();

	void do_i2c();

	uint16_t m_io_regs[0x100];

	uint8_t m_uart_rx_fifo[8];
	uint8_t m_uart_rx_fifo_start;
	uint8_t m_uart_rx_fifo_end;
	uint8_t m_uart_rx_fifo_count;
	bool m_uart_rx_available;
	bool m_uart_rx_irq;
	bool m_uart_tx_irq;

	uint8_t m_spi_tx_fifo[16];
	uint8_t m_spi_tx_fifo_start;
	uint8_t m_spi_tx_fifo_end;
	uint8_t m_spi_tx_fifo_count;
	uint8_t m_spi_tx_buf;
	uint8_t m_spi_tx_bit;

	uint8_t m_spi_rx_fifo[16];
	uint8_t m_spi_rx_fifo_start;
	uint8_t m_spi_rx_fifo_end;
	uint8_t m_spi_rx_fifo_count;
	uint8_t m_spi_rx_buf;
	uint8_t m_spi_rx_bit;

	bool m_extint[2];

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;
	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;

	devcb_read16::array<4> m_adc_in;
	emu_timer *m_adc_timer[4];

	devcb_write8 m_i2c_w;
	devcb_read8 m_i2c_r;

	devcb_write8 m_uart_tx;
	devcb_write_line m_spi_tx;

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

	emu_timer *m_rng_timer;
	emu_timer *m_watchdog_timer;

	uint32_t m_spi_rate;
	emu_timer* m_spi_tx_timer;

	uint8_t m_sio_bits_remaining;
	bool m_sio_writing;

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;

	devcb_read16 m_pal_read_cb;

	devcb_write_line m_timer_irq_cb;
	devcb_write_line m_uart_adc_irq_cb;
	devcb_write_line m_external_irq_cb;
	devcb_write_line m_ffreq_tmr1_irq_cb;
	devcb_write_line m_ffreq_tmr2_irq_cb;

	devcb_write8 m_fiq_vector_w;
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

	virtual void io_extended_w(offs_t offset, uint16_t data) override;
};

DECLARE_DEVICE_TYPE(SPG24X_IO, spg24x_io_device)
DECLARE_DEVICE_TYPE(SPG28X_IO, spg28x_io_device)

#endif // MAME_MACHINE_SPG2XX_IO_H
