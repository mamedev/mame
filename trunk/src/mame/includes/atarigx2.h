/*************************************************************************

    Atari GX2 hardware

*************************************************************************/

#include "machine/atarigen.h"

class atarigx2_state : public atarigen_state
{
public:
	atarigx2_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16			m_playfield_base;

	UINT32 *		m_mo_command;
	UINT32 *		m_protection_base;

	UINT16			m_current_control;
	UINT8			m_playfield_tile_bank;
	UINT8			m_playfield_color_bank;
	UINT16			m_playfield_xscroll;
	UINT16			m_playfield_yscroll;

	UINT16			m_last_write;
	UINT16			m_last_write_offset;

	device_t *		m_rle;
};


/*----------- defined in video/atarigx2.c -----------*/

VIDEO_START( atarigx2 );
SCREEN_EOF( atarigx2 );
SCREEN_UPDATE( atarigx2 );

WRITE16_HANDLER( atarigx2_mo_control_w );

void atarigx2_scanline_update(screen_device &screen, int scanline);
