// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    dac.h

    Four quadrant multiplying DAC.

    Binary Weighted Resistor Network, R-2R Ladder & PWM

    Binary, Ones Complement, Twos Complement or Sign Magnitude coding

***************************************************************************/

#ifndef MAME_SOUND_DAC_H
#define MAME_SOUND_DAC_H

#pragma once

#include <type_traits>


#define DAC_VREF_POS_INPUT (0)
#define DAC_VREF_NEG_INPUT (1)

class dac_bit_interface
{
public:
	virtual DECLARE_WRITE_LINE_MEMBER(write) = 0;
	virtual DECLARE_WRITE8_MEMBER(data_w) = 0;
};

class dac_byte_interface
{
public:
	virtual void write(unsigned char data) = 0;
	virtual DECLARE_WRITE8_MEMBER(data_w) = 0;
};

class dac_word_interface
{
public:
	virtual void write(unsigned short data) = 0;
	virtual DECLARE_WRITE16_MEMBER(data_w) = 0;
};

template <unsigned bits>
constexpr stream_sample_t dac_multiply(const double vref, const stream_sample_t code)
{
	return (bits > 1) ? ((vref * code) / (1 << (bits))) : (vref * code);
}

template <unsigned bits>
class dac_code
{
protected:
	dac_code(double gain) :
		m_stream(nullptr),
		m_code(0),
		m_gain(gain)
	{
	}

	sound_stream * m_stream;
	stream_sample_t m_code;
	const double m_gain;

	inline void setCode(stream_sample_t code)
	{
		code &= ~(~std::make_unsigned_t<stream_sample_t>(0) << bits);
		if (m_code != code)
		{
			m_stream->update();
			m_code = code;
		}
	}

	virtual void sound_stream_update_tag(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) = 0;
};

template <unsigned bits>
class dac_code_binary : protected dac_code<bits>
{
protected:
	using dac_code<bits>::dac_code;

	virtual void sound_stream_update_tag(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override
	{
		for (int samp = 0; samp < samples; samp++)
		{
			double const vref_pos = inputs[DAC_VREF_POS_INPUT][samp] * this->m_gain;
			double const vref_neg = inputs[DAC_VREF_NEG_INPUT][samp] * this->m_gain;
			stream_sample_t const vout = vref_neg + dac_multiply<bits>(vref_pos - vref_neg, this->m_code);
			outputs[0][samp] = vout;
		}
	}
};

template <unsigned bits>
class dac_code_ones_complement : protected dac_code<bits>
{
protected:
	using dac_code<bits>::dac_code;

	virtual void sound_stream_update_tag(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override
	{
		if (this->m_code & (1 << (bits - 1)))
		{
			for (int samp = 0; samp < samples; samp++)
			{
				double const vref_neg = inputs[DAC_VREF_NEG_INPUT][samp] * this->m_gain;
				stream_sample_t const vout = dac_multiply<bits - 1>(vref_neg, this->m_code ^ ~(~0U << bits));
				outputs[0][samp] = vout;
			}
		}
		else
		{
			for (int samp = 0; samp < samples; samp++)
			{
				double const vref_pos = inputs[DAC_VREF_POS_INPUT][samp] * this->m_gain;
				stream_sample_t const vout = dac_multiply<bits - 1>(vref_pos, this->m_code);
				outputs[0][samp] = vout;
			}
		}
	}
};

template <unsigned bits>
class dac_code_twos_complement : protected dac_code<bits>
{
protected:
	using dac_code<bits>::dac_code;

	virtual void sound_stream_update_tag(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override
	{
		for (int samp = 0; samp < samples; samp++)
		{
			double const vref_pos = inputs[DAC_VREF_POS_INPUT][samp] * this->m_gain;
			double const vref_neg = inputs[DAC_VREF_NEG_INPUT][samp] * this->m_gain;
			stream_sample_t const vout = vref_neg + dac_multiply<bits>(vref_pos - vref_neg, this->m_code ^ (1 << (bits - 1)));
			outputs[0][samp] = vout;
		}
	}
};

template <unsigned bits>
class dac_code_sign_magntitude : protected dac_code<bits>
{
protected:
	using dac_code<bits>::dac_code;

	virtual void sound_stream_update_tag(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override
	{
		if (this->m_code & (1 << (bits - 1)))
		{
			for (int samp = 0; samp < samples; samp++)
			{
				double const vref_neg = inputs[DAC_VREF_NEG_INPUT][samp] * this->m_gain;
				stream_sample_t const vout = dac_multiply<bits - 1>(vref_neg, this->m_code ^ (1 << (bits - 1)));
				outputs[0][samp] = vout;
			}
		}
		else
		{
			for (int samp = 0; samp < samples; samp++)
			{
				double const vref_pos = inputs[DAC_VREF_POS_INPUT][samp] * this->m_gain;
				stream_sample_t const vout = dac_multiply<bits - 1>(vref_pos, this->m_code);
				outputs[0][samp] = vout;
			}
		}
	}
};


template <typename _dac_code>
class dac_device : public device_t,
	public device_sound_interface,
	protected _dac_code
{
protected:
	dac_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, double gain) :
		device_t(mconfig, type, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		_dac_code(gain)
	{
	}

	virtual void device_start() override
	{
		this->m_stream = stream_alloc(2, 1, 48000 * 4);

		save_item(NAME(this->m_code));
	}

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override
	{
		_dac_code::sound_stream_update_tag(stream, inputs, outputs, samples);
	}
};

template <typename dac_interface, typename _dac_code> class dac_generator;

template <typename _dac_code>
class dac_generator<dac_bit_interface, _dac_code> :
	public dac_bit_interface,
	public dac_device<_dac_code>
{
public:
	dac_generator(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, double gain) :
		dac_device<_dac_code>(mconfig, type, tag, owner, clock, gain)
	{
	}

	virtual WRITE_LINE_MEMBER(write) override { this->setCode(state); }
	virtual WRITE8_MEMBER(data_w) override { this->setCode(data); }
};

template <typename _dac_code>
class dac_generator<dac_byte_interface, _dac_code> :
	public dac_byte_interface,
	public dac_device<_dac_code>
{
public:
	dac_generator(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, double gain) :
		dac_device<_dac_code>(mconfig, type, tag, owner, clock, gain)
	{
	}

	virtual void write(unsigned char data) override { this->setCode(data); }
	virtual DECLARE_WRITE8_MEMBER(data_w) override { this->setCode(data); }
};

template <typename _dac_code>
class dac_generator<dac_word_interface, _dac_code> :
	public dac_word_interface,
	public dac_device<_dac_code>
{
public:
	dac_generator(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, double gain) :
		dac_device<_dac_code>(mconfig, type, tag, owner, clock, gain)
	{
	}

	virtual void write(unsigned short data) override { this->setCode(data); }
	virtual DECLARE_WRITE16_MEMBER(data_w) override { this->setCode(data); }
};

constexpr double dac_gain_r2r = 1.0;
constexpr double dac_gain_binary_weighted = 2.0;

#ifndef DAC_GENERATOR_EPILOG
#define DAC_GENERATOR_EPILOG(_dac_type, _dac_class, _dac_description, _dac_shortname)
#endif

#define DAC_GENERATOR(_dac_type, _dac_class, _dac_interface, _dac_coding, _dac_gain, _dac_description, _dac_shortname) \
DECLARE_DEVICE_TYPE(_dac_type, _dac_class) \
class _dac_class : public dac_generator<_dac_interface, _dac_coding> \
{\
public: \
	_dac_class(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) : \
		dac_generator(mconfig, _dac_type, tag, owner, clock, _dac_gain) {} \
}; \
DAC_GENERATOR_EPILOG(_dac_type, _dac_class, _dac_description, _dac_shortname)

// DAC chips
DAC_GENERATOR(AD557, ad557_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "AD557", "ad557")
DAC_GENERATOR(AD7224, ad7224_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "AD7224", "ad7224")
DAC_GENERATOR(AD7521, ad7521_device, dac_word_interface, dac_code_binary<12>, dac_gain_r2r, "AD7521", "ad7521")
DAC_GENERATOR(AD7523, ad7523_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "AD7523", "ad7523")
DAC_GENERATOR(AD7524, ad7524_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "AD7524", "ad7524")
DAC_GENERATOR(AD7528, ad7528_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "AD7528", "ad7528") /// 2 x vin + 2 x vout
DAC_GENERATOR(AD7533, ad7533_device, dac_word_interface, dac_code_binary<10>, dac_gain_r2r, "AD7533", "ad7533")
DAC_GENERATOR(AD7541, ad7541_device, dac_word_interface, dac_code_binary<12>, dac_gain_r2r, "AD7541", "ad7541")
DAC_GENERATOR(AM6012, am6012_device, dac_word_interface, dac_code_binary<12>, dac_gain_r2r, "AM6012", "am6012")
DAC_GENERATOR(DAC08, dac08_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "DAC08", "dac08")
DAC_GENERATOR(DAC0800, dac0800_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "DAC0800", "dac0800")
DAC_GENERATOR(DAC0832, dac0832_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "DAC0832", "dac0832") // should be double-buffered?
DAC_GENERATOR(DAC1200, dac1200_device, dac_word_interface, dac_code_binary<12>, dac_gain_r2r, "DAC1200", "dac1200")
DAC_GENERATOR(MC1408, mc1408_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "MC1408", "mc1408")
DAC_GENERATOR(MC3408, mc3408_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "MC3408", "mc3408")
DAC_GENERATOR(MC3410, mc3410_device, dac_word_interface, dac_code_binary<10>, dac_gain_r2r, "MC3410", "mc3410")
DAC_GENERATOR(MP1210, mp1210_device, dac_word_interface, dac_code_twos_complement<12>, dac_gain_r2r, "MP1210", "mp1210") // also addressable with separate 8-bit and 4-bit input latches
DAC_GENERATOR(PCM54HP, pcm54hp_device, dac_word_interface, dac_code_binary<16>, dac_gain_r2r, "PCM54HP", "pcm54hp")
DAC_GENERATOR(UDA1341TS, uda1341ts_device, dac_word_interface, dac_code_twos_complement<16>, dac_gain_r2r, "UDA1341TS", "uda1341ts") // I2C stereo audio codec
DAC_GENERATOR(ZN425E, zn425e_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "ZN425E", "zn425e")
DAC_GENERATOR(ZN428E, zn428e_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "ZN428E-8", "zn428e")
DAC_GENERATOR(ZN429E, zn429e_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "ZN429E-8", "zn429e")

// DAC circuits/unidentified chips
DAC_GENERATOR(DAC_1BIT, dac_1bit_device, dac_bit_interface, dac_code_binary<1>, 1.0, "1-Bit DAC", "dac")
DAC_GENERATOR(DAC_2BIT_BINARY_WEIGHTED, dac_2bit_binary_weighted_device, dac_byte_interface, dac_code_binary<2>, dac_gain_binary_weighted, "2-Bit Binary Weighted DAC", "dac_2bit_bw")
DAC_GENERATOR(DAC_2BIT_BINARY_WEIGHTED_ONES_COMPLEMENT, dac_2bit_binary_weighted_ones_complement_device, dac_byte_interface, dac_code_ones_complement<2>, dac_gain_binary_weighted, "2-Bit Binary Weighted Ones Complement DAC", "dac_2bit_bw_oc")
DAC_GENERATOR(DAC_2BIT_R2R, dac_2bit_r2r_device, dac_byte_interface, dac_code_binary<2>, dac_gain_r2r, "2-Bit R-2R DAC", "dac_2bit_r2r")
DAC_GENERATOR(DAC_3BIT_BINARY_WEIGHTED, dac_3bit_binary_weighted_device, dac_byte_interface, dac_code_binary<3>, dac_gain_binary_weighted, "3-Bit Binary Weighted DAC", "dac_3bit_bw")
DAC_GENERATOR(DAC_4BIT_BINARY_WEIGHTED, dac_4bit_binary_weighted_device, dac_byte_interface, dac_code_binary<4>, dac_gain_binary_weighted, "4-Bit Binary Weighted DAC", "dac_4bit_bw")
DAC_GENERATOR(DAC_4BIT_BINARY_WEIGHTED_SIGN_MAGNITUDE, dac_4bit_binary_weighted_sign_magnitude_device, dac_byte_interface, dac_code_sign_magntitude<4>, dac_gain_binary_weighted, "4-Bit Binary Weighted Sign Magnitude DAC", "dac_4bit_bw_sm")
DAC_GENERATOR(DAC_4BIT_R2R, dac_4bit_r2r_device, dac_byte_interface, dac_code_binary<4>, dac_gain_r2r, "4-Bit R-2R DAC", "dac_4bit_r2r")
DAC_GENERATOR(DAC_6BIT_BINARY_WEIGHTED, dac_6bit_binary_weighted_device, dac_byte_interface, dac_code_binary<6>, dac_gain_binary_weighted, "6-Bit Binary Weighted DAC", "dac_6bit_bw")
DAC_GENERATOR(DAC_6BIT_R2R, dac_6bit_r2r_device, dac_byte_interface, dac_code_binary<6>, dac_gain_r2r, "6-Bit R-2R DAC", "dac_6bit_r2r")
DAC_GENERATOR(DAC_8BIT_BINARY_WEIGHTED, dac_binary_weighted_8bit_device, dac_byte_interface, dac_code_binary<8>, dac_gain_binary_weighted, "8-Bit Binary Weighted DAC", "dac_8bit_bw")
DAC_GENERATOR(DAC_8BIT_PWM, dac_8bit_pwm_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "8-Bit PWM DAC", "dac_8bit_pwm")
DAC_GENERATOR(DAC_8BIT_R2R, dac_8bit_r2r_device, dac_byte_interface, dac_code_binary<8>, dac_gain_r2r, "8-Bit R-2R DAC", "dac_8bit_r2r")
DAC_GENERATOR(DAC_8BIT_R2R_TWOS_COMPLEMENT, dac_8bit_r2r_twos_complement_device, dac_byte_interface, dac_code_twos_complement<8>, dac_gain_r2r, "8-Bit R-2R Twos Complement DAC", "dac_8bit_r2r_tc")
DAC_GENERATOR(DAC_10BIT_R2R, dac_10bit_r2r_device, dac_word_interface, dac_code_binary<10>, dac_gain_r2r, "10-Bit R-2R DAC", "dac_10bit_r2r")
DAC_GENERATOR(DAC_12BIT_R2R, dac_12bit_r2r_device, dac_word_interface, dac_code_binary<12>, dac_gain_r2r, "12-Bit R-2R DAC", "dac_12bit_r2r")
DAC_GENERATOR(DAC_12BIT_R2R_TWOS_COMPLEMENT, dac_12bit_r2r_twos_complement_device, dac_word_interface, dac_code_twos_complement<12>, dac_gain_r2r, "12-Bit R-2R Twos Complement DAC", "dac_12bit_r2r_tc")
DAC_GENERATOR(DAC_16BIT_R2R, dac_16bit_r2r_device, dac_word_interface, dac_code_binary<16>, dac_gain_r2r, "16-Bit R-2R DAC", "dac_16bit_r2r")
DAC_GENERATOR(DAC_16BIT_R2R_TWOS_COMPLEMENT, dac_16bit_r2r_twos_complement_device, dac_word_interface, dac_code_twos_complement<16>, dac_gain_r2r, "16-Bit R-2R Twos Complement DAC", "dac_16bit_r2r_tc")

#undef DAC_GENERATOR
#undef DAC_GENERATOR_EPILOG

#endif // MAME_SOUND_DAC_H
