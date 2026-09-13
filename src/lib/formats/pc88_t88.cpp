// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*********************************************************************

    pc88_t88.cpp

    Format handler for PC-8801 T88 tape images.

    Reference: http://quagma.sakura.ne.jp/manuke/t88format.html

    A T88 file is a 24-byte header ("PC-8801 Tape Image(T88)" + NUL)
    followed by a sequence of tags:

        ID (2 bytes, LE), data length (2 bytes, LE), data[length]

    All times are expressed in "ticks" of 1/4800 s.

        0x0000  end          - no payload
        0x0001  version      - 2 bytes
        0x0100  blank        - start tick (4), length (4)   : no carrier
        0x0101  data         - start tick (4), length (4),
                               byte count (2), data type (2), bytes[]
        0x0102  space        - start tick (4), length (4)   : 1200 Hz carrier
        0x0103  mark         - start tick (4), length (4)   : 2400 Hz carrier

    The data tag stores *bytes*, not a waveform: they have to be
    FSK-modulated here.  A '0' bit is one full period of 1200 Hz, a '1'
    bit is two full periods of 2400 Hz; at 600 baud both are doubled.

*********************************************************************/

#include "pc88_t88.h"

#include "coretmpl.h" 	// util::BIT
#include "multibyte.h"	// get_u16le / get_u32le
#include "osdcore.h"  	// osd_printf_*


namespace {

#define VERBOSE_T88 0

#if VERBOSE_T88
#define LOG_T88(...) osd_printf_info(__VA_ARGS__)
#else
#define LOG_T88(...) do { } while (false)
#endif


// 1 tick = 1/4800 s.  Sampling at 48000 Hz makes every quantity integral:
// 10 samples per tick, 20 samples per 2400 Hz cycle, 40 per 1200 Hz cycle.
constexpr uint32_t T88_TICK_FREQUENCY    = 4800;
constexpr uint32_t T88_SAMPLE_FREQUENCY  = 48000;
constexpr uint32_t T88_SAMPLES_PER_TICK  = T88_SAMPLE_FREQUENCY / T88_TICK_FREQUENCY;
constexpr int16_t  T88_AMPLITUDE         = 0x6000;

constexpr size_t   T88_HEADER_SIZE       = 24;

// flush the working buffer once it grows past this (~5 s of audio)
constexpr size_t   T88_FLUSH_THRESHOLD   = 256 * 1024;

// A 1200 Hz cycle spans 4 ticks and a 2400 Hz one spans 2, so chunks have to
// be a multiple of 4 ticks: otherwise a long carrier gets cut mid cycle and
// the next chunk restarts the waveform in phase 0, producing one half cycle
// of the wrong length that reads as a carrier dropout.
constexpr uint64_t T88_MAX_CHUNK_TICKS   = (T88_FLUSH_THRESHOLD / T88_SAMPLES_PER_TICK) & ~uint64_t(3);

// refuse obviously broken images rather than trying to allocate for them
constexpr uint64_t T88_MAX_TICKS         = uint64_t(T88_TICK_FREQUENCY) * 60 * 90; // 90 minutes

constexpr uint16_t TAG_END     = 0x0000;
constexpr uint16_t TAG_VERSION = 0x0001;
constexpr uint16_t TAG_BLANK   = 0x0100;
constexpr uint16_t TAG_DATA    = 0x0101;
constexpr uint16_t TAG_SPACE   = 0x0102;
constexpr uint16_t TAG_MARK    = 0x0103;


//-------------------------------------------------
//  waveform accumulator
//
//  Collects samples for a contiguous run starting at
//  m_start_tick and hands them to the cassette image in
//  bounded chunks, so that a long carrier or a large data
//  tag never forces a huge allocation.
//-------------------------------------------------

class wave_builder
{
public:
	wave_builder(cassette_image *cassette) : m_cassette(cassette) { }

	// begin a new run at an absolute position on the tape
	cassette_image::error seek(uint64_t start_tick)
	{
		cassette_image::error err = flush();
		if (err != cassette_image::error::SUCCESS)
			return err;

		m_start_tick = start_tick;
		m_run_ticks  = 0;
		return cassette_image::error::SUCCESS;
	}

	// append a square wave of the given frequency for the given number of ticks
	cassette_image::error tone(uint32_t frequency, uint64_t ticks)
	{
		const size_t period = T88_SAMPLE_FREQUENCY / frequency;
		const size_t half   = period / 2;

		m_run_ticks  += ticks;
		m_total_ticks += ticks;

		while (ticks)
		{
			// cap each pass so the buffer stays small
			const uint64_t chunk_ticks = std::min<uint64_t>(ticks, T88_MAX_CHUNK_TICKS);
			const size_t   count       = size_t(chunk_ticks) * T88_SAMPLES_PER_TICK;
			const size_t   base        = m_buffer.size();

			m_buffer.resize(base + count);
			for (size_t i = 0; i < count; i++)
				m_buffer[base + i] = ((i % period) < half) ? T88_AMPLITUDE : -T88_AMPLITUDE;

			ticks -= chunk_ticks;

			if (m_buffer.size() >= T88_FLUSH_THRESHOLD)
			{
				cassette_image::error err = flush();
				if (err != cassette_image::error::SUCCESS)
					return err;
			}
		}
		return cassette_image::error::SUCCESS;
	}

	cassette_image::error flush()
	{
		if (m_buffer.empty())
			return cassette_image::error::SUCCESS;

		const double time_index    = double(m_start_tick) / T88_TICK_FREQUENCY;
		const double sample_period = double(m_buffer.size()) / T88_SAMPLE_FREQUENCY;
		const uint64_t chunk_ticks = m_buffer.size() / T88_SAMPLES_PER_TICK;

		const cassette_image::error err = m_cassette->put_samples(
				0, time_index, sample_period,
				m_buffer.size(), sizeof(int16_t),
				m_buffer.data(), cassette_image::WAVEFORM_16BIT);

		// every unit we emit is a whole number of ticks, so this stays exact
		m_start_tick += chunk_ticks;
		m_buffer.clear();

		return err;
	}

	bool empty() const { return m_written == 0; }
	void mark_written() { m_written++; }

private:
	cassette_image        *m_cassette;
	std::vector<int16_t>   m_buffer;
	uint64_t               m_start_tick = 0;
	uint64_t               m_run_ticks = 0;
	uint64_t               m_total_ticks = 0;
	unsigned               m_written = 0;
};


//-------------------------------------------------
//  emit one framed byte
//-------------------------------------------------

struct frame_params
{
	uint32_t bit_ticks;     // duration of one bit
	uint32_t stop_ticks;    // total duration of the stop bits
	int      data_bits;
	bool     parity_enable;
	bool     parity_even;
};

frame_params decode_data_type(uint16_t data_type)
{
	frame_params fp;

	// bit 8: 0 = 600 baud, 1 = 1200 baud
	fp.bit_ticks     = (data_type & 0x0100) ? 4 : 8;

	// bits 3-2: character length (5 + n)
	fp.data_bits     = 5 + ((data_type >> 2) & 0x03);

	// bit 4: parity enable, bit 5: 0 = odd, 1 = even
	fp.parity_enable = util::BIT(data_type, 4);
	fp.parity_even   = util::BIT(data_type, 5);

	// bits 7-6: 1 = 1 stop bit, 2 = 1.5, 3 = 2 (0 is invalid)
	switch ((data_type >> 6) & 0x03)
	{
	case 1:  fp.stop_ticks = fp.bit_ticks;          break;
	case 2:  fp.stop_ticks = fp.bit_ticks * 3 / 2;  break;
	default: fp.stop_ticks = fp.bit_ticks * 2;      break;
	}

	return fp;
}

cassette_image::error emit_byte(wave_builder &wave, uint8_t value, const frame_params &fp)
{
	// a '0' is a 1200 Hz cycle, a '1' is a 2400 Hz cycle - always, at both baud rates
	auto emit_bit = [&wave] (bool bit, uint32_t ticks)
	{
		return wave.tone(bit ? 2400 : 1200, ticks);
	};

	// start bit
	cassette_image::error err = emit_bit(false, fp.bit_ticks);
	if (err != cassette_image::error::SUCCESS)
		return err;

	// data bits, LSB first
	int ones = 0;
	for (int i = 0; i < fp.data_bits; i++)
	{
		const bool bit = util::BIT(value, i);
		if (bit)
			ones++;
		err = emit_bit(bit, fp.bit_ticks);
		if (err != cassette_image::error::SUCCESS)
			return err;
	}

	// optional parity bit
	if (fp.parity_enable)
	{
		const bool bit = fp.parity_even ? util::BIT(ones, 0) : !util::BIT(ones, 0);
		err = emit_bit(bit, fp.bit_ticks);
		if (err != cassette_image::error::SUCCESS)
			return err;
	}

	// stop bits
	return emit_bit(true, fp.stop_ticks);
}

} // anonymous namespace


//-------------------------------------------------
//  t88_cassette_identify
//-------------------------------------------------

static cassette_image::error t88_cassette_identify(cassette_image *cassette, cassette_image::Options *opts)
{
	if (cassette->image_size() < T88_HEADER_SIZE)
		return cassette_image::error::INVALID_IMAGE;

	uint8_t header[T88_HEADER_SIZE];
	cassette->image_read(header, 0, T88_HEADER_SIZE);

	// 23 characters plus the terminating NUL
	if (std::memcmp(header, "PC-8801 Tape Image(T88)", T88_HEADER_SIZE) != 0)
		return cassette_image::error::INVALID_IMAGE;

	// this is mandatory: open_choices() copies these straight into the image,
	// and a zero sample frequency / channel count silently discards everything
	opts->channels         = 1;
	opts->bits_per_sample  = 16;
	opts->sample_frequency = T88_SAMPLE_FREQUENCY;

	return cassette_image::error::SUCCESS;
}


//-------------------------------------------------
//  t88_cassette_load
//-------------------------------------------------

static cassette_image::error t88_cassette_load(cassette_image *cassette)
{
	const uint64_t image_size = cassette->image_size();
	if (image_size < T88_HEADER_SIZE)
		return cassette_image::error::INVALID_IMAGE;

	const size_t buffer_size = size_t(image_size);
	std::vector<uint8_t> buffer(buffer_size);
	cassette->image_read(buffer.data(), 0, buffer_size);

	wave_builder wave(cassette);
	uint64_t offset = T88_HEADER_SIZE;

	while (offset + 4 <= image_size)
	{
		const uint16_t tag_id  = get_u16le(&buffer[offset]);
		const uint16_t tag_len = get_u16le(&buffer[offset + 2]);
		offset += 4;

		if (tag_id == TAG_END)
			break;

		// truncated tag: stop rather than walk off the end
		if (offset + tag_len > image_size)
			break;

		const uint8_t *const payload = &buffer[offset];
		const uint64_t next_offset = offset + tag_len;

		switch (tag_id)
		{
		case TAG_SPACE:
		case TAG_MARK:
			// blank tags carry no signal at all, so they are simply skipped:
			// untouched regions of the cassette image already read as silence
			if (tag_len >= 8)
			{
				const uint32_t start_tick = get_u32le(payload);
				const uint32_t length     = get_u32le(payload + 4);
				LOG_T88("T88 LOAD BLANK (%x) [start %u - length %u]\n", tag_id, 
					start_tick, length);

				if (length && uint64_t(start_tick) + length <= T88_MAX_TICKS)
				{
					cassette_image::error err = wave.seek(start_tick);
					if (err != cassette_image::error::SUCCESS)
						return err;

					err = wave.tone((tag_id == TAG_MARK) ? 2400 : 1200, length);
					if (err != cassette_image::error::SUCCESS)
						return err;

					wave.mark_written();
				}
			}
			break;

		case TAG_DATA:
			if (tag_len >= 12)
			{
				const uint32_t start_tick = get_u32le(payload);
				const uint32_t area_len   = get_u32le(payload + 4);
				const uint16_t byte_count = get_u16le(payload + 8);
				const uint16_t data_type  = get_u16le(payload + 10);
				LOG_T88("T88 LOAD DATA (%x) [start %u - length %u - baud %u]\n", 
					tag_id, start_tick, area_len, (data_type & 0x0100) ? 1200 : 600);

				// the byte count is authoritative, but never trust it past the tag
				const size_t avail = std::min<size_t>(byte_count, tag_len - 12);

				if (avail && uint64_t(start_tick) + area_len <= T88_MAX_TICKS)
				{
					const frame_params fp = decode_data_type(data_type);

					cassette_image::error err = wave.seek(start_tick);
					if (err != cassette_image::error::SUCCESS)
						return err;

					for (size_t i = 0; i < avail; i++)
					{
						err = emit_byte(wave, payload[12 + i], fp);
						if (err != cassette_image::error::SUCCESS)
							return err;
					}

					wave.mark_written();
				}
			}
			break;

		case TAG_VERSION:
		case TAG_BLANK:
		default:
			// unknown tags get skipped as well
			LOG_T88("T88 LOAD OTHER (%x)\n", tag_id);
			break;
		}

		offset = next_offset;
	}

	const cassette_image::error err = wave.flush();
	if (err != cassette_image::error::SUCCESS)
		return err;

	if (wave.empty())
		return cassette_image::error::INVALID_IMAGE;

	return cassette_image::error::SUCCESS;
}


//-------------------------------------------------
//  format registration
//-------------------------------------------------

static const cassette_image::Format t88_cassette_format =
{
	"t88",
	t88_cassette_identify,
	t88_cassette_load,
	nullptr
};

CASSETTE_FORMATLIST_START(t88_cassette_formats)
	CASSETTE_FORMAT(t88_cassette_format)
CASSETTE_FORMATLIST_END
