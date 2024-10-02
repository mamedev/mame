// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
#ifndef MAME_FUUKI_FUUKIFH_H
#define MAME_FUUKI_FUUKIFH_H

#pragma once

class fuukivid_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	typedef device_delegate<void (u32 &code)> tile_delegate;
	typedef device_delegate<void (u32 &colour, u32 &pri_mask)> colpri_cb_delegate;

	fuukivid_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_color_base(u16 base) { m_colbase = base; }
	void set_color_num(u16 num) { m_colnum = num; }
	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_colpri_callback(T &&... args) { m_colpri_cb.set(std::forward<T>(args)...); }

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, bool flip_screen, u16 *spriteram, u32 size);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	tile_delegate m_tile_cb;
	colpri_cb_delegate m_colpri_cb;
	required_memory_region m_gfx_region;
	u16 m_colbase;
	u16 m_colnum;
};

DECLARE_DEVICE_TYPE(FUUKI_VIDEO, fuukivid_device)

#endif // MAME_FUUKI_FUUKIFH_H
