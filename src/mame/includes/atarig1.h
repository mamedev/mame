// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "audio/atarijsa.h"
#include "machine/atarigen.h"
#include "video/atarirle.h"
#include "cpu/m68000/m68000.h"

class atarig1_state : public atarigen_state
{
public:
	atarig1_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_rle(*this, "rle"),
			m_mo_command(*this, "mo_command") { }

	required_device<atari_jsa_ii_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	bool            m_is_pitfight;

	uint8_t           m_which_input;
	required_shared_ptr<uint16_t> m_mo_command;

	uint16_t *        m_bslapstic_base;
	std::unique_ptr<uint8_t[]>          m_bslapstic_bank0;
	uint8_t           m_bslapstic_bank;
	bool            m_bslapstic_primed;

	int             m_pfscroll_xoffset;
	uint16_t          m_current_control;
	uint8_t           m_playfield_tile_bank;
	uint16_t          m_playfield_xscroll;
	uint16_t          m_playfield_yscroll;

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
	DECLARE_DRIVER_INIT(pitfight);
	DECLARE_DRIVER_INIT(pitfightb);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(atarig1);
	DECLARE_MACHINE_RESET(atarig1);
	DECLARE_VIDEO_START(atarig1);
	uint32_t screen_update_atarig1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void atarig1(machine_config &config);
	void pitfightj(machine_config &config);
	void pitfight7(machine_config &config);
	void pitfight9(machine_config &config);
	void pitfightb(machine_config &config);
	void pitfight(machine_config &config);
	void hydrap(machine_config &config);
	void hydra(machine_config &config);
	void main_map(address_map &map);
private:
	void pitfightb_cheap_slapstic_init();
};
