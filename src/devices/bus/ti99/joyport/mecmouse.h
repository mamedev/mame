// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Mechatronic mouse with adapter
    See mecmouse.c for documentation

    Michael Zapf
    October 2010
    January 2012: rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_JOYPORT_MECMOUSE_H
#define MAME_BUS_TI99_JOYPORT_MECMOUSE_H

#include "joyport.h"

namespace bus::ti99::joyport {

class mecmouse_device : public device_t, public device_ti99_joyport_interface
{
public:
	mecmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read_dev() override;
	void  write_dev(uint8_t data) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(poll_mouse);

private:
	required_ioport m_mousex;
	required_ioport m_mousey;
	required_ioport m_mousebutton;

	int     m_last_select;
	bool    m_read_y_axis;
	int     m_x;
	int     m_y;
	int     m_x_buf;
	int     m_y_buf;
	int     m_last_mx;
	int     m_last_my;

	emu_timer   *m_poll_timer;
};

} // end namespace bus::ti99::joyport

DECLARE_DEVICE_TYPE_NS(TI99_MECMOUSE, bus::ti99::joyport, mecmouse_device)

#endif // MAME_BUS_TI99_JOYPORT_MECMOUSE_H
