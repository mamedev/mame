/*************************************************************************

    Atari Cops'n Robbers hardware

*************************************************************************/

class copsnrob_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, copsnrob_state(machine)); }

	copsnrob_state(running_machine &machine) { }

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
};


/*----------- defined in machine/copsnrob.c -----------*/

READ8_HANDLER( copsnrob_gun_position_r );


/*----------- defined in video/copsnrob.c -----------*/

VIDEO_UPDATE( copsnrob );
