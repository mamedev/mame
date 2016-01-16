// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    CD-i Mono-I SLAVE MCU simulation
    -------------------

    written by Ryan Holtz


*******************************************************************************

STATUS:

- Just enough for the Mono-I CD-i board to work somewhat properly.

TODO:

- Decapping and proper emulation.

*******************************************************************************/

#pragma once

#ifndef __CDISLAVE_H__
#define __CDISLAVE_H__

#include "emu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDISLAVE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MACHINE_CDISLAVE, 0)
#define MCFG_CDISLAVE_REPLACE(_tag) \
	MCFG_DEVICE_REPLACE(_tag, MACHINE_CDISLAVE, 0)
//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdislave_device

class cdislave_device : public device_t
{
public:
	// construction/destruction
	cdislave_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// external callbacks
	DECLARE_INPUT_CHANGED_MEMBER( mouse_update );

	UINT8* get_lcd_state() { return m_lcd_state; }

	DECLARE_READ16_MEMBER( slave_r );
	DECLARE_WRITE16_MEMBER( slave_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

	// internal callbacks
	TIMER_CALLBACK_MEMBER( trigger_readback_int );

private:
	// internal state
	class channel_state
	{
	public:
		channel_state() { }

		UINT8 m_out_buf[4];
		UINT8 m_out_index;
		UINT8 m_out_count;
		UINT8 m_out_cmd;
	};

	channel_state m_channel[4];
	emu_timer *m_interrupt_timer;

	UINT8 m_in_buf[17];
	UINT8 m_in_index;
	UINT8 m_in_count;

	UINT8 m_polling_active;

	UINT8 m_xbus_interrupt_enable;

	UINT8 m_lcd_state[16];

	UINT16 m_real_mouse_x;
	UINT16 m_real_mouse_y;

	UINT16 m_fake_mouse_x;
	UINT16 m_fake_mouse_y;

	// static internal members

	// non-static internal members
	void prepare_readback(const attotime &delay, UINT8 channel, UINT8 count, UINT8 data0, UINT8 data1, UINT8 data2, UINT8 data3, UINT8 cmd);
	void perform_mouse_update();
	void set_mouse_position();
};


// device type definition
extern const device_type MACHINE_CDISLAVE;

#endif // __CDISLAVE_H__
