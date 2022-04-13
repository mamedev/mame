// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_MJKJIDAI_H
#define MAME_INCLUDES_MJKJIDAI_H

#pragma once

#include "machine/nvram.h"
#include "sound/msm5205.h"
#include "emupal.h"
#include "tilemap.h"

class mjkjidai_state : public driver_device
{
public:
	mjkjidai_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_nvram(*this, "nvram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_adpcmrom(*this, "adpcm"),
		m_videoram(*this, "videoram"),
		m_mainbank(*this, "mainbank"),
		m_row(*this, "ROW.%u", 0)
	{ }

	void mjkjidai(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(keyboard_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<nvram_device> m_nvram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_region_ptr<uint8_t> m_adpcmrom;
	required_shared_ptr<uint8_t> m_videoram;
	required_memory_bank m_mainbank;

	required_ioport_array<12> m_row;

	int m_adpcm_pos = 0;
	int m_adpcm_end = 0;
	int m_keyb = 0;
	bool m_nmi_enable = false;
	bool m_display_enable = false;
	tilemap_t *m_bg_tilemap = nullptr;

	void keyboard_select_lo_w(uint8_t data);
	void keyboard_select_hi_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void ctrl_w(uint8_t data);
	void adpcm_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map);
	void prg_map(address_map &map);
};

#endif // MAME_INCLUDES_MJKJIDAI_H
