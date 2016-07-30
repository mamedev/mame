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


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apricot_keyboard_hle_device

class apricot_keyboard_hle_device : public device_t, public device_apricot_keyboard_interface, public device_serial_interface
{
public:
	// construction/destruction
	apricot_keyboard_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(key_callback);

	// from host
	virtual void out_w(int state) override;

protected:
	// device_t overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_callback() override;
	virtual void rcv_complete() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	int m_rxd;

	UINT8 m_data_in;
	UINT8 m_data_out;
};


// device type definition
extern const device_type APRICOT_KEYBOARD_HLE;


#endif // __APRICOT_KEYBOARD_HLE_H__
