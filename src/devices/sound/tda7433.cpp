// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

	tda7433.cpp - STMicroelectronics TDA7433 I2C volume/tone control
    Emulation by R. Belmont

	Master volume, bass and treble, and 4 independent speaker attenuators
	with mute switches.

	TODO: bass/treble control

***************************************************************************/

#include "emu.h"
#include "tda7433.h"

#define LOG_REGISTERS (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"

// Subaddresses.  Bit 4 of the sub address enables auto-increment, so only
// bits 0-3 are a register number.
enum
{
	REG_INPUT_SELECT = 0,
	REG_VOLUME,
	REG_BASS_TREBLE,
	REG_ATTEN_CH_0,
	REG_ATTEN_CH_1,
	REG_ATTEN_CH_2,
	REG_ATTEN_CH_3,

	NUM_REGISTERS
};

// Volume, where 0 = +20 dB, 32 = 0 dB, and 111 = -79 dB.
// The volume register runs in 1 dB steps with 0 dB at 32, going up to +20 dB as the value
// falls and down to -79 dB as it rises.
constexpr u8  VOLUME_0DB = 32;
constexpr int VOLUME_MAX_GAIN_DB = 20;
constexpr int VOLUME_MIN_GAIN_DB = -79;

// Each speaker attenuator is 1 dB per step over 5 bits, with a separate mute bit above it.
constexpr u8 ATTENUATOR_LEVEL_MASK = 0x1f;
constexpr u8 ATTENUATOR_MUTE = 0x20;

DEFINE_DEVICE_TYPE(TDA7433, tda7433_device, "tda7433", "STmicro TDA7433 volume/tone control")

tda7433_device::tda7433_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, TDA7433, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	i2c_hle_interface(mconfig, *this, 0x45),
	m_stream(nullptr)
{
}

void tda7433_device::device_start()
{
	m_stream = stream_alloc(4, 4, SAMPLE_RATE_OUTPUT_ADAPTIVE);

	save_item(NAME(m_registers));
	save_item(NAME(m_gain));

	std::fill(std::begin(m_registers), std::end(m_registers), 0);
	m_registers[REG_VOLUME] = VOLUME_0DB - VOLUME_MIN_GAIN_DB;
	for (int reg = REG_ATTEN_CH_0; reg <= REG_ATTEN_CH_3; reg++)
	{
		m_registers[reg] = ATTENUATOR_MUTE | ATTENUATOR_LEVEL_MASK;
	}

	recalculate_gains();
}

void tda7433_device::device_reset()
{
}

void tda7433_device::sound_stream_update(sound_stream &stream)
{
	for (int channel = 0; channel < 4; channel++)
	{
		for (int sample = 0; sample < stream.samples(); sample++)
		{
			stream.put(channel, sample, stream.get(channel, sample) * m_gain[channel]);
		}
	}
}

float tda7433_device::attenuator_gain(u8 reg) const
{
	if (m_registers[reg] & ATTENUATOR_MUTE)
	{
		return 0.0f;
	}

	return powf(10.0f, -float(m_registers[reg] & ATTENUATOR_LEVEL_MASK) / 20.0f);
}

void tda7433_device::recalculate_gains()
{
	const int volume_db = std::clamp(VOLUME_0DB - int(m_registers[REG_VOLUME]), VOLUME_MIN_GAIN_DB, VOLUME_MAX_GAIN_DB);
	const float volume = powf(10.0f, float(volume_db) / 20.0f);

	m_gain[0] = volume * attenuator_gain(REG_ATTEN_CH_0);
	m_gain[1] = volume * attenuator_gain(REG_ATTEN_CH_1);
	m_gain[2] = volume * attenuator_gain(REG_ATTEN_CH_2);
	m_gain[3] = volume * attenuator_gain(REG_ATTEN_CH_3);
}

// this part can't be read back
u8 tda7433_device::read_data(u16 offset)
{
	return 0xff;
}

void tda7433_device::write_data(u16 offset, u8 data)
{
	const u8 reg = offset & 0x0f;
	if (reg >= NUM_REGISTERS)
	{
		LOGMASKED(LOG_REGISTERS, "%s: write %02x to invalid register %d\n", tag(), data, reg);
		return;
	}

	LOGMASKED(LOG_REGISTERS, "%s: register %d = %02x\n", tag(), reg, data);

	if (m_registers[reg] != data)
	{
		m_stream->update();
		m_registers[reg] = data;
		recalculate_gains();
	}
}
