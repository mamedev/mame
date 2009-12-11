/***************************************************************************

    Track'n'Field - Hyper Sports - Yie Ar Kung-Fu - Super Basketball
     (these drivers share sound hardware handling)

***************************************************************************/

#include "sound/msm5205.h"


typedef struct _trackfld_state trackfld_state;
struct _trackfld_state
{
	/* memory pointers */
	UINT8 *  videoram;	// trackfld, hyperspt, yiear, sbasketb
	UINT8 *  colorram;	// trackfld, hyperspt, sbasketb
	UINT8 *  scroll;		// trackfld, hyperspt
	UINT8 *  scroll2;		// trackfld
	UINT8 *  spriteram;
	UINT8 *  spriteram2;
//  UINT8 *  nvram;     // currently this uses generic nvram handling (trackfld & hyperspt)
	size_t   spriteram_size;
	UINT8 *  palettebank;		// sbasketb
	UINT8 *  spriteram_select;	// sbasketb

	/* video-related */
	tilemap  *bg_tilemap;
	int      bg_bank, sprite_bank1, sprite_bank2;	// trackfld
	int      old_gfx_bank;					// needed by atlantol


	/* sound-related */
	int      SN76496_latch;
	int      last_addr;
	int      last_irq;

	/* game specific */
	UINT8    hyprolyb_adpcm_ready;	// only bootlegs
	UINT8    hyprolyb_adpcm_busy;
	UINT8    hyprolyb_vck_ready;
	int      yiear_nmi_enable;		// yiear

	/* devices */
	const device_config *audiocpu;
	const device_config *vlm;
};


/*----------- defined in audio/trackfld.c -----------*/

WRITE8_HANDLER( konami_sh_irqtrigger_w );
READ8_HANDLER( trackfld_sh_timer_r );
READ8_DEVICE_HANDLER( trackfld_speech_r );
WRITE8_DEVICE_HANDLER( trackfld_sound_w );
READ8_HANDLER( hyperspt_sh_timer_r );
WRITE8_DEVICE_HANDLER( hyperspt_sound_w );
WRITE8_HANDLER( konami_SN76496_latch_w );
WRITE8_DEVICE_HANDLER( konami_SN76496_w );


/*----------- defined in drivers/trackfld.c -----------*/
/*-------------- (needed by hypersptb) ----------------*/

extern const msm5205_interface hyprolyb_msm5205_config;
extern WRITE8_HANDLER( hyprolyb_adpcm_w );
ADDRESS_MAP_EXTERN( hyprolyb_adpcm_map, 8 );


/*----------- defined in video/trackfld.c -----------*/

WRITE8_HANDLER( trackfld_videoram_w );
WRITE8_HANDLER( trackfld_colorram_w );
WRITE8_HANDLER( trackfld_flipscreen_w );
WRITE8_HANDLER( atlantol_gfxbank_w );

PALETTE_INIT( trackfld );
VIDEO_START( trackfld );
VIDEO_UPDATE( trackfld );


/*----------- defined in video/hyperspt.c -----------*/

WRITE8_HANDLER( hyperspt_videoram_w );
WRITE8_HANDLER( hyperspt_colorram_w );
WRITE8_HANDLER( hyperspt_flipscreen_w );

PALETTE_INIT( hyperspt );
VIDEO_START( hyperspt );
VIDEO_UPDATE( hyperspt );
VIDEO_START( roadf );


/*----------- defined in video/sbasketb.c -----------*/

WRITE8_HANDLER( sbasketb_videoram_w );
WRITE8_HANDLER( sbasketb_colorram_w );
WRITE8_HANDLER( sbasketb_flipscreen_w );

PALETTE_INIT( sbasketb );
VIDEO_START( sbasketb );
VIDEO_UPDATE( sbasketb );


/*----------- defined in video/yiear.c -----------*/

WRITE8_HANDLER( yiear_videoram_w );
WRITE8_HANDLER( yiear_control_w );

PALETTE_INIT( yiear );
VIDEO_START( yiear );
VIDEO_UPDATE( yiear );
