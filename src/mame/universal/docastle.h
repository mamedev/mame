// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  Mr. Do's Castle hardware

***************************************************************************/
#ifndef MAME_UNIVERSAL_DOCASTLE_H
#define MAME_UNIVERSAL_DOCASTLE_H

#pragma once

#include "machine/tms1024.h"
#include "video/mc6845.h"
#include "sound/msm5205.h"
#include "sound/sn76496.h"

#include "emupal.h"
#include "tilemap.h"

class docastle_state : public driver_device
{
public:
	docastle_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_spritecpu(*this, "spritecpu"),
		m_crtc(*this, "crtc"),
		m_sn(*this, "sn%u", 1U),
		m_msm(*this, "msm"),
		m_inp(*this, "inp%u", 1),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void dorunrun(machine_config &config);
	void idsoccer(machine_config &config);
	void docastle(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_spritecpu;
	required_device<hd6845s_device> m_crtc;
	required_device_array<sn76489a_device, 4> m_sn;
	optional_device<msm5205_device> m_msm;
	required_device_array<tms1025_device, 2> m_inp;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// misc
	int m_prev_ma6 = 0;
	int m_adpcm_pos = 0;
	int m_adpcm_idle = 0;
	int m_adpcm_data = 0;
	int m_adpcm_status = 0;

	uint8_t m_shared_latch = 0;
	bool m_maincpu_defer_access = false;
	bool m_subcpu_defer_access = false;

	tilemap_t *m_do_tilemap = nullptr;

	uint8_t main_from_sub_r();
	void main_to_sub_w(uint8_t data);
	uint8_t sub_from_main_r();
	void sub_to_main_w(uint8_t data);
	void subcpu_nmi_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	uint8_t inputs_flipscreen_r(offs_t offset);
	void flipscreen_w(offs_t offset, uint8_t data);
	uint8_t idsoccer_adpcm_status_r();
	void idsoccer_adpcm_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	void docastle_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(dorunrun);
	uint32_t screen_update_docastle(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void video_start_common(uint32_t tile_transmask);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void docastle_tint(int state);
	void idsoccer_adpcm_int(int state);

	void docastle_io_map(address_map &map) ATTR_COLD;
	void docastle_map(address_map &map) ATTR_COLD;
	void docastle_map2(address_map &map) ATTR_COLD;
	void docastle_map3(address_map &map) ATTR_COLD;
	void dorunrun_map(address_map &map) ATTR_COLD;
	void dorunrun_map2(address_map &map) ATTR_COLD;
	void idsoccer_map(address_map &map) ATTR_COLD;
};

#endif // MAME_UNIVERSAL_DOCASTLE_H
