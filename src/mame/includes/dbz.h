// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont, Pierpaolo Prazzoli
/*************************************************************************

    Dragon Ball Z

*************************************************************************/
#ifndef MAME_INCLUDES_DBZ_H
#define MAME_INCLUDES_DBZ_H

#pragma once

#include "machine/k053252.h"
#include "machine/timer.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053936.h"
#include "video/k053251.h"
#include "video/konami_helper.h"
#include "tilemap.h"

class dbz_state : public driver_device
{
public:
	dbz_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg1_videoram(*this, "bg1_videoram"),
		m_bg2_videoram(*this, "bg2_videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_k056832(*this, "k056832"),
		m_k053936_1(*this, "k053936_1"),
		m_k053936_2(*this, "k053936_2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_dsw2(*this, "DSW2")
	{ }

	void dbz(machine_config &config);
	void dbz2bl(machine_config &config);

	void init_dbza();
	void init_dbz();
	void init_dbz2();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg1_videoram;
	required_shared_ptr<uint16_t> m_bg2_videoram;

	/* video-related */
	tilemap_t    *m_bg1_tilemap = nullptr;
	tilemap_t    *m_bg2_tilemap = nullptr;
	int          m_layer_colorbase[6]{};
	int          m_layerpri[5]{};
	int          m_sprite_colorbase = 0;

	/* misc */
	int           m_control = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	required_device<k053252_device> m_k053252;
	required_device<k056832_device> m_k056832;
	required_device<k053936_device> m_k053936_1;
	required_device<k053936_device> m_k053936_2;
	required_device<gfxdecode_device> m_gfxdecode;

	required_ioport m_dsw2;

	void dbzcontrol_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dbz_sound_cause_nmi(uint16_t data);
	void dbz_bg2_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dbz_bg1_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	DECLARE_WRITE_LINE_MEMBER(dbz_irq2_ack_w);
	TILE_GET_INFO_MEMBER(get_dbz_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_dbz_bg1_tile_info);
	uint32_t screen_update_dbz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(dbz_scanline);
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
	void dbz_map(address_map &map);
	void dbz2bl_map(address_map &map);
	void dbz_sound_io_map(address_map &map);
	void dbz_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_DBZ_H
