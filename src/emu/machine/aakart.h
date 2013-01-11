/***************************************************************************

Acorn Archimedes KART interface

***************************************************************************/

#pragma once

#ifndef __AAKARTDEV_H__
#define __AAKARTDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AAKART_ADD(_tag, _freq, _config) \
	MCFG_DEVICE_ADD(_tag, AAKART, _freq) \
	MCFG_DEVICE_CONFIG(_config) \

#define AAKART_INTERFACE(name) \
	const aakart_interface (name) =

struct aakart_interface
{
	devcb_write_line        m_out_tx_cb;
	devcb_write_line        m_out_rx_cb;
};

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

class aakart_device : public device_t,
						public aakart_interface
{
public:
	// construction/destruction
	aakart_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();

private:
	static const device_timer_id RX_TIMER = 1;
	static const device_timer_id TX_TIMER = 2;
	static const device_timer_id MOUSE_TIMER = 3;
	static const device_timer_id KEYB_TIMER = 4;
	emu_timer *         m_rxtimer;
	emu_timer *         m_txtimer;
	emu_timer *         m_mousetimer;
	emu_timer *         m_keybtimer;

	devcb_resolved_write_line   m_out_tx_func;
	devcb_resolved_write_line   m_out_rx_func;
	int m_tx_latch, m_rx_latch;
	int m_rx;
	int m_new_command;
	int m_status;
	int m_mouse_enable;
	int m_keyb_enable;
};


// device type definition
extern const device_type AAKART;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
