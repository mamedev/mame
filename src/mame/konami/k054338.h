// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_KONAMI_K054338_H
#define MAME_KONAMI_K054338_H

#pragma once

#include "k055555.h"
#include "emupal.h"

#define K338_REG_BGC_R      0
#define K338_REG_BGC_GB     1
#define K338_REG_SHAD1R     2
#define K338_REG_BRI3       11
#define K338_REG_PBLEND     13
#define K338_REG_CONTROL    15

#define K338_CTL_KILL       0x01    /* 0 = no video output, 1 = enable */
#define K338_CTL_MIXPRI     0x02
#define K338_CTL_SHDPRI     0x04
#define K338_CTL_BRTPRI     0x08
#define K338_CTL_WAILSL     0x10
#define K338_CTL_CLIPSL     0x20


class k054338_device : public device_t,
						public device_video_interface
{
public:
	template <typename T> k054338_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&mixer_tag)
		: k054338_device(mconfig, tag, owner, clock)
	{
		m_k055555.set_tag(std::forward<T>(mixer_tag));
	}

	k054338_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_alpha_invert(int alpha_inv) { m_alpha_inv = alpha_inv; }

	void word_w(offs_t offset, u16 data, u16 mem_mask = ~0); // "CLCT" registers

	u16 register_r(offs_t offset);
	void update_all_shadows(int rushingheroes_hack, palette_device &palette);          // called at the beginning of SCREEN_UPDATE()
	void fill_solid_bg(bitmap_rgb32 &bitmap, const rectangle &cliprect);             // solid backcolor fill
	void fill_backcolor(bitmap_rgb32 &bitmap, const rectangle &cliprect, const pen_t *pal_ptr, int mode);  // solid or gradient fill using k055555
	int  set_alpha_level(int pblend);                         // blend style 0-2
	void invert_alpha(int invert);                                // 0=0x00(invis)-0x1f(solid), 1=0x1f(invis)-0x00(solod)
	void export_config(int **shdRGB);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	uint16_t      m_regs[32];
	int         m_shd_rgb[9];
	int         m_alpha_inv;

	optional_device<k055555_device> m_k055555;  /* used to fill BG color */
};

DECLARE_DEVICE_TYPE(K054338, k054338_device)

#endif // MAME_KONAMI_K054338_H
