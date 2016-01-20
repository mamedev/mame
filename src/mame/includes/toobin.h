// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Toobin' hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class toobin_state : public atarigen_state
{
public:
	toobin_state(const machine_config &mconfig, device_type type, std::string tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob"),
			m_interrupt_scan(*this, "interrupt_scan") { }

	required_device<atari_jsa_i_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	required_shared_ptr<UINT16> m_interrupt_scan;

	double          m_brightness;
	bitmap_ind16 m_pfbitmap;

	virtual void update_interrupts() override;

	DECLARE_WRITE16_MEMBER(interrupt_scan_w);
	DECLARE_WRITE16_MEMBER(paletteram_w);
	DECLARE_WRITE16_MEMBER(intensity_w);
	DECLARE_WRITE16_MEMBER(xscroll_w);
	DECLARE_WRITE16_MEMBER(yscroll_w);
	DECLARE_WRITE16_MEMBER(slip_w);

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);

	DECLARE_MACHINE_START(toobin);
	DECLARE_MACHINE_RESET(toobin);
	DECLARE_VIDEO_START(toobin);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
