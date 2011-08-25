/*************************************************************************

    Atari G1 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarig1_state : public atarigen_state
{
public:
	atarig1_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT8			m_is_pitfight;

	UINT8			m_which_input;
	UINT16 *		m_mo_command;

	UINT16 *		m_bslapstic_base;
	void *			m_bslapstic_bank0;
	UINT8			m_bslapstic_bank;
	UINT8			m_bslapstic_primed;

	int 			m_pfscroll_xoffset;
	UINT16			m_current_control;
	UINT8			m_playfield_tile_bank;
	UINT16			m_playfield_xscroll;
	UINT16			m_playfield_yscroll;

	device_t *		m_rle;
};


/*----------- defined in video/atarig1.c -----------*/

WRITE16_HANDLER( atarig1_mo_control_w );

VIDEO_START( atarig1 );
SCREEN_EOF( atarig1 );
SCREEN_UPDATE( atarig1 );

void atarig1_scanline_update(screen_device &screen, int scanline);
