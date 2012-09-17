/*************************************************************************

    Atari Toobin' hardware

*************************************************************************/

#include "machine/atarigen.h"

class toobin_state : public atarigen_state
{
public:
	toobin_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
		  m_interrupt_scan(*this, "interrupt_scan") { }

	required_shared_ptr<UINT16> m_interrupt_scan;

	double			m_brightness;
	bitmap_ind16 m_pfbitmap;
	DECLARE_WRITE16_MEMBER(interrupt_scan_w);
	DECLARE_READ16_MEMBER(special_port1_r);
	DECLARE_DRIVER_INIT(toobin);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(toobin);
	DECLARE_MACHINE_RESET(toobin);
	DECLARE_VIDEO_START(toobin);
	UINT32 screen_update_toobin(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/toobin.c -----------*/

DECLARE_WRITE16_HANDLER( toobin_paletteram_w );
DECLARE_WRITE16_HANDLER( toobin_intensity_w );
DECLARE_WRITE16_HANDLER( toobin_xscroll_w );
DECLARE_WRITE16_HANDLER( toobin_yscroll_w );
DECLARE_WRITE16_HANDLER( toobin_slip_w );



