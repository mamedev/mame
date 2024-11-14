// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Expansion Port emulation

**********************************************************************

                    GND       1      2       BO2
                    BO1       3      4       BO0
                    BA0       5      6       BA1
                    BA2       7      8       BA3
                    BA4       9      10      BA5
                    BA6      11      12      BA7
                    +5V      13      14      BA8
                    BA9      15      16      _CS1
                   BA10      17      18      BO7
                    BO6      19      20      BO5
                    BO4      21      22      BO3
                   BA11      23      24      BA12
                    BRC      25      26      ?
                 EXTRES      27      28      _CS2
                      ?      29      30      UV202/35
                 HBLANK      31      32      UV201/5
                UV201/2      33      34      UV201/4
                UV201/3      35      36      BISTROBE
                UV202/1      37      38      BA13
               UV202/18      39      40      ?
                    R/W      41      42      +5V
                    GND      43      44      GND
                    GND      45      46      N/C
                      ?      47      48      N/C
                    N/C      49      50      N/C

**********************************************************************/

#ifndef MAME_BUS_VIDBRAIN_EXP_H
#define MAME_BUS_VIDBRAIN_EXP_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class videobrain_expansion_slot_device;

// ======================> device_videobrain_expansion_card_interface

class device_videobrain_expansion_card_interface : public device_interface
{
	friend class videobrain_expansion_slot_device;

public:
	// construction/destruction
	virtual ~device_videobrain_expansion_card_interface() { }

protected:
	device_videobrain_expansion_card_interface(const machine_config &mconfig, device_t &device);

	// initialization
	virtual uint8_t* videobrain_rom_pointer(running_machine &machine, size_t size);
	virtual uint8_t* videobrain_ram_pointer(running_machine &machine, size_t size);

	// runtime
	virtual uint8_t videobrain_bo_r(offs_t offset, int cs1, int cs2) { return 0; }
	virtual void videobrain_bo_w(offs_t offset, uint8_t data, int cs1, int cs2) { }
	virtual void videobrain_extres_w() { }

	videobrain_expansion_slot_device *m_slot;

	std::vector<uint8_t> m_rom;
	std::vector<uint8_t> m_ram;

	size_t m_rom_mask;
	size_t m_ram_mask;
};


// ======================> videobrain_expansion_slot_device

class videobrain_expansion_slot_device : public device_t,
											public device_single_card_slot_interface<device_videobrain_expansion_card_interface>,
											public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	videobrain_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, char const* dflt) :
		videobrain_expansion_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	videobrain_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto extres_wr_callback() { return m_write_extres.bind(); }

	// computer interface
	uint8_t bo_r(offs_t offset, int cs1, int cs2) { return m_cart ? m_cart->videobrain_bo_r(offset, cs1, cs2) : 0; }
	void bo_w(offs_t offset, uint8_t data, int cs1, int cs2) { if (m_cart) m_cart->videobrain_bo_w(offset, data, cs1, cs2); }

	uint8_t cs1_r(offs_t offset) { return bo_r(offset + 0x1000, 0, 1); }
	void cs1_w(offs_t offset, uint8_t data) { bo_w(offset + 0x1000, data, 0, 1); }
	uint8_t cs2_r(offs_t offset) { return bo_r(offset + 0x1800, 1, 0); }
	void cs2_w(offs_t offset, uint8_t data) { bo_w(offset + 0x1800, data, 1, 0); }
	uint8_t unmap_r(offs_t offset) { return bo_r(offset + 0x3000, 1, 1); }
	void unmap_w(offs_t offset, uint8_t data) { bo_w(offset + 0x3000, data, 1, 1); }

	// cartridge interface
	void extres_w(int state) { m_write_extres(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "vidbrain_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	devcb_write_line m_write_extres;

	device_videobrain_expansion_card_interface *m_cart;
};


// device type definition
DECLARE_DEVICE_TYPE(VIDEOBRAIN_EXPANSION_SLOT, videobrain_expansion_slot_device)


void vidbrain_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_VIDBRAIN_EXP_H
