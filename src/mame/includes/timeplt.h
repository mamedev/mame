/***************************************************************************

    Time Pilot
    Tutankham
    Pooyan
    Roc'n Rope
    Rally X (because of Loco-Motion, Jungler, etc.)

    These drivers share the sound emulation, and hence driver data struct

    Juno First (this shares Tutankham video code, but not Time Pilot sound hw)

***************************************************************************/

struct jungler_star
{
	int x, y, color;
};

#define MAX_STARS 1000

typedef struct _timeplt_state timeplt_state;
struct _timeplt_state
{
	/* memory pointers */
	UINT8 *  videoram;	// timeplt, tutankham, junofrst, rocnrope, pooyan, rallyx
	UINT8 *  colorram;	// timeplt, rocnrope, pooyan
	UINT8 *  spriteram;	// timeplt, rocnrope, pooyan, rallyx
	UINT8 *  spriteram2;	// timeplt, rocnrope, pooyan, rallyx
	UINT8 *  paletteram;	// tutankham, junofrst
	UINT8 *  scroll;		// tutankham, junofrst
	UINT8 *  radarattr;	// jungler
	UINT8 *  radarx;	// jungler
	UINT8 *  radary;	// jungler
//  UINT8 *  namco_soundregs;   // rallyx - these are dealt with in emu/sound/namco.c
	size_t   videoram_size;	// tutankham, junofrst
	size_t   spriteram_size;	// rocnrope

	/* video-related */
	tilemap  *bg_tilemap;
	tilemap  *fg_tilemap;		// jungler
	UINT8     blitterdata[4];	// junofrst
	UINT8     flip_x, flip_y;	// tutankham, junofrst

	/* sound-related */
	UINT8    last_irq_state;

	/* misc */
	UINT8    nmi_enable;	// timeplt
	UINT8    irq_toggle, irq_enable;	// tutankham, pooyan
	int      i8039_status, last_irq;	// junofrst
	int      last_bang;	// jungler
	int      spriteram_base, stars_enable, total_stars;	// jungler
	UINT8    drawmode_table[4];	// jungler
	struct jungler_star stars[MAX_STARS];	// jungler

	/* devices */
	const device_config *maincpu;
	const device_config *soundcpu;
	const device_config *i8039;		// junofrst
	const device_config *samples;		// rallyx
	const device_config *filter_0_0;
	const device_config *filter_0_1;
	const device_config *filter_0_2;
	const device_config *filter_1_0;
	const device_config *filter_1_1;
	const device_config *filter_1_2;
};


/*----------- defined in audio/timeplt.c -----------*/

WRITE8_HANDLER( timeplt_sh_irqtrigger_w );

MACHINE_DRIVER_EXTERN( timeplt_sound );
MACHINE_DRIVER_EXTERN( locomotn_sound );

/*----------- defined in video/pooyan.c -----------*/

WRITE8_HANDLER( pooyan_videoram_w );
WRITE8_HANDLER( pooyan_colorram_w );
WRITE8_HANDLER( pooyan_flipscreen_w );

PALETTE_INIT( pooyan );
VIDEO_START( pooyan );
VIDEO_UPDATE( pooyan );

/*----------- defined in video/rallyx.c -----------*/

WRITE8_HANDLER( rallyx_videoram_w );
WRITE8_HANDLER( rallyx_scrollx_w );
WRITE8_HANDLER( rallyx_scrolly_w );
WRITE8_HANDLER( tactcian_starson_w );

PALETTE_INIT( rallyx );
PALETTE_INIT( jungler );
VIDEO_START( rallyx );
VIDEO_START( jungler );
VIDEO_START( locomotn );
VIDEO_START( commsega );
VIDEO_UPDATE( rallyx );
VIDEO_UPDATE( jungler );
VIDEO_UPDATE( locomotn );

/*----------- defined in video/rocnrope.c -----------*/

WRITE8_HANDLER( rocnrope_videoram_w );
WRITE8_HANDLER( rocnrope_colorram_w );
WRITE8_HANDLER( rocnrope_flipscreen_w );

PALETTE_INIT( rocnrope );
VIDEO_START( rocnrope );
VIDEO_UPDATE( rocnrope );

/*----------- defined in video/timeplt.c -----------*/

READ8_HANDLER( timeplt_scanline_r );
WRITE8_HANDLER( timeplt_videoram_w );
WRITE8_HANDLER( timeplt_colorram_w );
WRITE8_HANDLER( timeplt_flipscreen_w );

PALETTE_INIT( timeplt );
VIDEO_START( timeplt );
VIDEO_UPDATE( timeplt );

/*----------- defined in video/tutankhm.c -----------*/

WRITE8_HANDLER( junofrst_blitter_w );

WRITE8_HANDLER( tutankhm_flip_screen_x_w );
WRITE8_HANDLER( tutankhm_flip_screen_y_w );

VIDEO_UPDATE( tutankhm );
