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

#ifndef MAME_MACHINE_CDISLAVE_H
#define MAME_MACHINE_CDISLAVE_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDISLAVE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CDISLAVE, 0)
#define MCFG_CDISLAVE_REPLACE(_tag) \
	MCFG_DEVICE_REPLACE(_tag, CDISLAVE, 0)
//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdislave_device

class cdislave_device : public device_t
{
public:
	// construction/destruction
	cdislave_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// external callbacks
	DECLARE_INPUT_CHANGED_MEMBER( mouse_update );

	uint8_t* get_lcd_state() { return m_lcd_state; }

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

		uint8_t m_out_buf[4];
		uint8_t m_out_index;
		uint8_t m_out_count;
		uint8_t m_out_cmd;
	};

	channel_state m_channel[4];
	emu_timer *m_interrupt_timer;

	uint8_t m_in_buf[17];
	uint8_t m_in_index;
	uint8_t m_in_count;

	uint8_t m_polling_active;

	uint8_t m_xbus_interrupt_enable;

	uint8_t m_lcd_state[16];

	uint16_t m_real_mouse_x;
	uint16_t m_real_mouse_y;

	uint16_t m_fake_mouse_x;
	uint16_t m_fake_mouse_y;

	// static internal members

	// non-static internal members
	void prepare_readback(const attotime &delay, uint8_t channel, uint8_t count, uint8_t data0, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t cmd);
	void perform_mouse_update();
	void set_mouse_position();
};


// device type definition
DECLARE_DEVICE_TYPE(CDISLAVE, cdislave_device)

#endif // __CDISLAVE_H__
