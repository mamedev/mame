// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*************************************************************************

    Hana Yayoi & other Dynax games (using 1st version of their blitter)

*************************************************************************/
#include "sound/msm5205.h"

class hnayayoi_state : public driver_device
{
public:
	hnayayoi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_msm(*this, "msm") { }

	/* video-related */
	UINT8      *m_pixmap[8];
	int        m_palbank;
	int        m_total_pixmaps;
	UINT8      m_blit_layer;
	UINT16     m_blit_dest;
	UINT32     m_blit_src;

	/* misc */
	int        m_keyb;
	DECLARE_READ8_MEMBER(keyboard_0_r);
	DECLARE_READ8_MEMBER(keyboard_1_r);
	DECLARE_WRITE8_MEMBER(keyboard_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev1_param_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev1_start_w);
	DECLARE_WRITE8_MEMBER(dynax_blitter_rev1_clear_w);
	DECLARE_WRITE8_MEMBER(hnayayoi_palbank_w);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_WRITE8_MEMBER(adpcm_vclk_w);
	DECLARE_WRITE8_MEMBER(adpcm_reset_w);
	DECLARE_WRITE8_MEMBER(adpcm_reset_inv_w);
	DECLARE_DRIVER_INIT(hnfubuki);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(untoucha);
	UINT32 screen_update_hnayayoi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void common_vh_start( int num_pixmaps );
	void copy_pixel( int x, int y, int pen );
	void draw_layer_interleaved( bitmap_ind16 &bitmap, const rectangle &cliprect, int left_pixmap, int right_pixmap, int palbase, int transp );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
};
