/*************************************************************************

    Operation Thunderbolt

*************************************************************************/

struct othunder_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};


typedef struct _othunder_state othunder_state;
struct _othunder_state
{
	/* memory pointers */
	UINT16 *   spriteram;
	size_t     spriteram_size;

	/* video-related */
	struct othunder_tempsprite *spritelist;

	/* misc */
	int        vblank_irq, ad_irq;
	INT32      banknum;
	int        pan[4];

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *eeprom;
	const device_config *tc0220ioc;
	const device_config *tc0100scn;
	const device_config *tc0110pcr;
	const device_config *tc0140syt;
	const device_config *_2610_0l;
	const device_config *_2610_0r;
	const device_config *_2610_1l;
	const device_config *_2610_1r;
	const device_config *_2610_2l;
	const device_config *_2610_2r;
};


/*----------- defined in video/othunder.c -----------*/

VIDEO_START( othunder );
VIDEO_UPDATE( othunder );
