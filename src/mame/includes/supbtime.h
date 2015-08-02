// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*************************************************************************

    Super Burger Time & China Town

*************************************************************************/

#include "video/decospr.h"
#include "video/deco16ic.h"

class supbtime_state : public driver_device
{
public:
	supbtime_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_pf1_rowscroll(*this, "pf1_rowscroll"),
		m_pf2_rowscroll(*this, "pf2_rowscroll"),
		m_sprgen(*this, "spritegen"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_deco_tilegen1(*this, "tilegen1")
	{ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_pf1_rowscroll;
	required_shared_ptr<UINT16> m_pf2_rowscroll;
	optional_device<decospr_device> m_sprgen;

	/* video-related */

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	DECLARE_READ16_MEMBER(supbtime_controls_r);
	DECLARE_WRITE16_MEMBER(sound_w);
	virtual void machine_start();
	UINT32 screen_update_supbtime(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
