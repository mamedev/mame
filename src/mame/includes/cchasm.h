/*************************************************************************

    Cinematronics Cosmic Chasm hardware

*************************************************************************/

#include "machine/z80ctc.h"

class cchasm_state : public driver_device
{
public:
	cchasm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_sound_flags;
	int m_coin_flag;
	device_t *m_ctc;
	int m_channel_active[2];
	int m_output[2];
	UINT16 *m_ram;
	int m_xcenter;
	int m_ycenter;
	DECLARE_WRITE16_MEMBER(cchasm_led_w);
	DECLARE_WRITE16_MEMBER(cchasm_refresh_control_w);
};


/*----------- defined in machine/cchasm.c -----------*/


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

VIDEO_START( cchasm );

