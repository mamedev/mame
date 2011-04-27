/*************************************************************************

    Atari Toobin' hardware

*************************************************************************/

#include "machine/atarigen.h"

class toobin_state : public atarigen_state
{
public:
	toobin_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16 *		m_interrupt_scan;

	double			m_brightness;
	bitmap_t *		m_pfbitmap;
};


/*----------- defined in video/toobin.c -----------*/

WRITE16_HANDLER( toobin_paletteram_w );
WRITE16_HANDLER( toobin_intensity_w );
WRITE16_HANDLER( toobin_xscroll_w );
WRITE16_HANDLER( toobin_yscroll_w );
WRITE16_HANDLER( toobin_slip_w );

VIDEO_START( toobin );
SCREEN_UPDATE( toobin );
