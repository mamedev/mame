// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_EVO_TSCONF_RS232_H
#define MAME_SINCLAIR_EVO_TSCONF_RS232_H

#pragma once

#include "diserial.h"


class tsconf_rs232_device : public device_t, public device_serial_interface
{
public:
	tsconf_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto out_txd_callback() { return m_out_txd_cb.bind(); }
	auto out_rts_callback() { return m_out_rts_cb.bind(); }
	auto out_int_callback() { return m_out_int_cb.bind(); }

	void rxd_w(int state) { device_serial_interface::rx_w(state); };
	void cts_w(int state);

	// index is the high byte of the xxEF port
	u8 reg_r(offs_t offset);
	void reg_w(offs_t offset, u8 data);
	u8 dr_r();
	void dr_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
	TIMER_CALLBACK_MEMBER(systick);

	// 16550 emulation registers
	struct {
		u8 dll; //Divisor Latch LSB
		u8 dlm; //Divisor Latch MSB
		u8 ier; //Interrupt Enable
		u8 isr; //Interrupt Identification
		//u8 fcr; //FIFO Control
		u8 lcr; //Line Control
		u8 mcr; //Modem Control
		u8 lsr; //Line Status
		u8 msr; //Modem Status
		u8 scr; //Scratch Pad
	} m_regs;

	// frame format follows LCR only after the first LCR write, power-on state is 8N2
	int m_word_len;
	int m_parity;    // parity_t
	int m_stop_bits; // stop_bits_t
	u32 m_baud_applied = 0;

	devcb_write_line    m_out_txd_cb;
	devcb_write_line    m_out_rts_cb;
	devcb_write_line    m_out_int_cb;

	u8 m_select_zf;
	u8 m_zf_int_mask;
	u8 m_zf_int_src;
	u8 m_zf_api;
	u8 m_zf_err;

	u8 m_rs_txbuff[0x100];
	u8 m_rs_rxbuff[0x200];
	u8 m_rs_tx_hd, m_rs_tx_tl;
	u16 m_rs_rx_hd, m_rs_rx_tl;
	u8 m_rs_ibtr;
	u8 m_rs_itor;
	u8 m_rs_tmo_cnt;

	u8 m_zf_txbuff[0x100];
	u8 m_zf_rxbuff[0x200];
	u8 m_zf_tx_hd, m_zf_tx_tl;
	u16 m_zf_rx_hd, m_zf_rx_tl;
	u8 m_zf_ibtr;
	u8 m_zf_itor;
	u8 m_zf_tmo_cnt;

	emu_timer *m_systick_timer;

	void update_serial();
	void check_int();
	void set_format();
	u32 rs_baud() const;
	void zf_tx_w(u8 data);
	void rs_tx_w(u8 data);
	static u16 inc_rx_ptr(u16 &ptr) { const u16 tmp = ptr; ptr = (ptr + 1) & 0x01ff; return tmp; }
	static u8 inc_tx_ptr(u8 &ptr) { const u8 tmp = ptr; ptr = u8(ptr + 1); return tmp; }
	u16 zf_ifr_r() const { return (m_zf_rx_hd - m_zf_rx_tl) & 0x01ff; }
	u16 rs_ifr_r() const { return (m_rs_rx_hd - m_rs_rx_tl) & 0x01ff; }
};


DECLARE_DEVICE_TYPE(TSCONF_RS232, tsconf_rs232_device)

#endif // MAME_SINCLAIR_EVO_TSCONF_RS232_H
