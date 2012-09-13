/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

  The Apple I used a Motorola 6820 PIA for its keyboard and display
  I/O.  The keyboard was mapped to PIA port A, and the display to port
  B.

  Port A, the keyboard, was an input port connected to a standard
  ASCII-encoded keyboard.  The high bit of the port was tied to +5V.
  The keyboard strobe signal was connected to the PIA's CA1 control
  input so that the keyboard could signal each keypress to the PIA.
  The processor could check for a keypress by testing the IRQA1 flag
  in the Port A Control Register and then reading the character value
  from Port A.

  The keyboard connector also had two special lines, RESET and CLEAR
  SCREEN, which were meant to be connected to pushbutton switches on
  the keyboard.  RESET was tied to the reset inputs for the CPU and
  PIA; it allowed the user to stop a program and return control to the
  Monitor.  CLEAR SCREEN was directly tied to the video hardware and
  would clear the display.

  Port B, the display, was an output port which accepted 7-bit ASCII
  characters from the PIA and wrote them on the display.  The details
  of this are described in video/apple1.c.  Control line CB2 served
  as an output signal to inform the display of a new character.  (CB2
  was also connected to line 7 of port B, which was configured as an
  input, so that the CPU could more easily check the status of the
  write.)  The CB1 control input signaled the PIA when the display had
  finished writing the character and could accept a new one.

  MAME models the 6821 instead of the earlier 6820 used in the Apple
  I, but there is no difference in functionality between the two
  chips; the 6821 simply has a better ability to drive electrical
  loads.

  The Apple I had an optional cassette interface which plugged into
  the expansion connector.  This is described below in the "Cassette
  interface I/O" section.

***************************************************************************/

#include "emu.h"
#include "includes/apple1.h"
#include "machine/6821pia.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"

static TIMER_CALLBACK(apple1_kbd_poll);
static TIMER_CALLBACK(apple1_kbd_strobe_end);

static READ8_DEVICE_HANDLER( apple1_pia0_kbdin );
static WRITE8_DEVICE_HANDLER( apple1_pia0_dspout );
static WRITE8_DEVICE_HANDLER( apple1_pia0_dsp_write_signal );

static TIMER_CALLBACK(apple1_dsp_ready_start);
static TIMER_CALLBACK(apple1_dsp_ready_end);

/*****************************************************************************
**  Structures
*****************************************************************************/

/* Handlers for the PIA.  The inputs for Port B, CA1, and CB1 are
   handled by writing to them at the moment when their values change,
   rather than updating them when they are read; thus they don't need
   handler functions. */

const pia6821_interface apple1_pia0 =
{
	DEVCB_DEVICE_HANDLER("pia", apple1_pia0_kbdin),				/* Port A input (keyboard) */
	DEVCB_NULL,										/* Port B input (display status) */
	DEVCB_NULL,										/* CA1 input (key pressed) */
	DEVCB_NULL,										/* CB1 input (display ready) */
	DEVCB_NULL,										/* CA2 not used as input */
	DEVCB_NULL,										/* CB2 not used as input */
	DEVCB_NULL,										/* Port A not used as output */
	DEVCB_DEVICE_HANDLER("pia", apple1_pia0_dspout),				/* Port B output (display) */
	DEVCB_NULL,										/* CA2 not used as output */
	DEVCB_DEVICE_HANDLER("pia", apple1_pia0_dsp_write_signal),	/* CB2 output (display write) */
	DEVCB_NULL,										/* IRQA not connected */
	DEVCB_NULL										/* IRQB not connected */
};

/* Use the same keyboard mapping as on a modern keyboard.  This is not
   the same as the keyboard mapping of the actual teletype-style
   keyboards used with the Apple I, but it's less likely to cause
   confusion for people who haven't memorized that layout.

   The Backspace key is mapped to the '_' (underscore) character
   because the Apple I ROM Monitor used "back-arrow" to erase
   characters, rather than backspace, and back-arrow is an earlier
   form of the underscore. */

#define ESCAPE	'\x1b'

static const UINT8 apple1_unshifted_keymap[] =
{
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '-', '=', '[', ']', ';', '\'',
	',', '.', '/', '\\', 'A', 'B', 'C', 'D',
	'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', '\r', '_',
	' ', ESCAPE
};

static const UINT8 apple1_shifted_keymap[] =
{
	')', '!', '@', '#', '$', '%', '^', '&',
	'*', '(', '_', '+', '[', ']', ':', '"',
	'<', '>', '?', '\\', 'A', 'B', 'C', 'D',
	'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', '\r', '_',
	' ', ESCAPE
};

/* Control key mappings, like the other mappings, conform to a modern
   keyboard where possible.  Note that the Apple I ROM Monitor ignores
   most control characters. */

static const UINT8 apple1_control_keymap[] =
{
	'0', '1', '\x00', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f',
	'8', '9', '\x1f', '=', '\x1b', '\x1d', ';', '\'',
	',', '.', '/', '\x1c', '\x01', '\x02', '\x03', '\x04',
	'\x05', '\x06', '\x07', '\x08', '\x09', '\x0a', '\x0b', '\x0c',
	'\x0d', '\x0e', '\x0f', '\x10', '\x11', '\x12', '\x13', '\x14',
	'\x15', '\x16', '\x17', '\x18', '\x19', '\x1a', '\r', '_',
	'\x00', ESCAPE
};



/*****************************************************************************
**  DRIVER_INIT:  driver-specific setup, executed once at MESS startup.
*****************************************************************************/

DRIVER_INIT_MEMBER(apple1_state,apple1)
{
	address_space* space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	/* Set up the handlers for MESS's dynamically-sized RAM. */
	space->install_readwrite_bank(0x0000, machine().device<ram_device>(RAM_TAG)->size() - 1, "bank1");
	membank("bank1")->set_base(machine().device<ram_device>(RAM_TAG)->pointer());

	/* Poll the keyboard input ports periodically.  These include both
       ordinary keys and the RESET and CLEAR SCREEN pushbutton
       switches.  We can't handle these switches in a VBLANK_INT or
       PERIODIC_INT because both switches need to be monitored even
       while the CPU is suspended during RESET; VBLANK_INT and
       PERIODIC_INT callbacks aren't run while the CPU is in this
       state.

       A 120-Hz poll rate seems to be fast enough to ensure no
       keystrokes are missed. */
	machine().scheduler().timer_pulse(attotime::from_hz(120), FUNC(apple1_kbd_poll));
}


/*****************************************************************************
**  MACHINE_RESET:  actions to perform on each cold boot.
*****************************************************************************/

void apple1_state::machine_reset()
{
	/* Reset the display hardware. */
	apple1_vh_dsp_clr(machine());
}


/*****************************************************************************
**  apple1_verify_header
*****************************************************************************/
static int apple1_verify_header (UINT8 *data)
{
	/* Verify the format for the snapshot */
	if ((data[0] == 'L') &&
		(data[1] == 'O') &&
		(data[2] == 'A') &&
		(data[3] == 'D') &&
		(data[4] == ':') &&
		(data[7] == 'D') &&
		(data[8] == 'A') &&
		(data[9] == 'T') &&
		(data[10]== 'A') &&
		(data[11]== ':'))
	{
		return(IMAGE_VERIFY_PASS);
	}
	else
	{
		return(IMAGE_VERIFY_FAIL);
	}
}

#define SNAP_HEADER_LEN			12

/*****************************************************************************
**  snapshot_load_apple1
**
**  Format of the binary snapshot image is:
**
**  [ LOAD:xxyyDATA:zzzzzz...]
**
**  where xxyy is the binary starting address (in big-endian byte
**  order) to load the binary data zzzzzz to.
**
**  The image can be of arbitrary length, but it must fit in available
**  memory.
*****************************************************************************/
SNAPSHOT_LOAD(apple1)
{
	UINT64 filesize, datasize;
	UINT8 *snapbuf, *snapptr;
	UINT16 start_addr, end_addr, addr;

	filesize = image.length();

	/* Read the snapshot data into a temporary array */
	if (filesize < SNAP_HEADER_LEN)
		return IMAGE_INIT_FAIL;
	snapbuf = (UINT8*)image.ptr();
	if (!snapbuf)
		return IMAGE_INIT_FAIL;

	/* Verify the snapshot header */
	if (apple1_verify_header(snapbuf) == IMAGE_VERIFY_FAIL)
	{
		logerror("apple1 - Snapshot Header is in incorrect format - needs to be LOAD:xxyyDATA:\n");
		return IMAGE_INIT_FAIL;
	}

	datasize = filesize - SNAP_HEADER_LEN;

	/* Extract the starting address to load the snapshot to. */
	start_addr = (snapbuf[5] << 8) | (snapbuf[6]);
	logerror("apple1 - LoadAddress is 0x%04x\n", start_addr);

	end_addr = start_addr + datasize - 1;

	if ((start_addr < 0xE000 && end_addr > image.device().machine().device<ram_device>(RAM_TAG)->size() - 1)
		|| end_addr > 0xEFFF)
	{
		logerror("apple1 - Snapshot won't fit in this memory configuration;\n"
			   "needs memory from $%04X to $%04X.\n", start_addr, end_addr);
		return IMAGE_INIT_FAIL;
	}

	/* Copy the data into memory space. */
	for (addr = start_addr, snapptr = snapbuf + SNAP_HEADER_LEN;
		 addr <= end_addr;
		 addr++, snapptr++)
		image.device().machine().device("maincpu")->memory().space(AS_PROGRAM)->write_byte(addr, *snapptr);


	return IMAGE_INIT_PASS;
}


/*****************************************************************************
**  apple1_kbd_poll
**
**  Keyboard polling handles both ordinary keys and the special RESET
**  and CLEAR SCREEN switches.
**
**  For ordinary keys, this implements 2-key rollover to reduce the
**  chance of missed keypresses.  If we press a key and then press a
**  second key while the first hasn't been completely released, as
**  might happen during rapid typing, only the second key is
**  registered; the first key is ignored.
**
**  If multiple newly-pressed keys are found, the one closest to the
**  end of the input ports list is counted; the others are ignored.
*****************************************************************************/
static TIMER_CALLBACK(apple1_kbd_poll)
{
	apple1_state *state = machine.driver_data<apple1_state>();
	int port, bit;
	int key_pressed;
	UINT32 shiftkeys, ctrlkeys;
	pia6821_device *pia = machine.device<pia6821_device>("pia");
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3" };

	/* This holds the values of all the input ports for ordinary keys
       seen during the last scan. */

	/* First we check the RESET and CLEAR SCREEN pushbutton switches. */

	/* The RESET switch resets the CPU and the 6820 PIA. */
	if (machine.root_device().ioport("KEY5")->read() & 0x0001)
	{
		if (!state->m_reset_flag) {
			state->m_reset_flag = 1;
			/* using PULSE_LINE does not allow us to press and hold key */
			machine.device("maincpu")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			pia->reset();
		}
	}
	else if (state->m_reset_flag) {
		/* RESET released--allow the processor to continue. */
		state->m_reset_flag = 0;
		machine.device("maincpu")->execute().set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}

	/* The CLEAR SCREEN switch clears the video hardware. */
	if (machine.root_device().ioport("KEY5")->read() & 0x0002)
	{
		if (!state->m_vh_clrscrn_pressed)
		{
			/* Ignore further video writes, and clear the screen. */
			state->m_vh_clrscrn_pressed = 1;
			apple1_vh_dsp_clr(machine);
		}
	}
	else if (state->m_vh_clrscrn_pressed)
	{
		/* CLEAR SCREEN released--pay attention to video writes again. */
		state->m_vh_clrscrn_pressed = 0;
	}

	/* Now we scan all the input ports for ordinary keys, recording
       new keypresses while ignoring keys that were already pressed in
       the last scan. */

	state->m_kbd_data = 0;
	key_pressed = 0;

	/* The keyboard strobe line should always be low when a scan starts. */
	pia->ca1_w(0);

	shiftkeys = machine.root_device().ioport("KEY4")->read() & 0x0003;
	ctrlkeys = machine.root_device().ioport("KEY4")->read() & 0x000c;

	for (port = 0; port < 4; port++)
	{
		UINT32 portval, newkeys;

		portval = machine.root_device().ioport(keynames[port])->read();
		newkeys = portval & ~(state->m_kbd_last_scan[port]);

		if (newkeys)
		{
			key_pressed = 1;
			for (bit = 0; bit < 16; bit++) {
				if (newkeys & 1)
				{
					state->m_kbd_data = (ctrlkeys)
					  ? apple1_control_keymap[port*16 + bit]
					  : (shiftkeys)
					  ? apple1_shifted_keymap[port*16 + bit]
					  : apple1_unshifted_keymap[port*16 + bit];
				}
				newkeys >>= 1;
			}
		}
		state->m_kbd_last_scan[port] = portval;
	}

	if (key_pressed)
	{
		/* The keyboard will pulse its strobe line when a key is
           pressed.  A 10-usec pulse is typical. */
		pia->ca1_w(1);
		machine.scheduler().timer_set(attotime::from_usec(10), FUNC(apple1_kbd_strobe_end));
	}
}

static TIMER_CALLBACK(apple1_kbd_strobe_end)
{
	pia6821_device *pia = machine.device<pia6821_device>("pia");

	/* End of the keyboard strobe pulse. */
	pia->ca1_w(0);
}


/*****************************************************************************
**  READ/WRITE HANDLERS
*****************************************************************************/
static READ8_DEVICE_HANDLER( apple1_pia0_kbdin )
{
	apple1_state *state = device->machine().driver_data<apple1_state>();
	/* Bit 7 of the keyboard input is permanently wired high.  This is
       what the ROM Monitor software expects. */
	return state->m_kbd_data | 0x80;
}

static WRITE8_DEVICE_HANDLER( apple1_pia0_dspout )
{
	/* Send an ASCII character to the video hardware. */
	apple1_vh_dsp_w(device->machine(), data);
}

static WRITE8_DEVICE_HANDLER( apple1_pia0_dsp_write_signal )
{
	/* PIA output CB2 is inverted to become the DA signal, used to
       signal a display write to the video hardware. */

	/* DA is directly connected to PIA input PB7, so the processor can
       read bit 7 of port B to test whether the display has completed
       a write. */
    pia6821_device *pia = downcast<pia6821_device *>(device);
	pia->portb_w((!data) << 7);

	/* Once DA is asserted, the display will wait until it can perform
       the write, when the cursor position is about to be refreshed.
       Only then will it assert \RDA to signal readiness for another
       write.  Thus the write delay depends on the cursor position and
       where the display is in the refresh cycle. */
	if (!data)
		device->machine().scheduler().timer_set(apple1_vh_dsp_time_to_ready(device->machine()), FUNC(apple1_dsp_ready_start));
}

static TIMER_CALLBACK(apple1_dsp_ready_start)
{
	pia6821_device *pia = machine.device<pia6821_device>("pia");

	/* When the display asserts \RDA to signal it is ready, it
       triggers a 74123 one-shot to send a 3.5-usec low pulse to PIA
       input CB1.  The end of this pulse will tell the PIA that the
       display is ready for another write. */
	pia->cb1_w(0);
	machine.scheduler().timer_set(attotime::from_nsec(3500), FUNC(apple1_dsp_ready_end));
}

static TIMER_CALLBACK(apple1_dsp_ready_end)
{
	pia6821_device *pia = machine.device<pia6821_device>("pia");

	/* The one-shot pulse has ended; return CB1 to high, so we can do
       another display write. */
	pia->cb1_w(1);
}


/*****************************************************************************
**  Cassette interface I/O
**
**  The Apple I's cassette interface was a small card that plugged
**  into the expansion connector on the motherboard.  (This was a
**  slot-type connector, separate from the motherboard's edge
**  connector, but with the same signals.)  The cassette interface
**  provided separate cassette input and output jacks, some very
**  simple interface hardware, and 256 bytes of ROM containing the
**  cassette I/O code.
**
**  The interface was mostly software-controlled.  The only hardware
**  was an output flip-flop for generating the cassette output signal,
**  a National Semiconductor LM311 voltage comparator for generating a
**  digital signal from the analog cassette input, an input
**  signal-level LED, and some gates to control the interface logic
**  and address decoding.  The cassette ROM code did most of the work
**  of generating and interpreting tape signals.  It also contained
**  its own mini-monitor for issuing tape read and write commands.
**
**  The cassette interface was assigned to the $C000-$CFFF block of
**  addresses, although it did not use most of the space in that
**  block.  Addresses were mapped as follows:
**
**      $C000-$C0FF:  Cassette I/O space.
**                    Any access here toggles the output signal.
**          $C000-$C07F:  Cassette output only; input disabled.
**                        Mirrors $C100-$C17F on reads.
**          $C080-$C0FF:  Cassette input and output.
**                        When input low, mirrors $C180-$C1FF on reads.
**                        When input high, both odd and even addresses
**                        mirror even ROM addresses $C180-$C1FE.
**      $C100-$C1FF:  Cassette ROM code.
**
**  Note the peculiar addressing scheme.  Data was written simply
**  through repeated accesses, rather than by writing to an address.
**  Data was read by reading an odd input address and comparing the
**  ROM byte returned to detect signal changes.
**
**  The standard tape signal was a simple square wave, although this
**  was often greatly distorted by the cassette recorder.  A single
**  tape record consisted of a 10-second 800-Hz leader, followed by a
**  single short square-wave cycle used as a sync bit, followed by the
**  tape data.  The data was encoded using a single square-wave cycle
**  for each bit; "1" bits were at 1000 Hz, "0" bits at 2000 Hz.  (All
**  of these frequencies are approximate and could vary due to
**  differences in recorder speed.)  Each byte was written starting
**  from the most significant bit; bytes were written from low to high
**  addresses.  No error detection was provided.  Multiple records
**  could be placed on a single tape.
*****************************************************************************/

static cassette_image_device *cassette_device_image(running_machine &machine)
{
	return machine.device<cassette_image_device>(CASSETTE_TAG);
}

/* The cassette output signal for writing tapes is generated by a
   flip-flop which is toggled to produce the output waveform.  Any
   access to the cassette I/O range, whether a read or a write,
   toggles this flip-flop. */
static void cassette_toggle_output(running_machine &machine)
{
	apple1_state *state = machine.driver_data<apple1_state>();
	state->m_cassette_output_flipflop = !state->m_cassette_output_flipflop;
	cassette_device_image(machine)->output(state->m_cassette_output_flipflop ? 1.0 : -1.0);
}

READ8_MEMBER(apple1_state::apple1_cassette_r)
{
	cassette_toggle_output(machine());

	if (offset <= 0x7f)
	{
		/* If the access is to address range $C000-$C07F, the cassette
           input signal is ignored .  In this case the value read
           always comes from the corresponding cassette ROM location
           in $C100-$C17F. */

		return space.read_byte(0xc100 + offset);
	}
    else
	{
		/* For accesses to address range $C080-$C0FF, the cassette
           input signal is enabled.  If the signal is low, the value
           read comes from the corresponding cassette ROM location in
           $C180-$C1FF.  If the signal is high, the low bit of the
           address is masked before the corresponding cassette ROM
           location is accessed; e.g., a read from $C081 would return
           the ROM byte at $C180.  The cassette ROM routines detect
           changes in the cassette input signal by repeatedly reading
           from $C081 and comparing the values read. */

		/* (Don't try putting a non-zero "noise threshhold" here,
           because it can cause tape header bits on real cassette
           images to be misread as data bits.) */
		if (cassette_device_image(machine())->input() > 0.0)
			return space.read_byte(0xc100 + (offset & ~1));
		else
			return space.read_byte(0xc100 + offset);
	}
}

WRITE8_MEMBER(apple1_state::apple1_cassette_w)
{
	/* Writes toggle the output flip-flop in the same way that reads
       do; other than that they have no effect.  Any repeated accesses
       to the cassette I/O address range can be used to write data to
       cassette, and the cassette ROM always uses reads to do this.
       However, we still have to handle writes, since they may be done
       by user code. */

	cassette_toggle_output(machine());
}
