#include "emu.h"
#include "imagedev/cassette.h"
#include "crsshair.h"
#include "includes/cbm.h"
#include "formats/cbm_tap.h"


/***********************************************

    Input Reading - Common Components

***********************************************/

/* These are needed by c64, c65 and c128, each machine has also additional specific
components in its INTERRUPT_GEN */

/* keyboard lines */
UINT8 c64_keyline[10];

void cbm_common_init(void)
{
	int i;

	for (i = 0; i < ARRAY_LENGTH(c64_keyline); i++)
		c64_keyline[i] = 0xff;
}

static TIMER_CALLBACK( lightpen_tick )
{
	if (((machine.root_device().ioport("CTRLSEL")->read() & 0x07) == 0x04) || ((machine.root_device().ioport("CTRLSEL")->read() & 0x07) == 0x06))
	{
		/* enable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_NONE);
	}
}

void cbm_common_interrupt( device_t *device )
{
	int value, i;
	int controller1 = device->machine().root_device().ioport("CTRLSEL")->read() & 0x07;
	int controller2 = device->machine().root_device().ioport("CTRLSEL")->read() & 0x70;
	static const char *const c64ports[] = { "ROW0", "ROW1", "ROW2", "ROW3", "ROW4", "ROW5", "ROW6", "ROW7" };

	/* Lines 0-7 : common keyboard */
	for (i = 0; i < 8; i++)
	{
		value = 0xff;
		value &= ~device->machine().root_device().ioport(c64ports[i])->read();

		/* Shift Lock is mapped on Left Shift */
		if ((i == 1) && (device->machine().root_device().ioport("SPECIAL")->read() & 0x40))
			value &= ~0x80;

		c64_keyline[i] = value;
	}


	value = 0xff;
	switch(controller1)
	{
		case 0x00:
			value &= ~device->machine().root_device().ioport("JOY1_1B")->read();            /* Joy1 Directions + Button 1 */
			break;

		case 0x01:
			if (device->machine().root_device().ioport("OTHER")->read() & 0x40)         /* Paddle2 Button */
				value &= ~0x08;
			if (device->machine().root_device().ioport("OTHER")->read() & 0x80)         /* Paddle1 Button */
				value &= ~0x04;
			break;

		case 0x02:
			if (device->machine().root_device().ioport("OTHER")->read() & 0x02)         /* Mouse Button Left */
				value &= ~0x10;
			if (device->machine().root_device().ioport("OTHER")->read() & 0x01)         /* Mouse Button Right */
				value &= ~0x01;
			break;

		case 0x03:
			value &= ~(device->machine().root_device().ioport("JOY1_2B")->read() & 0x1f);   /* Joy1 Directions + Button 1 */
			break;

		case 0x04:
/* was there any input on the lightpen? where is it mapped? */
//          if (device->machine().root_device().ioport("OTHER")->read() & 0x04)           /* Lightpen Signal */
//              value &= ?? ;
			break;

		case 0x07:
			break;

		default:
			logerror("Invalid Controller 1 Setting %d\n", controller1);
			break;
	}

	c64_keyline[8] = value;


	value = 0xff;
	switch(controller2)
	{
		case 0x00:
			value &= ~device->machine().root_device().ioport("JOY2_1B")->read();            /* Joy2 Directions + Button 1 */
			break;

		case 0x10:
			if (device->machine().root_device().ioport("OTHER")->read() & 0x10)         /* Paddle4 Button */
				value &= ~0x08;
			if (device->machine().root_device().ioport("OTHER")->read() & 0x20)         /* Paddle3 Button */
				value &= ~0x04;
			break;

		case 0x20:
			if (device->machine().root_device().ioport("OTHER")->read() & 0x02)         /* Mouse Button Left */
				value &= ~0x10;
			if (device->machine().root_device().ioport("OTHER")->read() & 0x01)         /* Mouse Button Right */
				value &= ~0x01;
			break;

		case 0x30:
			value &= ~(device->machine().root_device().ioport("JOY2_2B")->read() & 0x1f);   /* Joy2 Directions + Button 1 */
			break;

		case 0x40:
/* was there any input on the lightpen? where is it mapped? */
//          if (device->machine().root_device().ioport("OTHER")->read() & 0x04)           /* Lightpen Signal */
//              value &= ?? ;
			break;

		case 0x70:
			break;

		default:
			logerror("Invalid Controller 2 Setting %d\n", controller2);
			break;
	}

	c64_keyline[9] = value;

//  vic2_frame_interrupt does nothing so this is not necessary
//  vic2_frame_interrupt (device);

	/* check if lightpen has been chosen as input: if so, enable crosshair */
	device->machine().scheduler().timer_set(attotime::zero, FUNC(lightpen_tick));

	set_led_status (device->machine(), 1, device->machine().root_device().ioport("SPECIAL")->read() & 0x40 ? 1 : 0);        /* Shift Lock */
	set_led_status (device->machine(), 0, device->machine().root_device().ioport("CTRLSEL")->read() & 0x80 ? 1 : 0);        /* Joystick Swap */
}


/***********************************************

    CIA Common Handlers

***********************************************/

/* These are shared by c64, c65 and c128. c65 and c128 also have additional specific
components (to select/read additional keyboard lines) */

/*
 *  CIA 0 - Port A
 * bits 7-0 keyboard line select
 * bits 7,6: paddle select( 01 port a, 10 port b)
 * bit 4: joystick a fire button
 * bits 3,2: Paddles port a fire button
 * bits 3-0: joystick a direction
 *
 *  CIA 0 - Port B
 * bits 7-0: keyboard raw values
 * bit 4: joystick b fire button, lightpen select
 * bits 3,2: paddle b fire buttons (left,right)
 * bits 3-0: joystick b direction
 *
 * flag cassette read input, serial request in
 * irq to irq connected
 */

UINT8 cbm_common_cia0_port_a_r( device_t *device, UINT8 output_b )
{
	UINT8 value = 0xff;

	if (!(output_b & 0x80))
	{
		UINT8 t = 0xff;
		if (!(c64_keyline[7] & 0x80)) t &= ~0x80;
		if (!(c64_keyline[6] & 0x80)) t &= ~0x40;
		if (!(c64_keyline[5] & 0x80)) t &= ~0x20;
		if (!(c64_keyline[4] & 0x80)) t &= ~0x10;
		if (!(c64_keyline[3] & 0x80)) t &= ~0x08;
		if (!(c64_keyline[2] & 0x80)) t &= ~0x04;
		if (!(c64_keyline[1] & 0x80)) t &= ~0x02;
		if (!(c64_keyline[0] & 0x80)) t &= ~0x01;
		value &= t;
	}

	if (!(output_b & 0x40))
	{
		UINT8 t = 0xff;
		if (!(c64_keyline[7] & 0x40)) t &= ~0x80;
		if (!(c64_keyline[6] & 0x40)) t &= ~0x40;
		if (!(c64_keyline[5] & 0x40)) t &= ~0x20;
		if (!(c64_keyline[4] & 0x40)) t &= ~0x10;
		if (!(c64_keyline[3] & 0x40)) t &= ~0x08;
		if (!(c64_keyline[2] & 0x40)) t &= ~0x04;
		if (!(c64_keyline[1] & 0x40)) t &= ~0x02;
		if (!(c64_keyline[0] & 0x40)) t &= ~0x01;
		value &= t;
	}

	if (!(output_b & 0x20))
	{
		UINT8 t = 0xff;
		if (!(c64_keyline[7] & 0x20)) t &= ~0x80;
		if (!(c64_keyline[6] & 0x20)) t &= ~0x40;
		if (!(c64_keyline[5] & 0x20)) t &= ~0x20;
		if (!(c64_keyline[4] & 0x20)) t &= ~0x10;
		if (!(c64_keyline[3] & 0x20)) t &= ~0x08;
		if (!(c64_keyline[2] & 0x20)) t &= ~0x04;
		if (!(c64_keyline[1] & 0x20)) t &= ~0x02;
		if (!(c64_keyline[0] & 0x20)) t &= ~0x01;
		value &= t;
	}

	if (!(output_b & 0x10))
	{
		UINT8 t = 0xff;
		if (!(c64_keyline[7] & 0x10)) t &= ~0x80;
		if (!(c64_keyline[6] & 0x10)) t &= ~0x40;
		if (!(c64_keyline[5] & 0x10)) t &= ~0x20;
		if (!(c64_keyline[4] & 0x10)) t &= ~0x10;
		if (!(c64_keyline[3] & 0x10)) t &= ~0x08;
		if (!(c64_keyline[2] & 0x10)) t &= ~0x04;
		if (!(c64_keyline[1] & 0x10)) t &= ~0x02;
		if (!(c64_keyline[0] & 0x10)) t &= ~0x01;
		value &= t;
	}

	if (!(output_b & 0x08))
	{
		UINT8 t = 0xff;
		if (!(c64_keyline[7] & 0x08)) t &= ~0x80;
		if (!(c64_keyline[6] & 0x08)) t &= ~0x40;
		if (!(c64_keyline[5] & 0x08)) t &= ~0x20;
		if (!(c64_keyline[4] & 0x08)) t &= ~0x10;
		if (!(c64_keyline[3] & 0x08)) t &= ~0x08;
		if (!(c64_keyline[2] & 0x08)) t &= ~0x04;
		if (!(c64_keyline[1] & 0x08)) t &= ~0x02;
		if (!(c64_keyline[0] & 0x08)) t &= ~0x01;
		value &= t;
	}

	if (!(output_b & 0x04))
	{
		UINT8 t = 0xff;
		if (!(c64_keyline[7] & 0x04)) t &= ~0x80;
		if (!(c64_keyline[6] & 0x04)) t &= ~0x40;
		if (!(c64_keyline[5] & 0x04)) t &= ~0x20;
		if (!(c64_keyline[4] & 0x04)) t &= ~0x10;
		if (!(c64_keyline[3] & 0x04)) t &= ~0x08;
		if (!(c64_keyline[2] & 0x04)) t &= ~0x04;
		if (!(c64_keyline[1] & 0x04)) t &= ~0x02;
		if (!(c64_keyline[0] & 0x04)) t &= ~0x01;
		value &= t;
	}

	if (!(output_b & 0x02))
	{
		UINT8 t = 0xff;
		if (!(c64_keyline[7] & 0x02)) t &= ~0x80;
		if (!(c64_keyline[6] & 0x02)) t &= ~0x40;
		if (!(c64_keyline[5] & 0x02)) t &= ~0x20;
		if (!(c64_keyline[4] & 0x02)) t &= ~0x10;
		if (!(c64_keyline[3] & 0x02)) t &= ~0x08;
		if (!(c64_keyline[2] & 0x02)) t &= ~0x04;
		if (!(c64_keyline[1] & 0x02)) t &= ~0x02;
		if (!(c64_keyline[0] & 0x02)) t &= ~0x01;
		value &= t;
	}

	if (!(output_b & 0x01))
	{
		UINT8 t = 0xff;
		if (!(c64_keyline[7] & 0x01)) t &= ~0x80;
		if (!(c64_keyline[6] & 0x01)) t &= ~0x40;
		if (!(c64_keyline[5] & 0x01)) t &= ~0x20;
		if (!(c64_keyline[4] & 0x01)) t &= ~0x10;
		if (!(c64_keyline[3] & 0x01)) t &= ~0x08;
		if (!(c64_keyline[2] & 0x01)) t &= ~0x04;
		if (!(c64_keyline[1] & 0x01)) t &= ~0x02;
		if (!(c64_keyline[0] & 0x01)) t &= ~0x01;
		value &= t;
	}

	if ( device->machine().root_device().ioport("CTRLSEL")->read() & 0x80 )
		value &= c64_keyline[8];
	else
		value &= c64_keyline[9];

	return value;
}

UINT8 cbm_common_cia0_port_b_r( device_t *device, UINT8 output_a )
{
	UINT8 value = 0xff;

	if (!(output_a & 0x80)) value &= c64_keyline[7];
	if (!(output_a & 0x40)) value &= c64_keyline[6];
	if (!(output_a & 0x20)) value &= c64_keyline[5];
	if (!(output_a & 0x10)) value &= c64_keyline[4];
	if (!(output_a & 0x08)) value &= c64_keyline[3];
	if (!(output_a & 0x04)) value &= c64_keyline[2];
	if (!(output_a & 0x02)) value &= c64_keyline[1];
	if (!(output_a & 0x01)) value &= c64_keyline[0];

	if ( device->machine().root_device().ioport("CTRLSEL")->read() & 0x80 )
		value &= c64_keyline[9];
	else
		value &= c64_keyline[8];

	return value;
}


/***********************************************

    CBM Cartridges

***********************************************/


/*  All the cartridge specific code has been moved
    to machine/ drivers. Once more informations
    surface about the cart expansions for systems
    in c65.c, c128.c, cbmb.c and pet.c, the shared
    code could be refactored to have here the
    common functions                                */



/***********************************************

    CBM Datasette Tapes

***********************************************/

const cassette_interface cbm_cassette_interface =
{
	cbm_cassette_formats,
	NULL,
	(cassette_state) (CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED),
	NULL,
	NULL
};
