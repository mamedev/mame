/*************************************************************************

    Cinematronics Cosmic Chasm hardware

*************************************************************************/

#include "machine/z80ctc.h"

class cchasm_state : public driver_device
{
public:
	cchasm_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int sound_flags;
	int coin_flag;
	device_t *ctc;
	int channel_active[2];
	int output[2];
	UINT16 *ram;
	int xcenter;
	int ycenter;
};


/*----------- defined in machine/cchasm.c -----------*/

WRITE16_HANDLER( cchasm_led_w );

/*----------- defined in audio/cchasm.c -----------*/

extern const z80ctc_interface cchasm_ctc_intf;

WRITE8_HANDLER( cchasm_reset_coin_flag_w );
INPUT_CHANGED( cchasm_set_coin_flag );
READ8_HANDLER( cchasm_coin_sound_r );
READ8_HANDLER( cchasm_soundlatch2_r );
WRITE8_HANDLER( cchasm_soundlatch4_w );

WRITE16_HANDLER( cchasm_io_w );
READ16_HANDLER( cchasm_io_r );

SOUND_START( cchasm );


/*----------- defined in video/cchasm.c -----------*/

WRITE16_HANDLER( cchasm_refresh_control_w );
VIDEO_START( cchasm );

