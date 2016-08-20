// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Yun Sung 8 Bit Games

*************************************************************************/

#include "sound/msm5205.h"

class yunsung8_state : public driver_device
{
public:
	yunsung8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu") ,
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* video-related */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;
	UINT8       *m_videoram_0;
	UINT8       *m_videoram_1;
	int         m_layers_ctrl;
	int         m_videobank;

	/* misc */
	int         m_adpcm;
	int         m_toggle;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory */
	UINT8      m_videoram[0x4000];

	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(adpcm_w);
	DECLARE_WRITE8_MEMBER(videobank_w);
	DECLARE_READ8_MEMBER(videoram_r);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
