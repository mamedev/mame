// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Sega Game Gear EXT port emulation
    Also known as Gear-to-Gear (or VS, in Japan) cable connector

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_GAMEGEAR_GGEXT_H
#define MAME_BUS_GAMEGEAR_GGEXT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gg_ext_port_device

class device_gg_ext_port_interface;

class gg_ext_port_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	gg_ext_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: gg_ext_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	gg_ext_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~gg_ext_port_device();

	// static configuration helpers
	auto th_input_handler() { return m_th_pin_handler.bind(); }

	// Currently, only the support for SMS Controller Adaptor is emulated,
	// for when SMS Compatibility mode is enabled. In that mode, the 10 pins
	// of the EXT port follows the same numbering of a SMS Control port.

	// Data returned by the port_r methods:
	// bit 0 - pin 1 - Up
	// bit 1 - pin 2 - Down
	// bit 2 - pin 3 - Left
	// bit 3 - pin 4 - Right
	// bit 4 - pin 5 - Vcc (no data)
	// bit 5 - pin 6 - TL (Button 1/Light Phaser Trigger)
	// bit 6 - pin 7 - TH (Light Phaser sensor)
	//         pin 8 - GND
	// bit 7 - pin 9 - TR (Button 2)
	//         pin 10 - Not connected
	//
	uint8_t port_r();
	void port_w( uint8_t data );

	void th_pin_w(int state);

	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	// for peripherals that interact with the machine's screen
	required_device<screen_device> m_screen;

//protected:
	// device-level overrides
	virtual void device_start() override;

	device_gg_ext_port_interface *m_device;

private:
	devcb_write_line m_th_pin_handler;
};


// ======================> device_gg_ext_port_interface

// class representing interface-specific live sms_expansion card
class device_gg_ext_port_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_gg_ext_port_interface();

	virtual uint8_t peripheral_r() { return 0xff; }
	virtual void peripheral_w(uint8_t data) { }

protected:
	device_gg_ext_port_interface(const machine_config &mconfig, device_t &device);

	gg_ext_port_device *m_port;
};


// device type definition
DECLARE_DEVICE_TYPE(GG_EXT_PORT, gg_ext_port_device)


void gg_ext_port_devices(device_slot_interface &device);


#endif // MAME_BUS_GAMEGEAR_GGEXT_H
