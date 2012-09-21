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

#define VERBOSE_POKEY	1
#define VERBOSE_SERIAL	1
#define VERBOSE_TIMERS	1

static void a600xl_mmu(running_machine &machine, UINT8 new_mmu);

static void pokey_reset(running_machine &machine);

void atari_interrupt_cb(pokey_device *device, int mask)
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

	device->machine().device("maincpu")->execute().set_input_line(0, HOLD_LINE);
}

/**************************************************************
 *
 * PIA interface
 *
 **************************************************************/

READ8_DEVICE_HANDLER(atari_pia_pa_r)
{
	return space.machine().root_device().ioport("djoy_0_1")->read_safe(0);
}

READ8_DEVICE_HANDLER(atari_pia_pb_r)
{
	return space.machine().root_device().ioport("djoy_2_3")->read_safe(0);
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
		machine.device("maincpu")->memory().space(AS_PROGRAM).nop_readwrite(0x5000, 0x57ff);
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", machine.system().name);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0x5000, 0x57ff, "bank2");
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0x5000, 0x57ff);
		machine.root_device().membank("bank2")->set_base(machine.root_device().memregion("maincpu")->base() + 0x5000);
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

POKEY_KEYBOARD_HANDLER(atari_a800_keyboard)
{
	int ipt;
	static const char *const tag[] = {
		"keyboard_0", "keyboard_1", "keyboard_2", "keyboard_3",
		"keyboard_4", "keyboard_5", "keyboard_6", "keyboard_7"
	};
	UINT8 ret = 0x00;

	/* decode special */
	switch (k543210)
	{
	case pokey_device::POK_KEY_BREAK:
		/* special case ... */
		ret |= ((device->machine().root_device().ioport(tag[0])->read_safe(0) & 0x04) ? 0x02 : 0x00);
		break;
	case pokey_device::POK_KEY_CTRL:
		/* CTRL */
		ret |= ((device->machine().root_device().ioport("fake")->read_safe(0) & 0x02) ? 0x02 : 0x00);
		break;
	case pokey_device::POK_KEY_SHIFT:
		/* SHIFT */
		ret |= ((device->machine().root_device().ioport("fake")->read_safe(0) & 0x01) ? 0x02 : 0x00);
		break;
	}

	/* return on BREAK key now! */
	if (k543210 == AKEY_BREAK || k543210 == AKEY_NONE)
		return ret;

	/* decode regular key */
	ipt = device->machine().root_device().ioport(tag[k543210 >> 3])->read_safe(0);

	if (ipt & (1 << (k543210 & 0x07)))
		ret |= 0x01;

	return ret;
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

POKEY_KEYBOARD_HANDLER(atari_a5200_keypads)
{
	int ipt;
	static const char *const tag[] = { "keypad_0", "keypad_1", "keypad_2", "keypad_3" };
	UINT8 ret = 0x00;

	/* decode special */
	switch (k543210)
	{
	case pokey_device::POK_KEY_BREAK:
		/* special case ... */
		ret |= ((device->machine().root_device().ioport(tag[0])->read_safe(0) & 0x01) ? 0x02 : 0x00);
		break;
	case pokey_device::POK_KEY_CTRL:
	case pokey_device::POK_KEY_SHIFT:
		break;
	}

	/* decode regular key */
	/* if kr5 and kr0 not set just return */
	if ((k543210 & 0x21) != 0x21)
		return ret;

	k543210 = (k543210 >> 1) & 0x0f;

	/* return on BREAK key now! */
	if (k543210 == 0)
		return ret;

	ipt = device->machine().root_device().ioport(tag[k543210 >> 2])->read_safe(0);

	if (ipt & (1 <<(k543210 & 0x03)))
		ret |= 0x01;

	return ret;
}


/*************************************
 *
 *  Generic Atari Code
 *
 *************************************/


static void pokey_reset(running_machine &machine)
{
	pokey_device *pokey = downcast<pokey_device *>(machine.device("pokey"));
	pokey->write(15,0);
}


static UINT8 console_read(address_space &space)
{
	return space.machine().root_device().ioport("console")->read();
}


static void console_write(address_space &space, UINT8 data)
{
	dac_device *dac = space.machine().device<dac_device>("dac");
	if (data & 0x08)
		dac->write_unsigned8((UINT8)-120);
	else
		dac->write_unsigned8(+120);
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
	if (machine.root_device().ioport("console") != NULL)
		gtia_intf.console_read = console_read;
	if (machine.device<dac_device>("dac") != NULL)
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
