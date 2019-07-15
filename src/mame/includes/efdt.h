// license:BSD-3-Clause
// copyright-holders: ElSemi, Roberto Fresca.
#ifndef MAME_INCLUDES_EFDT_H
#define MAME_INCLUDES_EFDT_H

#pragma once

#include "machine/74259.h"
#include "emupal.h"


class efdt_state : public driver_device
{
public:
	efdt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_audiocpu(*this, "audiocpu"),
		m_vlatch(*this, "vlatch%u", 1U),
		m_videoram(*this, "videoram", 8)
	{ }

	void efdt(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<cpu_device> m_audiocpu;
	required_device_array<ls259_device, 2> m_vlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	uint8_t m_soundlatch[4];
	uint8_t m_soundCommand;
	uint8_t m_soundControl;


	/* video-related */
	tilemap_t      *m_tilemap[2];
	int             m_tilebank;

	DECLARE_WRITE8_MEMBER(efdt_videoram_w);
	DECLARE_WRITE8_MEMBER(efdt_vregs1_w);
	DECLARE_WRITE8_MEMBER(efdt_vregs2_w);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);

	void efdt_palette(palette_device &palette) const;

	DECLARE_WRITE_LINE_MEMBER(vblank_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_clear_w);

	DECLARE_READ8_MEMBER(main_soundlatch_r);
	DECLARE_WRITE8_MEMBER(main_soundlatch_w);

	DECLARE_READ8_MEMBER(soundlatch_0_r);
	DECLARE_READ8_MEMBER(soundlatch_1_r);
	DECLARE_READ8_MEMBER(soundlatch_2_r);
	DECLARE_READ8_MEMBER(soundlatch_3_r);

	DECLARE_WRITE8_MEMBER(soundlatch_0_w);
	DECLARE_WRITE8_MEMBER(soundlatch_1_w);
	DECLARE_WRITE8_MEMBER(soundlatch_2_w);
	DECLARE_WRITE8_MEMBER(soundlatch_3_w);

	uint32_t screen_update_efdt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void efdt_map(address_map &map);
	void efdt_snd_map(address_map &map);
};

#endif // MAME_INCLUDES_EFDT_H
