// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
// thanks-to:rfka01
#ifndef MAME_BUS_DMV_K210_H
#define MAME_BUS_DMV_K210_H

#pragma once

#include "dmvbus.h"
#include "machine/i8255.h"
#include "bus/centronics/ctronics.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmv_k210_device

class dmv_k210_device :
		public device_t,
		public device_dmvslot_interface
{
public:
	// construction/destruction
	dmv_k210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr) override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	// dmvcart_interface overrides
	virtual void io_read(int ifsel, offs_t offset, uint8_t &data) override;
	virtual void io_write(int ifsel, offs_t offset, uint8_t data) override;

private:
	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_READ8_MEMBER(portc_r);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_WRITE8_MEMBER(portc_w);

	DECLARE_WRITE_LINE_MEMBER(cent_ack_w);
	DECLARE_WRITE_LINE_MEMBER(cent_busy_w);
	DECLARE_WRITE_LINE_MEMBER(cent_slct_w);
	DECLARE_WRITE_LINE_MEMBER(cent_pe_w);
	DECLARE_WRITE_LINE_MEMBER(cent_fault_w);
	DECLARE_WRITE_LINE_MEMBER(cent_autofd_w);
	DECLARE_WRITE_LINE_MEMBER(cent_init_w);

	required_device<i8255_device> m_ppi;
	required_device<centronics_device> m_centronics;
	required_device<input_buffer_device> m_cent_data_in;
	required_device<output_latch_device> m_cent_data_out;

	emu_timer * m_clk1_timer;
	uint8_t       m_portb;
	uint8_t       m_portc;
};


// device type definition
DECLARE_DEVICE_TYPE(DMV_K210, dmv_k210_device)

#endif  // MAME_BUS_DMV_K210_H
