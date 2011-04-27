/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

class offtwall_state : public atarigen_state
{
public:
	offtwall_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16 *m_bankswitch_base;
	UINT16 *m_bankrom_base;
	UINT32 m_bank_offset;

	UINT16 *m_spritecache_count;
	UINT16 *m_unknown_verify_base;
};


/*----------- defined in video/offtwall.c -----------*/

VIDEO_START( offtwall );
SCREEN_UPDATE( offtwall );
