// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Phil Stroffolino, Tomasz Slanina, Adam Bousley
/**************************************************************************

Taito America's Change Lanes

The video hardware is an evolution of Midway Space Encounters. This game
was designed by the same person (Dave Needle), it's not a Taito Japan game.

TODO:
- filtering on ay2

***************************************************************************/

#include "emu.h"

#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "changela.lh"


namespace {

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
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
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
	bitmap_ind16 m_tree_bitmap[2];

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
	u8 m_tree_prio = 0;
	u8 m_tree_on[2] = { };

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

	// handlers
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

	void changela_map(address_map &map) ATTR_COLD;
};


/*********************************
    Initialization
*********************************/

void changela_state::machine_start()
{
	// video
	save_item(NAME(m_treeram2));
	save_item(NAME(m_stateram));
	save_item(NAME(m_sloperom_bank));
	save_item(NAME(m_tree_en));
	save_item(NAME(m_horizon));
	save_item(NAME(m_mem_dev_selected));
	save_item(NAME(m_v_count_river));
	save_item(NAME(m_v_count_tree));
	save_item(NAME(m_tree_prio));
	save_item(NAME(m_tree_on));

	// mcu
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_port_c_out));
	save_item(NAME(m_mcu_out));
	save_item(NAME(m_mcu_in));

	// misc
	save_item(NAME(m_tree0_col));
	save_item(NAME(m_tree1_col));
	save_item(NAME(m_left_bank_col));
	save_item(NAME(m_right_bank_col));
	save_item(NAME(m_boat_shore_col));
	save_item(NAME(m_collision_reset));
	save_item(NAME(m_tree_collision_reset));
	save_item(NAME(m_prev_value_31));
	save_item(NAME(m_dir_31));
}

void changela_state::machine_reset()
{
	// video
	m_sloperom_bank = 0;
	m_tree_en = 0;
	m_horizon = 0;
	m_mem_dev_selected = 0;
	m_v_count_river = 0;
	m_v_count_tree = 0;
	m_tree_on[0] = 0;
	m_tree_on[1] = 0;

	// misc
	m_tree0_col = 0;
	m_tree1_col = 0;
	m_left_bank_col = 0;
	m_right_bank_col = 0;
	m_boat_shore_col = 0;
	m_collision_reset = 0;
	m_tree_collision_reset = 0;
	m_prev_value_31 = 0;
	m_dir_31 = 0;
}

void changela_state::video_start()
{
	m_riverram = make_unique_clear<u8[]>(0x800);
	save_pointer(NAME(m_riverram), 0x800);

	m_treeram = make_unique_clear<u8[]>(0x800);
	save_pointer(NAME(m_treeram), 0x800);

	m_screen->register_screen_bitmap(m_obj0_bitmap);
	m_screen->register_screen_bitmap(m_river_bitmap);
	m_screen->register_screen_bitmap(m_tree_bitmap[0]);
	m_screen->register_screen_bitmap(m_tree_bitmap[1]);
}


/*********************************
    Screen Update
*********************************/

u32 changela_state::screen_update_changela(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_river_bitmap, 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, m_obj0_bitmap, 0, 0, 0, 0, cliprect, 0);
	copybitmap_trans(bitmap, m_tree_bitmap[0 ^ m_tree_prio], 0, 0, 0, 0, cliprect, 0);
	copybitmap_trans(bitmap, m_tree_bitmap[1 ^ m_tree_prio], 0, 0, 0, 0, cliprect, 0);
	draw_obj1(bitmap);

	return 0;
}


/*
--+-------------------+-----------------------------------------------------+-----------------------------------------------------------------
St|  PROM contents:   |                Main signals:                        |                      DESCRIPTION
at+-------------------+-----------------------------------------------------+-----------------------------------------------------------------
e:|7 6 5 4 3 2 1 0 Hex|/RAMw /RAMr /ROM  /AdderOutput  AdderInput TrainInputs|
  |                   |           enable GateU61Enable Enable     Enable    |
--+-------------------+-----------------------------------------------------+-----------------------------------------------------------------
00|0 0 0 0 1 1 0 1 0d | 1     1      0      1            0          1       |                                       (noop ROM 00-lsb to adder)
01|0 0 0 0 1 1 1 1 0f | 0     1      0      1            1          0       |   ROM 00-lsb to train, and to RAM 00
02|0 1 0 0 1 1 0 1 4d | 1     0      1      1            0          1       |                                       (noop RAM 00 to adder)
03|0 0 1 0 1 1 1 1 2f | 0     1      0      1            1          0       |   ROM 00-msb to train, and to RAM 01
04|1 1 0 0 1 1 0 1 cd | 1     0      1      1            0          1       |                                       (noop RAM 00 to adder)
05|0 0 0 0 1 1 1 1 0f | 0     1      0      1            1          0       |   ROM 01-lsb to train, and to RAM 02
06|0 1 0 0 1 1 0 1 4d | 1     0      1      1            0          1       |                                       (noop RAM 02 to adder)
07|0 0 1 0 1 1 1 1 2f | 0     1      0      1            1          0       |   ROM 01-msb to train, and to RAM 03
08|1 1 0 0 0 1 0 1 c5 | 1     0      1      1            0          1       |   CLR carry
09|0 0 0 0 1 1 0 1 0d | 1     1      0      1            0          1       |   ROM 02-lsb to adder
0a|0 1 1 0 1 1 0 1 6d | 1     0      1      1            0          1       |   RAM 05 to adder
0b|1 1 1 0 1 1 1 0 ee | 0     1      1      0            1          0       |   Adder to train, and to RAM 05, CLOCK carry
0c|0 0 0 0 1 1 0 1 0d | 1     1      0      1            0          1       |   ROM 02-msb to adder
0d|0 1 1 0 1 1 0 1 6d | 1     0      1      1            0          1       |   RAM 07 to adder
0e|1 1 1 0 1 1 1 0 ee | 0     1      1      0            1          0       |   Adder to train, and to RAM 07, CLOCK carry
0f|0 0 0 0 1 1 0 1 0d | 1     1      0      1            0          1       |   ROM 03-lsb to adder
10|0 1 1 0 1 1 0 1 6d | 1     0      1      1            0          1       |   RAM 09 to adder
11|1 1 1 0 1 1 1 0 ee | 0     1      1      0            1          0       |   Adder to train, and to RAM 09, CLOCK carry
12|1 0 0 0 1 1 0 1 8d | 1     1      0      1            0          1       |                                       (noop ROM 03-msb to adder)
13|0 1 0 0 1 1 0 1 4d | 1     0      1      1            0          1       |                                       (noop RAM 0c to adder)
14|0 0 0 0 0 0 0 1 01 | 1     1      0      1            0          1       |   ROM 04-lsb to adder, CLR carry
15|0 1 1 0 1 0 0 1 69 | 1     0      1      1            0          1       |   RAM 0d to adder
16|1 1 1 0 1 0 1 0 ea | 0     1      1      0            1          0       |   Adder to train and to RAM 0d, CLOCK carry
17|0 0 0 0 1 0 0 1 09 | 1     1      0      1            0          1       |   ROM 04-msb to adder
18|0 1 1 0 1 0 0 1 69 | 1     0      1      1            0          1       |   RAM 0f to adder
19|1 1 1 0 1 0 1 0 ea | 0     1      1      0            1          0       |   Adder to train and to RAM 0f, CLOCK carry
1a|0 0 0 1 1 0 0 1 19 | 1     1      0      1            0          1       |   ROM 05-lsb to adder, /LD HOSC
1b|0 1 1 0 1 0 0 1 69 | 1     0      1      1            0          1       |   RAM 11 to adder
1c|1 1 1 0 1 0 1 0 ea | 0     1      1      0            1          0       |   Adder to train and to RAM 11, CLOCK carry
1d|0 0 0 0 1 0 0 1 09 | 1     1      0      1            0          1       |   ROM 05-msb to adder
1e|0 1 1 0 1 0 0 1 69 | 1     0      1      1            0          1       |   RAM 13 to adder
1f|1 1 1 0 1 0 1 0 ea | 0     1      1      0            1          0       |   Adder to train and to RAM 13, CLOCK carry
                        *   =========================   ====================
                        *   only one of these signals   these signals select
                        *   can be active at a time     the output for the result
                        *    ------- SOURCE --------     ----- TARGET -----
                        *
                 ******************
                 result needs to be
                 written back to RAM
*/

TIMER_DEVICE_CALLBACK_MEMBER(changela_state::changela_scanline)
{
	int sy = param;

	// vblank irq
	if (sy == 256)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xdf); // Z80

	// timer irq, 3 times per given vblank field
	else if (((sy % 64) == 0))
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xcf); // Z80

	if (sy < 256)
	{
		// clear the current scanline first
		const rectangle rect(0, 255, sy, sy);
		m_river_bitmap.fill(0x00, rect);
		m_obj0_bitmap.fill(0x00, rect);
		m_tree_bitmap[0].fill(0x00, rect);
		m_tree_bitmap[1].fill(0x00, rect);

		draw_river(m_river_bitmap, sy);
		draw_obj0(m_obj0_bitmap, sy);
		draw_tree(m_tree_bitmap[0], sy, 0);
		draw_tree(m_tree_bitmap[1], sy, 1);
	}

	if (sy >= 32 && sy < 256)
	{
		// Collision Detection
		for (int sx = 1; sx < 256; sx++)
		{
			u16 pix = m_river_bitmap.pix(sy, sx);
			const bool riv_col = (pix == 0x08 || pix == 0x09 || pix == 0x0a);

			pix = m_river_bitmap.pix(sy, sx-1);
			const bool prev_col = (pix == 0x08 || pix == 0x09 || pix == 0x0a);

			if (m_obj0_bitmap.pix(sy, sx) == 0x14) // Car Outline Color
			{
				// Tree 0 Collision
				if (m_tree_bitmap[0].pix(sy, sx) != 0)
					m_tree0_col = 1;

				// Tree 1 Collision
				if (m_tree_bitmap[1].pix(sy, sx) != 0)
					m_tree1_col = 1;

				// Hit Right Bank
				if (!riv_col && prev_col)
					m_right_bank_col = 1;

				// Hit Left Bank
				if (riv_col && !prev_col)
					m_left_bank_col = 1;

				// Boat Hit Shore
				if (riv_col)
					m_boat_shore_col = 1;
			}
		}
		if (!m_tree_collision_reset)
		{
			m_tree0_col = 0;
			m_tree1_col = 0;
		}
		if (!m_collision_reset)
		{
			m_left_bank_col = 0;
			m_right_bank_col = 0;
			m_boat_shore_col = 0;
		}
	}
}


/*********************************
    River Video Generator
*********************************/

void changela_state::draw_river(bitmap_ind16 &bitmap, int sy)
{
	u8 const *const ROM = m_sloperom;
	u8 *const RAM = m_stateram;
	u8 const *const TILE_ROM = m_tilerom;
	u8 const *const TILE_RAM = m_riverram.get();
	u8 const *const PROM = m_proms;

	int preload = ((sy < 32) ? 1 : 0);

	u8 math_train[10] = { 0 };
	u8 pre_train[3] = { 0 };

	u8 prev_state = 0;

	u8 ram_count = 0;
	u8 rom_count = 0;

	int hosc = 0;
	int carry = 0;

	// Update Counters
	if (sy == 32)
		m_v_count_river = m_horizon;
	m_v_count_river = (m_v_count_river + 1) & 0xff;

	// ----- STATE MACHINE -----
	for (int i = 0; i < 0x20; i++)
	{
		u8 curr_state = PROM[i];

		// Update Counters
		if (prev_state & 0x80)
			ram_count = (ram_count + 1) & 0x0f;
		if ((curr_state & 0x40) && !(prev_state & 0x40))
			rom_count = (rom_count + 1) & 0x0f;

		if (prev_state & 0x02)
			carry = (((pre_train[1] + pre_train[2] + carry) > 0x0f) ? 1 : 0);
		if (!(curr_state & 0x08))
			carry = 0;

		if (prev_state & 0x10)
			hosc = (math_train[8] << 4) | math_train[9];

		int rom_addr = m_sloperom_bank | ((m_v_count_river & 0x7e) << 2) | ((rom_count & 0x0e) >> 1);
		int ram_a5 = ((curr_state & 0x01) & ((curr_state & 0x40) >> 6) & preload) ^ 0x01;
		int ram_addr = (ram_a5 << 5) | (ram_count << 1) | ((curr_state & 0x20) >> 5);
		int mux45 = rom_count & 0x01;
		int mux61 = m_v_count_river & 0x01;

		switch (curr_state)
		{
			case 0x01: case 0x09: case 0x19: case 0x0d: case 0x8d:
				pre_train[0] = (mux45 ? ((ROM[rom_addr] & 0xf0) >> 4) : (ROM[rom_addr] & 0x0f));
				break;
			case 0x0f: case 0x2f:
				math_train[0] = RAM[ram_addr] = (mux45 ? ((ROM[rom_addr] & 0xf0) >> 4) : (ROM[rom_addr] & 0x0f));
				break;
			case 0x4d: case 0x69: case 0x6d: case 0xc5: case 0xcd:
				pre_train[0] = RAM[ram_addr] & 0x0f;
				break;
			case 0xea: case 0xee:
				math_train[0] = RAM[ram_addr] = (mux61 ? (pre_train[1]) : ((pre_train[1] + pre_train[2] + carry) & 0x0f));
				break;
			default:
				break;
		}

		// Shift each item down the train
		if (curr_state & 0x02)
		{
			for (int j = 9; j > 0; j--)
			{
				math_train[j] = math_train[j - 1];
			}
		}
		else
		{
			pre_train[2] = pre_train[1];
			pre_train[1] = pre_train[0];
		}

		prev_state = curr_state;
	}

	if (!(m_v_count_river & 0x80))
	{
		int h_count = 0x80 | (hosc >> 1);
		int tile_v = ((math_train[3] & 0x0c) >> 2) | ((math_train[2] & 0x0f) << 2) | ((math_train[1] & 0x07) << 6);
		int tile_h = ((math_train[7] & 0x0f) | ((math_train[6] & 0x0f) << 4) | ((math_train[5] & 0x01) << 8)) + 4;

		for (int sx = 0; sx < 256; sx++)
		{
			for (int i = 0; i < ((sx < 16) ? 1 : 4); i++)
			{
				if (h_count > 0xff)
				{
					h_count = ((math_train[9] & 0x0f) >> 1) | ((math_train[8] & 0x0f) << 3) | 0x80;
					tile_h = (tile_h + 1) & 0xfff;

					// Skip one count if LSB is high
					if (((math_train[9] & 0x01) && (tile_h & 0x01)))
						h_count--;
				}
				else
					h_count++;
			}

			int ram_addr = ((tile_h & 0x1f8) >> 3) | ((tile_v & 0x1f0) << 2);
			int rom_addr = ((tile_h & 0x06) >> 1) | ((tile_v & 0x0f) << 2) | ((TILE_RAM[ram_addr] & 0x7f) << 6);

			int col;
			if (tile_h & 0x01)
				col = TILE_ROM[rom_addr] & 0x0f;
			else
				col = (TILE_ROM[rom_addr] & 0xf0) >> 4;

			bitmap.pix(sy, sx) = col;
		}
	}

	// Determine tree priority for this frame
	if (m_v_count_river == 0x7f)
		m_tree_prio = BIT(math_train[2], 2);
}


/*********************************
    Tree Generators
*********************************/

void changela_state::draw_tree(bitmap_ind16 &bitmap, int sy, int tree_num)
{
	// State machine
	u8 const *const ROM = m_sloperom;
	u8 *const RAM = m_stateram + 0x40 + 0x40 * tree_num;
	u8 const *const PROM = m_proms;

	// Tree Data
	u8 *const RAM2 = m_treeram2 + 0x20 * tree_num;
	u8 const *const TILE_ROM = (tree_num ? (m_treerom + 0x1000) : (m_tilerom + 0x2000));
	u8 const *const TILE_RAM = (tree_num ? m_treerom : m_treeram.get());

	int preload = ((sy < 32) ? 1 : 0);

	u8 math_train[10] = { 0 };
	u8 pre_train[3] = { 0 };
	u8 tree_train[3] = { 0 };

	u8 prev_state = 0;

	u8 ram_count = 0;
	u8 rom_count = 0;

	int hosc = 0;
	int carry = 0;
	int tree_carry = 0;

	int h_count, tile_v, tile_h;
	int all_ff;

	// Update Counters
	if (sy == 32)
	{
		m_tree_on[tree_num] = 0;
		if (tree_num == 0)
			m_v_count_tree = m_horizon;
	}
	if (tree_num == 0)
		m_v_count_tree = (m_v_count_tree + 1) & 0xff;

	// ----- STATE MACHINE -----
	for (int i = 0; i < 0x20; i++)
	{
		u8 curr_state = PROM[i];

		// Update Counters
		if (prev_state & 0x80)
			ram_count = (ram_count + 1) & 0x0f;
		if ((curr_state & 0x40) && !(prev_state & 0x40))
			rom_count = (rom_count + 1) & 0x0f;

		if (prev_state & 0x02)
		{
			carry = (((pre_train[1] + pre_train[2] + carry) > 0x0f) ? 1 : 0);
			tree_carry = (((tree_train[1] + tree_train[2] + tree_carry) > 0x0f) ? 1 : 0);
		}
		if (!(curr_state & 0x08))
			carry = tree_carry = 0;

		if (prev_state & 0x10)
			hosc = (math_train[8] << 4) | math_train[9];

		int rom_addr = m_sloperom_bank | ((m_v_count_tree & 0x7e) << 2) | ((rom_count & 0x0e) >> 1);
		int ram_a5 = ((curr_state & 0x01) & ((curr_state & 0x40) >> 6) & preload) ^ 0x01;
		int ram_addr = (ram_a5 << 5) | (ram_count << 1) | ((curr_state & 0x20) >> 5);
		int ram2_addr = (ram_count << 1) | ((curr_state & 0x20) >> 5);
		int mux45 = rom_count & 0x01;
		int mux61 = m_v_count_tree & 0x01;

		switch (curr_state)
		{
			case 0x01: case 0x09: case 0x19: case 0x0d: case 0x8d:
				pre_train[0] = (mux45 ? ((ROM[rom_addr] & 0xf0) >> 4) : (ROM[rom_addr] & 0x0f));
				break;
			case 0x0f: case 0x2f:
				RAM[ram_addr] = (mux45 ? ((ROM[rom_addr] & 0xf0) >> 4) : (ROM[rom_addr] & 0x0f));
				break;
			case 0x4d: case 0x69: case 0x6d: case 0xc5: case 0xcd:
				pre_train[0] = RAM[ram_addr] & 0x0f;
				break;
			case 0xea: case 0xee:
				RAM[ram_addr] = (mux61 ? (pre_train[1]) : ((pre_train[1] + pre_train[2] + carry) & 0x0f));
				break;
			default:
				break;
		}

		if (!m_tree_on[tree_num])
		{
			int mux82 = (m_v_count_tree & 0x01) ^ 0x01;

			switch (curr_state)
			{
				case 0x01: case 0x09: case 0x19: case 0x0d: case 0x8d:
					tree_train[0] = RAM2[ram2_addr] = pre_train[0];
					break;
				case 0x0f: case 0x2f:
					math_train[0] = RAM2[ram2_addr] = RAM[ram_addr] & 0x0f;
					break;
				case 0x4d: case 0x69: case 0x6d: case 0xc5: case 0xcd:
					tree_train[0] = RAM2[ram2_addr] = pre_train[0];
					break;
				case 0xea: case 0xee:
					math_train[0] = RAM2[ram2_addr] = (mux82 ? ((tree_train[1] + tree_train[2] + tree_carry) & 0x0f) : (tree_train[1]));
					break;
				default:
					break;
			}
		}
		else
		{
			int mux82 = ((curr_state & 0x04) ? 0 : 1);

			switch (curr_state)
			{
				case 0x01: case 0x09: case 0x19: case 0x0d: case 0x8d:
					tree_train[0] = RAM2[ram2_addr];
					break;
				case 0x0f: case 0x2f:
					math_train[0] = RAM2[ram2_addr];
					break;
				case 0x4d: case 0x69: case 0x6d: case 0xc5: case 0xcd:
					tree_train[0] = RAM2[ram2_addr];
					break;
				case 0xea: case 0xee:
					math_train[0] = RAM2[ram2_addr] = (mux82 ? ((tree_train[1] + tree_train[2] + tree_carry) & 0x0f) : (tree_train[1]));
					break;
				default:
					break;
			}
		}

		// Shift each item down the train
		if (curr_state & 0x02)
		{
			for (int j = 9; j > 0; j--)
				math_train[j] = math_train[j-1];
		}
		else
		{
			pre_train[2] = pre_train[1];
			pre_train[1] = pre_train[0];
			tree_train[2] = tree_train[1];
			tree_train[1] = tree_train[0];
		}

		prev_state = curr_state;
	}

	h_count = 0x80 | (hosc >> 1);
	tile_v = ((math_train[3] & 0x0c) >> 2) | ((math_train[2] & 0x0f) << 2) | ((math_train[1] & 0x07) << 6);
	tile_h = ((math_train[7] & 0x0f) | ((math_train[6] & 0x0f) << 4) | ((math_train[5] & 0x01) << 8)) + 4;
	all_ff = 1;

	for (int sx = 0; sx < 256; sx++)
	{
		for (int i = 0; i < ((sx < 16) ? 1 : 4); i++)
		{
			if (h_count > 0xff)
			{
				h_count = ((math_train[9] & 0x0f) >> 1) | ((math_train[8] & 0x0f) << 3) | 0x80;
				tile_h = (tile_h + 1) & 0xfff;

				// Skip one count if LSB is high
				if (((math_train[9] & 0x01) && (tile_h & 0x01)))
					h_count--;
			}
			else
				h_count++;
		}

		int ram_addr = ((tile_h & 0x1f8) >> 3) | ((tile_v & 0x1f0) << 2);
		int rom_addr = ((tile_h & 0x06) >> 1) | ((tile_v & 0x0f) << 2) | ((TILE_RAM[ram_addr] & 0x7f) << 6);

		if (!(m_v_count_tree & 0x80) && (m_tree_en & (0x01 << tree_num)) && ((TILE_ROM[rom_addr] & 0xf0) == 0))
			m_tree_on[tree_num] = 1;

		if (m_tree_on[tree_num])
		{
			int col;
			if (tile_h & 0x01)
				col = TILE_ROM[rom_addr] & 0x0f;
			else
				col = (TILE_ROM[rom_addr] & 0xf0) >> 4;

			if (col != 0x0f)
				all_ff = 0;

			if (col != 0x0f && col != 0x00)
				bitmap.pix(sy, sx) = col | 0x30;
		}
	}

	// Tree on only stays high if a pixel that is not 0xf is encountered, because any non 0xf pixel sets U56 high
	if (all_ff)
		m_tree_on[tree_num] = 0;
}


/*********************************
    Obj 0 - Sprite Layer
*********************************/

void changela_state::draw_obj0(bitmap_ind16 &bitmap, int sy)
{
	u8 const *const ROM = m_obj0rom;
	u8 const *const RAM = m_spriteram;

	for (int sx = 0; sx < 256; sx++)
	{
		int vr = (RAM[sx * 4 + 0] & 0x80) >> 7;
		int hr = (RAM[sx * 4 + 0] & 0x40) >> 6;
		int hs = (RAM[sx * 4 + 0] & 0x20) >> 5;
		u32 vsize = RAM[sx * 4 + 0] & 0x1f;
		u8 ypos = ~RAM[sx * 4 + 1] + 1;
		u8 tile = RAM[sx * 4 + 2];
		u8 xpos = RAM[sx * 4 + 3];

		if (sy - ypos <= vsize)
		{
			for (int i = 0; i < 16; i++)
			{
				u8 sum = sy - ypos;

				u8 counter = i;
				if (hr) counter ^= 0x0f;

				u32 A8 = ((tile & 0x02) >> 1) ^ ((hr & hs) ^ hs);
				u32 A7 = ((((vr ^ ((sum & 0x10) >> 4)) & ((vsize & 0x10) >> 4)) ^ 0x01) & (tile & 0x01)) ^ 0x01;
				u32 rom_addr = (counter >> 1) | ((sum & 0x0f) << 3) | (A7 << 7) | (A8 << 8) | ((tile >> 2) << 9);
				if (vr) rom_addr ^= (0x0f << 3);

				u8 data;
				if (counter & 1)
					data = ROM[rom_addr] & 0x0f;
				else
					data = (ROM[rom_addr] & 0xf0) >> 4;

				if ((data != 0x0f) && (data != 0) && (xpos + i) < 256)
					bitmap.pix(sy, xpos + i) = data | 0x10;

				if (hs)
				{
					if (counter & 1)
						data = ROM[rom_addr ^ 0x100] & 0x0f;
					else
						data = (ROM[rom_addr ^ 0x100] & 0xf0) >> 4;

					if ((data != 0x0f) && (data != 0) && (xpos + i + 16) < 256)
						bitmap.pix(sy, xpos + i + 16) = data | 0x10;
				}
			}
		}
	}
}


/*********************************
    Obj 1 - Text Layer
*********************************/

void changela_state::draw_obj1(bitmap_ind16 &bitmap)
{
	u8 const *const ROM = m_obj1rom;
	u8 const *const RAM = m_videoram;

	u8 reg[4] = { 0 }; // 4x4-bit registers (U58, U59)

	u8 attrib = 0;

	for (int sy = 0; sy < 256; sy++)
	{
		for (int sx = 0; sx < 256; sx++)
		{
			// 11 Bits: H1, H3, H4, H5, H6, H7, V3, V4, V5, V6, V7
			int ram_addr = ((sx & 0xf8) >> 2) | ((sy & 0xf8) << 3);
			int tile_addr = RAM[ram_addr];

			if (!(RAM[ram_addr + 1] & 0x10) && (sx & 0x04)) // D4=0 enables latch at U32
				attrib = RAM[ram_addr + 1];

			u8 tile = ROM[(tile_addr << 4) | ((sx & 0x04) >> 2) | ((sy & 0x07) << 1)];
			reg[(sx & 0x0c) >> 2] = tile;
			int sum = (sx & 0x0f) + (attrib & 0x0f); // 4-bit adder (U45)

			// Multiplexers (U57)
			int c0, c1;
			if ((sum & 0x03) == 0)
			{
				c0 = (reg[(sum & 0x0c) >> 2] & 0x08) >> 3;
				c1 = (reg[(sum & 0x0c) >> 2] & 0x80) >> 7;
			}
			else if ((sum & 0x03) == 1)
			{
				c0 = (reg[(sum & 0x0c) >> 2] & 0x04) >> 2;
				c1 = (reg[(sum & 0x0c) >> 2] & 0x40) >> 6;
			}
			else if ((sum & 0x03) == 2)
			{
				c0 = (reg[(sum & 0x0c) >> 2] & 0x02) >> 1;
				c1 = (reg[(sum & 0x0c) >> 2] & 0x20) >> 5;
			}
			else
			{
				c0 = (reg[(sum & 0x0c) >> 2] & 0x01) >> 0;
				c1 = (reg[(sum & 0x0c) >> 2] & 0x10) >> 4;
			}

			int col = c0 | (c1 << 1) | ((attrib & 0xc0) >> 4);
			if ((col & 0x07) != 0x07)
				bitmap.pix(sy, sx) = col | 0x20;
		}
	}
}


/*********************************
    Video I/O
*********************************/

void changela_state::changela_colors_w(offs_t offset, u8 data)
{
	/* Each color is combined from 3 bits from open-collector outputs of ram.
	Each of the bits is connected to a 220, 470, or 1000 Ohm resistor.
	There is also a 680 Ohm pull-up resistor connected to 5V, and a
	2.2k resistor connected to GND. Thus these output voltages are obtained:
	    Val     |   Vout
	    000     |   0.766   (220 || 470 || 1k || 2.2k)
	    001     |   0.855   (220 || 470 || 2.2k)
	    010     |   0.984   (220 || 1k || 2.2k)
	    011     |   1.136   (220 || 2.2k)
	    100     |   1.455   (470 || 1k || 2.2k)
	    101     |   1.814   (470 || 2.2k)
	    110     |   2.514   (1k || 2.2k)
	    111     |   3.819   (2.2k)
	Which were normalized to produce the following table: */

	static const u8 color_table[8] = { 0, 7, 18, 31, 58, 88, 146, 255 };

	int r, g, b;
	u32 c, color_index;

	c = data | ((offset & 0x01) << 8); // a0 used as D8 bit input
	c ^= 0x1ff; // active low

	color_index = offset >> 1;
	color_index ^= 0x30; // A4 and A5 lines are negated

	r = color_table[(c >> 0) & 0x07];
	g = color_table[(c >> 3) & 0x07];
	b = color_table[(c >> 6) & 0x07];

	m_palette->set_pen_color(color_index,r,g,b);
}

void changela_state::changela_mem_device_select_w(u8 data)
{
	/*
	(data & 0x07) possible settings:
	0 - not connected (no device)
	1 - ADR1 is 2114 RAM at U59 (state machine) (accessible range: 0x0000-0x003f)
	2 - ADR2 is 2128 RAM at U109 (River RAM)    (accessible range: 0x0000-0x07ff)
	3 - ADR3 is 2128 RAM at U114 (Tree RAM)     (accessible range: 0x0000-0x07ff)
	4 - ADR4 is 2732 ROM at U7 (Tree ROM)       (accessible range: 0x0000-0x07ff)
	5 - SLOPE is ROM at U44 (state machine)     (accessible range: 0x0000-0x07ff)
	*/
	m_mem_dev_selected = data & 0x07;
	m_tree_en = (data & 0x30) >> 4;
}

void changela_state::changela_mem_device_w(offs_t offset, u8 data)
{
	switch (m_mem_dev_selected)
	{
		case 1:
			for (int i = 0; i < 3; i++)
				m_stateram[(i * 0x40) + (offset & 0x3f)] = data & 0xf;
			break;

		case 2:
			m_riverram[offset] = data;
			break;

		case 3:
			m_treeram[offset] = data;
			break;

		default:
			break;
	}
}

u8 changela_state::changela_mem_device_r(offs_t offset)
{
	switch (m_mem_dev_selected)
	{
		case 1: return m_stateram[offset & 0x3f];
		case 2: return m_riverram[offset];
		case 3: return m_treeram[offset];
		case 4: return m_treerom[offset];
		case 5: return m_sloperom[offset];

		default:
			break;
	}

	return 0;
}

void changela_state::changela_slope_rom_addr_hi_w(u8 data)
{
	m_sloperom_bank = (data & 0x03) << 9;
}

void changela_state::changela_slope_rom_addr_lo_w(u8 data)
{
	m_horizon = data;
}

u8 changela_state::changela_25_r()
{
	// collisions on bits 3,2, bits 1,0-N/C inputs
	return (m_tree1_col << 3) | (m_tree0_col << 2) | 0x03;
}

void changela_state::collision_reset_0_w(int state)
{
	m_collision_reset = state;
}

void changela_state::collision_reset_1_w(int state)
{
	m_tree_collision_reset = state;
}


/*********************************
    MCU I/O
*********************************/

void changela_state::changela_68705_port_a_w(u8 data)
{
	m_port_a_out = data;
}

void changela_state::changela_68705_port_c_w(u8 data)
{
	// PC3 is connected to the CLOCK input of the LS374, so we latch the data on rising edge
	if (BIT(data, 3) && !BIT(m_port_c_out, 3))
		m_mcu_out = m_port_a_out;

	// PC2 is connected to the /OE input of the LS374
	m_mcu->pa_w(BIT(data, 2) ? 0xff : m_mcu_in);

	m_port_c_out = data;
}

// latch LS374 at U40
u8 changela_state::mcu_r()
{
	//osd_printf_debug("Z80 MCU  R = %x\n", m_mcu_out);
	return m_mcu_out & (BIT(m_port_c_out, 2) ? 0xff : m_mcu_in);
}

// latch LS374 at U39
void changela_state::mcu_w(u8 data)
{
	m_mcu_in = data;
	if (!BIT(m_port_c_out, 2))
		m_mcu->pa_w(data);
}

// U30
u8 changela_state::changela_24_r()
{
	return (BIT(m_port_c_out, 1) << 3) | 0x07; // bits 2,1,0-N/C inputs
}

void changela_state::mcu_pc_0_w(int state)
{
	m_mcu->pc_w(0xfe | state);
}

INTERRUPT_GEN_MEMBER(changela_state::chl_mcu_irq)
{
	m_mcu->pulse_input_line(0, m_mcu->minimum_quantum_time());
}


/*********************************
    Other I/O
*********************************/

u8 changela_state::changela_30_r()
{
	return m_wheel->read() & 0x0f; // wheel control (clocked input) signal on bits 3,2,1,0
}

u8 changela_state::changela_31_r()
{
	/* If the new value is less than the old value, and it did not wrap around,
	   or if the new value is greater than the old value, and it did wrap around,
	   then we are moving LEFT. */
	u8 cur = m_wheel->read();
	u8 prev = m_prev_value_31;

	if ((cur < prev && (prev - cur) < 0x80) || (cur > prev && (cur - prev) > 0x80))
		m_dir_31 = 1;
	if ((prev < cur && (cur - prev) < 0x80) || (prev > cur && (prev - cur) > 0x80))
		m_dir_31 = 0;

	m_prev_value_31 = cur;

	// wheel UP/DOWN control signal on bit 3, collisions on bits:2,1,0
	return (m_dir_31 << 3) | (m_left_bank_col << 2) | (m_right_bank_col << 1) | m_boat_shore_col;
}

u8 changela_state::changela_2d_r()
{
	// Gas pedal is made up of 2 switches, 1 active low, 1 active high
	const u8 lut_gas[3] = { 1, 0, 2 };
	u8 gas = lut_gas[m_gas->read() % 3];

	u8 v8 = BIT(m_screen->vpos(), 8);

	return (m_inputs[1]->read() & 0x20) | (gas << 6) | (v8 << 4);
}

void changela_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void changela_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


/*********************************
    Address Map
*********************************/

void changela_state::changela_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().share("spriteram"); // OBJ0 RAM
	map(0x9000, 0x97ff).ram().share("videoram"); // OBJ1 RAM
	map(0xa000, 0xa07f).w(FUNC(changela_state::changela_colors_w)).share("colorram"); // Color 93419 RAM 64x9(nine!!!) bits A0-used as the 8th bit data input (d0-d7->normal, a0->d8)
	map(0xb000, 0xbfff).rom();

	map(0xc000, 0xc7ff).rw(FUNC(changela_state::changela_mem_device_r), FUNC(changela_state::changela_mem_device_w)); // RAM4 (River Bed RAM); RAM5 (Tree RAM)

	// LS138 - U16
	map(0xc800, 0xc800).nopw(); // not connected
	map(0xc900, 0xc900).w(FUNC(changela_state::changela_mem_device_select_w)); // selects the memory device to be accessible at 0xc000-0xc7ff
	map(0xca00, 0xca00).w(FUNC(changela_state::changela_slope_rom_addr_hi_w));
	map(0xcb00, 0xcb00).w(FUNC(changela_state::changela_slope_rom_addr_lo_w));

	map(0xd000, 0xd001).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xd010, 0xd011).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));

	// LS259 - U44
	map(0xd020, 0xd027).w("outlatch", FUNC(ls259_device::write_d0));

	// LS139 - U24
	map(0xd024, 0xd024).r(FUNC(changela_state::changela_24_r));
	map(0xd025, 0xd025).r(FUNC(changela_state::changela_25_r));
	map(0xd028, 0xd028).r(FUNC(changela_state::mcu_r));
	map(0xd02c, 0xd02c).portr("IN0");
	map(0xd02d, 0xd02d).r(FUNC(changela_state::changela_2d_r));

	map(0xd030, 0xd030).rw(FUNC(changela_state::changela_30_r), FUNC(changela_state::mcu_w));
	map(0xd031, 0xd031).r(FUNC(changela_state::changela_31_r));

	map(0xe000, 0xe000).w("watchdog", FUNC(watchdog_timer_device::reset_w));

	map(0xf000, 0xf7ff).ram(); // RAM2 (Processor RAM)
}


/*********************************
    Input Ports
*********************************/

static INPUT_PORTS_START( changela )
	PORT_START("DSWA") // DSWA
	PORT_DIPNAME( 0x07, 0x01, "Steering Wheel Ratio" )      PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Ignore Memory Failures" )    PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Controls ) )         PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, "Steering Wheel" )
	PORT_DIPNAME( 0x40, 0x40, "Diagnostic" )                PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Players ) )          PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSWB") // DSWB
	PORT_DIPNAME( 0x03, 0x00, "Max Bonus Fuels" )           PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "99" )
	PORT_DIPNAME( 0x0c, 0x08, "Game Difficulty" )           PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x20, "Traffic Difficulty" )        PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x00, "Land Collisions" )           PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, "Off (Cheat)" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Car Collision" )             PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, "Off (Cheat)" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC") // DSWC - coinage
	PORT_DIPNAME( 0xf0, 0x10, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWC:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x0f, 0x01, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SWC:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )

	PORT_START("DSWD") // DSWD - bonus
	PORT_DIPNAME( 0x01, 0x01, "Right Slot" )                PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x01, "On Right (Bottom) Counter" )
	PORT_DIPSETTING(    0x00, "On Left (Top) Counter" )
	PORT_DIPNAME( 0x02, 0x00, "Left Slot" )                 PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x02, "On Right (Bottom) Counter" )
	PORT_DIPSETTING(    0x00, "On Left (Top) Counter" )
	PORT_DIPNAME( 0x1c, 0x00, "Credits For Bonus" )         PORT_DIPLOCATION("SWD:3,4,5")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x14, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPSETTING(    0x1c, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWD:6" ) // Listed as "Unused"
	PORT_DIPNAME( 0x40, 0x00, "'King Of The World' Name Length" ) PORT_DIPLOCATION("SWD:7")
	PORT_DIPSETTING(    0x40, "3 Letters" )
	PORT_DIPSETTING(    0x00, "Long" )
	PORT_DIPNAME( 0x80, 0x00, "'King Of The World' Name" )  PORT_DIPLOCATION("SWD:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MCU") // MCU
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // COIN3 on schematics, but game only has 2 coin slots
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN0") // 0xDx2C
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSWA", 0x20, EQUALS, 0x00) // SPARE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSWA", 0x20, EQUALS, 0x00) // SPARE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("DSWA", 0x20, EQUALS, 0x20)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("DSWA", 0x20, EQUALS, 0x20)
	PORT_BIT( 0x40, 0x40, IPT_CUSTOM ) PORT_CONDITION("GEAR", 0x01, EQUALS, 0x00) // FWD
	PORT_BIT( 0x80, 0x80, IPT_CUSTOM ) PORT_CONDITION("GEAR", 0x01, EQUALS, 0x01) // REV

	PORT_START("IN1") // 0xDx2D
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("WHEEL") // 0xDx30
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_CONDITION("DSWA", 0x20, EQUALS, 0x00)

	PORT_START("GAS")
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0x02) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) PORT_CONDITION("DSWA", 0x20, EQUALS, 0x00)

	PORT_START("GEAR")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Gear Shift") PORT_TOGGLE
INPUT_PORTS_END


/*********************************
    Machine Config
*********************************/

void changela_state::changela(machine_config &config)
{
	Z80(config, m_maincpu, 20_MHz_XTAL/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &changela_state::changela_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(changela_state::changela_scanline), "screen", 0, 1);

	M68705P3(config, m_mcu, 20_MHz_XTAL/8);
	m_mcu->portb_r().set_ioport("MCU");
	m_mcu->porta_w().set(FUNC(changela_state::changela_68705_port_a_w));
	m_mcu->portc_w().set(FUNC(changela_state::changela_68705_port_c_w));
	m_mcu->set_vblank_int("screen", FUNC(changela_state::chl_mcu_irq));

	ls259_device &outlatch(LS259(config, "outlatch")); // U44 on Sound I/O Board
	outlatch.q_out_cb<0>().set(FUNC(changela_state::collision_reset_0_w));
	outlatch.q_out_cb<1>().set(FUNC(changela_state::coin_counter_1_w));
	outlatch.q_out_cb<2>().set(FUNC(changela_state::coin_counter_2_w));
	outlatch.q_out_cb<4>().set(FUNC(changela_state::mcu_pc_0_w));
	outlatch.q_out_cb<5>().set(FUNC(changela_state::collision_reset_1_w));

	WATCHDOG_TIMER(config, "watchdog");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(20_MHz_XTAL/4, 318, 8, 256, 262, 32, 256);
	m_screen->set_screen_update(FUNC(changela_state::screen_update_changela));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(0x40);

	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 20_MHz_XTAL/16)); // U8
	ay1.port_a_read_callback().set_ioport("DSWA");
	ay1.port_b_read_callback().set_ioport("DSWB");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);

	ay8910_device &ay2(AY8910(config, "ay2", 20_MHz_XTAL/16)); // U9
	ay2.port_a_read_callback().set_ioport("DSWC");
	ay2.port_b_read_callback().set_ioport("DSWD");
	ay2.add_route(ALL_OUTPUTS, "mono", 0.50);
}


/*********************************
    ROM Definitions
*********************************/

ROM_START( changela )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80 code
	ROM_LOAD( "cl25a",  0x0000, 0x2000, CRC(38530a60) SHA1(0b0ef1abe11c5271fcd1671322b77165217553c3) )
	ROM_LOAD( "cl24a",  0x2000, 0x2000, CRC(2fcf4a82) SHA1(c33355e2d4d3fab32c8d713a680ec0fceedab341) )
	ROM_LOAD( "cl23",   0x4000, 0x2000, CRC(08385891) SHA1(d8d66664ec25db067d5a4a6c35ec0ac65b9e0c6a) )
	ROM_LOAD( "cl22a",  0x6000, 0x2000, CRC(796e0abd) SHA1(64dd9fc1f9bc44519a253ef0c02e181dd13904bf) ) // confirmed dumped from a cl22a labeled ROM. Previously MAME had it as cl22. Wrongly labeled before or do they really have same contents?
	ROM_LOAD( "cl27",   0xb000, 0x1000, CRC(3668afb8) SHA1(bcfb788baf806edcb129ea9f9dcb1d4260684773) )

	ROM_REGION( 0x00800, "mcu", 0 ) // 68705P3
	ROM_LOAD( "cl38a",  0x0000, 0x800, CRC(b70156ce) SHA1(c5eab8bbd65c4f587426298da4e22f991ce01dde) )

	ROM_REGION( 0x4000, "tiles", 0 ) // tile data
	ROM_LOAD( "cl111",  0x0000, 0x2000, CRC(41c0149d) SHA1(3ea53a3821b044b3d0451fec1b4ee2c28da393ca) )
	ROM_LOAD( "cl113",  0x2000, 0x2000, CRC(ddf99926) SHA1(e816b88302c5639c7284f4845d450f232d63a10c) )

	ROM_REGION( 0x8000, "obj0", 0 ) // obj 0 data
	ROM_LOAD( "cl100",  0x0000, 0x2000, CRC(3fa9e4fa) SHA1(9abd7df5fcf143a0c476bd8c8753c5ea294b9f74) )
	ROM_LOAD( "cl99",   0x2000, 0x2000, CRC(67b27b9e) SHA1(7df0f93851959359218c8d2272e30d242a77039d) )
	ROM_LOAD( "cl98",   0x4000, 0x2000, CRC(bffe4149) SHA1(5cf0b98f9d342bd06d575c565ea01bbd79f5e04b) )
	ROM_LOAD( "cl97",   0x6000, 0x2000, CRC(5abab8f9) SHA1(f5156855bbcdf0740fd44520386318ee53ebbf9a) )

	ROM_REGION( 0x1000, "obj1", 0 ) // obj 1 data
	ROM_LOAD( "cl46",   0x0000, 0x1000, CRC(9c0a7d28) SHA1(fac9180ea0d9aeea56a84b35cc0958f0dd86a801) )

	ROM_REGION( 0x1000, "slope", 0 ) // math tables: SLOPE ROM (river-tree schematic page 1/3)
	ROM_LOAD( "cl44",   0x0000, 0x1000, CRC(160d2bc7) SHA1(2609208c2bd4618ea340923ee01af69278980c36) ) // first and 2nd half identical

	ROM_REGION( 0x3000, "tree", 0 ) // math tables: TREE ROM (river-tree schematic page 3/3)
	ROM_LOAD( "cl7",    0x0000, 0x0800, CRC(01e3efca) SHA1(b26203787f105ba32773e37035c39253050f9c82) ) // fixed bits: 0xxxxxxx
	ROM_LOAD( "cl9",    0x1000, 0x2000, CRC(4e53cdd0) SHA1(6255411cfdccbe2c581c83f9127d582623453c3a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "cl88",   0x0000, 0x0020, CRC(da4d6625) SHA1(2d9a268973518252eb36f479ab650af8c16c885c) ) // math train state machine
ROM_END

} // anonymous namespace


/*********************************
    Game Driver
*********************************/

//     YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        SCREEN  COMPANY                      FULLNAME        FLAGS
GAMEL( 1983, changela, 0,      changela, changela, changela_state, empty_init, ROT180, "Taito America Corporation", "Change Lanes", MACHINE_SUPPORTS_SAVE, layout_changela )
