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
	dmv_k210_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	uint8_t porta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t portb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t portc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void cent_ack_w(int state);
	void cent_busy_w(int state);
	void cent_slct_w(int state);
	void cent_pe_w(int state);
	void cent_fault_w(int state);
	void cent_autofd_w(int state);
	void cent_init_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr) override;

	// dmvcart_interface overrides
	virtual void io_read(address_space &space, int ifsel, offs_t offset, uint8_t &data) override;
	virtual void io_write(address_space &space, int ifsel, offs_t offset, uint8_t data) override;

private:
	required_device<i8255_device> m_ppi;
	required_device<centronics_device> m_centronics;
	required_device<input_buffer_device> m_cent_data_in;
	required_device<output_latch_device> m_cent_data_out;
	dmvcart_slot_device * m_bus;

	emu_timer * m_clk1_timer;
	uint8_t       m_portb;
	uint8_t       m_portc;
};


// device type definition
extern const device_type DMV_K210;

#endif  /* __DMV_K210_H__ */
