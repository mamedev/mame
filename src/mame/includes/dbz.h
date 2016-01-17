// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont, Pierpaolo Prazzoli
/*************************************************************************

    Dragonball Z

*************************************************************************/

#include "machine/k053252.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053936.h"
#include "video/k053251.h"
#include "video/konami_helper.h"

class dbz_state : public driver_device
{
public:
	dbz_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
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
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_bg1_videoram;
	required_shared_ptr<UINT16> m_bg2_videoram;

	/* video-related */
	tilemap_t    *m_bg1_tilemap;
	tilemap_t    *m_bg2_tilemap;
	int          m_layer_colorbase[6];
	int          m_layerpri[5];
	int          m_sprite_colorbase;

	/* misc */
	int           m_control;

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

	DECLARE_WRITE16_MEMBER(dbzcontrol_w);
	DECLARE_WRITE16_MEMBER(dbz_sound_command_w);
	DECLARE_WRITE16_MEMBER(dbz_sound_cause_nmi);
	DECLARE_WRITE16_MEMBER(dbz_bg2_videoram_w);
	DECLARE_WRITE16_MEMBER(dbz_bg1_videoram_w);
	DECLARE_WRITE_LINE_MEMBER(dbz_irq2_ack_w);
	DECLARE_DRIVER_INIT(dbza);
	DECLARE_DRIVER_INIT(dbz);
	DECLARE_DRIVER_INIT(dbz2);
	TILE_GET_INFO_MEMBER(get_dbz_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_dbz_bg1_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_dbz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(dbz_scanline);
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
};
