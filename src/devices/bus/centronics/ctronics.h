// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Centronics printer interface

***************************************************************************/

#pragma once

#ifndef __CTRONICS_H__
#define __CTRONICS_H__

#include "emu.h"
#include "machine/buffer.h"
#include "machine/latch.h"


#define MCFG_CENTRONICS_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, CENTRONICS, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_CENTRONICS_STROBE_HANDLER(_devcb) \
	devcb = &centronics_device::set_strobe_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_DATA0_HANDLER(_devcb) \
	devcb = &centronics_device::set_data0_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_DATA1_HANDLER(_devcb) \
	devcb = &centronics_device::set_data1_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_DATA2_HANDLER(_devcb) \
	devcb = &centronics_device::set_data2_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_DATA3_HANDLER(_devcb) \
	devcb = &centronics_device::set_data3_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_DATA4_HANDLER(_devcb) \
	devcb = &centronics_device::set_data4_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_DATA5_HANDLER(_devcb) \
	devcb = &centronics_device::set_data5_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_DATA6_HANDLER(_devcb) \
	devcb = &centronics_device::set_data6_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_DATA7_HANDLER(_devcb) \
	devcb = &centronics_device::set_data7_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_ACK_HANDLER(_devcb) \
	devcb = &centronics_device::set_ack_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_BUSY_HANDLER(_devcb) \
	devcb = &centronics_device::set_busy_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_PERROR_HANDLER(_devcb) \
	devcb = &centronics_device::set_perror_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_SELECT_HANDLER(_devcb) \
	devcb = &centronics_device::set_select_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_AUTOFD_HANDLER(_devcb) \
	devcb = &centronics_device::set_autofd_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_FAULT_HANDLER(_devcb) \
	devcb = &centronics_device::set_fault_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_INIT_HANDLER(_devcb) \
	devcb = &centronics_device::set_init_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_SELECT_IN_HANDLER(_devcb) \
	devcb = &centronics_device::set_select_in_handler(*device, DEVCB_##_devcb);

#define MCFG_CENTRONICS_OUTPUT_LATCH_ADD(_tag, _centronics_tag) \
	MCFG_DEVICE_ADD(_tag, OUTPUT_LATCH, 0) \
	MCFG_OUTPUT_LATCH_BIT0_HANDLER(DEVWRITELINE(_centronics_tag, centronics_device, write_data0)) \
	MCFG_OUTPUT_LATCH_BIT1_HANDLER(DEVWRITELINE(_centronics_tag, centronics_device, write_data1)) \
	MCFG_OUTPUT_LATCH_BIT2_HANDLER(DEVWRITELINE(_centronics_tag, centronics_device, write_data2)) \
	MCFG_OUTPUT_LATCH_BIT3_HANDLER(DEVWRITELINE(_centronics_tag, centronics_device, write_data3)) \
	MCFG_OUTPUT_LATCH_BIT4_HANDLER(DEVWRITELINE(_centronics_tag, centronics_device, write_data4)) \
	MCFG_OUTPUT_LATCH_BIT5_HANDLER(DEVWRITELINE(_centronics_tag, centronics_device, write_data5)) \
	MCFG_OUTPUT_LATCH_BIT6_HANDLER(DEVWRITELINE(_centronics_tag, centronics_device, write_data6)) \
	MCFG_OUTPUT_LATCH_BIT7_HANDLER(DEVWRITELINE(_centronics_tag, centronics_device, write_data7))

#define MCFG_CENTRONICS_DATA_INPUT_BUFFER(_tag) \
	MCFG_CENTRONICS_DATA0_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit0)) \
	MCFG_CENTRONICS_DATA1_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit1)) \
	MCFG_CENTRONICS_DATA2_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit2)) \
	MCFG_CENTRONICS_DATA3_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit3)) \
	MCFG_CENTRONICS_DATA4_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit4)) \
	MCFG_CENTRONICS_DATA5_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit5)) \
	MCFG_CENTRONICS_DATA6_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit6)) \
	MCFG_CENTRONICS_DATA7_HANDLER(DEVWRITELINE(_tag, input_buffer_device, write_bit7))

extern const device_type CENTRONICS;

class device_centronics_peripheral_interface;

class centronics_device : public device_t,
	public device_slot_interface
{
	friend class device_centronics_peripheral_interface;

public:
	centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_strobe_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_strobe_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data0_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_data0_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data1_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_data1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data2_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_data2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data3_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_data3_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data4_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_data4_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data5_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_data5_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data6_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_data6_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_data7_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_data7_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ack_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_ack_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_busy_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_busy_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_perror_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_perror_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_select_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_select_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_autofd_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_autofd_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_fault_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_fault_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_init_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_init_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_select_in_handler(device_t &device, _Object object) { return downcast<centronics_device &>(device).m_select_in_handler.set_callback(object); }

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
	device_centronics_peripheral_interface(const machine_config &mconfig, device_t &device);
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
	virtual void input_strobe(int state) {}
	virtual void input_data0(int state) {}
	virtual void input_data1(int state) {}
	virtual void input_data2(int state) {}
	virtual void input_data3(int state) {}
	virtual void input_data4(int state) {}
	virtual void input_data5(int state) {}
	virtual void input_data6(int state) {}
	virtual void input_data7(int state) {}
	virtual void input_ack(int state) {}
	virtual void input_busy(int state) {}
	virtual void input_perror(int state) {}
	virtual void input_select(int state) {}
	virtual void input_autofd(int state) {}
	virtual void input_fault(int state) {}
	virtual void input_init(int state) {}
	virtual void input_select_in(int state) {}

	centronics_device *m_slot;
};


SLOT_INTERFACE_EXTERN( centronics_devices );

#endif
