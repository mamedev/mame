// license:BSD-3-Clause
// copyright-holders:David Haywood
#pragma once
#ifndef __K054338_H__
#define __K054338_H__

#include "k055555.h"

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
	k054338_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~k054338_device() {}

	// static configuration
	static void set_mixer_tag(device_t &device, const char  *tag) { downcast<k054338_device &>(device).m_k055555_tag = tag; }
	static void set_alpha_invert(device_t &device, int alpha_inv) { downcast<k054338_device &>(device).m_alpha_inv = alpha_inv; }

	DECLARE_WRITE16_MEMBER( word_w ); // "CLCT" registers
	DECLARE_WRITE32_MEMBER( long_w );

	DECLARE_READ16_MEMBER( word_r );        // CLTC

	int register_r(int reg);
	void update_all_shadows(int rushingheroes_hack, palette_device *palette);          // called at the beginning of SCREEN_UPDATE()
	void fill_solid_bg(bitmap_rgb32 &bitmap, const rectangle &cliprect);             // solid backcolor fill
	void fill_backcolor(bitmap_rgb32 &bitmap, const rectangle &cliprect, const pen_t *pal_ptr, int mode);  // solid or gradient fill using k055555
	int  set_alpha_level(int pblend);                         // blend style 0-2
	void invert_alpha(int invert);                                // 0=0x00(invis)-0x1f(solid), 1=0x1f(invis)-0x00(solod)
	void export_config(int **shdRGB);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT16      m_regs[32];
	int         m_shd_rgb[9];
	int         m_alpha_inv;
	const char  *m_k055555_tag;

	k055555_device *m_k055555;  /* used to fill BG color */
};

extern const device_type K054338;


#define MCFG_K054338_MIXER(_tag) \
	k054338_device::set_mixer_tag(*device, _tag);

#define MCFG_K054338_ALPHAINV(_alphainv) \
	k054338_device::set_alpha_invert(*device, _alphainv);

#define MCFG_K054338_SET_SCREEN MCFG_VIDEO_SET_SCREEN

#endif
