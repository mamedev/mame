// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Master Compact Joystick/Mouse port

**********************************************************************

    A 9 way 'D' type plug (PL8) is provided for connection to
    external devices. It is compatible with existing 'ATARI' type joysticks
    whose digital outputs are converted by the operating system into suitable
    ADVAL's to emulate BBC analogue joystick operation.

  Pinout: 9 way D-type plug

    +-----------+
    | 1 2 3 4 5 |
     \ 6 7 8 9 /
      +-------+

      1 PB3    6 PB0
      2 PB2    7 +5v
      3 PB1    8 0v
      4 PB4    9 CB2 (LPTSTB if connected)
      5 CB1

**********************************************************************/

#ifndef MAME_BUS_BBC_JOYPORT_JOYPORT_H
#define MAME_BUS_BBC_JOYPORT_JOYPORT_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_joyport_slot_device

class device_bbc_joyport_interface;

class bbc_joyport_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T>
	bbc_joyport_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: bbc_joyport_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_joyport_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto cb1_handler() { return m_cb1_handler.bind(); }
	auto cb2_handler() { return m_cb2_handler.bind(); }

	DECLARE_WRITE_LINE_MEMBER(cb1_w) { m_cb1_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(cb2_w) { m_cb2_handler(state); }

	DECLARE_READ8_MEMBER(pb_r);
	DECLARE_WRITE8_MEMBER(pb_w);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	device_bbc_joyport_interface *m_device;

private:
	devcb_write_line m_cb1_handler;
	devcb_write_line m_cb2_handler;
};


// ======================> device_bbc_joyport_interface

class device_bbc_joyport_interface : public device_slot_card_interface
{
public:
	virtual DECLARE_READ8_MEMBER(pb_r) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(pb_w) { }

protected:
	device_bbc_joyport_interface(const machine_config &mconfig, device_t &device);

	bbc_joyport_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_JOYPORT_SLOT, bbc_joyport_slot_device)

void bbc_joyport_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_JOYPORT_JOYPORT_H
