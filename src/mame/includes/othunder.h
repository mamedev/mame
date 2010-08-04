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


class othunder_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, othunder_state(machine)); }

	othunder_state(running_machine &machine)
		: driver_data_t(machine) { }

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
	running_device *maincpu;
	running_device *audiocpu;
	running_device *eeprom;
	running_device *tc0220ioc;
	running_device *tc0100scn;
	running_device *tc0110pcr;
	running_device *tc0140syt;
	running_device *_2610_0l;
	running_device *_2610_0r;
	running_device *_2610_1l;
	running_device *_2610_1r;
	running_device *_2610_2l;
	running_device *_2610_2r;
};


/*----------- defined in video/othunder.c -----------*/

VIDEO_START( othunder );
VIDEO_UPDATE( othunder );
