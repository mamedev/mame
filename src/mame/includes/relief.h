// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"

class relief_state : public atarigen_state
{
public:
	relief_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_vad(*this, "vad"),
			m_oki(*this, "oki"),
			m_ym2413(*this, "ymsnd"),
			m_okibank(*this, "okibank")
			{ }

	required_device<atari_vad_device> m_vad;
	required_device<okim6295_device> m_oki;
	required_device<ym2413_device> m_ym2413;
	required_memory_bank m_okibank;

	UINT8           m_ym2413_volume;
	UINT8           m_overall_volume;
	UINT8           m_adpcm_bank;
	virtual void update_interrupts() override;
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_WRITE16_MEMBER(audio_control_w);
	DECLARE_WRITE16_MEMBER(audio_volume_w);
	DECLARE_DRIVER_INIT(relief);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	DECLARE_MACHINE_START(relief);
	DECLARE_MACHINE_RESET(relief);
	DECLARE_VIDEO_START(relief);
	UINT32 screen_update_relief(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
};
