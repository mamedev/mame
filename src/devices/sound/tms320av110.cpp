// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

/*
 * Texas Instruments TMS320AV110 MPEG-1 audio decoder (preliminary)
 *
 * The implemented subset consists of the host register window, memory-mapped
 * compressed-data input, reset-controlled REQ signalling, and MPEG-1 Audio
 * Layer II reconstruction.  Input-buffer-full backpressure and other
 * control/status register behaviour are not yet fully implemented.
 *
 * Reference: Texas Instruments data sheet SCSS013C, revised August 1995.
 */

#include "emu.h"
#include "tms320av110.h"

#include "mpeg_audio.h"

namespace {

// The host interface has seven address pins, SADDR6 through SADDR0.
constexpr unsigned HOST_ADDRESS_BITS = 7;
constexpr offs_t HOST_ADDRESS_MASK = (1U << HOST_ADDRESS_BITS) - 1;

// DATAIN is the data sheet's memory-mapped compressed-audio input register.
constexpr offs_t REG_DATAIN = 0x18;

// MPEG audio is presented as left and right PCM output slots, including mono streams.
constexpr unsigned LEFT_CHANNEL = 0;
constexpr unsigned RIGHT_CHANNEL = 1;
constexpr unsigned OUTPUT_CHANNELS = 2;

// Used for sound scheduling until an MPEG header supplies the stream rate.
constexpr u32 INITIAL_SAMPLE_RATE = 44'100;

// MPEG-1 Layer II produces 1,152 samples per channel for each decoded frame.
constexpr unsigned MPEG_LAYER_II_SAMPLES_PER_FRAME = 1'152;

// A Layer II frame is largest at the maximum bit rate and minimum sample rate.
constexpr unsigned MAX_MPEG1_LAYER_II_BIT_RATE = 384'000;
constexpr unsigned MIN_MPEG1_SAMPLE_RATE = 32'000;
constexpr unsigned MPEG1_LAYER_II_FRAME_SCALE = 144;
constexpr unsigned MPEG_FRAME_PADDING_BYTES = 1;
constexpr unsigned MAX_MPEG_AUDIO_FRAME_BYTES =
	(MPEG1_LAYER_II_FRAME_SCALE * MAX_MPEG1_LAYER_II_BIT_RATE / MIN_MPEG1_SAMPLE_RATE) + MPEG_FRAME_PADDING_BYTES;

// Retain two maximum-sized frames so the frame-based decoder can accept streaming input.
constexpr unsigned INPUT_BUFFER_BYTES = 2 * MAX_MPEG_AUDIO_FRAME_BYTES;

} // anonymous namespace

DEFINE_DEVICE_TYPE(TMS320AV110, tms320av110_device, "tms320av110", "Texas Instruments TMS320AV110 MPEG Audio Decoder")

struct tms320av110_device::decoder_state
{
	std::array<u8, INPUT_BUFFER_BYTES> input{};
	std::array<s16, MPEG_LAYER_II_SAMPLES_PER_FRAME * OUTPUT_CHANNELS> pcm{};
	std::unique_ptr<mpeg_audio> decoder;
	unsigned input_bytes = 0;
	unsigned input_bit_position = 0;
	unsigned pcm_position = 0;
	unsigned pcm_count = 0;
	unsigned pcm_channels = 0;
};

tms320av110_device::tms320av110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, TMS320AV110, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_req_cb(*this),
	m_decoder(std::make_unique<decoder_state>()),
	m_reset_asserted(true)
{
}

tms320av110_device::~tms320av110_device() = default;

void tms320av110_device::device_start()
{
	m_stream = stream_alloc(0, OUTPUT_CHANNELS, INITIAL_SAMPLE_RATE);
	decoder_create();

	save_item(NAME(m_decoder->input));
	save_item(NAME(m_decoder->pcm));
	save_item(NAME(m_decoder->input_bytes));
	save_item(NAME(m_decoder->input_bit_position));
	save_item(NAME(m_decoder->pcm_position));
	save_item(NAME(m_decoder->pcm_count));
	save_item(NAME(m_decoder->pcm_channels));
	save_item(NAME(m_reset_asserted));
}

void tms320av110_device::device_reset()
{
	decoder_reset();
	m_reset_asserted = true;
	update_req();
}

void tms320av110_device::decoder_create()
{
	m_decoder->decoder = std::make_unique<mpeg_audio>(m_decoder->input.data(), mpeg_audio::L2, false, 0);
}

void tms320av110_device::decoder_reset()
{
	if (m_stream)
	{
		m_stream->update();
		m_stream->set_sample_rate(INITIAL_SAMPLE_RATE);
	}

	m_decoder->input.fill(0);
	m_decoder->pcm.fill(0);
	m_decoder->input_bytes = 0;
	m_decoder->input_bit_position = 0;
	m_decoder->pcm_position = 0;
	m_decoder->pcm_count = 0;
	m_decoder->pcm_channels = 0;
	decoder_create();
}

bool tms320av110_device::decode_frame()
{
	int position = m_decoder->input_bit_position;
	int sample_count = 0;
	int sample_rate = 0;
	int channels = 0;
	if (!m_decoder->decoder->decode_buffer(
			position,
			m_decoder->input_bytes * 8,
			m_decoder->pcm.data(),
			sample_count,
			sample_rate,
			channels))
	{
		return false;
	}

	assert(sample_count <= MPEG_LAYER_II_SAMPLES_PER_FRAME);
	assert((channels == 1) || (channels == OUTPUT_CHANNELS));
	m_decoder->pcm_position = 0;
	m_decoder->pcm_count = sample_count;
	m_decoder->pcm_channels = channels;

	const unsigned bytes_consumed = position / 8;
	if (bytes_consumed)
	{
		std::move(
			m_decoder->input.begin() + bytes_consumed,
			m_decoder->input.begin() + m_decoder->input_bytes,
			m_decoder->input.begin());
		m_decoder->input_bytes -= bytes_consumed;
	}
	m_decoder->input_bit_position = position & 7;
	update_req();

	if (sample_rate && (sample_rate != m_stream->sample_rate()))
		m_stream->set_sample_rate(sample_rate);

	return true;
}

void tms320av110_device::fifo_w(u8 data)
{
	if (m_decoder->input_bytes == m_decoder->input.size())
	{
		logerror("%s: compressed-audio input overflow\n", machine().describe_context());
		return;
	}

	m_decoder->input[m_decoder->input_bytes++] = data;
	update_req();
}

void tms320av110_device::update_req()
{
	// REQ is active low and is held inactive during reset or while the input buffer is full.
	m_req_cb((m_reset_asserted || (m_decoder->input_bytes == m_decoder->input.size())) ? ASSERT_LINE : CLEAR_LINE);
}

void tms320av110_device::reset_w(int state)
{
	const bool asserted = !state;
	if (asserted && !m_reset_asserted)
		decoder_reset();

	m_reset_asserted = asserted;
	update_req();
}

u8 tms320av110_device::read(offs_t offset)
{
	if (m_reset_asserted)
		return 0; // The data sheet specifies that RESET low disables host accesses.

	offset &= HOST_ADDRESS_MASK;
	if (!machine().side_effects_disabled())
		logerror("%s: unimplemented register read %02x\n", machine().describe_context(), offset);
	return 0;
}

void tms320av110_device::write(offs_t offset, u8 data)
{
	if (m_reset_asserted)
		return; // The data sheet specifies that RESET low disables host accesses.

	offset &= HOST_ADDRESS_MASK;
	if (offset == REG_DATAIN)
	{
		fifo_w(data);
		return;
	}

	logerror("%s: unimplemented register write %02x = %02x\n", machine().describe_context(), offset, data);
}

void tms320av110_device::sound_stream_update(sound_stream &stream)
{
	for (int sample = 0; sample < stream.samples(); sample++)
	{
		if ((m_decoder->pcm_position == m_decoder->pcm_count) && !decode_frame())
		{
			stream.put(LEFT_CHANNEL, sample, 0.0F);
			stream.put(RIGHT_CHANNEL, sample, 0.0F);
			continue;
		}

		const unsigned position = m_decoder->pcm_position * m_decoder->pcm_channels;
		const s16 left = m_decoder->pcm[position];
		const s16 right = (m_decoder->pcm_channels == OUTPUT_CHANNELS)
			? m_decoder->pcm[position + RIGHT_CHANNEL]
			: left;
		stream.put_int(LEFT_CHANNEL, sample, left, 32'768);
		stream.put_int(RIGHT_CHANNEL, sample, right, 32'768);
		m_decoder->pcm_position++;
	}
}
