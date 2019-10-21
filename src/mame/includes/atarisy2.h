// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari System 2 hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "video/atarimo.h"
#include "cpu/m6502/m6502.h"
#include "cpu/t11/t11.h"
#include "machine/bankdev.h"
#include "machine/watchdog.h"
#include "sound/ym2151.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "emupal.h"
#include "slapstic.h"
#include "tilemap.h"

class atarisy2_state : public atarigen_state
{
public:
	atarisy2_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_mob(*this, "mob")
		, m_slapstic_base(*this, "slapstic_base")
		, m_vrambank(*this, "vrambank")
		, m_playfield_tilemap(*this, "playfield")
		, m_alpha_tilemap(*this, "alpha")
		, m_soundcomm(*this, "soundcomm")
		, m_ym2151(*this, "ymsnd")
		, m_pokey(*this, "pokey%u", 1U)
		, m_tms5220(*this, "tms")
		, m_rombank(*this, "rombank%u", 1U)
		, m_slapstic(*this, "slapstic")
		, m_leds(*this, "led%u", 0U)
	{ }

	void init_ssprint();
	void init_apb();
	void init_csprint();
	void init_paperboy();
	void init_720();

	void atarisy2(machine_config &config);
	void apb(machine_config &config);
	void paperboy(machine_config &config);
	void ssprint(machine_config &config);
	void _720(machine_config &config);
	void csprint(machine_config &config);

protected:
	virtual void device_post_load() override;
	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;

private:
	required_device<t11_device> m_maincpu;
	required_device<m6502_device> m_audiocpu;
	required_device<atari_motion_objects_device> m_mob;
	required_shared_ptr<uint16_t> m_slapstic_base;
	required_device<address_map_bank_device> m_vrambank;

	uint8_t           m_interrupt_enable;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;

	int8_t            m_pedal_count;

	required_device<atari_sound_comm_device> m_soundcomm;
	required_device<ym2151_device> m_ym2151;
	required_device_array<pokey_device, 2> m_pokey;
	optional_device<tms5220_device> m_tms5220;

	uint8_t           m_p2portwr_state;
	uint8_t           m_p2portrd_state;

	required_memory_bank_array<2> m_rombank;
	required_device<atari_slapstic_device> m_slapstic;

	uint8_t           m_sound_reset_state;

	emu_timer *     m_yscroll_reset_timer;
	uint32_t          m_playfield_tile_bank[2];

	// 720 fake joystick
	double          m_joy_last_angle;
	int             m_joy_rotations;

	// 720 fake spinner
	uint32_t          m_spin_last_rotate_count;
	int32_t           m_spin_pos;                 /* track fake position of spinner */
	uint32_t          m_spin_center_count;

	output_finder<2> m_leds;

	DECLARE_WRITE16_MEMBER(int0_ack_w);
	DECLARE_WRITE16_MEMBER(int1_ack_w);
	DECLARE_WRITE16_MEMBER(int_enable_w);
	DECLARE_WRITE16_MEMBER(bankselect_w);
	DECLARE_READ16_MEMBER(switch_r);
	DECLARE_READ8_MEMBER(switch_6502_r);
	DECLARE_WRITE8_MEMBER(switch_6502_w);
	DECLARE_READ8_MEMBER(leta_r);
	DECLARE_WRITE8_MEMBER(mixer_w);
	DECLARE_WRITE8_MEMBER(sound_reset_w);
	DECLARE_READ16_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(sound_6502_w);
	DECLARE_READ8_MEMBER(sound_6502_r);
	DECLARE_WRITE8_MEMBER(tms5220_w);
	DECLARE_WRITE8_MEMBER(tms5220_strobe_w);
	DECLARE_WRITE8_MEMBER(coincount_w);

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(atarisy2);
	DECLARE_MACHINE_RESET(atarisy2);
	DECLARE_VIDEO_START(atarisy2);
	uint32_t screen_update_atarisy2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_int);
	TIMER_CALLBACK_MEMBER(delayed_int_enable_w);
	TIMER_CALLBACK_MEMBER(reset_yscroll_callback);
	DECLARE_READ16_MEMBER(slapstic_r);
	DECLARE_WRITE16_MEMBER(slapstic_w);
	DECLARE_WRITE16_MEMBER(yscroll_w);
	DECLARE_WRITE16_MEMBER(xscroll_w);
	DECLARE_WRITE16_MEMBER(spriteram_w);
	static rgb_t RRRRGGGGBBBBIIII(uint32_t raw);

	static const atari_motion_objects_config s_mob_config;
	void main_map(address_map &map);
	void sound_map(address_map &map);
	void vrambank_map(address_map &map);
};
