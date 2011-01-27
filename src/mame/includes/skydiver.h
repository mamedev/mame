/*************************************************************************

    Atari Skydiver hardware

*************************************************************************/

#include "sound/discrete.h"

/* Discrete Sound Input Nodes */
#define SKYDIVER_RANGE_DATA		NODE_01
#define SKYDIVER_NOTE_DATA		NODE_02
#define SKYDIVER_RANGE3_EN		NODE_03
#define SKYDIVER_NOISE_DATA		NODE_04
#define SKYDIVER_NOISE_RST		NODE_05
#define SKYDIVER_WHISTLE1_EN	NODE_06
#define SKYDIVER_WHISTLE2_EN	NODE_07
#define SKYDIVER_OCT1_EN		NODE_08
#define SKYDIVER_OCT2_EN		NODE_09
#define SKYDIVER_SOUND_EN		NODE_10


class skydiver_state : public driver_device
{
public:
	skydiver_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int nmion;
	UINT8 *videoram;
	tilemap_t *bg_tilemap;
	int width;
};


/*----------- defined in audio/skydiver.c -----------*/

DISCRETE_SOUND_EXTERN( skydiver );


/*----------- defined in video/skydiver.c -----------*/

MACHINE_RESET( skydiver );
WRITE8_HANDLER( skydiver_videoram_w );
WRITE8_HANDLER( skydiver_wram_w );	/* the signal is WRAM, presumably Work RAM */
READ8_HANDLER( skydiver_wram_r );
WRITE8_HANDLER( skydiver_start_lamp_1_w );
WRITE8_HANDLER( skydiver_start_lamp_2_w );
WRITE8_HANDLER( skydiver_lamp_s_w );
WRITE8_HANDLER( skydiver_lamp_k_w );
WRITE8_HANDLER( skydiver_lamp_y_w );
WRITE8_HANDLER( skydiver_lamp_d_w );
WRITE8_HANDLER( skydiver_2000_201F_w );
WRITE8_HANDLER( skydiver_width_w );
WRITE8_HANDLER( skydiver_coin_lockout_w );
VIDEO_START( skydiver );
VIDEO_UPDATE( skydiver );
