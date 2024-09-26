// license:BSD-3-Clause
// copyright-holders:Vas Crabb, Sandro Ronco, Miodrag Milanovic
/*********************************************************************

    z180csio.h

*********************************************************************/
#ifndef MAME_CPU_Z180_Z180CSIO_H
#define MAME_CPU_Z180_Z180CSIO_H

#pragma once


//**************************************************************************
//  z180csio_device
//**************************************************************************

class z180csio_device : public device_t
{
public:
	z180csio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto cks_handler() { return m_cks_cb.bind(); }
	auto txs_handler() { return m_txs_cb.bind(); }

	void cks_wr(int state);
	void rxs_wr(int state);

	u8 cntr_r();
	u8 trdr_r();
	void cntr_w(u8 data);
	void trdr_w(u8 data);

	void state_add(device_state_interface &parent);

	int  check_interrupt() { return BIT(m_cntr, 7) && BIT(m_cntr, 6); }

protected:
	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void clock_edge(u8 cks);

	TIMER_CALLBACK_MEMBER(internal_clock);

	devcb_write_line m_cks_cb;
	devcb_write_line m_txs_cb;

	emu_timer *m_internal_clock;

	u8   m_cntr;       // CSI/O Control/Status Register
	u8   m_trdr;       // CSI/O Transmit/Receive Data Register
	u8   m_shift_cnt;
	u8   m_irq;
	u8   m_cks_in;
	u8   m_rxs_in;
	u8   m_cks_out;
	u8   m_txs_out;
};


//**************************************************************************
//  DEVICE TYPE DEFINITIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(Z180CSIO, z180csio_device)

#endif // MAME_CPU_Z180_Z180CSIO_H
