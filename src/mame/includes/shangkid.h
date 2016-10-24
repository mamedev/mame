// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
class shangkid_state : public driver_device
{
public:
	shangkid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_bbx(*this, "bbx"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_videoreg(*this, "videoreg")  { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_bbx;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_videoreg;

	uint8_t m_bbx_sound_enable;
	uint8_t m_sound_latch;
	int m_gfx_type;
	tilemap_t *m_background;

	// shangkid and chinhero
	void maincpu_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bbx_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t soundlatch_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// game specific
	void chinhero_ay8910_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shangkid_ay8910_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void init_shangkid();
	void init_chinhero();
	void machine_reset_chinhero();
	void video_start_shangkid();
	void palette_init_dynamski(palette_device &palette);
	void machine_reset_shangkid();

	uint32_t screen_update_shangkid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dynamski(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprite(const uint8_t *source, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void shangkid_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void dynamski_draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri );
	void dynamski_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
