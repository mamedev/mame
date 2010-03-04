/*************************************************************************

    Epos games

**************************************************************************/

class epos_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, epos_state(machine)); }

	epos_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  videoram;
	size_t   videoram_size;

	/* video-related */
	UINT8    palette;

	/* misc */
	int      counter;
};


/*----------- defined in video/epos.c -----------*/

WRITE8_HANDLER( epos_port_1_w );
VIDEO_UPDATE( epos );
