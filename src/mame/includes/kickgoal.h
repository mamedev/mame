// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Kick Goal - Action Hollywood

*************************************************************************/

#include "sound/okim6295.h"
#include "machine/eepromser.h"

class kickgoal_state : public driver_device
{
public:
	kickgoal_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_fgram(*this, "fgram"),
		m_bgram(*this, "bgram"),
		m_bg2ram(*this, "bg2ram"),
		m_scrram(*this, "scrram"),
		m_spriteram(*this, "spriteram"),
		m_adpcm(*this, "oki"),
		m_eeprom(*this, "eeprom") ,
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_fgram;
	required_shared_ptr<UINT16> m_bgram;
	required_shared_ptr<UINT16> m_bg2ram;
	required_shared_ptr<UINT16> m_scrram;
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	tilemap_t     *m_fgtm;
	tilemap_t     *m_bgtm;
	tilemap_t     *m_bg2tm;

	/* misc */
	int         m_melody_loop;
	int         m_snd_new;
	int         m_snd_sam[4];
	int         m_m6295_comm;
	int         m_m6295_bank;
	UINT16      m_m6295_key_delay;

	int m_fg_base;

	int m_bg_base;
	int m_bg_mask;

	int m_bg2_base;
	int m_bg2_mask;
	int m_bg2_region;

	int m_sprbase;

	/* devices */
	required_device<okim6295_device> m_adpcm;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	DECLARE_READ16_MEMBER(kickgoal_eeprom_r);
	DECLARE_WRITE16_MEMBER(kickgoal_eeprom_w);
	DECLARE_WRITE16_MEMBER(kickgoal_fgram_w);
	DECLARE_WRITE16_MEMBER(kickgoal_bgram_w);
	DECLARE_WRITE16_MEMBER(kickgoal_bg2ram_w);
	DECLARE_WRITE16_MEMBER(actionhw_snd_w);
	DECLARE_DRIVER_INIT(kickgoal);
	TILE_GET_INFO_MEMBER(get_kickgoal_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_kickgoal_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_kickgoal_bg2_tile_info);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_kicksfg);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_kicksbg);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_kicksbg2);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_actionhwbg2);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(kickgoal);
	DECLARE_VIDEO_START(actionhw);
	UINT32 screen_update_kickgoal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(kickgoal_interrupt);
	void kickgoal_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
