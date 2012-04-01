/***************************************************************************

    audio/mcr.h

    Functions to emulate general the various MCR sound cards.

***************************************************************************/

#include "machine/6821pia.h"



/************ Generic MCR routines ***************/

void mcr_sound_init(running_machine &machine, UINT8 config);
void mcr_sound_reset(running_machine &machine);

WRITE8_HANDLER( ssio_data_w );
READ8_HANDLER( ssio_status_r );
READ8_HANDLER( ssio_input_port_r );
WRITE8_HANDLER( ssio_output_port_w );
void ssio_reset_w(running_machine &machine, int state);
void ssio_set_custom_input(int which, int mask, read8_space_func handler);
void ssio_set_custom_output(int which, int mask, write8_space_func handler);

WRITE8_HANDLER( csdeluxe_data_w );
READ8_HANDLER( csdeluxe_status_r );
void csdeluxe_reset_w(running_machine &machine, int state);

WRITE8_HANDLER( turbocs_data_w );
READ8_HANDLER( turbocs_status_r );
void turbocs_reset_w(running_machine &machine, int state);

WRITE8_HANDLER( soundsgood_data_w );
READ8_HANDLER( soundsgood_status_r );
void soundsgood_reset_w(running_machine &machine, int state);

WRITE8_HANDLER( squawkntalk_data_w );
void squawkntalk_reset_w(running_machine &machine, int state);



/************ Sound Configuration ***************/

#define MCR_SSIO				0x01
#define MCR_CHIP_SQUEAK_DELUXE	0x02
#define MCR_SOUNDS_GOOD			0x04
#define MCR_TURBO_CHIP_SQUEAK	0x08
#define MCR_SQUAWK_N_TALK		0x10
#define MCR_WILLIAMS_SOUND		0x20



/************ SSIO input ports ***************/

#define SSIO_INPUT_PORTS \
	AM_RANGE(0x00, 0x04) AM_MIRROR(0x18) AM_READ_LEGACY(ssio_input_port_r) \
	AM_RANGE(0x07, 0x07) AM_MIRROR(0x18) AM_READ_LEGACY(ssio_status_r) \
	AM_RANGE(0x00, 0x07) AM_MIRROR(0x03) AM_WRITE_LEGACY(ssio_output_port_w) \
	AM_RANGE(0x1c, 0x1f) AM_WRITE_LEGACY(ssio_data_w)



/************ External definitions ***************/

MACHINE_CONFIG_EXTERN( mcr_ssio );
MACHINE_CONFIG_EXTERN( chip_squeak_deluxe );
MACHINE_CONFIG_EXTERN( chip_squeak_deluxe_stereo );
MACHINE_CONFIG_EXTERN( sounds_good );
MACHINE_CONFIG_EXTERN( turbo_chip_squeak );
MACHINE_CONFIG_EXTERN( squawk_n_talk );
