/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/


#define CPUTAG_MCU "mcu"


class champbas_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, champbas_state(machine)); }

	champbas_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *        bg_videoram;
	UINT8 *        spriteram;
	UINT8 *        spriteram_2;
	size_t         spriteram_size;

	/* video-related */
	tilemap_t        *bg_tilemap;
	UINT8          gfx_bank;
	UINT8          palette_bank;

	/* misc */
	int            watchdog_count;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mcu;
};


/*----------- defined in video/champbas.c -----------*/

WRITE8_HANDLER( champbas_bg_videoram_w );
WRITE8_HANDLER( champbas_gfxbank_w );
WRITE8_HANDLER( champbas_palette_bank_w );
WRITE8_HANDLER( champbas_flipscreen_w );

PALETTE_INIT( champbas );
PALETTE_INIT( exctsccr );
VIDEO_START( champbas );
VIDEO_START( exctsccr );
VIDEO_UPDATE( champbas );
VIDEO_UPDATE( exctsccr );


