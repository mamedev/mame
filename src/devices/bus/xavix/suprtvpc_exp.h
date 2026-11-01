// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_BUS_XAVIX_SUPRTVPC_EXP_H
#define MAME_BUS_XAVIX_SUPRTVPC_EXP_H

#pragma once

class device_suprtvpc_exp_interface;

// Super TV-PC's rear expansion port.  Connector pin numbering is not yet known.
class suprtvpc_exp_port_device : public device_t, public device_single_card_slot_interface<device_suprtvpc_exp_interface>
{
public:
	suprtvpc_exp_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void data_w(int state);
	void clock_w(int state);
	int ready_r();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_suprtvpc_exp_interface *m_peripheral;
};

class device_suprtvpc_exp_interface : public device_interface
{
public:
	virtual void data_w(int state);
	virtual void clock_w(int state);
	virtual int ready_r();

protected:
	device_suprtvpc_exp_interface(const machine_config &mconfig, device_t &device);
};

DECLARE_DEVICE_TYPE(SUPRTVPC_EXP_PORT, suprtvpc_exp_port_device)

#endif // MAME_BUS_XAVIX_SUPRTVPC_EXP_H
