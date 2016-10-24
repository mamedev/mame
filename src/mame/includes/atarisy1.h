// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari System 1 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "sound/tms5220.h"
#include "video/atarimo.h"

class atarisy1_state : public atarigen_state
{
public:
	atarisy1_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_audiocpu(*this, "audiocpu"),
			m_soundcomm(*this, "soundcomm"),
			m_bankselect(*this, "bankselect"),
			m_mob(*this, "mob"),
			m_joystick_timer(*this, "joystick_timer"),
			m_playfield_tilemap(*this, "playfield"),
			m_alpha_tilemap(*this, "alpha"),
			m_yscroll_reset_timer(*this, "yreset_timer"),
			m_scanline_timer(*this, "scan_timer"),
			m_int3off_timer(*this, "int3off_timer"),
			m_tms(*this, "tms") { }

	required_device<cpu_device> m_audiocpu;
	required_device<atari_sound_comm_device> m_soundcomm;

	required_shared_ptr<uint16_t> m_bankselect;
	required_device<atari_motion_objects_device> m_mob;

	uint8_t           m_joystick_type;
	uint8_t           m_trackball_type;

	required_device<timer_device> m_joystick_timer;
	uint8_t           m_joystick_int;
	uint8_t           m_joystick_int_enable;
	uint8_t           m_joystick_value;

	/* playfield parameters */
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	uint16_t          m_playfield_lookup[256];
	uint8_t           m_playfield_tile_bank;
	uint16_t          m_playfield_priority_pens;
	required_device<timer_device> m_yscroll_reset_timer;

	/* INT3 tracking */
	int             m_next_timer_scanline;
	required_device<timer_device> m_scanline_timer;
	required_device<timer_device> m_int3off_timer;

	/* speech */
	required_device<tms5220_device> m_tms;

	/* graphics bank tracking */
	uint8_t           m_bank_gfx[3][8];
	uint8_t           m_bank_color_shift[MAX_GFX_ELEMENTS];

	uint8_t           m_cur[2][2];
	virtual void update_interrupts() override;
	uint16_t joystick_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void joystick_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t trakball_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t switch_6502_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void led_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void via_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t via_pa_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void via_pb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t via_pb_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void init_roadblst();
	void init_peterpak();
	void init_marble();
	void init_roadrunn();
	void init_indytemp();
	void get_alpha_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_playfield_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_atarisy1();
	void machine_reset_atarisy1();
	void video_start_atarisy1();
	uint32_t screen_update_atarisy1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void delayed_joystick_int(timer_device &timer, void *ptr, int32_t param);
	void atarisy1_reset_yscroll_callback(timer_device &timer, void *ptr, int32_t param);
	void atarisy1_int3off_callback(timer_device &timer, void *ptr, int32_t param);
	void atarisy1_int3_callback(timer_device &timer, void *ptr, int32_t param);
	void update_timers(int scanline);
	void decode_gfx(uint16_t *pflookup, uint16_t *molookup);
	int get_bank(uint8_t prom1, uint8_t prom2, int bpp);
	uint16_t atarisy1_int3state_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void atarisy1_spriteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void atarisy1_bankselect_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void atarisy1_xscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void atarisy1_yscroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void atarisy1_priority_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	static const atari_motion_objects_config s_mob_config;
};
