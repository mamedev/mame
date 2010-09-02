/*************************************************************************

    Moero Pro Yakyuu Homerun & Dynamic Shooting

*************************************************************************/

class homerun_state : public driver_device
{
public:
	homerun_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *tilemap;
	int        gfx_ctrl;

	/* misc */
	int        xpa, xpb, xpc;
	int        gc_up, gc_down;
};


/*----------- defined in video/homerun.c -----------*/

WRITE8_HANDLER( homerun_videoram_w );
WRITE8_HANDLER( homerun_color_w );
WRITE8_DEVICE_HANDLER( homerun_banking_w );

VIDEO_START(homerun);
VIDEO_UPDATE(homerun);
