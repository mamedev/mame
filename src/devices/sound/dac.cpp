// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    dac.cpp

    Four quadrant multiplying DAC.

    Binary Weighted Resistor Network, R-2R Ladder & PWM

    Binary, Ones Complement, Twos Complement or Sign Magnitude coding

***************************************************************************/

#include "emu.h"

#define DAC_GENERATOR_EPILOG(_dac_type, _dac_class, _dac_description, _dac_shortname) \
DEFINE_DEVICE_TYPE(_dac_type, _dac_class, _dac_shortname, _dac_description)

#include "dac.h"


//-------------------------------------------------
//  dac_mapper_unsigned - map an unsigned value of
//  the given number of bits to a sample value
//-------------------------------------------------

stream_buffer::sample_t dac_mapper_unsigned(u32 input, u8 bits)
{
	stream_buffer::sample_t scale = 1.0 / stream_buffer::sample_t((bits > 1) ? (1 << bits) : 1);
	input &= (1 << bits) - 1;
	return stream_buffer::sample_t(input) * scale;
}


//-------------------------------------------------
//  dac_mapper_signed - map a signed value of
//  the given number of bits to a sample value
//-------------------------------------------------

stream_buffer::sample_t dac_mapper_signed(u32 input, u8 bits)
{
	return dac_mapper_unsigned(input ^ (1 << (bits - 1)), bits);
}


//-------------------------------------------------
//  dac_mapper_ones_complement - map a value where
//  the top bit indicates the lower bits should be
//  treated as a negative 1s complement
//-------------------------------------------------

stream_buffer::sample_t dac_mapper_ones_complement(u32 input, u8 bits)
{
	// this mapping assumes symmetric reference voltages,
	// which is true for all existing cases
	if (BIT(input, bits - 1))
		return 0.5 - 0.5 * dac_mapper_unsigned(~input, bits - 1);
	else
		return 0.5 + 0.5 * dac_mapper_unsigned(input, bits - 1);
}


//-------------------------------------------------
//  dac_mapper_ones_complement - map a value where
//  the top bit is a sign bit and the lower bits
//  are absolute magnitude
//-------------------------------------------------

stream_buffer::sample_t dac_mapper_sign_magnitude(u32 input, u8 bits)
{
	// this mapping assumes symmetric reference voltages,
	// which is true for all existing cases
	if (BIT(input, bits - 1))
		return 0.5 - 0.5 * dac_mapper_unsigned(input, bits - 1);
	else
		return 0.5 + 0.5 * dac_mapper_unsigned(input, bits - 1);
}


//-------------------------------------------------
//  dac_device_base - constructor
//-------------------------------------------------

dac_device_base::dac_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 bits, dac_mapper_callback mapper, stream_buffer::sample_t gain) :
	device_t(mconfig, type, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_stream(nullptr),
	m_curval(0),
	m_value_map(1 << bits),
	m_bits(bits),
	m_mapper(mapper),
	m_gain(gain),
	m_vref_base(0),
	m_vref_range(0)
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
	int inputs = 0;
	if (m_vref_range == 0)
		inputs += 2;

	// create the stream
	m_stream = stream_alloc(inputs, 1, 48000 * 4, STREAM_DISABLE_INPUT_RESAMPLING);

	// save data
	save_item(NAME(m_curval));
}


//-------------------------------------------------
//  sound_stream_update - stream updates
//-------------------------------------------------

void dac_device_base::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &out = outputs[0];

	// rails are constant
	if (inputs.size() == 0)
	{
		out.fill(m_vref_base + m_curval * m_vref_range);
		return;
	}

	auto &pos = inputs[DAC_VREF_POS_INPUT];
	auto &neg = inputs[DAC_VREF_NEG_INPUT];

	// rails are streams but effectively constant
	if (pos.sample_rate() == SAMPLE_RATE_MINIMUM && neg.sample_rate() == SAMPLE_RATE_MINIMUM)
		out.fill(neg.get(0) + m_curval * (pos.get(0) - neg.get(0)));

	// rails are streams matching our output rate
	else if (pos.sample_rate() == out.sample_rate() && neg.sample_rate() == out.sample_rate())
	{
		for (int sampindex = 0; sampindex < out.samples(); sampindex++)
			out.put(sampindex, neg.get(sampindex) + m_curval * (pos.get(sampindex) - neg.get(sampindex)));
	}

	// other cases not supported for now
	else
		throw emu_fatalerror("Unsupported case: DAC input rate does not match DAC output rate");
}
