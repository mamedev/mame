/*************************************************************************

    Cinematronics Cosmic Chasm hardware

*************************************************************************/

#include "machine/z80ctc.h"

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

extern UINT16 *cchasm_ram;

WRITE16_HANDLER( cchasm_refresh_control_w );
VIDEO_START( cchasm );

