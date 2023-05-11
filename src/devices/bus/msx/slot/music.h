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
	void set_ym2413_tag(const char *tag) { m_ym2413_tag = tag; }

protected:
	virtual void device_start() override;

private:
	ym2413_device *m_ym2413;
	const char *m_ym2413_tag;
};


#endif // MAME_BUS_MSX_SLOT_MUSIC_H
