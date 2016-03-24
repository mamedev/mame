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

	required_shared_ptr<UINT16> m_bankselect;
	required_device<atari_motion_objects_device> m_mob;

	UINT8           m_joystick_type;
	UINT8           m_trackball_type;

	required_device<timer_device> m_joystick_timer;
	UINT8           m_joystick_int;
	UINT8           m_joystick_int_enable;
	UINT8           m_joystick_value;

	/* playfield parameters */
	required_device<tilemap_device> m_playfield_tilemap;
	required_device<tilemap_device> m_alpha_tilemap;
	UINT16          m_playfield_lookup[256];
	UINT8           m_playfield_tile_bank;
	UINT16          m_playfield_priority_pens;
	required_device<timer_device> m_yscroll_reset_timer;

	/* INT3 tracking */
	int             m_next_timer_scanline;
	required_device<timer_device> m_scanline_timer;
	required_device<timer_device> m_int3off_timer;

	/* speech */
	required_device<tms5220_device> m_tms;

	/* graphics bank tracking */
	UINT8           m_bank_gfx[3][8];
	UINT8           m_bank_color_shift[MAX_GFX_ELEMENTS];

	UINT8           m_cur[2][2];
	virtual void update_interrupts() override;
	DECLARE_READ16_MEMBER(joystick_r);
	DECLARE_WRITE16_MEMBER(joystick_w);
	DECLARE_READ16_MEMBER(trakball_r);
	DECLARE_READ8_MEMBER(switch_6502_r);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(via_pa_w);
	DECLARE_READ8_MEMBER(via_pa_r);
	DECLARE_WRITE8_MEMBER(via_pb_w);
	DECLARE_READ8_MEMBER(via_pb_r);
	DECLARE_DRIVER_INIT(roadblst);
	DECLARE_DRIVER_INIT(peterpak);
	DECLARE_DRIVER_INIT(marble);
	DECLARE_DRIVER_INIT(roadrunn);
	DECLARE_DRIVER_INIT(indytemp);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(atarisy1);
	DECLARE_MACHINE_RESET(atarisy1);
	DECLARE_VIDEO_START(atarisy1);
	UINT32 screen_update_atarisy1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(delayed_joystick_int);
	TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_reset_yscroll_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_int3off_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(atarisy1_int3_callback);
	void update_timers(int scanline);
	void decode_gfx(UINT16 *pflookup, UINT16 *molookup);
	int get_bank(UINT8 prom1, UINT8 prom2, int bpp);
	DECLARE_READ16_MEMBER( atarisy1_int3state_r );
	DECLARE_WRITE16_MEMBER( atarisy1_spriteram_w );
	DECLARE_WRITE16_MEMBER( atarisy1_bankselect_w );
	DECLARE_WRITE16_MEMBER( atarisy1_xscroll_w );
	DECLARE_WRITE16_MEMBER( atarisy1_yscroll_w );
	DECLARE_WRITE16_MEMBER( atarisy1_priority_w );

	static const atari_motion_objects_config s_mob_config;
};
