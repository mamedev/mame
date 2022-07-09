// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_VIDEO_K007121_H
#define MAME_VIDEO_K007121_H

#pragma once

#include "emupal.h"


class k007121_device : public device_t
{
public:
	k007121_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }

	uint8_t ctrlram_r(offs_t offset);
	void ctrl_w(offs_t offset, uint8_t data);

	/* shall we move source in the interface? */
	/* also notice that now we directly pass *gfx[chip] instead of **gfx !! */
	void sprites_draw( bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx, device_palette_interface &palette, const uint8_t *source, int base_color, int global_x_offset, int bank_base, bitmap_ind8 &priority_bitmap, uint32_t pri_mask, bool is_flakatck = false );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint8_t    m_ctrlram[8];
	int      m_flipscreen;
	required_device<palette_device> m_palette;
};

DECLARE_DEVICE_TYPE(K007121, k007121_device)

#endif // MAME_VIDEO_K007121_H
