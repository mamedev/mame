
#include "sound/sn76496.h"

class hyperspt_state : public driver_device
{
public:
	hyperspt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_sn(*this, "snsnd")
	{ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	optional_device<sn76496_new_device> m_sn;
	UINT8 *  m_scroll2;
	UINT8 *  m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int		 m_sprites_gfx_banked;

	UINT8    m_irq_mask;
	DECLARE_WRITE8_MEMBER(hyperspt_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(hyperspt_videoram_w);
	DECLARE_WRITE8_MEMBER(hyperspt_colorram_w);
	DECLARE_WRITE8_MEMBER(hyperspt_flipscreen_w);
	DECLARE_DRIVER_INIT(hyperspt);

	UINT8 m_SN76496_latch;
	DECLARE_WRITE8_MEMBER( konami_SN76496_latch_w ) { m_SN76496_latch = data; };
	DECLARE_WRITE8_MEMBER( konami_SN76496_w ) { m_sn->write(space, offset, m_SN76496_latch); };
};

/*----------- defined in video/hyperspt.c -----------*/


PALETTE_INIT( hyperspt );
VIDEO_START( hyperspt );
SCREEN_UPDATE_IND16( hyperspt );
VIDEO_START( roadf );

