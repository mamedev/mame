// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#include "emu.h"
#include "audio/exidy.h"

#include "cpu/z80/z80.h"
#include "machine/input_merger.h"
#include "machine/rescap.h"
#include "cpu/m6502/m6502.h"
#include "speaker.h"



/*************************************
 *
 *  Constants
 *
 *************************************/

#define CRYSTAL_OSC             (XTAL(3'579'545))
#define SH8253_CLOCK            (CRYSTAL_OSC / 2)
#define SH6840_CLOCK            (CRYSTAL_OSC / 4)
#define SH6532_CLOCK            (CRYSTAL_OSC / 4)
#define CVSD_CLOCK              (1.0 / (0.693 * (RES_K(2.4) + 2.0 * RES_K(20)) * CAP_P(2200)))
#define CVSD_Z80_CLOCK          (CRYSTAL_OSC / 2)
#define BASE_VOLUME             (32767 / 6)


/*************************************
 *
 *  6840 clock counting helper
 *
 *************************************/

inline void exidy_sound_device::sh6840_apply_clock(exidy_sound_device::sh6840_timer_channel *t, int clocks)
{
	/* dual 8-bit case */
	if (t->cr & 0x04)
	{
		/* handle full decrements */
		while (clocks > t->counter.b.l)
		{
			clocks -= t->counter.b.l + 1;
			t->counter.b.l = t->timer;

			/* decrement MSB */
			if (!t->counter.b.h--)
			{
				t->state = 0;
				t->counter.w = t->timer;
			}

			/* state goes high when MSB is 0 */
			else if (!t->counter.b.h)
			{
				t->state = 1;
				t->clocks++;
			}
		}

		/* subtract off the remainder */
		t->counter.b.l -= clocks;
	}

	/* 16-bit case */
	else
	{
		/* handle full decrements */
		while (clocks > t->counter.w)
		{
			clocks -= t->counter.w + 1;
			t->state ^= 1;
			t->clocks += t->state;
			t->counter.w = t->timer;
		}

		/* subtract off the remainder */
		t->counter.w -= clocks;
	}
}



/*************************************
 *
 *  Noise generation helper
 *
 *************************************/

inline int exidy_sound_device::sh6840_update_noise(int clocks)
{
	uint32_t newxor;
	int noise_clocks = 0;
	int i;

	/* loop over clocks */
	for (i = 0; i < clocks; i++)
	{
		/* shift the LFSR. its a LOOOONG LFSR, so we need
		* four longs to hold it all!
		* first we grab new sample, then shift the high bits,
		* then the low ones; finally or in the result and see if we've
		* had a 0->1 transition */
		newxor = (m_sh6840_LFSR_3 ^ m_sh6840_LFSR_2) >> 31; /* high bits of 3 and 2 xored is new xor */
		m_sh6840_LFSR_3 <<= 1;
		m_sh6840_LFSR_3 |= m_sh6840_LFSR_2 >> 31;
		m_sh6840_LFSR_2 <<= 1;
		m_sh6840_LFSR_2 |= m_sh6840_LFSR_1 >> 31;
		m_sh6840_LFSR_1 <<= 1;
		m_sh6840_LFSR_1 |= m_sh6840_LFSR_0 >> 31;
		m_sh6840_LFSR_0 <<= 1;
		m_sh6840_LFSR_0 |= newxor ^ m_sh6840_LFSR_oldxor;
		m_sh6840_LFSR_oldxor = newxor;
		/*printf("LFSR: %4x, %4x, %4x, %4x\n", sh6840_LFSR_3, sh6840_LFSR_2, sh6840_LFSR_1, sh6840_LFSR_0);*/
		/* if we clocked 0->1, that will serve as an external clock */
		if ((m_sh6840_LFSR_2 & 0x03) == 0x01) /* tap is at 96th bit */
		{
			noise_clocks++;
		}
	}
	return noise_clocks;
}



/*************************************
 *
 *  6840 state saving
 *
 *************************************/

void exidy_sound_device::sh6840_register_state_globals()
{
	save_item(NAME(m_sh6840_volume));
	save_item(NAME(m_sh6840_MSB_latch));
	save_item(NAME(m_sh6840_LSB_latch));
	save_item(NAME(m_sh6840_LFSR_oldxor));
	save_item(NAME(m_sh6840_LFSR_0));
	save_item(NAME(m_sh6840_LFSR_1));
	save_item(NAME(m_sh6840_LFSR_2));
	save_item(NAME(m_sh6840_LFSR_3));
	save_item(NAME(m_sh6840_clock_count));
	save_item(NAME(m_sfxctrl));
	save_item(NAME(m_sh6840_timer[0].cr));
	save_item(NAME(m_sh6840_timer[0].state));
	save_item(NAME(m_sh6840_timer[0].leftovers));
	save_item(NAME(m_sh6840_timer[0].timer));
	save_item(NAME(m_sh6840_timer[0].clocks));
	save_item(NAME(m_sh6840_timer[0].counter.w));
	save_item(NAME(m_sh6840_timer[1].cr));
	save_item(NAME(m_sh6840_timer[1].state));
	save_item(NAME(m_sh6840_timer[1].leftovers));
	save_item(NAME(m_sh6840_timer[1].timer));
	save_item(NAME(m_sh6840_timer[1].clocks));
	save_item(NAME(m_sh6840_timer[1].counter.w));
	save_item(NAME(m_sh6840_timer[2].cr));
	save_item(NAME(m_sh6840_timer[2].state));
	save_item(NAME(m_sh6840_timer[2].leftovers));
	save_item(NAME(m_sh6840_timer[2].timer));
	save_item(NAME(m_sh6840_timer[2].clocks));
	save_item(NAME(m_sh6840_timer[2].counter.w));
}

/*************************************
 *
 *  Audio startup routines
 *
 *************************************/

void exidy_sound_device::common_sh_start()
{
	int sample_rate = SH8253_CLOCK.value();

	m_sh6840_clocks_per_sample = (int)(SH6840_CLOCK.dvalue() / (double)sample_rate * (double)(1 << 24));

	/* allocate the stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 1, sample_rate);

	sh6840_register_state_globals();
}

DEFINE_DEVICE_TYPE(EXIDY, exidy_sound_device, "exidy_sfx", "Exidy SFX")

exidy_sound_device::exidy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: exidy_sound_device(mconfig, EXIDY, tag, owner, clock)
{
}

exidy_sh8253_sound_device::exidy_sh8253_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: exidy_sound_device(mconfig, type, tag, owner, clock),
		m_riot(*this, "riot"),
		m_cvsd(*this, "cvsd"),
		m_cvsdcpu(*this, "cvsdcpu"),
		m_tms(*this, "tms"),
		m_pia(*this, "pia")
{
}

exidy_sound_device::exidy_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_freq_to_step(0),
		m_sh6840_MSB_latch(0),
		m_sh6840_LSB_latch(0),
		m_sh6840_LFSR_oldxor(0),
		m_sh6840_LFSR_0(0xffffffff),
		m_sh6840_LFSR_1(0xffffffff),
		m_sh6840_LFSR_2(0xffffffff),
		m_sh6840_LFSR_3(0xffffffff),
		m_sh6840_clocks_per_sample(0),
		m_sh6840_clock_count(0),
		m_sfxctrl(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void exidy_sound_device::device_start()
{
	common_sh_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void exidy_sound_device::device_reset()
{
	common_sh_reset();
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void exidy_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	sh6840_timer_channel *sh6840_timer = m_sh6840_timer;

	/* hack to skip the expensive lfsr noise generation unless at least one of the 3 channels actually depends on it being generated */
	int noisy = ((sh6840_timer[0].cr & sh6840_timer[1].cr & sh6840_timer[2].cr & 0x02) == 0);
	stream_sample_t *buffer = outputs[0];

	/* loop over samples */
	while (samples--)
	{
		sh6840_timer_channel *t;
		int clocks;
		stream_sample_t sample = 0;

		/* determine how many 6840 clocks this sample */
		m_sh6840_clock_count += m_sh6840_clocks_per_sample;
		int clocks_this_sample = m_sh6840_clock_count >> 24;
		m_sh6840_clock_count &= (1 << 24) - 1;

		/* skip if nothing enabled */
		if ((sh6840_timer[0].cr & 0x01) == 0)
		{
			int noise_clocks_this_sample = 0;
			uint32_t chan0_clocks;

			/* generate E-clocked noise if configured to do so */
			if (noisy && !(m_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(clocks_this_sample);

			/* handle timer 0 if enabled */
			t = &sh6840_timer[0];
			chan0_clocks = t->clocks;
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			sh6840_apply_clock(t, clocks);
			if (t->state && !(m_sfxctrl & 0x02) && (t->cr & 0x80))
				sample += m_sh6840_volume[0];

			/* generate channel 0-clocked noise if configured to do so */
			if (noisy && (m_sfxctrl & 0x01))
				noise_clocks_this_sample = sh6840_update_noise(t->clocks - chan0_clocks);

			/* handle timer 1 if enabled */
			t = &sh6840_timer[1];
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			sh6840_apply_clock(t, clocks);
			if (t->state && (t->cr & 0x80))
				sample += m_sh6840_volume[1];

			/* handle timer 2 if enabled */
			t = &sh6840_timer[2];
			clocks = (t->cr & 0x02) ? clocks_this_sample : noise_clocks_this_sample;
			/* prescale */
			if (t->cr & 0x01)
			{
				clocks += t->leftovers;
				t->leftovers = clocks % 8;
				clocks /= 8;
			}
			sh6840_apply_clock(t, clocks);
			if (t->state && (t->cr & 0x80))
				sample += m_sh6840_volume[2];
		}

		/* music (if present) */
		sample += generate_music_sample();

		/* stash */
		*buffer++ = sample;
	}
}

stream_sample_t exidy_sound_device::generate_music_sample()
{
	return 0;
}

stream_sample_t exidy_sh8253_sound_device::generate_music_sample()
{
	sh8253_timer_channel *c;
	stream_sample_t sample = 0;

	/* music channel 0 */
	c = &m_sh8253_timer[0];
	if (c->enable)
	{
		c->fraction += c->step;
		if (c->fraction & 0x0800000)
			sample += BASE_VOLUME;
	}

	/* music channel 1 */
	c = &m_sh8253_timer[1];
	if (c->enable)
	{
		c->fraction += c->step;
		if (c->fraction & 0x0800000)
			sample += BASE_VOLUME;
	}

	/* music channel 2 */
	c = &m_sh8253_timer[2];
	if (c->enable)
	{
		c->fraction += c->step;
		if (c->fraction & 0x0800000)
			sample += BASE_VOLUME;
	}

	return sample;
}



/*************************************
 *
 *  Audio reset routines
 *
 *************************************/

void exidy_sound_device::common_sh_reset()
{
	/* 6840 */
	memset(m_sh6840_timer, 0, sizeof(m_sh6840_timer));
	m_sh6840_MSB_latch = 0;
	m_sh6840_LSB_latch = 0;
	m_sh6840_volume[0] = 0;
	m_sh6840_volume[1] = 0;
	m_sh6840_volume[2] = 0;
	m_sh6840_clock_count = 0;
	m_sfxctrl = 0;

	/* LFSR */
	m_sh6840_LFSR_oldxor = 0;
	m_sh6840_LFSR_0 = 0xffffffff;
	m_sh6840_LFSR_1 = 0xffffffff;
	m_sh6840_LFSR_2 = 0xffffffff;
	m_sh6840_LFSR_3 = 0xffffffff;
}


/*************************************
 *
 *  6532 interface
 *
 *************************************/

WRITE8_MEMBER(exidy_sh8253_sound_device::r6532_porta_w)
{
	if (m_cvsd.found())
		m_cvsdcpu->set_input_line(INPUT_LINE_RESET, (data & 0x10) ? CLEAR_LINE : ASSERT_LINE);

	if (m_tms.found())
	{
		logerror("(%f)%s:TMS5220 data write = %02X\n", machine().time().as_double(), machine().describe_context(), m_riot->porta_out_get());
		m_tms->data_w(data);
	}
}

READ8_MEMBER(exidy_sh8253_sound_device::r6532_porta_r)
{
	uint8_t status = 0xff;
	if (m_tms.found())
	{
		status = m_tms->status_r();
		logerror("(%f)%s:TMS5220 status read = %02X\n", machine().time().as_double(), machine().describe_context(), status);
	}
	return status;
}

WRITE8_MEMBER(exidy_sh8253_sound_device::r6532_portb_w)
{
	if (m_tms.found())
	{
		m_tms->rsq_w(BIT(data, 0));
		m_tms->wsq_w(BIT(data, 1));
	}
}


READ8_MEMBER(exidy_sh8253_sound_device::r6532_portb_r)
{
	uint8_t newdata = m_riot->portb_in_get();
	if (m_tms.found())
	{
		newdata &= ~0x0c;
		if (m_tms->readyq_r()) newdata |= 0x04;
		if (m_tms->intq_r()) newdata |= 0x08;
	}
	return newdata;
}


/*************************************
 *
 *  8253 state saving
 *
 *************************************/


void exidy_sh8253_sound_device::sh8253_register_state_globals()
{
	save_item(NAME(m_sh8253_timer[0].clstate));
	save_item(NAME(m_sh8253_timer[0].enable));
	save_item(NAME(m_sh8253_timer[0].count));
	save_item(NAME(m_sh8253_timer[0].step));
	save_item(NAME(m_sh8253_timer[0].fraction));
	save_item(NAME(m_sh8253_timer[1].clstate));
	save_item(NAME(m_sh8253_timer[1].enable));
	save_item(NAME(m_sh8253_timer[1].count));
	save_item(NAME(m_sh8253_timer[1].step));
	save_item(NAME(m_sh8253_timer[1].fraction));
	save_item(NAME(m_sh8253_timer[2].clstate));
	save_item(NAME(m_sh8253_timer[2].enable));
	save_item(NAME(m_sh8253_timer[2].count));
	save_item(NAME(m_sh8253_timer[2].step));
	save_item(NAME(m_sh8253_timer[2].fraction));
}

/*************************************
 *
 *  8253 timer handlers
 *
 *************************************/

WRITE8_MEMBER(exidy_sh8253_sound_device::sh8253_w)
{
	int chan;

	m_stream->update();

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			chan = offset;
			if (!m_sh8253_timer[chan].clstate)
			{
				m_sh8253_timer[chan].clstate = 1;
				m_sh8253_timer[chan].count = (m_sh8253_timer[chan].count & 0xff00) | (data & 0x00ff);
			}
			else
			{
				m_sh8253_timer[chan].clstate = 0;
				m_sh8253_timer[chan].count = (m_sh8253_timer[chan].count & 0x00ff) | ((data << 8) & 0xff00);
				if (m_sh8253_timer[chan].count)
					m_sh8253_timer[chan].step = m_freq_to_step * SH8253_CLOCK.dvalue() / m_sh8253_timer[chan].count;
				else
					m_sh8253_timer[chan].step = 0;
			}
			break;

		case 3:
			chan = (data & 0xc0) >> 6;
			m_sh8253_timer[chan].enable = ((data & 0x0e) != 0);
			break;
	}
}



/*************************************
 *
 *  6840 timer handlers
 *
 *************************************/

READ8_MEMBER(exidy_sound_device::sh6840_r)
{
	/* force an update of the stream */
	m_stream->update();

	switch (offset)
	{
		/* offset 0: Motorola datasheet says it isn't used, Hitachi datasheet says it reads as 0s always*/
		case 0:
		return 0;
		/* offset 1 reads the status register: bits 2 1 0 correspond to ints on channels 2,1,0, and bit 7 is an 'OR' of bits 2,1,0 */
		case 1:
		logerror("%s:exidy_sh6840_r - unexpected read, status register is TODO!\n", machine().describe_context());
		return 0;
		/* offsets 2,4,6 read channel 0,1,2 MSBs and latch the LSB*/
		case 2: case 4: case 6:
		m_sh6840_LSB_latch = m_sh6840_timer[((offset>>1)-1)].counter.b.l;
		return m_sh6840_timer[((offset>>1)-1)].counter.b.h;
		/* offsets 3,5,7 read the LSB latch*/
		default: /* case 3,5,7 */
		return m_sh6840_LSB_latch;
	}
}


WRITE8_MEMBER(exidy_sound_device::sh6840_w)
{
	sh6840_timer_channel *sh6840_timer = m_sh6840_timer;

	/* force an update of the stream */
	m_stream->update();

	switch (offset)
	{
		/* offset 0 writes to either channel 0 control or channel 2 control */
		case 0:
			if (sh6840_timer[1].cr & 0x01)
				sh6840_timer[0].cr = data;
			else
				sh6840_timer[2].cr = data;

			/* only support mode 0 and 2 */
			if (((data >> 3) & 5) != 0)
				fatalerror("exidy_sh6840_w - channel %d configured for mode %d\n", (sh6840_timer[1].cr & 0x01) ? 0 : 2, (data >> 3) & 7);
			break;

		/* offset 1 writes to channel 1 control */
		case 1:
			sh6840_timer[1].cr = data;

			/* only support mode 0 and 2 */
			if (((data >> 3) & 5) != 0)
				fatalerror("exidy_sh6840_w - channel 1 configured for mode %d\n", (data >> 3) & 7);
			break;

		/* offsets 2/4/6 write to the common MSB latch */
		case 2:
		case 4:
		case 6:
			m_sh6840_MSB_latch = data;
			break;

		/* offsets 3/5/7 write to the LSB controls */
		case 3:
		case 5:
		case 7:
		{
			/* latch the timer value */
			int ch = (offset - 3) / 2;
			sh6840_timer[ch].timer = (m_sh6840_MSB_latch << 8) | (data & 0xff);

			/* if CR4 is clear, the value is loaded immediately */
			if (!(sh6840_timer[ch].cr & 0x10))
				sh6840_timer[ch].counter.w = sh6840_timer[ch].timer;
			break;
		}
	}
}



/*************************************
 *
 *  External sound effect controls
 *
 *************************************/

WRITE8_MEMBER(exidy_sound_device::sfxctrl_w)
{
	m_stream->update();

	switch (offset)
	{
		case 0:
			m_sfxctrl = data;
			break;

		case 1:
		case 2:
		case 3:
			m_sh6840_volume[offset - 1] = ((data & 7) * BASE_VOLUME) / 7;
			break;
	}
}



/*************************************
 *
 *  Sound filter control
 *
 *************************************/

WRITE8_MEMBER(venture_sound_device::filter_w)
{
	logerror("exidy_sound_filter_w = %02X\n", data);
}



/*************************************
 *
 *  Venture, etc.
 *
 *************************************/


DEFINE_DEVICE_TYPE(EXIDY_VENTURE, venture_sound_device, "venture_sound", "Exidy SFX+PSG")

venture_sound_device::venture_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: venture_sound_device(mconfig, EXIDY_VENTURE, tag, owner, clock)
{
}

venture_sound_device::venture_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: exidy_sh8253_sound_device(mconfig, type, tag, owner, clock)
	, m_pa_callback(*this)
	, m_pb_callback(*this)
	, m_ca2_callback(*this)
	, m_cb2_callback(*this)
{
}

void venture_sound_device::device_resolve_objects()
{
	m_pa_callback.resolve_safe();
	m_pb_callback.resolve_safe();
	m_ca2_callback.resolve_safe();
	m_cb2_callback.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void exidy_sh8253_sound_device::device_start()
{
	common_sh_start();

	/* 8253 */
	m_freq_to_step = (1 << 24) / SH8253_CLOCK;

	sh8253_register_state_globals();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void exidy_sh8253_sound_device::device_reset()
{
	common_sh_reset();

	/* 8253 */
	memset(m_sh8253_timer, 0, sizeof(m_sh8253_timer));
}


WRITE8_MEMBER(venture_sound_device::pa_w)
{
	m_pia->write_porta(data);
}


WRITE8_MEMBER(venture_sound_device::pb_w)
{
	m_pia->write_portb(data);
}


WRITE_LINE_MEMBER(venture_sound_device::ca_w)
{
	m_pia->ca1_w(state);
}


WRITE_LINE_MEMBER(venture_sound_device::cb_w)
{
	m_pia->cb1_w(state);
}


WRITE8_MEMBER(venture_sound_device::pia_pa_w)
{
	m_pa_callback(data);
}


WRITE8_MEMBER(venture_sound_device::pia_pb_w)
{
	m_pb_callback(data);
}


WRITE_LINE_MEMBER(venture_sound_device::pia_ca2_w)
{
	m_ca2_callback(state);
}


WRITE_LINE_MEMBER(venture_sound_device::pia_cb2_w)
{
	m_cb2_callback(state);
}


void venture_sound_device::venture_audio_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x007f).mirror(0x0780).ram();
	map(0x0800, 0x087f).mirror(0x0780).rw("riot", FUNC(riot6532_device::read), FUNC(riot6532_device::write));
	map(0x1000, 0x1003).mirror(0x07fc).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1800, 0x1803).mirror(0x07fc).w(FUNC(venture_sound_device::sh8253_w));
	map(0x2000, 0x27ff).w(FUNC(venture_sound_device::filter_w));
	map(0x2800, 0x2807).mirror(0x07f8).rw(FUNC(venture_sound_device::sh6840_r), FUNC(venture_sound_device::sh6840_w));
	map(0x3000, 0x3003).mirror(0x07fc).w(FUNC(venture_sound_device::sfxctrl_w));
	map(0x5800, 0x7fff).rom();
}


void venture_sound_device::device_add_mconfig(machine_config &config)
{
	m6502_device &audiocpu(M6502(config, "audiocpu", 3579545/4));
	audiocpu.set_addrmap(AS_PROGRAM, &venture_sound_device::venture_audio_map);

	RIOT6532(config, m_riot, SH6532_CLOCK);
	m_riot->in_pa_callback().set(FUNC(venture_sound_device::r6532_porta_r));
	m_riot->out_pa_callback().set(FUNC(venture_sound_device::r6532_porta_w));
	m_riot->in_pb_callback().set(FUNC(venture_sound_device::r6532_portb_r));
	m_riot->out_pb_callback().set(FUNC(venture_sound_device::r6532_portb_w));
	m_riot->irq_callback().set("audioirq", FUNC(input_merger_device::in_w<0>));

	PIA6821(config, m_pia, 0);
	m_pia->writepa_handler().set(FUNC(venture_sound_device::pia_pa_w));
	m_pia->writepb_handler().set(FUNC(venture_sound_device::pia_pb_w));
	m_pia->ca2_handler().set(FUNC(venture_sound_device::pia_ca2_w));
	m_pia->cb2_handler().set(FUNC(venture_sound_device::pia_cb2_w));
	m_pia->irqb_handler().set("audioirq", FUNC(input_merger_device::in_w<1>));

	INPUT_MERGER_ANY_HIGH(config, "audioirq").output_handler().set_inputline("audiocpu", m6502_device::IRQ_LINE); // open collector

	SPEAKER(config, "mono").front_center();

	this->add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*************************************
 *
 *  CVSD sound for Mouse Trap
 *
 *************************************/

DEFINE_DEVICE_TYPE(EXIDY_MTRAP, mtrap_sound_device, "mtrap_sound", "Exidy SFX+PSG+CVSD")

mtrap_sound_device::mtrap_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: venture_sound_device(mconfig, EXIDY_MTRAP, tag, owner, clock)
{
}

WRITE8_MEMBER(mtrap_sound_device::voiceio_w)
{
	if (!(offset & 0x10))
		m_cvsd->digit_w(data & 1);

	if (!(offset & 0x20))
		m_riot->portb_in_set(data & 1, 0xff);
}


READ8_MEMBER(mtrap_sound_device::voiceio_r)
{
	if (!(offset & 0x80))
	{
		uint8_t porta = m_riot->porta_out_get();
		uint8_t data = (porta & 0x06) >> 1;
		data |= (porta & 0x01) << 2;
		data |= (porta & 0x08);
		return data;
	}

	if (!(offset & 0x40))
		return m_cvsd->clock_state_r() << 7;

	return 0;
}


void mtrap_sound_device::cvsd_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).rom().region("cvsdcpu", 0);
}


void mtrap_sound_device::cvsd_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).rw(FUNC(mtrap_sound_device::voiceio_r), FUNC(mtrap_sound_device::voiceio_w));
}


void mtrap_sound_device::device_add_mconfig(machine_config &config)
{
	venture_sound_device::device_add_mconfig(config);

	Z80(config, m_cvsdcpu, CVSD_Z80_CLOCK);
	m_cvsdcpu->set_addrmap(AS_PROGRAM, &mtrap_sound_device::cvsd_map);
	m_cvsdcpu->set_addrmap(AS_IO, &mtrap_sound_device::cvsd_iomap);

	/* audio hardware */
	MC3417(config, m_cvsd, CVSD_CLOCK).add_route(ALL_OUTPUTS, "mono", 0.80);
}


/*************************************
 *
 *  Victory
 *
 *************************************/

#define VICTORY_AUDIO_CPU_CLOCK     (XTAL(3'579'545) / 4)
#define VICTORY_LOG_SOUND           0



READ8_MEMBER(victory_sound_device::response_r)
{
	uint8_t ret = m_pia->b_output();

	if (!machine().side_effects_disabled())
	{
		if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound response read = %02X\n", machine().describe_context(), ret);

		m_pia_cb1 = 0;
		m_pia->cb1_w(m_pia_cb1);
	}

	return ret;
}


READ8_MEMBER(victory_sound_device::status_r)
{
	uint8_t ret = (m_pia_ca1 << 7) | (m_pia_cb1 << 6);

	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound status read = %02X\n", machine().describe_context(), ret);

	return ret;
}


TIMER_CALLBACK_MEMBER(victory_sound_device::delayed_command_w)
{
	m_pia->write_porta(param);
	m_pia_ca1 = 0;
	m_pia->ca1_w(m_pia_ca1);
}

WRITE8_MEMBER(victory_sound_device::command_w)
{
	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound command = %02X\n", machine().describe_context(), data);

	machine().scheduler().synchronize(timer_expired_delegate(FUNC(victory_sound_device::delayed_command_w), this), data);
}


WRITE_LINE_MEMBER(victory_sound_device::irq_clear_w)
{
	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound IRQ clear = %02X\n", machine().describe_context(), state);

	if (!state)
	{
		m_pia_ca1 = 1;
		m_pia->ca1_w(m_pia_ca1);
	}
}


WRITE_LINE_MEMBER(victory_sound_device::main_ack_w)
{
	if (VICTORY_LOG_SOUND) logerror("%s:!!!! Sound Main ACK W = %02X\n", machine().describe_context(), state);

	if (m_victory_sound_response_ack_clk && !state)
	{
		m_pia_cb1 = 1;
		m_pia->cb1_w(m_pia_cb1);
	}

	m_victory_sound_response_ack_clk = state;
}


DEFINE_DEVICE_TYPE(EXIDY_VICTORY, victory_sound_device, "victory_sound", "Exidy SFX+PSG+Speech")

victory_sound_device::victory_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: exidy_sh8253_sound_device(mconfig, EXIDY_VICTORY, tag, owner, clock)
	, m_victory_sound_response_ack_clk(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victory_sound_device::device_start()
{
	save_item(NAME(m_victory_sound_response_ack_clk));
	save_item(NAME(m_pia_ca1));
	save_item(NAME(m_pia_cb1));

	exidy_sh8253_sound_device::device_start();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victory_sound_device::device_reset()
{
	exidy_sh8253_sound_device::device_reset();

	/* the flip-flop @ F4 is reset */
	m_victory_sound_response_ack_clk = 0;
	m_pia_cb1 = 1;
	m_pia->cb1_w(m_pia_cb1);

	/* these two lines shouldn't be needed, but it avoids the log entry
	   as the sound CPU checks port A before the main CPU ever writes to it */
	m_pia->write_porta(0);
	m_pia_ca1 = 1;
	m_pia->ca1_w(m_pia_ca1);
}


void victory_sound_device::victory_audio_map(address_map &map)
{
	map(0x0000, 0x00ff).mirror(0x0f00).ram();
	map(0x1000, 0x107f).mirror(0x0f80).rw("riot", FUNC(riot6532_device::read), FUNC(riot6532_device::write));
	map(0x2000, 0x2003).mirror(0x0ffc).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x3000, 0x3003).mirror(0x0ffc).w(FUNC(victory_sound_device::sh8253_w));
	map(0x4000, 0x4fff).noprw();
	map(0x5000, 0x5007).mirror(0x0ff8).rw(FUNC(victory_sound_device::sh6840_r), FUNC(victory_sound_device::sh6840_w));
	map(0x6000, 0x6003).mirror(0x0ffc).w(FUNC(victory_sound_device::sfxctrl_w));
	map(0x7000, 0xafff).noprw();
	map(0xb000, 0xffff).rom();
}


void victory_sound_device::device_add_mconfig(machine_config &config)
{
	m6502_device &audiocpu(M6502(config, "audiocpu", VICTORY_AUDIO_CPU_CLOCK));
	audiocpu.set_addrmap(AS_PROGRAM, &victory_sound_device::victory_audio_map);

	RIOT6532(config, m_riot, SH6532_CLOCK);
	m_riot->in_pa_callback().set(FUNC(victory_sound_device::r6532_porta_r));
	m_riot->out_pa_callback().set(FUNC(victory_sound_device::r6532_porta_w));
	m_riot->in_pb_callback().set(FUNC(victory_sound_device::r6532_portb_r));
	m_riot->out_pb_callback().set(FUNC(victory_sound_device::r6532_portb_w));
	m_riot->irq_callback().set("audioirq", FUNC(input_merger_device::in_w<0>));

	PIA6821(config, m_pia, 0);
	m_pia->ca2_handler().set(FUNC(victory_sound_device::irq_clear_w));
	m_pia->cb2_handler().set(FUNC(victory_sound_device::main_ack_w));
	m_pia->irqb_handler().set("audioirq", FUNC(input_merger_device::in_w<1>));

	INPUT_MERGER_ANY_HIGH(config, "audioirq").output_handler().set_inputline("audiocpu", m6502_device::IRQ_LINE); // open collector

	SPEAKER(config, "mono").front_center();

	this->add_route(ALL_OUTPUTS, "mono", 1.0);

	TMS5220(config, m_tms, 640000).add_route(ALL_OUTPUTS, "mono", 1.0);
}
