// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_TSCONF_RS232_H
#define MAME_SINCLAIR_TSCONF_RS232_H

#pragma once

#include "bus/rs232/rs232.h"


class tsconf_rs232_device : public device_t, public device_serial_interface
{
public:
	tsconf_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u8 reg_r(offs_t offset);
	void reg_w(offs_t offset, u8 data);
	u8 dr_r();
	void dr_w(u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void rcv_callback() override;
	virtual void tra_callback() override;
	virtual void tra_complete() override;

private:
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

	u8 m_select_zf;
	u8 m_zf_int_mask;
	u8 m_zf_int_src;
	u8 m_zf_api;
	u8 m_zf_err;

	u8 m_rs_txbuff[0x200];
	u8 m_rs_rxbuff[0x200];
	u8 m_rs_tx_hd, m_rs_tx_tl, m_rs_rx_hd, m_rs_rx_tl;
	u8 m_rs_ibtr;
	u8 m_rs_itor;
	u8 m_rs_tmo_cnt;

	u8 m_zf_txbuff[0x200];
	u8 m_zf_rxbuff[0x200];
	u8 m_zf_tx_hd, m_zf_tx_tl, m_zf_rx_hd, m_zf_rx_tl;
	u8 m_zf_ibtr;
	u8 m_zf_itor;
	u8 m_zf_tmo_cnt;

	required_device<rs232_port_device> m_rs232;

	void update_serial(int state);
	u16 zf_ifr_r() { return (m_zf_rx_hd - m_zf_rx_tl) & 0x01ff; }
	u16 rs_ifr_r() { return (m_rs_rx_hd - m_rs_rx_tl) & 0x01ff; }
};


DECLARE_DEVICE_TYPE(TSCONF_RS232, tsconf_rs232_device)

#endif // MAME_SINCLAIR_TSCONF_RS232_H
