// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Couriersud,Derrick Renaud,Frank Palazzolo,Jonathan Gevaryahu
#include "emu.h"
#include "flt_biquad.h"

// we need the M_SQRT2 constant
#ifndef M_SQRT2
#define M_SQRT2 1.41421356237309504880
#endif

// device type definition
DEFINE_DEVICE_TYPE(FILTER_BIQUAD, filter_biquad_device, "filter_biquad", "Biquad Filter")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  filter_biquad_device - constructor
//-------------------------------------------------

filter_biquad_device::filter_biquad_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FILTER_BIQUAD, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_type(HIGHPASS),
		m_last_sample_rate(0),
		m_fc(16.0),
		m_q(M_SQRT2/2.0),
		m_gain(1.0),
		m_input(0.0),
		m_w0(0.0),
		m_w1(0.0),
		m_w2(0.0),
		m_output(0.0),
		m_a1(0.0),
		m_a2(0.0),
		m_b0(1.0),
		m_b1(0.0),
		m_b2(0.0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void filter_biquad_device::device_start()
{
	m_stream = stream_alloc(1, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	m_last_sample_rate = 0;
	recalc();

	save_item(NAME(m_type));
	save_item(NAME(m_last_sample_rate));
	save_item(NAME(m_fc));
	save_item(NAME(m_q));
	save_item(NAME(m_gain));
	save_item(NAME(m_input));
	save_item(NAME(m_w0));
	save_item(NAME(m_w1));
	save_item(NAME(m_w2));
	save_item(NAME(m_output));
	save_item(NAME(m_a1));
	save_item(NAME(m_a2));
	save_item(NAME(m_b0));
	save_item(NAME(m_b1));
	save_item(NAME(m_b2));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void filter_biquad_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &src = inputs[0];
	auto &dst = outputs[0];

	if (m_last_sample_rate != m_stream->sample_rate())
	{
		recalc();
		m_last_sample_rate = m_stream->sample_rate();
	}

	for (int sampindex = 0; sampindex < dst.samples(); sampindex++)
	{
		m_input = src.get(sampindex);
		step();
		dst.put(sampindex, m_output);
	}
}


/* Calculate the filter context based on the passed filter type info.
 * m_type - 1 of the 9 defined filter types
 * m_fc   - center frequency
 * m_q    - 'Q' (quality) factor of filter (1/damp)
 * m_gain - overall filter gain. Set to 1.0 if not needed. The exact meaning of gain changes depending on the filter type.
 */
void filter_biquad_device::recalc()
{
	double const MGain = fabs(m_gain); // absolute multiplicative gain
	double const DBGain = log10(MGain) * 20.0; // gain in dB
	double const AMGain = pow(10, fabs(DBGain) / 20.0); // multiplicative gain of absolute DB
	double const K = tan(M_PI * m_fc / m_stream->sample_rate());
	double const Ksquared = K * K;
	double const KoverQ = K / m_q;
	double normal = 1.0 / (1.0 + KoverQ + Ksquared);

	m_a1 = 2.0 * (Ksquared - 1.0) * normal;
	m_a2 = (1.0 - KoverQ + Ksquared) * normal;

	switch (m_type)
	{
		case LOWPASS1P:
			m_a1 = exp(-2.0 * M_PI * (m_fc / m_stream->sample_rate()));
			m_b0 = 1.0 - m_a1;
			m_a1 = -m_a1;
			m_b1 = m_b2 = m_a2 = 0.0;
			break;
		case HIGHPASS1P:
			m_a1 = -exp(-2.0 * M_PI * (0.5 - m_fc / m_stream->sample_rate()));
			m_b0 = 1.0 + m_a1;
			m_a1 = -m_a1;
			m_b1 = m_b2 = m_a2 = 0.0;
			break;
		case LOWPASS:
			m_b0 = Ksquared * normal;
			m_b1 = 2.0 * m_b0;
			m_b2 = 1.0 * m_b0;
			m_a1 = 2.0 * (Ksquared - 1.0) * normal;
			m_a2 = (1.0 - KoverQ + Ksquared) * normal;
			break;
		case HIGHPASS:
			m_b0 = 1.0 * normal;
			m_b1 = -2.0 * m_b0;
			m_b2 = 1.0 * m_b0;
			m_a1 = 2.0 * (Ksquared - 1.0) * normal;
			m_a2 = (1.0 - KoverQ + Ksquared) * normal;
			break;
		case BANDPASS:
			m_b0 = KoverQ * normal;
			m_b1 = 0.0;
			m_b2 = -1.0 * m_b0;
			m_a1 = 2.0 * (Ksquared - 1.0) * normal;
			m_a2 = (1.0 - KoverQ + Ksquared) * normal;
			break;
		case NOTCH:
			m_b0 = (1.0 + Ksquared) * normal;
			m_b1 = 2.0 * (Ksquared - 1.0) * normal;
			m_b2 = 1.0 * m_b0;
			m_a1 = 1.0 * m_b1;
			m_a2 = (1.0 - KoverQ + Ksquared) * normal;
			break;
		case PEAK:
			if (DBGain >= 0.0)
			{
				m_b0 = (1.0 + (AMGain * KoverQ) + Ksquared) * normal;
				m_b1 = 2.0 * (Ksquared - 1.0) * normal;
				m_b2 = (1.0 - (AMGain * KoverQ) + Ksquared) * normal;
				m_a1 = 1.0 * m_b1;
				m_a2 = (1.0 - KoverQ + Ksquared) * normal;
			}
			else
			{
				normal = 1.0 / (1.0 + (AMGain * KoverQ) + Ksquared);
				m_b0 = (1.0 + KoverQ + Ksquared) * normal;
				m_b1 = 2.0 * (Ksquared - 1.0) * normal;
				m_b2 = (1.0 - KoverQ + Ksquared) * normal;
				m_a1 = 1.0 * m_b1;
				m_a2 = (1.0 - (AMGain * KoverQ) + Ksquared) * normal;
			}
			break;
		case LOWSHELF:
			if (DBGain >= 0.0)
			{
				normal = 1.0 / (1.0 + M_SQRT2 * K + Ksquared);
				m_b0 = (1.0 + sqrt(2.0 * AMGain) * K + AMGain * Ksquared) * normal;
				m_b1 = 2.0 * (AMGain * Ksquared - 1.0) * normal;
				m_b2 = (1.0 - sqrt(2.0 * AMGain) * K + AMGain * Ksquared) * normal;
				m_a1 = 2.0 * (Ksquared - 1.0) * normal;
				m_a2 = (1.0 - M_SQRT2 * K + Ksquared) * normal;
			}
			else
			{
				normal = 1.0 / (1.0 + sqrt(2.0 * AMGain) * K + AMGain * Ksquared);
				m_b0 = (1.0 + M_SQRT2 * K + Ksquared) * normal;
				m_b1 = 2.0 * (Ksquared - 1.0) * normal;
				m_b2 = (1.0 - M_SQRT2 * K + Ksquared) * normal;
				m_a1 = 2.0 * (AMGain * Ksquared - 1.0) * normal;
				m_a2 = (1.0 - sqrt(2.0 * AMGain) * K + AMGain * Ksquared) * normal;
			}
			break;
		case HIGHSHELF:
			if (DBGain >= 0.0)
			{
				normal = 1.0 / (1.0 + M_SQRT2 * K + Ksquared);
				m_b0 = (AMGain + sqrt(2.0 * AMGain) * K + Ksquared) * normal;
				m_b1 = 2.0 * (Ksquared - AMGain) * normal;
				m_b2 = (AMGain - sqrt(2.0 * AMGain) * K + Ksquared) * normal;
				m_a1 = 2.0 * (Ksquared - 1.0) * normal;
				m_a2 = (1.0 - M_SQRT2 * K + Ksquared) * normal;
			}
			else
			{
				normal = 1.0 / (AMGain + sqrt(2.0 * AMGain) * K + Ksquared);
				m_b0 = (1.0 + M_SQRT2 * K + Ksquared) * normal;
				m_b1 = 2.0 * (Ksquared - 1.0) * normal;
				m_b2 = (1.0 - M_SQRT2 * K + Ksquared) * normal;
				m_a1 = 2.0 * (Ksquared - AMGain) * normal;
				m_a2 = (AMGain - sqrt(2.0 * AMGain) * K + Ksquared) * normal;
			}
			break;
		default:
			fatalerror("filter_bidquad_device::recalc() - Invalid filter type!");
			break;
	}
#ifdef FLT_BIQUAD_DEBUG
	logerror("Calculated Parameters:\n");
	logerror( "Gain (dB): %f, (raw): %f\n", DBGain, MGain);
	logerror( "k: %f\n", K);
	logerror( "normal: %f\n", normal);
	logerror("b0: %f\n", m_b0);
	logerror("b1: %f\n", m_b1);
	logerror("b2: %f\n", m_b2);
	logerror("a1: %f\n", m_a1);
	logerror("a2: %f\n", m_a2);
#endif
	// peak and shelf filters do not use gain for the entire signal, only for the peak/shelf portions
	// side note: the first order lowpass and highpass filter analogues technically don't have gain either,
	// but this can be 'faked' by adjusting the bx factors, so we support that anyway, even if it isn't realistic.
	if ( (m_type != PEAK)
		&& (m_type != LOWSHELF)
		&& (m_type != HIGHSHELF) )
	{
		m_b0 *= m_gain;
		m_b1 *= m_gain;
		m_b2 *= m_gain;
#ifdef FLT_BIQUAD_DEBUG
		logerror("b0g: %f\n", m_b0);
		logerror("b1g: %f\n", m_b1);
		logerror("b2g: %f\n", m_b2);
#endif
	}
#ifdef FLT_BIQUAD_DEBUG
	fflush(stderr);
#endif
}

/* Step the filter */
void filter_biquad_device::step()
{
	m_w2 = m_w1;
	m_w1 = m_w0;
	m_w0 = (-m_a1 * m_w1) + (-m_a2 * m_w2) + m_input;
	m_output = (m_b0 * m_w0) + (m_b1 * m_w1) + (m_b2 * m_w2);
}
