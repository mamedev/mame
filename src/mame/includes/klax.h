/*************************************************************************

    Atari Klax hardware

*************************************************************************/

#include "machine/atarigen.h"

class klax_state : public atarigen_state
{
public:
	klax_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }
	DECLARE_WRITE16_MEMBER(interrupt_ack_w);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(klax);
	DECLARE_MACHINE_RESET(klax);
	DECLARE_VIDEO_START(klax);
};


/*----------- defined in video/klax.c -----------*/

DECLARE_WRITE16_HANDLER( klax_latch_w );


SCREEN_UPDATE_IND16( klax );
