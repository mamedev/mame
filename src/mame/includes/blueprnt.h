// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Blue Print

***************************************************************************/

class blueprnt_state : public driver_device
{
public:
	blueprnt_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_videoram(*this, "videoram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* device/memory pointers */
	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_scrollram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	int     m_gfx_bank;

	/* misc */
	int     m_dipsw;

	DECLARE_READ8_MEMBER(blueprnt_sh_dipsw_r);
	DECLARE_READ8_MEMBER(grasspin_sh_dipsw_r);
	DECLARE_WRITE8_MEMBER(blueprnt_sound_command_w);
	DECLARE_WRITE8_MEMBER(blueprnt_coin_counter_w);
	DECLARE_WRITE8_MEMBER(blueprnt_videoram_w);
	DECLARE_WRITE8_MEMBER(blueprnt_colorram_w);
	DECLARE_WRITE8_MEMBER(blueprnt_flipscreen_w);
	DECLARE_WRITE8_MEMBER(dipsw_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(blueprnt);
	DECLARE_PALETTE_INIT(blueprnt);
	UINT32 screen_update_blueprnt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
