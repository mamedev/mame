// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "sound/okim6295.h"
#include "cpu/pic16c5x/pic16c5x.h"

class drgnmst_state : public driver_device
{
public:
	drgnmst_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vidregs(*this, "vidregs"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_md_videoram(*this, "md_videoram"),
		m_rowscrollram(*this, "rowscrollram"),
		m_vidregs2(*this, "vidregs2"),
		m_spriteram(*this, "spriteram"),
			m_oki_1(*this, "oki1"),
			m_oki_2(*this, "oki2") ,
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_vidregs;
	required_shared_ptr<UINT16> m_fg_videoram;
	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_md_videoram;
	required_shared_ptr<UINT16> m_rowscrollram;
	required_shared_ptr<UINT16> m_vidregs2;
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_md_tilemap;

	/* misc */
	UINT16      m_snd_command;
	UINT16      m_snd_flag;
	UINT8       m_oki_control;
	UINT8       m_oki_command;
	UINT8       m_pic16c5x_port0;
	UINT8       m_oki0_bank;
	UINT8       m_oki1_bank;

	/* devices */
	required_device<okim6295_device> m_oki_1;
	required_device<okim6295_device> m_oki_2;
	DECLARE_WRITE16_MEMBER(drgnmst_coin_w);
	DECLARE_WRITE16_MEMBER(drgnmst_snd_command_w);
	DECLARE_WRITE16_MEMBER(drgnmst_snd_flag_w);
	DECLARE_READ8_MEMBER(pic16c5x_port0_r);
	DECLARE_READ8_MEMBER(drgnmst_snd_command_r);
	DECLARE_READ8_MEMBER(drgnmst_snd_flag_r);
	DECLARE_WRITE8_MEMBER(drgnmst_pcm_banksel_w);
	DECLARE_WRITE8_MEMBER(drgnmst_oki_w);
	DECLARE_WRITE8_MEMBER(drgnmst_snd_control_w);
	DECLARE_READ_LINE_MEMBER(PIC16C5X_T0_clk_r);
	DECLARE_WRITE16_MEMBER(drgnmst_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(drgnmst_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(drgnmst_md_videoram_w);
	DECLARE_DRIVER_INIT(drgnmst);
	TILE_GET_INFO_MEMBER(get_drgnmst_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_drgnmst_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_drgnmst_md_tile_info);
	TILEMAP_MAPPER_MEMBER(drgnmst_fg_tilemap_scan_cols);
	TILEMAP_MAPPER_MEMBER(drgnmst_md_tilemap_scan_cols);
	TILEMAP_MAPPER_MEMBER(drgnmst_bg_tilemap_scan_cols);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_drgnmst(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap,const rectangle &cliprect );
	UINT8 drgnmst_asciitohex( UINT8 data );
	required_device<cpu_device> m_maincpu;
	required_device<pic16c55_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
