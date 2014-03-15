#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "machine/keyboard.h"

#define TERMINAL_SCREEN_TAG "terminal_screen"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct terminal_interface
{
	devcb_write8 m_keyboard_cb;
};

#define GENERIC_TERMINAL_INTERFACE(name) const terminal_interface (name) =

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_GENERIC_TERMINAL_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, GENERIC_TERMINAL, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_GENERIC_TERMINAL_REMOVE(_tag)      \
	MCFG_DEVICE_REMOVE(_tag)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

#define TERMINAL_WIDTH 80
#define TERMINAL_HEIGHT 24

INPUT_PORTS_EXTERN( generic_terminal );

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
	optional_device<palette_device> m_palette;
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

#endif /* __TERMINAL_H__ */
