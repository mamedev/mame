/*************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

*************************************************************************/

#include "machine/nvram.h"

class capbowl_state : public driver_device
{
public:
	capbowl_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	void init_nvram(nvram_device &nvram, void *base, size_t size);

	/* memory pointers */
	UINT8 *  rowaddress;

	/* video-related */
	offs_t blitter_addr;

	/* input-related */
	UINT8 last_trackball_val[2];

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
};

/*----------- defined in video/capbowl.c -----------*/

VIDEO_START( capbowl );
VIDEO_UPDATE( capbowl );

WRITE8_HANDLER( bowlrama_blitter_w );
READ8_HANDLER( bowlrama_blitter_r );

WRITE8_HANDLER( capbowl_tms34061_w );
READ8_HANDLER( capbowl_tms34061_r );
