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
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_BBC_USERPORT_SLOT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, BBC_USERPORT_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_BBC_USERPORT_CB1_HANDLER(_devcb) \
	devcb = &downcast<bbc_userport_slot_device &>(*device).set_cb1_handler(DEVCB_##_devcb);

#define MCFG_BBC_USERPORT_CB2_HANDLER(_devcb) \
	devcb = &downcast<bbc_userport_slot_device &>(*device).set_cb2_handler(DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> bbc_userport_slot_device

class device_bbc_userport_interface;

class bbc_userport_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	bbc_userport_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	template <class Object> devcb_base &set_cb1_handler(Object &&cb) { return m_cb1_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_cb2_handler(Object &&cb) { return m_cb2_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER(cb1_w) { m_cb1_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(cb2_w) { m_cb2_handler(state); }

	DECLARE_READ8_MEMBER(pb_r);
	DECLARE_WRITE8_MEMBER(pb_w);

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

	virtual DECLARE_READ8_MEMBER(pb_r) { return 0xff; }
	virtual DECLARE_WRITE8_MEMBER(pb_w) { }

protected:
	device_bbc_userport_interface(const machine_config &mconfig, device_t &device);

	bbc_userport_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_USERPORT_SLOT, bbc_userport_slot_device)


void bbc_userport_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_USERPORT_USERPORT_H
