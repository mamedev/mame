// license:BSD-3-Clause
// copyright-holders:Luca Elia,David Haywood,Paul Priest
/*
    Jaleco Megasystem 32 sprite hardware

    TODO:
    - verify hardware configuration;
    - suchie2: on attract gals background display is cut off on the right side;

    used by:
    ms32.cpp
    bnstars.cpp (Dual screen configuration)
    tetrisp2.cpp (Slightly different - no zoom, stepstag with YUV)

    Sprite format (16 byte per each sprite):
    Offset Bits              Description
           fedcba98 76543210
    00     -xxxxxxx -------- stepstage only: Palette Select for YUV format
           -------- xxxx---- Priority
           -------- -----x-- Visible
           -------- ------x- Flip Y
           -------- -------x Flip X
    02     xxxxxxxx -------- Source Y offset (1 pixel each)
           -------- xxxxxxxx Source X offset (1 pixel each)
    04     xxxx---- -------- Palette Select (normal)
           ----xxxx xxxxxxxx Source Texture Select (each texture is 256 x 256 pixels)
    06     xxxxxxxx -------- Source Width - 1 (1 pixel each)
           -------- xxxxxxxx Source Height - 1 (1 pixel each)
    08     ------xx xxxxxxxx Y (10 bits signed)
    0a     -----xxx xxxxxxxx X (11 bits signed)
    0c     xxxxxxxx xxxxxxxx Zoom X (some hardware disabled this, 0x200 = 50%, 0x100 = 100%, 0x80 = 200%)*
    0e     xxxxxxxx xxxxxxxx Zoom Y (some hardware disabled this, 0x200 = 50%, 0x100 = 100%, 0x80 = 200%)*

    * : Source position add, 8.8 Fixed point
*/

#include "emu.h"
#include "ms32_sprite.h"
#include "drawgfxt.ipp"

DEFINE_DEVICE_TYPE(JALECO_MEGASYSTEM32_SPRITE, ms32_sprite_device, "ms32spr", "Jaleco Megasystem 32 Sprite hardware")

ms32_sprite_device::ms32_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, JALECO_MEGASYSTEM32_SPRITE, tag, owner, clock)
	, device_gfx_interface(mconfig, *this, nullptr)
	, m_color_base(0)
	, m_color_entries(0x10)
	, m_has_zoom(true)
	, m_has_yuv(false)
{
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

/* sprites are contained in 256x256 "tiles" */
static const u32 sprite_xoffset[256] =
{
	STEP8(8*8*8*0,    8), STEP8(8*8*8*1,    8), STEP8(8*8*8*2,    8), STEP8(8*8*8*3,    8),
	STEP8(8*8*8*4,    8), STEP8(8*8*8*5,    8), STEP8(8*8*8*6,    8), STEP8(8*8*8*7,    8),
	STEP8(8*8*8*8,    8), STEP8(8*8*8*9,    8), STEP8(8*8*8*10,   8), STEP8(8*8*8*11,   8),
	STEP8(8*8*8*12,   8), STEP8(8*8*8*13,   8), STEP8(8*8*8*14,   8), STEP8(8*8*8*15,   8),
	STEP8(8*8*8*16,   8), STEP8(8*8*8*17,   8), STEP8(8*8*8*18,   8), STEP8(8*8*8*19,   8),
	STEP8(8*8*8*20,   8), STEP8(8*8*8*21,   8), STEP8(8*8*8*22,   8), STEP8(8*8*8*23,   8),
	STEP8(8*8*8*24,   8), STEP8(8*8*8*25,   8), STEP8(8*8*8*26,   8), STEP8(8*8*8*27,   8),
	STEP8(8*8*8*28,   8), STEP8(8*8*8*29,   8), STEP8(8*8*8*30,   8), STEP8(8*8*8*31,   8)
};
static const u32 sprite_yoffset[256] =
{
	STEP8(8*8*8*0,  8*8), STEP8(8*8*8*32, 8*8), STEP8(8*8*8*64, 8*8), STEP8(8*8*8*96, 8*8),
	STEP8(8*8*8*128,8*8), STEP8(8*8*8*160,8*8), STEP8(8*8*8*192,8*8), STEP8(8*8*8*224,8*8),
	STEP8(8*8*8*256,8*8), STEP8(8*8*8*288,8*8), STEP8(8*8*8*320,8*8), STEP8(8*8*8*352,8*8),
	STEP8(8*8*8*384,8*8), STEP8(8*8*8*416,8*8), STEP8(8*8*8*448,8*8), STEP8(8*8*8*480,8*8),
	STEP8(8*8*8*512,8*8), STEP8(8*8*8*544,8*8), STEP8(8*8*8*576,8*8), STEP8(8*8*8*608,8*8),
	STEP8(8*8*8*640,8*8), STEP8(8*8*8*672,8*8), STEP8(8*8*8*704,8*8), STEP8(8*8*8*736,8*8),
	STEP8(8*8*8*768,8*8), STEP8(8*8*8*800,8*8), STEP8(8*8*8*832,8*8), STEP8(8*8*8*864,8*8),
	STEP8(8*8*8*896,8*8), STEP8(8*8*8*928,8*8), STEP8(8*8*8*960,8*8), STEP8(8*8*8*992,8*8)
};
static const gfx_layout spritelayout =
{
	256, 256,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*8,
	sprite_xoffset,
	sprite_yoffset
};

GFXDECODE_MEMBER(ms32_sprite_device::gfxinfo)
	GFXDECODE_DEVICE(DEVICE_SELF, 0, spritelayout, 0, 16)
GFXDECODE_END

void ms32_sprite_device::device_start()
{
	// decode our graphics
	decode_gfx(gfxinfo);
	gfx(0)->set_colorbase(m_color_base);
	gfx(0)->set_colors(m_color_entries);
}

void ms32_sprite_device::extract_parameters(const u16 *ram, bool &disable, u8 &pri, bool &flipx, bool &flipy, u32 &code, u32 &color, u8 &tx, u8 &ty, u16 &srcwidth, u16 &srcheight, s32 &sx, s32 &sy, u16 &incx, u16 &incy)
{
	const u16 attr =   ram[0];
	pri            = ( attr & 0x00f0);
	disable        = (~attr & 0x0004);

	flipx          =   attr & 1;
	flipy          =   attr & 2;
	code           =   ram[1];
	color          =   ram[2];
	tx             =   (code >> 0) & 0xff;
	ty             =   (code >> 8) & 0xff;

	code           =   (color & 0x0fff);
	// encoded to first word when YUV sprites are used
	if (m_has_yuv)
		color      =   (attr & 0x7f00) >> 8;
	else
		color      =   (color >> 12) & 0xf;
	const u16 size =   ram[3];

	srcwidth       =   ((size >> 0) & 0xff) + 1;
	srcheight      =   ((size >> 8) & 0xff) + 1;

	sx             =   (ram[5] & 0x3ff) - (ram[5] & 0x400);
	sy             =   (ram[4] & 0x1ff) - (ram[4] & 0x200);

	if (m_has_zoom)
	{
		incx       =   (ram[6] & 0xffff);
		incy       =   (ram[7] & 0xffff);
	}
	else
	{
		incx       =   0x100;
		incy       =   0x100;

		// hack for tetrisp2?
		if (srcwidth > 0x100 - tx)
			srcwidth = 0x100 - tx;

		if (srcheight > 0x100 - ty)
			srcheight = 0x100 - ty;
	}
}

/***************************************************************************
    DRAWGFX IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    opaque - render a gfx element with
    no transparency
-------------------------------------------------*/

void ms32_sprite_device::opaque(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);

	color = gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	code %= gfx(0)->elements();
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, [color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_OPAQUE(destp, srcp); });
}

void ms32_sprite_device::opaque(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->opaque(dest, cliprect, code, color, flipx, flipy, destx, desty);

	const pen_t *paldata = palette().pens() + gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	code %= gfx(0)->elements();
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, [paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });
}

/*-------------------------------------------------
    transpen - render a gfx element with
    a single transparent pen
-------------------------------------------------*/

void ms32_sprite_device::transpen(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 trans_pen)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight);

	// use pen usage to optimize
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = gfx(0)->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight);
	}

	// render
	color = gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, [trans_pen, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

void ms32_sprite_device::transpen(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 trans_pen)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight);

	// use pen usage to optimize
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = gfx(0)->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight);
	}

	// render
	const pen_t *paldata = palette().pens() + gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, [trans_pen, paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN(destp, srcp); });
}


/*-------------------------------------------------
    transpen_raw - render a gfx element
    with a single transparent pen and no color
    lookups
-------------------------------------------------*/

void ms32_sprite_device::transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 trans_pen)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// early out if completely transparent
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage() && (gfx(0)->pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, [trans_pen, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

void ms32_sprite_device::transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 trans_pen)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, trans_pen);

	// early out if completely transparent
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage() && (gfx(0)->pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, [trans_pen, color](u32 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

/***************************************************************************
    DRAWGFXZOOM IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    zoom_opaque - render a scaled gfx
    element with no transparency
-------------------------------------------------*/

void ms32_sprite_device::zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight);

	// render
	color = gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	code %= gfx(0)->elements();
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, [color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_OPAQUE(destp, srcp); });
}

void ms32_sprite_device::zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight);

	// render
	const pen_t *paldata = palette().pens() + gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	code %= gfx(0)->elements();
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, [paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE(destp, srcp); });
}


/*-------------------------------------------------
    zoom_transpen - render a scaled gfx
    element with a single transparent pen
-------------------------------------------------*/

void ms32_sprite_device::zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, u32 trans_pen)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy);

	// use pen usage to optimize
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = gfx(0)->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy);
	}

	// render
	color = gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, [trans_pen, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

void ms32_sprite_device::zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, u32 trans_pen)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy);

	// use pen usage to optimize
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = gfx(0)->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy);
	}

	// render
	const pen_t *paldata = palette().pens() + gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, [trans_pen, paldata](u32 &destp, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN(destp, srcp); });
}


/*-------------------------------------------------
    zoom_transpen_raw - render a scaled gfx
    element with a single transparent pen and no
    color lookups
-------------------------------------------------*/

void ms32_sprite_device::zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, u32 trans_pen)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, trans_pen);

	// early out if completely transparent
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage() && (gfx(0)->pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, [trans_pen, color](u16 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

void ms32_sprite_device::zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, u32 trans_pen)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, trans_pen);

	// early out if completely transparent
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage() && (gfx(0)->pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// render
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, [trans_pen, color](u32 &destp, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN(destp, srcp); });
}

/***************************************************************************
    PDRAWGFX IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    prio_opaque - render a gfx element with
    no transparency, checking against the priority
    bitmap
-------------------------------------------------*/

void ms32_sprite_device::prio_opaque(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		bitmap_ind8 &priority, u32 pmask)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	code %= gfx(0)->elements();
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, [pmask, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void ms32_sprite_device::prio_opaque(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		bitmap_ind8 &priority, u32 pmask)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = palette().pens() + gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	code %= gfx(0)->elements();
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, [pmask, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_transpen - render a gfx element with
    a single transparent pen, checking against the
    priority bitmap
-------------------------------------------------*/

void ms32_sprite_device::prio_transpen(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->prio_transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask);

	// use pen usage to optimize
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = gfx(0)->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, [pmask, trans_pen, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void ms32_sprite_device::prio_transpen(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->prio_transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask);

	// use pen usage to optimize
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = gfx(0)->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = palette().pens() + gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, [pmask, trans_pen, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

/*-------------------------------------------------
    priotranspen_raw - render a gfx element
    with a single transparent pen and no color
    lookups, checking against the priority bitmap
-------------------------------------------------*/

void ms32_sprite_device::prio_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->prio_transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage() && (gfx(0)->pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, [pmask, trans_pen, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void ms32_sprite_device::prio_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		bitmap_ind8 &priority, u32 pmask, u32 trans_pen)
{
	// non-clip case
	if (tx == 0 && ty == 0 && srcwidth == 0x100 && srcheight == 0x100)
		gfx(0)->prio_transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage() && (gfx(0)->pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	draw_sprite_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, [pmask, trans_pen, color](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

/***************************************************************************
    PDRAWGFXZOOM IMPLEMENTATIONS
***************************************************************************/

/*-------------------------------------------------
    prio_zoom_opaque - render a scaled gfx
    element with no transparency, checking against
    the priority bitmap
-------------------------------------------------*/

void ms32_sprite_device::prio_zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	code %= gfx(0)->elements();
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, [pmask, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_OPAQUE_PRIORITY(destp, pri, srcp); });
}

void ms32_sprite_device::prio_zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return prio_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask);

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = palette().pens() + gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	code %= gfx(0)->elements();
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, [pmask, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_OPAQUE_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_zoom_transpen - render a scaled gfx
    element with a single transparent pen,
    checking against the priority bitmap
-------------------------------------------------*/

void ms32_sprite_device::prio_zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return prio_transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, pmask);

	// use pen usage to optimize
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = gfx(0)->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	color = gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, [pmask, trans_pen, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void ms32_sprite_device::prio_zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return prio_transpen(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask, trans_pen);

	// special case invalid pens to opaque
	if (trans_pen > 0xff)
		return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, pmask);

	// use pen usage to optimize
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage())
	{
		// fully transparent; do nothing
		u32 usage = gfx(0)->pen_usage(code);
		if ((usage & ~(1 << trans_pen)) == 0)
			return;

		// fully opaque; draw as such
		if ((usage & (1 << trans_pen)) == 0)
			return prio_zoom_opaque(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, pmask);
	}

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	const pen_t *paldata = palette().pens() + gfx(0)->colorbase() + gfx(0)->granularity() * (color % gfx(0)->colors());
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, [pmask, trans_pen, paldata](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REMAP_TRANSPEN_PRIORITY(destp, pri, srcp); });
}


/*-------------------------------------------------
    prio_zoom_transpen_raw - render a scaled gfx
    element with a single transparent pen and no
    color lookups, checking against the priority
    bitmap
-------------------------------------------------*/

void ms32_sprite_device::prio_zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return prio_transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage() && (gfx(0)->pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, [pmask, trans_pen, color](u16 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

void ms32_sprite_device::prio_zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect,
		u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight,
		u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask,
		u32 trans_pen)
{
	// non-zoom case
	if (incx == 0x100 && incy == 0x100)
		return prio_transpen_raw(dest, cliprect, code, color, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, priority, pmask, trans_pen);

	// early out if completely transparent
	code %= gfx(0)->elements();
	if (gfx(0)->has_pen_usage() && (gfx(0)->pen_usage(code) & ~(1 << trans_pen)) == 0)
		return;

	// high bit of the mask is implicitly on
	pmask |= 1 << 31;

	// render
	draw_sprite_zoom_core(dest, cliprect, code, flipx, flipy, destx, desty, tx, ty, srcwidth, srcheight, incx, incy, priority, [pmask, trans_pen, color](u32 &destp, u8 &pri, const u8 &srcp) { PIXEL_OP_REBASE_TRANSPEN_PRIORITY(destp, pri, srcp); });
}

/***************************************************************************
    BASIC DRAWGFX CORE
***************************************************************************/

/*
    Input parameters:
        bitmap_t &dest - the bitmap to render to
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        gfx_element *gfx - pointer to the gfx_element to render
        u32 code - index of the entry within gfx_element
        int flipx - non-zero means render right-to-left instead of left-to-right
        int flipy - non-zero means render bottom-to-top instead of top-to-bottom
        s32 destx - the top-left X coordinate to render to
        s32 desty - the top-left Y coordinate to render to
        bitmap_t &priority - the priority bitmap (if and only if priority is to be applied)
*/


template <typename BitmapType, typename FunctionClass>
inline void ms32_sprite_device::draw_sprite_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, FunctionClass pixel_op)
{
	g_profiler.start(PROFILER_DRAWGFX);
	do {
		assert(dest.valid());
		assert(dest.cliprect().contains(cliprect));
		assert(code < gfx(0)->elements());

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// compute final pixel in X and exit if we are entirely clipped
		s32 destendx = destx + srcwidth - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		u32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = cliprect.left() - destx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		s32 destendy = desty + srcheight - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			break;

		// apply top clip
		u32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = cliprect.top() - desty;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

		// apply X flipping
		s32 dx = 1;
		if (flipx)
		{
			srcx = srcwidth - 1 - srcx;
			dx = -dx;
		}

		// apply Y flipping
		s32 dy = 1;
		if (flipy)
		{
			srcy = srcheight - 1 - srcy;
			dy = -dy;
		}

		// fetch the source data
		const u8 *srcdata = gfx(0)->get_data(code);

		// compute how many blocks of 4 pixels we have
		u32 numblocks = (destendx + 1 - destx) / 4;
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// iterate over pixels in Y
		for (s32 cury = desty; cury <= destendy; cury++)
		{
			u32 drawy = ty + srcy;
			srcy += dy;
			if (drawy >= gfx(0)->height())
				continue;

			auto *destptr = &dest.pix(cury, destx);
			const u8 *srcptr = srcdata + (drawy * gfx(0)->rowbytes());

			u32 cursrcx = srcx;
			// iterate over unrolled blocks of 4
			for (s32 curx = 0; curx < numblocks; curx++)
			{
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[0], srcptr[tx + cursrcx]); } cursrcx += dx;
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[1], srcptr[tx + cursrcx]); } cursrcx += dx;
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[2], srcptr[tx + cursrcx]); } cursrcx += dx;
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[3], srcptr[tx + cursrcx]); } cursrcx += dx;

				destptr += 4;
			}

			// iterate over leftover pixels
			for (s32 curx = 0; curx < leftovers; curx++)
			{
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[0], srcptr[tx + cursrcx]); } cursrcx += dx;
				destptr++;
			}
		}
	} while (0);
	g_profiler.stop();
}


template <typename BitmapType, typename PriorityType, typename FunctionClass>
inline void ms32_sprite_device::draw_sprite_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, PriorityType &priority, FunctionClass pixel_op)
{
	g_profiler.start(PROFILER_DRAWGFX);
	do {
		assert(dest.valid());
		assert(priority.valid());
		assert(dest.cliprect().contains(cliprect));
		assert(code < gfx(0)->elements());

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		// compute final pixel in X and exit if we are entirely clipped
		s32 destendx = destx + srcwidth - 1;
		if (destx > cliprect.right() || destendx < cliprect.left())
			break;

		// apply left clip
		u32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = cliprect.left() - destx;
			destx = cliprect.left();
		}

		// apply right clip
		if (destendx > cliprect.right())
			destendx = cliprect.right();

		// compute final pixel in Y and exit if we are entirely clipped
		s32 destendy = desty + srcheight - 1;
		if (desty > cliprect.bottom() || destendy < cliprect.top())
			break;

		// apply top clip
		u32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = cliprect.top() - desty;
			desty = cliprect.top();
		}

		// apply bottom clip
		if (destendy > cliprect.bottom())
			destendy = cliprect.bottom();

		// apply X flipping
		s32 dx = 1;
		if (flipx)
		{
			srcx = srcwidth - 1 - srcx;
			dx = -dx;
		}

		// apply Y flipping
		s32 dy = 1;
		if (flipy)
		{
			srcy = srcheight - 1 - srcy;
			dy = -dy;
		}

		// fetch the source data
		const u8 *srcdata = gfx(0)->get_data(code);

		// compute how many blocks of 4 pixels we have
		u32 numblocks = (destendx + 1 - destx) / 4;
		u32 leftovers = (destendx + 1 - destx) - 4 * numblocks;

		// iterate over pixels in Y
		for (s32 cury = desty; cury <= destendy; cury++)
		{
			u32 drawy = ty + srcy;
			srcy += dy;
			if (drawy >= gfx(0)->height())
				continue;

			auto *priptr = &priority.pix(cury, destx);
			auto *destptr = &dest.pix(cury, destx);
			const u8 *srcptr = srcdata + (drawy * gfx(0)->rowbytes());

			u32 cursrcx = srcx;
			// iterate over unrolled blocks of 4
			for (s32 curx = 0; curx < numblocks; curx++)
			{
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[0], priptr[0], srcptr[tx + cursrcx]); } cursrcx += dx;
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[1], priptr[1], srcptr[tx + cursrcx]); } cursrcx += dx;
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[2], priptr[2], srcptr[tx + cursrcx]); } cursrcx += dx;
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[3], priptr[3], srcptr[tx + cursrcx]); } cursrcx += dx;

				destptr += 4;
				priptr += 4;
			}

			// iterate over leftover pixels
			for (s32 curx = 0; curx < leftovers; curx++)
			{
				if (tx + cursrcx < gfx(0)->width()) { pixel_op(destptr[0], priptr[0], srcptr[tx + cursrcx]); } cursrcx += dx;
				destptr++;
				priptr++;
			}
		}
	} while (0);
	g_profiler.stop();
}
/***************************************************************************
    BASIC DRAWGFXZOOM CORE
***************************************************************************/

/*
    Input parameters:
        bitmap_t &dest - the bitmap to render to
        const rectangle &cliprect - a clipping rectangle (assumed to be clipped to the size of 'dest')
        gfx_element *gfx - pointer to the gfx_element to render
        u32 code - index of the entry within gfx_element
        int flipx - non-zero means render right-to-left instead of left-to-right
        int flipy - non-zero means render bottom-to-top instead of top-to-bottom
        s32 destx - the top-left X coordinate to render to
        s32 desty - the top-left Y coordinate to render to
        u32 scalex - the 16.16 scale factor in the X dimension
        u32 scaley - the 16.16 scale factor in the Y dimension
        bitmap_t &priority - the priority bitmap (if and only if priority is to be applied)
*/


template <typename BitmapType, typename FunctionClass>
inline void ms32_sprite_device::draw_sprite_zoom_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, FunctionClass pixel_op)
{
	if (!incx || !incy)
		return;

	g_profiler.start(PROFILER_DRAWGFX);
	do {
		assert(dest.valid());
		assert(dest.cliprect().contains(cliprect));

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		s32 srcstartx = tx << 8;
		s32 srcstarty = ty << 8;
		s32 srcendx = srcwidth << 8;
		s32 srcendy = srcheight << 8;
		// apply left clip
		u32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = (cliprect.left() - destx) * incx;
			destx = cliprect.left();
		}
		if (srcx >= srcendx)
			break;

		// apply top clip
		u32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = (cliprect.top() - desty) * incy;
			desty = cliprect.top();
		}
		if (srcy >= srcendy)
			break;

		// fetch the source data
		const u8 *srcdata = gfx(0)->get_data(code);

		// iterate over pixels in Y
		for (s32 cury = desty; (cury <= cliprect.bottom()) && (srcy < srcendy); cury++, srcy += incy)
		{
			u32 drawy = (srcstarty + (flipy ? (srcendy - srcy - 1) : srcy)) >> 8;
			if (drawy >= gfx(0)->height())
				continue;

			auto *destptr = &dest.pix(cury);
			const u8 *srcptr = srcdata + drawy * gfx(0)->rowbytes();
			u32 cursrcx = srcx;

			// iterate over pixels
			for (s32 curx = destx; (curx <= cliprect.right()) && (cursrcx < srcendx); curx++, cursrcx += incx)
			{
				u32 drawx = (srcstartx + (flipx ? (srcendx - cursrcx - 1) : cursrcx)) >> 8;
				if (drawx >= gfx(0)->width())
					continue;

				pixel_op(destptr[curx], srcptr[drawx]);
			}
		}
	} while (0);
	g_profiler.stop();
}


template <typename BitmapType, typename PriorityType, typename FunctionClass>
inline void ms32_sprite_device::draw_sprite_zoom_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, PriorityType &priority, FunctionClass pixel_op)
{
	if (!incx || !incy)
		return;

	g_profiler.start(PROFILER_DRAWGFX);
	do {
		assert(dest.valid());
		assert(priority.valid());
		assert(dest.cliprect().contains(cliprect));

		// ignore empty/invalid cliprects
		if (cliprect.empty())
			break;

		s32 srcstartx = tx << 8;
		s32 srcstarty = ty << 8;
		s32 srcendx = srcwidth << 8;
		s32 srcendy = srcheight << 8;
		// apply left clip
		u32 srcx = 0;
		if (destx < cliprect.left())
		{
			srcx = (cliprect.left() - destx) * incx;
			destx = cliprect.left();
		}
		if (srcx >= srcendx)
			break;

		// apply top clip
		u32 srcy = 0;
		if (desty < cliprect.top())
		{
			srcy = (cliprect.top() - desty) * incy;
			desty = cliprect.top();
		}
		if (srcy >= srcendy)
			break;

		// fetch the source data
		const u8 *srcdata = gfx(0)->get_data(code);

		// iterate over pixels in Y
		for (s32 cury = desty; (cury <= cliprect.bottom()) && (srcy < srcendy); cury++, srcy += incy)
		{
			u32 drawy = (srcstarty + (flipy ? (srcendy - srcy - 1) : srcy)) >> 8;
			if (drawy >= gfx(0)->height())
				continue;

			auto *priptr = &priority.pix(cury);
			auto *destptr = &dest.pix(cury);
			const u8 *srcptr = srcdata + drawy * gfx(0)->rowbytes();
			u32 cursrcx = srcx;

			// iterate over pixels
			for (s32 curx = destx; (curx <= cliprect.right()) && (cursrcx < srcendx); curx++, cursrcx += incx)
			{
				u32 drawx = (srcstartx + (flipx ? (srcendx - cursrcx - 1) : cursrcx)) >> 8;
				if (drawx >= gfx(0)->width())
					continue;

				pixel_op(destptr[curx], priptr[curx], srcptr[drawx]);
			}
		}
	} while (0);
	g_profiler.stop();
}
