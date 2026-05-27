// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_VIDEO_KY3211_KY10510_H
#define MAME_VIDEO_KY3211_KY10510_H

#pragma once

class ky3211_device : public device_t, public device_gfx_interface
{
public:
	ky3211_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> ky3211_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag)
		: ky3211_device(mconfig, tag, owner, clock)
	{
		set_palette(std::forward<T>(palette_tag));
	}

	void vregs_w(offs_t offset, u8 data);
	u8 vregs_r(offs_t offset);
	void vtable_w(offs_t offset, u8 data);
	u8 vtable_r(offs_t offset);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, const u8 *spriteram, int pri_mask);

protected:
	ky3211_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

	virtual bool get_attr(const u8 *src, u32 &color, u8 &gfx, bool &flipx, bool &flipy);

	void save_states();

private:
	memory_share_creator<u8> m_vregs;
	memory_share_creator<u8> m_vtable;
	bitmap_ind16 m_sprite_bitmap;

	DECLARE_GFXDECODE_MEMBER(gfx_ky3211);
};

class ky10510_device : public ky3211_device
{
public:
	ky10510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> ky10510_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag)
		: ky10510_device(mconfig, tag, owner, clock)
	{
		set_palette(std::forward<T>(palette_tag));
	}

protected:
	virtual void device_start() override ATTR_COLD;

	virtual bool get_attr(const u8 *src, u32 &color, u8 &gfx, bool &flipx, bool &flipy) override;

private:
	DECLARE_GFXDECODE_MEMBER(gfx_ky10510);
};

DECLARE_DEVICE_TYPE(KY3211,  ky3211_device)
DECLARE_DEVICE_TYPE(KY10510, ky10510_device)

#endif // MAME_VIDEO_KY3211_KY10510_H
