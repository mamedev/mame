// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari System 2 hardware

*************************************************************************/

#include "cpu/m6502/m6502.h"
#include "cpu/t11/t11.h"
#include "machine/gen_latch.h"
#include "slapstic.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "sound/ymopm.h"
#include "atarimo.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class atarisy2_state : public driver_device
{
public:
	atarisy2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_mob(*this, "mob")
		, m_slapstic_region(*this, "maincpu")
		, m_playfield_tilemap(*this, "playfield")
		, m_alpha_tilemap(*this, "alpha")
		, m_xscroll(*this, "xscroll")
		, m_yscroll(*this, "yscroll")
		, m_soundlatch(*this, "soundlatch")
		, m_mainlatch(*this, "mainlatch")
		, m_ym2151(*this, "ymsnd")
		, m_pokey(*this, "pokey%u", 1U)
		, m_tms5220(*this, "tms")
		, m_rombank(*this, "rombank%u", 1U)
		, m_slapstic(*this, "slapstic")
		, m_vmmu(*this, "vmmu")
		, m_playfieldt(*this, "playfieldt")
		, m_playfieldb(*this, "playfieldb")
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
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void update_interrupts();

	required_device<t11_device> m_maincpu;
	required_device<m6502_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<atari_motion_objects_device> m_mob;
	required_region_ptr<uint16_t> m_slapstic_region;

	uint8_t           m_interrupt_enable = 0U;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	required_shared_ptr<uint16_t> m_xscroll;
	required_shared_ptr<uint16_t> m_yscroll;

	int8_t            m_pedal_count = 0U;

	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_mainlatch;
	required_device<ym2151_device> m_ym2151;
	required_device_array<pokey_device, 2> m_pokey;
	optional_device<tms5220_device> m_tms5220;

	bool            m_scanline_int_state = 0;
	bool            m_video_int_state = 0;
	bool            m_p2portwr_state = 0;
	bool            m_p2portrd_state = 0;

	required_memory_bank_array<2> m_rombank;
	required_device<atari_slapstic_device> m_slapstic;
	memory_view m_vmmu;
	required_shared_ptr<uint16_t> m_playfieldt;
	required_shared_ptr<uint16_t> m_playfieldb;

	uint8_t         m_sound_reset_state = 0U;

	emu_timer *     m_yscroll_reset_timer = nullptr;
	uint32_t        m_playfield_tile_bank[2]{};

	// 720 fake joystick
	double          m_joy_last_angle = 0;
	int             m_joy_rotations = 0;

	// 720 fake spinner
	uint32_t        m_spin_last_rotate_count = 0U;
	int32_t         m_spin_pos = 0;                 /* track fake position of spinner */
	uint32_t        m_spin_center_count = 0U;

	output_finder<2> m_leds;

	void scanline_int_ack_w(uint8_t data);
	void video_int_ack_w(uint8_t data);
	void int0_ack_w(uint8_t data);
	void sound_reset_w(uint8_t data);
	void int_enable_w(uint8_t data);
	INTERRUPT_GEN_MEMBER(sound_irq_gen);
	void sound_irq_ack_w(uint8_t data);
	void bankselect_w(offs_t offset, uint16_t data);
	uint8_t leta_r(offs_t offset);
	void mixer_w(uint8_t data);
	void sndrst_6502_w(uint8_t data);
	uint16_t sound_r();
	void sound_6502_w(uint8_t data);
	uint8_t sound_6502_r();
	void tms5220_w(uint8_t data);
	void tms5220_strobe_w(offs_t offset, uint8_t data);
	void coincount_w(uint8_t data);
	void switch_6502_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_update);

	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	uint32_t screen_update_atarisy2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_int(int state);
	TIMER_CALLBACK_MEMBER(delayed_int_enable_w);
	TIMER_CALLBACK_MEMBER(reset_yscroll_callback);
	void yscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void xscroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void spriteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void playfieldt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void playfieldb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	static rgb_t RRRRGGGGBBBBIIII(uint32_t raw);

	static const atari_motion_objects_config s_mob_config;
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};
