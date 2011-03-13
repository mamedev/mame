struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class undrfire_state : public driver_device
{
public:
	undrfire_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 coin_word;
	UINT16 port_sel;
	int frame_counter;
	UINT32 *ram;
	UINT32 *shared_ram;
	struct tempsprite *spritelist;
	UINT16 rotate_ctrl[8];
	UINT8 dislayer[6];
	UINT32 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/undrfire.c -----------*/

VIDEO_START( undrfire );
SCREEN_UPDATE( undrfire );
SCREEN_UPDATE( cbombers );
