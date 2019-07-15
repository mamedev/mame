// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC User Port emulation

**********************************************************************

  Pinout:

            +5V   1   2  CB1
            +5V   3   4  CB2
             0V   5   6  PB0
             0V   7   8  PB1
             0V   9  10  PB2
             0V  11  12  PB3
             0V  13  14  PB4
             0V  15  16  PB5
             0V  17  18  PB6
             0V  19  20  PB7

  Signal Definitions:

  Connected directly to the 6522 User VIA.

**********************************************************************/

#ifndef MAME_BUS_BBC_USERPORT_USERPORT_H
#define MAME_BUS_BBC_USERPORT_USERPORT_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> bbc_userport_slot_device

class device_bbc_userport_interface;

class bbc_userport_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	bbc_userport_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: bbc_userport_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_userport_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto cb1_handler() { return m_cb1_handler.bind(); }
	auto cb2_handler() { return m_cb2_handler.bind(); }

	DECLARE_WRITE_LINE_MEMBER(cb1_w) { m_cb1_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(cb2_w) { m_cb2_handler(state); }

	uint8_t pb_r();
	void pb_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	device_bbc_userport_interface *m_device;

private:
	devcb_write_line m_cb1_handler;
	devcb_write_line m_cb2_handler;
};


// ======================> device_bbc_userport_interface

class device_bbc_userport_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_bbc_userport_interface();

	virtual uint8_t pb_r() { return 0xff; }
	virtual void pb_w(uint8_t data) { }

protected:
	device_bbc_userport_interface(const machine_config &mconfig, device_t &device);

	bbc_userport_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_USERPORT_SLOT, bbc_userport_slot_device)


void bbc_userport_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_USERPORT_USERPORT_H
