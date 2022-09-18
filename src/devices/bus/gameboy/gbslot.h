// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Game Boy cartridge slot

 Cartridge edge connector (0.05" pitch single-sided):
 Pin   Name     Description
   1   VCC      +5 V DC power
   2   PHI      CPU clock output
   3   /WR      write strobe
   4   /RD      read strobe
   5   /CS      RAM chip select (active for address 0xa000-0xdfff)
   6   A0       address bit 0 (least significant)
   7   A1       address bit 1
   8   A2       address bit 2
   9   A3       address bit 3
  10   A4       address bit 4
  11   A5       address bit 5
  12   A6       address bit 6
  13   A7       address bit 7
  14   A8       address bit 8
  15   A9       address bit 9
  16   A10      address bit 10
  17   A11      address bit 11
  18   A12      address bit 12
  19   A13      address bit 13
  20   A14      address bit 14
  21   A15      address bit 15 (most significant)
  22   D0       data bit 0 (least significant)
  23   D1       data bit 1
  24   D2       data bit 2
  25   D3       data bit 3
  26   D4       data bit 4
  27   D5       data bit 5
  28   D6       data bit 6
  29   D7       data bit 7 (most significant)
  30   /RST     system reset
  31   AUDIOIN  connected to SoC VIN pin (only used by some music players)
  32   GND      Ground

 ***************************************************************************/
#ifndef MAME_BUS_GAMEBOY_GBSLOT_H
#define MAME_BUS_GAMEBOY_GBSLOT_H

#pragma once

#include "slot.h"

#include <string>


class gb_cart_slot_device : public gb_cart_slot_device_base
{
public:
	template <typename T>
	gb_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt) :
		gb_cart_slot_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	gb_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_image_interface implementation
	virtual const char *image_interface() const noexcept override { return "gameboy_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,gb,gbc,gbx"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_reset_after_children() override ATTR_COLD;

	// gb_cart_slot_device_base implementation
	virtual image_init_result load_image_file(util::random_read &file) override ATTR_COLD;

	void allocate_cart_ram(u8 const *basepage) ATTR_COLD;
};


DECLARE_DEVICE_TYPE(GB_CART_SLOT, gb_cart_slot_device)

#endif // MAME_BUS_GAMEBOY_GBSLOT_H
