struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class superchs_state : public driver_device
{
public:
	superchs_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 coin_word;
	UINT32 *ram;
	UINT32 *shared_ram;
	int steer;
	struct tempsprite *spritelist;
	UINT32 mem[2];
};


/*----------- defined in video/superchs.c -----------*/

VIDEO_START( superchs );
VIDEO_UPDATE( superchs );
