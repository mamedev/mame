// license:???
// copyright-holders:Jarek Burczynski
/*************************************************************************

    Rastan

*************************************************************************/
#include "sound/msm5205.h"
#include "video/pc080sn.h"
#include "video/pc090oj.h"

class rastan_state : public driver_device
{
public:
	rastan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_pc080sn(*this, "pc080sn"),
		m_pc090oj(*this, "pc090oj") { }

	/* video-related */
	UINT16      m_sprite_ctrl;
	UINT16      m_sprites_flipscreen;

	/* misc */
	int         m_adpcm_pos;
	int         m_adpcm_data;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<pc080sn_device> m_pc080sn;
	required_device<pc090oj_device> m_pc090oj;
	DECLARE_WRITE8_MEMBER(rastan_msm5205_address_w);
	DECLARE_WRITE16_MEMBER(rastan_spritectrl_w);
	DECLARE_WRITE8_MEMBER(rastan_bankswitch_w);
	DECLARE_WRITE8_MEMBER(rastan_msm5205_start_w);
	DECLARE_WRITE8_MEMBER(rastan_msm5205_stop_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_rastan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(rastan_msm5205_vck);
};
