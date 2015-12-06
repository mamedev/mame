// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "video/tecmo_spr.h"

class wc90_state : public driver_device
{
public:
	wc90_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprgen(*this, "spritegen"),
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"),
		m_txvideoram(*this, "txvideoram"),
		m_scroll0xlo(*this, "scroll0xlo"),
		m_scroll0xhi(*this, "scroll0xhi"),
		m_scroll1xlo(*this, "scroll1xlo"),
		m_scroll1xhi(*this, "scroll1xhi"),
		m_scroll2xlo(*this, "scroll2xlo"),
		m_scroll2xhi(*this, "scroll2xhi"),
		m_scroll0ylo(*this, "scroll0ylo"),
		m_scroll0yhi(*this, "scroll0yhi"),
		m_scroll1ylo(*this, "scroll1ylo"),
		m_scroll1yhi(*this, "scroll1yhi"),
		m_scroll2ylo(*this, "scroll2ylo"),
		m_scroll2yhi(*this, "scroll2yhi"),
		m_spriteram(*this, "spriteram")
	{ }


	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tecmo_spr_device> m_sprgen;

	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_txvideoram;
	required_shared_ptr<UINT8> m_scroll0xlo;
	required_shared_ptr<UINT8> m_scroll0xhi;
	required_shared_ptr<UINT8> m_scroll1xlo;
	required_shared_ptr<UINT8> m_scroll1xhi;
	required_shared_ptr<UINT8> m_scroll2xlo;
	required_shared_ptr<UINT8> m_scroll2xhi;
	required_shared_ptr<UINT8> m_scroll0ylo;
	required_shared_ptr<UINT8> m_scroll0yhi;
	required_shared_ptr<UINT8> m_scroll1ylo;
	required_shared_ptr<UINT8> m_scroll1yhi;
	required_shared_ptr<UINT8> m_scroll2ylo;
	required_shared_ptr<UINT8> m_scroll2yhi;
	required_shared_ptr<UINT8> m_spriteram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(bankswitch1_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(bgvideoram_w);
	DECLARE_WRITE8_MEMBER(fgvideoram_w);
	DECLARE_WRITE8_MEMBER(txvideoram_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(track_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(track_get_fg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(wc90t);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
