struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int pri;
};

class groundfx_state : public driver_device
{
public:
	groundfx_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 coin_word;
	UINT16 frame_counter;
	UINT16 port_sel;
	UINT32 *ram;
	struct tempsprite *spritelist;
	UINT16 rotate_ctrl[8];
	rectangle hack_cliprect;
};


/*----------- defined in video/groundfx.c -----------*/

VIDEO_START( groundfx );
SCREEN_UPDATE( groundfx );
