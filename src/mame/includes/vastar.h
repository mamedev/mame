// license:BSD-3-Clause
// copyright-holders:Allard van der Bas
class vastar_state : public driver_device
{
public:
	vastar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_sprite_priority(*this, "sprite_priority"),
		m_sharedram(*this, "sharedram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bg1videoram;
	required_shared_ptr<uint8_t> m_bg2videoram;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_sprite_priority;
	required_shared_ptr<uint8_t> m_sharedram;

	// these are pointers into m_fgvideoram
	uint8_t* m_bg1_scroll;
	uint8_t* m_bg2_scroll;
	uint8_t* m_spriteram1;
	uint8_t* m_spriteram2;
	uint8_t* m_spriteram3;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;
	uint8_t m_nmi_mask;

	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(bg1videoram_w);
	DECLARE_WRITE8_MEMBER(bg2videoram_w);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	void vastar(machine_config &config);
	void cpu2_map(address_map &map);
	void cpu2_port_map(address_map &map);
	void main_map(address_map &map);
	void main_port_map(address_map &map);
};
