/***************************************************************************

 Espial hardware games (drivers: espial.c, marineb.c and zodiack.c)

***************************************************************************/

class espial_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, espial_state(machine)); }

	espial_state(running_machine &machine)
		: driver_data_t(machine) { }

	UINT8 *   videoram;	// espial, zodiack, marineb
	UINT8 *   colorram;	// espial, marineb
	UINT8 *   attributeram;	// espial, zodiack
	UINT8 *   scrollram;	// espial
	UINT8 *   spriteram_1;	// espial
	UINT8 *   spriteram_2;	// espial
	UINT8 *   spriteram_3;	// espial
	UINT8 *   spriteram;	// zodiack, marineb (hoccer only)
	UINT8 *   videoram_2;	// zodiack
	UINT8 *   bulletsram;	// zodiack
	size_t    videoram_size;	// zodiack
	size_t    spriteram_size;	// zodiack
	size_t    bulletsram_size;	// zodiack

	/* video-related */
	tilemap_t   *bg_tilemap, *fg_tilemap;
	int       flipscreen;	// espial
	UINT8     palette_bank;	// marineb
	UINT8     column_scroll;	// marineb
	UINT8     flipscreen_x, flipscreen_y;	// marineb
	UINT8     marineb_active_low_flipscreen;	// marineb

	/* sound-related */
	UINT8     sound_nmi_enabled;	// espial

	/* misc */
	int       percuss_hardware;	// zodiack

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in drivers/espial.c -----------*/

MACHINE_RESET( espial );
MACHINE_START( espial );

WRITE8_HANDLER( zodiac_master_interrupt_enable_w );
INTERRUPT_GEN( zodiac_master_interrupt );
WRITE8_HANDLER( zodiac_master_soundlatch_w );
WRITE8_HANDLER( espial_sound_nmi_enable_w );
INTERRUPT_GEN( espial_sound_nmi_gen );


/*----------- defined in video/espial.c -----------*/

PALETTE_INIT( espial );
VIDEO_START( espial );
VIDEO_START( netwars );
WRITE8_HANDLER( espial_videoram_w );
WRITE8_HANDLER( espial_colorram_w );
WRITE8_HANDLER( espial_attributeram_w );
WRITE8_HANDLER( espial_scrollram_w );
WRITE8_HANDLER( espial_flipscreen_w );
VIDEO_UPDATE( espial );


/*----------- defined in video/marineb.c -----------*/

WRITE8_HANDLER( marineb_videoram_w );
WRITE8_HANDLER( marineb_colorram_w );
WRITE8_HANDLER( marineb_column_scroll_w );
WRITE8_HANDLER( marineb_palette_bank_0_w );
WRITE8_HANDLER( marineb_palette_bank_1_w );
WRITE8_HANDLER( marineb_flipscreen_x_w );
WRITE8_HANDLER( marineb_flipscreen_y_w );

VIDEO_START( marineb );
VIDEO_UPDATE( marineb );
VIDEO_UPDATE( changes );
VIDEO_UPDATE( springer );
VIDEO_UPDATE( hoccer );
VIDEO_UPDATE( hopprobo );


/*----------- defined in video/zodiack.c -----------*/

WRITE8_HANDLER( zodiack_videoram_w );
WRITE8_HANDLER( zodiack_videoram2_w );
WRITE8_HANDLER( zodiack_attributes_w );
WRITE8_HANDLER( zodiack_flipscreen_w );

PALETTE_INIT( zodiack );
VIDEO_START( zodiack );
VIDEO_UPDATE( zodiack );

