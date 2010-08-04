
class lemmings_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, lemmings_state(machine)); }

	lemmings_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT16 *  pixel_0_data;
	UINT16 *  pixel_1_data;
	UINT16 *  vram_data;
	UINT16 *  control_data;
	UINT16 *  paletteram;
//  UINT16 *  spriteram;    // this currently uses generic buffered spriteram
//  UINT16 *  spriteram2;   // this currently uses generic buffered spriteram

	/* video-related */
	bitmap_t *bitmap0;
	tilemap_t *vram_tilemap;
	UINT16 *sprite_triple_buffer_0,*sprite_triple_buffer_1;
	UINT8 *vram_buffer;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/lemmings.c -----------*/

WRITE16_HANDLER( lemmings_pixel_0_w );
WRITE16_HANDLER( lemmings_pixel_1_w );
WRITE16_HANDLER( lemmings_vram_w );

VIDEO_START( lemmings );
VIDEO_EOF( lemmings );
VIDEO_UPDATE( lemmings );
