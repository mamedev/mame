// license:???
// copyright-holders:Steven Frew, Phil Stroffolino, Paul Leaman
/***************************************************************************

    Bionic Commando

***************************************************************************/

#include "video/bufsprite.h"
#include "video/tigeroad_spr.h"

class bionicc_state : public driver_device
{
public:
	bionicc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_txvideoram(*this, "txvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritegen(*this, "spritegen")
	{ }

	/* memory pointers */
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_txvideoram;
	required_shared_ptr<UINT16> m_fgvideoram;
	required_shared_ptr<UINT16> m_bgvideoram;

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	UINT16    m_scroll[4];

	UINT16    m_inp[3];
	UINT16    m_soundcommand;

	DECLARE_WRITE16_MEMBER(hacked_controls_w);
	DECLARE_READ16_MEMBER(hacked_controls_r);
	DECLARE_WRITE16_MEMBER(bionicc_mpu_trigger_w);
	DECLARE_WRITE16_MEMBER(hacked_soundcommand_w);
	DECLARE_READ16_MEMBER(hacked_soundcommand_r);
	DECLARE_WRITE16_MEMBER(bionicc_bgvideoram_w);
	DECLARE_WRITE16_MEMBER(bionicc_fgvideoram_w);
	DECLARE_WRITE16_MEMBER(bionicc_txvideoram_w);
	DECLARE_WRITE16_MEMBER(bionicc_scroll_w);
	DECLARE_WRITE16_MEMBER(bionicc_gfxctrl_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_DECODER(RRRRGGGGBBBBIIII);
	UINT32 screen_update_bionicc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(bionicc_scanline);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tigeroad_spr_device> m_spritegen;
};
