// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari G42 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "cpu/m68000/m68000.h"
#include "machine/asic65.h"

class atarig42_state : public atarigen_state
{
public:
	atarig42_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_rle(*this, "rle"),
			m_asic65(*this, "asic65"),
			m_mo_command(*this, "mo_command") { }

	required_device<cpu_device> m_maincpu;
	required_device<atari_jsa_iii_device> m_jsa;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;
	required_device<asic65_device> m_asic65;

	UINT16          m_playfield_base;

	UINT16          m_current_control;
	UINT8           m_playfield_tile_bank;
	UINT8           m_playfield_color_bank;
	UINT16          m_playfield_xscroll;
	UINT16          m_playfield_yscroll;

	UINT8           m_analog_data;
	required_shared_ptr<UINT16> m_mo_command;

	int             m_sloop_bank;
	int             m_sloop_next_bank;
	int             m_sloop_offset;
	int             m_sloop_state;
	UINT16 *        m_sloop_base;

	UINT32          m_last_accesses[8];
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_WRITE16_MEMBER(a2d_select_w);
	DECLARE_READ16_MEMBER(a2d_data_r);
	DECLARE_WRITE16_MEMBER(io_latch_w);
	DECLARE_WRITE16_MEMBER(mo_command_w);
	DECLARE_READ16_MEMBER(roadriot_sloop_data_r);
	DECLARE_WRITE16_MEMBER(roadriot_sloop_data_w);
	DECLARE_READ16_MEMBER(guardians_sloop_data_r);
	DECLARE_WRITE16_MEMBER(guardians_sloop_data_w);
	void roadriot_sloop_tweak(int offset);
	void guardians_sloop_tweak(int offset);
	DECLARE_DIRECT_UPDATE_MEMBER(atarig42_sloop_direct_handler);
	DECLARE_DRIVER_INIT(roadriot);
	DECLARE_DRIVER_INIT(guardian);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILEMAP_MAPPER_MEMBER(atarig42_playfield_scan);
	DECLARE_MACHINE_START(atarig42);
	DECLARE_MACHINE_RESET(atarig42);
	DECLARE_VIDEO_START(atarig42);
	UINT32 screen_update_atarig42(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
