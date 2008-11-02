/*************************************************************************

    Cinematronics Cosmic Chasm hardware

*************************************************************************/

#include "sound/custom.h"
#include "machine/z80ctc.h"

/*----------- defined in machine/cchasm.c -----------*/

WRITE16_HANDLER( cchasm_led_w );

/*----------- defined in audio/cchasm.c -----------*/

extern z80ctc_interface cchasm_ctc_intf;

READ8_HANDLER( cchasm_snd_io_r );
WRITE8_HANDLER( cchasm_snd_io_w );

WRITE16_HANDLER( cchasm_io_w );
READ16_HANDLER( cchasm_io_r );

SOUND_START( cchasm );


/*----------- defined in video/cchasm.c -----------*/

extern UINT16 *cchasm_ram;

WRITE16_HANDLER( cchasm_refresh_control_w );
VIDEO_START( cchasm );

