// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_MSX_SLOT_MUSIC_H
#define MAME_BUS_MSX_SLOT_MUSIC_H

#pragma once

#include "rom.h"
#include "slot.h"
#include "sound/ymopl.h"


DECLARE_DEVICE_TYPE(MSX_SLOT_MUSIC, msx_slot_music_device)


class msx_slot_music_device : public msx_slot_rom_device
{
public:
	msx_slot_music_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	template <class ObjectClass, bool Required>
	void set_ym2413(const device_finder<ObjectClass, Required> &finder) {
		m_ym2413_base = &finder.finder_target().first;
		m_ym2413_tag = finder.finder_target().second;
	}

protected:
	virtual void device_start() override;

private:
	device_t *m_ym2413_base;
	ym2413_device *m_ym2413;
	const char *m_ym2413_tag;
};


#endif // MAME_BUS_MSX_SLOT_MUSIC_H
