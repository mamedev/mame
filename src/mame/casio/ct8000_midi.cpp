// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************
    Casiotone 8000 / Symphonytron MIDI interface

    Translates MIDI messages into the 4-bit parallel protocol used by the Symphonytron.
    Aside from Note Off/On and Program Change messages, this also handles:
    - CC #1 (vibrato)
    - CC #64 (sustain pedal)
    - CC #69 (sustain switch)
    - CC #91 (reverb)
    - CC #93 (chorus)
    - CC #120 (all sound off)
    - CC #121 (reset controllers)
    - CC #123 (all notes off)
    - CC #124-127 (equivalent to 123, otherwise no effect)
    - MIDI reset (status 0xFF)

    Vibrato, reverb, and chorus CCs all behave as switches, where 64-127 is "on".

-------------------------------------------------------------------------------

    Summary of 4-bit Symphonytron messages

    Messages are sent in 4-bit increments over the DIN connector, in the following order:
    - high nibble of command
    - number of following nibbles (normally 1 or 3)
    - low nibble of command
    - high nibble of data (if any)
    - low nibble of data (if any)

    The exception is the reset command, which is a single 0xF nibble.

    The CT-8000 recognizes these messages:
    10 xx - set effects
    11 xx - lock effects (corresponding buttons/pedal do nothing while set)
    12 xx - ??? effects
       ^^ bit 0 = reverb
          bit 1 = vibrato
          bit 2 = sustain pedal (only affects the local keyboard)
          bit 3 = stereo chorus
          bit 4 = sustain switch

    20 xx - set lowest possible note to send (default is none)
    21 xx - set highest possible note to send (default is none)
    22 xx - set lowest possible note to receive/play (default is 0x21 = C2)
    23 xx - set highest possible note to receive/play (default is 0x61 = C6)
    24    - stop all notes
    28 xx - note on or off
       ^^ bit 7 = note on, bits 6-4 = octave, bits 3-0 = note from 0x1 (C) to 0xC (B)

    C0 xx - set tuning (signed, 1.5 cent increments, 0x32 = quarter tone up)

    E1 xx - set patch number

    F     - reset instrument

    The MB-1 recognizes all of the above, plus:
    A0 xx - ???
    A1 xx - ???
    A2 xx - ???
    A3 xx - ???
    A4    - clear values from A0-A3

***************************************************************************/

#include "emu.h"
#include "ct8000_midi.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(CT8000_MIDI, ct8000_midi_device, "ct8000_midi", "Casiotone 8000 MIDI adapter")

namespace {

INPUT_PORTS_START(ct8000_midi)
	PORT_START("CHANNEL")
	PORT_CONFNAME(0xff, 0x00, "MIDI channel")
	PORT_CONFSETTING(   0x00, "1")
	PORT_CONFSETTING(   0x01, "2")
	PORT_CONFSETTING(   0x02, "3")
	PORT_CONFSETTING(   0x03, "4")
	PORT_CONFSETTING(   0x04, "5")
	PORT_CONFSETTING(   0x05, "6")
	PORT_CONFSETTING(   0x06, "7")
	PORT_CONFSETTING(   0x07, "8")
	PORT_CONFSETTING(   0x08, "9")
	PORT_CONFSETTING(   0x09, "10")
	PORT_CONFSETTING(   0x0a, "11")
	PORT_CONFSETTING(   0x0b, "12")
	PORT_CONFSETTING(   0x0c, "13")
	PORT_CONFSETTING(   0x0d, "14")
	PORT_CONFSETTING(   0x0e, "15")
	PORT_CONFSETTING(   0x0f, "16")
INPUT_PORTS_END

} // anonymous namespace

ct8000_midi_device::ct8000_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CT8000_MIDI, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_channel(*this, "CHANNEL")
	, m_int_cb(*this)
	, m_base_program(0)
{
}

/**************************************************************************/
ioport_constructor ct8000_midi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ct8000_midi);
}

/**************************************************************************/
void ct8000_midi_device::device_start()
{
	m_midi_data[0] = m_midi_data[1] = 0;

	m_ack = 1;
	std::fill(std::begin(m_out_data), std::end(m_out_data), 0);

	m_in_data = 0xf;
	m_in_strobe = 0;

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(31250);
	set_tra_rate(0);

	save_item(NAME(m_status));
	save_item(NAME(m_midi_data));
	save_item(NAME(m_midi_data_pos));
	save_item(NAME(m_midi_data_size));
	save_item(NAME(m_midi_notes_on));
	save_item(NAME(m_sustain));

	save_item(NAME(m_ack));
	save_item(NAME(m_int));
	save_item(NAME(m_out_data));
	save_item(NAME(m_out_data_read));
	save_item(NAME(m_out_data_write));
	save_item(NAME(m_out_data_space));
	save_item(NAME(m_out_data_controls));

	save_item(NAME(m_in_data));
	save_item(NAME(m_in_strobe));
}

/**************************************************************************/
void ct8000_midi_device::device_reset()
{
	m_status = 0;
	m_midi_data_pos = m_midi_data_size = 0;
	m_out_data_space = sizeof(m_out_data);
	m_sustain = 0;

	std::fill(std::begin(m_midi_notes_on), std::begin(m_midi_notes_on), 0);

	m_out_data_read = m_out_data_write = 0;
	m_out_data_controls = 0;

	m_int_cb(m_int = 0);
}

/**************************************************************************/
void ct8000_midi_device::rcv_complete()
{
	receive_register_extract();

	if (is_receive_framing_error())
	{
		m_status = 0;
		return;
	}

	const u8 data = get_received_char();
	if (data < 0x80)
	{
		if (m_status >= 0x80)
		{
			m_midi_data[m_midi_data_pos++] = data;
			if (m_midi_data_pos == m_midi_data_size)
			{
				midi_message();
				m_midi_data_pos = 0;
			}
		}
	}
	else if (data < 0xf0)
	{
		m_status = data;

		if (data >= 0xc0 && data <= 0xdf)
			m_midi_data_size = 1;
		else
			m_midi_data_size = 2;
		m_midi_data_pos = 0;
	}
	else if (data < 0xf8)
	{
		m_status = 0;
	}
	else if (data == 0xff)
	{
		reset();
		send_nibble(0xf);
	}
}

/**************************************************************************/
u8 ct8000_midi_device::data_r()
{
	return m_out_data[m_out_data_read];
}

/**************************************************************************/
void ct8000_midi_device::ack_w(int state)
{
	if (m_ack && !state)
	{
		m_int_cb(m_int = 0);
		m_out_data_read++;
		m_out_data_space++;
		m_out_data_read %= sizeof(m_out_data);
	}
	else if (!m_ack && state)
	{
		if (m_out_data_read != m_out_data_write)
			m_int_cb(m_int = 1);
	}

	m_ack = state;
}

/**************************************************************************/
void ct8000_midi_device::strobe_w(int state)
{
	if (!m_in_strobe && state)
		logerror("data write: %x\n", m_in_data);

	m_in_strobe = state;
}

/**************************************************************************/
void ct8000_midi_device::midi_message()
{
	if ((m_status & 0xf) != m_channel->read())
		return;

	switch (m_status >> 4)
	{
	case 0x8: // note off
		m_midi_data[1] = 0;
		[[fallthrough]];
	case 0x9: // note on
		if (m_midi_data[0] >= NOTE_MIN && m_midi_data[0] <= NOTE_MAX)
			send_note(m_midi_data[0] - NOTE_MIN, m_midi_data[1]);
		break;

	case 0xb: // controller
		switch (m_midi_data[0])
		{
		case 1: // mod wheel / vibrato
			update_control(FX_VIBRATO);
			break;

		case 64: // sustain pedal
			update_sustain(m_midi_data[1]);
			break;

		case 69: // hold 2 / sustain switch
			update_control(FX_SUSTAIN);
			break;

		case 91: // reverb
			update_control(FX_REVERB);
			break;

		case 93: // chorus
			update_control(FX_CHORUS);
			break;

		case 120: // all sound off
			send_cmd(CMD_STOP);
			break;

		case 121: // reset controllers
			m_out_data_controls = 0;
			send_cmd(CMD_EFFECTS, 0);
			update_sustain(0);
			break;

		case 123: // all notes off
		case 124: // omni off
		case 125: // omni on
		case 126: // mono mode
		case 127: // poly mode
			for (unsigned i = 0; i < sizeof(m_midi_notes_on); i++)
			{
				if (m_midi_notes_on[i])
					send_note(i, 0);
			}
			break;
		}
		break;

	case 0xc: // program change
		send_cmd(CMD_TONE, m_midi_data[0] + m_base_program);
		break;
	}
}

/**************************************************************************/
void ct8000_midi_device::send_nibble(u8 data)
{
	m_out_data[m_out_data_write++] = data;
	m_out_data_write %= sizeof(m_out_data);
	m_out_data_space--;

	if (m_ack && !m_int)
		m_int_cb(m_int = 1);
}

/**************************************************************************/
void ct8000_midi_device::send_cmd(u8 cmd)
{
	if (m_out_data_space >= 3)
	{
		send_nibble(cmd >> 4);
		send_nibble(1); // number of following nibbles
		send_nibble(cmd & 0xf);
	}
}

/**************************************************************************/
void ct8000_midi_device::send_cmd(u8 cmd, u8 data)
{
	if (m_out_data_space >= 5)
	{
		send_nibble(cmd >> 4);
		send_nibble(3); // number of following nibbles
		send_nibble(cmd & 0xf);
		send_nibble(data >> 4);
		send_nibble(data & 0xf);
	}
}

/**************************************************************************/
void ct8000_midi_device::send_note(u8 note, u8 on)
{
	if (!on && m_sustain)
	{
		m_midi_notes_on[note] = 0x80;
		return;
	}

	u8 out_note = ((note / 12) << 4) + (note % 12) + 1;

	if (on && m_midi_notes_on[note])
	{
		// force existing note off even if sustained
		send_cmd(CMD_NOTE, out_note);
	}

	if (on)
		out_note |= 0x80;

	send_cmd(CMD_NOTE, out_note);
	m_midi_notes_on[note] = on;
}

/**************************************************************************/
void ct8000_midi_device::update_control(u8 control)
{
	if (m_midi_data[1] >= 0x40)
	{
		if (m_out_data_controls & control)
			return;

		m_out_data_controls |= control;
	}
	else
	{
		if (!(m_out_data_controls & control))
			return;

		m_out_data_controls &= ~control;
	}

	send_cmd(CMD_EFFECTS, m_out_data_controls);
}

/**************************************************************************/
void ct8000_midi_device::update_sustain(u8 value)
{
	if (value >= 0x40)
	{
		m_sustain = 1;
	}
	else if (m_sustain)
	{
		m_sustain = 0;
		for (unsigned i = 0; i < sizeof(m_midi_notes_on); i++)
		{
			if (BIT(m_midi_notes_on[i], 7))
				send_note(i, 0);
		}
	}
}
