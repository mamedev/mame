/***************************************************************************

 Espial hardware games

***************************************************************************/

/*----------- defined in drivers/espial.c -----------*/

MACHINE_RESET( espial );

WRITE8_HANDLER( zodiac_master_interrupt_enable_w );
INTERRUPT_GEN( zodiac_master_interrupt );
WRITE8_HANDLER( zodiac_master_soundlatch_w );
WRITE8_HANDLER( espial_sound_nmi_enable_w );
INTERRUPT_GEN( espial_sound_nmi_gen );


/*----------- defined in video/espial.c -----------*/

extern UINT8 *espial_videoram;
extern UINT8 *espial_colorram;
extern UINT8 *espial_attributeram;
extern UINT8 *espial_scrollram;
extern UINT8 *espial_spriteram_1;
extern UINT8 *espial_spriteram_2;
extern UINT8 *espial_spriteram_3;

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

extern UINT8 *marineb_videoram;
extern UINT8 *marineb_colorram;
extern UINT8 marineb_active_low_flipscreen;

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


/*----------- defined in drivers/zodiack.c -----------*/

extern int percuss_hardware;

/*----------- defined in video/zodiack.c -----------*/

extern UINT8 *zodiack_videoram2;
extern UINT8 *zodiack_attributesram;
extern UINT8 *zodiack_bulletsram;
extern size_t zodiack_bulletsram_size;

WRITE8_HANDLER( zodiack_videoram_w );
WRITE8_HANDLER( zodiack_videoram2_w );
WRITE8_HANDLER( zodiack_attributes_w );
WRITE8_HANDLER( zodiack_flipscreen_w );

PALETTE_INIT( zodiack );
VIDEO_START( zodiack );
VIDEO_UPDATE( zodiack );

