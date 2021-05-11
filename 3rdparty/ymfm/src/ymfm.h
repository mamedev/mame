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

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace ymfm
{

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



//*********************************************************
//  HELPER CLASSES
//*********************************************************

// forward declarations
enum envelope_state : uint32_t;
int16_t roundtrip_fp(int32_t value);


// ======================> ymfm_output

// struct containing output values
template<int NumOutputs>
struct ymfm_output
{
	ymfm_output &clear() { return init(0); }
	ymfm_output &init(int32_t value) { for (int index = 0; index < NumOutputs; index++) data[index] = value; return *this; }
	ymfm_output &clamp16() { for (int index = 0; index < NumOutputs; index++) data[index] = std::clamp(data[index], -32768, 32767); return *this; }
	ymfm_output &roundtrip_fp() { for (int index = 0; index < NumOutputs; index++) data[index] = ymfm::roundtrip_fp(data[index]); return *this; }
	int32_t data[NumOutputs];
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
	void save(int16_t &data) { write(data).write(data >> 8); }
	void save(uint16_t &data) { write(data).write(data >> 8); }
	void save(int32_t &data) { write(data).write(data >> 8).write(data >> 16).write(data >> 24); }
	void save(uint32_t &data) { write(data).write(data >> 8).write(data >> 16).write(data >> 24); }
	void save(envelope_state &data) { write(uint8_t(data)); }
	template<typename DataType, int Count>
	void save(DataType (&data)[Count]) { for (int index = 0; index < Count; index++) save(data[index]); }

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
	void restore(DataType (&data)[Count]) { for (int index = 0; index < Count; index++) restore(data[index]); }

	// internal helper
	ymfm_saved_state &write(uint8_t data) { m_buffer.push_back(data); return *this; }
	uint8_t read() { return (m_offset < m_buffer.size()) ? m_buffer[m_offset++] : 0; }

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
	// logging helper
	template<typename... Params>
	void log(Params &&... args)
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer), std::forward<Params>(args)...);
		buffer[sizeof(buffer) - 1] = 0;
		ymfm_log(buffer);
	}

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

	// the chip implementation calls this whenever the internal clock prescaler
	// changes; our responsibility is to adjust our clocking of the chip in
	// response to produce the correct output rate
	virtual void ymfm_prescale_changed() { }

	//
	// I/O functions
	//

	// the chip implementation calls this when the state of the IRQ signal has
	// changed due to a status change; our responsibility is to respond as
	// needed to the change in IRQ state, signaling any consumers
	virtual void ymfm_update_irq(bool asserted) { }

	// the chip implementation calls this whenever a new value is written to
	// one of the chip's output ports (only applies to some chip types); our
	// responsibility is to pass the written data on to any consumers
	virtual void ymfm_io_write(uint8_t port, uint8_t data) { }

	// the chip implementation calls this whenever an on-chip register is read
	// which returns data from one of the chip's input ports; our responsibility
	// is to produce the current input value so that it can be reflected by the
	// read operation
	virtual uint8_t ymfm_io_read(uint8_t port) { return 0; }

	// the chip implementation calls this whenever the ADPCM-A engine needs to
	// fetch data for sound generation; our responsibility is to read the data
	// from the appropriate ROM/RAM at the given offset and return it
	virtual uint8_t ymfm_adpcm_a_read(uint32_t offset) { return 0; }

	// the chip implementation calls this whenever the ADPCM-B engine needs to
	// fetch data for sound generation; our responsibility is to read the data
	// from the appropriate ROM/RAM at the given offset and return it
	virtual uint8_t ymfm_adpcm_b_read(uint32_t offset) { return 0; }

	// the chip implementation calls this whenever the ADPCM-B engine requests
	// a write to the sound data; our responsibility is to write the data to
	// the appropriate RAM at the given offset
	virtual void ymfm_adpcm_b_write(uint32_t offset, uint8_t data) { }

	// the chip implementation calls this to log warnings or other information;
	// our responsibility is to either ignore it or surface it for debugging
	virtual void ymfm_log(char const *string) { }

protected:
	// pointer to engine callbacks -- this is set directly by the engine at
	// construction time
	ymfm_engine_callbacks *m_engine;
};

}

#endif // YMFM_H
