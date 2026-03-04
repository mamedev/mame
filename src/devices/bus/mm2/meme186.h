// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 2 MEME186 emulation

*********************************************************************/

#ifndef MAME_BUS_MM2_MEME186_H
#define MAME_BUS_MM2_MEME186_H

#pragma once

#include "exp.h"

DECLARE_DEVICE_TYPE(NOKIA_MEME186, meme186_device)

class meme186_device : public device_t, public device_mikromikko2_expansion_bus_card_interface
{
public:
	meme186_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	memory_share_creator<uint16_t> m_ram;
};

#endif // MAME_BUS_MM2_MEME186_H
