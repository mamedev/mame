// license:BSD-3-Clause
// copyright-holders:superctr, Valley Bell
/***************************************************************************

  Capcom QSound DL-1425 (HLE)
  ===========================

  Driver by superctr with thanks to Valley Bell.

  Based on disassembled DSP code.

  Links:
  https://siliconpr0n.org/map/capcom/dl-1425

***************************************************************************/

#include "emu.h"
#include "qsoundhle.h"

#include <algorithm>
#include <limits>

// device type definition
DEFINE_DEVICE_TYPE(QSOUND_HLE, qsound_hle_device, "qsound_hle", "QSound (HLE)")

// DSP internal ROM region
ROM_START( qsound_hle )
	ROM_REGION16_LE( 0x2000, "dsp", 0 )
	// removing WORD_SWAP from original definition
	ROM_LOAD16_WORD( "dl-1425.bin", 0x0000, 0x2000, CRC(d6cf5ef5) SHA1(555f50fe5cdf127619da7d854c03f4a244a0c501) )
	ROM_IGNORE( 0x4000 )
ROM_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qsound_hle_device - constructor
//-------------------------------------------------

qsound_hle_device::qsound_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, QSOUND_HLE, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_dsp_rom(*this, "dsp")
	, m_data_latch(0)
{
}

//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void qsound_hle_device::rom_bank_pre_change()
{
	m_stream->update();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qsound_hle_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / 2 / 1248); // DSP program uses 1248 machine cycles per iteration

	init_register_map();

	// state save
	// PCM registers
	// PCM voices
	save_item(STRUCT_MEMBER(m_voice, m_bank));
	save_item(STRUCT_MEMBER(m_voice, m_addr));
	save_item(STRUCT_MEMBER(m_voice, m_phase));
	save_item(STRUCT_MEMBER(m_voice, m_rate));
	save_item(STRUCT_MEMBER(m_voice, m_loop_len));
	save_item(STRUCT_MEMBER(m_voice, m_end_addr));
	save_item(STRUCT_MEMBER(m_voice, m_volume));
	save_item(STRUCT_MEMBER(m_voice, m_echo));

	// ADPCM voices
	save_item(STRUCT_MEMBER(m_adpcm, m_start_addr));
	save_item(STRUCT_MEMBER(m_adpcm, m_end_addr));
	save_item(STRUCT_MEMBER(m_adpcm, m_bank));
	save_item(STRUCT_MEMBER(m_adpcm, m_volume));
	save_item(STRUCT_MEMBER(m_adpcm, m_flag));
	save_item(STRUCT_MEMBER(m_adpcm, m_cur_vol));
	save_item(STRUCT_MEMBER(m_adpcm, m_step_size));
	save_item(STRUCT_MEMBER(m_adpcm, m_cur_addr));

	// PCM voices
	save_item(NAME(m_voice_pan));

	// QSound registers
	save_item(NAME(m_echo.m_end_pos));
	save_item(NAME(m_echo.m_feedback));
	save_item(NAME(m_echo.m_length));
	save_item(NAME(m_echo.m_last_sample));
	save_item(NAME(m_echo.m_delay_line));
	save_item(NAME(m_echo.m_delay_pos));

	// left, right
	save_item(STRUCT_MEMBER(m_filter, m_tap_count));
	save_item(STRUCT_MEMBER(m_filter, m_delay_pos));
	save_item(STRUCT_MEMBER(m_filter, m_table_pos));
	save_item(STRUCT_MEMBER(m_filter, m_taps));
	save_item(STRUCT_MEMBER(m_filter, m_delay_line));

	save_item(STRUCT_MEMBER(m_alt_filter, m_tap_count));
	save_item(STRUCT_MEMBER(m_alt_filter, m_delay_pos));
	save_item(STRUCT_MEMBER(m_alt_filter, m_table_pos));
	save_item(STRUCT_MEMBER(m_alt_filter, m_taps));
	save_item(STRUCT_MEMBER(m_alt_filter, m_delay_line));

	save_item(STRUCT_MEMBER(m_wet, m_delay));
	save_item(STRUCT_MEMBER(m_wet, m_volume));
	save_item(STRUCT_MEMBER(m_wet, m_write_pos));
	save_item(STRUCT_MEMBER(m_wet, m_read_pos));
	save_item(STRUCT_MEMBER(m_wet, m_delay_line));

	save_item(STRUCT_MEMBER(m_dry, m_delay));
	save_item(STRUCT_MEMBER(m_dry, m_volume));
	save_item(STRUCT_MEMBER(m_dry, m_write_pos));
	save_item(STRUCT_MEMBER(m_dry, m_read_pos));
	save_item(STRUCT_MEMBER(m_dry, m_delay_line));

	save_item(NAME(m_state));
	save_item(NAME(m_next_state));
	save_item(NAME(m_delay_update));
	save_item(NAME(m_state_counter));
	save_item(NAME(m_ready_flag));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_out));
}

//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const tiny_rom_entry *qsound_hle_device::device_rom_region() const
{
	return ROM_NAME( qsound_hle );
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void qsound_hle_device::device_reset()
{
	m_ready_flag = 0;
	m_out[0] = m_out[1] = 0;
	m_state = STATE_BOOT;
	m_state_counter = 0;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void qsound_hle_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i ++)
	{
		update_sample();
		stream.put_int(0, i, m_out[0], 32768);
		stream.put_int(1, i, m_out[1], 32768);
	}
}


void qsound_hle_device::qsound_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_data_latch = (m_data_latch & 0x00ff) | (data << 8);
			break;

		case 1:
			m_data_latch = (m_data_latch & 0xff00) | data;
			break;

		case 2:
			m_stream->update();
			write_data(data, m_data_latch);
			break;

		default:
			logerror("%s: qsound_w %d = %02x\n", machine().describe_context(), offset, data);
			break;
	}
}


uint8_t qsound_hle_device::qsound_r()
{
	// ready bit (0x00 = busy, 0x80 == ready)
	m_stream->update();
	return m_ready_flag;
}


void qsound_hle_device::write_data(uint8_t address, uint16_t data)
{
	uint16_t *destination = m_register_map[address];
	if (destination)
		*destination = data;
	m_ready_flag = 0;
}

void qsound_hle_device::init_register_map()
{
	// unused registers
	std::fill(std::begin(m_register_map), std::end(m_register_map), nullptr);

	// PCM registers
	for (int i = 0; i < 16; i++) // PCM voices
	{
		m_register_map[(i << 3) + 0] = (uint16_t*)&m_voice[(i + 1) % 16].m_bank; // Bank applies to the next channel
		m_register_map[(i << 3) + 1] = (uint16_t*)&m_voice[i].m_addr; // Current sample position and start position.
		m_register_map[(i << 3) + 2] = (uint16_t*)&m_voice[i].m_rate; // 4.12 fixed point decimal.
		m_register_map[(i << 3) + 3] = (uint16_t*)&m_voice[i].m_phase;
		m_register_map[(i << 3) + 4] = (uint16_t*)&m_voice[i].m_loop_len;
		m_register_map[(i << 3) + 5] = (uint16_t*)&m_voice[i].m_end_addr;
		m_register_map[(i << 3) + 6] = (uint16_t*)&m_voice[i].m_volume;
		m_register_map[(i << 3) + 7] = nullptr; // unused
		m_register_map[i + 0x80] = (uint16_t*)&m_voice_pan[i];
		m_register_map[i + 0xba] = (uint16_t*)&m_voice[i].m_echo;
	}

	// ADPCM registers
	for (int i = 0; i < 3; i++) // ADPCM voices
	{
		// ADPCM sample rate is fixed to 8khz. (one channel is updated every third sample)
		m_register_map[(i << 2) + 0xca] = (uint16_t*)&m_adpcm[i].m_start_addr;
		m_register_map[(i << 2) + 0xcb] = (uint16_t*)&m_adpcm[i].m_end_addr;
		m_register_map[(i << 2) + 0xcc] = (uint16_t*)&m_adpcm[i].m_bank;
		m_register_map[(i << 2) + 0xcd] = (uint16_t*)&m_adpcm[i].m_volume;
		m_register_map[i + 0xd6] = (uint16_t*)&m_adpcm[i].m_flag; // non-zero to start ADPCM playback
		m_register_map[i + 0x90] = (uint16_t*)&m_voice_pan[16 + i];
	}

	// QSound registers
	m_register_map[0x93] = (uint16_t*)&m_echo.m_feedback;
	m_register_map[0xd9] = (uint16_t*)&m_echo.m_end_pos;
	m_register_map[0xe2] = (uint16_t*)&m_delay_update; // non-zero to update delays
	m_register_map[0xe3] = (uint16_t*)&m_next_state;
	for (int i = 0; i < 2; i++)  // left, right
	{
		// Wet
		m_register_map[(i << 1) + 0xda] = (uint16_t*)&m_filter[i].m_table_pos;
		m_register_map[(i << 1) + 0xde] = (uint16_t*)&m_wet[i].m_delay;
		m_register_map[(i << 1) + 0xe4] = (uint16_t*)&m_wet[i].m_volume;
		// Dry
		m_register_map[(i << 1) + 0xdb] = (uint16_t*)&m_alt_filter[i].m_table_pos;
		m_register_map[(i << 1) + 0xdf] = (uint16_t*)&m_dry[i].m_delay;
		m_register_map[(i << 1) + 0xe5] = (uint16_t*)&m_dry[i].m_volume;
	}
}

int16_t qsound_hle_device::read_sample(uint16_t bank, uint16_t address)
{
	bank &= 0x7FFF;
	const uint32_t rom_addr = (bank << 16) | (address << 0);
	const uint8_t sample_data = read_byte(rom_addr);
	return (int16_t)(sample_data << 8); // bit0-7 is tied to ground
}

/********************************************************************/

// updates one DSP sample
void qsound_hle_device::update_sample()
{
	switch (m_state)
	{
		default:
		case STATE_INIT1:
		case STATE_INIT2:
			return state_init();
		case STATE_REFRESH1:
			return state_refresh_filter_1();
		case STATE_REFRESH2:
			return state_refresh_filter_2();
		case STATE_NORMAL1:
		case STATE_NORMAL2:
			return state_normal_update();
	}
}

// Initialization routine
void qsound_hle_device::state_init()
{
	int mode = (m_state == STATE_INIT2) ? 1 : 0;

	// we're busy for 4 samples, including the filter refresh.
	if (m_state_counter >= 2)
	{
		m_state_counter = 0;
		m_state = m_next_state;
		return;
	}
	else if (m_state_counter == 1)
	{
		m_state_counter++;
		return;
	}

	std::fill(std::begin(m_voice), std::end(m_voice), qsound_voice());
	std::fill(std::begin(m_adpcm), std::end(m_adpcm), qsound_adpcm());
	std::fill(std::begin(m_filter), std::end(m_filter), qsound_fir());
	std::fill(std::begin(m_alt_filter), std::end(m_alt_filter), qsound_fir());
	std::fill(std::begin(m_wet), std::end(m_wet), qsound_delay());
	std::fill(std::begin(m_dry), std::end(m_dry), qsound_delay());
	m_echo = qsound_echo();

	for (int i = 0; i < 19; i++)
	{
		m_voice_pan[i] = DATA_PAN_TAB + 0x10;
		m_voice_output[i] = 0;
	}

	for (int i = 0; i < 16; i++)
		m_voice[i].m_bank = 0x8000;
	for (int i = 0; i < 3; i++)
		m_adpcm[i].m_bank = 0x8000;

	if (mode == 0)
	{
		// mode 1
		m_wet[0].m_delay = 0;
		m_dry[0].m_delay = 46;
		m_wet[1].m_delay = 0;
		m_dry[1].m_delay = 48;
		m_filter[0].m_table_pos = DATA_FILTER_TAB + (FILTER_ENTRY_SIZE*1);
		m_filter[1].m_table_pos = DATA_FILTER_TAB + (FILTER_ENTRY_SIZE*2);
		m_echo.m_end_pos = DELAY_BASE_OFFSET + 6;
		m_next_state = STATE_REFRESH1;
	}
	else
	{
		// mode 2
		m_wet[0].m_delay = 1;
		m_dry[0].m_delay = 0;
		m_wet[1].m_delay = 0;
		m_dry[1].m_delay = 0;
		m_filter[0].m_table_pos = 0xf73;
		m_filter[1].m_table_pos = 0xfa4;
		m_alt_filter[0].m_table_pos = 0xf73;
		m_alt_filter[1].m_table_pos = 0xfa4;
		m_echo.m_end_pos = DELAY_BASE_OFFSET2 + 6;
		m_next_state = STATE_REFRESH2;
	}

	m_wet[0].m_volume = 0x3fff;
	m_dry[0].m_volume = 0x3fff;
	m_wet[1].m_volume = 0x3fff;
	m_dry[1].m_volume = 0x3fff;

	m_delay_update = 1;
	m_ready_flag = 0;
	m_state_counter = 1;
}

// Updates filter parameters for mode 1
void qsound_hle_device::state_refresh_filter_1()
{
	for (int ch = 0; ch < 2; ch++)
	{
		m_filter[ch].m_delay_pos = 0;
		m_filter[ch].m_tap_count = 95;

		for (int i = 0; i < 95; i++)
			m_filter[ch].m_taps[i] = read_dsp_rom(m_filter[ch].m_table_pos + i);
	}

	m_state = m_next_state = STATE_NORMAL1;
}

// Updates filter parameters for mode 2
void qsound_hle_device::state_refresh_filter_2()
{
	for (int ch = 0; ch < 2; ch++)
	{
		m_filter[ch].m_delay_pos = 0;
		m_filter[ch].m_tap_count = 45;

		for (int i = 0; i < 45; i++)
			m_filter[ch].m_taps[i] = (int16_t)read_dsp_rom(m_filter[ch].m_table_pos + i);

		m_alt_filter[ch].m_delay_pos = 0;
		m_alt_filter[ch].m_tap_count = 44;

		for (int i = 0; i < 44; i++)
			m_alt_filter[ch].m_taps[i] = (int16_t)read_dsp_rom(m_alt_filter[ch].m_table_pos + i);
	}

	m_state = m_next_state = STATE_NORMAL2;
}

// Updates a PCM voice. There are 16 voices, each are updated every sample
// with full rate and volume control.
int16_t qsound_hle_device::qsound_voice::update(qsound_hle_device &dsp, int32_t *echo_out)
{
	// Read sample from rom and apply volume
	const int16_t output = (m_volume * dsp.read_sample(m_bank, m_addr)) >> 14;

	*echo_out += (output * m_echo) << 2;

	// Add delta to the phase and loop back if required
	int32_t new_phase = m_rate + ((m_addr << 12) | (m_phase >> 4));

	if ((new_phase >> 12) >= m_end_addr)
		new_phase -= (m_loop_len << 12);

	new_phase = std::clamp<int32_t>(new_phase, -0x8000000, 0x7FFFFFF);
	m_addr = new_phase >> 12;
	m_phase = (new_phase << 4)&0xffff;

	return output;
}

// Updates an ADPCM voice. There are 3 voices, one is updated every sample
// (effectively making the ADPCM rate 1/3 of the master sample rate), and
// volume is set when starting samples only.
// The ADPCM algorithm is supposedly similar to Yamaha ADPCM. It also seems
// like Capcom never used it, so this was not emulated in the earlier QSound
// emulators.
int16_t qsound_hle_device::qsound_adpcm::update(qsound_hle_device &dsp, int16_t curr_sample, int nibble)
{
	int8_t step;
	if (!nibble)
	{
		// Mute voice when it reaches the end address.
		if (m_cur_addr == m_end_addr)
			m_cur_vol = 0;

		// Playback start flag
		if (m_flag)
		{
			curr_sample = 0;
			m_flag = 0;
			m_step_size = 10;
			m_cur_vol = m_volume;
			m_cur_addr = m_start_addr;
		}

		// get top nibble
		step = dsp.read_sample(m_bank, m_cur_addr) >> 8;
	}
	else
	{
		// get bottom nibble
		step = dsp.read_sample(m_bank, m_cur_addr++) >> 4;
	}

	// shift with sign extend
	step >>= 4;

	// delta = (0.5 + abs(step)) * m_step_size
	int32_t delta = ((1 + abs(step << 1)) * m_step_size) >> 1;
	if (step <= 0)
		delta = -delta;
	delta += curr_sample;
	delta = std::clamp<int32_t>(delta, -32768, 32767);

	m_step_size = (dsp.read_dsp_rom(DATA_ADPCM_TAB + 8 + step) * m_step_size) >> 6;
	m_step_size = std::clamp<int16_t>(m_step_size, 1, 2000);

	return (delta * m_cur_vol) >> 16;
}

// The echo effect is pretty simple. A moving average filter is used on
// the output from the delay line to smooth samples over time.
int16_t qsound_hle_device::qsound_echo::apply(int32_t input)
{
	// get average of last 2 samples from the delay line
	int32_t old_sample = m_delay_line[m_delay_pos];
	const int32_t last_sample = m_last_sample;
	m_last_sample = old_sample;
	old_sample = (old_sample + last_sample) >> 1;

	// add current sample to the delay line
	int32_t new_sample = input + ((old_sample * m_feedback) << 2);
	m_delay_line[m_delay_pos++] = new_sample >> 16;

	if (m_delay_pos >= m_length)
		m_delay_pos = 0;

	return old_sample;
}

// Process a sample update
void qsound_hle_device::state_normal_update()
{
	m_ready_flag = 0x80;

	// recalculate echo length
	if (m_state == STATE_NORMAL2)
		m_echo.m_length = m_echo.m_end_pos - DELAY_BASE_OFFSET2;
	else
		m_echo.m_length = m_echo.m_end_pos - DELAY_BASE_OFFSET;

	m_echo.m_length = std::clamp<int16_t>(m_echo.m_length, 0, 1024);

	// update PCM voices
	int32_t echo_input = 0;
	for (int i = 0; i < 16; i++)
		m_voice_output[i] = m_voice[i].update(*this, &echo_input);

	// update ADPCM voices (one every third sample)
	const int adpcm_voice = m_state_counter % 3;
	m_voice_output[16 + adpcm_voice] = m_adpcm[adpcm_voice].update(*this, m_voice_output[16 + adpcm_voice], m_state_counter / 3);

	int16_t echo_output = m_echo.apply(echo_input);

	// now, we do the magic stuff
	for (int ch = 0; ch < 2; ch++)
	{
		// Echo is output on the unfiltered component of the left channel and
		// the filtered component of the right channel.
		int32_t wet = (ch == 1) ? echo_output << 14 : 0;
		int32_t dry = (ch == 0) ? echo_output << 14 : 0;

		for (int i = 0; i < 19; i++)
		{
			uint16_t pan_index = m_voice_pan[i] + (ch * PAN_TABLE_CH_OFFSET);

			// Apply different volume tables on the dry and wet inputs.
			dry -= (m_voice_output[i] * (int16_t)read_dsp_rom(pan_index + PAN_TABLE_DRY));
			wet -= (m_voice_output[i] * (int16_t)read_dsp_rom(pan_index + PAN_TABLE_WET));
		}
		// Saturate accumulated voices
		dry = std::clamp<int32_t>(dry, -0x1fffffff, 0x1fffffff) << 2;
		wet = std::clamp<int32_t>(wet, -0x1fffffff, 0x1fffffff) << 2;

		// Apply FIR filter on 'wet' input
		wet = m_filter[ch].apply(wet >> 16);

		// in mode 2, we do this on the 'dry' input too
		if (m_state == STATE_NORMAL2)
			dry = m_alt_filter[ch].apply(dry >> 16);

		// output goes through a delay line and attenuation
		int32_t output = (m_wet[ch].apply(wet) + m_dry[ch].apply(dry));

		// DSP round function
		output = (output + 0x2000) & ~0x3fff;
		m_out[ch] = std::clamp<int32_t>(output >> 14, -0x7fff, 0x7fff);

		if (m_delay_update)
		{
			m_wet[ch].update();
			m_dry[ch].update();
		}
	}

	m_delay_update = 0;

	// after 6 samples, the next state is executed.
	m_state_counter++;
	if (m_state_counter > 5)
	{
		m_state_counter = 0;
		m_state = m_next_state;
	}
}

// Apply the FIR filter used as the Q1 transfer function
int32_t qsound_hle_device::qsound_fir::apply(int16_t input)
{
	int32_t output = 0, tap = 0;
	for (; tap < (m_tap_count - 1); tap++)
	{
		output -= (m_taps[tap] * m_delay_line[m_delay_pos++]) << 2;

		if (m_delay_pos >= m_tap_count - 1)
			m_delay_pos = 0;
	}

	output -= (m_taps[tap] * input) << 2;

	m_delay_line[m_delay_pos++] = input;
	if (m_delay_pos >= m_tap_count - 1)
		m_delay_pos = 0;

	return output;
}

// Apply delay line and component volume
int32_t qsound_hle_device::qsound_delay::apply(const int32_t input)
{
	m_delay_line[m_write_pos++] = input >> 16;
	if (m_write_pos >= 51)
		m_write_pos = 0;

	const int32_t output = m_delay_line[m_read_pos++] * m_volume;
	if (m_read_pos >= 51)
		m_read_pos = 0;

	return output;
}

// Update the delay read position to match new delay length
void qsound_hle_device::qsound_delay::update()
{
	const int16_t new_read_pos = (m_write_pos - m_delay) % 51;
	if (new_read_pos < 0)
		m_read_pos = new_read_pos + 51;
	else
		m_read_pos = new_read_pos;
}
