/*************************************************************************

    Atari System 2 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarisy2_state : public atarigen_state
{
public:
	atarisy2_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_slapstic_base(*this, "slapstic_base"),
		  m_bankselect(*this, "bankselect"),
		  m_rombank1(*this, "rombank1"),
		  m_rombank2(*this, "rombank2") { }

	required_shared_ptr<UINT16> m_slapstic_base;

	UINT8			m_interrupt_enable;
	required_shared_ptr<UINT16> m_bankselect;

	INT8			m_pedal_count;

	UINT8			m_has_tms5220;

	UINT8			m_which_adc;

	UINT8			m_p2portwr_state;
	UINT8			m_p2portrd_state;

	required_shared_ptr<UINT16> m_rombank1;
	required_shared_ptr<UINT16> m_rombank2;

	UINT8			m_sound_reset_state;

	emu_timer *		m_yscroll_reset_timer;
	UINT32			m_playfield_tile_bank[2];
	UINT32			m_videobank;

	// 720 fake joystick
	double			m_joy_last_angle;
	int				m_joy_rotations;

	// 720 fake spinner
	UINT32			m_spin_last_rotate_count;
	INT32			m_spin_pos;					/* track fake position of spinner */
	UINT32			m_spin_center_count;

	UINT16			m_vram[0x8000/2];
	DECLARE_WRITE16_MEMBER(int0_ack_w);
	DECLARE_WRITE16_MEMBER(int1_ack_w);
	DECLARE_WRITE16_MEMBER(int_enable_w);
	DECLARE_WRITE16_MEMBER(bankselect_w);
	DECLARE_READ16_MEMBER(switch_r);
	DECLARE_READ8_MEMBER(switch_6502_r);
	DECLARE_WRITE8_MEMBER(switch_6502_w);
	DECLARE_WRITE16_MEMBER(adc_strobe_w);
	DECLARE_READ16_MEMBER(adc_r);
	DECLARE_READ8_MEMBER(leta_r);
	DECLARE_WRITE8_MEMBER(mixer_w);
	DECLARE_WRITE8_MEMBER(sound_reset_w);
	DECLARE_READ16_MEMBER(sound_r);
	DECLARE_WRITE8_MEMBER(sound_6502_w);
	DECLARE_READ8_MEMBER(sound_6502_r);
	DECLARE_WRITE8_MEMBER(tms5220_w);
	DECLARE_WRITE8_MEMBER(tms5220_strobe_w);
	DECLARE_WRITE8_MEMBER(coincount_w);
};


/*----------- defined in video/atarisy2.c -----------*/

READ16_HANDLER( atarisy2_slapstic_r );
READ16_HANDLER( atarisy2_videoram_r );

WRITE16_HANDLER( atarisy2_slapstic_w );
WRITE16_HANDLER( atarisy2_yscroll_w );
WRITE16_HANDLER( atarisy2_xscroll_w );
WRITE16_HANDLER( atarisy2_videoram_w );
WRITE16_HANDLER( atarisy2_paletteram_w );

VIDEO_START( atarisy2 );
SCREEN_UPDATE_IND16( atarisy2 );
