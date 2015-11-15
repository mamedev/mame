// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Keyboard (HLE)

***************************************************************************/

#pragma once

#ifndef __APRICOTKB_HLE__
#define __APRICOTKB_HLE__

#include "emu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_APRICOT_KEYBOARD_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, APRICOT_KEYBOARD_HLE, 0)

#define MCFG_APRICOT_KEYBOARD_TXD_HANDLER(_write) \
	devcb = &apricot_keyboard_hle_device::set_txd_handler(*device, DEVCB_##_write);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apricot_keyboard_hle_device

class apricot_keyboard_hle_device : public device_t, public device_serial_interface
{
public:
	// construction/destruction
	apricot_keyboard_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_txd_handler(device_t &device, _Object object)
		{ return downcast<apricot_keyboard_hle_device &>(device).m_txd_handler.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER(rxd_w);
	DECLARE_INPUT_CHANGED_MEMBER(key_callback);

protected:
	// device_t overrides
	virtual ioport_constructor device_input_ports() const;
	virtual void device_start();
	virtual void device_reset();

	// device_serial_interface overrides
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_callback();
	virtual void rcv_complete();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	devcb_write_line m_txd_handler;

	int m_rxd;

	UINT8 m_data_in;
	UINT8 m_data_out;
};


// device type definition
extern const device_type APRICOT_KEYBOARD_HLE;


#endif // __APRICOTKB_HLE__
