// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    IBM PC AT compatibles 8042 keyboard controller

***************************************************************************/

#pragma once

#ifndef __AT_KEYBC_H__
#define __AT_KEYBC_H__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AT_KEYBOARD_CONTROLLER_SYSTEM_RESET_CB(_devcb) \
	devcb = &at_keyboard_controller_device::set_system_reset_callback(*device, DEVCB_##_devcb);

#define MCFG_AT_KEYBOARD_CONTROLLER_GATE_A20_CB(_devcb) \
	devcb = &at_keyboard_controller_device::set_gate_a20_callback(*device, DEVCB_##_devcb);

#define MCFG_AT_KEYBOARD_CONTROLLER_INPUT_BUFFER_FULL_CB(_devcb) \
	devcb = &at_keyboard_controller_device::set_input_buffer_full_callback(*device, DEVCB_##_devcb);

#define MCFG_AT_KEYBOARD_CONTROLLER_OUTPUT_BUFFER_EMPTY_CB(_devcb) \
	devcb = &at_keyboard_controller_device::set_output_buffer_empty_callback(*device, DEVCB_##_devcb);

#define MCFG_AT_KEYBOARD_CONTROLLER_KEYBOARD_CLOCK_CB(_devcb) \
	devcb = &at_keyboard_controller_device::set_keyboard_clock_callback(*device, DEVCB_##_devcb);

#define MCFG_AT_KEYBOARD_CONTROLLER_KEYBOARD_DATA_CB(_devcb) \
	devcb = &at_keyboard_controller_device::set_keyboard_data_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> at_keyboard_controller_device

class at_keyboard_controller_device : public device_t
{
public:
	// construction/destruction
	at_keyboard_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_system_reset_callback(device_t &device, _Object object) { return downcast<at_keyboard_controller_device &>(device).m_system_reset_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_gate_a20_callback(device_t &device, _Object object) { return downcast<at_keyboard_controller_device &>(device).m_gate_a20_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_input_buffer_full_callback(device_t &device, _Object object) { return downcast<at_keyboard_controller_device &>(device).m_input_buffer_full_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_output_buffer_empty_callback(device_t &device, _Object object) { return downcast<at_keyboard_controller_device &>(device).m_output_buffer_empty_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_keyboard_clock_callback(device_t &device, _Object object) { return downcast<at_keyboard_controller_device &>(device).m_keyboard_clock_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_keyboard_data_callback(device_t &device, _Object object) { return downcast<at_keyboard_controller_device &>(device).m_keyboard_data_cb.set_callback(object); }

	// internal 8042 interface
	DECLARE_READ8_MEMBER( t0_r );
	DECLARE_READ8_MEMBER( t1_r );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );

	// interface to the host pc
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( command_w );

	// interface to the keyboard
	DECLARE_WRITE_LINE_MEMBER( keyboard_clock_w );
	DECLARE_WRITE_LINE_MEMBER( keyboard_data_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual const rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	// internal state
	upi41_cpu_device *m_cpu;

	// interface to the host pc
	devcb_write_line    m_system_reset_cb;
	devcb_write_line    m_gate_a20_cb;
	devcb_write_line    m_input_buffer_full_cb;
	devcb_write_line    m_output_buffer_empty_cb;

	// interface to the keyboard
	devcb_write_line    m_keyboard_clock_cb;
	devcb_write_line    m_keyboard_data_cb;

	UINT8 m_clock_signal;
	UINT8 m_data_signal;
};


// device type definition
extern const device_type AT_KEYBOARD_CONTROLLER;


#endif  /* __AT_KEYBC__ */
