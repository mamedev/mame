// license:BSD-3-Clause
// copyright-holders:Tuth
#ifndef MAME_MACHINE_JCM_WBA_H
#define MAME_MACHINE_JCM_WBA_H

#pragma once

#include "diserial.h"

// JCM WBA bill validator with the ID-003 interface, from "ID-003 Communication Specification" rev. 05.
// 9600 bps, 8 data bits, even parity, 1 stop bit (2).
// Frames: FCh, LNG (SYNC through CRC), CMD/status, DATA, CRC-CCITT low/high (3, appendix 1).
// Power up (7-1): POWER UP until RESET, then INITIALIZE while the settings arrive, then ENABLE or DISABLE.
// Accepting (7-2): ACCEPTING, ESCROW + code, STACK-1/2 from the controller, STACKING, VEND VALID until the
// controller's ACK, STACKED, ENABLE. ENQ for the interrupt modes (6-7 (3), 6-8).
class jcm_wba_device : public device_t, public device_buffered_serial_interface<64>
{
public:
	jcm_wba_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto tx_cb() { return m_tx_cb.bind(); }
	void rx_w(int state) { device_serial_interface::rx_w(state); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void tra_callback() override { m_tx_cb(transmit_register_get_data_bit()); }

private:
	enum : u8
	{
		ST_ENABLE = 0x11, ST_ACCEPTING = 0x12, ST_ESCROW = 0x13, ST_STACKING = 0x14, ST_VEND_VALID = 0x15,
		ST_STACKED = 0x16, ST_REJECTING = 0x17, ST_RETURNING = 0x18, ST_HOLDING = 0x19, ST_DISABLE = 0x1a,
		ST_INITIALIZE = 0x1b, ST_POWER_UP = 0x40, ACK = 0x50, ENQ = 0x05, INVALID = 0x4b
	};

	virtual void received_byte(u8 byte) override;
	static u16 crc(const u8 *d, int n);
	void send(const u8 *body, int n);
	void command();
	void set_status(u8 st);
	TIMER_CALLBACK_MEMBER(step);

	devcb_write_line m_tx_cb;
	required_ioport m_bills;
	emu_timer *m_timer;
	u8 m_rx[256];
	int m_rxn;
	u8 m_status, m_escrow;
	u8 m_enable[2], m_security[2], m_comm, m_inhibit, m_direction, m_optional[2];
	u8 m_prev_bills;
};

DECLARE_DEVICE_TYPE(JCM_WBA, jcm_wba_device)

#endif // MAME_MACHINE_JCM_WBA_H
