// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari GX2 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "includes/slapstic.h"


class atarigx2_state : public atarigen_state
{
public:
	atarigx2_state(const machine_config &mconfig, device_type type, std::string tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_mo_command(*this, "mo_command"),
			m_protection_base(*this, "protection_base"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_rle(*this, "rle")
			{ }

	UINT16          m_playfield_base;

	required_device<atari_jsa_iiis_device> m_jsa;

	required_shared_ptr<UINT32> m_mo_command;
	required_shared_ptr<UINT32> m_protection_base;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	UINT16          m_current_control;
	UINT8           m_playfield_tile_bank;
	UINT8           m_playfield_color_bank;
	UINT16          m_playfield_xscroll;
	UINT16          m_playfield_yscroll;

	UINT16          m_last_write;
	UINT16          m_last_write_offset;

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_READ32_MEMBER(special_port2_r);
	DECLARE_READ32_MEMBER(special_port3_r);
	DECLARE_READ32_MEMBER(a2d_data_r);
	DECLARE_WRITE32_MEMBER(latch_w);
	DECLARE_WRITE32_MEMBER(mo_command_w);
	DECLARE_WRITE32_MEMBER(atarigx2_protection_w);
	DECLARE_READ32_MEMBER(atarigx2_protection_r);
	DECLARE_READ32_MEMBER(rrreveng_prot_r);
	DECLARE_DRIVER_INIT(spclords);
	DECLARE_DRIVER_INIT(rrreveng);
	DECLARE_DRIVER_INIT(motofren);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILEMAP_MAPPER_MEMBER(atarigx2_playfield_scan);
	DECLARE_MACHINE_START(atarigx2);
	DECLARE_MACHINE_RESET(atarigx2);
	DECLARE_VIDEO_START(atarigx2);
	UINT32 screen_update_atarigx2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE16_MEMBER( atarigx2_mo_control_w );
};
