// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

 MSX Yamaha module slot

 Cartridge edge connector (double-sided):
          GND -  2  1 - SOUND OUT
Phase control -  4  3 - GND
          B-Y -  6  5 - Y
          R-Y -  8  7 - C VIDEO
CLOCK INT/EXT - 10  9 - EXT CLOCK
         /CS2 - 12 11 - /CS1
       /SLTSL - 14 13 - /CS12
        /RFSH - 16 15 - reserved
         /INT - 18 17 - /WAIT
      /BUSDIR - 20 19 - /M1
        /MREQ - 22 21 - /IORQ
          /RD - 24 23 - /WR
     reserved - 26 25 - /RESET
          A15 - 28 27 - A9
          A10 - 30 29 - A11
           A6 - 32 31 - A7
           A8 - 34 33 - A12
          A13 - 36 35 - A14
           A0 - 38 37 - A1
           A2 - 40 39 - A3
           A4 - 42 41 - A5
           D0 - 44 43 - D1
           D2 - 46 45 - D3
           D4 - 48 47 - D5
           D6 - 50 49 - D7
        CLOCK - 52 51 - GND
          SW1 - 54 53 - GND
          SW2 - 56 55 - +5V
         +12V - 58 57 - +5V
         -12V - 60 59 - SOUNDIN

 ***************************************************************************/
#ifndef MAME_BUS_MSX_MODULE_MODULE_H
#define MAME_BUS_MSX_MODULE_MODULE_H

#pragma once

#include "bus/msx/slot/cartridge.h"


void msx_yamaha_60pin(device_slot_interface &device, bool is_in_subslot);   // 60 pin expansion slots as found in yamaha machines
DECLARE_DEVICE_TYPE(MSX_SLOT_YAMAHA_EXPANSION, msx_slot_yamaha_expansion_device)


class msx_slot_yamaha_expansion_device : public msx_slot_cartridge_base_device
{
public:
	msx_slot_yamaha_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual const char *image_interface() const noexcept override { return "msx_yamaha_60pin"; }
	virtual const char *image_type_name() const noexcept override { return "cartridge60pin"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cart60p"; }
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

protected:
	virtual void device_start() override ATTR_COLD;
};

#endif // MAME_BUS_MSX_MODULE_MODULE_H
