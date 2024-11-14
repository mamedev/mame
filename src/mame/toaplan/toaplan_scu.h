// license:BSD-3-Clause
// copyright-holders:Quench
/* toaplan SCU */
#ifndef MAME_TOAPLAN_TOAPLAN_SCU_H
#define MAME_TOAPLAN_TOAPLAN_SCU_H

#pragma once

class toaplan_scu_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	toaplan_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	typedef device_delegate<void (u8 priority, u32 &pri_mask)> pri_cb_delegate;

	// configuration
	void set_xoffsets(int xoffs, int xoffs_flipped)
	{
		m_xoffs = xoffs;
		m_xoffs_flipped = xoffs_flipped;
	}
	template <typename... T> void set_pri_callback(T &&... args) { m_pri_cb.set(std::forward<T>(args)...); }

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, u16* spriteram, u32 bytes);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, u16* spriteram, u32 bytes);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	template<class BitmapClass> void draw_sprites_common(BitmapClass &bitmap, const rectangle &cliprect, u16* spriteram, u32 bytes);

	static const gfx_layout spritelayout;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	pri_cb_delegate m_pri_cb;
	int m_xoffs = 0;
	int m_xoffs_flipped = 0;
};

DECLARE_DEVICE_TYPE(TOAPLAN_SCU, toaplan_scu_device)


#endif // MAME_TOAPLAN_TOAPLAN_SCU_H
