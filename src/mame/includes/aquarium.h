


class aquarium_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, aquarium_state(machine)); }

	aquarium_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT16 *  scroll;
	UINT16 *  txt_videoram;
	UINT16 *  mid_videoram;
	UINT16 *  bak_videoram;
	UINT16 *  spriteram;
//  UINT16 *  paletteram;   // currently this uses generic palette handling
	size_t    spriteram_size;

	/* video-related */
	tilemap_t  *txt_tilemap, *mid_tilemap, *bak_tilemap;

	/* misc */
	int aquarium_snd_ack;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/aquarium.c -----------*/

WRITE16_HANDLER( aquarium_txt_videoram_w );
WRITE16_HANDLER( aquarium_mid_videoram_w );
WRITE16_HANDLER( aquarium_bak_videoram_w );

VIDEO_START(aquarium);
VIDEO_UPDATE(aquarium);
