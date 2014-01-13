#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "emu.h"
#include "machine/serial.h"
#include "machine/keyboard.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct terminal_interface
{
	devcb_write8 m_keyboard_cb;
};

#define GENERIC_TERMINAL_INTERFACE(name) const terminal_interface (name) =

struct serial_terminal_interface
{
	devcb_write_line m_out_tx_cb;
};

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define TERMINAL_TAG "terminal"
#define TERMINAL_SCREEN_TAG "terminal_screen"

#define MCFG_GENERIC_TERMINAL_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, GENERIC_TERMINAL, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_GENERIC_TERMINAL_REMOVE(_tag)      \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_SERIAL_TERMINAL_ADD(_tag, _intrf, _clock) \
	MCFG_DEVICE_ADD(_tag, SERIAL_TERMINAL, _clock) \
	MCFG_DEVICE_CONFIG(_intrf)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 24

class generic_terminal_device :
	public device_t,
	public terminal_interface
{
public:
	generic_terminal_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	generic_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_WRITE8_MEMBER(write) { term_write(data); }
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT32 update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual ioport_constructor device_input_ports() const;
	virtual machine_config_constructor device_mconfig_additions() const;
protected:
	required_ioport m_io_term_conf;

	virtual void term_write(UINT8 data);
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();
	virtual void send_key(UINT8 code) { m_keyboard_func(0, code); }
	UINT8 m_buffer[TERMINAL_WIDTH*50]; // make big enough for teleprinter
	UINT8 m_x_pos;
private:
	void scroll_line();
	void write_char(UINT8 data);
	void clear();

	UINT8 m_framecnt;
	UINT8 m_y_pos;

	devcb_resolved_write8 m_keyboard_func;
};

extern const device_type GENERIC_TERMINAL;

class serial_terminal_device :
	public generic_terminal_device,
	public device_serial_interface,
	public device_serial_port_interface,
	public serial_terminal_interface
{
public:
	serial_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER(rx_w) { m_tbit = state; device_serial_interface::rx_w(state); }
	DECLARE_READ_LINE_MEMBER(tx_r);
	virtual void tx(UINT8 state) { rx_w(state); }
	virtual ioport_constructor device_input_ports() const;

	DECLARE_WRITE_LINE_MEMBER(update_serial);

protected:
	virtual void device_start();
	virtual void device_config_complete();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void tra_callback();
	virtual void tra_complete();
	virtual void rcv_complete();
	virtual void input_callback(UINT8 state) { m_input_state = state; }
	virtual void send_key(UINT8 code);

private:
	required_ioport m_io_term_txbaud;
	required_ioport m_io_term_rxbaud;
	required_ioport m_io_term_startbits;
	required_ioport m_io_term_databits;
	required_ioport m_io_term_parity;
	required_ioport m_io_term_stopbits;

	serial_port_device *m_owner;
	bool m_slot;
	UINT8 m_curr_key;
	bool m_key_valid;
	devcb_resolved_write_line m_out_tx_func;
};

extern const device_type SERIAL_TERMINAL;

#endif /* __TERMINAL_H__ */
