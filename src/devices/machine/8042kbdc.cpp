// license:BSD-3-Clause
// copyright-holders:Peter Trauner
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


#include "machine/8042kbdc.h"


/***************************************************************************

    Constants & macros

***************************************************************************/

#define PS2_MOUSE_ON    1
#define KEYBOARD_ON     1

#define LOG_KEYBOARD    0
#define LOG_ACCESSES    0

const device_type KBDC8042 = &device_creator<kbdc8042_device>;

//-------------------------------------------------
//  kbdc8042_device - constructor
//-------------------------------------------------

kbdc8042_device::kbdc8042_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, KBDC8042, "8042 Keyboard Controller", tag, owner, clock, "kbdc8042", __FILE__)
	, m_keyboard_dev(*this, "at_keyboard")
	, m_system_reset_cb(*this)
	, m_gate_a20_cb(*this)
	, m_input_buffer_full_cb(*this)
	, m_output_buffer_empty_cb(*this)
	, m_speaker_cb(*this)
{
}

static MACHINE_CONFIG_FRAGMENT( keyboard )
	MCFG_AT_KEYB_ADD("at_keyboard", 1, WRITELINE(kbdc8042_device, keyboard_w))
MACHINE_CONFIG_END

machine_config_constructor kbdc8042_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( keyboard );
}

/*-------------------------------------------------
    device_start - device-specific startup
-------------------------------------------------*/

void kbdc8042_device::device_start()
{
	// resolve callbacks
	m_system_reset_cb.resolve_safe();
	m_gate_a20_cb.resolve_safe();
	m_input_buffer_full_cb.resolve_safe();
	m_output_buffer_empty_cb.resolve_safe();
	m_speaker_cb.resolve_safe();
	m_operation_write_state = 0; /* first write to 0x60 might occur before anything can set this */
	memset(&m_keyboard, 0x00, sizeof(m_keyboard));
	memset(&m_mouse, 0x00, sizeof(m_mouse));
	m_sending = 0;
	m_last_write_to_control = 0;
	m_status_read_mode = 0;
}

/*-------------------------------------------------
    device_reset - device-specific reset
-------------------------------------------------*/

void kbdc8042_device::device_reset()
{
	m_poll_delay = 10;

	/* ibmat bios wants 0x20 set! (keyboard locked when not set) 0x80 */
	m_inport = 0xa0;
	at_8042_set_outport(0xfe, 1);
}

void kbdc8042_device::at_8042_set_outport(UINT8 data, int initial)
{
	UINT8 change;
	change = initial ? 0xFF : (m_outport ^ data);
	m_outport = data;
	if (change & 0x02)
	{
		if (!m_gate_a20_cb.isnull())
			m_gate_a20_cb(data & 0x02 ? 1 : 0);
	}
}

WRITE_LINE_MEMBER( kbdc8042_device::keyboard_w )
{
	if(state)
		at_8042_check_keyboard();
}

TIMER_CALLBACK_MEMBER( kbdc8042_device::kbdc8042_clr_int )
{
	/* Lets 8952's timers do their job before clear the interrupt line, */
	/* else Keyboard interrupt never happens. */
	m_input_buffer_full_cb(0);
}

void kbdc8042_device::at_8042_receive(UINT8 data)
{
	if (LOG_KEYBOARD)
		logerror("at_8042_receive Received 0x%02x\n", data);

	m_data = data;
	m_keyboard.received = 1;

	if (!m_input_buffer_full_cb.isnull())
	{
		m_input_buffer_full_cb(1);
		/* Lets 8952's timers do their job before clear the interrupt line, */
		/* else Keyboard interrupt never happens. */
		/* Why was this done?  It dies when an extended scan code is received */
		//machine().scheduler().timer_set(attotime::from_usec(2), timer_expired_delegate(FUNC(kbdc8042_device::kbdc8042_clr_int),this));
	}
}

void kbdc8042_device::at_8042_check_keyboard()
{
	int data;

	if (!m_keyboard.received && !m_mouse.received)
	{
		if((data = m_keyboard_dev->read(machine().driver_data()->generic_space(), 0)))
			at_8042_receive(data);
	}
}


void kbdc8042_device::at_8042_clear_keyboard_received()
{
	if (m_keyboard.received)
	{
		if (LOG_KEYBOARD)
			logerror("kbdc8042_8_r(): Clearing m_keyboard.received\n");
	}

	m_keyboard.received = 0;
	m_mouse.received = 0;
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

READ8_MEMBER(kbdc8042_device::data_r)
{
	UINT8 data = 0;

	switch (offset) {
	case 0:
		data = m_data;
		m_input_buffer_full_cb(0);
		if ((m_status_read_mode != 3) || (data != 0xfa))
		{
			if (m_keybtype != KBDC8042_AT386 || (data != 0x55))
			{
				/* at386 self test doesn't like this */
				at_8042_clear_keyboard_received();
			}
			at_8042_check_keyboard();
		}
		else
		{
			m_status_read_mode = 4;
		}
		break;

	case 1:
		data = m_speaker;
		data &= ~0xc0; /* AT BIOS don't likes this being set */

		/* needed for AMI BIOS, maybe only some keyboard controller revisions! */
		at_8042_clear_keyboard_received();

		/* polled for changes in ibmat bios */
		if (--m_poll_delay < 0)
		{
			if (m_keybtype != KBDC8042_PS2)
				m_poll_delay = 4; /* ibmat */
			else
				m_poll_delay = 8; /* ibm ps2m30 */
			m_offset1 ^= 0x10;
		}
		data = (data & ~0x10) | m_offset1;

		if (m_speaker & 1)
			data |= 0x20;
		else
			data &= ~0x20; /* ps2m30 wants this */
		break;

	case 2:
		if (m_out2)
			data |= 0x20;
		else
			data &= ~0x20;
		break;

	case 4:
		at_8042_check_keyboard();

		if (m_keyboard.received || m_mouse.received)
			data |= 1;
		if (m_sending)
			data |= 2;

		m_sending = 0; /* quicker than normal */
		data |= 4; /* selftest ok */

		if (m_last_write_to_control)
			data |= 8;

		switch (m_status_read_mode) {
		case 0:
			if (!m_keyboard.on) data|=0x10;
			if (m_mouse.received) data|=0x20;
			break;
		case 1:
			data |= m_inport&0xf;
			break;
		case 2:
			data |= m_inport<<4;
			break;
		case 4:
			at_8042_receive(0xaa);
			m_status_read_mode = 0;
			break;
		}
		break;
	}

	if (LOG_ACCESSES)
		logerror("kbdc8042_8_r(): offset=%d data=0x%02x\n", offset, (unsigned) data);
	return data;
}



WRITE8_MEMBER(kbdc8042_device::data_w)
{
	switch (offset) {
	case 0:
		m_last_write_to_control = 0;
		m_status_read_mode = 0;
		switch (m_operation_write_state) {
		case 0:
			if ((data == 0xf4) || (data == 0xff)) /* keyboard enable or keyboard reset */
			{
				at_8042_receive(0xfa); /* ACK, delivered a bit differently */

				if (data == 0xff)
				{
					m_status_read_mode = 3; /* keyboard buffer to be written again after next read */
				}

				break;
			}

			/* normal case */
			m_data = data;
			m_sending=1;
			m_keyboard_dev->write(space, 0, data);
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
			at_8042_set_outport(data, 0);
			break;

		case 2:
			/* preceded by writing 0xD2 to port 60h */
			m_data = data;
			m_sending=1;
			m_keyboard_dev->write(space, 0, data);
			break;

		case 3:
			/* preceded by writing 0xD3 to port 60h */
			m_data = data;
			break;

		case 4:
			/* preceded by writing 0xD4 to port 60h */
			m_data = data;
			break;

		case 5:
			/* preceded by writing 0x60 to port 60h */
			m_command = data;
			break;
		}
		m_operation_write_state = 0;
		break;

	case 1:
		m_speaker = data;
		if (!m_speaker_cb.isnull())
					m_speaker_cb((offs_t)0, m_speaker);

		break;

	case 4:
		m_last_write_to_control=0;

		/* switch based on the command */
		switch(data) {
		case 0x20:  /* current 8042 command byte is placed on port 60h */
			m_data = m_command;
			break;
		case 0x60:  /* next data byte is placed in 8042 command byte */
			m_operation_write_state = 5;
			m_send_to_mouse = 0;
			break;
		case 0xa7:  /* disable auxilary interface */
			m_mouse.on = 0;
			break;
		case 0xa8:  /* enable auxilary interface */
			m_mouse.on = 1;
			break;
		case 0xa9:  /* test mouse */
			at_8042_receive(PS2_MOUSE_ON ? 0x00 : 0xff);
			break;
		case 0xaa:  /* selftest */
			at_8042_receive(0x55);
			break;
		case 0xab:  /* test keyboard */
			at_8042_receive(KEYBOARD_ON ? 0x00 : 0xff);
			break;
		case 0xad:  /* disable keyboard interface */
			m_keyboard.on = 0;
			break;
		case 0xae:  /* enable keyboard interface */
			m_keyboard.on = 1;
			break;
		case 0xc0:  /* read input port */
			/*  |7|6|5|4|3 2 1 0|  8042 Input Port
			 *   | | | |    |
			 *   | | | |    `------- undefined
			 *   | | | |
			 *   | | | `--------- 1=enable 2nd 256k of Motherboard RAM
			 *   | | `---------- 1=manufacturing jumper installed
			 *   | `----------- 1=primary display is MDA, 0=CGA
			 *   `------------ 1=keyboard not inhibited; 0=inhibited
			 */
			at_8042_receive(m_inport);
			break;
		case 0xc1:  /* read input port 3..0 until write to 0x60 */
			m_status_read_mode = 1;
			break;
		case 0xc2:  /* read input port 7..4 until write to 0x60 */
			m_status_read_mode = 2;
			break;
		case 0xd0:  /* read output port */
			at_8042_receive(m_outport);
			break;
		case 0xd1:
			/* write output port; next byte written to port 60h is placed on
			 * 8042 output port */
			m_operation_write_state = 1;
			return; /* instant delivery */
		case 0xd2:
			/* write keyboard output register; on PS/2 systems next port 60h
			 * write is written to port 60h output register as if initiated
			 * by a device; invokes interrupt if enabled */
			m_operation_write_state = 2;
			m_send_to_mouse = 0;
			break;
		case 0xd3:
			/* write auxillary output register; on PS/2 systems next port 60h
			 * write is written to port 60h input register as if initiated
			 * by a device; invokes interrupt if enabled */
			m_operation_write_state = 3;
			m_send_to_mouse = 1;
			break;
		case 0xd4:
			/* write auxillary device; on PS/2 systems the next data byte
			 * written to input register a port at 60h is sent to the
			 * auxiliary device  */
			m_operation_write_state = 4;
			break;
		case 0xe0:
			/* read test inputs; read T1/T0 test inputs into bit 1/0 */
			at_8042_receive(0x00);
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
			m_system_reset_cb(ASSERT_LINE);
			m_system_reset_cb(CLEAR_LINE);
			at_8042_set_outport(m_outport | 0x02, 0);
			break;
		}
		m_sending = 1;
		break;
	}
}

WRITE_LINE_MEMBER(kbdc8042_device::write_out2)
{
	m_out2 = state;
}
