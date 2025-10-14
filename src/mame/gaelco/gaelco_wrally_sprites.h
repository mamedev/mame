// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Mike Coates, Nicola Salmoria, Miguel Angel Horna, Luca Elia, David Haywood
#ifndef MAME_GAELCO_GAELCO_WRALLY_SPRITES_H
#define MAME_GAELCO_GAELCO_WRALLY_SPRITES_H

#pragma once

#include "screen.h"

class gaelco_wrally_sprites_device : public device_t, public device_video_interface
{
public:
	gaelco_wrally_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }

	void draw_sprites(const rectangle &cliprect, uint16_t* spriteram, int flip_screen);
	void mix_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority);

protected:
	gaelco_wrally_sprites_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void get_sprites_info(uint16_t* spriteram, int& sx, int& sy, int& number, int& color, int& color_effect, int& attr, int& high_priotiy, int &end);

private:
	required_device<gfxdecode_device> m_gfxdecode;

	bitmap_ind16 m_temp_bitmap_sprites;
};

class blmbycar_sprites_device : public gaelco_wrally_sprites_device
{
public:
	blmbycar_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void get_sprites_info(uint16_t* spriteram, int& sx, int& sy, int& number, int& color, int& color_effect, int& attr, int& high_priotiy, int &end) override;
};

DECLARE_DEVICE_TYPE(GAELCO_WRALLY_SPRITES, gaelco_wrally_sprites_device)
DECLARE_DEVICE_TYPE(BLMBYCAR_SPRITES, blmbycar_sprites_device)

#endif // MAME_GAELCO_GAELCO_WRALLY_SPRITES_H
