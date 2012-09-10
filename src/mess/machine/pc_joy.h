/*************************************************************************
 *
 *      pc_joy.h
 *
 *      joystick port
 *
 *************************************************************************/

#ifndef PC_JOY_H
#define PC_JOY_H

#include "emu.h"

READ8_HANDLER ( pc_JOY_r );
WRITE8_HANDLER ( pc_JOY_w );

INPUT_PORTS_EXTERN( pc_joystick_none );
INPUT_PORTS_EXTERN( pc_joystick );

#define MCFG_PC_JOY_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PC_JOY, 0)

class pc_joy_device : public device_t
{
public:
	pc_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER(joy_port_r) { return pc_JOY_r(&space, offset); }
	DECLARE_WRITE8_MEMBER(joy_port_w) { pc_JOY_w(&space, offset, data); }
protected:
	virtual void device_start() {}
};

extern const device_type PC_JOY;
#endif /* PC_JOY_H */
