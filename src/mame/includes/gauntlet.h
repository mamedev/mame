// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Gauntlet hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "video/atarimo.h"

class gauntlet_state : public atarigen_state
{
public:
	gauntlet_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob")  { }

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;

	UINT16          m_sound_reset_val;
	UINT8           m_vindctr2_screen_refresh;
	UINT8           m_playfield_tile_bank;
	UINT8           m_playfield_color_bank;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_WRITE16_MEMBER(sound_reset_w);
	DECLARE_READ8_MEMBER(switch_6502_r);
	DECLARE_WRITE8_MEMBER(sound_ctl_w);
	DECLARE_WRITE8_MEMBER(mixer_w);
	void swap_memory(void *ptr1, void *ptr2, int bytes);
	void common_init(int slapstic, int vindctr2);
	DECLARE_DRIVER_INIT(gauntlet2);
	DECLARE_DRIVER_INIT(gaunt2p);
	DECLARE_DRIVER_INIT(gauntlet);
	DECLARE_DRIVER_INIT(vindctr2);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(gauntlet);
	DECLARE_MACHINE_RESET(gauntlet);
	DECLARE_VIDEO_START(gauntlet);
	UINT32 screen_update_gauntlet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE16_MEMBER( gauntlet_xscroll_w );
	DECLARE_WRITE16_MEMBER( gauntlet_yscroll_w );

	static const atari_motion_objects_config s_mob_config;
};
