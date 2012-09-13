/*************************************************************************

    Atari "Round" hardware

*************************************************************************/

#include "machine/atarigen.h"

class relief_state : public atarigen_state
{
public:
	relief_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag) { }

	UINT8			m_ym2413_volume;
	UINT8			m_overall_volume;
	UINT32			m_adpcm_bank_base;
	DECLARE_READ16_MEMBER(relief_atarivc_r);
	DECLARE_WRITE16_MEMBER(relief_atarivc_w);
	DECLARE_READ16_MEMBER(special_port2_r);
	DECLARE_WRITE16_MEMBER(audio_control_w);
	DECLARE_WRITE16_MEMBER(audio_volume_w);
	DECLARE_DRIVER_INIT(relief);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	TILE_GET_INFO_MEMBER(get_playfield2_tile_info);
	DECLARE_MACHINE_START(relief);
	DECLARE_MACHINE_RESET(relief);
	DECLARE_VIDEO_START(relief);
};


/*----------- defined in video/relief.c -----------*/


SCREEN_UPDATE_IND16( relief );
