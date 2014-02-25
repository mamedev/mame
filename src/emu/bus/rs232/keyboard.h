#ifndef __RS232_KEYBOARD_H__
#define __RS232_KEYBOARD_H__

#include "rs232.h"
#include "machine/keyboard.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct serial_keyboard_interface
{
	devcb_write_line m_out_tx_cb;
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SERIAL_KEYBOARD_ADD(_tag, _intrf, _clock) \
	MCFG_DEVICE_ADD(_tag, SERIAL_KEYBOARD, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class serial_keyboard_device :
	public generic_keyboard_device,
	public device_serial_interface,
	public device_rs232_port_interface,
	public serial_keyboard_interface
{
public:
	serial_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	serial_keyboard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) { device_serial_interface::rx_w(state); }
	DECLARE_READ_LINE_MEMBER(tx_r);

	virtual ioport_constructor device_input_ports() const;

	DECLARE_INPUT_CHANGED_MEMBER(update_frame);

protected:
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void send_key(UINT8 code);
private:
	int m_rbit;
	UINT8 m_curr_key;
	bool m_key_valid;
	devcb_resolved_write_line m_out_tx_func;
	required_ioport m_io_term_frame;
};

extern const device_type SERIAL_KEYBOARD;

#endif /* __RS232_KEYBOARD_H__ */
