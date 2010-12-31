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


class othunder_state : public driver_device
{
public:
	othunder_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
	device_t *maincpu;
	device_t *audiocpu;
	device_t *eeprom;
	device_t *tc0220ioc;
	device_t *tc0100scn;
	device_t *tc0110pcr;
	device_t *tc0140syt;
	device_t *_2610_0l;
	device_t *_2610_0r;
	device_t *_2610_1l;
	device_t *_2610_1r;
	device_t *_2610_2l;
	device_t *_2610_2r;
};


/*----------- defined in video/othunder.c -----------*/

VIDEO_START( othunder );
VIDEO_UPDATE( othunder );
