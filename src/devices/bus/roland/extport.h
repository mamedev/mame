// license:BSD-3-Clause
// copyright-holders:Carl Lom
/**********************************************************************

    Roland EXT port emulation

The Roland EXT port is a DE-9 connector found on the S-50/S-330/S-550
(and probably other Roland samplers/workstations of the era; the W-30
does not have one). It is used to attach a mouse (MU-1), an external
numeric keypad (RC-100), or a digitizer tablet (DT-100) — only one
device at a time is recognized by the firmware, selected by holding a
panel key at power-on.

The MU-1's protocol is identical to the MSX mouse protocol (see
bus/msx/ctrl/mouse.cpp), and MSX mice are known to work when plugged
into this port. The EXT port additionally allows the direction of two
of its pins to be switched at runtime (register C500 on the S-330),
which the RC-100 is believed to use to have the host drive output
lines instead of just reading device-driven input; the MU-1 leaves
both pins as inputs.

**********************************************************************/

#ifndef MAME_BUS_ROLAND_EXTPORT_H
#define MAME_BUS_ROLAND_EXTPORT_H

#pragma once


class roland_ext_port_device;


class device_roland_ext_port_interface : public device_interface
{
public:
	virtual ~device_roland_ext_port_interface() { }

	// data register (S-330: C400)
	virtual u8 read() { return 0xff; }
	virtual void write(u8 data, u8 mem_mask) { }

protected:
	device_roland_ext_port_interface(const machine_config &mconfig, device_t &device);

	roland_ext_port_device *m_port;
};


class roland_ext_port_device : public device_t
							, public device_single_card_slot_interface<device_roland_ext_port_interface>
{
public:
	template <typename T>
	roland_ext_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, char const *dflt)
		: roland_ext_port_device(mconfig, tag, owner)
	{
		set_options(std::forward<T>(opts), dflt, false);
	}
	roland_ext_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u8 read() { return exists() ? m_device->read() : 0xff; }

	// data write; which bits the host is actually driving is derived from
	// the pin direction register (see pindir_w) rather than passed in here
	void write(u8 data);

	// pin direction register (S-330: C500); bits 0-1 are believed to select
	// the direction of pins 6-7 (0 = input, matching MU-1; 1 = output, used
	// by RC-100) — unconfirmed against real RC-100/DT-100 hardware
	void pindir_w(u8 data) { m_pindir = data; }

	bool exists() const { return m_device != nullptr; }

protected:
	virtual void device_start() override ATTR_COLD;

	device_roland_ext_port_interface *m_device;
	u8 m_pindir;
};


DECLARE_DEVICE_TYPE(ROLAND_EXT_PORT, roland_ext_port_device)

void roland_ext_port_devices(device_slot_interface &device);

#endif // MAME_BUS_ROLAND_EXTPORT_H
