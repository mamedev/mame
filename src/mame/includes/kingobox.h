// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/*************************************************************************

    King of Boxer - Ring King

*************************************************************************/

class kingofb_state : public driver_device
{
public:
	kingofb_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_scroll_y(*this, "scroll_y"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"),
		m_spriteram(*this, "spriteram"),
		m_video_cpu(*this, "video"),
		m_sprite_cpu(*this, "sprite"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_scroll_y;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	int        m_palette_bank;

	/* misc */
	int        m_nmi_enable;

	/* devices */
	required_device<cpu_device> m_video_cpu;
	required_device<cpu_device> m_sprite_cpu;
	DECLARE_WRITE8_MEMBER(video_interrupt_w);
	DECLARE_WRITE8_MEMBER(sprite_interrupt_w);
	DECLARE_WRITE8_MEMBER(scroll_interrupt_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(kingofb_videoram_w);
	DECLARE_WRITE8_MEMBER(kingofb_colorram_w);
	DECLARE_WRITE8_MEMBER(kingofb_videoram2_w);
	DECLARE_WRITE8_MEMBER(kingofb_colorram2_w);
	DECLARE_WRITE8_MEMBER(kingofb_f800_w);
	DECLARE_DRIVER_INIT(ringkingw);
	DECLARE_DRIVER_INIT(ringking3);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(ringking_get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(kingofb);
	DECLARE_PALETTE_INIT(kingofb);
	DECLARE_VIDEO_START(ringking);
	DECLARE_PALETTE_INIT(ringking);
	UINT32 screen_update_kingofb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ringking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(kingofb_interrupt);
	void palette_init_common( palette_device &palette, const UINT8 *color_prom, void (kingofb_state::*get_rgb_data)(const UINT8 *, int, int *, int *, int *) );
	void kingofb_get_rgb_data( const UINT8 *color_prom, int i, int *r_data, int *g_data, int *b_data );
	void ringking_get_rgb_data( const UINT8 *color_prom, int i, int *r_data, int *g_data, int *b_data );
	void kingofb_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ringking_draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
