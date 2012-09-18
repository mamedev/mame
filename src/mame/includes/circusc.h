/*************************************************************************

    Circus Charlie

*************************************************************************/

#include "sound/dac.h"
#include "sound/sn76496.h"

class circusc_state : public driver_device
{
public:
	circusc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spritebank(*this, "spritebank"),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_spriteram(*this, "spriteram"),
		m_sn_1(*this, "sn1"),
		m_sn_2(*this, "sn2"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spritebank;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram_2;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t        *m_bg_tilemap;

	/* sound-related */
	UINT8          m_sn_latch;

	/* devices */
	cpu_device *m_audiocpu;
	required_device<sn76496_new_device> m_sn_1;
	required_device<sn76496_new_device> m_sn_2;
	dac_device *m_dac;
	device_t *m_discrete;

	UINT8          m_irq_mask;
	DECLARE_READ8_MEMBER(circusc_sh_timer_r);
	DECLARE_WRITE8_MEMBER(circusc_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(circusc_coin_counter_w);
	DECLARE_WRITE8_MEMBER(circusc_sound_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(circusc_videoram_w);
	DECLARE_WRITE8_MEMBER(circusc_colorram_w);
	DECLARE_WRITE8_MEMBER(circusc_flipscreen_w);
	DECLARE_DRIVER_INIT(circusc);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_circusc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
