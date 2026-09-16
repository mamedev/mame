// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    dfac2.cpp - Apple Digitally Filtered Audio Chip, which controls input and
              output gain and filtering.

    DFAC2 is a newer version of the DFAC chip which uses a true I2C interface.

    Inputs are:
    - a digital stereo signal from the sound chip with bit and sample clocks
    - a mono microphone input
    - a mono modem input
    - a general purpose stereo input

    Outputs are:
    - an analog monoural internal speaker
    - a stereo line level output

***************************************************************************/

#include "emu.h"
#include "dfac2.h"

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(APPLE_DFAC2, dfac2_device, "apldfac2", "Apple Digitally Filtered Audio Chip II")

static constexpr double atten_table[8] =
{
	0.0,                    // -infinite
	0.125892541179417,      // -18dB
	0.177827941003892,      // -15dB
	0.251188643150958,      // -12dB
	0.354813389233575,      // -9dB
	0.501187233627272,      // -6dB
	0.707945784384138,      // -3dB
	1.0                     // No attenuation
};

dfac2_device::dfac2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APPLE_DFAC2, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	i2c_hle_interface(mconfig, *this, 0x6f),
	m_stream(nullptr),
	m_data(false),
	m_clock(false),
	m_dfaclatch(false),
	m_settings_byte(0)
{
}

void dfac2_device::device_start()
{
	m_stream = stream_alloc(8, 2, clock(), STREAM_SYNCHRONOUS);

	save_item(NAME(m_clock));
	save_item(NAME(m_data));
	save_item(NAME(m_dfaclatch));
	save_item(NAME(m_settings_byte));
}

void dfac2_device::sound_stream_update(sound_stream &stream)
{
	sound_stream::sample_t l = stream.get(0, 0);
	sound_stream::sample_t r = stream.get(1, 0);
	stream.put(0, 0, l * atten_table[m_settings_byte >> 5]);
	stream.put(1, 0, r * atten_table[m_settings_byte >> 5]);
}

/*
    Reverse-engineering DFAC2 is a work in progress.

    Known registers:
    0x02
    0x04
    0x06
    0x08
    0x0a    - input select, 0x10 = internal mic and 0x17 = external mic on Color Classic
    0x0c    - output enable?  0x47 always written before the boot chime.
    0x0d    - 7.5.0 writes 0x2a here on startup
    0x0f    - output enable?  0x41 always written before the boot chime.
*/
void dfac2_device::write_data(u16 offset, u8 data)
{
	LOG("DFAC2: write %02x to %04x (%s)\n", data, offset, machine().describe_context());
}
