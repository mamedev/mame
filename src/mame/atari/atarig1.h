// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "atarijsa.h"
#include "machine/adc0808.h"
#include "atarigen.h"
#include "slapstic.h"
#include "machine/timer.h"
#include "atarirle.h"
#include "cpu/m68000/m68000.h"
#include "tilemap.h"

class atarig1_state : public atarigen_state
{
public:
	atarig1_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_slapstic(*this, "slapstic"),
			m_slapstic_bank(*this, "slapstic_bank"),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_rle(*this, "rle"),
			m_adc(*this, "adc"),
			m_in1(*this, "IN1"),
			m_mo_command(*this, "mo_command") { }

	void atarig1(machine_config &config);
	void pfslap111(machine_config &config);
	void pfslap112(machine_config &config);
	void pfslap113(machine_config &config);
	void pfslap114(machine_config &config);
	void pitfightb(machine_config &config);
	void hydrap(machine_config &config);
	void hydra(machine_config &config);

	void init_hydrap();
	void init_hydra();
	void init_pitfight();
	void init_pitfightb();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	optional_device<atari_slapstic_device> m_slapstic;
	optional_memory_bank m_slapstic_bank;
	required_device<atari_jsa_ii_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_rle_objects_device> m_rle;

	optional_device<adc0808_device> m_adc;
	optional_ioport m_in1;

	bool            m_is_pitfight = false;

	required_shared_ptr<uint16_t> m_mo_command;

	bool            m_bslapstic_primed = false;

	uint8_t           m_pfscroll_xoffset = 0;
	uint16_t          m_current_control = 0;
	uint8_t           m_playfield_tile_bank = 0;
	uint16_t          m_playfield_xscroll = 0;
	uint16_t          m_playfield_yscroll = 0;

	void video_int_ack_w(uint16_t data = 0);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);
	void mo_command_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void a2d_select_w(offs_t offset, uint16_t data);
	uint16_t a2d_data_r();
	void pitfightb_cheap_slapstic_tweak(offs_t offset);
	void update_bank(int bank);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_atarig1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
	void pitfight_map(address_map &map) ATTR_COLD;
	void hydra_map(address_map &map) ATTR_COLD;
};
