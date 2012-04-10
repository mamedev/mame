/*************************************************************************

    Atari Gauntlet hardware

*************************************************************************/

#include "machine/atarigen.h"

class gauntlet_state : public atarigen_state
{
public:
	gauntlet_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16			m_sound_reset_val;
	UINT8			m_vindctr2_screen_refresh;
	UINT8			m_playfield_tile_bank;
	UINT8			m_playfield_color_bank;
	DECLARE_READ16_MEMBER(port4_r);
	DECLARE_WRITE16_MEMBER(sound_reset_w);
	DECLARE_READ8_MEMBER(switch_6502_r);
	DECLARE_WRITE8_MEMBER(sound_ctl_w);
	DECLARE_WRITE8_MEMBER(mixer_w);
};


/*----------- defined in video/gauntlet.c -----------*/

WRITE16_HANDLER( gauntlet_xscroll_w );
WRITE16_HANDLER( gauntlet_yscroll_w );

VIDEO_START( gauntlet );
SCREEN_UPDATE_IND16( gauntlet );
