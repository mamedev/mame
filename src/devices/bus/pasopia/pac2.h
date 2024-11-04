// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Toshiba Pasopia PAC2 slot

**********************************************************************/

#ifndef MAME_BUS_PASOPIA_PAC2_H
#define MAME_BUS_PASOPIA_PAC2_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class pac2_card_interface;

// ======================> pasopia_pac2_slot_device

class pasopia_pac2_slot_device : public device_t, public device_single_card_slot_interface<pac2_card_interface>
{
	friend class pac2_card_interface;

public:
	// construction/destruction
	template <typename T>
	pasopia_pac2_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: pasopia_pac2_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	pasopia_pac2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// memory accesses
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-specific overrides
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	pac2_card_interface *m_card;
};


// ======================> pac2_card_interface

class pac2_card_interface : public device_interface
{
	friend class pasopia_pac2_slot_device;

protected:
	// construction/destruction
	pac2_card_interface(const machine_config &mconfig, device_t &device);

	// required overrides
	virtual u8 pac2_read(offs_t offset) = 0;
	virtual void pac2_write(offs_t offset, u8 data) = 0;

private:
	virtual void interface_pre_start() override;

	pasopia_pac2_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(PASOPIA_PAC2, pasopia_pac2_slot_device)

void pac2_subslot_devices(device_slot_interface &device);
void pac2_default_devices(device_slot_interface &device);

#endif // MAME_BUS_PASOPIA_PAC2_H
