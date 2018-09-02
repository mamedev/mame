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

    2                   PCM Frequency
                        Waveform Pitch Lo

    3                   Waveform Pitch Hi

    4                   PCM Sample Start / 0x1000           [Start/End in bytes]
                        Waveform Envelope Time

    5                   PCM Sample End 0x100 - (Sample End / 0x1000)    [PCM ROM is Max 1MB?]
                        Waveform Envelope No.
    6                   Reserved
    7                   Reserved

    offset 0x0000 - 0x0fff  Wave form data
    offset 0x1000 - 0x1fff  Envelope data

    *1 : when 0 is specified, hardware interrupt is caused (always return soon)

***************************************************************************/

#include "emu.h"
#include "x1_010.h"


#define VERBOSE_SOUND 0
#define VERBOSE_REGISTER_WRITE 0
#define VERBOSE_REGISTER_READ 0

#define LOG_SOUND(...) do { if (VERBOSE_SOUND) logerror(__VA_ARGS__); } while (0)
#define LOG_REGISTER_WRITE(...) do { if (VERBOSE_REGISTER_WRITE) logerror(__VA_ARGS__); } while (0)
#define LOG_REGISTER_READ(...) do { if (VERBOSE_REGISTER_READ) logerror(__VA_ARGS__); } while (0)


namespace {

#define FREQ_BASE_BITS        8                 // Frequency fixed decimal shift bits
#define ENV_BASE_BITS        16                 // wave form envelope fixed decimal shift bits
#define VOL_BASE    (2*32*256/30)               // Volume base

/* this structure defines the parameters for a channel */
struct X1_010_CHANNEL {
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

x1_010_device::x1_010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X1_010, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this, 20)
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
	int i;

	m_base_clock    = clock();
	m_rate          = clock() / 512;

	for( i = 0; i < NUM_CHANNELS; i++ ) {
		m_smp_offset[i] = 0;
		m_env_offset[i] = 0;
	}
	/* Print some more debug info */
	LOG_SOUND("masterclock = %d rate = %d\n", clock(), m_rate );

	/* get stream channels */
	m_stream = machine().sound().stream_alloc(*this, 0, 2, m_rate);

	m_reg = make_unique_clear<uint8_t[]>(0x2000);
	m_HI_WORD_BUF = make_unique_clear<uint8_t[]>(0x2000);

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
//  rom_bank_updated - the rom bank has changed
//-------------------------------------------------

void x1_010_device::rom_bank_updated()
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
	int channel, reg;

	channel = offset/sizeof(X1_010_CHANNEL);
	reg     = offset%sizeof(X1_010_CHANNEL);

	if( channel < NUM_CHANNELS && reg == 0
		&& (m_reg[offset]&1) == 0 && (data&1) != 0 ) {
		m_smp_offset[channel] = 0;
		m_env_offset[channel] = 0;
	}
	LOG_REGISTER_WRITE("%s: offset %6X : data %2X\n", machine().describe_context(), offset, data);
	m_reg[offset] = data;
}


/* Use these for 16 bit CPUs */

u16 x1_010_device::word_r(offs_t offset)
{
	uint16_t  ret;

	ret = m_HI_WORD_BUF[offset]<<8;
	ret += (read( offset )&0xff);
	LOG_REGISTER_READ("%s: Read X1-010 Offset:%04X Data:%04X\n", machine().describe_context(), offset, ret);
	return ret;
}

void x1_010_device::word_w(offs_t offset, u16 data)
{
	m_HI_WORD_BUF[offset] = (data>>8)&0xff;
	write( offset, data&0xff );
	LOG_REGISTER_WRITE("%s: Write X1-010 Offset:%04X Data:%04X\n", machine().describe_context(), offset, data);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void x1_010_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	X1_010_CHANNEL  *reg;
	int     ch, i, volL, volR, freq, div;
	int8_t   data;
	uint8_t  *env;
	uint32_t start, end, smp_offs, smp_step, env_offs, env_step, delta;

	// mixer buffer zero clear
	memset( outputs[0], 0, samples*sizeof(*outputs[0]) );
	memset( outputs[1], 0, samples*sizeof(*outputs[1]) );

//  if( m_sound_enable == 0 ) return;

	for( ch = 0; ch < NUM_CHANNELS; ch++ ) {
		reg = (X1_010_CHANNEL *)&(m_reg[ch*sizeof(X1_010_CHANNEL)]);
		if( (reg->status&1) != 0 ) {                            // Key On
			stream_sample_t *bufL = outputs[0];
			stream_sample_t *bufR = outputs[1];
			div = (reg->status&0x80) ? 1 : 0;
			if( (reg->status&2) == 0 ) {                        // PCM sampling
				start    = reg->start*0x1000;
				end      = (0x100-reg->end)*0x1000;
				volL     = ((reg->volume>>4)&0xf)*VOL_BASE;
				volR     = ((reg->volume>>0)&0xf)*VOL_BASE;
				smp_offs = m_smp_offset[ch];
				freq     = reg->frequency>>div;
				// Meta Fox does write the frequency register, but this is a hack to make it "work" with the current setup
				// This is broken for Arbalester (it writes 8), but that'll be fixed later.
				if( freq == 0 ) freq = 4;
				smp_step = (uint32_t)((float)m_base_clock/8192.0f
							*freq*(1<<FREQ_BASE_BITS)/(float)m_rate);
				if( smp_offs == 0 ) {
					LOG_SOUND("Play sample %p - %p, channel %X volume %d:%d freq %X step %X offset %X\n",
						start, end, ch, volL, volR, freq, smp_step, smp_offs);
				}
				for( i = 0; i < samples; i++ ) {
					delta = smp_offs>>FREQ_BASE_BITS;
					// sample ended?
					if( start+delta >= end ) {
						reg->status &= 0xfe;                    // Key off
						break;
					}
					data = (int8_t)(read_byte(start+delta));
					*bufL++ += (data*volL/256);
					*bufR++ += (data*volR/256);
					smp_offs += smp_step;
				}
				m_smp_offset[ch] = smp_offs;
			} else {                                            // Wave form
				start    = (reg->volume*128+0x1000);
				smp_offs = m_smp_offset[ch];
				freq     = ((reg->pitch_hi<<8)+reg->frequency)>>div;
				smp_step = (uint32_t)((float)m_base_clock/128.0f/1024.0f/4.0f*freq*(1<<FREQ_BASE_BITS)/(float)m_rate);

				env      = (uint8_t *)&(m_reg[reg->end*128]);
				env_offs = m_env_offset[ch];
				env_step = (uint32_t)((float)m_base_clock/128.0f/1024.0f/4.0f*reg->start*(1<<ENV_BASE_BITS)/(float)m_rate);
				/* Print some more debug info */
				if( smp_offs == 0 ) {
					LOG_SOUND("Play waveform %X, channel %X volume %X freq %4X step %X offset %X\n",
						reg->volume, ch, reg->end, freq, smp_step, smp_offs);
				}
				for( i = 0; i < samples; i++ ) {
					int vol;
					delta = env_offs>>ENV_BASE_BITS;
					// Envelope one shot mode
					if( (reg->status&4) != 0 && delta >= 0x80 ) {
						reg->status &= 0xfe;                    // Key off
						break;
					}
					vol = *(env+(delta&0x7f));
					volL = ((vol>>4)&0xf)*VOL_BASE;
					volR = ((vol>>0)&0xf)*VOL_BASE;
					data  = (int8_t)(m_reg[start+((smp_offs>>FREQ_BASE_BITS)&0x7f)]);
					*bufL++ += (data*volL/256);
					*bufR++ += (data*volR/256);
					smp_offs += smp_step;
					env_offs += env_step;
				}
				m_smp_offset[ch] = smp_offs;
				m_env_offset[ch] = env_offs;
			}
		}
	}
}
