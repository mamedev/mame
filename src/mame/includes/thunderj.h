/*************************************************************************

    Atari ThunderJaws hardware

*************************************************************************/

#include "machine/atarigen.h"

class thunderj_state : public atarigen_state
{
public:
	thunderj_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT8           m_alpha_tile_bank;
	virtual void update_interrupts();
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_WRITE16_MEMBER(latch_w);
	DECLARE_READ16_MEMBER(thunderj_atarivc_r);
	DECLARE_WRITE16_MEMBER(thunderj_atarivc_w);
	DECLARE_DRIVER_INIT(thunderj);
	TILE_GET_INFO_MEMBER(get_alpha_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	DECLARE_MACHINE_START(thunderj);
	DECLARE_MACHINE_RESET(thunderj);
	DECLARE_VIDEO_START(thunderj);
	UINT32 screen_update_thunderj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
