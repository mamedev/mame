// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Mouse for use with the v9938 color bus

    Used with the Geneve 9640 and 80 column cards (like the EVPC)

    Michael Zapf, 2017-03-18

*****************************************************************************/

#ifndef __BUSMOUSE__
#define __BUSMOUSE__

#include "colorbus.h"

extern const device_type BUSMOUSE;

class busmouse_device : public colorbus_attached_device
{
public:
	busmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void poll(int& delta_x, int& delta_y, int& buttons) override;

protected:
	virtual void device_start(void) override;
	virtual void device_reset(void) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	int             m_last_mx;
	int             m_last_my;
};
#endif
