// license:BSD-3-Clause
// copyright-holders:David Haywood,Bryan McPhail
/*************************************************************************

    Tumble Pop

*************************************************************************/

#include "video/decospr.h"
#include "video/deco16ic.h"

class tumblep_state : public driver_device
{
public:
	tumblep_state(const machine_config &mconfig, device_type type, std::string tag)
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

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	DECLARE_WRITE16_MEMBER(tumblep_sound_w);
	DECLARE_READ16_MEMBER(tumblepop_controls_r);
	DECLARE_DRIVER_INIT(tumblep);
	virtual void machine_start() override;
	UINT32 screen_update_tumblep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
