/*************************************************************************

    Atari Cops'n Robbers hardware

*************************************************************************/

#include "sound/discrete.h"


class copsnrob_state : public driver_device
{
public:
	copsnrob_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *        videoram;
	UINT8 *        trucky;
	UINT8 *        truckram;
	UINT8 *        bulletsram;
	UINT8 *        cary;
	UINT8 *        carimage;
	size_t         videoram_size;

	/* misc */
	UINT8          misc;
	UINT8          ic_h3_data;
};


/*----------- defined in machine/copsnrob.c -----------*/

READ8_HANDLER( copsnrob_gun_position_r );


/*----------- defined in video/copsnrob.c -----------*/

VIDEO_UPDATE( copsnrob );


/*----------- defined in audio/copsnrob.c -----------*/

DISCRETE_SOUND_EXTERN( copsnrob );
WRITE8_HANDLER( copsnrob_misc_w );
