// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Keyboard (HLE)

***************************************************************************/

#pragma once

#ifndef __APRICOT_KEYBOARD_HLE_H__
#define __APRICOT_KEYBOARD_HLE_H__

#include "emu.h"
#include "keyboard.h"
#include "machine/keyboard.h"
#include "machine/msm5832.h"


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
	apricot_keyboard_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// from host
	virtual void out_w(int state) override;

protected:
	// device_t overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;
	virtual void received_byte(UINT8 byte) override;

	// device_matrix_keyboard_interface overrides
	virtual void key_make(UINT8 row, UINT8 column) override;
	virtual void key_break(UINT8 row, UINT8 column) override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_device<msm5832_device> m_rtc;

	enum {
		CMD_REQ_TIME_AND_DATE = 0xe1,
		CMD_SET_TIME_AND_DATE = 0xe4
	};

	int m_rtc_index;
};


// device type definition
extern const device_type APRICOT_KEYBOARD_HLE;


#endif // __APRICOT_KEYBOARD_HLE_H__
