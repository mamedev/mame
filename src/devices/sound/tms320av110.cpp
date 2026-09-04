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

#include <cmath>

#define LOG_REQUESTS (1U << 1)
#define LOG_REGISTERS (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TMS320AV110, tms320av110_device, "tms320av110", "Texas Instruments TMS320AV110 MPEG Audio Decoder")

tms320av110_device::tms320av110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, TMS320AV110, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_req_cb(*this),
	m_input_fifo_read(0),
	m_input_fifo_write(0),
	m_input_fifo_count(0),
	m_input_bytes(0),
	m_input_bit_position(0),
	m_pcm_position(0),
	m_pcm_count(0),
	m_pcm_channels(0),
	m_external_dram(false),
	m_input_timer(nullptr),
	m_reset_timer(nullptr),
	m_reset_asserted(false),
	m_reset_cycle(false),
	m_data_access(false),
	m_req_state(CLEAR_LINE)
{
}

void tms320av110_device::device_start()
{
	m_stream = stream_alloc(0, OUTPUT_CHANNELS, INITIAL_SAMPLE_RATE);
	m_input_timer = timer_alloc(FUNC(tms320av110_device::input_tick), this);
	m_reset_timer = timer_alloc(FUNC(tms320av110_device::reset_complete), this);
	m_input_fifo = std::make_unique<u8[]>(INPUT_FIFO_BYTES);
	m_decoder = std::make_unique<mpeg_audio>(m_input, mpeg_audio::L2, false, 0);
	m_decoder->register_save_state(*this);

	save_item(NAME(m_registers));
	save_pointer(NAME(m_input_fifo), INPUT_FIFO_BYTES);
	save_item(NAME(m_input));
	save_item(NAME(m_pcm));
	save_item(NAME(m_input_fifo_read));
	save_item(NAME(m_input_fifo_write));
	save_item(NAME(m_input_fifo_count));
	save_item(NAME(m_input_bytes));
	save_item(NAME(m_input_bit_position));
	save_item(NAME(m_pcm_position));
	save_item(NAME(m_pcm_count));
	save_item(NAME(m_pcm_channels));
	save_item(NAME(m_reset_asserted));
	save_item(NAME(m_reset_cycle));
	save_item(NAME(m_data_access));
	save_item(NAME(m_req_state));
}

void tms320av110_device::device_reset()
{
	m_input_timer->adjust(attotime::never);
	m_reset_timer->adjust(attotime::never);
	std::fill(std::begin(m_registers), std::end(m_registers), 0);
	m_reset_asserted = false;
	m_reset_cycle = false;
	start_reset(true);
}

void tms320av110_device::device_post_load()
{
	m_req_cb(m_req_state);
}

void tms320av110_device::decoder_reset()
{
	if (m_stream)
	{
		m_stream->update();
		m_stream->set_sample_rate(INITIAL_SAMPLE_RATE);
	}

	std::fill(std::begin(m_input), std::end(m_input), 0);
	std::fill(std::begin(m_pcm), std::end(m_pcm), 0);
	m_input_bytes = 0;
	m_input_bit_position = 0;
	m_pcm_position = 0;
	m_pcm_count = 0;
	m_pcm_channels = 0;
	m_registers[REG_SYNC_ST] = 0;
	m_registers[REG_PCM_FS] = 0;
	m_decoder->clear();
}

void tms320av110_device::input_fifo_reset()
{
	m_input_timer->adjust(attotime::never);
	std::fill_n(m_input_fifo.get(), INPUT_FIFO_BYTES, 0);
	m_input_fifo_read = 0;
	m_input_fifo_write = 0;
	m_input_fifo_count = 0;
	m_data_access = false;
}

void tms320av110_device::start_reset(bool pin_reset)
{
	decoder_reset();
	input_fifo_reset();
	m_registers[REG_INTR_L] = 0;
	m_registers[REG_INTR_H] = 0;
	m_registers[REG_INTR_EN_L] = 0;
	m_registers[REG_INTR_EN_H] = 0;
	m_registers[REG_RESET] = 1;
	m_registers[REG_RESTART] = 0;
	if (pin_reset)
	{
		m_registers[REG_PLAY] = 0;
		m_registers[REG_MUTE] = 0;
		m_registers[REG_PCM_DIV] = 0;
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
	m_registers[REG_INTR_L] = 0;
	m_registers[REG_INTR_H] = 0;
	m_registers[REG_INTR_EN_L] = 0;
	m_registers[REG_INTR_EN_H] = 0;
	m_registers[REG_RESTART] = 1;
	m_reset_cycle = true;

	// Restart completion timing is not specified.  Complete it on the next
	// oscillator clock so the command and REQ transition remain observable.
	m_reset_timer->adjust(clocks_to_attotime(1));
	update_req();
}

TIMER_CALLBACK_MEMBER(tms320av110_device::reset_complete)
{
	m_registers[REG_RESET] = 0;
	m_registers[REG_RESTART] = 0;
	m_reset_cycle = false;
	update_req();
	start_input_timer();
}

bool tms320av110_device::decode_frame()
{
	int position = m_input_bit_position;
	int sample_count = 0;
	int sample_rate = 0;
	int channels = 0;
	if (!m_decoder->decode_buffer(
			position,
			m_input_bytes * 8,
			m_pcm,
			sample_count,
			sample_rate,
			channels))
	{
		return false;
	}

	assert(sample_count <= MPEG_LAYER_II_SAMPLES_PER_FRAME);
	assert((channels == 1) || (channels == OUTPUT_CHANNELS));
	m_pcm_position = 0;
	m_pcm_count = sample_count;
	m_pcm_channels = channels;
	m_registers[REG_SYNC_ST] = 0x03;

	const u32 bytes_consumed = position / 8;
	if (bytes_consumed)
	{
		std::move(m_input + bytes_consumed, m_input + m_input_bytes, m_input);
		m_input_bytes -= bytes_consumed;
	}
	m_input_bit_position = position & 7;
	start_input_timer();

	if (sample_rate)
	{
		switch (sample_rate)
		{
		case 44'100: m_registers[REG_PCM_FS] = 0; break;
		case 48'000: m_registers[REG_PCM_FS] = 1; break;
		case 32'000: m_registers[REG_PCM_FS] = 2; break;
		default: break;
		}

		if (sample_rate != m_stream->sample_rate())
			m_stream->set_sample_rate(sample_rate);
	}

	return true;
}

void tms320av110_device::start_input_timer()
{
	if (!m_reset_asserted && !m_reset_cycle && m_input_fifo_count &&
		(m_data_access || (m_input_bytes != DECODER_INPUT_BYTES)))
	{
		m_input_timer->adjust(clocks_to_attotime(1));
	}
}

TIMER_CALLBACK_MEMBER(tms320av110_device::input_tick)
{
	LOGMASKED(LOG_REQUESTS, "%s: input tick, FIFO=%u staging=%u\n", machine().describe_context(),
		m_input_fifo_count, m_input_bytes);
	m_stream->update();
	if (m_input_fifo_count && (m_input_bytes != DECODER_INPUT_BYTES))
	{
		m_input[m_input_bytes++] = m_input_fifo[m_input_fifo_read];
		m_input_fifo_read = (m_input_fifo_read + 1) % INPUT_FIFO_BYTES;
		m_input_fifo_count--;
	}
	if ((m_pcm_position == m_pcm_count) && (m_input_bytes == DECODER_INPUT_BYTES))
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
		m_input_fifo_count, m_input_bytes);
	if (m_reset_cycle)
	{
		LOGMASKED(LOG_REGISTERS, "%s: DATAIN write %02x during reset/restart\n", machine().describe_context(), data);
		return;
	}

	m_stream->update();
	const u32 capacity = m_external_dram ? INPUT_BUFFER_BYTES_WITH_DRAM : INPUT_BUFFER_BYTES_WITHOUT_DRAM;
	if (m_input_fifo_count > capacity)
	{
		logerror("%s: compressed-audio input overflow\n", machine().describe_context());
		return;
	}

	m_input_fifo[m_input_fifo_write] = data;
	m_input_fifo_write = (m_input_fifo_write + 1) % INPUT_FIFO_BYTES;
	m_input_fifo_count++;
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
	const u32 capacity = m_external_dram ? INPUT_BUFFER_BYTES_WITH_DRAM : INPUT_BUFFER_BYTES_WITHOUT_DRAM;
	set_req((m_reset_asserted || m_reset_cycle || m_data_access ||
		(m_input_fifo_count >= capacity)) ? ASSERT_LINE : CLEAR_LINE);
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

template <offs_t Register>
u8 tms320av110_device::register_r()
{
	if (m_reset_asserted)
		return 0; // The data sheet specifies that RESET low disables host accesses.

	return m_registers[Register];
}

void tms320av110_device::store_register(offs_t reg, u8 data, u8 mask, bool update_stream)
{
	if (m_reset_asserted)
		return; // The data sheet specifies that RESET low disables host accesses.

	LOGMASKED(LOG_REGISTERS, "%s: register write %02x = %02x\n", machine().describe_context(), reg, data);
	if (update_stream)
		m_stream->update();
	m_registers[reg] = data & mask;
}

void tms320av110_device::free_form_low_w(u8 data)
{
	store_register(REG_FREE_FORM_L, data, 0xff, false);
}

void tms320av110_device::free_form_high_w(u8 data)
{
	store_register(REG_FREE_FORM_H, data, 0x07, false);
}

void tms320av110_device::pcm_precision_w(u8 data)
{
	store_register(REG_PCM_18, data, 0x01, false);
}

void tms320av110_device::interrupt_enable_w(offs_t offset, u8 data)
{
	store_register(REG_INTR_EN_L + offset, data, 0xff, false);
}

void tms320av110_device::left_attenuation_w(u8 data)
{
	store_register(REG_ATTEN_L, data, 0x3f, true);
}

void tms320av110_device::right_attenuation_w(u8 data)
{
	store_register(REG_ATTEN_R, data, 0x3f, true);
}

void tms320av110_device::audio_id_w(u8 data)
{
	store_register(REG_AUD_ID, data, 0x1f, false);
}

void tms320av110_device::audio_id_enable_w(u8 data)
{
	store_register(REG_AUD_ID_EN, data, 0x01, false);
}

void tms320av110_device::sync_words_w(u8 data)
{
	store_register(REG_SYNC_LCK, data, 0x03, false);
}

void tms320av110_device::crc_error_concealment_w(u8 data)
{
	store_register(REG_CRC_ECM, data, 0x03, false);
}

void tms320av110_device::sync_error_concealment_w(u8 data)
{
	store_register(REG_SYNC_ECM, data, 0x03, false);
}

void tms320av110_device::play_w(u8 data)
{
	store_register(REG_PLAY, data, 0x01, true);
}

void tms320av110_device::mute_w(u8 data)
{
	store_register(REG_MUTE, data, 0x01, true);
}

void tms320av110_device::skip_w(u8 data)
{
	store_register(REG_SKIP, data, 0x01, false);
}

void tms320av110_device::repeat_w(u8 data)
{
	store_register(REG_REPEAT, data, 0x01, false);
}

void tms320av110_device::stream_select_w(u8 data)
{
	store_register(REG_STR_SEL, data, 0x03, false);
}

void tms320av110_device::pcm_order_w(u8 data)
{
	store_register(REG_PCM_ORD, data, 0x01, false);
}

void tms320av110_device::latency_w(u8 data)
{
	store_register(REG_LATENCY, data, 0x01, false);
}

void tms320av110_device::pcm_divider_w(u8 data)
{
	store_register(REG_PCM_DIV, data, 0x3f, false);
}

void tms320av110_device::pcm_justification_w(u8 data)
{
	store_register(REG_DIF, data, 0x01, false);
}

void tms320av110_device::serial_input_enable_w(u8 data)
{
	store_register(REG_SIN_EN, data, 0x01, false);
}

u8 tms320av110_device::buffer_low_r()
{
	return m_reset_asserted ? 0 : (m_input_fifo_count / 4) & 0xff;
}

u8 tms320av110_device::buffer_high_r()
{
	return m_reset_asserted ? 0 : (m_input_fifo_count / 4) >> 8;
}

u8 tms320av110_device::external_dram_r()
{
	return m_reset_asserted ? 0 : m_external_dram;
}

void tms320av110_device::data_w(u8 data)
{
	if (!m_reset_asserted)
		fifo_w(data);
}

void tms320av110_device::reset_command_w(u8 data)
{
	if (m_reset_asserted)
		return;

	LOGMASKED(LOG_REGISTERS, "%s: register write %02x = %02x\n", machine().describe_context(), REG_RESET, data);
	if (data & 0x01)
		start_reset(false);
}

void tms320av110_device::restart_command_w(u8 data)
{
	if (m_reset_asserted)
		return;

	LOGMASKED(LOG_REGISTERS, "%s: register write %02x = %02x\n", machine().describe_context(), REG_RESTART, data);
	if (data & 0x01)
		start_restart();
}

u8 tms320av110_device::unimplemented_r(offs_t offset)
{
	if (!m_reset_asserted && !machine().side_effects_disabled())
		LOGMASKED(LOG_REGISTERS, "%s: unimplemented register read %02x\n", machine().describe_context(), offset);
	return 0;
}

void tms320av110_device::unimplemented_w(offs_t offset, u8 data)
{
	if (!m_reset_asserted)
		LOGMASKED(LOG_REGISTERS, "%s: register write %02x = %02x\n", machine().describe_context(), offset, data);
}

void tms320av110_device::map(address_map &map)
{
	map(0x00, 0x7f).rw(FUNC(tms320av110_device::unimplemented_r), FUNC(tms320av110_device::unimplemented_w));
	map(0x12, 0x12).r(FUNC(tms320av110_device::buffer_low_r));
	map(0x13, 0x13).r(FUNC(tms320av110_device::buffer_high_r));
	map(0x14, 0x14).rw(FUNC(tms320av110_device::register_r<REG_FREE_FORM_L>), FUNC(tms320av110_device::free_form_low_w));
	map(0x15, 0x15).rw(FUNC(tms320av110_device::register_r<REG_FREE_FORM_H>), FUNC(tms320av110_device::free_form_high_w));
	map(0x16, 0x16).rw(FUNC(tms320av110_device::register_r<REG_PCM_18>), FUNC(tms320av110_device::pcm_precision_w));
	map(0x18, 0x18).w(FUNC(tms320av110_device::data_w));
	map(0x1a, 0x1a).r(FUNC(tms320av110_device::register_r<REG_INTR_L>));
	map(0x1b, 0x1b).r(FUNC(tms320av110_device::register_r<REG_INTR_H>));
	map(0x1c, 0x1d).w(FUNC(tms320av110_device::interrupt_enable_w));
	map(0x1c, 0x1c).r(FUNC(tms320av110_device::register_r<REG_INTR_EN_L>));
	map(0x1d, 0x1d).r(FUNC(tms320av110_device::register_r<REG_INTR_EN_H>));
	map(0x1e, 0x1e).rw(FUNC(tms320av110_device::register_r<REG_ATTEN_L>), FUNC(tms320av110_device::left_attenuation_w));
	map(0x20, 0x20).rw(FUNC(tms320av110_device::register_r<REG_ATTEN_R>), FUNC(tms320av110_device::right_attenuation_w));
	map(0x22, 0x22).rw(FUNC(tms320av110_device::register_r<REG_AUD_ID>), FUNC(tms320av110_device::audio_id_w));
	map(0x24, 0x24).rw(FUNC(tms320av110_device::register_r<REG_AUD_ID_EN>), FUNC(tms320av110_device::audio_id_enable_w));
	map(0x26, 0x26).r(FUNC(tms320av110_device::register_r<REG_SYNC_ST>));
	map(0x28, 0x28).rw(FUNC(tms320av110_device::register_r<REG_SYNC_LCK>), FUNC(tms320av110_device::sync_words_w));
	map(0x2a, 0x2a).rw(FUNC(tms320av110_device::register_r<REG_CRC_ECM>), FUNC(tms320av110_device::crc_error_concealment_w));
	map(0x2c, 0x2c).rw(FUNC(tms320av110_device::register_r<REG_SYNC_ECM>), FUNC(tms320av110_device::sync_error_concealment_w));
	map(0x2e, 0x2e).rw(FUNC(tms320av110_device::register_r<REG_PLAY>), FUNC(tms320av110_device::play_w));
	map(0x30, 0x30).rw(FUNC(tms320av110_device::register_r<REG_MUTE>), FUNC(tms320av110_device::mute_w));
	map(0x32, 0x32).rw(FUNC(tms320av110_device::register_r<REG_SKIP>), FUNC(tms320av110_device::skip_w));
	map(0x34, 0x34).rw(FUNC(tms320av110_device::register_r<REG_REPEAT>), FUNC(tms320av110_device::repeat_w));
	map(0x36, 0x36).rw(FUNC(tms320av110_device::register_r<REG_STR_SEL>), FUNC(tms320av110_device::stream_select_w));
	map(0x38, 0x38).rw(FUNC(tms320av110_device::register_r<REG_PCM_ORD>), FUNC(tms320av110_device::pcm_order_w));
	map(0x3c, 0x3c).rw(FUNC(tms320av110_device::register_r<REG_LATENCY>), FUNC(tms320av110_device::latency_w));
	map(0x3e, 0x3e).r(FUNC(tms320av110_device::external_dram_r));
	map(0x40, 0x40).rw(FUNC(tms320av110_device::register_r<REG_RESET>), FUNC(tms320av110_device::reset_command_w));
	map(0x42, 0x42).rw(FUNC(tms320av110_device::register_r<REG_RESTART>), FUNC(tms320av110_device::restart_command_w));
	map(0x44, 0x44).r(FUNC(tms320av110_device::register_r<REG_PCM_FS>));
	map(0x6e, 0x6e).rw(FUNC(tms320av110_device::register_r<REG_PCM_DIV>), FUNC(tms320av110_device::pcm_divider_w));
	map(0x6f, 0x6f).rw(FUNC(tms320av110_device::register_r<REG_DIF>), FUNC(tms320av110_device::pcm_justification_w));
	map(0x70, 0x70).rw(FUNC(tms320av110_device::register_r<REG_SIN_EN>), FUNC(tms320av110_device::serial_input_enable_w));
}

void tms320av110_device::sound_stream_update(sound_stream &stream)
{
	const bool playing = BIT(m_registers[REG_PLAY], 0) && !m_reset_asserted && !m_reset_cycle;
	stream.fill(LEFT_CHANNEL, 0);
	stream.fill(RIGHT_CHANNEL, 0);
	if (!playing)
		return;

	const bool muted = BIT(m_registers[REG_MUTE], 0);
	const float left_gain = std::pow(10.0F, -float(m_registers[REG_ATTEN_L]) / 10.0F);
	const float right_gain = std::pow(10.0F, -float(m_registers[REG_ATTEN_R]) / 10.0F);

	for (int sample = 0; sample < stream.samples(); sample++)
	{
		if ((m_pcm_position == m_pcm_count) && !decode_frame())
			break;

		const u32 position = m_pcm_position * m_pcm_channels;
		const s16 left = m_pcm[position];
		const s16 right = (m_pcm_channels == OUTPUT_CHANNELS)
			? m_pcm[position + RIGHT_CHANNEL]
			: left;
		if (!muted)
		{
			stream.put(LEFT_CHANNEL, sample, (float(left) / 32'768.0F) * left_gain);
			stream.put(RIGHT_CHANNEL, sample, (float(right) / 32'768.0F) * right_gain);
		}
		m_pcm_position++;
	}
}
