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


/**********************************************************************************************

     CONSTANTS

***********************************************************************************************/

#define LOG_COMMANDS            0
#define RAINE_CHECK             0

#if MAKE_WAVS
#include "sound/wavwrite.h"
#endif


#define MAX_SAMPLE_CHUNK        10000
#define ULAW_MAXBITS            8

#define CONTROL_BS1             0x8000
#define CONTROL_BS0             0x4000
#define CONTROL_CMPD            0x2000
#define CONTROL_CA2             0x1000
#define CONTROL_CA1             0x0800
#define CONTROL_CA0             0x0400
#define CONTROL_LP4             0x0200
#define CONTROL_LP3             0x0100
#define CONTROL_IRQ             0x0080
#define CONTROL_DIR             0x0040
#define CONTROL_IRQE            0x0020
#define CONTROL_BLE             0x0010
#define CONTROL_LPE             0x0008
#define CONTROL_LEI             0x0004
#define CONTROL_STOP1           0x0002
#define CONTROL_STOP0           0x0001

#define CONTROL_BSMASK          (CONTROL_BS1 | CONTROL_BS0)
#define CONTROL_CAMASK          (CONTROL_CA2 | CONTROL_CA1 | CONTROL_CA0)
#define CONTROL_LPMASK          (CONTROL_LP4 | CONTROL_LP3)
#define CONTROL_LOOPMASK        (CONTROL_BLE | CONTROL_LPE)
#define CONTROL_STOPMASK        (CONTROL_STOP1 | CONTROL_STOP0)


es550x_device::es550x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_sample_rate(0),
		m_write_latch(0),
		m_read_latch(0),
		m_master_clock(0),
		m_current_page(0),
		m_active_voices(0),
		m_mode(0),
		m_wst(0),
		m_wend(0),
		m_lrend(0),
		m_irqv(0),
		m_scratch(nullptr),
		m_ulaw_lookup(nullptr),
		m_volume_lookup(nullptr),
		#if MAKE_WAVS
		m_wavraw(NULL),
		#endif
		m_eslog(nullptr),
		m_region0(nullptr),
		m_region1(nullptr),
		m_region2(nullptr),
		m_region3(nullptr),
		m_channels(0),
		m_irq_cb(*this),
		m_read_port_cb(*this)
{
	for (auto & elem : m_region_base)
	{
		elem = nullptr;
	}
}

const device_type ES5506 = &device_creator<es5506_device>;

es5506_device::es5506_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: es550x_device(mconfig, ES5506, "ES5506", tag, owner, clock, "es5506", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void es550x_device::device_start()
{
}

void es5506_device::device_start()
{
	int j;
	UINT32 accum_mask;
	int channels = 1;  /* 1 channel by default, for backward compatibility */

	/* only override the number of channels if the value is in the valid range 1 .. 6 */
	if (1 <= m_channels && m_channels <= 6)
		channels = m_channels;

	/* debugging */
	if (LOG_COMMANDS && !m_eslog)
		m_eslog = fopen("es.log", "w");

	/* create the stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 2 * channels, clock() / (16*32));

	/* initialize the regions */
	m_region_base[0] = m_region0 ? (UINT16 *)machine().root_device().memregion(m_region0)->base() : nullptr;
	m_region_base[1] = m_region1 ? (UINT16 *)machine().root_device().memregion(m_region1)->base() : nullptr;
	m_region_base[2] = m_region2 ? (UINT16 *)machine().root_device().memregion(m_region2)->base() : nullptr;
	m_region_base[3] = m_region3 ? (UINT16 *)machine().root_device().memregion(m_region3)->base() : nullptr;

	/* initialize the rest of the structure */
	m_master_clock = clock();
	m_irq_cb.resolve();
	m_read_port_cb.resolve();
	m_irqv = 0x80;
	m_channels = channels;

	/* KT-76 assumes all voices are active on an ES5506 without setting them! */
	m_active_voices = 31;
	m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
	m_stream->set_sample_rate(m_sample_rate);

	/* compute the tables */
	compute_tables();

	/* init the voices */
	accum_mask = 0xffffffff;
	for (j = 0; j < 32; j++)
	{
		m_voice[j].index = j;
		m_voice[j].control = CONTROL_STOPMASK;
		m_voice[j].lvol = 0xffff;
		m_voice[j].rvol = 0xffff;
		m_voice[j].exbank = 0;
		m_voice[j].accum_mask = accum_mask;
	}

	/* allocate memory */
	m_scratch = auto_alloc_array_clear(machine(), INT32, 2 * MAX_SAMPLE_CHUNK);

	/* register save */
	save_item(NAME(m_sample_rate));
	save_item(NAME(m_write_latch));
	save_item(NAME(m_read_latch));

	save_item(NAME(m_current_page));
	save_item(NAME(m_active_voices));
	save_item(NAME(m_mode));
	save_item(NAME(m_wst));
	save_item(NAME(m_wend));
	save_item(NAME(m_lrend));
	save_item(NAME(m_irqv));

	save_pointer(NAME(m_scratch), 2 * MAX_SAMPLE_CHUNK);

	for (j = 0; j < 32; j++)
	{
		save_item(NAME(m_voice[j].control), j);
		save_item(NAME(m_voice[j].freqcount), j);
		save_item(NAME(m_voice[j].start), j);
		save_item(NAME(m_voice[j].lvol), j);
		save_item(NAME(m_voice[j].end), j);
		save_item(NAME(m_voice[j].lvramp), j);
		save_item(NAME(m_voice[j].accum), j);
		save_item(NAME(m_voice[j].rvol), j);
		save_item(NAME(m_voice[j].rvramp), j);
		save_item(NAME(m_voice[j].ecount), j);
		save_item(NAME(m_voice[j].k2), j);
		save_item(NAME(m_voice[j].k2ramp), j);
		save_item(NAME(m_voice[j].k1), j);
		save_item(NAME(m_voice[j].k1ramp), j);
		save_item(NAME(m_voice[j].o4n1), j);
		save_item(NAME(m_voice[j].o3n1), j);
		save_item(NAME(m_voice[j].o3n2), j);
		save_item(NAME(m_voice[j].o2n1), j);
		save_item(NAME(m_voice[j].o2n2), j);
		save_item(NAME(m_voice[j].o1n1), j);
		save_item(NAME(m_voice[j].exbank), j);
		save_item(NAME(m_voice[j].filtcount), j);
	}

	/* success */
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void es550x_device::device_reset()
{
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void es550x_device::device_stop()
{
	/* debugging */
	if (LOG_COMMANDS && m_eslog)
	{
		fclose(m_eslog);
		m_eslog = nullptr;
	}

	#if MAKE_WAVS
	{
		int i;

		for (i = 0; i < MAX_ES5506; i++)
		{
			if (es5506[i].m_wavraw)
				wav_close(es5506[i].m_wavraw);
		}
	}
	#endif
}

const device_type ES5505 = &device_creator<es5505_device>;

es5505_device::es5505_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: es550x_device(mconfig, ES5505, "ES5505", tag, owner, clock, "es5505", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void es5505_device::device_start()
{
	int j;
	UINT32 accum_mask;
	int channels = 1;  /* 1 channel by default, for backward compatibility */

	/* only override the number of channels if the value is in the valid range 1 .. 4 */
	if (1 <= m_channels && m_channels <= 4)
		channels = m_channels;

	/* debugging */
	if (LOG_COMMANDS && !m_eslog)
		m_eslog = fopen("es.log", "w");

	/* create the stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 2 * channels, clock() / (16*32));

	/* initialize the regions */
	m_region_base[0] = m_region0 ? (UINT16 *)machine().root_device().memregion(m_region0)->base() : nullptr;
	m_region_base[1] = m_region1 ? (UINT16 *)machine().root_device().memregion(m_region1)->base() : nullptr;

	/* initialize the rest of the structure */
	m_master_clock = clock();
	m_irq_cb.resolve();
	m_read_port_cb.resolve();
	m_irqv = 0x80;
	m_channels = channels;

	/* compute the tables */
	compute_tables();

	/* init the voices */
	accum_mask = 0x7fffffff;
	for (j = 0; j < 32; j++)
	{
		m_voice[j].index = j;
		m_voice[j].control = CONTROL_STOPMASK;
		m_voice[j].lvol = 0xffff;
		m_voice[j].rvol = 0xffff;
		m_voice[j].exbank = 0;
		m_voice[j].accum_mask = accum_mask;
	}

	/* allocate memory */
	m_scratch = auto_alloc_array_clear(machine(), INT32, 2 * MAX_SAMPLE_CHUNK);

	/* register save */
	save_item(NAME(m_sample_rate));
	save_item(NAME(m_write_latch));
	save_item(NAME(m_read_latch));

	save_item(NAME(m_current_page));
	save_item(NAME(m_active_voices));
	save_item(NAME(m_mode));
	save_item(NAME(m_wst));
	save_item(NAME(m_wend));
	save_item(NAME(m_lrend));
	save_item(NAME(m_irqv));

	save_pointer(NAME(m_scratch), 2 * MAX_SAMPLE_CHUNK);

	for (j = 0; j < 32; j++)
	{
		save_item(NAME(m_voice[j].control), j);
		save_item(NAME(m_voice[j].freqcount), j);
		save_item(NAME(m_voice[j].start), j);
		save_item(NAME(m_voice[j].lvol), j);
		save_item(NAME(m_voice[j].end), j);
		save_item(NAME(m_voice[j].lvramp), j);
		save_item(NAME(m_voice[j].accum), j);
		save_item(NAME(m_voice[j].rvol), j);
		save_item(NAME(m_voice[j].rvramp), j);
		save_item(NAME(m_voice[j].ecount), j);
		save_item(NAME(m_voice[j].k2), j);
		save_item(NAME(m_voice[j].k2ramp), j);
		save_item(NAME(m_voice[j].k1), j);
		save_item(NAME(m_voice[j].k1ramp), j);
		save_item(NAME(m_voice[j].o4n1), j);
		save_item(NAME(m_voice[j].o3n1), j);
		save_item(NAME(m_voice[j].o3n2), j);
		save_item(NAME(m_voice[j].o2n1), j);
		save_item(NAME(m_voice[j].o2n2), j);
		save_item(NAME(m_voice[j].o1n1), j);
		save_item(NAME(m_voice[j].exbank), j);
		save_item(NAME(m_voice[j].filtcount), j);
	}

	/* success */
}


/**********************************************************************************************

     update_irq_state -- update the IRQ state

***********************************************************************************************/


void es550x_device::update_irq_state()
{
	/* ES5505/6 irq line has been set high - inform the host */
	if (!m_irq_cb.isnull())
		m_irq_cb(1); /* IRQB set high */
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

	m_irqv=0x80;

	if (!m_irq_cb.isnull())
		m_irq_cb(0); /* IRQB set low */
}

/**********************************************************************************************

     compute_tables -- compute static tables

***********************************************************************************************/

void es550x_device::compute_tables()
{
	int i;

	/* allocate ulaw lookup table */
	m_ulaw_lookup = auto_alloc_array_clear(machine(), INT16, 1 << ULAW_MAXBITS);

	/* generate ulaw lookup table */
	for (i = 0; i < (1 << ULAW_MAXBITS); i++)
	{
		UINT16 rawval = (i << (16 - ULAW_MAXBITS)) | (1 << (15 - ULAW_MAXBITS));
		UINT8 exponent = rawval >> 13;
		UINT32 mantissa = (rawval << 3) & 0xffff;

		if (exponent == 0)
			m_ulaw_lookup[i] = (INT16)mantissa >> 7;
		else
		{
			mantissa = (mantissa >> 1) | (~mantissa & 0x8000);
			m_ulaw_lookup[i] = (INT16)mantissa >> (7 - exponent);
		}
	}

	/* allocate volume lookup table */
	m_volume_lookup = auto_alloc_array_clear(machine(), UINT16, 4096);

	/* generate volume lookup table */
	for (i = 0; i < 4096; i++)
	{
		UINT8 exponent = i >> 8;
		UINT32 mantissa = (i & 0xff) | 0x100;

		m_volume_lookup[i] = (mantissa << 11) >> (20 - exponent);
	}
}



/**********************************************************************************************

     interpolate -- interpolate between two samples

***********************************************************************************************/

#define interpolate(sample1, sample2, accum)                                \
		(sample1 * (INT32)(0x800 - (accum & 0x7ff)) +                       \
			sample2 * (INT32)(accum & 0x7ff)) >> 11;



/**********************************************************************************************

     apply_filters -- apply the 4-pole digital filter to the sample

***********************************************************************************************/

#define apply_filters(voice, sample)                                                            \
do                                                                                              \
{                                                                                               \
	/* pole 1 is always low-pass using K1 */                                                    \
	sample = ((INT32)(voice->k1 >> 2) * (sample - voice->o1n1) / 16384) + voice->o1n1;          \
	voice->o1n1 = sample;                                                                       \
																								\
	/* pole 2 is always low-pass using K1 */                                                    \
	sample = ((INT32)(voice->k1 >> 2) * (sample - voice->o2n1) / 16384) + voice->o2n1;          \
	voice->o2n2 = voice->o2n1;                                                                  \
	voice->o2n1 = sample;                                                                       \
																								\
	/* remaining poles depend on the current filter setting */                                  \
	switch (voice->control & CONTROL_LPMASK)                                                    \
	{                                                                                           \
		case 0:                                                                                 \
			/* pole 3 is high-pass using K2 */                                                  \
			sample = sample - voice->o2n2 + ((INT32)(voice->k2 >> 2) * voice->o3n1) / 32768 + voice->o3n1 / 2; \
			voice->o3n2 = voice->o3n1;                                                          \
			voice->o3n1 = sample;                                                               \
																								\
			/* pole 4 is high-pass using K2 */                                                  \
			sample = sample - voice->o3n2 + ((INT32)(voice->k2 >> 2) * voice->o4n1) / 32768 + voice->o4n1 / 2; \
			voice->o4n1 = sample;                                                               \
			break;                                                                              \
																								\
		case CONTROL_LP3:                                                                       \
			/* pole 3 is low-pass using K1 */                                                   \
			sample = ((INT32)(voice->k1 >> 2) * (sample - voice->o3n1) / 16384) + voice->o3n1;  \
			voice->o3n2 = voice->o3n1;                                                          \
			voice->o3n1 = sample;                                                               \
																								\
			/* pole 4 is high-pass using K2 */                                                  \
			sample = sample - voice->o3n2 + ((INT32)(voice->k2 >> 2) * voice->o4n1) / 32768 + voice->o4n1 / 2; \
			voice->o4n1 = sample;                                                               \
			break;                                                                              \
																								\
		case CONTROL_LP4:                                                                       \
			/* pole 3 is low-pass using K2 */                                                   \
			sample = ((INT32)(voice->k2 >> 2) * (sample - voice->o3n1) / 16384) + voice->o3n1;  \
			voice->o3n2 = voice->o3n1;                                                          \
			voice->o3n1 = sample;                                                               \
																								\
			/* pole 4 is low-pass using K2 */                                                   \
			sample = ((INT32)(voice->k2 >> 2) * (sample - voice->o4n1) / 16384) + voice->o4n1;  \
			voice->o4n1 = sample;                                                               \
			break;                                                                              \
																								\
		case CONTROL_LP4 | CONTROL_LP3:                                                         \
			/* pole 3 is low-pass using K1 */                                                   \
			sample = ((INT32)(voice->k1 >> 2) * (sample - voice->o3n1) / 16384) + voice->o3n1;  \
			voice->o3n2 = voice->o3n1;                                                          \
			voice->o3n1 = sample;                                                               \
																								\
			/* pole 4 is low-pass using K2 */                                                   \
			sample = ((INT32)(voice->k2 >> 2) * (sample - voice->o4n1) / 16384) + voice->o4n1;  \
			voice->o4n1 = sample;                                                               \
			break;                                                                              \
	}                                                                                           \
} while (0)



/**********************************************************************************************

     update_envelopes -- update the envelopes

***********************************************************************************************/

#define update_envelopes(voice, samples)                                            \
do                                                                                  \
{                                                                                   \
	int count = (samples > 1 && samples > voice->ecount) ? voice->ecount : samples; \
																					\
	/* decrement the envelope counter */                                            \
	voice->ecount -= count;                                                         \
																					\
	/* ramp left volume */                                                          \
	if (voice->lvramp)                                                              \
	{                                                                               \
		voice->lvol += (INT8)voice->lvramp * count;                                 \
		if ((INT32)voice->lvol < 0) voice->lvol = 0;                                \
		else if (voice->lvol > 0xffff) voice->lvol = 0xffff;                        \
	}                                                                               \
																					\
	/* ramp right volume */                                                         \
	if (voice->rvramp)                                                              \
	{                                                                               \
		voice->rvol += (INT8)voice->rvramp * count;                                 \
		if ((INT32)voice->rvol < 0) voice->rvol = 0;                                \
		else if (voice->rvol > 0xffff) voice->rvol = 0xffff;                        \
	}                                                                               \
																					\
	/* ramp k1 filter constant */                                                   \
	if (voice->k1ramp && ((INT32)voice->k1ramp >= 0 || !(voice->filtcount & 7)))    \
	{                                                                               \
		voice->k1 += (INT8)voice->k1ramp * count;                                   \
		if ((INT32)voice->k1 < 0) voice->k1 = 0;                                    \
		else if (voice->k1 > 0xffff) voice->k1 = 0xffff;                            \
	}                                                                               \
																					\
	/* ramp k2 filter constant */                                                   \
	if (voice->k2ramp && ((INT32)voice->k2ramp >= 0 || !(voice->filtcount & 7)))    \
	{                                                                               \
		voice->k2 += (INT8)voice->k2ramp * count;                                   \
		if ((INT32)voice->k2 < 0) voice->k2 = 0;                                    \
		else if (voice->k2 > 0xffff) voice->k2 = 0xffff;                            \
	}                                                                               \
																					\
	/* update the filter constant counter */                                        \
	voice->filtcount += count;                                                      \
																					\
} while (0)



/**********************************************************************************************

     check_for_end_forward
     check_for_end_reverse -- check for loop end and loop appropriately

***********************************************************************************************/

#define check_for_end_forward(voice, accum)                                         \
do                                                                                  \
{                                                                                   \
	/* are we past the end? */                                                      \
	if (accum > voice->end && !(voice->control & CONTROL_LEI))                      \
	{                                                                               \
		/* generate interrupt if required */                                        \
		if (voice->control&CONTROL_IRQE)                                            \
			voice->control |= CONTROL_IRQ;                                          \
																					\
		/* handle the different types of looping */                                 \
		switch (voice->control & CONTROL_LOOPMASK)                                  \
		{                                                                           \
			/* non-looping */                                                       \
			case 0:                                                                 \
				voice->control |= CONTROL_STOP0;                                    \
				goto alldone;                                                       \
																					\
			/* uni-directional looping */                                           \
			case CONTROL_LPE:                                                       \
				accum = (voice->start + (accum - voice->end)) & voice->accum_mask;  \
				break;                                                              \
																					\
			/* trans-wave looping */                                                \
			case CONTROL_BLE:                                                       \
				accum = (voice->start + (accum - voice->end)) & voice->accum_mask;  \
				voice->control = (voice->control & ~CONTROL_LOOPMASK) | CONTROL_LEI;\
				break;                                                              \
																					\
			/* bi-directional looping */                                            \
			case CONTROL_LPE | CONTROL_BLE:                                         \
				accum = (voice->end - (accum - voice->end)) & voice->accum_mask;    \
				voice->control ^= CONTROL_DIR;                                      \
				goto reverse;                                                       \
		}                                                                           \
	}                                                                               \
} while (0)


#define check_for_end_reverse(voice, accum)                                         \
do                                                                                  \
{                                                                                   \
	/* are we past the end? */                                                      \
	if (accum < voice->start && !(voice->control & CONTROL_LEI))                    \
	{                                                                               \
		/* generate interrupt if required */                                        \
		if (voice->control&CONTROL_IRQE)                                            \
			voice->control |= CONTROL_IRQ;                                          \
																					\
		/* handle the different types of looping */                                 \
		switch (voice->control & CONTROL_LOOPMASK)                                  \
		{                                                                           \
			/* non-looping */                                                       \
			case 0:                                                                 \
				voice->control |= CONTROL_STOP0;                                    \
				goto alldone;                                                       \
																					\
			/* uni-directional looping */                                           \
			case CONTROL_LPE:                                                       \
				accum = (voice->end - (voice->start - accum)) & voice->accum_mask;  \
				break;                                                              \
																					\
			/* trans-wave looping */                                                \
			case CONTROL_BLE:                                                       \
				accum = (voice->end - (voice->start - accum)) & voice->accum_mask;  \
				voice->control = (voice->control & ~CONTROL_LOOPMASK) | CONTROL_LEI;\
				break;                                                              \
																					\
			/* bi-directional looping */                                            \
			case CONTROL_LPE | CONTROL_BLE:                                         \
				accum = (voice->start + (voice->start - accum)) & voice->accum_mask;\
				voice->control ^= CONTROL_DIR;                                      \
				goto reverse;                                                       \
		}                                                                           \
	}                                                                               \
} while (0)



/**********************************************************************************************

     generate_dummy -- generate nothing, just apply envelopes

***********************************************************************************************/

void es550x_device::generate_dummy(es550x_voice *voice, UINT16 *base, INT32 *lbuffer, INT32 *rbuffer, int samples)
{
	UINT32 freqcount = voice->freqcount;
	UINT32 accum = voice->accum & voice->accum_mask;

	/* outer loop, in case we switch directions */
	while (samples > 0 && !(voice->control & CONTROL_STOPMASK))
	{
reverse:
		/* two cases: first case is forward direction */
		if (!(voice->control & CONTROL_DIR))
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				accum = (accum + freqcount) & voice->accum_mask;

				/* update filters/volumes */
				if (voice->ecount != 0)
					update_envelopes(voice, 1);

				/* check for loop end */
				check_for_end_forward(voice, accum);
			}
		}

		/* two cases: second case is backward direction */
		else
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				accum = (accum - freqcount) & voice->accum_mask;

				/* update filters/volumes */
				if (voice->ecount != 0)
					update_envelopes(voice, 1);

				/* check for loop end */
				check_for_end_reverse(voice, accum);
			}
		}
	}

	/* if we stopped, process any additional envelope */
alldone:
	voice->accum = accum;
	if (samples > 0)
		update_envelopes(voice, samples);
}



/**********************************************************************************************

     generate_ulaw -- general u-law decoding routine

***********************************************************************************************/

void es550x_device::generate_ulaw(es550x_voice *voice, UINT16 *base, INT32 *lbuffer, INT32 *rbuffer, int samples)
{
	UINT32 freqcount = voice->freqcount;
	UINT32 accum = voice->accum & voice->accum_mask;
	INT32 lvol = m_volume_lookup[voice->lvol >> 4];
	INT32 rvol = m_volume_lookup[voice->rvol >> 4];

	/* pre-add the bank offset */
	base += voice->exbank;

	/* outer loop, in case we switch directions */
	while (samples > 0 && !(voice->control & CONTROL_STOPMASK))
	{
reverse:
		/* two cases: first case is forward direction */
		if (!(voice->control & CONTROL_DIR))
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				INT32 val1 = base[accum >> 11];
				INT32 val2 = base[((accum + (1 << 11)) & voice->accum_mask) >> 11];

				/* decompress u-law */
				val1 = m_ulaw_lookup[val1 >> (16 - ULAW_MAXBITS)];
				val2 = m_ulaw_lookup[val2 >> (16 - ULAW_MAXBITS)];

				/* interpolate */
				val1 = interpolate(val1, val2, accum);
				accum = (accum + freqcount) & voice->accum_mask;

				/* apply filters */
				apply_filters(voice, val1);

				/* update filters/volumes */
				if (voice->ecount != 0)
				{
					update_envelopes(voice, 1);
					lvol = m_volume_lookup[voice->lvol >> 4];
					rvol = m_volume_lookup[voice->rvol >> 4];
				}

				/* apply volumes and add */
				*lbuffer++ += (val1 * lvol) >> 11;
				*rbuffer++ += (val1 * rvol) >> 11;

				/* check for loop end */
				check_for_end_forward(voice, accum);
			}
		}

		/* two cases: second case is backward direction */
		else
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				INT32 val1 = base[accum >> 11];
				INT32 val2 = base[((accum + (1 << 11)) & voice->accum_mask) >> 11];

				/* decompress u-law */
				val1 = m_ulaw_lookup[val1 >> (16 - ULAW_MAXBITS)];
				val2 = m_ulaw_lookup[val2 >> (16 - ULAW_MAXBITS)];

				/* interpolate */
				val1 = interpolate(val1, val2, accum);
				accum = (accum - freqcount) & voice->accum_mask;

				/* apply filters */
				apply_filters(voice, val1);

				/* update filters/volumes */
				if (voice->ecount != 0)
				{
					update_envelopes(voice, 1);
					lvol = m_volume_lookup[voice->lvol >> 4];
					rvol = m_volume_lookup[voice->rvol >> 4];
				}

				/* apply volumes and add */
				*lbuffer++ += (val1 * lvol) >> 11;
				*rbuffer++ += (val1 * rvol) >> 11;

				/* check for loop end */
				check_for_end_reverse(voice, accum);
			}
		}
	}

	/* if we stopped, process any additional envelope */
alldone:
	voice->accum = accum;
	if (samples > 0)
		update_envelopes(voice, samples);
}



/**********************************************************************************************

     generate_pcm -- general PCM decoding routine

***********************************************************************************************/

void es550x_device::generate_pcm(es550x_voice *voice, UINT16 *base, INT32 *lbuffer, INT32 *rbuffer, int samples)
{
	UINT32 freqcount = voice->freqcount;
	UINT32 accum = voice->accum & voice->accum_mask;
	INT32 lvol = m_volume_lookup[voice->lvol >> 4];
	INT32 rvol = m_volume_lookup[voice->rvol >> 4];

	/* pre-add the bank offset */
	base += voice->exbank;

	/* outer loop, in case we switch directions */
	while (samples > 0 && !(voice->control & CONTROL_STOPMASK))
	{
reverse:
		/* two cases: first case is forward direction */
		if (!(voice->control & CONTROL_DIR))
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				INT32 val1 = (INT16)base[accum >> 11];
				INT32 val2 = (INT16)base[((accum + (1 << 11)) & voice->accum_mask) >> 11];

				/* interpolate */
				val1 = interpolate(val1, val2, accum);
				accum = (accum + freqcount) & voice->accum_mask;

				/* apply filters */
				apply_filters(voice, val1);

				/* update filters/volumes */
				if (voice->ecount != 0)
				{
					update_envelopes(voice, 1);
					lvol = m_volume_lookup[voice->lvol >> 4];
					rvol = m_volume_lookup[voice->rvol >> 4];
				}

				/* apply volumes and add */
				*lbuffer++ += (val1 * lvol) >> 11;
				*rbuffer++ += (val1 * rvol) >> 11;

				/* check for loop end */
				check_for_end_forward(voice, accum);
			}
		}

		/* two cases: second case is backward direction */
		else
		{
			/* loop while we still have samples to generate */
			while (samples--)
			{
				/* fetch two samples */
				INT32 val1 = (INT16)base[accum >> 11];
				INT32 val2 = (INT16)base[((accum + (1 << 11)) & voice->accum_mask) >> 11];

				/* interpolate */
				val1 = interpolate(val1, val2, accum);
				accum = (accum - freqcount) & voice->accum_mask;

				/* apply filters */
				apply_filters(voice, val1);

				/* update filters/volumes */
				if (voice->ecount != 0)
				{
					update_envelopes(voice, 1);
					lvol = m_volume_lookup[voice->lvol >> 4];
					rvol = m_volume_lookup[voice->rvol >> 4];
				}

				/* apply volumes and add */
				*lbuffer++ += (val1 * lvol) >> 11;
				*rbuffer++ += (val1 * rvol) >> 11;

				/* check for loop end */
				check_for_end_reverse(voice, accum);
			}
		}
	}

	/* if we stopped, process any additional envelope */
alldone:
	voice->accum = accum;
	if (samples > 0)
		update_envelopes(voice, samples);
}



/**********************************************************************************************

     generate_samples -- tell each voice to generate samples

***********************************************************************************************/

void es5506_device::generate_samples(INT32 **outputs, int offset, int samples)
{
	int v;

	/* skip if nothing to do */
	if (!samples)
		return;

	/* clear out the accumulators */
	for (int i = 0; i < m_channels << 1; i++)
	{
		memset(outputs[i] + offset, 0, sizeof(INT32) * samples);
	}

	/* loop over voices */
	for (v = 0; v <= m_active_voices; v++)
	{
		es550x_voice *voice = &m_voice[v];
		UINT16 *base = m_region_base[voice->control >> 14];

		/* special case: if end == start, stop the voice */
		if (voice->start == voice->end)
			voice->control |= CONTROL_STOP0;

		int voice_channel = (voice->control & CONTROL_CAMASK) >> 10;
		int channel = voice_channel % m_channels;
		int l = channel << 1;
		int r = l + 1;
		INT32 *left = outputs[l] + offset;
		INT32 *right = outputs[r] + offset;

		/* generate from the appropriate source */
		if (!base)
		{
			logerror("es5506: NULL region base %d\n",voice->control >> 14);
			generate_dummy(voice, base, left, right, samples);
		}
		else if (voice->control & 0x2000)
			generate_ulaw(voice, base, left, right, samples);
		else
			generate_pcm(voice, base, left, right, samples);

		/* does this voice have it's IRQ bit raised? */
		if (voice->control&CONTROL_IRQ)
		{
			logerror("es5506: IRQ raised on voice %d!!\n",v);

			/* only update voice vector if existing IRQ is acked by host */
			if (m_irqv&0x80)
			{
				/* latch voice number into vector, and set high bit low */
				m_irqv=v&0x7f;

				/* take down IRQ bit on voice */
				voice->control&=~CONTROL_IRQ;

				/* inform host of irq */
				update_irq_state();
			}
		}
	}
}

void es5505_device::generate_samples(INT32 **outputs, int offset, int samples)
{
	int v;

	/* skip if nothing to do */
	if (!samples)
		return;

	/* clear out the accumulators */
	for (int i = 0; i < m_channels << 1; i++)
	{
		memset(outputs[i] + offset, 0, sizeof(INT32) * samples);
	}

	/* loop over voices */
	for (v = 0; v <= m_active_voices; v++)
	{
		es550x_voice *voice = &m_voice[v];
		UINT16 *base = m_region_base[voice->control >> 14];

		/* special case: if end == start, stop the voice */
		if (voice->start == voice->end)
			voice->control |= CONTROL_STOP0;

		int voice_channel = (voice->control & CONTROL_CAMASK) >> 10;
		int channel = voice_channel % m_channels;
		int l = channel << 1;
		int r = l + 1;
		INT32 *left = outputs[l] + offset;
		INT32 *right = outputs[r] + offset;

		/* generate from the appropriate source */
		if (!base)
		{
			logerror("es5506: NULL region base %d\n",voice->control >> 14);
			generate_dummy(voice, base, left, right, samples);
		}
		else if (voice->control & 0x2000)
			generate_ulaw(voice, base, left, right, samples);
		else
			generate_pcm(voice, base, left, right, samples);

		/* does this voice have it's IRQ bit raised? */
		if (voice->control&CONTROL_IRQ)
		{
			logerror("es5506: IRQ raised on voice %d!!\n",v);

			/* only update voice vector if existing IRQ is acked by host */
			if (m_irqv&0x80)
			{
				/* latch voice number into vector, and set high bit low */
				m_irqv=v&0x7f;

				/* take down IRQ bit on voice */
				voice->control&=~CONTROL_IRQ;

				/* inform host of irq */
				update_irq_state();
			}
		}
	}
}



/**********************************************************************************************

     reg_write -- handle a write to the selected ES5506 register

***********************************************************************************************/

inline void es5506_device::reg_write_low(es550x_voice *voice, offs_t offset, UINT32 data)
{
	switch (offset)
	{
		case 0x00/8:    /* CR */
			voice->control = data & 0xffff;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, control=%04x\n", m_current_page & 0x1f, voice->control);
			break;

		case 0x08/8:    /* FC */
			voice->freqcount = data & 0x1ffff;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, freq count=%08x\n", m_current_page & 0x1f, voice->freqcount);
			break;

		case 0x10/8:    /* LVOL */
			voice->lvol = data & 0xffff;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, left vol=%04x\n", m_current_page & 0x1f, voice->lvol);
			break;

		case 0x18/8:    /* LVRAMP */
			voice->lvramp = (data & 0xff00) >> 8;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, left vol ramp=%04x\n", m_current_page & 0x1f, voice->lvramp);
			break;

		case 0x20/8:    /* RVOL */
			voice->rvol = data & 0xffff;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, right vol=%04x\n", m_current_page & 0x1f, voice->rvol);
			break;

		case 0x28/8:    /* RVRAMP */
			voice->rvramp = (data & 0xff00) >> 8;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, right vol ramp=%04x\n", m_current_page & 0x1f, voice->rvramp);
			break;

		case 0x30/8:    /* ECOUNT */
			voice->ecount = data & 0x1ff;
			voice->filtcount = 0;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, envelope count=%04x\n", m_current_page & 0x1f, voice->ecount);
			break;

		case 0x38/8:    /* K2 */
			voice->k2 = data & 0xffff;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, K2=%04x\n", m_current_page & 0x1f, voice->k2);
			break;

		case 0x40/8:    /* K2RAMP */
			voice->k2ramp = ((data & 0xff00) >> 8) | ((data & 0x0001) << 31);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, K2 ramp=%04x\n", m_current_page & 0x1f, voice->k2ramp);
			break;

		case 0x48/8:    /* K1 */
			voice->k1 = data & 0xffff;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, K1=%04x\n", m_current_page & 0x1f, voice->k1);
			break;

		case 0x50/8:    /* K1RAMP */
			voice->k1ramp = ((data & 0xff00) >> 8) | ((data & 0x0001) << 31);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, K1 ramp=%04x\n", m_current_page & 0x1f, voice->k1ramp);
			break;

		case 0x58/8:    /* ACTV */
		{
			m_active_voices = data & 0x1f;
			m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
			m_stream->set_sample_rate(m_sample_rate);

			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "active voices=%d, sample_rate=%d\n", m_active_voices, m_sample_rate);
			break;
		}

		case 0x60/8:    /* MODE */
			m_mode = data & 0x1f;
			break;

		case 0x68/8:    /* PAR - read only */
		case 0x70/8:    /* IRQV - read only */
			break;

		case 0x78/8:    /* PAGE */
			m_current_page = data & 0x7f;
			break;
	}
}

inline void es5506_device::reg_write_high(es550x_voice *voice, offs_t offset, UINT32 data)
{
	switch (offset)
	{
		case 0x00/8:    /* CR */
			voice->control = data & 0xffff;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, control=%04x\n", m_current_page & 0x1f, voice->control);
			break;

		case 0x08/8:    /* START */
			voice->start = data & 0xfffff800;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, loop start=%08x\n", m_current_page & 0x1f, voice->start);
			break;

		case 0x10/8:    /* END */
			voice->end = data & 0xffffff80;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, loop end=%08x\n", m_current_page & 0x1f, voice->end);
			break;

		case 0x18/8:    /* ACCUM */
			voice->accum = data;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, accum=%08x\n", m_current_page & 0x1f, voice->accum);
			break;

		case 0x20/8:    /* O4(n-1) */
			voice->o4n1 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, O4(n-1)=%05x\n", m_current_page & 0x1f, voice->o4n1 & 0x3ffff);
			break;

		case 0x28/8:    /* O3(n-1) */
			voice->o3n1 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, O3(n-1)=%05x\n", m_current_page & 0x1f, voice->o3n1 & 0x3ffff);
			break;

		case 0x30/8:    /* O3(n-2) */
			voice->o3n2 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, O3(n-2)=%05x\n", m_current_page & 0x1f, voice->o3n2 & 0x3ffff);
			break;

		case 0x38/8:    /* O2(n-1) */
			voice->o2n1 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, O2(n-1)=%05x\n", m_current_page & 0x1f, voice->o2n1 & 0x3ffff);
			break;

		case 0x40/8:    /* O2(n-2) */
			voice->o2n2 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, O2(n-2)=%05x\n", m_current_page & 0x1f, voice->o2n2 & 0x3ffff);
			break;

		case 0x48/8:    /* O1(n-1) */
			voice->o1n1 = (INT32)(data << 14) >> 14;
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "voice %d, O1(n-1)=%05x\n", m_current_page & 0x1f, voice->o1n1 & 0x3ffff);
			break;

		case 0x50/8:    /* W_ST */
			m_wst = data & 0x7f;
			break;

		case 0x58/8:    /* W_END */
			m_wend = data & 0x7f;
			break;

		case 0x60/8:    /* LR_END */
			m_lrend = data & 0x7f;
			break;

		case 0x68/8:    /* PAR - read only */
		case 0x70/8:    /* IRQV - read only */
			break;

		case 0x78/8:    /* PAGE */
			m_current_page = data & 0x7f;
			break;
	}
}

inline void es5506_device::reg_write_test(es550x_voice *voice, offs_t offset, UINT32 data)
{
	switch (offset)
	{
		case 0x00/8:    /* CHANNEL 0 LEFT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 0 left test write %08x\n", data);
			break;

		case 0x08/8:    /* CHANNEL 0 RIGHT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 0 right test write %08x\n", data);
			break;

		case 0x10/8:    /* CHANNEL 1 LEFT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 1 left test write %08x\n", data);
			break;

		case 0x18/8:    /* CHANNEL 1 RIGHT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 1 right test write %08x\n", data);
			break;

		case 0x20/8:    /* CHANNEL 2 LEFT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 2 left test write %08x\n", data);
			break;

		case 0x28/8:    /* CHANNEL 2 RIGHT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 2 right test write %08x\n", data);
			break;

		case 0x30/8:    /* CHANNEL 3 LEFT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 3 left test write %08x\n", data);
			break;

		case 0x38/8:    /* CHANNEL 3 RIGHT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 3 right test write %08x\n", data);
			break;

		case 0x40/8:    /* CHANNEL 4 LEFT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 4 left test write %08x\n", data);
			break;

		case 0x48/8:    /* CHANNEL 4 RIGHT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 4 right test write %08x\n", data);
			break;

		case 0x50/8:    /* CHANNEL 5 LEFT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 5 left test write %08x\n", data);
			break;

		case 0x58/8:    /* CHANNEL 6 RIGHT */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Channel 5 right test write %08x\n", data);
			break;

		case 0x60/8:    /* EMPTY */
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "Test write EMPTY %08x\n", data);
			break;

		case 0x68/8:    /* PAR - read only */
		case 0x70/8:    /* IRQV - read only */
			break;

		case 0x78/8:    /* PAGE */
			m_current_page = data & 0x7f;
			break;
	}
}

WRITE8_MEMBER( es5506_device::write )
{
	es550x_voice *voice = &m_voice[m_current_page & 0x1f];
	int shift = 8 * (offset & 3);

	/* accumulate the data */
	m_write_latch = (m_write_latch & ~(0xff000000 >> shift)) | (data << (24 - shift));

	/* wait for a write to complete */
	if (shift != 24)
		return;

	/* force an update */
	m_stream->update();

	/* switch off the page and register */
	if (m_current_page < 0x20)
		reg_write_low(voice, offset / 4, m_write_latch);
	else if (m_current_page < 0x40)
		reg_write_high(voice, offset / 4, m_write_latch);
	else
		reg_write_test(voice, offset / 4, m_write_latch);

	/* clear the write latch when done */
	m_write_latch = 0;
}



/**********************************************************************************************

     reg_read -- read from the specified ES5506 register

***********************************************************************************************/

inline UINT32 es5506_device::reg_read_low(es550x_voice *voice, offs_t offset)
{
	UINT32 result = 0;

	switch (offset)
	{
		case 0x00/8:    /* CR */
			result = voice->control;
			break;

		case 0x08/8:    /* FC */
			result = voice->freqcount;
			break;

		case 0x10/8:    /* LVOL */
			result = voice->lvol;
			break;

		case 0x18/8:    /* LVRAMP */
			result = voice->lvramp << 8;
			break;

		case 0x20/8:    /* RVOL */
			result = voice->rvol;
			break;

		case 0x28/8:    /* RVRAMP */
			result = voice->rvramp << 8;
			break;

		case 0x30/8:    /* ECOUNT */
			result = voice->ecount;
			break;

		case 0x38/8:    /* K2 */
			result = voice->k2;
			break;

		case 0x40/8:    /* K2RAMP */
			result = (voice->k2ramp << 8) | (voice->k2ramp >> 31);
			break;

		case 0x48/8:    /* K1 */
			result = voice->k1;
			break;

		case 0x50/8:    /* K1RAMP */
			result = (voice->k1ramp << 8) | (voice->k1ramp >> 31);
			break;

		case 0x58/8:    /* ACTV */
			result = m_active_voices;
			break;

		case 0x60/8:    /* MODE */
			result = m_mode;
			break;

		case 0x68/8:    /* PAR */
			if (!m_read_port_cb.isnull())
				result = m_read_port_cb(0);
			break;

		case 0x70/8:    /* IRQV */
			result = m_irqv;
			update_internal_irq_state();
			break;

		case 0x78/8:    /* PAGE */
			result = m_current_page;
			break;
	}
	return result;
}


inline UINT32 es5506_device::reg_read_high(es550x_voice *voice, offs_t offset)
{
	UINT32 result = 0;

	switch (offset)
	{
		case 0x00/8:    /* CR */
			result = voice->control;
			break;

		case 0x08/8:    /* START */
			result = voice->start;
			break;

		case 0x10/8:    /* END */
			result = voice->end;
			break;

		case 0x18/8:    /* ACCUM */
			result = voice->accum;
			break;

		case 0x20/8:    /* O4(n-1) */
			result = voice->o4n1 & 0x3ffff;
			break;

		case 0x28/8:    /* O3(n-1) */
			result = voice->o3n1 & 0x3ffff;
			break;

		case 0x30/8:    /* O3(n-2) */
			result = voice->o3n2 & 0x3ffff;
			break;

		case 0x38/8:    /* O2(n-1) */
			result = voice->o2n1 & 0x3ffff;
			break;

		case 0x40/8:    /* O2(n-2) */
			result = voice->o2n2 & 0x3ffff;
			break;

		case 0x48/8:    /* O1(n-1) */
			result = voice->o1n1 & 0x3ffff;
			break;

		case 0x50/8:    /* W_ST */
			result = m_wst;
			break;

		case 0x58/8:    /* W_END */
			result = m_wend;
			break;

		case 0x60/8:    /* LR_END */
			result = m_lrend;
			break;

		case 0x68/8:    /* PAR */
			if (!m_read_port_cb.isnull())
				result = m_read_port_cb(0);
			break;

		case 0x70/8:    /* IRQV */
			result = m_irqv;
			update_internal_irq_state();
			break;

		case 0x78/8:    /* PAGE */
			result = m_current_page;
			break;
	}
	return result;
}
inline UINT32 es5506_device::reg_read_test(es550x_voice *voice, offs_t offset)
{
	UINT32 result = 0;

	switch (offset)
	{
		case 0x68/8:    /* PAR */
			if (!m_read_port_cb.isnull())
				result = m_read_port_cb(0);
			break;

		case 0x70/8:    /* IRQV */
			result = m_irqv;
			break;

		case 0x78/8:    /* PAGE */
			result = m_current_page;
			break;
	}
	return result;
}

READ8_MEMBER( es5506_device::read )
{
	es550x_voice *voice = &m_voice[m_current_page & 0x1f];
	int shift = 8 * (offset & 3);

	/* only read on offset 0 */
	if (shift != 0)
		return m_read_latch >> (24 - shift);

	if (LOG_COMMANDS && m_eslog)
		fprintf(m_eslog, "read from %02x/%02x -> ", m_current_page, offset / 4 * 8);

	/* force an update */
	m_stream->update();

	/* switch off the page and register */
	if (m_current_page < 0x20)
		m_read_latch = reg_read_low(voice, offset / 4);
	else if (m_current_page < 0x40)
		m_read_latch = reg_read_high(voice, offset / 4);
	else
		m_read_latch = reg_read_test(voice, offset / 4);

	if (LOG_COMMANDS && m_eslog)
		fprintf(m_eslog, "%08x\n", m_read_latch);

	/* return the high byte */
	return m_read_latch >> 24;
}



void es5506_device::voice_bank_w(int voice, int bank)
{
	m_voice[voice].exbank=bank;
}


/**********************************************************************************************

     reg_write -- handle a write to the selected ES5505 register

***********************************************************************************************/

inline void es5505_device::reg_write_low(es550x_voice *voice, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	switch (offset)
	{
		case 0x00:  /* CR */
			if (ACCESSING_BITS_0_7)
			{
#if RAINE_CHECK
				voice->control &= ~(CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_DIR);
#else
				voice->control &= ~(CONTROL_STOPMASK | CONTROL_BS0 | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ);
#endif
				voice->control |= (data & (CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ)) |
									((data << 12) & CONTROL_BS0);
			}
			if (ACCESSING_BITS_8_15)
			{
				voice->control &= ~(CONTROL_CA0 | CONTROL_CA1 | CONTROL_LPMASK);
				voice->control |= ((data >> 2) & CONTROL_LPMASK) |
									((data << 2) & (CONTROL_CA0 | CONTROL_CA1));
			}

			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, control=%04x (raw=%04x & %04x)\n", machine().describe_context(), m_current_page & 0x1f, voice->control, data, mem_mask ^ 0xffff);
			break;

		case 0x01:  /* FC */
			if (ACCESSING_BITS_0_7)
				voice->freqcount = (voice->freqcount & ~0x001fe) | ((data & 0x00ff) << 1);
			if (ACCESSING_BITS_8_15)
				voice->freqcount = (voice->freqcount & ~0x1fe00) | ((data & 0xff00) << 1);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, freq count=%08x\n", machine().describe_context(), m_current_page & 0x1f, voice->freqcount);
			break;

		case 0x02:  /* STRT (hi) */
			if (ACCESSING_BITS_0_7)
				voice->start = (voice->start & ~0x03fc0000) | ((data & 0x00ff) << 18);
			if (ACCESSING_BITS_8_15)
				voice->start = (voice->start & ~0x7c000000) | ((data & 0x1f00) << 18);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, loop start=%08x\n", machine().describe_context(), m_current_page & 0x1f, voice->start);
			break;

		case 0x03:  /* STRT (lo) */
			if (ACCESSING_BITS_0_7)
				voice->start = (voice->start & ~0x00000380) | ((data & 0x00e0) << 2);
			if (ACCESSING_BITS_8_15)
				voice->start = (voice->start & ~0x0003fc00) | ((data & 0xff00) << 2);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, loop start=%08x\n", machine().describe_context(), m_current_page & 0x1f, voice->start);
			break;

		case 0x04:  /* END (hi) */
			if (ACCESSING_BITS_0_7)
				voice->end = (voice->end & ~0x03fc0000) | ((data & 0x00ff) << 18);
			if (ACCESSING_BITS_8_15)
				voice->end = (voice->end & ~0x7c000000) | ((data & 0x1f00) << 18);
#if RAINE_CHECK
			voice->control |= CONTROL_STOP0;
#endif
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, loop end=%08x\n", machine().describe_context(), m_current_page & 0x1f, voice->end);
			break;

		case 0x05:  /* END (lo) */
			if (ACCESSING_BITS_0_7)
				voice->end = (voice->end & ~0x00000380) | ((data & 0x00e0) << 2);
			if (ACCESSING_BITS_8_15)
				voice->end = (voice->end & ~0x0003fc00) | ((data & 0xff00) << 2);
#if RAINE_CHECK
			voice->control |= CONTROL_STOP0;
#endif
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, loop end=%08x\n", machine().describe_context(), m_current_page & 0x1f, voice->end);
			break;

		case 0x06:  /* K2 */
			if (ACCESSING_BITS_0_7)
				voice->k2 = (voice->k2 & ~0x00f0) | (data & 0x00f0);
			if (ACCESSING_BITS_8_15)
				voice->k2 = (voice->k2 & ~0xff00) | (data & 0xff00);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, K2=%04x\n", machine().describe_context(), m_current_page & 0x1f, voice->k2);
			break;

		case 0x07:  /* K1 */
			if (ACCESSING_BITS_0_7)
				voice->k1 = (voice->k1 & ~0x00f0) | (data & 0x00f0);
			if (ACCESSING_BITS_8_15)
				voice->k1 = (voice->k1 & ~0xff00) | (data & 0xff00);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, K1=%04x\n", machine().describe_context(), m_current_page & 0x1f, voice->k1);
			break;

		case 0x08:  /* LVOL */
			if (ACCESSING_BITS_8_15)
				voice->lvol = (voice->lvol & ~0xff00) | (data & 0xff00);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, left vol=%04x\n", machine().describe_context(), m_current_page & 0x1f, voice->lvol);
			break;

		case 0x09:  /* RVOL */
			if (ACCESSING_BITS_8_15)
				voice->rvol = (voice->rvol & ~0xff00) | (data & 0xff00);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, right vol=%04x\n", machine().describe_context(), m_current_page & 0x1f, voice->rvol);
			break;

		case 0x0a:  /* ACC (hi) */
			if (ACCESSING_BITS_0_7)
				voice->accum = (voice->accum & ~0x03fc0000) | ((data & 0x00ff) << 18);
			if (ACCESSING_BITS_8_15)
				voice->accum = (voice->accum & ~0x7c000000) | ((data & 0x1f00) << 18);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, accum=%08x\n", machine().describe_context(), m_current_page & 0x1f, voice->accum);
			break;

		case 0x0b:  /* ACC (lo) */
			if (ACCESSING_BITS_0_7)
				voice->accum = (voice->accum & ~0x000003fc) | ((data & 0x00ff) << 2);
			if (ACCESSING_BITS_8_15)
				voice->accum = (voice->accum & ~0x0003fc00) | ((data & 0xff00) << 2);
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, accum=%08x\n", machine().describe_context(), m_current_page & 0x1f, voice->accum);
			break;

		case 0x0c:  /* unused */
			break;

		case 0x0d:  /* ACT */
			if (ACCESSING_BITS_0_7)
			{
				m_active_voices = data & 0x1f;
				m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
				m_stream->set_sample_rate(m_sample_rate);

				if (LOG_COMMANDS && m_eslog)
					fprintf(m_eslog, "active voices=%d, sample_rate=%d\n", m_active_voices, m_sample_rate);
			}
			break;

		case 0x0e:  /* IRQV - read only */
			break;

		case 0x0f:  /* PAGE */
			if (ACCESSING_BITS_0_7)
				m_current_page = data & 0x7f;
			break;
	}
}


inline void es5505_device::reg_write_high(es550x_voice *voice, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	switch (offset)
	{
		case 0x00:  /* CR */
			if (ACCESSING_BITS_0_7)
			{
				voice->control &= ~(CONTROL_STOPMASK | CONTROL_BS0 | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ);
				voice->control |= (data & (CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ)) |
									((data << 12) & CONTROL_BS0);
			}
			if (ACCESSING_BITS_8_15)
			{
				voice->control &= ~(CONTROL_CA0 | CONTROL_CA1 | CONTROL_LPMASK);
				voice->control |= ((data >> 2) & CONTROL_LPMASK) |
									((data << 2) & (CONTROL_CA0 | CONTROL_CA1));
			}
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, control=%04x (raw=%04x & %04x)\n", machine().describe_context(), m_current_page & 0x1f, voice->control, data, mem_mask);
			break;

		case 0x01:  /* O4(n-1) */
			if (ACCESSING_BITS_0_7)
				voice->o4n1 = (voice->o4n1 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o4n1 = (INT16)((voice->o4n1 & ~0xff00) | (data & 0xff00));
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, O4(n-1)=%05x\n", machine().describe_context(), m_current_page & 0x1f, voice->o4n1 & 0x3ffff);
			break;

		case 0x02:  /* O3(n-1) */
			if (ACCESSING_BITS_0_7)
				voice->o3n1 = (voice->o3n1 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o3n1 = (INT16)((voice->o3n1 & ~0xff00) | (data & 0xff00));
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, O3(n-1)=%05x\n", machine().describe_context(), m_current_page & 0x1f, voice->o3n1 & 0x3ffff);
			break;

		case 0x03:  /* O3(n-2) */
			if (ACCESSING_BITS_0_7)
				voice->o3n2 = (voice->o3n2 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o3n2 = (INT16)((voice->o3n2 & ~0xff00) | (data & 0xff00));
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, O3(n-2)=%05x\n", machine().describe_context(), m_current_page & 0x1f, voice->o3n2 & 0x3ffff);
			break;

		case 0x04:  /* O2(n-1) */
			if (ACCESSING_BITS_0_7)
				voice->o2n1 = (voice->o2n1 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o2n1 = (INT16)((voice->o2n1 & ~0xff00) | (data & 0xff00));
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, O2(n-1)=%05x\n", machine().describe_context(), m_current_page & 0x1f, voice->o2n1 & 0x3ffff);
			break;

		case 0x05:  /* O2(n-2) */
			if (ACCESSING_BITS_0_7)
				voice->o2n2 = (voice->o2n2 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o2n2 = (INT16)((voice->o2n2 & ~0xff00) | (data & 0xff00));
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, O2(n-2)=%05x\n", machine().describe_context(), m_current_page & 0x1f, voice->o2n2 & 0x3ffff);
			break;

		case 0x06:  /* O1(n-1) */
			if (ACCESSING_BITS_0_7)
				voice->o1n1 = (voice->o1n1 & ~0x00ff) | (data & 0x00ff);
			if (ACCESSING_BITS_8_15)
				voice->o1n1 = (INT16)((voice->o1n1 & ~0xff00) | (data & 0xff00));
			if (LOG_COMMANDS && m_eslog)
				fprintf(m_eslog, "%s:voice %d, O1(n-1)=%05x (accum=%08x)\n", machine().describe_context(), m_current_page & 0x1f, voice->o2n1 & 0x3ffff, voice->accum);
			break;

		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:  /* unused */
			break;

		case 0x0d:  /* ACT */
			if (ACCESSING_BITS_0_7)
			{
				m_active_voices = data & 0x1f;
				m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
				m_stream->set_sample_rate(m_sample_rate);

				if (LOG_COMMANDS && m_eslog)
					fprintf(m_eslog, "active voices=%d, sample_rate=%d\n", m_active_voices, m_sample_rate);
			}
			break;

		case 0x0e:  /* IRQV - read only */
			break;

		case 0x0f:  /* PAGE */
			if (ACCESSING_BITS_0_7)
				m_current_page = data & 0x7f;
			break;
	}
}


inline void es5505_device::reg_write_test(es550x_voice *voice, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	switch (offset)
	{
		case 0x00:  /* CH0L */
		case 0x01:  /* CH0R */
		case 0x02:  /* CH1L */
		case 0x03:  /* CH1R */
		case 0x04:  /* CH2L */
		case 0x05:  /* CH2R */
		case 0x06:  /* CH3L */
		case 0x07:  /* CH3R */
			break;

		case 0x08:  /* SERMODE */
			m_mode = data & 0x0007;
			break;

		case 0x09:  /* PAR */
			break;

		case 0x0d:  /* ACT */
			if (ACCESSING_BITS_0_7)
			{
				m_active_voices = data & 0x1f;
				m_sample_rate = m_master_clock / (16 * (m_active_voices + 1));
				m_stream->set_sample_rate(m_sample_rate);

				if (LOG_COMMANDS && m_eslog)
					fprintf(m_eslog, "active voices=%d, sample_rate=%d\n", m_active_voices, m_sample_rate);
			}
			break;

		case 0x0e:  /* IRQV - read only */
			break;

		case 0x0f:  /* PAGE */
			if (ACCESSING_BITS_0_7)
				m_current_page = data & 0x7f;
			break;
	}
}


WRITE16_MEMBER( es5505_device::write )
{
	es550x_voice *voice = &m_voice[m_current_page & 0x1f];

//  logerror("%s:ES5505 write %02x/%02x = %04x & %04x\n", machine().describe_context(), m_current_page, offset, data, mem_mask);

	/* force an update */
	m_stream->update();

	/* switch off the page and register */
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

inline UINT16 es5505_device::reg_read_low(es550x_voice *voice, offs_t offset)
{
	UINT16 result = 0;

	switch (offset)
	{
		case 0x00:  /* CR */
			result = (voice->control & (CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ)) |
						((voice->control & CONTROL_BS0) >> 12) |
						((voice->control & CONTROL_LPMASK) << 2) |
						((voice->control & (CONTROL_CA0 | CONTROL_CA1)) >> 2) |
						0xf000;
			break;

		case 0x01:  /* FC */
			result = voice->freqcount >> 1;
			break;

		case 0x02:  /* STRT (hi) */
			result = voice->start >> 18;
			break;

		case 0x03:  /* STRT (lo) */
			result = voice->start >> 2;
			break;

		case 0x04:  /* END (hi) */
			result = voice->end >> 18;
			break;

		case 0x05:  /* END (lo) */
			result = voice->end >> 2;
			break;

		case 0x06:  /* K2 */
			result = voice->k2;
			break;

		case 0x07:  /* K1 */
			result = voice->k1;
			break;

		case 0x08:  /* LVOL */
			result = voice->lvol;
			break;

		case 0x09:  /* RVOL */
			result = voice->rvol;
			break;

		case 0x0a:  /* ACC (hi) */
			result = voice->accum >> 18;
			break;

		case 0x0b:  /* ACC (lo) */
			result = voice->accum >> 2;
			break;

		case 0x0c:  /* unused */
			break;

		case 0x0d:  /* ACT */
			result = m_active_voices;
			break;

		case 0x0e:  /* IRQV */
			result = m_irqv;
			update_internal_irq_state();
			break;

		case 0x0f:  /* PAGE */
			result = m_current_page;
			break;
	}
	return result;
}


inline UINT16 es5505_device::reg_read_high(es550x_voice *voice, offs_t offset)
{
	UINT16 result = 0;

	switch (offset)
	{
		case 0x00:  /* CR */
			result = (voice->control & (CONTROL_STOPMASK | CONTROL_LOOPMASK | CONTROL_IRQE | CONTROL_DIR | CONTROL_IRQ)) |
						((voice->control & CONTROL_BS0) >> 12) |
						((voice->control & CONTROL_LPMASK) << 2) |
						((voice->control & (CONTROL_CA0 | CONTROL_CA1)) >> 2) |
						0xf000;
			break;

		case 0x01:  /* O4(n-1) */
			result = voice->o4n1;
			break;

		case 0x02:  /* O3(n-1) */
			result = voice->o3n1;
			break;

		case 0x03:  /* O3(n-2) */
			result = voice->o3n2;
			break;

		case 0x04:  /* O2(n-1) */
			result = voice->o2n1;
			break;

		case 0x05:  /* O2(n-2) */
			result = voice->o2n2;
			break;

		case 0x06:  /* O1(n-1) */
			/* special case for the Taito F3 games: they set the accumulator on a stopped */
			/* voice and assume the filters continue to process the data. They then read */
			/* the O1(n-1) in order to extract raw data from the sound ROMs. Since we don't */
			/* want to waste time filtering stopped channels, we just look for a read from */
			/* this register on a stopped voice, and return the raw sample data at the */
			/* accumulator */
			if ((voice->control & CONTROL_STOPMASK) && m_region_base[voice->control >> 14])
			{
				voice->o1n1 = m_region_base[voice->control >> 14][voice->exbank + (voice->accum >> 11)];
				// logerror("%02x %08x ==> %08x\n",voice->o1n1,voice->control >> 14,voice->exbank + (voice->accum >> 11));
			}
			result = voice->o1n1;
			break;

		case 0x07:
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:  /* unused */
			break;

		case 0x0d:  /* ACT */
			result = m_active_voices;
			break;

		case 0x0e:  /* IRQV */
			result = m_irqv;
			update_internal_irq_state();
			break;

		case 0x0f:  /* PAGE */
			result = m_current_page;
			break;
	}
	return result;
}


inline UINT16 es5505_device::reg_read_test(es550x_voice *voice, offs_t offset)
{
	UINT16 result = 0;

	switch (offset)
	{
		case 0x00:  /* CH0L */
		case 0x01:  /* CH0R */
		case 0x02:  /* CH1L */
		case 0x03:  /* CH1R */
		case 0x04:  /* CH2L */
		case 0x05:  /* CH2R */
		case 0x06:  /* CH3L */
		case 0x07:  /* CH3R */
			break;

		case 0x08:  /* SERMODE */
			result = m_mode;
			break;

		case 0x09:  /* PAR */
			if (!m_read_port_cb.isnull())
				result = m_read_port_cb(0);
			break;

		case 0x0f:  /* PAGE */
			result = m_current_page;
			break;
	}
	return result;
}


READ16_MEMBER( es5505_device::read )
{
	es550x_voice *voice = &m_voice[m_current_page & 0x1f];
	UINT16 result = 0;

	if (LOG_COMMANDS && m_eslog)
		fprintf(m_eslog, "read from %02x/%02x -> ", m_current_page, offset);

	/* force an update */
	m_stream->update();

	/* switch off the page and register */
	if (m_current_page < 0x20)
		result = reg_read_low(voice, offset);
	else if (m_current_page < 0x40)
		result = reg_read_high(voice, offset);
	else
		result = reg_read_test(voice, offset);

	if (LOG_COMMANDS && m_eslog)
		fprintf(m_eslog, "%04x (accum=%08x)\n", result, voice->accum);

	/* return the high byte */
	return result;
}



void es5505_device::voice_bank_w(int voice, int bank)
{
#if RAINE_CHECK
	m_voice[voice].control = CONTROL_STOPMASK;
#endif
	m_voice[voice].exbank=bank;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void es550x_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}

void es5506_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
#if MAKE_WAVS
	/* start the logging once we have a sample rate */
	if (m_sample_rate)
	{
		if (!m_wavraw)
			m_wavraw = wav_open("raw.wav", m_sample_rate, 2);
	}
#endif

	/* loop until all samples are output */
	int offset = 0;
	while (samples)
	{
		int length = (samples > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : samples;

		generate_samples(outputs, offset, length);

#if MAKE_WAVS
		/* log the raw data */
		if (m_wavraw) {
			/* determine left/right source data */
			INT32 *lsrc = m_scratch, *rsrc = m_scratch + length;
			int channel;
			memset(lsrc, 0, sizeof(INT32) * length * 2);
			/* loop over the output channels */
			for (channel = 0; channel < m_channels; channel++) {
				INT32 *l = outputs[(channel << 1)] + offset;
				INT32 *r = outputs[(channel << 1) + 1] + offset;
				/* add the current channel's samples to the WAV data */
				for (samp = 0; samp < length; samp++) {
					lsrc[samp] += l[samp];
					rsrc[samp] += r[samp];
				}
			}
			wav_add_data_32lr(m_wavraw, lsrc, rsrc, length, 4);
		}
#endif

		/* account for these samples */
		offset += length;
		samples -= length;
	}
}

void es5505_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
#if MAKE_WAVS
	/* start the logging once we have a sample rate */
	if (m_sample_rate)
	{
		if (!m_wavraw)
			m_wavraw = wav_open("raw.wav", m_sample_rate, 2);
	}
#endif

	/* loop until all samples are output */
	int offset = 0;
	while (samples)
	{
		int length = (samples > MAX_SAMPLE_CHUNK) ? MAX_SAMPLE_CHUNK : samples;

		generate_samples(outputs, offset, length);

#if MAKE_WAVS
		/* log the raw data */
		if (m_wavraw) {
			/* determine left/right source data */
			INT32 *lsrc = m_scratch, *rsrc = m_scratch + length;
			int channel;
			memset(lsrc, 0, sizeof(INT32) * length * 2);
			/* loop over the output channels */
			for (channel = 0; channel < m_channels; channel++) {
				INT32 *l = outputs[(channel << 1)] + offset;
				INT32 *r = outputs[(channel << 1) + 1] + offset;
				/* add the current channel's samples to the WAV data */
				for (samp = 0; samp < length; samp++) {
					lsrc[samp] += l[samp];
					rsrc[samp] += r[samp];
				}
			}
			wav_add_data_32lr(m_wavraw, lsrc, rsrc, length, 4);
		}
#endif

		/* account for these samples */
		offset += length;
		samples -= length;
	}
}
