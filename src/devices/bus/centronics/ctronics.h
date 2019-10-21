// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Centronics printer interface

***************************************************************************/
#ifndef MAME_BUS_CENTRONICS_CTRONICS_H
#define MAME_BUS_CENTRONICS_CTRONICS_H

#pragma once

#include "machine/buffer.h"
#include "machine/output_latch.h"


DECLARE_DEVICE_TYPE(CENTRONICS, centronics_device)

class device_centronics_peripheral_interface;

class centronics_device : public device_t,
	public device_slot_interface
{
	friend class device_centronics_peripheral_interface;

public:
	template <typename T>
	centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: centronics_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto strobe_handler() { return m_strobe_handler.bind(); }

	auto ack_handler() { return m_ack_handler.bind(); }
	auto busy_handler() { return m_busy_handler.bind(); }
	auto perror_handler() { return m_perror_handler.bind(); }
	auto select_handler() { return m_select_handler.bind(); }
	auto autofd_handler() { return m_autofd_handler.bind(); }
	auto fault_handler() { return m_fault_handler.bind(); }
	auto init_handler() { return m_init_handler.bind(); }
	auto select_in_handler() { return m_select_in_handler.bind(); }

	template <typename T> void set_data_input_buffer(T &&tag)
	{
		m_data0_handler.bind().set(tag, FUNC(input_buffer_device::write_bit0));
		m_data1_handler.bind().set(tag, FUNC(input_buffer_device::write_bit1));
		m_data2_handler.bind().set(tag, FUNC(input_buffer_device::write_bit2));
		m_data3_handler.bind().set(tag, FUNC(input_buffer_device::write_bit3));
		m_data4_handler.bind().set(tag, FUNC(input_buffer_device::write_bit4));
		m_data5_handler.bind().set(tag, FUNC(input_buffer_device::write_bit5));
		m_data6_handler.bind().set(tag, FUNC(input_buffer_device::write_bit6));
		m_data7_handler.bind().set(tag, FUNC(input_buffer_device::write_bit7));
	}

	void set_output_latch(output_latch_device &latch);

	DECLARE_WRITE_LINE_MEMBER( write_strobe );
	DECLARE_WRITE_LINE_MEMBER( write_data0 );
	DECLARE_WRITE_LINE_MEMBER( write_data1 );
	DECLARE_WRITE_LINE_MEMBER( write_data2 );
	DECLARE_WRITE_LINE_MEMBER( write_data3 );
	DECLARE_WRITE_LINE_MEMBER( write_data4 );
	DECLARE_WRITE_LINE_MEMBER( write_data5 );
	DECLARE_WRITE_LINE_MEMBER( write_data6 );
	DECLARE_WRITE_LINE_MEMBER( write_data7 );
	DECLARE_WRITE_LINE_MEMBER( write_ack );
	DECLARE_WRITE_LINE_MEMBER( write_busy );
	DECLARE_WRITE_LINE_MEMBER( write_perror );
	DECLARE_WRITE_LINE_MEMBER( write_select );
	DECLARE_WRITE_LINE_MEMBER( write_autofd );
	DECLARE_WRITE_LINE_MEMBER( write_fault );
	DECLARE_WRITE_LINE_MEMBER( write_init );
	DECLARE_WRITE_LINE_MEMBER( write_select_in );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	devcb_write_line m_strobe_handler;
	devcb_write_line m_data0_handler;
	devcb_write_line m_data1_handler;
	devcb_write_line m_data2_handler;
	devcb_write_line m_data3_handler;
	devcb_write_line m_data4_handler;
	devcb_write_line m_data5_handler;
	devcb_write_line m_data6_handler;
	devcb_write_line m_data7_handler;
	devcb_write_line m_ack_handler;
	devcb_write_line m_busy_handler;
	devcb_write_line m_perror_handler;
	devcb_write_line m_select_handler;
	devcb_write_line m_autofd_handler;
	devcb_write_line m_fault_handler;
	devcb_write_line m_init_handler;
	devcb_write_line m_select_in_handler;

private:
	device_centronics_peripheral_interface *m_dev;
};


class device_centronics_peripheral_interface : public device_slot_card_interface
{
	friend class centronics_device;

public:
	virtual ~device_centronics_peripheral_interface();

	DECLARE_WRITE_LINE_MEMBER( output_strobe ) { m_slot->m_strobe_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_data0 ) { m_slot->m_data0_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_data1 ) { m_slot->m_data1_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_data2 ) { m_slot->m_data2_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_data3 ) { m_slot->m_data3_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_data4 ) { m_slot->m_data4_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_data5 ) { m_slot->m_data5_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_data6 ) { m_slot->m_data6_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_data7 ) { m_slot->m_data7_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_ack ) { m_slot->m_ack_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_busy ) { m_slot->m_busy_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_perror ) { m_slot->m_perror_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_select ) { m_slot->m_select_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_autofd ) { m_slot->m_autofd_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_fault ) { m_slot->m_fault_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_init ) { m_slot->m_init_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_select_in ) { m_slot->m_select_in_handler(state); }

protected:
	device_centronics_peripheral_interface(const machine_config &mconfig, device_t &device);

	virtual DECLARE_WRITE_LINE_MEMBER( input_strobe ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data7 ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_ack ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_busy )  { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_perror ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_select ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_autofd ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_fault ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_init ) { }
	virtual DECLARE_WRITE_LINE_MEMBER( input_select_in ) { }

	centronics_device *m_slot;
};


void centronics_devices(device_slot_interface &device);

#endif // MAME_BUS_CENTRONICS_CTRONICS_H
