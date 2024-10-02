// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_CAPCOM_TIGEROAD_SPR_H
#define MAME_CAPCOM_TIGEROAD_SPR_H

#pragma once

class tigeroad_spr_device : public device_t, public device_gfx_interface
{
public:
	tigeroad_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configurations
	void set_color_base(u16 base) { m_colbase = base; }

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, u16* ram, u32 size, bool flip_screen, bool rev_y);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_memory_region m_gfx_region;

	// internal states
	u16 m_colbase;
};

DECLARE_DEVICE_TYPE(TIGEROAD_SPRITE, tigeroad_spr_device)

#endif // MAME_CAPCOM_TIGEROAD_SPR_H
