// license:BSD-3-Clause
// copyright-holders:Paul Hampson, Nicola Salmoria
/*************************************************************************

    Super Dodge Ball hardware

*************************************************************************/
#ifndef MAME_INCLUDES_SPDODGEB_H
#define MAME_INCLUDES_SPDODGEB_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class spdodgeb_state : public driver_device
{
public:
	spdodgeb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram")
	{ }

	void spdodgeb(machine_config &config);

	DECLARE_READ_LINE_MEMBER(mcu_busy_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_mcu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;

	int m_adpcm_pos[2]{};
	int m_adpcm_end[2]{};
	int m_adpcm_idle[2]{};
	int m_adpcm_data[2]{};
	uint8_t m_mcu_status = 0;
	uint8_t m_inputs[5]{};

	int m_tile_palbank = 0;
	int m_sprite_palbank = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	int m_lastscroll = 0;

	void spd_adpcm_w(offs_t offset, uint8_t data);
	uint8_t mcu63701_r(offs_t offset);
	void mcu_data_w(offs_t offset, uint8_t data);
	void mcu_status_w(uint8_t data);
	void mcu_nmi_w(uint8_t data);

	void scrollx_lo_w(uint8_t data);
	void ctrl_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(spd_adpcm_int_1);
	DECLARE_WRITE_LINE_MEMBER(spd_adpcm_int_2);

	TILEMAP_MAPPER_MEMBER(background_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void spdodgeb_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);

	void spd_adpcm_int(msm5205_device *device, int chip);
	void spdodgeb_map(address_map &map);
	void spdodgeb_sound_map(address_map &map);
	void mcu_map(address_map &map);
};

#endif // MAME_INCLUDES_SPDODGEB_H
