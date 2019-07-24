// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

  Mr. Do's Castle hardware

***************************************************************************/
#ifndef MAME_INCLUDES_DOCASTLE_H
#define MAME_INCLUDES_DOCASTLE_H

#pragma once

#include "machine/tms1024.h"
#include "video/mc6845.h"
#include "sound/msm5205.h"
#include "emupal.h"

class docastle_state : public driver_device
{
public:
	docastle_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_slave(*this, "slave"),
		m_cpu3(*this, "cpu3"),
		m_crtc(*this, "crtc"),
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
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_slave;
	required_device<cpu_device> m_cpu3;
	required_device<hd6845s_device> m_crtc;
	optional_device<msm5205_device> m_msm;
	required_device_array<tms1025_device, 2> m_inp;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_do_tilemap;

	/* misc */
	int      m_prev_ma6;
	int      m_adpcm_pos;
	int      m_adpcm_idle;
	int      m_adpcm_data;
	int      m_adpcm_status;
	uint8_t    m_buffer0[9];
	uint8_t    m_buffer1[9];

	DECLARE_READ8_MEMBER(docastle_shared0_r);
	DECLARE_READ8_MEMBER(docastle_shared1_r);
	DECLARE_WRITE8_MEMBER(docastle_shared0_w);
	DECLARE_WRITE8_MEMBER(docastle_shared1_w);
	DECLARE_WRITE8_MEMBER(docastle_nmitrigger_w);
	DECLARE_WRITE8_MEMBER(docastle_videoram_w);
	DECLARE_WRITE8_MEMBER(docastle_colorram_w);
	DECLARE_READ8_MEMBER(inputs_flipscreen_r);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_READ8_MEMBER(idsoccer_adpcm_status_r);
	DECLARE_WRITE8_MEMBER(idsoccer_adpcm_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void docastle_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(dorunrun);
	uint32_t screen_update_docastle(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void video_start_common( uint32_t tile_transmask );
	void draw_sprites( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE_LINE_MEMBER(docastle_tint);
	DECLARE_WRITE_LINE_MEMBER(idsoccer_adpcm_int);
	void docastle_io_map(address_map &map);
	void docastle_map(address_map &map);
	void docastle_map2(address_map &map);
	void docastle_map3(address_map &map);
	void dorunrun_map(address_map &map);
	void dorunrun_map2(address_map &map);
	void idsoccer_map(address_map &map);
};

#endif // MAME_INCLUDES_DOCASTLE_H
