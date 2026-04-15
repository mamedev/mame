// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
IGS023 (PGM) style video

Used by:
- igs/pgm.cpp
- igs/igs_68k_023vid.cpp
- igs/igs_m027_023vid.cpp

TODO:
- Interrupt handling and background scaling are not implemented
- Is video register area mirrored?

*/

#include "emu.h"
#include "igs023_video.h"

#include "screen.h"


#define LOG_UNK     (1U << 1)

#define LOG_ALL     (LOG_UNK)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGUNK(...) LOGMASKED(LOG_UNK, __VA_ARGS__)

namespace {

const gfx_layout pgm32_charlayout =
{
	32,32,
	RGN_FRAC(1,1),
	5,
	{ 4, 3, 2, 1, 0 },
	{ STEP32(0,5) },
	{ STEP32(0,5*32) },
	32*32*5
};

constexpr bool get_flipy(u8 flip) { return BIT(flip, 1); }
constexpr bool get_flipx(u8 flip) { return BIT(flip, 0); }

} // anonymous namespace

void igs023_video_device::videoram_map(address_map &map)
{
	map(0x0000, 0x0fff).mirror(0x3000).rw(FUNC(igs023_video_device::bg_videoram_r), FUNC(igs023_video_device::bg_videoram_w));
	map(0x4000, 0x5fff).mirror(0x2000).rw(FUNC(igs023_video_device::tx_videoram_r), FUNC(igs023_video_device::tx_videoram_w));
	map(0x7000, 0x7fff).rw(FUNC(igs023_video_device::rowscrollram_r), FUNC(igs023_video_device::rowscrollram_w));
}

void igs023_video_device::videoregs_map(address_map &map)
{
	map(0x0000, 0x0fff).rw(FUNC(igs023_video_device::spritebuffer_r), FUNC(igs023_video_device::spritebuffer_w)); // read only?
	map(0x1000, 0x103f).w(FUNC(igs023_video_device::zoomram_w));
	map(0x2000, 0x2001).rw(FUNC(igs023_video_device::bg_yscroll_r), FUNC(igs023_video_device::bg_yscroll_w));
	map(0x3000, 0x3001).rw(FUNC(igs023_video_device::bg_xscroll_r), FUNC(igs023_video_device::bg_xscroll_w));
	map(0x4000, 0x4001).rw(FUNC(igs023_video_device::bg_scale_r), FUNC(igs023_video_device::bg_scale_w));
	map(0x5000, 0x5001).rw(FUNC(igs023_video_device::tx_yscroll_r), FUNC(igs023_video_device::tx_yscroll_w));
	map(0x6000, 0x6001).rw(FUNC(igs023_video_device::tx_xscroll_r), FUNC(igs023_video_device::tx_xscroll_w));
	map(0x7000, 0x7001).lr16(NAME([this]() -> u16 { return screen().vpos(); }));
	map(0xe000, 0xe001).rw(FUNC(igs023_video_device::ctrl_r), FUNC(igs023_video_device::ctrl_w));
}

DEFINE_DEVICE_TYPE(IGS023_VIDEO, igs023_video_device, "igs023", "IGS023 Video System")

igs023_video_device::igs023_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, IGS023_VIDEO, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, gfxinfo)
	, device_video_interface(mconfig, *this)
	, m_gfx_region(*this, DEVICE_SELF)
	, m_adata(*this, "sprcol")
	, m_bdata(*this, "sprmask")
	, m_bg_videoram(*this, "bg_videoram", 0x1000, ENDIANNESS_BIG) // or 0x4000?
	, m_tx_videoram(*this, "tx_videoram", 0x2000, ENDIANNESS_BIG)
	, m_rowscrollram(*this, "rowscrollram", 0x1000, ENDIANNESS_BIG)
	, m_spritebuffer(*this, "spritebuffer", 0x1000, ENDIANNESS_BIG)
	, m_zoomram(*this, "zoomram", 0x40, ENDIANNESS_BIG)
	, m_readspriteram_cb(*this, 0)
	, m_sprite_ptr_pre(nullptr)
	, m_bg_tilemap(nullptr)
	, m_tx_tilemap(nullptr)
	, m_aoffset(0)
	, m_abit(0)
	, m_boffset(0)
	, m_bg_yscroll(0)
	, m_bg_xscroll(0)
	, m_bg_scale(0)
	, m_tx_yscroll(0)
	, m_tx_xscroll(0)
	, m_ctrl(0)
{
}

GFXDECODE_MEMBER( igs023_video_device::gfxinfo )
	GFXDECODE_DEVICE(            DEVICE_SELF, 0, gfx_8x8x4_packed_lsb, 0x800, 32 ) /* 8x8x4 Tiles */
	GFXDECODE_DEVICE_REVERSEBITS(DEVICE_SELF, 0, pgm32_charlayout,     0x400, 32 ) /* 32x32x5 Tiles */
GFXDECODE_END


// video RAM
u16 igs023_video_device::bg_videoram_r(offs_t offset)
{
	return m_bg_videoram[offset];
}

void igs023_video_device::bg_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

u16 igs023_video_device::tx_videoram_r(offs_t offset)
{
	return m_tx_videoram[offset];
}

void igs023_video_device::tx_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset / 2);
}

u16 igs023_video_device::rowscrollram_r(offs_t offset)
{
	return m_rowscrollram[offset];
}

void igs023_video_device::rowscrollram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_rowscrollram[offset]);
}

// video registers
u16 igs023_video_device::spritebuffer_r(offs_t offset)
{
	return m_spritebuffer[offset];
}

void igs023_video_device::spritebuffer_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_spritebuffer[offset]);
}

void igs023_video_device::zoomram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_zoomram[offset]);
}

u16 igs023_video_device::bg_yscroll_r()
{
	return m_bg_yscroll;
}

void igs023_video_device::bg_yscroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_yscroll);
}

u16 igs023_video_device::bg_xscroll_r()
{
	return m_bg_xscroll;
}

void igs023_video_device::bg_xscroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_bg_xscroll);
}

u16 igs023_video_device::bg_scale_r()
{
	return m_bg_scale;
}

void igs023_video_device::bg_scale_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*
		Background scale register format

		Bit                 Description
		fedc ba98 7654 3210 
		---- --xx xxx- ---- Vertical scale
		---- ---- ---x xxxx Horizontal scale

		* Scale range is 50% to 200%, 0b10000 being 100%
		* Unmarked bits are can be set but unknown and/or unused
		TODO: not implemented, unknown algorithm
	*/
	COMBINE_DATA(&m_bg_scale);
	if (m_bg_scale != 0x210)
		LOGUNK("%s: Unknown bg_scale_w write %04x & %04x", machine().describe_context(), data, mem_mask);
}

u16 igs023_video_device::tx_yscroll_r()
{
	return m_tx_yscroll;
}

void igs023_video_device::tx_yscroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_yscroll);
	m_tx_tilemap->set_scrolly(0, m_tx_yscroll);
}

u16 igs023_video_device::tx_xscroll_r()
{
	return m_tx_xscroll;
}

void igs023_video_device::tx_xscroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_xscroll);
	m_tx_tilemap->set_scrollx(0, m_tx_xscroll);
}

u16 igs023_video_device::ctrl_r()
{
	return m_ctrl;
}

void igs023_video_device::ctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 prev = m_ctrl;
	/*
		Control register format

		Bit                 Description
		fedc ba98 7654 3210 
		--x- ---- ---- ---- Disable high priority sprites
		---x ---- ---- ---- Disable background layer
		---- x--- ---- ---- Disable text layer
		---- ---- ---- x--- enable interrupt triggered each VBlank
		---- ---- ---- -x-- enable interrupt triggered every 62 scanlines (Not synced with VBlank)
		---- ---- ---- ---x Sprite DMA enable

		* Unmarked bits are can be set but unknown and/or unused
	*/
	COMBINE_DATA(&m_ctrl);
	if ((prev ^ m_ctrl) & 0xc7f2)
		LOGUNK("%s: Unknown ctrl_w write %04x & %04x", machine().describe_context(), data, mem_mask);
}


/******************************************************************************
 Sprites

 these are fairly complex to render due to the data format, unless you
 pre-decode the data you have to draw pixels in the order they're decoded from
 the ROM which becomes quite complex with flipped and zoomed cases
******************************************************************************/

// nothing pri is 0
// bg pri is 2
// sprite already here is 1 / 3

inline void igs023_video_device::pgm_draw_pix(int xdrawpos, int pri, u16 *dest, u8 *destpri, const rectangle &cliprect, u16 srcdat)
{
	if ((xdrawpos >= cliprect.min_x) && (xdrawpos <= cliprect.max_x))
	{
		if (!(destpri[xdrawpos] & 1))
		{
			if ((!pri) || (!(destpri[xdrawpos] & 2)))
				dest[xdrawpos] = srcdat;
		}

		destpri[xdrawpos] |= 1;
	}
}

inline u8 igs023_video_device::get_sprite_pix()
{
	const u8 srcdat = ((m_adata[m_aoffset & (m_adata.length() - 1)] >> m_abit) & 0x1f);
	m_abit += 5; // 5 bit per pixel, 3 pixels in each word; 15 bit used
	if (m_abit >= 15)
	{
		m_aoffset++;
		m_abit = 0;
	}
	return srcdat;
}

/*************************************************************************
 Full Sprite Renderer
  for complex zoomed cases
*************************************************************************/

inline void igs023_video_device::draw_sprite_pixel(int &xdrawpos, int &xcntdraw, u16 *dest, u8 *destpri, const rectangle &cliprect, int xpos, int flip, int pri, int realxsize, u16 srcdat)
{
	if (!get_flipx(flip))
		xdrawpos = xpos + xcntdraw;
	else
		xdrawpos = xpos + realxsize - xcntdraw;

	pgm_draw_pix(xdrawpos, pri, dest, destpri, cliprect, srcdat);

	xcntdraw++;
}

void igs023_video_device::draw_sprite_line(int wide, u16 *dest, u8 *destpri, const rectangle &cliprect, int xzoom, bool xgrow, int flip, int xpos, int pri, int realxsize, int palt, bool draw)
{
	int xoffset = 0;
	int xdrawpos = 0;
	int xcntdraw = 0;

	for (int xcnt = 0; xcnt < wide; xcnt++)
	{
		u16 msk = m_bdata[m_boffset & (m_bdata.length() - 1)];

		for (int x = 0; x < 16; x++)
		{
			const bool xzoombit = BIT(xzoom, xoffset & 0x1f);
			xoffset++;

			if (!(BIT(msk, 0)))
			{
				const u16 srcdat = get_sprite_pix() + palt * 32;

				if (draw)
				{
					if (xgrow || (!xzoombit))
					{
						const int pixel_column = xzoombit ? 2 : 1;
						for (int i = 0; i < pixel_column; i++)
						{
							draw_sprite_pixel(xdrawpos, xcntdraw, dest, destpri, cliprect, xpos, flip, pri, realxsize, srcdat);
						}
					}
				}
			}
			else
			{
				if (xgrow || (!xzoombit))
					xcntdraw += xzoombit ? 2 : 1;
			}

			msk >>= 1;
		}

		m_boffset++;
	}
}

void igs023_video_device::draw_sprite_new_zoomed(int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, u32 xzoom, bool xgrow, u32 yzoom, bool ygrow, int pri)
{
	int ydrawpos;
	int xcnt = 0;

	m_aoffset = (m_bdata[(m_boffset + 1) & (m_bdata.length() - 1)] << 16) | (m_bdata[(m_boffset + 0) & (m_bdata.length() - 1)] << 0);
	m_aoffset = m_aoffset >> 2;
	m_abit = 0;

	m_boffset += 2;

	/* precalculate where drawing will end, for flipped zoomed cases. */
	/* if we're to avoid pre-decoding the data for each sprite each time we draw then we have to draw the sprite data
	   in the order it is in ROM due to the nature of the compresson scheme.  This means drawing upwards from the end point
	   in the case of flipped sprites */
	int ycnt = 0;
	int ycntdraw = 0;
	int realysize = 0;

	while (ycnt < high)
	{
		const bool yzoombit = BIT(yzoom, ycnt & 0x1f);
		if (ygrow || (!yzoombit))
			realysize += yzoombit ? 2 : 1;

		ycnt++;
	}
	realysize--;

	int realxsize = 0;

	while (xcnt < wide * 16)
	{
		const bool xzoombit = BIT(xzoom, xcnt & 0x1f);
		if (xgrow || (!xzoombit))
			realxsize += xzoombit ? 2 : 1;

		xcnt++;
	}
	realxsize--;

	/* now draw it */
	ycnt = 0;
	ycntdraw = 0;

	while (ycnt < high)
	{
		const bool yzoombit = BIT(yzoom, ycnt & 0x1f);

		if (yzoombit && ygrow) // double this line
		{
			const int temp_aoffset = m_aoffset;
			const int temp_abit = m_abit;
			const int temp_boffset = m_boffset;

			if (!get_flipy(flip))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
			{
				u16 *dest = &bitmap.pix(ydrawpos);
				u8 *destpri = &priority_bitmap.pix(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);
			}

			ycntdraw++;

			// we need to draw this line again, so restore our pointers to previous values
			m_aoffset = temp_aoffset;
			m_abit = temp_abit;
			m_boffset = temp_boffset;

			if (!get_flipy(flip))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
			{
				u16 *dest = &bitmap.pix(ydrawpos);
				u8 *destpri = &priority_bitmap.pix(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);

				if (!get_flipy(flip))
				{
					if (ydrawpos >= cliprect.max_y)
						return;
				}
				else
				{
					if (ydrawpos < cliprect.min_y)
						return;
				}
			}

			ycntdraw++;

		}
		else if (yzoombit && (!ygrow))
		{
			/* skip this line */
			draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);
		}
		else /* normal line */
		{
			if (!get_flipy(flip))
				ydrawpos = ypos + ycntdraw;
			else
				ydrawpos = ypos + realysize - ycntdraw;

			if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
			{
				u16 *dest = &bitmap.pix(ydrawpos);
				u8 *destpri = &priority_bitmap.pix(ydrawpos);
				draw_sprite_line(wide, dest, destpri, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, true);
			}
			else
			{
				draw_sprite_line(wide, nullptr, nullptr, cliprect, xzoom, xgrow, flip, xpos, pri, realxsize, palt, false);

				if (!get_flipy(flip))
				{
					if (ydrawpos >= cliprect.max_y)
						return;
				}
				else
				{
					if (ydrawpos < cliprect.min_y)
						return;
				}

			}

			ycntdraw++;
		}

		ycnt++;
	}
}


void igs023_video_device::draw_sprite_line_basic(int wide, u16 *dest, u8 *destpri, const rectangle &cliprect, int flip, int xpos, int pri, int realxsize, int palt, bool draw)
{
	int xdrawpos = 0;
	int xcntdraw = 0;

	for (int xcnt = 0; xcnt < wide; xcnt++)
	{
		u16 msk = m_bdata[m_boffset & (m_bdata.length() - 1)];

		for (int x = 0; x < 16; x++)
		{
			if (!(BIT(msk, 0)))
			{
				const u16 srcdat = get_sprite_pix() + palt * 32;

				if (draw)
					draw_sprite_pixel(xdrawpos, xcntdraw, dest, destpri, cliprect, xpos, flip, pri, realxsize, srcdat);
			}
			else
				xcntdraw++;

			msk >>= 1;
		}

		m_boffset++;
	}
}

/*************************************************************************
 Basic Sprite Renderer
  simplified version for non-zoomed cases, a bit faster
*************************************************************************/

void igs023_video_device::draw_sprite_new_basic(int wide, int high, int xpos, int ypos, int palt, int flip, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, int pri)
{
	int ydrawpos;

	m_aoffset = (m_bdata[(m_boffset + 1) & (m_bdata.length() - 1)] << 16) | (m_bdata[(m_boffset + 0) & (m_bdata.length() - 1)] << 0);
	m_aoffset = m_aoffset >> 2;
	m_abit = 0;

	m_boffset += 2;

	const int realysize = high - 1;
	const int realxsize = (wide * 16) - 1;

	/* now draw it */
	int ycnt = 0;
	int ycntdraw = 0;

	while (ycnt < high)
	{
		if (!get_flipy(flip))
			ydrawpos = ypos + ycntdraw;
		else
			ydrawpos = ypos + realysize - ycntdraw;

		if ((ydrawpos >= cliprect.min_y) && (ydrawpos <= cliprect.max_y))
		{
			u16 *dest = &bitmap.pix(ydrawpos);
			u8 *destpri = &priority_bitmap.pix(ydrawpos);
			draw_sprite_line_basic(wide, dest, destpri, cliprect, flip, xpos, pri, realxsize, palt, true);
		}
		else
		{
			draw_sprite_line_basic(wide, nullptr, nullptr, cliprect, flip, xpos, pri, realxsize, palt, false);

			if (!get_flipy(flip))
			{
				if (ydrawpos >= cliprect.max_y)
					return;
			}
			else
			{
				if (ydrawpos < cliprect.min_y)
					return;
			}
		}

		ycntdraw++;
		ycnt++;
	}
}


void igs023_video_device::draw_sprites(bitmap_ind16& spritebitmap, const rectangle &cliprect, bitmap_ind8& priority_bitmap)
{
	struct sprite_t *sprite_ptr = m_sprite_ptr_pre;
	while (sprite_ptr != m_spritelist.get())
	{
		sprite_ptr--;

		if (BIT(m_ctrl, 13) && (!sprite_ptr->pri))
			continue;

		m_boffset = sprite_ptr->offs;
		if ((!sprite_ptr->xzoom) && (!sprite_ptr->yzoom))
		{
			draw_sprite_new_basic(sprite_ptr->width, sprite_ptr->height,
				sprite_ptr->x, sprite_ptr->y,
				sprite_ptr->color, sprite_ptr->flip,
				spritebitmap, cliprect, priority_bitmap,
				sprite_ptr->pri);
		}
		else
		{
			draw_sprite_new_zoomed(sprite_ptr->width, sprite_ptr->height,
				sprite_ptr->x, sprite_ptr->y,
				sprite_ptr->color, sprite_ptr->flip,
				spritebitmap, cliprect, priority_bitmap,
				sprite_ptr->xzoom, sprite_ptr->xgrow, sprite_ptr->yzoom, sprite_ptr->ygrow, sprite_ptr->pri);
		}
	}
}

/*
        Sprite list format (10 bytes per sprites, 256 entries)

    Offset Bits
           fedcba98 76543210
    00     x------- -------- Horizontal Zoom/Shrink mode select
           -xxxx--- -------- Horizontal Zoom/Shrink table select
           -----xxx xxxxxxxx X position (11 bit signed)

    02     x------- -------- Vertical Zoom/Shrink mode select
           -xxxx--- -------- Vertical Zoom/Shrink table select
           ------xx xxxxxxxx Y position (10 bit signed)

    04     -x------ -------- Flip Y
           --x----- -------- Flip X
           ---xxxxx -------- Palette select (32 color each)
           -------- x------- Priority (Over(0) or Under(1) background)
           -------- -xxxxxxx Sprite mask ROM address MSB
    06     xxxxxxxx xxxxxxxx Sprite mask ROM address LSB

    08     x------- -------- Another sprite width bit?
           -xxxxxx- -------- Sprite width (16 pixel each)
           -------x xxxxxxxx Sprite height (1 pixel each)

*/
void igs023_video_device::get_sprites()
{
	if (!sprite_dma())
		return;

	m_sprite_ptr_pre = m_spritelist.get();

	int sprite_num = 0;

	while (sprite_num < 0x1000/2)
	{
		const u16 spr4 = m_spritebuffer[sprite_num + 4];
		if ((spr4 & 0x7fff) == 0) break; // verified on hardware

		const u16 spr0 = m_spritebuffer[sprite_num + 0];
		const bool xgrow =         BIT(spr0, 15);
		int xzom =                 (spr0 & 0x7800) >> 11;
		m_sprite_ptr_pre->x =      util::sext(spr0 & 0x07ff, 11);

		const u16 spr1 = m_spritebuffer[sprite_num + 1];
		const bool ygrow =         BIT(spr1, 15);
		int yzom =                 (spr1 & 0x7800) >> 11;
		m_sprite_ptr_pre->y =      util::sext(spr1 & 0x03ff, 10);

		const u16 spr2 = m_spritebuffer[sprite_num + 2];
		const u16 spr3 = m_spritebuffer[sprite_num + 3];

		m_sprite_ptr_pre->flip =   (spr2 & 0x6000) >> 13;
		m_sprite_ptr_pre->color =  (spr2 & 0x1f00) >> 8;
		m_sprite_ptr_pre->pri =    BIT(spr2, 7);
		m_sprite_ptr_pre->offs =  ((spr2 & 0x007f) << 16) | (spr3 & 0xffff);

		m_sprite_ptr_pre->width =  (spr4 & 0x7e00) >> 9;
		m_sprite_ptr_pre->height =  spr4 & 0x01ff;

		if (xgrow)
		{
		//  xzom = 0xf - xzom; // would make more sense but everything gets zoomed slightly in dragon world 2 ?!
			xzom = 0x10 - xzom; // this way it doesn't but there is a bad line when zooming after the level select?
		}

		if (ygrow)
		{
		//  yzom = 0xf - yzom; // see comment above
			yzom = 0x10 - yzom;
		}

		// some games (e.g. ddp3) have zero in last zoom table entry but expect 1
		// is the last entry hard-coded to 1, or does zero have the same effect as 1?
		m_sprite_ptr_pre->xzoom = (xzom < 0x10) ? (xzom == 0xf) ? 1 : ((u32(m_zoomram[xzom * 2]) << 16) | m_zoomram[xzom * 2 + 1]) : 0;
		m_sprite_ptr_pre->yzoom = (yzom < 0x10) ? (yzom == 0xf) ? 1 : ((u32(m_zoomram[yzom * 2]) << 16) | m_zoomram[yzom * 2 + 1]) : 0;
		m_sprite_ptr_pre->xgrow = xgrow;
		m_sprite_ptr_pre->ygrow = ygrow;
		m_sprite_ptr_pre++;
		sprite_num += 8;
	}
}

/* TX Layer */
TILE_GET_INFO_MEMBER(igs023_video_device::get_tx_tile_info)
{
/* 0x904000 - 0x90ffff is the Text Overlay Ram (pgm_tx_videoram)
    each tile uses 4 bytes, the tilemap is 64x128?

   the layer uses 4bpp 8x8 tiles from the 'T' roms
   colours from 0xA01000 - 0xA013FF

   scroll registers are at 0xB05000 (Y) and 0xB06000 (X)

    ---- ---- ffpp ppp- nnnn nnnn nnnn nnnn

    n = tile number
    p = palette
    f = flip
*/
	const u32 tileno = m_tx_videoram[tile_index * 2] & 0xffff;
	const u32 colour = (m_tx_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	const u8  flipyx = (m_tx_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	tileinfo.set(0, tileno, colour, TILE_FLIPYX(flipyx));
}

/* BG Layer */

TILE_GET_INFO_MEMBER(igs023_video_device::get_bg_tile_info)
{
	/* pretty much the same as tx layer */

	const u32 tileno = m_bg_videoram[tile_index *2] & 0xffff;
	const u32 colour = (m_bg_videoram[tile_index * 2 + 1] & 0x3e) >> 1;
	const u8  flipyx = (m_bg_videoram[tile_index * 2 + 1] & 0xc0) >> 6;

	tileinfo.set(1, tileno, colour, TILE_FLIPYX(flipyx));
}



void igs023_video_device::device_start()
{
	// assumes it can make an address mask with .length() - 1 on these
	assert(!(m_adata.length() & (m_adata.length() - 1)));
	assert(!(m_bdata.length() & (m_bdata.length() - 1)));

	m_spritelist = std::make_unique<sprite_t[]>(0xa00/2/5);
	m_sprite_ptr_pre = m_spritelist.get();

	m_aoffset = 0;
	m_abit = 0;
	m_boffset = 0;

	m_tx_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(igs023_video_device::get_tx_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_tx_tilemap->set_transparent_pen(15);

	m_bg_tilemap = &machine().tilemap().create(*this, tilemap_get_info_delegate(*this, FUNC(igs023_video_device::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 32, 32, 64, 16);
	m_bg_tilemap->set_transparent_pen(31);
	m_bg_tilemap->set_scroll_rows(16 * 32);

	save_item(NAME(m_bg_yscroll));
	save_item(NAME(m_bg_xscroll));
	save_item(NAME(m_bg_scale));
	save_item(NAME(m_tx_yscroll));
	save_item(NAME(m_tx_xscroll));
	save_item(NAME(m_ctrl));
}

void igs023_video_device::device_reset()
{
}


u32 igs023_video_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x3ff, cliprect); // ddp2 igs logo needs 0x3ff

	screen.priority().fill(0, cliprect);

	if (BIT(~m_ctrl, 12))
	{
		m_bg_tilemap->set_scrolly(0, m_bg_yscroll);

		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			m_bg_tilemap->set_scrollx((y + m_bg_yscroll) & 0x1ff, m_bg_xscroll + m_rowscrollram[y]);

		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 2);
	}

	draw_sprites(bitmap, cliprect, screen.priority());

	if (BIT(~m_ctrl, 11))
		m_tx_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

bool igs023_video_device::sprite_dma()
{
	// verified on hardware
	constexpr u16 ram_mask[5] = { 0xffff, 0xfbff, 0x7fff, 0xffff, 0xffff };
	if (BIT(~m_ctrl, 0))
		return false;

	for (int i = 0, dst = 0, offs = 0; i < 256; i++, dst += 8)
	{
		for (int src = 0; src < 5; src++)
		{
			m_spritebuffer[dst + src] = m_readspriteram_cb(offs++) & ram_mask[src];
		}
		if ((m_spritebuffer[dst + 4] & 0x7fff) == 0)
			return true;
	}
	return true;
}
