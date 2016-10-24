// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Angel Kids

*************************************************************************/

class angelkds_state : public driver_device
{
public:
	angelkds_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bgtopvideoram(*this, "bgtopvideoram"),
		m_bgbotvideoram(*this, "bgbotvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram"),
		m_subcpu(*this, "sub"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_decrypted_opcodes(*this, "decrypted_opcodes") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_bgtopvideoram;
	required_shared_ptr<uint8_t> m_bgbotvideoram;
	required_shared_ptr<uint8_t> m_txvideoram;
	required_shared_ptr<uint8_t> m_spriteram;

	tilemap_t    *m_tx_tilemap;
	tilemap_t    *m_bgbot_tilemap;
	tilemap_t    *m_bgtop_tilemap;
	int        m_txbank;
	int        m_bgbotbank;
	int        m_bgtopbank;

	uint8_t      m_sound[4];
	uint8_t      m_sound2[4];
	uint8_t      m_layer_ctrl;

	/* devices */
	required_device<cpu_device> m_subcpu;
	uint8_t angeklds_ff_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return 0xff; };
	void angelkds_cpu_bank_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void angelkds_main_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t angelkds_main_sound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void angelkds_sub_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t angelkds_sub_sound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void angelkds_txvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void angelkds_txbank_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void angelkds_bgtopvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void angelkds_bgtopbank_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void angelkds_bgtopscroll_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void angelkds_bgbotvideoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void angelkds_bgbotbank_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void angelkds_bgbotscroll_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void angelkds_layer_ctrl_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_angelkds();
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bgtop_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bgbot_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_angelkds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int enable_n);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
};
