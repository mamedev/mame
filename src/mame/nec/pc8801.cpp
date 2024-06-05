// license:BSD-3-Clause
// copyright-holders: Angelo Salese
// thanks-to: Alex Marshall
/**************************************************************************************************

    PC-8801 (c) 1981 NEC

    driver by Angelo Salese, original MESS PC-88SR driver by ???

    TODO:
    - cassette support;
    - waitstates;
    - support for partial palette updates (pretty off in p8suite analog RGB test if enabled);
    - understand why i8214 needs a dis hack setter (depends on attached i8212?);
    - clean-ups:
      - slotify extended work RAM, make sure that p8suite memtest88 detects it properly;
      - better state machine isolation of features between various models.
        Vanilla PC-8801 doesn't have analog palette, PC80S31 device as default
        (uses external minidisk), only model with working border color, other misc banking bits.
      - refactor memory banking to use address maps & views;
      - double check dipswitches;
      - Slotify PC80S31K, also needed by PC-6601SR, PC-88VA, (vanilla & optional) PC-9801. **partially done**
        Also notice that there are common points with SPC-1000 and TF-20 FDDs;
      - backport/merge what is portable to PC-8001;
      - Kanji LV1/LV2 ROM hookups needs to be moved at slot level.
        Needs identification effort about what's internal to machine models and what instead
        can be optionally installed;
    - implement proper joypad / mouse (PC-8872) DB9 port connector, consider deriving from vcs_ctrl;
    - Pinpoint number of EXPansion slots for each machine (currently hardwired to 1),
      guessing from the back panels seems that each model can install between 1 to 3 cards.
      Also note: most cards aren't compatible between each other;

    Notes:
    - Later models have washed out palette with some SWs, with no red component.
      This is because you have to set up the V1 / V2 DIP-SW to V1 mode for those games
      (BIOS sets up analog palette and never changes back otherwise).
      cfr. SW list usage SW notes that specifically needs V1.

======================================================================================================================================

    PC-88xx Models (and similar machines like PC-80xx and PC-98DO)

    Model            | release |      CPU     |                      BIOS components                        |       |
                     |         |     clock    | N-BASIC | N88-BASIC | N88-BASIC Enh |  Sound  |  CD |  Dict |  Disk | Notes
    ==================================================================================================================================
    PC-8001          | 1979-03 |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   |
    PC-8001A         |   ??    |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   | (U)
    PC-8801          | 1981-11 |   z80A @ 4   |    X    |     X     |       -       |    -    |  -  |   -   |   -   | (KO)
    PC-8801A         |   ??    |   z80A @ 4   |    X    |     X     |       -       |    -    |  -  |   -   |   -   | (U)
    PC-8001 mkII     | 1983-03 |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   | (GE),(KO)
    PC-8001 mkIIA    |   ??    |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   | (U),(GE)
    PC-8801 mkII     | 1983-11 |   z80A @ 4   |    X    |     X     |       -       |    -    |  -  |   -   | (FDM) | (K1)
    PC-8001 mkII SR  | 1985-01 |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   | (GE),(NE),(KO)
    PC-8801 mkII SR  | 1985-03 |   z80A @ 4   |    X    |     X     |       X       |    X    |  -  |   -   | (FDM) | (K1)
    PC-8801 mkII TR  | 1985-10 |   z80A @ 4   |    X    |     X     |       X       |    X    |  -  |   -   | (FD2) | (K1)
    PC-8801 mkII FR  | 1985-11 |   z80A @ 4   |    X    |     X     |       X       |    X    |  -  |   -   | (FDM) | (K1)
    PC-8801 mkII MR  | 1985-11 |   z80A @ 4   |    X    |     X     |       X       |    X    |  -  |   -   | (FDH) | (K2)
    PC-8801 FH       | 1986-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FDM) | (K2)
    PC-8801 MH       | 1986-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FDH) | (K2)
    PC-88 VA         | 1987-03 | z80H+v30 @ 8 |    -    |     X     |       X       |    X    |  -  |   X   | (FDH) | (K2)
    PC-8801 FA       | 1987-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FD2) | (K2)
    PC-8801 MA       | 1987-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   X   | (FDH) | (K2)
    PC-88 VA2        | 1988-03 | z80H+v30 @ 8 |    -    |     X     |       X       |    X    |  -  |   X   | (FDH) | (K2)
    PC-88 VA3        | 1988-03 | z80H+v30 @ 8 |    -    |     X     |       X       |    X    |  -  |   X   | (FD3) | (K2)
    PC-8801 FE       | 1988-10 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FD2) | (K2)
    PC-8801 MA2      | 1988-10 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   X   | (FDH) | (K2)
    PC-98 DO         | 1989-06 |   z80H @ 8   |    X    |     X     |       X       |    X    |  -  |   -   | (FDH) | (KE)
    PC-8801 FE2      | 1989-10 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FD2) | (K2)
    PC-8801 MC       | 1989-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  X  |   X   | (FDH) | (K2)
    PC-98 DO+        | 1990-10 |   z80H @ 8   |    X    |     X     |       X       |    X    |  -  |   -   | (FDH) | (KE)

    info for PC-98 DO & DO+ refers to their 88-mode

    Disk Drive options:
    (FDM): there exist three model of this computer: Model 10 (base model, only optional floppy drive), Model 20
        (1 floppy drive for 5.25" 2D disks) and Model 30 (2 floppy drive for 5.25" 2D disks)
    (FD2): 2 floppy drive for 5.25" 2D disks
    (FDH): 2 floppy drive for both 5.25" 2D disks and 5.25" HD disks
    (FD3): 2 floppy drive for both 5.25" 2D disks and 5.25" HD disks + 1 floppy drive for 3.5" 2TD disks

    Notes:
    (U): US version
    (GE): Graphic Expansion for PC-8001
    (NE): N-BASIC Expansion for PC-8001 (similar to N88-BASIC Expansion for PC-88xx)
    (KO): Optional Kanji ROM
    (K1): Kanji 1st Level ROM
    (K2): Kanji 2nd Level ROM
    (KE): Kanji Enhanced ROM

    Memory mounting locations:
     * N-BASIC 0x0000 - 0x5fff, N-BASIC Expansion & Graph Enhhancement 0x6000 - 0x7fff
     * N-BASIC 0x0000 - 0x5fff, N-BASIC Expansion & Graph Enhhancement 0x6000 - 0x7fff
     * N88-BASIC 0x0000 - 0x7fff, N88-BASIC Expansion & Graph Enhhancement 0x6000 - 0x7fff
     * Sound BIOS: 0x6000 - 0x7fff
     * CD-ROM BIOS: 0x0000 - 0x7fff
     * Dictionary: 0xc000 - 0xffff (32 Banks)

    References:
    - https://retrocomputerpeople.web.fc2.com/machines/nec/8801/
    - http://mydocuments.g2.xrea.com/html/p8/vraminfo.html
    - http://www7b.biglobe.ne.jp/~crazyunit/pc88.html
    - http://www.maroon.dti.ne.jp/youkan/pc88/index.html

*************************************************************************************************************************************/


#include "emu.h"
#include "pc8801.h"

#include "softlist_dev.h"

#include "utf8.h"


#define PC8801FH_OSC1   XTAL(28'636'363)
#define PC8801FH_OSC2   XTAL(42'105'200)
#define PC8801FH_OSC3   XTAL(31'948'800)    // called OSC1 on PC-8801FE board

#define MASTER_CLOCK (PC8801FH_OSC3 / 8)
// Crazy Unit page shows following measurements on N88 BASIC:
// 15kHz 25 lines 62.422 Hz, 20 lines 61.462 Hz
#define PIXEL_CLOCK_15KHz (PC8801FH_OSC1 / 2)
// 24kHz 25 lines 55.416 Hz, 20 lines 56.424 Hz
#define PIXEL_CLOCK_24KHz (PC8801FH_OSC2 / 2)
// Note: games may set these up differently, namely xak2 in 15 kHz going 68-ish Hz

void pc8801_state::video_start()
{
	m_screen->register_screen_bitmap(m_text_bitmap);

	save_item(NAME(m_attr_info));
}

void pc8801_state::palette_reset()
{
	int i;

	// bitmap init
	for (i = 0; i < 8; i ++)
	{
		m_palram[i].b = i & 1 ? 7 : 0;
		m_palram[i].r = i & 2 ? 7 : 0;
		m_palram[i].g = i & 4 ? 7 : 0;
		m_palette->set_pen_color(i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
	}
	m_palette->set_pen_color(BGPAL_PEN, 0, 0, 0);
	m_palette->set_pen_color(BORDER_PEN, 0, 0, 0);
}

UPD3301_FETCH_ATTRIBUTE( pc8801_state::attr_fetch )
{
	const u8 attr_max_size = 80;
	std::array<u16, attr_max_size> attr_extend_info = pc8001_base_state::attr_fetch(attr_row, gfx_mode, y, attr_fifo_size, row_size);
	// In case we are in a b&w mode copy the attribute structure in an internal buffer for color processing.
	// It's unknown at this time if decoration attributes applies to bitmap as well
	if ((m_gfx_ctrl & 0x18) == 0x08 && gfx_mode == 2)
	{
		for (int ey = y; ey < y + m_crtc->lines_per_char(); ey ++)
			for (int ex = 0; ex < attr_max_size; ex ++)
				m_attr_info[ey][ex] = attr_extend_info[ex];
	}
	return attr_extend_info;
}

void pc8801_state::draw_bitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device *palette, std::function<u8(u32 bitmap_offset, int y, int x, int xi)> dot_func)
{
	uint16_t y_double = get_screen_frequency();
	if ((m_gfx_ctrl & 0x11) == 0)
		y_double = 0;
	int32_t y_line_size = y_double + 1;

	for(int y = cliprect.min_y; y <= cliprect.max_y; y += y_line_size)
	{
		for(int x = cliprect.min_x; x <= cliprect.max_x; x += 8)
		{
			u8 x_char = (x >> 3);
			u32 bitmap_offset = (y >> y_double) * 80 + x_char;
			for(int xi = 0; xi < 8; xi++)
			{
				u8 pen_dot = dot_func(bitmap_offset, y, x_char, 7 - xi);

				if (pen_dot == 0)
					continue;

				// TODO: some real HW snaps implies that output is only even or odd line when in 3bpp mode, verify
				// 3301 skip line? interlace artifact? other?
				for (int yi = 0; yi < y_line_size; yi ++)
				{
					int res_x = x + xi;
					int res_y = y + yi;
					// still need to check against cliprect,
					// in the rare case that 3301 CRTC is set to non-canon values (such as any width != 640).
					if (cliprect.contains(res_x, res_y))
						bitmap.pix(res_y, res_x) = palette->pen(pen_dot);
				}
			}
		}
	}
}

uint32_t pc8801_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if(m_gfx_ctrl & 8)
	{
		// BG Pal applies to 1bpp mode only, sharrier draws blue backdrop with pen #0
		const bool bitmap_color_mode = bool(m_gfx_ctrl & 0x10);
		bitmap.fill(m_palette->pen(bitmap_color_mode ? 0 : BGPAL_PEN), cliprect);

		if(bitmap_color_mode)
			draw_bitmap(bitmap, cliprect, m_palette, [&](u32 bitmap_offset, int y, int x, int xi){
				u8 res = 0;

				// note: layer masking doesn't occur in 3bpp mode, bugattac relies on this
				for (int plane = 0; plane < 3; plane ++)
					res |= ((m_gvram[bitmap_offset + plane * 0x4000] >> xi) & 1) << plane;

				return res;
			});
		else
		{
			if (m_gfx_ctrl & 1)
			{
				// b&w 640x200x3
				draw_bitmap(bitmap, cliprect, m_palette, [&](u32 bitmap_offset, int y, int x, int xi){
					u8 res = 0;

					// in this mode all three planes can potentially form the output
					// it's the only place where I/O $53 bits 1-3 have an actual effect
					for (int plane = 0; plane < 3; plane ++)
					{
						u8 mask = (m_bitmap_layer_mask >> plane) & 1;
						res |= ((m_gvram[bitmap_offset + plane * 0x4000] >> xi) & mask);
					}

					if (!res)
						return 0;

					return m_crtc->is_gfx_color_mode() ? (m_attr_info[y][x] >> 13) & 7 : 7;
				});
			}
			else
			{
				// true 400 line mode, 640x400x1
				// - p1demo2d expects to use CRTC palette on demonstration
				//   (white text that is set to black on previous title screen animation,
				//    that runs in 3bpp)
				// - byoin set a transparent text layer (ASCII=0x20 / attribute = 0x80 0x00)
				//   but it's in gfx_mode = 0 (b&w) so it just draw white from here.
				draw_bitmap(bitmap, cliprect, m_crtc_palette, [&](u32 bitmap_offset, int y, int x, int xi){
					u8 res = 0;
					// HW pick ups just the first two planes (R and B), G is unused for drawing purposes.
					// Plane switch happens at half screen, VRAM areas 0x3e80-0x3fff is unused again.
					// TODO: confirm that a 15 kHz monitor cannot work with this
					// - jettermi just uses the other b&w mode;
					// - casablan/byoin doesn't bother in changing resolution so only the upper part is drawn.
					// Update: real HW capture shows an ugly overlap with the two layers,
					// implying that the second plane just latches on the same signals as the first,
					// YAGNI unless found in concrete example.
					int plane_offset = y >= 200 ? 384 : 0;

					res |= ((m_gvram[bitmap_offset + plane_offset] >> xi) & 1);
					if (!res)
						return 0;

					return m_crtc->is_gfx_color_mode() ? (m_attr_info[y][x] >> 13) & 7 : 7;
				});
			}
		}
	}
	else
		bitmap.fill(0, cliprect);

	if(!m_text_layer_mask)
	{
		m_text_bitmap.fill(0, cliprect);
		m_crtc->screen_update(screen, m_text_bitmap, cliprect);
		copybitmap_trans(bitmap, m_text_bitmap, 0, 0, 0, 0, cliprect, 0);
	}

	return 0;
}

uint8_t pc8801_state::dma_mem_r(offs_t offset)
{
	// TODO: TVRAM readback
	return m_work_ram[offset & 0xffff];
}

uint8_t pc8801_state::alu_r(offs_t offset)
{
	uint8_t b, r, g;

	/* store data to ALU regs */
	for(int i = 0; i < 3; i++)
		m_alu_reg[i] = m_gvram[i*0x4000 + offset];

	b = m_gvram[offset + 0x0000];
	r = m_gvram[offset + 0x4000];
	g = m_gvram[offset + 0x8000];
	if(!(m_alu_ctrl2 & 1)) { b^=0xff; }
	if(!(m_alu_ctrl2 & 2)) { r^=0xff; }
	if(!(m_alu_ctrl2 & 4)) { g^=0xff; }

	return b & r & g;
}

void pc8801_state::alu_w(offs_t offset, uint8_t data)
{
	int i;

	// ALU write mode
	switch(m_alu_ctrl2 & 0x30)
	{
		// logic operation
		case 0x00:
		{
			uint8_t logic_op;

			for(i = 0; i < 3; i++)
			{
				logic_op = (m_alu_ctrl1 & (0x11 << i)) >> i;

				switch(logic_op)
				{
					case 0x00: { m_gvram[i*0x4000 + offset] &= ~data; } break;
					case 0x01: { m_gvram[i*0x4000 + offset] |= data; } break;
					case 0x10: { m_gvram[i*0x4000 + offset] ^= data; } break;
					case 0x11: break; // NOP
				}
			}
		}
		break;

		// restore data from ALU regs
		case 0x10:
		{
			for(i = 0; i < 3; i++)
				m_gvram[i*0x4000 + offset] = m_alu_reg[i];
		}
		break;

		// swap ALU reg 1 into R GVRAM
		case 0x20:
			m_gvram[0x0000 + offset] = m_alu_reg[1];
			break;

		// swap ALU reg 0 into B GVRAM
		case 0x30:
			m_gvram[0x4000 + offset] = m_alu_reg[0];
			break;
	}
}


uint8_t pc8801_state::wram_r(offs_t offset)
{
	return m_work_ram[offset];
}

void pc8801_state::wram_w(offs_t offset, uint8_t data)
{
	m_work_ram[offset] = data;
}

uint8_t pc8801_state::ext_wram_r(offs_t offset)
{
	if(offset < m_extram_size)
		return m_ext_work_ram[offset];

	return 0xff;
}

void pc8801_state::ext_wram_w(offs_t offset, uint8_t data)
{
	if(offset < m_extram_size)
		m_ext_work_ram[offset] = data;
}

uint8_t pc8801_state::nbasic_rom_r(offs_t offset)
{
	return m_n80rom[offset];
}

uint8_t pc8801_state::n88basic_rom_r(offs_t offset)
{
	return m_n88rom[offset];
}

uint8_t pc8801_state::gvram_r(offs_t offset)
{
	return m_gvram[offset];
}

void pc8801_state::gvram_w(offs_t offset, uint8_t data)
{
	m_gvram[offset] = data;
}

uint8_t pc8801_state::high_wram_r(offs_t offset)
{
	return m_hi_work_ram[offset];
}

void pc8801_state::high_wram_w(offs_t offset, uint8_t data)
{
	m_hi_work_ram[offset] = data;
}

// TODO: remove these virtual trampolines once we modernize memory map
// Needs confirmation about really not being there tho, given the design
// may be that both dictionary and CD-ROM are generic slots instead.
inline uint8_t pc8801_state::dictionary_rom_r(offs_t offset)
{
	return 0xff;
}

inline bool pc8801_state::dictionary_rom_enable()
{
	return false;
}

inline uint8_t pc8801_state::cdbios_rom_r(offs_t offset)
{
	return 0xff;
}

inline bool pc8801_state::cdbios_rom_enable()
{
	return false;
}

uint8_t pc8801_state::mem_r(offs_t offset)
{
	if(offset <= 0x7fff)
	{
		if(m_extram_mode & 1)
			return ext_wram_r(offset | (m_extram_bank * 0x8000));

		if(m_gfx_ctrl & 2)
			return wram_r(offset);

		if(cdbios_rom_enable())
			return cdbios_rom_r(offset & 0x7fff);

		if(m_gfx_ctrl & 4)
			return nbasic_rom_r(offset);

		if(offset >= 0x6000 && offset <= 0x7fff && ((m_ext_rom_bank & 1) == 0))
			return n88basic_rom_r(0x8000 + (offset & 0x1fff) + (0x2000 * (m_misc_ctrl & 3)));

		return n88basic_rom_r(offset);
	}
	else if(offset >= 0x8000 && offset <= 0x83ff) // work RAM window
	{
		uint32_t window_offset;

		// work RAM read select or N-Basic select always banks this as normal work RAM
		if(m_gfx_ctrl & 6)
			return wram_r(offset);

		window_offset = (offset & 0x3ff) + (m_window_offset_bank << 8);

		// castlex and imenes accesses this
		// TODO: high TVRAM even
		if(((window_offset & 0xf000) == 0xf000) && (m_misc_ctrl & 0x10))
			return high_wram_r(window_offset & 0xfff);

		return wram_r(window_offset);
	}
	else if(offset >= 0x8400 && offset <= 0xbfff)
	{
		return wram_r(offset);
	}
	else if(offset >= 0xc000 && offset <= 0xffff)
	{
		if(dictionary_rom_enable())
			return dictionary_rom_r(offset & 0x3fff);

		if(m_misc_ctrl & 0x40)
		{
			if(!machine().side_effects_disabled())
				m_vram_sel = 3;

			if(m_alu_ctrl2 & 0x80)
				return alu_r(offset & 0x3fff);
		}

		if(m_vram_sel == 3)
		{
			if(offset >= 0xf000 && offset <= 0xffff && (m_misc_ctrl & 0x10))
				return high_wram_r(offset & 0xfff);

			return wram_r(offset);
		}

		return gvram_r((offset & 0x3fff) + (0x4000 * m_vram_sel));
	}

	return 0xff;
}

void pc8801_state::mem_w(offs_t offset, uint8_t data)
{
	if(offset <= 0x7fff)
	{
		if(m_extram_mode & 0x10)
			ext_wram_w(offset | (m_extram_bank * 0x8000),data);
		else
			wram_w(offset,data);

		return;
	}
	else if(offset >= 0x8000 && offset <= 0x83ff)
	{
		// work RAM read select or N-Basic select always banks this as normal work RAM
		if(m_gfx_ctrl & 6)
			wram_w(offset,data);
		else
		{
			uint32_t window_offset;

			window_offset = (offset & 0x3ff) + (m_window_offset_bank << 8);

			// castlex and imenes accesses this
			// TODO: high TVRAM even
			// μPD3301 DMAs from this instead of the regular work RAM in later models
			// to resolve a bus bottleneck.
			if(((window_offset & 0xf000) == 0xf000) && (m_misc_ctrl & 0x10))
				high_wram_w(window_offset & 0xfff,data);
			else
				wram_w(window_offset,data);
		}

		return;
	}
	else if(offset >= 0x8400 && offset <= 0xbfff)
	{
		wram_w(offset,data);
		return;
	}
	else if(offset >= 0xc000 && offset <= 0xffff)
	{
		if(m_misc_ctrl & 0x40)
		{
			if(!machine().side_effects_disabled())
				m_vram_sel = 3;

			if(m_alu_ctrl2 & 0x80)
			{
				alu_w(offset & 0x3fff,data);
				return;
			}
		}

		if(m_vram_sel == 3)
		{
			if(offset >= 0xf000 && offset <= 0xffff && (m_misc_ctrl & 0x10))
			{
				high_wram_w(offset & 0xfff,data);
				return;
			}

			wram_w(offset,data);
			return;
		}

		gvram_w((offset & 0x3fff) + (0x4000 * m_vram_sel),data);
		return;
	}
}

void pc8801_state::main_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(pc8801_state::mem_r), FUNC(pc8801_state::mem_w));
}

uint8_t pc8801_state::ext_rom_bank_r()
{
	return m_ext_rom_bank;
}

void pc8801_state::ext_rom_bank_w(uint8_t data)
{
	// TODO: bits 1 to 3 written to at POST
	// selection for EXP slot ROMs?
	m_ext_rom_bank = data;
}

// inherited from pc8001.cpp
#if 0
void pc8801_state::port30_w(uint8_t data)
{
	m_txt_width = data & 1;
	m_txt_color = data & 2;

	m_cassette->change_state(BIT(data, 3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}
#endif

/*
 * I/O Port $31 (w/o) "System Control Port (2)"
 * N88-BASIC buffer port $e6c2
 *
 * --x- ---- 25LINE: line control in high speed CRT mode (1) 25 lines (0) 20 lines
 * ---x ---- HCOLOR: color graphic display mode
 * ---1 ----         color mode
 * ---0 ----         monochrome mode
 * ---- x--- GRPH: Graphic display mode yes (1) / no (0)
 * ---- -x-- RMODE: ROM mode control N-BASIC (1, ROM 1 & 2) / N88-BASIC (0, ROM 3 & 4)
 * ---- --x- MMODE: RAM mode control yes (1, full RAM) / no (0, ROM/RAM mixed)
 * ---- ---x 200LINE: 200 lines (1) / 400 lines (0) in 1bpp mode
 *
 */
void pc8801_state::port31_w(uint8_t data)
{
	m_gfx_ctrl = data;

//  set_screen_frequency((data & 0x11) != 0x11);
//  dynamic_res_change();
}

/*
 * I/O Port $40 reads "Strobe Port"
 *
 * 1--- ---- UOP2: SW1-8
 * -1-- ---- UOP1:
 * --x- ---- VRTC: vblank signal (0) display (1) vblank
 * ---x ---- CDI: upd1990a data read
 * ---- x--- /EXTON: Minidisc unit connection signal (SW2-7)
 * ---- -x-- DCD: SIO Data Carrier Detect signal (0) no carrier (1) with
 * ---- --x- /SHG: monitor resolution mode (0) high res (1) normal res
 * ---- ---x BUSY: printer (0) READY (1) BUSY
 *
 */
uint8_t pc8801_state::port40_r()
{
	// TODO: merge with PC8001
	uint8_t data = 0x00;

	data |= m_centronics_busy;
//  data |= m_centronics_ack << 1;
	data |= ioport("CTRL")->read() & 0xca;
	data |= m_rtc->data_out_r() << 4;
	data |= m_crtc->vrtc_r() << 5;
	// TODO: enable line from pc80s31k (bit 3, active_low)

	return data;
}

/*
 * I/O Port $40 writes "Strobe Port"
 * N88-BASIC buffer port $e6c1
 *
 * x--- ---- UOP2: general purpose output 2 / sound port
 *                 SING (buzzer mask?)
 * -x-- ---- UOP1: general purpose output 1
 *                 generally used for mouse latch (JOP1, routes on OPN sound port A)
 * --x- ---- BEEP: beeper enable
 * ---x ---- FLASH: flash mode control (active high)
 * ---- x--- /CLDS: "CRT I/F sync control" (init CRT and controller sync pulses?)
 * ---- -x-- CCK: upd1990a clock bit
 * ---- --x- CSTB: upd1990a strobe bit
 * ---- ---x /PSTB: printer strobe (active low)
 *
 */
void pc8801_state::port40_w(uint8_t data)
{
	// TODO: merge (and fix) from pc8001.cpp
	m_centronics->write_strobe(BIT(data, 0));

	m_rtc->stb_w(BIT(data, 1));
	m_rtc->clk_w(BIT(data, 2));

	if(((m_device_ctrl_data & 0x20) == 0x00) && ((data & 0x20) == 0x20))
		m_beeper->set_state(1);

	if(((m_device_ctrl_data & 0x20) == 0x20) && ((data & 0x20) == 0x00))
		m_beeper->set_state(0);

	m_mouse_port->pin_8_w(BIT(data, 6));

	// TODO: is SING a buzzer mask? bastard leaves beeper to ON state otherwise
	if(m_device_ctrl_data & 0x80)
		m_beeper->set_state(0);

	m_device_ctrl_data = data;
}

uint8_t pc8801_state::vram_select_r()
{
	return 0xf8 | ((m_vram_sel == 3) ? 0 : (1 << m_vram_sel));
}

void pc8801_state::vram_select_w(offs_t offset, uint8_t data)
{
	m_vram_sel = offset & 3;
}

void pc8801_state::irq_level_w(uint8_t data)
{
	m_pic->b_sgs_w(~data);
}

/*
 * ---- -x-- /RXMF RXRDY irq mask
 * ---- --x- /VRMF VRTC irq mask
 * ---- ---x /RTMF Real-time clock irq mask
 *
 */
void pc8801_state::irq_mask_w(uint8_t data)
{
	m_irq_state.enable &= ~7;
	// mapping reversed to the correlated irq levels
	m_irq_state.enable |= bitswap<3>(data & 7, 0, 1, 2);

	check_irq(RXRDY_IRQ_LEVEL);
	check_irq(VRTC_IRQ_LEVEL);
	check_irq(CLOCK_IRQ_LEVEL);
}


uint8_t pc8801_state::window_bank_r()
{
	return m_window_offset_bank;
}

void pc8801_state::window_bank_w(uint8_t data)
{
	m_window_offset_bank = data;
}

void pc8801_state::window_bank_inc_w(uint8_t data)
{
	m_window_offset_bank ++;
	m_window_offset_bank &= 0xff;
}

/*
 * I/O Port $32 (R/W)
 * Not on vanilla PC-8801 (mkII onward)
 *
 * x--- ---- sound irq mask (0) irq enabled (1) irq masked
 * -x-- ---- Graphic VRAM access mode (0) independent access mode (1) ALU mode
 * --x- ---- analog (1) / digital (0) palette select
 * ---x ---- high speed RAM select (for TVRAM) (1) main RAM bank (0) dedicated Text RAM
 * ---- xx-- Screen output mode
 * ---- 00-- TV / video mode
 * ---- 01-- None (as in disabling the screen entirely?)
 * ---- 10-- Analog RGB mode
 * ---- 11-- Optional mode
 * ---- --xx internal EROM selection
 */
uint8_t pc8801_state::misc_ctrl_r()
{
	return m_misc_ctrl;
}

void pc8801_state::misc_ctrl_w(uint8_t data)
{
	m_misc_ctrl = data;

	m_sound_irq_enable = ((data & 0x80) == 0);

	// Note: this will map to no irq anyway if there isn't any device interested in INT4
	if (m_sound_irq_enable)
		int4_irq_w(m_sound_irq_pending);
}

/*
 * I/O Port $52 "Border and background color control"
 *
 * -RGB ---- BGx: Background color, index for pen #0
 * ---- -RGB Rx: Border color
 *
 * NB: according to several sources a non-vanilla PC8801 hardwires border to black,
 *     leaving this portion unconnected.
 *     For debugging reasons we leave it in for every machine instead.
 *
 */
void pc8801_state::bgpal_w(uint8_t data)
{
	// sorcerml uses BG Pal extensively:
	// - On bootup message it sets register $54 to white and bgpal to 0, expecting the layer to be transparent;
	// - On playlist sets BG Pal to 0x10 (blue background);
	m_palette->set_pen_color(BGPAL_PEN, pal1bit(BIT(data, 6)), pal1bit(BIT(data, 5)), pal1bit(BIT(data, 4)));
	m_palette->set_pen_color(BORDER_PEN, pal1bit(BIT(data, 2)), pal1bit(BIT(data, 1)), pal1bit(BIT(data, 0)));
}

void pc8801_state::palram_w(offs_t offset, uint8_t data)
{
	if(m_misc_ctrl & 0x20) //analog palette
	{
		if((data & 0x40) == 0)
		{
			m_palram[offset].b = data & 0x7;
			m_palram[offset].r = (data & 0x38) >> 3;
		}
		else
		{
			m_palram[offset].g = data & 0x7;
		}
	}
	else //digital palette
	{
		m_palram[offset].b = data & 1 ? 7 : 0;
		m_palram[offset].r = data & 2 ? 7 : 0;
		m_palram[offset].g = data & 4 ? 7 : 0;
	}

	// TODO: What happens to the palette contents when the analog/digital palette mode changes?
	// Preserve content? Translation? Undefined?
	m_palette->set_pen_color(offset, pal3bit(m_palram[offset].r), pal3bit(m_palram[offset].g), pal3bit(m_palram[offset].b));
	// TODO: at least analog mode can do rasters, unconfirmed for digital mode
	// p8suite Analog RGB test cross bars (reportedly works in 24 kHz / 80 column only)
	// NB: it uses a bunch of non-waitstate related opcodes to cycle time it right,
	// implying a stress-test for Z80 opcode cycles.
    m_screen->update_partial(m_screen->vpos());
}


/*
 * ---- x--- green gvram masked flag
 * ---- -x-- red gvram masked flag
 * ---- --x- blue gvram masked flag
 * ---- ---x text vram masked
 */
void pc8801_state::layer_masking_w(uint8_t data)
{
	m_text_layer_mask = bool(BIT(data, 0));
	m_bitmap_layer_mask = ((data & 0xe) >> 1) ^ 7;
}

uint8_t pc8801_state::extram_mode_r()
{
	return (m_extram_mode ^ 0x11) | 0xee;
}

void pc8801_state::extram_mode_w(uint8_t data)
{
	/*
	---x ---- Write EXT RAM access at 0x0000 - 0x7fff
	---- ---x Read EXT RAM access at 0x0000 - 0x7fff
	*/

	m_extram_mode = data & 0x11;
}

uint8_t pc8801_state::extram_bank_r()
{
	return m_extram_bank;
}

void pc8801_state::extram_bank_w(uint8_t data)
{
	// TODO: bits 2 and 3 also accesses bank for PC-8801-17 "VAB" card
	m_extram_bank = data;
}

void pc8801_state::alu_ctrl1_w(uint8_t data)
{
	m_alu_ctrl1 = data;
}

void pc8801_state::alu_ctrl2_w(uint8_t data)
{
	m_alu_ctrl2 = data;
}

/*
 * $e8-$eb kanji LV1
 * $ec-$ef kanji LV2
 *
 */
template <unsigned kanji_level> uint8_t pc8801_state::kanji_r(offs_t offset)
{
	if((offset & 2) == 0)
	{
		const u8 *kanji_rom = kanji_level ? m_kanji_lv2_rom : m_kanji_rom;
		const u32 kanji_address = (m_knj_addr[kanji_level] * 2) + ((offset & 1) ^ 1);
		return kanji_rom[kanji_address];
	}

	return 0xff;
}

template <unsigned kanji_level> void pc8801_state::kanji_w(offs_t offset, uint8_t data)
{
	if((offset & 2) == 0)
	{
		m_knj_addr[kanji_level] = (
			((offset & 1) == 0) ?
			((m_knj_addr[kanji_level] & 0xff00) | (data & 0xff)) :
			((m_knj_addr[kanji_level] & 0x00ff) | (data << 8))
		);
	}
	// TODO: document and implement what the upper two regs does
	// read latches on write? "read start/end sign" according to
	// https://retrocomputerpeople.web.fc2.com/machines/nec/8801/io_map88.html
}


/*
 * PC8801FH overrides (CPU clock switch)
 */

uint8_t pc8801fh_state::cpuclock_r()
{
	return 0x10 | m_clock_setting;
}

uint8_t pc8801fh_state::baudrate_r()
{
	return 0xf0 | m_baudrate_val;
}

void pc8801fh_state::baudrate_w(uint8_t data)
{
	m_baudrate_val = data & 0xf;
}

/*
 * PC8801MA overrides (dictionary)
 */

inline uint8_t pc8801ma_state::dictionary_rom_r(offs_t offset)
{
	return m_dictionary_rom[offset + ((m_dic_bank & 0x1f) * 0x4000)];
}

inline bool pc8801ma_state::dictionary_rom_enable()
{
	return m_dic_ctrl;
}

void pc8801ma_state::dic_bank_w(uint8_t data)
{
	m_dic_bank = data & 0x1f;
}

void pc8801ma_state::dic_ctrl_w(uint8_t data)
{
	m_dic_ctrl = (data ^ 1) & 1;
}

/*
 * PC8801MC overrides (CD-ROM)
 */

inline uint8_t pc8801mc_state::cdbios_rom_r(offs_t offset)
{
	return m_cdrom_bios[offset | ((m_gfx_ctrl & 4) ? 0x8000 : 0x0000)];
}

inline bool pc8801mc_state::cdbios_rom_enable()
{
	return m_cdrom_bank;
}

void pc8801_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x00).portr("KEY0");
	map(0x01, 0x01).portr("KEY1");
	map(0x02, 0x02).portr("KEY2");
	map(0x03, 0x03).portr("KEY3");
	map(0x04, 0x04).portr("KEY4");
	map(0x05, 0x05).portr("KEY5");
	map(0x06, 0x06).portr("KEY6");
	map(0x07, 0x07).portr("KEY7");
	map(0x08, 0x08).portr("KEY8");
	map(0x09, 0x09).portr("KEY9");
	map(0x0a, 0x0a).portr("KEY10");
	map(0x0b, 0x0b).portr("KEY11");
	map(0x0c, 0x0c).portr("KEY12");
	map(0x0d, 0x0d).portr("KEY13");
	map(0x0e, 0x0e).portr("KEY14");
	map(0x0f, 0x0f).portr("KEY15");
	map(0x10, 0x10).w(FUNC(pc8801_state::port10_w));
	map(0x20, 0x21).mirror(0x0e).rw(m_usart, FUNC(i8251_device::read), FUNC(i8251_device::write)); // CMT / RS-232C ch. 0
	map(0x30, 0x30).portr("DSW1").w(FUNC(pc8801_state::port30_w));
	map(0x31, 0x31).portr("DSW2").w(FUNC(pc8801_state::port31_w));
	map(0x32, 0x32).rw(FUNC(pc8801_state::misc_ctrl_r), FUNC(pc8801_state::misc_ctrl_w));
//  map(0x33, 0x33) PC8001mkIISR port, mirror on PC8801?
	// TODO: ALU not installed on pre-mkIISR machines
	// NB: anything after 0x32 reads 0xff on a PC8801MA real HW test
	map(0x34, 0x34).w(FUNC(pc8801_state::alu_ctrl1_w));
	map(0x35, 0x35).w(FUNC(pc8801_state::alu_ctrl2_w));
//  map(0x35, 0x35).r <unknown>, accessed by cancanb during OP, mistake? Mirror for intended HW?
	map(0x40, 0x40).rw(FUNC(pc8801_state::port40_r), FUNC(pc8801_state::port40_w));
//  map(0x44, 0x47).rw internal OPN/OPNA sound card for 8801mkIISR and beyond
//  uPD3301
	map(0x50, 0x51).rw(m_crtc, FUNC(upd3301_device::read), FUNC(upd3301_device::write));

	map(0x52, 0x52).w(FUNC(pc8801_state::bgpal_w));
	map(0x53, 0x53).w(FUNC(pc8801_state::layer_masking_w));
	map(0x54, 0x5b).w(FUNC(pc8801_state::palram_w));
	map(0x5c, 0x5c).r(FUNC(pc8801_state::vram_select_r));
	map(0x5c, 0x5f).w(FUNC(pc8801_state::vram_select_w));
//  i8257
	map(0x60, 0x68).rw(m_dma, FUNC(i8257_device::read), FUNC(i8257_device::write));

//  map(0x6e, 0x6f) clock settings (8801FH and later)
	map(0x70, 0x70).rw(FUNC(pc8801_state::window_bank_r), FUNC(pc8801_state::window_bank_w));
	map(0x71, 0x71).rw(FUNC(pc8801_state::ext_rom_bank_r), FUNC(pc8801_state::ext_rom_bank_w));
	map(0x78, 0x78).w(FUNC(pc8801_state::window_bank_inc_w));
//  map(0x82, 0x82).w access window for PC8801-16
//  map(0x8e, 0x8e).r <unknown>, accessed by scruiser on boot (a board ID?)
//  map(0x90, 0x9f) PC-8801-31 CD-ROM i/f (8801MC)
//  map(0xa0, 0xa3) GSX-8800 or network board
//  map(0xa8, 0xad).rw expansion OPN (Sound Board) or OPNA (Sound Board II)
//  map(0xb0, 0xb3) General Purpose I/O
//  map(0xb4, 0xb4) PC-8801-17 Video art board
//  map(0xb5, 0xb5) PC-8801-18 Video digitizing unit
//  map(0xbc, 0xbf) External mini floppy disk I/F (i8255), PC-8801-13 / -20 / -22
//  map(0xc0, 0xc3) USART RS-232C ch. 1 / ch. 2
//  map(0xc4, 0xc7) PC-8801-10 Music interface board (MIDI), GSX-8800 PIT?
//  map(0xc8, 0xc8) RS-232C ch. 1 "prohibited gate" (?)
//  map(0xca, 0xca) RS-232C ch. 2 "prohibited gate" (?)
//  map(0xc8, 0xcd) JMB-X1 OPM / SSG chips
//  map(0xd0, 0xdf) GP-IB
//  map(0xd3, 0xd4) PC-8801-10 Music interface board (MIDI)
//  map(0xdc, 0xdf) PC-8801-12 MODEM (built-in for mkIITR)
	// $e2-$e3 are standard for mkIIMR, MH / MA / MA2 / MC
	// also used by expansion boards -02 / -02N, -22,
	// and -17 video art board (transfers from RAM?)
	map(0xe2, 0xe2).rw(FUNC(pc8801_state::extram_mode_r), FUNC(pc8801_state::extram_mode_w));
	map(0xe3, 0xe3).rw(FUNC(pc8801_state::extram_bank_r), FUNC(pc8801_state::extram_bank_w));
	map(0xe4, 0xe4).w(FUNC(pc8801_state::irq_level_w));
	map(0xe6, 0xe6).w(FUNC(pc8801_state::irq_mask_w));
//  map(0xe7, 0xe7).noprw(); /* arcus writes here, mirror of above? */
	map(0xe8, 0xeb).rw(FUNC(pc8801_state::kanji_r<0>), FUNC(pc8801_state::kanji_w<0>));
	map(0xec, 0xef).rw(FUNC(pc8801_state::kanji_r<1>), FUNC(pc8801_state::kanji_w<1>));
//  map(0xf0, 0xf1) dictionary bank (8801MA and later)
//  map(0xf3, 0xf3) DMA floppy (direct access like PC88VA?)
//  map(0xf4, 0xf7) DMA 5'25-inch floppy (?)
//  map(0xf8, 0xfb) DMA 8-inch floppy (?)
	map(0xfc, 0xff).m(m_pc80s31, FUNC(pc80s31_device::host_map));
}

void pc8801mk2sr_state::main_io(address_map &map)
{
	pc8801_state::main_io(map);
	map(0x44, 0x45).rw(m_opn, FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}

void pc8801fh_state::main_io(address_map &map)
{
	pc8801_state::main_io(map);
	map(0x44, 0x47).rw(m_opna, FUNC(ym2608_device::read), FUNC(ym2608_device::write));

	map(0x6e, 0x6e).r(FUNC(pc8801fh_state::cpuclock_r));
	map(0x6f, 0x6f).rw(FUNC(pc8801fh_state::baudrate_r), FUNC(pc8801fh_state::baudrate_w));
}

void pc8801ma_state::main_io(address_map &map)
{
	pc8801fh_state::main_io(map);
	map(0xf0, 0xf0).w(FUNC(pc8801ma_state::dic_bank_w));
	map(0xf1, 0xf1).w(FUNC(pc8801ma_state::dic_ctrl_w));
}

void pc8801mc_state::main_io(address_map &map)
{
	pc8801ma_state::main_io(map);
	map(0x90, 0x9f).m(m_cdrom_if, FUNC(pc8801_31_device::amap));
}

void pc8801fh_state::opna_map(address_map &map)
{
	// TODO: confirm it really is ROMless
	// TODO: confirm size
	map(0x000000, 0x1fffff).ram();
}

/* Input Ports */

// TODO: move to a pc8801_keyboard_device, merge with pc8001.cpp implementation
/* 2008-05 FP:
Small note about the strange default mapping of function keys:
the top line of keys in PC8801 keyboard is as follows
[STOP][COPY]      [F1][F2][F3][F4][F5]      [ROLL UP][ROLL DOWN]
Therefore, in Full Emulation mode, "F1" goes to 'F3' and so on

Also, the Keypad has 16 keys, making impossible to map it in a satisfactory
way to a PC keypad. Therefore, default settings for these keys in Full
Emulation are currently based on the effect of the key rather than on
their real position

About natural keyboards: currently,
- "Stop" is mapped to 'Pause'
- "Copy" is mapped to 'Print Screen'
- "Kana" is mapped to 'F6'
- "Grph" is mapped to 'F7'
- "Roll Up" and "Roll Down" are mapped to 'Page Up' and 'Page Down'
- "Help" is mapped to 'F8'
 */

static INPUT_PORTS_START( pc8801 )
	PORT_START("KEY0")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)       PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)       PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)       PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)       PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)       PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)       PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)       PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)       PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("KEY1")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)       PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)       PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)    PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)        PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)        PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)     PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)

	PORT_START("KEY2")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('@')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEY3")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY4")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("KEY5")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)  PORT_CHAR(0xA5) PORT_CHAR('|')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('^')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("KEY6")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY7")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("  _") PORT_CODE(KEYCODE_DEL)            PORT_CHAR(0) PORT_CHAR('_')

	PORT_START("KEY8")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clr Home") PORT_CODE(KEYCODE_HOME)      PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)   PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Grph") PORT_CODE(KEYCODE_LALT)  PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Kana") PORT_CODE(KEYCODE_LCONTROL) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)                        PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("KEY9")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_F1)            PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)                              PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)                              PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)                              PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)                              PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)                              PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                           PORT_CHAR(' ')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)                             PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("KEY10")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)                             PORT_CHAR('\t')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN)   PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help") PORT_CODE(KEYCODE_END)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Copy") PORT_CODE(KEYCODE_F2)            PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)                       PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)                       PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("KEY11")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Roll Up") PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Roll Down") PORT_CODE(KEYCODE_F9)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY12")     /* port 0x0c */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY13")     /* port 0x0d */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY14")     /* port 0x0e */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY15")     /* port 0x0f */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "BASIC" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, "N88-BASIC" )
	PORT_DIPSETTING(    0x00, "N-BASIC" )
	PORT_DIPNAME( 0x02, 0x02, "Terminal mode" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Text width" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x04, "40 chars/line" )
	PORT_DIPSETTING(    0x00, "80 chars/line" )
	PORT_DIPNAME( 0x08, 0x00, "Text height" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x08, "20 lines/screen" )
	PORT_DIPSETTING(    0x00, "25 lines/screen" )
	PORT_DIPNAME( 0x10, 0x10, "Enable S parameter" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Enable DEL code" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// TODO: these really maps to "general purpose inputs" UIP1 / UIP2
	PORT_DIPNAME( 0x40, 0x40, "Memory wait" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Disable CMD SING" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Parity generate" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Parity type" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPSETTING(    0x02, "Odd" )
	PORT_DIPNAME( 0x04, 0x00, "Serial character length" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "7 bits/char" )
	PORT_DIPSETTING(    0x00, "8 bits/char" )
	PORT_DIPNAME( 0x08, 0x08, "Stop bit length" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x10, "Enable X parameter" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Duplex" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Half" )
	PORT_DIPSETTING(    0x00, "Full" )
	// TODO: vanilla PC8801 and mkII doesn't have V2
	PORT_DIPNAME( 0x40, 0x40, "BASIC speed select" ) PORT_DIPLOCATION("SW3:1") // actually SW3:0!
	PORT_DIPSETTING(    0x40, "High Speed Mode (V1H, V2)" )
	PORT_DIPSETTING(    0x00, "Standard Mode (V1S)" )
	PORT_DIPNAME( 0x80, 0x00, "BASIC Version select" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x80, "V1 Mode" )
	PORT_DIPSETTING(    0x00, "V2 Mode" )

	PORT_START("CTRL")
	PORT_DIPNAME( 0x02, 0x02, "Monitor Type" )
	PORT_DIPSETTING(    0x02, "15 KHz" )
	PORT_DIPSETTING(    0x00, "24 KHz" )
//  PORT_BIT 0x04 USART DCD signal carrier
	PORT_DIPNAME( 0x08, 0x00, "Auto-boot floppy at start-up" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("upd1990a", upd1990a_device, data_out_r)
//  PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// TODO: Coming from the old legacy driver as "EXSWITCH", where this maps?
	PORT_START("CFG")
	#if 0
	PORT_DIPNAME( 0x0f, 0x08, "Serial speed" )
	PORT_DIPSETTING(    0x01, "75bps" )
	PORT_DIPSETTING(    0x02, "150bps" )
	PORT_DIPSETTING(    0x03, "300bps" )
	PORT_DIPSETTING(    0x04, "600bps" )
	PORT_DIPSETTING(    0x05, "1200bps" )
	PORT_DIPSETTING(    0x06, "2400bps" )
	PORT_DIPSETTING(    0x07, "4800bps" )
	PORT_DIPSETTING(    0x08, "9600bps" )
	PORT_DIPSETTING(    0x09, "19200bps" )
	#endif
	// TODO: unemulated waitstate weight
	PORT_DIPNAME( 0x40, 0x40, "Speed mode" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x40, DEF_STR( High ) )

	PORT_START("MEM")
	PORT_CONFNAME( 0x0f, 0x0a, "Extension memory" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "32KB (PC-8012-02 x 1)" )
	PORT_CONFSETTING(    0x02, "64KB (PC-8012-02 x 2)" )
	PORT_CONFSETTING(    0x03, "128KB (PC-8012-02 x 4)" )
	PORT_CONFSETTING(    0x04, "128KB (PC-8801-02N x 1)" )
	PORT_CONFSETTING(    0x05, "256KB (PC-8801-02N x 2)" )
	PORT_CONFSETTING(    0x06, "512KB (PC-8801-02N x 4)" )
	PORT_CONFSETTING(    0x07, "1M (PIO-8234H-1M x 1)" )
	PORT_CONFSETTING(    0x08, "2M (PIO-8234H-2M x 1)" )
	PORT_CONFSETTING(    0x09, "4M (PIO-8234H-2M x 2)" )
	PORT_CONFSETTING(    0x0a, "8M (PIO-8234H-2M x 4)" )
	PORT_CONFSETTING(    0x0b, "1.1M (PIO-8234H-1M x 1 + PC-8801-02N x 1)" )
	PORT_CONFSETTING(    0x0c, "2.1M (PIO-8234H-2M x 1 + PC-8801-02N x 1)" )
	PORT_CONFSETTING(    0x0d, "4.1M (PIO-8234H-2M x 2 + PC-8801-02N x 1)" )

	PORT_START("BOARD_CONFIG")
	// TODO: extend both via slot options
//  PORT_CONFNAME( 0x01, 0x01, "Sound Board" )
//  PORT_CONFSETTING(    0x00, "OPN (YM2203)" )
//  PORT_CONFSETTING(    0x01, "OPNA (YM2608)" )
INPUT_PORTS_END

static INPUT_PORTS_START( pc8801fh )
	PORT_INCLUDE( pc8801 )

	// TODO: KEY12, KEY13 and KEY14 have extended meaning
	// "KEY12" F6 - F10, BS, INS, DEL
	// "KEY13" kanji control (lower 4 bits)
	// "KEY14" Normal & Numpad RETURN, Left Shift, Right Shift.
	//         bit 7 acts as extension identifier (0 for FH+ keyboards).

	PORT_MODIFY("CFG")
	PORT_DIPNAME( 0x80, 0x80, "Main CPU clock" )
	PORT_DIPSETTING(    0x80, "4MHz" )
	PORT_DIPSETTING(    0x00, "8MHz" )
INPUT_PORTS_END

/* Graphics Layouts */

static const gfx_layout char_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout kanji_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

// debugging only
static GFXDECODE_START( gfx_pc8801 )
	GFXDECODE_ENTRY( "cgrom",     0, char_layout,  0, 1 )
	GFXDECODE_ENTRY( "kanji",     0, kanji_layout, 0, 1 )
	GFXDECODE_ENTRY( "kanji_lv2", 0, kanji_layout, 0, 1 )
GFXDECODE_END

void pc8801_state::machine_start()
{
	pc8001_base_state::machine_start();

	// TODO: ready signal connected to FDRDY, presumably for the floppy ch.0 and 1
	m_dma->ready_w(1);

	m_work_ram = make_unique_clear<uint8_t[]>(0x10000);
	m_hi_work_ram = make_unique_clear<uint8_t[]>(0x1000);
	m_ext_work_ram = make_unique_clear<uint8_t[]>(0x8000*0x100);
	m_gvram = make_unique_clear<uint8_t[]>(0xc000);

	save_pointer(NAME(m_work_ram), 0x10000);
	save_pointer(NAME(m_hi_work_ram), 0x1000);
	save_pointer(NAME(m_ext_work_ram), 0x8000*0x100);
	save_pointer(NAME(m_gvram), 0xc000);
	save_item(NAME(m_gfx_ctrl));
	save_item(NAME(m_ext_rom_bank));
	save_item(NAME(m_vram_sel));
	save_item(NAME(m_misc_ctrl));
	save_item(NAME(m_device_ctrl_data));
	save_item(NAME(m_window_offset_bank));
	save_item(NAME(m_text_layer_mask));
	save_item(NAME(m_bitmap_layer_mask));
	save_pointer(NAME(m_alu_reg), 3);
	save_item(NAME(m_alu_ctrl1));
	save_item(NAME(m_alu_ctrl2));
	save_item(NAME(m_extram_mode));
	save_item(NAME(m_extram_bank));
	save_item(NAME(m_extram_size));
	save_pointer(NAME(m_knj_addr), 2);
	save_item(STRUCT_MEMBER(m_palram, r));
	save_item(STRUCT_MEMBER(m_palram, g));
	save_item(STRUCT_MEMBER(m_palram, b));
	save_item(STRUCT_MEMBER(m_irq_state, enable));
	save_item(STRUCT_MEMBER(m_irq_state, pending));
	save_item(NAME(m_sound_irq_enable));
	save_item(NAME(m_sound_irq_pending));
}

void pc8801_state::machine_reset()
{
	#define kB 1024
	#define MB 1024*1024
	const uint32_t extram_type[] = { 0*kB, 32*kB,64*kB,128*kB,128*kB,256*kB,512*kB,1*MB,2*MB,4*MB,8*MB,1*MB+128*kB,2*MB+128*kB,4*MB+128*kB, 0*kB, 0*kB };
	#undef kB
	#undef MB

	m_ext_rom_bank = 0xff;
	m_gfx_ctrl = 0x31;
	m_window_offset_bank = 0x80;
	m_misc_ctrl = 0x80;
	// N-BASIC never pings $53, definitely expects text layer to be enabled on default
	m_text_layer_mask = false;
	m_bitmap_layer_mask = 0;
	m_vram_sel = 3;

	// initialize ALU
	for(int i = 0; i < 3; i++)
		m_alu_reg[i] = 0x00;

	m_beeper->set_state(0);

	// initialize irq section
	{
		m_pic->etlg_w(1);
		m_pic->inte_w(1);
		m_irq_state.pending = 0;
		m_irq_state.enable = 0;
		m_sound_irq_enable = false;
		m_sound_irq_pending = false;
	}

	{
		m_extram_bank = 0;
		m_extram_mode = 0;
	}

	palette_reset();

	m_extram_size = extram_type[ioport("MEM")->read() & 0x0f];
//  m_has_opna = ioport("BOARD_CONFIG")->read() & 1;

	const bool pixel_clock_setting = bool(!BIT(ioport("CTRL")->read(), 1));
	set_screen_frequency(pixel_clock_setting);
	m_crtc->set_unscaled_clock(pixel_clock_setting ? PIXEL_CLOCK_24KHz : PIXEL_CLOCK_15KHz);
}

void pc8801fh_state::machine_start()
{
	pc8801mk2sr_state::machine_start();

	save_item(NAME(m_clock_setting));
	save_item(NAME(m_baudrate_val));
}

void pc8801fh_state::machine_reset()
{
	pc8801_state::machine_reset();

	m_clock_setting = ioport("CFG")->read() & 0x80;

	m_maincpu->set_unscaled_clock(m_clock_setting ? (PC8801FH_OSC3 / 8) : (PC8801FH_OSC3 / 4));
	// TODO: FDC board shouldn't be connected to the clock setting, verify
//  m_fdccpu->set_unscaled_clock(m_clock_setting ?  XTAL(4'000'000) : XTAL(8'000'000));
	m_baudrate_val = 0;
}

void pc8801ma_state::machine_start()
{
	pc8801fh_state::machine_start();

	save_item(NAME(m_dic_bank));
	save_item(NAME(m_dic_ctrl));
}

void pc8801ma_state::machine_reset()
{
	pc8801fh_state::machine_reset();

	m_dic_bank = 0;
	m_dic_ctrl = 0;
}

void pc8801mc_state::machine_start()
{
	pc8801ma_state::machine_start();

	save_item(NAME(m_cdrom_bank));
}

void pc8801mc_state::machine_reset()
{
	pc8801ma_state::machine_reset();

	// Hold STOP during boot to bypass CDROM BIOS at POST (PC=0x10)
	m_cdrom_bank = true;
}

// DE-9 mouse port on front panel (labelled "マウス") - MSX-compatible
uint8_t pc8801mk2sr_state::opn_porta_r()
{
	return BIT(m_mouse_port->read(), 0, 4) | 0xf0;
}

uint8_t pc8801mk2sr_state::opn_portb_r()
{
	return BIT(m_mouse_port->read(), 4, 2) | 0xfc;
}

void pc8801mk2sr_state::opn_portb_w(uint8_t data)
{
	m_mouse_port->pin_6_w(BIT(data, 0));
	m_mouse_port->pin_7_w(BIT(data, 1));
}

// Cassette Configuration
void pc8801_state::txdata_callback(int state)
{
	//m_cassette->output( (state) ? 1.0 : -1.0);
}

void pc8801_state::rxrdy_irq_w(int state)
{
	if (state)
		assert_irq(RXRDY_IRQ_LEVEL);
}

/*
 * 0 RXRDY
 * 1 VRTC
 * 2 CLOCK
 * 3 INT3 (GSX-8800)
 * 4 INT4 (any OPN, external boards included with different irq mask at $aa)
 * 5 INT5
 * 6 FDCINT1
 * 7 FDCINT2
 *
 */
IRQ_CALLBACK_MEMBER(pc8801_state::int_ack_cb)
{
	// TODO: schematics sports a μPB8212 too, with DI2-DI4 connected to 8214 A0-A2
	// Seems just an intermediate bridge for translating raw levels to vectors
	// with no access from outside world?
	u8 level = m_pic->a_r();
	m_pic->r_w(level, 1);

	return (7 - level) * 2;
}

void pc8801_state::int4_irq_w(int state)
{
	bool irq_state = m_sound_irq_enable & state;

	// remember current setting so that an enable reg variation will pick up
	// particularly needed by Telenet games (xzr2, valis2)
	// TODO: understand how exactly the external irq source works out (Sound Board II)
	// has a separate irq mask for secondary OPNA but still sends INT4s,
	// we separate the logic from the others since this exact function needs templatized array for enable and pending anyway
	// (and won't otherwise work for xzr2 anyway).
	m_pic->r_w(7 ^ INT4_IRQ_LEVEL, !irq_state);
	m_sound_irq_pending = state;
}

// FIXME: convert to pure write-line-style member
// Works with 0 -> 1 F/F transitions
TIMER_DEVICE_CALLBACK_MEMBER(pc8801_state::clock_irq_w)
{
	// TODO: castlex sound notes in BGM loop are pretty erratic
	// (uses clock irq instead of the dedicated INT4, started happening on last OPN rewrite, is it just missing some interpolation in the sound core?
	assert_irq(CLOCK_IRQ_LEVEL);
}

void pc8801_state::check_irq(u8 level)
{
	u8 mask = 1 << level;

	// megamit and babylon are particularly fussy if the VRTC irq isn't disabled when requested
	// - megamit jumps to PC=0
	// - babylon has just a ret coded in the VRTC irq, so accepting that will wreck the program flow and hang at title screen with no sound (because it expects INT4s)
	if (!(m_irq_state.enable & mask))
		m_pic->r_w(7 ^ level, 1);
	else if (m_irq_state.enable & m_irq_state.pending & mask)
		assert_irq(level);
}

void pc8801_state::assert_irq(u8 level)
{
	u8 mask = 1 << level;

	if (mask & m_irq_state.enable)
	{
		m_irq_state.pending &= ~mask;
		m_pic->r_w(7 ^ level, 0);
	}
	else
		m_irq_state.pending |= mask;
}

void pc8801_state::vrtc_irq_w(int state)
{
//  bool irq_state = m_vrtc_irq_enable & state;
	if (state)
	{
		assert_irq(VRTC_IRQ_LEVEL);
	}
}

void pc8801_state::irq_w(int state)
{
	m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}


void pc8801_state::pc8801(machine_config &config)
{
	Z80(config, m_maincpu, MASTER_CLOCK); // ~4 MHz, selectable to ~8 MHz on late models
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8801_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &pc8801_state::main_io);
	m_maincpu->set_irq_acknowledge_callback(FUNC(pc8801_state::int_ack_cb));

	PC80S31(config, m_pc80s31, MASTER_CLOCK);
	config.set_perfect_quantum(m_maincpu);
	config.set_perfect_quantum("pc80s31:fdc_cpu");

//  config.set_maximum_quantum(attotime::from_hz(MASTER_CLOCK/1024));

	I8214(config, m_pic, MASTER_CLOCK);
	m_pic->int_wr_callback().set(FUNC(pc8801_state::irq_w));
	m_pic->set_int_dis_hack(true);

	UPD1990A(config, m_rtc);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(pc8801_state::write_centronics_ack));
	m_centronics->busy_handler().set(FUNC(pc8801_state::write_centronics_busy));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	// TODO: needs T88 format support
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->set_interface("pc8801_cass");

	SOFTWARE_LIST(config, "tape_list").set_original("pc8801_cass");

	// TODO: clock, receiver handler, DCD?
	I8251(config, m_usart, 0);
	m_usart->txd_handler().set(FUNC(pc8801_state::txdata_callback));
	m_usart->rxrdy_handler().set(FUNC(pc8801_state::rxrdy_irq_w));

	SOFTWARE_LIST(config, "disk_n88_list").set_original("pc8801_flop");
	SOFTWARE_LIST(config, "disk_n_list").set_compatible("pc8001_flop");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
//  m_screen->set_raw(PIXEL_CLOCK_24KHz,848,0,640,448,0,400);
	m_screen->set_raw(PIXEL_CLOCK_15KHz, 896, 0, 640, 260, 0, 200);
	m_screen->set_screen_update(FUNC(pc8801_state::screen_update));
//  m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_pc8801);
	PALETTE(config, m_palette, palette_device::BLACK, 0x8 + 2); // +2 for BG Pal and border colors
	PALETTE(config, m_crtc_palette, palette_device::BRG_3BIT);

	UPD3301(config, m_crtc, PIXEL_CLOCK_15KHz);
	m_crtc->set_character_width(8);
	m_crtc->set_display_callback(FUNC(pc8801_state::draw_text));
	m_crtc->set_attribute_fetch_callback(FUNC(pc8801_state::attr_fetch));
	m_crtc->drq_wr_callback().set(m_dma, FUNC(i8257_device::dreq2_w));
	m_crtc->rvv_wr_callback().set(FUNC(pc8801_state::crtc_reverse_w));
//  Note: 3301 isn't actually connected to INT so its internal irq mask doesn't have any effect in PC88
	m_crtc->vrtc_wr_callback().set(FUNC(pc8801_state::vrtc_irq_w));
	m_crtc->set_screen(m_screen);

	I8257(config, m_dma, MASTER_CLOCK);
	m_dma->out_hrq_cb().set(FUNC(pc8801_state::hrq_w));
	m_dma->in_memr_cb().set(FUNC(pc8801_state::dma_mem_r));
	// CH0: 5-inch floppy DMA
	// CH1: 8-inch floppy DMA, SCSI CD-ROM interface (on MA/MC)
	m_dma->out_iow_cb<2>().set(m_crtc, FUNC(upd3301_device::dack_w));
	// CH3: <autoload only?>

	TIMER(config, "rtc_timer").configure_periodic(FUNC(pc8801_state::clock_irq_w), attotime::from_hz(600));

	// Note: original models up to OPNA variants really have an internal mono speaker,
	// but user eventually can have a stereo mixing audio card mounted so for simplicity we MCM here.
	SPEAKER(config, m_lspeaker).front_left();
	SPEAKER(config, m_rspeaker).front_right();

	// TODO: DAC_1BIT
	// 2400 Hz according to schematics, unaffected by clock speed setting (confirmed on real HW)
	BEEP(config, m_beeper, MASTER_CLOCK / 16 / 13 / 8);

	for (auto &speaker : { m_lspeaker, m_rspeaker })
	{
		m_cassette->add_route(ALL_OUTPUTS, speaker, 0.025);
		m_beeper->add_route(ALL_OUTPUTS, speaker, 0.10);
	}

	MSX_GENERAL_PURPOSE_PORT(config, m_mouse_port, msx_general_purpose_port_devices, "joystick");

	PC8801_EXP_SLOT(config, m_exp, pc8801_exp_devices, nullptr);
	m_exp->set_iospace(m_maincpu, AS_IO);
	m_exp->int3_callback().set([this] (bool state) { m_pic->r_w(7 ^ INT3_IRQ_LEVEL, !state); });
	m_exp->int4_callback().set([this] (bool state) { m_pic->r_w(7 ^ INT4_IRQ_LEVEL, !state); });
	m_exp->int5_callback().set([this] (bool state) { m_pic->r_w(7 ^ INT5_IRQ_LEVEL, !state); });
}

void pc8801mk2sr_state::pc8801mk2sr(machine_config &config)
{
	pc8801(config);

	YM2203(config, m_opn, MASTER_CLOCK);
	m_opn->irq_handler().set(FUNC(pc8801mk2sr_state::int4_irq_w));
	m_opn->port_a_read_callback().set(FUNC(pc8801mk2sr_state::opn_porta_r));
	m_opn->port_b_read_callback().set(FUNC(pc8801mk2sr_state::opn_portb_r));
	m_opn->port_b_write_callback().set(FUNC(pc8801mk2sr_state::opn_portb_w));

	for (auto &speaker : { m_lspeaker, m_rspeaker })
	{
		// TODO: per-channel mixing is unconfirmed
		m_opn->add_route(0, speaker, 0.125);
		m_opn->add_route(1, speaker, 0.125);
		m_opn->add_route(2, speaker, 0.125);
		m_opn->add_route(3, speaker, 0.125);
	}
}

void pc8801mk2sr_state::pc8801mk2mr(machine_config &config)
{
	pc8801mk2sr(config);
	PC80S31K(config.replace(), m_pc80s31, MASTER_CLOCK);
}

void pc8801fh_state::pc8801fh(machine_config &config)
{
	pc8801mk2mr(config);

	config.device_remove("opn");

	YM2608(config, m_opna, MASTER_CLOCK*2);
	m_opna->set_addrmap(0, &pc8801fh_state::opna_map);
	m_opna->irq_handler().set(FUNC(pc8801fh_state::int4_irq_w));
	m_opna->port_a_read_callback().set(FUNC(pc8801fh_state::opn_porta_r));
	m_opna->port_b_read_callback().set(FUNC(pc8801fh_state::opn_portb_r));
	m_opna->port_b_write_callback().set(FUNC(pc8801fh_state::opn_portb_w));

	// TODO: per-channel mixing is unconfirmed
	m_opna->add_route(0, m_lspeaker, 0.25);
	m_opna->add_route(0, m_rspeaker, 0.25);
	m_opna->add_route(1, m_lspeaker, 0.75);
	m_opna->add_route(2, m_rspeaker, 0.75);

	// TODO: add possible configuration override for baudrate here
	// ...
}

void pc8801ma_state::pc8801ma(machine_config &config)
{
	pc8801fh(config);
	// TODO: option slot for CD-ROM bus
	// ...
}

void pc8801mc_state::pc8801mc(machine_config &config)
{
	pc8801ma(config);

	PC8801_31(config, m_cdrom_if, 0);
	m_cdrom_if->rom_bank_cb().set([this](bool state) { m_cdrom_bank = state; });
}

ROM_START( pc8801 )
	ROM_REGION( 0x8000,  "n80rom", ROMREGION_ERASEFF ) // 1.2
	ROM_LOAD( "n80.rom",   0x0000, 0x8000, CRC(5cb8b584) SHA1(063609dd518c124a4fc9ba35d1bae35771666a34) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 1.0 V1
	ROM_LOAD( "n88.rom",   0x0000, 0x8000, CRC(ffd68be0) SHA1(3518193b8207bdebf22c1380c2db8c554baff329) )
	ROM_LOAD( "n88_0.rom", 0x8000, 0x2000, CRC(61984bab) SHA1(d1ae642aed4f0584eeb81ff50180db694e5101d4) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_LOAD( "font.rom", 0x0000, 0x0800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )
ROM_END

/*
 * The dump only included "maincpu".
 * Other roms arbitrariely taken from PC-8801 & PC-8801 MkIISR
 * (there should be at least 1 Kanji ROM).
 */
ROM_START( pc8801mk2 )
	ROM_REGION( 0x8000,  "n80rom", ROMREGION_ERASEFF ) // 1.4
	ROM_LOAD( "m2_n80.rom",   0x0000, 0x8000, CRC(91d84b1a) SHA1(d8a1abb0df75936b3fc9d226ccdb664a9070ffb1) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 1.3 V1
	ROM_LOAD( "m2_n88.rom",   0x0000, 0x8000, CRC(f35169eb) SHA1(ef1f067f819781d9fb2713836d195866f0f81501) )
	ROM_LOAD( "m2_n88_0.rom", 0x8000, 0x2000, CRC(5eb7a8d0) SHA1(95a70af83b0637a5a0f05e31fb0452bb2cb68055) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD_OPTIONAL( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x800 )
ROM_END

ROM_START( pc8801mk2sr )
	ROM_REGION( 0x8000,  "n80rom", ROMREGION_ERASEFF ) // 1.5
	ROM_LOAD( "mk2sr_n80.rom",   0x0000, 0x8000, CRC(27e1857d) SHA1(5b922ed9de07d2a729bdf1da7b57c50ddf08809a) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.0 V2 / 1.4 V1
	ROM_LOAD( "mk2sr_n88.rom",   0x0000, 0x8000, CRC(a0fc0473) SHA1(3b31fc68fa7f47b21c1a1cb027b86b9e87afbfff) )
	ROM_LOAD( "mk2sr_n88_0.rom", 0x8000, 0x2000, CRC(710a63ec) SHA1(d239c26ad7ac5efac6e947b0e9549b1534aa970d) )
	ROM_LOAD( "n88_1.rom",       0xa000, 0x2000, CRC(c0bd2aa6) SHA1(8528eef7946edf6501a6ccb1f416b60c64efac7c) )
	ROM_LOAD( "n88_2.rom",       0xc000, 0x2000, CRC(af2b6efa) SHA1(b7c8bcea219b77d9cc3ee0efafe343cc307425d1) )
	ROM_LOAD( "n88_3.rom",       0xe000, 0x2000, CRC(7713c519) SHA1(efce0b51cab9f0da6cf68507757f1245a2867a72) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )
	// not on stock mkIISR
	ROM_LOAD_OPTIONAL( "kanji2.rom", 0x00000, 0x20000, CRC(154803cc) SHA1(7e6591cd465cbb35d6d3446c5a83b46d30fafe95) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x800 )
ROM_END

ROM_START( pc8801mk2fr )
	ROM_REGION( 0x8000,  "n80rom", ROMREGION_ERASEFF ) // 1.5
	ROM_LOAD( "m2fr_n80.rom",   0x0000, 0x8000, CRC(27e1857d) SHA1(5b922ed9de07d2a729bdf1da7b57c50ddf08809a) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.1 V2 / 1.5 V1
	ROM_LOAD( "m2fr_n88.rom",   0x0000, 0x8000, CRC(b9daf1aa) SHA1(696a480232bcf8c827c7aeea8329db5c44420d2a) )
	ROM_LOAD( "m2fr_n88_0.rom", 0x8000, 0x2000, CRC(710a63ec) SHA1(d239c26ad7ac5efac6e947b0e9549b1534aa970d) )
	ROM_LOAD( "m2fr_n88_1.rom", 0xa000, 0x2000, CRC(e3e78a37) SHA1(85ecd287fe72b56e54c8b01ea7492ca4a69a7470) )
	ROM_LOAD( "m2fr_n88_2.rom", 0xc000, 0x2000, CRC(98c3a7b2) SHA1(fc4980762d3caa56964d0ae583424756f511d186) )
	ROM_LOAD( "m2fr_n88_3.rom", 0xe000, 0x2000, CRC(0ca08abd) SHA1(a5a42d0b7caa84c3bc6e337c9f37874d82f9c14b) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x800 )
ROM_END

ROM_START( pc8801mk2mr )
	ROM_REGION( 0x8000,  "n80rom", ROMREGION_ERASEFF ) // 1.8
	ROM_LOAD( "m2mr_n80.rom",   0x0000, 0x8000, CRC(f074b515) SHA1(ebe9cf4cf57f1602c887f609a728267f8d953dce) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.2 V2 / 1.7 V1
	ROM_LOAD( "m2mr_n88.rom",   0x0000, 0x8000, CRC(69caa38e) SHA1(3c64090237152ee77c76e04d6f36bad7297bea93) )
	ROM_LOAD( "m2mr_n88_0.rom", 0x8000, 0x2000, CRC(710a63ec) SHA1(d239c26ad7ac5efac6e947b0e9549b1534aa970d) )
	ROM_LOAD( "m2mr_n88_1.rom", 0xa000, 0x2000, CRC(e3e78a37) SHA1(85ecd287fe72b56e54c8b01ea7492ca4a69a7470) )
	ROM_LOAD( "m2mr_n88_2.rom", 0xc000, 0x2000, CRC(11176e0b) SHA1(f13f14f3d62df61498a23f7eb624e1a646caea45) )
	ROM_LOAD( "m2mr_n88_3.rom", 0xe000, 0x2000, CRC(0ca08abd) SHA1(a5a42d0b7caa84c3bc6e337c9f37874d82f9c14b) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji1.rom",      0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )
	ROM_LOAD( "m2mr_kanji2.rom", 0x00000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x800 )
ROM_END

ROM_START( pc8801mh )
	ROM_REGION( 0x8000,  "n80rom", ROMREGION_ERASEFF ) // 1.8, but different BIOS code?
	ROM_LOAD( "mh_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3 V2 / 1.8 V1
	ROM_LOAD( "mh_n88.rom",   0x0000, 0x8000, CRC(64c5d162) SHA1(3e0aac76fb5d7edc99df26fa9f365fd991742a5d) )
	ROM_LOAD( "mh_n88_0.rom", 0x8000, 0x2000, CRC(deb384fb) SHA1(5f38cafa8aab16338038c82267800446fd082e79) )
	ROM_LOAD( "mh_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "mh_n88_2.rom", 0xc000, 0x2000, CRC(6aa6b6d8) SHA1(2a077ab444a4fd1470cafb06fd3a0f45420c39cc) )
	ROM_LOAD( "mh_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji1.rom",    0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )
	ROM_LOAD( "mh_kanji2.rom", 0x00000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )
ROM_END

ROM_START( pc8801fa )
	ROM_REGION( 0x8000,  "n80rom", ROMREGION_ERASEFF ) // 1.8
	ROM_LOAD( "fa_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3 V2 / 1.9 V1
	ROM_LOAD( "fa_n88.rom",   0x0000, 0x8000, CRC(73573432) SHA1(9b1346d44044eeea921c4cce69b5dc49dbc0b7e9) )
	ROM_LOAD( "fa_n88_0.rom", 0x8000, 0x2000, CRC(a72697d7) SHA1(5aedbc5916d67ef28767a2b942864765eea81bb8) )
	ROM_LOAD( "fa_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "fa_n88_2.rom", 0xc000, 0x2000, CRC(6aee9a4e) SHA1(e94278682ef9e9bbb82201f72c50382748dcea2a) )
	ROM_LOAD( "fa_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji1.rom",    0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )
	ROM_LOAD( "fa_kanji2.rom", 0x00000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )
ROM_END

// newer floppy BIOS and Jisyo (dictionary) ROM, otherwise same as FA
ROM_START( pc8801ma )
	ROM_REGION( 0x8000,  "n80rom", ROMREGION_ERASEFF ) // 1.8
	ROM_LOAD( "ma_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3 V2 / 1.9 V1
	ROM_LOAD( "ma_n88.rom",   0x0000, 0x8000, CRC(73573432) SHA1(9b1346d44044eeea921c4cce69b5dc49dbc0b7e9) )
	ROM_LOAD( "ma_n88_0.rom", 0x8000, 0x2000, CRC(a72697d7) SHA1(5aedbc5916d67ef28767a2b942864765eea81bb8) )
	ROM_LOAD( "ma_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "ma_n88_2.rom", 0xc000, 0x2000, CRC(6aee9a4e) SHA1(e94278682ef9e9bbb82201f72c50382748dcea2a) )
	ROM_LOAD( "ma_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji1.rom",    0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )
	ROM_LOAD( "ma_kanji2.rom", 0x00000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )

	ROM_REGION( 0x80000, "dictionary", 0 )
	ROM_LOAD( "ma_jisyo.rom", 0x00000, 0x80000, CRC(a6108f4d) SHA1(3665db538598abb45d9dfe636423e6728a812b12) )
ROM_END

ROM_START( pc8801ma2 )
	ROM_REGION( 0x8000,  "n80rom", ROMREGION_ERASEFF ) // 1.8
	ROM_LOAD( "ma2_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3 (2.31?) V2 / 1.91 V1
	ROM_LOAD( "ma2_n88.rom",   0x0000, 0x8000, CRC(ae1a6ebc) SHA1(e53d628638f663099234e07837ffb1b0f86d480d) )
	ROM_LOAD( "ma2_n88_0.rom", 0x8000, 0x2000, CRC(a72697d7) SHA1(5aedbc5916d67ef28767a2b942864765eea81bb8) )
	ROM_LOAD( "ma2_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "ma2_n88_2.rom", 0xc000, 0x2000, CRC(1d6277b6) SHA1(dd9c3e50169b75bb707ef648f20d352e6a8bcfe4) )
	ROM_LOAD( "ma2_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji1.rom",     0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )
	ROM_LOAD( "ma2_kanji2.rom", 0x00000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )

	ROM_REGION( 0x80000, "dictionary", 0 )
	ROM_LOAD( "ma2_jisyo.rom", 0x00000, 0x80000, CRC(856459af) SHA1(06241085fc1d62d4b2968ad9cdbdadc1e7d7990a) )
ROM_END

ROM_START( pc8801mc )
	ROM_REGION( 0x08000, "n80rom", ROMREGION_ERASEFF ) // 1.8
	ROM_LOAD( "mc_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3 (2.33?) V2 / 1.93 V1
	ROM_LOAD( "mc_n88.rom",   0x0000, 0x8000, CRC(356d5719) SHA1(5d9ba80d593a5119f52aae1ccd61a1457b4a89a1) )
	ROM_LOAD( "mc_n88_0.rom", 0x8000, 0x2000, CRC(a72697d7) SHA1(5aedbc5916d67ef28767a2b942864765eea81bb8) )
	ROM_LOAD( "mc_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "mc_n88_2.rom", 0xc000, 0x2000, CRC(1d6277b6) SHA1(dd9c3e50169b75bb707ef648f20d352e6a8bcfe4) )
	ROM_LOAD( "mc_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x10000, "cdrom_bios", 0 )
	ROM_LOAD( "cdbios.rom", 0x0000, 0x10000, CRC(5c230221) SHA1(6394a8a23f44ea35fcfc3e974cf940bc8f84d62a) )

	ROM_REGION( 0x20000, "kanji", ROMREGION_ERASEFF )
	ROM_LOAD( "kanji1.rom",    0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x20000, "kanji_lv2", ROMREGION_ERASEFF )
	ROM_LOAD( "mc_kanji2.rom", 0x00000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )

	ROM_REGION( 0x80000, "dictionary", 0 )
	ROM_LOAD( "mc_jisyo.rom", 0x00000, 0x80000, CRC(bd6eb062) SHA1(deef0cc2a9734ba891a6d6c022aa70ffc66f783e) )
ROM_END


COMP( 1981, pc8801,      0,      0,      pc8801,      pc8801, pc8801_state, empty_init,      "NEC",   "PC-8801",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
// PC-8801A (120V, USA & Canada) / PC-8801B (240V, Export?) for Western markets according to a NEC brochure
COMP( 1983, pc8801mk2,   pc8801, 0,      pc8801,      pc8801, pc8801_state, empty_init,      "NEC",   "PC-8801mkII",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )

// internal OPN
COMP( 1985, pc8801mk2sr, 0,           0,      pc8801mk2sr, pc8801, pc8801mk2sr_state, empty_init, "NEC",   "PC-8801mkIISR", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
//COMP( 1985, pc8801mk2tr, pc8801mk2sr, 0,      pc8801mk2sr, pc8801, pc8801mk2sr_state, empty_init, "NEC",   "PC-8801mkIITR", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, pc8801mk2fr, pc8801mk2sr, 0,      pc8801mk2sr, pc8801, pc8801mk2sr_state, empty_init, "NEC",   "PC-8801mkIIFR", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1985, pc8801mk2mr, pc8801mk2sr, 0,      pc8801mk2mr, pc8801, pc8801mk2sr_state, empty_init, "NEC",   "PC-8801mkIIMR", MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )

// internal OPNA
//COMP( 1986, pc8801fh,    pc8801mh, 0,      pc8801fh,    pc8801fh, pc8801fh_state, empty_init, "NEC",   "PC-8801FH",     MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1986, pc8801mh,    0,        0,      pc8801fh,    pc8801fh, pc8801fh_state, empty_init, "NEC",   "PC-8801MH",     MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, pc8801fa,    pc8801mh, 0,      pc8801fh,    pc8801fh, pc8801fh_state, empty_init, "NEC",   "PC-8801FA",     MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1987, pc8801ma,    0,        0,      pc8801ma,    pc8801fh, pc8801ma_state, empty_init, "NEC",   "PC-8801MA",     MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
//COMP( 1988, pc8801fe,    pc8801ma, 0,      pc8801fa,    pc8801fh, pc8801ma_state, empty_init, "NEC",   "PC-8801FE",     MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1988, pc8801ma2,   pc8801ma, 0,      pc8801ma,    pc8801fh, pc8801ma_state, empty_init, "NEC",   "PC-8801MA2",    MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
//COMP( 1989, pc8801fe2,   pc8801ma, 0,      pc8801fa,    pc8801fh, pc8801ma_state, empty_init, "NEC",   "PC-8801FE2",    MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )
COMP( 1989, pc8801mc,    pc8801ma, 0,      pc8801mc,    pc8801fh, pc8801mc_state, empty_init, "NEC",   "PC-8801MC",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_TIMING | MACHINE_IMPERFECT_GRAPHICS )

// PC98DO (PC88+PC98, V33 + μPD70008AC)
// belongs to own driver
//COMP( 1989, pc98do,      0,      0,      pc98do,      pc98do, pc8801_state, empty_init, "NEC",   "PC-98DO",       MACHINE_NOT_WORKING )
//COMP( 1990, pc98dop,     0,      0,      pc98do,      pc98do, pc8801_state, empty_init, "NEC",   "PC-98DO+",      MACHINE_NOT_WORKING )
