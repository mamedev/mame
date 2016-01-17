// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*************************************************************************

    Atari Cops'n Robbers hardware

*************************************************************************/

#include "sound/discrete.h"


class copsnrob_state : public driver_device
{
public:
	copsnrob_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_trucky(*this, "trucky"),
		m_truckram(*this, "truckram"),
		m_bulletsram(*this, "bulletsram"),
		m_carimage(*this, "carimage"),
		m_cary(*this, "cary"),
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_trucky;
	required_shared_ptr<UINT8> m_truckram;
	required_shared_ptr<UINT8> m_bulletsram;
	required_shared_ptr<UINT8> m_carimage;
	required_shared_ptr<UINT8> m_cary;
	required_shared_ptr<UINT8> m_videoram;
	required_device<discrete_device> m_discrete;
	/* misc */
	UINT8          m_misc;
	UINT8          m_ic_h3_data;
	DECLARE_READ8_MEMBER(copsnrob_misc_r);
	DECLARE_WRITE8_MEMBER(copsnrob_misc2_w);
	DECLARE_WRITE8_MEMBER(copsnrob_misc_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_copsnrob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
};

/*----------- defined in audio/copsnrob.c -----------*/
DISCRETE_SOUND_EXTERN( copsnrob );
