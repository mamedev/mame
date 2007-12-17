/***************************************************************************

    audio/mcr.c

    Functions to emulate general the various MCR sound cards.

***************************************************************************/

#include "machine/6821pia.h"



/************ Generic MCR routines ***************/

void mcr_sound_init(UINT8 config);
void mcr_sound_reset(void);

WRITE8_HANDLER( ssio_data_w );
READ8_HANDLER( ssio_status_r );
READ8_HANDLER( ssio_input_port_r );
WRITE8_HANDLER( ssio_output_port_w );
void ssio_reset_w(int state);
void ssio_set_custom_input(int which, int mask, read8_handler handler);
void ssio_set_custom_output(int which, int mask, write8_handler handler);

WRITE8_HANDLER( csdeluxe_data_w );
READ8_HANDLER( csdeluxe_status_r );
void csdeluxe_reset_w(int state);

WRITE8_HANDLER( turbocs_data_w );
READ8_HANDLER( turbocs_status_r );
void turbocs_reset_w(int state);

WRITE8_HANDLER( soundsgood_data_w );
READ8_HANDLER( soundsgood_status_r );
void soundsgood_reset_w(int state);

WRITE8_HANDLER( squawkntalk_data_w );
void squawkntalk_reset_w(int state);



/************ Sound Configuration ***************/

#define MCR_SSIO				0x01
#define MCR_CHIP_SQUEAK_DELUXE	0x02
#define MCR_SOUNDS_GOOD			0x04
#define MCR_TURBO_CHIP_SQUEAK	0x08
#define MCR_SQUAWK_N_TALK		0x10
#define MCR_WILLIAMS_SOUND		0x20



/************ SSIO input ports ***************/

#define SSIO_INPUT_PORTS \
	AM_RANGE(0x00, 0x04) AM_MIRROR(0x18) AM_READ(ssio_input_port_r) \
	AM_RANGE(0x07, 0x07) AM_MIRROR(0x18) AM_READ(ssio_status_r) \
	AM_RANGE(0x00, 0x07) AM_MIRROR(0x03) AM_WRITE(ssio_output_port_w) \
	AM_RANGE(0x1c, 0x1f) AM_WRITE(ssio_data_w)



/************ External definitions ***************/

MACHINE_DRIVER_EXTERN( mcr_ssio );
MACHINE_DRIVER_EXTERN( chip_squeak_deluxe );
MACHINE_DRIVER_EXTERN( chip_squeak_deluxe_stereo );
MACHINE_DRIVER_EXTERN( sounds_good );
MACHINE_DRIVER_EXTERN( turbo_chip_squeak );
MACHINE_DRIVER_EXTERN( turbo_chip_squeak_plus_sounds_good );
MACHINE_DRIVER_EXTERN( squawk_n_talk );
