// license:BSD-3-Clause
// copyright-holders:Luca Elia,David Haywood,Paul Priest
/*
    Jaleco Megasystem 32 sprite hardware
*/

#ifndef MAME_JALECO_MS32_SPRITE_H
#define MAME_JALECO_MS32_SPRITE_H

#pragma once

class ms32_sprite_device : public device_t, public device_gfx_interface
{
public:
	// construction
	ms32_sprite_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_color_base(u16 base) { m_color_base = base; }
	void set_color_entries(u16 entries) { m_color_entries = entries; }
	void set_zoom(bool enable) { m_has_zoom = enable; }
	void set_yuv(bool enable) { m_has_yuv = enable; }

	void extract_parameters(const u16 *ram, bool &disable, u8 &pri, bool &flipx, bool &flipy, u32 &code, u32 &color, u8 &tx, u8 &ty, u16 &srcwidth, u16 &srcheight, s32 &sx, s32 &sy, u16 &incx, u16 &incy);

	// ----- core graphics drawing -----

	// core drawgfx implementation
	template <typename BitmapType, typename FunctionClass> void draw_sprite_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, FunctionClass pixel_op);

	// specific drawgfx implementations for each transparency type
	void opaque(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight);
	void opaque(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight);
	void transpen(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 trans_pen);
	void transpen(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 trans_pen);
	void transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 trans_pen);
	void transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 trans_pen);

	// ----- zoomed graphics drawing -----

	// core zoom implementation
	template <typename BitmapType, typename FunctionClass> void draw_sprite_zoom_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, FunctionClass pixel_op);

	// specific zoom implementations for each transparency type
	void zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy);
	void zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy);
	void zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, u32 trans_pen);
	void zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, u32 trans_pen);
	void zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, u32 trans_pen);
	void zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, u32 trans_pen);

	// ----- priority masked graphics drawing -----

	// core prio implementation
	template <typename BitmapType, typename PriorityType, typename FunctionClass> void draw_sprite_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, PriorityType &priority, FunctionClass pixel_op);

	// specific prio implementations for each transparency type
	void prio_opaque(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, bitmap_ind8 &priority, u32 pmask);
	void prio_opaque(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, bitmap_ind8 &priority, u32 pmask);
	void prio_transpen(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);
	void prio_transpen(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);
	void prio_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);
	void prio_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);

	// ----- priority masked zoomed graphics drawing -----

	// core prio_zoom implementation
	template <typename BitmapType, typename PriorityType, typename FunctionClass> void draw_sprite_zoom_core(BitmapType &dest, const rectangle &cliprect, u32 code, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, PriorityType &priority, FunctionClass pixel_op);

	// specific prio_zoom implementations for each transparency type
	void prio_zoom_opaque(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask);
	void prio_zoom_opaque(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask);
	void prio_zoom_transpen(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);
	void prio_zoom_transpen(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);
	void prio_zoom_transpen_raw(bitmap_ind16 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);
	void prio_zoom_transpen_raw(bitmap_rgb32 &dest, const rectangle &cliprect, u32 code, u32 color, int flipx, int flipy, s32 destx, s32 desty, u32 tx, u32 ty, u32 srcwidth, u32 srcheight, u32 incx, u32 incy, bitmap_ind8 &priority, u32 pmask, u32 trans_pen);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

private:
	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	// configurations
	u16 m_color_base, m_color_entries;
	bool m_has_zoom;
	bool m_has_yuv;
};

DECLARE_DEVICE_TYPE(JALECO_MEGASYSTEM32_SPRITE, ms32_sprite_device)

#endif  // MAME_JALECO_MS32_SPRITE_H
