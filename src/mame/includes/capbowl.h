/*************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

*************************************************************************/

class capbowl_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, capbowl_state(machine)); }

	capbowl_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *  rowaddress;
//  UINT8 *  nvram; // currently this uses generic_nvram

	/* video-related */
	offs_t blitter_addr;

	/* input-related */
	UINT8 last_trackball_val[2];

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/capbowl.c -----------*/

VIDEO_START( capbowl );
VIDEO_UPDATE( capbowl );

WRITE8_HANDLER( bowlrama_blitter_w );
READ8_HANDLER( bowlrama_blitter_r );

WRITE8_HANDLER( capbowl_tms34061_w );
READ8_HANDLER( capbowl_tms34061_r );
