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


struct SPRITE_HELPER 
{
	UINT8 bitmap[10],x1,x2,y1,y2, res1, res2;
};

struct SPRITE 
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
};

struct vc4000_video_t 
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
} ;

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
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/vc4000.c -----------*/

extern INTERRUPT_GEN( vc4000_video_line );
extern VIDEO_START( vc4000 );
extern SCREEN_UPDATE_IND16( vc4000 );


/*----------- defined in audio/vc4000.c -----------*/

class vc4000_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	vc4000_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vc4000_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type VC4000;

void vc4000_soundport_w (device_t *device, int mode, int data);


#endif /* VC4000_H_ */
