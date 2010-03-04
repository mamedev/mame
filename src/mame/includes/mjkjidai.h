class mjkjidai_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mjkjidai_state(machine)); }

	mjkjidai_state(running_machine &machine) { }

	UINT8 *videoram;
	UINT8 *spriteram1;
	UINT8 *spriteram2;
	UINT8 *spriteram3;

	int keyb;
	int nvram_init_count;
	UINT8 *nvram;
	size_t nvram_size;
};


/*----------- defined in video/mjkjidai.c -----------*/

VIDEO_START( mjkjidai );
VIDEO_UPDATE( mjkjidai );
WRITE8_HANDLER( mjkjidai_videoram_w );
WRITE8_HANDLER( mjkjidai_ctrl_w );


