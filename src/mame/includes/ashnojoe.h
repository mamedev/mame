// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Success Joe / Ashita no Joe

*************************************************************************/
#include "sound/msm5205.h"

class ashnojoe_state : public driver_device
{
public:
	ashnojoe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tileram_3(*this, "tileram_3"),
		m_tileram_4(*this, "tileram_4"),
		m_tileram_5(*this, "tileram_5"),
		m_tileram_2(*this, "tileram_2"),
		m_tileram_6(*this, "tileram_6"),
		m_tileram_7(*this, "tileram_7"),
		m_tileram(*this, "tileram"),
		m_tilemap_reg(*this, "tilemap_reg"),
		m_audiocpu(*this, "audiocpu"),
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	UINT16 *    m_tileram_1;
	required_shared_ptr<UINT16> m_tileram_3;
	required_shared_ptr<UINT16> m_tileram_4;
	required_shared_ptr<UINT16> m_tileram_5;
	required_shared_ptr<UINT16> m_tileram_2;
	required_shared_ptr<UINT16> m_tileram_6;
	required_shared_ptr<UINT16> m_tileram_7;
	required_shared_ptr<UINT16> m_tileram;
	required_shared_ptr<UINT16> m_tilemap_reg;

	/* video-related */
	tilemap_t     *m_joetilemap;
	tilemap_t     *m_joetilemap2;
	tilemap_t     *m_joetilemap3;
	tilemap_t     *m_joetilemap4;
	tilemap_t     *m_joetilemap5;
	tilemap_t     *m_joetilemap6;
	tilemap_t     *m_joetilemap7;

	/* sound-related */
	UINT8       m_adpcm_byte;
	int         m_soundlatch_status;
	int         m_msm5205_vclk_toggle;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	DECLARE_READ16_MEMBER(fake_4a00a_r);
	DECLARE_WRITE16_MEMBER(ashnojoe_soundlatch_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);
	DECLARE_READ8_MEMBER(sound_latch_r);
	DECLARE_READ8_MEMBER(sound_latch_status_r);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram2_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram3_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram4_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram5_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram6_w);
	DECLARE_WRITE16_MEMBER(ashnojoe_tileram7_w);
	DECLARE_WRITE16_MEMBER(joe_tilemaps_xscroll_w);
	DECLARE_WRITE16_MEMBER(joe_tilemaps_yscroll_w);
	DECLARE_WRITE8_MEMBER(ym2203_write_a);
	DECLARE_WRITE8_MEMBER(ym2203_write_b);
	DECLARE_DRIVER_INIT(ashnojoe);
	TILE_GET_INFO_MEMBER(get_joe_tile_info);
	TILE_GET_INFO_MEMBER(get_joe_tile_info_2);
	TILE_GET_INFO_MEMBER(get_joe_tile_info_3);
	TILE_GET_INFO_MEMBER(get_joe_tile_info_4);
	TILE_GET_INFO_MEMBER(get_joe_tile_info_5);
	TILE_GET_INFO_MEMBER(get_joe_tile_info_6);
	TILE_GET_INFO_MEMBER(get_joe_tile_info_7);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_ashnojoe(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(ym2203_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(ashnojoe_vclk_cb);
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
};
