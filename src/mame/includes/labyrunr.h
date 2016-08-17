// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Labyrinth Runner

*************************************************************************/

#include "video/k007121.h"
#include "video/k051733.h"

class labyrunr_state : public driver_device
{
public:
	labyrunr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_k007121(*this, "k007121"),
		m_maincpu(*this,"maincpu"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* devices */
	required_device<k007121_device> m_k007121;

	required_device<cpu_device> m_maincpu;
	/* memory pointers */
	required_shared_ptr<UINT8> m_scrollram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram1;
	required_shared_ptr<UINT8> m_videoram2;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t    *m_layer0;
	tilemap_t    *m_layer1;
	rectangle  m_clip0;
	rectangle  m_clip1;

	DECLARE_WRITE8_MEMBER(labyrunr_bankswitch_w);
	DECLARE_WRITE8_MEMBER(labyrunr_vram1_w);
	DECLARE_WRITE8_MEMBER(labyrunr_vram2_w);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(labyrunr);
	UINT32 screen_update_labyrunr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(labyrunr_vblank_interrupt);
	INTERRUPT_GEN_MEMBER(labyrunr_timer_interrupt);
};
