// license:BSD-3-Clause
// copyright-holders:Dan Boris
/*************************************************************************

    Hitme hardware

*************************************************************************/
#ifndef MAME_INCLUDES_HITME_H
#define MAME_INCLUDES_HITME_H

#pragma once

#include "sound/discrete.h"
#include "screen.h"

/* Discrete Sound Input Nodes */
#define HITME_DOWNCOUNT_VAL      NODE_01
#define HITME_OUT0               NODE_02
#define HITME_ENABLE_VAL         NODE_03
#define HITME_OUT1               NODE_04

class hitme_state : public driver_device
{
public:
	hitme_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen")
	{ }

	void hitme(machine_config &config);
	void barricad(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;

	/* video-related */
	tilemap_t  *m_tilemap;

	/* misc */
	attotime m_timeout_time;
	DECLARE_WRITE8_MEMBER(hitme_vidram_w);
	DECLARE_READ8_MEMBER(hitme_port_0_r);
	DECLARE_READ8_MEMBER(hitme_port_1_r);
	DECLARE_READ8_MEMBER(hitme_port_2_r);
	DECLARE_READ8_MEMBER(hitme_port_3_r);
	DECLARE_WRITE8_MEMBER(output_port_0_w);
	DECLARE_WRITE8_MEMBER(output_port_1_w);
	TILE_GET_INFO_MEMBER(get_hitme_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(barricad);
	uint32_t screen_update_hitme(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_barricad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint8_t read_port_and_t0( int port );
	uint8_t read_port_and_t0_and_hblank( int port );
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	void hitme_map(address_map &map);
	void hitme_portmap(address_map &map);
};


/*----------- defined in audio/hitme.c -----------*/
DISCRETE_SOUND_EXTERN( hitme_discrete );

#endif // MAME_INCLUDES_HITME_H
