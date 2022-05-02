// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    z180asci.h


*********************************************************************/

#ifndef MAME_CPU_Z180_Z180ASCI_H
#define MAME_CPU_Z180_Z180ASCI_H

#pragma once

#include "diserial.h"

class z180asci_channel :  public device_t,
	public device_buffered_serial_interface<4U>
{
public:
	// construction/destruction
	z180asci_channel(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t cntla_r();
	uint8_t cntlb_r();
	uint8_t stat_r();
	uint8_t tdr_r();
	uint8_t rdr_r();
	void cntla_w(uint8_t data);
	void cntlb_w(uint8_t data);
	void stat_w(uint8_t data);
	void tdr_w(uint8_t data);
	void rdr_w(uint8_t data);

	// inline configuration
	void set_id(uint8_t id) { m_id = id; }

	auto tx_handler() { return m_tx_handler.bind(); }
	DECLARE_WRITE_LINE_MEMBER( write_rx );
	int check_interrupt();
	void clear_interrupt();
protected:
	z180asci_channel(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void received_byte(u8 byte) override;
private:
	uint8_t   m_asci_cntla;                  // ASCI control register A ch 0-1
	uint8_t   m_asci_cntlb;                  // ASCI control register B ch 0-1
	uint8_t   m_asci_stat;                   // ASCI status register 0-1
	uint8_t   m_asci_tdr;                    // ASCI transmit data register 0-1
	uint8_t   m_asci_rdr;                    // ASCI receive data register 0-1
	int       m_irq;
	int       m_id;

	devcb_write_line m_tx_handler;
};

// device type definition
DECLARE_DEVICE_TYPE(Z180ASCI_CHANNEL,   z180asci_channel)

#endif // MAME_CPU_Z180_Z180ASCI_H
