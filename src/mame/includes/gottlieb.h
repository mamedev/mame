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
	gottlieb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 m_votrax_queue[100];
	UINT8 m_votrax_queuepos;
	emu_timer *m_nmi_timer;
	UINT8 m_nmi_rate;
	UINT8 m_nmi_state;
	UINT8 m_speech_control;
	UINT8 m_last_command;
	UINT8 *m_dac_data;
	UINT8 *m_psg_latch;
	UINT8 *m_sp0250_latch;
	int m_score_sample;
	int m_random_offset;
	int m_last;
	UINT8 m_joystick_select;
	UINT8 m_track[2];
	device_t *m_laserdisc;
	emu_timer *m_laserdisc_bit_timer;
	emu_timer *m_laserdisc_philips_timer;
	UINT8 m_laserdisc_select;
	UINT8 m_laserdisc_status;
	UINT16 m_laserdisc_philips_code;
	UINT8 *m_laserdisc_audio_buffer;
	UINT16 m_laserdisc_audio_address;
	INT16 m_laserdisc_last_samples[2];
	attotime m_laserdisc_last_time;
	attotime m_laserdisc_last_clock;
	UINT8 m_laserdisc_zero_seen;
	UINT8 m_laserdisc_audio_bits;
	UINT8 m_laserdisc_audio_bit_count;
	UINT8 m_gfxcharlo;
	UINT8 m_gfxcharhi;
	UINT8 *m_charram;
	UINT8 m_background_priority;
	UINT8 m_spritebank;
	UINT8 m_transparent0;
	tilemap_t *m_bg_tilemap;
	double m_weights[4];
	UINT8 *m_spriteram;
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
