#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "emu.h"
#include "machine/serial.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct keyboard_interface
{
	devcb_write8 m_keyboard_cb;
};

#define ASCII_KEYBOARD_INTERFACE(name) const keyboard_interface (name) =

struct serial_keyboard_interface
{
	devcb_write_line m_out_tx_cb;
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define KEYBOARD_TAG "keyboard"

#define MCFG_ASCII_KEYBOARD_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, GENERIC_KEYBOARD, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_SERIAL_KEYBOARD_ADD(_tag, _intrf, _clock) \
	MCFG_DEVICE_ADD(_tag, SERIAL_KEYBOARD, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class generic_keyboard_device :
	public device_t,
	public keyboard_interface
{
public:
	generic_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	generic_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual ioport_constructor device_input_ports() const;
	virtual machine_config_constructor device_mconfig_additions() const;
protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();
	virtual void send_key(UINT8 code) { m_keyboard_func(0, code); }
	emu_timer *m_timer;
private:
	UINT8 keyboard_handler(UINT8 last_code, UINT8 *scan_line);
	UINT8 row_number(UINT8 code);
	UINT8 m_last_code;
	UINT8 m_scan_line;

	devcb_resolved_write8 m_keyboard_func;
};

extern const device_type GENERIC_KEYBOARD;

class serial_keyboard_device :
	public generic_keyboard_device,
	public device_serial_interface,
	public device_serial_port_interface,
	public serial_keyboard_interface
{
public:
	serial_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER(rx_w) { m_tbit = state; check_for_start(state); }
	DECLARE_READ_LINE_MEMBER(tx_r);
	virtual ioport_constructor device_input_ports() const;

	DECLARE_INPUT_CHANGED_MEMBER(update_frame);
protected:
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_reset();
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void input_callback(UINT8 state) { m_input_state = state; }
	virtual void send_key(UINT8 code);
private:
	serial_port_device *m_owner;
	bool m_slot;
	UINT8 m_curr_key;
	bool m_key_valid;
	devcb_resolved_write_line m_out_tx_func;
};

extern const device_type SERIAL_KEYBOARD;

#endif /* __KEYBOARD_H__ */
