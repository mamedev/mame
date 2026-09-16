// license:BSD-3-Clause
// copyright-holders:Vas Crabb
// I²C/SPI UART with 64-byte transmit and receive FIFOs
#ifndef MAME_MACHINE_SC16IS741_H
#define MAME_MACHINE_SC16IS741_H

#pragma once


class sc16is741a_device : public device_t
{
public:
	sc16is741a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~sc16is741a_device();

	auto so_cb() { return m_so_cb.bind(); }
	auto irq_cb() { return m_irq_cb.bind(); }
	auto tx_cb() { return m_tx_cb.bind(); }
	auto rts_cb() { return m_rts_cb.bind(); }

	void sclk_w(int state);
	void cs_w(int state);
	void si_w(int state);
	void rx_w(int state);
	void cts_w(int state);

protected:
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	static inline constexpr u8 FIFO_LENGTH = 64;

	enum class phase : u8;
	enum class parity : u8;
	enum interrupt : u8;

	void update_irq();
	void update_tx();
	void set_rts(u8 state);

	void reg_r(bool first);
	void reg_w();

	void iir_r(bool first);
	void lsr_r(bool first);
	void msr_r(bool first);
	void txlvl_r(bool first);
	void rxlvl_r(bool first);
	void xon_xoff_r(bool first);

	void thr_w();
	void ier_w();
	void fcr_w();
	void lcr_w();
	void mcr_w();
	void tcr_w();
	void tlr_w();
	void reserved_w();
	void uart_reset_w();
	void dl_w();
	void efr_w();
	void xon_xoff_w();

	void pop_rx_fifo();
	bool check_tx();

	u8 fifo_spaces(unsigned n) const;
	u8 fifo_fill_level(unsigned n) const;
	void fifo_reset(unsigned n);
	u8 fifo_push(unsigned n);
	u8 fifo_pop(unsigned n);

	TIMER_CALLBACK_MEMBER(rx_shift);
	TIMER_CALLBACK_MEMBER(tx_shift);
	TIMER_CALLBACK_MEMBER(rx_timeout);

	void update_trigger_levels();
	void update_data_frame();
	void update_divisor();

	devcb_write_line m_so_cb;
	devcb_write_line m_irq_cb;
	devcb_write_line m_tx_cb;
	devcb_write_line m_rts_cb;

	emu_timer *m_shift_timer[2];
	emu_timer *m_rx_timeout_timer;

	u8 m_irq, m_tx, m_rts;
	u8 m_rx, m_cts;

	u8 m_sclk, m_cs, m_si;
	phase m_phase;
	u8 m_bits, m_buffer;
	u8 m_command;

	u8 m_ier;
	u8 m_fcr;
	u8 m_lcr;
	u8 m_mcr;
	u8 m_spr;
	u8 m_tcr;
	u8 m_tlr;
	u16 m_dl;
	u8 m_efr;
	u8 m_xon_xoff[4];

	u16 m_shift_reg[2];
	u8 m_rx_remain, m_rx_count;
	u8 m_tx_remain, m_tx_count;

	u8 m_fifo_head[2], m_fifo_tail[2];
	bool m_fifo_empty[2];
	u8 m_fifo_data[3][FIFO_LENGTH];
	u8 m_fifo_errors;

	u8 m_interrupts;

	u32 m_divisor;
	u8 m_word_length;
	parity m_parity;
	u8 m_rx_intervals, m_tx_intervals;
	u8 m_rx_trigger, m_tx_trigger;
};


DECLARE_DEVICE_TYPE(SC16IS741A, sc16is741a_device)

#endif // MAME_MACHINE_SC16IS741_H
