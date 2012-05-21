#include "sound/discrete.h"

class skyraid_state : public driver_device
{
public:
	skyraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_pos_ram(*this, "pos_ram"),
		m_alpha_num_ram(*this, "alpha_num_ram"),
		m_obj_ram(*this, "obj_ram"){ }

	int m_analog_range;
	int m_analog_offset;

	int m_scroll;

	required_shared_ptr<UINT8> m_pos_ram;
	required_shared_ptr<UINT8> m_alpha_num_ram;
	required_shared_ptr<UINT8> m_obj_ram;

	bitmap_ind16 m_helper;
	DECLARE_READ8_MEMBER(skyraid_port_0_r);
	DECLARE_WRITE8_MEMBER(skyraid_range_w);
	DECLARE_WRITE8_MEMBER(skyraid_offset_w);
	DECLARE_WRITE8_MEMBER(skyraid_scroll_w);
};


/*----------- defined in audio/skyraid.c -----------*/

DISCRETE_SOUND_EXTERN( skyraid );

WRITE8_DEVICE_HANDLER( skyraid_sound_w );


/*----------- defined in video/skyraid.c -----------*/

VIDEO_START(skyraid);
SCREEN_UPDATE_IND16(skyraid);
