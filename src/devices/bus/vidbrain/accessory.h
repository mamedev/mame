// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Accessory Jack emulation

**********************************************************************

                    I/O-16       1
                       GND       2
                     RESET       3
                       GND       4
                   EXT INT       5
                    I/O-15       6
                 SOUND OUT       7
                       +5V       8
                       GND       9

**********************************************************************/

#ifndef MAME_BUS_VIDBRAIN_ACCESSORY_H
#define MAME_BUS_VIDBRAIN_ACCESSORY_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class videobrain_accessory_port_device;

// ======================> device_videobrain_accessory_interface

class device_videobrain_accessory_interface : public device_interface
{
	friend class videobrain_accessory_port_device;

protected:
	device_videobrain_accessory_interface(const machine_config &mconfig, device_t &device);

	// I/O lines are active low wired-AND, bit 0 is I/O-15 and bit 1 is I/O-16 (1 = pulled low)
	virtual void videobrain_io_w(uint8_t data);
	virtual uint8_t videobrain_io_r();
	virtual void videobrain_reset_w(int state);

	videobrain_accessory_port_device *m_port;
};


// ======================> videobrain_accessory_port_device

class videobrain_accessory_port_device : public device_t,
										 public device_single_card_slot_interface<device_videobrain_accessory_interface>
{
public:
	// construction/destruction
	template <typename T>
	videobrain_accessory_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, char const *dflt) :
		videobrain_accessory_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	videobrain_accessory_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto ext_int_callback() { return m_write_ext_int.bind(); }

	// computer interface
	void io_w(uint8_t data) { if (m_card) m_card->videobrain_io_w(data); }
	uint8_t io_r() { return m_card ? m_card->videobrain_io_r() : 0; }
	void reset_w(int state) { if (m_card) m_card->videobrain_reset_w(state); }

	// accessory interface
	void ext_int_w(int state) { m_write_ext_int(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	devcb_write_line m_write_ext_int;

	device_videobrain_accessory_interface *m_card;
};


// device type declaration
DECLARE_DEVICE_TYPE(VIDEOBRAIN_ACCESSORY_PORT, videobrain_accessory_port_device)


void videobrain_accessories(device_slot_interface &device);

#endif // MAME_BUS_VIDBRAIN_ACCESSORY_H
