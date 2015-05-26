// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
class vastar_state : public driver_device
{
public:
	vastar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_sprite_priority(*this, "sprite_priority"),
		m_sharedram(*this, "sharedram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_bg1videoram;
	required_shared_ptr<UINT8> m_bg2videoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_sprite_priority;
	required_shared_ptr<UINT8> m_sharedram;

	// these are pointers into m_fgvideoram
	UINT8* m_bg1_scroll;
	UINT8* m_bg2_scroll;
	UINT8* m_spriteram1;
	UINT8* m_spriteram2;
	UINT8* m_spriteram3;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;


	UINT8 m_nmi_mask;
	DECLARE_WRITE8_MEMBER(vastar_hold_cpu2_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(vastar_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(vastar_bg1videoram_w);
	DECLARE_WRITE8_MEMBER(vastar_bg2videoram_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_vastar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
