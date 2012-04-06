#include "machine/6522via.h"

class beezer_state : public driver_device
{
public:
	beezer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_pbus;
	int m_banklatch;

	device_t *m_maincpu;
	DECLARE_WRITE8_MEMBER(beezer_bankswitch_w);
	DECLARE_WRITE8_MEMBER(beezer_map_w);
	DECLARE_READ8_MEMBER(beezer_line_r);
};


/*----------- defined in machine/beezer.c -----------*/

extern const via6522_interface b_via_0_interface;
extern const via6522_interface b_via_1_interface;

DRIVER_INIT( beezer );

/*----------- defined in audio/beezer.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(BEEZER, beezer_sound);

READ8_DEVICE_HANDLER( beezer_sh6840_r );
WRITE8_DEVICE_HANDLER( beezer_sh6840_w );
WRITE8_DEVICE_HANDLER( beezer_sfxctrl_w );
WRITE8_DEVICE_HANDLER( beezer_timer1_w );
READ8_DEVICE_HANDLER( beezer_noise_r );

/*----------- defined in video/beezer.c -----------*/

TIMER_DEVICE_CALLBACK( beezer_interrupt );
SCREEN_UPDATE_IND16( beezer );
