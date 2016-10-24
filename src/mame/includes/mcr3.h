// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Midway MCR-3 system

**************************************************************************/

class mcr3_state : public mcr_state
{
public:
	mcr3_state(const machine_config &mconfig, device_type type, const char *tag)
		: mcr_state(mconfig, type, tag),
		m_spyhunt_alpharam(*this, "spyhunt_alpha"),
		m_screen(*this, "screen")
	{ }

	optional_shared_ptr<uint8_t> m_spyhunt_alpharam;
	required_device<screen_device> m_screen;

	uint8_t m_input_mux;
	uint8_t m_latched_input;
	uint8_t m_last_op4;
	uint8_t m_maxrpm_adc_control;
	uint8_t m_maxrpm_adc_select;
	uint8_t m_maxrpm_last_shift;
	int8_t m_maxrpm_p1_shift;
	int8_t m_maxrpm_p2_shift;
	uint8_t m_spyhunt_sprite_color_mask;
	int16_t m_spyhunt_scroll_offset;
	int16_t m_spyhunt_scrollx;
	int16_t m_spyhunt_scrolly;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_alpha_tilemap;

	void mcr3_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spyhunt_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spyhunt_alpharam_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void spyhunt_scroll_value_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcrmono_control_port_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t demoderm_ip1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t demoderm_ip2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void demoderm_op6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t maxrpm_ip1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t maxrpm_ip2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void maxrpm_op5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void maxrpm_op6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rampage_ip4_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void rampage_op6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t powerdrv_ip2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void powerdrv_op5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void powerdrv_op6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t stargrds_ip0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void stargrds_op5_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void stargrds_op6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t spyhunt_ip1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t spyhunt_ip2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void spyhunt_op4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t turbotag_ip2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t turbotag_kludge_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_crater();
	void init_demoderm();
	void init_turbotag();
	void init_powerdrv();
	void init_stargrds();
	void init_maxrpm();
	void init_rampage();
	void init_spyhunt();
	void init_sarge();
	void mcrmono_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	tilemap_memory_index spyhunt_bg_scan(uint32_t col, uint32_t row, uint32_t num_cols, uint32_t num_rows);
	void spyhunt_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void spyhunt_get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_mcrmono();
	void video_start_spyhunt();
	void palette_init_spyhunt(palette_device &palette);
	uint32_t screen_update_mcr3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_spyhunt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mcr3_update_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int code_xor, int dx, int dy, int interlaced);
	void mcr_common_init();

};
