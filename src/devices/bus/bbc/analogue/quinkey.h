// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Microwriter Quinkey

**********************************************************************/

#ifndef MAME_BUS_BBC_ANALOGUE_QUINKEY_H
#define MAME_BUS_BBC_ANALOGUE_QUINKEY_H

#pragma once

#include "analogue.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_quinkey_intf_device

class bbc_quinkey_slot_device;

class bbc_quinkey_intf_device : public device_t, public device_bbc_analogue_interface
{
public:
	// construction/destruction
	bbc_quinkey_intf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t ch_r(int channel) override;

private:
	required_device_array<bbc_quinkey_slot_device, 4> m_quinkey;
};


// ======================> bbc_quinkey_slot_device

class device_bbc_quinkey_interface;

class bbc_quinkey_slot_device : public device_t, public device_single_card_slot_interface<device_bbc_quinkey_interface>
{
public:
	// construction/destruction
	template <typename T>
	bbc_quinkey_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: bbc_quinkey_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_quinkey_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	uint8_t read();

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	device_bbc_quinkey_interface *m_card;
};


// ======================> device_bbc_quinkey_interface

class device_bbc_quinkey_interface : public device_interface
{
public:
	virtual uint8_t read() { return 0x00; }

protected:
	device_bbc_quinkey_interface(const machine_config &mconfig, device_t &device);

	bbc_quinkey_slot_device *const m_slot;
};


// ======================> bbc_quinkey_device

class bbc_quinkey_device : public device_t, public device_bbc_quinkey_interface
{
public:
	// construction/destruction
	bbc_quinkey_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read() override;

private:
	required_ioport m_keys;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_QUINKEY_INTF, bbc_quinkey_intf_device)
DECLARE_DEVICE_TYPE(BBC_QUINKEY_SLOT, bbc_quinkey_slot_device)
DECLARE_DEVICE_TYPE(BBC_QUINKEY,      bbc_quinkey_device)

void bbc_quinkey_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_ANALOGUE_QUINKEY_H
