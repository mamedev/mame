

/* this is compied from Exerion, but it should be correct */
#define FCOMBAT_MASTER_CLOCK        (20000000)
#define FCOMBAT_CPU_CLOCK           (FCOMBAT_MASTER_CLOCK / 6)
#define FCOMBAT_AY8910_CLOCK        (FCOMBAT_CPU_CLOCK / 2)
#define FCOMBAT_PIXEL_CLOCK         (FCOMBAT_MASTER_CLOCK / 3)
#define FCOMBAT_HCOUNT_START        (0x58)
#define FCOMBAT_HTOTAL              (512-FCOMBAT_HCOUNT_START)
#define FCOMBAT_HBEND               (12*8)	/* ?? */
#define FCOMBAT_HBSTART             (52*8)	/* ?? */
#define FCOMBAT_VTOTAL              (256)
#define FCOMBAT_VBEND               (16)
#define FCOMBAT_VBSTART             (240)

#define BACKGROUND_X_START		32
#define BACKGROUND_X_START_FLIP	72

#define VISIBLE_X_MIN			(12*8)
#define VISIBLE_X_MAX			(52*8)
#define VISIBLE_Y_MIN			(2*8)
#define VISIBLE_Y_MAX			(30*8)


typedef struct _fcombat_state fcombat_state;
struct _fcombat_state
{
	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    spriteram;
	size_t     videoram_size;
	size_t     spriteram_size;

	/* video-related */
	tilemap    *bgmap;
	UINT8      cocktail_flip;
	UINT8      char_palette, sprite_palette;
	UINT8      char_bank;

	/* misc */
	int        fcombat_sh;
	int        fcombat_sv;
	int        tx, ty;

	/* devices */
	const device_config *maincpu;
};



/*----------- defined in video/fcombat.c -----------*/

PALETTE_INIT( fcombat );
VIDEO_START( fcombat );
VIDEO_UPDATE( fcombat );

WRITE8_HANDLER( fcombat_videoreg_w );
