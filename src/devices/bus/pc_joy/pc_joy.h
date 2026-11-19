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


class device_pc_joy_interface : public device_interface
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

class pc_joy_device : public device_t, public device_single_card_slot_interface<device_pc_joy_interface>
{
public:
	pc_joy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t joy_port_r();
	void joy_port_w(uint8_t data);

protected:
	virtual void device_start() override { m_stime = machine().time(); }
	virtual void device_config_complete() override;

private:
	attotime m_stime;
	device_pc_joy_interface *m_dev;
};

DECLARE_DEVICE_TYPE(PC_JOY, pc_joy_device)

#endif // MAME_BUS_PC_JOY_PC_JOY_H
