// license:BSD-3-Clause
// copyright-holders:Carl
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
	MCFG_DEVICE_ADD(_tag, PC_JOY, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(pc_joysticks, "basic_joy", false)

SLOT_INTERFACE_EXTERN(pc_joysticks);

class device_pc_joy_interface: public device_slot_card_interface
{
public:
	device_pc_joy_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pc_joy_interface();

	virtual UINT8 x1(int delta) { return 0; }
	virtual UINT8 x2(int delta) { return 0; }
	virtual UINT8 y1(int delta) { return 0; }
	virtual UINT8 y2(int delta) { return 0; }
	virtual UINT8 btn() { return 0xf; }
	virtual void port_write() { }
};

class pc_joy_device :  public device_t,
							public device_slot_interface
{
public:
	pc_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(joy_port_r);
	DECLARE_WRITE8_MEMBER(joy_port_w);
protected:
	virtual void device_start() { m_stime = machine().time(); }
	virtual void device_config_complete();
private:
	attotime m_stime;
	device_pc_joy_interface *m_dev;
};

extern const device_type PC_JOY;

class pc_basic_joy_device : public device_t,
							public device_pc_joy_interface
{
public:
	pc_basic_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ioport_constructor device_input_ports() const;

	virtual UINT8 x1(int delta) { return (m_x1->read() > delta); }
	virtual UINT8 x2(int delta) { return (m_x2->read() > delta); }
	virtual UINT8 y1(int delta) { return (m_y1->read() > delta); }
	virtual UINT8 y2(int delta) { return (m_y2->read() > delta); }
	virtual UINT8 btn() { return m_btn->read(); }

protected:
	virtual void device_start() {}

private:
	required_ioport m_btn;
	required_ioport m_x1;
	required_ioport m_y1;
	required_ioport m_x2;
	required_ioport m_y2;
};

#endif /* PC_JOY_H */
