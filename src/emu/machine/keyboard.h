// license:BSD-3-Clause
// copyright-holders:smf
#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "emu.h"

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_GENERIC_KEYBOARD_CB(_devcb) \
	devcb = &generic_keyboard_device::set_keyboard_callback(*device, DEVCB_##_devcb);

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

INPUT_PORTS_EXTERN( generic_keyboard );

class generic_keyboard_device : public device_t
{
public:
	generic_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	generic_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_keyboard_callback(device_t &device, _Object object) { return downcast<generic_keyboard_device &>(device).m_keyboard_cb.set_callback(object); }

	virtual ioport_constructor device_input_ports() const;
	virtual machine_config_constructor device_mconfig_additions() const;
protected:
	required_ioport m_io_kbd0;
	required_ioport m_io_kbd1;
	required_ioport m_io_kbd2;
	required_ioport m_io_kbd3;
	required_ioport m_io_kbd4;
	required_ioport m_io_kbd5;
	required_ioport m_io_kbd6;
	required_ioport m_io_kbd7;
	required_ioport m_io_kbd8;
	required_ioport m_io_kbd9;
	required_ioport m_io_kbdc;

	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void send_key(UINT8 code) { m_keyboard_cb((offs_t)0, code); }
	emu_timer *m_timer;
private:
	virtual UINT8 keyboard_handler(UINT8 last_code, UINT8 *scan_line);
	UINT8 row_number(UINT8 code);
	UINT8 m_last_code;
	UINT8 m_scan_line;

	devcb_write8 m_keyboard_cb;
};

extern const device_type GENERIC_KEYBOARD;

#endif /* __KEYBOARD_H__ */
