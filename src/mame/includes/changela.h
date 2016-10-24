// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Phil Stroffolino, Tomasz Slanina


class changela_state : public driver_device
{
public:
	changela_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_mcu(*this, "mcu"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	/* video-related */
	bitmap_ind16 m_obj0_bitmap;
	bitmap_ind16 m_river_bitmap;
	bitmap_ind16 m_tree0_bitmap;
	bitmap_ind16 m_tree1_bitmap;
	std::unique_ptr<uint8_t[]>   m_tree_ram;
	std::unique_ptr<uint8_t[]>   m_memory_devices;
	uint32_t   m_mem_dev_selected;    /* an offset within memory_devices area */
	uint32_t   m_slopeROM_bank;
	uint8_t    m_tree_en;
	uint8_t    m_horizon;
	uint8_t    m_v_count_river;
	uint8_t    m_v_count_tree;
	int      m_tree_on[2];
	emu_timer* m_scanline_timer;

	/* misc */
	uint8_t    m_tree0_col;
	uint8_t    m_tree1_col;
	uint8_t    m_left_bank_col;
	uint8_t    m_right_bank_col;
	uint8_t    m_boat_shore_col;
	uint8_t    m_collision_reset;
	uint8_t    m_tree_collision_reset;
	uint8_t    m_prev_value_31;
	int      m_dir_31;

	/* mcu-related */
	uint8_t    m_port_a_in;
	uint8_t    m_port_a_out;
	uint8_t    m_ddr_a;
	uint8_t    m_port_b_out;
	uint8_t    m_ddr_b;
	uint8_t    m_port_c_in;
	uint8_t    m_port_c_out;
	uint8_t    m_ddr_c;

	uint8_t    m_mcu_out;
	uint8_t    m_mcu_in;
	uint8_t    m_mcu_pc_1;
	uint8_t    m_mcu_pc_0;

	/* devices */
	optional_device<cpu_device> m_mcu;
	uint8_t mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t changela_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void changela_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t changela_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void changela_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t changela_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void changela_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t changela_24_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t changela_25_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t changela_30_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t changela_31_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t changela_2c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t changela_2d_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_pc_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_collision_reset_0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_collision_reset_1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_colors_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_mem_device_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_mem_device_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t changela_mem_device_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void changela_slope_rom_addr_hi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void changela_slope_rom_addr_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_changela(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void chl_mcu_irq(device_t &device);
	void changela_scanline_callback(void *ptr, int32_t param);
	void changela_scanline(timer_device &timer, void *ptr, int32_t param);
	void draw_obj0( bitmap_ind16 &bitmap, int sy );
	void draw_obj1( bitmap_ind16 &bitmap );
	void draw_river( bitmap_ind16 &bitmap, int sy );
	void draw_tree( bitmap_ind16 &bitmap, int sy, int tree_num );
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
