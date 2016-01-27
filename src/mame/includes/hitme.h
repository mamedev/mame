// license:BSD-3-Clause
// copyright-holders:Dan Boris
/*************************************************************************

    Hitme hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define HITME_DOWNCOUNT_VAL      NODE_01
#define HITME_OUT0               NODE_02
#define HITME_ENABLE_VAL         NODE_03
#define HITME_OUT1               NODE_04

class hitme_state : public driver_device
{
public:
	hitme_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;

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
	UINT32 screen_update_hitme(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_barricad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT8 read_port_and_t0( int port );
	UINT8 read_port_and_t0_and_hblank( int port );
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
};


/*----------- defined in audio/hitme.c -----------*/
DISCRETE_SOUND_EXTERN( hitme );
