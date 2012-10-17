/*************************************************************************

    Atari System 1 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarisy1_state : public atarigen_state
{
public:
	atarisy1_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_bankselect(*this, "bankselect"),
		  m_joystick_timer(*this, "joystick_timer"),
		  m_yscroll_reset_timer(*this, "yreset_timer"),
		  m_scanline_timer(*this, "scan_timer"),
		  m_int3off_timer(*this, "int3off_timer") { }

	required_shared_ptr<UINT16> m_bankselect;

	UINT8			m_joystick_type;
	UINT8			m_trackball_type;

	required_device<timer_device> m_joystick_timer;
	UINT8			m_joystick_int;
	UINT8			m_joystick_int_enable;
	UINT8			m_joystick_value;

	/* playfield parameters */
	UINT16			m_playfield_lookup[256];
	UINT8			m_playfield_tile_bank;
	UINT16			m_playfield_priority_pens;
	required_device<timer_device> m_yscroll_reset_timer;

	/* INT3 tracking */
	int 			m_next_timer_scanline;
	required_device<timer_device> m_scanline_timer;
	required_device<timer_device> m_int3off_timer;

	/* graphics bank tracking */
	UINT8			m_bank_gfx[3][8];
	UINT8			m_bank_color_shift[MAX_GFX_ELEMENTS];

	UINT8			m_cur[2][2];
	virtual void update_interrupts();
	DECLARE_READ16_MEMBER(joystick_r);
	DECLARE_WRITE16_MEMBER(joystick_w);
	DECLARE_READ16_MEMBER(trakball_r);
	DECLARE_READ16_MEMBER(port4_r);
	DECLARE_READ8_MEMBER(switch_6502_r);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_WRITE8_MEMBER(via_pa_w);
	DECLARE_READ8_MEMBER(via_pa_r);
	DECLARE_WRITE8_MEMBER(via_pb_w);
	DECLARE_READ8_MEMBER(via_pb_r);
	DECLARE_DRIVER_INIT(roadb110);
	DECLARE_DRIVER_INIT(roadb109);
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
};

DECLARE_READ16_HANDLER( atarisy1_int3state_r );

DECLARE_WRITE16_HANDLER( atarisy1_spriteram_w );
DECLARE_WRITE16_HANDLER( atarisy1_bankselect_w );
DECLARE_WRITE16_HANDLER( atarisy1_xscroll_w );
DECLARE_WRITE16_HANDLER( atarisy1_yscroll_w );
DECLARE_WRITE16_HANDLER( atarisy1_priority_w );
