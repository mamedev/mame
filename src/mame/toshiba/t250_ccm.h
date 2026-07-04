// license:BSD-3-Clause
// copyright-holders:Dave L. Rand
#ifndef MAME_TOSHIBA_T250_CCM_H
#define MAME_TOSHIBA_T250_CCM_H

#pragma once

#include "bus/rs232/rs232.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"


// ============================================================================
//  t250_ccm_device — Comms Control Module (CCM) board.
//
//  A separate PCB carrying the Intel 8251 USART, the Intel 8253 PIT (counter 2
//  generates the baud clock for both TxC and RxC), and the RS-232 connector.
//  Standard on every T-200/T-250 but lives on its own card edge, so it is
//  encapsulated as a device.  The RS-232 connector physically sits on this
//  card edge, so the port is a sub-device of the CCM: address it on the
//  command line as -ccm:rs232.
// ============================================================================

class t250_ccm_device : public device_t
{
public:
	t250_ccm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto rxrdy_handler() { return m_rxrdy_cb.bind(); }
	auto txrdy_handler() { return m_txrdy_cb.bind(); }

	u8   pit_r(offs_t offset)            { return m_pit->read(offset); }
	void pit_w(offs_t offset, u8 data)   { m_pit->write(offset, data); }
	u8   usart_r(offs_t offset)          { return m_usart->read(offset); }
	void usart_w(offs_t offset, u8 data);   // 0xA4/A5 — first-mode-byte quirk
	void mode_w(u8 data);                   // 0xA6 — CCM mode register
	u8   cicts_r();                         // 0xA7 — CI/CTS status

	bool rxrdy_state() const { return m_rxrdy_state; }
	bool txrdy_state() const { return m_txrdy_state; }
	bool cts_r() const       { return m_cts; }
	bool ci_r()  const       { return m_ri;  }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void usart_rxrdy_w(int state);
	void usart_txrdy_w(int state);
	void rs232_cts_w(int state);
	void rs232_ri_w(int state);

	required_device<pit8253_device>    m_pit;
	required_device<i8251_device>      m_usart;
	required_device<rs232_port_device> m_rs232;

	devcb_write_line m_rxrdy_cb;
	devcb_write_line m_txrdy_cb;

	bool m_rxrdy_state;
	bool m_txrdy_state;
	bool m_usart_inited;
	bool m_cts;
	bool m_ri;
};

DECLARE_DEVICE_TYPE(T250_CCM, t250_ccm_device)

#endif // MAME_TOSHIBA_T250_CCM_H
