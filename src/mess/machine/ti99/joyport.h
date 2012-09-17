/****************************************************************************

    Joystick port

    Now explicitly implemented as a slot device
    A joystick port allows for plugging in a single or twin joystick, or
    a Mechatronics mouse.

    The TI-99/4 also offers an infrared handset, connected to this port. For
    this reason we also need an interrupt line.

    Michael Zapf

    June 2012

*****************************************************************************/

#ifndef __JOYPORT__
#define __JOYPORT__

#include "emu.h"

extern const device_type JOYPORT;

struct joyport_config
{
	devcb_write_line		interrupt;
	int						vdp_clock;
};

#define JOYPORT_CONFIG(name) \
	const joyport_config(name) =

class joyport_device;

/********************************************************************
    Common parent class of all devices attached to the joystick port
********************************************************************/
class joyport_attached_device : public device_t
{
	friend class joyport_device;
public:
	joyport_attached_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock) { }

protected:
	void device_config_complete();
	joyport_device*	m_joyport;

private:
	virtual UINT8 read_dev() =0;
	virtual void write_dev(UINT8 data) =0;
};

/********************************************************************
    Joystick port
********************************************************************/
class joyport_device : public device_t, public device_slot_interface
{
public:
	joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	UINT8	read_port();
	void	write_port(int data);
	void	set_interrupt(int state);

protected:
	void device_start() { };
	void device_config_complete();

private:
	devcb_resolved_write_line	m_interrupt;
	joyport_attached_device*	m_connected;
};

SLOT_INTERFACE_EXTERN(joystick_port);
SLOT_INTERFACE_EXTERN(joystick_port_994);
SLOT_INTERFACE_EXTERN(joystick_port_gen);

#define MCFG_GENEVE_JOYPORT_ADD( _tag, _conf )	\
	MCFG_DEVICE_ADD(_tag, JOYPORT, 0) \
	MCFG_DEVICE_CONFIG( _conf ) \
	MCFG_DEVICE_SLOT_INTERFACE(joystick_port_gen, "twinjoy", NULL, false)

#define MCFG_TI_JOYPORT4A_ADD( _tag, _conf )	\
	MCFG_DEVICE_ADD(_tag, JOYPORT, 0) \
	MCFG_DEVICE_CONFIG( _conf ) \
	MCFG_DEVICE_SLOT_INTERFACE(joystick_port, "twinjoy", NULL, false)

#define MCFG_TI_JOYPORT4_ADD( _tag, _conf )	\
	MCFG_DEVICE_ADD(_tag, JOYPORT, 0) \
	MCFG_DEVICE_CONFIG( _conf ) \
	MCFG_DEVICE_SLOT_INTERFACE(joystick_port_994, "twinjoy", NULL, false)

#endif /* __JOYPORT__ */
