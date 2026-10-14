// license:GPL-2.0
// copyright-holders:Giulio Zausa
/***************************************************************************

    Roland/Fujitsu MB87126 Reverb/EQ DSP (DEP-5, D-50, RE-3, ...)
    Originally developed by The Usual Suspects for the Gearmulator project, the
    MAME version actually executes the original mask ROM program step by step.

    Serial SCK/SXD: 40 bits, LSB first, complemented selector and 16-bit DC,
    complemented signed 12-bit DD mantissa, active-low 2-bit range and two
    protocol bits (not modeled). INCK behavior is also not yet modeled.

    Each nominal frame takes 160 * 8 oscillator periods. An external board
    timer can stretch the interval in half-oscillator periods. The stream
    holds the ideal converter outputs between frames. Analog filters, DAC
    settling and sub-frame host arbitration are not modeled.

    The chip has an internal 160 x 35-bit mask ROM with the DSP program.
    The D-50 uses only 128 words to run at 32.768 MHz.
    It's maybe possible to dump the ROM using the chip test mode through the RD
    pins 5 bits at a time, but it's still not fully known how.

    Bit       Meaning
    34..30    RD4..RD0 board strobes
      29      active-low strobe after SAR ADC trial, not modeled
      28      active-low SAR decision / next trial
      27      restart SAR ADC (low with b28)
      26      select output latch B
      25      capture data bus into latch b24:23
    24..23    bus latch number
    22,20     inverted operand latch address (m-2)
    21,19     inverted whole-word preload latch (m)
    18..17    next operand source: 01 fetched latch, 00 capture + keep, 10 kept, 11 converter
      16      unknown, not modeled, zero in DEP-5 program
    15..14    second adder input (m+1): 11 zero, 01 preload latch, 10 sum, 00 register
      13      saturate/capture previous sum (m+1)
      12      DAC source: converter / output latch
      11      unknown, not modeled (output-phase control?)
      10      capture -> output latch
       9      output latch select: 1 A, 0 B
     8..7     saved-sum register read address (m)
       6      save sum from two words ago (m+2)
     5..4     saved-sum register write address
       3      bus data w/r, low drives capture onto the bus
       2      /WE
       1      CAS enable
       0      unknown, not modeled (isolated pulse at word 96?)

    m names a multiply, not a ROM address: its controls span m-2..m+2.
    Capture, bus transfer and operand fetch order matters. A store and a
    kept operand take the capture BEFORE this word's b13. A capture saturates
    the running sum in place. A bus-latch selection takes two words to reach
    the multiplier. Four bus latches and four 28-bit saved sums suffice.

    Reverse engineered from silicon using Furrtek's die photographs of
    Roland R15229841 (MB87126-002) and by probing the DRAM and DAC bus
    with a logic analyzer. The ROM is still BAD_DUMP: some vias were
    difficult to read in the photograph, so 13 uncertain bits were
    corrected manually by probing the chip's DRAM/DAC behavior.

***************************************************************************/
#include "emu.h"
#include "mb87126.h"

#include <algorithm>

namespace {
constexpr unsigned inverted_address(uint64_t word, unsigned high, unsigned low)
{
	return 3 ^ ((BIT(word, high) << 1) | BIT(word, low));
}
} // namespace

mb87126_device::mb87126_core::mb87126_core()
{
	reset();
}

uint64_t mb87126_device::mb87126_core::program_word(const uint8_t *program, unsigned pc)
{
	uint64_t word = 0;
	for (unsigned byte = 0; byte < 5; ++byte)
		word |= uint64_t(program[pc * 5 + byte]) << (byte * 8);
	return word & 0x7ffffffffULL;
}

void mb87126_device::mb87126_core::reset()
{
	m_dc.fill(0);
	m_dd.fill(0);
	m_dram.fill(0);
	m_latch.fill(0);
	m_register.fill(0);
	m_base = 0;
	m_sum = m_older_sum = 0;
	m_capture = m_operand = m_kept_operand = m_fetch = m_bus = 0;
	m_converter = 1;
	m_trial = 0;
	m_trial_bit = -1;
	m_output.fill(0);
	m_output_select = false;
}

void mb87126_device::mb87126_core::write_parameter(uint8_t selector, uint16_t delay, uint16_t coefficient)
{
	if (selector < STEPS)
	{
		m_dc[selector] = delay;
		m_dd[selector] = coefficient & 0x3fff;
	}
}

void mb87126_device::mb87126_core::bus_w(uint32_t data, uint32_t mem_mask)
{
	const uint32_t value = ((uint32_t(m_bus) & ~mem_mask) | (data & mem_mask)) & 0xffffff;
	m_bus = int32_t(value ^ 0x800000) - 0x800000;
}

int64_t mb87126_device::mb87126_core::wrap28(int64_t value)
{
	return ((value + (int64_t(1) << 27)) & 0xfffffff) - (int64_t(1) << 27);
}

int32_t mb87126_device::mb87126_core::clip24(int64_t value)
{
	if (value > 0x7fffff)
		return 0x7fffff;
	if (value < -0x800000)
		return -0x7fffff;
	return int32_t(value);
}

int64_t mb87126_device::mb87126_core::term(int32_t operand, uint16_t coefficient)
{
	const int32_t mantissa = (coefficient & 0x800) ? (coefficient & 0xfff) - 4096 : coefficient & 0xfff;
	const unsigned range = 3 - ((coefficient >> 12) & 3);
	const int64_t scale = range ? (int64_t(256) << (2 * range)) : 1;
	int64_t product = int64_t(operand) * mantissa * scale;
	if (operand == -0x800000 && mantissa == -2048)
		product = -product; // 24 x 12 multiplier overflow
	return product < 0 ? (product + 1) / (int64_t(1) << 21) : product / (int64_t(1) << 21);
}

void mb87126_device::mb87126_core::run_frame(const uint8_t *program, int16_t adc, const bus_handler &bus_cycle)
{
	m_output_select = false;
	const uint16_t target = uint16_t(adc) ^ 0x8000;
	for (unsigned pc = 0; pc < STEPS; ++pc)
	{
		const uint64_t word = program_word(program, pc);
		const uint64_t next = program_word(program, (pc + 1) % STEPS);
		const unsigned slot = (pc + STEPS - 1) % STEPS;
		const int32_t old_capture = m_capture;

		if (BIT(word, 13))
		{
			m_sum = clip24(m_sum);
			m_capture = int32_t(m_sum);
		}

		const unsigned address = (m_base + m_dc[slot]) & (DRAM_WORDS - 1);
		if (BIT(word, 1))
		{
			if (BIT(word, 2))
				m_bus = m_dram[address];
			else
				m_bus = m_dram[address] = old_capture;
		}
		else
		{
			const bool driving = !BIT(word, 3);
			if (driving)
				m_bus = old_capture;
		}
		// Board strobes are visible even during delay-RAM cycles. An external
		// receiver can drive bus pins synchronously before this word's latch.
		if (bus_cycle)
			bus_cycle(uint8_t(word >> 30) | (!BIT(word, 3) ? BUS_DRIVE : 0) | (BIT(word, 1) ? BUS_CAS : 0), uint32_t(m_bus) & 0xffffff);

		if (BIT(word, 25))
			m_latch[(word >> 23) & 3] = m_bus;
		if (BIT(word, 6))
			m_register[(word >> 4) & 3] = m_older_sum;

		int64_t sum = term(m_operand, m_dd[slot]);
		switch ((next >> 14) & 3)
		{
		case 0:
			sum += m_register[(word >> 7) & 3];
			break;
		case 1:
			sum += m_latch[inverted_address(word, 21, 19)];
			break;
		case 2:
			sum += m_sum;
			break;
		case 3:
			break;
		}
		m_older_sum = m_sum;
		m_sum = wrap28(sum);

		switch ((word >> 17) & 3)
		{
		case 0:
			m_operand = m_kept_operand = old_capture;
			break;
		case 1:
			m_operand = m_fetch;
			break;
		case 2:
			m_operand = m_kept_operand;
			break;
		case 3:
			m_operand = m_converter;
			break;
		}
		m_fetch = m_latch[inverted_address(word, 22, 20)];

		if (BIT(word, 10))
			m_output[BIT(word, 9) ? 0 : 1] = old_capture >> 8;
		if (BIT(word, 26))
			m_output_select = true;

		// Ideal integer comparator, without analog settling/filter models. ROM
		// strobes still sequence all sixteen decisions and the final result.
		if (!BIT(word, 28))
		{
			if (!BIT(word, 27))
			{
				m_trial = 0x7fff;
				m_trial_bit = 15;
			}
			else if (m_trial_bit >= 0)
			{
				if (target > m_trial)
					m_trial |= 1U << m_trial_bit;
				if (--m_trial_bit >= 0)
					m_trial &= ~(1U << m_trial_bit);
			}
			m_converter = int32_t(int16_t(m_trial ^ 0x8000)) * 256 + 1;
		}
	}
	// The physical base-pointer update phase is unknown (b0 is a candidate).
	m_base = (m_base + 1) & (DRAM_WORDS - 1);
}

bool mb87126_device::write_serial_pins(bool clock, bool data)
{
	bool committed = false;
	if (clock && !m_serial_clock)
	{
		auto &byte = m_serial[m_serial_bits / 8];
		byte = uint8_t((byte >> 1) | (data ? 0x80 : 0));
		if (++m_serial_bits == 40)
		{
			m_serial_bits = 0;
			// ROM 4c71/4c9a complement selector and DC; 4cc0 complements
			// only the 12-bit DD mantissa, preserving its two range bits.
			m_core.write_parameter(uint8_t(~m_serial[0]), uint16_t(~(m_serial[1] | (uint16_t(m_serial[2]) << 8))),
								   uint16_t((m_serial[3] | ((m_serial[4] & 0x3f) << 8)) ^ 0x0fff));
			committed = true;
		}
	}
	m_serial_clock = clock;
	return committed;
}

DEFINE_DEVICE_TYPE(MB87126, mb87126_device, "mb87126", "Roland MB87126-002 reverb DSP")
mb87126_device::mb87126_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB87126, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_program(*this, "program")
	, m_bus_cb(*this)
	, m_serial{}
	, m_serial_bits(0)
	, m_serial_clock(true)
	, m_ercl(true)
	, m_busy_until(attotime::zero)
	, m_sync_error(false)
	, m_frame_clocks(640)
	, m_next_frame_clocks(640)
	, m_frame_accumulator(640)
{
}

void mb87126_device::device_start()
{
	// Eight ticks per nominal frame; the external timer stretches the frame in
	// 20.48 MHz clocks. Keep a fixed stream rate and hold the converter outputs.
	m_stream = stream_alloc(1, 2, clock() / 160);
	save_item(NAME(m_serial));
	save_item(NAME(m_serial_bits));
	save_item(NAME(m_serial_clock));
	save_item(NAME(m_ercl));
	save_item(NAME(m_busy_until));
	save_item(NAME(m_sync_error));
	save_item(NAME(m_core.m_dc));
	save_item(NAME(m_core.m_dd));
	save_item(NAME(m_core.m_dram));
	save_item(NAME(m_core.m_latch));
	save_item(NAME(m_core.m_register));
	save_item(NAME(m_core.m_base));
	save_item(NAME(m_core.m_sum));
	save_item(NAME(m_core.m_older_sum));
	save_item(NAME(m_core.m_capture));
	save_item(NAME(m_core.m_operand));
	save_item(NAME(m_core.m_kept_operand));
	save_item(NAME(m_core.m_fetch));
	save_item(NAME(m_core.m_bus));
	save_item(NAME(m_core.m_converter));
	save_item(NAME(m_core.m_trial));
	save_item(NAME(m_core.m_trial_bit));
	save_item(NAME(m_core.m_output));
	save_item(NAME(m_core.m_output_select));
	save_item(NAME(m_frame_clocks));
	save_item(NAME(m_next_frame_clocks));
	save_item(NAME(m_frame_accumulator));
}

void mb87126_device::device_reset()
{
	m_serial = {};
	m_serial_bits = 0;
	m_serial_clock = m_ercl = true;
	m_busy_until = attotime::zero;
	m_sync_error = false;
	m_core.reset();
	m_frame_accumulator = m_frame_clocks = m_next_frame_clocks = 640;
}

void mb87126_device::serial_w(bool clock, bool data)
{
	m_stream->update();
	if (write_serial_pins(clock, data))
	{
		// Firmware sends three 01 00 00 00 00 synchronization probes and
		// expects BUSY to remain asserted until the next ERCL pulse. The
		// interpretation as a framing error is inferred from that sequence.
		if (m_serial == std::array<u8, 5>{1, 0, 0, 0, 0})
			m_sync_error = true;
		// Duration is provisional. Firmware sees one finite pulse per transfer.
		m_busy_until = machine().time() + attotime::from_usec(2);
	}
}

void mb87126_device::control_w(bool /*inck*/, bool ercl)
{
	m_stream->update();
	if (ercl && !m_ercl)
	{
		m_serial_bits = 0;
		m_sync_error = false;
		m_busy_until = attotime::zero;
	}
	m_ercl = ercl;
}

int mb87126_device::busy_r()
{
	return m_sync_error || machine().time() < m_busy_until;
}

void mb87126_device::frame_clocks_w(u32 clocks)
{
	m_stream->update();
	m_next_frame_clocks = std::max<u32>(clocks, 640);
}

void mb87126_device::sound_stream_update(sound_stream &stream)
{
	mb87126_core::bus_handler bus_cycle;
	if (!m_bus_cb.isunset())
		bus_cycle = [this](u8 signals, u32 data) { m_bus_cb(signals, data); };
	for (int i = 0; i < stream.samples(); ++i)
	{
		if (m_frame_accumulator >= m_frame_clocks)
		{
			m_frame_accumulator -= m_frame_clocks;
			const s16 adc = std::clamp<int>(stream.get(0, i) * 32768, -32768, 32767);
			m_core.run_frame(m_program, adc, bus_cycle);
			m_frame_clocks = m_next_frame_clocks;
		}
		m_frame_accumulator += 80;
		stream.put_int(0, i, m_core.m_output[0], 32768);
		stream.put_int(1, i, m_core.m_output[1], 32768);
	}
}
