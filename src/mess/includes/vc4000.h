/*****************************************************************************
 *
 * includes/vc4000.h
 *
 ****************************************************************************/

#ifndef VC4000_H_
#define VC4000_H_

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "imagedev/cartslot.h"
#include "imagedev/snapquik.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"

// define this to use digital inputs instead of the slow
// autocentering analog mame joys
#define ANALOG_HACK


typedef struct
{
	UINT8 bitmap[10],x1,x2,y1,y2, res1, res2;
} SPRITE_HELPER;

typedef struct
{
	const SPRITE_HELPER *data;
	int mask;
	int state;
	int delay;
	int size;
	int y;
	UINT8 scolor;
	int finished;
	int finished_now;
} SPRITE;

typedef struct
{
	SPRITE sprites[4];
	int line;
	UINT8 sprite_collision;
	UINT8 background_collision;
	union
	{
		UINT8 data[0x100];
		struct
		{
			SPRITE_HELPER sprites[3];
			UINT8 res[0x10];
			SPRITE_HELPER sprite4;
			UINT8 res2[0x30];
			UINT8 grid[20][2];
			UINT8 grid_control[5];
			UINT8 res3[0x13];
			UINT8 sprite_sizes;
			UINT8 sprite_colors[2];
			UINT8 score_control;
			UINT8 res4[2];
			UINT8 background;
			UINT8 sound;
			UINT8 bcd[2];
			UINT8 background_collision;
			UINT8 sprite_collision;
		} d;
	} reg;
} vc4000_video_t;

class vc4000_state : public driver_device
{
public:
	vc4000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_cass(*this, CASSETTE_TAG)
	{ }

	DECLARE_WRITE8_MEMBER(vc4000_sound_ctl);
	DECLARE_READ8_MEMBER(vc4000_key_r);
	DECLARE_READ8_MEMBER(vc4000_video_r);
	DECLARE_WRITE8_MEMBER(vc4000_video_w);
	DECLARE_READ8_MEMBER(vc4000_vsync_r);
	DECLARE_READ8_MEMBER(elektor_cass_r);
	DECLARE_WRITE8_MEMBER(elektor_cass_w);
	vc4000_video_t m_video;
	UINT8 m_sprite_collision[0x20];
	UINT8 m_background_collision[0x20];
	UINT8 m_joy1_x;
	UINT8 m_joy1_y;
	UINT8 m_joy2_x;
	UINT8 m_joy2_y;
	UINT8 m_objects[512];
	UINT8 m_irq_pause;
	bitmap_ind16 *m_bitmap;
	optional_device<cassette_image_device> m_cass;
};


/*----------- defined in video/vc4000.c -----------*/

extern INTERRUPT_GEN( vc4000_video_line );
extern VIDEO_START( vc4000 );
extern SCREEN_UPDATE_IND16( vc4000 );


/*----------- defined in audio/vc4000.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(VC4000, vc4000_sound);
void vc4000_soundport_w (device_t *device, int mode, int data);


#endif /* VC4000_H_ */
