/*************************************************************************

    Driver for early Williams games

**************************************************************************/


#include "machine/6821pia.h"

class williams_state : public driver_device
{
public:
	williams_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
	UINT8 sound_int_state;
	UINT8 audio_talkback;
	UINT8 audio_sync;
	device_t *sound_cpu;
	device_t *soundalt_cpu;
	UINT8 *mayday_protection;
	UINT8 *videoram;
	UINT8 *williams2_tileram;
	UINT8 *blaster_palette_0;
	UINT8 *blaster_scanline_control;
	UINT8 blitter_config;
	UINT16 blitter_clip_address;
	UINT8 blitter_window_enable;
	UINT8 williams2_tilemap_config;
	UINT8 cocktail;
	UINT8 blaster_bank;
	UINT8 vram_bank;
	UINT16 joust2_current_sound_data;
	UINT8 port_select;
	rgb_t *palette_lookup;
	UINT8 blitterram[8];
	UINT8 blitter_xor;
	UINT8 blitter_remap_index;
	const UINT8 *blitter_remap;
	UINT8 *blitter_remap_lookup;
	rgb_t blaster_color0;
	UINT8 blaster_video_control;
	tilemap_t *bg_tilemap;
	UINT16 tilemap_xscroll;
	UINT8 williams2_fg_color;
};


/*----------- defined in drivers/williams.c -----------*/

void defender_install_io_space(address_space *space);


/*----------- defined in machine/williams.c -----------*/

/* Generic old-Williams PIA interfaces */
extern const pia6821_interface williams_pia_0_intf;
extern const pia6821_interface williams_muxed_pia_0_intf;
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
extern const pia6821_interface mysticm_pia_1_intf;
extern const pia6821_interface tshoot_pia_0_intf;
extern const pia6821_interface tshoot_pia_1_intf;
extern const pia6821_interface tshoot_snd_pia_intf;
extern const pia6821_interface joust2_pia_1_intf;

/* timer callbacks */
TIMER_DEVICE_CALLBACK( williams_va11_callback );
TIMER_DEVICE_CALLBACK( williams_count240_callback );
TIMER_DEVICE_CALLBACK( williams2_va11_callback );
TIMER_DEVICE_CALLBACK( williams2_endscreen_callback );

/* initialization */
MACHINE_START( defender );
MACHINE_RESET( defender );
MACHINE_START( williams );
MACHINE_RESET( williams );
MACHINE_START( blaster );
MACHINE_RESET( blaster );
MACHINE_START( williams2 );
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
CUSTOM_INPUT( williams_mux_r );

/* Mayday protection */
READ8_HANDLER( mayday_protection_r );

WRITE8_HANDLER( defender_video_control_w );

/*----------- defined in video/williams.c -----------*/

#define WILLIAMS_BLITTER_NONE		0		/* no blitter */
#define WILLIAMS_BLITTER_SC01		1		/* SC-01 blitter */
#define WILLIAMS_BLITTER_SC02		2		/* SC-02 "fixed" blitter */

#define WILLIAMS_TILEMAP_MYSTICM	0		/* IC79 is a 74LS85 comparator */
#define WILLIAMS_TILEMAP_TSHOOT		1		/* IC79 is a 74LS157 selector jumpered to be enabled */
#define WILLIAMS_TILEMAP_JOUST2		2		/* IC79 is a 74LS157 selector jumpered to be disabled */

WRITE8_HANDLER( williams_blitter_w );
WRITE8_HANDLER( blaster_remap_select_w );
WRITE8_HANDLER( blaster_video_control_w );
READ8_HANDLER( williams_video_counter_r );
READ8_HANDLER( williams2_video_counter_r );

VIDEO_START( williams );
VIDEO_START( blaster );
VIDEO_START( williams2 );

SCREEN_UPDATE( williams );
SCREEN_UPDATE( blaster );
SCREEN_UPDATE( williams2 );


WRITE8_HANDLER( williams2_tileram_w );
WRITE8_HANDLER( williams2_paletteram_w );
WRITE8_HANDLER( williams2_fg_select_w );
WRITE8_HANDLER( williams2_bg_select_w );
WRITE8_HANDLER( williams2_xscroll_low_w );
WRITE8_HANDLER( williams2_xscroll_high_w );
WRITE8_HANDLER( williams2_blit_window_enable_w );
