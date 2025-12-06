// license:BSD-3-Clause
// copyright-holders:Hiromitsu Shioya, Olivier Galibert
/*********************************************************/
/*    Hohner 6ch 8bit PCM sample player                  */
/*********************************************************/

#include "emu.h"
#include "hohnerpcm.h"

//#include <algorithm>

#define LOG_DEV    (1 << 1U)

//#define VERBOSE (LOG_DEV)
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(HOHNERPCM, hohnerpcm_device, "hohnerpcm", "Hohner PCM Rhythm")


//-------------------------------------------------
//  hohnerpcm_device - constructor
//-------------------------------------------------

hohnerpcm_device::hohnerpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HOHNERPCM, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_stream(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hohnerpcm_device::device_start()
{
    for (int i = 0; i < 6; i++)
    {
        memset(&m_channels[i], 0, sizeof(Channel));
    }
    m_stream = stream_alloc(0, 2, 22050);
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void hohnerpcm_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / 128);
}


//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void hohnerpcm_device::rom_bank_pre_change()
{
	m_stream->update();
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void hohnerpcm_device::sound_stream_update(sound_stream &stream)
{
	/* clear the buffers */
	stream.fill(0, 0);
	stream.fill(1, 0);

	// instruction  function
	// ------------------------------------------------
	// 0xf0     init channel
	// 0xf3     sample start?
	// 0xf4     next address
	// 0xff     nop?

    auto next_byte = [&](auto& ch) {
        return read_byte(ch.address++);

    };
    auto get_byte = [&](const auto& ch) {
        return read_byte(ch.address);
    };

	/* loop over channels */
	for (int ch = 0; ch < 6; ch++)
	{
        auto& channel = m_channels[ch];

        if (!channel.active) {
            // NB: this loops in the original implementation
            continue;
        }

        auto out = ch < 2 ? 0 : 1;

        /* loop over samples on this channel */
        for (int i = 0; i < stream.samples(); i++)
        {
            uint8_t val = next_byte(channel);

            // check for instructions
            for (int i = 0; i < 16 && (val & 0xf0) == 0xf0; i++)
            {
                if (val == 0xf3)
                {
                    val = next_byte(channel);
                    LOGMASKED(LOG_DEV, "sample start: %02X\n", val);
                }
                else if (val == 0xf4)
                {
                    auto lo = next_byte(channel);
                    auto hi = next_byte(channel);
                    channel.address = (hi << 8) | lo;
                    val = get_byte(channel);
                    LOGMASKED(LOG_DEV, "next address:%04X val:%02X\n", channel.address, val);
                    // new address may point to next sample
                }
                else if (val == 0xf0)
                {
                    channel.active = false;
                    break;
                }
                else
                {
                    LOGMASKED(LOG_DEV, "unknown instruction %02X\n", val);
                    val = next_byte(channel);
                }
            }
            if (!channel.active) {
                break;
            }

            stream.add_int(out, i, (val - 0x80) * channel.volume, 32768);
        }
    }
}


void hohnerpcm_device::write(offs_t offset, uint8_t data)
{
	m_stream->update();

    auto instruction = read_byte(data);
    if (instruction != 0xf0) {
      LOGMASKED(LOG_DEV, "command not found\n");
      return;
    }
    auto address = read_byte(data + 1) | (read_byte(data + 2) << 8);

    auto volume = read_byte(data + 3);

    auto channel = (data >> 2) & 0x7;
    if (channel > 5) {
      LOGMASKED(LOG_DEV, "invalid channel\n");
      return;
    }
    m_channels[channel].address = address;
    m_channels[channel].volume = volume;
    m_channels[channel].active = true;
}
