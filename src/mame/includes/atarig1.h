// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "cpu/m68000/m68000.h"

class atarig1_state : public atarigen_state
{
public:
	atarig1_state(const machine_config &mconfig, device_type type, std::string tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_rle(*this, "rle"),
			m_mo_command(*this, "mo_command") { }

	required_device<cpu_device> m_maincpu;
	required_device<atari_jsa_ii_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	bool            m_is_pitfight;

	UINT8           m_which_input;
	required_shared_ptr<UINT16> m_mo_command;

	UINT16 *        m_bslapstic_base;
	std::unique_ptr<UINT8[]>          m_bslapstic_bank0;
	UINT8           m_bslapstic_bank;
	bool            m_bslapstic_primed;

	int             m_pfscroll_xoffset;
	UINT16          m_current_control;
	UINT8           m_playfield_tile_bank;
	UINT16          m_playfield_xscroll;
	UINT16          m_playfield_yscroll;

	virtual void device_post_load() override;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_WRITE16_MEMBER(mo_command_w);
	DECLARE_READ16_MEMBER(special_port0_r);
	DECLARE_WRITE16_MEMBER(a2d_select_w);
	DECLARE_READ16_MEMBER(a2d_data_r);
	DECLARE_READ16_MEMBER(pitfightb_cheap_slapstic_r);
	void update_bank(int bank);
	DECLARE_DRIVER_INIT(hydrap);
	DECLARE_DRIVER_INIT(hydra);
	DECLARE_DRIVER_INIT(pitfight9);
	DECLARE_DRIVER_INIT(pitfight7);
	DECLARE_DRIVER_INIT(pitfightj);
	DECLARE_DRIVER_INIT(pitfight);
	DECLARE_DRIVER_INIT(pitfightb);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(atarig1);
	DECLARE_MACHINE_RESET(atarig1);
	DECLARE_VIDEO_START(atarig1);
	UINT32 screen_update_atarig1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
private:
	void init_common(offs_t slapstic_base, int slapstic, bool is_pitfight);
	void pitfightb_cheap_slapstic_init();
};
