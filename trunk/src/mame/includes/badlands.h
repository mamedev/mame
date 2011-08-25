/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/

#include "machine/atarigen.h"

class badlands_state : public atarigen_state
{
public:
	badlands_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT8			m_pedal_value[2];

	UINT8 *			m_bank_base;
	UINT8 *			m_bank_source_data;

	UINT8			m_playfield_tile_bank;
};


/*----------- defined in video/badlands.c -----------*/

WRITE16_HANDLER( badlands_pf_bank_w );

VIDEO_START( badlands );
SCREEN_UPDATE( badlands );
