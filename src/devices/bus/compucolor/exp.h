// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Compucolor II I/O expansion slot

**********************************************************************/

#ifndef MAME_BUS_COMPUCOLOR_EXP_H
#define MAME_BUS_COMPUCOLOR_EXP_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_compucolor_expansion_card_interface;


// ======================> compucolor_expansion_slot_device

class compucolor_expansion_slot_device : public device_t, public device_single_card_slot_interface<device_compucolor_expansion_card_interface>
{
	friend class device_compucolor_expansion_card_interface;

public:
	// construction/destruction
	template <typename T>
	compucolor_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&opts, const char *dflt)
		: compucolor_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	compucolor_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	required_address_space m_io;
};


// ======================> device_compucolor_expansion_card_interface

class device_compucolor_expansion_card_interface : public device_interface
{
protected:
	// construction/destruction
	device_compucolor_expansion_card_interface(const machine_config &mconfig, device_t &device);

	address_space &io_space() { return *m_slot->m_io; }

	compucolor_expansion_slot_device *m_slot;
};


// device type declaration
DECLARE_DEVICE_TYPE(COMPUCOLOR_EXPANSION_SLOT, compucolor_expansion_slot_device)


void compucolor_expansion_cards(device_slot_interface &device) ATTR_COLD;


#endif // MAME_BUS_COMPUCOLOR_EXP_H
