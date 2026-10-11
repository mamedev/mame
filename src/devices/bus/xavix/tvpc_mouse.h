// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_BUS_XAVIX_TVPC_MOUSE_H
#define MAME_BUS_XAVIX_TVPC_MOUSE_H

#pragma once

class device_tvpc_mouse_interface;

class tvpc_mouse_port_device : public device_t, public device_mixer_interface, public device_single_card_slot_interface<device_tvpc_mouse_interface>
{
public:
	tvpc_mouse_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_mouse_index(unsigned index) { m_mouse_index = index; }
	unsigned mouse_index() const { return m_mouse_index; }

	u8 x_r();
	u8 y_r();
	int button_r();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_tvpc_mouse_interface *m_mouse;
	unsigned m_mouse_index;
};

// Shared motion and button abstraction: cumulative samples feed the host counters.
// Original TV-PC and Super TV-PC mouse connectors differ; their physical pinouts
// and signalling are not yet confirmed.
class device_tvpc_mouse_interface : public device_interface
{
public:
	virtual u8 x_r();
	virtual u8 y_r();
	virtual int button_r();

protected:
	device_tvpc_mouse_interface(const machine_config &mconfig, device_t &device);
};

DECLARE_DEVICE_TYPE(TVPC_MOUSE_PORT, tvpc_mouse_port_device)
DECLARE_DEVICE_TYPE(TVPC_MOUSE, device_tvpc_mouse_interface)
DECLARE_DEVICE_TYPE(TVPC_HK_MOUSE, device_tvpc_mouse_interface)
DECLARE_DEVICE_TYPE(SUPRTVPC_MOUSE, device_tvpc_mouse_interface)
DECLARE_DEVICE_TYPE(EPO_NTPJ_MOUSE, device_tvpc_mouse_interface)

#endif // MAME_BUS_XAVIX_TVPC_MOUSE_H
