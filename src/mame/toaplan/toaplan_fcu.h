// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************

Toaplan 'FCU' sprite generator

***************************************************************************/
#ifndef MAME_TOAPLAN_TOAPLAN_FCU_H
#define MAME_TOAPLAN_TOAPLAN_FCU_H

#pragma once

class toaplan_fcu_device : public device_t, public device_gfx_interface
{
public:
	using pri_cb_delegate = device_delegate<void (u8 priority, u32 &pri_mask)>;

	toaplan_fcu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> toaplan_fcu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: toaplan_fcu_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename T> void set_spriteram_tag(T &&tag) { m_spriteram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_spritesizeram_tag(T &&tag) { m_spritesizeram.set_tag(std::forward<T>(tag)); }
	void set_colorbase(u16 base)
	{
		m_colbase = base;
	}
	template <typename... T> void set_pri_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }
	auto frame_done_cb() { return m_frame_done_cb.bind(); }

	// host interfaces
	u16 frame_done_r();
	void flipscreen_w(u8 data);
	u16 spriteram_offs_r();
	void spriteram_offs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 spriteram_r();
	void spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 spritesizeram_r();
	void spritesizeram_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank(int state);

	void host_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template<class BitmapClass> void draw_sprites_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect);

	// memory pointers
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_spritesizeram;

	std::unique_ptr<u16[]> m_buffered_spriteram;
	std::unique_ptr<u16[]> m_buffered_spritesizeram;

	// configurations
	pri_cb_delegate m_pri_cb;
	devcb_read_line m_frame_done_cb;
	u16 m_colbase;

	// internal states
	bool m_flipscreen;
	s32 m_spriteram_offs;
};

DECLARE_DEVICE_TYPE(TOAPLAN_FCU, toaplan_fcu_device)


#endif // MAME_TOAPLAN_TOAPLAN_FCU_H
