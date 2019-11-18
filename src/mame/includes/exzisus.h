// license:BSD-3-Clause
// copyright-holders:Yochizo
#ifndef MAME_INCLUDES_EXZIUS_H
#define MAME_INCLUDES_EXZIUS_H

#pragma once

#include "emupal.h"

class exzisus_state : public driver_device
{
public:
	exzisus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_cpuc(*this, "cpuc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_objectram1(*this, "objectram1"),
		m_videoram1(*this, "videoram1"),
		m_sharedram_ac(*this, "sharedram_ac"),
		m_sharedram_ab(*this, "sharedram_ab"),
		m_objectram0(*this, "objectram0"),
		m_videoram0(*this, "videoram0")
	{ }

	void exzisus(machine_config &config);

private:
	required_device<cpu_device> m_cpuc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_objectram1;
	required_shared_ptr<uint8_t> m_videoram1;
	required_shared_ptr<uint8_t> m_sharedram_ac;
	required_shared_ptr<uint8_t> m_sharedram_ab;
	required_shared_ptr<uint8_t> m_objectram0;
	required_shared_ptr<uint8_t> m_videoram0;

	DECLARE_WRITE8_MEMBER(cpua_bankswitch_w);
	DECLARE_WRITE8_MEMBER(cpub_bankswitch_w);
	DECLARE_WRITE8_MEMBER(coincounter_w);
	DECLARE_WRITE8_MEMBER(cpub_reset_w);

	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cpua_map(address_map &map);
	void cpub_map(address_map &map);
	void cpuc_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_EXZIUS_H
