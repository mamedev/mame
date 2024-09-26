// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Tecmo Mixer */
#ifndef MAME_TECMO_TECMO_MIX_H
#define MAME_TECMO_TECMO_MIX_H

#pragma once

#include "emupal.h"


class tecmo_mix_device : public device_t, public device_video_interface
{
public:
	tecmo_mix_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mix_bitmaps(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device &palette, bitmap_ind16* bitmap_bg, bitmap_ind16* bitmap_fg, bitmap_ind16* bitmap_tx, bitmap_ind16* bitmap_sp);
	void set_mixer_shifts(int sprpri_shift, int sprbln_shift, int sprcol_shift)
	{
		m_sprpri_shift = sprpri_shift;
		m_sprbln_shift = sprbln_shift;
		m_sprcol_shift = sprcol_shift;
	}
	void set_blendcols(int bgblend_comp, int fgblend_comp, int txblend_comp, int spblend_comp)
	{
		m_bgblend_comp = bgblend_comp;
		m_fgblend_comp = fgblend_comp;
		m_txblend_comp = txblend_comp;
		m_spblend_comp = spblend_comp;
	}
	void set_regularcols(int bgregular_comp, int fgregular_comp, int txregular_comp, int spregular_comp)
	{
		m_bgregular_comp = bgregular_comp;
		m_fgregular_comp = fgregular_comp;
		m_txregular_comp = txregular_comp;
		m_spregular_comp = spregular_comp;
	}
	void set_blendsource(int spblend_source, int fgblend_source)
	{
		m_spblend_source = spblend_source;
		m_fgblend_source = fgblend_source;
	}
	void set_bgpen(int bgpen, int bgpen_blend)
	{
		m_bgpen       = bgpen;
		m_bgpen_blend = bgpen_blend;
	}
	void set_revspritetile() { m_revspritetile = 3; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// mixer shifts
	int m_sprpri_shift;
	int m_sprbln_shift;
	int m_sprcol_shift;

	// when the blend bit is specified in the attribute the source blend palette for that sprite / fg pixel comes from these offsets instead
	int m_spblend_source;
	int m_fgblend_source;
	// the second blend component depends on the pixel we are blending with, the following palettes get used instead of the regular ones
	int m_bgblend_comp;
	int m_fgblend_comp;
	int m_txblend_comp;
	int m_spblend_comp;

	// otherwise the regular palettes are
	int m_bgregular_comp;
	int m_fgregular_comp;
	int m_txregular_comp;
	int m_spregular_comp;

	int m_bgpen;
	int m_bgpen_blend;

	int m_revspritetile;

private:
	uint32_t sum_colors(const pen_t *pal, int c1_idx, int c2_idx);
};

DECLARE_DEVICE_TYPE(TECMO_MIXER, tecmo_mix_device)



#endif // MAME_TECMO_TECMO_MIX_H
