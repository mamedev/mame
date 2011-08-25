#include "machine/6522via.h"

class beezer_state : public driver_device
{
public:
	beezer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	int m_pbus;
	int m_banklatch;
	int m_scanline;
};


/*----------- defined in machine/beezer.c -----------*/

extern const via6522_interface b_via_0_interface;
extern const via6522_interface b_via_1_interface;

DRIVER_INIT( beezer );
WRITE8_HANDLER( beezer_bankswitch_w );

/*----------- defined in audio/beezer.c -----------*/

DECLARE_LEGACY_SOUND_DEVICE(BEEZER, beezer_sound);

READ8_DEVICE_HANDLER( beezer_sh6840_r );
WRITE8_DEVICE_HANDLER( beezer_sh6840_w );
WRITE8_DEVICE_HANDLER( beezer_sfxctrl_w );
WRITE8_DEVICE_HANDLER( beezer_timer1_w );
READ8_DEVICE_HANDLER( beezer_noise_r );

/*----------- defined in video/beezer.c -----------*/

INTERRUPT_GEN( beezer_interrupt );
SCREEN_UPDATE( beezer );
WRITE8_HANDLER( beezer_map_w );
READ8_HANDLER( beezer_line_r );
