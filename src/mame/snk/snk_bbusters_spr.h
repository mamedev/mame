// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_SNK_SNK_BBUSTERS_SPR_H
#define MAME_SNK_SNK_BBUSTERS_SPR_H

#pragma once

#include "video/bufsprite.h"

class snk_bbusters_spr_device : public device_t, public device_gfx_interface
{
public:
	snk_bbusters_spr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_spriteram_tag(T &&tag) { m_spriteram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_scaletable_tag(T &&tag) { m_scale_table.set_tag(std::forward<T>(tag)); }

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	const uint8_t *get_source_ptr(gfx_element *tilegfx, uint32_t sprite, int dx, int dy, int block);
	void draw_block(bitmap_ind16 &dest, const rectangle &cliprect, int x, int y, int size, int flipx, int flipy, uint32_t sprite, int color, int block);

	template <int Size>
	static int adjust_spritecode(int dx, int dy, int code);

	const uint8_t *m_scale_table_ptr = nullptr;
	uint8_t m_scale_line_count = 0;

	required_region_ptr<uint8_t> m_scale_table;
	required_device<buffered_spriteram16_device> m_spriteram;
};

DECLARE_DEVICE_TYPE(SNK_BBUSTERS_SPR, snk_bbusters_spr_device)

#endif // MAME_SNK_SNK_BBUSTERS_SPR_H
