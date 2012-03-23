/***************************************************************************

    Zaccaria Quasar

****************************************************************************/

#include "includes/cvs.h"

class quasar_state : public cvs_state
{
public:
	quasar_state(const machine_config &mconfig, device_type type, const char *tag)
		: cvs_state(mconfig, type, tag) { }

	UINT8 *    m_effectram;
	UINT8      m_effectcontrol;
	UINT8      m_page;
	UINT8      m_io_page;
};


/*----------- defined in video/quasar.c -----------*/

PALETTE_INIT( quasar );
SCREEN_UPDATE_IND16( quasar );
VIDEO_START( quasar );
