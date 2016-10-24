// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari System 2 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "cpu/m6502/m6502.h"
#include "cpu/t11/t11.h"
#include "machine/watchdog.h"
#include "sound/ym2151.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "slapstic.h"

class atarisy2_state : public atarigen_state
{
public:
	atarisy2_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_mob(*this, "mob"),
			m_slapstic_base(*this, "slapstic_base"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_soundcomm(*this, "soundcomm"),
			m_ym2151(*this, "ymsnd"),
			m_pokey1(*this, "pokey1"),
			m_pokey2(*this, "pokey2"),
			m_tms5220(*this, "tms"),
			m_rombank1(*this, "rombank1"),
			m_rombank2(*this, "rombank2"),
			m_slapstic(*this, "slapstic")
			{ }

	required_device<t11_device> m_maincpu;
	required_device<m6502_device> m_audiocpu;
	required_device<atari_motion_objects_device> m_mob;
	required_shared_ptr<uint16_t> m_slapstic_base;

	uint8_t           m_interrupt_enable;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;

	int8_t            m_pedal_count;

	required_device<atari_sound_comm_device> m_soundcomm;
	required_device<ym2151_device> m_ym2151;
	required_device<pokey_device> m_pokey1;
	required_device<pokey_device> m_pokey2;
	optional_device<tms5220_device> m_tms5220;

	uint8_t           m_which_adc;

	uint8_t           m_p2portwr_state;
	uint8_t           m_p2portrd_state;

	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_device<atari_slapstic_device> m_slapstic;

	uint8_t           m_sound_reset_state;

	emu_timer *     m_yscroll_reset_timer;
	uint32_t          m_playfield_tile_bank[2];
	uint32_t          m_videobank;

	// 720 fake joystick
	double          m_joy_last_angle;
	int             m_joy_rotations;

	// 720 fake spinner
	uint32_t          m_spin_last_rotate_count;
	int32_t           m_spin_pos;                 /* track fake position of spinner */
	uint32_t          m_spin_center_count;

	uint16_t          m_vram[0x8000/2];

	virtual void device_post_load() override;

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	void int0_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void int1_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void int_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bankselect_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t switch_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t switch_6502_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void switch_6502_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adc_strobe_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t adc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t leta_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mixer_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_reset_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t sound_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_6502_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_6502_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tms5220_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tms5220_strobe_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coincount_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_ssprint();
	void init_apb();
	void init_csprint();
	void init_paperboy();
	void init_720();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_atarisy2();
	void machine_reset_atarisy2();
	void video_start_atarisy2();
	uint32_t screen_update_atarisy2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_int(device_t &device);
	void delayed_int_enable_w(void *ptr, int32_t param);
	void reset_yscroll_callback(void *ptr, int32_t param);
	uint16_t slapstic_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t videoram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void slapstic_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void yscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void xscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	static rgb_t RRRRGGGGBBBBIIII_decoder(uint32_t raw);

	static const atari_motion_objects_config s_mob_config;
};
