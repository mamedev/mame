// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K007121_H
#define MAME_KONAMI_K007121_H

#pragma once

#include "emupal.h"


class k007121_device : public device_t, public device_gfx_interface
{
public:
	using dirtytiles_delegate = device_delegate<void()>;

	k007121_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template<typename T> k007121_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: k007121_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	auto set_flipscreen_cb() { return m_flipscreen_cb.bind(); }
	template <typename... T> void set_dirtytiles_cb(T &&... args) { m_dirtytiles_cb.set(std::forward<T>(args)...); }

	void set_spriteram(uint8_t *spriteram) { m_spriteram = spriteram; }

	bool flipscreen() { return m_flipscreen; }
	uint8_t ctrlram_r(offs_t offset);
	void ctrl_w(offs_t offset, uint8_t data);

	void sprites_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int base_color, int global_x_offset, int bank_base, bitmap_ind8 &priority_bitmap, uint32_t pri_mask);
	void sprites_buffer(int state = 1);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	uint8_t m_ctrlram[8];
	bool m_flipscreen;
	uint8_t *m_spriteram;
	uint8_t m_sprites_buffer[0x800];

	devcb_write_line m_flipscreen_cb;
	dirtytiles_delegate m_dirtytiles_cb;
};

DECLARE_DEVICE_TYPE(K007121, k007121_device)

#endif // MAME_KONAMI_K007121_H
