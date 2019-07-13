// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_K053936_H
#define MAME_VIDEO_K053936_H

#pragma once

#include "emupal.h"


void K053936_0_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,uint32_t priority, int glfgreat_hack);
void K053936_wraparound_enable(int chip, int status);
void K053936_set_offset(int chip, int xoffs, int yoffs);

// GX specific implementations...
void K053936GP_set_offset(int chip, int xoffs, int yoffs);
void K053936GP_clip_enable(int chip, int status);
void K053936GP_set_cliprect(int chip, int minx, int maxx, int miny, int maxy);
void K053936GP_0_zoom_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tmap, int tilebpp, int blend, int alpha, int pixeldouble_output, uint16_t* temp_m_k053936_0_ctrl_16, uint16_t* temp_m_k053936_0_linectrl_16, uint16_t* temp_m_k053936_0_ctrl, uint16_t* temp_m_k053936_0_linectrl, palette_device &palette);


class k053936_device : public device_t
{
public:
	k053936_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k053936_device() {}

	// configuration
	void set_wrap(int wrap) { m_wrap = wrap; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_xoff = x_offset;
		m_yoff = y_offset;
	}

	DECLARE_WRITE16_MEMBER( ctrl_w );
	DECLARE_READ16_MEMBER( ctrl_r );
	DECLARE_WRITE16_MEMBER( linectrl_w );
	DECLARE_READ16_MEMBER( linectrl_r );
	void zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tmap, int flags, uint32_t priority, int glfgreat_hack);
	void zoom_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tmap, int flags, uint32_t priority, int glfgreat_hack);
	// void wraparound_enable(int status);   unused? // shall we merge this into the configuration intf?
	// void set_offset(int xoffs, int yoffs); unused?   // shall we merge this into the configuration intf?

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	std::unique_ptr<uint16_t[]>    m_ctrl;
	std::unique_ptr<uint16_t[]>    m_linectrl;
	int       m_wrap, m_xoff, m_yoff;
};

DECLARE_DEVICE_TYPE(K053936, k053936_device)

#endif // MAME_VIDEO_K053936_H
