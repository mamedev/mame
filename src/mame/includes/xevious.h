// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria

class xevious_state : public galaga_state
{
public:
	xevious_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaga_state(mconfig, type, tag),
		m_xevious_sr1(*this, "xevious_sr1"),
		m_xevious_sr2(*this, "xevious_sr2"),
		m_xevious_sr3(*this, "xevious_sr3"),
		m_xevious_fg_colorram(*this, "fg_colorram"),
		m_xevious_bg_colorram(*this, "bg_colorram"),
		m_xevious_fg_videoram(*this, "fg_videoram"),
		m_xevious_bg_videoram(*this, "bg_videoram"),
		m_samples(*this, "samples"),
		m_subcpu3(*this, "sub3") { }

	required_shared_ptr<uint8_t> m_xevious_sr1;
	required_shared_ptr<uint8_t> m_xevious_sr2;
	required_shared_ptr<uint8_t> m_xevious_sr3;
	required_shared_ptr<uint8_t> m_xevious_fg_colorram;
	required_shared_ptr<uint8_t> m_xevious_bg_colorram;
	required_shared_ptr<uint8_t> m_xevious_fg_videoram;
	required_shared_ptr<uint8_t> m_xevious_bg_videoram;
	optional_device<samples_device> m_samples;

	int32_t m_xevious_bs[2];
	void init_xevious();
	void init_xevios();
	void init_battles();
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_xevious();
	void palette_init_xevious(palette_device &palette);
	void machine_reset_xevios();
	void machine_reset_battles();
	uint32_t screen_update_xevious(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void battles_interrupt_4(device_t &device);
	void battles_nmi_generate(timer_device &timer, void *ptr, int32_t param);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void xevious_fg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void xevious_fg_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void xevious_bg_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void xevious_bg_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void xevious_vh_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void xevious_bs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t xevious_bb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// Custom I/O
	void battles_customio_init();

	uint8_t battles_customio0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t battles_customio_data0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t battles_customio3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t battles_customio_data3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t battles_input_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void battles_customio0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battles_customio_data0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battles_customio3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battles_customio_data3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battles_CPU4_coin_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void battles_noise_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t m_customio[16];
	char m_battles_customio_command;
	char m_battles_customio_prev_command;
	char m_battles_customio_command_count;
	char m_battles_customio_data;
	char m_battles_sound_played;

	optional_device<cpu_device> m_subcpu3;
};
