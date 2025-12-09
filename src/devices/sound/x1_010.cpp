// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -= Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)

                    rewrite by Manbow-J(manbowj@hamal.freemail.ne.jp)

                    X1-010 Seta Custom Sound Chip (80 Pin PQFP)

 Custom programmed Mitsubishi M60016 Gate Array, 3608 gates, 148 Max I/O ports

    The X1-010 is a 16-Voice sound generator, each channel gets its
    waveform from RAM (128 bytes per waveform, 8 bit signed data)
    or sampling PCM (8 bit signed data).

Registers:
    8 registers per channel (mapped to the lower bytes of 16 words on the 68K)

    Reg:    Bits:       Meaning:

    0       7--- ----   Frequency divider flag (only downtown seems to set this)
            -654 3---
            ---- -2--   PCM/Waveform repeat flag (0:Once 1:Repeat) (*1)
            ---- --1-   Sound out select (0:PCM 1:Waveform)
            ---- ---0   Key on / off

    1       7654 ----   PCM Volume 1 (L?)
            ---- 3210   PCM Volume 2 (R?)
                        Waveform No.

    2                   PCM Frequency (4.4 fixed point)
                        Waveform Pitch Lo (6.10 fixed point)

    3                   Waveform Pitch Hi (6.10 fixed point)

    4                   PCM Sample Start / 0x1000           [Start/End in bytes]
                        Waveform Envelope Time (.10 fixed point)

    5                   PCM Sample End 0x100 - (Sample End / 0x1000)    [PCM ROM is Max 1MB?]
                        Waveform Envelope No.
    6                   Reserved
    7                   Reserved

    offset 0x0000 - 0x007f  Channel data
    offset 0x0080 - 0x0fff  Envelope data
    offset 0x1000 - 0x1fff  Wave form data

    *1 : when 0 is specified, hardware interrupt is caused (always return soon)

***************************************************************************/

#include "emu.h"
#include "x1_010.h"


#define LOG_SOUND          (1U << 1)
#define LOG_REGISTER_WRITE (1U << 2)
#define LOG_REGISTER_READ  (1U << 3)
#define VERBOSE (0)
#include "logmacro.h"


namespace {

#define VOL_BASE    (2*32*256/30)               // Volume base

/* this structure defines the parameters for a channel */
struct X1_010_CHANNEL
{
	unsigned char   status;
	unsigned char   volume;                     //        volume / wave form no.
	unsigned char   frequency;                  //     frequency / pitch lo
	unsigned char   pitch_hi;                   //      reserved / pitch hi
	unsigned char   start;                      // start address / envelope time
	unsigned char   end;                        //   end address / envelope no.
	unsigned char   reserve[2];
};

} // anonymous namespace


DEFINE_DEVICE_TYPE(X1_010, x1_010_device, "x1_010", "Seta X1-010")

x1_010_device::x1_010_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, X1_010, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_rate(0)
	, m_stream(nullptr)
	, m_sound_enable(0)
	, m_reg(nullptr)
	, m_HI_WORD_BUF(nullptr)
	, m_base_clock(0)
{
	std::fill(std::begin(m_smp_offset), std::end(m_smp_offset), 0);
	std::fill(std::begin(m_env_offset), std::end(m_env_offset), 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x1_010_device::device_start()
{
	m_base_clock    = clock();
	m_rate          = clock() / 512;

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		m_smp_offset[i] = 0;
		m_env_offset[i] = 0;
	}
	/* Print some more debug info */
	LOGMASKED(LOG_SOUND, "masterclock = %d rate = %d\n", clock(), m_rate);

	/* get stream channels */
	m_stream = stream_alloc(0, 2, m_rate);

	m_reg = make_unique_clear<u8[]>(0x2000);
	m_HI_WORD_BUF = make_unique_clear<u8[]>(0x2000);

	save_item(NAME(m_rate));
	save_item(NAME(m_sound_enable));
	save_pointer(NAME(m_reg), 0x2000);
	save_pointer(NAME(m_HI_WORD_BUF), 0x2000);
	save_item(NAME(m_smp_offset));
	save_item(NAME(m_env_offset));
	save_item(NAME(m_base_clock));
}

//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void x1_010_device::device_clock_changed()
{
	m_base_clock    = clock();
	m_rate          = clock() / 512;

	m_stream->set_sample_rate(m_rate);
}

//-------------------------------------------------
//  rom_bank_pre_change - refresh the stream if the
//  ROM banking changes
//-------------------------------------------------

void x1_010_device::rom_bank_pre_change()
{
	m_stream->update();
}


void x1_010_device::enable_w(int data)
{
	m_sound_enable = data;
}

/* Use these for 8 bit CPUs */

u8 x1_010_device::read(offs_t offset)
{
	return m_reg[offset];
}

void x1_010_device::write(offs_t offset, u8 data)
{
	int channel = offset/sizeof(X1_010_CHANNEL);
	int reg     = offset%sizeof(X1_010_CHANNEL);

	if (channel < NUM_CHANNELS && reg == 0
		&& (m_reg[offset] & 1) == 0 && (data & 1) != 0)
	{
		m_smp_offset[channel] = 0;
		m_env_offset[channel] = 0;
	}
	LOGMASKED(LOG_REGISTER_WRITE, "%s: offset %6X : data %2X\n", machine().describe_context(), offset, data);
	m_reg[offset] = data;
}


/* Use these for 16 bit CPUs */

u16 x1_010_device::word_r(offs_t offset)
{
	u16 ret;
	ret = m_HI_WORD_BUF[offset] << 8;
	ret |= (read(offset) & 0xff);
	LOGMASKED(LOG_REGISTER_READ, "%s: Read X1-010 Offset:%04X Data:%04X\n", machine().describe_context(), offset, ret);
	return ret;
}

void x1_010_device::word_w(offs_t offset, u16 data)
{
	m_HI_WORD_BUF[offset] = (data >> 8) & 0xff;
	write(offset, data & 0xff);
	LOGMASKED(LOG_REGISTER_WRITE, "%s: Write X1-010 Offset:%04X Data:%04X\n", machine().describe_context(), offset, data);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void x1_010_device::sound_stream_update(sound_stream &stream)
{
//  if (m_sound_enable == 0) return;

	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		X1_010_CHANNEL *reg = (X1_010_CHANNEL *)&(m_reg[ch*sizeof(X1_010_CHANNEL)]);
		if ((reg->status & 1) != 0)                            // Key On
		{
			const int div = (reg->status & 0x80) ? 1 : 0;
			if ((reg->status & 2) == 0)                        // PCM sampling
			{
				const u32 start = reg->start << 12;
				const u32 end   = (0x100 - reg->end) << 12;
				const int volL  = ((reg->volume >> 4) & 0xf) * VOL_BASE;
				const int volR  = ((reg->volume >> 0) & 0xf) * VOL_BASE;
				u32 smp_offs    = m_smp_offset[ch];
				int freq        = reg->frequency >> div;
				// Meta Fox does write the frequency register, but this is a hack to make it "work" with the current setup
				// This is broken for Arbalester (it writes 8), but that'll be fixed later.
				if (freq == 0) freq = 4;
				const u32 smp_step = freq;
				if (smp_offs == 0)
				{
					LOGMASKED(LOG_SOUND, "Play sample %p - %p, channel %X volume %d:%d freq %X step %X offset %X\n",
						start, end, ch, volL, volR, freq, smp_step, smp_offs);
				}
				for (int i = 0; i < stream.samples(); i++)
				{
					const u32 delta = smp_offs >> 4;
					// sample ended?
					if (start+delta >= end)
					{
						reg->status &= 0xfe;                    // Key off
						break;
					}
					const s8 data = (s8)(read_byte(start+delta));
					stream.add_int(0, i, data * volL, 32768 * 256);
					stream.add_int(1, i, data * volR, 32768 * 256);
					smp_offs += smp_step;
				}
				m_smp_offset[ch] = smp_offs;
			}
			else                                            // Wave form
			{
				const u16 start    = ((reg->volume << 7) + 0x1000);
				u32 smp_offs       = m_smp_offset[ch];
				const int freq     = ((reg->pitch_hi<<8)+reg->frequency) >> div;
				const u32 smp_step = freq;

				const u16 env      = reg->end << 7;
				u32 env_offs       = m_env_offset[ch];
				const u32 env_step = reg->start;
				/* Print some more debug info */
				if (smp_offs == 0)
				{
					LOGMASKED(LOG_SOUND, "Play waveform %X, channel %X volume %X freq %4X step %X offset %X\n",
						reg->volume, ch, reg->end, freq, smp_step, smp_offs);
				}
				for (int i = 0; i < stream.samples(); i++)
				{
					const u32 delta = env_offs >> 10;
					// Envelope one shot mode
					if ((reg->status & 4) != 0 && delta >= 0x80)
					{
						reg->status &= 0xfe;                    // Key off
						break;
					}
					const u8 vol   = m_reg[env + (delta & 0x7f)];
					const int volL = ((vol >> 4) & 0xf) * VOL_BASE;
					const int volR = ((vol >> 0) & 0xf) * VOL_BASE;
					const s8 data  = (s8)(m_reg[start + ((smp_offs >> 10) & 0x7f)]);
					stream.add_int(0, i, data * volL, 32768 * 256);
					stream.add_int(1, i, data * volR, 32768 * 256);
					smp_offs += smp_step;
					env_offs += env_step;
				}
				m_smp_offset[ch] = smp_offs;
				m_env_offset[ch] = env_offs;
			}
		}
	}
}
