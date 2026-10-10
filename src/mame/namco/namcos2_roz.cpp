// license:BSD-3-Clause
// copyright-holders:David Haywood, Phil Stroffolino

/*
    Namco System 2 ROZ Tilemap - found on Namco System 2 video board (standard type)

    C102 - ROZ address generator, also controls CPU access to the ROZ memory.
    80 pin QFP, implemented after furrtek's reverse engineered RTL.

    Registers, write only (A3-A1):
    0   X delta per pixel       12 bits plus sign in bit 15, 0x100 is one pixel,
    1   Y delta per pixel       bits 12-14 are not stored
    2   X delta per line
    3   Y delta per line
    4   X start                 1/16 pixel
    5   Y start
    6   -
    7   xxxx---- --------       X chip select bits
        ----xxxx --------       Y chip select bits
        -------- xxxx----       X mask bits
        -------- ----xxxx       Y mask bits

    The chip is two identical units, one per axis, each with a 20-bit accumulator:
    - while RESTART is set (vertical blanking) it is reloaded from the start register << 4,
      so the first visible line starts there
    - on every other line the per line delta is added once, when horizontal sync ends
    - during the line the per pixel delta is added on every I6M clock
    Bits 19-8 are the position in the plane. Bits 9-3 go out as MAx (ROZ RAM address) and
    bits 2-0 as RAx (pixel inside the 8x8 tile, low bits of the ROM address). Bits 11-8 are
    ANDed with the register 7 nibbles: any chip select bit set pulls nCSx low, any mask bit
    set pulls nMASK low. nCSx is taken here as the upper half of the RAM on that axis, which
    gives the 2048x2048 plane the games expect.
    Games use chip select 4 with mask 0 (wraps at 2048), 8 (2048x2048, no wrap),
    c (1024x1024) or e (512x512).

    The CPU side (DTACK, RAM output enables, read latch) needs no emulation. The chip does not
    drive the bus on register reads; the written values are still returned as before.

    used by the following drivers
    namcos2.cpp (all games EXCEPT Final Lap 1,2,3 , Lucky & Wild , Steel Gunner 1,2 , Suzuka 8 Hours 1,2 , Metal Hawk)


*/

#include "emu.h"
#include "namcos2_roz.h"

#include <algorithm>


namespace {

// pixel clocks from the end of horizontal sync to the first visible pixel
constexpr int XOFFSET = 38;

constexpr int ROZ_BLOCK_SIZE = 8;

constexpr s32 roz_delta(u16 data)
{
	return s32(data & 0x0fff) - (BIT(data, 15) ? 0x1000 : 0);
}

// one C102 unit: MAx/RAx on bits 9-0, nCSx asserted on bit 10, and bit 11 set
// when the unit pulls nMASK low
constexpr u32 roz_unit_out(u32 acc, u32 cs, u32 mask)
{
	const u32 pos = (acc >> 8) & 0xfff;
	const u32 top = pos >> 8;
	return ((top & mask) ? 0x800 : 0) | ((top & cs) ? 0x400 : 0) | (pos & 0x3ff);
}

} // anonymous namespace

GFXDECODE_START( namcos2_roz_device::gfxinfo )
	GFXDECODE_DEVICE( DEVICE_SELF, 0, gfx_8x8x8_raw, 0, 16 )
GFXDECODE_END

DEFINE_DEVICE_TYPE(NAMCOS2_ROZ, namcos2_roz_device, "namcos2_roz", "Namco System 2 ROZ (C102)")

namcos2_roz_device::namcos2_roz_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NAMCOS2_ROZ, tag, owner, clock),
	device_gfx_interface(mconfig, *this, gfxinfo),
	device_video_interface(mconfig, *this),
	m_rozram(*this, finder_base::DUMMY_TAG),
	m_roz_ctrl{ 0, 0, 0, 0, 0, 0, 0, 0 },
	m_accx(0),
	m_accy(0),
	m_line_timer(nullptr)
{
}

void namcos2_roz_device::device_start()
{
	m_tilemap_roz = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(namcos2_roz_device::roz_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 256, 256);
	m_tilemap_roz->set_transparent_pen(0xff);

	const int lines = screen().height();
	m_lines = std::make_unique<line_state []>(lines);

	m_line_timer = timer_alloc(FUNC(namcos2_roz_device::line_start), this);
	m_line_timer->adjust(screen().time_until_pos(0, screen().width() - XOFFSET), 1);

	save_item(NAME(m_roz_ctrl));
	save_item(NAME(m_accx));
	save_item(NAME(m_accy));
	save_pointer(STRUCT_MEMBER(m_lines, startx), lines);
	save_pointer(STRUCT_MEMBER(m_lines, starty), lines);
	save_pointer(STRUCT_MEMBER(m_lines, incxx), lines);
	save_pointer(STRUCT_MEMBER(m_lines, incxy), lines);
	save_pointer(STRUCT_MEMBER(m_lines, ctrl), lines);
}


TILE_GET_INFO_MEMBER(namcos2_roz_device::roz_tile_info)
{
	tileinfo.set(0, m_rozram[tile_index], 0/*color*/, 0);
}


TIMER_CALLBACK_MEMBER(namcos2_roz_device::line_start)
{
	const rectangle visarea = screen().visible_area();
	const int line = param;
	const int prev = line ? (line - 1) : (screen().height() - 1);

	if ((prev < visarea.min_y) || (prev > visarea.max_y))
	{
		// RESTART
		m_accx = u32(m_roz_ctrl[4]) << 4;
		m_accy = u32(m_roz_ctrl[5]) << 4;
	}
	else
	{
		m_accx = (m_accx + roz_delta(m_roz_ctrl[2])) & 0xfffff;
		m_accy = (m_accy + roz_delta(m_roz_ctrl[3])) & 0xfffff;
	}

	// snapshot for draw_roz; later writes to the per pixel deltas and to
	// register 7 are only seen from the next line
	line_state &ls = m_lines[line];
	ls.startx = m_accx;
	ls.starty = m_accy;
	ls.incxx = roz_delta(m_roz_ctrl[0]);
	ls.incxy = roz_delta(m_roz_ctrl[1]);
	ls.ctrl = m_roz_ctrl[7];

	// the next line starts where the horizontal sync of this one ends
	m_line_timer->adjust(screen().time_until_pos(line, screen().width() - XOFFSET), (line + 1) % screen().height());
}


void namcos2_roz_device::draw_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t gfx_ctrl, uint8_t prival, uint8_t primask)
{
	const u16 color = gfx_ctrl & 0x0f00;

	bitmap_ind8 &destprimap = screen.priority();
	bitmap_ind16 &srcbitmap = m_tilemap_roz->pixmap();
	bitmap_ind8 &flagsbitmap = m_tilemap_roz->flagsmap();

	/* The screen is walked in small blocks rather than row by row. Close to
	    90 or 270 degrees of rotation a row of the screen goes down a column of
	    the plane, and a straight row by row walk would touch a different cache
	    line for nearly every pixel. The source pixels of a small screen block
	    stay close together at any angle. Each line still uses its own C102 state.
	*/
	for (int y0 = cliprect.min_y; y0 <= cliprect.max_y; y0 += ROZ_BLOCK_SIZE)
	{
		const int y1 = std::min(y0 + ROZ_BLOCK_SIZE - 1, cliprect.max_y);

		for (int x0 = cliprect.min_x; x0 <= cliprect.max_x; x0 += ROZ_BLOCK_SIZE)
		{
			const int x1 = std::min(x0 + ROZ_BLOCK_SIZE - 1, cliprect.max_x);

			for (int y = y0; y <= y1; y++)
			{
				const line_state &ls = m_lines[y];
				const u32 csx = BIT(ls.ctrl, 12, 4);
				const u32 csy = BIT(ls.ctrl, 8, 4);
				const u32 maskx = BIT(ls.ctrl, 4, 4);
				const u32 masky = BIT(ls.ctrl, 0, 4);

				u32 accx = ls.startx + ((x0 + XOFFSET) * ls.incxx);
				u32 accy = ls.starty + ((x0 + XOFFSET) * ls.incxy);
				u16 *dest = &bitmap.pix(y, x0);
				u8 *destpri = &destprimap.pix(y, x0);

				for (int x = x0; x <= x1; x++)
				{
					const u32 posx = roz_unit_out(accx, csx, maskx);
					const u32 posy = roz_unit_out(accy, csy, masky);

					// nothing where nMASK is low
					if (!((posx | posy) & 0x800) && (flagsbitmap.pix(posy, posx) & TILEMAP_PIXEL_LAYER0))
					{
						*dest = srcbitmap.pix(posy, posx) + color;
						*destpri = (*destpri & primask) | prival;
					}

					accx += ls.incxx;
					accy += ls.incxy;
					dest++;
					destpri++;
				}
			}
		}
	}
}

void namcos2_roz_device::rozram_word_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rozram[offset]);
	m_tilemap_roz->mark_tile_dirty(offset);
}

uint16_t namcos2_roz_device::control_r(offs_t offset)
{
	return m_roz_ctrl[offset];
}

void namcos2_roz_device::control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_roz_ctrl[offset]);
}
