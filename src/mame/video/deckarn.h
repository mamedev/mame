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
	void set_gfxdecode_tag(const char *tag) { m_gfxdecode.set_tag(tag); }
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

#define MCFG_DECO_KARNOVSPRITES_GFXDECODE(_gfxtag) \
	downcast<deco_karnovsprites_device &>(*device).set_gfxdecode_tag(_gfxtag);

#define MCFG_DECO_KARNOVSPRITES_GFX_REGION(_region) \
	downcast<deco_karnovsprites_device &>(*device).set_gfx_region(_region);

#endif // MAME_VIDEO_DECKARN_H
