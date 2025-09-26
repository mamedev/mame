// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    dac.cpp

    Four quadrant multiplying DAC.

    Binary Weighted Resistor Network, R-2R Ladder & PWM

    Binary, Ones Complement, or Twos Complement coding

***************************************************************************/

#include "emu.h"

#define DAC_GENERATOR_EPILOG(_dac_type, _dac_class, _dac_description, _dac_shortname) \
DEFINE_DEVICE_TYPE(_dac_type, _dac_class, _dac_shortname, _dac_description)

#include "dac.h"


//-------------------------------------------------
//  dac_mapper_unsigned - map an unsigned value of
//  the given number of bits to a sample value
//-------------------------------------------------

sound_stream::sample_t dac_mapper_unsigned(u32 input, u8 bits)
{
	sound_stream::sample_t scale = 1.0 / sound_stream::sample_t((bits > 1) ? (1 << bits) : 1);
	input &= (1 << bits) - 1;
	return sound_stream::sample_t(input) * scale;
}


//-------------------------------------------------
//  dac_mapper_signed - map a signed (2s complement)
//  value of the given number of bits to a sample value
//-------------------------------------------------

sound_stream::sample_t dac_mapper_signed(u32 input, u8 bits)
{
	return dac_mapper_unsigned(input ^ (1 << (bits - 1)), bits);
}


//-------------------------------------------------
//  dac_mapper_ones_complement - map a value where
//  the top bit indicates the lower bits should be
//  treated as a negative 1s complement
//-------------------------------------------------

sound_stream::sample_t dac_mapper_ones_complement(u32 input, u8 bits)
{
	// this mapping assumes symmetric reference voltages,
	// which is true for all existing cases
	if (BIT(input, bits - 1))
		return 0.5 - 0.5 * dac_mapper_unsigned(~input, bits - 1);
	else
		return 0.5 + 0.5 * dac_mapper_unsigned(input, bits - 1);
}


//-------------------------------------------------
//  dac_device_base - constructor
//-------------------------------------------------

dac_device_base::dac_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 bits, dac_mapper_callback mapper, sound_stream::sample_t gain) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_curval(0),
	m_value_map(1 << bits),
	m_bits(bits),
	m_mapper(mapper),
	m_gain(gain),
	m_range_min((bits == 1) ? 0.0 : -1.0),
	m_range_max(1.0)
{
}


//-------------------------------------------------
//  device_start - device startup
//-------------------------------------------------

void dac_device_base::device_start()
{
	// precompute all gain-applied values
	for (s32 code = 0; code < m_value_map.size(); code++)
		m_value_map[code] = m_mapper(code, m_bits) * m_gain;

	// determine the number of inputs
	int inputs = (get_sound_requested_inputs_mask() == 0) ? 0 : 2;

	// large stream buffer to favour emu/sound.cpp resample quality
	m_stream = stream_alloc(inputs, 1, 48000 * 32);

	// save data
	save_item(NAME(m_curval));
}


//-------------------------------------------------
//  sound_stream_update - stream updates
//-------------------------------------------------

void dac_device_base::sound_stream_update(sound_stream &stream)
{
	// rails are constant
	if (stream.input_count() == 0)
	{
		stream.fill(0, m_range_min + m_curval * (m_range_max - m_range_min));
		return;
	}

	// constant lo, streaming hi
	if (!BIT(get_sound_requested_inputs_mask(), DAC_INPUT_RANGE_LO))
	{
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
			stream.put(0, sampindex, m_range_min + m_curval * (stream.get(DAC_INPUT_RANGE_HI, sampindex) - m_range_min));
	}

	// constant hi, streaming lo
	else if (!BIT(get_sound_requested_inputs_mask(), DAC_INPUT_RANGE_HI))
	{
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
			stream.put(0, sampindex, stream.get(DAC_INPUT_RANGE_LO, sampindex) + m_curval * (m_range_max - stream.get(DAC_INPUT_RANGE_LO, sampindex)));
	}

	// both streams provided
	else
	{
		for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
			stream.put(0, sampindex, stream.get(DAC_INPUT_RANGE_LO, sampindex) + m_curval * (stream.get(DAC_INPUT_RANGE_HI, sampindex) - stream.get(DAC_INPUT_RANGE_LO, sampindex)));
	}
}
