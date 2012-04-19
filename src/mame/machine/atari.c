/***************************************************************************

    Atari 400/800

    Machine driver

    Juergen Buchmueller, June 1998

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/atari.h"
#include "sound/pokey.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "video/gtia.h"

#define VERBOSE_POKEY	0
#define VERBOSE_SERIAL	0
#define VERBOSE_TIMERS	0

static void a600xl_mmu(running_machine &machine, UINT8 new_mmu);

static void pokey_reset(running_machine &machine);

void atari_interrupt_cb(device_t *device, int mask)
{

	if (VERBOSE_POKEY)
	{
		if (mask & 0x80)
			logerror("atari interrupt_cb BREAK\n");
		if (mask & 0x40)
			logerror("atari interrupt_cb KBCOD\n");
	}
	if (VERBOSE_SERIAL)
	{
		if (mask & 0x20)
			logerror("atari interrupt_cb SERIN\n");
		if (mask & 0x10)
			logerror("atari interrupt_cb SEROR\n");
		if (mask & 0x08)
			logerror("atari interrupt_cb SEROC\n");
	}
	if (VERBOSE_TIMERS)
	{
		if (mask & 0x04)
			logerror("atari interrupt_cb TIMR4\n");
		if (mask & 0x02)
			logerror("atari interrupt_cb TIMR2\n");
		if (mask & 0x01)
			logerror("atari interrupt_cb TIMR1\n");
	}

	cputag_set_input_line(device->machine(), "maincpu", 0, HOLD_LINE);
}

/**************************************************************
 *
 * PIA interface
 *
 **************************************************************/

READ8_DEVICE_HANDLER(atari_pia_pa_r)
{
	return input_port_read_safe(device->machine(), "djoy_0_1", 0);
}

READ8_DEVICE_HANDLER(atari_pia_pb_r)
{
	return input_port_read_safe(device->machine(), "djoy_2_3", 0);
}

WRITE8_DEVICE_HANDLER(a600xl_pia_pb_w) { a600xl_mmu(device->machine(), data); }

WRITE_LINE_DEVICE_HANDLER(atari_pia_cb2_w) { }	// This is used by Floppy drive on Atari 8bits Home Computers

const pia6821_interface atarixl_pia_interface =
{
	DEVCB_HANDLER(atari_pia_pa_r),		/* port A in */
	DEVCB_HANDLER(atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_HANDLER(a600xl_pia_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_LINE(atari_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


/**************************************************************
 *
 * Memory banking
 *
 **************************************************************/

void a600xl_mmu(running_machine &machine, UINT8 new_mmu)
{
	/* check if self-test ROM changed */
	if ( new_mmu & 0x80 )
	{
		logerror("%s MMU SELFTEST RAM\n", machine.system().name);
		machine.device("maincpu")->memory().space(AS_PROGRAM)->nop_readwrite(0x5000, 0x57ff);
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", machine.system().name);
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_read_bank(0x5000, 0x57ff, "bank2");
		machine.device("maincpu")->memory().space(AS_PROGRAM)->unmap_write(0x5000, 0x57ff);
		machine.root_device().subbank("bank2")->set_base(machine.region("maincpu")->base() + 0x5000);
	}
}



/**************************************************************
 *
 * Keyboard
 *
 **************************************************************/

#define AKEY_BREAK		0x03	/* this not really a scancode */
#define AKEY_NONE		0x09

/**************************************************************

    Keyboard inputs use 6bits to read the 64keys in the key matrix.
    We currently read the key matrix by lines and convert the input
    to the value expected by the POKEY (see the code below to
    determine atari_code values).

    K2,K1,K0 | 000 | 001 | 010 | 011 | 100 | 101 | 110 | 111 |
    K5,K4,K3
    ----------------------------------------------------------
        000  |  L  |  J  |  ;  | (*) |     |  K  |  +  |  *  |
        001  |  O  |     |  P  |  U  | Ret |  I  |  -  |  =  |
        010  |  V  |     |  C  |     |     |  B  |  X  |  Z  |
        011  |  4  |     |  3  |  6  | Esc |  5  |  2  |  1  |
        100  |  ,  | Spc |  .  |  N  |     |  M  |  /  |Atari|
        101  |  R  |     |  E  |  Y  | Tab |  T  |  W  |  Q  |
        110  |  9  |     |  0  |  7  |Bkspc|  8  |  <  |  >  |
        111  |  F  |  H  |  D  |     | Caps|  G  |  S  |  A  |

    (*) We use this value to read Break, but in fact it would be read
        in KR2 bit. This has to be properly implemented for later
        Atari systems because here we would have F1.

    To Do: investigate implementation of KR2 to read accurately Break,
    Shift and Control keys.

 **************************************************************/

static int atari_last;

void a800_handle_keyboard(running_machine &machine)
{
	device_t *pokey = machine.device("pokey");
	int atari_code, count, ipt, i;
	static const char *const tag[] = {
		"keyboard_0", "keyboard_1", "keyboard_2", "keyboard_3",
		"keyboard_4", "keyboard_5", "keyboard_6", "keyboard_7"
	};

	/* check keyboard */
	for( i = 0; i < 8; i++ )
	{
		ipt = input_port_read_safe(machine, tag[i], 0);

		if( ipt )
		{
			count = 0;
			while(ipt / 2)
			{
				ipt = ipt/2;
				count++;
			}

			atari_code = i*8 + count;

			/* SHIFT */
			if(input_port_read_safe(machine, "fake", 0) & 0x01)
				atari_code |= 0x40;

			/* CTRL */
			if(input_port_read_safe(machine, "fake", 0) & 0x02)
				atari_code |= 0x80;

			if( atari_code != AKEY_NONE )
			{
				if( atari_code == atari_last )
					return;
				atari_last = atari_code;

				if( (atari_code & 0x3f) == AKEY_BREAK )
				{
					pokey_break_w(pokey, atari_code & 0x40);
					return;
				}

				pokey_kbcode_w(pokey, atari_code, 1);
				return;
			}
		}

	}
	/* remove key pressed status bit from skstat */
	pokey_kbcode_w(pokey, AKEY_NONE, 0);
	atari_last = AKEY_NONE;
}

/**************************************************************
 *
 * Keypad
 *
 **************************************************************/

/**************************************************************

    A5200 keypad inputs use 4bits to read the 16keys in the key
    matrix. We currently read the key matrix by lines and convert
    the input to the value expected by the POKEY (see the code
    below to determine atari_code values).

    K2,K1,K0 | 00x | 01x | 10x | 11x |
    K5,K4,K3
    ----------------------------------
        x00  |     |  #  |  0  |  *  |
        x01  |Reset|  9  |  8  |  7  |
        x10  |Pause|  6  |  5  |  4  |
        x11  |Start|  3  |  2  |  1  |

    K0 & K5 are ignored (we send them as 1, see the code below where
    we pass "(atari_code << 1) | 0x21" )

    To Do: investigate implementation of KR2 to read accurately the
    secondary Fire button (primary read through GTIA).

 **************************************************************/

void a5200_handle_keypads(running_machine &machine)
{
	device_t *pokey = machine.device("pokey");
	int atari_code, count, ipt, i;
	static const char *const tag[] = { "keypad_0", "keypad_1", "keypad_2", "keypad_3" };

	/* check keypad */
	for( i = 0; i < 4; i++ )
	{
		ipt = input_port_read_safe(machine, tag[i], 0);

		if( ipt )
		{
			count = 0;
			while(ipt / 2)
			{
				ipt = ipt/2;
				count++;
			}

			atari_code = i*4 + count;

			if( atari_code == atari_last )
				return;
			atari_last = atari_code;

			if( atari_code == 0 )
			{
				pokey_break_w(pokey, atari_code & 0x40);
				return;
			}

			pokey_kbcode_w(pokey, (atari_code << 1) | 0x21, 1);
			return;
		}

	}

	/* check top button */
	if ((input_port_read(machine, "djoy_b") & 0x10) == 0)
	{
		if (atari_last == 0xfe)
			return;
		pokey_kbcode_w(pokey, 0x61, 1);
		//pokey_break_w(pokey, 0x40);
		atari_last = 0xfe;
		return;
	}
	else if (atari_last == 0xfe)
		pokey_kbcode_w(pokey, 0x21, 1);

	/* remove key pressed status bit from skstat */
	pokey_kbcode_w(pokey, 0xff, 0);
	atari_last = 0xff;
}


/*************************************
 *
 *  Generic Atari Code
 *
 *************************************/


static void pokey_reset(running_machine &machine)
{
	device_t *pokey = machine.device("pokey");
	pokey_w(pokey,15,0);
	atari_last = 0xff;
}


static UINT8 console_read(address_space *space)
{
	return input_port_read(space->machine(), "console");
}


static void console_write(address_space *space, UINT8 data)
{
	device_t *dac = space->machine().device("dac");
	if (data & 0x08)
		dac_data_w(dac, (UINT8)-120);
	else
		dac_data_w(dac, +120);
}


static void _antic_reset(running_machine &machine)
{
	antic_reset();
}


void atari_machine_start(running_machine &machine)
{
	gtia_interface gtia_intf;

	/* GTIA */
	memset(&gtia_intf, 0, sizeof(gtia_intf));
	if (machine.port("console") != NULL)
		gtia_intf.console_read = console_read;
	if (machine.device("dac") != NULL)
		gtia_intf.console_write = console_write;
	gtia_init(machine, &gtia_intf);

	/* pokey */
	machine.add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(pokey_reset), &machine));

	/* ANTIC */
	machine.add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(_antic_reset), &machine));

	/* save states */
	state_save_register_global_pointer(machine, ((UINT8 *) &antic.r), sizeof(antic.r));
	state_save_register_global_pointer(machine, ((UINT8 *) &antic.w), sizeof(antic.w));
}


/*************************************
 *
 *  Atari 600XL
 *
 *************************************/

MACHINE_START( atarixl )
{
	atari_machine_start(machine);
}
