// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
#pragma once

#ifndef __DMV_K210_H__
#define __DMV_K210_H__

#include "emu.h"
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
	dmv_k210_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

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

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr) override;

	// dmvcart_interface overrides
	virtual void io_read(address_space &space, int ifsel, offs_t offset, UINT8 &data) override;
	virtual void io_write(address_space &space, int ifsel, offs_t offset, UINT8 data) override;

private:
	required_device<i8255_device> m_ppi;
	required_device<centronics_device> m_centronics;
	required_device<input_buffer_device> m_cent_data_in;
	required_device<output_latch_device> m_cent_data_out;
	dmvcart_slot_device * m_bus;

	emu_timer * m_clk1_timer;
	UINT8       m_portb;
	UINT8       m_portc;
};


// device type definition
extern const device_type DMV_K210;

#endif  /* __DMV_K210_H__ */
