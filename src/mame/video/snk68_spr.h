// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Acho A. Tang, Nicola Salmoria
#ifndef MAME_VIDEO_SNK68_SPR_H
#define MAME_VIDEO_SNK68_SPR_H

#pragma once

#include "screen.h"

typedef device_delegate<void (int&, int&, int&, int&)> snk68_tile_indirection_delegate;

#define MCFG_SNK68_SPR_GFXDECODE(_gfxtag) \
	downcast<snk68_spr_device &>(*device).set_gfxdecode_tag(_gfxtag);
#define MCFG_SNK68_SPR_SET_TILE_INDIRECT( _class, _method) \
	downcast<snk68_spr_device &>(*device).set_tile_indirect_cb(snk68_tile_indirection_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));
#define MCFG_SNK68_SPR_NO_PARTIAL \
	downcast<snk68_spr_device &>(*device).set_no_partial();


class snk68_spr_device : public device_t
{
public:
	snk68_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration
	void set_gfxdecode_tag(const char *tag) { m_gfxdecode.set_tag(tag); }
	template <typename Object> void set_tile_indirect_cb(Object &&cb) { m_newtilecb = std::forward<Object>(cb); }
	void set_no_partial() { m_partialupdates = 0; }

	DECLARE_READ16_MEMBER(spriteram_r);
	DECLARE_WRITE16_MEMBER(spriteram_w);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int group);
	void draw_sprites_all(bitmap_ind16 &bitmap, const rectangle &cliprect);

	snk68_tile_indirection_delegate m_newtilecb;

	void tile_callback_noindirect(int& tile, int& fx, int& fy, int& region);
	void set_flip(int flip);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device<screen_device> m_screen;
	int m_flipscreen;
	int m_partialupdates; // the original hardware needs this, the cloned hardware does not.
};


DECLARE_DEVICE_TYPE(SNK68_SPR, snk68_spr_device)

#endif // MAME_VIDEO_SNK68_SPR_H
