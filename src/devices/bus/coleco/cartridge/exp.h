// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    ColecoVision cartridge port emulation

**********************************************************************

                     D2       1      2       /C000
                     D1       3      4       D3
                     D0       5      6       D4
                     A0       7      8       D5
                     A1       9      10      D6
                     A2      11      12      D7
                   SHLD      13      14      A11
                     A3      15      16      A10
                     A4      17      18      /8000
                    A13      19      20      A14
                     A5      21      22      /A000
                     A6      23      24      A12
                     A7      25      26      A9
                  /E000      27      28      A8
                    GND      29      30      +5V


**********************************************************************/

#ifndef MAME_BUS_COLECO_EXP_H
#define MAME_BUS_COLECO_EXP_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define COLECOVISION_CARTRIDGE_SLOT_TAG      "cartslot"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> colecovision_cartridge_slot_device

class device_colecovision_cartridge_interface;

class colecovision_cartridge_slot_device : public device_t,
											public device_single_card_slot_interface<device_colecovision_cartridge_interface>,
											public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	colecovision_cartridge_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: colecovision_cartridge_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	colecovision_cartridge_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// computer interface
	uint8_t read(offs_t offset, int _8000, int _a000, int _c000, int _e000);
	void write(offs_t offset, uint8_t data, int _8000, int _a000, int _c000, int _e000);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "coleco_cart"; }
	virtual const char *file_extensions() const noexcept override { return "rom,col,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	device_colecovision_cartridge_interface *m_card;
};


// ======================> device_colecovision_cartridge_interface

class device_colecovision_cartridge_interface : public device_interface
{
	friend class colecovision_cartridge_slot_device;

public:
	virtual uint8_t read(offs_t offset, int _8000, int _a000, int _c000, int _e000) { return 0xff; }
	virtual void write(offs_t offset, uint8_t data, int _8000, int _a000, int _c000, int _e000) { }

	void rom_alloc(size_t size);

protected:
	// construction/destruction
	device_colecovision_cartridge_interface(const machine_config &mconfig, device_t &device);

	uint8_t *m_rom;
	size_t m_rom_size;

	colecovision_cartridge_slot_device *m_slot;
};

// device type definition
DECLARE_DEVICE_TYPE(COLECOVISION_CARTRIDGE_SLOT, colecovision_cartridge_slot_device)

void colecovision_cartridges(device_slot_interface &device);

#endif // MAME_BUS_COLECO_EXP_H
