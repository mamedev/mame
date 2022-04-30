// license:BSD-3-Clause
// copyright-holders:R. Belmont,Aaron Giles
/*********************************************************************

    midiin.c

    MIDI In image device and serial transmitter

*********************************************************************/

#include "emu.h"
#include "midiin.h"
#include "osdepend.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

DEFINE_DEVICE_TYPE(MIDIIN, midiin_device, "midiin", "MIDI In image device")

namespace {

INPUT_PORTS_START(midiin)
	PORT_START("CFG")
	PORT_CONFNAME(0xff, 0xff, "MIDI file mode")
	PORT_CONFSETTING(   0xff, "Multi")
	PORT_CONFSETTING(   0x00, "Poly: Channel 1")
	PORT_CONFSETTING(   0x01, "Poly: Channel 2")
	PORT_CONFSETTING(   0x02, "Poly: Channel 3")
	PORT_CONFSETTING(   0x03, "Poly: Channel 4")
	PORT_CONFSETTING(   0x04, "Poly: Channel 5")
	PORT_CONFSETTING(   0x05, "Poly: Channel 6")
	PORT_CONFSETTING(   0x06, "Poly: Channel 7")
	PORT_CONFSETTING(   0x07, "Poly: Channel 8")
	PORT_CONFSETTING(   0x08, "Poly: Channel 9")
	PORT_CONFSETTING(   0x09, "Poly: Channel 10")
	PORT_CONFSETTING(   0x0a, "Poly: Channel 11")
	PORT_CONFSETTING(   0x0b, "Poly: Channel 12")
	PORT_CONFSETTING(   0x0c, "Poly: Channel 13")
	PORT_CONFSETTING(   0x0d, "Poly: Channel 14")
	PORT_CONFSETTING(   0x0e, "Poly: Channel 15")
	PORT_CONFSETTING(   0x0f, "Poly: Channel 16")
INPUT_PORTS_END

} // anonymous namespace

/*-------------------------------------------------
    ctor
-------------------------------------------------*/

midiin_device::midiin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MIDIIN, tag, owner, clock),
		device_image_interface(mconfig, *this),
		device_serial_interface(mconfig, *this),
		m_midi(),
		m_config(*this, "CFG"),
		m_timer(nullptr),
		m_input_cb(*this),
		m_xmit_read(0),
		m_xmit_write(0),
		m_tx_busy(false)
{
}

ioport_constructor midiin_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(midiin);
}

/*-------------------------------------------------
    device_start
-------------------------------------------------*/

void midiin_device::device_start()
{
	m_input_cb.resolve_safe();
	m_timer = timer_alloc(0);
	m_midi.reset();
	m_timer->enable(false);
}

void midiin_device::device_reset()
{
	m_tx_busy = false;
	m_xmit_read = m_xmit_write = 0;

	// we don't Rx, we Tx at 31250 8-N-1
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rcv_rate(0);
	set_tra_rate(31250);
}

/*-------------------------------------------------
    device_timer
-------------------------------------------------*/

void midiin_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == 0)
	{
		// if there's a sequence playing, that takes priority
		midi_event *event = m_sequence.current_event();
		if (event != nullptr)
		{
			attotime curtime = timer.expire();
			if (curtime < m_sequence_start)
			{
				// we could get called before we're supposed to start; show a countdown
				attotime delta = m_sequence_start - curtime;
				popmessage("Waiting to start MIDI playback... %d", delta.seconds());
				m_timer->adjust(std::min(delta, attotime(1, 0)));
			}
			else
			{
				// update the playback time
				curtime -= m_sequence_start;
				popmessage("Playing MIDI file: %d:%02d / %d:%02d", curtime.seconds() / 60, curtime.seconds() % 60, m_sequence.duration().seconds() / 60, m_sequence.duration().seconds() % 60);

				// if it's time to process the current event, do it and advance
				if (curtime >= event->time())
				{
					const u8 force_channel = m_config->read();

					for (u8 curbyte : event->data())
					{
						if (force_channel <= 15 && curbyte >= 0x80 && curbyte < 0xf0)
							curbyte = (curbyte & 0xf0) | force_channel;

						xmit_char(curbyte);
					}
					event = m_sequence.advance_event();
				}

				// if there are more events, set a timer to trigger them
				// (minimum duration 1 sec so that our playback time doesn't skip)
				if (event != nullptr)
					m_timer->adjust(std::min(event->time() - curtime, attotime(1, 0)));
				else
					popmessage("End of MIDI file");
			}
		}
		else if (m_midi)
		{
			uint8_t buf[8192*4];
			int bytesRead;

			while (m_midi->poll())
			{
				bytesRead = m_midi->read(buf);

				if (bytesRead > 0)
				{
					for (int i = 0; i < bytesRead; i++)
					{
						xmit_char(buf[i]);
					}
				}
			}
		}
	}
}

/*-------------------------------------------------
    call_load
-------------------------------------------------*/

image_init_result midiin_device::call_load()
{
	// attempt to load if it's a real file
	m_err = load_image_by_path(OPEN_FLAG_READ, filename());
	if (!m_err)
	{
		// if the parsing succeeds, schedule the start to happen at least
		// 10 seconds after starting to allow the keyboards to initialize
		// TODO: this should perhaps be a driver-configurable parameter?
		if (m_sequence.parse(image_core_file(), length()))
		{
			m_sequence_start = std::max(machine().time(), attotime(10, 0));
			m_timer->adjust(attotime::zero);
			return image_init_result::PASS;
		}
		return image_init_result::FAIL;
	}
	else
	{
		m_midi = machine().osd().create_midi_device();

		if (!m_midi->open_input(filename()))
		{
			m_midi.reset();
			return image_init_result::FAIL;
		}

		m_timer->adjust(attotime::from_hz(1500), 0, attotime::from_hz(1500));
		return image_init_result::PASS;
	}
}

/*-------------------------------------------------
    call_unload
-------------------------------------------------*/

void midiin_device::call_unload()
{
	if (m_midi)
	{
		m_midi->close();
	}
	m_midi.reset();
	m_sequence.clear();
	m_timer->enable(false);
}

void midiin_device::tra_complete()
{
	// is there more waiting to send?
	if (m_xmit_read != m_xmit_write)
	{
//      printf("tx1 %02x\n", m_xmitring[m_xmit_read]);
		transmit_register_setup(m_xmitring[m_xmit_read++]);
		if (m_xmit_read >= XMIT_RING_SIZE)
		{
			m_xmit_read = 0;
		}
	}
	else
	{
		m_tx_busy = false;
	}
}

void midiin_device::tra_callback()
{
	int bit = transmit_register_get_data_bit();
	m_input_cb(bit);
}

void midiin_device::xmit_char(uint8_t data)
{
//  printf("MIDI in: xmit %02x\n", data);

	// if tx is busy it'll pick this up automatically when it completes
	if (!m_tx_busy)
	{
		m_tx_busy = true;
//      printf("tx0 %02x\n", data);
		transmit_register_setup(data);
	}
	else
	{
		// tx is busy, it'll pick this up next time
		m_xmitring[m_xmit_write++] = data;
		if (m_xmit_write >= XMIT_RING_SIZE)
		{
			m_xmit_write = 0;
		}
	}
}


//-------------------------------------------------
//  fourcc_le - helper to compute the little-endian
//  version of a fourcc value from a string
//-------------------------------------------------

static constexpr u32 fourcc_le(char const *string)
{
	return string[0] | (string[1] << 8) | (string[2] << 16) | (string[3] << 24);
}


//-------------------------------------------------
//  midi_parser - constructor
//-------------------------------------------------

midiin_device::midi_parser::midi_parser(util::random_read &stream, u32 length, u32 offset) :
	m_stream(stream),
	m_length(length),
	m_offset(offset)
{
}


//-------------------------------------------------
//  subset - construct a midi_parser that
//  represents a subset of the buffer, and advance
//  our offset past it
//-------------------------------------------------

midiin_device::midi_parser midiin_device::midi_parser::subset(u32 length)
{
	check_bounds(length);
	midi_parser result(m_stream, m_offset + length, m_offset);
	m_offset += length;
	return result;
}


//-------------------------------------------------
//  rewind - back up by the given number of bytes
//-------------------------------------------------

midiin_device::midi_parser &midiin_device::midi_parser::rewind(u32 count)
{
	count = std::min(count, m_offset);
	m_offset -= count;
	return *this;
}


//-------------------------------------------------
//  variable - return the MIDI standard "variable"
//  value
//-------------------------------------------------

u32 midiin_device::midi_parser::variable()
{
	u32 result = 0;
	for (int which = 0; which < 4; which++)
	{
		u8 curbyte = byte();
		result = (result << 7) | (curbyte & 0x7f);
		if ((curbyte & 0x80) == 0)
			return result;
	}
	throw error("Invalid variable length field");
}


//-------------------------------------------------
//  byte - read 8 bits of data
//-------------------------------------------------

u8 midiin_device::midi_parser::byte()
{
	check_bounds(1);

	u8 result = 0;
	std::size_t actual = 0;
	if (m_stream.read_at(m_offset, &result, 1, actual) || actual != 1)
		throw error("Error reading data");
	m_offset++;
	return result;
}


//-------------------------------------------------
//  word_be - read 16 bits of big-endian data
//-------------------------------------------------

u16 midiin_device::midi_parser::word_be()
{
	check_bounds(2);

	u16 result = 0;
	std::size_t actual = 0;
	if (m_stream.read_at(m_offset, &result, 2, actual) || actual != 2)
		throw error("Error reading data");
	m_offset += 2;
	return big_endianize_int16(result);
}


//-------------------------------------------------
//  triple_be - read 24 bits of big-endian data
//-------------------------------------------------

u32 midiin_device::midi_parser::triple_be()
{
	check_bounds(3);

	u32 result = 0;
	std::size_t actual = 0;
	if (m_stream.read_at(m_offset, &result, 3, actual) || actual != 3)
		throw error("Error reading data");
	m_offset += 3;
	return big_endianize_int32(result) >> 8;
}


//-------------------------------------------------
//  dword_be - read 32 bits of big-endian data
//-------------------------------------------------

u32 midiin_device::midi_parser::dword_be()
{
	check_bounds(4);

	u32 result = 0;
	std::size_t actual = 0;
	if (m_stream.read_at(m_offset, &result, 4, actual) || actual != 4)
		throw error("Error reading data");
	m_offset += 4;
	return big_endianize_int32(result);
}


//-------------------------------------------------
//  dword_le - read 32 bits of little-endian data
//-------------------------------------------------

u32 midiin_device::midi_parser::dword_le()
{
	check_bounds(4);

	u32 result = 0;
	std::size_t actual = 0;
	if (m_stream.read_at(m_offset, &result, 4, actual) || actual != 4)
		throw error("Error reading data");
	m_offset += 4;
	return little_endianize_int32(result);
}


//-------------------------------------------------
//  check_bounds - check to see if we have at least
//  'length' bytes left to consume; if not,
//  throw an error
//-------------------------------------------------

void midiin_device::midi_parser::check_bounds(u32 length)
{
	if (m_offset + length > m_length)
		throw error("Out of bounds error");
}


//-------------------------------------------------
//  event_at - return a reference to the sequence
//  event at the given tick, or create a new one
//  if one doesn't yet exist
//-------------------------------------------------

midiin_device::midi_event &midiin_device::midi_sequence::event_at(u32 tick)
{
	for (auto it = m_list.begin(); it != m_list.end(); ++it)
	{
		if (it->tick() == tick)
			return *it;
		if (it->tick() > tick)
			return *m_list.emplace(it, tick);
	}
	m_list.emplace_back(tick);
	return m_list.back();
}


//-------------------------------------------------
//  parse - parse a MIDI sequence from a buffer
//-------------------------------------------------

bool midiin_device::midi_sequence::parse(util::random_read &stream, u32 length)
{
	// start with an empty list of events
	m_list.clear();

	// by default parse the whole data
	midi_parser buffer(stream, length, 0);

	// catch errors to make parsing easier
	try
	{
		// if not a RIFF-encoed MIDI, just parse as-is
		if (buffer.dword_le() != fourcc_le("RIFF"))
			parse_midi_data(buffer.reset());
		else
		{
			// check the RIFF type and size
			u32 riffsize = buffer.dword_le();
			u32 rifftype = buffer.dword_le();
			if (rifftype != fourcc_le("RMID"))
				throw midi_parser::error("Input RIFF file is not of type RMID");

			// loop over RIFF chunks
			midi_parser riffdata = buffer.subset(riffsize - 4);
			while (!riffdata.eob())
			{
				u32 chunktype = riffdata.dword_le();
				u32 chunksize = riffdata.dword_le();
				midi_parser chunk = riffdata.subset(chunksize);
				if (chunktype == fourcc_le("data"))
				{
					parse_midi_data(chunk);
					break;
				}
			}
		}
		m_iterator = m_list.begin();
		return true;
	}
	catch (midi_parser::error &err)
	{
		osd_printf_error("MIDI file error: %s\n", err.description());
		m_list.clear();
		m_iterator = m_list.begin();
		return false;
	}
}


//-------------------------------------------------
//  parse_midi_data - parse the core MIDI format
//  into tracks
//-------------------------------------------------

void midiin_device::midi_sequence::parse_midi_data(midi_parser &buffer)
{
	// scan for syntactic correctness, and to find global state
	u32 headertype = buffer.dword_le();
	if (headertype != fourcc_le("MThd"))
		throw midi_parser::error("Input file is not a MIDI file.");
	if (buffer.dword_be() != 0x00000006)
		throw midi_parser::error("Invalid MIDI file header.");

	// parse format info
	int format = buffer.word_be();
	if (format > 2)
		throw midi_parser::error("Invalid MIDI file header.");

	// parse track count
	u16 tracks = buffer.word_be();
	if (format == 0 && tracks != 1)
		throw midi_parser::error("MIDI format 0 expects exactly one track.");

	// parse time divisor
	u16 timediv = buffer.word_be();
	if ((timediv & 0x8000) != 0)
		throw midi_parser::error("SMPTE timecode time division not supported.");
	if (timediv == 0)
		throw midi_parser::error("Invalid time divisor of 0.");

	// iterate over tracks
	u32 curtick = 0;
	for (u16 index = 0; index < tracks; index++)
	{
		// verify header
		if (buffer.dword_le() != fourcc_le("MTrk"))
			throw midi_parser::error("Invalid MIDI track header.");
		u32 chunksize = buffer.dword_be();

		// parse the track data
		midi_parser trackdata = buffer.subset(chunksize);
		u32 numticks = parse_track_data(trackdata, curtick);
		if (format == 2)
			curtick += numticks;
	}

	// now go through the event list and compute times
	u32 lasttick = 0;
	attotime ticktime = attotime::from_usec(1000000) / timediv;
	attotime curtime;
	for (auto &event : m_list)
	{
		// update the elapsed time
		u32 curtick = event.tick();
		curtime += ticktime * (curtick - lasttick);
		lasttick = curtick;

		// determine if we have a new tempo here before replacing the time
		if (!event.time().is_zero())
			ticktime = event.time() / timediv;
		event.set_time(curtime);
	}
}


//-------------------------------------------------
//  parse_track_data - parse data from a track and
//  add it to the buffer
//-------------------------------------------------

u32 midiin_device::midi_sequence::parse_track_data(midi_parser &buffer, u32 start_tick)
{
	u32 curtick = start_tick;
	u8 last_type = 0;
	while (!buffer.eob())
	{
		// parse the time delta
		curtick += buffer.variable();
		midi_event &event = event_at(curtick);

		// handle running status
		u8 type = buffer.byte();
		if (BIT(type, 7) != 0)
			last_type = type;
		else
		{
			type = last_type;
			buffer.rewind(1);
		}

		// determine the event class
		uint8_t eclass = type >> 4;
		if (eclass != 15)
		{
			// simple events: all but program change and aftertouch have a second parameter
			event.append(type);
			event.append(buffer.byte());
			if (eclass != 12 && eclass != 13)
				event.append(buffer.byte());
		}
		else if (type != 0xff)
		{
			// handle non-meta events
			midi_parser eventdata = buffer.subset(buffer.variable());
			event.append(type);
			while (!eventdata.eob())
				event.append(eventdata.byte());
		}
		else
		{
			// handle meta-events
			u8 type = buffer.byte();
			midi_parser eventdata = buffer.subset(buffer.variable());

			// end of data?
			if (type == 0x2f)
				break;

			// only care about tempo events; set the "time" to the new tick
			// value; we will sweep this later and compute actual times
			if (type == 0x51)
			{
				u32 usec_per_quarter = eventdata.triple_be();
				if (usec_per_quarter != 0)
					event.set_time(attotime::from_usec(usec_per_quarter));
			}
		}
	}
	return curtick;
}
