// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Sega SG-1000 expansion slot emulation

**********************************************************************


**********************************************************************/

#ifndef MAME_BUS_SG1000_EXP_SG1000EXP_H
#define MAME_BUS_SG1000_EXP_SG1000EXP_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sg1000_expansion_slot_device

class device_sg1000_expansion_slot_interface;

class sg1000_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_sg1000_expansion_slot_interface>
{
public:
	// construction/destruction
	template <typename T>
	sg1000_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt, bool const fixed)
		: sg1000_expansion_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
	}

	sg1000_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~sg1000_expansion_slot_device();

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	bool is_readable(uint8_t offset);
	bool is_writeable(uint8_t offset);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	device_sg1000_expansion_slot_interface *m_device;
};


// ======================> device_sg1000_expansion_slot_interface

// class representing interface-specific live sg1000_expansion card
class device_sg1000_expansion_slot_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_sg1000_expansion_slot_interface();

	virtual uint8_t peripheral_r(offs_t offset) { return 0xff; }
	virtual void peripheral_w(offs_t offset, uint8_t data) { }

	virtual bool is_readable(uint8_t offset) { return true; }
	virtual bool is_writeable(uint8_t offset) { return true; }

protected:
	device_sg1000_expansion_slot_interface(const machine_config &mconfig, device_t &device);

	sg1000_expansion_slot_device *m_port;
};


// device type definition
DECLARE_DEVICE_TYPE(SG1000_EXPANSION_SLOT, sg1000_expansion_slot_device)


void sg1000_expansion_devices(device_slot_interface &device);


#endif // MAME_BUS_SG1000_EXP_SG1000EXP_H
