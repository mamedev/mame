/*********************************************************************

    8042kbdc.c

    Code specific to fun IBM AT stuff


    PeT's notes about various Power On Self Tests (POSTs)

        at post
        -------
        f81d2 01
        f82e6 05
        f8356 07
        f83e5 0a
        f847e 0e
        f8e7c 10
        f8f3a 13
        f9058 1a
        f913a 1e
        fa8ba 30
        fa96c 36
        fa9d3 3c
        fa9f4 3e
        ff122 50
        ff226 5b
        ff29f 5f
        f9228 70
        f92b2 74 ide?

        ibm at bios
        -----------
        f0101 after switch to real mode jump back!!!!!!!!!!
        jumping table
        f0123
        f098e memory tests
        f10b4 ???
        not reached
        f1050
        f1617
        f0119
        f10d8
        f10b7 system board error

        f019f
        f025b
        f02e6
        f0323
        f03b3 0e
        f03d7 0f
        f058d
        at8042 signal timing test
        sets errorcode!

        f0655
        f06a3 postcode 15
        f06ba 16
        f0747 18
        f0763 enter pm! (0x81, 0x85)
        f0766 1a
        f07b3 first 640kb memory test
        f084c 1c
        f086e extended memory test
        f0928 1f
        f097d 20
        ???
        f0ff4 34
        ???
        f1675 f0
        f16cb f2
        illegal access trap test!!!!
        f16fe f3
        task descriptor test!!!!
        f174a f4
        f17af f5
        f1800 f6 writing to non write segment
        f1852 f7 arpl
        f1880 f8 lar, lsl
        f18ca fa
        f10d8
        f10ec 35
        f1106 36
        f1137 !!!!!!!keyboard test fails

        f11aa 3a
        f1240 3c harddisk controller test!!!
        f13f3 3b
        f1a6d xthdd bios init
        f1429
        f1462
        f1493 40
        f1532
        keyboard lock
        f1 to unlock
        f155c
        jumps to f0050 (reset)  without enabling of the a20 gate --> hangs
        0412 bit 5 must be set to reach f1579
        f1579
        f15c3 41
        f1621 43

        routines
        f1945 read cmos ram
        f195f write to cmos al value ah
        f1a3a poll 0x61 bit 4
        f1a49 sets something in cmos ram
        f1d30 switch to protected mode

        neat
        ----
        f80b9

        at386
        -----
        fd28c fd
        fd2c3 fc
        f40dc
        fd949
        fd8e3
        fd982
        f4219 01
        f4296 03
        f42f3 04
        f4377 05
        f43ec 06
        f4430 08
        f6017 switches to PM
        f4456 09
        f55a2
        f44ec 0d
        f4557 20
        f462d 27 my special friend, the keyboard controller once more
        ed0a1
        f4679 28
        fa16a
        f46d6
        f4768 2c
        f47f0 2e
        f5081
        fa16a
        f9a83
            Message: "Checksum Error on Extended CMOS"
        f4840 34
        f488c 35
        reset
        f48ee
        f493e 3a
        f49cd
        f4fc7
        fe842
        f4a5a
        f4b01 38
            (Memory Test)
        f4b41 3b
        f4c0f
            Message: "Invalid configuration information - please run SETUP program"
        f4c5c
            f86fc
            f8838
        f4c80
        f4ca2
        f4d4c
        f4e15   (int 19h)

        [f9a83 output text at return address!, return after text]


        at486
        -----
        f81a5 03
        f1096 0f 09 wbinvd i486 instruction

*********************************************************************/


#include "emu.h"
#include "memconv.h"

#include "machine/pckeybrd.h"
#include "machine/8042kbdc.h"
#include "machine/pcshare.h"


/***************************************************************************

    Constants & macros

***************************************************************************/

#define PS2_MOUSE_ON	1
#define KEYBOARD_ON		1

#define LOG_KEYBOARD	0
#define LOG_ACCESSES	0



/***************************************************************************

    Type definitions

***************************************************************************/

static struct
{
	kbdc8042_type_t type;
	void (*set_gate_a20)(running_machine &machine, int a20);
	void (*keyboard_interrupt)(running_machine &machine, int state);
	void (*set_spkr)(running_machine &machine, int speaker);
	int (*get_out2)(running_machine &machine);

	UINT8 inport, outport, data, command;

	struct {
		int received;
		int on;
	} keyboard;
	struct {
		int received;
		int on;
	} mouse;

	int last_write_to_control;
	int sending;
	int send_to_mouse;

	int operation_write_state;
	int status_read_mode;

	int speaker;

	/* temporary hack */
	int offset1;
} kbdc8042;

static int poll_delay;

static void at_8042_check_keyboard(running_machine &machine);



/***************************************************************************

    Code

***************************************************************************/

static void at_8042_set_outport(running_machine &machine, UINT8 data, int initial)
{
	UINT8 change;
	change = initial ? 0xFF : (kbdc8042.outport ^ data);
	kbdc8042.outport = data;
	if (change & 0x02)
	{
		if (kbdc8042.set_gate_a20)
			kbdc8042.set_gate_a20(machine, data & 0x02 ? 1 : 0);
	}
}



static TIMER_CALLBACK( kbdc8042_time )
{
	at_keyboard_polling();
	at_8042_check_keyboard(machine);
}

static TIMER_CALLBACK( kbdc8042_clr_int )
{
	/* Lets 8952's timers do their job before clear the interrupt line, */
	/* else Keyboard interrupt never happens. */
	kbdc8042.keyboard_interrupt(machine, 0);
}


void kbdc8042_init(running_machine &machine, const struct kbdc8042_interface *intf)
{
	poll_delay = 10;
	memset(&kbdc8042, 0, sizeof(kbdc8042));
	kbdc8042.type = intf->type;
	kbdc8042.set_gate_a20 = intf->set_gate_a20;
	kbdc8042.keyboard_interrupt = intf->keyboard_interrupt;
	kbdc8042.get_out2 = intf->get_out2;
	kbdc8042.set_spkr = intf->set_spkr;

	/* ibmat bios wants 0x20 set! (keyboard locked when not set) 0x80 */
	kbdc8042.inport = 0xa0;
	at_8042_set_outport(machine, 0xfe, 1);

	machine.scheduler().timer_pulse(attotime::from_hz(60), FUNC(kbdc8042_time));
}

static void at_8042_receive(running_machine &machine, UINT8 data)
{
	if (LOG_KEYBOARD)
		logerror("at_8042_receive Received 0x%02x\n", data);

	kbdc8042.data = data;
	kbdc8042.keyboard.received = 1;

	if (kbdc8042.keyboard_interrupt)
	{
		kbdc8042.keyboard_interrupt(machine, 1);
		/* Lets 8952's timers do their job before clear the interrupt line, */
		/* else Keyboard interrupt never happens. */
		machine.scheduler().timer_set( attotime::from_usec(2), FUNC(kbdc8042_clr_int),0,0 );
	}
}

static void at_8042_check_keyboard(running_machine &machine)
{
	int data;

	if (!kbdc8042.keyboard.received
		&& !kbdc8042.mouse.received)
	{
		if ( (data = at_keyboard_read())!=-1)
			at_8042_receive(machine, data);
	}
}



static void at_8042_clear_keyboard_received(void)
{
	if (kbdc8042.keyboard.received)
	{
		if (LOG_KEYBOARD)
			logerror("kbdc8042_8_r(): Clearing kbdc8042.keyboard.received\n");
	}

	kbdc8042.keyboard.received = 0;
	kbdc8042.mouse.received = 0;
}



/* **************************************************************************
 * Port 0x60 Input and Output Buffer (keyboard and mouse data)
 * Port 0x64 Read Status Register
 *           Write operation for controller
 *
 *  Output port controller:
 *      7: Keyboard data
 *      6: Keyboard clock
 *      5: Mouse buffer full
 *      4: Keyboard buffer full
 *      3: Mouse clock
 *      2: Mouse data
 *      1: 0 A20 cleared
 *      0: 0 system reset
 *
 *  Input port controller
 *      7: 0=Keyboard Locked
 *      6: 1 = Monochrome 0 = Color (true for real IBM, clones are undefined and use CMOS RAM data)
 *      5..2: reserved
 *      1: Mouse data in
 *      0: Keyboard data in
 */

READ8_HANDLER(kbdc8042_8_r)
{
	UINT8 data = 0;

	switch (offset) {
	case 0:
		data = kbdc8042.data;
		if (kbdc8042.type != KBDC8042_AT386 || (data != 0x55))
		{
			/* at386 self test doesn't like this */
			at_8042_clear_keyboard_received();
		}
		at_8042_check_keyboard(space.machine());
		break;

	case 1:
		data = kbdc8042.speaker;
		data &= ~0xc0; /* AT BIOS don't likes this being set */

		/* needed for AMI BIOS, maybe only some keyboard controller revisions! */
		at_8042_clear_keyboard_received();

		/* polled for changes in ibmat bios */
		if (--poll_delay < 0)
		{
			if (kbdc8042.type != KBDC8042_PS2)
				poll_delay = 4; /* ibmat */
			else
				poll_delay = 8; /* ibm ps2m30 */
			kbdc8042.offset1 ^= 0x10;
		}
		data = (data & ~0x10) | kbdc8042.offset1;

		if (kbdc8042.speaker & 1)
			data |= 0x20;
		else
			data &= ~0x20; /* ps2m30 wants this */
		break;

	case 2:
		if (kbdc8042.get_out2(space.machine()))
			data |= 0x20;
		else
			data &= ~0x20;
		break;

	case 4:
		at_8042_check_keyboard(space.machine());

		if (kbdc8042.keyboard.received || kbdc8042.mouse.received)
			data |= 1;
		if (kbdc8042.sending)
			data |= 2;

		kbdc8042.sending = 0; /* quicker than normal */
		data |= 4; /* selftest ok */

		if (kbdc8042.last_write_to_control)
			data |= 8;

		switch (kbdc8042.status_read_mode) {
		case 0:
			if (!kbdc8042.keyboard.on) data|=0x10;
			if (kbdc8042.mouse.received) data|=0x20;
			break;
		case 1:
			data |= kbdc8042.inport&0xf;
			break;
		case 2:
			data |= kbdc8042.inport<<4;
			break;
		}
		break;
	}

	if (LOG_ACCESSES)
		logerror("kbdc8042_8_r(): offset=%d data=0x%02x\n", offset, (unsigned) data);
	return data;
}



WRITE8_HANDLER(kbdc8042_8_w)
{
	switch (offset) {
	case 0:
		kbdc8042.last_write_to_control = 0;
		kbdc8042.status_read_mode = 0;
		switch (kbdc8042.operation_write_state) {
		case 0:
			/* normal case */
			kbdc8042.data = data;
			kbdc8042.sending=1;
			at_keyboard_write(space.machine(), data);
			break;

		case 1:
			/* preceded by writing 0xD1 to port 60h
             *  |7|6|5|4|3|2|1|0|  8042 Output Port
             *   | | | | | | | `---- system reset line
             *   | | | | | | `----- gate A20
             *   | | | | `-------- undefined
             *   | | | `--------- output buffer full
             *   | | `---------- input buffer empty
             *   | `----------- keyboard clock (output)
             *   `------------ keyboard data (output)
             */
			at_8042_set_outport(space.machine(), data, 0);
			break;

		case 2:
			/* preceded by writing 0xD2 to port 60h */
			kbdc8042.data = data;
			kbdc8042.sending=1;
			at_keyboard_write(space.machine(), data);
			break;

		case 3:
			/* preceded by writing 0xD3 to port 60h */
			kbdc8042.data = data;
			break;

		case 4:
			/* preceded by writing 0xD4 to port 60h */
			kbdc8042.data = data;
			break;

		case 5:
			/* preceded by writing 0x60 to port 60h */
			kbdc8042.command = data;
			break;
		}
		kbdc8042.operation_write_state = 0;
		break;

	case 1:
		kbdc8042.speaker = data;
		if (kbdc8042.set_spkr)
					kbdc8042.set_spkr(space.machine(), kbdc8042.speaker);

		break;

	case 4:
		kbdc8042.last_write_to_control=0;

		/* switch based on the command */
		switch(data) {
		case 0x20:	/* current 8042 command byte is placed on port 60h */
			kbdc8042.data = kbdc8042.command;
			break;
		case 0x60:	/* next data byte is placed in 8042 command byte */
			kbdc8042.operation_write_state = 5;
			kbdc8042.send_to_mouse = 0;
			break;
		case 0xa7:	/* disable auxilary interface */
			kbdc8042.mouse.on = 0;
			break;
		case 0xa8:	/* enable auxilary interface */
			kbdc8042.mouse.on = 1;
			break;
		case 0xa9:	/* test mouse */
			at_8042_receive(space.machine(), PS2_MOUSE_ON ? 0x00 : 0xff);
			break;
		case 0xaa:	/* selftest */
			at_8042_receive(space.machine(), 0x55);
			break;
		case 0xab:	/* test keyboard */
			at_8042_receive(space.machine(), KEYBOARD_ON ? 0x00 : 0xff);
			break;
		case 0xad:	/* disable keyboard interface */
			kbdc8042.keyboard.on = 0;
			break;
		case 0xae:	/* enable keyboard interface */
			kbdc8042.keyboard.on = 1;
			break;
		case 0xc0:	/* read input port */
			/*  |7|6|5|4|3 2 1 0|  8042 Input Port
             *   | | | |    |
             *   | | | |    `------- undefined
             *   | | | |
             *   | | | `--------- 1=enable 2nd 256k of Motherboard RAM
             *   | | `---------- 1=manufacturing jumper installed
             *   | `----------- 1=primary display is MDA, 0=CGA
             *   `------------ 1=keyboard not inhibited; 0=inhibited
             */
			at_8042_receive(space.machine(), kbdc8042.inport);
			break;
		case 0xc1:	/* read input port 3..0 until write to 0x60 */
			kbdc8042.status_read_mode = 1;
			break;
		case 0xc2:	/* read input port 7..4 until write to 0x60 */
			kbdc8042.status_read_mode = 2;
			break;
		case 0xd0:	/* read output port */
			at_8042_receive(space.machine(), kbdc8042.outport);
			break;
		case 0xd1:
			/* write output port; next byte written to port 60h is placed on
             * 8042 output port */
			kbdc8042.operation_write_state = 1;
			break;
		case 0xd2:
			/* write keyboard output register; on PS/2 systems next port 60h
             * write is written to port 60h output register as if initiated
             * by a device; invokes interrupt if enabled */
			kbdc8042.operation_write_state = 2;
			kbdc8042.send_to_mouse = 0;
			break;
		case 0xd3:
			/* write auxillary output register; on PS/2 systems next port 60h
             * write is written to port 60h input register as if initiated
             * by a device; invokes interrupt if enabled */
			kbdc8042.operation_write_state = 3;
			kbdc8042.send_to_mouse = 1;
			break;
		case 0xd4:
			/* write auxillary device; on PS/2 systems the next data byte
             * written to input register a port at 60h is sent to the
             * auxiliary device  */
			kbdc8042.operation_write_state = 4;
			break;
		case 0xe0:
			/* read test inputs; read T1/T0 test inputs into bit 1/0 */
			at_8042_receive(space.machine(), 0x00);
			break;

		case 0xf0:
		case 0xf2:
		case 0xf4:
		case 0xf6:
		case 0xf8:
		case 0xfa:
		case 0xfc:
		case 0xfe:
			/* Commands 0xF0...0xFF causes certain output lines to be pulsed
             * low for six milliseconds.  The bits pulsed low correspond to
             * the bits low set in the command byte.  The only pulse that has
             * an effect currently is bit 0, which pulses the CPU's reset line
             */
			space.machine().firstcpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
			at_8042_set_outport(space.machine(), kbdc8042.outport | 0x02, 0);
			break;
		}
		kbdc8042.sending = 1;
		break;
	}
}


READ64_HANDLER( kbdc8042_64be_r )
{
	return read64be_with_read8_handler(kbdc8042_8_r, space, offset, mem_mask);
}



WRITE64_HANDLER( kbdc8042_64be_w )
{
	write64be_with_write8_handler(kbdc8042_8_w, space, offset, data, mem_mask);
}
