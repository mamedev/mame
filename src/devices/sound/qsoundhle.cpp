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

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

// device type definition
DEFINE_DEVICE_TYPE(QSOUND_HLE, qsound_hle_device, "qsound_hle", "QSound (HLE)")

// DSP internal ROM region
ROM_START( qsound_hle )
	ROM_REGION( 0x2000, "dsp", 0 )
	// removing WORD_SWAP from original definition
	ROM_LOAD16_WORD( "dl-1425.bin", 0x0000, 0x2000, CRC(d6cf5ef5) SHA1(555f50fe5cdf127619da7d854c03f4a244a0c501) )
	ROM_IGNORE( 0x4000 )
ROM_END

// DSP states
enum {
	STATE_INIT1		= 0x288,
	STATE_INIT2		= 0x61a,
	STATE_REFRESH1	= 0x039,
	STATE_REFRESH2	= 0x04f,
	STATE_NORMAL1	= 0x314,
	STATE_NORMAL2 	= 0x6b2,
};

enum {
	PANTBL_DRY		= 0,
	PANTBL_WET		= 1,
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qsound_hle_device - constructor
//-------------------------------------------------

qsound_hle_device::qsound_hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, QSOUND_HLE, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this, 24)
	, m_stream(nullptr)
	, m_data_latch(0)
{
}

//-------------------------------------------------
//  rom_bank_updated - the rom bank has changed
//-------------------------------------------------

void qsound_hle_device::rom_bank_updated()
{
	m_stream->update();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qsound_hle_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / 2 / 1248); // DSP program uses 1248 machine cycles per iteration

	// copy tables from the DSP rom
	uint16_t *dsp_rom = (uint16_t*)memregion("dsp")->base();
	memcpy(m_pan_tables, &dsp_rom[0x110], sizeof(m_pan_tables));
	memcpy(m_adpcm_shift, &dsp_rom[0x9dc], sizeof(m_adpcm_shift));
	memcpy(m_filter_data, &dsp_rom[0xd53], sizeof(m_filter_data));
	memcpy(m_filter_data2, &dsp_rom[0xf2e], sizeof(m_filter_data2));

	init_register_map();

	// state save
	// PCM registers
	for(int j=0;j<16;j++) // PCM voices
	{
		save_item(NAME(m_voice[j].bank),j);
		save_item(NAME(m_voice[j].addr),j);
		save_item(NAME(m_voice[j].phase),j);
		save_item(NAME(m_voice[j].rate),j);
		save_item(NAME(m_voice[j].loop_len),j);
		save_item(NAME(m_voice[j].end_addr),j);
		save_item(NAME(m_voice[j].volume),j);
		save_item(NAME(m_voice[j].echo),j);
	}

	for(int j=0;j<3;j++) // ADPCM voices
	{
		save_item(NAME(m_adpcm[j].start_addr),j);
		save_item(NAME(m_adpcm[j].end_addr),j);
		save_item(NAME(m_adpcm[j].bank),j);
		save_item(NAME(m_adpcm[j].volume),j);
		save_item(NAME(m_adpcm[j].flag),j);
		save_item(NAME(m_adpcm[j].cur_vol),j);
		save_item(NAME(m_adpcm[j].step_size),j);
		save_item(NAME(m_adpcm[j].cur_addr),j);
	}

	for(int j=0;j<19;j++) // PCM voices
	{
		save_item(NAME(m_voice_pan[j]),j);
	}

	// QSound registers
	save_item(NAME(m_echo.end_pos));
	save_item(NAME(m_echo.feedback));
	save_item(NAME(m_echo.length));
	save_item(NAME(m_echo.last_sample));
	save_item(NAME(m_echo.delay_line));
	save_item(NAME(m_echo.delay_pos));

	for(int j=0;j<2;j++)  // left, right
	{
		save_item(NAME(m_filter[j].tap_count),j);
		save_item(NAME(m_filter[j].delay_pos),j);
		save_item(NAME(m_filter[j].table_pos),j);
		save_item(NAME(m_filter[j].taps),j);
		save_item(NAME(m_filter[j].delay_line),j);

		save_item(NAME(m_alt_filter[j].tap_count),j);
		save_item(NAME(m_alt_filter[j].delay_pos),j);
		save_item(NAME(m_alt_filter[j].table_pos),j);
		save_item(NAME(m_alt_filter[j].taps),j);
		save_item(NAME(m_alt_filter[j].delay_line),j);

		save_item(NAME(m_wet[j].delay),j);
		save_item(NAME(m_wet[j].volume),j);
		save_item(NAME(m_wet[j].write_pos),j);
		save_item(NAME(m_wet[j].read_pos),j);
		save_item(NAME(m_wet[j].delay_line),j);

		save_item(NAME(m_dry[j].delay),j);
		save_item(NAME(m_dry[j].volume),j);
		save_item(NAME(m_dry[j].write_pos),j);
		save_item(NAME(m_dry[j].read_pos),j);
		save_item(NAME(m_dry[j].delay_line),j);
	}

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
	m_state = 0;
	m_state_counter = 0;
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void qsound_hle_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// Clear the buffers
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
	memset(outputs[1], 0, samples * sizeof(*outputs[1]));

	for (int i = 0; i < samples; i ++)
	{
		update_sample();
		outputs[0][i] = m_out[0];
		outputs[1][i] = m_out[1];
	}
}


WRITE8_MEMBER(qsound_hle_device::qsound_w)
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


READ8_MEMBER(qsound_hle_device::qsound_r)
{
	// ready bit (0x00 = busy, 0x80 == ready)
	m_stream->update();
	return m_ready_flag;
}


void qsound_hle_device::write_data(u8 address, u16 data)
{
	uint16_t *destination = register_map[address];
	if(destination)
		*destination = data;
	m_ready_flag = 0;
}

void qsound_hle_device::init_register_map()
{
	// unused registers
	for(int i=0;i<256;i++)
		register_map[i] = NULL;

	// PCM registers
	for(int i=0;i<16;i++) // PCM voices
	{
		register_map[(i<<3)+0] = (uint16_t*)&m_voice[(i+1)%16].bank; // Bank applies to the next channel
		register_map[(i<<3)+1] = (uint16_t*)&m_voice[i].addr; // Current sample position and start position.
		register_map[(i<<3)+2] = (uint16_t*)&m_voice[i].rate; // 4.12 fixed point decimal.
		register_map[(i<<3)+3] = (uint16_t*)&m_voice[i].phase;
		register_map[(i<<3)+4] = (uint16_t*)&m_voice[i].loop_len;
		register_map[(i<<3)+5] = (uint16_t*)&m_voice[i].end_addr;
		register_map[(i<<3)+6] = (uint16_t*)&m_voice[i].volume;
		register_map[(i<<3)+7] = NULL;	// unused
		register_map[i+0x80] = (uint16_t*)&m_voice_pan[i];
		register_map[i+0xba] = (uint16_t*)&m_voice[i].echo;
	}

	// ADPCM registers
	for(int i=0;i<3;i++) // ADPCM voices
	{
		// ADPCM sample rate is fixed to 8khz. (one channel is updated every third sample)
		register_map[(i<<2)+0xca] = (uint16_t*)&m_adpcm[i].start_addr;
		register_map[(i<<2)+0xcb] = (uint16_t*)&m_adpcm[i].end_addr;
		register_map[(i<<2)+0xcc] = (uint16_t*)&m_adpcm[i].bank;
		register_map[(i<<2)+0xcd] = (uint16_t*)&m_adpcm[i].volume;
		register_map[i+0xd6] = (uint16_t*)&m_adpcm[i].flag; // non-zero to start ADPCM playback
		register_map[i+0x90] = (uint16_t*)&m_voice_pan[16+i];
	}

	// QSound registers
	register_map[0x93] = (uint16_t*)&m_echo.feedback;
	register_map[0xd9] = (uint16_t*)&m_echo.end_pos;
	register_map[0xe2] = (uint16_t*)&m_delay_update; // non-zero to update delays
	register_map[0xe3] = (uint16_t*)&m_next_state;
	for(int i=0;i<2;i++)  // left, right
	{
		// Wet
		register_map[(i<<1)+0xda] = (uint16_t*)&m_filter[i].table_pos;
		register_map[(i<<1)+0xde] = (uint16_t*)&m_wet[i].delay;
		register_map[(i<<1)+0xe4] = (uint16_t*)&m_wet[i].volume;
		// Dry
		register_map[(i<<1)+0xdb] = (uint16_t*)&m_alt_filter[i].table_pos;
		register_map[(i<<1)+0xdf] = (uint16_t*)&m_dry[i].delay;
		register_map[(i<<1)+0xe5] = (uint16_t*)&m_dry[i].volume;
	}
}

int16_t qsound_hle_device::get_sample(uint16_t bank,uint16_t address)
{
	uint32_t rom_addr;
	uint8_t sample_data;

	bank &= 0x7FFF;
	rom_addr = (bank << 16) | (address << 0);

	sample_data = read_byte(rom_addr);

	return (int16_t)(sample_data << 8);	// bit0-7 is tied to ground
}

int16_t* qsound_hle_device::get_filter_table(uint16_t offset)
{
	int index;

	if (offset >= 0xf2e && offset < 0xfff)
		return (int16_t*)&m_filter_data2[offset-0xf2e];	// overlapping filter data

	index = (offset-0xd53)/95;
	if(index >= 0 && index < 5)
		return (int16_t*)&m_filter_data[index];	// normal tables

	return NULL;	// no filter found.
}

/********************************************************************/

// updates one DSP sample
void qsound_hle_device::update_sample()
{
	switch(m_state)
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
	if(m_state_counter >= 2)
	{
		m_state_counter = 0;
		m_state = m_next_state;
		return;
	}
	else if(m_state_counter == 1)
	{
		m_state_counter++;
		return;
	}

	memset(m_voice, 0, sizeof(m_voice));
	memset(m_adpcm, 0, sizeof(m_adpcm));
	memset(m_filter, 0, sizeof(m_filter));
	memset(m_alt_filter, 0, sizeof(m_alt_filter));
	memset(m_wet, 0, sizeof(m_wet));
	memset(m_dry, 0, sizeof(m_dry));
	memset(&m_echo, 0, sizeof(m_echo));

	for(int i=0;i<19;i++)
	{
		m_voice_pan[i] = 0x120;
		m_voice_output[i] = 0;
	}

	for(int i=0;i<16;i++)
		m_voice[i].bank = 0x8000;
	for(int i=0;i<3;i++)
		m_adpcm[i].bank = 0x8000;

	if(mode == 0)
	{
		// mode 1
		m_wet[0].delay = 0;
		m_dry[0].delay = 46;
		m_wet[1].delay = 0;
		m_dry[1].delay = 48;
		m_filter[0].table_pos = 0xdb2;
		m_filter[1].table_pos = 0xe11;
		m_echo.end_pos = 0x554 + 6;
		m_next_state = STATE_REFRESH1;
	}
	else
	{
		// mode 2
		m_wet[0].delay = 1;
		m_dry[0].delay = 0;
		m_wet[1].delay = 0;
		m_dry[1].delay = 0;
		m_filter[0].table_pos = 0xf73;
		m_filter[1].table_pos = 0xfa4;
		m_alt_filter[0].table_pos = 0xf73;
		m_alt_filter[1].table_pos = 0xfa4;
		m_echo.end_pos = 0x53c + 6;
		m_next_state = STATE_REFRESH2;
	}

	m_wet[0].volume = 0x3fff;
	m_dry[0].volume = 0x3fff;
	m_wet[1].volume = 0x3fff;
	m_dry[1].volume = 0x3fff;

	m_delay_update = 1;
	m_ready_flag = 0;
	m_state_counter = 1;
}

// Updates filter parameters for mode 1
void qsound_hle_device::state_refresh_filter_1()
{
	const int16_t *table;

	for(int ch=0; ch<2; ch++)
	{
		m_filter[ch].delay_pos = 0;
		m_filter[ch].tap_count = 95;

		table = get_filter_table(m_filter[ch].table_pos);
		if (table != NULL)
			memcpy(m_filter[ch].taps, table, 95 * sizeof(int16_t));
	}

	m_state = m_next_state = STATE_NORMAL1;
}

// Updates filter parameters for mode 2
void qsound_hle_device::state_refresh_filter_2()
{
	const int16_t *table;

	for(int ch=0; ch<2; ch++)
	{
		m_filter[ch].delay_pos = 0;
		m_filter[ch].tap_count = 45;

		table = get_filter_table(m_filter[ch].table_pos);
		if (table != NULL)
			memcpy(m_filter[ch].taps, table, 45 * sizeof(int16_t));

		m_alt_filter[ch].delay_pos = 0;
		m_alt_filter[ch].tap_count = 44;

		table = get_filter_table(m_filter[ch].table_pos);
		if (table != NULL)
			memcpy(m_alt_filter[ch].taps, table, 44 * sizeof(int16_t));
	}

	m_state = m_next_state = STATE_NORMAL2;
}

// Updates a PCM voice. There are 16 voices, each are updated every sample
// with full rate and volume control.
int16_t qsound_hle_device::pcm_update(struct qsound_voice *v, int32_t *echo_out)
{
	// Read sample from rom and apply volume
	int16_t output = (v->volume * get_sample(v->bank, v->addr))>>14;

	*echo_out += (output * v->echo)<<2;

	// Add delta to the phase and loop back if required
	int32_t new_phase = v->rate + ((v->addr<<12) | (v->phase>>4));

	if((new_phase>>12) >= v->end_addr)
		new_phase -= (v->loop_len<<12);

	new_phase = CLAMP(new_phase, -0x8000000, 0x7FFFFFF);
	v->addr = new_phase>>12;
	v->phase = (new_phase<<4)&0xffff;

	return output;
}

// Updates an ADPCM voice. There are 3 voices, one is updated every sample
// (effectively making the ADPCM rate 1/3 of the master sample rate), and
// volume is set when starting samples only.
// The ADPCM algorithm is supposedly similar to Yamaha ADPCM. It also seems
// like Capcom never used it, so this was not emulated in the earlier QSound
// emulators.
void qsound_hle_device::adpcm_update(int voice_no, int nibble)
{
	struct qsound_adpcm *v = &m_adpcm[voice_no];

	int32_t delta;
	int8_t step;

	if(!nibble)
	{
		// Mute voice when it reaches the end address.
		if(v->cur_addr == v->end_addr)
			v->cur_vol = 0;

		// Playback start flag
		if(v->flag)
		{
			m_voice_output[16+voice_no] = 0;
			v->flag = 0;
			v->step_size = 10;
			v->cur_vol = v->volume;
			v->cur_addr = v->start_addr;
		}

		// get top nibble
		step = get_sample(v->bank, v->cur_addr) >> 8;
	}
	else
	{
		// get bottom nibble
		step = get_sample(v->bank, v->cur_addr++) >> 4;
	}

	// shift with sign extend
	step >>= 4;

	// delta = (0.5 + abs(v->step)) * v->step_size
	delta = ((1+abs(step<<1)) * v->step_size)>>1;
	if(step <= 0)
		delta = -delta;
	delta += m_voice_output[16+voice_no];
	delta = CLAMP(delta,-32768,32767);

	m_voice_output[16+voice_no] = (delta * v->cur_vol)>>16;

	v->step_size = (m_adpcm_shift[8+step] * v->step_size) >> 6;
	v->step_size = CLAMP(v->step_size, 1, 2000);
}

// The echo effect is pretty simple. A moving average filter is used on
// the output from the delay line to smooth samples over time.
int16_t qsound_hle_device::echo(struct qsound_echo *r,int32_t input)
{
	// get average of last 2 samples from the delay line
	int32_t old_sample = r->delay_line[r->delay_pos];
	int32_t last_sample = r->last_sample;
	r->last_sample = old_sample;
	old_sample = (old_sample+last_sample) >> 1;

	// add current sample to the delay line
	int32_t new_sample = input + ((old_sample * r->feedback)<<2);
	r->delay_line[r->delay_pos++] = new_sample>>16;

	if(r->delay_pos >= r->length)
		r->delay_pos = 0;

	return old_sample;
}

// Process a sample update
void qsound_hle_device::state_normal_update()
{
	m_ready_flag = 0x80;
	int32_t echo_input = 0;
	int16_t echo_output;

	// recalculate echo length
	if(m_state == STATE_NORMAL2)
		m_echo.length = m_echo.end_pos - 0x53c;
	else
		m_echo.length = m_echo.end_pos - 0x554;

	m_echo.length = CLAMP(m_echo.length, 0, 1024);

	// update PCM voices
	for(int i=0; i<16; i++)
		m_voice_output[i] = pcm_update(&m_voice[i], &echo_input);

	// update ADPCM voices (one every third sample)
	adpcm_update(m_state_counter % 3, m_state_counter / 3);

	echo_output = echo(&m_echo,echo_input);

	// now, we do the magic stuff
	for(int ch=0; ch<2; ch++)
	{
		// Echo is output on the unfiltered component of the left channel and
		// the filtered component of the right channel.
		int32_t wet = (ch == 1) ? echo_output<<16 : 0;
		int32_t dry = (ch == 0) ? echo_output<<16 : 0;
		int32_t output = 0;

		for(int i=0; i<19; i++)
		{
			uint16_t pan_index = m_voice_pan[i]-0x110;
			if(pan_index > 97)
				pan_index = 97;

			// Apply different volume tables on the dry and wet inputs.
			dry -= (m_voice_output[i] * m_pan_tables[ch][PANTBL_DRY][pan_index])<<2;
			wet -= (m_voice_output[i] * m_pan_tables[ch][PANTBL_WET][pan_index])<<2;
		}

		// Apply FIR filter on 'wet' input
		wet = fir(&m_filter[ch], wet >> 16);

		// in mode 2, we do this on the 'dry' input too
		if(m_state == STATE_NORMAL2)
			dry = fir(&m_alt_filter[ch], dry >> 16);

		// output goes through a delay line and attenuation
		output = (delay(&m_wet[ch], wet) + delay(&m_dry[ch], dry))<<2;

		// DSP round function
		output = (output + 0x8000) & ~0xffff;
		m_out[ch] = output >> 16;

		if(m_delay_update)
		{
			delay_update(&m_wet[ch]);
			delay_update(&m_dry[ch]);
		}
	}

	m_delay_update = 0;

	// after 6 samples, the next state is executed.
	m_state_counter++;
	if(m_state_counter > 5)
	{
		m_state_counter = 0;
		m_state = m_next_state;
	}
}

// Apply the FIR filter used as the Q1 transfer function
int32_t qsound_hle_device::fir(struct qsound_fir *f, int16_t input)
{
	int32_t output = 0, tap = 0;

	for(; tap < (f->tap_count-1); tap++)
	{
		output -= (f->taps[tap] * f->delay_line[f->delay_pos++])<<2;

		if(f->delay_pos >= f->tap_count-1)
			f->delay_pos = 0;
	}

	output -= (f->taps[tap] * input)<<2;

	f->delay_line[f->delay_pos++] = input;
	if(f->delay_pos >= f->tap_count-1)
		f->delay_pos = 0;

	return output;
}

// Apply delay line and component volume
int32_t qsound_hle_device::delay(struct qsound_delay *d, int32_t input)
{
	int32_t output;

	d->delay_line[d->write_pos++] = input>>16;
	if(d->write_pos >= 51)
		d->write_pos = 0;

	output = d->delay_line[d->read_pos++]*d->volume;
	if(d->read_pos >= 51)
		d->read_pos = 0;

	return output;
}

// Update the delay read position to match new delay length
void qsound_hle_device::delay_update(struct qsound_delay *d)
{
	int16_t new_read_pos = (d->write_pos - d->delay) % 51;
	if(new_read_pos < 0)
		new_read_pos += 51;

	d->read_pos = new_read_pos;
}
