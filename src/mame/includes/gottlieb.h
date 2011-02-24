/***************************************************************************

    Gottlieb hardware

***************************************************************************/

#include "machine/6532riot.h"


#define GOTTLIEB_VIDEO_HCOUNT	318
#define GOTTLIEB_VIDEO_HBLANK	256
#define GOTTLIEB_VIDEO_VCOUNT	256
#define GOTTLIEB_VIDEO_VBLANK	240


class gottlieb_state : public driver_device
{
public:
	gottlieb_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 votrax_queue[100];
	UINT8 votrax_queuepos;
	emu_timer *nmi_timer;
	UINT8 nmi_rate;
	UINT8 nmi_state;
	UINT8 speech_control;
	UINT8 last_command;
	UINT8 *dac_data;
	UINT8 *psg_latch;
	UINT8 *sp0250_latch;
	int score_sample;
	int random_offset;
	int last;
	UINT8 joystick_select;
	UINT8 track[2];
	device_t *laserdisc;
	emu_timer *laserdisc_bit_timer;
	emu_timer *laserdisc_philips_timer;
	UINT8 laserdisc_select;
	UINT8 laserdisc_status;
	UINT16 laserdisc_philips_code;
	UINT8 *laserdisc_audio_buffer;
	UINT16 laserdisc_audio_address;
	INT16 laserdisc_last_samples[2];
	attotime laserdisc_last_time;
	attotime laserdisc_last_clock;
	UINT8 laserdisc_zero_seen;
	UINT8 laserdisc_audio_bits;
	UINT8 laserdisc_audio_bit_count;
	UINT8 gfxcharlo;
	UINT8 gfxcharhi;
	UINT8 *charram;
	UINT8 background_priority;
	UINT8 spritebank;
	UINT8 transparent0;
	tilemap_t *bg_tilemap;
	double weights[4];
};


/*----------- defined in audio/gottlieb.c -----------*/

WRITE8_HANDLER( gottlieb_sh_w );

MACHINE_CONFIG_EXTERN( gottlieb_soundrev1 );
MACHINE_CONFIG_EXTERN( gottlieb_soundrev2 );

INPUT_PORTS_EXTERN( gottlieb1_sound );
INPUT_PORTS_EXTERN( gottlieb2_sound );


/*----------- defined in video/gottlieb.c -----------*/

extern WRITE8_HANDLER( gottlieb_videoram_w );
extern WRITE8_HANDLER( gottlieb_charram_w );
extern WRITE8_HANDLER( gottlieb_video_control_w );
extern WRITE8_HANDLER( gottlieb_laserdisc_video_control_w );
extern WRITE8_HANDLER( gottlieb_paletteram_w );

VIDEO_START( gottlieb );
VIDEO_START( screwloo );
SCREEN_UPDATE( gottlieb );
