// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Batman hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"

class batman_state : public atarigen_state
{
public:
	batman_state(const machine_config &mconfig, device_type type, std::string tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_vad(*this, "vad") { }

	required_device<atari_jsa_iii_device> m_jsa;
	required_device<atari_vad_device> m_vad;

	UINT16          m_latch_data;
	UINT8           m_alpha_tile_bank;

	virtual void update_interrupts() override;
	DECLARE_WRITE16_MEMBER(latch_w);
	DECLARE_DRIVER_INIT(batman);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	DECLARE_MACHINE_START(batman);
	DECLARE_MACHINE_RESET(batman);
	DECLARE_VIDEO_START(batman);
	UINT32 screen_update_batman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
