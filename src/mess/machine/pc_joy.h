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

#define MCFG_PC_JOY_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PC_JOY, 0)

class pc_joy_device : public device_t
{
public:
	pc_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER(joy_port_r);
	DECLARE_WRITE8_MEMBER(joy_port_w);
protected:
	virtual void device_start() {}

private:
	required_ioport m_btn;
	required_ioport m_x1;
	required_ioport m_y1;
	required_ioport m_x2;
	required_ioport m_y2;

	attotime m_stime;
};

extern const device_type PC_JOY;
#endif /* PC_JOY_H */
