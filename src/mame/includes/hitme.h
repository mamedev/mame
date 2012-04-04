/*************************************************************************

    Hitme hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define HITME_DOWNCOUNT_VAL      NODE_01
#define HITME_OUT0               NODE_02
#define HITME_ENABLE_VAL         NODE_03
#define HITME_OUT1               NODE_04

class hitme_state : public driver_device
{
public:
	hitme_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_videoram;

	/* video-related */
	tilemap_t  *m_tilemap;

	/* misc */
	attotime m_timeout_time;
	DECLARE_WRITE8_MEMBER(hitme_vidram_w);
	DECLARE_READ8_MEMBER(hitme_port_0_r);
	DECLARE_READ8_MEMBER(hitme_port_1_r);
	DECLARE_READ8_MEMBER(hitme_port_2_r);
	DECLARE_READ8_MEMBER(hitme_port_3_r);
};


/*----------- defined in audio/hitme.c -----------*/

DISCRETE_SOUND_EXTERN( hitme );
