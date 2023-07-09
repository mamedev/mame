// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Phil Stroffolino, Tomasz Slanina

#include "cpu/m6805/m68705.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"


class changela_state : public driver_device
{
public:
	changela_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mcu(*this, "mcu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_spriteram(*this, "spriteram")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_tilerom(*this, "tiles")
		, m_obj0rom(*this, "obj0")
		, m_obj1rom(*this, "obj1")
		, m_sloperom(*this, "slope")
		, m_treerom(*this, "tree")
		, m_proms(*this, "proms")
		, m_inputs(*this, "IN%u", 0)
		, m_gas(*this, "GAS")
		, m_wheel(*this, "WHEEL")
	{
	}

	void changela(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<m68705p_device> m_mcu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<u8> m_spriteram;
	required_shared_ptr<u8> m_videoram;
	required_shared_ptr<u8> m_colorram;
	required_region_ptr<u8> m_tilerom;
	required_region_ptr<u8> m_obj0rom;
	required_region_ptr<u8> m_obj1rom;
	required_region_ptr<u8> m_sloperom;
	required_region_ptr<u8> m_treerom;
	required_region_ptr<u8> m_proms;

	// input ports
	required_ioport_array<2> m_inputs;
	required_ioport m_gas;
	required_ioport m_wheel;

	// video-related
	bitmap_ind16 m_obj0_bitmap;
	bitmap_ind16 m_river_bitmap;
	bitmap_ind16 m_tree0_bitmap;
	bitmap_ind16 m_tree1_bitmap;

	std::unique_ptr<u8[]> m_riverram;
	std::unique_ptr<u8[]> m_treeram;
	u8 m_treeram2[0x20 * 2] = { };
	u8 m_stateram[0x40 * 3] = { };

	u8 m_mem_dev_selected = 0;
	u32 m_sloperom_bank = 0;
	u8 m_tree_en = 0;
	u8 m_horizon = 0;
	u8 m_v_count_river = 0;
	u8 m_v_count_tree = 0;
	u8 m_tree_on[2] = { };
	emu_timer *m_scanline_timer = nullptr;

	// mcu-related
	u8 m_port_a_out = 0xff;
	u8 m_port_c_out = 0xff;
	u8 m_mcu_out = 0xff;
	u8 m_mcu_in = 0xff;

	// misc
	u8 m_tree0_col = 0;
	u8 m_tree1_col = 0;
	u8 m_left_bank_col = 0;
	u8 m_right_bank_col = 0;
	u8 m_boat_shore_col = 0;
	u8 m_collision_reset = 0;
	u8 m_tree_collision_reset = 0;
	u8 m_prev_value_31 = 0;
	u8 m_dir_31 = 0;

	// devices
	u8 mcu_r();
	void mcu_w(u8 data);
	void changela_68705_port_a_w(u8 data);
	void changela_68705_port_c_w(u8 data);
	u8 changela_24_r();
	u8 changela_25_r();
	u8 changela_30_r();
	u8 changela_31_r();
	u8 changela_2d_r();
	void mcu_pc_0_w(int state);
	void collision_reset_0_w(int state);
	void collision_reset_1_w(int state);
	void coin_counter_1_w(int state);
	void coin_counter_2_w(int state);
	void changela_colors_w(offs_t offset, u8 data);
	void changela_mem_device_select_w(u8 data);
	void changela_mem_device_w(offs_t offset, u8 data);
	u8 changela_mem_device_r(offs_t offset);
	void changela_slope_rom_addr_hi_w(u8 data);
	void changela_slope_rom_addr_lo_w(u8 data);

	INTERRUPT_GEN_MEMBER(chl_mcu_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(changela_scanline);
	TIMER_CALLBACK_MEMBER(changela_scanline_callback);

	u32 screen_update_changela(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_obj0(bitmap_ind16 &bitmap, int sy);
	void draw_obj1(bitmap_ind16 &bitmap);
	void draw_river(bitmap_ind16 &bitmap, int sy);
	void draw_tree(bitmap_ind16 &bitmap, int sy, int tree_num);

	void changela_map(address_map &map);
};
