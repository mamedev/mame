// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Acorn Archimedes KART interface

***************************************************************************/

#pragma once

#ifndef __AAKARTDEV_H__
#define __AAKARTDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AAKART_OUT_TX_CB(_devcb) \
	devcb = &aakart_device::set_out_tx_callback(*device, DEVCB_##_devcb);

#define MCFG_AAKART_OUT_RX_CB(_devcb) \
	devcb = &aakart_device::set_out_rx_callback(*device, DEVCB_##_devcb);


enum{
	STATUS_NORMAL = 0,
	STATUS_KEYUP,
	STATUS_KEYDOWN,
	STATUS_MOUSE,
	STATUS_HRST,
	STATUS_UNDEFINED
};

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aakart_device

class aakart_device : public device_t
{
public:
	// construction/destruction
	aakart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_out_tx_callback(device_t &device, _Object object) { return downcast<aakart_device &>(device).m_out_tx_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_rx_callback(device_t &device, _Object object) { return downcast<aakart_device &>(device).m_out_rx_cb.set_callback(object); }

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	void send_keycode_down(UINT8 row, UINT8 col);
	void send_keycode_up(UINT8 row, UINT8 col);
protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	static const device_timer_id RX_TIMER = 1;
	static const device_timer_id TX_TIMER = 2;
	static const device_timer_id MOUSE_TIMER = 3;
	static const device_timer_id KEYB_TIMER = 4;
	emu_timer *         m_rxtimer;
	emu_timer *         m_txtimer;
	emu_timer *         m_mousetimer;
	emu_timer *         m_keybtimer;

	devcb_write_line        m_out_tx_cb;
	devcb_write_line        m_out_rx_cb;
	UINT8 m_tx_latch;
	//UINT8 m_rx_latch;
	UINT8 m_rx;
	UINT8 m_new_command;
	UINT8 m_status;
	UINT8 m_mouse_enable;
	UINT8 m_keyb_enable;
	UINT8 m_keyb_row;
	UINT8 m_keyb_col;
	UINT8 m_keyb_state;

};


// device type definition
extern const device_type AAKART;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
