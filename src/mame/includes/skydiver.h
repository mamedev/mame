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
	skydiver_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_nmion;
	UINT8 *m_videoram;
	tilemap_t *m_bg_tilemap;
	int m_width;
	DECLARE_WRITE8_MEMBER(skydiver_nmion_w);
	DECLARE_WRITE8_MEMBER(skydiver_videoram_w);
	DECLARE_READ8_MEMBER(skydiver_wram_r);
	DECLARE_WRITE8_MEMBER(skydiver_wram_w);
	DECLARE_WRITE8_MEMBER(skydiver_width_w);
	DECLARE_WRITE8_MEMBER(skydiver_coin_lockout_w);
	DECLARE_WRITE8_MEMBER(skydiver_start_lamp_1_w);
	DECLARE_WRITE8_MEMBER(skydiver_start_lamp_2_w);
	DECLARE_WRITE8_MEMBER(skydiver_lamp_s_w);
	DECLARE_WRITE8_MEMBER(skydiver_lamp_k_w);
	DECLARE_WRITE8_MEMBER(skydiver_lamp_y_w);
	DECLARE_WRITE8_MEMBER(skydiver_lamp_d_w);
	DECLARE_WRITE8_MEMBER(skydiver_2000_201F_w);
};


/*----------- defined in audio/skydiver.c -----------*/

DISCRETE_SOUND_EXTERN( skydiver );


/*----------- defined in video/skydiver.c -----------*/

MACHINE_RESET( skydiver );
VIDEO_START( skydiver );
SCREEN_UPDATE_IND16( skydiver );
