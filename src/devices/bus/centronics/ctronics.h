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

class centronics_device : public device_t, public device_slot_interface
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
	auto sense_handler() { return m_sense_handler.bind(); }
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

	void write_strobe(int state);
	void write_data0(int state);
	void write_data1(int state);
	void write_data2(int state);
	void write_data3(int state);
	void write_data4(int state);
	void write_data5(int state);
	void write_data6(int state);
	void write_data7(int state);
	void write_ack(int state);
	void write_busy(int state);
	void write_perror(int state);
	void write_select(int state);
	void write_autofd(int state);
	void write_fault(int state);
	void write_init(int state);
	void write_select_in(int state);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

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
	devcb_write_line m_sense_handler;
	devcb_write_line m_select_in_handler;

private:
	device_centronics_peripheral_interface *m_dev;
};


class device_centronics_peripheral_interface : public device_interface
{
	friend class centronics_device;

public:
	virtual ~device_centronics_peripheral_interface();

	void output_strobe(int state) { m_slot->m_strobe_handler(state); }
	void output_data0(int state) { m_slot->m_data0_handler(state); }
	void output_data1(int state) { m_slot->m_data1_handler(state); }
	void output_data2(int state) { m_slot->m_data2_handler(state); }
	void output_data3(int state) { m_slot->m_data3_handler(state); }
	void output_data4(int state) { m_slot->m_data4_handler(state); }
	void output_data5(int state) { m_slot->m_data5_handler(state); }
	void output_data6(int state) { m_slot->m_data6_handler(state); }
	void output_data7(int state) { m_slot->m_data7_handler(state); }
	void output_ack(int state) { m_slot->m_ack_handler(state); }
	void output_busy(int state) { m_slot->m_busy_handler(state); }
	void output_perror(int state) { m_slot->m_perror_handler(state); }
	void output_select(int state) { m_slot->m_select_handler(state); }
	void output_autofd(int state) { m_slot->m_autofd_handler(state); }
	void output_fault(int state) { m_slot->m_fault_handler(state); }
	void output_init(int state) { m_slot->m_init_handler(state); }
	void output_select_in(int state) { m_slot->m_select_in_handler(state); }

protected:
	device_centronics_peripheral_interface(const machine_config &mconfig, device_t &device);

	virtual void input_strobe(int state) { }
	virtual void input_data0(int state) { }
	virtual void input_data1(int state) { }
	virtual void input_data2(int state) { }
	virtual void input_data3(int state) { }
	virtual void input_data4(int state) { }
	virtual void input_data5(int state) { }
	virtual void input_data6(int state) { }
	virtual void input_data7(int state) { }
	virtual void input_ack(int state) { }
	virtual void input_busy(int state)  { }
	virtual void input_perror(int state) { }
	virtual void input_select(int state) { }
	virtual void input_autofd(int state) { }
	virtual void input_fault(int state) { }
	virtual void input_init(int state) { }
	virtual void input_select_in(int state) { }

	virtual bool supports_pin35_5v() { return false; }

	centronics_device *m_slot;
};


void centronics_devices(device_slot_interface &device);

#endif // MAME_BUS_CENTRONICS_CTRONICS_H
