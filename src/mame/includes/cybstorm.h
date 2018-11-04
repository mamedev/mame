// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Phil Bennett
/*************************************************************************

    Atari Cyberstorm hardware

*************************************************************************/

#include "audio/atarijsa.h"
#include "machine/atarigen.h"
#include "machine/bankdev.h"


class cybstorm_state : public atarigen_state
{
public:
	cybstorm_state(const machine_config &mconfig, device_type type, const char *tag)
	: atarigen_state(mconfig, type, tag)
	, m_jsa(*this, "jsa")
	, m_vad(*this, "vad")
	, m_vadbank(*this, "vadbank")
	{ }

	optional_device<atari_jsa_iiis_device> m_jsa;
	required_device<atari_vad_device> m_vad;
	required_device<address_map_bank_device> m_vadbank;

	uint32_t m_latch_data;
	uint8_t m_alpha_tile_bank;

	virtual void update_interrupts() override;

	DECLARE_READ32_MEMBER(special_port1_r);
	DECLARE_WRITE32_MEMBER(latch_w);

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	TILEMAP_MAPPER_MEMBER(playfield_scan);

	DECLARE_DRIVER_INIT(cybstorm);
	DECLARE_MACHINE_START(cybstorm);
	DECLARE_MACHINE_RESET(cybstorm);

	DECLARE_VIDEO_START(cybstorm);
	uint32_t screen_update_cybstorm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	static const atari_motion_objects_config s_mob_config;
	void cybstorm(machine_config &config);
	void round2(machine_config &config);
	void main_map(address_map &map);
	void vadbank_map(address_map &map);
};
