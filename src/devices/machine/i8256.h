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

#include "diserial.h"


class i8256_device : public device_t, public device_serial_interface
{
public:
	static constexpr flags_type emulation_flags() { return flags::SAVE_UNSUPPORTED; }

	i8256_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto inta_callback()    { return m_in_inta_cb.bind(); }
	auto int_callback()     { return m_out_int_cb.bind(); }
	auto extint_callback()  { return m_in_extint_cb.bind(); }

	auto txd_handler() { return m_txd_handler.bind(); }

	auto in_p2_callback()   { return m_in_p2_cb.bind(); }
	auto out_p2_callback()  { return m_out_p2_cb.bind(); }
	auto in_p1_callback()   { return m_in_p1_cb.bind(); }
	auto out_p1_callback()  { return m_out_p1_cb.bind(); }

	void write_rxc(int state);
	void write_rxd(int state);
	void write_cts(int state);
	void write_txc(int state);

	void write(offs_t offset, u8 data);
	uint8_t read(offs_t offset);

	uint8_t p1_r();
	void    p1_w(uint8_t data);
	uint8_t p2_r();
	void    p2_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read_line m_in_inta_cb;
	devcb_write_line m_out_int_cb;
	devcb_read_line m_in_extint_cb;

	devcb_write_line m_txd_handler;

	devcb_read8 m_in_p2_cb;
	devcb_write8 m_out_p2_cb;
	devcb_read8 m_in_p1_cb;
	devcb_write8 m_out_p1_cb;

	int32_t m_rxc;
	int32_t m_rxd;
	int32_t m_cts;
	int32_t m_txc;

	uint8_t m_command1, m_command2, m_command3;
	uint8_t m_data_bits_count;
	parity_t m_parity;
	stop_bits_t m_stop_bits;

	uint8_t m_mode;
	uint8_t m_port1_control;
	uint8_t m_interrupts, m_current_interrupt_level;
	uint8_t m_tx_buffer, m_rx_buffer;
	uint8_t m_port1_int, m_port2_int;
	uint8_t m_timers[5];
	emu_timer *m_timer;

	uint8_t m_status, m_modification;

	uint8_t m_sync_byte_count, m_rxc_count, m_txc_count;
	uint8_t m_br_factor;
	uint8_t m_rxd_bits;
	uint8_t m_rx_data, m_tx_data;
	uint8_t m_sync1, m_sync2, m_sync8, m_sync16;

	TIMER_CALLBACK_MEMBER(timer_check);

	void receive_clock();
	void sync1_rxc();
	void sync2_rxc();
	bool is_tx_enabled();
	void check_for_tx_start();
	void start_tx();
	void transmit_clock();
	void receive_character(uint8_t ch);
};

DECLARE_DEVICE_TYPE(I8256, i8256_device)

#endif // MAME_MACHINE_I8256_H
