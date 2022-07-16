// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
#ifndef MAME_INCLUDES_WIPING_H
#define MAME_INCLUDES_WIPING_H

#pragma once

#include "emupal.h"

class wiping_state : public driver_device
{
public:
	wiping_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram")
	{ }

	void wiping(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_flipscreen = 0;
	uint8_t *m_soundregs = nullptr;  // if 0-ed
	uint8_t m_main_irq_mask = 0;
	uint8_t m_sound_irq_mask = 0;

	uint8_t ports_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(main_irq_mask_w);
	DECLARE_WRITE_LINE_MEMBER(sound_irq_mask_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);

	void wiping_palette(palette_device &palette) const;
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);

	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_WIPING_H
