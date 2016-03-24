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
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* video-related */
	bitmap_ind16 m_obj0_bitmap;
	bitmap_ind16 m_river_bitmap;
	bitmap_ind16 m_tree0_bitmap;
	bitmap_ind16 m_tree1_bitmap;
	std::unique_ptr<UINT8[]>   m_tree_ram;
	std::unique_ptr<UINT8[]>   m_memory_devices;
	UINT32   m_mem_dev_selected;    /* an offset within memory_devices area */
	UINT32   m_slopeROM_bank;
	UINT8    m_tree_en;
	UINT8    m_horizon;
	UINT8    m_v_count_river;
	UINT8    m_v_count_tree;
	int      m_tree_on[2];
	emu_timer* m_scanline_timer;

	/* misc */
	UINT8    m_tree0_col;
	UINT8    m_tree1_col;
	UINT8    m_left_bank_col;
	UINT8    m_right_bank_col;
	UINT8    m_boat_shore_col;
	UINT8    m_collision_reset;
	UINT8    m_tree_collision_reset;
	UINT8    m_prev_value_31;
	int      m_dir_31;

	/* mcu-related */
	UINT8    m_port_a_in;
	UINT8    m_port_a_out;
	UINT8    m_ddr_a;
	UINT8    m_port_b_out;
	UINT8    m_ddr_b;
	UINT8    m_port_c_in;
	UINT8    m_port_c_out;
	UINT8    m_ddr_c;

	UINT8    m_mcu_out;
	UINT8    m_mcu_in;
	UINT8    m_mcu_pc_1;
	UINT8    m_mcu_pc_0;

	/* devices */
	optional_device<cpu_device> m_mcu;
	DECLARE_READ8_MEMBER(mcu_r);
	DECLARE_WRITE8_MEMBER(mcu_w);
	DECLARE_READ8_MEMBER(changela_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(changela_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(changela_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(changela_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(changela_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(changela_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(changela_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(changela_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(changela_68705_ddr_c_w);
	DECLARE_READ8_MEMBER(changela_24_r);
	DECLARE_READ8_MEMBER(changela_25_r);
	DECLARE_READ8_MEMBER(changela_30_r);
	DECLARE_READ8_MEMBER(changela_31_r);
	DECLARE_READ8_MEMBER(changela_2c_r);
	DECLARE_READ8_MEMBER(changela_2d_r);
	DECLARE_WRITE8_MEMBER(mcu_pc_0_w);
	DECLARE_WRITE8_MEMBER(changela_collision_reset_0);
	DECLARE_WRITE8_MEMBER(changela_collision_reset_1);
	DECLARE_WRITE8_MEMBER(changela_coin_counter_w);
	DECLARE_WRITE8_MEMBER(changela_colors_w);
	DECLARE_WRITE8_MEMBER(changela_mem_device_select_w);
	DECLARE_WRITE8_MEMBER(changela_mem_device_w);
	DECLARE_READ8_MEMBER(changela_mem_device_r);
	DECLARE_WRITE8_MEMBER(changela_slope_rom_addr_hi_w);
	DECLARE_WRITE8_MEMBER(changela_slope_rom_addr_lo_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_changela(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(chl_mcu_irq);
	TIMER_CALLBACK_MEMBER(changela_scanline_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(changela_scanline);
	void draw_obj0( bitmap_ind16 &bitmap, int sy );
	void draw_obj1( bitmap_ind16 &bitmap );
	void draw_river( bitmap_ind16 &bitmap, int sy );
	void draw_tree( bitmap_ind16 &bitmap, int sy, int tree_num );
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};
