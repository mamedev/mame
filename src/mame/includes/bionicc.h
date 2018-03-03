// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Paul Leaman
// thanks-to: Steven Frew (the author of Slutte)
/***************************************************************************

    Bionic Commando

***************************************************************************/

#include "machine/gen_latch.h"
#include "machine/timer.h"
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
		m_spritegen(*this, "spritegen"),
		m_soundlatch(*this, "soundlatch")
	{ }

	/* memory pointers */
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_txvideoram;
	required_shared_ptr<uint16_t> m_fgvideoram;
	required_shared_ptr<uint16_t> m_bgvideoram;

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	uint16_t    m_scroll[4];

	uint16_t    m_inp[3];
	uint16_t    m_soundcommand;

	DECLARE_WRITE16_MEMBER(hacked_controls_w);
	DECLARE_READ16_MEMBER(hacked_controls_r);
	DECLARE_WRITE16_MEMBER(mpu_trigger_w);
	DECLARE_WRITE16_MEMBER(hacked_soundcommand_w);
	DECLARE_READ16_MEMBER(hacked_soundcommand_r);
	DECLARE_WRITE16_MEMBER(bgvideoram_w);
	DECLARE_WRITE16_MEMBER(fgvideoram_w);
	DECLARE_WRITE16_MEMBER(txvideoram_w);
	DECLARE_WRITE16_MEMBER(scroll_w);
	DECLARE_WRITE16_MEMBER(gfxctrl_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_DECODER(RRRRGGGGBBBBIIII);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tigeroad_spr_device> m_spritegen;
	required_device<generic_latch_8_device> m_soundlatch;
	void bionicc(machine_config &config);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};
