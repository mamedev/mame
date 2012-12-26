/***************************************************************************

	PC-9801 Keyboard simulation

***************************************************************************/

#pragma once

#ifndef __PC9801_KBDDEV_H__
#define __PC9801_KBDDEV_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PC9801_KBD_ADD(_tag,_freq,_config) \
	MCFG_DEVICE_ADD(_tag, PC9801_KBD, _freq) \
	MCFG_DEVICE_CONFIG(_config)

#define PC9801_KBD_INTERFACE(name) \
	const pc9801_kbd_interface (name) =


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> pc9801_kbd_interface

struct pc9801_kbd_interface
{
	devcb_write_line		m_irq_cb;
};


// ======================> pc9801_kbd_device

class pc9801_kbd_device : public device_t,
                          public pc9801_kbd_interface
{
public:
	// construction/destruction
	pc9801_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const;

	// I/O operations
	DECLARE_WRITE8_MEMBER( tx_w );
	DECLARE_READ8_MEMBER( rx_r );
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();

	devcb_resolved_write_line	m_irq_func;

	static const device_timer_id RX_TIMER = 1;
	emu_timer *			m_rxtimer;
	UINT8 				m_rx_buf[0x80];
	UINT8				m_keyb_tx;
	UINT8				m_keyb_rx;
	UINT8				m_caps_lock_state;
	UINT8				m_kana_lock_state;
};


// device type definition
extern const device_type PC9801_KBD;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
