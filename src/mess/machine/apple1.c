// license:???
// copyright-holders:Paul Daniels, Colin Howell, R. Belmont
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

/*****************************************************************************
**  Structures
*****************************************************************************/

/* Use the same keyboard mapping as on a modern keyboard.  This is not
   the same as the keyboard mapping of the actual teletype-style
   keyboards used with the Apple I, but it's less likely to cause
   confusion for people who haven't memorized that layout.

   The Backspace key is mapped to the '_' (underscore) character
   because the Apple I ROM Monitor used "back-arrow" to erase
   characters, rather than backspace, and back-arrow is an earlier
   form of the underscore. */

#define ESCAPE  '\x1b'

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
	address_space& space = m_maincpu->space(AS_PROGRAM);
	/* Set up the handlers for MESS's dynamically-sized RAM. */
	space.install_readwrite_bank(0x0000, m_ram->size() - 1, "bank1");
	membank("bank1")->set_base(m_ram->pointer());

	/* Poll the keyboard input ports periodically.  These include both
	   ordinary keys and the RESET and CLEAR SCREEN pushbutton
	   switches.  We can't handle these switches in a VBLANK_INT or
	   PERIODIC_INT because both switches need to be monitored even
	   while the CPU is suspended during RESET; VBLANK_INT and
	   PERIODIC_INT callbacks aren't run while the CPU is in this
	   state.

	   A 120-Hz poll rate seems to be fast enough to ensure no
	   keystrokes are missed. */
	machine().scheduler().timer_pulse(attotime::from_hz(120), timer_expired_delegate(FUNC(apple1_state::apple1_kbd_poll),this));
}


void apple1_state::machine_reset()
{
	/* Reset the display hardware. */
	apple1_vh_dsp_clr();
}


/*****************************************************************************
**  apple1_verify_header
*****************************************************************************/
int apple1_state::apple1_verify_header (UINT8 *data)
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

#define SNAP_HEADER_LEN         12

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
SNAPSHOT_LOAD_MEMBER( apple1_state,apple1)
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

	if ((start_addr < 0xE000 && end_addr > m_ram->size() - 1)
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
		m_maincpu->space(AS_PROGRAM).write_byte(addr, *snapptr);


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
TIMER_CALLBACK_MEMBER(apple1_state::apple1_kbd_poll)
{
	int port, bit;
	int key_pressed;
	UINT32 shiftkeys, ctrlkeys;
	pia6821_device *pia = machine().device<pia6821_device>("pia");
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3" };

	/* This holds the values of all the input ports for ordinary keys
	   seen during the last scan. */

	/* First we check the RESET and CLEAR SCREEN pushbutton switches. */

	/* The RESET switch resets the CPU and the 6820 PIA. */
	if (ioport("KEY5")->read() & 0x0001)
	{
		if (!m_reset_flag) {
			m_reset_flag = 1;
			/* using PULSE_LINE does not allow us to press and hold key */
			m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			pia->reset();
		}
	}
	else if (m_reset_flag) {
		/* RESET released--allow the processor to continue. */
		m_reset_flag = 0;
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}

	/* The CLEAR SCREEN switch clears the video hardware. */
	if (ioport("KEY5")->read() & 0x0002)
	{
		if (!m_vh_clrscrn_pressed)
		{
			/* Ignore further video writes, and clear the screen. */
			m_vh_clrscrn_pressed = 1;
			apple1_vh_dsp_clr();
		}
	}
	else if (m_vh_clrscrn_pressed)
	{
		/* CLEAR SCREEN released--pay attention to video writes again. */
		m_vh_clrscrn_pressed = 0;
	}

	/* Now we scan all the input ports for ordinary keys, recording
	   new keypresses while ignoring keys that were already pressed in
	   the last scan. */

	m_kbd_data = 0;
	key_pressed = 0;

	/* The keyboard strobe line should always be low when a scan starts. */
	pia->ca1_w(0);

	shiftkeys = ioport("KEY4")->read() & 0x0003;
	ctrlkeys = ioport("KEY4")->read() & 0x000c;

	for (port = 0; port < 4; port++)
	{
		UINT32 portval, newkeys;

		portval = ioport(keynames[port])->read();
		newkeys = portval & ~(m_kbd_last_scan[port]);

		if (newkeys)
		{
			key_pressed = 1;
			for (bit = 0; bit < 16; bit++) {
				if (newkeys & 1)
				{
					m_kbd_data = (ctrlkeys)
						? apple1_control_keymap[port*16 + bit]
						: (shiftkeys)
						? apple1_shifted_keymap[port*16 + bit]
						: apple1_unshifted_keymap[port*16 + bit];
				}
				newkeys >>= 1;
			}
		}
		m_kbd_last_scan[port] = portval;
	}

	if (key_pressed)
	{
		/* The keyboard will pulse its strobe line when a key is
		   pressed.  A 10-usec pulse is typical. */
		pia->ca1_w(1);
		machine().scheduler().timer_set(attotime::from_usec(10), timer_expired_delegate(FUNC(apple1_state::apple1_kbd_strobe_end),this));
	}
}

TIMER_CALLBACK_MEMBER(apple1_state::apple1_kbd_strobe_end)
{
	pia6821_device *pia = machine().device<pia6821_device>("pia");

	/* End of the keyboard strobe pulse. */
	pia->ca1_w(0);
}


/*****************************************************************************
**  READ/WRITE HANDLERS
*****************************************************************************/
READ8_MEMBER(apple1_state::apple1_pia0_kbdin)
{
	/* Bit 7 of the keyboard input is permanently wired high.  This is
	   what the ROM Monitor software expects. */
	return m_kbd_data | 0x80;
}

WRITE8_MEMBER(apple1_state::apple1_pia0_dspout)
{
	/* Send an ASCII character to the video hardware. */
	apple1_vh_dsp_w(data);
}

WRITE_LINE_MEMBER(apple1_state::apple1_pia0_dsp_write_signal)
{
	device_t *device = machine().device("pia");
	/* PIA output CB2 is inverted to become the DA signal, used to
	   signal a display write to the video hardware. */

	/* DA is directly connected to PIA input PB7, so the processor can
	   read bit 7 of port B to test whether the display has completed
	   a write. */
	pia6821_device *pia = downcast<pia6821_device *>(device);
	pia->portb_w((!state) << 7);

	/* Once DA is asserted, the display will wait until it can perform
	   the write, when the cursor position is about to be refreshed.
	   Only then will it assert \RDA to signal readiness for another
	   write.  Thus the write delay depends on the cursor position and
	   where the display is in the refresh cycle. */
	if (!state)
		machine().scheduler().timer_set(apple1_vh_dsp_time_to_ready(), timer_expired_delegate(FUNC(apple1_state::apple1_dsp_ready_start),this));
}

TIMER_CALLBACK_MEMBER(apple1_state::apple1_dsp_ready_start)
{
	pia6821_device *pia = machine().device<pia6821_device>("pia");

	/* When the display asserts \RDA to signal it is ready, it
	   triggers a 74123 one-shot to send a 3.5-usec low pulse to PIA
	   input CB1.  The end of this pulse will tell the PIA that the
	   display is ready for another write. */
	pia->cb1_w(0);
	machine().scheduler().timer_set(attotime::from_nsec(3500), timer_expired_delegate(FUNC(apple1_state::apple1_dsp_ready_end),this));
}

TIMER_CALLBACK_MEMBER(apple1_state::apple1_dsp_ready_end)
{
	pia6821_device *pia = machine().device<pia6821_device>("pia");

	/* The one-shot pulse has ended; return CB1 to high, so we can do
	   another display write. */
	pia->cb1_w(1);
}
