// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore CBM-II Expansion Port emulation

**********************************************************************

                     A0       1      A       BD0
                     A1       2      B       BD1
                     A2       3      C       BD2
                     A3       4      D       BD3
                     A4       5      E       BD4
                     A5       6      F       BD5
                     A6       7      H       BD6
                     A7       8      J       BD7
                     A8       9      K       GND
                     A9      10      L       GND
                    A10      11      M       SR/_W
                    A11      12      N       Sphi2
                    A12      13      P       _CSBANK1
                    +5V      14      R       _CSBANK2
                    +5V      15      S       _CSBANK2

**********************************************************************/

#ifndef MAME_BUS_CBM2_EXP_H
#define MAME_BUS_CBM2_EXP_H

#pragma once

#include "imagedev/cartrom.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm2_expansion_slot_device

class device_cbm2_expansion_card_interface;

class cbm2_expansion_slot_device : public device_t,
									public device_single_card_slot_interface<device_cbm2_expansion_card_interface>,
									public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	cbm2_expansion_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&opts, char const *dflt)
		: cbm2_expansion_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	cbm2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// computer interface
	uint8_t read(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3);
	void write(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3);

	// cartridge interface
	int phi2() { return clock(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "cbm2_cart"; }
	virtual const char *file_extensions() const noexcept override { return "20,40,60"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	device_cbm2_expansion_card_interface *m_card;
};


// ======================> device_cbm2_expansion_card_interface

class device_cbm2_expansion_card_interface : public device_interface
{
	friend class cbm2_expansion_slot_device;

public:
	// construction/destruction
	virtual ~device_cbm2_expansion_card_interface();

	virtual uint8_t cbm2_bd_r(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3) { return data; }
	virtual void cbm2_bd_w(offs_t offset, uint8_t data, int csbank1, int csbank2, int csbank3) { }

protected:
	device_cbm2_expansion_card_interface(const machine_config &mconfig, device_t &device);

	std::unique_ptr<uint8_t[]> m_bank1;
	std::unique_ptr<uint8_t[]> m_bank2;
	std::unique_ptr<uint8_t[]> m_bank3;

	cbm2_expansion_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(CBM2_EXPANSION_SLOT, cbm2_expansion_slot_device)


void cbm2_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_CBM2_EXP_H
