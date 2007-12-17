/*************************************************************************

    Driver for early Williams games

**************************************************************************/


#include "machine/6821pia.h"

/*----------- defined in drivers/williams.c -----------*/

void defender_install_io_space(void);


/*----------- defined in machine/williams.c -----------*/

/* Generic old-Williams PIA interfaces */
extern const pia6821_interface williams_pia_0_intf;
extern const pia6821_interface williams_muxed_pia_0_intf;
extern const pia6821_interface williams_dual_muxed_pia_0_intf;
extern const pia6821_interface williams_49way_pia_0_intf;
extern const pia6821_interface williams_49way_muxed_pia_0_intf;
extern const pia6821_interface williams_pia_1_intf;
extern const pia6821_interface williams_snd_pia_intf;

/* Game-specific old-Williams PIA interfaces */
extern const pia6821_interface lottofun_pia_0_intf;
extern const pia6821_interface sinistar_snd_pia_intf;
extern const pia6821_interface playball_pia_1_intf;
extern const pia6821_interface spdball_pia_3_intf;

/* Generic later-Williams PIA interfaces */
extern const pia6821_interface williams2_muxed_pia_0_intf;
extern const pia6821_interface williams2_pia_1_intf;
extern const pia6821_interface williams2_snd_pia_intf;

/* Game-specific later-Williams PIA interfaces */
extern const pia6821_interface mysticm_pia_0_intf;
extern const pia6821_interface tshoot_pia_0_intf;
extern const pia6821_interface tshoot_snd_pia_intf;
extern const pia6821_interface joust2_pia_1_intf;

/* initialization */
MACHINE_RESET( defender );
MACHINE_RESET( williams );
MACHINE_RESET( blaster );
MACHINE_RESET( williams2 );
MACHINE_START( joust2 );
MACHINE_RESET( joust2 );

/* banking */
WRITE8_HANDLER( defender_bank_select_w );
WRITE8_HANDLER( williams_vram_select_w );
WRITE8_HANDLER( sinistar_vram_select_w );
WRITE8_HANDLER( blaster_bank_select_w );
WRITE8_HANDLER( blaster_vram_select_w );
WRITE8_HANDLER( williams2_bank_select_w );

/* misc */
WRITE8_HANDLER( williams_cmos_w );
WRITE8_HANDLER( bubbles_cmos_w );
WRITE8_HANDLER( williams_watchdog_reset_w );
WRITE8_HANDLER( williams2_watchdog_reset_w );
WRITE8_HANDLER( williams2_7segment_w );

/* Mayday protection */
extern UINT8 *mayday_protection;
READ8_HANDLER( mayday_protection_r );

WRITE8_HANDLER( defender_video_control_w );

/*----------- defined in video/williams.c -----------*/

#define WILLIAMS_BLITTER_NONE		0		/* no blitter */
#define WILLIAMS_BLITTER_SC01		1		/* SC-01 blitter */
#define WILLIAMS_BLITTER_SC02		2		/* SC-02 "fixed" blitter */

#define WILLIAMS_TILEMAP_MYSTICM	0		/* IC79 is a 74LS85 comparator */
#define WILLIAMS_TILEMAP_TSHOOT		1		/* IC79 is a 74LS157 selector jumpered to be enabled */
#define WILLIAMS_TILEMAP_JOUST2		2		/* IC79 is a 74LS157 selector jumpered to be disabled */

/* RAM globals */
extern UINT8 *williams_videoram;
extern UINT8 *williams2_tileram;
extern UINT8 *blaster_palette_0;
extern UINT8 *blaster_scanline_control;

/* blitter globals */
extern UINT8 williams_blitter_config;
extern UINT16 williams_blitter_clip_address;
extern UINT8 williams_blitter_window_enable;

/* tilemap globals */
extern UINT8 williams2_tilemap_config;

/* rendering globals */
extern UINT8 williams_cocktail;


WRITE8_HANDLER( williams_blitter_w );
WRITE8_HANDLER( williams_paletteram_w );
WRITE8_HANDLER( blaster_remap_select_w );
WRITE8_HANDLER( blaster_palette_0_w );
WRITE8_HANDLER( blaster_video_control_w );
WRITE8_HANDLER( blaster_scanline_control_w );
READ8_HANDLER( williams_video_counter_r );
READ8_HANDLER( williams2_video_counter_r );

VIDEO_START( williams );
VIDEO_START( blaster );
VIDEO_START( williams2 );

VIDEO_UPDATE( williams );
VIDEO_UPDATE( blaster );
VIDEO_UPDATE( williams2 );


WRITE8_HANDLER( williams2_tileram_w );
READ8_HANDLER( williams2_paletteram_r );
WRITE8_HANDLER( williams2_paletteram_w );
WRITE8_HANDLER( williams2_fg_select_w );
WRITE8_HANDLER( williams2_bg_select_w );
WRITE8_HANDLER( williams2_xscroll_low_w );
WRITE8_HANDLER( williams2_xscroll_high_w );
WRITE8_HANDLER( williams2_blit_window_enable_w );
