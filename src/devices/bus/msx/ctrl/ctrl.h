// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    MSX General Purpose port emulation

The MSX General Purpose port uses a DE-9 connector; the pin assignment
is similar to but slightly different from the Atari VCS controller port.

**********************************************************************/

#ifndef MAME_BUS_MSX_CTRL_CTRL_H
#define MAME_BUS_MSX_CTRL_CTRL_H

#pragma once


class msx_general_purpose_port_device;


class device_msx_general_purpose_port_interface : public device_interface
{
public:
	virtual ~device_msx_general_purpose_port_interface() { }

	virtual u8 read() { return 0xff; }
	virtual void pin_6_w(int state) { }
	virtual void pin_7_w(int state) { }
	virtual void pin_8_w(int state) { }

protected:
	device_msx_general_purpose_port_interface(const machine_config &mconfig, device_t &device);

	msx_general_purpose_port_device *m_port;
};


class msx_general_purpose_port_device : public device_t
									, public device_single_card_slot_interface<device_msx_general_purpose_port_interface>
{
public:
	template <typename T>
	msx_general_purpose_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, char const* dflt)
		: msx_general_purpose_port_device(mconfig, tag, owner)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	msx_general_purpose_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// computer interface
	//
	// Data returned by the read method:
	// bit 0 - pin 1 - Up (I)
	// bit 1 - pin 2 - Down (I)
	// bit 2 - pin 3 - Left (I)
	// bit 3 - pin 4 - Right (I)
	//         pin 5 - +5V
	// bit 4 - pin 6 - Button 1 (I/O)
	// bit 5 - pin 7 - Button 2 (I/O)
	// bit 6 - pin 8 - Strobe (O for MSX, I/O for FM Towns)
	//         pin 9 - GND
	//
	u8 read() { return exists() ? m_device->read() : 0xff; }

	void pin_6_w(int state) { if (exists()) m_device->pin_6_w(state); }
	void pin_7_w(int state) { if (exists()) m_device->pin_7_w(state); }
	void pin_8_w(int state) { if (exists()) m_device->pin_8_w(state); }

	bool exists() const { return m_device != nullptr; }

protected:
	virtual void device_start() override ATTR_COLD;

	device_msx_general_purpose_port_interface *m_device;
};


DECLARE_DEVICE_TYPE(MSX_GENERAL_PURPOSE_PORT, msx_general_purpose_port_device)

void msx_general_purpose_port_devices(device_slot_interface &device);

#endif // MAME_BUS_MSX_CTRL_CTRL_H
