/*************************************************************************

    Psikyo PS6807 (PS4)

*************************************************************************/

#define MASTER_CLOCK 57272700	// main oscillator frequency


class psikyo4_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, psikyo4_state(machine)); }

	psikyo4_state(running_machine &machine) { }

	/* memory pointers */
	UINT32 *       vidregs;
	UINT32 *       paletteram;
	UINT32 *       ram;
	UINT32 *       io_select;
	UINT32 *       bgpen_1;
	UINT32 *       bgpen_2;
	UINT32 *       spriteram;
	size_t         spriteram_size;

	/* video-related */
	double         oldbrt1, oldbrt2;

	/* misc */
	UINT32         sample_offs;	// only used if ROMTEST = 1

	/* devices */
	running_device *maincpu;
};


/*----------- defined in video/psikyo4.c -----------*/

VIDEO_START( psikyo4 );
VIDEO_UPDATE( psikyo4 );
