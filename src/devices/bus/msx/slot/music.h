// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_MUSIC_H
#define MAME_BUS_MSX_SLOT_MUSIC_H

#pragma once

#include "rom.h"
#include "slot.h"

#include "sound/ymopl.h"

#include <utility>


DECLARE_DEVICE_TYPE(MSX_SLOT_MUSIC, msx_slot_music_device)


class msx_slot_music_device : public msx_slot_rom_device
{
public:
	msx_slot_music_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_ym2413_tag(T &&tag) { m_ym2413.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	required_device<ym2413_device> m_ym2413;
};


#endif // MAME_BUS_MSX_SLOT_MUSIC_H
