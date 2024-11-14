// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Keyboard (HLE)

***************************************************************************/

#ifndef MAME_BUS_APRICOT_KEYBOARD_HLE_H
#define MAME_BUS_APRICOT_KEYBOARD_HLE_H

#pragma once

#include "keyboard.h"
#include "machine/keyboard.h"
#include "machine/msm5832.h"
#include "machine/timer.h"
#include "diserial.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apricot_keyboard_hle_device

class apricot_keyboard_hle_device : public device_t,
									public device_apricot_keyboard_interface,
									public device_buffered_serial_interface<16>,
									protected device_matrix_keyboard_interface<13>
{
public:
	// construction/destruction
	apricot_keyboard_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from host
	virtual void out_w(int state) override;

protected:
	// device_t overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;
	virtual void received_byte(uint8_t byte) override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(mouse_callback);

	enum {
		CMD_REQ_TIME_AND_DATE = 0xe1,
		CMD_SET_TIME_AND_DATE = 0xe4,
		CMD_ENABLE_MOUSE      = 0xe5,
		CMD_DISABLE_MOUSE     = 0xe6,
		CMD_KEYBOARD_RESET    = 0xe8
	};

	enum {
		ACK_DIAGNOSTICS = 0xfb
	};

	required_device<msm5832_device> m_rtc;
	required_ioport m_mouse_b;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;

	int m_rtc_index;
	bool m_mouse_enabled;
	uint8_t m_mouse_last_b;
	uint8_t m_mouse_last_x;
	uint8_t m_mouse_last_y;
};


// device type definition
DECLARE_DEVICE_TYPE(APRICOT_KEYBOARD_HLE, apricot_keyboard_hle_device)


#endif // MAME_BUS_APRICOT_KEYBOARD_HLE_H
