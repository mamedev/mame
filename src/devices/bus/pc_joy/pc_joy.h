// license:BSD-3-Clause
// copyright-holders:Carl
/*************************************************************************
 *
 *      pc_joy.h
 *
 *      joystick port
 *
 *************************************************************************/

#ifndef MAME_BUS_PC_JOY_PC_JOY_H
#define MAME_BUS_PC_JOY_PC_JOY_H

#pragma once


void pc_joysticks(device_slot_interface &device);

class device_pc_joy_interface : public device_slot_card_interface
{
public:
	virtual ~device_pc_joy_interface();

	virtual uint8_t x1(int delta) { return 0; }
	virtual uint8_t x2(int delta) { return 0; }
	virtual uint8_t y1(int delta) { return 0; }
	virtual uint8_t y2(int delta) { return 0; }
	virtual uint8_t btn() { return 0xf; }
	virtual void port_write() { }

protected:
	device_pc_joy_interface(const machine_config &mconfig, device_t &device);
};

class pc_joy_device : public device_t, public device_slot_interface
{
public:
	pc_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_READ8_MEMBER(joy_port_r);
	DECLARE_WRITE8_MEMBER(joy_port_w);

protected:
	virtual void device_start() override { m_stime = machine().time(); }
	virtual void device_config_complete() override;

private:
	attotime m_stime;
	device_pc_joy_interface *m_dev;
};

DECLARE_DEVICE_TYPE(PC_JOY, pc_joy_device)

class pc_basic_joy_device : public device_t,
							public device_pc_joy_interface
{
public:
	pc_basic_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t x1(int delta) override { return (m_x1->read() > delta); }
	virtual uint8_t x2(int delta) override { return (m_x2->read() > delta); }
	virtual uint8_t y1(int delta) override { return (m_y1->read() > delta); }
	virtual uint8_t y2(int delta) override { return (m_y2->read() > delta); }
	virtual uint8_t btn() override { return m_btn->read(); }

protected:
	virtual void device_start() override { }

private:
	required_ioport m_btn;
	required_ioport m_x1;
	required_ioport m_y1;
	required_ioport m_x2;
	required_ioport m_y2;
};

#endif // MAME_BUS_PC_JOY_PC_JOY_H
