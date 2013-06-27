/*************************************************************************

    Atari Batman hardware

*************************************************************************/

#include "machine/atarigen.h"

class batman_state : public atarigen_state
{
public:
	batman_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT16          m_latch_data;

	UINT8           m_alpha_tile_bank;
	virtual void update_interrupts();
	virtual void scanline_update(screen_device &screen, int scanline);
	DECLARE_READ16_MEMBER(batman_atarivc_r);
	DECLARE_WRITE16_MEMBER(batman_atarivc_w);
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_WRITE16_MEMBER(latch_w);
	DECLARE_DRIVER_INIT(batman);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	DECLARE_MACHINE_START(batman);
	DECLARE_MACHINE_RESET(batman);
	DECLARE_VIDEO_START(batman);
	UINT32 screen_update_batman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/*----------- defined in video/batman.c -----------*/
void batman_scanline_update(screen_device &screen, int scanline);
