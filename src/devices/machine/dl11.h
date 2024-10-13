// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    DEC DL11-type SLU (serial line unit)

***************************************************************************/

#ifndef MAME_MACHINE_DL11_H
#define MAME_MACHINE_DL11_H

#pragma once

#include "machine/pdp11.h"
#include "machine/z80daisy.h"

#include "diserial.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> dl11_device

class dl11_device : public device_t,
					public device_serial_interface,
					public device_z80daisy_interface
{
public:
	// construction/destruction
	dl11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_rxc(int clock) { m_rxc = clock; }
	void set_txc(int clock) { m_txc = clock; }
	void set_rxvec(int vec) { m_rxvec = vec; }
	void set_txvec(int vec) { m_txvec = vec; }

	auto txd_wr_callback() { return m_write_txd.bind(); }
	auto txrdy_wr_callback() { return m_write_txrdy.bind(); }
	auto rxrdy_wr_callback() { return m_write_rxrdy.bind(); }

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	int rxrdy_r();
	int txrdy_r();

	void rx_w(int state) { device_serial_interface::rx_w(state); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	// device_z80daisy_interface overrides
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

private:
	/* registers */

	static constexpr uint16_t DLRCSR_RD      = CSR_DONE | CSR_IE;
	static constexpr uint16_t DLRCSR_WR      = CSR_IE;

	static constexpr uint16_t DLRBUF_ERR     = 0100000;
	static constexpr uint16_t DLRBUF_OVR     = 0040000;
	static constexpr uint16_t DLRBUF_RBRK    = 0020000;
	static constexpr uint16_t DLRBUF_PERR    = 0020000;

	static constexpr uint16_t DLTCSR_XBRK    = 0000001;
	static constexpr uint16_t DLTCSR_RD      = CSR_DONE | CSR_IE | DLTCSR_XBRK;
	static constexpr uint16_t DLTCSR_WR      = CSR_IE | DLTCSR_XBRK;

	devcb_write_line   m_write_txd;
	devcb_write_line   m_write_rxrdy;
	devcb_write_line   m_write_txrdy;

	line_state  m_rxrdy;
	line_state  m_txrdy;

	int m_rxc;
	int m_txc;
	int m_rxvec;
	int m_txvec;

	uint16_t m_rcsr;
	uint16_t m_rbuf;
	uint16_t m_tcsr;
	uint16_t m_tbuf;
};


// device type definition
DECLARE_DEVICE_TYPE(DL11, dl11_device)

#endif
