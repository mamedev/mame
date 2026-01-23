// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    NEC PC-9801-26 sound card

***************************************************************************/

#ifndef MAME_BUS_PC98_CBUS_SOUND_ORCHESTRA_H
#define MAME_BUS_PC98_CBUS_SOUND_ORCHESTRA_H

#pragma once

#include "slot.h"

#include "pc9801_26.h"
#include "sound/ymopl.h"


class sound_orchestra_device : public pc9801_26_device
{
public:
	// construction/destruction
	sound_orchestra_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void io_map(address_map &map) override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<ym3812_device> m_opl2;

};


DECLARE_DEVICE_TYPE(SOUND_ORCHESTRA, sound_orchestra_device)


#endif // MAME_BUS_PC98_CBUS_SOUND_ORCHESTRA_H
