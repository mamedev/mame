/*************************************************************************

    Pushman

*************************************************************************/

class pushman_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, pushman_state(machine)); }

	pushman_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT16 *   videoram;
	UINT16 *   spriteram;
//  UINT16 *   paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *bg_tilemap, *tx_tilemap;
	UINT16     control[2];

	/* misc */
	UINT8      shared_ram[8];
	UINT16     latch, new_latch;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mcu;
};


/*----------- defined in video/pushman.c -----------*/

WRITE16_HANDLER( pushman_scroll_w );
WRITE16_HANDLER( pushman_videoram_w );

VIDEO_START( pushman );

VIDEO_UPDATE( pushman );
