// license:BSD-3-Clause
// copyright-holders:Uki
/*************************************************************************

    Dr. Micro

*************************************************************************/
#include "sound/msm5205.h"

class drmicro_state : public driver_device
{
public:
	drmicro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_msm(*this, "msm"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	std::unique_ptr<UINT8[]>       m_videoram;

	/* video-related */
	tilemap_t        *m_bg1;
	tilemap_t        *m_bg2;
	int            m_flipscreen;

	/* misc */
	int            m_nmi_enable;
	int            m_pcm_adr;

	/* devices */
	required_device<msm5205_device> m_msm;
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(pcm_set_w);
	DECLARE_WRITE8_MEMBER(drmicro_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(drmicro);
	UINT32 screen_update_drmicro(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(drmicro_interrupt);
	DECLARE_WRITE_LINE_MEMBER(pcm_w);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
