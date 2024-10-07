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


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define DAC_INPUT_RANGE_HI (0)
#define DAC_INPUT_RANGE_LO (1)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

constexpr stream_buffer::sample_t dac_gain_r2r = 1.0;
constexpr stream_buffer::sample_t dac_gain_bw = 2.0;


// ======================> dac_mapper_callback

using dac_mapper_callback = stream_buffer::sample_t (*)(u32 input, u8 bits);

stream_buffer::sample_t dac_mapper_unsigned(u32 input, u8 bits);
stream_buffer::sample_t dac_mapper_signed(u32 input, u8 bits);
stream_buffer::sample_t dac_mapper_ones_complement(u32 input, u8 bits);


// ======================> dac_bit_interface

class dac_bit_interface
{
public:
	virtual void write(int state) = 0;
	virtual void data_w(u8 data) = 0;
};


// ======================> dac_byte_interface

class dac_byte_interface
{
public:
	virtual void write(u8 data) = 0;
	virtual void data_w(u8 data) = 0;
};


// ======================> dac_word_interface

class dac_word_interface
{
public:
	virtual void write(u16 data) = 0;
	virtual void data_w(u16) = 0;
};


// ======================> dac_device_base

class dac_device_base : public device_t, public device_sound_interface
{
protected:
	// constructor
	dac_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 bits, dac_mapper_callback mapper, stream_buffer::sample_t gain);

	// device startup
	virtual void device_start() override ATTR_COLD;

	// stream generation
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// set the current value
	void set_value(u32 value)
	{
		m_stream->update();
		m_curval = m_value_map[value & (m_value_map.size() - 1)];
	}

public:
	// configuration: default output range is -1..1 for all cases except
	// for 1-bit DACs, which default to 0..1
	dac_device_base &set_output_range(stream_buffer::sample_t range_min, stream_buffer::sample_t range_max)
	{
		m_range_min = range_min;
		m_range_max = range_max;
		return *this;
	}
	dac_device_base &set_output_range(stream_buffer::sample_t vref) { return set_output_range(-vref, vref); }

private:
	// internal state
	sound_stream *m_stream;
	stream_buffer::sample_t m_curval;
	std::vector<stream_buffer::sample_t> m_value_map;

	// configuration state
	u8 const m_bits;
	dac_mapper_callback const m_mapper;
	stream_buffer::sample_t const m_gain;
	stream_buffer::sample_t m_range_min;
	stream_buffer::sample_t m_range_max;
};


// ======================> dac_bit_device_base

class dac_bit_device_base : public dac_device_base, public dac_bit_interface
{
protected:
	dac_bit_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 bits, dac_mapper_callback mapper, stream_buffer::sample_t gain) :
		dac_device_base(mconfig, type, tag, owner, clock, bits, mapper, gain)
	{
	}

public:
	virtual void write(int state) override { this->set_value(state); }
	virtual void data_w(u8 data) override { this->set_value(data); }
};


// ======================> dac_byte_device_base

class dac_byte_device_base : public dac_device_base, public dac_byte_interface
{
protected:
	dac_byte_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 bits, dac_mapper_callback mapper, stream_buffer::sample_t gain) :
		dac_device_base(mconfig, type, tag, owner, clock, bits, mapper, gain)
	{
	}

public:
	virtual void write(u8 data) override { this->set_value(data); }
	virtual void data_w(u8 data) override { this->set_value(data); }
};


// ======================> dac_word_device_base

class dac_word_device_base : public dac_device_base, public dac_word_interface
{
protected:
	dac_word_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 bits, dac_mapper_callback mapper, stream_buffer::sample_t gain) :
		dac_device_base(mconfig, type, tag, owner, clock, bits, mapper, gain)
	{
	}

public:
	virtual void write(u16 data) override { this->set_value(data); }
	virtual void data_w(u16 data) override { this->set_value(data); }
};



//**************************************************************************
//  DAC GENERATORS
//**************************************************************************

// epilog only defined in dac.cpp
#ifndef DAC_GENERATOR_EPILOG
#define DAC_GENERATOR_EPILOG(_dac_type, _dac_class, _dac_description, _dac_shortname)
#endif

#define DAC_GENERATOR(_dac_type, _dac_class, _dac_base_class, _dac_mapper, _dac_bits, _dac_gain, _dac_description, _dac_shortname) \
DECLARE_DEVICE_TYPE(_dac_type, _dac_class) \
class _dac_class : public _dac_base_class \
{\
public: \
	_dac_class(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0) : \
		_dac_base_class(mconfig, _dac_type, tag, owner, clock, _dac_bits, _dac_mapper, _dac_gain) {} \
}; \
DAC_GENERATOR_EPILOG(_dac_type, _dac_class, _dac_description, _dac_shortname)

// DAC chips
DAC_GENERATOR(AD557,     ad557_device,     dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "AD557 DAC",     "ad557")
DAC_GENERATOR(AD558,     ad558_device,     dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "AD558 DAC",     "ad558")
DAC_GENERATOR(AD7224,    ad7224_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "AD7224 DAC",    "ad7224")
DAC_GENERATOR(AD7520,    ad7520_device,    dac_word_device_base, dac_mapper_unsigned, 10, dac_gain_r2r, "AD7520 DAC",    "ad7520")
DAC_GENERATOR(AD7521,    ad7521_device,    dac_word_device_base, dac_mapper_unsigned, 12, dac_gain_r2r, "AD7521 DAC",    "ad7521")
DAC_GENERATOR(AD7523,    ad7523_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "AD7523 DAC",    "ad7523")
DAC_GENERATOR(AD7524,    ad7524_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "AD7524 DAC",    "ad7524")
DAC_GENERATOR(AD7528,    ad7528_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "AD7528 DAC",    "ad7528") // 2 x vin + 2 x vout
DAC_GENERATOR(AD7533,    ad7533_device,    dac_word_device_base, dac_mapper_unsigned, 10, dac_gain_r2r, "AD7533 DAC",    "ad7533")
DAC_GENERATOR(AD7541,    ad7541_device,    dac_word_device_base, dac_mapper_unsigned, 12, dac_gain_r2r, "AD7541 DAC",    "ad7541")
DAC_GENERATOR(AM6012,    am6012_device,    dac_word_device_base, dac_mapper_unsigned, 12, dac_gain_r2r, "AM6012 DAC",    "am6012")
DAC_GENERATOR(DAC08,     dac08_device,     dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "DAC08 DAC",     "dac08")
DAC_GENERATOR(DAC0800,   dac0800_device,   dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "DAC0800 DAC",   "dac0800")
DAC_GENERATOR(DAC0832,   dac0832_device,   dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "DAC0832 DAC",   "dac0832") // should be double-buffered?
DAC_GENERATOR(DAC1200,   dac1200_device,   dac_word_device_base, dac_mapper_unsigned, 12, dac_gain_r2r, "DAC1200 DAC",   "dac1200")
DAC_GENERATOR(MC1408,    mc1408_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "MC1408 DAC",    "mc1408")
DAC_GENERATOR(MC3408,    mc3408_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "MC3408 DAC",    "mc3408")
DAC_GENERATOR(MC3410,    mc3410_device,    dac_word_device_base, dac_mapper_unsigned, 10, dac_gain_r2r, "MC3410 DAC",    "mc3410")
DAC_GENERATOR(MP1210,    mp1210_device,    dac_word_device_base, dac_mapper_signed,   12, dac_gain_r2r, "MP1210 DAC",    "mp1210") // also addressable with separate 8-bit and 4-bit input latches
DAC_GENERATOR(PCM54HP,   pcm54hp_device,   dac_word_device_base, dac_mapper_unsigned, 16, dac_gain_r2r, "PCM54HP DAC",   "pcm54hp")
DAC_GENERATOR(UDA1341TS, uda1341ts_device, dac_word_device_base, dac_mapper_signed,   16, dac_gain_r2r, "UDA1341TS DAC", "uda1341ts") // I2C stereo audio codec
DAC_GENERATOR(ZN425E,    zn425e_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "ZN425E DAC",    "zn425e")
DAC_GENERATOR(ZN426E,    zn426e_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "ZN426E-8 DAC",  "zn426e")
DAC_GENERATOR(ZN428E,    zn428e_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "ZN428E-8 DAC",  "zn428e")
DAC_GENERATOR(ZN429E,    zn429e_device,    dac_byte_device_base, dac_mapper_unsigned,  8, dac_gain_r2r, "ZN429E-8 DAC",  "zn429e")

// DAC circuits/unidentified chips
DAC_GENERATOR(DAC_1BIT,                      dac_1bit_device,                      dac_bit_device_base,  dac_mapper_unsigned,        1, 1.0,          "1-Bit DAC",                       "dac")
DAC_GENERATOR(DAC_2BIT_BINARY_WEIGHTED,      dac_2bit_binary_weighted_device,      dac_byte_device_base, dac_mapper_unsigned,        2, dac_gain_bw,  "2-Bit Binary Weighted DAC",       "dac_2bit_bw")
DAC_GENERATOR(DAC_2BIT_R2R,                  dac_2bit_r2r_device,                  dac_byte_device_base, dac_mapper_unsigned,        2, dac_gain_r2r, "2-Bit R-2R DAC",                  "dac_2bit_r2r")
DAC_GENERATOR(DAC_2BIT_ONES_COMPLEMENT,      dac_2bit_ones_complement_device,      dac_byte_device_base, dac_mapper_ones_complement, 2, 1.0,          "2-Bit Ones Complement DAC",       "dac_2bit_oc")
DAC_GENERATOR(DAC_3BIT_BINARY_WEIGHTED,      dac_3bit_binary_weighted_device,      dac_byte_device_base, dac_mapper_unsigned,        3, dac_gain_bw,  "3-Bit Binary Weighted DAC",       "dac_3bit_bw")
DAC_GENERATOR(DAC_3BIT_R2R,                  dac_3bit_r2r_device,                  dac_byte_device_base, dac_mapper_unsigned,        3, dac_gain_r2r, "3-Bit R-2R DAC",                  "dac_3bit_r2r")
DAC_GENERATOR(DAC_4BIT_BINARY_WEIGHTED,      dac_4bit_binary_weighted_device,      dac_byte_device_base, dac_mapper_unsigned,        4, dac_gain_bw,  "4-Bit Binary Weighted DAC",       "dac_4bit_bw")
DAC_GENERATOR(DAC_4BIT_R2R,                  dac_4bit_r2r_device,                  dac_byte_device_base, dac_mapper_unsigned,        4, dac_gain_r2r, "4-Bit R-2R DAC",                  "dac_4bit_r2r")
DAC_GENERATOR(DAC_5BIT_BINARY_WEIGHTED,      dac_5bit_binary_weighted_device,      dac_byte_device_base, dac_mapper_unsigned,        5, dac_gain_bw,  "5-Bit Binary Weighted DAC",       "dac_5bit_bw")
DAC_GENERATOR(DAC_5BIT_R2R,                  dac_5bit_r2r_device,                  dac_byte_device_base, dac_mapper_unsigned,        5, dac_gain_r2r, "5-Bit R-2R DAC",                  "dac_5bit_r2r")
DAC_GENERATOR(DAC_6BIT_BINARY_WEIGHTED,      dac_6bit_binary_weighted_device,      dac_byte_device_base, dac_mapper_unsigned,        6, dac_gain_bw,  "6-Bit Binary Weighted DAC",       "dac_6bit_bw")
DAC_GENERATOR(DAC_6BIT_R2R,                  dac_6bit_r2r_device,                  dac_byte_device_base, dac_mapper_unsigned,        6, dac_gain_r2r, "6-Bit R-2R DAC",                  "dac_6bit_r2r")
DAC_GENERATOR(DAC_8BIT_BINARY_WEIGHTED,      dac_8bit_binary_weighted_device,      dac_byte_device_base, dac_mapper_unsigned,        8, dac_gain_bw,  "8-Bit Binary Weighted DAC",       "dac_8bit_bw")
DAC_GENERATOR(DAC_8BIT_PWM,                  dac_8bit_pwm_device,                  dac_byte_device_base, dac_mapper_unsigned,        8, dac_gain_r2r, "8-Bit PWM DAC",                   "dac_8bit_pwm")
DAC_GENERATOR(DAC_8BIT_R2R,                  dac_8bit_r2r_device,                  dac_byte_device_base, dac_mapper_unsigned,        8, dac_gain_r2r, "8-Bit R-2R DAC",                  "dac_8bit_r2r")
DAC_GENERATOR(DAC_8BIT_R2R_TWOS_COMPLEMENT,  dac_8bit_r2r_twos_complement_device,  dac_byte_device_base, dac_mapper_signed,          8, dac_gain_r2r, "8-Bit R-2R Twos Complement DAC",  "dac_8bit_r2r_tc")
DAC_GENERATOR(DAC_10BIT_R2R,                 dac_10bit_r2r_device,                 dac_word_device_base, dac_mapper_unsigned,       10, dac_gain_r2r, "10-Bit R-2R DAC",                 "dac_10bit_r2r")
DAC_GENERATOR(DAC_12BIT_R2R,                 dac_12bit_r2r_device,                 dac_word_device_base, dac_mapper_unsigned,       12, dac_gain_r2r, "12-Bit R-2R DAC",                 "dac_12bit_r2r")
DAC_GENERATOR(DAC_12BIT_R2R_TWOS_COMPLEMENT, dac_12bit_r2r_twos_complement_device, dac_word_device_base, dac_mapper_signed,         12, dac_gain_r2r, "12-Bit R-2R Twos Complement DAC", "dac_12bit_r2r_tc")
DAC_GENERATOR(DAC_16BIT_R2R,                 dac_16bit_r2r_device,                 dac_word_device_base, dac_mapper_unsigned,       16, dac_gain_r2r, "16-Bit R-2R DAC",                 "dac_16bit_r2r")
DAC_GENERATOR(DAC_16BIT_R2R_TWOS_COMPLEMENT, dac_16bit_r2r_twos_complement_device, dac_word_device_base, dac_mapper_signed,         16, dac_gain_r2r, "16-Bit R-2R Twos Complement DAC", "dac_16bit_r2r_tc")


#undef DAC_GENERATOR
#undef DAC_GENERATOR_EPILOG

#endif // MAME_SOUND_DAC_H
