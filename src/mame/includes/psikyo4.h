/*************************************************************************

    Psikyo PS6807 (PS4)

*************************************************************************/

#define MASTER_CLOCK 57272700	// main oscillator frequency


typedef struct _psikyo4_state psikyo4_state;
struct _psikyo4_state
{
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
	const device_config *maincpu;
};


/*----------- defined in video/psikyo4.c -----------*/

VIDEO_START( psikyo4 );
VIDEO_UPDATE( psikyo4 );
