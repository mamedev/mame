// license:???
// copyright-holders:Jarek Burczynski, Phil Stroffolino, Tomasz Slanina
/**************************************************************************
Change Lanes - Video Hardware
(C) Taito 1983

Jarek Burczynski
Phil Stroffolino
Tomasz Slanina
Adam Bousley

Todo: Priority between tree0 and tree1.

***************************************************************************/

#include "emu.h"
#include "includes/changela.h"




void changela_state::video_start()
{
	m_memory_devices = auto_alloc_array(machine(), UINT8, 4 * 0x800); /* 0 - not connected, 1,2,3 - RAMs*/
	m_tree_ram = auto_alloc_array(machine(), UINT8, 2 * 0x20);

	m_screen->register_screen_bitmap(m_obj0_bitmap);
	m_screen->register_screen_bitmap(m_river_bitmap);
	m_screen->register_screen_bitmap(m_tree0_bitmap);
	m_screen->register_screen_bitmap(m_tree1_bitmap);

	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(changela_state::changela_scanline_callback),this));
	m_scanline_timer->adjust(m_screen->time_until_pos(30), 30);

	save_pointer(NAME(m_memory_devices), 4 * 0x800);
	save_pointer(NAME(m_tree_ram), 2 * 0x20);
}

/**************************************************************************

    Obj 0 - Sprite Layer

***************************************************************************/

void changela_state::draw_obj0( bitmap_ind16 &bitmap, int sy )
{
	int sx, i;

	UINT8* ROM = memregion("user1")->base();
	UINT8* RAM = m_spriteram;

	for (sx = 0; sx < 256; sx++)
	{
		int vr = (RAM[sx * 4 + 0] & 0x80) >> 7;
		int hr = (RAM[sx * 4 + 0] & 0x40) >> 6;
		int hs = (RAM[sx * 4 + 0] & 0x20) >> 5;
		UINT32 vsize = RAM[sx * 4 + 0] & 0x1f;
		UINT8 ypos = ~RAM[sx * 4 + 1];
		UINT8 tile = RAM[sx * 4 + 2];
		UINT8 xpos = RAM[sx * 4 + 3];

		if (sy - ypos <= vsize)
		{
			for (i = 0; i < 16; i++)
			{
				UINT32 A7, A8, rom_addr;
				UINT8 counter, data;
				UINT8 sum = sy - ypos;

				counter = i;
				if (hr) counter ^= 0x0f;

				A8 = ((tile & 0x02) >> 1) ^ ((hr & hs) ^ hs);
				A7 = ((((vr ^ ((sum & 0x10) >> 4)) & ((vsize & 0x10) >> 4)) ^ 0x01) & (tile & 0x01) ) ^ 0x01;
				rom_addr = (counter >> 1) | ((sum & 0x0f) << 3) | (A7 << 7) | (A8 << 8) | ((tile >> 2) << 9);
				if (vr) rom_addr ^= (0x0f << 3);

				if (counter & 1)
					data = ROM[rom_addr] & 0x0f;
				else
					data = (ROM[rom_addr] & 0xf0) >> 4;

				if ((data != 0x0f) && (data != 0))
					bitmap.pix16(sy, xpos + i) = data | 0x10;

				if (hs)
				{
					if (counter & 1)
						data = ROM[rom_addr ^ 0x100] & 0x0f;
					else
						data = (ROM[rom_addr ^ 0x100] & 0xf0) >> 4;

					if ((data != 0x0f) && (data != 0))
						bitmap.pix16(sy, xpos + i + 16) = data | 0x10;
				}
			}
		}
	}
}

/**************************************************************************

    Obj 1 - Text Layer

***************************************************************************/

void changela_state::draw_obj1( bitmap_ind16 &bitmap )
{
	int sx, sy;

	UINT8* ROM = memregion("gfx2")->base();
	UINT8* RAM = m_videoram;

	UINT8 reg[4] = { 0 }; /* 4x4-bit registers (U58, U59) */

	UINT8 tile;
	UINT8 attrib = 0;

	for (sy = 0; sy < 256; sy++)
	{
		for (sx = 0; sx < 256; sx++)
		{
			int c0, c1, col, sum;

			/* 11 Bits: H1, H3, H4, H5, H6, H7, V3, V4, V5, V6, V7 */
			int ram_addr = ((sx & 0xf8) >> 2) | ((sy & 0xf8) << 3);
			int tile_addr = RAM[ram_addr];

			if (!(RAM[ram_addr + 1] & 0x10) && (sx & 0x04)) /* D4=0 enables latch at U32 */
				attrib = RAM[ram_addr + 1];

			tile = ROM[(tile_addr << 4) | ((sx & 0x04) >> 2) | ((sy & 0x07) << 1)];
			reg[(sx & 0x0c) >> 2] = tile;
			sum = (sx & 0x0f) + (attrib & 0x0f); /* 4-bit adder (U45) */

			/* Multiplexors (U57) */
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

			col = c0 | (c1 << 1) | ((attrib & 0xc0) >> 4);
			if ((col & 0x07) != 0x07)
				bitmap.pix16(sy, sx) = col | 0x20;
		}
	}
}

/**************************************************************************

    River Video Generator

***************************************************************************/

void changela_state::draw_river( bitmap_ind16 &bitmap, int sy )
{
	int sx, i, j;

	UINT8* ROM = memregion("user2")->base();
	UINT8* RAM = m_memory_devices + 0x800;
	UINT8* TILE_ROM = memregion("gfx1")->base();
	UINT8* TILE_RAM = m_memory_devices + 0x1000;
	UINT8* PROM = memregion("proms")->base();

	int preload = ((sy < 32) ? 1 : 0);

	UINT8 math_train[10] = { 0 };
	UINT8 pre_train[3] = { 0 };

	UINT8 curr_state = 0;
	UINT8 prev_state = 0;

	UINT8 ram_count = 0;
	UINT8 rom_count = 0;

	int hosc = 0;
	int carry = 0;

	/* Update Counters */
	if (sy == 30) m_v_count_river = m_horizon;
	m_v_count_river = (m_v_count_river + 1) & 0xff;

	/* ----- STATE MACHINE ----- */
	for (i = 0; i < 0x20; i++)
	{
		int rom_addr, ram_addr, ram_a5;
		int mux45, mux61;

		curr_state = PROM[i];

		/* Update Counters */
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

		rom_addr = m_slopeROM_bank | ((m_v_count_river & 0x7e) << 2) | ((rom_count & 0x0e) >> 1);
		ram_a5 = ((curr_state & 0x01) & ((curr_state & 0x40) >> 6) & preload) ^ 0x01;
		ram_addr =  (ram_a5 << 5) | (ram_count << 1) | ((curr_state & 0x20) >> 5);
		mux45 = rom_count & 0x01;
		mux61 = m_v_count_river & 0x01;

		switch (curr_state)
		{
			case 0x01:
			case 0x09:
			case 0x19:
			case 0x0d:
			case 0x8d:
				pre_train[0] = (mux45 ? ((ROM[rom_addr] & 0xf0) >> 4) : (ROM[rom_addr] & 0x0f));
				break;
			case 0x0f:
			case 0x2f:
				math_train[0] = RAM[ram_addr] = (mux45 ? ((ROM[rom_addr] & 0xf0) >> 4) : (ROM[rom_addr] & 0x0f));
				break;
			case 0x4d:
			case 0x69:
			case 0x6d:
			case 0xc5:
			case 0xcd:
				pre_train[0] = RAM[ram_addr] & 0x0f;
				break;
			case 0xea:
			case 0xee:
				math_train[0] = RAM[ram_addr] = (mux61 ? (pre_train[1]) : ((pre_train[1] + pre_train[2] + carry) & 0x0f));
				break;
			default:
				break;
		}

		/* Shift each item down the train */
		if (curr_state & 0x02)
		{
			for (j = 9; j > 0; j--)
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
		int tile_h = (math_train[7] & 0x0f) | ((math_train[6] & 0x0f) << 4) | ((math_train[5] & 0x01) << 8);

		/* Burst of 16 10Mhz Clocks */
		for (sx = 0; sx < 16; sx++)
		{
			int ram_addr, rom_addr;
			int col;

			for (i = 0; i < 2; i++)
			{
				if (h_count > 0xff)
				{
					h_count = ((math_train[9] & 0x0f) >> 1) | ((math_train[8] & 0x0f) << 3) | 0x80;
					tile_h = (tile_h+1) & 0xfff;

					/* Skip one count if LSB is high */
					if (((math_train[9] & 0x01) && (tile_h & 0x01)))
						h_count--;
				}
				else
					h_count++;
			}

			ram_addr = ((tile_h & 0x1f8) >> 3) | ((tile_v & 0x1f0) << 2);
			rom_addr = ((tile_h & 0x06) >> 1) | ((tile_v & 0x0f) << 2) | ((TILE_RAM[ram_addr] & 0x7f) << 6);

			if (tile_h & 0x01)
				col = TILE_ROM[rom_addr] & 0x0f;
			else
				col = (TILE_ROM[rom_addr] & 0xf0) >> 4;

			bitmap.pix16(sy, sx) = col;
		}

		for (sx = 16; sx < 256; sx++)
		{
			int ram_addr, rom_addr;
			int col;

			for (i = 0; i < 4; i++)
			{
				if (h_count > 0xff)
				{
					h_count = ((math_train[9] & 0x0f) >> 1) | ((math_train[8] & 0x0f) << 3) | 0x80;
					tile_h = (tile_h+1) & 0xfff;

					/* Skip one count if LSB is high */
					if (((math_train[9] & 0x01) && (tile_h & 0x01)))
						h_count--;
				}
				else
					h_count++;
			}

			ram_addr = ((tile_h & 0x1f8) >> 3) | ((tile_v & 0x1f0) << 2);
			rom_addr = ((tile_h & 0x06) >> 1) | ((tile_v & 0x0f) << 2) | ((TILE_RAM[ram_addr] & 0x7f) << 6);

			if (tile_h & 0x01)
				col = TILE_ROM[rom_addr] & 0x0f;
			else
				col = (TILE_ROM[rom_addr] & 0xf0) >> 4;

			bitmap.pix16(sy, sx) = col;
		}
	}
}

/**************************************************************************

    Tree Generators

***************************************************************************/

void changela_state::draw_tree( bitmap_ind16 &bitmap, int sy, int tree_num )
{
	int sx, i, j;

	/* State machine */
	UINT8* ROM = memregion("user2")->base();
	UINT8* RAM = m_memory_devices + 0x840 + 0x40 * tree_num;
	UINT8* PROM = memregion("proms")->base();

	/* Tree Data */
	UINT8* RAM2 = m_tree_ram + 0x20 * tree_num;
	UINT8* TILE_ROM = (tree_num ? (memregion("user3")->base() + 0x1000) : (memregion("gfx1")->base() + 0x2000));
	UINT8* TILE_RAM = (tree_num ? (memregion("user3")->base()) : (m_memory_devices + 0x1800));

	int preload = ((sy < 32) ? 1 : 0);

	UINT8 math_train[10] = { 0 };
	UINT8 pre_train[3] = { 0 };
	UINT8 tree_train[3] = { 0 };

	UINT8 curr_state = 0;
	UINT8 prev_state = 0;

	UINT8 ram_count = 0;
	UINT8 rom_count = 0;

	int hosc = 0;
	int carry = 0;
	int tree_carry = 0;

	int h_count, tile_v, tile_h;
	int all_ff;

	/* Update Counters */
	if (sy == 30)
	{
		m_tree_on[tree_num] = 0;
		if (tree_num == 0)
			m_v_count_tree = m_horizon;
	}
	if (tree_num == 0)
		m_v_count_tree = (m_v_count_tree + 1) & 0xff;

	/* ----- STATE MACHINE ----- */
	for (i = 0; i < 0x20; i++)
	{
		int rom_addr, ram_addr, ram_a5, ram2_addr;
		int mux45, mux61;

		curr_state = PROM[i];

		/* Update Counters */
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

		rom_addr = m_slopeROM_bank | ((m_v_count_tree & 0x7e) << 2) | ((rom_count & 0x0e) >> 1);
		ram_a5 = ((curr_state & 0x01) & ((curr_state & 0x40) >> 6) & preload) ^ 0x01;
		ram_addr = (ram_a5 << 5) | (ram_count << 1) | ((curr_state & 0x20) >> 5);
		ram2_addr = (ram_count << 1) | ((curr_state & 0x20) >> 5);
		mux45 = rom_count & 0x01;
		mux61 = m_v_count_tree & 0x01;

		switch(curr_state)
		{
			case 0x01:  case 0x09:  case 0x19:  case 0x0d:  case 0x8d:
				pre_train[0] = ( mux45 ? ((ROM[rom_addr] & 0xf0) >> 4) : (ROM[rom_addr] & 0x0f) );
				break;
			case 0x0f:  case 0x2f:
				RAM[ram_addr] = ( mux45 ? ((ROM[rom_addr] & 0xf0) >> 4) : (ROM[rom_addr] & 0x0f) );
				break;
			case 0x4d:  case 0x69:  case 0x6d:  case 0xc5:  case 0xcd:
				pre_train[0] = RAM[ram_addr] & 0x0f;
				break;
			case 0xea:  case 0xee:
				RAM[ram_addr] = ( mux61 ? (pre_train[1]) : ((pre_train[1] + pre_train[2] + carry) & 0x0f) );
				break;
			default:
				break;
		}

		if (!m_tree_on[tree_num])
		{
			int mux82 = (m_v_count_tree & 0x01) ^ 0x01;

			switch(curr_state)
			{
				case 0x01:  case 0x09:  case 0x19:  case 0x0d:  case 0x8d:
					tree_train[0] = RAM2[ram2_addr] = pre_train[0];
					break;
				case 0x0f:  case 0x2f:
					math_train[0] = RAM2[ram2_addr] = RAM[ram_addr] & 0x0f;
					break;
				case 0x4d:  case 0x69:  case 0x6d:  case 0xc5:  case 0xcd:
					tree_train[0] = RAM2[ram2_addr] = pre_train[0];
					break;
				case 0xea:  case 0xee:
					math_train[0] = RAM2[ram2_addr] = ( mux82 ? ((tree_train[1] + tree_train[2] + tree_carry) & 0x0f) : (tree_train[1]) );
					break;
				default:
					break;
			}
		}
		else
		{
			int mux82 = ((curr_state & 0x04) ? 0 : 1);

			switch(curr_state)
			{
				case 0x01:  case 0x09:  case 0x19:  case 0x0d:  case 0x8d:
					tree_train[0] = RAM2[ram2_addr];
					break;
				case 0x0f:  case 0x2f:
					math_train[0] = RAM2[ram2_addr];
					break;
				case 0x4d:  case 0x69:  case 0x6d:  case 0xc5:  case 0xcd:
					tree_train[0] = RAM2[ram2_addr];
					break;
				case 0xea:  case 0xee:
					math_train[0] = RAM2[ram2_addr] = (mux82 ? ((tree_train[1] + tree_train[2] + tree_carry) & 0x0f) : (tree_train[1]));
					break;
				default:
					break;
			}
		}

		/* Shift each item down the train */
		if (curr_state & 0x02)
		{
			for (j = 9; j > 0; j--)
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
	tile_h = (math_train[7] & 0x0f) | ((math_train[6] & 0x0f) << 4) | ((math_train[5] & 0x01) << 8);
	all_ff = 1;

	/* Burst of 16 10Mhz clocks */
	for (sx = 0; sx < 16; sx++)
	{
		int ram_addr, rom_addr, col;

		for (i = 0; i < 2; i++)
		{
			if (h_count > 0xff)
			{
				h_count = ((math_train[9] & 0x0f) >> 1) | ((math_train[8] & 0x0f) << 3) | 0x80;
				tile_h = (tile_h+1) & 0xfff;

				/* Skip one count if LSB is high */
				if (((math_train[9] & 0x01) && (tile_h & 0x01)))
					h_count--;
			}
			else
				h_count++;
		}

		ram_addr = ((tile_h & 0x1f8) >> 3) | ((tile_v & 0x1f0) << 2);
		rom_addr = ((tile_h & 0x06) >> 1) | ((tile_v & 0x0f) << 2) | ((TILE_RAM[ram_addr] & 0x7f) << 6);

		if (!(m_v_count_tree & 0x80) && (m_tree_en & (0x01 << tree_num)) && ((TILE_ROM[rom_addr] & 0xf0) == 0))
			m_tree_on[tree_num] = 1;

		if (m_tree_on[tree_num])
		{
			if (tile_h & 0x01)
				col = TILE_ROM[rom_addr] & 0x0f;
			else
				col = (TILE_ROM[rom_addr] & 0xf0) >> 4;

			if (col != 0x0f)
				all_ff = 0;

			if (col != 0x0f && col != 0x00)
				bitmap.pix16(sy, sx) = col | 0x30;
		}
	}

	for (sx = 16; sx < 256; sx++)
	{
		int ram_addr, rom_addr, col;

		for (i = 0; i < 4; i++)
		{
			if (h_count > 0xff)
			{
				h_count = ((math_train[9] & 0x0f) >> 1) | ((math_train[8] & 0x0f) << 3) | 0x80;
				tile_h = (tile_h+1) & 0xfff;

				/* Skip one count if LSB is high */
				if (((math_train[9] & 0x01) && (tile_h & 0x01)))
					h_count--;
			}
			else
				h_count++;
		}

		ram_addr = ((tile_h & 0x1f8) >> 3) | ((tile_v & 0x1f0) << 2);
		rom_addr = ((tile_h & 0x06) >> 1) | ((tile_v & 0x0f) << 2) | ((TILE_RAM[ram_addr] & 0x7f) << 6);

		if (!(m_v_count_tree & 0x80) && (m_tree_en & (0x01 << tree_num)) && ((TILE_ROM[rom_addr] & 0xf0) == 0))
			m_tree_on[tree_num] = 1;

		if (m_tree_on[tree_num])
		{
			if (tile_h & 0x01)
				col = TILE_ROM[rom_addr] & 0x0f;
			else
				col = (TILE_ROM[rom_addr] & 0xf0) >> 4;

			if (col != 0x0f)
				all_ff = 0;

			if (col != 0x0f && col != 0x00)
				bitmap.pix16(sy, sx) = col | 0x30;
		}
	}

	/* Tree on only stays high if a pixel that is not 0xf is encountered,
	   because any non 0xf pixel sets U56 high */
	if (all_ff) m_tree_on[tree_num] = 0;
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

TIMER_CALLBACK_MEMBER(changela_state::changela_scanline_callback)
{
	int sy = param;
	int sx;

	/* clear the current scanline first */
	const rectangle rect(0, 255, sy, sy);
	m_river_bitmap.fill(0x00, rect);
	m_obj0_bitmap.fill(0x00, rect);
	m_tree0_bitmap.fill(0x00, rect);
	m_tree1_bitmap.fill(0x00, rect);

	draw_river(m_river_bitmap, sy);
	draw_obj0(m_obj0_bitmap, sy);
	draw_tree(m_tree0_bitmap, sy, 0);
	draw_tree(m_tree1_bitmap, sy, 1);

	/* Collision Detection */
	for (sx = 1; sx < 256; sx++)
	{
		int riv_col, prev_col;

		if ((m_river_bitmap.pix16(sy, sx) == 0x08)
		|| (m_river_bitmap.pix16(sy, sx) == 0x09)
		|| (m_river_bitmap.pix16(sy, sx) == 0x0a))
			riv_col = 1;
		else
			riv_col = 0;

		if ((m_river_bitmap.pix16(sy, sx-1) == 0x08)
		|| (m_river_bitmap.pix16(sy, sx-1) == 0x09)
		|| (m_river_bitmap.pix16(sy, sx-1) == 0x0a))
			prev_col = 1;
		else
			prev_col = 0;

		if (m_obj0_bitmap.pix16(sy, sx) == 0x14) /* Car Outline Color */
		{
			/* Tree 0 Collision */
			if (m_tree0_bitmap.pix16(sy, sx) != 0)
				m_tree0_col = 1;

			/* Tree 1 Collision */
			if (m_tree1_bitmap.pix16(sy, sx) != 0)
				m_tree1_col = 1;

			/* Hit Right Bank */
			if (riv_col == 0 && prev_col == 1)
				m_right_bank_col = 1;

			/* Hit Left Bank */
			if (riv_col == 1 && prev_col == 0)
				m_left_bank_col = 1;

			/* Boat Hit Shore */
			if (riv_col == 1)
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

	sy++;
	if (sy > 256) sy = 30;
	m_scanline_timer->adjust(m_screen->time_until_pos(sy), sy);
}

UINT32 changela_state::screen_update_changela(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_river_bitmap, 0, 0, 0, 0, cliprect);
	copybitmap_trans(bitmap, m_obj0_bitmap,  0, 0, 0, 0, cliprect, 0);
	copybitmap_trans(bitmap, m_tree0_bitmap, 0, 0, 0, 0, cliprect, 0);
	copybitmap_trans(bitmap, m_tree1_bitmap, 0, 0, 0, 0, cliprect, 0);
	draw_obj1(bitmap);

	return 0;
}

WRITE8_MEMBER(changela_state::changela_colors_w)
{
	/* Each color is combined from 3 bits from open-colelctor outputs of ram.
	Each of the bits is connected to a 220, 470, or 1000 Ohm resistor.
	There is also a 680 Ohm pull-up resistor connected to 5V, and a
	2.2k resisor connected to GND. Thus these output voltages are obtained:
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

	static const UINT8 color_table[8] = { 0, 7, 18, 31, 58, 88, 146, 255 };

	int r, g, b;
	UINT32 c, color_index;

	c = (data) | ((offset & 0x01) << 8); /* a0 used as D8 bit input */

	c ^= 0x1ff; /* active low */

	color_index = offset >> 1;
	color_index ^= 0x30;    /* A4 and A5 lines are negated */

	r = color_table[(c >> 0) & 0x07];
	g = color_table[(c >> 3) & 0x07];
	b = color_table[(c >> 6) & 0x07];

	m_palette->set_pen_color(color_index,r,g,b);
}


WRITE8_MEMBER(changela_state::changela_mem_device_select_w)
{
	m_mem_dev_selected = (data & 0x07) * 0x800;
	m_tree_en = (data & 0x30) >> 4;

	/*
	(data & 0x07) possible settings:
	0 - not connected (no device)
	1 - ADR1 is 2114 RAM at U59 (state machine()) (accessible range: 0x0000-0x003f)
	2 - ADR2 is 2128 RAM at U109 (River RAM)    (accessible range: 0x0000-0x07ff)
	3 - ADR3 is 2128 RAM at U114 (Tree RAM)    (accessible range: 0x0000-0x07ff)
	4 - ADR4 is 2732 ROM at U7    (Tree ROM)    (accessible range: 0x0000-0x07ff)
	5 - SLOPE is ROM at U44 (state machine())     (accessible range: 0x0000-0x07ff)
	*/
}

WRITE8_MEMBER(changela_state::changela_mem_device_w)
{
	m_memory_devices[m_mem_dev_selected + offset] = data;

	if (m_mem_dev_selected == 0x800)
	{
		m_memory_devices[m_mem_dev_selected + 0x40 + offset] = data;
		m_memory_devices[m_mem_dev_selected + 0x80 + offset] = data;
	}
}


READ8_MEMBER(changela_state::changela_mem_device_r)
{
	return m_memory_devices[m_mem_dev_selected + offset];
}


WRITE8_MEMBER(changela_state::changela_slope_rom_addr_hi_w)
{
	m_slopeROM_bank = (data & 0x03) << 9;
}

WRITE8_MEMBER(changela_state::changela_slope_rom_addr_lo_w)
{
	m_horizon = data;
}
