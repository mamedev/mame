/*************************************************************************

    Sega G-80 raster hardware

*************************************************************************/

#include "machine/segag80.h"

class segag80r_state : public driver_device
{
public:
	segag80r_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 sound_state[2];
	UINT8 sound_rate;
	UINT16 sound_addr;
	UINT8 sound_data;
	UINT8 square_state;
	UINT8 square_count;
	emu_timer *sega005_sound_timer;
	sound_stream *sega005_stream;
	UINT8 n7751_command;
	UINT8 n7751_busy;
	UINT8 *videoram;
	segag80_decrypt_func decrypt;
	UINT8 *mainram;
	UINT8 background_pcb;
	double rweights[3];
	double gweights[3];
	double bweights[2];
	UINT8 video_control;
	UINT8 video_flip;
	UINT8 vblank_latch;
	tilemap_t *spaceod_bg_htilemap;
	tilemap_t *spaceod_bg_vtilemap;
	UINT16 spaceod_hcounter;
	UINT16 spaceod_vcounter;
	UINT8 spaceod_fixed_color;
	UINT8 spaceod_bg_control;
	UINT8 spaceod_bg_detect;
	tilemap_t *bg_tilemap;
	UINT8 bg_enable;
	UINT8 bg_char_bank;
	UINT16 bg_scrollx;
	UINT16 bg_scrolly;
	UINT8 pignewt_bg_color_offset;
};


/*----------- defined in audio/segag80r.c -----------*/

MACHINE_CONFIG_EXTERN( astrob_sound_board );
MACHINE_CONFIG_EXTERN( 005_sound_board );
MACHINE_CONFIG_EXTERN( spaceod_sound_board );
MACHINE_CONFIG_EXTERN( monsterb_sound_board );

WRITE8_HANDLER( astrob_sound_w );

WRITE8_HANDLER( spaceod_sound_w );


/*----------- defined in video/segag80r.c -----------*/

#define G80_BACKGROUND_NONE			0
#define G80_BACKGROUND_SPACEOD		1
#define G80_BACKGROUND_MONSTERB		2
#define G80_BACKGROUND_PIGNEWT		3
#define G80_BACKGROUND_SINDBADM		4


INTERRUPT_GEN( segag80r_vblank_start );

WRITE8_HANDLER( segag80r_videoram_w );

READ8_HANDLER( segag80r_video_port_r );
WRITE8_HANDLER( segag80r_video_port_w );

VIDEO_START( segag80r );
SCREEN_UPDATE( segag80r );


READ8_HANDLER( spaceod_back_port_r );
WRITE8_HANDLER( spaceod_back_port_w );


WRITE8_HANDLER( monsterb_videoram_w );
WRITE8_HANDLER( monsterb_back_port_w );


WRITE8_HANDLER( pignewt_videoram_w );
WRITE8_HANDLER( pignewt_back_port_w );
WRITE8_HANDLER( pignewt_back_color_w );


INTERRUPT_GEN( sindbadm_vblank_start );

WRITE8_HANDLER( sindbadm_videoram_w );
WRITE8_HANDLER( sindbadm_back_port_w );
