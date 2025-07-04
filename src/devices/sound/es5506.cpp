// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************************************

     Ensoniq ES5505/6 driver
     by Aaron Giles

Ensoniq OTIS - ES5505                                            Ensoniq OTTO - ES5506

  OTIS is a VLSI device designed in a 2 micron double metal        OTTO is a VLSI device designed in a 1.5 micron double metal
   CMOS process. The device is the next generation of audio         CMOS process. The device is the next generation of audio
   technology from ENSONIQ. This new chip achieves a new            technology from ENSONIQ. All calculations in the device are
   level of audio fidelity performance. These improvements          made with at least 18-bit accuracy.
   are achieved through the use of frequency interpolation
   and on board real time digital filters. All calculations       The major features of OTTO are:
   in the device are made with at least 16 bit accuracy.           - 68 pin PLCC package
                                                                   - On chip real time digital filters
 The major features of OTIS are:                                   - Frequency interpolation
  - 48 Pin dual in line package                                    - 32 independent voices
  - On chip real time digital filters                              - Loop start and stop posistions for each voice
  - Frequency interpolation                                        - Bidirectional and reverse looping
  - 32 independent voices (up from 25 in DOCII)                    - 68000 compatibility for asynchronous bus communication
  - Loop start and stop positions for each voice                   - separate host and sound memory interface
  - Bidirectional and reverse looping                              - 6 channel stereo serial communication port
  - 68000 compatibility for asynchronous bus communication         - Programmable clocks for defining serial protocol
  - On board pulse width modulation D to A                         - Internal volume multiplication and stereo panning
  - 4 channel stereo serial communication port                     - A to D input for pots and wheels
  - Internal volume multiplication and stereo panning              - Hardware support for envelopes
  - A to D input for pots and wheels                               - Support for dual OTTO systems
  - Up to 10MHz operation                                          - Optional compressed data format for sample data
                                                                   - Up to 16MHz operation
              ______    ______
            _|o     \__/      |_
 A17/D13 - |_|1             48|_| - VSS                                                           A A A A A A
            _|                |_                                                                  2 1 1 1 1 1 A
 A18/D14 - |_|2             47|_| - A16/D12                                                       0 9 8 7 6 5 1
            _|                |_                                                                  / / / / / / 4
 A19/D15 - |_|3             46|_| - A15/D11                                   H H H H H H H V V H D D D D D D /
            _|                |_                                              D D D D D D D S D D 1 1 1 1 1 1 D
      BS - |_|4             45|_| - A14/D10                                   0 1 2 3 4 5 6 S D 7 5 4 3 2 1 0 9
            _|                |_                                             ------------------------------------+
  PWZERO - |_|5             44|_| - A13/D9                                  / 9 8 7 6 5 4 3 2 1 6 6 6 6 6 6 6 6  |
            _|                |_                                           /                    8 7 6 5 4 3 2 1  |
    SER0 - |_|6             43|_| - A12/D8                                |                                      |
            _|       E        |_                                      SER0|10                                  60|A13/D8
    SER1 - |_|7      N      42|_| - A11/D7                            SER1|11                                  59|A12/D7
            _|       S        |_                                      SER2|12                                  58|A11/D6
    SER2 - |_|8      O      41|_| - A10/D6                            SER3|13              ENSONIQ             57|A10/D5
            _|       N        |_                                      SER4|14                                  56|A9/D4
    SER3 - |_|9      I      40|_| - A9/D5                             SER5|15                                  55|A8/D3
            _|       Q        |_                                      WCLK|16                                  54|A7/D2
 SERWCLK - |_|10            39|_| - A8/D4                            LRCLK|17               ES5506             53|A6/D1
            _|                |_                                      BCLK|18                                  52|A5/D0
   SERLR - |_|11            38|_| - A7/D3                             RESB|19                                  51|A4
            _|                |_                                       HA5|20                                  50|A3
 SERBCLK - |_|12     E      37|_| - A6/D2                              HA4|21                OTTO              49|A2
            _|       S        |_                                       HA3|22                                  48|A1
     RLO - |_|13     5      36|_| - A5/D1                              HA2|23                                  47|A0
            _|       5        |_                                       HA1|24                                  46|BS1
     RHI - |_|14     0      35|_| - A4/D0                              HA0|25                                  45|BS0
            _|       5        |_                                    POT_IN|26                                  44|DTACKB
     LLO - |_|15            34|_| - CLKIN                                 |   2 2 2 3 3 3 3 3 3 3 3 3 3 4 4 4 4  |
            _|                |_                                          |   7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3  |
     LHI - |_|16            33|_| - CAS                                   +--------------------------------------+
            _|                |_                                              B E E B E B B D S B B B E K B W W
     POT - |_|17     O      32|_| - AMUX                                      S B L N L S S D S S X S   L Q / /
            _|       T        |_                                              E E R E H M C V V A U A   C R R R
   DTACK - |_|18     I      31|_| - RAS                                       R R D H           R M C     I M
            _|       S        |_                                              _ D                 A
     R/W - |_|19            30|_| - E                                         T
            _|                |_                                              O
      MS - |_|20            29|_| - IRQ                                       P
            _|                |_
      CS - |_|21            28|_| - A3
            _|                |_
     RES - |_|22            27|_| - A2
            _|                |_
     VSS - |_|23            26|_| - A1
            _|                |_
     VDD - |_|24            25|_| - A0
             |________________|

***********************************************************************************************/

#include "emu.h"
#include "es5506.h"
#include <algorithm>

#if ES5506_MAKE_WAVS
#include "sound/wavwrite.h"
#endif


/**********************************************************************************************

     CONSTANTS

***********************************************************************************************/

#define LOG_SERIAL              (1U << 1)

#define VERBOSE                 0
#include "logmacro.h"

#define RAINE_CHECK             0

static constexpr u32 FINE_FILTER_BIT = 16;
static constexpr u32 FILTER_BIT      = 12;
static constexpr u32 FILTER_SHIFT    = FINE_FILTER_BIT - FILTER_BIT;

static constexpr u32 ULAW_MAXBITS        = 8;

namespace {

enum : u16 {
	CONTROL_BS1         = 0x8000,
	CONTROL_BS0         = 0x4000,
	CONTROL_CMPD        = 0x2000,
	CONTROL_CA2         = 0x1000,
	CONTROL_CA1         = 0x0800,
	CONTROL_CA0         = 0x0400,
	CONTROL_LP4         = 0x0200,
	CONTROL_LP3         = 0x0100,
	CONTROL_IRQ         = 0x0080,
	CONTROL_DIR         = 0x0040,
	CONTROL_IRQE        = 0x0020,
	CONTROL_BLE         = 0x0010,
	CONTROL_LPE         = 0x0008,
	CONTROL_LEI         = 0x0004,
	CONTROL_STOP1       = 0x0002,
	CONTROL_STOP0       = 0x0001,

	CONTROL_BSMASK      = (CONTROL_BS1 | CONTROL_BS0),
	CONTROL_CAMASK      = (CONTROL_CA2 | CONTROL_CA1 | CONTROL_CA0),
	CONTROL_LPMASK      = (CONTROL_LP4 | CONTROL_LP3),
	CONTROL_LOOPMASK    = (CONTROL_BLE | CONTROL_LPE),
	CONTROL_STOPMASK    = (CONTROL_STOP1 | CONTROL_STOP0),

	// ES5505 has sightly different control bit
	CONTROL_5505_LP4    = 0x0800,
	CONTROL_5505_LP3    = 0x0400,
	CONTROL_5505_CA1    = 0x0200,
	CONTROL_5505_CA0    = 0x0100,

	CONTROL_5505_LPMASK = (CONTROL_5505_LP4 | CONTROL_5505_LP3),
	CONTROL_5505_CAMASK = (CONTROL_5505_CA1 | CONTROL_5505_CA0)
};

}

es550x_device::es550x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_sample_rate(0)
	, m_master_clock(0)
	, m_address_acc_shift(0)
	, m_address_acc_mask(0)
	, m_volume_shift(0)
	, m_volume_acc_shift(0)
	, m_current_page(0)
	, m_active_voices(0x1f)
	, m_mode(0)
	, m_irqv(0x80)
	, m_voice_index(0)
#if ES5506_MAKE_WAVS
	, m_wavraw(nullptr)
#endif
	, m_region0(*this, finder_base::DUMMY_TAG)
	, m_region1(*this, finder_base::DUMMY_TAG)
	, m_region2(*this, finder_base::DUMMY_TAG)
	, m_region3(*this, finder_base::DUMMY_TAG)
	, m_channels(0)
	, m_irq_cb(*this)
	, m_read_port_cb(*this, 0)
	, m_sample_rate_changed_cb(*this)
{
}

DEFINE_DEVICE_TYPE(ES5506, es5506_device, "es5506", "Ensoniq ES5506")

es5506_device::es5506_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: es550x_device(mconfig, ES5506, tag, owner, clock)
	, m_bank0_config("bank0", ENDIANNESS_BIG, 16, 21, -1) // 21 bit address bus, word addressing only
	, m_bank1_config("bank1", ENDIANNESS_BIG, 16, 21, -1)
	, m_bank2_config("bank2", ENDIANNESS_BIG, 16, 21, -1)
	, m_bank3_config("bank3", ENDIANNESS_BIG, 16, 21, -1)
	, m_write_latch(0)
	, m_read_latch(0)
	, m_wst(0)
	, m_wend(0)
	, m_lrend(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void es550x_device::device_start()
{
	// initialize the rest of the structure
	m_master_clock = clock();
	m_irqv = 0x80;

	// register save
	save_item(NAME(m_sample_rate));

	save_item(NAME(m_current_page));
	save_item(NAME(m_active_voices));
	save_item(NAME(m_mode));
	save_item(NAME(m_irqv));
	save_item(NAME(m_voice_index));

	save_item(STRUCT_MEMBER(m_voice, control));
	save_item(STRUCT_MEMBER(m_voice, freqcount));
	save_item(STRUCT_MEMBER(m_voice, start));
	save_item(STRUCT_MEMBER(m_voice, end));
	save_item(STRUCT_MEMBER(m_voice, accum));
	save_item(STRUCT_MEMBER(m_voice, lvol));
	save_item(STRUCT_MEMBER(m_voice, rvol));
	save_item(STRUCT_MEMBER(m_voice, k2));
	save_item(STRUCT_MEMBER(m_voice, k1));
	save_item(STRUCT_MEMBER(m_voice, o4n1));
	save_item(STRUCT_MEMBER(m_voice, o3n1));
	save_item(STRUCT_MEMBER(m_voice, o3n2));
	save_item(STRUCT_MEMBER(m_voice, o2n1));
	save_item(STRUCT_MEMBER(m_voice, o2n2));
	save_item(STRUCT_MEMBER(m_voice, o1n1));
}

void es5506_device::device_start()
{
	es550x_device::device_start();
	int channels = 1;  // 1 channel by default, for backward compatibility

	// only override the number of channels if the value is in the valid range 1 .. 6
	if (1 <= m_channels && m_channels <= 6)
		channels = m_channels;

	// create the stream
	m_stream = stream_alloc(0, 2 * channels, clock() / (16*32));

	// initialize the regions
	if (m_region0 && !has_configured_map(0))
		space(0).install_rom(0, std::min<offs_t>((1 << ADDRESS_INTEGER_BIT_ES5506) - 1, (m_region0->bytes() / 2) - 1), m_region0->base());

	if (m_region1 && !has_configured_map(1))
		space(1).install_rom(0, std::min<offs_t>((1 << ADDRESS_INTEGER_BIT_ES5506) - 1, (m_region1->bytes() / 2) - 1), m_region1->base());

	if (m_region2 && !has_configured_map(2))
		space(2).install_rom(0, std::min<offs_t>((1 << ADDRESS_INTEGER_BIT_ES5506) - 1, (m_region2->bytes() / 2) - 1), m_region2->base());

	if (m_region3 && !has_configured_map(3))
		space(3).install_rom(0, std::min<offs_t>((1 << ADDRESS_INTEGER_BIT_ES5506) - 1, (m_region3->bytes() / 2) - 1), m_region3->base());

	for (int s = 0; s < 4; s++)
		space(s).cache(m_cache[s]);

	// compute the tables
	compute_tables(VOLUME_BIT_ES5506, 4, 8); // 4 bit exponent, 8 bit mantissa

	// initialize the rest of the structure
	m_channels = channels;

	// KT-76 assumes all voices are active on an ES5506 without setting them!
	m_active_voices = 31;
	m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
	m_stream->set_sample_rate(m_sample_rate);

	// 21 bit integer and 11 bit fraction
	get_accum_mask(ADDRESS_INTEGER_BIT_ES5506, ADDRESS_FRAC_BIT_ES5506);

	// register save
	save_item(NAME(m_write_latch));
	save_item(NAME(m_read_latch));

	save_item(NAME(m_wst));
	save_item(NAME(m_wend));
	save_item(NAME(m_lrend));

	save_item(STRUCT_MEMBER(m_voice, ecount));
	save_item(STRUCT_MEMBER(m_voice, lvramp));
	save_item(STRUCT_MEMBER(m_voice, rvramp));
	save_item(STRUCT_MEMBER(m_voice, k2ramp));
	save_item(STRUCT_MEMBER(m_voice, k1ramp));
	save_item(STRUCT_MEMBER(m_voice, filtcount));

	// success
}

//-------------------------------------------------
//  device_clock_changed
//-------------------------------------------------

void es550x_device::device_clock_changed()
{
	m_master_clock = clock();
	m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
	m_stream->set_sample_rate(m_sample_rate);
	m_sample_rate_changed_cb(m_sample_rate);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void es550x_device::device_reset()
{
}

void es5506_device::device_reset() // RESB for Reset input
{
	m_active_voices = 0x1f; // 32 voice at reset
	m_mode = 0x17; // Dual, Slave, BCLK_EN high, WCLK_EN high, LRCLK_EN high
	notify_clock_changed();
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void es550x_device::device_stop()
{
#if ES5506_MAKE_WAVS
	{
		wav_close(m_wavraw);
	}
#endif
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector es5506_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_bank0_config),
		std::make_pair(1, &m_bank1_config),
		std::make_pair(2, &m_bank2_config),
		std::make_pair(3, &m_bank3_config)
	};
}

DEFINE_DEVICE_TYPE(ES5505, es5505_device, "es5505", "Ensoniq ES5505")

es5505_device::es5505_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: es550x_device(mconfig, ES5505, tag, owner, clock)
	, m_bank0_config("bank0", ENDIANNESS_BIG, 16, 20, -1) // 20 bit address bus, word addressing only
	, m_bank1_config("bank1", ENDIANNESS_BIG, 16, 20, -1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void es5505_device::device_start()
{
	es550x_device::device_start();
	int channels = 1;  // 1 channel by default, for backward compatibility

	// only override the number of channels if the value is in the valid range 1 .. 4
	if (1 <= m_channels && m_channels <= 4)
		channels = m_channels;

	// create the stream
	m_stream = stream_alloc(0, 2 * channels, clock() / (16*32));

	// initialize the regions
	if (m_region0 && !has_configured_map(0))
		space(0).install_rom(0, std::min<offs_t>((1 << ADDRESS_INTEGER_BIT_ES5505) - 1, (m_region0->bytes() / 2) - 1), m_region0->base());

	if (m_region1 && !has_configured_map(1))
		space(1).install_rom(0, std::min<offs_t>((1 << ADDRESS_INTEGER_BIT_ES5505) - 1, (m_region1->bytes() / 2) - 1), m_region1->base());

	for (int s = 0; s < 2; s++)
		space(s).cache(m_cache[s]);

	// compute the tables
	compute_tables(VOLUME_BIT_ES5505, 4, 4); // 4 bit exponent, 4 bit mantissa

	// initialize the rest of the structure
	m_channels = channels;

	// 20 bit integer and 9 bit fraction
	get_accum_mask(ADDRESS_INTEGER_BIT_ES5505, ADDRESS_FRAC_BIT_ES5505);

	// success
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector es5505_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(0, &m_bank0_config),
		std::make_pair(1, &m_bank1_config)
	};
}

/**********************************************************************************************

     update_irq_state -- update the IRQ state

***********************************************************************************************/


void es550x_device::update_irq_state()
{
	// ES5505/6 irq line has been set high - inform the host
	m_irq_cb(1); // IRQB set high
}

void es550x_device::update_internal_irq_state()
{
	/*  Host (cpu) has just read the voice interrupt vector (voice IRQ ack).

	    Reset the voice vector to show the IRQB line is low (top bit set).
	    If we have any stacked interrupts (other voices waiting to be
	    processed - with their IRQ bit set) then they will be moved into
	    the vector next time the voice is processed.  In emulation
	    terms they get updated next time generate_samples() is called.
	*/

	m_irqv = 0x80;

	m_irq_cb(0); // IRQB set low
}

/**********************************************************************************************

     compute_tables -- compute static tables

***********************************************************************************************/

void es550x_device::compute_tables(u32 total_volume_bit, u32 exponent_bit, u32 mantissa_bit)
{
	// allocate ulaw lookup table
	m_ulaw_lookup.resize(1 << ULAW_MAXBITS);

	// generate ulaw lookup table
	for (int i = 0; i < (1 << ULAW_MAXBITS); i++)
	{
		const u16 rawval = (i << (16 - ULAW_MAXBITS)) | (1 << (15 - ULAW_MAXBITS));
		const u8 exponent = rawval >> 13;
		u32 mantissa = (rawval << 3) & 0xffff;

		if (exponent == 0)
			m_ulaw_lookup[i] = s16(mantissa) >> 7;
		else
		{
			mantissa = (mantissa >> 1) | (~mantissa & 0x8000);
			m_ulaw_lookup[i] = s16(mantissa) >> (7 - exponent);
		}
	}

	const u32 volume_bit = (exponent_bit + mantissa_bit);
	m_volume_shift = total_volume_bit - volume_bit;
	const u32 volume_len = 1 << volume_bit;
	// allocate volume lookup table
	m_volume_lookup.resize(volume_len);

	// generate volume lookup table
	const u32 exponent_shift = 1 << exponent_bit;
	const u32 exponent_mask = exponent_shift - 1;

	const u32 mantissa_len = (1 << mantissa_bit);
	const u32 mantissa_mask = (mantissa_len - 1);
	const u32 mantissa_shift = exponent_shift - mantissa_bit - 1;

	for (int i = 0; i < volume_len; i++)
	{
		const u32 exponent = (i >> mantissa_bit) & exponent_mask;
		const u32 mantissa = (i & mantissa_mask) | mantissa_len;

		m_volume_lookup[i] = (mantissa << mantissa_shift) >> (exponent_shift - exponent);
	}
	m_volume_acc_shift = (16 + exponent_mask) - VOLUME_ACC_BIT;

	// init the voices
	for (int j = 0; j < 32; j++)
	{
		m_voice[j].index = j;
		m_voice[j].control = CONTROL_STOPMASK;
		m_voice[j].lvol = (1 << (total_volume_bit - 1));
		m_voice[j].rvol = (1 << (total_volume_bit - 1));
	}
}

/**********************************************************************************************

     get_accum_mask -- get address accumulator mask

***********************************************************************************************/

void es550x_device::get_accum_mask(u32 address_integer, u32 address_frac)
{
	m_address_acc_shift = ADDRESS_FRAC_BIT - address_frac;
	m_address_acc_mask = lshift_signed<u64, s8>(((((1 << address_integer) - 1) << address_frac) | ((1 << address_frac) - 1)), m_address_acc_shift);
	if (m_address_acc_shift > 0)
		m_address_acc_mask |= ((1 << m_address_acc_shift) - 1);
}


/**********************************************************************************************

     interpolate -- interpolate between two samples

***********************************************************************************************/

inline s32 es550x_device::interpolate(s32 sample1, s32 sample2, u64 accum)
{
	const u32 shifted = 1 << ADDRESS_FRAC_BIT;
	const u32 mask = shifted - 1;
	accum &= mask & m_address_acc_mask;
	return (sample1 * (s32)(shifted - accum) +
			sample2 * (s32)(accum)) >> ADDRESS_FRAC_BIT;
}


/**********************************************************************************************

     apply_filters -- apply the 4-pole digital filter to the sample

***********************************************************************************************/

// apply lowpass/highpass result
static inline s32 apply_lowpass(s32 out, s32 cutoff, s32 in)
{
	return ((s32)(cutoff >> FILTER_SHIFT) * (out - in) / (1 << FILTER_BIT)) + in;
}

static inline s32 apply_highpass(s32 out, s32 cutoff, s32 in, s32 prev)
{
	return out - prev + ((s32)(cutoff >> FILTER_SHIFT) * in) / (1 << (FILTER_BIT + 1)) + in / 2;
}

// update poles from outputs
static inline void update_pole(s32 &pole, s32 sample)
{
	pole = sample;
}

static inline void update_2_pole(s32 &prev, s32 &pole, s32 sample)
{
	prev = pole;
	pole = sample;
}

inline void es550x_device::apply_filters(es550x_voice *voice, s32 &sample)
{
	// pole 1 is always low-pass using K1
	sample = apply_lowpass(sample, voice->k1, voice->o1n1);
	update_pole(voice->o1n1, sample);

	// pole 2 is always low-pass using K1
	sample = apply_lowpass(sample, voice->k1, voice->o2n1);
	update_2_pole(voice->o2n2, voice->o2n1, sample);

	// remaining poles depend on the current filter setting
	switch (get_lp(voice->control))
	{
		case 0:
			// pole 3 is high-pass using K2
			sample = apply_highpass(sample, voice->k2, voice->o3n1, voice->o2n2);
			update_2_pole(voice->o3n2, voice->o3n1, sample);

			// pole 4 is high-pass using K2
			sample = apply_highpass(sample, voice->k2, voice->o4n1, voice->o3n2);
			update_pole(voice->o4n1, sample);
			break;

		case LP3:
			// pole 3 is low-pass using K1
			sample = apply_lowpass(sample, voice->k1, voice->o3n1);
			update_2_pole(voice->o3n2, voice->o3n1, sample);

			// pole 4 is high-pass using K2
			sample = apply_highpass(sample, voice->k2, voice->o4n1, voice->o3n2);
			update_pole(voice->o4n1, sample);
			break;

		case LP4:
			// pole 3 is low-pass using K2
			sample = apply_lowpass(sample, voice->k2, voice->o3n1);
			update_2_pole(voice->o3n2, voice->o3n1, sample);

			// pole 4 is low-pass using K2
			sample = apply_lowpass(sample, voice->k2, voice->o4n1);
			update_pole(voice->o4n1, sample);
			break;

		case LP3 | LP4:
			// pole 3 is low-pass using K1
			sample = apply_lowpass(sample, voice->k1, voice->o3n1);
			update_2_pole(voice->o3n2, voice->o3n1, sample);

			// pole 4 is low-pass using K2
			sample = apply_lowpass(sample, voice->k2, voice->o4n1);
			update_pole(voice->o4n1, sample);
			break;
	}
}


/**********************************************************************************************

     update_envelopes -- update the envelopes

***********************************************************************************************/

inline void es5506_device::update_envelopes(es550x_voice *voice)
{
	const u32 volume_max = (1 << VOLUME_BIT_ES5506) - 1;
	// decrement the envelope counter
	voice->ecount--;

	// ramp left volume
	if (voice->lvramp)
	{
		voice->lvol += (int8_t)voice->lvramp;
		if ((s32)voice->lvol < 0) voice->lvol = 0;
		else if (voice->lvol > volume_max) voice->lvol = volume_max;
	}

	// ramp right volume
	if (voice->rvramp)
	{
		voice->rvol += (int8_t)voice->rvramp;
		if ((s32)voice->rvol < 0) voice->rvol = 0;
		else if (voice->rvol > volume_max) voice->rvol = volume_max;
	}

	// ramp k1 filter constant
	if (voice->k1ramp && ((s32)voice->k1ramp >= 0 || !(voice->filtcount & 7)))
	{
		voice->k1 += (int8_t)voice->k1ramp;
		if ((s32)voice->k1 < 0) voice->k1 = 0;
		else if (voice->k1 > 0xffff) voice->k1 = 0xffff;
	}

	// ramp k2 filter constant
	if (voice->k2ramp && ((s32)voice->k2ramp >= 0 || !(voice->filtcount & 7)))
	{
		voice->k2 += (int8_t)voice->k2ramp;
		if ((s32)voice->k2 < 0) voice->k2 = 0;
		else if (voice->k2 > 0xffff) voice->k2 = 0xffff;
	}

	// update the filter constant counter
	voice->filtcount++;

}

inline void es5505_device::update_envelopes(es550x_voice *voice)
{
	// no envelopes in ES5505
	voice->ecount = 0;
}


/**********************************************************************************************

     check_for_end_forward
     check_for_end_reverse -- check for loop end and loop appropriately

***********************************************************************************************/

inline void es5506_device::check_for_end_forward(es550x_voice *voice, u64 &accum)
{
	// are we past the end?
	if (accum > voice->end && !(voice->control & CONTROL_LEI))
	{
		// generate interrupt if required
		if (voice->control & CONTROL_IRQE)
			voice->control |= CONTROL_IRQ;

		// handle the different types of looping
		switch (voice->control & CONTROL_LOOPMASK)
		{
			// non-looping
			case 0:
				voice->control |= CONTROL_STOP0;
				break;

			// uni-directional looping
			case CONTROL_LPE:
				accum = (voice->start + (accum - voice->end)) & m_address_acc_mask;
				break;

			// trans-wave looping
			case CONTROL_BLE:
				accum = (voice->start + (accum - voice->end)) & m_address_acc_mask;
				voice->control = (voice->control & ~CONTROL_LOOPMASK) | CONTROL_LEI;
				break;

			// bi-directional looping
			case CONTROL_LPE | CONTROL_BLE:
				accum = (voice->end - (accum - voice->end)) & m_address_acc_mask;
				voice->control ^= CONTROL_DIR;
				break;
		}
	}
}

inline void es5506_device::check_for_end_reverse(es550x_voice *voice, u64 &accum)
{
	// are we past the end?
	if (accum < voice->start && !(voice->control & CONTROL_LEI))
	{
		// generate interrupt if required
		if (voice->control & CONTROL_IRQE)
			voice->control |= CONTROL_IRQ;

		// handle the different types of looping
		switch (voice->control & CONTROL_LOOPMASK)
		{
			// non-looping
			case 0:
				voice->control |= CONTROL_STOP0;
				break;

			// uni-directional looping
			case CONTROL_LPE:
				accum = (voice->end - (voice->start - accum)) & m_address_acc_mask;
				break;

			// trans-wave looping
			case CONTROL_BLE:
				accum = (voice->end - (voice->start - accum)) & m_address_acc_mask;
				voice->control = (voice->control & ~CONTROL_LOOPMASK) | CONTROL_LEI;
				break;

			// bi-directional looping
			case CONTROL_LPE | CONTROL_BLE:
				accum = (voice->start + (voice->start - accum)) & m_address_acc_mask;
				voice->control ^= CONTROL_DIR;
				break;
		}
	}
}

// ES5505 : BLE is ignored when LPE = 0
inline void es5505_device::check_for_end_forward(es550x_voice *voice, u64 &accum)
{
	// are we past the end?
	if (accum > voice->end)
	{
		// generate interrupt if required
		if (voice->control & CONTROL_IRQE)
			voice->control |= CONTROL_IRQ;

		// handle the different types of looping
		switch (voice->control & CONTROL_LOOPMASK)
		{
			// non-looping
			case 0:
			case CONTROL_BLE:
				voice->control |= CONTROL_STOP0;
				break;

			// uni-directional looping
			case CONTROL_LPE:
				accum = (voice->start + (accum - voice->end)) & m_address_acc_mask;
				break;

			// bi-directional looping
			case CONTROL_LPE | CONTROL_BLE:
				accum = (voice->end - (accum - voice->end)) & m_address_acc_mask;
				voice->control ^= CONTROL_DIR;
				break;
		}
	}
}

inline void es5505_device::check_for_end_reverse(es550x_voice *voice, u64 &accum)
{
	// are we past the end?
	if (accum < voice->start)
	{
		// generate interrupt if required
		if (voice->control & CONTROL_IRQE)
			voice->control |= CONTROL_IRQ;

		// handle the different types of looping
		switch (voice->control & CONTROL_LOOPMASK)
		{
			// non-looping
			case 0:
			case CONTROL_BLE:
				voice->control |= CONTROL_STOP0;
				break;

			// uni-directional looping
			case CONTROL_LPE:
				accum = (voice->end - (voice->start - accum)) & m_address_acc_mask;
				break;

			// bi-directional looping
			case CONTROL_LPE | CONTROL_BLE:
				accum = (voice->start + (voice->start - accum)) & m_address_acc_mask;
				voice->control ^= CONTROL_DIR;
				break;
		}
	}
}


/**********************************************************************************************

     generate_ulaw -- general u-law decoding routine

***********************************************************************************************/

void es550x_device::generate_ulaw(es550x_voice *voice, s32 *dest)
{
	const u32 freqcount = voice->freqcount;
	u64 accum = voice->accum & m_address_acc_mask;

	// outer loop, in case we switch directions
	if (!(voice->control & CONTROL_STOPMASK))
	{
		// two cases: first case is forward direction
		if (!(voice->control & CONTROL_DIR))
		{
			// fetch two samples
			s32 val1 = read_sample(voice, get_integer_addr(accum));
			s32 val2 = read_sample(voice, get_integer_addr(accum, 1));

			// decompress u-law
			val1 = m_ulaw_lookup[val1 >> (16 - ULAW_MAXBITS)];
			val2 = m_ulaw_lookup[val2 >> (16 - ULAW_MAXBITS)];

			// interpolate
			val1 = interpolate(val1, val2, accum);
			accum = (accum + freqcount) & m_address_acc_mask;

			// apply filters
			apply_filters(voice, val1);

			// update filters/volumes
			if (voice->ecount != 0)
				update_envelopes(voice);

			// apply volumes and add
			dest[0] += get_sample(val1, voice->lvol);
			dest[1] += get_sample(val1, voice->rvol);

			// check for loop end
			check_for_end_forward(voice, accum);
		}

		// two cases: second case is backward direction
		else
		{
			// fetch two samples
			s32 val1 = read_sample(voice, get_integer_addr(accum));
			s32 val2 = read_sample(voice, get_integer_addr(accum, 1));

			// decompress u-law
			val1 = m_ulaw_lookup[val1 >> (16 - ULAW_MAXBITS)];
			val2 = m_ulaw_lookup[val2 >> (16 - ULAW_MAXBITS)];

			// interpolate
			val1 = interpolate(val1, val2, accum);
			accum = (accum - freqcount) & m_address_acc_mask;

			// apply filters
			apply_filters(voice, val1);

			// update filters/volumes
			if (voice->ecount != 0)
				update_envelopes(voice);

			// apply volumes and add
			dest[0] += get_sample(val1, voice->lvol);
			dest[1] += get_sample(val1, voice->rvol);

			// check for loop end
			check_for_end_reverse(voice, accum);
		}
	}
	else
	{
		// if we stopped, process any additional envelope
		if (voice->ecount != 0)
			update_envelopes(voice);
	}

	voice->accum = accum;
}



/**********************************************************************************************

     generate_pcm -- general PCM decoding routine

***********************************************************************************************/

void es550x_device::generate_pcm(es550x_voice *voice, s32 *dest)
{
	const u32 freqcount = voice->freqcount;
	u64 accum = voice->accum & m_address_acc_mask;

	// outer loop, in case we switch directions
	if (!(voice->control & CONTROL_STOPMASK))
	{
		// two cases: first case is forward direction
		if (!(voice->control & CONTROL_DIR))
		{
			// fetch two samples
			s32 val1 = (s16)read_sample(voice, get_integer_addr(accum));
			s32 val2 = (s16)read_sample(voice, get_integer_addr(accum, 1));

			// interpolate
			val1 = interpolate(val1, val2, accum);
			accum = (accum + freqcount) & m_address_acc_mask;

			// apply filters
			apply_filters(voice, val1);

			// update filters/volumes
			if (voice->ecount != 0)
				update_envelopes(voice);

			// apply volumes and add
			dest[0] += get_sample(val1, voice->lvol);
			dest[1] += get_sample(val1, voice->rvol);

			// check for loop end
			check_for_end_forward(voice, accum);
		}

		// two cases: second case is backward direction
		else
		{
			// fetch two samples
			s32 val1 = (s16)read_sample(voice, get_integer_addr(accum));
			s32 val2 = (s16)read_sample(voice, get_integer_addr(accum, 1));

			// interpolate
			val1 = interpolate(val1, val2, accum);
			accum = (accum - freqcount) & m_address_acc_mask;

			// apply filters
			apply_filters(voice, val1);

			// update filters/volumes
			if (voice->ecount != 0)
				update_envelopes(voice);

			// apply volumes and add
			dest[0] += get_sample(val1, voice->lvol);
			dest[1] += get_sample(val1, voice->rvol);

			// check for loop end
			check_for_end_reverse(voice, accum);
		}
	}
	else
	{
		// if we stopped, process any additional envelope
		if (voice->ecount != 0)
			update_envelopes(voice);
	}

	voice->accum = accum;
}

/**********************************************************************************************

     generate_irq -- general interrupt handling routine

***********************************************************************************************/

inline void es550x_device::generate_irq(es550x_voice *voice, int v)
{
	// does this voice have it's IRQ bit raised?
	if (voice->control & CONTROL_IRQ)
	{
		LOG("es5506: IRQ raised on voice %d!!\n",v);

		// only update voice vector if existing IRQ is acked by host
		if (m_irqv & 0x80)
		{
			// latch voice number into vector, and set high bit low
			m_irqv = v & 0x1f;

			// take down IRQ bit on voice
			voice->control &= ~CONTROL_IRQ;

			// inform host of irq
			update_irq_state();
		}
	}
}


/**********************************************************************************************

     generate_samples -- tell each voice to generate samples

***********************************************************************************************/

void es5506_device::generate_samples(sound_stream &stream)
{
	// loop while we still have samples to generate
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		// loop over voices
		s32 cursample[12] = { 0 };
		for (int v = 0; v <= m_active_voices; v++)
		{
			es550x_voice *voice = &m_voice[v];

			// special case: if end == start, stop the voice
			if (voice->start == voice->end)
				voice->control |= CONTROL_STOP0;

			const int voice_channel = get_ca(voice->control);
			const int channel = voice_channel % m_channels;
			const int l = channel << 1;

			// generate from the appropriate source
			if (voice->control & CONTROL_CMPD)
				generate_ulaw(voice, &cursample[l]);
			else
				generate_pcm(voice, &cursample[l]);

			// does this voice have it's IRQ bit raised?
			generate_irq(voice, v);
		}

		for (int c = 0; c < stream.output_count(); c++)
			stream.put_int(c, sampindex, cursample[c], 32768);
	}
}

void es5505_device::generate_samples(sound_stream &stream)
{
	// loop while we still have samples to generate
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		// loop over voices
		s32 cursample[12] = { 0 };
		for (int v = 0; v <= m_active_voices; v++)
		{
			es550x_voice *voice = &m_voice[v];

// This special case does not appear to match the behaviour observed in the es5505 in
// actual Ensoniq synthesizers: those, it turns out, do set loop start and end to the
// same value, and expect the voice to keep running. Examples can be found among the
// transwaves on the VFX / SD-1 series of synthesizers.
#if 0
			// special case: if end == start, stop the voice
			if (voice->start == voice->end)
				voice->control |= CONTROL_STOP0;
#endif

			const int voice_channel = get_ca(voice->control);
			const int channel = voice_channel % m_channels;
			const int l = channel << 1;

			// generate from the appropriate source
			// no compressed sample support
			generate_pcm(voice, &cursample[l]);

			// does this voice have it's IRQ bit raised?
			generate_irq(voice, v);
		}

		for (int c = 0; c < stream.output_count(); c++)
			stream.put_int(c, sampindex, cursample[c], 32768);
	}
}



/**********************************************************************************************

     reg_write -- handle a write to the selected ES5506 register

***********************************************************************************************/

inline void es5506_device::reg_write_low(es550x_voice *voice, offs_t offset, u32 data)
{
	switch (offset)
	{
		case 0x00/8:    // CR
			voice->control = data & 0xffff;
			LOG("voice %d, control=%04x\n", m_current_page & 0x1f, voice->control);
			break;

		case 0x08/8:    // FC
			voice->freqcount = get_address_acc_shifted_val(data & 0x1ffff);
			LOG("voice %d, freq count=%08x\n", m_current_page & 0x1f, get_address_acc_res(voice->freqcount));
			break;

		case 0x10/8:    // LVOL
			voice->lvol = data & 0xffff; // low 4 bit is used for finer envelope control
			LOG("voice %d, left vol=%04x\n", m_current_page & 0x1f, voice->lvol);
			break;

		case 0x18/8:    // LVRAMP
			voice->lvramp = (data & 0xff00) >> 8;
			LOG("voice %d, left vol ramp=%04x\n", m_current_page & 0x1f, voice->lvramp);
			break;

		case 0x20/8:    // RVOL
			voice->rvol = data & 0xffff; // low 4 bit is used for finer envelope control
			LOG("voice %d, right vol=%04x\n", m_current_page & 0x1f, voice->rvol);
			break;

		case 0x28/8:    // RVRAMP
			voice->rvramp = (data & 0xff00) >> 8;
			LOG("voice %d, right vol ramp=%04x\n", m_current_page & 0x1f, voice->rvramp);
			break;

		case 0x30/8:    // ECOUNT
			voice->ecount = data & 0x1ff;
			voice->filtcount = 0;
			LOG("voice %d, envelope count=%04x\n", m_current_page & 0x1f, voice->ecount);
			break;

		case 0x38/8:    // K2
			voice->k2 = data & 0xffff; // low 4 bit is used for finer envelope control
			LOG("voice %d, K2=%04x\n", m_current_page & 0x1f, voice->k2);
			break;

		case 0x40/8:    // K2RAMP
			voice->k2ramp = ((data & 0xff00) >> 8) | ((data & 0x0001) << 31);
			LOG("voice %d, K2 ramp=%04x\n", m_current_page & 0x1f, voice->k2ramp);
			break;

		case 0x48/8:    // K1
			voice->k1 = data & 0xffff; // low 4 bit is used for finer envelope control
			LOG("voice %d, K1=%04x\n", m_current_page & 0x1f, voice->k1);
			break;

		case 0x50/8:    // K1RAMP
			voice->k1ramp = ((data & 0xff00) >> 8) | ((data & 0x0001) << 31);
			LOG("voice %d, K1 ramp=%04x\n", m_current_page & 0x1f, voice->k1ramp);
			break;

		case 0x58/8:    // ACTV
		{
			m_active_voices = data & 0x1f;
			m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
			m_stream->set_sample_rate(m_sample_rate);
			m_sample_rate_changed_cb(m_sample_rate);

			LOG("active voices=%d, sample_rate=%d\n", m_active_voices, m_sample_rate);
			break;
		}

		case 0x60/8:    // MODE
			// [4:3] = 00 : Single, Master, Early address mode
			// [4:3] = 01 : Single, Master, Normal address mode
			// [4:3] = 10 : Dual, Slave, Normal address mode
			// [4:3] = 11 : Dual, Master, Normal address mode
			m_mode = data & 0x1f; // MODE1[4], MODE0[3], BCLK_EN[2], WCLK_EN[1], LRCLK_EN[0]
			break;

		case 0x68/8:    // PAR - read only
		case 0x70/8:    // IRQV - read only
			break;

		case 0x78/8:    // PAGE
			m_current_page = data & 0x7f;
			break;
	}
}

inline void es5506_device::reg_write_high(es550x_voice *voice, offs_t offset, u32 data)
{
	switch (offset)
	{
		case 0x00/8:    // CR
			voice->control = data & 0xffff;
			LOG("voice %d, control=%04x\n", m_current_page & 0x1f, voice->control);
			break;

		case 0x08/8:    // START
			voice->start = get_address_acc_shifted_val(data & 0xfffff800);
			LOG("voice %d, loop start=%08x\n", m_current_page & 0x1f, get_address_acc_res(voice->start));
			break;

		case 0x10/8:    // END
			voice->end = get_address_acc_shifted_val(data & 0xffffff80);
			LOG("voice %d, loop end=%08x\n", m_current_page & 0x1f, get_address_acc_res(voice->end));
			break;

		case 0x18/8:    // ACCUM
			voice->accum = get_address_acc_shifted_val(data);
			LOG("voice %d, accum=%08x\n", m_current_page & 0x1f, get_address_acc_res(voice->accum));
			break;

		case 0x20/8:    // O4(n-1); TODO: 16.1 signed fixed point according to datasheet
			voice->o4n1 = util::sext(data, 18);
			LOG("voice %d, O4(n-1)=%05x\n", m_current_page & 0x1f, voice->o4n1 & 0x3ffff);
			break;

		case 0x28/8:    // O3(n-1)
			voice->o3n1 = util::sext(data, 18);
			LOG("voice %d, O3(n-1)=%05x\n", m_current_page & 0x1f, voice->o3n1 & 0x3ffff);
			break;

		case 0x30/8:    // O3(n-2)
			voice->o3n2 = util::sext(data, 18);
			LOG("voice %d, O3(n-2)=%05x\n", m_current_page & 0x1f, voice->o3n2 & 0x3ffff);
			break;

		case 0x38/8:    // O2(n-1)
			voice->o2n1 = util::sext(data, 18);
			LOG("voice %d, O2(n-1)=%05x\n", m_current_page & 0x1f, voice->o2n1 & 0x3ffff);
			break;

		case 0x40/8:    // O2(n-2)
			voice->o2n2 = util::sext(data, 18);
			LOG("voice %d, O2(n-2)=%05x\n", m_current_page & 0x1f, voice->o2n2 & 0x3ffff);
			break;

		case 0x48/8:    // O1(n-1)
			voice->o1n1 = util::sext(data, 18);
			LOG("voice %d, O1(n-1)=%05x\n", m_current_page & 0x1f, voice->o1n1 & 0x3ffff);
			break;

		case 0x50/8:    // W_ST
			m_wst = data & 0x7f;
			LOGMASKED(LOG_SERIAL, "%s: word clock start = %02x\n", machine().describe_context(), m_wst);
			break;

		case 0x58/8:    // W_END
			m_wend = data & 0x7f;
			LOGMASKED(LOG_SERIAL, "%s: word clock end = %02x\n", machine().describe_context(), m_wend);
			break;

		case 0x60/8:    // LR_END
			m_lrend = data & 0x7f;
			LOGMASKED(LOG_SERIAL, "%s: left/right clock end = %02x\n", machine().describe_context(), m_lrend);
			break;

		case 0x68/8:    // PAR - read only
		case 0x70/8:    // IRQV - read only
			break;

		case 0x78/8:    // PAGE
			m_current_page = data & 0x7f;
			break;
	}
}

inline void es5506_device::reg_write_test(es550x_voice *voice, offs_t offset, u32 data)
{
	switch (offset)
	{
		case 0x00/8:    // CHANNEL 0 LEFT
			LOG("Channel 0 left test write %08x\n", data);
			break;

		case 0x08/8:    // CHANNEL 0 RIGHT
			LOG("Channel 0 right test write %08x\n", data);
			break;

		case 0x10/8:    // CHANNEL 1 LEFT
			LOG("Channel 1 left test write %08x\n", data);
			break;

		case 0x18/8:    // CHANNEL 1 RIGHT
			LOG("Channel 1 right test write %08x\n", data);
			break;

		case 0x20/8:    // CHANNEL 2 LEFT
			LOG("Channel 2 left test write %08x\n", data);
			break;

		case 0x28/8:    // CHANNEL 2 RIGHT
			LOG("Channel 2 right test write %08x\n", data);
			break;

		case 0x30/8:    // CHANNEL 3 LEFT
			LOG("Channel 3 left test write %08x\n", data);
			break;

		case 0x38/8:    // CHANNEL 3 RIGHT
			LOG("Channel 3 right test write %08x\n", data);
			break;

		case 0x40/8:    // CHANNEL 4 LEFT
			LOG("Channel 4 left test write %08x\n", data);
			break;

		case 0x48/8:    // CHANNEL 4 RIGHT
			LOG("Channel 4 right test write %08x\n", data);
			break;

		case 0x50/8:    // CHANNEL 5 LEFT
			LOG("Channel 5 left test write %08x\n", data);
			break;

		case 0x58/8:    // CHANNEL 6 RIGHT
			LOG("Channel 5 right test write %08x\n", data);
			break;

		case 0x60/8:    // EMPTY
			LOG("Test write EMPTY %08x\n", data);
			break;

		case 0x68/8:    // PAR - read only
		case 0x70/8:    // IRQV - read only
			break;

		case 0x78/8:    // PAGE
			m_current_page = data & 0x7f;
			break;
	}
}

void es5506_device::write(offs_t offset, u8 data)
{
	es550x_voice *voice = &m_voice[m_current_page & 0x1f];
	int shift = 8 * (offset & 3);

	// accumulate the data
	m_write_latch = (m_write_latch & ~(0xff000000 >> shift)) | (data << (24 - shift));

	// wait for a write to complete
	if (shift != 24)
		return;

	// force an update
	m_stream->update();

	// switch off the page and register
	if (m_current_page < 0x20)
		reg_write_low(voice, offset / 4, m_write_latch);
	else if (m_current_page < 0x40)
		reg_write_high(voice, offset / 4, m_write_latch);
	else
		reg_write_test(voice, offset / 4, m_write_latch);

	// clear the write latch when done
	m_write_latch = 0;
}



/**********************************************************************************************

     reg_read -- read from the specified ES5506 register

***********************************************************************************************/

inline u32 es5506_device::reg_read_low(es550x_voice *voice, offs_t offset)
{
	u32 result = 0;

	switch (offset)
	{
		case 0x00/8:    // CR
			result = voice->control;
			break;

		case 0x08/8:    // FC
			result = get_address_acc_res(voice->freqcount);
			break;

		case 0x10/8:    // LVOL
			result = voice->lvol;
			break;

		case 0x18/8:    // LVRAMP
			result = voice->lvramp << 8;
			break;

		case 0x20/8:    // RVOL
			result = voice->rvol;
			break;

		case 0x28/8:    // RVRAMP
			result = voice->rvramp << 8;
			break;

		case 0x30/8:    // ECOUNT
			result = voice->ecount;
			break;

		case 0x38/8:    // K2
			result = voice->k2;
			break;

		case 0x40/8:    // K2RAMP
			result = (voice->k2ramp << 8) | (voice->k2ramp >> 31);
			break;

		case 0x48/8:    // K1
			result = voice->k1;
			break;

		case 0x50/8:    // K1RAMP
			result = (voice->k1ramp << 8) | (voice->k1ramp >> 31);
			break;

		case 0x58/8:    // ACTV
			result = m_active_voices;
			break;

		case 0x60/8:    // MODE
			result = m_mode;
			break;

		case 0x68/8:    // PAR
			if (!m_read_port_cb.isunset())
				result = m_read_port_cb(0) & 0x3ff; // 10 bit, 9:0
			break;

		case 0x70/8:    // IRQV
			result = m_irqv;
			if (!machine().side_effects_disabled())
				update_internal_irq_state();
			break;

		case 0x78/8:    // PAGE
			result = m_current_page;
			break;
	}
	return result;
}


inline u32 es5506_device::reg_read_high(es550x_voice *voice, offs_t offset)
{
	u32 result = 0;

	switch (offset)
	{
		case 0x00/8:    // CR
			result = voice->control;
			break;

		case 0x08/8:    // START
			result = get_address_acc_res(voice->start);
			break;

		case 0x10/8:    // END
			result = get_address_acc_res(voice->end);
			break;

		case 0x18/8:    // ACCUM
			result = get_address_acc_res(voice->accum);
			break;

		case 0x20/8:    // O4(n-1); TODO: 16.1 signed fixed point according to datasheet
			result = voice->o4n1 & 0x3ffff;
			break;

		case 0x28/8:    // O3(n-1)
			result = voice->o3n1 & 0x3ffff;
			break;

		case 0x30/8:    // O3(n-2)
			result = voice->o3n2 & 0x3ffff;
			break;

		case 0x38/8:    // O2(n-1)
			result = voice->o2n1 & 0x3ffff;
			break;

		case 0x40/8:    // O2(n-2)
			result = voice->o2n2 & 0x3ffff;
			break;

		case 0x48/8:    // O1(n-1)
			result = voice->o1n1 & 0x3ffff;
			break;

		case 0x50/8:    // W_ST
			result = m_wst;
			break;

		case 0x58/8:    // W_END
			result = m_wend;
			break;

		case 0x60/8:    // LR_END
			result = m_lrend;
			break;

		case 0x68/8:    // PAR
			if (!m_read_port_cb.isunset())
				result = m_read_port_cb(0) & 0x3ff; // 10 bit, 9:0
			break;

		case 0x70/8:    // IRQV
			result = m_irqv;
			if (!machine().side_effects_disabled())
				update_internal_irq_state();
			break;

		case 0x78/8:    // PAGE
			result = m_current_page;
			break;
	}
	return result;
}
inline u32 es5506_device::reg_read_test(es550x_voice *voice, offs_t offset)
{
	u32 result = 0;

	switch (offset)
	{
		case 0x68/8:    // PAR
			if (!m_read_port_cb.isunset())
				result = m_read_port_cb(0) & 0x3ff; // 10 bit, 9:0
			break;

		case 0x70/8:    // IRQV
			result = m_irqv;
			break;

		case 0x78/8:    // PAGE
			result = m_current_page;
			break;
	}
	return result;
}

u8 es5506_device::read(offs_t offset)
{
	es550x_voice *voice = &m_voice[m_current_page & 0x1f];
	int shift = 8 * (offset & 3);

	// only read on offset 0
	if (shift != 0)
		return m_read_latch >> (24 - shift);

	LOG("read from %02x/%02x -> ", m_current_page, offset / 4 * 8);

	// force an update
	m_stream->update();

	// switch off the page and register
	if (m_current_page < 0x20)
		m_read_latch = reg_read_low(voice, offset / 4);
	else if (m_current_page < 0x40)
		m_read_latch = reg_read_high(voice, offset / 4);
	else
		m_read_latch = reg_read_test(voice, offset / 4);

	LOG("%08x\n", m_read_latch);

	// return the high byte
	return m_read_latch >> 24;
}


/**********************************************************************************************

     reg_write -- handle a write to the selected ES5505 register

***********************************************************************************************/

inline void es5505_device::reg_write_low(es550x_voice *voice, offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0x00:  // CR
			voice->control |= 0xf000; // bit 15-12 always 1
			if (ACCESSING_BITS_0_7)
			{
#if RAINE_CHECK
				voice->control &= ~(CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_DIR);
#else
				voice->control &= ~0x00ff;
#endif
				voice->control |= (data & 0x00ff);
			}
			if (ACCESSING_BITS_8_15)
				voice->control = (voice->control & ~0x0f00) | (data & 0x0f00);

			LOG("%s:voice %d, control=%04x (raw=%04x & %04x)\n", machine().describe_context(), m_current_page & 0x1f, voice->control, data, mem_mask ^ 0xffff);
			break;

		case 0x01:  // FC
			if (ACCESSING_BITS_0_7)
				voice->freqcount = (voice->freqcount & ~get_address_acc_shifted_val(0x00fe, 1)) | (get_address_acc_shifted_val((data & 0x00fe), 1));
			if (ACCESSING_BITS_8_15)
				voice->freqcount = (voice->freqcount & ~get_address_acc_shifted_val(0xff00, 1)) | (get_address_acc_shifted_val((data & 0xff00), 1));
			LOG("%s:voice %d, freq count=%08x\n", machine().describe_context(), m_current_page & 0x1f, get_address_acc_res(voice->freqcount, 1));
			break;

		case 0x02:  // STRT (hi)
			if (ACCESSING_BITS_0_7)
				voice->start = (voice->start & ~get_address_acc_shifted_val(0x00ff0000)) | (get_address_acc_shifted_val((data & 0x00ff) << 16));
			if (ACCESSING_BITS_8_15)
				voice->start = (voice->start & ~get_address_acc_shifted_val(0x1f000000)) | (get_address_acc_shifted_val((data & 0x1f00) << 16));
			LOG("%s:voice %d, loop start=%08x\n", machine().describe_context(), m_current_page & 0x1f, get_address_acc_res(voice->start));
			break;

		case 0x03:  // STRT (lo)
			if (ACCESSING_BITS_0_7)
				voice->start = (voice->start & ~get_address_acc_shifted_val(0x000000e0)) | (get_address_acc_shifted_val(data & 0x00e0));
			if (ACCESSING_BITS_8_15)
				voice->start = (voice->start & ~get_address_acc_shifted_val(0x0000ff00)) | (get_address_acc_shifted_val(data & 0xff00));
			LOG("%s:voice %d, loop start=%08x\n", machine().describe_context(), m_current_page & 0x1f, get_address_acc_res(voice->start));
			break;

		case 0x04:  // END (hi)
			if (ACCESSING_BITS_0_7)
				voice->end = (voice->end & ~get_address_acc_shifted_val(0x00ff0000)) | (get_address_acc_shifted_val((data & 0x00ff) << 16));
			if (ACCESSING_BITS_8_15)
				voice->end = (voice->end & ~get_address_acc_shifted_val(0x1f000000)) | (get_address_acc_shifted_val((data & 0x1f00) << 16));
#if RAINE_CHECK
			voice->control |= CONTROL_STOP0;
#endif
			LOG("%s:voice %d, loop end=%08x\n", machine().describe_context(), m_current_page & 0x1f, get_address_acc_res(voice->end));
			break;

		case 0x05:  // END (lo)
			if (ACCESSING_BITS_0_7)
				voice->end = (voice->end & ~get_address_acc_shifted_val(0x000000e0)) | (get_address_acc_shifted_val(data & 0x00e0));
			if (ACCESSING_BITS_8_15)
				voice->end = (voice->end & ~get_address_acc_shifted_val(0x0000ff00)) | (get_address_acc_shifted_val(data & 0xff00));
#if RAINE_CHECK
			voice->control |= CONTROL_STOP0;
#endif
			LOG("%s:voice %d, loop end=%08x\n", machine().describe_context(), m_current_page & 0x1f, get_address_acc_res(voice->end));
			break;

		case 0x06:  // K2
			if (ACCESSING_BITS_0_7)
				voice->k2 = (voice->k2 & ~0x00f0) | (data & 0x00f0);
			if (ACCESSING_BITS_8_15)
				voice->k2 = (voice->k2 & ~0xff00) | (data & 0xff00);
			LOG("%s:voice %d, K2=%03x\n", machine().describe_context(), m_current_page & 0x1f, voice->k2 >> FILTER_SHIFT);
			break;

		case 0x07:  // K1
			if (ACCESSING_BITS_0_7)
				voice->k1 = (voice->k1 & ~0x00f0) | (data & 0x00f0);
			if (ACCESSING_BITS_8_15)
				voice->k1 = (voice->k1 & ~0xff00) | (data & 0xff00);
			LOG("%s:voice %d, K1=%03x\n", machine().describe_context(), m_current_page & 0x1f, voice->k1 >> FILTER_SHIFT);
			break;

		case 0x08:  // LVOL
			if (ACCESSING_BITS_8_15)
				voice->lvol = (voice->lvol & ~0xff) | ((data & 0xff00) >> 8);
			LOG("%s:voice %d, left vol=%02x\n", machine().describe_context(), m_current_page & 0x1f, voice->lvol);
			break;

		case 0x09:  // RVOL
			if (ACCESSING_BITS_8_15)
				voice->rvol = (voice->rvol & ~0xff) | ((data & 0xff00) >> 8);
			LOG("%s:voice %d, right vol=%02x\n", machine().describe_context(), m_current_page & 0x1f, voice->rvol);
			break;

		case 0x0a:  // ACC (hi)
			if (ACCESSING_BITS_0_7)
				voice->accum = (voice->accum & ~get_address_acc_shifted_val(0x00ff0000)) | (get_address_acc_shifted_val((data & 0x00ff) << 16));
			if (ACCESSING_BITS_8_15)
				voice->accum = (voice->accum & ~get_address_acc_shifted_val(0x1f000000)) | (get_address_acc_shifted_val((data & 0x1f00) << 16));
			LOG("%s:voice %d, accum=%08x\n", machine().describe_context(), m_current_page & 0x1f, get_address_acc_res(voice->accum));
			break;

		case 0x0b:  // ACC (lo)
			if (ACCESSING_BITS_0_7)
				voice->accum = (voice->accum & ~get_address_acc_shifted_val(0x000000ff)) | (get_address_acc_shifted_val(data & 0x00ff));
			if (ACCESSING_BITS_8_15)
				voice->accum = (voice->accum & ~get_address_acc_shifted_val(0x0000ff00)) | (get_address_acc_shifted_val(data & 0xff00));
			LOG("%s:voice %d, accum=%08x\n", machine().describe_context(), m_current_page & 0x1f, get_address_acc_res(voice->accum));
			break;

		case 0x0c:  // unused
			break;

		case 0x0d:  // ACT
			if (ACCESSING_BITS_0_7)
			{
				m_active_voices = data & 0x1f;
				m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
				m_stream->set_sample_rate(m_sample_rate);
				m_sample_rate_changed_cb(m_sample_rate);

				LOG("active voices=%d, sample_rate=%d\n", m_active_voices, m_sample_rate);
			}
			break;

		case 0x0e:  // IRQV - read only
			break;

		case 0x0f:  // PAGE
			if (ACCESSING_BITS_0_7)
				m_current_page = data & 0x7f;
			break;
	}
}


inline void es5505_device::reg_write_high(es550x_voice *voice, offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0x00:  // CR
			voice->control |= 0xf000; // bit 15-12 always 1
			if (ACCESSING_BITS_0_7)
				voice->control = (voice->control & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->control = (voice->control & ~0x0f00) | (data & 0x0f00);

			LOG("%s:voice %d, control=%04x (raw=%04x & %04x)\n", machine().describe_context(), m_current_page & 0x1f, voice->control, data, mem_mask);
			break;

		case 0x01:  // O4(n-1)
			if (ACCESSING_BITS_0_7)
				voice->o4n1 = (voice->o4n1 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o4n1 = (s16)((voice->o4n1 & ~0xff00) | (data & 0xff00));
			LOG("%s:voice %d, O4(n-1)=%04x\n", machine().describe_context(), m_current_page & 0x1f, voice->o4n1 & 0xffff);
			break;

		case 0x02:  // O3(n-1)
			if (ACCESSING_BITS_0_7)
				voice->o3n1 = (voice->o3n1 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o3n1 = (s16)((voice->o3n1 & ~0xff00) | (data & 0xff00));
			LOG("%s:voice %d, O3(n-1)=%04x\n", machine().describe_context(), m_current_page & 0x1f, voice->o3n1 & 0xffff);
			break;

		case 0x03:  // O3(n-2)
			if (ACCESSING_BITS_0_7)
				voice->o3n2 = (voice->o3n2 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o3n2 = (s16)((voice->o3n2 & ~0xff00) | (data & 0xff00));
			LOG("%s:voice %d, O3(n-2)=%04x\n", machine().describe_context(), m_current_page & 0x1f, voice->o3n2 & 0xffff);
			break;

		case 0x04:  // O2(n-1)
			if (ACCESSING_BITS_0_7)
				voice->o2n1 = (voice->o2n1 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o2n1 = (s16)((voice->o2n1 & ~0xff00) | (data & 0xff00));
			LOG("%s:voice %d, O2(n-1)=%04x\n", machine().describe_context(), m_current_page & 0x1f, voice->o2n1 & 0xffff);
			break;

		case 0x05:  // O2(n-2)
			if (ACCESSING_BITS_0_7)
				voice->o2n2 = (voice->o2n2 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o2n2 = (s16)((voice->o2n2 & ~0xff00) | (data & 0xff00));
			LOG("%s:voice %d, O2(n-2)=%04x\n", machine().describe_context(), m_current_page & 0x1f, voice->o2n2 & 0xffff);
			break;

		case 0x06:  // O1(n-1)
			if (ACCESSING_BITS_0_7)
				voice->o1n1 = (voice->o1n1 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o1n1 = (s16)((voice->o1n1 & ~0xff00) | (data & 0xff00));
			LOG("%s:voice %d, O1(n-1)=%04x (accum=%08x)\n", machine().describe_context(), m_current_page & 0x1f, voice->o1n1 & 0xffff, get_address_acc_res(voice->accum));
			break;

		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:  // unused
			break;

		case 0x0d:  // ACT
			if (ACCESSING_BITS_0_7)
			{
				m_active_voices = data & 0x1f;
				m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
				m_stream->set_sample_rate(m_sample_rate);
				m_sample_rate_changed_cb(m_sample_rate);

				LOG("active voices=%d, sample_rate=%d\n", m_active_voices, m_sample_rate);
			}
			break;

		case 0x0e:  // IRQV - read only
			break;

		case 0x0f:  // PAGE
			if (ACCESSING_BITS_0_7)
				m_current_page = data & 0x7f;
			break;
	}
}


inline void es5505_device::reg_write_test(es550x_voice *voice, offs_t offset, u16 data, u16 mem_mask)
{
	switch (offset)
	{
		case 0x00:  // CH0L
		case 0x01:  // CH0R
		case 0x02:  // CH1L
		case 0x03:  // CH1R
		case 0x04:  // CH2L
		case 0x05:  // CH2R
		case 0x06:  // CH3L
		case 0x07:  // CH3R
			break;

		case 0x08:  // SERMODE
			m_mode |= 0x7f8; // bit 10-3 always 1
			if (ACCESSING_BITS_8_15)
				m_mode = (m_mode & ~0xf800) | (data & 0xf800); // MSB[4:0] (unknown purpose)
			if (ACCESSING_BITS_0_7)
				m_mode = (m_mode & ~0x0007) | (data & 0x0007); // SONY/BB, TEST, A/D
			LOGMASKED(LOG_SERIAL, "%s: serial mode = %04x & %04x", machine().describe_context(), m_mode, mem_mask);
			break;

		case 0x09:  // PAR
			break;

		case 0x0d:  // ACT
			if (ACCESSING_BITS_0_7)
			{
				m_active_voices = data & 0x1f;
				m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
				m_stream->set_sample_rate(m_sample_rate);
				m_sample_rate_changed_cb(m_sample_rate);

				LOG("active voices=%d, sample_rate=%d\n", m_active_voices, m_sample_rate);
			}
			break;

		case 0x0e:  // IRQV - read only
			break;

		case 0x0f:  // PAGE
			if (ACCESSING_BITS_0_7)
				m_current_page = data & 0x7f;
			break;
	}
}


void es5505_device::write(offs_t offset, u16 data, u16 mem_mask)
{
	es550x_voice *voice = &m_voice[m_current_page & 0x1f];

//  logerror("%s:ES5505 write %02x/%02x = %04x & %04x\n", machine().describe_context(), m_current_page, offset, data, mem_mask);

	// force an update
	m_stream->update();

	// switch off the page and register
	if (m_current_page < 0x20)
		reg_write_low(voice, offset, data, mem_mask);
	else if (m_current_page < 0x40)
		reg_write_high(voice, offset, data, mem_mask);
	else
		reg_write_test(voice, offset, data, mem_mask);
}



/**********************************************************************************************

     reg_read -- read from the specified ES5505 register

***********************************************************************************************/

inline u16 es5505_device::reg_read_low(es550x_voice *voice, offs_t offset)
{
	u16 result = 0;

	switch (offset)
	{
		case 0x00:  // CR
			result = voice->control | 0xf000;
			break;

		case 0x01:  // FC
			result = get_address_acc_res(voice->freqcount, 1);
			break;

		case 0x02:  // STRT (hi)
			result = get_address_acc_res(voice->start) >> 16;
			break;

		case 0x03:  // STRT (lo)
			result = get_address_acc_res(voice->start);
			break;

		case 0x04:  // END (hi)
			result = get_address_acc_res(voice->end) >> 16;
			break;

		case 0x05:  // END (lo)
			result = get_address_acc_res(voice->end);
			break;

		case 0x06:  // K2
			result = voice->k2;
			break;

		case 0x07:  // K1
			result = voice->k1;
			break;

		case 0x08:  // LVOL
			result = voice->lvol << 8;
			break;

		case 0x09:  // RVOL
			result = voice->rvol << 8;
			break;

		case 0x0a:  // ACC (hi)
			result = get_address_acc_res(voice->accum) >> 16;
			break;

		case 0x0b:  // ACC (lo)
			result = get_address_acc_res(voice->accum);
			break;

		case 0x0c:  // unused
			break;

		case 0x0d:  // ACT
			result = m_active_voices;
			break;

		case 0x0e:  // IRQV
			result = m_irqv;
			if (!machine().side_effects_disabled())
				update_internal_irq_state();
			break;

		case 0x0f:  // PAGE
			result = m_current_page;
			break;
	}
	return result;
}


inline u16 es5505_device::reg_read_high(es550x_voice *voice, offs_t offset)
{
	u16 result = 0;

	switch (offset)
	{
		case 0x00:  // CR
			result = voice->control | 0xf000;
			break;

		case 0x01:  // O4(n-1)
			result = voice->o4n1 & 0xffff;
			break;

		case 0x02:  // O3(n-1)
			result = voice->o3n1 & 0xffff;
			break;

		case 0x03:  // O3(n-2)
			result = voice->o3n2 & 0xffff;
			break;

		case 0x04:  // O2(n-1)
			result = voice->o2n1 & 0xffff;
			break;

		case 0x05:  // O2(n-2)
			result = voice->o2n2 & 0xffff;
			break;

		case 0x06:  // O1(n-1)
			// special case for the Taito F3 games: they set the accumulator on a stopped
			// voice and assume the filters continue to process the data. They then read
			// the O1(n-1) in order to extract raw data from the sound ROMs. Since we don't
			// want to waste time filtering stopped channels, we just look for a read from
			// this register on a stopped voice, and return the raw sample data at the
			// accumulator
			if ((voice->control & CONTROL_STOPMASK))
			{
				voice->o1n1 = read_sample(voice, get_integer_addr(voice->accum));
				// logerror("%02x %08x ==> %08x\n",voice->o1n1,get_bank(voice->control),get_integer_addr(voice->accum));
			}
			result = voice->o1n1 & 0xffff;
			break;

		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:  // unused
			break;

		case 0x0d:  // ACT
			result = m_active_voices;
			break;

		case 0x0e:  // IRQV
			result = m_irqv;
			if (!machine().side_effects_disabled())
				update_internal_irq_state();
			break;

		case 0x0f:  // PAGE
			result = m_current_page;
			break;
	}
	return result;
}


inline u16 es5505_device::reg_read_test(es550x_voice *voice, offs_t offset)
{
	u16 result = 0;

	switch (offset)
	{
		case 0x00:  // CH0L
		case 0x01:  // CH0R
		case 0x02:  // CH1L
		case 0x03:  // CH1R
		case 0x04:  // CH2L
		case 0x05:  // CH2R
		case 0x06:  // CH3L
		case 0x07:  // CH3R
			break;

		case 0x08:  // SERMODE
			result = m_mode | 0x7f8;
			break;

		case 0x09:  // PAR
			if (!m_read_port_cb.isunset())
				result = m_read_port_cb(0) & 0xffc0; // 10 bit, 15:6
			break;

		// The following are global, and thus accessible form all pages
		case 0x0d:  // ACT
			result = m_active_voices;
			break;

		case 0x0e:  // IRQV
			result = m_irqv;
			if (!machine().side_effects_disabled())
				update_internal_irq_state();
			break;

		case 0x0f:  // PAGE
			result = m_current_page;
			break;
	}
	return result;
}


u16 es5505_device::read(offs_t offset)
{
	es550x_voice *voice = &m_voice[m_current_page & 0x1f];
	u16 result;

	LOG("read from %02x/%02x -> ", m_current_page, offset);

	// force an update
	m_stream->update();

	// switch off the page and register
	if (m_current_page < 0x20)
		result = reg_read_low(voice, offset);
	else if (m_current_page < 0x40)
		result = reg_read_high(voice, offset);
	else
		result = reg_read_test(voice, offset);

	LOG("%04x (accum=%08x)\n", result, voice->accum);

	// return the high byte
	return result;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void es550x_device::sound_stream_update(sound_stream &stream)
{
#if ES5506_MAKE_WAVS
	// start the logging once we have a sample rate
	if (m_sample_rate)
	{
		if (!m_wavraw)
			m_wavraw = wav_open("raw.wav", m_sample_rate, 2);
	}
#endif

	// loop until all samples are output
	generate_samples(stream);

#if ES5506_MAKE_WAVS
	// log the raw data
	if (m_wavraw)
	{
		// determine left/right source data

		s32 *lsrc = m_scratch, *rsrc = m_scratch + length;
		int channel;
		memset(lsrc, 0, sizeof(s32) * length * 2);
		// loop over the output channels
		for (channel = 0; channel < m_channels; channel++)
		{
			s32 *l = outputs[(channel << 1)] + sampindex;
			s32 *r = outputs[(channel << 1) + 1] + sampindex;
			// add the current channel's samples to the WAV data
			for (samp = 0; samp < length; samp++)
			{
				lsrc[samp] += l[samp];
				rsrc[samp] += r[samp];
			}
		}
		wav_add_data_32lr(m_wavraw, lsrc, rsrc, length, 4);
	}
#endif
}
