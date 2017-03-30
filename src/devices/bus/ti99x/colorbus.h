// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    v9938 color bus

    Used with the Geneve 9640 and 80 column cards (like the EVPC)
    for the TI-99/4A

    Michael Zapf, 2017-03-18

*****************************************************************************/

#ifndef __COLORBUS__
#define __COLORBUS__

#include "video/v9938.h"

extern const device_type COLORBUS;
class colorbus_device;

/********************************************************************
    Common parent class of all devices attached to the color bus
********************************************************************/
class colorbus_attached_device : public device_t
{
public:
	colorbus_attached_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source), m_colorbus(nullptr) { };

	virtual void poll(int& delta_x, int& delta_y, int& buttons) =0;

protected:
	virtual void device_config_complete() override;
	colorbus_device* m_colorbus;
};

/********************************************************************
    Color bus port
********************************************************************/
class colorbus_device : public device_t, public device_slot_interface
{
public:
	colorbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	line_state left_button();  // left button is not connected to the V9938 but to a TMS9901 pin
	void poll();

protected:
	void device_start() override { };
	void device_config_complete() override;

private:
	colorbus_attached_device*   m_connected;
	required_device<v9938_device> m_v9938;
	bool m_left_button_pressed;
};

SLOT_INTERFACE_EXTERN(colorbus_port);

#define MCFG_COLORBUS_MOUSE_ADD( _tag )  \
	MCFG_DEVICE_ADD(_tag, COLORBUS, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(colorbus_port, "busmouse", false)

#endif
