

class freekick_state : public driver_device
{
public:
	freekick_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *freek_tilemap;

	/* misc */
	int        inval, outval, cnt;	// used by oigas
	int        romaddr;
	int        spinner;
	int        nmi_en;
	int        ff_data;
};


/*----------- defined in video/freekick.c -----------*/

VIDEO_START(freekick);
SCREEN_UPDATE(gigas);
SCREEN_UPDATE(pbillrd);
SCREEN_UPDATE(freekick);
WRITE8_HANDLER( freek_videoram_w );
