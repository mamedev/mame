// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*****************************************************************************
 *
 * includes/banctec.h
 *
 ****************************************************************************/

#ifndef BANCTEC_H_
#define BANCTEC_H_

#include "video/mc6845.h"

class banctec_state : public driver_device
{
public:
	banctec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram") { }

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_banctec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<UINT8> m_videoram;

	tilemap_t *m_bg_tilemap;
};

#endif /* BANCTEC_H_ */
