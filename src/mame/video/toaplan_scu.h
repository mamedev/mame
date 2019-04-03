// license:BSD-3-Clause
// copyright-holders:Quench
/* toaplan SCU */
#ifndef MAME_VIDEO_TOAPLAN_SCU_H
#define MAME_VIDEO_TOAPLAN_SCU_H

#pragma once


class toaplan_scu_device : public device_t, public device_gfx_interface
{
public:
	toaplan_scu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_xoffsets(int xoffs, int xoffs_flipped)
	{
		m_xoffs = xoffs;
		m_xoffs_flipped = xoffs_flipped;
	}

	void draw_sprites_to_tempbitmap(const rectangle &cliprect, u16* spriteram, u32 bytes);
	void copy_sprites_from_tempbitmap(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);
	void alloc_sprite_bitmap(screen_device &screen);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static const gfx_layout spritelayout;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	bitmap_ind16 m_temp_spritebitmap;
	int m_xoffs;
	int m_xoffs_flipped;
};

DECLARE_DEVICE_TYPE(TOAPLAN_SCU, toaplan_scu_device)


#endif // MAME_VIDEO_TOAPLAN_SCU_H
