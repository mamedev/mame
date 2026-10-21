// license:BSD-3-Clause
// copyright-holders:Carl Lom
/***************************************************************************

    Fujitsu MB654419U (Roland TVF gate array) + TVF-16 filter chip

    Per-voice filter and level for the SA-16 voices in the S-330 / W-30.
    Register usage from the S-330 system 1.03 and W-30 system 1.10 firmware.
    The filter is a Chamberlin state variable low-pass (12 dB/oct) running
    at the 30 kHz voice rate, whose two coefficients are the cutoff and
    resonance registers in 2.14 fixed point (0x4000 = 1.0).
    Fitted to recordings of a real S-330 (cutoff 40/70/100/127, resonance
    0/32/64/92/127; harmonic levels match within 1.6 dB rms):
        lp += f * bp;  hp = in - lp - q * bp;  bp += f * hp;  out = lp
        f = cutoff / 0x4000   (corner fs/pi * asin(f/2): 0x3bb0 -> ~4.6 kHz,
                               where resonance peaks; with low resonance the
                               top cutoff is nearly flat, -1.6 dB at 10 kHz)
        q = resonance / 0x4000 (Q = 1/q; 0x4000 -> Q 1, 0x196 -> Q ~40)
    The meaning of most setup registers and the internal table are unknown.

    Host interface (even bytes):
      f000/f002  register address lo/hi
      f004/f006  register data lo/hi; the f006 write commits the register
      f008-f012  setup registers written once at init (f008=1b f00a=64
                 f00c=07/ff f00e=19 f012=0f, f010 command bytes)
    During init the firmware also streams 64 24-bit table entries through
    f004/f006 without setting an address; those writes are ignored here.

    Register address = parameter * 0x10 + slot, slot 1..16 (slot 16 is
    written as 0x10 of the previous parameter group, e.g. cutoff of slot 16
    at 0x10), i.e. channel = (address - 1) & 15, parameter = (address - 1) >> 4.
    The firmware maps SA-16 voice v to slot v+3 (v = 0..13), 1 (v = 14) and
    2 (v = 15), so voice v is channel (v + 2) & 15: the TVF slots run three
    ahead of the SA-16 voice order.

    Parameters (16-bit):
      0 (+0x00) cutoff: SVF frequency coefficient f (2.14), one semitone
                per firmware step, 10 .. 0x3bb0 (corner ~30 Hz .. 4.6 kHz);
                0x4000 is written when the tone's filter is switched off
      1 (+0x10) level: 0x4000 = unity (the firmware maximum is just below)
      2 (+0x20) resonance: SVF damping coefficient q (2.14), 0x4000 for
                resonance 0 (Q 1), halving every 24 steps of the tone
                parameter down to 0x196 (resonance 127)
      3 (+0x30) unknown, 0x3800 at init
      4 (+0x40) unknown, 0 while a voice is set up, 0x3800 after key-on
      6 (+0x60) unknown, written with the cutoff at voice start
                (perhaps the initial value of an interpolated cutoff)

    Debugger: the "regs" address space of this device shows the current
    registers as 16-bit words at parameter * 0x10 + channel (chip address
    minus one); view it with 16 words per row to get one parameter per row.
    Writes from the debugger take effect immediately.

    TODO:
    - Parameters 3, 4 and 6, the setup registers and the internal table.
    - Cutoff and level changes are applied instantly, so the firmware's
      updates (about every 10 ms) show up as 100 Hz stepping in the harmonic
      levels during a filter sweep. On S-330 recordings of a sawtooth sweep
      that stepping is absent where MAME shows it (C4, C5), which suggests the
      chip smooths cutoff changes.
      Parameters 4 (+0x40, 0 during voice setup, 0x3800 after key-on) and
      6 (+0x60, the cutoff at voice start) may be related.

***************************************************************************/

#include "emu.h"
#include "mb654419u.h"

#include <algorithm>
#include <iterator>


#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MB654419U, mb654419u_device, "mb654419u", "Fujitsu MB654419U TVF")

mb654419u_device::mb654419u_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB654419U, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_regs_config("regs", ENDIANNESS_LITTLE, 16, 7, -1, address_map_constructor(FUNC(mb654419u_device::regs_map), this))
	, m_stream(nullptr)
	, m_address(0)
	, m_data(0)
	, m_address_set(false)
	, m_output_level(0.0)
{
}

device_memory_interface::space_config_vector mb654419u_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_regs_config) };
}

void mb654419u_device::regs_map(address_map &map)
{
	map(0x00, 0x7f).lrw16(
		NAME([this] (offs_t offset) -> u16 { return m_regs[offset >> 4][offset & 0x0f]; }),
		NAME([this] (offs_t offset, u16 data) { register_w(offset + 1, data); }));
}

void mb654419u_device::device_start()
{
	// one input per SA-16 voice, mono output
	m_stream = stream_alloc(NUM_CHANNELS, 1, SAMPLE_RATE_INPUT_ADAPTIVE);

	save_item(NAME(m_address));
	save_item(NAME(m_data));
	save_item(NAME(m_address_set));
	save_item(NAME(m_setup));
	save_item(NAME(m_regs));
	save_item(STRUCT_MEMBER(m_channel, lp));
	save_item(STRUCT_MEMBER(m_channel, bp));
	save_item(NAME(m_output_level));
}

void mb654419u_device::device_post_load()
{
	// the filter coefficients are derived from the registers
	for (auto &ch : m_channel)
		ch.dirty = true;
}

void mb654419u_device::device_reset()
{
	m_address = 0;
	m_data = 0;
	m_address_set = false;
	std::fill(std::begin(m_setup), std::end(m_setup), 0);
	for (auto &param : m_regs)
		std::fill(std::begin(param), std::end(param), 0);
	for (int c = 0; c < NUM_CHANNELS; c++)
	{
		m_regs[PARAM_CUTOFF][c] = 0x4000;
		m_regs[PARAM_RESONANCE][c] = 0x4000;
		m_channel[c].lp = m_channel[c].bp = 0.0;
		m_channel[c].dirty = true;
	}
}

u8 mb654419u_device::read(offs_t offset)
{
	// the firmware never reads the TVF
	return 0xff;
}

void mb654419u_device::write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0: m_address = (m_address & 0xff00) | data; m_address_set = true; break;
	case 1: m_address = (m_address & 0x00ff) | (data << 8); m_address_set = true; break;
	case 2: m_data = (m_data & 0xff00) | data; break;
	case 3:
		m_data = (m_data & 0x00ff) | (data << 8);
		if (m_address_set)
			register_w(m_address, m_data);
		else
			LOG("table data %04x\n", m_data);
		m_address_set = false;
		break;
	case 4: case 5: case 6: case 7: case 8: case 9:
		LOG("setup %02x <= %02x\n", 0x08 + 2 * (offset - 4), data);
		m_setup[offset - 4] = data;
		break;
	default:
		LOG("write %02x <= %02x\n", offset * 2, data);
		break;
	}
}

void mb654419u_device::register_w(u16 address, u16 data)
{
	if (address == 0)
	{
		LOG("reg 00 <= %04x\n", data);
		return;
	}
	const int channel = (address - 1) & (NUM_CHANNELS - 1);
	const int param = ((address - 1) >> 4) & (NUM_PARAMS - 1);

	m_stream->update();
	m_regs[param][channel] = data;
	if (param == PARAM_CUTOFF || param == PARAM_RESONANCE)
		m_channel[channel].dirty = true;
	LOG("%.4f param %d channel %2d <= %04x\n", machine().time().as_double(), param, channel, data);
}

void mb654419u_device::update_coefficients(int channel)
{
	channel_t &ch = m_channel[channel];
	const u16 cutoff = m_regs[PARAM_CUTOFF][channel];
	const u16 resonance = m_regs[PARAM_RESONANCE][channel];

	ch.dirty = false;
	// 0x4000 is what the firmware writes for a tone with the filter switched
	// off; passing through differs from running the SVF by < 0.5 dB.
	ch.bypass = cutoff >= 0x4000;

	// 2.14 fixed-point coefficients of the Chamberlin SVF
	ch.f = double(cutoff) / 0x4000;
	ch.q = double(resonance) / 0x4000;
}

void mb654419u_device::sound_stream_update(sound_stream &stream)
{
	// The coefficients are per sample: the stream must run at the SA-16 voice
	// rate (30 kHz), which SAMPLE_RATE_INPUT_ADAPTIVE gives us.
	for (int c = 0; c < NUM_CHANNELS; c++)
		if (m_channel[c].dirty)
			update_coefficients(c);

	for (int i = 0; i < stream.samples(); i++)
	{
		double mix = 0.0;
		for (int v = 0; v < NUM_CHANNELS; v++)
		{
			// SA-16 voice v arrives in TVF channel (v + 2) & 15
			const int c = (v + 2) & (NUM_CHANNELS - 1);
			channel_t &ch = m_channel[c];
			double out = stream.get(v, i);

			if (!ch.bypass)
			{
				ch.lp += ch.f * ch.bp;
				const double hp = out - ch.lp - ch.q * ch.bp;
				ch.bp += ch.f * hp;
				out = ch.lp;
			}

			mix += out * (double(m_regs[PARAM_LEVEL][c]) / 0x4000);
		}
		stream.put(0, i, mix);
		m_output_level = mix;
	}
}

double mb654419u_device::output_level()
{
	m_stream->update();
	return m_output_level;
}
