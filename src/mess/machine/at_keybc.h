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

#define MCFG_AT_KEYBOARD_CONTROLLER_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, AT_KEYBOARD_CONTROLLER, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> at_keyboard_controller_interface

struct at_keyboard_controller_interface
{
	// interface to the host pc
	devcb_write_line    m_system_reset_cb;
	devcb_write_line    m_gate_a20_cb;
	devcb_write_line    m_input_buffer_full_cb;
	devcb_write_line    m_output_buffer_empty_cb;

	// interface to the keyboard
	devcb_write_line    m_keyboard_clock_cb;
	devcb_write_line    m_keyboard_data_cb;
};

// ======================> at_keyboard_controller_device

class at_keyboard_controller_device : public device_t,
										public at_keyboard_controller_interface
{
public:
	// construction/destruction
	at_keyboard_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
	// internal state
	upi41_cpu_device *m_cpu;

	devcb_resolved_write_line   m_system_reset_func;
	devcb_resolved_write_line   m_gate_a20_func;
	devcb_resolved_write_line   m_input_buffer_full_func;
	devcb_resolved_write_line   m_output_buffer_empty_func;
	devcb_resolved_write_line   m_keyboard_clock_func;
	devcb_resolved_write_line   m_keyboard_data_func;

	UINT8 m_clock_signal;
	UINT8 m_data_signal;
};


// device type definition
extern const device_type AT_KEYBOARD_CONTROLLER;


#endif  /* __AT_KEYBC__ */
