// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Cyberball hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "audio/atarijsa.h"
#include "video/atarimo.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "sound/dac.h"
#include "sound/ym2151.h"
#include "screen.h"

class cyberbal_state : public atarigen_state
{
public:
	cyberbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_audiocpu(*this, "audiocpu"),
			m_extracpu(*this, "extra"),
			m_daccpu(*this, "dac"),
			m_rdac(*this, "rdac"),
			m_ldac(*this, "ldac"),
			m_soundcomm(*this, "soundcomm"),
			m_ymsnd(*this, "ymsnd"),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob"),
			m_playfield2_tilemap(*this, "playfield2"),
			m_alpha2_tilemap(*this, "alpha2"),
			m_mob2(*this, "mob2"),
			m_lscreen(*this, "lscreen"),
			m_rscreen(*this, "rscreen") { }

	optional_device<m6502_device> m_audiocpu;
	optional_device<cpu_device> m_extracpu;
	optional_device<cpu_device> m_daccpu;
	optional_device<dac_word_interface> m_rdac;
	optional_device<dac_word_interface> m_ldac;
	optional_device<atari_sound_comm_device> m_soundcomm;
	optional_device<ym2151_device> m_ymsnd;
	optional_device<atari_jsa_ii_device> m_jsa;
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	optional_device<tilemap_device> m_playfield2_tilemap;
	optional_device<tilemap_device> m_alpha2_tilemap;
	optional_device<atari_motion_objects_device> m_mob2;
	optional_device<screen_device> m_lscreen;
	optional_device<screen_device> m_rscreen;

	uint16_t          m_current_slip[2];
	uint8_t           m_playfield_palette_bank[2];
	uint16_t          m_playfield_xscroll[2];
	uint16_t          m_playfield_yscroll[2];

	uint8_t           m_fast_68k_int;
	uint8_t           m_io_68k_int;
	uint8_t           m_sound_data_from_68k;
	uint8_t           m_sound_data_from_6502;
	uint8_t           m_sound_data_from_68k_ready;
	uint8_t           m_sound_data_from_6502_ready;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_READ16_MEMBER(sound_state_r);
	DECLARE_WRITE16_MEMBER(p2_reset_w);
	DECLARE_READ8_MEMBER(special_port3_r);
	DECLARE_READ8_MEMBER(sound_6502_stat_r);
	DECLARE_WRITE8_MEMBER(sound_bank_select_w);
	DECLARE_READ8_MEMBER(sound_68k_6502_r);
	DECLARE_WRITE8_MEMBER(sound_68k_6502_w);
	DECLARE_WRITE16_MEMBER(io_68k_irq_ack_w);
	DECLARE_READ16_MEMBER(sound_68k_r);
	DECLARE_WRITE16_MEMBER(sound_68k_w);
	DECLARE_WRITE16_MEMBER(sound_68k_dac_w);
	DECLARE_DRIVER_INIT(cyberbalt);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_alpha2_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	DECLARE_MACHINE_START(cyberbal);
	DECLARE_MACHINE_START(cyberbal2p);
	DECLARE_MACHINE_RESET(cyberbal);
	DECLARE_VIDEO_START(cyberbal);
	DECLARE_MACHINE_RESET(cyberbal2p);
	DECLARE_VIDEO_START(cyberbal2p);
	uint32_t screen_update_cyberbal_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cyberbal_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cyberbal2p(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(sound_68k_irq_gen);

	static const atari_motion_objects_config s_mob_config;

	void cyberbalt(machine_config &config);
	void cyberbal2p(machine_config &config);
	void cyberbal(machine_config &config);
	void cyberbal2p_map(address_map &map);
	void extra_map(address_map &map);
	void main_map(address_map &map);
	void sound_68k_map(address_map &map);
	void sound_map(address_map &map);
private:
	void video_start_common(int screens);
	void cyberbal_sound_reset();
	uint32_t update_one_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index);
	void update_sound_68k_interrupts();
};
