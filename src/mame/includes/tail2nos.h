// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Tail to Nose / Super Formula

*************************************************************************/

#include "video/k051316.h"

class tail2nos_state : public driver_device
{
public:
	tail2nos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_txvideoram(*this, "txvideoram"),
		m_spriteram(*this, "spriteram"),
		m_zoomram(*this, "k051316"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k051316(*this, "k051316"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* memory pointers */
	required_shared_ptr<UINT16> m_txvideoram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_zoomram;

	/* video-related */
	tilemap_t   *m_tx_tilemap;
	int         m_txbank;
	int         m_txpalette;
	int         m_video_enable;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k051316_device> m_k051316;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE16_MEMBER(tail2nos_txvideoram_w);
	DECLARE_WRITE16_MEMBER(tail2nos_zoomdata_w);
	DECLARE_WRITE16_MEMBER(tail2nos_gfxbank_w);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_tail2nos(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void tail2nos_postload();
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	K051316_CB_MEMBER(zoom_callback);
};
