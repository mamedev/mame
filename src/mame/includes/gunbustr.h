struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class gunbustr_state : public driver_device
{
public:
	gunbustr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 coin_word;
	UINT32 *ram;
	struct tempsprite *spritelist;
	UINT32 mem[2];
};


/*----------- defined in video/gunbustr.c -----------*/

VIDEO_START( gunbustr );
VIDEO_UPDATE( gunbustr );
