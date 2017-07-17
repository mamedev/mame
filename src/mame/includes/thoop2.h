// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Peter Ferrie
class thoop2_state : public driver_device
{
public:
	thoop2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_shareram(*this, "shareram"),
		m_mcu_ram(*this, "mcu")	
	{ }

	DECLARE_WRITE16_MEMBER(OKIM6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(coin_w);
	DECLARE_WRITE16_MEMBER(vram_w);

	DECLARE_READ8_MEMBER(dallas_ram_r);
	DECLARE_WRITE8_MEMBER(dallas_ram_w);
	DECLARE_READ8_MEMBER(dallas_share_r);
	DECLARE_WRITE8_MEMBER(dallas_share_w);

	virtual void machine_start() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_tile_info_screen0);
	TILE_GET_INFO_MEMBER(get_tile_info_screen1);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sort_sprites();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);

private:
	int m_sprite_count[5];
	std::unique_ptr<int[]> m_sprite_table[5];
	tilemap_t *m_pant[2];

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_shareram;
	required_region_ptr<uint8_t> m_mcu_ram;
};
