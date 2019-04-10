// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef DEVICES_BUS_SGIKBD_HLEKBD_H
#define DEVICES_BUS_SGIKBD_HLEKBD_H

#pragma once

#include "sgikbd.h"
#include "machine/keyboard.h"
#include "sound/beep.h"
#include "diserial.h"


namespace bus { namespace sgikbd {

class hle_device : public device_t
	, public device_buffered_serial_interface<16U>
	, public device_sgi_keyboard_port_interface
	, protected device_matrix_keyboard_interface<8U>
{
public:
	// constructor/destructor
	hle_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	virtual ~hle_device() override;

	virtual DECLARE_WRITE_LINE_MEMBER(input_txd) override;

protected:
	// device overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;

	// customised transmit_byte method
	void transmit_byte(uint8_t byte);

private:
	enum
	{
		TIMER_CLICK = 30000,
		TIMER_BEEP  = 30001
	};

	enum
	{
		LED_NUM,
		LED_CAPS,
		LED_SCROLL,
		LED_USER1,
		LED_USER2,
		LED_USER3,
		LED_USER4
	};

	enum
	{
		BEEPER_BELL     = 0x01,
		BEEPER_CLICK    = 0x02
	};

	enum
	{
		RX_IDLE,
	};

	enum
	{
		CTRL_A_SBEEP        = 1,
		CTRL_A_LBEEP        = 2,
		CTRL_A_NOCLICK      = 3,
		CTRL_A_RCB          = 4,
		CTRL_A_NUMLK        = 5,
		CTRL_A_CAPSLK       = 6,
		CTRL_A_AUTOREP      = 7,

		CTRL_B              = 0,
		CTRL_B_CMPL_DS1_2   = 1,
		CTRL_B_SCRLK        = 2,
		CTRL_B_L1           = 3,
		CTRL_B_L2           = 4,
		CTRL_B_L3           = 5,
		CTRL_B_L4           = 6
	};

	// device_buffered_serial_interface overrides
	virtual void received_byte(uint8_t byte) override;

	emu_timer                       *m_click_timer;
	emu_timer                       *m_beep_timer;
	required_device<beep_device>    m_beeper;
	output_finder<7>                m_leds;

	uint8_t     m_make_count;

	bool        m_keyclick;
	bool        m_auto_repeat;
	uint8_t     m_beeper_state;
	uint8_t     m_led_state;
};

} } // namespace bus::sgikbd


DECLARE_DEVICE_TYPE_NS(SGI_HLE_KEYBOARD, bus::sgikbd, hle_device)

#endif // DEVICES_BUS_SGIKBD_HLEKBD_H
