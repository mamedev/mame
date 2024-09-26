// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria
#ifndef MAME_SHARED_SNK68_SPR_H
#define MAME_SHARED_SNK68_SPR_H

#pragma once

#include "screen.h"


class snk68_spr_device : public device_t
{
public:
	typedef device_delegate<void (int &, int &, int &, int &)> tile_indirection_delegate;

	snk68_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_tile_indirect_cb(T &&... args) { m_newtilecb.set(std::forward<T>(args)...); }
	void set_no_partial() { m_partialupdates = 0; }
	void set_xpos_shift(u8 data) { m_xpos_shift = data; }
	void set_color_entry_mask(u16 data) { m_color_entry_mask = data; }

	u16 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int group, u16 start_offset, u16 end_offset);
	void draw_sprites_all(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites_alt(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void tile_callback_noindirect(int& tile, int& fx, int& fy, int& region);
	void set_flip(bool flip);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	tile_indirection_delegate m_newtilecb;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device<screen_device> m_screen;
	bool m_flipscreen;
	int m_partialupdates; // the original hardware needs this, the cloned hardware does not.
	u8  m_xpos_shift = 0;
	u16 m_color_entry_mask = 0;
};


DECLARE_DEVICE_TYPE(SNK68_SPR, snk68_spr_device)

#endif // MAME_SHARED_SNK68_SPR_H
