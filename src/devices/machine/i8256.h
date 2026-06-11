// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/**********************************************************************

    Intel 8256(AH) Multifunction microprocessor support controller emulation

**********************************************************************
                            _____   _____
                   AD0   1 |*    \_/     | 40  Vcc
                   AD1   2 |             | 39  P10
                   AD2   3 |             | 38  P11
                   AD3   4 |             | 37  P12
                   AD4   5 |             | 36  P13
                   DB5   6 |             | 35  P14
                   DB6   7 |             | 34  P15
                   DB7   8 |             | 33  P16
                   ALE   9 |             | 32  P17
                    RD  10 |    8256     | 31  P20
                    WR  11 |    8256AH   | 30  P21
                 RESET  12 |             | 29  P22
                    CS  13 |             | 28  P23
                  INTA  14 |             | 27  P24
                   INT  15 |             | 26  P25
                EXTINT  16 |             | 25  P26
                   CLK  17 |             | 24  P27
                   RxC  18 |             | 23  TxD
                   RxD  19 |             | 22  TxC
                   GND  20 |_____________| 21  CTS

**********************************************************************/

#ifndef MAME_MACHINE_I8256_H
#define MAME_MACHINE_I8256_H

#pragma once


class i8256_device : public device_t
{
public:
	i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_callback()     { return m_out_int_cb.bind(); }
	auto txd_handler()      { return m_txd_handler.bind(); }

	auto in_p1_callback()   { return m_in_p1_cb.bind(); }
	auto out_p1_callback()  { return m_out_p1_cb.bind(); }
	auto in_p2_callback()   { return m_in_p2_cb.bind(); }
	auto out_p2_callback()  { return m_out_p2_cb.bind(); }

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

	// interrupt acknowledge (RST n instruction in 8085 mode, vector 40H-47H in 8086 mode)
	uint8_t inta_r();

	void write_rxd(int state);
	void write_rxc(int state);
	void write_txc(int state);
	void write_cts(int state);
	void write_extint(int state);

	uint8_t p1_r();
	void    p1_w(uint8_t data);
	uint8_t p2_r();
	void    p2_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(timer_check);
	TIMER_CALLBACK_MEMBER(brg_tick);

	void soft_reset();

	void receiver_tick();
	void transmitter_tick();
	uint16_t rx_sample_point() const;
	uint16_t stop_length() const;
	void output_txd(int state);

	void request_interrupt(int level);
	int acknowledge();
	void update_int();

	devcb_write_line m_out_int_cb;
	devcb_write_line m_txd_handler;

	devcb_read8 m_in_p1_cb;
	devcb_write8 m_out_p1_cb;
	devcb_read8 m_in_p2_cb;
	devcb_write8 m_out_p2_cb;

	emu_timer *m_timer;
	emu_timer *m_brg_timer;

	// registers
	uint8_t m_command1, m_command2, m_command3;
	uint8_t m_mode;
	uint8_t m_port1_control;
	uint8_t m_modification;
	uint8_t m_int_enable, m_int_request;
	uint8_t m_status;
	uint8_t m_rx_buffer, m_tx_buffer;
	uint8_t m_port1_int, m_port2_int;
	uint8_t m_timers[5];

	// serial frame settings
	uint8_t m_data_bits;
	bool m_parity_enable, m_parity_even;
	uint8_t m_stop_sel;
	u16 m_divide;
	uint8_t m_rx_sample;

	// input line states
	uint8_t m_rxd, m_cts, m_extint;
	uint8_t m_rxc, m_txc;

	// interrupt output
	bool m_int_state;

	// receiver
	uint8_t m_rx_state;
	u16 m_rx_counter;
	uint8_t m_rx_bits;
	u16 m_rx_shift;
	bool m_rx_parity;

	// transmitter
	uint8_t m_tx_state;
	u16 m_tx_counter;
	uint8_t m_tx_bits;
	uint8_t m_tx_shift;
	bool m_tx_parity;
	bool m_tx_break;
	uint8_t m_txd;
};

DECLARE_DEVICE_TYPE(I8256, i8256_device)

#endif // MAME_MACHINE_I8256_H
