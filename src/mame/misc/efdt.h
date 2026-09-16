// license:BSD-3-Clause
// copyright-holders: ElSemi, Roberto Fresca.
#ifndef MAME_MISC_EFDT_H
#define MAME_MISC_EFDT_H

#pragma once

#include "machine/74259.h"
#include "emupal.h"
#include "tilemap.h"


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
		m_videoram(*this, "videoram")
	{ }

	void efdt(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<cpu_device> m_audiocpu;
	required_device_array<ls259_device, 2> m_vlatch;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	uint8_t m_soundlatch[4]{};
	uint8_t m_soundCommand = 0U;
	uint8_t m_soundControl = 0U;


	/* video-related */
	tilemap_t      *m_tilemap[2]{};
	int             m_tilebank = 0;

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);

	void efdt_palette(palette_device &palette) const;

	void vblank_nmi_w(int state);
	void nmi_clear_w(int state);

	uint8_t main_soundlatch_r(offs_t offset);
	void main_soundlatch_w(offs_t offset, uint8_t data);

	uint8_t soundlatch_0_r();
	uint8_t soundlatch_1_r();
	uint8_t soundlatch_2_r();
	uint8_t soundlatch_3_r();

	void soundlatch_0_w(uint8_t data);
	void soundlatch_1_w(uint8_t data);
	void soundlatch_2_w(uint8_t data);
	void soundlatch_3_w(uint8_t data);

	uint32_t screen_update_efdt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void efdt_map(address_map &map) ATTR_COLD;
	void efdt_snd_map(address_map &map) ATTR_COLD;
};

#endif // MAME_MISC_EFDT_H
