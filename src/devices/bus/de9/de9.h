// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Generic DE-9 port emulation

**********************************************************************/

#ifndef MAME_BUS_DE9_DE9_H
#define MAME_BUS_DE9_DE9_H

#pragma once


class de9_port_device;


class device_de9_port_interface : public device_interface
{
public:
	virtual ~device_de9_port_interface() { }

	virtual u16 read() { return 0xff; }
	virtual void pin_w(int pin, int state) { }

protected:
	device_de9_port_interface(const machine_config &mconfig, device_t &device);

	de9_port_device *m_port;
};


class de9_port_device : public device_t,
						public device_slot_interface
{
public:
	template <typename T>
	de9_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, char const* dflt)
		: de9_port_device(mconfig, tag, owner)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	de9_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// computer interface
	//
	// Mapping between bits and pins:
	// bit 0 - pin 1
	// bit 1 - pin 2
	// bit 2 - pin 3
	// bit 3 - pin 4
	// bit 4 - pin 5
	// bit 5 - pin 6
	// bit 6 - pin 7
	// bit 7 - pin 8
	// bit 8 - pin 9
	//
	u16 read() { return exists() ? m_device->read() : 0xffff; }
	template<int Pin> void pin_w(int state)
	{
		static_assert(Pin >= 1 && Pin <= 9, "Invalid DE-9 pin number");
		if (exists())
			m_device->pin_w(Pin, state);
	}

	bool exists() { return m_device != nullptr; }

protected:
	virtual void device_start() override;

	device_de9_port_interface *m_device;
};


DECLARE_DEVICE_TYPE(DE9_PORT, de9_port_device)


#endif // MAME_BUS_DE9_DE9_H
