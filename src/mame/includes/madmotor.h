// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Mad Motor

*************************************************************************/

#include "video/decbac06.h"
#include "video/decmxc06.h"
#include "audio/deco6280.h"

class madmotor_state : public driver_device
{
public:
	madmotor_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_tilegen1(*this, "tilegen1"),
		m_tilegen2(*this, "tilegen2"),
		m_tilegen3(*this, "tilegen3"),
		m_spritegen(*this, "spritegen"),
		m_decosnd(*this, DECOSND_TAG) { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	int             m_flipscreen;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<deco_bac06_device> m_tilegen1;
	required_device<deco_bac06_device> m_tilegen2;
	required_device<deco_bac06_device> m_tilegen3;
	required_device<deco_mxc06_device> m_spritegen;
	required_device<deco_6280_ym2203_2xoki_device> m_decosnd;

	DECLARE_DRIVER_INIT(madmotor);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_madmotor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void madmotor(machine_config &config);
};
