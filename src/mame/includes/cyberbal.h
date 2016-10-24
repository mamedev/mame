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

class cyberbal_state : public atarigen_state
{
public:
	cyberbal_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_extracpu(*this, "extra"),
			m_daccpu(*this, "dac"),
			m_rdac(*this, "rdac"),
			m_ldac(*this, "ldac"),
			m_soundcomm(*this, "soundcomm"),
			m_jsa(*this, "jsa"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_mob(*this, "mob"),
			m_playfield2_tilemap(*this, "playfield2"),
			m_alpha2_tilemap(*this, "alpha2"),
			m_mob2(*this, "mob2"),
			m_lscreen(*this, "lscreen"),
			m_rscreen(*this, "rscreen") { }

	required_device<cpu_device> m_maincpu;
	optional_device<m6502_device> m_audiocpu;
	optional_device<cpu_device> m_extracpu;
	optional_device<cpu_device> m_daccpu;
	optional_device<dac_word_interface> m_rdac;
	optional_device<dac_word_interface> m_ldac;
	optional_device<atari_sound_comm_device> m_soundcomm;
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
	uint16_t sound_state_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void p2_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t special_port3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sound_6502_stat_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_68k_6502_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_68k_6502_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void io_68k_irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sound_68k_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_68k_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_68k_dac_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_cyberbalt();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_cyberbal();
	void machine_start_cyberbal2p();
	void machine_reset_cyberbal();
	void video_start_cyberbal();
	void machine_reset_cyberbal2p();
	void video_start_cyberbal2p();
	uint32_t screen_update_cyberbal_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cyberbal_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cyberbal2p(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_68k_irq_gen(device_t &device);

	static const atari_motion_objects_config s_mob_config;

private:
	void video_start_common(int screens);
	void cyberbal_sound_reset();
	uint32_t update_one_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index);
	void update_sound_68k_interrupts();
};
