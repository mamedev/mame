// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

/*
 * Texas Instruments TMS320AV110 MPEG-1 audio decoder (preliminary)
 *
 * The implemented subset consists of the host register window, compressed-
 * data buffering and the REQ handshake, reset/restart sequencing, PLAY and
 * MUTE control, and MPEG-1 Audio Layer II reconstruction.
 *
 * Reference: Texas Instruments data sheet SCSS013C, revised August 1995.
 */

#include "emu.h"
#include "tms320av110.h"

#include "mpeg_audio.h"

#include <cmath>

#define LOG_REQUESTS (1U << 0)
#define LOG_REGISTERS (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

namespace {

// The host interface has seven address pins, SADDR6 through SADDR0.
constexpr unsigned HOST_ADDRESS_BITS = 7;
constexpr unsigned HOST_REGISTER_COUNT = 1U << HOST_ADDRESS_BITS;
constexpr offs_t HOST_ADDRESS_MASK = HOST_REGISTER_COUNT - 1;

// Host-interface register addresses from the data sheet.
constexpr offs_t REG_BUFF_L = 0x12;   // input buffer word count, bits 7:0
constexpr offs_t REG_BUFF_H = 0x13;   // input buffer word count, bits 14:8
constexpr offs_t REG_FREE_FORM_L = 0x14; // free-format frame length, bits 7:0
constexpr offs_t REG_FREE_FORM_H = 0x15; // free-format frame length, bits 10:8
constexpr offs_t REG_PCM_18 = 0x16;   // PCM output precision
constexpr offs_t REG_DATAIN = 0x18;   // memory-mapped compressed-data input
constexpr offs_t REG_INTR_L = 0x1a;   // interrupt status, bits 7:0
constexpr offs_t REG_INTR_H = 0x1b;   // interrupt status, bits 15:8
constexpr offs_t REG_INTR_EN_L = 0x1c; // interrupt enable, bits 7:0
constexpr offs_t REG_INTR_EN_H = 0x1d; // interrupt enable, bits 15:8
constexpr offs_t REG_ATTEN_L = 0x1e;  // left output attenuation
constexpr offs_t REG_ATTEN_R = 0x20;  // right output attenuation
constexpr offs_t REG_AUD_ID = 0x22;   // MPEG system/packet audio stream ID
constexpr offs_t REG_AUD_ID_EN = 0x24; // audio stream ID filtering enable
constexpr offs_t REG_SYNC_ST = 0x26;  // synchronization status
constexpr offs_t REG_SYNC_LCK = 0x28; // required additional synchronization words
constexpr offs_t REG_CRC_ECM = 0x2a;  // CRC error concealment mode
constexpr offs_t REG_SYNC_ECM = 0x2c; // synchronization error concealment mode
constexpr offs_t REG_PLAY = 0x2e;     // decoded audio output enable
constexpr offs_t REG_MUTE = 0x30;     // decoded audio mute
constexpr offs_t REG_SKIP = 0x32;     // skip next audio frame
constexpr offs_t REG_REPEAT = 0x34;   // repeat next audio frame
constexpr offs_t REG_STR_SEL = 0x36;  // compressed input stream format
constexpr offs_t REG_PCM_ORD = 0x38;  // PCM output bit order
constexpr offs_t REG_LATENCY = 0x3c;  // synchronization lookahead enable
constexpr offs_t REG_DRAM_EXT = 0x3e; // external input-buffer DRAM present
constexpr offs_t REG_RESET = 0x40;    // decoder reset command/status
constexpr offs_t REG_RESTART = 0x42;  // data-buffer flush command/status
constexpr offs_t REG_PCM_FS = 0x44;   // decoded sampling frequency
constexpr offs_t REG_PCM_DIV = 0x6e;  // PCM clock divider
constexpr offs_t REG_DIF = 0x6f;      // 18-bit PCM justification
constexpr offs_t REG_SIN_EN = 0x70;   // serial compressed-data input enable

constexpr u8 SINGLE_BIT_MASK = 0x01;
constexpr u8 TWO_BIT_MASK = 0x03;
constexpr u8 THREE_BIT_MASK = 0x07;
constexpr u8 FIVE_BIT_MASK = 0x1f;
constexpr u8 SIX_BIT_MASK = 0x3f;

// Without external DRAM, the on-chip compressed-data input SRAM is 256 bytes.
constexpr unsigned INPUT_BUFFER_BYTES_WITHOUT_DRAM = 256;
// The supported external DRAM is 256K locations by four bits.
constexpr unsigned INPUT_BUFFER_BYTES_WITH_DRAM = 256 * 1024 * 4 / 8;
// The interface permits one further byte after REQ reports full, held
// separately from the selected input buffer.
constexpr unsigned INPUT_FIFO_BYTES = INPUT_BUFFER_BYTES_WITH_DRAM + 1;

// A no-DRAM reset takes approximately 700 microseconds at the nominal 24 MHz
// OSCIN frequency, corresponding to 16,800 oscillator clocks.
constexpr unsigned RESET_CYCLES_WITHOUT_DRAM = 16'800;
// With external DRAM, reset takes approximately 3.7 ms at 24 MHz.
constexpr unsigned RESET_CYCLES_WITH_DRAM = 88'800;

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

// This is private storage for adapting the streaming device to the frame-based
// MPEG helper.  It is not the AV110's hardware-visible compressed-data SRAM.
constexpr unsigned DECODER_INPUT_BYTES = 2 * MAX_MPEG_AUDIO_FRAME_BYTES;

} // anonymous namespace

DEFINE_DEVICE_TYPE(TMS320AV110, tms320av110_device, "tms320av110", "Texas Instruments TMS320AV110 MPEG Audio Decoder")

struct tms320av110_device::decoder_state
{
	std::array<u8, HOST_REGISTER_COUNT> registers{};
	std::array<u8, INPUT_FIFO_BYTES> input_fifo{};
	std::array<u8, DECODER_INPUT_BYTES> input{};
	std::array<s16, MPEG_LAYER_II_SAMPLES_PER_FRAME * OUTPUT_CHANNELS> pcm{};
	std::unique_ptr<mpeg_audio> decoder;
	unsigned input_fifo_read = 0;
	unsigned input_fifo_write = 0;
	unsigned input_fifo_count = 0;
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
	m_external_dram(false),
	m_input_timer(nullptr),
	m_reset_timer(nullptr),
	m_reset_asserted(false),
	m_reset_cycle(false),
	m_data_access(false),
	m_req_state(CLEAR_LINE)
{
}

tms320av110_device::~tms320av110_device() = default;

void tms320av110_device::device_start()
{
	m_stream = stream_alloc(0, OUTPUT_CHANNELS, INITIAL_SAMPLE_RATE);
	m_input_timer = timer_alloc(FUNC(tms320av110_device::input_tick), this);
	m_reset_timer = timer_alloc(FUNC(tms320av110_device::reset_complete), this);
	decoder_create();
	m_decoder->decoder->register_save_state(*this);

	save_item(NAME(m_decoder->registers));
	save_item(NAME(m_decoder->input_fifo));
	save_item(NAME(m_decoder->input));
	save_item(NAME(m_decoder->pcm));
	save_item(NAME(m_decoder->input_fifo_read));
	save_item(NAME(m_decoder->input_fifo_write));
	save_item(NAME(m_decoder->input_fifo_count));
	save_item(NAME(m_decoder->input_bytes));
	save_item(NAME(m_decoder->input_bit_position));
	save_item(NAME(m_decoder->pcm_position));
	save_item(NAME(m_decoder->pcm_count));
	save_item(NAME(m_decoder->pcm_channels));
	save_item(NAME(m_reset_asserted));
	save_item(NAME(m_reset_cycle));
	save_item(NAME(m_data_access));
	save_item(NAME(m_req_state));
}

void tms320av110_device::device_reset()
{
	m_input_timer->adjust(attotime::never);
	m_reset_timer->adjust(attotime::never);
	m_decoder->registers.fill(0);
	m_reset_asserted = false;
	m_reset_cycle = false;
	start_reset(true);
}

void tms320av110_device::device_post_load()
{
	m_req_cb(m_req_state);
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
	m_decoder->registers[REG_SYNC_ST] = 0;
	m_decoder->registers[REG_PCM_FS] = 0;
	m_decoder->decoder->clear();
}

void tms320av110_device::input_fifo_reset()
{
	m_input_timer->adjust(attotime::never);
	m_decoder->input_fifo.fill(0);
	m_decoder->input_fifo_read = 0;
	m_decoder->input_fifo_write = 0;
	m_decoder->input_fifo_count = 0;
	m_data_access = false;
}

void tms320av110_device::start_reset(bool pin_reset)
{
	decoder_reset();
	input_fifo_reset();
	m_decoder->registers[REG_INTR_L] = 0;
	m_decoder->registers[REG_INTR_H] = 0;
	m_decoder->registers[REG_INTR_EN_L] = 0;
	m_decoder->registers[REG_INTR_EN_H] = 0;
	m_decoder->registers[REG_RESET] = 1;
	m_decoder->registers[REG_RESTART] = 0;
	if (pin_reset)
	{
		m_decoder->registers[REG_PLAY] = 0;
		m_decoder->registers[REG_MUTE] = 0;
		m_decoder->registers[REG_PCM_DIV] = 0;
	}

	m_reset_cycle = true;
	m_reset_timer->adjust(clocks_to_attotime(
		m_external_dram ? RESET_CYCLES_WITH_DRAM : RESET_CYCLES_WITHOUT_DRAM));
	update_req();
}

void tms320av110_device::start_restart()
{
	decoder_reset();
	input_fifo_reset();
	m_decoder->registers[REG_INTR_L] = 0;
	m_decoder->registers[REG_INTR_H] = 0;
	m_decoder->registers[REG_INTR_EN_L] = 0;
	m_decoder->registers[REG_INTR_EN_H] = 0;
	m_decoder->registers[REG_RESTART] = 1;
	m_reset_cycle = true;

	// Restart completion timing is not specified.  Complete it on the next
	// oscillator clock so the command and REQ transition remain observable.
	m_reset_timer->adjust(clocks_to_attotime(1));
	update_req();
}

TIMER_CALLBACK_MEMBER(tms320av110_device::reset_complete)
{
	m_decoder->registers[REG_RESET] = 0;
	m_decoder->registers[REG_RESTART] = 0;
	m_reset_cycle = false;
	update_req();
	start_input_timer();
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
	m_decoder->registers[REG_SYNC_ST] = 0x03;

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
	start_input_timer();

	if (sample_rate)
	{
		switch (sample_rate)
		{
		case 44'100: m_decoder->registers[REG_PCM_FS] = 0; break;
		case 48'000: m_decoder->registers[REG_PCM_FS] = 1; break;
		case 32'000: m_decoder->registers[REG_PCM_FS] = 2; break;
		default: break;
		}

		if (sample_rate != m_stream->sample_rate())
			m_stream->set_sample_rate(sample_rate);
	}

	return true;
}

void tms320av110_device::start_input_timer()
{
	if (!m_reset_asserted && !m_reset_cycle && m_decoder->input_fifo_count &&
		(m_data_access || (m_decoder->input_bytes != m_decoder->input.size())))
	{
		m_input_timer->adjust(clocks_to_attotime(1));
	}
}

TIMER_CALLBACK_MEMBER(tms320av110_device::input_tick)
{
	LOGMASKED(LOG_REQUESTS, "%s: input tick, FIFO=%u staging=%u\n", machine().describe_context(),
		m_decoder->input_fifo_count, m_decoder->input_bytes);
	m_stream->update();
	if (m_decoder->input_fifo_count && (m_decoder->input_bytes != m_decoder->input.size()))
	{
		m_decoder->input[m_decoder->input_bytes++] = m_decoder->input_fifo[m_decoder->input_fifo_read];
		m_decoder->input_fifo_read = (m_decoder->input_fifo_read + 1) % m_decoder->input_fifo.size();
		m_decoder->input_fifo_count--;
	}
	if ((m_decoder->pcm_position == m_decoder->pcm_count) &&
		(m_decoder->input_bytes == m_decoder->input.size()))
	{
		decode_frame();
	}

	m_data_access = false;
	update_req();
	start_input_timer();
}

void tms320av110_device::fifo_w(u8 data)
{
	LOGMASKED(LOG_REQUESTS, "%s: DATAIN %02x, FIFO=%u staging=%u\n", machine().describe_context(), data,
		m_decoder->input_fifo_count, m_decoder->input_bytes);
	if (m_reset_cycle)
	{
		LOGMASKED(LOG_REGISTERS, "%s: DATAIN write %02x during reset/restart\n", machine().describe_context(), data);
		return;
	}

	m_stream->update();
	const unsigned capacity = m_external_dram ? INPUT_BUFFER_BYTES_WITH_DRAM : INPUT_BUFFER_BYTES_WITHOUT_DRAM;
	if (m_decoder->input_fifo_count > capacity)
	{
		logerror("%s: compressed-audio input overflow\n", machine().describe_context());
		return;
	}

	m_decoder->input_fifo[m_decoder->input_fifo_write] = data;
	m_decoder->input_fifo_write = (m_decoder->input_fifo_write + 1) % m_decoder->input_fifo.size();
	m_decoder->input_fifo_count++;
	m_data_access = true;
	update_req();
	start_input_timer();
}

void tms320av110_device::set_req(int state)
{
	if (m_req_state != state)
	{
		LOGMASKED(LOG_REQUESTS, "%s: REQ=%d\n", machine().describe_context(), state);
		m_req_state = state;
		m_req_cb(state);
	}
}

void tms320av110_device::update_req()
{
	// REQ is active low.  DATAIN raises it for the access handshake, and it
	// remains high while reset is active or the selected input buffer is full.
	const unsigned capacity = m_external_dram ? INPUT_BUFFER_BYTES_WITH_DRAM : INPUT_BUFFER_BYTES_WITHOUT_DRAM;
	set_req((m_reset_asserted || m_reset_cycle || m_data_access ||
		(m_decoder->input_fifo_count >= capacity)) ? ASSERT_LINE : CLEAR_LINE);
}

void tms320av110_device::reset_w(int state)
{
	const bool asserted = !state;
	if (asserted && !m_reset_asserted)
	{
		m_reset_asserted = true;
		start_reset(true);
	}
	else if (!asserted && m_reset_asserted)
	{
		m_reset_asserted = false;
		update_req();
	}
}

u8 tms320av110_device::read(offs_t offset)
{
	if (m_reset_asserted)
		return 0; // The data sheet specifies that RESET low disables host accesses.

	offset &= HOST_ADDRESS_MASK;
	switch (offset)
	{
	case REG_BUFF_L:
		return (m_decoder->input_fifo_count / 4) & 0xff;

	case REG_BUFF_H:
		return (m_decoder->input_fifo_count / 4) >> 8;

	case REG_DRAM_EXT:
		return m_external_dram;

	case REG_SYNC_ST:
	case REG_PCM_FS:
	case REG_FREE_FORM_L:
	case REG_FREE_FORM_H:
	case REG_PCM_18:
	case REG_INTR_L:
	case REG_INTR_H:
	case REG_INTR_EN_L:
	case REG_INTR_EN_H:
	case REG_ATTEN_L:
	case REG_ATTEN_R:
	case REG_AUD_ID:
	case REG_AUD_ID_EN:
	case REG_SYNC_LCK:
	case REG_CRC_ECM:
	case REG_SYNC_ECM:
	case REG_PLAY:
	case REG_MUTE:
	case REG_SKIP:
	case REG_REPEAT:
	case REG_STR_SEL:
	case REG_PCM_ORD:
	case REG_LATENCY:
	case REG_RESET:
	case REG_RESTART:
	case REG_PCM_DIV:
	case REG_DIF:
	case REG_SIN_EN:
		return m_decoder->registers[offset];
	}

	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_REGISTERS, "%s: unimplemented register read %02x\n", machine().describe_context(), offset);
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

	LOGMASKED(LOG_REGISTERS, "%s: register write %02x = %02x\n", machine().describe_context(), offset, data);
	switch (offset)
	{
	case REG_PLAY:
	case REG_MUTE:
	case REG_ATTEN_L:
	case REG_ATTEN_R:
		m_stream->update();
		break;
	}

	switch (offset)
	{
	case REG_FREE_FORM_L:
	case REG_INTR_EN_L:
	case REG_INTR_EN_H:
		m_decoder->registers[offset] = data;
		break;

	case REG_FREE_FORM_H:
		m_decoder->registers[offset] = data & THREE_BIT_MASK;
		break;

	case REG_SYNC_LCK:
	case REG_CRC_ECM:
	case REG_SYNC_ECM:
	case REG_STR_SEL:
		m_decoder->registers[offset] = data & TWO_BIT_MASK;
		break;

	case REG_AUD_ID:
		m_decoder->registers[offset] = data & FIVE_BIT_MASK;
		break;

	case REG_ATTEN_L:
	case REG_ATTEN_R:
	case REG_PCM_DIV:
		m_decoder->registers[offset] = data & SIX_BIT_MASK;
		break;

	case REG_PCM_18:
	case REG_AUD_ID_EN:
	case REG_PLAY:
	case REG_MUTE:
	case REG_SKIP:
	case REG_REPEAT:
	case REG_PCM_ORD:
	case REG_LATENCY:
	case REG_DIF:
	case REG_SIN_EN:
		m_decoder->registers[offset] = data & SINGLE_BIT_MASK;
		break;

	case REG_RESET:
		if (data & SINGLE_BIT_MASK)
			start_reset(false);
		break;

	case REG_RESTART:
		if (data & SINGLE_BIT_MASK)
			start_restart();
		break;

	default:
		break;
	}
}

void tms320av110_device::sound_stream_update(sound_stream &stream)
{
	const bool playing = BIT(m_decoder->registers[REG_PLAY], 0) && !m_reset_asserted && !m_reset_cycle;
	const bool muted = BIT(m_decoder->registers[REG_MUTE], 0);
	const float left_gain = std::pow(10.0F, -float(m_decoder->registers[REG_ATTEN_L]) / 10.0F);
	const float right_gain = std::pow(10.0F, -float(m_decoder->registers[REG_ATTEN_R]) / 10.0F);

	for (int sample = 0; sample < stream.samples(); sample++)
	{
		if (!playing || ((m_decoder->pcm_position == m_decoder->pcm_count) && !decode_frame()))
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
		stream.put(LEFT_CHANNEL, sample, muted ? 0.0F : (float(left) / 32'768.0F) * left_gain);
		stream.put(RIGHT_CHANNEL, sample, muted ? 0.0F : (float(right) / 32'768.0F) * right_gain);
		m_decoder->pcm_position++;
	}
}
