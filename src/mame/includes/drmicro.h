// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Dr. Micro

*************************************************************************/
#ifndef MAME_INCLUDES_DRMICRO_H
#define MAME_INCLUDES_DRMICRO_H

#pragma once

#include "sound/msm5205.h"
#include "emupal.h"

class drmicro_state : public driver_device
{
public:
	drmicro_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_msm(*this, "msm"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void drmicro(machine_config &config);

private:
	/* memory pointers */
	std::unique_ptr<uint8_t[]>       m_videoram;

	/* video-related */
	tilemap_t        *m_bg1;
	tilemap_t        *m_bg2;
	int            m_flipscreen;

	/* misc */
	int            m_nmi_enable;
	int            m_pcm_adr;

	/* devices */
	required_device<msm5205_device> m_msm;
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(pcm_set_w);
	DECLARE_WRITE8_MEMBER(drmicro_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void drmicro_palette(palette_device &palette) const;
	uint32_t screen_update_drmicro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(drmicro_interrupt);
	DECLARE_WRITE_LINE_MEMBER(pcm_w);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void drmicro_map(address_map &map);
	void io_map(address_map &map);
};

#endif // MAME_INCLUDES_DRMICRO_H
