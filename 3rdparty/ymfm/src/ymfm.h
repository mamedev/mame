// BSD 3-Clause License
//
// Copyright (c) 2021, Aaron Giles
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef YMFM_H
#define YMFM_H

#pragma once

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
 #define _CRT_SECURE_NO_WARNINGS
#endif

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace ymfm
{

//*********************************************************
//  DEBUGGING
//*********************************************************

class debug
{
public:
	// masks to help isolate specific channels
	static constexpr uint32_t GLOBAL_FM_CHANNEL_MASK = 0xffffffff;
	static constexpr uint32_t GLOBAL_ADPCM_A_CHANNEL_MASK = 0xffffffff;
	static constexpr uint32_t GLOBAL_ADPCM_B_CHANNEL_MASK = 0xffffffff;
	static constexpr uint32_t GLOBAL_PCM_CHANNEL_MASK = 0xffffffff;

	// types of logging
	static constexpr bool LOG_FM_WRITES = false;
	static constexpr bool LOG_KEYON_EVENTS = false;
	static constexpr bool LOG_UNEXPECTED_READ_WRITES = false;

	// helpers to write based on the log type
	template<typename... Params> static void log_fm_write(Params &&... args) { if (LOG_FM_WRITES) log(args...); }
	template<typename... Params> static void log_keyon(Params &&... args) { if (LOG_KEYON_EVENTS) log(args...); }
	template<typename... Params> static void log_unexpected_read_write(Params &&... args) { if  (LOG_UNEXPECTED_READ_WRITES) log(args...); }

	// downstream helper to output log data; defaults to printf
	template<typename... Params> static void log(Params &&... args) { printf(args...); }
};



//*********************************************************
//  GLOBAL HELPERS
//*********************************************************

//-------------------------------------------------
//  bitfield - extract a bitfield from the given
//  value, starting at bit 'start' for a length of
//  'length' bits
//-------------------------------------------------

inline uint32_t bitfield(uint32_t value, int start, int length = 1)
{
	return (value >> start) & ((1 << length) - 1);
}


//-------------------------------------------------
//  clamp - clamp between the minimum and maximum
//  values provided
//-------------------------------------------------

inline int32_t clamp(int32_t value, int32_t minval, int32_t maxval)
{
	if (value < minval)
		return minval;
	if (value > maxval)
		return maxval;
	return value;
}


//-------------------------------------------------
//  count_leading_zeros - return the number of
//  leading zeros in a 32-bit value; CPU-optimized
//  versions for various architectures are included
//  below
//-------------------------------------------------

#if defined(__GNUC__)

inline uint8_t count_leading_zeros(uint32_t value)
{
	if (value == 0)
		return 32;
	return __builtin_clz(value);
}

#elif defined(_MSC_VER)

inline uint8_t count_leading_zeros(uint32_t value)
{
	unsigned long index;
	return _BitScanReverse(&index, value) ? uint8_t(31U - index) : 32U;
}

#else

inline uint8_t count_leading_zeros(uint32_t value)
{
	if (value == 0)
		return 32;
	uint8_t count;
	for (count = 0; int32_t(value) >= 0; count++)
		value <<= 1;
	return count;
}

#endif


// Many of the Yamaha FM chips emit a floating-point value, which is sent to
// a DAC for processing. The exact format of this floating-point value is
// documented below. This description only makes sense if the "internal"
// format treats sign as 1=positive and 0=negative, so the helpers below
// presume that.
//
// Internal OPx data      16-bit signed data     Exp Sign Mantissa
// =================      =================      === ==== ========
// 1 1xxxxxxxx------  ->  0 1xxxxxxxx------  ->  111   1  1xxxxxxx
// 1 01xxxxxxxx-----  ->  0 01xxxxxxxx-----  ->  110   1  1xxxxxxx
// 1 001xxxxxxxx----  ->  0 001xxxxxxxx----  ->  101   1  1xxxxxxx
// 1 0001xxxxxxxx---  ->  0 0001xxxxxxxx---  ->  100   1  1xxxxxxx
// 1 00001xxxxxxxx--  ->  0 00001xxxxxxxx--  ->  011   1  1xxxxxxx
// 1 000001xxxxxxxx-  ->  0 000001xxxxxxxx-  ->  010   1  1xxxxxxx
// 1 000000xxxxxxxxx  ->  0 000000xxxxxxxxx  ->  001   1  xxxxxxxx
// 0 111111xxxxxxxxx  ->  1 111111xxxxxxxxx  ->  001   0  xxxxxxxx
// 0 111110xxxxxxxx-  ->  1 111110xxxxxxxx-  ->  010   0  0xxxxxxx
// 0 11110xxxxxxxx--  ->  1 11110xxxxxxxx--  ->  011   0  0xxxxxxx
// 0 1110xxxxxxxx---  ->  1 1110xxxxxxxx---  ->  100   0  0xxxxxxx
// 0 110xxxxxxxx----  ->  1 110xxxxxxxx----  ->  101   0  0xxxxxxx
// 0 10xxxxxxxx-----  ->  1 10xxxxxxxx-----  ->  110   0  0xxxxxxx
// 0 0xxxxxxxx------  ->  1 0xxxxxxxx------  ->  111   0  0xxxxxxx

//-------------------------------------------------
//  encode_fp - given a 32-bit signed input value
//  convert it to a signed 3.10 floating-point
//  value
//-------------------------------------------------

inline int16_t encode_fp(int32_t value)
{
	// handle overflows first
	if (value < -32768)
		return (7 << 10) | 0x000;
	if (value > 32767)
		return (7 << 10) | 0x3ff;

	// we need to count the number of leading sign bits after the sign
	// we can use count_leading_zeros if we invert negative values
	int32_t scanvalue = value ^ (int32_t(value) >> 31);

	// exponent is related to the number of leading bits starting from bit 14
	int exponent = 7 - count_leading_zeros(scanvalue << 17);

	// smallest exponent value allowed is 1
	exponent = std::max(exponent, 1);

	// mantissa
	int32_t mantissa = value >> (exponent - 1);

	// assemble into final form, inverting the sign
	return ((exponent << 10) | (mantissa & 0x3ff)) ^ 0x200;
}


//-------------------------------------------------
//  decode_fp - given a 3.10 floating-point value,
//  convert it to a signed 16-bit value
//-------------------------------------------------

inline int16_t decode_fp(int16_t value)
{
	// invert the sign and the exponent
	value ^= 0x1e00;

	// shift mantissa up to 16 bits then apply inverted exponent
	return int16_t(value << 6) >> bitfield(value, 10, 3);
}


//-------------------------------------------------
//  roundtrip_fp - compute the result of a round
//  trip through the encode/decode process above
//-------------------------------------------------

inline int16_t roundtrip_fp(int32_t value)
{
	// handle overflows first
	if (value < -32768)
		return -32768;
	if (value > 32767)
		return 32767;

	// we need to count the number of leading sign bits after the sign
	// we can use count_leading_zeros if we invert negative values
	int32_t scanvalue = value ^ (int32_t(value) >> 31);

	// exponent is related to the number of leading bits starting from bit 14
	int exponent = 7 - count_leading_zeros(scanvalue << 17);

	// smallest exponent value allowed is 1
	exponent = std::max(exponent, 1);

	// apply the shift back and forth to zero out bits that are lost
	exponent -= 1;
    int32_t mask = (1 << exponent) - 1;
	return value & ~mask;
}



//*********************************************************
//  HELPER CLASSES
//*********************************************************

// various envelope states
enum envelope_state : uint32_t
{
	EG_DEPRESS = 0,		// OPLL only; set EG_HAS_DEPRESS to enable
	EG_ATTACK = 1,
	EG_DECAY = 2,
	EG_SUSTAIN = 3,
	EG_RELEASE = 4,
	EG_REVERB = 5,		// OPQ/OPZ only; set EG_HAS_REVERB to enable
	EG_STATES = 6
};

// external I/O access classes
enum access_class : uint32_t
{
	ACCESS_IO = 0,
	ACCESS_ADPCM_A,
	ACCESS_ADPCM_B,
	ACCESS_PCM,
	ACCESS_CLASSES
};



//*********************************************************
//  HELPER CLASSES
//*********************************************************

// ======================> ymfm_output

// struct containing an array of output values
template<int NumOutputs>
struct ymfm_output
{
	// clear all outputs to 0
	ymfm_output &clear()
	{
		for (uint32_t index = 0; index < NumOutputs; index++)
			data[index] = 0;
		return *this;
	}

	// clamp all outputs to a 16-bit signed value
	ymfm_output &clamp16()
	{
		for (uint32_t index = 0; index < NumOutputs; index++)
			data[index] = clamp(data[index], -32768, 32767);
		return *this;
	}

	// run each output value through the floating-point processor
	ymfm_output &roundtrip_fp()
	{
		for (uint32_t index = 0; index < NumOutputs; index++)
			data[index] = ymfm::roundtrip_fp(data[index]);
		return *this;
	}

	// internal state
	int32_t data[NumOutputs];
};


// ======================> ymfm_wavfile

// this class is a debugging helper that accumulates data and writes it to wav files
template<int Channels>
class ymfm_wavfile
{
public:
	// construction
	ymfm_wavfile(uint32_t samplerate = 44100) :
		m_samplerate(samplerate)
	{
	}

	// configuration
	ymfm_wavfile &set_index(uint32_t index) { m_index = index; return *this; }
	ymfm_wavfile &set_samplerate(uint32_t samplerate) { m_samplerate = samplerate; return *this; }

	// destruction
	~ymfm_wavfile()
	{
		if (!m_buffer.empty())
		{
			// create file
			char name[20];
			snprintf(&name[0], sizeof(name), "wavlog-%02d.wav", m_index);
			FILE *out = fopen(name, "wb");

			// make the wav file header
			uint8_t header[44];
			memcpy(&header[0], "RIFF", 4);
			*(uint32_t *)&header[4] = m_buffer.size() * 2 + 44 - 8;
			memcpy(&header[8], "WAVE", 4);
			memcpy(&header[12], "fmt ", 4);
			*(uint32_t *)&header[16] = 16;
			*(uint16_t *)&header[20] = 1;
			*(uint16_t *)&header[22] = Channels;
			*(uint32_t *)&header[24] = m_samplerate;
			*(uint32_t *)&header[28] = m_samplerate * 2 * Channels;
			*(uint16_t *)&header[32] = 2 * Channels;
			*(uint16_t *)&header[34] = 16;
			memcpy(&header[36], "data", 4);
			*(uint32_t *)&header[40] = m_buffer.size() * 2 + 44 - 44;

			// write header then data
			fwrite(&header[0], 1, sizeof(header), out);
			fwrite(&m_buffer[0], 2, m_buffer.size(), out);
			fclose(out);
		}
	}

	// add data to the file
	template<int Outputs>
	void add(ymfm_output<Outputs> output)
	{
		int16_t sum[Channels] = { 0 };
		for (int index = 0; index < Outputs; index++)
			sum[index % Channels] += output.data[index];
		for (int index = 0; index < Channels; index++)
			m_buffer.push_back(sum[index]);
	}

	// add data to the file, using a reference
	template<int Outputs>
	void add(ymfm_output<Outputs> output, ymfm_output<Outputs> const &ref)
	{
		int16_t sum[Channels] = { 0 };
		for (int index = 0; index < Outputs; index++)
			sum[index % Channels] += output.data[index] - ref.data[index];
		for (int index = 0; index < Channels; index++)
			m_buffer.push_back(sum[index]);
	}

private:
	// internal state
	uint32_t m_index;
	uint32_t m_samplerate;
	std::vector<int16_t> m_buffer;
};


// ======================> ymfm_saved_state

// this class contains a managed vector of bytes that is used to save and
// restore state
class ymfm_saved_state
{
public:
	// construction
	ymfm_saved_state(std::vector<uint8_t> &buffer, bool saving) :
		m_buffer(buffer),
		m_offset(saving ? -1 : 0)
	{
		if (saving)
			buffer.resize(0);
	}

	// are we saving or restoring?
	bool saving() const { return (m_offset < 0); }

	// generic save/restore
	template<typename DataType>
	void save_restore(DataType &data)
	{
		if (saving())
			save(data);
		else
			restore(data);
	}

public:
	// save data to the buffer
	void save(bool &data) { write(data ? 1 : 0); }
	void save(int8_t &data) { write(data); }
	void save(uint8_t &data) { write(data); }
	void save(int16_t &data) { write(uint8_t(data)).write(data >> 8); }
	void save(uint16_t &data) { write(uint8_t(data)).write(data >> 8); }
	void save(int32_t &data) { write(data).write(data >> 8).write(data >> 16).write(data >> 24); }
	void save(uint32_t &data) { write(data).write(data >> 8).write(data >> 16).write(data >> 24); }
	void save(envelope_state &data) { write(uint8_t(data)); }
	template<typename DataType, int Count>
	void save(DataType (&data)[Count]) { for (uint32_t index = 0; index < Count; index++) save(data[index]); }

	// restore data from the buffer
	void restore(bool &data) { data = read() ? true : false; }
	void restore(int8_t &data) { data = read(); }
	void restore(uint8_t &data) { data = read(); }
	void restore(int16_t &data) { data = read(); data |= read() << 8; }
	void restore(uint16_t &data) { data = read(); data |= read() << 8; }
	void restore(int32_t &data) { data = read(); data |= read() << 8; data |= read() << 16; data |= read() << 24; }
	void restore(uint32_t &data) { data = read(); data |= read() << 8; data |= read() << 16; data |= read() << 24; }
	void restore(envelope_state &data) { data = envelope_state(read()); }
	template<typename DataType, int Count>
	void restore(DataType (&data)[Count]) { for (uint32_t index = 0; index < Count; index++) restore(data[index]); }

	// internal helper
	ymfm_saved_state &write(uint8_t data) { m_buffer.push_back(data); return *this; }
	uint8_t read() { return (m_offset < int32_t(m_buffer.size())) ? m_buffer[m_offset++] : 0; }

	// internal state
	std::vector<uint8_t> &m_buffer;
	int32_t m_offset;
};



//*********************************************************
//  INTERFACE CLASSES
//*********************************************************

// ======================> ymfm_engine_callbacks

// this class represents functions in the engine that the ymfm_interface
// needs to be able to call; it is represented here as a separate interface
// that is independent of the actual engine implementation
class ymfm_engine_callbacks
{
public:
	virtual ~ymfm_engine_callbacks() = default;

	// timer callback; called by the interface when a timer fires
	virtual void engine_timer_expired(uint32_t tnum) = 0;

	// check interrupts; called by the interface after synchronization
	virtual void engine_check_interrupts() = 0;

	// mode register write; called by the interface after synchronization
	virtual void engine_mode_write(uint8_t data) = 0;
};


// ======================> ymfm_interface

// this class represents the interface between the fm_engine and the outside
// world; it provides hooks for timers, synchronization, and I/O
class ymfm_interface
{
	// the engine is our friend
	template<typename RegisterType> friend class fm_engine_base;

public:
	virtual ~ymfm_interface() = default;

	// the following functions must be implemented by any derived classes; the
	// default implementations are sufficient for some minimal operation, but will
	// likely need to be overridden to integrate with the outside world; they are
	// all prefixed with ymfm_ to reduce the likelihood of namespace collisions

	//
	// timing and synchronizaton
	//

	// the chip implementation calls this when a write happens to the mode
	// register, which could affect timers and interrupts; our responsibility
	// is to ensure the system is up to date before calling the engine's
	// engine_mode_write() method
	virtual void ymfm_sync_mode_write(uint8_t data) { m_engine->engine_mode_write(data); }

	// the chip implementation calls this when the chip's status has changed,
	// which may affect the interrupt state; our responsibility is to ensure
	// the system is up to date before calling the engine's
	// engine_check_interrupts() method
	virtual void ymfm_sync_check_interrupts() { m_engine->engine_check_interrupts(); }

	// the chip implementation calls this when one of the two internal timers
	// has changed state; our responsibility is to arrange to call the engine's
	// engine_timer_expired() method after the provided number of clocks; if
	// duration_in_clocks is negative, we should cancel any outstanding timers
	virtual void ymfm_set_timer(uint32_t tnum, int32_t duration_in_clocks) { }

	// the chip implementation calls this to indicate that the chip should be
	// considered in a busy state until the given number of clocks has passed;
	// our responsibility is to compute and remember the ending time based on
	// the chip's clock for later checking
	virtual void ymfm_set_busy_end(uint32_t clocks) { }

	// the chip implementation calls this to see if the chip is still currently
	// is a busy state, as specified by a previous call to ymfm_set_busy_end();
	// our responsibility is to compare the current time against the previously
	// noted busy end time and return true if we haven't yet passed it
	virtual bool ymfm_is_busy() { return false; }

	//
	// I/O functions
	//

	// the chip implementation calls this when the state of the IRQ signal has
	// changed due to a status change; our responsibility is to respond as
	// needed to the change in IRQ state, signaling any consumers
	virtual void ymfm_update_irq(bool asserted) { }

	// the chip implementation calls this whenever data is read from outside
	// of the chip; our responsibility is to provide the data requested
	virtual uint8_t ymfm_external_read(access_class type, uint32_t address) { return 0; }

	// the chip implementation calls this whenever data is written outside
	// of the chip; our responsibility is to pass the written data on to any consumers
	virtual void ymfm_external_write(access_class type, uint32_t address, uint8_t data) { }

protected:
	// pointer to engine callbacks -- this is set directly by the engine at
	// construction time
	ymfm_engine_callbacks *m_engine;
};

}

#endif // YMFM_H
