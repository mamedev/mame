// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
#ifndef MAME_VIDEO_DECMXC06_H
#define MAME_VIDEO_DECMXC06_H

#pragma once


class deco_mxc06_device : public device_t, public device_video_interface
{
public:
	deco_mxc06_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfx_region(int region) { m_gfxregion = region; }
	void set_ram_size(int size) { m_ramsize = size; }

	void set_gfxregion(int region) { m_gfxregion = region; };
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t* spriteram16, int pri_mask, int pri_val, int col_mask);
	void draw_sprites_bootleg(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t* spriteram, int pri_mask, int pri_val, int col_mask);
	void set_pri_type(int type) { m_priority_type = type; }
	void set_flip_screen(bool flip) { m_flip_screen = flip; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_gfxregion;
	int m_priority_type; // just so we can support the existing drivers without converting everything to pdrawgfx just yet
	int m_ramsize;
	bool m_flip_screen;

private:
	required_device<gfxdecode_device> m_gfxdecode;
};

DECLARE_DEVICE_TYPE(DECO_MXC06, deco_mxc06_device)

#endif // MAME_VIDEO_DECMXC06_H
