/***************************************************************************

    Green Beret

***************************************************************************/

class gberet_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, gberet_state(machine)); }

	gberet_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *     videoram;
	UINT8 *     colorram;
	UINT8 *     spriteram;
	UINT8 *     spriteram2;
	UINT8 *     scrollram;
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *bg_tilemap;
	UINT8       spritebank;

	/* misc */
	UINT8       nmi_enable, irq_enable;
};


/*----------- defined in video/gberet.c -----------*/

WRITE8_HANDLER( gberet_videoram_w );
WRITE8_HANDLER( gberet_colorram_w );
WRITE8_HANDLER( gberet_scroll_w );
WRITE8_HANDLER( gberetb_scroll_w );
WRITE8_HANDLER( gberet_sprite_bank_w );

PALETTE_INIT( gberet );
VIDEO_START( gberet );
VIDEO_UPDATE( gberet );
VIDEO_UPDATE( gberetb );
