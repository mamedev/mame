// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Mega Duck cartridge slot

 Cartridge edge connector (0.1" pitch double-sided):
  VCC    1    2
  /WR    3    4   /RD
         5    6   A0
   A1    7    8   A2
   A3    9   10   A4
   A5   11   12   A6
   A7   13   14   A8
   A9   15   16   A10
  A11   17   18   A12
  A13   19   20   A14
  A15   21   22   D0
   D1   23   24   D2
   D3   25   26   D4
   D5   27   28   D6
   D7   29   30   /RST
        31   32
        33   34
  GND   35   36   GND

 Pinout based on cartridge connections.  Unlabelled contacts may be
 connected in console.

 ***************************************************************************/
#ifndef MAME_BUS_GAMEBOY_MDSLOT_H
#define MAME_BUS_GAMEBOY_MDSLOT_H

#pragma once

#include "slot.h"

#include <string>


class megaduck_cart_slot_device : public gb_cart_slot_device_base
{
public:
	template <typename T>
	megaduck_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt) :
		megaduck_cart_slot_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	megaduck_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device_image_interface implementation
	virtual char const *image_interface() const noexcept override { return "megaduck_cart"; }
	virtual char const *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override ATTR_COLD;

protected:
	// gb_cart_slot_device_base implementation
	virtual image_init_result load_image_file(util::random_read &file) override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(MEGADUCK_CART_SLOT, megaduck_cart_slot_device)

#endif // MAME_BUS_GAMEBOY_MDSLOT_H
