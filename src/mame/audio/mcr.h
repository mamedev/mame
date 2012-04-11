/***************************************************************************

    audio/mcr.h

    Functions to emulate general the various MCR sound cards.

***************************************************************************/

#include "machine/6821pia.h"



/************ Generic MCR routines ***************/

void mcr_sound_init(running_machine &machine, UINT8 config);
void mcr_sound_reset(running_machine &machine);

void ssio_reset_w(running_machine &machine, int state);
void ssio_set_custom_input(int which, int mask, read8_delegate handler);
void ssio_set_custom_output(int which, int mask, write8_delegate handler);

void csdeluxe_reset_w(running_machine &machine, int state);

void turbocs_reset_w(running_machine &machine, int state);

void soundsgood_reset_w(running_machine &machine, int state);

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
	AM_RANGE(0x00, 0x04) AM_MIRROR(0x18) AM_READ(ssio_input_port_r) \
	AM_RANGE(0x07, 0x07) AM_MIRROR(0x18) AM_READ(ssio_status_r) \
	AM_RANGE(0x00, 0x07) AM_MIRROR(0x03) AM_WRITE(ssio_output_port_w) \
	AM_RANGE(0x1c, 0x1f) AM_WRITE(ssio_data_w)



/************ External definitions ***************/

MACHINE_CONFIG_EXTERN( mcr_ssio );
MACHINE_CONFIG_EXTERN( chip_squeak_deluxe );
MACHINE_CONFIG_EXTERN( chip_squeak_deluxe_stereo );
MACHINE_CONFIG_EXTERN( sounds_good );
MACHINE_CONFIG_EXTERN( turbo_chip_squeak );
MACHINE_CONFIG_EXTERN( squawk_n_talk );
