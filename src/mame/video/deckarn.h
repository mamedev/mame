// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,David Haywood
#ifndef MAME_VIDEO_DECKARN_H
#define MAME_VIDEO_DECKARN_H

#pragma once


class deco_karnovsprites_device : public device_t
{
public:
	deco_karnovsprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	void set_gfxregion(int region) { m_gfxregion = region; }
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t* spriteram, int size, int priority);
	void set_flip_screen(bool flip) { m_flip_screen = flip; }

	// configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfx_region(int region) { m_gfxregion = region; }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_gfxregion;
	bool m_flip_screen;
private:
	required_device<gfxdecode_device> m_gfxdecode;
};

DECLARE_DEVICE_TYPE(DECO_KARNOVSPRITES, deco_karnovsprites_device)

#endif // MAME_VIDEO_DECKARN_H
